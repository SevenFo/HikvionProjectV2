#include "hikvisonviewer.h"
#include <QtWidgets/QWidget>
#include <QtQuickWidgets/QQuickWidget>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QMediaDevices>
#include <models.h>
#include <vector>
#include <QQuickItem>
/*
 * [todo]
 * 1. 画面声音不同步，原因是audio_sink的播放有一段缓冲时间，即需要等待缓冲完成？后才能播放，这样声音就会比画面延迟2.5秒左右
 * 2. 添加人连检测结果展示界面（左下角？）
 * 3. ...
 */


HikvisonViewer::HikvisonViewer(QQuickItem *parent):IMageViewer(parent)
{

    hik = new HikvisonHandler(); // init hikvison
    audiohandler = new audio_handler();
    image_handler = new ImageHandler();

    audio_handler_thread = new QThread(this);
    image_handler_thread =  new QThread(this);

    hik->moveToThread(&this->hikvisonThread);
    audiohandler->moveToThread(audio_handler_thread); //等调试通过之后在放到新的线程里面
    image_handler->moveToThread(image_handler_thread);

    timerDisplay = new QTimer(this);
    timerDisplay->setInterval(1000/30); // 设置刷新频率30Hz

    inferAudioTimer = new QTimer(this);
    inferAudioTimer->setInterval(1000); //1s推理一次


    delayPlayAudioTimer = new QTimer(this);
    delayPlayAudioTimer->setInterval(2000);
    delayPlayAudioTimer->setSingleShot(true);


    // 暂时固定参数，后面可以自行更改
    audio_device = new AudioDevice(8000,16,this);
    audio_device->set_ready_duration(6);


    QAudioFormat af;
    af.setChannelCount(1);
    af.setSampleRate(8000);
    af.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice info(QMediaDevices::defaultAudioOutput());


    if (!info.isFormatSupported(af)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }


    audio_sink = new QAudioSink(info,af,this);

    connect(timerDisplay,&QTimer::timeout,this,&HikvisonViewer::DisplayImage);
    connect(audio_device, &AudioDevice::ready,audiohandler, &audio_handler::doInferate,Qt::QueuedConnection);
    connect(this, &HikvisonViewer::getSnapedImage,image_handler,&ImageHandler::handle_image,Qt::QueuedConnection);

    //处理hikvisonhandler发给来的人脸照片的原始数据（yuv转pixmap）并发给qml显示出来
//    connect(hik, &HikvisonHandler::getFaceImage, this,[this](const QByteArray &rawData,const qsizetype score, QSize image_size){
//        cv::Mat yuvPic,jpg_mat;
//        yuvPic.create(image_size.height()*1.5,image_size.width(),CV_8UC1); // rows x cols
//        qDebug()<<rawData.size() <<","<<yuvPic.rows*yuvPic.cols;
//        memcpy(yuvPic.data,rawData.data(),rawData.size());
//        cv::cvtColor(yuvPic,jpg_mat,cv::COLOR_YUV2BGR_YV12);
////        jpg_mat = cv::imread("pic.jpg");
//        image = QImage(jpg_mat.data,image_size.width(),image_size.height(),QImage::Format_BGR888);
//        emit getSnapedImage(QPixmap::fromImage(image),image.width(), image.height());
//    });


    typedef void (HikvisonHandler::*signaltype_getFaceImage_1)(const QPixmap &);
    signaltype_getFaceImage_1 signal_getFaceImage_1 = &HikvisonHandler::getFaceImage;
    connect(hik, signal_getFaceImage_1, this,[this](const QPixmap &pmap){
        emit getSnapedImage(pmap,pmap.width(), pmap.height());
    });

    typedef void (audio_handler::*inferated_1)(std::vector<float>);
    inferated_1 infrated1 = &audio_handler::inferatFinished;
    connect(audiohandler,infrated1,this,[=](std::vector<float> result){
        qDebug()<<result;
    },Qt::QueuedConnection);


    connect(delayPlayAudioTimer, &QTimer::timeout,this,[=](){
        audio_sink->setVolume(0.7);
//        audio_sink->setBufferSize(170000);
        audio_device->fill(QByteArray(640,0));
        qInfo()<<"audio sink buffer size:"<<audio_sink->bufferSize();
        audio_sink->start(audio_device);
        inferAudioTimer->start();
    });

    connect(inferAudioTimer,&QTimer::timeout, this, [=](){
        audio_device->getLatestData(6);
    });

    audio_handler_thread->start();
    image_handler_thread->start();
    hikvisonThread.start();

}
HikvisonViewer::~HikvisonViewer()
{
    inferAudioTimer->stop();
    audio_handler_thread->quit();
    audio_handler_thread->wait();
    hikvisonThread.quit();
    hikvisonThread.wait();
    image_handler_thread->quit();
    image_handler_thread->wait();
}

void HikvisonViewer::loginAndSetup()
{
    if(!isLogin())
    {
        qDebug()<<"setting up hivison";
        hik->SetupCamera();
    }
    else
    {
        //login out
    }
    emit loginStateChanged();

}

void HikvisonViewer::startDecode()
{
    if(!isOpenVideo())
    {
        if(!hik->SetupRealPlayV2())
            return;
        timerDisplay->start();   //开始播放
        delayPlayAudioTimer->start();

        emit openVideoStateChange();
    }
    else
    {
        //off real play
        if(!hik->StopRealPlay())
            return;

        timerDisplay->stop();
        audio_sink->stop();
        emit openVideoStateChange();
    }

}

void HikvisonViewer::setHost(QString &d)
{
    hik->host = d;
    this->hik->port = d;
    qDebug()<<"set host:"<<d;
}

void HikvisonViewer::setPort(QString &d)
{
    hik->port = d;
    qDebug()<<"set port:"<<d;

}
void HikvisonViewer::setUserid(QString &d)
{
    hik->userid = d;
    qDebug()<<"set userid:"<<d;

}
void HikvisonViewer::setPassword(QString &d)
{
    hik->password = d;
    qDebug()<<"set password:"<<d;

}
void HikvisonViewer::setLoginState(bool &d)
{
    //do nothing
}
void HikvisonViewer::setOpenVideoState(bool d)
{

}

void HikvisonViewer::DisplayImage()
{

    auto images = hik->rawImages;
    auto audios = hik->audioDatas;

    if(images->size()-30*2>0)
    {
        cv::Mat jpg_mat;
        QByteArray tmp = images->takeFirst();

        cv::Mat yuvPic;
        yuvPic.create(576*1.5,704,CV_8UC1); // rows x cols
        memcpy(yuvPic.data,tmp.data(),tmp.size());
        cv::cvtColor(yuvPic,jpg_mat,cv::COLOR_YUV2BGR_YV12);

        image = QImage(jpg_mat.data,704,576,QImage::Format_BGR888);
        this->pixmap = QPixmap::fromImage(image);
        this->updateImage();

    }

    // audio
    while(audios->size())
    {
        audio_device->fill(audios->takeFirst());

    }

}

QString HikvisonViewer::getHost() const
{
    return hik->host;
}
QString HikvisonViewer::getPort() const
{
    return hik->port;
}
QString HikvisonViewer::getUserid() const
{
    return hik->userid;
}
QString HikvisonViewer::getPassword() const
{
    return hik->password;
}
bool HikvisonViewer::isLogin() const{
    return hik->IsLogin();
}
bool HikvisonViewer::isOpenVideo() const{
    qDebug()<<"is open video:"<<hik->hasStartedDecode;
    return hik->hasStartedDecode;
}




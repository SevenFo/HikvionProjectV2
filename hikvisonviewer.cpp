#include "hikvisonviewer.h"
#include <QtWidgets/QWidget>
#include <QtQuickWidgets/QQuickWidget>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QMediaDevices>
#include <models.h>
#include <vector>
#include <QQuickItem>
#include <algorithm>
/*
 * [todo]
 * 1. 画面声音不同步，原因是audio_sink的播放有一段缓冲时间，即需要等待缓冲完成？后才能播放，这样声音就会比画面延迟2.5秒左右
 * 2. 添加人连检测结果展示界面（左下角？）
 * 3. ...
 */


HikvisonViewer::HikvisonViewer(QQuickItem *parent):IMageViewer(parent)
{

    this->setFocus(true);
    hik = new HikvisonHandler(); // init hikvison
    audiohandler = new audio_handler();
    image_handler = new FaceImageHandler();
    can_handler = new CANControler();
    cardata_handler = new CarDataHandler();

    audio_handler_thread = new QThread(this);
    image_handler_thread =  new QThread(this);
    can_handler_thread = new QThread(this);
    cardata_handler_thread = new QThread(this);


    hik->moveToThread(&this->hikvisonThread);
    audiohandler->moveToThread(audio_handler_thread); //等调试通过之后在放到新的线程里面
    image_handler->moveToThread(image_handler_thread);
    can_handler->moveToThread(can_handler_thread);
    cardata_handler->moveToThread(cardata_handler_thread);

    timerDisplay = new QTimer(this); //视频显示定时器
    timerDisplay->setInterval(1000/30); // 设置刷新频率30Hz

    inferAudioTimer = new QTimer(this); //定时前向推理定时器
    inferAudioTimer->setInterval(1000); //1s推理一次


    delayPlayAudioTimer = new QTimer(this); //延迟2s播放音频，同步
    delayPlayAudioTimer->setInterval(2000);
    delayPlayAudioTimer->setSingleShot(true);

    fakeCanSendTimer = new QTimer(this);
    fakeCanSendTimer->setInterval(70); //freq: 20Hz

    predictFaceImageTimer = new QTimer(this);
    predictFaceImageTimer->setInterval(200); //freq: 10Hz

    checkDangerValueTimer = new QTimer(this);
    checkDangerValueTimer->setInterval(1000);

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

    wb_ = new xlnt::workbook();


    //can init
//    if(!can_handler->openCAN()){
//        qWarning()<<"open can device failed";
//    }



    connect(timerDisplay,&QTimer::timeout,this,&HikvisonViewer::DisplayImage);
    connect(audio_device, &AudioDevice::ready,audiohandler, &audio_handler::doInferate,Qt::QueuedConnection);
    connect(predictFaceImageTimer, &QTimer::timeout,this,&HikvisonViewer::OnPredictFaceImageTimerOut,Qt::AutoConnection);
    connect(this, &HikvisonViewer::runFacePrediction, image_handler, &FaceImageHandler::run, Qt::QueuedConnection);
//    connect(this, &HikvisonViewer::runCanThread, can_handler, &CANControler::run, Qt::QueuedConnection);
    connect(can_handler,&CANControler::getProtocolData,this,&HikvisonViewer::OnGetProtocolData);

    // 读取xlsx文件
    wb_->load("../构建数据_带四种场景.xlsx");
    ws_ = new xlnt::worksheet(wb_->sheet_by_index(1));

    auto ClearAllSpace = [](std::string str){
        int index = 0;
        if (!str.empty()) {
            //string::npos 表示查找没有匹配
            while((index = str.find(' ', index)) != std::string::npos) {
                str.erase(index, 1);
            }
        }
        return str;
    };

    connect(this, &HikvisonViewer::runCanThread, this,[=](){fakeCanSendTimer->start();});
    connect(this, &HikvisonViewer::stopCanThread, can_handler,&CANControler::stop);
    connect(this, &HikvisonViewer::stopCanThread, fakeCanSendTimer,&QTimer::stop);

    //转发snap face信号到UI（HikvisonViwer）
//    typedef void (HikvisonHandler::*signaltype_getFaceImage_1)(const QPixmap &);
//    signaltype_getFaceImage_1 signal_getFaceImage_1 = &HikvisonHandler::getFaceImage;
//    connect(hik, signal_getFaceImage_1, this,[this](const QPixmap &pmap){
//        emit getSnapedImage(pmap.scaled(pmap.width()/2,pmap.height()/2),pmap.width()/2, pmap.height()/2);
//    });
    connect(image_handler,&FaceImageHandler::handleImageFinished, this,[this](const QPixmap &pmap, int is_tired){
       emit getSnapedImage(pmap,pmap.width(), pmap.height());
        emit isTired(is_tired);
        if (is_tired){
            is_tired_value = 1;
        }
        else{
            is_tired_value = 0;
        }
    });
    //转发表情预测结果到UI
    connect(image_handler,&FaceImageHandler::predictFaceExprerationFinished,this,[=](QString label,int index, float p, QVector<float> ps){
        if(p<0.9){
            label = QString("中性");
            index = 6;
        }
        emit getFaceExprationResult(label,p);


        switch(index){
        case 0:case 1:case 2:case 4:case 5:
//            danger_value += (danger_value+3>200?0:3);
            driver_level = 1;
            break;
        default:
//            danger_value -= (danger_value-2>0?2:0);
            driver_level = 0;
            break;
        }
        qDebug()<<"face expration results:"<<ps;
    });


    //转发音频预测结果到UI（HikvionViwer）
    typedef void (audio_handler::*inferated_1)(std::vector<float>);
    inferated_1 infrated1 = &audio_handler::inferatFinished;
    connect(audiohandler,infrated1,this,[=](std::vector<float> result){
        qDebug()<<result;
        QVector<QString> labels{"爆炸","枪声","争吵、尖叫","玻璃碎裂","正常"};
        auto max_index = max_element(result.begin(), result.end());
        qDebug()<<"max index: "<<max_index-result.begin()<<". value: "<<*max_index <<". event name: "<<labels.at(max_index-result.begin());
        auto index_ = max_index-result.begin();
        if(*max_index < 0.6 && index_!= 2){
            index_ = 4;
        }
        switch(index_){
        case 0:case 1: case 3:
//            danger_value += (danger_value+30>200? 0: 30);
            sound_level = 3;
            break;
        case 2:
            sound_level = 2;
            break;
        default:
            sound_level = 0;
//            danger_value -= (danger_value-20<0? 0: 20);
            break;
        }
        emit getAudioInferedResult(labels.at(index_));
    },Qt::QueuedConnection);

    //延迟2s播放音频（OneShot）
    connect(delayPlayAudioTimer, &QTimer::timeout,this,[=](){
        audio_sink->setVolume(0.7);
//        audio_sink->setBufferSize(170000);
        audio_device->fill(QByteArray(640,0));
        qInfo()<<"audio sink buffer size:"<<audio_sink->bufferSize();
        audio_sink->start(audio_device);
        inferAudioTimer->start();
        qDebug()<<"run can thread!";
        emit runCanThread();
    });

    //定时进行音频预测
    connect(inferAudioTimer,&QTimer::timeout, this, [=](){
        audio_device->getLatestData(6);
    });


    //虚拟Can发送器
    connect(fakeCanSendTimer,&QTimer::timeout, this, [=](){
        //从某个地方读取数据并且发送给自己
#ifdef TRUE_SEND
        auto speed_data = std::stoull(ClearAllSpace(((ws_->rows()[fake_data_row_counter_]))[0].to_string()),0,16); // speed string data to hex raw data
        auto speed_id = 0x0039;
        auto steering_wheel_data = std::stoull(ClearAllSpace(((ws_->rows()[fake_data_row_counter_]))[4].to_string()),0,16);
        auto steering_wheel_id = 0x0158;
        auto brake_angle_data = std::stoull(ClearAllSpace(((ws_->rows()[fake_data_row_counter_++]))[8].to_string()),0,16);
        auto brake_angle_id = 0x0188;

        can_handler->sendData(speed_id,(unsigned char*)&speed_data);
        can_handler->sendData(steering_wheel_id,(unsigned char*)&steering_wheel_data);
        can_handler->sendData(brake_angle_id,(unsigned char*)&brake_angle_data);
#else
        auto speed_data = std::stoull(ClearAllSpace(((ws_->rows()[fake_data_row_counter_]))[2].to_string()),0,10);
        auto steering_angle_data = std::stoull(ClearAllSpace(((ws_->rows()[fake_data_row_counter_]))[6].to_string()),0,10);
        auto brake_angle_data = std::stoull(ClearAllSpace(((ws_->rows()[fake_data_row_counter_]))[10].to_string()),0,10);
        cardata_handler->addSpeed(speed_data,80);
        if (is_speedup)
            fake_data_row_counter_++;
        emit getSpeed(speed_data);
        cardata_handler->addSteeringAngle(steering_angle_data,10);
        emit getSteeringAngle(steering_angle_data);
        cardata_handler->addBrakeAngle(brake_angle_data,10);
        emit getBrakeAngle(brake_angle_data);
//        list_car_speeds.append(speed_data);
//        list_car_steering_angles.append(steering_angle_data);
//        list_car_brake_angles.append(brake_angle_data);
        qDebug()<<"speed: "<<speed_data<<", steering angle:"<<steering_angle_data<<", brake angle:"<<brake_angle_data;
#endif
    });

    connect(cardata_handler, &CarDataHandler::overspeed,this,[=](CarDataHandler::OverSpeedLevel level){
//        switch(level){
//        case CarDataHandler::LOW:
//            danger_value += (danger_value+1>200? 0: 1);
//        case CarDataHandler::MIDDLE:
//            danger_value += (danger_value+2>200? 0: 2);
//            break;
//        case CarDataHandler::HIGH:
//            danger_value += (danger_value+3>200? 0: 3);
//            break;
//        }
    });

    connect(checkDangerValueTimer,&QTimer::timeout, this, [=](){
      CheckDangerValue();
      qDebug()<<"danger value:"<<danger_value;
    });


    audio_handler_thread->start();
    image_handler_thread->start();
    can_handler_thread->start();
    hikvisonThread.start();

}
HikvisonViewer::~HikvisonViewer()
{
    predictFaceImageTimer->stop();
    inferAudioTimer->stop();
    fakeCanSendTimer->stop();
    can_handler_thread->quit();
    can_handler_thread->wait();
    audio_handler_thread->quit();
    audio_handler_thread->wait();
    hikvisonThread.quit();
    hikvisonThread.wait();
    image_handler_thread->quit();
    image_handler_thread->wait();
}

void HikvisonViewer::keyPressEvent(QKeyEvent *event)
{
    auto key = event->key();
    switch (key) {
        case Qt::Key_L:
    {
        is_speedup = !is_speedup;
        break;
    }
    default:{
        break;
    }

    }
}


void HikvisonViewer::loginAndSetup()
{
    if(!isLogin())
    {
        qDebug()<<"set up hivison";
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
        predictFaceImageTimer->start();

        delayPlayAudioTimer->start();

        checkDangerValueTimer->start();

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

void HikvisonViewer::OnPredictFaceImageTimerOut()
{

    auto images = hik->rawImages;

    if(images->size()-30*2>0)
    {
        cv::Mat jpg_mat;
        QByteArray tmp = images->at(0);

        cv::Mat yuvPic;
        yuvPic.create(576*1.5,704,CV_8UC1); // rows x cols
        memcpy(yuvPic.data,tmp.data(),tmp.size());
        cv::cvtColor(yuvPic,jpg_mat,cv::COLOR_YUV2BGR_YV12);

        auto myimage = QImage(jpg_mat.data,704,576,QImage::Format_BGR888).scaled(QSize(960,540));
        emit runFacePrediction(QPixmap::fromImage(myimage),myimage.width(),myimage.height());
    }
}

void HikvisonViewer::OnGetProtocolData(VCI_CAN_OBJ *vci,DWORD dwRel){
    //todo
    printf("CAN%d RX ID:0x%08X", 0, vci->ID);//ID
    if(vci->ExternFlag==0) printf(" Standard ");//帧格式：标准帧
    if(vci->ExternFlag==1) printf(" Extend   ");//帧格式：扩展帧
    if(vci->RemoteFlag==0) printf(" Data   ");//帧类型：数据帧
    if(vci->RemoteFlag==1) printf(" Remote ");//帧类型：远程帧
    printf("DLC:0x%02X",vci->DataLen);//帧长度
    printf(" data:0x");	//数据
    for(int i = 0; i < vci->DataLen; i++)
    {
        printf(" %02X", vci->Data[i]);
    }
    printf(" TimeStamp:0x%08X",vci->TimeStamp);//时间标识。
    printf("\n");
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




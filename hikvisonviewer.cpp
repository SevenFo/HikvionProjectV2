#include "hikvisonviewer.h"
#include <QtWidgets/QWidget>
#include <QtQuickWidgets/QQuickWidget>

HikvisonViewer::HikvisonViewer(QQuickItem *parent)
{
    hik = new HikvisonHandler(); // init hikvison
    qDebug()<<"inited hikvision!";
    hik->moveToThread(&this->hikvisonThread);
    qDebug()<<"move hikvison to thread";

    timerDisplay = new QTimer(parent);
    timerDisplay->setInterval(40); // 设置刷新频率

//    void (HikvisonViewer::*updateImage2)(QImage image) = &HikvisonViewer::updateImage;
//    connect(hik,&HikvisonHandler::ReceivedData,this,&HikvisonViewer::DisplayImage,Qt::QueuedConnection);
    connect(hik,&HikvisonHandler::DisplayDecodedImages,this,&HikvisonViewer::DisplayImages,Qt::QueuedConnection);
    connect(timerDisplay,&QTimer::timeout,this,&HikvisonViewer::DisplayImage);

    connect(hik,&HikvisonHandler::StartDisplayDecodedImages,[=](){
        std::cout << "start display"<<std::endl;
       timerDisplay->start();   //开始播放
       std::cout << "displaying "<<std::endl;

    });

}
HikvisonViewer::~HikvisonViewer()
{

}

void HikvisonViewer::loginAndSetup()
{
    qDebug()<<"setting up hivison";
    hik->SetupCamera();
}

void HikvisonViewer::startDecode()
{
    hik->SetupRealPlayV2();

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

void HikvisonViewer::DisplayImage()
{

    std::cout << "timer" << std::endl;
    auto images = hik->rawImages;
#define TEST
#ifdef TEST
    cv::Mat frame = cv::imread("/home/siky/QtProjects/dog.jpg");
    cv::Mat frame_yv12;
    cv::Mat frame_rgb;

    cv::cvtColor(frame,frame_yv12,cv::COLOR_RGB2YUV_YV12);

    auto frame_yv12_size = frame_yv12.total() * frame_yv12.elemSize();
    QByteArray tmp_yuv;
    tmp_yuv.resize(frame_yv12_size);
    memcpy(tmp_yuv.data(),frame_yv12.data,frame_yv12_size);
    images->append(tmp_yuv);
#endif

    if(images->size())
    {
        cv::Mat jpg_mat;
        QByteArray tmp = images->takeFirst();

        cv::Mat yuvPic;
        yuvPic.create(576*1.5,768,CV_8UC1); // rows x cols
        memcpy(yuvPic.data,tmp.data(),tmp.size());
        cv::cvtColor(yuvPic,jpg_mat,cv::COLOR_YUV2RGB_YV12);

        image = QImage(jpg_mat.data,768,576,QImage::Format_BGR888);
        this->pixmap = QPixmap::fromImage(image);
        this->updateImage();
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

void HikvisonViewer::DisplayImages(QVector<QByteArray> *images)
{
    // 改用定时器的模式，以25Hz播放
    QBuffer *buf = new QBuffer();
    buf->open(QBuffer::ReadWrite);
    while(true)
    {
        qDebug()<<"size:"<<images->size();
        QThread::msleep(10);
        if(images->size())
        {
            qDebug()<<"take first image";
            cv::Mat jpg_mat;

            QByteArray tmp = images->takeFirst();
    //        mutex.unlock();
            HikvisonHandler::yuv2jpg(tmp,jpg_mat,704,576);

            image = QImage(jpg_mat.data,jpg_mat.cols,jpg_mat.rows,QImage::Format_BGR888);
            cv::waitKey(1);
            image.bits();
            this->updateImage();
            qDebug()<<"buf size:"<<buf->size();
//            this->UpdateImage(buf);
        }
    }
}


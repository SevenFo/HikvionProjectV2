#include "detector.h"
#include <QDebug>
#include <QThread>

Detector::Detector(QObject *parent) : QObject(parent)
{

}
Detector::~Detector()
{

}
bool Detector::Init()
{
//    return yolox.LoadModel(".\\yolox_s.mge");
    return 0;
}


bool Detector::Detect(QImage &input_img)
{

    cv::Mat mat = cv::Mat(input_img.height(), input_img.width(), CV_8UC3, (void*)input_img.constBits(), input_img.bytesPerLine());
    cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
    cv::Mat result;
//    yolox.Inference(mat,result);
    return true;
}
bool Detector::Detect(cv::Mat &input_img)
{

    cv::Mat result;
    cv::Mat threeChannel = cv::Mat(input_img.rows,input_img.cols,CV_8UC3);
    cv::cvtColor(input_img,threeChannel,cv::COLOR_RGBA2BGR);
//    qDebug()<<"channel "<<threeChannel.channels();
//    cv::imshow("jpg pic",threeChannel);
//    cv::waitKey();
//    yolox.Inference(threeChannel,result);
    return true;
}
void Detector::InferredImages(QVector<QImage> &images, QMutex &mutex, QWaitCondition &condi)
{
    QImage image;
    while(runInferredImages)
    {
        mutex.lock();
        while(!images.size())
        {
            qDebug()<<"no image to infer, wait...";
            condi.wait(&mutex);
        }
        qDebug()<<"take first image";
        image = images.takeFirst();
        mutex.unlock();
        Detect(image);
    }
}

/*
 * params: QVector<QByteArray> &images 存储YUVpics 的数组
 * */
void Detector::InferredImages(QVector<QByteArray> *images, QMutex *mutex, QWaitCondition *condi)
{
    QByteArray image;
    while(runInferredImages)
    {
//        qDebug()<<"loop begin @"<<QThread::currentThread();
//        if(!mutex.tryLock(6000))
//            qDebug()<<"lock failed";
//        qDebug()<<"loop run infferred images, length:"<<images.size();
//        while(!images.size())
//        {
//            qDebug()<<"no image to infer, wait...";
////            mutex.lock();
//            condi.wait(&mutex);
//        }
        qDebug()<<"size:"<<images->size();
        QThread::msleep(1);
        if(images->size())
        {
            qDebug()<<"take first image";
            image = images->takeFirst();
    //        mutex.unlock();
            cv::Mat jpgmat;
            Detector::_yuv2jpg(image,&jpgmat,704,576);

            Detect(jpgmat);
        }
    }
}


void Detector::_yuv2jpg(QByteArray &yuvPicData, cv::Mat *jpg, int w, int h)
{
    qDebug()<<"yuv2jpg: "<<yuvPicData.size();
    cv::Mat yuvPic;
    yuvPic.create(h*3.0/2.0,w,CV_8UC1);
    memcpy_s(yuvPic.data,yuvPicData.size(),yuvPicData.data(),yuvPicData.size());
    cv::cvtColor(yuvPic,*jpg,cv::COLOR_YUV2BGRA_YV12);

}

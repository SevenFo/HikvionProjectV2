#ifndef DETECTOR_H
#define DETECTOR_H

#undef foreach
//#include "yolox.h"
#include <config_parser.h>
#include <object_detector.h>
#include <config_parser.h>
#include <paddle_inference_api.h>
#include <QObject>
#include <QImage>
#include <QVector>
#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>




class Detector : public QObject
{
    Q_OBJECT

public:
    explicit Detector(QObject *parent = nullptr);
    virtual ~Detector();

    bool runInferredImages;

    bool Init();
    bool Detect(QImage &input_img);
    bool Detect(cv::Mat &input_img);

    void InferredImages(QVector<QImage> & images,QMutex &mutex, QWaitCondition &condi);
    void InferredImages(QVector<QByteArray> *images, QMutex *mutex, QWaitCondition *condi);

    static void _yuv2jpg(QByteArray &yuvPicData, cv::Mat *jpg, int w, int h);


private:
//    Yolox yolox;

signals:
    void DetectResult(QImage result);
};

#endif // DETECTOR_H

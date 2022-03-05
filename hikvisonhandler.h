#ifndef HIKVISONHANDLER_H
#define HIKVISONHANDLER_H
#define NOMINMAX
#ifdef __linux__
typedef int LONG ;
typedef unsigned char* PBYTE;
#else
typedef long LONG ;
#include "windows.h"
#endif
//#include "detector.h"
#include "PlayM4.h"
#include "HCNetSDK.h"
#include "thread"
#include "hikvisonhandlercstyelfuncs.h"
#include "opencv2/opencv.hpp"

#include <QObject>
#include <QWidget>
#include <QXmlStreamReader>
#include <QObject>
#include <QByteArray>
#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTimer>
#include <QDebug>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

//extern void TestCFun();
class HikvisonHandler : public QObject
{
    Q_OBJECT
public:

    HikvisonHandler(QObject *parent = nullptr);
    virtual ~HikvisonHandler();

    bool SetupCamera();//初始化相机、播放器
    bool SetupFaceDet();
    bool SetupPlayer();
    bool SetupRealPlay(WId winId = NULL);
    /*
     * 开启摄像头的实时预览功能，使用类内部回调函数
     * params
     * */
    bool SetupRealPlayV2(WId winId = NULL);

    bool StopRealPlay();
    bool GetSnapedFaceImg(QByteArray *dist);
    bool StartDecode();
    /*
     * 将摄像头的实时流数据填充到M4解码器中，并在回调函数中处理解码数据
     * params
     * */
    bool StartDecodeV2();
    bool StopDecoding();
    bool IsLogin();
    inline bool HasSetupRealPlay(){
        return hasSetupRealPlay;
    }


    bool hasStartedDecode;
    const int MAX_AUDIO_BUFFER = 100; // 100*0.1s = 10s

    QString host,port,userid,password;

    QByteArray streamHeadDataBuf,audioDataBuf,streamBodyDataBuf;

    QVector<QByteArray> *audioDatas; // 保存解码得到的音频数据，每个ByteArray代码一帧，大约为:0.1s
    QVector<QByteArray> *rawImages; // 保存的是YUV格式的图片，需要先转成JPG在送入模型
    QVector<QByteArray> *inferredImages;
    QMutex rawImagesMutex;
    QWaitCondition rawImagesCondi;


    QThread inferenceThread;

//    Detector detector;

    static void CallbackRealData(LONG lReanHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *that);
    static void yuv2jpg(std::vector<char> &yuv, cv::Mat &jpg, int w, int h);
    static void yuv2jpg(QByteArray &yuvPicData, cv::Mat &jpg, int w, int h);
    static void yuv2jpg(QByteArray &yuvPicData, QImage *jpg, int w, int h);

    static void CallbackDecodedData(LONG nPort, char *pBuf, LONG nSize, FRAME_INFO *pFrameInfo, void *nUser, int nReserved2);
    static void CallbackFaceDetAlarm(LONG lCommand,NET_DVR_ALARMER *pAlarmer,char *pAlarmInfo,DWORD dwBufLen,void *pUser);

private:
    LONG userID;
    LONG playPort;
    LONG realPlayID;
    bool isLogin;
    bool hasSetupRealPlay;
signals:
    void getFaceImage(const QByteArray &rawData,const qsizetype score, QSize image_size);
    void getFaceImage(const QPixmap &pmap);
    void StartInputStreamData();
    void GetAudioData();
    void ReceivedData(QVector<QByteArray> *images);

    void StartDisplayDecodedImages();


};




#endif // HIKVISONHANDLER_H

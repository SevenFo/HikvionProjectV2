#ifndef HIKVISONHANDLERCSTYELFUNCS_H
#define HIKVISONHANDLERCSTYELFUNCS_H
#define LINUX
#ifndef LINUX
#include "windows.h"
#endif
#include "PlayM4.h"
#include "HCNetSDK.h"

#include "QMutex"
#include <QWaitCondition>
#include <QList>
#include <QByteArray>
#include <QByteArray>
#include <QThread>
#include <QFile>
#include <QTime>

extern "C"{

extern void RealDataCallback(LONG lReanHandle, DWORD dwDataType,BYTE *pBuffer,DWORD dwBufSize, void *pUser);//实时预览的回调函数
extern void FaceDetAlarmCallback(  LONG               lCommand,
                            NET_DVR_ALARMER    *pAlarmer,
                            char               *pAlarmInfo,
                            DWORD              dwBufLen,
                            void               *pUser);//人脸抓拍-预警回调函数
extern void DecodedDataCallback(LONG nPort, char *pBuf, LONG nSize, FRAME_INFO *pFrameInfo, void *nUser, void *nReserved2);//解码回调函数

extern void TestCFun();

extern unsigned char * streamHeadDataBuff;
extern unsigned int streamHeadDataBuffSize;

extern unsigned char * streamDataBuff;
extern unsigned int streamDataBuffSize;


extern unsigned char * snapedFaceImgBuff;
extern unsigned int snapedFaceImgBuffSize;

extern unsigned char * decodedImgBuff;
extern unsigned int decodedImgBuffSize;

extern unsigned char * decodedAudioBuff;
extern unsigned int decodedAudioBuffSize;

extern bool havNewSnapedFaceImg;
extern bool hasStartDecode;
extern bool hasNewStreamData;
extern bool hasNewAudioData;
extern bool checkerReady;
extern bool callbackReady;
extern bool audioCheckerReady;
enum CALLBACK_TYPE{STREAM_RAW_DATA_CB,DECODED_AUDIO_DATA_CB};

extern enum CALLBACK_TYPE callbackType;



}
extern QMutex dataMutex;
extern QMutex audioDataMutex;
extern QWaitCondition streamDataCondi;
extern QWaitCondition streamNewDataCondi;
extern QList<QByteArray> snapedFaceImgs;
extern QList<QByteArray> decodedImgs;
extern QList<QByteArray> audioDatas;

extern QMutex snapedFaceImgMutex;
extern QWaitCondition snapedFaceImgFullCondi;
extern QWaitCondition snapedFaceImgEmptyCondi;

extern QMutex decodedImgMutex;
extern QWaitCondition decodedImgFullCondi;
extern QWaitCondition decodedImgEmptyCondi;

extern QWaitCondition audioDataFullCondi;
extern QWaitCondition audioDataEmptyCondi;


#endif // HIKVISONHANDLERCSTYELFUNCS_H

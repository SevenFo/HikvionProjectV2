#include "hikvisonhandlercstyelfuncs.h"
#include "opencv2/opencv.hpp"
#include "vector"

unsigned char * streamHeadDataBuff = NULL;
unsigned int streamHeadDataBuffSize = 0;
unsigned char * streamDataBuff = NULL;
unsigned int streamDataBuffSize = 0;
unsigned char * snapedFaceImgBuff = NULL;
unsigned int snapedFaceImgBuffSize = 0;
unsigned char * decodedImgBuff = NULL;
unsigned int decodedImgBuffSize = 0;
unsigned char * decodedAudioBuff = NULL;
unsigned int decodedAudioBuffSize = 0;

QList<QByteArray> snapedFaceImgs;//max size is 50
QList<QByteArray> decodedImgs;//max size is 50
QList<QByteArray> audioDatas;

bool havNewSnapedFaceImg = false;
bool hasStartDecode = false;
bool hasNewStreamData = false;
bool hasNewAudioData = false;
bool checkerReady = true;
bool audioCheckerReady = true;



enum CALLBACK_TYPE callbackType;

QMutex dataMutex;
QMutex audioDataMutex;
QMutex snapedFaceImgMutex;
QWaitCondition streamDataCondi;
QWaitCondition streamNewDataCondi;
QWaitCondition newAudioDataCondi;
QWaitCondition audioDataCondi;
QWaitCondition snapedFaceImgFullCondi;
QWaitCondition snapedFaceImgEmptyCondi;

QMutex decodedImgMutex;
QWaitCondition decodedImgFullCondi;
QWaitCondition decodedImgEmptyCondi;

QWaitCondition audioDataFullCondi;
QWaitCondition audioDataEmptyCondi;

/*
 * 实时预览数据的的回调函数，收到decoder函数的影响，当decoder速度比较慢的时候将被睡眠
 * */
void RealDataCallback(LONG lReanHandle, DWORD dwDataType,BYTE *pBuffer,DWORD dwBufSize, void *pUser)//实时预览的回调函数
{
    switch (dwDataType) {
    case NET_DVR_SYSHEAD:{
        qDebug()<<"system head date";
        streamHeadDataBuff =  (unsigned char*)malloc(dwBufSize); //分配内存空间
        memcpy(streamHeadDataBuff,pBuffer,dwBufSize);
        streamHeadDataBuffSize = dwBufSize;
        break;
    }
    case NET_DVR_STREAMDATA:{
        callbackType = STREAM_RAW_DATA_CB;
        while(!checkerReady){
            //等待checker上锁 wait，因为callback有可能抢在checker lock之前得到mutex，会导致checker无法被唤醒
        }
        dataMutex.lock();
        streamDataBuff =  (unsigned char*)malloc(dwBufSize); //分配内存空间
        memcpy(streamDataBuff,pBuffer,dwBufSize);
        streamDataBuffSize = dwBufSize;
        if(hasStartDecode)
        {
            streamNewDataCondi.notify_one();//得到新的数据，唤起发送信号的线程
            streamDataCondi.wait(&dataMutex);//施放锁，等待input后唤起主线程
        }
        dataMutex.unlock();
        free(streamDataBuff);
        break;
    }
    case NET_DVR_AUDIOSTREAMDATA:{qDebug()<<"audio date";break;}
    case NET_DVR_PRIVATE_DATA:{qDebug()<<"private date";break;}
    default:{qDebug()<<"unknown date";break;}
    }
}
void FaceDetAlarmCallback(  LONG lCommand,NET_DVR_ALARMER *pAlarmer,char *pAlarmInfo,DWORD dwBufLen,void *pUser)
{
    NET_VCA_FACESNAP_RESULT struFacenapResult;
    NET_DVR_FACE_DETECTION struFaceDetection;
    QFile f = QFile("pic.jpg");
    switch(lCommand)
    {
    //人脸抓拍
    case COMM_UPLOAD_FACESNAP_RESULT:{
        memcpy(&struFacenapResult,pAlarmInfo,((NET_VCA_FACESNAP_RESULT*)pAlarmInfo)->dwSize);
        snapedFaceImgMutex.lock();
        qDebug()<<QThread::currentThreadId()<<":dwFaceScore:"<<struFacenapResult.dwFaceScore<<"dwFacePicLen:"<<struFacenapResult.dwFacePicLen<<"byUploadEventDataType:"<<struFacenapResult.byUploadEventDataType<<"fStayDuration:"<<struFacenapResult.fStayDuration<<"dwBackgroundPicLen:"<<struFacenapResult.dwBackgroundPicLen<<"struRect: width:"<<struFacenapResult.struRect.fWidth<<"height:"<<struFacenapResult.struRect.fHeight<<"x:"<<struFacenapResult.struRect.fX<<"y:"<<struFacenapResult.struRect.fY;
        if(snapedFaceImgs.size()<150)
            snapedFaceImgs.append(QByteArray((char *)(((NET_VCA_FACESNAP_RESULT*)pAlarmInfo)->pBuffer1),((NET_VCA_FACESNAP_RESULT*)pAlarmInfo)->dwFacePicLen));
        else
        {
            qDebug()<<"snaped img full! wait deal";
            snapedFaceImgFullCondi.wait(&snapedFaceImgMutex);
        }
        snapedFaceImgMutex.unlock();
        snapedFaceImgEmptyCondi.notify_one();//消费者可能处于等待状态（将buff消耗完了）
        break;
    //人脸侦测
    case COMM_ALARM_FACE_DETECTION:{

        break;
        }
    }
    default:
    {
        qDebug()<<"use less msg";
        break;
    }
    }
}
void DecodedDataCallback(LONG nPort, char *pBuf, LONG nSize, FRAME_INFO *pFrameInfo, void *nUser, int nReserved2)//解码回调函数
{
//    qDebug()<<"copied video data info: ";
//    qDebug()<<"width: "<<pFrameInfo->nWidth<<" height:"<<pFrameInfo->nHeight<<" rate:"<<pFrameInfo->nFrameRate<<" frameNumber:"<<pFrameInfo->dwFrameNum<<" type:"<<pFrameInfo->nType <<"stamp:"<<pFrameInfo->nStamp;
    switch(pFrameInfo->nType)
    {
    case T_AUDIO16:{
        audioDataMutex.lock();
//        qDebug()<<QThread::currentThreadId()<<":dwFaceScore:"<<struFacenapResult.dwFaceScore<<"dwFacePicLen:"<<struFacenapResult.dwFacePicLen<<"byUploadEventDataType:"<<struFacenapResult.byUploadEventDataType<<"fStayDuration:"<<struFacenapResult.fStayDuration<<"dwBackgroundPicLen:"<<struFacenapResult.dwBackgroundPicLen<<"struRect: width:"<<struFacenapResult.struRect.fWidth<<"height:"<<struFacenapResult.struRect.fHeight<<"x:"<<struFacenapResult.struRect.fX<<"y:"<<struFacenapResult.struRect.fY;
        if(audioDatas.size()<20)
        {
            QByteArray audioData = QByteArray();
//            memcpy(imgData.data(),&pFrameInfo->nHeight,4);
//            memcpy(imgData.data()+4,&pFrameInfo->nWidth,4);
            audioData.append(QByteArray((char *)(pBuf),nSize));
            audioDatas.append(audioData);
        }
        else
        {
            qDebug()<<"audio data full! wait deal";
            audioDataFullCondi.wait(&audioDataMutex);
        }
        audioDataMutex.unlock();
        audioDataEmptyCondi.notify_one();//消费者可能处于等待状态（将buff消耗完了）
        break;
    }
    case  T_YV12:{
        decodedImgMutex.lock();
//        qDebug()<<QThread::currentThreadId()<<":dwFaceScore:"<<struFacenapResult.dwFaceScore<<"dwFacePicLen:"<<struFacenapResult.dwFacePicLen<<"byUploadEventDataType:"<<struFacenapResult.byUploadEventDataType<<"fStayDuration:"<<struFacenapResult.fStayDuration<<"dwBackgroundPicLen:"<<struFacenapResult.dwBackgroundPicLen<<"struRect: width:"<<struFacenapResult.struRect.fWidth<<"height:"<<struFacenapResult.struRect.fHeight<<"x:"<<struFacenapResult.struRect.fX<<"y:"<<struFacenapResult.struRect.fY;
        if(decodedImgs.size()<20)
        {
            QByteArray imgData = QByteArray(8,0);
            memcpy(imgData.data(),&pFrameInfo->nHeight,4);
            memcpy(imgData.data()+4,&pFrameInfo->nWidth,4);
            imgData.append(QByteArray((char *)(pBuf),nSize));
            decodedImgs.append(imgData);
        }
        else
        {
            qDebug()<<"decoded img full! wait deal";
            decodedImgFullCondi.wait(&decodedImgMutex);
        }
//        qDebug()<<"size:"<<decodedImgs.size();
        decodedImgMutex.unlock();
        decodedImgEmptyCondi.notify_one();//消费者可能处于等待状态（将buff消耗完了）
        break;
    }
    default:
    {
        qDebug()<<"other data";
        break;
    }
    }
}
void TestCFun()
{
    printf("hello\n");
}

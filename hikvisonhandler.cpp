#include "hikvisonhandler.h"
#include "opencv2/opencv.hpp"

const int IMAGE_BUFFER_SIZE = 24*5;

HikvisonHandler::HikvisonHandler(QObject *parent):
    QObject(parent),playPort(0),hasSetupRealPlay(false),runCheckStreamData(true),runCheckSnapedFaceImg(true),
    runCheckDecodedImgData(true),runCheckAudioData(true)
{

    snapedFaceImgData =  new QByteArray(); //存储相机SDK捕捉到的脸部图像
    rawImages = new QVector<QByteArray>(); //原始图片
    inferredImages = new QVector<QByteArray>(); //处理过的图片


//****************初始化解码器**********************
    if(!PlayM4_GetPort(&playPort))
    {
        qDebug()<<"get player port failed error code:"<<PlayM4_GetLastError(playPort);
        return;
    }
    qDebug()<<"player port:"<<playPort;

    if(!PlayM4_SetStreamOpenMode(playPort,0))//此模式（默认）下, 会尽量保正实时性, 防止数据阻塞; 而且数据检查严格
    {
        qDebug()<<"set stream mode failed error code:"<<PlayM4_GetLastError(playPort);
        return;
    }
    qDebug()<<"setup player success";


}
HikvisonHandler::~HikvisonHandler()
{
    runCheckDecodedImgData = false;
    runCheckSnapedFaceImg =  false;
    runCheckStreamData = false;
    runCheckAudioData = false;
    checkAudioDataThread->join();
    checkImgDataThread->join();
    checkSnapedFaceImgThread->join();
    checkStreamDataThread->join();
}


void HikvisonHandler::SetupCamera()
{
    //*****************初始化相机SDK并登录*****************
    NET_DVR_Init();
    NET_DVR_SetExceptionCallBack_V30(0,NULL,NULL,NULL);

    NET_DVR_USER_LOGIN_INFO struLoginInfo;  //登入信息结构体
    memset(&struLoginInfo,0,sizeof(struLoginInfo));
    qDebug()<<"host:"<<this->host;
    strcpy(struLoginInfo.sDeviceAddress,this->host.toLocal8Bit().data());
    struLoginInfo.wPort=8000;
    strcpy(struLoginInfo.sUserName ,this->userid.toLocal8Bit().data()); //设备登录用户名
    strcpy(struLoginInfo.sPassword,this->password.toLocal8Bit().data()); //设备登录密码

    NET_DVR_DEVICEINFO_V40 struDeviceInfo;//设备信息(同步登录即pLoginInfo中bUseAsynLogin为0时有效)
    memset(&struDeviceInfo,0,sizeof(struDeviceInfo));


    userID = NET_DVR_Login_V40(&struLoginInfo,&struDeviceInfo);

    if(userID == -1)
    {
        qDebug()<<"login failed!";
        return;
    }
    //    qDebug()<<"userid: "<<userID<<" device type:"<<struDeviceInfo.struDeviceV30.wDevType<<" chanNumber:"<<struDeviceInfo.struDeviceV30.byChanNum<<" chanStart"<<struDeviceInfo.struDeviceV30.byStartChan<<" subproto"<<struDeviceInfo.struDeviceV30.bySubProto;


//    // 获取设别编码能力参数
//    QFile infile = QFile("in.xml",this);
//    if(!infile.open(QIODevice::ReadOnly)){
//        qDebug()<<"open file failed";
//    }
//    QFile outfile = QFile("out.text",this);
//    outfile.open(QIODevice::WriteOnly);
//    char *poutBuffer = (char *)malloc(10240);
//    QByteArray inbuffer = QByteArray(infile.readAll());
//    qDebug()<<"inbuffer"<<QString(inbuffer.data());
//    qDebug()<<"in size:"<<inbuffer.size();

//    if(!NET_DVR_GetDeviceAbility(userID,DEVICE_ENCODE_ALL_ABILITY_V20,inbuffer.data(),inbuffer.size(),poutBuffer,10240))
//    {
//        qDebug()<<"NET_DVR_GetDeviceAbility error code:"<<NET_DVR_GetLastError();
//    }
//    else
//    {
//        outfile.write(poutBuffer);
//    }

//    infile.close();
//    outfile.close();

    qDebug()<<"setup camera success";
    return;
}

/*
 * 开启实时预览
 * param: 被设置为预览窗口的窗口句柄
*/
bool HikvisonHandler::SetupRealPlay(WId winId)
{
    std::cout<<"setting up real play..."<<std::endl;

    NET_DVR_PREVIEWINFO struPreviewInfo;//实时预览的参数
    struPreviewInfo.dwStreamType = 1;//主流 大分辨率
    struPreviewInfo.dwLinkMode = 0;//TCP
    struPreviewInfo.hPlayWnd = (HWND)winId;//解码显示的预览窗口
    struPreviewInfo.bBlocked = false;
    struPreviewInfo.lChannel = 1;
    realPlayID = NET_DVR_RealPlay_V40(userID,&struPreviewInfo,&HikvisonHandler::CallbackRealData,this);//设置数据回调函数
    if(realPlayID == -1)
    {
        std::cout<<"open real play failed, error code:"<<NET_DVR_GetLastError()<<std::endl;
        hasSetupRealPlay = false;
        return false;
    }
    else
    {
        qDebug()<<"open real stream success";
        hasSetupRealPlay = true;
        return true;
    }

}


/*
 * 开启实时预览，回调函数封装在类中
 * callback: CallbackRealData 处理接相机送来的实时数据
 * param: 被设置为预览窗口的窗口句柄
*/
bool HikvisonHandler::SetupRealPlayV2(WId winId)
{
    qDebug()<<"setting up real play...";

    NET_DVR_PREVIEWINFO struPreviewInfo;//实时预览的参数
    struPreviewInfo.dwStreamType = 1;//主流 大分辨率
    struPreviewInfo.dwLinkMode = 0;//TCP
    struPreviewInfo.hPlayWnd = (HWND)winId;//解码显示的预览窗口
    struPreviewInfo.bBlocked = false;
    struPreviewInfo.lChannel = 1;
    realPlayID = NET_DVR_RealPlay_V40(userID,&struPreviewInfo,&HikvisonHandler::CallbackRealData,this);//设置数据回调函数
#define TEST
#ifdef TEST
    emit StartDisplayDecodedImages();
    hasStartedDecode = true;;
#endif
    if(realPlayID == -1)
    {
        qDebug()<<"open real play failed, error code:"<<NET_DVR_GetLastError();
        hasSetupRealPlay = false;
        return false;
    }
    else
    {
        qDebug()<<"open real stream success";
        hasSetupRealPlay = true;
        return true;
    }


}

/*
 * 停止实时预览
*/
bool HikvisonHandler::StopRealPlay()
{
    auto result = NET_DVR_StopRealPlay(realPlayID);
    if(result == -1)
    {
        std::cout<<"NET_DVR_StopRealPlay failed, error code:"<<NET_DVR_GetLastError()<<std::endl;
//        hasSetupRealPlay = true;
        return false;
    }
    else
    {
        hasSetupRealPlay = false;
        return true;
    }
}



/*
 * 调用M4Player解码相机放回的数据，需要先传入数据流的头数据
 * callback: CallbackDecodedData 处理解码完成的数据
 * emit: DisplayDecodedImages 通知界面开始显示图片，传入缓冲区
*/
bool HikvisonHandler::StartDecodeV2()
{

    qDebug()<<"start decode v2";
    if(!streamHeadDataBuf.size())
    {
        qDebug()<<"no stream head data";
        return false;
    }
    else
        qDebug()<<"stream head data:"<<streamHeadDataBuf.size();
    if(!PlayM4_OpenStream(playPort,(PBYTE)streamHeadDataBuf.data(),streamHeadDataBuf.size(),SOURCE_BUF_MAX))
    {
        qDebug()<<"PlayM4_OpenStream failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }

    if(!PlayM4_Play(playPort,NULL))
    {
        qDebug()<<"PlayM4_Play failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }

    if(!PlayM4_SetDecCallBackExMend(playPort,&HikvisonHandler::CallbackDecodedData,NULL,0,this))
    {
        qDebug()<<"PlayM4_SetDecCallBackExMend failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }

    if(!PlayM4_PlaySound(playPort))
    {
        qDebug()<<"PlayM4_PlaySound failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }
    if(!PlayM4_SetVolume(playPort,0))
    {
        qDebug()<<"PlayM4_SetVolume failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }
    qDebug()<<"started docoder v2";


//    qDebug()<<"try to run inferred images";
//    detector.runInferredImages=true;
//    emit RunInferredImages(rawImages,&rawImagesMutex,&rawImagesCondi);
    emit DisplayDecodedImages(rawImages);

    hasStartedDecode = true;;
    return true;
}

void HikvisonHandler::CallbackRealData(LONG lReanHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *that)
{
    switch (dwDataType) {
    case NET_DVR_SYSHEAD:{
        qDebug()<<"system head date v2";
        ((HikvisonHandler*)that)->streamHeadDataBuf = QByteArray((char *)pBuffer,dwBufSize);
        qDebug()<<"head data size:"<<((HikvisonHandler*)that)->streamHeadDataBuf.size();
        streamHeadDataBuff = pBuffer;
        streamHeadDataBuffSize = dwBufSize;
        //向播放器填充头数据，开始解码
        ((HikvisonHandler*)that)->StartDecodeV2();
//        ((HikvisonHandler*)that)->StartDecode();
        break;
    }
    case NET_DVR_STREAMDATA:{
        //接收到视频流数据之后直接填充到播放器中，如果开始解码则填充数据
        callbackType = STREAM_RAW_DATA_CB;
//        qDebug()<<"stream date";
        ((HikvisonHandler*)that)->streamBodyDataBuf = QByteArray((char *)pBuffer,dwBufSize);
        if(((HikvisonHandler*)that)->hasStartedDecode)
        {
            if(!PlayM4_InputData(((HikvisonHandler*)that)->playPort,(PBYTE)((HikvisonHandler*)that)->streamBodyDataBuf.data(),((HikvisonHandler*)that)->streamBodyDataBuf.size()))
            {
                qDebug()<<"input stream data failed error code:"<<PlayM4_GetLastError(((HikvisonHandler*)that)->playPort);
                return;
            }
            else
            {
            }
        }
        else
        {
            qDebug()<<"not started";
        }
        break;
    }
    case NET_DVR_AUDIOSTREAMDATA:{qDebug()<<"audio date";break;}
    case NET_DVR_PRIVATE_DATA:{qDebug()<<"private date";break;}
    default:{qDebug()<<"unknown date";break;}
    }
}

void HikvisonHandler::yuv2jpg(std::vector<char> &yuv, cv::Mat &jpg, int w, int h)
{
    cv::Mat yuvPic;
    yuvPic.create(h*1.5,w,CV_8UC3);
    cv::cvtColor(yuvPic,jpg,cv::COLOR_YUV2BGR_YV12);

}
/*
 * Convet yuvImage (QByteArray) to jpg (cv::Mat)
 * */
void HikvisonHandler::yuv2jpg(QByteArray &yuvPicData, cv::Mat &jpg, int w, int h)
{
    cv::Mat yuvPic;
    yuvPic.create(h*1.5,w,CV_8UC1);
    memcpy(yuvPic.data,yuvPicData.data(),yuvPicData.size());
    cv::cvtColor(yuvPic,jpg,cv::COLOR_YUV2BGR_YV12);
}

/*
 * Convet yuvImage (QByteArray) to jpg (QImage)
 * */
void HikvisonHandler::yuv2jpg(QByteArray &yuvPicData, QImage *jpg, int w, int h)
{
    cv::Mat yuvPic,cvjpg;
    yuvPic.create(h*1.5,w,CV_8UC1);
    memcpy(yuvPic.data,yuvPicData.data(),yuvPicData.size());
    cv::cvtColor(yuvPic,cvjpg,cv::COLOR_YUV2RGB_YV12);
//    cv::imshow("jgpBGR",cvjpg);
    cv::waitKey(5);
    qDebug()<<"conveted jpg dims:"<<cvjpg.dims <<" type"<<cvjpg.type();
    auto tjpg = QImage(cvjpg.data,cvjpg.cols,cvjpg.rows,cvjpg.step,QImage::Format_RGB888);
    qDebug()<<"qimage size:"<<tjpg.size();
    *jpg = QImage(tjpg);
    jpg->bits();

}

void HikvisonHandler::CallbackDecodedData(LONG nPort, char *pBuf, LONG nSize, FRAME_INFO *pFrameInfo, void *nUser, int nReserved2)//解码回调函数
{
    HikvisonHandler *that = (HikvisonHandler*)nUser;

    switch(pFrameInfo->nType)
    {
    case T_AUDIO16:{
        that->audioDataBuf = QByteArray((char *)(pBuf),nSize);
        break;
    }
    case  T_YV12:{
        QByteArray tmp = QByteArray((char *)pBuf,nSize);//读取数据到QByteArray中

//        QMutexLocker locker(&that->rawImagesMutex);//lock
//        qDebug()<<"locking mutex @:"<<QThread::currentThread();

//        that->rawImagesMutex.lock();
        if(that->rawImages->size()<IMAGE_BUFFER_SIZE)
        {
            qDebug()<<"add an image";
            that->rawImages->append(tmp);
//            that->rawImagesMutex.unlock();
            that->rawImagesCondi.notify_all();
        }
//        that->rawImagesMutex.unlock();
        break;
    }
    default:
    {
        qDebug()<<"other data";
        break;
    }
    }
    return ;
}





























//****************以下都是旧版本的函数******************


/*
 * 检查是否有新的流数据，如有新的数据，则调用playerAPI对数据进行解码并显示
 * */
void HikvisonHandler::CheckStreamData()
{
    qDebug()<<"run CheckStreamData thread:"<<QThread::currentThreadId() <<runCheckStreamData;
    streamDataCondi.wakeAll();
    while(runCheckStreamData)
    {
        dataMutex.lock();
        checkerReady = true;
        streamNewDataCondi.wait(&dataMutex);//释放锁，等待新数据
        checkerReady = false;
//        emit HasNewData((char *)streamDataBuff,streamDataBuffSize);//得到新数据，发射信号
        if(!PlayM4_InputData(playPort,(PBYTE)streamDataBuff,streamDataBuffSize))
        {
            qDebug()<<"input stream data failed error code::"<<PlayM4_GetLastError(playPort);
            return;
        }
        dataMutex.unlock();
        //此时，callback还没lock，checker unlock，没有人拥有mutex
        streamDataCondi.notify_one();
    }
    qDebug()<<"stop stream checker";
}

void HikvisonHandler::CheckAudioData()
{
    qDebug()<<"run thread:"<<QThread::currentThreadId();
//    audioDataCondi.wakeAll();
    while(runCheckAudioData)
    {
        audioDataMutex.lock();
        while(audioDatas.isEmpty())
        {
//            qDebug()<<"snaped face img buff is empty wait";
           audioDataEmptyCondi.wait(&audioDataMutex);
        }
        //have data
        QByteArray data = audioDatas.takeFirst();
        audioDataMutex.unlock();
        emit HasNewAudioData(data);
        audioDataFullCondi.notify_one();
        //通知完过后，回调函数线程被唤醒，但要等待check线程wait之后他才能lock（反正回调函数执行过快，在checkaudiowait之前就notify），才能通知checkemit
        //正常应该是若缓冲区是满的，则callback wati等待emit，如果缓冲区是空的，说明check执行的比callback快，就没必要wait了
        //对于check来说，若缓冲区是空的，则等待，非则就不需要等待
        //在这里是不管怎么样check都等待，即默认callback的速度比checker慢
    }
    qDebug()<<"stop audio checker";
}

void HikvisonHandler::CheckImgData()
{
    qDebug()<<"run thread:"<<QThread::currentThreadId();
//    audioDataCondi.wakeAll();
    while(runCheckDecodedImgData)
    {
        decodedImgMutex.lock();
        while(decodedImgs.isEmpty())
        {
           decodedImgEmptyCondi.wait(&decodedImgMutex);
        }
        //have data
        QByteArray data = decodedImgs.takeFirst();
        decodedImgMutex.unlock();
        emit HasNewDecodedImgData(data);
        decodedImgFullCondi.notify_one();
    }
    qDebug()<<"stop img checker";
}

void HikvisonHandler::CheckSnapedFaceImg()
{
    while(runCheckSnapedFaceImg)
    {
        snapedFaceImgMutex.lock();
        while(snapedFaceImgs.isEmpty())
        {
            snapedFaceImgEmptyCondi.wait(&snapedFaceImgMutex);
        }
        //have data
        auto data = snapedFaceImgs.takeFirst();
        snapedFaceImgMutex.unlock();
        emit HasNewSnapedFaceImg(data);
        snapedFaceImgFullCondi.notify_one();
    }
    qDebug()<<"stop snap checker";
}

// 丢弃
/*
bool HikvisonHandler::StartDecode()
{
    checkStreamDataThread = new std::thread(std::bind(&HikvisonHandler::CheckStreamData,this));//检查是否有新的流数据，并对其进行处理
    checkAudioDataThread = new std::thread(std::bind(&HikvisonHandler::CheckAudioData,this));
    checkImgDataThread = new std::thread(std::bind(&HikvisonHandler::CheckImgData,this));

    connect(this,&HikvisonHandler::HasNewData,[=](char * data,size_t size){
        //有新的数据接收到，填充到播放器中
       if(!PlayM4_InputData(playPort,(PBYTE)data,size))
       {
           qDebug()<<"input stream data failed error code::"<<PlayM4_GetLastError(playPort);
           return;
       }
    });

    if(!streamHeadDataBuffSize)//检查是否接收到数据头
    {
        qDebug()<<"pls open real play firstly";
        return false;
    }

//    if(!PlayM4_OpenStream(playPort,streamHeadDataBuff,streamHeadDataBuffSize,SOURCE_BUF_MAX))
//    {
//        qDebug()<<"PlayM4_OpenStream failed error code:"<<PlayM4_GetLastError(playPort);
//        return false;
//    }


//    if(!PlayM4_Play(playPort,NULL))
//    {
//        qDebug()<<"PlayM4_Play failed error code:"<<PlayM4_GetLastError(playPort);
//        return false;
//    }

    if(!PlayM4_SetDecCallBackExMend(playPort,&DecodedDataCallback,NULL,0,0))
    {
        qDebug()<<"PlayM4_SetDecCallBackExMend failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }

    if(!PlayM4_PlaySound(playPort))
    {
        qDebug()<<"PlayM4_PlaySound failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }
    if(!PlayM4_SetVolume(playPort,0))
    {
        qDebug()<<"PlayM4_SetVolume failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }
    qDebug()<<"decoder checked";

    hasStartDecode = true;;
    return true;
}
*/

/*
 * 初始化人脸检测的API （准备丢弃）
 * */
bool HikvisonHandler::SetupFaceDet()
{
    //需要先打开实时预览
    if(!hasSetupRealPlay)
    {
        qDebug()<<"pls setup real play firstly";
        return false;
    }
    //注册回调消息回调函数
    if(!NET_DVR_SetDVRMessageCallBack_V50(1,&FaceDetAlarmCallback,NULL))
        return false;

    //警示消息的参数
    NET_DVR_SETUPALARM_PARAM *struSetupAlarmParam = new NET_DVR_SETUPALARM_PARAM();
    memset(struSetupAlarmParam,0,struSetupAlarmParam->dwSize);
    struSetupAlarmParam->dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);
    struSetupAlarmParam->byLevel = 0;
    struSetupAlarmParam->byFaceAlarmDetection = 0;
    struSetupAlarmParam->byAlarmTypeURL = 0;//二进制传输
    //打开通道
    auto alarmChanHandler = NET_DVR_SetupAlarmChan_V41(userID,struSetupAlarmParam);
    delete struSetupAlarmParam;

    if(alarmChanHandler == -1)
    {
        qDebug()<<"SetupAlarmChan failed, error code:"<<NET_DVR_GetLastError();
        return false;
    }

    qDebug()<<"seted face det"<<QThread::currentThreadId();
    checkSnapedFaceImgThread = new std::thread(std::bind(&HikvisonHandler::CheckSnapedFaceImg,this));//在新的线程中检查是否有新的人脸抓拍图像
    return true;
}

//？？？
bool HikvisonHandler::GetSnapedFaceImg(QByteArray *dist)
{
    if(!havNewSnapedFaceImg)
    {
        return false;
    }
    dist->clear();
    dist->append(*snapedFaceImgData);
    return true;
}

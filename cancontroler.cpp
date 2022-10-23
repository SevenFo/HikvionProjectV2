#include "cancontroler.h"
#include "controlcan.h" //linux
#include <QTime>
#include <QCoreApplication>
#include <QMetaType>
#include <string.h>
VCI_BOARD_INFO vbi;
CANControler::CANControler(QObject *parent) : QObject{parent},
    deviceType(VCI_USBCAN2),
    debicIndex(0),
    debicCom(0),
    baundRate(125)
{
    stopped = false;
    qRegisterMetaType<VCI_CAN_OBJ>("VCI_CAN_OBJ");
    qRegisterMetaType<DWORD>("DWORD");
}
CANControler::~CANControler(){

}

void CANControler::stop()
{
    stopped = true;
}

bool CANControler::openCAN()
{
    int nDeviceType = deviceType; /* USBCAN-2A或USBCAN-2C或CANalyst-II */
    int nDeviceInd = debicIndex; /* 第1个设备 */
    int nCANInd = debicCom; /* 第1个通道 */
    DWORD dwRel;
    dwRel = VCI_OpenDevice(nDeviceType, nDeviceInd, 0); //打开设备
    if(dwRel != 1)
    {
//        qDebug()<<"open fail:"<< MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
//        qDebug()<<"open success";

    dwRel = VCI_ClearBuffer(nDeviceType, nDeviceInd, nCANInd);
    dwRel = VCI_ClearBuffer(nDeviceType, nDeviceInd, nCANInd+1);
    VCI_INIT_CONFIG vic;
//    vic.AccCode=0x80000008;
    vic.AccCode=0;
    vic.AccMask=0xFFFFFFFF;
    vic.Filter=1;
    vic.Mode=0;
    switch (baundRate) {
    case 10:
        vic.Timing0=0x31;
        vic.Timing1=0x1c;
        break;
    case 20:
        vic.Timing0=0x18;
        vic.Timing1=0x1c;
        break;
    case 40:
        vic.Timing0=0x87;
        vic.Timing1=0xff;
        break;
    case 50:
        vic.Timing0=0x09;
        vic.Timing1=0x1c;
        break;
    case 80:
        vic.Timing0=0x83;
        vic.Timing1=0xff;
        break;
    case 100:
        vic.Timing0=0x04;
        vic.Timing1=0x1c;
        break;
    case 125:
        vic.Timing0=0x03;
        vic.Timing1=0x1c;
        break;
    case 200:
        vic.Timing0=0x81;
        vic.Timing1=0xfa;
        break;
    case 250:
        vic.Timing0=0x01;
        vic.Timing1=0x1c;
        break;
    case 400:
        vic.Timing0=0x80;
        vic.Timing1=0xfa;
        break;
    case 500:
        vic.Timing0=0x00;
        vic.Timing1=0x1c;
        break;
    case 666:
        vic.Timing0=0x80;
        vic.Timing1=0xb6;
        break;
    case 800:
        vic.Timing0=0x00;
        vic.Timing1=0x16;
        break;
    case 1000:
        vic.Timing0=0x00;
        vic.Timing1=0x14;
        break;
    case 33:
        vic.Timing0=0x09;
        vic.Timing1=0x6f;
        break;
    case 66:
        vic.Timing0=0x04;
        vic.Timing1=0x6f;
        break;
    case 83:
        vic.Timing0=0x03;
        vic.Timing1=0x6f;
        break;
    default:
        break;
    }
    dwRel = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd, &vic);
    dwRel = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd+1, &vic);
    if(dwRel !=1)
    {
//        qDebug()<<"init fail:"<<MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
//        qDebug()<<"init success";


    dwRel = VCI_ReadBoardInfo(nDeviceType, nDeviceInd, &vbi);
    if(dwRel != 1)
    {
//        qDebug()<<"get dev message fail:"<<MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
    {
        qDebug()<<"CAN通道数："<<vbi.can_Num;
        qDebug()<<"硬件版本号:"<<vbi.hw_Version;
        qDebug()<<"接口库版本号："<<vbi.in_Version;
        qDebug()<<"中断号"<<vbi.irq_Num;
    }

    if(VCI_StartCAN(nDeviceType, nDeviceInd, nCANInd) !=1)
    {
//        qDebug()<<"start"<<nCANInd<<"fail:"<<MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
        qDebug()<<"start"<<nCANInd<<"success:";

    int comid = nCANInd+1;
    if(VCI_StartCAN(nDeviceType, nDeviceInd, comid) !=1)
    {
//        qDebug()<<"start"<<comid<<"fail:"<<MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
        qDebug()<<"start"<<comid<<"success:";

    return true;
}

void CANControler::closeCAN()
{
    VCI_CloseDevice(deviceType, debicIndex);
}

void CANControler::sendData(int ID,unsigned char *ch)
{
    int count = strlen((char*)ch);
    int temp1 = count /8; // 1
    int temp2 = count %8; // 0
    if(temp2>0)
        count = temp1+1;
    else
        count = temp1;
    int dwRel;
    VCI_CAN_OBJ vco[3];
    int i=0;
    for(;i<temp1;i++)
    {
        vco[i].ID =ID ;
        vco[i].RemoteFlag = 0;
        vco[i].ExternFlag = 0;
        vco[i].DataLen = 8;
        for(int j = 0;j<8;j++)
            vco[i].Data[j] = ch[j+(i*8)];
    }
    if(temp2>0)
    {
        vco[i].ID = i;
        vco[i].RemoteFlag = 0;
        vco[i].ExternFlag = 0;
        vco[i].DataLen = temp2;
        for(int j = 0;j<temp2;j++)
            vco[i].Data[j] = ch[j+(i*8)];
    }
    dwRel = VCI_Transmit(deviceType, debicIndex, debicCom, vco,count);
    if(dwRel>0)
        qDebug()<<"发送帧数："<<dwRel;
    else
        qDebug()<<"发送错误："<<dwRel;

}

void CANControler::run()
{
    while(!stopped)
    {
        DWORD dwRel;
        VCI_CAN_OBJ vco[2500];
        dwRel = VCI_Receive(deviceType, debicIndex, debicCom+1, vco,2500,0);
        if(dwRel > 0)
        {
//            for(int i = 0;i<dwRel;i++)
//            {
//                qDebug()<<"第"<<i<<"帧";
//                for(int j = 0;j<8;j++)
//                    qDebug()<<vco[i].Data[j];
//            }
//            qDebug()<<"from thread slot::" <<QThread::currentThreadId();
            emit getProtocolData(vco,dwRel);
        }
        else if(dwRel == -1)
        {
        /* USB-CAN设备不存在或USB掉线，可以调用VCI_CloseDevice并重新
        VCI_OpenDevice。如此可以达到USB-CAN设备热插拔的效果。 */
            qDebug()<<"设备不存在或USB掉线";
//            VCI_CloseDevice(deviceType, debicIndex);
//            VCI_OpenDevice(deviceType, debicIndex, debicCom);
        }
//        else
//            qDebug()<<"接收其它故障:"<<dwRel;
        sleep(1000);
    }
    stopped = false;
}

void CANControler::sleep(unsigned int msec)
{
   QTime dieTime = QTime::currentTime().addMSecs(msec);
   while( QTime::currentTime() < dieTime )
       QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

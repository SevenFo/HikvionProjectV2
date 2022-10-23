#ifndef CANCONTROLER_H
#define CANCONTROLER_H

#include <QObject>
#include <QThread>
#include "controlcan.h" //linux 平台
#include <QDebug>

class CANControler : public QObject
{
    Q_OBJECT
public:
    explicit CANControler(QObject *parent = nullptr);
    virtual ~CANControler();
    void stop();

    bool openCAN();

    void closeCAN();

    void sendData(int ID,unsigned char *ch);

    void run();

    int deviceType;
    int debicIndex;
    int baundRate;
    int debicCom;

    bool stopped;

signals:
    void getProtocolData(VCI_CAN_OBJ *vci,DWORD dwRel);

private:
    void sleep(unsigned int msec);

};

#endif // CANCONTROLER_H

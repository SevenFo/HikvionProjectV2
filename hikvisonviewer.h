#ifndef HIKVISONVIEWER_H
#define HIKVISONVIEWER_H


#include <imageviewer.h>
#include <hikvisonhandler.h>
#include <hikvisonhandlercstyelfuncs.h>
#include <QObject>
#include <QQuickItem>
#include <QBuffer>
#include <QTimer>
#include <audiodevice.h>
#include <QAudioSink>
#include "audio_handler.h"
#include <imagehandler.h>
#include "cancontroler.h"
//#include <QQuickImageProvider>
#include <xlnt/xlnt.hpp>
#include "faceimagehandler.h"
#include "cardatahandler.h"

//class SnapedFaceImageProvider

class HikvisonViewer : public IMageViewer
{
    Q_OBJECT

    Q_PROPERTY(QString host READ getHost WRITE setHost );
    Q_PROPERTY(QString port READ getPort WRITE setPort);
    Q_PROPERTY(QString userid READ getUserid WRITE setUserid );
    Q_PROPERTY(QString password READ getPassword WRITE setPassword);
    Q_PROPERTY(bool is_login READ isLogin WRITE setLoginState NOTIFY loginStateChanged);
    Q_PROPERTY(bool is_open_video READ isOpenVideo WRITE setOpenVideoState NOTIFY openVideoStateChange); //是否显示实时视频
//    Q_PROPERTY( READ getDriverLevel WRITE setDriverLevel NOTIFY driverLevelChanged);

//    Q_PROPERTY(int cardata_speed READ getSpeed, WRITE setSpeed NOTIFY cardataSpeedChanged);
//    Q_PROPERTY(int cardata_steering_angle READ getSteeringAngle, WRITE setSteeringAngle NOTIFY cardataSteeringAngleChanged);
//    Q_PROPERTY(int cardata_brake_angle READ getBrakeAngle, WRITE setBrakeAngle NOTIFY cardataBrakeAngleChanged);

public:
    HikvisonViewer(QQuickItem *parent = nullptr);
    virtual ~HikvisonViewer();

    Q_INVOKABLE void loginAndSetup();
    Q_INVOKABLE void startDecode();



    bool isLogin() const;
    bool isOpenVideo() const;
    QString getHost() const;
    QString getPort() const;
    QString getUserid() const;
    QString getPassword() const;

    void setHost(QString &d);
    void setPort(QString &d);
    void setUserid(QString &d);
    void setPassword(QString &d);
    void setLoginState(bool &d);
    void setOpenVideoState(bool d);

    void DisplayImage();
    void OnGetProtocolData(VCI_CAN_OBJ *vci,DWORD dwRel);
    void OnPredictFaceImageTimerOut();
protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void loginStateChanged();
    void runCanThread();
    void stopCanThread();
    void openVideoStateChange();
    void getSnapedImage(QPixmap pixmap, int width, int height);
    void getAudioInferedResult(QString event);
    void getFaceExprationResult(QString label,float p);
    void runFacePrediction(QPixmap pixmap, int width, int height);
    void dangerValueChecked(unsigned int level);
    void getSpeed(float value);
    void getSteeringAngle(float value);
    void getBrakeAngle(float value);

    void driverLevelChanged();
    void soundLevelChanged();
    void speedLevelChanged();
    void dangerLevelChanged();

    void isTired(int d);

private:
    unsigned char driver_level{0}, sound_level{0}, speed_level{0}, danger_level{0}, is_tired_value{0};

    void inline CheckDangerValue(){
        danger_level = cardata_handler->getSpeedLevel() + driver_level + sound_level + is_tired_value;
        if(danger_level == 0){
            emit dangerValueChecked(0);
        }
        else if(danger_level == 1){
            emit dangerValueChecked(1);
        }
        else if(danger_level == 2){
            emit dangerValueChecked(2);
        }
        else{
            qDebug()<<danger_level;
            emit dangerValueChecked(3);
        }
        danger_value = 0;
    }

    HikvisonHandler *hik;
    FaceImageHandler *image_handler;
    audio_handler *audiohandler;
    CarDataHandler *cardata_handler;
    CANControler *can_handler;
    QAudioSink *audio_sink; //输出音频
    AudioDevice *audio_device;
    QThread hikvisonThread, *audio_handler_thread, *image_handler_thread, *can_handler_thread, *cardata_handler_thread;
    QTimer *timerDisplay;
    QTimer * delayPlayAudioTimer;
    QTimer *inferAudioTimer;
    QTimer *fakeCanSendTimer;
    QTimer *predictFaceImageTimer;
    QTimer *checkDangerValueTimer;

    //用来操作xlsx文件
    xlnt::workbook *wb_;
    xlnt::worksheet *ws_;

    int fake_data_id_, fake_data_row_counter_{1};

    QVector<float> list_car_speeds, list_car_steering_angles, list_car_brake_angles;
    bool is_login;

    int danger_value{0};

    bool is_speedup{false};

};

#endif // HIKVISONVIEWER_H

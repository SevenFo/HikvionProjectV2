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
//#include <QQuickImageProvider>

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

signals:
    void loginStateChanged();
    void openVideoStateChange();
    void getSnapedImage(QPixmap pixmap, int width, int height);

private:
    HikvisonHandler *hik;
    audio_handler *audiohandler;
    QAudioSink *audio_sink; //输出音频
    AudioDevice *audio_device;
    QThread hikvisonThread, audio_handler_thread;
    QTimer *timerDisplay;

    bool is_login;

};

#endif // HIKVISONVIEWER_H

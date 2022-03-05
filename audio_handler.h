#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include <QObject>
#include <models.h>

class audio_handler : public QObject
{
    Q_OBJECT
public:
    explicit audio_handler(QObject *parent = nullptr);
    ~audio_handler();
    void inferate(QByteArray audio_data, qint64 sample_rate, qint64 sample_size, qint64 sample_length);

    inline void open_inferation(){
        isOpen = true;
    }
    inline void close_inferation(){
        isOpen = false;
    }
private:
    Models *CRNN;
    bool isOpen;

    bool resample(qsizetype in_rate, qsizetype out_rate, const QByteArray &indata, QByteArray &outdata);
    bool resample(qsizetype in_rate, qsizetype out_rate, const std::vector<float> &indata, std::vector<float> &outdata);

signals:

};

#endif // AUDIO_HANDLER_H

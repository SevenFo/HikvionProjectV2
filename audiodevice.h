#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include<QIODevice>
#include<QDebug>

class AudioDevice : public QIODevice
{
    Q_OBJECT
public:

    explicit AudioDevice(QObject *parent = nullptr);
    explicit AudioDevice(qint64 sample_rate, qint64 sample_bit_deep,QObject *parent = nullptr);
    ~AudioDevice();
    // return curruent size of buffer
    qint64 fill(const QByteArray &data);
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;
    QByteArray getLatestData(qint64 dua);

    inline void set_sample_rate(qint64 sr){
        sample_rate = sr;
        _set_readysize();
    }
    inline void set_sample_bit_deep(qint64 sbd){
        sample_bit_deep = sbd;
        _set_readysize();
    }
    inline void set_ready_duration(qint64 second)
    {
        readydura = second;
        _set_readysize();
    }
private:
    QByteArray pcm_data;
    qint64 pcm_data_offset;
    qint64 sample_rate; //采样率
    qint64 readydura;  //ready的时间 单位s
    qint64 readysize;  //ready的size = sr * dua * 2(16b)
    qint64 counter;
    qint64 sample_bit_deep;

    inline void _set_readysize()
    {
        readysize = sample_rate*readydura*(sample_bit_deep/8);
    }


signals:
    void ready(QByteArray d, qint64 sample_rate, qint64 sample_size, qint64 sample_length);
    void testSignal();
};

#endif // AUDIODEVICE_H

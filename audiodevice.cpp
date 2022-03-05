#include "audiodevice.h"
//还少了一个位深度才能计算readysize
AudioDevice::AudioDevice(QObject *parent):
    QIODevice{parent},
    pcm_data_offset(0),
    sample_rate(8000),
    readydura(9),
    readysize(72000*2),
    counter(0)
{
    if(!open(ReadOnly))
        qDebug()<<"open audio device failed!";
    else
        qDebug()<<"open audio device success!";

}
AudioDevice::AudioDevice(qint64 sample_rate, qint64 sample_bit_deep,QObject *parent):
    QIODevice{parent},
    pcm_data_offset(0),
    sample_rate(sample_rate),
    readydura(9),
    sample_bit_deep(sample_bit_deep),
    readysize(72000*2),
    counter(0)
{
    if(!open(ReadOnly))
        qDebug()<<"open audio device failed!";
    else
        qDebug()<<"open audio device success!";

}


AudioDevice::~AudioDevice()
{

}

qint64 AudioDevice::fill(const QByteArray &data)
{
    pcm_data.append(data);
//    qInfo()<<"pcm_data size:"<<pcm_data.size();
    if(pcm_data.size()/readysize>counter)
    {
        emit ready(QByteArray(pcm_data.data()+(counter++*readysize),readysize),sample_rate,sample_bit_deep,readysize/(sample_bit_deep/8));
        qDebug()<<"ready counter:"<<counter;
    }
    return pcm_data.size();
}

qint64 AudioDevice::readData(char *data, qint64 maxlen)
{
    if(pcm_data_offset > pcm_data.size())
    {
        qDebug()<<"play over!";
        return 0;
    }

    auto reallen = (pcm_data.size() - pcm_data_offset) > maxlen ? maxlen:(pcm_data.size() - pcm_data_offset); // 是否填充最大缓冲区
//    qDebug()<<"try push data:"<<reallen;
    memcpy(data, pcm_data.data()+pcm_data_offset,reallen);
    pcm_data_offset += reallen;
    return pcm_data_offset;
}

qint64 AudioDevice::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}
qint64 AudioDevice::bytesAvailable() const
{
    return pcm_data.size() + QIODevice::bytesAvailable();
}

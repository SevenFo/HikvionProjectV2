#include "audiodevice.h"
#include <QFile>
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
    //默认应该是16位深度，长度4s -> size: 8000*16/8*4=64000
    QIODevice{parent},
    pcm_data_offset(0),
    sample_rate(sample_rate),
    readydura(4),
    sample_bit_deep(sample_bit_deep),
    readysize(64000),
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
    return pcm_data.size();
}
QByteArray AudioDevice::getLatestData(qint64 dua){
   auto data_size = dua*sample_rate*sample_bit_deep/8;
   if(pcm_data.size() - data_size < 0)
   {
       qDebug()<<"audio data is not ready, wait, ("<<pcm_data.size()<<"<"<<data_size<<")";
       return QByteArray();
   }
   auto tmp = pcm_data.sliced(pcm_data.size()-data_size,data_size);
   emit ready(tmp,sample_rate,sample_bit_deep,readysize/(sample_bit_deep/8));
   return tmp;
}
qint64 AudioDevice::readData(char *data, qint64 maxlen)
{
    if(pcm_data_offset > pcm_data.size())
    {
        qDebug()<<"play over!";
        return 0;
    }

    auto reallen = (pcm_data.size() - pcm_data_offset) > maxlen ? maxlen:(pcm_data.size() - pcm_data_offset); // 是否填充最大缓冲区
//    qDebug()<<"try push data:"<<reallen<<", maxlen:"<<maxlen;
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

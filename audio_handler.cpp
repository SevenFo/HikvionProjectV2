#include "audio_handler.h"
#include "soxr.h"
#include <QtDebug>
#include <QFile>

audio_handler::audio_handler(QObject *parent)
    : QObject{parent},
      isOpen(true)
{
    CRNN = new Models(Models::CRNN,"../c5model.onnx");
}



audio_handler::~audio_handler()
{ 
}



void audio_handler::inferate(QByteArray d, qint64 sample_rate, qint64 sample_size, qint64 sample_length)
{

    if(!isOpen)
        return;
    if(sample_length != d.size()/(sample_size/8))
    {
        qWarning()<<"length of input audio :"<<sample_length<<" not match the data size:"<<d.size();
        return;
    }
    QFile f1("resample_org.pem");
    f1.open(QFile::WriteOnly);
    f1.write((char *)d.data(),d.size());
    f1.close();
    std::vector<float> audio_data(sample_length),resampled_data;
    std::vector<float> result;

    qsizetype index = 0;
    //归一化，转化为float类型
    for(float & item:audio_data)
    {
        item = (float)(*((short *)&d[index])) / pow(2,16)*2;
        index += 2;
    }

    if(!resample(8000,44100,audio_data,resampled_data))
    {
        qWarning()<<"audio resample error!";
    }


    if(!CRNN->infer(resampled_data,result))
    {
        qWarning()<<"infer error!";
    }
    qInfo()<<"result:";
    for(auto item : result)
    {
        qInfo()<<item;
    }
}



/*
 * example on shot
    soxr_error_t error = soxr_oneshot(irate, orate, 1,      // Rates and # of chans.
                                      in, AL(in), NULL,     // Input.
                                      out, olen, &odone,    // Output.
                                      NULL, NULL, NULL);    // Default configuration.
 */
bool audio_handler::resample(qsizetype in_rate, qsizetype out_rate, const QByteArray &indata, QByteArray &outdata)
{
    qsizetype channel = 1;
    auto outsize = static_cast<qsizetype>(static_cast<float>(out_rate)/in_rate*indata.size())+1;
    size_t rsize = outsize;
    outdata.resize(outsize);
    soxr_error_t error = soxr_oneshot(in_rate, out_rate, channel, /* Rates and # of chans. */
                                      indata.data(), indata.size(), NULL,                              /* Input. */
                                      outdata.data(),outsize, &rsize,                             /* Output. */
                                      NULL, NULL, NULL);
    if(error)
        return false;
    if(outsize != rsize)
        outdata.resize(rsize);
    return true;

}


bool audio_handler::resample(qsizetype in_rate, qsizetype out_rate, const std::vector<float> &indata, std::vector<float> &outdata)
{
    std::cout <<"resample..."<<std::endl;
    QFile file_t("test_file_after_resample.pem");//    qInfo()<<"pcm_data size:"<<pcm_data.size();
    qsizetype channel = 1;
    auto outsize = static_cast<qsizetype>(static_cast<float>(out_rate)/in_rate*indata.size())+1;
    size_t rsize = outsize;
    outdata.resize(outsize);


    soxr_error_t error = soxr_oneshot(in_rate, out_rate, channel, /* Rates and # of chans. */
                                      indata.data(), indata.size(), NULL,                              /* Input. */
                                      outdata.data(),outsize, &rsize,                             /* Output. */
                                      NULL, NULL, NULL);
    if(error)
        return false;
    if(outsize != rsize)
        outdata.resize(rsize);

    QFile f1("resample_before.pem");
    f1.open(QFile::WriteOnly);
    f1.write((char *)indata.data(),indata.size()*sizeof(float));
    f1.close();
    f1.setFileName("resample_after.pem");
    f1.open(QFile::WriteOnly);
    f1.write((char *)outdata.data(),outdata.size()*sizeof (float));
    f1.close();


    return true;
}


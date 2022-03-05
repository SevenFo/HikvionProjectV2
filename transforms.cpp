//
// Created by siky on 2022/2/20.
//
#include "transforms.h"

Vectorf pad(Vectorf &x, int left, int right, const std::string &mode, float value){
    Vectorf x_paded = Vectorf::Constant(left+x.size()+right, value);
    x_paded.segment(left, x.size()) = x;

    if (mode.compare("reflect") == 0){
        for (int i = 0; i < left; ++i){
            x_paded[left-(i+1)] = x_paded[left+(i+1)];
        }
        for (int i = 0; i < right; ++i){
            x_paded[i+x.size()+left] = x_paded[x.size()-(i+1)+left-1];
        }
    }

    if (mode.compare("symmetric") == 0){
        for (int i = 0; i < left; ++i){
            x_paded[i] = x[left-i-1];
        }
        for (int i = left; i < left+right; ++i){
            x_paded[i+x.size()] = x[x.size()-1-i+left];
        }
    }

    if (mode.compare("edge") == 0){
        for (int i = 0; i < left; ++i){
            x_paded[i] = x[0];
        }
        for (int i = left; i < left+right; ++i){
            x_paded[i+x.size()] = x[x.size()-1];
        }
    }
    return x_paded;
}
Vectorf  _diff(const Vectorf &x)
{
    Vectorf result = Vectorf::Zero(x.size());
    for(int i = 0;i<x.size()-1;i++){
        result[i] = x[i+1]-x[i];
    }
    return result;
}
float hz_to_mel(float x_hz, bool htk)
{
    if (htk) {
        return 2595.0f * log10f(1.0f + x_hz / 700.0f); //check
    }

    float min_log_hz = 1000.0;
    float f_min = 0.0f;
    float f_sp = 200.0/3.0f;
    float min_log_mel = (min_log_hz - f_min) / f_sp;
    auto logstep = log(6.4) / 27.0;


    float mels = (x_hz - f_min) / f_sp;
    if (x_hz >= min_log_hz)
        mels = min_log_mel + log(x_hz / min_log_hz) / logstep;
    return mels;
}
Vectorf hz_to_mel(Vectorf x_hz, bool htk)
{
    if (htk)
        return 2595.0f * (Eigen::log10(x_hz.array()/700.0f + 1.0f));
    for(float &value : x_hz)
    {
        value = hz_to_mel(value, false);
    }
    return x_hz;
}

float mel_to_hz(float mels, bool htk)
{
    if (htk) {
        return 700.0 * (pow(10.0 ,(mels / 2595.0)) - 1.0);
    }

    float min_log_hz = 1000.0;
    float f_min = 0.0f;
    float f_sp = 200.0/3.0f;
    float min_log_mel = (min_log_hz - f_min) / f_sp;
    auto logstep = log10(6.4) / 27.0;
    float freqs = f_min + f_sp * mels;

    if (mels >= min_log_hz)
        freqs = min_log_hz + exp(logstep*(mels-min_log_mel));
    return freqs;
}
Vectorf mel_to_hz(Vectorf mels, bool htk)
{
    if (htk)
        return  700.0 * (Eigen::pow(10.0 ,(mels.array() / 2595.0)) - 1.0);
    for(float &value : mels)
    {
        value = mel_to_hz(value, false);
    }
    return mels;
}

Matrixf z_transform(Matrixf x)
{
    float mean = x.mean();
    Matrixf error = (x.array() - mean);
    float v = (error.array() * error.array()).sum()/(x.size()-1);
    float std = sqrt(v);
    return (x.array()-mean)/std;
}

Matrixf mel_filtere(int sr, int n_fft, int n_mels, int fmin, int fmax,int norm) {

    if (fmax = -1)
        fmax = sr/2;
    // initialize the weights
    Matrixf weights(n_mels,1+n_fft/2);
    weights.fill(0);

    // center freqs of each FFT bin
    Vectorf fftfreqs = Vectorf::LinSpaced(n_fft/2+1,0.0,(float)sr/2.0f);
    float min_mel = hz_to_mel(fmin,true);
    float max_mel = hz_to_mel(fmax,true);
    Vectorf mel_f = mel_to_hz(Vectorf::LinSpaced(n_mels+2,min_mel,max_mel),true);
    Vectorf fdiff = _diff(mel_f);
    Matrixf ramps = mel_f.transpose().rowwise().replicate(fftfreqs.size()).array() - fftfreqs.colwise().replicate(mel_f.size()).array();
    for (int i = 0;i<n_mels;i++)
    {
        Vectorf lower = -ramps.row(i).array() / fdiff[i];
        Vectorf upper = ramps.row(i+2).array() / fdiff[i+1];
        Matrixf minimum(2,lower.size());
        minimum.row(0) = lower;
        minimum.row(1) = upper;
        lower = minimum.colwise().minCoeff();
        Matrixf maximum(2,lower.size());
        maximum.row(0) = lower;
        maximum.row(1) = Vectorf::Zero(lower.size());
        weights.row(i) = maximum.colwise().maxCoeff();
    }
    if(norm == 1)
    {
        //slancy norm
    }
    else
    {
    }
    return weights;

}

Matrixf mel_sepctrogram(Vectorf &data, int sr, int n_fft, int n_hop,
                               const std::string &win , bool center ,
                               const std::string &mode , float power ,
                               int n_mels , int , int f_max)
{
    auto fft = Eigen::FFT<float>();
    Eigen::Matrix<double,1,Eigen::Dynamic,Eigen::RowMajor> window_temp = 0.5*(1.0-(Eigen::Matrix<double,1,Eigen::Dynamic,Eigen::RowMajor>::LinSpaced(n_fft, 0.0, static_cast<float>(n_fft-1.0))*2.0*EIGEN_PI/n_fft).array().cos());
    Vectorf window = window_temp.cast<float>();
    Vectorf x_paded = pad(data, n_fft/2, n_fft/2, "reflect", 0.f);
    int n_f = n_fft/2+1; //n_fft_real
    int n_frames = 1+(x_paded.size()-n_fft) / n_hop;
    Matrixcf X(n_frames, n_fft);

    for (int i = 0; i < n_frames; ++i){
        Vectorf x_frame = window.array()*x_paded.segment(i*n_hop, n_fft).array();
//        if(false && i%10==0) {
//            std::cout << "fft_input:(" << i << "):";
//            for (int j = 0; j < x_frame.size(); j++)
//                std::cout << x_frame[j] << ",";
//            std::cout << std::endl;
//        }
        X.row(i) = fft.fwd(x_frame);
//        if(false && i%10==0) {
//            std::cout << "fft_output:(" << i << "):";
//            for (int j = 0; j < X.row(i).size(); j+=100)
//                std::cout << X.row(i)[j] << ",";
//            std::cout << std::endl;
//        }

    }
    Matrixcf stft_result = X.leftCols(n_f);
    Matrixf magnitude_stft = stft_result.array().abs();
//    std::cout <<"row:"<<power_stft.rows()<<" col:"<<power_stft.cols()<<std::endl;
//    for(int r_count = 0;r_count<power_stft.rows();r_count+=10) {
//        std::cout <<"row:"<<r_count<<":";
//        for (auto begin = power_stft.row(r_count).begin(); begin < power_stft.row(r_count).end(); begin+=20) {
//            std::cout << *begin << ",";
//        }
//        std::cout<<std::endl;
//    }

    Matrixf mel_basis = mel_filtere(sr, n_fft, n_mels, 0, sr/2);
//    for(int r_count = 0;r_count<mel_basis.rows();r_count+=1) {
//        std::cout <<"row:"<<r_count<<":";
//        for (auto begin = mel_basis.row(r_count).begin(); begin < mel_basis.row(r_count).end(); begin+=20) {
//            std::cout << *begin << ",";
//        }
//        std::cout<<std::endl;
//    }
    Matrixf testsound_spec = z_transform(magnitude_stft * mel_basis.transpose());
//    std::cout << "mel_output:";
//    for(int i =0;i <testsound_spec.rows();i++) {
//        for (int j = 0; j < testsound_spec.cols(); j += 1)
//            std::cout << testsound_spec.row(i)[j] << ",";
//        std::cout << std::endl;
//    }
    return testsound_spec.transpose();
}

int64_t num_stft_bins(int64_t lengths, int64_t fft_length, int64_t hop_length, int64_t pad)
{
    return (lengths + 2 * pad - fft_length + hop_length)/hop_length;
}

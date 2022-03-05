//
// Created by siky on 2022/2/20.
//

#ifndef MFCC_TRANSFORMS_H
#define MFCC_TRANSFORMS_H

#include <iostream>
#include "AudioFile.h"
#include "unsupported/Eigen/FFT"
#include "Eigen/Eigen"
#include "math.h"
#include "Eigen/Core"
 #include "onnxruntime_cxx_api.h"
#include "CsvPraser.h"
typedef Eigen::Matrix<float,1,Eigen::Dynamic,Eigen::RowMajor> Matrix1Xf;
typedef Eigen::Matrix<std::complex<float>, 1, Eigen::Dynamic, Eigen::RowMajor> Vectorcf;
typedef Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor> Vectorf;
typedef Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrixcf;
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrixf;


static Vectorf pad(Vectorf &x, int left, int right, const std::string &mode, float value);


static Vectorf  _diff(const Vectorf &x);

static float hz_to_mel(float x_hz, bool htk);

static Vectorf hz_to_mel(Vectorf x_hz, bool htk);

static float mel_to_hz(float mels, bool htk);

static Vectorf mel_to_hz(Vectorf mels, bool htk);

static Matrixf z_transform(Matrixf x);

static Matrixf mel_filtere(int sr, int n_fft, int n_mels, int fmin, int fmax=-1,int norm = 0);

Matrixf mel_sepctrogram(Vectorf &data, int sr, int n_fft=2048, int n_hop=1024,
const std::string &win = "hann", bool center = false,
const std::string &mode = "reflect", float power = 2.0f,
int n_mels = 128, int f_min=0, int f_max=44100/2);

int64_t num_stft_bins(int64_t lengths, int64_t fft_length, int64_t hop_length, int64_t pad);

#endif //MFCC_TRANSFORMS_H

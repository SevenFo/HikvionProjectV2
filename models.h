#ifndef MODELS_H
#define MODELS_H

#include <iostream>
#include "AudioFile.h"
#include "unsupported/Eigen/FFT"
#include "Eigen/Eigen"
#include "Eigen/Core"
#include "onnxruntime_cxx_api.h"
#include "CsvPraser.h"
#include "transforms.h"
#include "chrono"
class Models
{
public:
    enum MODELTYPE{
        CRNN,
        CNN_M
    };
public:
    Models(enum MODELTYPE type, const std::string & model_path);
    ~Models();

    bool _initCRNN();
    bool _CRNNInfer(const std::vector<_Float32> &input_audio);
    bool infer(std::vector<float> &audio_data, std::vector<float> &result);

private:
    Ort::Env *env;
    Ort::Session *session;
    enum MODELTYPE model_type;
};

#endif // MODELS_H

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
#include "opencv2/opencv.hpp"

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
    bool runCNN(const cv::Mat &face_image, std::string &result_label, int &index,float &result_p, std::vector<float> &result_ps);
    bool parseOutput(std::vector<Ort::Value>& output_tensors, std::string &result_label,int &index ,float &result_p, std::vector<float> &result_ps);
private:
    Ort::Env *env;
    Ort::Session *session;
    enum MODELTYPE model_type;

    bool _parseCNNOutput(std::vector<Ort::Value>& output_tensors, std::string &result_label,int &index ,float &result_p, std::vector<float> &result_ps);
    bool _preHandleImage(const cv::Mat &raw_image, std::vector<float> &handled_image);
};

#endif // MODELS_H

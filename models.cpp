#include "models.h"
#include <chrono>
#include <transforms.h>
#include <QtDebug>

Models::Models(enum MODELTYPE type, const std::string & model_path) :model_type(type)
{
    env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "test");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
    session = new Ort::Session(*env, model_path.data(), session_options);


    auto n_input = session->GetInputCount();
    std::cout << "n_input:" << n_input;
    Ort::AllocatorWithDefaultOptions allocator;

    for(int index = 0;index<n_input;index++) {
        auto input_names = session->GetInputName(index, allocator);
        auto input_typeinfo = session->GetInputTypeInfo(index);
        auto input_shape = input_typeinfo.GetTensorTypeAndShapeInfo().GetShape();
        auto input_type = input_typeinfo.GetTensorTypeAndShapeInfo().GetElementType();
        std::cout<<";name["<<index<<"]:"<<input_names<<" type:"<<input_type<<" shape:";
        for(auto s : input_shape)
        {
            std::cout<<s<<",";
        }
    }
    std::cout<<std::endl;
    switch (type) {

    case CRNN:{
        _initCRNN();
        break;
    }
    case CNN_M:{
        break;
    }
    default:
    {
        break;
    }
    }

}
Models::~Models()
{

}

bool Models::_initCRNN()
{
    return true;
}

bool Models::infer(std::vector<float> &audio_data, std::vector<float> &result)
{
    std::cout <<"infering..."<<std::endl;
    int sr = 8000;
    int length = audio_data.size();
    auto infer_start_time = std::chrono::system_clock::now();

    Vectorf data = Eigen::Map<Vectorf>(audio_data.data(),audio_data.size()); //转化为Eigen用于计算mel
    Matrixf testsound_spec = mel_sepctrogram(data,sr); //mel result
    std::cout <<"data size:"<<data.cols()<<"mel reslut: shape:"<<testsound_spec.rows() <<"x"<<testsound_spec.cols()<<std::endl;
    if(testsound_spec.cols() < 64)
    {
        std::cerr <<"mel result to short to infer!"<<std::endl;
        return false;
    }

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<const char *> input_names{"input_data","input_time_length"};
    std::vector<const char *> output_names{"output_name"};

    //perpare input data
    size_t input_data_size = testsound_spec.cols()*testsound_spec.rows();
    std::vector<float> input_tensor_values(input_data_size,0.5);
    auto input_data_values = testsound_spec.data();
    std::vector<int64_t> input_data_dims = {1,1,testsound_spec.rows(),testsound_spec.cols()};
    std::vector<int64_t> input_data_length_values = std::vector<int64_t>{num_stft_bins(length,2048,1024,1024)};
    std::vector<int64_t> input_data_length_dims = std::vector<int64_t>{1};

    //create input data
    Ort::Value input_data = Ort::Value::CreateTensor<float>(memory_info, input_data_values, input_data_size, input_data_dims.data(), input_data_dims.size());
    Ort::Value input_data_length = Ort::Value::CreateTensor<int64_t>(memory_info, input_data_length_values.data(), input_data_length_values.size(), input_data_length_dims.data(),input_data_length_dims.size());
    std::vector<Ort::Value> inputs;
    inputs.push_back( std::move(input_data));
    inputs.push_back( std::move(input_data_length));
    auto output_tensors =session->Run(Ort::RunOptions{nullptr},input_names.data(),inputs.data(),inputs.size(),output_names.data(),output_names.size());
    auto output_typeinfo = output_tensors[0].GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();
    auto result_tmp = output_tensors[0].GetTensorMutableData<float>();
    result.resize(5);
    std::cout <<"result:";
    for(int i =0;i<5;i++)
    {
//        std::cout<<exp(result_tmp[i])<<",";
//        qInfo()<<exp(result_tmp[i])<<",";
        result[i] = exp(result_tmp[i]);

    }
//    std::cout<<std::endl;
    auto infer_end_time = std::chrono::system_clock::now();
    std::cout <<" time take "<<std::chrono::duration_cast<std::chrono::milliseconds>(   infer_end_time-infer_start_time).count()<<std::endl;
    return true;
}

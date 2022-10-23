#include "models.h"
#include <chrono>
#include <transforms.h>
#include <QtDebug>
#include <algorithm>

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

    switch (model_type) {

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

bool Models::runCNN(const cv::Mat &face_image, std::string &result_label,int &index ,float &result_p, std::vector<float> &result_ps){
    auto infer_start_time = std::chrono::system_clock::now();
    std::vector<float> face;
    if(not _preHandleImage(face_image,face)){
        return false;
    }
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,OrtMemTypeDefault);
    std::vector<const char *> input_names{"face_image"};
    std::vector<const char *> output_names{"result"};

    size_t input_data_size = 3*48*48;
    std::vector<int64_t> input_data_dims = {1,3,48,48};

    Ort::Value input_data = Ort::Value::CreateTensor<float>(memory_info, face.data(), input_data_size, input_data_dims.data(), input_data_dims.size());
    assert(input_data.IsTensor());

    std::vector<Ort::Value> inputs;
    inputs.push_back(std::move(input_data));

    auto output_datas = session->Run(Ort::RunOptions{nullptr},input_names.data(),inputs.data(),inputs.size(),output_names.data(),output_names.size());

    if(parseOutput(output_datas,result_label,index,result_p,result_ps)){
        auto infer_end_time = std::chrono::system_clock::now();
        std::cout<<"experation infer time:"<<std::chrono::duration_cast<std::chrono::milliseconds>(infer_end_time - infer_start_time).count();
        return true;
    }
    else
        return false;

}

bool Models::parseOutput(std::vector<Ort::Value>& output_tensors, std::string &result_label,int &index ,float &result_p, std::vector<float> &result_ps)
{
    switch (model_type) {

    case CRNN:{
        break;
    }
    case CNN_M:{
        return _parseCNNOutput(output_tensors,result_label,index,result_p,result_ps);
        break;
    }
    default:
    {
        return false;
        break;
    }
    }
    return false;
}

bool Models::_parseCNNOutput(std::vector<Ort::Value>& output_tensors, std::string &result_label,int &index, float &result_p, std::vector<float> &result_ps)
{
    std::vector<std::string>face_labels = {"生气","恶心","恐惧","高兴","悲伤", "惊讶", "中性"};
    std::vector<std::string>face_labels_reduced = {"积极","消极","中性"};
    const auto output = output_tensors.at(0).GetTensorMutableData<float>(); //只有一个输出
    auto output_size = 7;
    result_ps.resize(output_size);
    for(int i = 0; i<output_size; i++){
        result_ps[i] = (output[i]);
    }
    auto max_value_it = std::max_element(result_ps.begin(),result_ps.end());
    auto max_index = max_value_it-result_ps.begin();
    index= static_cast<int>(max_index);
    switch(index){
    case 0: case 1: case 2: case 4: case 5:
        result_label = std::string("消极");
        break;
    case 3:
        result_label = std::string("积极");
        break;
    case 6:
        result_label = std::string("中性");
        break;
    }
    result_p = *max_value_it;
    return true;
}

bool Models::_preHandleImage(const cv::Mat &raw_image, std::vector<float> &handled_image)
{
    const auto height = raw_image.size().height;
    const auto width = raw_image.size().width;
    const auto channels = raw_image.channels();
    //norm
    auto normlize_image = [=](const cv::Mat &mat, cv::Mat &result){
        int size[] = {3,48,48};
        result = cv::Mat(3,size,CV_32FC1);
        for(int i = 0;i<48;i++){
            for(int j = 0;j<48;j++){
                *(result.begin<float>()+48*i+j) = (*(mat.data+48*3*i+j*3+0)/255.0-0.485)/0.229;
            }
        }
        for(int i = 0;i<48;i++){
            for(int j = 0;j<48;j++){
                *(result.begin<float>()+48*48+48*i+j) = (*(mat.data+48*3*i+j*3+1)/255.0-0.456)/0.224;
            }
        }
        for(int i = 0;i<48;i++){
            for(int j = 0;j<48;j++){
                *(result.begin<float>()+48*48*2+48*i+j) = (*(mat.data+48*3*i+j*3+2)/255.0-0.406)/0.225;
            }
        }
    };

//    std::cout<<raw_image.size()<<" "<<raw_image.dims<<" "<<raw_image.step<<std::endl;
    cv::Mat gray_image;
    if(raw_image.type() != CV_8UC3){
        std::cout<<"face image type should be 8UC3! input image type:"<<raw_image.type()<<std::endl;
        return false;
    }
    cv::Mat rgb_gray_image;
    cv::cvtColor(raw_image,gray_image,cv::COLOR_BGR2GRAY);
    cv::cvtColor(gray_image,rgb_gray_image,cv::COLOR_GRAY2BGR);
//    std::cout<<gray_image.size()<<" "<<gray_image.dims<<" "<<gray_image.step<<std::endl;
//    cv::imshow("rgb_gray_image",rgb_gray_image);
//    std::cout<<rgb_gray_image.size()<<" "<<rgb_gray_image.dims<<" "<<rgb_gray_image.step[1]<<std::endl;
    double min = std::min(height,width);
    if(min < 48){
        std::cout <<"face image size < 48!"<<std::endl;
        return false;
    }
    cv::Point2d middle{width/2.0,height/2.0};
    cv::Mat squar_image = rgb_gray_image(cv::Range(static_cast<int>(middle.y-min/2),static_cast<int>(middle.y+min/2)-1),cv::Range(static_cast<int>(middle.x-min/2),static_cast<int>(middle.x+min/2)-1));
    cv::Mat input_image,resized_image;
    cv::Size squar_image_size; cv::Point ofs;
    squar_image.locateROI(squar_image_size,ofs);
//    std::cout<<"squar_image_size:"<<squar_image_size<<" ofs:"<<ofs<<std::endl;
    cv::resize(squar_image,resized_image,cv::Size(48,48));
//    std::cout<<resized_image.size()<<" "<<resized_image.dims<<" "<<resized_image.elemSize()<<std::endl;
//    std::cout<<resized_image.size()<<" "<<resized_image.dims<<std::endl;
//    cv::imshow("resized_image",resized_image);
//    cv::waitKey();

    cv::Mat norm_image;
    normlize_image(resized_image,norm_image);
    handled_image = std::vector<float>(norm_image.begin<float>(),norm_image.end<float>());
//    std::cout << resized_image << std::endl;


//    std::cout<<static_cast<unsigned int>(resized_image.at<unsigned char>(1,1))<<" "<<norm_image.at<float>(0,1,1)<<std::endl;
    //使用Matrix矩阵运算归一化图片再输入

    return true;

}

bool Models::infer(std::vector<float> &audio_data, std::vector<float> &result)
{
    int sr = 44100;
    int length = audio_data.size();
    auto infer_start_time = std::chrono::system_clock::now();

    Vectorf data = Eigen::Map<Vectorf>(audio_data.data(),audio_data.size()); //转化为Eigen用于计算mel
    Matrixf testsound_spec = mel_sepctrogram(data,sr); //mel result
    std::cout <<"data size: "<<data.cols()<<"; mel reslut: shape:"<<testsound_spec.rows() <<"x"<<testsound_spec.cols()<<std::endl;
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
    for(int i =0;i<5;i++)
    {
        result[i] = exp(result_tmp[i]);
    }
    auto infer_end_time = std::chrono::system_clock::now();
    std::cout <<"audio infer time take "<<std::chrono::duration_cast<std::chrono::milliseconds>(   infer_end_time-infer_start_time).count()<<std::endl;
    return true;
}

#include <ppdet_base.h>


int main()
{
	PaddleDetection::ConfigPaser config_paser;
	std::string model_dir = "D:\\programs\\C++\\PPDET\\PPDET\\PPDET_BASE\\model\\picodet_l_416_coco";
	config_paser.load_config(model_dir);
	
	paddle_infer::Config config;// ƒ£–Õ≈‰÷√
	std::string device = "GPU";
	PaddleDetection::ObjectDetector det(model_dir, device);
	auto mat = cv::imread("D:\\programs\\YOLOX\\assets\\dog.jpg");
	std::vector<PaddleDetection::ObjectResult>* result = new std::vector<PaddleDetection::ObjectResult>();
	std::vector<int>* bbox_num = new std::vector<int>();
	std::vector<double>* times = new std::vector<double>();
	const std::vector<int> color_map = PaddleDetection::GenerateColorMap(config_paser.label_list_.size());

	det.Predict(std::vector<cv::Mat>{mat},0.5,0,1,result,bbox_num,times);
	
	auto vis_img = PaddleDetection::VisualizeResult(mat, *result, config_paser.label_list_,color_map,false,false);
	
	for (auto i : *result)
	{
		std::cout << "result: class" << i.class_id << " conf:" << i.confidence;
	}

	cv::imshow("result", vis_img);
	cv::waitKey();
	return 0;
}

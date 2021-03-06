#include "yolox.h"

Yolox::Yolox()
{

}

bool Yolox::LoadModel(const std::string& input_model,
    const std::string& device,
    const int thread_number)
{
    serialization::GraphLoader::LoadConfig load_config;
    load_config.comp_graph = ComputingGraph::make();
    auto&& graph_opt = load_config.comp_graph->options();
    graph_opt.graph_opt_level = 0;

    if (device == "cuda") {
        load_config.comp_node_mapper = [](CompNode::Locator& loc) {
            loc.type = CompNode::DeviceType::CUDA;
        };
    }
    else if (device == "cpu") {
        load_config.comp_node_mapper = [](CompNode::Locator& loc) {
            loc.type = CompNode::DeviceType::CPU;
        };
    }
    else if (device == "multithread") {
        load_config.comp_node_mapper = [thread_number](CompNode::Locator& loc) {
            loc.type = CompNode::DeviceType::MULTITHREAD;
            loc.device = 0;
            loc.stream = thread_number;
        };
        std::cout << "use " << thread_number << " thread" << std::endl;
    }
    else {
        std::cout << "device only support cuda or cpu or multithread" << std::endl;
        return false;
    }

    if (device == "cuda") {
        std::cout << "choose format for cuda" << std::endl;
    }
    else {
        std::cout << "choose format for non-cuda" << std::endl;
    }
    std::cout << "read model..."<<std::endl;
    std::unique_ptr<serialization::InputFile> inp_file =
        serialization::InputFile::make_fs(input_model.c_str());
    std::cout << "loading model"<<std::endl;
    auto loader = serialization::GraphLoader::make(std::move(inp_file));
    network = loader->load(load_config, false);
    std::cout << "load success"<<std::endl;
    return true;
}

bool Yolox::Inference(cv::Mat& image, cv::Mat& result)
{
    auto start = std::chrono::system_clock::now();
    auto data = network.tensor_map["data"];
    cv::Mat pr_img = static_resize(image);

    float* data_ptr = data->resize({ 1, 3, 640, 640 }).ptr<float>();

    blobFromImage(pr_img, data_ptr);
    HostTensorND predict;
    std::unique_ptr<cg::AsyncExecutable> func = network.graph->compile(
        { make_callback_copy(network.output_var_map.begin()->second, predict) });
    std::cout << "excuate func"<<std::endl;
    func->execute();
    func->wait();
    std::cout << "excuated"<<std::endl;
    float* predict_ptr = predict.ptr<float>();
    int img_w = image.cols;
    int img_h = image.rows;
    float scale =
        std::min(INPUT_W / (image.cols * 1.0), INPUT_H / (image.rows * 1.0));
    std::vector<Object> objects;

    decode_outputs(predict_ptr, objects, scale, img_w, img_h);
    auto end = std::chrono::system_clock::now();
    auto dua = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout<<"dua:"<< double(dua.count());
    draw_objects(image, objects);
    return true;
}

Yolox::~Yolox()
{

}

cv::Mat static_resize(cv::Mat& img) {
    float r = std::min(INPUT_W / (img.cols * 1.0), INPUT_H / (img.rows * 1.0));
    int unpad_w = r * img.cols;
    int unpad_h = r * img.rows;
    cv::Mat re(unpad_h, unpad_w, CV_8UC3);
    cv::resize(img, re, re.size());
    cv::Mat out(INPUT_W, INPUT_H, CV_8UC3, cv::Scalar(114, 114, 114));
    re.copyTo(out(cv::Rect(0, 0, re.cols, re.rows)));
    return out;
}

void blobFromImage(cv::Mat& img, float* blob_data) {
    cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
    int channels = 3;
    int img_h = img.rows;
    int img_w = img.cols;
    std::vector<float> mean = { 0.485, 0.456, 0.406 };
    std::vector<float> std = { 0.229, 0.224, 0.225 };
    for (size_t c = 0; c < channels; c++) {
        for (size_t h = 0; h < img_h; h++) {
            for (size_t w = 0; w < img_w; w++) {
                blob_data[c * img_w * img_h + h * img_w + w] =
                    (((float)img.at<cv::Vec3b>(h, w)[c]) / 255.0f - mean[c]) / std[c];
            }
        }
    }
}


void
generate_grids_and_stride(const int target_size, std::vector<int>& strides,
    std::vector<GridAndStride>& grid_strides) {
    for (auto stride : strides) {
        int num_grid = target_size / stride;
        for (int g1 = 0; g1 < num_grid; g1++) {
            for (int g0 = 0; g0 < num_grid; g0++) {
                grid_strides.push_back((GridAndStride) { g0, g1, stride });
            }
        }
    }
}


void generate_yolox_proposals(std::vector<GridAndStride> grid_strides,
    const float* feat_ptr,
    float prob_threshold,
    std::vector<Object>& objects) {
    const int num_class = 80;
    const int num_anchors = grid_strides.size();

    for (int anchor_idx = 0; anchor_idx < num_anchors; anchor_idx++) {
        const int grid0 = grid_strides[anchor_idx].grid0;
        const int grid1 = grid_strides[anchor_idx].grid1;
        const int stride = grid_strides[anchor_idx].stride;

        const int basic_pos = anchor_idx * 85;

        float x_center = (feat_ptr[basic_pos + 0] + grid0) * stride;
        float y_center = (feat_ptr[basic_pos + 1] + grid1) * stride;
        float w = exp(feat_ptr[basic_pos + 2]) * stride;
        float h = exp(feat_ptr[basic_pos + 3]) * stride;
        float x0 = x_center - w * 0.5f;
        float y0 = y_center - h * 0.5f;

        float box_objectness = feat_ptr[basic_pos + 4];
        for (int class_idx = 0; class_idx < num_class; class_idx++) {
            float box_cls_score = feat_ptr[basic_pos + 5 + class_idx];
            float box_prob = box_objectness * box_cls_score;
            if (box_prob > prob_threshold) {
                Object obj;
                obj.rect.x = x0;
                obj.rect.y = y0;
                obj.rect.width = w;
                obj.rect.height = h;
                obj.label = class_idx;
                obj.prob = box_prob;

                objects.push_back(obj);
            }

        } // class loop

    } // point anchor loop
}

void qsort_descent_inplace(std::vector<Object>& faceobjects, int left,
    int right) {
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j) {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j) {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

#pragma omp parallel sections
    {
#pragma omp section
        {
            if (left < j)
                qsort_descent_inplace(faceobjects, left, j);
        }
#pragma omp section
        {
            if (i < right)
                qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

void qsort_descent_inplace(std::vector<Object>& objects) {
    if (objects.empty())
        return;

    qsort_descent_inplace(objects, 0, objects.size() - 1);
}

void nms_sorted_bboxes(const std::vector<Object>& faceobjects,
    std::vector<int>& picked, float nms_threshold) {
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++) {
        areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++) {
        const Object& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++) {
            const Object& b = faceobjects[picked[j]];

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            // float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}
void decode_outputs(const float* prob, std::vector<Object>& objects,
    float scale, const int img_w, const int img_h) {
    std::vector<Object> proposals;
    std::vector<int> strides = { 8, 16, 32 };
    std::vector<GridAndStride> grid_strides;

    generate_grids_and_stride(INPUT_W, strides, grid_strides);
    generate_yolox_proposals(grid_strides, prob, BBOX_CONF_THRESH, proposals);
    qsort_descent_inplace(proposals);

    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked, NMS_THRESH);
    int count = picked.size();
    objects.resize(count);

    for (int i = 0; i < count; i++) {
        objects[i] = proposals[picked[i]];

        // adjust offset to original unpadded
        float x0 = (objects[i].rect.x) / scale;
        float y0 = (objects[i].rect.y) / scale;
        float x1 = (objects[i].rect.x + objects[i].rect.width) / scale;
        float y1 = (objects[i].rect.y + objects[i].rect.height) / scale;

        // clip
        x0 = std::max(std::min(x0, (float)(img_w - 1)), 0.f);
        y0 = std::max(std::min(y0, (float)(img_h - 1)), 0.f);
        x1 = std::max(std::min(x1, (float)(img_w - 1)), 0.f);
        y1 = std::max(std::min(y1, (float)(img_h - 1)), 0.f);

        objects[i].rect.x = x0;
        objects[i].rect.y = y0;
        objects[i].rect.width = x1 - x0;
        objects[i].rect.height = y1 - y0;
    }
}

void draw_objects(const cv::Mat& bgr,
    const std::vector<Object>& objects) {
    static const char* class_names[] = {
        "person",        "bicycle",      "car",
        "motorcycle",    "airplane",     "bus",
        "train",         "truck",        "boat",
        "traffic light", "fire hydrant", "stop sign",
        "parking meter", "bench",        "bird",
        "cat",           "dog",          "horse",
        "sheep",         "cow",          "elephant",
        "bear",          "zebra",        "giraffe",
        "backpack",      "umbrella",     "handbag",
        "tie",           "suitcase",     "frisbee",
        "skis",          "snowboard",    "sports ball",
        "kite",          "baseball bat", "baseball glove",
        "skateboard",    "surfboard",    "tennis racket",
        "bottle",        "wine glass",   "cup",
        "fork",          "knife",        "spoon",
        "bowl",          "banana",       "apple",
        "sandwich",      "orange",       "broccoli",
        "carrot",        "hot dog",      "pizza",
        "donut",         "cake",         "chair",
        "couch",         "potted plant", "bed",
        "dining table",  "toilet",       "tv",
        "laptop",        "mouse",        "remote",
        "keyboard",      "cell phone",   "microwave",
        "oven",          "toaster",      "sink",
        "refrigerator",  "book",         "clock",
        "vase",          "scissors",     "teddy bear",
        "hair drier",    "toothbrush" };

    cv::Mat image = bgr.clone();

    for (size_t i = 0; i < objects.size(); i++) {
        const Object& obj = objects[i];

        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
            obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        cv::Scalar color =
            cv::Scalar(color_list[obj.label][0], color_list[obj.label][1],
                color_list[obj.label][2]);
        float c_mean = cv::mean(color)[0];
        cv::Scalar txt_color;
        if (c_mean > 0.5) {
            txt_color = cv::Scalar(0, 0, 0);
        }
        else {
            txt_color = cv::Scalar(255, 255, 255);
        }

        cv::rectangle(image, obj.rect, color * 255, 2);

        char text[256];
        sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

        int baseLine = 0;
        cv::Size label_size =
            cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.4, 1, &baseLine);

        cv::Scalar txt_bk_color = color * 0.7 * 255;

        int x = obj.rect.x;
        int y = obj.rect.y + 1;
        // int y = obj.rect.y - label_size.height - baseLine;
        if (y > image.rows)
            y = image.rows;
        // if (x + label_size.width > image.cols)
        // x = image.cols - label_size.width;

        cv::rectangle(
            image,
            cv::Rect(cv::Point(x, y),
                cv::Size(label_size.width, label_size.height + baseLine)),
            txt_bk_color, -1);

        cv::putText(image, text, cv::Point(x, y + label_size.height),
            cv::FONT_HERSHEY_SIMPLEX, 0.4, txt_color, 1);
    }
//    cv::imwrite(".\\out.jpg", image);
    cv::imshow("result",image);
    cv::waitKey(10);
//    std::cout << "save output to out.jpg" << std::endl;
}


cg::ComputingGraph::OutputSpecItem make_callback_copy(SymbolVar dev,
    HostTensorND& host) {
    auto cb = [&host](DeviceTensorND& d) { host.copy_from(d); };
    return { dev, cb };
}



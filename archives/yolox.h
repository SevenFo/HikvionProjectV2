#ifndef YOLOX_H
#define YOLOX_H

#include "megbrain/gopt/inference.h"
#include "megbrain/opr/search_policy/algo_chooser_helper.h"
#include "megbrain/serialization/serializer.h"
#include <iostream>
#include <iterator>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <string>
#include <vector>
#include <chrono>


/**
 * @brief Define names based depends on Unicode path support
 */
#define NMS_THRESH 0.45
#define BBOX_CONF_THRESH 0.25

constexpr int INPUT_W = 640;
constexpr int INPUT_H = 640;

using namespace mgb;
struct Object {
    cv::Rect_<float> rect;
    int label;
    float prob;
};

struct GridAndStride {
    int grid0;
    int grid1;
    int stride;
};

const float color_list[80][3] = {
    {0.000, 0.447, 0.741}, {0.850, 0.325, 0.098}, {0.929, 0.694, 0.125},
    {0.494, 0.184, 0.556}, {0.466, 0.674, 0.188}, {0.301, 0.745, 0.933},
    {0.635, 0.078, 0.184}, {0.300, 0.300, 0.300}, {0.600, 0.600, 0.600},
    {1.000, 0.000, 0.000}, {1.000, 0.500, 0.000}, {0.749, 0.749, 0.000},
    {0.000, 1.000, 0.000}, {0.000, 0.000, 1.000}, {0.667, 0.000, 1.000},
    {0.333, 0.333, 0.000}, {0.333, 0.667, 0.000}, {0.333, 1.000, 0.000},
    {0.667, 0.333, 0.000}, {0.667, 0.667, 0.000}, {0.667, 1.000, 0.000},
    {1.000, 0.333, 0.000}, {1.000, 0.667, 0.000}, {1.000, 1.000, 0.000},
    {0.000, 0.333, 0.500}, {0.000, 0.667, 0.500}, {0.000, 1.000, 0.500},
    {0.333, 0.000, 0.500}, {0.333, 0.333, 0.500}, {0.333, 0.667, 0.500},
    {0.333, 1.000, 0.500}, {0.667, 0.000, 0.500}, {0.667, 0.333, 0.500},
    {0.667, 0.667, 0.500}, {0.667, 1.000, 0.500}, {1.000, 0.000, 0.500},
    {1.000, 0.333, 0.500}, {1.000, 0.667, 0.500}, {1.000, 1.000, 0.500},
    {0.000, 0.333, 1.000}, {0.000, 0.667, 1.000}, {0.000, 1.000, 1.000},
    {0.333, 0.000, 1.000}, {0.333, 0.333, 1.000}, {0.333, 0.667, 1.000},
    {0.333, 1.000, 1.000}, {0.667, 0.000, 1.000}, {0.667, 0.333, 1.000},
    {0.667, 0.667, 1.000}, {0.667, 1.000, 1.000}, {1.000, 0.000, 1.000},
    {1.000, 0.333, 1.000}, {1.000, 0.667, 1.000}, {0.333, 0.000, 0.000},
    {0.500, 0.000, 0.000}, {0.667, 0.000, 0.000}, {0.833, 0.000, 0.000},
    {1.000, 0.000, 0.000}, {0.000, 0.167, 0.000}, {0.000, 0.333, 0.000},
    {0.000, 0.500, 0.000}, {0.000, 0.667, 0.000}, {0.000, 0.833, 0.000},
    {0.000, 1.000, 0.000}, {0.000, 0.000, 0.167}, {0.000, 0.000, 0.333},
    {0.000, 0.000, 0.500}, {0.000, 0.000, 0.667}, {0.000, 0.000, 0.833},
    {0.000, 0.000, 1.000}, {0.000, 0.000, 0.000}, {0.143, 0.143, 0.143},
    {0.286, 0.286, 0.286}, {0.429, 0.429, 0.429}, {0.571, 0.571, 0.571},
    {0.714, 0.714, 0.714}, {0.857, 0.857, 0.857}, {0.000, 0.447, 0.741},
    {0.314, 0.717, 0.741}, {0.50, 0.5, 0} };


static inline float intersection_area(const Object& a, const Object& b) {
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

cv::Mat static_resize(cv::Mat& img);
void blobFromImage(cv::Mat& img, float* blob_data);
static inline void
generate_grids_and_stride(const int target_size, std::vector<int>& strides,
    std::vector<GridAndStride>& grid_strides);
static inline void generate_yolox_proposals(std::vector<GridAndStride> grid_strides,
    const float* feat_ptr,
    float prob_threshold,
    std::vector<Object>& objects);

static inline void qsort_descent_inplace(std::vector<Object>& faceobjects, int left,
    int right);
static inline void qsort_descent_inplace(std::vector<Object>& objects);
static inline void nms_sorted_bboxes(const std::vector<Object>& faceobjects,
    std::vector<int>& picked, float nms_threshold);
static inline void decode_outputs(const float* prob, std::vector<Object>& objects,
    float scale, const int img_w, const int img_h);
static inline void draw_objects(const cv::Mat& bgr,
    const std::vector<Object>& objects);
cg::ComputingGraph::OutputSpecItem make_callback_copy(SymbolVar dev,
    HostTensorND& host);


class Yolox
{


public:
    Yolox();
    ~Yolox();

    bool LoadModel(const std::string& input_model, const std::string& device = "cuda", const int thread_number = 1);
    bool Inference(cv::Mat& image, cv::Mat& result);

private:
    serialization::GraphLoader::LoadResult network;

};

#endif // YOLOX_H

#include "faceimagehandler.h"
#include <QDebug>
#include <QSize>
FaceImageHandler::FaceImageHandler(QObject *parent)
    : QObject{parent}
{
    cnn_model_ = new Models(Models::CNN_M,"../onnx_fer_model.onnx");

}

int FaceImageHandler::isTired(const cv::Mat& face_image){
    cv::Mat sface_image = face_image.clone();
    cv::cvtColor(sface_image,sface_image,cv::COLOR_RGB2GRAY);
    auto eyes_roi = sface_image.rowRange(cv::Range(0,sface_image.rows/2));
//    cv::imshow("eyes_roi",eyes_roi);
    cv::CascadeClassifier eye_cascade("../haarcascade_eye.xml");
    std::vector<cv::Rect> faces,eyes;
    eye_cascade.detectMultiScale(eyes_roi ,eyes,1.1,3,0,cv::Size(10,10));
    std::cout<<"Detected "<<eyes.size()<<" eyes!"<<std::endl;
    std::vector<cv::Point2d> pupil;
    if(eyes.size() > 2){
        std::cout <<"more than 2 eyes"<<std::endl;
        for(auto &eye:eyes){
            cv::rectangle(face_image,eye,100,2);
        }
        std::cout<<std::endl;
        return 0;
    }
    else if(eyes.size() < 2){
        std::cout <<"no eyes"<<std::endl;
//        emit predictFaceExprerationFinished(QString("未监测到人脸"),0.00,QVector<float>{0,0,0,0,0,0,0});
        return 1;
    }
    else{
        for(auto &eye:eyes){
            cv::rectangle(face_image,eye,100,2);
        }
        pupil.push_back(cv::Point2d((eyes.at(0).x + eyes.at(0).width)/2,(eyes.at(0).y + eyes.at(0).height)/2));
        pupil.push_back(cv::Point2d((eyes.at(1).x + eyes.at(1).width)/2,(eyes.at(1).y + eyes.at(1).height)/2));
        std::cout << "eye position:"<<pupil<<std::endl;
    }
    return 0;
}

int FaceImageHandler::run(const QPixmap& image,const qint64 w, const qint64 h)
{
    cv::Mat image_ = FaceImageHandler::QPixmapToCvMat(image.scaled(QSize{(int)w,(int)h}));
    unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
    if(!pBuffer)
    {
        fprintf(stderr, "Can not alloc buffer.\n");
        return -1;
    }
    int * pResults = NULL;

    pResults = facedetect_cnn(pBuffer, (unsigned char*)(image_.ptr(0)), image_.cols, image_.rows, (int)image_.step);
    if(1 != (pResults ? *pResults : 0)){
//        qDebug()<<"unsuitable face";
        return -1;
    }
    short * p = ((short*)(pResults+1));
    int confidence = p[0];
    if(confidence < 90){
//        qDebug()<<"confidence to small:"<<confidence;
        return -1;
    }
    int x = p[1];
    int y = p[2];
    int w_ = p[3];
    int h_ = p[4];

    cv::Mat face_image = image_.clone();
    cv::Mat small_face_image;
    face_image = face_image(cv::Range(y,y+h_),cv::Range(x,x+w_));
    cv::resize(face_image,small_face_image,cv::Size(),0.8,0.8);
    int is_tired = isTired(small_face_image);
    emit handleImageFinished(QPixmap::fromImage(MatToQImage(small_face_image)),is_tired);

    //预测表情
    std::string result_label;
    float result_p;
    std::vector<float> result_ps;
    int index{0};
    if(! cnn_model_->runCNN(face_image,result_label,index,result_p,result_ps)){
        qDebug()<<"get experation failed!";
        return -1;
    }
    emit predictFaceExprerationFinished(QString::fromStdString(result_label),index,result_p,QVector<float>(result_ps.begin(),result_ps.end()));

    return 0;
}

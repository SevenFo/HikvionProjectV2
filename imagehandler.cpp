#include "imagehandler.h"
#include <vector>

ImageHandler::ImageHandler(QObject *parent)
    : QObject{parent}
{

}

ImageHandler::~ImageHandler(){

}



void ImageHandler::handle_image(const QPixmap& image,const qint64 w, const qint64 h){
    cv::Mat face_image = ImageHandler::QPixmapToCvMat(image);
    cv::Mat rotated_image;
    cv::cvtColor(face_image,face_image,cv::COLOR_RGB2GRAY);
    auto eyes_roi = face_image.rowRange(cv::Range(0,face_image.rows/2));
    cv::CascadeClassifier face_cascade("../haarcascade_frontalface_default.xml");
    cv::CascadeClassifier eye_cascade("../haarcascade_eye.xml");
    std::vector<cv::Rect> faces,eyes;
    eye_cascade.detectMultiScale(eyes_roi ,eyes,1.1,3,0,cv::Size(50,50));
    std::cout<<"Detected "<<eyes.size()<<" eyes!"<<std::endl;
    std::vector<cv::Point2d> pupil;
    if(eyes.size() > 2){
        std::cout <<"more than 2 eyes"<<std::endl;
        for(auto eye : eyes){
            std::cout<<eye<<", ";
        }
        std::cout<<std::endl;
        int biggest{0}, second_biggest{0};
        cv::Rect biggest_eye, second_biggest_eye;
        for(auto eye : eyes){
            if(eye.area() > biggest_eye.area())
                biggest_eye = eye;
            else if(eye.area() > second_biggest_eye.area()){
                second_biggest_eye = eye;
            }
        }
            cv::rectangle(face_image,biggest_eye,100,10);
            cv::rectangle(face_image,second_biggest_eye,100,10);
        pupil.push_back(cv::Point2d((biggest_eye.x + biggest_eye.width)/2,(biggest_eye.y + biggest_eye.height)/2));
        pupil.push_back(cv::Point2d((second_biggest_eye.x + second_biggest_eye.width)/2,(second_biggest_eye.y + second_biggest_eye.height)/2));
        std::cout << "eye position:"<<pupil<<std::endl;
    }
    else if(eyes.size() < 2){
        std::cout <<"no eyes"<<std::endl;
        return ;
    }
    else{
        for(auto &eye:eyes){
            cv::rectangle(face_image,eye,100,10);
        }
        pupil.push_back(cv::Point2d((eyes.at(0).x + eyes.at(0).width)/2,(eyes.at(0).y + eyes.at(0).height)/2));
        pupil.push_back(cv::Point2d((eyes.at(1).x + eyes.at(1).width)/2,(eyes.at(1).y + eyes.at(1).height)/2));
        std::cout << "eye position:"<<pupil<<std::endl;
    }
    _rotate_image(face_image,rotated_image,pupil.at(0).x>pupil.at(1).x?pupil.at(1):pupil.at(0),pupil.at(0).x>pupil.at(1).x?pupil.at(0):pupil.at(1));
//    cv::imshow("rotated",rotated_image);
//    cv::waitKey();
    //rotate test code

//    auto test_image = cv::imread("../dog.jpg");
//    cv::Mat des;

    //totate test pass
    emit handleImageFinished(QPixmap::fromImage(MatToQImage(rotated_image)));
    return ;

}

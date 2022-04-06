#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QPixmap>
#include <QDebug>
#include "opencv2/opencv.hpp"
#include <cmath>

#define PI (acos(-1))


class ImageHandler : public QObject
{
    Q_OBJECT
public:
    explicit ImageHandler(QObject *parent = nullptr);
    ~ImageHandler();

    inline QImage MatToQImage(cv::Mat mtx)
    {
        switch (mtx.type())
        {
        case CV_8UC1:
            {
                QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols, QImage::Format_Grayscale8);
                return img;
            }
            break;
        case CV_8UC3:
            {
                QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 3, QImage::Format_RGB888);
                return img.rgbSwapped();
            }
            break;
        case CV_8UC4:
            {
                QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 4, QImage::Format_ARGB32);
                return img;
            }
            break;
        default:
            {
                QImage img;
                return img;
            }
            break;
        }
    }

    inline cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData = true )
    {
        switch ( inImage.format() )
        {
        // 8-bit, 4 channel
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        {
            cv::Mat  mat( inImage.height(), inImage.width(),
                          CV_8UC4,
                          const_cast<uchar*>(inImage.bits()),
                          static_cast<size_t>(inImage.bytesPerLine())
                          );

            return (inCloneImageData ? mat.clone() : mat);
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB32:
        case QImage::Format_RGB888:
        {
            if ( !inCloneImageData )
            {
                qWarning() << "CVS::QImageToCvMat() - Conversion requires cloning because we use a temporary QImage";
            }

            QImage   swapped = inImage;

            if ( inImage.format() == QImage::Format_RGB32 )
            {
                swapped = swapped.convertToFormat( QImage::Format_RGB888 );
            }

            swapped = swapped.rgbSwapped();

            return cv::Mat( swapped.height(), swapped.width(),
                            CV_8UC3,
                            const_cast<uchar*>(swapped.bits()),
                            static_cast<size_t>(swapped.bytesPerLine())
                            ).clone();
        }

        // 8-bit, 1 channel
        case QImage::Format_Indexed8:
        {
            cv::Mat  mat( inImage.height(), inImage.width(),
                          CV_8UC1,
                          const_cast<uchar*>(inImage.bits()),
                          static_cast<size_t>(inImage.bytesPerLine())
                          );

            return (inCloneImageData ? mat.clone() : mat);
        }

        default:
            qWarning() << "CVS::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
            break;
        }

        return cv::Mat();
    }

    inline cv::Mat QPixmapToCvMat( const QPixmap &inPixmap, bool inCloneImageData = true )
    {
        return QImageToCvMat( inPixmap.toImage(), inCloneImageData );
    }

    inline cv::Mat _rotate_image(const cv::Mat &in, cv::Mat &out, const cv::Point2d &left_eye, const cv::Point2d &right_eye)
    {
        auto x0 = (left_eye.x + right_eye.x)/2;
        auto y0 = (left_eye.y + right_eye.y)/2;

        auto x1 = left_eye.x;
        auto y1 = left_eye.y;

        auto x2 = right_eye.x;
        auto y2 = right_eye.y;

        auto dx =  x1-x0;
        auto dy =  y1-y0;

        auto alpha = std::atan(dy/dx)/PI * 180.0;
//        std::cout<<"x0:"<<x0<<"y0:"<<y0<<"x1:"<<x1<<"y1:"<<y1<<"x2:"<<x2<<"y2:"<<y2<<"  dx:"<<dx<<",dy"<<dy<<" rotate degree:"<<alpha<<std::endl;

        cv::Point2f center(in.cols/2,in.rows/2);

        auto rotate_m = cv::getRotationMatrix2D(center,alpha,1);
        cv::warpAffine(in,out,rotate_m,cv::Size(in.cols,in.rows));

        return out;

    }

    void handle_image(const QPixmap &face_image,const qint64 w, const qint64 h);
signals:
    void handleImageFinished(QPixmap pixmap);

};

#endif // IMAGEHANDLER_H

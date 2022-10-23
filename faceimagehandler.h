#ifndef FACEIMAGEHANDLER_H
#define FACEIMAGEHANDLER_H
#include <opencv2/opencv.hpp>
#include "facedetectcnn.h"
#include "models.h"
#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QDebug>
#define DETECT_BUFFER_SIZE 0x20000

class FaceImageHandler : public QObject
{
    Q_OBJECT

public:
    explicit FaceImageHandler(QObject *parent = nullptr);
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
            cv::Mat result = cv::Mat( swapped.height(), swapped.width(),
                                      CV_8UC3,
                                      const_cast<uchar*>(swapped.bits()),
                                      static_cast<size_t>(swapped.bytesPerLine())
                                      ).clone();
            return result;
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

    int run(const QPixmap& image,const qint64 w, const qint64 h);
private:
    Models* cnn_model_;
    int isTired(const cv::Mat& face_image);
signals:
    void handleImageFinished(QPixmap pixmap, int is_tired);
    void predictFaceExprerationFinished(QString result_label,int index, float p, QVector<float> ps);

};

#endif // FACEIMAGEHANDLER_H

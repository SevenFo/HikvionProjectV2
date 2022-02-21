#include "imageviewer.h"
#include <QImage>
#include <QPainter>
#include <QFile>

IMageViewer::IMageViewer(QQuickItem*parent):QQuickPaintedItem(parent)
{
    image = QImage();


}
IMageViewer::~IMageViewer()
{

}

void IMageViewer::paint(QPainter *painter)
{
    qDebug()<<"image size"<<image.size();
////    image.save("convetedresult.jpg");
//    auto t = QImage(image.width(),image.height(),QImage::Format_RGB32);
//    QByteArray buf;
//    image = QImage("convetedresult.jpg");
//    painter->drawImage(this->boundingRect(),image);
    painter->drawPixmap(this->boundingRect(),pixmap,QRectF(0,0,pixmap.height(),pixmap.width()));
}
void IMageViewer::updateImage()
{
    IMageViewer::update();
}
void IMageViewer::updateImage(QByteArray data, int w, int h)
{
    IMageViewer::update();
}
void IMageViewer::UpdateImage(QBuffer *buf)
{

}
void IMageViewer::updateImage(QImage i)
{
//    pixmap = QPixmap::fromImage(i);
    image = i;
    image.bits();
    qDebug()<<"display image heigh:"<<image.size().height();
    qDebug()<<"display image width:"<<image.size().width();
    qDebug()<<"display image data size:"<<image.sizeInBytes();
    IMageViewer::update();

}

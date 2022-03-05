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
    //The pixmap is scaled to fit the rectangle, if both the pixmap and rectangle size disagree.
    painter->drawPixmap(this->boundingRect(),pixmap,QRectF(0,0,pixmap.width(),pixmap.height()));
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
void IMageViewer::updateImage(QPixmap &mp){
    pixmap = mp;
    update();
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

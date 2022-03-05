#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QQuickPaintedItem>
#include <QObject>
#include <QQuickItem>
#include <hikvisonhandler.h>
#include <hikvisonhandlercstyelfuncs.h>
#include <QImage>
#include <QBuffer>
#include <QPixmap>

class IMageViewer : public QQuickPaintedItem
{
    Q_OBJECT
public:
    IMageViewer(QQuickItem *parent = 0);
    ~IMageViewer();

    Q_PROPERTY(QPixmap pixmap READ getPixmap WRITE updateImage);

protected:
    void paint(QPainter *painter);
    QImage image;
    QPixmap pixmap;
public slots:
    void updateImage();
    void updateImage(QByteArray data,int w, int h);
    void UpdateImage(QBuffer *buf);
    void updateImage(QImage i);
    void updateImage(QPixmap &pm);

    inline QPixmap getPixmap(){
        return pixmap;
    }


private:

};

#endif // IMAGEVIEWER_H

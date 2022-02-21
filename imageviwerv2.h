#ifndef IMAGEVIWERV2_H
#define IMAGEVIWERV2_H

#include <QQuickItem>

#include <QSGNode>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>
#include <QImage>

class ImageViwerV2 : public QQuickItem
{
    Q_OBJECT
public:
    explicit ImageViwerV2(QQuickItem *parent = nullptr);

    void updateImage(const QImage &);
signals:

protected:
    QSGNode * updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    QImage image;
};

#endif // IMAGEVIWERV2_H

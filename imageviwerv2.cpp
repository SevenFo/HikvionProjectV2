#include "imageviwerv2.h"

ImageViwerV2::ImageViwerV2(QQuickItem *parent):QQuickItem(parent)
{
    setFlag(ItemHasContents, true);

    image = QImage("dog.jpg");

}

void ImageViwerV2::updateImage(const QImage &i)
{
    image= i;
    update();
}

QSGNode * ImageViwerV2::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    auto node = dynamic_cast<QSGSimpleTextureNode *>(oldNode);

    if(!node){
        node = new QSGSimpleTextureNode();
    }

    QSGTexture *m_texture = window()->createTextureFromImage(image, QQuickWindow::TextureIsOpaque);
    node->setOwnsTexture(true);
    node->setRect(boundingRect());
    node->markDirty(QSGNode::DirtyForceUpdate);
    node->setTexture(m_texture);

    return node;

}

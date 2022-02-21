#include "mainwindow.h"
#include <QApplication>
#include <QQuickWindow>
#include <QImage>
#include <hikvisonviewer.h>
#include <imageviewer.h>
#include <imageviwerv2.h>

int main(int argc, char *argv[])
{

//    Detector d;
//    QImage img("dog.jpg");
//    d.Init();
//    d.Detect(img);

    qmlRegisterType<IMageViewer>("siky.ImageViewer",1,0,"ImageViewer");
    qmlRegisterType<ImageViwerV2>("siky.ImageViwerV2",1,0,"ImageViwerV2");

    qmlRegisterType<HikvisonViewer>("siky.HikvisonViewer",1,0,"HikvisonViewer");
    QApplication a(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::GraphicsApi::OpenGL);
    MainWindow w;
    w.show();

    return a.exec();
}

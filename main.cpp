#include "mainwindow.h"
#include <QApplication>
#include <QQuickWindow>
#include <QImage>
#include <hikvisonviewer.h>
#include <imageviewer.h>
#include <imageviwerv2.h>


int main(int argc, char *argv[])
{

#ifdef __linux__
    std::cout <<"Run in linux!"<<std::endl;
#else
    std::cout <<"Run in NOT linux!"<<std::endl;
#endif
    qmlRegisterType<IMageViewer>("siky.ImageViewer",1,0,"ImageViewer");
    qmlRegisterType<HikvisonViewer>("siky.HikvisonViewer",1,0,"HikvisonViewer");
    QApplication a(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::GraphicsApi::OpenGL);
    MainWindow w;
    w.show();

    return a.exec();
}

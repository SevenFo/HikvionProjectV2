#include "cardatahandler.h"

CarDataHandler::CarDataHandler(QObject *parent)
    : QObject{parent}
{
    list_speeds = new QVector<float>;
    list_brake_angle = new QVector<float>;
    list_steering_angles = new QVector<float>;
}

CarDataHandler::~CarDataHandler(){

    delete list_speeds;
    delete list_brake_angle;
    delete list_steering_angles;
}

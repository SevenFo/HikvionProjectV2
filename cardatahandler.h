#ifndef CARDATAHANDLER_H
#define CARDATAHANDLER_H

#include <QObject>
#include <QVector>

class CarDataHandler : public QObject
{
    Q_OBJECT
public:

    typedef enum LEVEL{
       LOW, MIDDLE, HIGH
    } OverSpeedLevel;


    explicit CarDataHandler(QObject *parent = nullptr);
    ~CarDataHandler();

    inline void addSpeed(const float speed, const float max_speed){
        checkSpeed_(speed,max_speed);
        if(list_speeds->count() >= num_speeds_max)
            list_speeds->takeFirst();
        list_speeds->append(speed);
    }

    inline void addSteeringAngle(const float steering_angle, const float max_diff_steering_angle){
        checkSteeringAngleDiff(steering_angle,max_diff_steering_angle);
        if(list_steering_angles->count() >= num_steering_angles_max)
            list_steering_angles->takeFirst();
        list_steering_angles->append(steering_angle);
    }
    inline void addBrakeAngle(const float brake_angle, const float max_diff){
        checkBrakeAngleDiff(brake_angle,max_diff);
        if(list_brake_angle->count() >= num_brake_angles_max)
            list_brake_angle->takeFirst();
        list_brake_angle->append(brake_angle);
    }

    inline unsigned char getSpeedLevel() const{
        return this->speed_level;
    }

private:

    int count_overspeed{0};
    unsigned char speed_level{0};
    const int num_speeds_max{10}, num_steering_angles_max{10}, num_brake_angles_max{10};

    inline void checkSpeed_(const float &speed, const float&max_speed){
        speed_level = 0;
        if(speed <= max_speed){
            count_overspeed = 0;
            return;
        }
        if(++count_overspeed < 2){
            return;
        }
        if(speed*10/max_speed < 11){
            speed_level = 1;
            emit overspeed(LOW);
        }
        else if(speed*10/max_speed <12){
            speed_level = 1;
            emit overspeed(MIDDLE);
        }
        else
        {
            speed_level = 1;
            emit overspeed(HIGH);
        }
    }

    inline void checkSteeringAngleDiff(const float steering_angle, const float max_diff){
        if(list_steering_angles->count() < 2)
            return;
        if(std::abs(steering_angle - list_steering_angles->last()) > max_diff)
            emit overDiffSteeringAngle();
    }
    inline void checkBrakeAngleDiff(const float brake_angle, const float max_diff){
        if(list_brake_angle->count() < 2)
            return;
        if(std::abs(brake_angle - list_brake_angle->last()) > max_diff)
            emit overDiffBrakeAngle();
    }

    QVector<float> *list_speeds, *list_steering_angles, *list_brake_angle;


signals:
    void overspeed(const enum LEVEL level);
    void overDiffSteeringAngle();
    void overDiffBrakeAngle();

};

#endif // CARDATAHANDLER_H

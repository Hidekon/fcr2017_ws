#ifndef _MOVEMENT_H_
#define _MOVEMENT_H_

#include "ros/ros.h"
#include "../namespaces/parameters.h"
#include "../namespaces/angleOps.h"
#include "geometry_msgs/Point.h"
#include "measurements/odometry_subscriber.h"
#include "movement/kinematics.h"
#include "pid.h"
#include <stdexcept>

class Movement
{
public:
    Movement(int argc, char *argv[]);
    ~Movement();

    /* Time management loop */
    void movementTimeLoop(ros::Duration);

    /* Open Loop Movements */
    void moveMeters(float, float);
    ros::Duration timeToDistance(float, float);
    void spinDegrees(float, float);
    ros::Duration timeToAngle(float, float);
    void stopMoving();

    /* Closed Loop Movements */
    double orientationError(geometry_msgs::Point);
    double locationError(geometry_msgs::Point);
    void go_to_goal(geometry_msgs::Point, float);

    /* Extern Interfaces */
    inline std::array<double, 3> getOdometry(){return this->odometryMonitor->getOdometry();};

    OdometySubscriber*    odometryMonitor;
    Kinematics* moveCommands;
    PID* anglePID;
};

#endif

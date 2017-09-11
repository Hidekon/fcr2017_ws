#include "../include/navigation.h"

Navigation::Navigation(int argc, char *argv[])
{
    this->moveCommands = new Kinematics(argc, argv);
    this->laserMonitor = new LaserSubscriber(argc, argv);
    this->odometryMonitor = new OdometySubscriber(argc, argv);
    this->sonarMonitor = new UltrasoundSubscriber(argc, argv);
}

Navigation::~Navigation()
{
    delete moveCommands;
    delete laserMonitor;
    delete odometryMonitor;
    delete sonarMonitor;
}

/* Open Loop Movements */

void Navigation::movementTimeLoop(ros::Duration timeMoving)
{
    auto begin = ros::Time::now();
    for(ros::Time timeLoop = ros::Time::now(); (timeLoop-begin) < timeMoving; timeLoop = ros::Time::now())
    {

    }
}

void Navigation::moveMeters(float distance, float linearVel)
{
    auto timeMoving = timeToDistance(distance, linearVel);
    this->moveCommands->moveLinear(linearVel);

    this->movementTimeLoop(timeMoving);

    this->moveCommands->moveStop();
}

ros::Duration Navigation::timeToDistance(float distance, float linearVel)
{
    auto timeMoving = std::abs(distance / linearVel);
    if(timeMoving < 0)
    {
        throw std::logic_error("Negative time deducted when moving");
        return ros::Duration(0);
    }
    else
        return ros::Duration(timeMoving);
}

void Navigation::spinDegrees(float angle, float angularVel)
{
    angle = angleOps::degreesToRadians(angle);
    auto timeSpinning = timeToAngle(angle, angularVel);
    this->moveCommands->moveAngular(angularVel);

    this->movementTimeLoop(timeSpinning);

    this->moveCommands->moveStop();
}

ros::Duration Navigation::timeToAngle(float angle, float angularVel)
{
    auto timeMoving = std::abs(angle / angularVel);
    if(timeMoving < 0)
    {
        throw std::logic_error("Negative time deducted when spinning");
        return ros::Duration(0);
    }
    else
        return ros::Duration(timeMoving);
}

void Navigation::stopMoving()
{
    this->moveCommands->moveStop();
    this->odometryMonitor->printOdometry();
    this->sonarMonitor->printSonar();
    // this->laserMonitor->printLaser();
}

/* Closed Loop Movements */
/*      Control Loops       */
double Navigation::orientationError(geometry_msgs::Point point)
{
    double diffX, diffY;
    diffX = point.x - this->odometryMonitor->X;
    diffY = point.y - this->odometryMonitor->Y;
    double goalOrientation = std::atan2(diffY, diffX);
    // goalOrientation = angleOps::constrainAngle(goalOrientation);
    double currentOrientation = this->odometryMonitor->Yaw;
    /* Get instantaneus orientation ajustment */
    double diffOrientation;
    diffOrientation = goalOrientation - currentOrientation;
    return(diffOrientation);
}

void Navigation::adjustOrientation(geometry_msgs::Point point, float vel, double tolerance)
{
    /* Proportional gain */
    float Kp = 1;
    float Ki = 0.01;
    float Kd = 0.01;
    double error = orientationError(point);
    double oldError = 0;
    double accumError = 0;
    float maxVel = 5*vel;
    /* Loop until orientation is adjusted */
    // Supports negative or positive angles
    while(std::abs(error) > tolerance)
    {
        float PID = error * Kp + Ki * accumError + Kd * (error - oldError);
        float adjustVel = std::abs(PID) < maxVel? PID : maxVel;
        this->moveCommands->moveAngular(adjustVel);
        oldError = error;
        accumError = error + accumError;
        error = orientationError(point);
    }
}

double Navigation::locationError(geometry_msgs::Point point)
{
    double diffX, diffY;
    diffX = point.x - this->odometryMonitor->X;
    diffY = point.y - this->odometryMonitor->Y;
    double locationError;
    locationError = sqrt(pow(diffX,2) + pow(diffY,2));
    return locationError;
}

void Navigation::adjustPosition(geometry_msgs::Point point, float vel, double toleranceModule)
{
    /* Proportional gain */
    float Kp = 1;
    float Ki = 0;
    float Kd = 0;
    double error = locationError(point);
    double oldError = 0;
    double accumError = 0;
    float maxVel = vel;
    /* Loop until position is adjusted */
    while(std::abs(error) > toleranceModule)
    {
        float PID = error * Kp + Ki * accumError + Kd * (error - oldError);
        float adjustVel = std::abs(PID) < maxVel? PID : maxVel;
        /* Stop angular movement, keeps only linear */
        this->moveCommands->moveLinear(adjustVel);
        oldError = error;
        accumError = error + accumError;
        error = locationError(point);
    }
}

void Navigation::moveToPosition(geometry_msgs::Point point, float vel)
{
    /* Tolerance for both orientation and localization */
    float toleranceAngle = 0.05;
    double toleranceModule = 0.05;
    /* Loop until orientation is adjusted */
    this->adjustOrientation(point, vel, toleranceAngle);
    /* Loop until position is adjusted */
    this->adjustPosition(point, vel, toleranceModule);
    /* End Movement */
    this->stopMoving();
}

void Navigation::nonStopFollow(geometry_msgs::Point point, float vel)
{
    /* Tolerance for both orientation and localization */
    float toleranceAngle = 0.01;
    float toleranceModule = 0.1;

    /* PID setup */
    float Kp = 2;
    float Ki = 0;
    float Kd = 0;
    double error = orientationError(point);
    double oldError = 0;
    double accumError = 0;
    float maxVel = 5*vel;
    /* Loop until orientation is adjusted and location is reached */
    double positionError = locationError(point);
    while(std::abs(error) > toleranceAngle || std::abs(positionError) > toleranceModule)
    {
        float PID = error * Kp + Ki * accumError + Kd * (error - oldError);
        // float adjustVel = std::abs(PID) < maxVel? PID : maxVel;
        float adjustVel = PID;
        this->moveCommands->moveAndSpin(vel, adjustVel);
        oldError = error;
        accumError = error + accumError;
        error = orientationError(point);
        positionError = locationError(point);
    }

    /* End Movement */
    this->stopMoving();
}

std::vector<geometry_msgs::Point> squarePoints()
{
    std::vector<geometry_msgs::Point> vecPoints;
    int arrayX[] { -1, -1, 0, 0 };
    int arrayY[] { 0, 1, 1, 0 };
    for(size_t i = 0; i < 4; i++)
    {
        geometry_msgs::Point point;
        point.x = arrayX[i];
        point.y = arrayY[i];
        vecPoints.push_back(point);
    }
    return vecPoints;
}

int main(int argc, char *argv[])
{
    Navigation navigate(argc, argv);
    ros::Rate loop_rate(100);
    loop_rate.sleep();
    float vel = 0.2;
    float angVel = 0.2;
    std::vector<geometry_msgs::Point> vecPoints;
    vecPoints = squarePoints();
    while(ros::ok())
    {
        char c = 0;
        std::cout << "Say the command\n-> ";
        std::cin >> c;
        switch (c)
        {
        case 'w':
            navigate.moveMeters(1, vel);
            break;
        case 'a':
            navigate.spinDegrees(90, angVel);
            break;
        case 'd':
            navigate.spinDegrees(90, -angVel);
            break;
        case 's':
            navigate.stopMoving();
            break;
        case 'p':
            for(size_t i = 0; i < vecPoints.size(); i++)
            {
                std::cout << "Heading to point ["
                          << vecPoints[i].x << ", "
                          << vecPoints[i].y << "] "
                          << '\n';
                navigate.moveToPosition(vecPoints[i], vel);
            }
            break;
        case 'n':
            for(size_t i = 0; i < vecPoints.size(); i++)
            {
                std::cout << "Heading to point ["
                          << vecPoints[i].x << ", "
                          << vecPoints[i].y << "] "
                          << '\n';
                navigate.nonStopFollow(vecPoints[i], vel);
            }
            break;
        default:
            break;
        }
        if(c == 'q')
            break;
    }
}

#include "../include/navigation.h"

bool startMenu();

int main(int argc, char *argv[])
{
    Navigation navigate(argc, argv);
    ros::spinOnce();

    geometry_msgs::Point point;

    while(ros::ok() && startMenu())
    {
        std::cout << "Where would you like to go? \n"
                  << "Insert X value:\n-> ";
        std::cin >> point.x;
        std::cout << "Insert Y value:\n-> ";
        std::cin >> point.y;

        std::cout << "Heading to point [" << point.x << ", " << point.y << "]" << '\n';

        navigate.go_to_goal(point);

        navigate.stopMoving();
    }
    return 0;
}

bool startMenu()
{
    std::cout << "Hello Human o/" << '\n';
    std::cout << "What would you like me to do?" << '\n';
    std::cout << "\tm --- move to a point" << '\n';
    std::cout << "\tq --- quit and sleep" << '\n';
    std::cout << "-> ";
    while (true)
    {
        char c;
        std::cin >> c;
        switch (c)
        {
            case 'm':
                return true;
                break;
            case 'q':
                return false;
                break;
            default:
                std::cout << "Oops, I didn't get it\ntry again: -> " << '\n';
                break;
        }
    }
}

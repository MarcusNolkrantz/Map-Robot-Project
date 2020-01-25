/*

file: communication.hpp
author: marno874, osklu414, felli675, edwjo109.
created: 2019-11-14

Main runner class.

Calculates robot behaviour, keeps track of robot position,
creates a representation of the map, sends data to pc program.

*/

#ifndef COMMUNICATION_HPP
#define COMMUNICATION_HPP

#include <ctime>
#include <string>
#include <memory>
#include "sensor.hpp"
#include "steering.hpp"
#include "rplidar.hpp"
#include "pc.hpp"
#include "socket.hpp"
#include "map.hpp"


enum class RobotMode
{
    MANUAL = 0,
    AUTONOMOUS = 1
};

enum class Mode : int {
    MOVING = 0,
    ROTATING_LEFT = 1,
    ROTATING_RIGHT_1 = 2,
    ROTATING_RIGHT_2 = 3,
    ROTATING_RIGHT_3 = 4
};

class Communication {

public:
    Communication
    (
        const std::string& sensor_file,
        const std::string& steering_file,
        const std::string& rplidar_file
    );
    ~Communication();
    /*transmit to and receive from AVRs, returns false when stopped*/
    bool update(); 

private:
    //-------Variables-------------------
    Map map;
    Sensor sensor;
    Steering steering;
    RPLidar rplidar;
    std::shared_ptr<PC> pc;
    Mode mode;
    Direction direction;
    
    int prev_dist;
    int y_pos;
    int x_pos;
	int target_dist; 
    float start_rot; 
    float target_rot;
    RobotMode robot_mode;

    //------Functions----------------------------------
    /*This function updates the position of x and y coordinate
     and calculates and updates the position in squares.*/
    void update_pos(std::vector<ScanNode>& nodes, float);
    /*This function calculates what the robot should do next
    depending on the current mode and sensor values.*/
    bool calc_inst(SensorMeasurement& sensor_measurments, vector<ScanNode>& nodes);
    /*This function returns the distance at a given angle measured with rplidar,
    returns 0 if no distance at that angle.*/
    float get_distance_at(float angle, std::vector<ScanNode>& nodes, float rot);
    /*This function updates the current moving direction when turning right.*/
    Direction right_turn(Direction dir);
    /*This function updates the current moving direction when turning left.*/
    Direction left_turn(Direction dir);
    /*Fix the position to the closest square */
    void correct_position();
    /*Update map*/
    void update_map(const std::vector<ScanNode>& nodes, const SensorMeasurement& measurement);
    /*Starts the autonomous mode*/
    void autonomous_init();
    /*Get most recent rplidar scan*/
    void get_rplidar_scan(std::vector<ScanNode>& curr_nodes, bool& new_data); 
};


#endif

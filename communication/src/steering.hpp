/*

file: steering.hpp
author: marno874, osklu414, felli675, edwjo109.
created: 2019-11-14

Choose what instructions that should be sent to steering module (AVR).

*/

#ifndef STEERING_HPP
#define STEERING_HPP

#include <stdint.h>
#include <ctime>
#include <string>

#include "module.hpp"
#include "serial.hpp"


enum class Rotation : int {
    LEFT = 0,
    RIGHT = 1,
    NONE = 2
};

enum class Direction : int {
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3
};


enum class SteeringTx : uint8_t
{
    PWM = 1,
    DIR = 2,
    IDENTIFIED = 3,
};


enum class SteeringCommand : uint8_t
{
	ROTATE_LEFT		= 0,
	ROTATE_RIGHT    = 1,
	DRIVE_FORWARD	= 2,
	DRIVE_BACKWARD	= 3,
	DRIVE_LEFT		= 4,
	DRIVE_RIGHT		= 5,
	HALT			= 6
};


struct SteeringControl
{
    float left_speed;
    float right_speed;
    bool left_forward;
    bool right_forward;
};

class Steering : public Module {
public:
    Steering(const std::string& file);
    ~Steering();

    //---------Variables----------------------
    Rotation rotation;

    //---------Functions----------------------
    /*Central function for autonomous mode. Everytime the robots movement is changed it has to go through this function.*/
    virtual void update();
    /*This function is used in manual mode, updates steering and speed depending on command*/
    void command(const SteeringCommand command);
    /*Sets values of regulation constans kp, kd*/
    void calibrate(float kp, float kd);
    /*Sets robot rotation (left, right, none).*/
    void set_rotation(Rotation rotation);
    /*Makes the robot rotate slower when approching prefered angle.*/
    void rotate_regulated(float rot);
    /*Sets variables needed for regulation.*/
    void update_regulation(float dist, float rot, bool, float);

private:
    //-------Variables---------------
    float kp, kd;
    SteeringControl latest_control;
    Rotation prev_rotation;
    std::clock_t clock_steering;
    float side_dist;
    float front_dist;
    float d_rot;
    bool regulate;

    //----------Functions-------------
    /*Creates an array containing three bytes (pwm_header, left_pwm, right_pwm) and passes it to serial class*/
    void transmit_pwm(uint8_t left_pwm, uint8_t right_pwm);
    /*Creates an array containing three bytes (dir_header, left_dir, right_dir) and passes it to serial class*/
    void transmit_dir(bool left_forward, bool right_forward);
    /*Creates an array containing one bytes (identified_header) and passes it to serial class*/
    void transmit_identified();
    /*Here is the regulation when robot moves forward implemented. Sets direction and speed of wheelpairs depending
    on sensor values*/ 
    void move_forward();
    /*Given a struct steering control, calls control_speed and control_direction.*/
    void control(const SteeringControl& control);
    /*Convert speed from a value between 0-1 to a value between 100-255. Passes new value to pc and to transmit function*/
    void control_speed(float left_speed, float right_speed);
    /*Passes value of direction to transmit function and to pc.*/
    void control_direction(bool left_forward, bool right_forward);
};

#endif

/*

file: steering.cpp
author: marno874, osklu414, felli675, edwjo109.
created: 2019-11-14

Choose what instructions that should be sent to steering module (AVR).

*/

#include "steering.hpp"
#include "logging.hpp"
#include "pc.hpp"
#include <ctime>

//Tune this depending on battery power
#define MAX_SPEED 0.15f
#define NEAR_WALL_SPEED 0.03f
#define NEAR_WALL_DIST 650
#define ROT_SPEED 0.15f
#define FORWARD_SPEED 0.1f


Steering::Steering(const std::string& file) :
	Module(file),
	kp(1.5f),
	kd(1.0f),
	latest_control(),
	rotation(Rotation::NONE),
	prev_rotation(Rotation::NONE),
    clock_steering(std::clock()),
    side_dist(0),
    front_dist(0),
    d_rot(0),
    regulate(true)
{
    transmit_identified();
}


Steering::~Steering() {
    command(SteeringCommand::HALT);
}


void 
Steering::set_rotation(Rotation rot) {
    rotation = rot;
}


void 
Steering::update() {
    //Check if changed state
    if(prev_rotation != rotation){
		control_speed(0.0f, 0.0f);
        control_direction(false, false);
		
		switch (rotation) {
			case Rotation::NONE:
				control_direction(true, true);
                control_speed(FORWARD_SPEED, FORWARD_SPEED);
				break;
			case Rotation::LEFT:
				control_direction(false, true);
                control_speed(ROT_SPEED, ROT_SPEED);
				break;
			case Rotation::RIGHT:
				control_direction(true, false);
                control_speed(ROT_SPEED, ROT_SPEED);
				break;
		}
    }
    // If robot is moving forward and regulation should be applied.
    else if (((std::clock() - clock_steering)/(float)CLOCKS_PER_SEC > 0.01f) && (rotation == Rotation::NONE)){
        clock_steering = std::clock();
        move_forward();
    }
    //Save rotation to be able to know if rotation has been changed.
	prev_rotation = rotation;
}


void
Steering::move_forward(){
    static const float PREF_SIDE_DIST = 130;
    static const float SIDE_DIST_RANGE = 80; 
    static const float GYRO_RANGE = 5; 
    static float max_speed = 0.15;
    
    //If no regulation should be applied then just move straight forward with low speed.
    if(!regulate){
        latest_control.left_forward = true;
        latest_control.right_forward = true;

        latest_control.left_speed = 0.0001; 
        latest_control.right_speed = 0.0001;

        control(latest_control);
        return;
    }

    //Make robot slow down when approching a wall.
    max_speed = (front_dist < NEAR_WALL_DIST) ? NEAR_WALL_SPEED : MAX_SPEED;

    //Clamp side dist to interval.
    if(side_dist == 0) side_dist = 299;
    side_dist = std::clamp(side_dist, PREF_SIDE_DIST - SIDE_DIST_RANGE, PREF_SIDE_DIST + SIDE_DIST_RANGE);
    //Clamp rotation to interval.
    d_rot = std::clamp(d_rot, -GYRO_RANGE, GYRO_RANGE);

    //Calculate regulation constants.
    float _kp = kp*(max_speed/SIDE_DIST_RANGE);
    float _kd = kd*(max_speed/GYRO_RANGE);

    //Proportional Term    
    float e = _kp*(side_dist - (float)PREF_SIDE_DIST);
    //Rotation from wanted angle
    float u = _kd*d_rot; 
    //Output from regulation
    float out = e + u;
    //Bind to min/max
    out = std::clamp(out, (-max_speed), max_speed);    

    //Set motor rotation direction
    latest_control.left_forward = true;
    latest_control.right_forward = true;

    //Set speed of wheels, the small term prevent the wheels for stop spinning.
    latest_control.left_speed = max_speed + out + 0.000001; 
    latest_control.right_speed = max_speed - out + 0.000001; 
   
    control(latest_control);
}


void
Steering::update_regulation(float right, float d_rotation, bool reg, float front){
    side_dist = right;
    front_dist = front;
    d_rot = d_rotation;
    regulate = reg;
}


void
Steering::rotate_regulated(float rot){
    float max = 0.3;
    float n_rot = rot/90.0f;
    float out = std::clamp(max*n_rot, 0.1f, max);
    control_speed(out, out);
}


void
Steering::command(const SteeringCommand command)
{
    const float speed_max = 0.5f;
    const float speed_min = 0.5f;

    //Set direction and speed of wheels depending on steering command.
    switch(command)
    {
        case SteeringCommand::ROTATE_LEFT:
            control_direction(false, true);
            control_speed(0.15f, 0.15f);
            break;
        case SteeringCommand::ROTATE_RIGHT:
            control_direction(true, false);
            control_speed(0.15f, 0.15f);
            break;
        case SteeringCommand::DRIVE_FORWARD:
            control_direction(true, true);
            control_speed(speed_max, speed_max);
            break;
        case SteeringCommand::DRIVE_BACKWARD:
            control_direction(false, false);
            control_speed(speed_max, speed_max);
            break;
        case SteeringCommand::DRIVE_LEFT:
            control_direction(true, true);
            control_speed(speed_min, speed_max);
            break;
        case SteeringCommand::DRIVE_RIGHT:
            control_direction(true, true);
            control_speed(speed_max, speed_min);
            break;
        case SteeringCommand::HALT:
        default:
            control_direction(true, true);
            control_speed(0.0f, 0.0f);
            break;
    }
}


void
Steering::control(const SteeringControl& control)
{
    control_speed(control.left_speed, control.right_speed);
    control_direction(control.left_forward, control.right_forward);
}


void
Steering::control_speed(float left_speed, float right_speed)
{
    //Clamp right and left spped to interval.
    if(left_speed > 1.0f) left_speed = 1.0f;
    if(right_speed > 1.0f) right_speed = 1.0f;
    
    // map from [0.0f, 1.0f] to [100, 255]
    uint8_t left_pwm = 0, right_pwm = 0; 
    if(left_speed != 0.0f) left_pwm = left_speed * (255 - 100) + 100;
    if(right_speed != 0.0f) right_pwm = right_speed * (255 - 100) + 100;
    
    transmit_pwm(left_pwm, right_pwm);

    latest_control.left_speed = left_speed;
    latest_control.right_speed = right_speed;
    
    pc->steering(latest_control);
}


void
Steering::control_direction(bool left_forward, bool right_forward)
{
    transmit_dir(left_forward, right_forward);

    latest_control.left_forward = left_forward;
    latest_control.right_forward = right_forward;
    pc->steering(latest_control);
}


void
Steering::calibrate(float kp, float kd)
{
    this->kp = kp;
    this->kd = kd;
}


void
Steering::transmit_pwm(uint8_t left_pwm, uint8_t right_pwm) {
    uint8_t bytes[3];
    bytes[0] = (uint8_t)SteeringTx::PWM;
    bytes[1] = left_pwm;
    bytes[2] = right_pwm;

    serial.write(bytes, 3);
}


void
Steering::transmit_dir(bool left_forward, bool right_forward)
{
    uint8_t bytes[3];
    bytes[0] = (uint8_t)SteeringTx::DIR;
    bytes[1] = (uint8_t)left_forward;
    bytes[2] = (uint8_t)right_forward;

    serial.write(bytes, 3);
}


void
Steering::transmit_identified() {
    uint8_t bytes[1];
    bytes[0] = (uint8_t)SteeringTx::IDENTIFIED;
    serial.write(bytes, 1);
}

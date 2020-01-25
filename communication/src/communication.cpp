/*

file: communication.cpp
author: marno874, osklu414, felli675, edwjo109.
created: 2019-11-13

Main runner class.

Calculates robot behaviour, keeps track of robot position,
creates a representation of the map, sends data to pc program.

*/

#include <unistd.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
#include <memory>
#include <thread>
#include "communication.hpp"
#include "serial.hpp"
#include "sensor.hpp"
#include "logging.hpp"


using json = nlohmann::json;


#define ROT_OFFSET 1

//Tune this constants depending on battery power
#define STOP_DIST 225
#define ROT_RIGHT_1_DIST 150
#define ROT_RIGHT_3_DIST 275


Direction Communication::left_turn(Direction dir){
	switch (dir) {
	    case Direction::UP: 	return Direction::LEFT;
	    case Direction::RIGHT:  return Direction::UP;
	    case Direction::DOWN: 	return Direction::RIGHT;
	    case Direction::LEFT: 	return Direction::DOWN;
	}
}

Direction Communication::right_turn(Direction dir) {
	switch (dir) {
	    case Direction::UP: 	return Direction::RIGHT;
	    case Direction::RIGHT:  return Direction::DOWN;
	    case Direction::DOWN: 	return Direction::LEFT;
	    case Direction::LEFT: 	return Direction::UP;
	}
}


Communication::Communication
(
    const std::string& sensor_file,
    const std::string& steering_file,
    const std::string& rplidar_file
):
    map(),
    sensor(sensor_file),
    steering(steering_file),
    rplidar(rplidar_file),
    target_rot(0),
    mode(Mode::MOVING),
    direction(Direction::UP),
    x_pos(0),
    y_pos(0),
	target_dist(0),
    prev_dist(0),
    pc(std::make_shared<PC>()),
    robot_mode(RobotMode::AUTONOMOUS)

{
    rplidar.start_scanning();
    pc->on_command([this](SteeringCommand command){if(this->robot_mode == RobotMode::MANUAL) this->steering.command(command);});
    pc->on_calibration([this](float kp, float kd){this->steering.calibrate(kp, kd);});
    sensor.set_pc(pc);
    sensor.on_competition([this](){ 
        if(this->robot_mode == RobotMode::MANUAL)this->robot_mode = RobotMode::AUTONOMOUS;
        else this->robot_mode = RobotMode::MANUAL; 
        
        // Wait 1 second to start autonomous mode
        auto timer = std::clock();
        while((std::clock() - timer)/CLOCKS_PER_SEC < 1);
        }
    );  
    steering.set_pc(pc);
}


Communication::~Communication(){
    rplidar.stop_motor();
}


bool 
Communication::update() {
    pc->update();
    sensor.update();

    /* Separate manual and autonoumus mode and
    init autonomous mode if it not has been done. */
    static bool inited_auto = false;
    if(robot_mode == RobotMode::MANUAL) {
        if(inited_auto) inited_auto = false;
        return true;
    }
    else {
        if(!inited_auto) {
            autonomous_init();
            inited_auto = true;
        }
    }   
   
    
    //Get measurements from sensors and rplidar.
    SensorMeasurement measurement = sensor.measurement();
    std::vector<ScanNode> curr_nodes;
    bool new_data = false;
    get_rplidar_scan(curr_nodes, new_data);

    //Calculate robot behaviour.
    bool done = calc_inst(measurement, curr_nodes);
    if (done) return false;

    steering.update();

    
    //Pc communication
    if (new_data) {
        pc->rplidar(curr_nodes);
        pc->robot((float)x_pos/400.0f + 0.5f, (float)y_pos/400.0f + 0.5f, measurement.rot * M_PI / 180.0f);
        std::thread(&Communication::update_map, this, curr_nodes, measurement).detach();
        pc->map(map);
    }

    return true;
}


void
Communication::get_rplidar_scan(std::vector<ScanNode>& curr_nodes, bool& new_data){
    static std::vector<ScanNode> old_nodes;

    //Set new rplidar measurement if any, otherwise take most recent ones.
    curr_nodes = rplidar.get_scan();
    if (curr_nodes.empty()){
        curr_nodes = old_nodes;
    } 
    else{
        old_nodes = curr_nodes;
        new_data = true;
    }
}


void
Communication::autonomous_init(){
    
    //Wait for rplidar to return values
    std::vector<ScanNode> curr_nodes;
    bool dummy = false;
    while(curr_nodes.empty()) get_rplidar_scan(curr_nodes,dummy);
  
    //Wait for side sensor to return values
    while(true){
    	sensor.update();
	if(sensor.measurement().right != 0) break;
    }
   
    //Init pos and gyro
    x_pos = 0;  
    y_pos = 0; 
    sensor.init_gyro(sensor.measurement().rot);
}


float 
Communication::get_distance_at(float angle, vector<ScanNode>& curr_nodes, float rot){
    //Calculate rplidar angle relative to robot rotation
    angle += rot-target_rot;

    //Make sure angle is between 0 and 360.
    if(angle > 360) angle -= 360;
    if(angle < 0) angle += 360;


    //static constants
    static const float offset = 1.0;
    static const float LOWER_LIMIT = angle - offset;
    static const float UPPER_LIMIT = angle + offset;


    //First measurment is sometimes more then 1 deg, therefore return first measurement in list.
    if(angle < 1) return curr_nodes[0].dist;
   
    //Go through nodes until the node with the given angle is found.
    for (const ScanNode &node: curr_nodes) {
        if (LOWER_LIMIT < node.angle && node.angle < UPPER_LIMIT) return node.dist;
    }

    // returns 0.0 if no node at the given angle was found.
    return 0.0;
}


void
Communication::update_pos(vector<ScanNode>& curr_nodes, float rot){
    int dist = get_distance_at(0.0, curr_nodes, rot);
    
    //Only update pos when we know the distance to the front.
    if(dist != 0.0 ){
        //If this is the first measurement then we have no referens point to compare with
        if (prev_dist == 0) prev_dist = dist;
        else {
            float dist_delta = prev_dist - dist;
            /*If the previous distance is bigger then the new one,
             then set the new one as referens point and do not update posistion*/
            if(dist_delta < 0) WARN("New distance: ",dist," was bigger than the previous: ", prev_dist);
            /*If distance changed more then 10 cm since last measurement, 
            then ignore last measurement and set the new one as referense point.*/
            if(abs(dist_delta) < 100){
                //Update target distance relative to new position.
                if (target_dist > 0) target_dist -= prev_dist - dist;
                //Update pos depending on direction.
                switch(direction){
                    case Direction::UP: {
                        y_pos += prev_dist - dist;
                        break;
                    }
                    case Direction::RIGHT: {        
                        x_pos += prev_dist - dist;
                        break;
                    }
                    case Direction::DOWN: {
                        y_pos -= prev_dist - dist;
                        break;
                    }
                    case Direction::LEFT: {
                        x_pos -= prev_dist - dist;                   
                        break;
                    }   
                }
            }
            //Save distance so that we can calculate the position delta next time we update the position.
            prev_dist = dist;
        }
    }
}


bool 
Communication::calc_inst(SensorMeasurement& sensor_measurements, vector<ScanNode>& curr_nodes){
    static bool started = false;
    
    //Check if we reached the end of the map
    if(-300 < x_pos && x_pos < 300 && -300 < y_pos && y_pos < 300 && started && (direction == Direction::UP)) {
        return true;
    }

    //If robot drove one tile then we have started.
    if((x_pos >= 400 || y_pos >= 400 || x_pos <= -400 || y_pos <= -400) && !started) started = true;
        
    static float rot = 0;    
	static uint16_t left = 0;
	static uint16_t right = 0;
    static bool regulate = true;
    static bool adjust_right = false;
    static bool adjust_left = false;
    static float prev_rot = 0;
    
    rot = sensor_measurements.rot;    
    left = sensor_measurements.left;
    right = sensor_measurements.right;

    float dist_front = get_distance_at(0.0, curr_nodes, rot);
    float dist_right = get_distance_at(90.0, curr_nodes, rot);
    float dist_down = get_distance_at(180.0, curr_nodes, rot);
    float dist_left = get_distance_at(270.0, curr_nodes, rot);
    

    //Calculate robot behaviour depending on current mode and sensor measurements.
    switch(mode){    
        case Mode::MOVING: {
            update_pos(curr_nodes, rot);
		
            //Check if we went passed the end of the wall to the right, then turn right.
            if(right == 0){
                target_dist = ROT_RIGHT_1_DIST;
                regulate = false;
                mode = Mode::ROTATING_RIGHT_1;     
            } 
            //Check if there is a wall in front of the robot, then turn left
            else if(dist_front != 0.0 && dist_front < STOP_DIST){;
                correct_position();
                mode = Mode::ROTATING_LEFT;
                direction = left_turn(direction);
                prev_dist = dist_left;
                target_rot += 90;
                steering.set_rotation(Rotation::LEFT);     
            }
        
            steering.update_regulation(right, (rot-target_rot), regulate, get_distance_at(0.0, curr_nodes, rot));
            break; 
        }
        case Mode::ROTATING_LEFT: {
            //If robot rotated into correct interval.
            if (rot >= target_rot - ROT_OFFSET && rot <= target_rot + ROT_OFFSET) {
                //If the reason behind the rotation was an over rotation to the right
                if(adjust_left){
                    mode = Mode::ROTATING_RIGHT_2;
                    adjust_left = false;
                    steering.set_rotation(Rotation::RIGHT);   
                } 
                else {    
                    mode = Mode::MOVING;
                    steering.set_rotation(Rotation::NONE);
                } 
            } 
            // If robot over rotated to the left.
            else if (rot >= target_rot + ROT_OFFSET) {
                WARN("Turned too far, adjusting", rot);
                mode = Mode::ROTATING_RIGHT_2;
                steering.set_rotation(Rotation::RIGHT);
                adjust_right = true;
            } 
            // If robot has not reached correct rotation, then continue rotating
            else {
                if (prev_rot != rot) steering.rotate_regulated(abs(target_rot-rot));
            }
            break;
        }
        case Mode::ROTATING_RIGHT_1:{
            update_pos(curr_nodes, rot); // Update pos only when moving forward
            //Check if rotation was initiated by a bad sensor value
            if(right != 0){
                WARN("Right not zero: ", right);
                mode = Mode::MOVING;
                regulate = true;
            }
            //Check if we drove out enough from the corner.
            else if(target_dist <= 0){
                correct_position();
                mode = Mode::ROTATING_RIGHT_2;
                direction = right_turn(direction);
                target_rot -=  90;
                steering.set_rotation(Rotation::RIGHT);
            }    
            break;
        }
        case Mode::ROTATING_RIGHT_2:{
            //Check if robot rotation is in correct interval.
            if (rot >= target_rot - ROT_OFFSET && rot <= target_rot + ROT_OFFSET) {
                //Check if the reason behind the rotation is because the robot over rotated to the left.
                if(adjust_right){
                    mode = Mode::MOVING;
                    adjust_right = false;
                    steering.set_rotation(Rotation::NONE);
                    break;
                }
                mode = Mode::ROTATING_RIGHT_3;
                target_dist = ROT_RIGHT_3_DIST;
                prev_dist = dist_front;
                steering.set_rotation(Rotation::NONE);
            } 
            //Check if the robot over rotated.
            else if(rot <= target_rot - ROT_OFFSET){
                WARN("Rotation went to far, adjusting", rot);
                mode = Mode::ROTATING_LEFT;
                steering.set_rotation(Rotation::LEFT);
                adjust_left = true;
            }
            // If the robot has not yet reached the correct rotation, the continue rotating
            else
            {
                if (prev_rot != rot) steering.rotate_regulated(abs(target_rot-rot));
            }
            break;
        }
        case Mode::ROTATING_RIGHT_3:{
            update_pos(curr_nodes, rot);
            //Check if robot drove in towards the wall enough after right turn.
            if(target_dist <= 0){ // || dist_front < STOP_DIST){
                regulate = true;
                mode = Mode::MOVING;
            }
            break;
        }
    }

    //Save prev rotation to be able to know if it changed since last time an instruction was calculated.
    prev_rot = rot;

    return false;
}


void
Communication::correct_position(){
    y_pos = round(y_pos/400.0f)*400;
    x_pos = round(x_pos/400.0f)*400;
}


void 
Communication::update_map(const std::vector<ScanNode>& nodes, const SensorMeasurement& measurement) {
    // update internal map
    for(const ScanNode& node : nodes){
        // delta vector between robot and hit tile
        float d_x = -(float)node.dist / (float)Map::TILE_SIZE * cos((-node.angle + measurement.rot - 90) * M_PI / 180.0f);
        float d_y = -(float)node.dist / (float)Map::TILE_SIZE * sin((-node.angle + measurement.rot - 90) * M_PI / 180.0f);

        // calculate coordinates, src = robot position, dst = hit position
        float src_x = (float)x_pos/400.0f + 0.5f;
        float src_y = (float)y_pos/400.0f + 0.5f;
        float dst_x = d_x + src_x;
        float dst_y = d_y + src_y;

        // create a small delta vector to use for iterating through points between src and dst
        float d = sqrt(pow(d_x, 2) + pow(d_y, 2));
        d_x /= d;
        d_y /= d;
        d_x *= 0.25f;
        d_y *= 0.25f;

        // keep track of distance between src and dst
        float current_distance = d;
        float last_distance = d + 1.0f;

        // go through tiles between src and dst and set them to empty
        int last_col = Map::MAP_SIZE, last_row = Map::MAP_SIZE;

        // keep setting to empty until distance between src and dst gets greater (meaning that src has passed dst)
        while(current_distance <= last_distance)
        {
            // col and row of tile closest to src (has to be floored since (-0.5, -0.5) corresponds to tile (-1, -1)))
            int empty_col = floor(src_x + Map::ORIGIN);
            int empty_row = floor(src_y + Map::ORIGIN);

            // only update every time a new tile is reached
            if(empty_col != last_col || empty_row != last_row)
            {
                map.update(empty_col, empty_row, Tile::EMPTY);
            }

            // move src by delta
            src_x += d_x;
            src_y += d_y;

            // update last distance and current distance
            last_distance = current_distance;
            current_distance = sqrt(pow(src_x - dst_x, 2) + pow(src_y - dst_y, 2));

            // update last col and row
            last_col = empty_col;
            last_row = empty_row;
        }

        int wall_col = floor(src_x + (float)Map::ORIGIN);
        int wall_row = floor(src_y + (float)Map::ORIGIN);

        // set tile at dst to wall (is this guaranteed to be a new tile from last?)
        map.update(wall_col, wall_row, Tile::WALL);
    }
}

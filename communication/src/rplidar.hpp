/*

file: rplidar.hpp
author: juska933
created: 2019-11-17

RPLIDAR driver wrapper class.

*/

#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <rplidar/rplidar_inc.h>
using namespace std;
using namespace rp::standalone::rplidar; // Only 4 variables in namespace

//using rplib = rp::standalone::rplidar;

struct ScanNode {
    uint32_t dist;
    float angle; 
    uint8_t quality;
    void print() const {
        int q = (int) quality;
        cout << "Dist: " << dist << " Angle: " << angle << " Quality: " << q << endl;
    }
};

enum RPLidarStatus : int {
    BAD_HEALTH = -3,
    NO_DRIVER_CONNECTION = -2,
    INSUFFICIENT_MEMORY = -1,
    OK = 0,
    SCANNING = 1
};

class RPLidar {

public:
    RPLidar(const std::string& file);
    ~RPLidar();
    bool check_health();
    bool is_ok();
    void stop_motor();
    void start_scanning();
    vector<ScanNode> get_scan();
    void print_scan();
private:
    RPLidarStatus status;
    RPlidarDriver* driver;
    std::string opt_com_path;
    u_result op_result;
    int try_set_baudrate();
    void print_err(const char* msg);
    void print_node(rplidar_response_measurement_node_hq_t node);
    void on_finish();
    
};

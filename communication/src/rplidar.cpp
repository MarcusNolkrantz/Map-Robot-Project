/*

file: rplidar.cpp
author: juska933
created: 2019-11-17

RPLIDAR driver wrapper class.

*/


#include "rplidar.hpp"
#include <vector>
#include <signal.h>
#include "stdio.h"
#include "logging.hpp"

RPLidar::RPLidar(const std::string& port_name) {
    INFO("Rplidar constructor, port: ", port_name);
    opt_com_path = port_name;
    driver = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
    if (!driver) {
        status = INSUFFICIENT_MEMORY;
        print_err("Insufficient memory.\n");
    }
    int baudrate = try_set_baudrate();
    if (baudrate < 0) {
        status = NO_DRIVER_CONNECTION;
        print_err("Could not connect to driver.\n");
    }
    if(!check_health()){
        status = BAD_HEALTH;
        print_err("Bad health.\n");
    }   
}

RPLidar::~RPLidar() {
    on_finish();
}

bool RPLidar::is_ok(){
    return status >= 0;
}

void RPLidar::print_err(const char* msg) {
    ERROR(msg);
}

void RPLidar::start_scanning(){
    if (status < 0){
        return;
    }
    driver->startMotor();
    driver->startScan(0, 1);
    status = SCANNING;
}

void RPLidar::print_scan(){
    vector<ScanNode> nodes = get_scan();
    for (const ScanNode &node: nodes) {
        node.print();
    }
}
vector<ScanNode> RPLidar::get_scan(){
    vector<ScanNode> res;

    if (status == OK) {
        start_scanning();
    } else if (status < 0) {
        return res;    
    }
    size_t count = 8192;
    rplidar_response_measurement_node_hq_t nodes[8192];

    // Timeout = 0 -> No waiting
    op_result = driver->grabScanDataHq(nodes, count, 0); 
    // cout is now equal to how many nodes were fetched
    if (IS_OK(op_result)) {
        driver->ascendScanData(nodes, count);
        for (int pos = 0; pos < (int)count ; ++pos) {
            res.push_back({
                nodes[pos].dist_mm_q2/4,
                nodes[pos].angle_z_q14 * 90.f / (1 << 14),
                nodes[pos].quality
            });
        
        }
    }
    return res;
}

void RPLidar::print_node(rplidar_response_measurement_node_hq_t node){
    printf("%s theta: %03.2f Dist: %08.2f Q: %d \n", 
    (node.flag & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ?"S ":"  ", 
    (node.angle_z_q14 * 90.f / (1 << 14)), 
    node.dist_mm_q2/4.0f,
    node.quality);
}

void RPLidar::stop_motor(){
    driver->stop();
    driver->stopMotor();
}

void RPLidar::on_finish(){
    RPlidarDriver::DisposeDriver(driver);
    driver = NULL;
}

int RPLidar::try_set_baudrate() {
    rplidar_response_device_info_t devinfo;

    vector<uint32_t> baudrates = {115200, 256000};
    for(uint32_t baudrate: baudrates)
    {
        if(!driver)
            driver = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
        if(IS_OK(driver->connect(opt_com_path.data(), baudrate)))
        {
            op_result = driver->getDeviceInfo(devinfo);

            if (IS_OK(op_result)) 
            {
                return baudrate;
            }
            else
            {
                delete driver;
                driver = NULL;
            }
        }
    }
    return -1;
}

bool RPLidar::check_health(){
    rplidar_response_device_health_t healthinfo;
    op_result = driver->getHealth(healthinfo);
    if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
        printf("RPLidar health status : %d\n", healthinfo.status);
        if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
            print_err("Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // driver->reset();
            return false;
        } else {
            return true;
        }

    } else {
        printf("RPLidar: Cannot retrieve the lidar health code\n");
        return false;
    }
}

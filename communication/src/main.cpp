/*

file: main.hpp
author: osklu414
created: 2019-11-14

Program entry point. Identifies modules connected via UART and creates communication object.

*/


#include <iostream>
#include <atomic>

#include <unistd.h>
#include <signal.h>

#include "logging.hpp"
#include "serial.hpp"
#include "communication.hpp"
#include "rplidar.hpp"

static std::atomic<bool> quit(false);

void signal_callback(int) { quit.store(true); }

void identify_modules(std::string& sensor_file, std::string& steering_file, std::string& rplidar_file);

int main(int argc, char* argv[])
{
    TRACE("communication module started");
    
    // make sure destructors are called normally
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_callback;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // identify modules
    std::string sensor_file, steering_file, rplidar_file;
    identify_modules(sensor_file, steering_file, rplidar_file);

    // start communication module and update it until signal or update returns false
    Communication communication(sensor_file, steering_file, rplidar_file);
    while(communication.update() && !quit.load());

    TRACE("communication module stopped");
    return 0;
}


void identify_modules
(
    std::string& sensor_file,
    std::string& steering_file, 
    std::string& rplidar_file
)
{
    // determine which port is belongs to which device/module
    TRACE("identifying modules");

    Serial serials[3];
    serials[0].open("/dev/ttyUSB0");
    serials[1].open("/dev/ttyUSB1");
    serials[2].open("/dev/ttyUSB2");
    
    
    // identify steering module
    TRACE("identifying steering module...");
    bool steering_identified = false;
    while(!steering_identified)
    {
        // make sure program quits when sending signal
        if(quit.load()) break;

        for(int s = 0; s < 3; s++)
        {
            uint8_t module_id;
            if(serials[s].read(&module_id, 1) == 1 && (ModuleId)module_id == STEERING)
            {
                steering_identified = true;
                std::swap(serials[s], serials[2]); // move identified serial to back
                steering_file = serials[2].get_file();
                break;
            }
        }
    }
    TRACE("steering module identified at ", steering_file);


    // identify sensor module
    TRACE("identifying sensor module...");
    bool sensor_identified = false;
    while(!sensor_identified)
    {
        // make sure program quits when sending signal
        if(quit.load()) break;

        for(int s = 0; s < 2; s++)
        {
            uint8_t module_id;
            if(serials[s].read(&module_id, 1) == 1 && (ModuleId)module_id == SENSOR)
            {
                sensor_identified = true;
                std::swap(serials[s], serials[1]); // move identified serial to back
                sensor_file = serials[1].get_file();
                break;
            }
        }
    }
    TRACE("sensor module identified at ", sensor_file);

    // identify rplidar device
    TRACE("identifying rplidar device...");
    rplidar_file = serials[0].get_file();
    TRACE("rplidar device identified at ", rplidar_file);

    serials[0].close();
    serials[1].close();
    serials[2].close();

}



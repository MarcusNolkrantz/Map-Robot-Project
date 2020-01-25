/*

file: pc.hpp
author: osklu414
created: 2019-11-25

Interface to PC client.

*/


#ifndef PC_HPP
#define PC_HPP

#include <string>
#include <functional>
#include <chrono>

#include <json/json.hpp>

#include "map.hpp"
#include "steering.hpp"
#include "sensor.hpp"
#include "rplidar.hpp"
#include "socket.hpp"


class PC
{
public:
    PC();
    ~PC();

    void update();

    // send any JSON to PC
    // json: json to send
    void send_json(const nlohmann::json& data);

    // send message to PC
    // text: message text
    void message(const std::string& text);

    // send tile to PC.
    // (col, row): tile's position
    // tile: tile's type
    void tile(const int col, const int row, Tile tile);
    
    // send full map to PC.
    // map: the map
    void map(const Map& map);

    // send robot state to PC.
    // (x, y): robot's position in tile units
    // r: robots rotation in radians
    void robot(const float x, const float y, const float r);

    // send rplidar scannodes to PC
    // nodes: vector of scannodes
    void rplidar(const std::vector<ScanNode>& nodes);

    // send debug point to PC
    // (col, row): point's position
    // color: point's color TODO
    void point(const float col, const float row);

    // send sensor measurement to PC
    // measurement: sensor measurement struct
    void sensor(const SensorMeasurement& measurement);

    // send steering instruction to PC
    // control: steering control struct
    void steering(const SteeringControl& control);


    using CommandCallback = std::function<void(SteeringCommand)>;
    using CalibrationCallback = std::function<void(float, float)>;

    // set steering command callback
    void on_command(CommandCallback callback);

    // set calibration callback
    void on_calibration(CalibrationCallback callback);

private:
    Socket socket;
    CommandCallback command_callback;
    CalibrationCallback calibration_callback;

    std::clock_t map_clock;
};

#endif // PC_HPP
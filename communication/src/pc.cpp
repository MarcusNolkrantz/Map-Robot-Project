/*

file: pc.cpp
author: osklu414, juska933
created: 2019-11-25

Interface to PC client.

*/


#include <vector>
#include <string>

#include <json/json.hpp>
#include <deque>
#include "logging.hpp"
#include "rplidar.hpp"
#include "pc.hpp"


using json = nlohmann::json;


PC::PC() : socket(30), command_callback(), calibration_callback(), map_clock(std::clock())
{
    // route all received data here
    socket.on_json([this](json data, int sd)
    {
        // json must have id
        if (!data.contains("id")) return;

        std::string id = data["id"];
        if(id == "command" && this->command_callback)
        {
            TRACE("received command from pc");
            SteeringCommand command = (SteeringCommand)data["type"].get<int>();
            this->command_callback(command);
        }
        else if(id == "calibration")
        {
            TRACE("received calibration from pc");
            float kp = data["kp"].get<float>();
            float kd = data["kd"].get<float>();
            this->calibration_callback(kp, kd);
        }
        else
        {
            TRACE("received unknown id from pc");
        }        
    });
    socket.start_socket();
}

PC::~PC()
{

}


void PC::update()
{
    socket.check_activity();
}


void PC::send_json(const json& data)
{
    TRACE("sending json to pc");
    socket.send_to_clients_json(data);
}

void PC::message(const std::string& text)
{
    //TRACE("sending message to pc");
    socket.send_to_clients_json
    ({
        {"id", "message"},
        {"text", text}  
    });
}

void PC::tile(const int col, const int row, Tile tile)
{
    //TRACE("sending tile to pc");
    socket.send_to_clients_json
    ({
        {"id", "tile"},
        {"col", col},
        {"row", row},
        {"type", (int)tile}
    });
}

void PC::map(const Map& map)
{
    std::clock_t now = std::clock();
    if((now - map_clock) / CLOCKS_PER_SEC >= 1)
    {
        std::vector<Tile> tiles;
        for(int r = 0; r < Map::MAP_SIZE; r++)
        {
            for(int c = 0; c < Map::MAP_SIZE; c++)
            {
                tiles.push_back(map.get(c, r));
            }
        }
        socket.send_to_clients_json
        ({
            {"id", "map"},
            {"tiles", tiles}
        });
        map_clock = std::clock();
    }
    //TRACE("sending map to pc");
}

void PC::robot(const float x, const float y, const float r)
{
    //TRACE("sending robot state to pc");
    socket.send_to_clients_json
    ({
        {"id", "robot"},
        {"x", x},
        {"y", y},
        {"r", r}
    });
}


void PC::rplidar(const std::vector<ScanNode>& nodes)
{
    std::vector<json> json_nodes;
    for (const ScanNode &node: nodes)
    {
        json_nodes.push_back
        ({
            {"dist", node.dist},
            {"angle", node.angle},
            {"quality", node.quality}
        });
    }
    socket.send_to_clients_json({
        {"id", "rplidar"},
        {"nodes", json_nodes}
    });
}


void PC::point(const float col, const float row)
{
    socket.send_to_clients_json
    ({
        {"id", "point"},
        {"col", col},
        {"row", row}
    });
}


void PC::sensor(const SensorMeasurement& measurement)
{
    //TRACE("sending sensor measurement to pc");
    socket.send_to_clients_json
    ({
        {"id", "sensor"},
        {"left", measurement.left},
        {"right", measurement.right},
        {"rot", measurement.rot}
    });
}


void PC::steering(const SteeringControl& control)
{
    //TRACE("sending steering control to pc");
    socket.send_to_clients_json
    ({
        {"id", "steering"},
        {"left_speed", control.left_speed},
        {"right_speed", control.right_speed},
        {"left_forward", control.left_forward},
        {"right_forward", control.right_forward}
    });
}


void PC::on_command(CommandCallback callback)
{
    command_callback = callback;
}

void PC::on_calibration(CalibrationCallback callback)
{
    calibration_callback = callback;
}

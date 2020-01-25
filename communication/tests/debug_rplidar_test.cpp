#include <assert.h>
#include <iostream>
#include <vector>
#include <string>

#include "../src/logging.hpp"
#include "../src/socket.hpp"
#include "../include/json/json.hpp"
#include "../src/rplidar.hpp"
#include "../src/ai.hpp"
#include "../src/map.hpp"

using json = nlohmann::json;
using namespace std;

int main (int argc, char* argv[]){
    Socket sock(30);
    std::string usb_name = "/dev/ttyUSB2";
    if (argc > 1) {
        usb_name = argv[1];
    }
    RPLidar rplidar(usb_name);
    rplidar.start_scanning();
    sock.on_json([](json data, int sd) {
        if (!data.contains("id")) {
            return;
        }
        TRACE("route: ", data["id"], ", data: ", data);
        std::string id = data["id"];
        
        if (id == "command") {
            INFO("command", data["type"]);
        } else if (id == "calibration") {
            INFO("Calibrating: ", data);
            float kp = data["kp"].get<float>();
            INFO("kp=", kp);
        } else {
            WARN("Could not find route for request", data);
        }
        
    });

    sock.start_socket();

    Vec2 robot_pos(12, 12);
    AI ai;


    while(true) {
        sock.check_activity();
        vector<ScanNode> nodes = rplidar.get_scan();
        if (nodes.size()) {
            vector<json> json_nodes;
            for (const ScanNode &node: nodes) {
                json_nodes.push_back({
                    {"dist", node.dist},
                    {"angle", node.angle},
                    {"quality", node.quality}
                });
            }
            sock.send_to_clients_json({
                {"id", "rplidar"},
                {"nodes", json_nodes}
            });
            // Update pos
            Vec2 delta = ai.robot_delta(nodes);
            if (delta.x && delta.y) {
                robot_pos += delta / Map::TILE_SIZE;
                sock.send_to_clients_json({
                    {"id", "robot"},
                    {"x", robot_pos.x},
                    {"y", robot_pos.y},
                    {"rot", 0}
                });
            }
        }
    }

    return 0;
}
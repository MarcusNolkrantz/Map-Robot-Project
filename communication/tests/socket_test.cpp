#include <assert.h>
#include <iostream>

#include "../src/logging.hpp"
#include "../src/socket.hpp"
#include "../include/json/json.hpp"

using json = nlohmann::json;

int main (){
    Socket sock(30);
    sock.on_json([](json data, int sd) {
        if (!data.contains("route")) {
            return;
        }
        TRACE("route: ", data["route"], ", data: ", data);
        std::string route = data["route"];
        
        if (route == "/remote") {
            INFO("/Remote", data["cmd"]);
        } else if (route == "/calibration") {
            INFO("Calibrating: ", data);
            uint8_t dist_kp = data["dist_kp"].get<uint8_t>();
            INFO("dist_kp=", (int) dist_kp);
        } else {
            WARN("Could not find route for request", data);
        }
        
    });
    sock.start_socket();
    while(true) {
        sock.check_activity();
    }

    return 0;
}
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>

#include "../src/logging.hpp"
#include "../src/rplidar.hpp"

using namespace std;


int main (int argc, char* argv[]){
    std::string usb_name = "/dev/ttyUSB2";
    if (argc > 1) {
        usb_name = argv[1];
    }
    RPLidar rp(usb_name);
    rp.start_scanning();
    while (true) {
        vector<ScanNode> nodes = rp.get_scan();
        if (nodes.size()) {
            cout << "Nodes retrieved: " << nodes.size() << endl;
            for (const ScanNode &node: nodes) {
                node.print();
            }
        }
    }

    return 0;
}
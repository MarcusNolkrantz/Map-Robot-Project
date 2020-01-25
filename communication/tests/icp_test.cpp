#include "../src/icp.hpp"
#include <vector>
#include "../src/vec2.hpp"

using namespace std;

void add_to_points(vector<Vec2> &points, Vec2 delta) {
    for (int i = 0; i < (int) points.size(); i++) {
        points[i] += delta;
    }
}

int main () {
    ICP icp;
    vector<Vec2> points = {
        {0,0}, {0,1}, {0, 2}
    };

    icp.get_delta(points);
    //add_to_points(points, {1, 0});
    add_to_points(points, {0, 1}); // will match 2 leave 1
    icp.get_delta(points);
}
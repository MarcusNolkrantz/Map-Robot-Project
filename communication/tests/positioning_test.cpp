#include "../src/ai.hpp"
#include "../src/logging.hpp"
#include "../src/map.hpp"
#include <iostream>
#include <vector>


using namespace std;

void test_operators() {
    Vec2 v(2.0, 3);
    cout << v + Vec2(-2, -3) << endl;
    cout << Vec2(-2, -2) + 2.0 << endl;
    
    cout << -v << endl;
    cout << v - Vec2(2, 3) << endl;
    cout << Vec2(2, 2) - 2 << endl;

    cout << Vec2(10, 10) / Vec2(5, 5) << endl;
    cout << Vec2(10, 10) / 5 << endl;

}

void add_to_poly(vector<Vec2> &points, Vec2 delta) {
    for (unsigned i = 0; i < points.size(); i++) {
        points[i] += delta;
    }
}

void test1() {
    AI ai;
    Vec2 pos(12, 12);
    vector<Vec2> points = {
        {3, 5}, {-3, 5}, {-3, -5}, {3, -5}
    };
    for (int i = 0; i < 30; i++) {
        Vec2 delta = ai.robot_delta(points);
        pos += delta/Map::TILE_SIZE;
        INFO("Pos ", i, ": ", pos);
        add_to_poly(points, Vec2(1, 1));
    }
}

int main (){

    //test_operators();
    test1();
    return 0;
}
/*

file: map.hpp
author: osklu414
created: 2019-11-15

Map model class.

*/


#ifndef MAP_HPP
#define MAP_HPP

#include <atomic>

enum class Tile
{
    UNKNOWN = 0,
    EMPTY = 1,
    WALL = 2
};

struct Robot
{
    Robot();

    // position of robot in tile coordinates
    float x, y;

    // rotation of robot in radians
    float r;
};


class Map
{
    friend class PC;
public:
    Map();
    ~Map();

    // set tile (use update_tile when filling in map)
    void set(const int col, const int row, const Tile tile);

    // get tile
    Tile get(const int col, const int row) const;

    // set tile with confidence in consideration (this should be used when filling in map)
    void update(const int col, const int row, const Tile tile);

    // clean up map by removing tiles with too low confidence values and ones outside the outer wall
    void clean();

    const static int AREA_SIZE = 10000;                         // area is 10x10m
    const static int TILE_SIZE = 400;                           // each "square" is 40x40cm
    const static int ORIGIN = AREA_SIZE/TILE_SIZE;              // intitial tile position of robot is (ORIGIN, ORIGIN)
    const static int MAP_SIZE = 2*(AREA_SIZE/TILE_SIZE) + 1;    // number of tiles in each dimension

private:
    std::atomic<Tile> tiles[MAP_SIZE][MAP_SIZE];
    unsigned long long confidence_empty[MAP_SIZE][MAP_SIZE];
    unsigned long long confidence_wall[MAP_SIZE][MAP_SIZE];

    const static unsigned int CONFIDENCE_MIN = 10;
};

#endif // MAP_HPP

/*

file: map.cpp
author: osklu414
created: 2019-11-15

Map model class.

*/


#include "map.hpp"


// robot starts in the middle of tile (0, 0)
Robot::Robot() : x(0.5f), y(0.5f), r(0) {}


Map::Map()
{
    // each tile starts of as unknown
    for(int r = 0; r < MAP_SIZE; r++)
    {
        for(int  c = 0; c < MAP_SIZE; c++)
        {
            tiles[r][c] = Tile::UNKNOWN;
            confidence_empty[r][c] = 0;
            confidence_wall[r][c] = 0;
        }
    }

    // the starting position of the robot is empty
    //tiles[ORIGIN][ORIGIN] = Tile::EMPTY;
}

Map::~Map()
{

}


void Map::set(const int col, const int row, Tile tile)
{
    tiles[row][col] = tile;
}


Tile Map::get(const int col, const int row) const
{
    return tiles[row][col];
}


void Map::update(const int col, const int row, const Tile tile)
{
    switch(tile)
    {
        case Tile::EMPTY:
        {
            if(++confidence_empty[row][col] > confidence_wall[row][col])
            {
                 if(confidence_empty[row][col] >= CONFIDENCE_MIN) tiles[row][col] = Tile::EMPTY;
            }
            break;
        }
        case Tile::WALL:
        {
            if(++confidence_wall[row][col] > confidence_empty[row][col])
            {
                if(confidence_wall[row][col] >= CONFIDENCE_MIN) tiles[row][col] = Tile::WALL;
            }
            break;
        }
        case Tile::UNKNOWN:
        {
            break;
        }
    }
}


void Map::clean()
{
    // start from origin and find outer walls
    int c = ORIGIN;
    int r = ORIGIN;


}
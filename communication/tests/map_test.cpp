#include <assert.h>
#include <iostream>

#include "../src/map.hpp"

int main(int argc, char* argv[])
{
    Map map;

    // make sure robots initial position gets set to empty
    assert(map.get(MapPos(0, 0).to_tile_pos()) == Tile::EMPTY);

    return 0;
}
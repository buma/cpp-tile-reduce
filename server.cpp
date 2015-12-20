#include "server.hpp"
#include <thread>

Server::Server(float minLon, float minLat, float maxLon, float maxLat, int zoom)
    : tileReader(MBTileReader())
{
    tileList = std::make_shared<TileList>(MBTileReader::get_tiles_inside_bbox(minLon, minLat, maxLon, maxLat, zoom));
    tile_location = tileList->begin();
}

Server::Server(std::string filepath, int zoom)
    : tileReader(MBTileReader(filepath))
{
    tileList = std::make_shared<TileList>(tileReader.get_all_tiles(zoom));
    tile_location = tileList->begin();
}

/**
  Each call gets a next tile in tileList if there are no more tiles std::range_error is thrown
 * @brief Server::get_tile
 * @return
 */
TileTuple Server::get_tile() {
    //TODO: fix copies
    if (this->tile_location == this->tileList->end()) {
        throw std::range_error("No more tiles");
    }
    TileTuple tileTuple = *this->tile_location;
    ++tile_location;

    return tileTuple;


}

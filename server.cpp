#include "server.hpp"
#include <thread>

Server::Server(float minLon, float minLat, float maxLon, float maxLat, int zoom)
    : tileReader(MBTileReader())
{
    tileList = std::make_shared<TileList>(MBTileReader::get_tiles_inside_bbox(minLon, minLat, maxLon, maxLat, zoom));
}

Server::Server(std::string filepath, int zoom)
    : tileReader(MBTileReader(filepath))
{
    tileList = std::make_shared<TileList>(tileReader.get_all_tiles(zoom));
}

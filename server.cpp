#include "server.hpp"
#include <thread>

Server::Server(std::string filepath, float minLon, float minLat, float maxLon, float maxLat, int zoom)
    : tileReader(MBTileReader(filepath))
{
    tileList = std::make_shared<TileList>(tileReader.get_tiles_inside(minLon, minLat, maxLon, maxLat, zoom));
}

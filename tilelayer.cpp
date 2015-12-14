#include "tilelayer.hpp"

TileLayer::TileLayer(std::string name, uint32_t version, uint32_t extent)
{
    this->name = name;
    this->extent = extent;
    this->version = version;
}


TileLayer::TileLayer(std::string name, uint32_t version) : TileLayer(name, version, 4096) {}





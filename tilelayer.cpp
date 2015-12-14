#include "tilelayer.hpp"

TileLayer::TileLayer(std::string name, uint32_t version, uint32_t extent)
{
    this->name = name;
    this->extent = extent;
    this->version = version;
}


TileLayer::TileLayer(std::string name, uint32_t version) : TileLayer(name, version, 4096) {}


std::ostream& operator<<(std::ostream &os, const TileLayer &tileLayer) {
    return os << "TileLayer { " << std::endl << "name:" << tileLayer.name << ", "
                   << "version:" << tileLayer.version << " ,"
                   << "extent:" << tileLayer.extent
                   <<"}";
}



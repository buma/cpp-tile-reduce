#ifndef TILELAYER_H
#define TILELAYER_H

#include <stdint.h>
#include <string>


class TileLayer
{
public:
    TileLayer(std::string, uint32_t version, uint32_t extent);
    TileLayer(std::string name, uint32_t version);


private:
    uint32_t extent;
    uint32_t version;
    std::string name;
};

#endif // TILELAYER_H

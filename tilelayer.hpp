#ifndef TILELAYER_H
#define TILELAYER_H

#include <stdint.h>
#include <string>
#include <ostream>


class TileLayer
{
public:
    TileLayer(std::string, uint32_t version, uint32_t extent);
    TileLayer(std::string name, uint32_t version);
    friend std::ostream& operator<<(std::ostream&, const TileLayer&);


private:
    uint32_t extent;
    uint32_t version;
    std::string name;
};

#endif // TILELAYER_H

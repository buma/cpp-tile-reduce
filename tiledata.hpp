#ifndef TILEDATA_H
#define TILEDATA_H

#include <unordered_map>
#include <ostream>
#include <sstream>
#include "tilelayer.hpp"
#include "generated/vector_tile.pb.hpp"



class TileData
{
public:
    TileData(mapnik::vector::tile &vector_tile, int z, unsigned x, unsigned y);
    TileData(std::string &message, int z, unsigned x, unsigned y);
    friend std::ostream& operator<<(std::ostream&, const TileData&);
    std::shared_ptr<TileLayer> getLayer(std::string);
    std::string toString() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

private:
    std::unordered_map<std::string, std::shared_ptr<TileLayer>> layers;
    void init(mapnik::vector::tile &vector_tile, int z, unsigned x, unsigned y);


};

#endif // TILEDATA_H

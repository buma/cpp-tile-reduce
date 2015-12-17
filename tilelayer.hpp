#ifndef TILELAYER_H
#define TILELAYER_H

#include <stdint.h>
#include <string>
#include <ostream>
#include <memory>
#include <vector>


#include "tilefeature.h"
#include "generated/vector_tile.pb.hpp"


class TileLayer
{
public:
    TileLayer(std::string, uint32_t version, uint32_t extent);
    TileLayer(std::string name, uint32_t version);
    friend std::ostream& operator<<(std::ostream&, const TileLayer&);
    size_t getNumFeatures() const {
        return this->features.size();
    }
    //void setLayer(std::unique_ptr<mapnik::vector::tile_layer> tile_layer);
    void setLayer(const mapnik::vector::tile_layer *tile_layer);

    TileFeature* getFeature(uint n, int z, unsigned x, unsigned y);
    TileFeature* getFeature(uint n) const;


private:
    uint32_t extent;
    uint32_t version;
    std::string name;
    std::vector<std::unique_ptr<TileFeature>> features;
    //std::unique_ptr<mapnik::vector::tile_layer> protobuf_layer;
    const mapnik::vector::tile_layer * protobuf_layer;
    size_t features_size;

};

#endif // TILELAYER_H

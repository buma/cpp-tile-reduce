#ifndef TILELAYER_H
#define TILELAYER_H

#include <stdint.h>
#include <string>
#include <ostream>
#include <memory>
#include <vector>

#include <functional>
#include "tilefeature.h"
#include "generated/vector_tile.pb.hpp"
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
using namespace std::placeholders;


class TileLayer : public std::enable_shared_from_this<TileLayer>
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

    std::shared_ptr<TileFeature> getFeature(uint n, int z, unsigned x, unsigned y);
    std::shared_ptr<TileFeature> getFeature(uint n) const;
    std::vector<std::shared_ptr<TileFeature>>  filter(std::function<bool(TileFeature*)> fun) const;


private:
    uint32_t extent;
    uint32_t version;
    std::string name;
    std::vector<std::shared_ptr<TileFeature>> features;
    //std::unique_ptr<mapnik::vector::tile_layer> protobuf_layer;
    const mapnik::vector::tile_layer * protobuf_layer;
    size_t features_size;
    static std::unique_ptr<geos::geom::GeometryFactory> geometry_factory;

};

#endif // TILELAYER_H

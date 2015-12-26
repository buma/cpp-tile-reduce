#include <iostream>
#include <memory>
#include "tiledata.hpp"
#include "tippecanoe/decode.hpp"

TileData::TileData(mapnik::vector::tile &vector_tile, int z, unsigned x, unsigned y) {
    this->init(vector_tile, z, x, y);
}

void TileData::init(mapnik::vector::tile &vector_tile, int z, unsigned x, unsigned y)
{
    for(auto&& layer : vector_tile.layers()) {
        auto tileLayer = std::make_shared<TileLayer>(layer.name(), layer.version());

        //Use https://github.com/mapbox/tippecanoe to get geometry
        //std::unique_ptr<mapnik::vector::tile_layer> tile_ptr(layer.);
        //tileLayer.get()->setLayer(std::move(tile_ptr));
        tileLayer.get()->setLayer(&layer);
        //FIXME: retarted way to insert features but OK for now
        for(int i=0; i < layer.features_size(); i++) {
            tileLayer.get()->getFeature(i,z,x,y);
            //break;
        }
        /*for(auto&& feature : layer.features()) {
            assert(feature.tags_size() % 2 == 0);
            std::cerr << "FEATURE: " << feature.id() << " Type: " << feature.type() << std::endl;

            for(int idx=0; idx < feature.tags_size(); idx+=2) {
                auto key = layer.keys(feature.tags(idx));
                auto value_type = layer.values(feature.tags(idx+1));

                std::cerr << " " << key << " " << value_type.DebugString() << " ";
                if (value_type.has_double_value()) {
                    std::cerr << "D: " << value_type.double_value();
                } else if (value_type.has_float_value()) {
                    std::cerr << "F: " << value_type.float_value() ;
                } else if (value_type.has_string_value()) {
                    std::cerr << "STR: " << value_type.string_value();
                }
                std::cerr << std::endl;
            }
            break;

        }*/
        this->layers.emplace(layer.name(), tileLayer);
    }
}

TileData::TileData(std::string & message, int z, unsigned x, unsigned y
                   #ifdef TIMING
                       , std::chrono::nanoseconds & protobuf_decode
                   #endif
                   ) {
    mapnik::vector::tile tile;
#ifdef TIMING
    auto start = std::chrono::high_resolution_clock::now();
#endif
    handle(message.data(), message.size(), z, x, y, tile);
#ifdef TIMING
    protobuf_decode+=(std::chrono::high_resolution_clock::now()-start);
#endif
    this->init(tile, z, x, y);
}

TileData::TileData(const char * data, std::size_t size, int z, unsigned x, unsigned y
                   #ifdef TIMING
                       , std::chrono::nanoseconds & protobuf_decode
                   #endif
                   ) {
    mapnik::vector::tile tile;
#ifdef TIMING
    auto start = std::chrono::high_resolution_clock::now();
#endif
    handle(data, size, z, x, y, tile);
#ifdef TIMING
    protobuf_decode+=(std::chrono::high_resolution_clock::now()-start);
#endif
    this->init(tile, z, x, y);
}

std::ostream& operator<<(std::ostream& os, const TileData& tileData) {
    os << "{ ";
    for (auto&& layer: tileData.layers) {
        os << layer.first << ":{" << std::endl;
        os << *layer.second.get() << "}";
    }
    os << "}";
    return os;

}

std::shared_ptr<TileLayer> TileData::getLayer(std::string layer) {
    return this->layers.at(layer);
}


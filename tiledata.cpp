#include <iostream>
#include <memory>
#include "tiledata.hpp"

TileData::TileData(mapnik::vector::tile &vector_tile, int z, unsigned x, unsigned y)
{
    for(auto&& layer : vector_tile.layers()) {
        std::unique_ptr<TileLayer> tileLayer(new TileLayer(layer.name(), layer.version()));

        //Use https://github.com/mapbox/tippecanoe to get geometry
        //std::unique_ptr<mapnik::vector::tile_layer> tile_ptr(layer.);
        //tileLayer.get()->setLayer(std::move(tile_ptr));
        tileLayer.get()->setLayer(&layer);
        //FIXME: retarted way to insert features but OK for now
        for(uint i=0; i < layer.features_size(); i++) {
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
        this->layers.insert(std::make_pair(layer.name(), std::move(tileLayer)));
    }
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


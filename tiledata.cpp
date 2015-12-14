#include <iostream>
#include "tiledata.hpp"

TileData::TileData(mapnik::vector::tile &vector_tile)
{
    //for(int i=0; i < vector_tile.layers_size(); i++) {
    //    mapnik::vector::tile::layer layer = vector_tile.layers();
    for(auto&& layer : vector_tile.layers()) {
        TileLayer tileLayer(layer.name(), layer.version());
        for(auto&& feature : layer.features()) {
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
            //break;

        }
        //layers[layer.name()] = tileLayer;
    }
}


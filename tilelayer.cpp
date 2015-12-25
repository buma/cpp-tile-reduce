#include "tilelayer.hpp"
#include <iostream>
#include "tippecanoe/decode.hpp"

TileLayer::TileLayer(std::string name, uint32_t version, uint32_t extent)
{
    this->name = name;
    this->extent = extent;
    this->version = version;
}


TileLayer::TileLayer(std::string name, uint32_t version) : TileLayer(name, version, 4096) {}

std::shared_ptr<TileFeature> TileLayer::getFeature(uint n) const {
    //n is bigger then number of features something is wrong
    if (n > this->features_size) {
        return nullptr;
    }
    //Not yet inserted
    if(n >= this->features.size()){
        return nullptr;
    }

    return this->features.at(n);
}

std::shared_ptr<TileFeature> TileLayer::getFeature(uint n, int z, unsigned x, unsigned y) {
    //n is bigger then number of features something is wrong
    if (n > this->features_size) {
        return nullptr;
    }
    //Not yet inserted
    if(n >= this->features.size()){
        auto feature = this->protobuf_layer->features(n);
        auto tileFeature = std::make_shared<TileFeature>(feature.tags_size());
        assert(feature.tags_size() % 2 == 0);
        //std::cerr << "FEATURE: " << feature.id() << " Type: " << feature.type() << std::endl;

        for(int idx=0; idx < feature.tags_size(); idx+=2) {
            auto key = this->protobuf_layer->keys(feature.tags(idx));
            auto value_type = this->protobuf_layer->values(feature.tags(idx+1));
            tileFeature->addTag(key, value_type);

            /*
            std::cerr << " " << key << " " << value_type.DebugString() << " ";
            if (value_type.has_double_value()) {
                std::cerr << "D: " << value_type.double_value();
            } else if (value_type.has_float_value()) {
                std::cerr << "F: " << value_type.float_value() ;
            } else if (value_type.has_string_value()) {
                std::cerr << "STR: " << value_type.string_value();
            }
            std::cerr << std::endl;*/
        }
        auto geometry = handle(feature, this->extent, z, x, y, std::move(TileLayer::geometry_factory));
        if (geometry) {
            tileFeature->addGeometry(std::move(geometry));
        }
        //this->features.insert(n, std::move(tileFeature));
        this->features.push_back(std::move(tileFeature));
    }
    return this->features.at(n);
}

//void TileLayer::setLayer(std::unique_ptr<mapnik::vector::tile_layer> tile_layer) {
    //this->protobuf_layer = std::move(tile_layer);
void TileLayer::setLayer(const mapnik::vector::tile_layer *tile_layer) {
    //this->protobuf_layer.reset(std::unique_ptr<mapnik::vector::tile_layer>(tile_layer));
    //this->protobuf_layer = std::move(tile_layer);
    this->protobuf_layer = tile_layer;
    this->features.reserve(tile_layer->features_size());
    this->features_size = tile_layer->features_size();
}


std::ostream& operator<<(std::ostream &os, const TileLayer &tileLayer) {
    os << "TileLayer { " << std::endl << "name:" << tileLayer.name << ", "
              << "version:" << tileLayer.version << " ,"
              << "extent:" << tileLayer.extent << ", "
              << "length:" << tileLayer.getNumFeatures() << "}";

    return os;
}

std::vector<std::shared_ptr<TileFeature>>  TileLayer::filter(std::function<bool(TileFeature*)> fun) const {
    std::vector<std::shared_ptr<TileFeature>> result;
    for(auto&& feature: this->features) {
        if (fun(feature.get())) {
            result.push_back(feature);
        }
    }
    return result;

}

//FIXME: 16byte memory leak here
std::unique_ptr<geos::geom::GeometryFactory> TileLayer::geometry_factory = std::unique_ptr<geos::geom::GeometryFactory>(
            new geos::geom::GeometryFactory(new geos::geom::PrecisionModel(), 4326));

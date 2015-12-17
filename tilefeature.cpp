#include "tilefeature.h"
#include <iostream>
#include <geos/io/WKTWriter.h>
#include <algorithm>

TileFeature::TileFeature()
{

}

void TileFeature::addTag(std::string & key, const mapnik::vector::tile_value & value) {
    //TODO: fix copies
    //long values seems to be doubles in protobuf file
    if (value.has_double_value()) {
        this->long_tags[key] = value.double_value();
    } else if (value.has_string_value()) {
        this->string_tags[key] = value.string_value();
    } else {
        std::cerr << "Weird type: " << value.DebugString() << std::endl;
    }
}

void TileFeature::addGeometry(std::unique_ptr<geos::geom::Geometry> geometry) {
    this->geometry = std::move(geometry);
}

std::ostream& operator<<(std::ostream& os, const TileFeature& tileFeature) {
    auto wkt = new geos::io::WKTWriter();
    os << "{";
    for (auto&& pair: tileFeature.long_tags) {
        os << pair.first << ":" << pair.second << ", " << std::endl;
    }
    for (auto&& pair: tileFeature.string_tags) {
        os << pair.first << ":\"" << pair.second << "\"," << std::endl;
    }
    if (tileFeature.geometry) {
        os << wkt->write(tileFeature.geometry.get()) << std::endl;
    }
    os << "}";
    return os;
}
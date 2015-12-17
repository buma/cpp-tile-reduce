#ifndef DECODE_HPP
#define DECODE_HPP

#include <string>
#include "generated/vector_tile.pb.hpp"
#include <geos/geom/Geometry.h>

void handle(std::string message, int z, unsigned x, unsigned y, int describe);
std::unique_ptr<geos::geom::Geometry> handle(mapnik::vector::tile_feature &tile, int extent, int z, unsigned x, unsigned y, int describe);

#endif // DECODE_HPP


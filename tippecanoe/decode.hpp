#ifndef DECODE_HPP
#define DECODE_HPP

#include <string>
#include "generated/vector_tile.pb.hpp"
#include <geos/geom/Geometry.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>


void handle(const char * data, std::size_t size, int z, unsigned x, unsigned y, mapnik::vector::tile & tile);
std::unique_ptr<geos::geom::Geometry> handle(mapnik::vector::tile_feature &tile, int extent,
                                             int z, unsigned x, unsigned y, std::unique_ptr<geos::geom::GeometryFactory> && geometry_factory);

#endif // DECODE_HPP


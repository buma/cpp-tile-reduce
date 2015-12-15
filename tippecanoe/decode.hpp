#ifndef DECODE_HPP
#define DECODE_HPP

#include <string>
#include "generated/vector_tile.pb.hpp"

void handle(std::string message, int z, unsigned x, unsigned y, int describe);
void handle(mapnik::vector::tile & tile, int z, unsigned x, unsigned y, int describe);

#endif // DECODE_HPP


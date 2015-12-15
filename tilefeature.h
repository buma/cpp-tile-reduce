#ifndef TILEFEATURE_H
#define TILEFEATURE_H

#include <unordered_map>
#include <string>
#include <generated/vector_tile.pb.hpp>

class TileFeature
{
public:
    TileFeature();
    void addTag(std::string & key, const mapnik::vector::tile_value & value);
    friend std::ostream& operator<<(std::ostream&, const TileFeature&);
private:
    std::unordered_map<std::string, unsigned long long int> long_tags;
    std::unordered_map<std::string, std::string> string_tags;
};

#endif // TILEFEATURE_H

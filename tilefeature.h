#ifndef TILEFEATURE_H
#define TILEFEATURE_H

#include <unordered_map>
#include <string>
#include <set>
#include <memory>
#include <sstream>
#include <generated/vector_tile.pb.hpp>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTWriter.h>


class TileFeature : public std::enable_shared_from_this<TileFeature>
{
public:
    TileFeature();
    TileFeature(int tag_number);
    void addTag(std::string & key, const mapnik::vector::tile_value & value);
    friend std::ostream& operator<<(std::ostream&, const TileFeature&);
    void addGeometry(std::unique_ptr<geos::geom::Geometry> geometry);
    bool hasTag(const std::string &key) const;
    bool hasTagValue(const std::string & key, const std::string & value) const;
    bool hasTagValue(const std::string & key, const std::set<std::string> & value) const;
    std::string getTagValueString(const std::string & key) const;
    std::string toString() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }
private:
    std::unordered_map<std::string, unsigned long long int> long_tags;
    std::unordered_map<std::string, std::string> string_tags;
    std::unique_ptr<geos::geom::Geometry> geometry;
    static std::unique_ptr<geos::io::WKTWriter> wkt;

};

#endif // TILEFEATURE_H

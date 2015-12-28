#ifndef MBTILEREADER_H
#define MBTILEREADER_H
#include <string>
#include <memory>
#include <sqlite3.h>
#include <vector>
#include <tuple>
#ifdef TIMING
#include <chrono>
#include "timemeasure.hpp"
#endif

#include "tiledata.hpp"
typedef std::tuple<unsigned, unsigned,int> TileTuple;
typedef std::vector<TileTuple> TileList;


class MBTileReader
{
public:
    MBTileReader();
    MBTileReader(const std::string & filepath);
    std::unique_ptr<TileData> get_tile(int z, unsigned x, unsigned y);
    static TileList get_tiles_inside_bbox(float minLon, float minLat, float maxLon, float maxLat, int zoom=12);
    TileList get_all_tiles(int zoom=-1) const;


    ~MBTileReader()
    {
        if (this->db != nullptr && sqlite3_close(this->db) != SQLITE_OK) {
            fprintf(stderr, "%s: could not close database: %s\n", this->filename.c_str(), sqlite3_errmsg(this->db));
            exit(EXIT_FAILURE);
        }
    }
    std::string get_filename() const;
#ifdef TIMING
    TimeMeasure timeMeasure;
#endif

private:
    sqlite3 *db;
    static const std::string sql;
    static const std::string sql_all;
    std::string filename;

};

#endif // MBTILEREADER_H

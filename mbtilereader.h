#ifndef MBTILEREADER_H
#define MBTILEREADER_H
#include <string>
#include <memory>
#include <sqlite3.h>

#include "tiledata.hpp"


class MBTileReader
{
public:
    MBTileReader(const std::string & filepath);
    TileData* get_tile(int z, unsigned x, unsigned y);

    ~MBTileReader()
    {
        if (sqlite3_close(this->db) != SQLITE_OK) {
            fprintf(stderr, "%s: could not close database: %s\n", this->filename.c_str(), sqlite3_errmsg(this->db));
            exit(EXIT_FAILURE);
        }
    }

private:
    sqlite3 *db;
    static const std::string sql;
    std::string filename;

};

#endif // MBTILEREADER_H

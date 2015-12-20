#ifndef SERVER_H
#define SERVER_H
#include <string>
#include <memory>
#include "mbtilereader.h"

class Server
{
public:
    Server(std::string filepath, float minLon=-1, float minLat=-1, float maxLon=-1, float maxLat=-1, int zoom=12);
    virtual void run(bool start_workers=false, unsigned int workers=0) = 0;
protected:
    MBTileReader tileReader;

    std::shared_ptr<TileList> tileList;

};

#endif // SERVER_H

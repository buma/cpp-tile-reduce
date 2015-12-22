#ifndef STDOUT_SERVER_HPP
#define STDOUT_SERVER_HPP
#include <string>
#include "server.hpp"


class Stdout_Server : public Server
{
public:
    Stdout_Server(float minLon=-1, float minLat=-1, float maxLon=-1, float maxLat=-1, int zoom=12);
    Stdout_Server(std::string filepath, int zoom=12);
    void run(bool start_workers=false, unsigned int workers=0) override;
private:
    uint sent_tiles,received_tiles,current_tile;
};

#endif // STDOUT_SERVER_HPP

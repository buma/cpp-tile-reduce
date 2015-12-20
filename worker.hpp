#ifndef WORKER_HPP
#define WORKER_HPP
#include <string>
#include "mbtilereader.h"


class Worker
{
public:
    Worker(std::string filepath);
    virtual void run() = 0;
    virtual void info() = 0;
protected:
    MBTileReader tileReader;
};

#endif // WORKER_HPP

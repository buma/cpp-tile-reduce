#include "worker.hpp"

Worker::Worker(std::string filepath) : tileReader(MBTileReader(filepath))
{

}


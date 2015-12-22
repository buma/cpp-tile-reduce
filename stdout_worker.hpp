#ifndef STDOUT_WORKER_HPP
#define STDOUT_WORKER_HPP
#include <string>
#include "worker.hpp"


class Stdout_Worker : public Worker
{
public:
    Stdout_Worker(std::string filepath);
    void run() override;
    void info() override;
private:
    uint sent_tiles,received_tiles;
    void send(int) override;
};

#endif // STDOUT_WORKER_HPP

#ifndef ZMQ_WORKER_HPP
#define ZMQ_WORKER_HPP
#include <string>
#include <memory>
#include <CpperoMQ/All.hpp>
#include <CpperoMQ/OutgoingMessage.hpp>
#include <CpperoMQ/IncomingMessage.hpp>
#include "worker.hpp"
#include "utils.hpp"

class ZMQ_Worker : public Worker
{
public:
    enum class Transport {
        TCP,
        IPC
    };
    ZMQ_Worker(std::string filepath, Transport _transport=Transport::TCP);
    void run() override;

private:
    uint sent_tiles,received_tiles;
    const Transport _transport;
    CpperoMQ::Context context;
    CpperoMQ::PullSocket pull_socket;
    CpperoMQ::PushSocket push_socket;
    CpperoMQ::RouterSocket ctrl_socket;
    CpperoMQ::SubscribeSocket subscriber_socket;
    bool isDone;
    void connect();
    const std::string get_addr(int port,std::string host="127.0.0.1") const;
    //void map(TileData* tileData);
    void info() override;
    void send(int) override;
};

#endif // ZMQ_WORKER_HPP

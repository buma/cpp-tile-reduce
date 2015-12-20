#ifndef ZMQ_SERVER_H
#define ZMQ_SERVER_H
#include "server.hpp"
#include <CpperoMQ/All.hpp>
#include <CpperoMQ/OutgoingMessage.hpp>
#include <CpperoMQ/IncomingMessage.hpp>


class ZMQ_Server : public Server
{
public:
    enum class Transport {
        TCP,
        IPC
    };
    ZMQ_Server(std::string filepath, float minLon=-1, float minLat=-1, float maxLon=-1, float maxLat=-1, int zoom=12,
               Transport _transport=Transport::TCP);
    void run(bool start_workers=false, unsigned int workers=0) override;


private:
    /*
    zmq::context_t context;
    zmq::socket_t pull_socket;
    zmq::socket_t push_socket;*/
    const Transport _transport;
    CpperoMQ::Context context;
    CpperoMQ::PullSocket pull_socket;
    CpperoMQ::PushSocket push_socket;
    CpperoMQ::RouterSocket ctrl_socket;
    uint sent_tiles,received_tiles,current_tile;

    void connect();
    const char *get_addr(int port,std::string host="127.0.0.1");
    void collect_task();
    void start_new_task();
    void manage();
    void socket_error();
};

#endif // ZMQ_SERVER_H

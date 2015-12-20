#include "zmq_server.hpp"
#include <thread>
#include <sstream>
#include <msgpack.hpp>

ZMQ_Server::ZMQ_Server(std::string filepath, float minLon, float minLat, float maxLon, float maxLat, int zoom, ZMQ_Server::Transport transport)
    : Server(filepath, minLon, minLat, maxLon, maxLat, zoom),
      _transport(transport),
      pull_socket(this->context.createPullSocket()),
      push_socket(this->context.createPushSocket()),
      ctrl_socket(this->context.createRouterSocket()),
      sent_tiles(0), received_tiles(0), current_tile(0)
{



}

void ZMQ_Server::run(bool start_workers, unsigned int workers) {
    if (workers == 0) {
        unsigned int n = std::thread::hardware_concurrency();
        std::cout << n << "threads";
        if (n != 0) {
            workers = n;
        }
    }
    this->connect();
    auto tile_location = this->tileList->begin();


    /*CpperoMQ::PollItem poll_pull(ZMQ_POLLIN, pull_socket, [&]{collect_task();});
    CpperoMQ::PollItem poll_push(ZMQ_POLLOUT, push_socket, [&]{start_new_task();});
    //CpperoMQ::PollItem poll_ctrl(ZMQ_POLLOUT, ctrl_socket, this->manage());

    CpperoMQ::PollItem poll_pull_error(ZMQ_POLLERR, pull_socket, [&]{socket_error();});
    CpperoMQ::PollItem poll_push_error(ZMQ_POLLERR, push_socket, [&]{socket_error();});*/
    //CpperoMQ::PollItem poll_ctrl_error(ZMQ_POLLERR, ctrl_socket, [&]{socket_error();});


    auto poll_push = CpperoMQ::isSendReady(push_socket, [&](){
        auto tile = *tile_location;
        std::cout << "S: " << current_tile << " tile ";
        std::cout << std::get<0>(tile) << ", " << std::get<1>(tile) << ", " << std::get<2>(tile) << std::endl;
        std::stringstream buffer;

        buffer << std::get<0>(tile) << ", " << std::get<1>(tile) << ", " << std::get<2>(tile);

        //msgpack::pack(buffer, tile);

        const std::string& tmp = buffer.str();
        std::cout << "B:" << tmp << std::endl;
        push_socket.send(CpperoMQ::OutgoingMessage(tmp.size(), tmp.data()));
        ++tile_location;
        this->sent_tiles++;
    });

    auto poll_pull = CpperoMQ::isReceiveReady(pull_socket, [&](){

        bool more = true;
        while(more) {
            CpperoMQ::IncomingMessage inMsg;
            inMsg.receive(pull_socket, more);
          /*  msgpack::unpacked result;
            msgpack::unpack(result, inMsg.charData(), inMsg.size());
            msgpack::object deserialized = result.get();

            std::cout << "R:" << deserialized << std::endl;*/
            std::cout << "R: " << inMsg.charData() << std::endl;
            this->received_tiles++;
        }
    });

    CpperoMQ::Poller poller;
    while(received_tiles < 4) {
        poller.poll(poll_push, poll_pull);
    }
    std::cout << "Sent: " << this->sent_tiles << std::endl;
    std::cout << "Rec: " << this->received_tiles << std::endl;




    /*for (auto& const  tile: tileList) {
        this->send_tile(tile);
        this->
    }*/
}

const char* ZMQ_Server::get_addr(int port,std::string host) {
    std::stringstream ss;
    switch (this->_transport) {
    case Transport::IPC:
        ss << "ipc://" << host << ".zmq";
        break;
    case Transport::TCP:
    default:
        ss << "tcp://" << host << ":" << port;
        break;
    }
    const std::string& tmp = ss.str();
    std::cerr << "Adress: " << tmp.c_str() << std::endl;
    return tmp.c_str();
}

void ZMQ_Server::connect() {
    this->pull_socket.bind(this->get_addr(6666));
    this->push_socket.bind(this->get_addr(5555));
    this->ctrl_socket.bind(this->get_addr(7777));

}

void ZMQ_Server::start_new_task() {

}

void ZMQ_Server::collect_task() {
    //read and save results
}

void ZMQ_Server::manage() {
    //read heartbeat
}

void ZMQ_Server::socket_error() {
    std::cerr << "Error in socker" << std::endl;
    //KILLALL
}



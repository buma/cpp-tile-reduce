#include "zmq_server.hpp"
#include <thread>
#include <sstream>
#include <msgpack.hpp>

ZMQ_Server::ZMQ_Server(float minLon, float minLat, float maxLon, float maxLat, int zoom, ZMQ_Server::Transport transport)
    : Server(minLon, minLat, maxLon, maxLat, zoom),
      _transport(transport),
      pull_socket(this->context.createPullSocket()),
      push_socket(this->context.createPushSocket()),
      ctrl_socket(this->context.createRouterSocket()),
      sent_tiles(0), received_tiles(0), current_tile(0)
{



}

ZMQ_Server::ZMQ_Server(std::string filepath, int zoom, ZMQ_Server::Transport transport)
    : Server(filepath, zoom),
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
        std::cout << n << "threads" << std::endl;
        if (n != 0) {
            workers = n;
        }
    }
    this->connect();
    auto tile_location = this->tileList->begin();

    //Without this tasks get sent only to the first worker
    std::cout << "Press Enter when the workers are ready: " << std::endl;
    getchar();
    std::cout << "Sending tasks to workers...\n" << std::endl;



    //CpperoMQ::PollItem poll_ctrl(ZMQ_POLLOUT, ctrl_socket, this->manage());


    //CpperoMQ::PollItem poll_ctrl_error(ZMQ_POLLERR, ctrl_socket, [&]{socket_error();});


    auto poll_push = CpperoMQ::isSendReady(push_socket, [&](){
        if (tile_location == this->tileList->end()) {
            std::cerr << "Sent all list" << std::endl;
            return;
        }
        auto tile = *tile_location;
        std::cout << "S: " << this->sent_tiles << " tile ";
        std::cout << std::get<0>(tile) << ", " << std::get<1>(tile) << ", " << std::get<2>(tile) << std::endl;
        std::stringstream buffer;

        //buffer << std::get<0>(tile) << ", " << std::get<1>(tile) << ", " << std::get<2>(tile);

        msgpack::pack(buffer, tile);

        const std::string& tmp = buffer.str();
        //std::cout << "B:" << tmp << std::endl;
        push_socket.send(CpperoMQ::OutgoingMessage(tmp.size(), tmp.data()));
        ++tile_location;
        this->sent_tiles++;
        //std::cerr << "ST:" << this->sent_tiles << std::endl;
    });


    auto poll_pull = CpperoMQ::isReceiveReady(pull_socket, [&](){

        bool more = true;
        while(more) {
            CpperoMQ::IncomingMessage inMsg;
            inMsg.receive(pull_socket, more);
            msgpack::unpacked result;
            msgpack::unpack(result, inMsg.charData(), inMsg.size());
            msgpack::object deserialized = result.get();

            std::cout << "R: " << this->received_tiles << " " << deserialized << std::endl;
            //std::cout << "R: " << inMsg.charData() << std::endl;
            this->received_tiles++;
        }
    });


    CpperoMQ::Poller poller(-1); //-1 waits indefinitely until there is something to sent/receive which means less CPU usage
    //until we didn't receive all the responses
    while(received_tiles < this->tileList->size()) {
        //If we sent all tiles we don't need to poll on send socket anymore
        if (this->sent_tiles < this->tileList->size()) {
            poller.poll(poll_push, poll_pull);
        } else {
            poller.poll(poll_pull);
        }
    }
    std::cout << "Sent: " << this->sent_tiles << std::endl;
    std::cout << "Rec: " << this->received_tiles << std::endl;

}

const std::string ZMQ_Server::get_addr(int port,std::string host) const {
    std::string tmp;
    switch (this->_transport) {
    case Transport::IPC:
        //ss << "ipc://" << host << ".zmq";
        host = "socket";
        tmp = "ipc://" + host + "_" + std::to_string(port) + ".zmq";
        break;
    case Transport::TCP:
    default:
        //ss << "tcp://" << host << ":" << port;
        tmp = "tcp://" + host + ":" + std::to_string(port);
        break;
    }
    //const std::string& tmp = ss.str();
    std::cerr << "Adress: " << tmp << std::endl;
    return tmp;
}

void ZMQ_Server::connect() {
    this->push_socket.setSendHighWaterMark(2);
    this->pull_socket.bind(this->get_addr(6666).c_str());

    this->push_socket.bind(this->get_addr(5555).c_str());
    this->ctrl_socket.bind(this->get_addr(7777).c_str());

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



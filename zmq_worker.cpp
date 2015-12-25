#include <sstream>
#include <msgpack.hpp>
#include "zmq_worker.hpp"

ZMQ_Worker::ZMQ_Worker(std::string filepath, Transport transport) : Worker(filepath),
    _transport(transport),
    pull_socket(this->context.createPullSocket()),
    push_socket(this->context.createPushSocket()),
    ctrl_socket(this->context.createRouterSocket()),
    subscriber_socket(this->context.createSubscribeSocket()),
    sent_tiles(0), received_tiles(0), isDone(false)
{

}

void ZMQ_Worker::run() {
    std::cout << "Waiting:" << std::endl;
    this->connect();
    auto poll_pull = CpperoMQ::isReceiveReady(pull_socket, [&](){
        bool more = true;
        while(more) {
            CpperoMQ::IncomingMessage inMsg;
            inMsg.receive(pull_socket, more);
            msgpack::unpacked result;
            msgpack::unpack(result, inMsg.charData(), inMsg.size());
            // deserialized object is valid during the msgpack::unpacked instance alive.
            msgpack::object deserialized = result.get();

            // convert msgpack::object instance into the original type.
             // if the type is mismatched, it throws msgpack::type_error exception.
            TileTuple dst;

            try{
                deserialized.convert(&dst);
                std::cout << "R: " << this->received_tiles << " " << deserialized << std::endl;
                //z x y
                auto tileData = this->tileReader.get_tile(std::get<2>(dst), std::get<0>(dst), std::get<1>(dst));
                if (tileData) {
                    map(std::move(tileData));
                } else {
                    //std::cerr << "Problems reading tile!" << std::endl;
                    send(0);
                }
            } catch(const msgpack::type_error e) {
                std::cerr << "Received wrong tuple: " << deserialized << std::endl;
                isDone=true;
            }


            this->received_tiles++;
            //std::cout << "R: " << inMsg.charData() << std::endl;
        }
    });

    auto poll_sub = CpperoMQ::isReceiveReady(subscriber_socket, [&](){
       bool more = true;
       while(more) {
        CpperoMQ::IncomingMessage inMsg;
        inMsg.receive(subscriber_socket, more);
        std::string message(inMsg.charData(), inMsg.size());
        if (message.compare(DONE)==0) {
            isDone = true;
        } else {
            std::cerr << "Strange command: " << message << std::endl;
        }
       }
    });

    CpperoMQ::Poller poller(-1); //-1 waits indefinitely until there is something to sent/receive which means less CPU usage
    while(!isDone) {
        poller.poll(poll_pull, poll_sub);
        //std::cout << "." << std::endl;
    }
    this->info();

}

void ZMQ_Worker::info() {
    std::cout << "Rec: " << this->received_tiles << std::endl;
    std::cout << "Worked: " << this->sent_tiles << std::endl;

}

void ZMQ_Worker::connect() {
    this->pull_socket.connect(this->get_addr(5555).c_str());
    this->push_socket.connect(this->get_addr(6666).c_str());
    this->ctrl_socket.connect(this->get_addr(7777).c_str());
    this->subscriber_socket.connect(this->get_addr(4444).c_str());
    this->subscriber_socket.subscribe();

}

const std::string ZMQ_Worker::get_addr(int port,std::string host) const {
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

void ZMQ_Worker::send(int dataToSend) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, dataToSend);
    push_socket.send(CpperoMQ::OutgoingMessage(sbuf.size(), sbuf.data()));
    this->sent_tiles++;
}

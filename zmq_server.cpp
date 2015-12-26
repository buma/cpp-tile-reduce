#include "zmq_server.hpp"
#include <thread>
#include <sstream>
#include <msgpack.hpp>

#include <zmq_worker.hpp>
#ifdef TIMING
#include <chrono>
#endif

ZMQ_Server::ZMQ_Server(float minLon, float minLat, float maxLon, float maxLat, int zoom, Transport transport)
    : Server(minLon, minLat, maxLon, maxLat, zoom),
      _transport(transport),
      pull_socket(this->context.createPullSocket()),
      push_socket(this->context.createPushSocket()),
      ctrl_socket(this->context.createRouterSocket()),
      publish_socket(this->context.createPublishSocket()),
      sent_tiles(0), received_tiles(0), current_tile(0)
{



}

ZMQ_Server::ZMQ_Server(std::string filepath, int zoom, Transport transport)
    : Server(filepath, zoom),
      _transport(transport),
      pull_socket(this->context.createPullSocket()),
      push_socket(this->context.createPushSocket()),
      ctrl_socket(this->context.createRouterSocket()),
      publish_socket(this->context.createPublishSocket()),
      sent_tiles(0), received_tiles(0), current_tile(0)
{



}

void ZMQ_Server::run(bool start_workers, unsigned int num_workers) {
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << "threads" << std::endl;
    if (num_workers == 0 || num_workers > n) {
        if (n != 0) {
            num_workers = n;
        }
    }

    this->connect();

    std::vector<std::thread> tt;
    std::vector<std::shared_ptr<ZMQ_Worker>> workers;
    tt.reserve(num_workers);
    workers.reserve(num_workers);

    if (start_workers) {
        std::cout << "Starting " << num_workers << " workers." << std::endl;
        std::shared_ptr<ZMQ_Worker> worker= std::make_shared<ZMQ_Worker>(this->tileReader.get_filename(), _transport);

        for(int i=0; i < num_workers; i++) {
            workers.push_back(std::make_shared<ZMQ_Worker>(this->tileReader.get_filename(), this->context));
            tt.push_back(std::thread(&ZMQ_Worker::run, workers[i].get()));
        }

    } else {
        //Without this tasks get sent only to the first worker
        std::cout << "Press Enter when the workers are ready: " << std::endl;
        getchar();
        std::cout << "Sending tasks to workers...\n" << std::endl;

    }
#ifdef TIMING
    std::chrono::nanoseconds msgpack_encoding(0);
    std::chrono::nanoseconds msgpack_decoding(0);
#endif

    long sum = 0;



    //CpperoMQ::PollItem poll_ctrl(ZMQ_POLLOUT, ctrl_socket, this->manage());


    //CpperoMQ::PollItem poll_ctrl_error(ZMQ_POLLERR, ctrl_socket, [&]{socket_error();});

    msgpack::sbuffer sbuf;


    auto poll_push = CpperoMQ::isSendReady(push_socket, [&](){
        try {
            auto tile = this->get_tile();

            std::cout << "S: " << this->sent_tiles << " tile ";
            std::cout << std::get<0>(tile) << ", " << std::get<1>(tile) << ", " << std::get<2>(tile) << std::endl;
            //std::stringstream buffer;

            //buffer << std::get<0>(tile) << ", " << std::get<1>(tile) << ", " << std::get<2>(tile);
#ifdef TIMING
            auto start = std::chrono::high_resolution_clock::now();
#endif
            msgpack::pack(sbuf, tile);
#ifdef TIMING
            msgpack_encoding+=(std::chrono::high_resolution_clock::now()-start);
#endif

            //std::cout << "B:" << tmp << std::endl;
            push_socket.send(CpperoMQ::OutgoingMessage(sbuf.size(), sbuf.data()));
            sbuf.clear();
            //++tile_location;
            this->sent_tiles++;
            //std::cerr << "ST:" << this->sent_tiles << std::endl;
        } catch (const std::range_error& e) {
            std::cout << "End of tiles" << std::endl;
        }
    });


    auto poll_pull = CpperoMQ::isReceiveReady(pull_socket, [&](){

        bool more = true;
        while(more) {
            CpperoMQ::IncomingMessage inMsg;
            inMsg.receive(pull_socket, more);
            //Client sends empty message if it failed to decode anything
            if (inMsg.size() > 0) {
#ifdef TIMING
                auto start = std::chrono::high_resolution_clock::now();
#endif
                msgpack::unpacked result;
                msgpack::unpack(result, inMsg.charData(), inMsg.size());
                msgpack::object deserialized = result.get();

                int number;
                deserialized.convert(&number);
#ifdef TIMING
                msgpack_decoding+=(std::chrono::high_resolution_clock::now()-start);
#endif

                sum+=number;

                //std::cout << "R: " << this->received_tiles << " " << deserialized << std::endl;
            } else {
                //std::cout << "R: EMPTY MESSAGE" << std::endl;
            }
            //std::cout << "R: " << inMsg.charData() << std::endl;
            this->received_tiles++;
        }
    });


#ifdef TIMING
                std::chrono::nanoseconds serverDuration;
                auto startServer = std::chrono::high_resolution_clock::now();
#endif
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
#ifdef TIMING
    serverDuration = (std::chrono::high_resolution_clock::now()-startServer);
#endif

    this->publish_socket.send(CpperoMQ::OutgoingMessage(DONE.c_str()));
    if (start_workers) {
        for (int i=0; i<num_workers;i++) {
            tt[i].join();
        }
    }
#ifdef TIMING
    std::cout << "RUNTIME of " << "Server" << ": " << \
        std::chrono::duration_cast<std::chrono::milliseconds>( \
               serverDuration \
        ).count() << " ms " << std::endl;
    std::cout << "RUNTIME of " << "msgpack_encoding" << ": " << \
        std::chrono::duration_cast<std::chrono::milliseconds>( \
               msgpack_encoding \
        ).count() << " ms " << std::endl;
    std::cout << "RUNTIME of " << "msgpack_decoding" << ": " << \
        std::chrono::duration_cast<std::chrono::milliseconds>( \
               msgpack_decoding \
        ).count() << " ms " << std::endl;
#endif
    std::cout << "Sent: " << this->sent_tiles << std::endl;
    std::cout << "Rec: " << this->received_tiles << std::endl;

}



void ZMQ_Server::connect() {
    this->push_socket.setSendHighWaterMark(2);
    this->pull_socket.bind(get_addr(6666, _transport).c_str());

    this->push_socket.bind(get_addr(5555, _transport).c_str());
    this->ctrl_socket.bind(get_addr(7777, _transport).c_str());

    this->publish_socket.bind(get_addr(4444, _transport).c_str());

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



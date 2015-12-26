#include <sstream>
#include <thread>
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

ZMQ_Worker::ZMQ_Worker(std::string filepath, CpperoMQ::Context &context1) : Worker(filepath),
    _transport(Transport::INPROC),
    pull_socket(context1.createPullSocket()),
    push_socket(context1.createPushSocket()),
    ctrl_socket(context1.createRouterSocket()),
    subscriber_socket(context1.createSubscribeSocket()),
    sent_tiles(0), received_tiles(0), isDone(false)
{

}

void ZMQ_Worker::run() {
    std::cout << "Waiting. Worker:" << std::this_thread::get_id() << std::endl;

    this->connect();

#ifdef TIMING
    std::chrono::nanoseconds msgpack_decoding(0);
    std::chrono::nanoseconds tile_decoding(0);
    std::chrono::nanoseconds tile_mapping(0);
#endif
    std::unique_ptr<TileData> tileData;
    auto poll_pull = CpperoMQ::isReceiveReady(pull_socket, [&](){
        bool more = true;
        while(more) {
            CpperoMQ::IncomingMessage inMsg;
            inMsg.receive(pull_socket, more);
#ifdef TIMING
            auto start = std::chrono::high_resolution_clock::now();
#endif
            msgpack::unpacked result;
            msgpack::unpack(result, inMsg.charData(), inMsg.size());
            // deserialized object is valid during the msgpack::unpacked instance alive.
            msgpack::object deserialized = result.get();

            // convert msgpack::object instance into the original type.
             // if the type is mismatched, it throws msgpack::type_error exception.
            TileTuple dst;

            try{
                deserialized.convert(&dst);
#ifdef TIMING
                msgpack_decoding+=(std::chrono::high_resolution_clock::now()-start);
                start = std::chrono::high_resolution_clock::now();
#endif
                //std::cout << "R: " << this->received_tiles << " " << deserialized << std::endl;
                //z x y
                tileData = this->tileReader.get_tile(std::get<2>(dst), std::get<0>(dst), std::get<1>(dst));
#ifdef TIMING
                tile_decoding+=(std::chrono::high_resolution_clock::now()-start);
#endif
                if (tileData) {
#ifdef TIMING
                    start = std::chrono::high_resolution_clock::now();
#endif
                    map(std::move(tileData));
#ifdef TIMING
                    tile_mapping+=(std::chrono::high_resolution_clock::now()-start);
#endif
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

#ifdef TIMING
    std::cout << "RUNTIME of " << "msgpack_decoding" << ": " << \
        std::chrono::duration_cast<std::chrono::milliseconds>( \
               msgpack_decoding \
        ).count() << " ms " << std::endl;
    std::cout << "RUNTIME of " << "tile_decoding" << ": " << \
        std::chrono::duration_cast<std::chrono::milliseconds>( \
               tile_decoding \
        ).count() << " ms " << std::endl;
    std::cout << "RUNTIME of " << "tile_mapping" << ": " << \
        std::chrono::duration_cast<std::chrono::milliseconds>( \
               tile_mapping \
        ).count() << " ms " << std::endl;
    std::cout << "RUNTIME of " << "protobuf_decoding" << ": " << \
        std::chrono::duration_cast<std::chrono::milliseconds>( \
               this->tileReader.protobuf_decode \
        ).count() << " ms " << std::endl;
#endif

    this->info();

}

void ZMQ_Worker::info() {
    std::cout << "THread:" << std::this_thread::get_id() << std::endl;
    std::cout << "Rec: " << this->received_tiles << std::endl;
    std::cout << "Worked: " << this->sent_tiles << std::endl;

}

void ZMQ_Worker::connect() {
    this->pull_socket.connect(get_addr(5555, _transport).c_str());
    this->push_socket.connect(get_addr(6666, _transport).c_str());
    this->ctrl_socket.connect(get_addr(7777, _transport).c_str());
    this->subscriber_socket.connect(get_addr(4444, _transport).c_str());
    this->subscriber_socket.subscribe();

}

void ZMQ_Worker::send(int dataToSend) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, dataToSend);
    push_socket.send(CpperoMQ::OutgoingMessage(sbuf.size(), sbuf.data()));
    this->sent_tiles++;
}

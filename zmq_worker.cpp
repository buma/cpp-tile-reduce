#include <sstream>
#include <msgpack.hpp>
#include "zmq_worker.hpp"

ZMQ_Worker::ZMQ_Worker(std::string filepath, Transport transport) : Worker(filepath),
    _transport(transport),
    pull_socket(this->context.createPullSocket()),
    push_socket(this->context.createPushSocket()),
    ctrl_socket(this->context.createRouterSocket()),
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
                    std::cerr << "Problems reading tile!" << std::endl;
                }
            } catch(const msgpack::type_error e) {
                std::cerr << "Received wrong tuple: " << deserialized << std::endl;
                isDone=true;
            }


            this->received_tiles++;
            //std::cout << "R: " << inMsg.charData() << std::endl;
        }
    });

    CpperoMQ::Poller poller(-1); //-1 waits indefinitely until there is something to sent/receive which means less CPU usage
    while(!isDone) {
        poller.poll(poll_pull);
        //std::cout << "." << std::endl;
    }
    this->info();

}

bool ZMQ_Worker::filterStreet(const TileFeature *feature) {
        //TODO: create this outside a function
        std::set<std::string> streetValues = {"paved",
                "concrete",
                "asphalt",
                "concrete:plates",
                "cobblestone",
                "cobblestone:flattened",
                "sett"};
    return feature->hasTagValue("highway", "footway") &&
            !feature->hasTagValue("footway", "sidewalk") &&
            !feature->hasTagValue("footway", "crossing") &&
            !feature->hasTagValue("area", "yes") &&
            !feature->hasTag("area:highway") &&
            !feature->hasTagValue("tunnel", "yes") &&
            (!feature->hasTag("surface") || feature->hasTagValue("surface", streetValues));
}

void ZMQ_Worker::info() {
    std::cout << "Rec: " << this->received_tiles << std::endl;
    std::cout << "Worked: " << this->sent_tiles << std::endl;

}

void ZMQ_Worker::map(std::unique_ptr<TileData> tileData) {

    auto footways = tileData->getLayer("osm")->filter(filterStreet);
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, footways.size());
    //std::cout << "Footways: " << footways.size() << std::endl;
    push_socket.send(CpperoMQ::OutgoingMessage(sbuf.size(), sbuf.data()));
    this->sent_tiles++;
}

void ZMQ_Worker::connect() {
    this->pull_socket.connect(this->get_addr(5555).c_str());
    this->push_socket.connect(this->get_addr(6666).c_str());
    this->ctrl_socket.connect(this->get_addr(7777).c_str());

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

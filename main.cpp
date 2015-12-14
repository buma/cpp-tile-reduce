#include <iostream>
#include <fstream>
#include <zhelpers.hpp>
#include <msgpack.hpp>
#include "generated/vector_tile.pb.hpp"
#include "tiledata.hpp"

using namespace std;


int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    auto filename = "/home/mabu/programiranje/osm-qa/tile-reduce-python/out.mapbox";

    mapnik::vector::tile vector_tile;
    fstream fs(filename, ios_base::binary | ios_base::in);
    if (!fs) {
        cerr << "File " << filename << " Not found" << endl;
    } else if(!vector_tile.ParseFromIstream(&fs)) {
        cerr << "Failed to parse vector tile";
        return -1;
    }



    cout << vector_tile.GetTypeName() << endl;

    mapnik::vector::tile::layer layer = vector_tile.layers(0);

    cout << "name " << layer.name() << endl;
    cout << "version " << layer.version() << endl;
    if (layer.has_extent()) {
        cout << "extent " << layer.extent() << endl;
    }
    cout << "features " << layer.features_size() << endl;

    TileData TileData(vector_tile);

    //cout << "Name: " << vector_tile->layer.name() << endl;
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}

void worker() {
    zmq::context_t context(1);

    zmq::socket_t pull_socket(context, ZMQ_PULL);
    pull_socket.connect("tcp://localhost:5555");

    zmq::socket_t push_socket(context, ZMQ_PUSH);
    push_socket.connect("tcp://localhost:6666");
    cout << "Hello World!" << endl;
    zmq::message_t message;

    while(1) {

        pull_socket.recv(&message);

        msgpack::unpacked result = msgpack::unpack((const char*)message.data(), message.size());

        msgpack::object deserialized = result.get();

        cout << deserialized << endl;

        msgpack::sbuffer sbuf;
        int num = 22;
        msgpack::pack(sbuf, num);
        message.rebuild(sbuf.size());
        memcpy(message.data(), sbuf.data(), sbuf.size());
        push_socket.send(message);


    }
}

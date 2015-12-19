/*
#include <msgpack.hpp>
#include <string>
#include <iostream>
#include <sstream>

int main(void)
{
    msgpack::type::tuple<int, bool, std::string> src(1, true, "example");

    // serialize the object into the buffer.
    // any classes that implements write(const char*,size_t) can be a buffer.
    std::stringstream buffer;
    msgpack::pack(buffer, src);

    // send the buffer ...
    buffer.seekg(0);

    // deserialize the buffer into msgpack::object instance.
    std::string str(buffer.str());

    msgpack::unpacked result;

    msgpack::unpack(result, str.data(), str.size());

    // deserialized object is valid during the msgpack::unpacked instance alive.
    msgpack::object deserialized = result.get();

    // msgpack::object supports ostream.
    std::cout << deserialized << std::endl;

    // convert msgpack::object instance into the original type.
    // if the type is mismatched, it throws msgpack::type_error exception.
    msgpack::type::tuple<int, bool, std::string> dst;
    deserialized.convert(&dst);



    std::cout << dst.get<0>() << std::endl;

    return 0;
}*/

#ifdef DELAJ
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
//#include <zhelpers.hpp>
//#include <msgpack.hpp>
#include "generated/vector_tile.pb.hpp"
#include "tiledata.hpp"

#include "tippecanoe/decode.hpp"
#include "mbtilereader.h"

extern "C" {
#include "tippecanoe/projection.h"
}

using namespace std;

//bool filterStreet(const TileFeature * feature, const std::set<std::string> & values) {
bool filterStreet(const TileFeature * feature) {
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

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    auto filename = "/home/mabu/programiranje/osm-qa/tile-reduce-python/out.mapbox";

    auto mbtiles_filename = "/home/mabu/programiranje/osm-qa/data/slovenia.mbtiles";

    /*std::ifstream fs(filename, ios_base::binary | ios_base::in);
    std::stringstream buffer;
    buffer << fs.rdbuf();*/

    /*mapnik::vector::tile vector_tile;
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
*/
    /*
    long long x_tile;
    long long y_tile;
    double lat=46.557611;
    double lon=15.6455;
    cout << "lat" << lat << " , " << "lon" << lon << endl;
    latlon2tile(lat,lon , 12, &x_tile, &y_tile);
    cout << x_tile << ", " << y_tile << endl;*/


    MBTileReader mbtiles(mbtiles_filename);

    auto tiles = mbtiles.get_tiles_inside(12,15.5760, 46.5086, 15.7139, 46.5985);
    cout << "Tiles:" << endl;
    for (auto& tile: tiles) {
        cout << std::get<0>(tile) << ", " << std::get<1>(tile) << ", " << std::get<2>(tile) << endl;
    }

    if (0) {
            //TileData tileData(vector_tile, 12, 2225, 1446);
            //std::string message = buffer.str();
            //TileData tileData(message, 12, 2225, 1446);
            TileData tileData = *mbtiles.get_tile(12,2225,1446);

            cout << *tileData.getLayer("osm")->getFeature(1) << endl;
            //auto filterStreet1 = std::bind(filterStreet, std::placeholders::_1, streetValues);
            auto footways = tileData.getLayer("osm")->filter(filterStreet);
            cout << "Footways:" << endl;
            for (auto&& footway: footways) {
                cout << *footway;
            }
    } else {
            //handle(vector_tile, 12,2225,1446,1);

    }


    //cout << "Name: " << vector_tile->layer.name() << endl;
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}

#ifdef ZMQ
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
#endif

#endif

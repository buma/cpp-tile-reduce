#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include "docopt.h"
#include "zmq_server.hpp"



static const char USAGE[] =
R"(CPP worker


        Usage:
          cpp-worker worker [zmq | output] <mbtiles>
          cpp-worker server [zmq | output] (<mbtiles> | (--bbox <minLon> <minLat> <maxLon> <maxLat>))
          cpp-worker (-h | --help)
          cpp-worker --version

        Options:
          -h --help   Show this screen.
          --version   Show version.
          --protocol  How does worker and server communicate. [default: zmq]
)";


int main(int argc, const char** argv) {
    using std::stof;
    using std::cout;
    using std::endl;
    std::map<std::string, docopt::value> args
            = docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "Tile reduce 0.1");
    for (auto const & arg: args) {
        std::cout << arg.first << " " << arg.second << std::endl;
    }
    bool server = args["server"].asBool();
    bool zmq = (args["output"].asBool() != true);
    bool hasBBox = args["--bbox"].asBool();
    std::string filepath;
    if (!hasBBox) {
        filepath = args["<mbtiles>"].asString();

        std::ifstream f(filepath);
        if (!f.good()) {
            f.close();
            std::cerr << "File " << filepath << " doesn't exists!";
            std::exit(EXIT_FAILURE);
        }

        std::cout << "Filepath: " << filepath << std::endl;
    }
    if (server) {
        std::cout << "We are on server ";

    } else {
        std::cout << "we are on client ";
    }
    if (zmq) {
        std::cout << "with zmq ";
    } else {
        std::cout << "with stdin/out ";
    }

    if (server) {
        std::unique_ptr<Server> server;
        if(hasBBox) {
            cout << endl;
            float minLon = stof(args["<minLon>"].asString());
            float minLat = stof(args["<minLat>"].asString());
            float maxLon = stof(args["<maxLon>"].asString());
            float maxLat = stof(args["<maxLat>"].asString());

            std::cout << "minLon: " << minLon << endl;
            std::cout << "minLat: " << minLat << endl;
            std::cout << "maxLon: " << maxLon << endl;
            std::cout << "maxLat: " << maxLat << endl;
            server = std::unique_ptr<Server>(new ZMQ_Server(minLon, minLat, maxLon, maxLat));
        }else {
            server = std::unique_ptr<Server>(new ZMQ_Server(filepath));
        }
        std::cout << std::endl << "Tiles:" << server->get_tiles_num() << std::endl;
        server->run();

    }

    std::cout << std::endl;
    return 0;
}


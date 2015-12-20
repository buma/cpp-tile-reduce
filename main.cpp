#include <iostream>
#include <string>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include "docopt.h"
#include "zmq_server.hpp"
#include "zmq_worker.hpp"



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
std::unique_ptr<Worker> worker;

void signal_handler(int signal) {
    if (worker) {
        std::cout << "finish" << std::endl;
        worker->info();
    }
    std::exit(0);
}

int main(int argc, const char** argv) {
    using std::stof;
    using std::cout;
    using std::endl;
    std::signal(SIGINT, signal_handler);
    std::map<std::string, docopt::value> args
            = docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "Tile reduce 0.1");
    for (auto const & arg: args) {
        std::cout << arg.first << " " << arg.second << std::endl;
    }
    bool is_server = args["server"].asBool();
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
    if (is_server) {
        std::cout << "We are on server ";

    } else {
        std::cout << "we are on client ";
    }
    if (zmq) {
        std::cout << "with zmq ";
    } else {
        std::cout << "with stdin/out ";
    }

    std::cout << std::endl;
    if (is_server) {
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

    } else {

        std::unique_ptr<Worker> worker = std::unique_ptr<Worker>(new ZMQ_Worker(filepath));
        worker->run();
    }

    std::cout << std::endl;
    return 0;
}


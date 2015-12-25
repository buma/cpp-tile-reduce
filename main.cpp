#include <iostream>
#include <string>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include "docopt.h"
#include "zmq_server.hpp"
#include "stdout_server.hpp"
#include "zmq_worker.hpp"
#include "stdout_worker.hpp"



static const char USAGE[] =
R"(CPP worker


        Usage:
          cpp-worker worker [zmq | output] <mbtiles>
          cpp-worker server [zmq | output] (<mbtiles> | (--bbox <minLon> <minLat> <maxLon> <maxLat>))
          cpp-worker collector
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
    using std::cerr;
    using std::endl;
    std::signal(SIGINT, signal_handler);
    std::map<std::string, docopt::value> args
            = docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "Tile reduce 0.1");
    for (auto const & arg: args) {
        std::cerr << arg.first << " " << arg.second << std::endl;
    }
    bool is_server = args["server"].asBool();
    bool is_worker = args["worker"].asBool();
    bool zmq = (args["output"].asBool() != true);
    bool hasBBox = args["--bbox"].asBool();
    std::string filepath;
    if (!hasBBox && (is_server || is_worker)) {
        filepath = args["<mbtiles>"].asString();

        std::ifstream f(filepath);
        if (!f.good()) {
            f.close();
            std::cerr << "File " << filepath << " doesn't exists!";
            std::exit(EXIT_FAILURE);
        }

        std::cerr << "Filepath: " << filepath << std::endl;
    }
    if (is_server) {
        std::cerr << "We are on server ";

    } else {
        std::cerr << "we are on client ";
    }
    if (zmq) {
        std::cerr << "with zmq ";
    } else {
        std::cerr << "with stdin/out ";
    }

    std::cerr << std::endl;
    if (is_server) {
        std::unique_ptr<Server> server;
        if(hasBBox) {
            cerr << endl;
            float minLon = stof(args["<minLon>"].asString());
            float minLat = stof(args["<minLat>"].asString());
            float maxLon = stof(args["<maxLon>"].asString());
            float maxLat = stof(args["<maxLat>"].asString());

            std::cerr << "minLon: " << minLon << endl;
            std::cerr << "minLat: " << minLat << endl;
            std::cerr << "maxLon: " << maxLon << endl;
            std::cerr << "maxLat: " << maxLat << endl;
            if (zmq) {
                server = std::unique_ptr<Server>(new ZMQ_Server(minLon, minLat, maxLon, maxLat, 12, Transport::TCP));
            } else {
                server = std::unique_ptr<Server>(new Stdout_Server(minLon, minLat, maxLon, maxLat));
            }
        }else {
            if (zmq) {
                server = std::unique_ptr<Server>(new ZMQ_Server(filepath,12, Transport::TCP));
            } else {
                server = std::unique_ptr<Server>(new Stdout_Server(filepath));
            }
        }
        std::cerr << std::endl << "Tiles:" << server->get_tiles_num() << std::endl;
        server->run();

    } else if (is_worker) {
        if (zmq) {
            worker = std::unique_ptr<Worker>(new ZMQ_Worker(filepath, Transport::TCP));
        } else {
            worker = std::unique_ptr<Worker>(new Stdout_Worker(filepath));
        }
        worker->run();
    } else {
        int sum=0;
        std::string line;
        int current;
        while(std::getline(std::cin, line)) {
            std::istringstream line_stream(line);
            //std::cerr << "LINE:" << line << std::endl;
            if (line_stream.get() == ']') {
                line_stream >> current;
                sum+=current;
            }
        }
        std::cerr << "Sum: " << sum << std::endl;
    }
    std::cerr << std::endl;
    return 0;
}


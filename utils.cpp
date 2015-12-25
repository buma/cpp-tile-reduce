#include <iostream>
#include "utils.hpp"


std::string get_addr(int port, Transport transport, std::string host)  {
    std::string tmp;
    switch (transport) {
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

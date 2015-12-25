#ifndef UTILS
#define UTILS
#include <string>

enum class Transport {
    TCP,
    IPC,
    INPROC
};
std::string get_addr(int port,Transport transport=Transport::TCP, std::string host="127.0.0.1");


const std::string CONNECT = "CONNECT";
const std::string DONE = "DONE";

#endif // UTILS


#include "stdout_worker.hpp"
#ifdef PROFILE
#include <gperftools/heap-profiler.h>
#endif


Stdout_Worker::Stdout_Worker(std::string filepath) :
    Worker(filepath), sent_tiles(0), received_tiles(0)
{

}

void Stdout_Worker::run() {
    std::cerr << "Waiting:" << std::endl;
#ifdef PROFILE
    //ProfilerStart("zmq_worker.log");
    HeapProfilerStart("worker_heap_2.log");
#endif
    std::string line;
    unsigned x, y;
    int z;
    while(std::getline(std::cin, line)) {
        std::istringstream line_stream(line);
        //std::cerr << "LINE:" << line << std::endl;
        if (line_stream.get() == '[') {
          line_stream >> x;
          line_stream >> y;
          line_stream >> z;
          auto tileData = this->tileReader.get_tile(z, x, y);
#ifdef PROFILE
          if (this->received_tiles%50==0) {
            HeapProfilerDump("TileRead");
          }
#endif
          this->received_tiles++;
          if (tileData) {
              //std::cerr << " Found tile" << std::endl;
              map(std::move(tileData));
          } else{
              send(0);
          }
        }
    }
#ifdef PROFILE
    HeapProfilerStop();
    //ProfilerStop();
#endif
}

void Stdout_Worker::info() {
    std::cerr << "Rec: " << this->received_tiles << std::endl;
    std::cerr << "Worked: " << this->sent_tiles << std::endl;

}

void Stdout_Worker::send(int dataToSend) {
    std::cout << "]" << dataToSend << std::endl;
    this->sent_tiles++;
}


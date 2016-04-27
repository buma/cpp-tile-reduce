#include "stdout_server.hpp"

Stdout_Server::Stdout_Server(float minLon, float minLat, float maxLon, float maxLat, int zoom)
    : Server(minLon, minLat, maxLon, maxLat, zoom),
      sent_tiles(0), received_tiles(0), current_tile(0)
{

}

Stdout_Server::Stdout_Server(std::string filepath, int zoom)
    :Server(filepath, zoom),
      sent_tiles(0), received_tiles(0), current_tile(0)
{

}

void Stdout_Server::run(bool start_workers, unsigned int workers) {
    if (!start_workers) {
        std::cout << "Press Enter when the workers are ready: " << std::endl;
        getchar();
        std::cout << "Sending tasks to workers...\n" << std::endl;
    }
    for (const auto& tile: *this->tileList.get()) {
        std::cout << "[" << std::get<0>(tile) << " " << std::get<1>(tile) << " " << std::get<2>(tile) << "\n";
    }

}

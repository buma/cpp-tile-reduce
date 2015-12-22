#include "worker.hpp"

Worker::Worker(std::string filepath) : tileReader(MBTileReader(filepath))
{

}

bool Worker::filterStreet(const TileFeature *feature) {
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

void Worker::map(std::unique_ptr<TileData> tileData) {

    auto footways = tileData->getLayer("osm")->filter(filterStreet);

    //std::cout << "Footways: " << footways.size() << std::endl;
    send(footways.size());
}


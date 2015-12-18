#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <zlib.h>
#include <math.h>
#include <memory>
#include <iostream>
#include "generated/vector_tile.pb.hpp"
#include "decode.hpp"

#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>

#include <geos/io/WKTWriter.h>

extern "C" {
#include "projection.h"
}

// https://github.com/mapbox/mapnik-vector-tile/blob/master/src/vector_tile_compression.hpp
inline bool is_compressed(std::string const &data) {
    return data.size() > 2 && (((uint8_t) data[0] == 0x78 && (uint8_t) data[1] == 0x9C) || ((uint8_t) data[0] == 0x1F && (uint8_t) data[1] == 0x8B));
}

// https://github.com/mapbox/mapnik-vector-tile/blob/master/src/vector_tile_compression.hpp
inline int decompress(std::string const &input, std::string &output) {
    z_stream inflate_s;
    inflate_s.zalloc = Z_NULL;
    inflate_s.zfree = Z_NULL;
    inflate_s.opaque = Z_NULL;
    inflate_s.avail_in = 0;
    inflate_s.next_in = Z_NULL;
    if (inflateInit2(&inflate_s, 32 + 15) != Z_OK) {
        fprintf(stderr, "error: %s\n", inflate_s.msg);
    }
    inflate_s.next_in = (Bytef *) input.data();
    inflate_s.avail_in = input.size();
    size_t length = 0;
    do {
        output.resize(length + 2 * input.size());
        inflate_s.avail_out = 2 * input.size();
        inflate_s.next_out = (Bytef *) (output.data() + length);
        int ret = inflate(&inflate_s, Z_FINISH);
        if (ret != Z_STREAM_END && ret != Z_OK && ret != Z_BUF_ERROR) {
            fprintf(stderr, "error: %s\n", inflate_s.msg);
            return 0;
        }

        length += (2 * input.size() - inflate_s.avail_out);
    } while (inflate_s.avail_out == 0);
    inflateEnd(&inflate_s);
    output.resize(length);
    return 1;
}

#define VT_POINT 1
#define VT_LINE 2
#define VT_POLYGON 3

#define VT_END 0
#define VT_MOVETO 1
#define VT_LINETO 2
#define VT_CLOSEPATH 7

#define VT_STRING 1
#define VT_NUMBER 2
#define VT_BOOLEAN 7

int dezig(unsigned n) {
	return (n >> 1) ^ (-(n & 1));
}

void printq(const char *s) {
	putchar('"');
	for (; *s; s++) {
		if (*s == '\\' || *s == '"') {
			printf("\\%c", *s);
		} else if (*s >= 0 && *s < ' ') {
			printf("\\u%04x", *s);
		} else {
			putchar(*s);
		}
	}
	putchar('"');
}

struct draw {
	int op;
	double lon;
	double lat;

	draw(int op, double lon, double lat) {
		this->op = op;
		this->lon = lon;
		this->lat = lat;
	}
};
void handle(std::string message, int z, unsigned x, unsigned y, mapnik::vector::tile &tile) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	// https://github.com/mapbox/mapnik-vector-tile/blob/master/examples/c%2B%2B/tileinfo.cpp
    //mapnik::vector::tile tile;

	if (is_compressed(message)) {
		std::string uncompressed;
		decompress(message, uncompressed);
		if (!tile.ParseFromString(uncompressed)) {
			fprintf(stderr, "Couldn't decompress tile %d/%u/%u\n", z, x, y);
			exit(EXIT_FAILURE);
		}
	} else if (!tile.ParseFromString(message)) {
		fprintf(stderr, "Couldn't parse tile %d/%u/%u\n", z, x, y);
		exit(EXIT_FAILURE);
	}
    //handle(tile, z, x, y, describe);
}

std::unique_ptr<geos::geom::Geometry> handle(mapnik::vector::tile_feature & feat, int extent, int z, unsigned x, unsigned y,std::unique_ptr<geos::geom::GeometryFactory> && geometry_factory) {

    int px = 0, py = 0;

    std::vector<draw> ops;

    for (int g = 0; g < feat.geometry_size(); g++) {
        uint32_t geom = feat.geometry(g);
        uint32_t op = geom & 7;
        uint32_t count = geom >> 3;

        if (op == VT_MOVETO || op == VT_LINETO) {
            for (unsigned k = 0; k < count; k++) {
                px += dezig(feat.geometry(g + 1));
                py += dezig(feat.geometry(g + 2));
                g += 2;

                long long scale = 1LL << (32 - z);
                long long wx = scale * x + (scale / extent) * (px + .5);
                long long wy = scale * y + (scale / extent) * (py + .5);

                double lat, lon;
                tile2latlon(wx, wy, 32, &lat, &lon);

                ops.push_back(draw(op, lon, lat));
            }
        } else {
            ops.push_back(draw(op, 0, 0));
        }
    }

    if (feat.type() == VT_POINT) {
        if (ops.size() == 1) {
            //printf("\"type\": \"Point\", \"coordinates\": [ %f, %f ]", ops[0].lon, ops[0].lat);
            auto point = geometry_factory->createPoint(geos::geom::Coordinate(ops[0].lon, ops[0].lat));
            //std::string tmp = wkt->write(point);
            //std::cout << tmp;
            return std::unique_ptr<geos::geom::Geometry>(point);

        } else {
            //printf("\"type\": \"MultiPoint\", \"coordinates\": [ ");
            for (unsigned i = 0; i < ops.size(); i++) {
                if (i != 0) {
                    //printf(", ");
                }
                //printf("[ %f, %f ]", ops[i].lon, ops[i].lat);
            }
            //printf(" ]");
        }
    } else if (feat.type() == VT_LINE) {
        int movetos = 0;
        for (unsigned i = 0; i < ops.size(); i++) {
            if (ops[i].op == VT_MOVETO) {
                movetos++;
            }
        }

        if (movetos < 2) {
            //printf("\"type\": \"LineString\", \"coordinates\": [ ");
            geos::geom::CoordinateArraySequence array_seq;
            for (unsigned i = 0; i < ops.size(); i++) {
                if (i != 0) {
                    //printf(", ");
                }
                array_seq.add(geos::geom::Coordinate(ops[i].lon, ops[i].lat));
                //printf("[ %f, %f ]", ops[i].lon, ops[i].lat);
            }
            auto linestring = geometry_factory->createLineString(array_seq);
            //std::cout << wkt->write(linestring);
            return std::unique_ptr<geos::geom::Geometry>(linestring);

            //printf(" ]");
        } else {
            //printf("\"type\": \"MultiLineString\", \"coordinates\": [ [ ");
            int state = 0;
            for (unsigned i = 0; i < ops.size(); i++) {
                if (ops[i].op == VT_MOVETO) {
                    if (state == 0) {
                        //printf("[ %f, %f ]", ops[i].lon, ops[i].lat);
                        state = 1;
                    } else {
                        //printf(" ], [ ");
                        //printf("[ %f, %f ]", ops[i].lon, ops[i].lat);
                        state = 1;
                    }
                } else {
                    //printf(", [ %f, %f ]", ops[i].lon, ops[i].lat);
                }
            }
            //printf(" ] ]");
        }
    } else if (feat.type() == VT_POLYGON) {
        std::vector<std::vector<draw> > rings;
        std::vector<double> areas;

        for (unsigned i = 0; i < ops.size(); i++) {
            if (ops[i].op == VT_MOVETO) {
                rings.push_back(std::vector<draw>());
                areas.push_back(0);
            }

            int n = rings.size() - 1;
            if (n >= 0) {
                rings[n].push_back(ops[i]);
            }
        }

        int outer = 0;

        for (unsigned i = 0; i < rings.size(); i++) {
            double area = 0;
            for (unsigned k = 0; k < rings[i].size(); k++) {
                if (rings[i][k].op != VT_CLOSEPATH) {
                    area += rings[i][k].lon * rings[i][(k + 1) % rings[i].size()].lat;
                    area -= rings[i][k].lat * rings[i][(k + 1) % rings[i].size()].lon;
                }
            }

            areas[i] = area;
            if (areas[i] <= 0 || i == 0) {
                outer++;
            }

            // //printf("area %f\n", area / .00000274 / .00000274);
        }

        if (outer > 1) {
            //printf("\"type\": \"MultiPolygon\", \"coordinates\": [ [ [ ");
        } else {
            //printf("\"type\": \"Polygon\", \"coordinates\": [ [ ");
        }

        int state = 0;
        for (unsigned i = 0; i < rings.size(); i++) {
            if (areas[i] <= 0) {
                if (state != 0) {
                    // new multipolygon
                    //printf(" ] ], [ [ ");
                }
                state = 1;
            }

            if (state == 2) {
                // new ring in the same polygon
                //printf(" ], [ ");
            }

            for (unsigned j = 0; j < rings[i].size(); j++) {
                if (rings[i][j].op != VT_CLOSEPATH) {
                    if (j != 0) {
                        //printf(", ");
                    }

                    //printf("[ %f, %f ]", rings[i][j].lon, rings[i][j].lat);
                } else {
                    if (j != 0) {
                        //printf(", ");
                    }

                    //printf("[ %f, %f ]", rings[i][0].lon, rings[i][0].lat);
                }
            }

            state = 2;
        }

        if (outer > 1) {
            //printf(" ] ] ]");
        } else {
            //printf(" ] ]");
        }
    }

    //printf(" } }\n");
    return nullptr;
}

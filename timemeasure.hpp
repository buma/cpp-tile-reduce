#ifndef TIMEMEASURE
#define TIMEMEASURE

#ifdef TIMING

#include <chrono>

struct TimeMeasure {
    std::chrono::nanoseconds protobuf_decode;
};

#endif


#endif // TIMEMEASURE


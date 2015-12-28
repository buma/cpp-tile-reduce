TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS_RELEASE = -march=native -O2 -fstack-protector-strong

DEFINES += TIMING

SOURCES += main.cpp \
    msgpack_test.cpp \
    generated/vector_tile.pb.cc \
    tiledata.cpp \
    tilelayer.cpp \
    tilefeature.cpp \
    tippecanoe/decode.cc \
    tippecanoe/projection.c \
    mbtilereader.cpp \
    server.cpp \
    zmq_server.cpp \
    worker.cpp \
    zmq_worker.cpp \
    stdout_server.cpp \
    stdout_worker.cpp \
    utils.cpp

INCLUDEPATH = /home/mabu/programiranje/cpp/msgpack-c/include /home/mabu/programiranje/cpp/CpperoMQ/include /home/mabu/programiranje/cpp/docopt.cpp

LIBS += -lzmq -lprotobuf -lm -lz -lgeos -lsqlite3 -pthread -L/home/mabu/programiranje/cpp/docopt.cpp/build -ldocopt

HEADERS += \
    tiledata.hpp \
    generated/vector_tile.pb.hpp \
    tilelayer.hpp \
    tilefeature.h \
    tippecanoe/projection.h \
    tippecanoe/decode.hpp \
    mbtilereader.h \
    zmq_server.hpp \
    server.hpp \
    worker.hpp \
    zmq_worker.hpp \
    utils.hpp \
    stdout_server.hpp \
    stdout_worker.hpp \
    timemeasure.hpp


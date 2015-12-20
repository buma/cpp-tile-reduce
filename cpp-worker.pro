TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

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
    zmq_server.cpp

INCLUDEPATH = /home/mabu/programiranje/cpp/msgpack-c/include /home/mabu/programiranje/cpp/CpperoMQ/include /home/mabu/programiranje/cpp/docopt.cpp

LIBS += -lzmq -lprotobuf -lm -lz -lgeos -lsqlite3 -L/home/mabu/programiranje/cpp/docopt.cpp/build -ldocopt

HEADERS += \
    tiledata.hpp \
    generated/vector_tile.pb.hpp \
    tilelayer.hpp \
    tilefeature.h \
    tippecanoe/projection.h \
    tippecanoe/decode.hpp \
    mbtilereader.h \
    zmq_server.hpp \
    server.hpp


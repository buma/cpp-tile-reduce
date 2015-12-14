TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    msgpack_test.cpp \
    generated/vector_tile.pb.cc \
    tiledata.cpp \
    tilelayer.cpp

INCLUDEPATH = /home/mabu/programiranje/cpp/msgpack-c/include libraries/zmq /home/mabu/programiranje/cpp/variant/include

LIBS += -lzmq -lprotobuf

HEADERS += \
    tiledata.hpp \
    generated/vector_tile.pb.hpp \
    tilelayer.hpp


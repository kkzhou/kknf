LIBEVENT_DIR = /usr/local/libevent
LIBCONFIG_DIR = /usr/local/libconfig

INSTALL_DIR = /usr/local/aanf

INCLUDE_DIR = $(LIBEVENT_DIR)/include -I$(LIBCONFIG_DIR)/include
LIBRARY_DIR = $(LIBEVENT_DIR)/lib -L$(LIBCONFIG_DIR)/lib

CC = g++
CPPFLAGS = -I$(INCLUDE_DIR)
LDFLAGS = -L$(LIBRARY_DIR)
CXXFLAGS = -g -O0

LIB_OBJS = bin_tcp_socket.o \
        line_tcp_socket.o \
        memblock.o \
        netframe.o \
        packet.o \
        server.o \
        socket.o \
        socket_pool.o \
        tcp_socket.o \
        test_bb_1.o \
        test_bb_2.o \
        test_bf.o \
        test_client.o \
        test_server.o \
        utils.o

LIB_OBJS = bin_tcp_socket.o \
        line_tcp_socket.o \
        memblock.o \
        netframe.o \
        packet.o \
        server.o \
        socket.o \
        socket_pool.o \
        tcp_socket.o \
        test_bb_1.o \
        test_bb_2.o \
        test_bf.o \
        test_client.o \
        test_server.o \
        utils.o

aanf_lib:

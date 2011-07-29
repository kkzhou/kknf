
bb1: test_bb1.o util.hpp socket.hpp processor.hpp server.hpp
    g++ -o bb1 test_bb1.cc

bb2: test_bb2.o util.hpp socket.hpp processor.hpp server.hpp
    g++ -o bb2 test_bb2.cc

bf: test_bf.o util.hpp socket.hpp processor.hpp server.hpp
    g++ -o bf test_bf.cc

client: test_client.o util.hpp socket.hpp client.hpp
    g++ -o client test_client.cc

simple_server: test_simple_server.o util.hpp socket.hpp processor.hpp server.hpp
    g++ -o simple_server test_simple_server.cc

simple_client: test_simple_client.o util.hpp socket.hpp client.hpp
    g++ -o simple_client test_simple_client.cc

all: bb1 bb2 bf client simple_server simple_client

.PHONY
clean:
    rm -f *.o bb1 bb2 bf simple_server simple_client

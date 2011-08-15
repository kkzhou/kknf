CC = g++
CXXFLAGS = -g -Wall -O0

all: bb1 bb2 bf c ss sc mpc mps

bb1: test_bb1.o util.hpp socket.hpp processor.hpp server.hpp
	$(CC) $(CXXFLAGS) -o bb1 test_bb1.cc -lpthread

bb2: test_bb2.o util.hpp socket.hpp processor.hpp server.hpp
	$(CC) $(CXXFLAGS) -o bb2 test_bb2.cc -lpthread

bf: test_bf.o util.hpp socket.hpp processor.hpp server.hpp
	$(CC) $(CXXFLAGS)  -o bf test_bf.cc -lpthread

c: test_c.o util.hpp socket.hpp client.hpp
	$(CC) $(CXXFLAGS)  -o c test_c.cc -lpthread

ss: test_simple_server.o util.hpp socket.hpp processor.hpp server.hpp
	$(CC) $(CXXFLAGS)  -o ss test_simple_server.cc -lpthread

sc: test_simple_client.o util.hpp socket.hpp client.hpp
	$(CC) $(CXXFLAGS) -o sc test_simple_client.cc -lpthread

mps: test_multi_pool_server.o util.hpp socket.hpp processor.hpp server.hpp
	$(CC) $(CXXFLAGS) -o mps test_multi_pool_server.cc -lpthread

mpc: test_multi_pool_client.o util.hpp socket.hpp client.hpp
	$(CC) $(CXXFLAGS) -o mpc test_multi_pool_client.cc -lpthread

.PHONY:clean
clean:
	rm -f *.o bb1 bb2 bf c ss sc mpc mps


all: bb1 bb2 bf client simple_server simple_client
bb1: test_bb1.o util.hpp socket.hpp processor.hpp server.hpp
	g++ -Wall -O0 -o bb1 test_bb1.cc -lpthread

bb2: test_bb2.o util.hpp socket.hpp processor.hpp server.hpp
	g++  -Wall -o bb2 test_bb2.cc -lpthread

bf: test_bf.o util.hpp socket.hpp processor.hpp server.hpp
	g++  -Wall -o bf test_bf.cc -lpthread

client: test_client.o util.hpp socket.hpp client.hpp
	g++ -Wall  -o client test_client.cc -lpthread

simple_server: test_simple_server.o util.hpp socket.hpp processor.hpp server.hpp
	g++ -Wall  -o simple_server test_simple_server.cc -lpthread

simple_client: test_simple_client.o util.hpp socket.hpp client.hpp
	g++ -Wall  -o simple_client test_simple_client.cc -lpthread


.PHONY:clean
clean:
	rm -f *.o bb1 bb2 bf client simple_server simple_client

CC = g++ -g -O0 -Wall

BOOST_DIR = /usr/local/boost/
INC_DIR = -I../include/ -I$(BOOST_DIR)/include/
LIB_DIR = -L$(BOOST_DIR)/lib/
LIBS = -lboost_system -lboost_date_time -lboost_random -lboost_thread


all : client bb1 bb2 bf client2

client :
	$(CC) -o client $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_client.cc

client2 :
	$(CC) -o client $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_client2.cc

bb1 :
	$(CC) -o bb1  $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bb1.cc

bb2 :
	$(CC) -o bb2  $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bb2.cc

bf :
	$(CC) -o bf $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bf.cc

.PHONY : clean

clean:
	rm -f bb1 bb2 bf client

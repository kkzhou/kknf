CC = g++ -g

BOOST_DIR = /usr/local/boost/
INC_DIR = -I../include/ -I$(BOOST_DIR)/include/
LIB_DIR = -L$(BOOST_DIR)/lib/
LIBS = -lboost_date_time -lboost_random -lboost_thread -lboost_system


all : client bb1 bb2 bf

client : 
	$(CC) -o client $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_client.cc

bb1 :
	$(CC) -o bb1  $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bb1.cc

bb2 :
	$(CC) -o bb2  $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bb2.cc

bf :
	$(CC) -o bf $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bf.cc

.PHONY : clean

clean:
	rm -f bb1 bb2 bf client


BOOST_DIR = /usr/local/boost/
INC_DIR = -I../include/ -I$(BOOST_DIR)/include/
LIB_DIR = -L$(BOOST_DIR)/lib/
LIBS = -lboost_date_time -lboost_random -lboost_thread -lboost_system

client : 
	g++ -o client $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_client.cc

bb1 :
	g++ -o bb1 $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bb1.cc

bb2 :
	g++ -o bb2 $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bb2.cc

bf :
	g++ -o bf $(INC_DIR) $(LIB_DIR) $(LIBS) test_simple_bf.cc


CC = g++ -g -O0 -Wall

#BOOST_DIR = /usr/local/environment/boost/
BOOST_DIR = /usr/local/boost/
INC_DIR = -I../include/ -I$(BOOST_DIR)/include/
LIB_DIR = -L$(BOOST_DIR)/lib/
LIBS = -lboost_system -lboost_date_time -lboost_random -lboost_thread


all :  udp_b udp_c cb_lv_c cb_lv_c_for_bb1 cb_lv_bb1 cb_lv_bb2 cb_lv_bf


udp_c :
	$(CC) -o udp_c $(INC_DIR) $(LIB_DIR) $(LIBS) udp_c.cc

udp_b :
	$(CC) -o udp_bb1  $(INC_DIR) $(LIB_DIR) $(LIBS) udp_bb1.cc

cb_lv_c :
	$(CC) -o cb_lv_c $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_c.cc

cb_lv_c_for_bb1 :
	$(CC) -o cb_lv_c_for_bb1 $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_c_for_bb1.cc

cb_lv_bb1 :
	$(CC) -o cb_lv_bb1  $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_bb1.cc

cb_lv_bb2 :
	$(CC) -o cb_lv_bb2  $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_bb2.cc

cb_lv_bf :
	$(CC) -o cb_lv_bf $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_bf.cc

.PHONY : clean

clean:
	rm -f udp_b udp_c
	rm -f cb_lv_c cb_lv_bb1 cb_lv_bb2 cb_lv_bf cb_lv_c_for_bb1

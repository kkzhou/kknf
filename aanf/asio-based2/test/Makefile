CC = g++ -g -O0 -Wall

#BOOST_DIR = /usr/local/environment/boost/
BOOST_DIR = /usr/local/boost/
INC_DIR = -I../include/ -I$(BOOST_DIR)/include/
LIB_DIR = -L$(BOOST_DIR)/lib/
LIBS = -lboost_system -lboost_date_time -lboost_random -lboost_thread


all : lv_c lv_bb1 lv_bb2 lv_bf lv_c_for_bb1 udp_c udp_bb1 udp_bb2 udp_bf \
        udp_c_for_bb1 http_c http_bb1 http_bb2 http_bf cb_lv_c cb_lv_bb1 \
        cb_lv_bb2 cb_lv_bf

lv_c :
	$(CC) -o lv_c $(INC_DIR) $(LIB_DIR) $(LIBS) lv_c.cc

lv_c_for_bb1 :
	$(CC) -o lv_c_for_bb1 $(INC_DIR) $(LIB_DIR) $(LIBS) lv_c_for_bb1.cc

lv_bb1 :
	$(CC) -o lv_bb1  $(INC_DIR) $(LIB_DIR) $(LIBS) lv_bb1.cc

lv_bb2 :
	$(CC) -o lv_bb2  $(INC_DIR) $(LIB_DIR) $(LIBS) lv_bb2.cc

lv_bf :
	$(CC) -o lv_bf $(INC_DIR) $(LIB_DIR) $(LIBS) lv_bf.cc

udp_c :
	$(CC) -o udp_c $(INC_DIR) $(LIB_DIR) $(LIBS) udp_c.cc

udp_c_for_bb1 :
	$(CC) -o udp_c_for_bb1 $(INC_DIR) $(LIB_DIR) $(LIBS) udp_c_for_bb1.cc

udp_bb1 :
	$(CC) -o udp_bb1  $(INC_DIR) $(LIB_DIR) $(LIBS) udp_bb1.cc

udp_bb2 :
	$(CC) -o udp_bb2  $(INC_DIR) $(LIB_DIR) $(LIBS) udp_bb2.cc

udp_bf :
	$(CC) -o udp_bf $(INC_DIR) $(LIB_DIR) $(LIBS) udp_bf.cc

http_c :
	$(CC) -o http_c $(INC_DIR) $(LIB_DIR) $(LIBS) http_c.cc

http_bb1 :
	$(CC) -o http_bb1  $(INC_DIR) $(LIB_DIR) $(LIBS) http_bb1.cc

http_bb2 :
	$(CC) -o http_bb2  $(INC_DIR) $(LIB_DIR) $(LIBS) http_bb2.cc

http_bf :
	$(CC) -o http_bf $(INC_DIR) $(LIB_DIR) $(LIBS) http_bf.cc

cb_lv_c :
	$(CC) -o cb_lv_c $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_c.cc

cb_lv_bb1 :
	$(CC) -o cb_lv_bb1  $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_bb1.cc

cb_lv_bb2 :
	$(CC) -o cb_lv_bb2  $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_bb2.cc

cb_lv_bf :
	$(CC) -o cb_lv_bf $(INC_DIR) $(LIB_DIR) $(LIBS) cb_lv_bf.cc

.PHONY : clean

clean:
	rm -f lv_c_for_bb1 lv_c lv_bb1 lv_bb2 lv_bf
	rm -f udp_bb1 udp_bb2 udp_bf udp_c udp_c_for_bb1
	rm -f http_bb1 http_bb2 http_bf http_c
	rm -f cb_lv_c cb_lv_bb1 cb_lv_bb2 cb_lv_bf

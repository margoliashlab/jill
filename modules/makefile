CFLAGS = -g -Wall 

jfilter:
	g++ jfilter.cc -o jfilter -I../jill ../jill/zmq.o ../jill/program_options.o ../jill/logger.o ../jill/jack_client.o -lboost_program_options -lboost_filesystem -lboost_date_time -lboost_system -ljack -lpthread -lrt -lzmq

jpop:
	g++ jpop.cc -o jpop -I../jill ../jill/zmq.o ../jill/program_options.o ../jill/logger.o ../jill/jack_client.o -lboost_program_options -lboost_filesystem -lboost_date_time -lboost_system -ljack -lpthread -lrt -lzmq
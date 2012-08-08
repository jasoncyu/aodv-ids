OBJS = cluster_test.o common.o cluster.o cluster_algorithm.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
GSLCOMPILE = -I/usr/local/include
GSLLINK = -L/usr/local/lib -lgsl -lgslcblas -lm
test: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o test $(GSLLINK)
cluster_test.o: cluster_test.cc common.h cluster.h
	$(CC) $(CFLAGS) cluster_test.cc
common.o: common.cc common.h
	$(CC) $(CFLAGS) common.cc
cluster.o: cluster.cc cluster.h common.h
	$(CC) $(CFLAGS) cluster.cc
cluster_algorithm.o: cluster_algorithm.cc cluster_algorithm.h
	$(CC) $(CFLAGS) cluster_algorithm.cc $(GSLCOMPILE) -o cluster_algorithm.o

clean:
	\rm *.o test

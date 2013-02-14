# Makefile

CC = g++ -w

all:
	$(CC) -g -o proxy proxy.cpp cacheEntry.cpp

clean:
	\rm -f *.o *~ *.out proxy

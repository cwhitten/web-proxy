# Makefile

CC = g++ -w
PTHREAD = -pthread

all:
	$(CC) -g -o proxy proxy.cpp $(PTHREAD)

clean:
	\rm -f *.o *~ *.out proxy

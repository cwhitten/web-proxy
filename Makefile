# Makefile

MAIN = proxy
PROJECT = proxy
CC = g++

all:
	$(CC) -o $(PROJECT) $(MAIN).cpp

clean:
	\rm -f *.o *~ *.out $(PROJECT)

tar:
	tar cfv $(PROJECT).tar $(MAIN).cpp

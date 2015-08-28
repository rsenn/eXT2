
# Makefile for libaam.so

CXX = g++

OPT = -O2
CFLAGS = -shared $(OPT) -lasound -ljack

all : libaam.so

libaam.so : jack.cpp
	$(CXX) $(CFLAGS) $< -o $@ 

clean :
	rm -f libaam.so



CXX=g++
CFLAGS=-lpthread -O3

all: server

server:
	$(CXX) $(CFLAGS) -o server *.cpp
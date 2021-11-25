CXX=g++
CFLAGS=-lpthread -g

all: test

test:
	$(CXX) $(CFLAGS) -o wrm agent.cpp client.cpp error_trace.cpp errors.cpp message_chain.cpp message_queue.cpp rwlock.cpp server.cpp shared_mem.cpp test_writer.cpp

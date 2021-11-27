CXX=g++
CFLAGS=-lpthread -g

all:
	$(CXX) $(CFLAGS) -o hw1srv agent.cpp client.cpp error_trace.cpp errors.cpp message_chain.cpp rwlock.cpp shared_mem.cpp server.cpp message_queue.cpp test_server.cpp
	$(CXX) $(CFLAGS) -o hw1cli agent.cpp client.cpp error_trace.cpp errors.cpp message_chain.cpp rwlock.cpp shared_mem.cpp server.cpp message_queue.cpp test_client.cpp

test:
	$(CXX) $(CFLAGS) -o wrm agent.cpp client.cpp error_trace.cpp errors.cpp message_chain.cpp rwlock.cpp shared_mem.cpp test_writer.cpp
	$(CXX) $(CFLAGS) -o wrs agent.cpp client.cpp error_trace.cpp errors.cpp message_chain.cpp rwlock.cpp shared_mem.cpp test_writer_slave.cpp
	$(CXX) $(CFLAGS) -o rds agent.cpp client.cpp error_trace.cpp errors.cpp message_chain.cpp rwlock.cpp shared_mem.cpp test_reader.cpp

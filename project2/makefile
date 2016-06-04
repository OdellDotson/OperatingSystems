all: server

server: server.o 
	g++ server.o -lpthread -o server
	rm *.o

server.o: server.cpp 
	g++ -std=c++0x -c server.cpp
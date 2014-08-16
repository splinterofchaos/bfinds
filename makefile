CXX = g++ -std=c++11

bfinds : main.cpp find.o
	${CXX} -g -fwhole-program -pthread -o bfinds main.cpp find.o

find.o : find.cpp find.h
	${CXX} -g -pthread -o find.o -c find.cpp


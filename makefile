CXX = g++ -std=c++11

bfinds : main.cpp find.o
	${CXX} -pthread -O2 -fwhole-program -o bfinds main.cpp find.o

find.o : find.cpp find.h
	${CXX} -O2 -o find.o -c find.cpp


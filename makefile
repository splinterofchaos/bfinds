CXX = g++ -std=c++1y

bfinds : main.cpp *.o match.h
	${CXX} -pthread -O2 -fwhole-program -o bfinds main.cpp match.o find.o

find.o : find.cpp find.h match.h
	${CXX} -O2 -o find.o -c find.cpp

match.o : match.h match.cpp
	${CXX} -O2 -o match.o -c match.cpp

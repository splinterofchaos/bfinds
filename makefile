CXX = g++ -std=c++11

bfinds : main.cpp find.*
	${CXX} -O2 -o bfinds main.cpp find.cpp


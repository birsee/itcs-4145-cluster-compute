# ITCS 4145, in-class exercise: the degree sum.
CC  = gcc
CXX = g++
CFLAGS   = -O2 -Wall -fopenmp
CXXFLAGS = -O2 -Wall -fopenmp -std=c++17

all: degree degree_cpp

degree: degree.c edgelist.h
	$(CC) $(CFLAGS) $< -o $@

degree_cpp: degree.cpp edgelist.hpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f degree degree_cpp

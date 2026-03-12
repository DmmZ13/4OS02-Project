# Makefile for timing question (Q1)
SRC_DIR=../src
CXX=g++ -fopenmp
CXXFLAGS=-std=c++17 -O3 -march=native -Wall
LIBS=-lpthread
VPATH=$(SRC_DIR)

.PHONY: all clean

all: ant_simu_headless.exe ant_simu.exe
	python3 run_timing.py

ant_simu.exe: ant.o fractal_land.o renderer.o window.o ant_simu.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS) -lSDL2

ant_simu_headless.exe: ant.o fractal_land.o ant_simu_headless.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# Shared sources (from src/)
ant.o: $(SRC_DIR)/ant.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

fractal_land.o: $(SRC_DIR)/fractal_land.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

renderer.o: $(SRC_DIR)/renderer.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

window.o: $(SRC_DIR)/window.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

# Question-specific main files (local)
ant_simu.o: ant_simu.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

ant_simu_headless.o: ant_simu_headless.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

clean:
	@rm -f *.csv *.o *.exe

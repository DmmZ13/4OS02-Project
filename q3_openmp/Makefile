# Makefile for OpenMP question (Q3)
SRC_DIR=../src
CXX=g++ -fopenmp
CXXFLAGS=-std=c++17 -O3 -march=native -Wall
LIBS=-lpthread -lSDL2
VPATH=$(SRC_DIR)

.PHONY: all clean

all: ant_simu_openmp.exe ant_simu_openmp_headless.exe

ant_simu_openmp.exe: ant_openmp.o fractal_land.o renderer.o window.o ant_simu_openmp.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

ant_simu_openmp_headless.exe: ant_openmp.o fractal_land.o ant_simu_openmp_headless.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread

# Shared sources (from src/)
ant_openmp.o: $(SRC_DIR)/ant_openmp.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

fractal_land.o: $(SRC_DIR)/fractal_land.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

renderer.o: $(SRC_DIR)/renderer.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

window.o: $(SRC_DIR)/window.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

# Question-specific main files (local)
ant_simu_openmp.o: ant_simu_openmp.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

ant_simu_openmp_headless.o: ant_simu_openmp_headless.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

clean:
	@rm -f *.o *.exe

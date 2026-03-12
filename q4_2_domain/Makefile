# Makefile for question 4.2 (domain-decomposed MPI)
SRC_DIR=../src
CXX=g++ -fopenmp
CXXFLAGS=-std=c++17 -O3 -march=native -Wall
LIBS=-lpthread
VPATH=$(SRC_DIR)

.PHONY: all clean

all: ant_simu_mpi_domain.exe
	python3 run_mpi_measure.py

ant_simu_mpi_domain.exe: ant.o fractal_land.o ant_simu_mpi_domain.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# Shared sources (from src/)
ant.o: $(SRC_DIR)/ant.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

fractal_land.o: $(SRC_DIR)/fractal_land.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

# Question-specific main file (local)
ant_simu_mpi_domain.o: ant_simu_mpi_domain.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)

clean:
	@rm -f *.o *.exe *.out *.csv

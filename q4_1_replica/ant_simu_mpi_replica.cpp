#if __has_include(<mpi.h>)
#include <mpi.h>
#else
#include "mpi_stub.h"
#endif
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "fractal_land.hpp"
#include "ant.hpp"
#include "pheronome.hpp"
#include "rand_generator.hpp"

void advance_time_mpi_replica(const fractal_land& land, pheronome& phen,
                              const position_t& pos_nest, const position_t& pos_food,
                              std::vector<ant>& ants, std::size_t& cpteur,
                              double& t_ant, double& t_evap, double& t_ph)
{
    auto start = std::chrono::high_resolution_clock::now();
    for(auto& a : ants) {
        a.advance(phen, land, pos_food, pos_nest, cpteur);
    }
    auto end = std::chrono::high_resolution_clock::now();
    t_ant += std::chrono::duration<double, std::milli>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    phen.do_evaporation();
    end = std::chrono::high_resolution_clock::now();
    t_evap += std::chrono::duration<double, std::milli>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    phen.update();
    end = std::chrono::high_resolution_clock::now();
    t_ph += std::chrono::duration<double, std::milli>(end - start).count();

    // synchronise pheromones across all ranks
    phen.mpi_sync(MPI_COMM_WORLD);
}

int main(int nargs, char* argv[])
{
    MPI_Init(&nargs, &argv);
    int rank, nranks;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nranks);

    // parameters (could be read from argv)
    const int nb_ants = 5000;
    const double eps = 0.8;
    const double alpha=0.7;
    const double beta=0.999;
    position_t pos_nest{256,256};
    position_t pos_food{500,500};

    std::size_t seed = 42 + rank; // different per rank for random moves

    // each rank generates the same landscape (same seed) so it's replicated
    auto t0 = std::chrono::high_resolution_clock::now();
    fractal_land land(8,2,1.,1024);
    auto t1 = std::chrono::high_resolution_clock::now();
    double time_land = std::chrono::duration<double, std::milli>(t1-t0).count();

    // normalisation (identique sur chaque rang)
    {
        double maxv = 0., minv = 1e9;
        for(auto i=0u;i<land.dimensions();++i)
            for(auto j=0u;j<land.dimensions();++j) {
                maxv = std::max(maxv, land(i,j));
                minv = std::min(minv, land(i,j));
            }
        double delta = maxv - minv;
        for(auto i=0u;i<land.dimensions();++i)
            for(auto j=0u;j<land.dimensions();++j)
                land(i,j) = (land(i,j)-minv)/delta;
    }

    ant::set_exploration_coef(eps);

    // distribute ants among ranks
    std::vector<ant> ants;
    for(int i = rank; i < nb_ants; i += nranks) {
        ants.emplace_back(pos_nest, seed + i);
    }

    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);

    std::size_t food = 0;
    double t_ant=0, t_evap=0, t_ph=0;
    std::size_t it = 0;

    auto tsim0 = std::chrono::high_resolution_clock::now();
    while(it < 100) {
        ++it;
        advance_time_mpi_replica(land, phen, pos_nest, pos_food, ants, food, t_ant, t_evap, t_ph);
    }
    auto tsim1 = std::chrono::high_resolution_clock::now();
    double ttotal = std::chrono::duration<double, std::milli>(tsim1-tsim0).count();

    // gather times to rank 0 for reporting
    double sum_ant, sum_evap, sum_ph;
    MPI_Reduce(&t_ant, &sum_ant, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_evap, &sum_evap, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_ph, &sum_ph, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        std::cout << "== MPI Replica ("<<nranks<<" ranks) ==\n";
        std::cout << "Land gen time: "<<time_land<<" ms\n";
        std::cout << "Total sim time: "<<ttotal<<" ms\n";
        std::cout << "Ant advance (sum over ranks): "<<sum_ant<<" ms\n";
        std::cout << "Evaporation (sum): "<<sum_evap<<" ms\n";
        std::cout << "Pheromone update (sum): "<<sum_ph<<" ms\n";
        std::cout << "Food collected (partial): "<<food<<" per rank.\n";
        std::cout << "===========================\n";
    }

    MPI_Finalize();
    return 0;
}

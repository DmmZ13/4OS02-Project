#if __has_include(<mpi.h>)
#include <mpi.h>
#else
#include "mpi_stub.h"
#endif
#include <vector>
#include <iostream>
#include <chrono>
#include "fractal_land.hpp"
#include "ant.hpp"
#include "pheronome.hpp"
#include "rand_generator.hpp"

// compute start/end row for given rank
static void compute_rows(int N, int nranks, int rank, int &start, int &end) {
    int base = N / nranks;
    int rem  = N % nranks;
    start = rank * base + std::min(rank, rem);
    end   = start + base + (rank < rem ? 1 : 0);
}

void exchange_boundaries(pheronome& phen, int rank, int nranks) {
    int N = phen.dim();
    int stride = N + 2;
    int start, end;
    compute_rows(N, nranks, rank, start, end);

    // we'll exchange the internal rows (start and end-1) with neighbors
    std::vector<double> sendbuf(stride * 2), recvbuf(stride * 2);
    MPI_Status status;
    // send first row to rank-1 and receive last row from rank-1
    if(rank > 0) {
        phen.pack_row(start+1, sendbuf.data()); // +1 for ghost offset
        MPI_Sendrecv(sendbuf.data(), sendbuf.size(), MPI_DOUBLE, rank-1, 1,
                     recvbuf.data(), recvbuf.size(), MPI_DOUBLE, rank-1, 2,
                     MPI_COMM_WORLD, &status);
        phen.unpack_row(start, recvbuf.data());
    }
    if(rank < nranks-1) {
        phen.pack_row(end, sendbuf.data());
        MPI_Sendrecv(sendbuf.data(), sendbuf.size(), MPI_DOUBLE, rank+1, 2,
                     recvbuf.data(), recvbuf.size(), MPI_DOUBLE, rank+1, 1,
                     MPI_COMM_WORLD, &status);
        phen.unpack_row(end+1, recvbuf.data());
    }
}

void advance_time_mpi_domain(const fractal_land& land, pheronome& phen,
                             const position_t& pos_nest, const position_t& pos_food,
                             std::vector<ant>& ants, std::size_t& cpteur,
                             double& t_ant, double& t_evap, double& t_ph,
                             int rank, int nranks)
{
    int N = land.dimensions();
    int start, end;
    compute_rows(N, nranks, rank, start, end);

    auto start_t = std::chrono::high_resolution_clock::now();
    for(auto& a : ants) {
        a.advance(phen, land, pos_food, pos_nest, cpteur);
    }
    auto end_t = std::chrono::high_resolution_clock::now();
    t_ant += std::chrono::duration<double, std::milli>(end_t - start_t).count();

    start_t = std::chrono::high_resolution_clock::now();
    // evaporation over the whole map (simpler prototype)
    phen.do_evaporation();
    end_t = std::chrono::high_resolution_clock::now();
    t_evap += std::chrono::duration<double, std::milli>(end_t - start_t).count();

    start_t = std::chrono::high_resolution_clock::now();
    phen.update();
    end_t = std::chrono::high_resolution_clock::now();
    t_ph += std::chrono::duration<double, std::milli>(end_t - start_t).count();

    // exchange only the boundary rows with neighbours to keep maps consistent
    exchange_boundaries(phen, rank, nranks);
}

int main(int nargs, char* argv[])
{
    MPI_Init(&nargs, &argv);
    int rank, nranks;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nranks);

    const int nb_ants = 5000;
    const double eps = 0.8;
    const double alpha=0.7;
    const double beta=0.999;
    position_t pos_nest{256,256};
    position_t pos_food{500,500};

    std::size_t seed = 42 + rank;

    fractal_land land(8,2,1.,1024);
    // normalisation same as before
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
    std::vector<ant> ants;
    for(int i = rank; i < nb_ants; i += nranks)
        ants.emplace_back(pos_nest, seed + i);

    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);
    std::size_t food = 0;
    double t_ant=0, t_evap=0, t_ph=0;
    std::size_t it = 0;
    auto tsim0 = std::chrono::high_resolution_clock::now();
    while(it < 100) {
        ++it;
        advance_time_mpi_domain(land, phen, pos_nest, pos_food, ants, food, t_ant, t_evap, t_ph, rank, nranks);
    }
    auto tsim1 = std::chrono::high_resolution_clock::now();
    double ttotal = std::chrono::duration<double, std::milli>(tsim1-tsim0).count();

    double sum_ant, sum_evap, sum_ph;
    MPI_Reduce(&t_ant, &sum_ant, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_evap, &sum_evap, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_ph, &sum_ph, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if(rank==0) {
        std::cout << "== MPI Domain ("<<nranks<<" ranks) ==\n";
        std::cout << "Total sim time: "<<ttotal<<" ms\n";
        std::cout << "Ant advance (sum): "<<sum_ant<<" ms\n";
        std::cout << "Evap (sum): "<<sum_evap<<" ms\n";
        std::cout << "Pher update (sum): "<<sum_ph<<" ms\n";
    }

    MPI_Finalize();
    return 0;
}

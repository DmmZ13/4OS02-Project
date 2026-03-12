#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include "fractal_land.hpp"
#include "ant_openmp.hpp"
#include "pheronome.hpp"
#include "rand_generator.hpp"

void advance_time_openmp( const fractal_land& land, pheronome& phen, 
                          const position_t& pos_nest, const position_t& pos_food,
                          ant_openmp& ants, std::size_t& cpteur,
                          double& time_ant_advance, double& time_evaporation, double& time_pheromone_update )
{
    auto start = std::chrono::high_resolution_clock::now();
    ants.advance_all(phen, land, pos_food, pos_nest, cpteur);
    auto end = std::chrono::high_resolution_clock::now();
    time_ant_advance += std::chrono::duration<double, std::milli>(end - start).count();
    
    start = std::chrono::high_resolution_clock::now();
    phen.do_evaporation();
    end = std::chrono::high_resolution_clock::now();
    time_evaporation += std::chrono::duration<double, std::milli>(end - start).count();
    
    start = std::chrono::high_resolution_clock::now();
    phen.update();
    end = std::chrono::high_resolution_clock::now();
    time_pheromone_update += std::chrono::duration<double, std::milli>(end - start).count();
}

int main(int nargs, char* argv[])
{
    std::size_t seed = 2026;
    const int nb_ants = 5000;
    const double eps = 0.8;
    const double alpha=0.7;
    const double beta=0.999;
    position_t pos_nest{256,256};
    position_t pos_food{500,500};
    
    auto time_begin = std::chrono::high_resolution_clock::now();
    fractal_land land(8,2,1.,1024);
    auto time_end = std::chrono::high_resolution_clock::now();
    double time_land_generation = std::chrono::duration<double, std::milli>(time_end - time_begin).count();
    
    double max_val = 0.0;
    double min_val = 0.0;
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j ) {
            max_val = std::max(max_val, land(i,j));
            min_val = std::min(min_val, land(i,j));
        }
    double delta = max_val - min_val;
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j )  {
            land(i,j) = (land(i,j)-min_val)/delta;
        }
    
    ant_openmp::set_exploration_coef(eps);
    
    ant_openmp ants(nb_ants);
    ants.initialize(land, seed);
    
    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);

    size_t food_quantity = 0;
    bool not_food_in_nest = true;
    std::size_t it = 0;
    
    double time_ant_advance = 0.0;
    double time_evaporation = 0.0;
    double time_pheromone_update = 0.0;
    
    auto time_sim_begin = std::chrono::high_resolution_clock::now();
    
    const int max_iterations = 100;
    while (it < max_iterations) {
        ++it;
        advance_time_openmp( land, phen, pos_nest, pos_food, ants, food_quantity,
                             time_ant_advance, time_evaporation, time_pheromone_update );
        
        if ( not_food_in_nest && food_quantity > 0 ) {
            std::cout << "First food arrived at nest at iteration " << it << std::endl;
            not_food_in_nest = false;
        }
    }
    
    auto time_sim_end = std::chrono::high_resolution_clock::now();
    double time_total = std::chrono::duration<double, std::milli>(time_sim_end - time_sim_begin).count();
    
    int num_threads = 0;
    #pragma omp parallel
    #pragma omp single
    num_threads = omp_get_num_threads();
    
    std::cout << "\n========== TIME MEASUREMENTS (OpenMP Version - Headless) ==========" << std::endl;
    std::cout << "Number of threads:        " << num_threads << std::endl;
    std::cout << "Land generation time:     " << std::fixed << std::setprecision(3) << time_land_generation << " ms" << std::endl;
    std::cout << "Number of iterations:     " << it << std::endl;
    std::cout << "Ant advance time:         " << std::fixed << std::setprecision(3) << time_ant_advance << " ms" << std::endl;
    std::cout << "Evaporation time:         " << std::fixed << std::setprecision(3) << time_evaporation << " ms" << std::endl;
    std::cout << "Pheromone update time:    " << std::fixed << std::setprecision(3) << time_pheromone_update << " ms" << std::endl;
    std::cout << "Total simulation time:    " << std::fixed << std::setprecision(3) << time_total << " ms" << std::endl;
    std::cout << "Time per iteration:       " << std::fixed << std::setprecision(3) << (time_ant_advance + time_evaporation + time_pheromone_update) / it << " ms" << std::endl;
    std::cout << "Food collected:           " << food_quantity << std::endl;
    std::cout << "====================================================================\n" << std::endl;
    
    return 0;
}

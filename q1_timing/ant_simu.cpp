#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include "fractal_land.hpp"
#include "ant.hpp"
#include "pheronome.hpp"
# include "renderer.hpp"
# include "window.hpp"
# include "rand_generator.hpp"

void advance_time( const fractal_land& land, pheronome& phen, 
                   const position_t& pos_nest, const position_t& pos_food,
                   std::vector<ant>& ants, std::size_t& cpteur,
                   double& time_ant_advance, double& time_evaporation, double& time_pheromone_update )
{
    auto start = std::chrono::high_resolution_clock::now();
    for ( size_t i = 0; i < ants.size(); ++i )
        ants[i].advance(phen, land, pos_food, pos_nest, cpteur);
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
    SDL_Init( SDL_INIT_VIDEO );
    std::size_t seed = 2026; // Graine pour la génération aléatoire ( reproductible )
    const int nb_ants = 5000; // Nombre de fourmis
    const double eps = 0.8;  // Coefficient d'exploration
    const double alpha=0.7; // Coefficient de chaos
    //const double beta=0.9999; // Coefficient d'évaporation
    const double beta=0.999; // Coefficient d'évaporation
    // Location du nid
    position_t pos_nest{256,256};
    // Location de la nourriture
    position_t pos_food{500,500};
    //const int i_food = 500, j_food = 500;    
    // Génération du territoire 512 x 512 ( 2*(2^8) par direction )
    
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
    /* On redimensionne les valeurs de fractal_land de sorte que les valeurs
    soient comprises entre zéro et un */
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j )  {
            land(i,j) = (land(i,j)-min_val)/delta;
        }
    // Définition du coefficient d'exploration de toutes les fourmis.
    ant::set_exploration_coef(eps);
    // On va créer des fourmis un peu partout sur la carte :
    std::vector<ant> ants;
    ants.reserve(nb_ants);
    auto gen_ant_pos = [&land, &seed] () { return rand_int32(0, land.dimensions()-1, seed); };
    for ( size_t i = 0; i < nb_ants; ++i )
        ants.emplace_back(position_t{gen_ant_pos(),gen_ant_pos()}, seed);
    // On crée toutes les fourmis dans la fourmilière.
    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);

    Window win("Ant Simulation", 2*land.dimensions()+10, land.dimensions()+266);
    Renderer renderer( land, phen, pos_nest, pos_food, ants );
    // Compteur de la quantité de nourriture apportée au nid par les fourmis
    size_t food_quantity = 0;
    SDL_Event event;
    bool cont_loop = true;
    bool not_food_in_nest = true;
    std::size_t it = 0;
    
    // Variables pour mesurer le temps
    double time_ant_advance = 0.0;
    double time_evaporation = 0.0;
    double time_pheromone_update = 0.0;
    double time_rendering = 0.0;
    
    auto time_sim_begin = std::chrono::high_resolution_clock::now();
    while (cont_loop) {
        ++it;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                cont_loop = false;
        }
        advance_time( land, phen, pos_nest, pos_food, ants, food_quantity,
                      time_ant_advance, time_evaporation, time_pheromone_update );
        
        auto render_begin = std::chrono::high_resolution_clock::now();
        renderer.display( win, food_quantity );
        win.blit();
        auto render_end = std::chrono::high_resolution_clock::now();
        time_rendering += std::chrono::duration<double, std::milli>(render_end - render_begin).count();
        if ( not_food_in_nest && food_quantity > 0 ) {
            std::cout << "La première nourriture est arrivée au nid a l'iteration " << it << std::endl;
            not_food_in_nest = false;
        }
        //SDL_Delay(10);
    }
    auto time_sim_end = std::chrono::high_resolution_clock::now();
    double time_total = std::chrono::duration<double, std::milli>(time_sim_end - time_sim_begin).count();
    
    // Affichage des résultats
    std::cout << "\n========== TIME MEASUREMENTS (Original Version) ==========" << std::endl;
    std::cout << "Land generation time:     " << std::fixed << std::setprecision(3) << time_land_generation << " ms" << std::endl;
    std::cout << "Number of iterations:     " << it << std::endl;
    std::cout << "Ant advance time:         " << std::fixed << std::setprecision(3) << time_ant_advance << " ms" << std::endl;
    std::cout << "Evaporation time:         " << std::fixed << std::setprecision(3) << time_evaporation << " ms" << std::endl;
    std::cout << "Pheromone update time:    " << std::fixed << std::setprecision(3) << time_pheromone_update << " ms" << std::endl;
    std::cout << "Rendering time:           " << std::fixed << std::setprecision(3) << time_rendering << " ms" << std::endl;
    std::cout << "Total simulation time:    " << std::fixed << std::setprecision(3) << time_total << " ms" << std::endl;
    std::cout << "Time per iteration:       " << std::fixed << std::setprecision(3) << (time_ant_advance + time_evaporation + time_pheromone_update) / it << " ms" << std::endl;
    std::cout << "Food collected:           " << food_quantity << std::endl;
    std::cout << "========================================================\n" << std::endl;
    
    SDL_Quit();
    return 0;
}
// ant_vectorized.hpp
#ifndef _ANT_VECTORIZED_HPP_
# define _ANT_VECTORIZED_HPP_
# include <vector>
# include <cstddef>
# include "pheronome.hpp"
# include "fractal_land.hpp"
# include "basic_types.hpp"

/**
 * @brief Vectorized ant colony representation
 * @details Stores ant data in separate arrays for better cache locality and easier parallelization
 *          Instead of having a vector of ant objects, we have separate vectors for:
 *          - positions (x, y coordinates)
 *          - states (loaded/unloaded)
 *          - seeds (random generator seeds)
 */
class ant_vectorized
{
public:
    /**
     * An ant can be in two states: unloaded (0) or loaded (1)
     */
    enum state { unloaded = 0, loaded = 1 };

    ant_vectorized(std::size_t nb_ants);
    ant_vectorized(const ant_vectorized&) = delete;
    ant_vectorized(ant_vectorized&&) = default;
    ~ant_vectorized() = default;

    // Initialize ants with random positions
    void initialize(const fractal_land& land, std::size_t seed);
    
    // Initialize a single ant
    void initialize_ant(std::size_t idx, const position_t& pos, std::size_t seed);

    // Set exploration coefficient (shared by all ants)
    static void set_exploration_coef(double eps) { m_eps = eps; }

    // Advance all ants by one time step
    void advance_all(pheronome& phen, const fractal_land& land,
                     const position_t& pos_food, const position_t& pos_nest, 
                     std::size_t& cpteur_food);

    // Get number of ants
    std::size_t size() const { return m_positions.size(); }

    // Access functions for debugging/rendering
    const std::vector<position_t>& get_positions() const { return m_positions; }
    const std::vector<int>& get_states() const { return m_states; }

private:
    static double m_eps; // Exploration coefficient shared by all ants
    
    std::vector<position_t> m_positions;  // x, y coordinates for each ant
    std::vector<int> m_states;            // loaded/unloaded state for each ant
    std::vector<std::size_t> m_seeds;     // random seed for each ant
};

#endif

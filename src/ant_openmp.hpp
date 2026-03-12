// ant_openmp.hpp
#ifndef _ANT_OPENMP_HPP_
# define _ANT_OPENMP_HPP_
# include <vector>
# include <cstddef>
# include "pheronome.hpp"
# include "fractal_land.hpp"
# include "basic_types.hpp"

/**
 * @brief OpenMP parallelized vectorized ant colony representation
 * @details Similar to ant_vectorized but with OpenMP parallelization
 */
class ant_openmp
{
public:
    enum state { unloaded = 0, loaded = 1 };

    ant_openmp(std::size_t nb_ants);
    ant_openmp(const ant_openmp&) = delete;
    ant_openmp(ant_openmp&&) = default;
    ~ant_openmp() = default;

    void initialize(const fractal_land& land, std::size_t seed);
    void initialize_ant(std::size_t idx, const position_t& pos, std::size_t seed);

    static void set_exploration_coef(double eps) { m_eps = eps; }

    void advance_all(pheronome& phen, const fractal_land& land,
                     const position_t& pos_food, const position_t& pos_nest, 
                     std::size_t& cpteur_food);

    std::size_t size() const { return m_positions.size(); }
    const std::vector<position_t>& get_positions() const { return m_positions; }
    const std::vector<int>& get_states() const { return m_states; }

private:
    static double m_eps;
    
    std::vector<position_t> m_positions;
    std::vector<int> m_states;
    std::vector<std::size_t> m_seeds;
};

#endif

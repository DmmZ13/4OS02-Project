// ant_vectorized.cpp
#include "ant_vectorized.hpp"
#include <algorithm>
#include "rand_generator.hpp"

double ant_vectorized::m_eps = 0.;

ant_vectorized::ant_vectorized(std::size_t nb_ants)
    : m_positions(nb_ants), m_states(nb_ants, unloaded), m_seeds(nb_ants)
{
}

void ant_vectorized::initialize(const fractal_land& land, std::size_t seed)
{
    for (std::size_t i = 0; i < m_positions.size(); ++i) {
        std::size_t x = rand_int32(0, land.dimensions() - 1, seed);
        std::size_t y = rand_int32(0, land.dimensions() - 1, seed);
        m_positions[i] = position_t{static_cast<int>(x), static_cast<int>(y)};
        m_seeds[i] = seed;
    }
}

void ant_vectorized::initialize_ant(std::size_t idx, const position_t& pos, std::size_t seed)
{
    if (idx >= m_positions.size()) return;
    m_positions[idx] = pos;
    m_states[idx] = unloaded;
    m_seeds[idx] = seed;
}

void ant_vectorized::advance_all(pheronome& phen, const fractal_land& land,
                                  const position_t& pos_food, const position_t& pos_nest,
                                  std::size_t& cpteur_food)
{
    for (std::size_t ant_idx = 0; ant_idx < m_positions.size(); ++ant_idx) {
        // Use lambda for random choices
        auto ant_choice = [&ant_idx, this]() mutable { 
            return rand_double(0., 1., this->m_seeds[ant_idx]); 
        };
        auto dir_choice = [&ant_idx, this]() mutable { 
            return rand_int32(1, 4, this->m_seeds[ant_idx]); 
        };

        double consumed_time = 0.;
        // Advance ant while it has movement time left
        while (consumed_time < 1.) {
            int ind_pher = (m_states[ant_idx] == loaded ? 1 : 0);
            double choix = ant_choice();
            position_t old_pos = m_positions[ant_idx];
            position_t new_pos = old_pos;

            double max_phen = std::max({
                phen(new_pos.x - 1, new_pos.y)[ind_pher],
                phen(new_pos.x + 1, new_pos.y)[ind_pher],
                phen(new_pos.x, new_pos.y - 1)[ind_pher],
                phen(new_pos.x, new_pos.y + 1)[ind_pher]
            });

            if ((choix > m_eps) || (max_phen <= 0.)) {
                // Random exploration
                do {
                    new_pos = old_pos;
                    int d = dir_choice();
                    if (d == 1) new_pos.x -= 1;
                    if (d == 2) new_pos.y -= 1;
                    if (d == 3) new_pos.x += 1;
                    if (d == 4) new_pos.y += 1;
                } while (phen[new_pos][ind_pher] == -1);
            } else {
                // Follow pheromone gradient
                if (phen(new_pos.x - 1, new_pos.y)[ind_pher] == max_phen)
                    new_pos.x -= 1;
                else if (phen(new_pos.x + 1, new_pos.y)[ind_pher] == max_phen)
                    new_pos.x += 1;
                else if (phen(new_pos.x, new_pos.y - 1)[ind_pher] == max_phen)
                    new_pos.y -= 1;
                else
                    new_pos.y += 1;
            }

            consumed_time += land(new_pos.x, new_pos.y);
            phen.mark_pheronome(new_pos);
            m_positions[ant_idx] = new_pos;

            // Check if at nest
            if (m_positions[ant_idx] == pos_nest) {
                if (m_states[ant_idx] == loaded) {
                    cpteur_food++;
                }
                m_states[ant_idx] = unloaded;
            }

            // Check if at food
            if (m_positions[ant_idx] == pos_food) {
                m_states[ant_idx] = loaded;
            }
        }
    }
}

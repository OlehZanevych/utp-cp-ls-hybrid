#ifndef ADAPTIVE_NEIGHBORHOOD_SELECTOR_H
#define ADAPTIVE_NEIGHBORHOOD_SELECTOR_H

#include <vector>
#include <unordered_map>
#include <random>

class AdaptiveNeighborhoodSelector {
    struct NeighborhoodStats {
        int attempts = 0;
        int improvements = 0;
        double avg_improvement = 0;
        double selection_probability = 0.25;  // Start with equal probability
    };

    std::vector<std::string> neighborhood_names = {"swap_rooms", "swap_times", "move_assignment", "chain_swap"};
    std::unordered_map<std::string, NeighborhoodStats> stats;
    std::mt19937 rng;

public:
    AdaptiveNeighborhoodSelector();

    std::string selectNeighborhood();
    void updateStats(const std::string& neighborhood, bool improved, double improvement);

private:
    void updateProbabilities();
};

#endif //ADAPTIVE_NEIGHBORHOOD_SELECTOR_H

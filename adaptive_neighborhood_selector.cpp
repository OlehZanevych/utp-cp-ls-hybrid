#include "adaptive_neighborhood_selector.h"

#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>

AdaptiveNeighborhoodSelector::AdaptiveNeighborhoodSelector() : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
    for (const auto& name : neighborhood_names) {
        stats[name] = NeighborhoodStats();
    }
}

std::string AdaptiveNeighborhoodSelector::selectNeighborhood() {
    std::vector<double> probs;
    for (const auto& name : neighborhood_names) {
        probs.push_back(stats[name].selection_probability);
    }

    std::discrete_distribution<> dist(probs.begin(), probs.end());
    return neighborhood_names[dist(rng)];
}

void AdaptiveNeighborhoodSelector::updateStats(const std::string& neighborhood, bool improved, double improvement) {
    auto& s = stats[neighborhood];
    s.attempts++;
    if (improved) {
        s.improvements++;
        s.avg_improvement = (s.avg_improvement * (s.improvements - 1) + improvement) / s.improvements;
    }

    // Update probabilities using adaptive pursuit algorithm
    updateProbabilities();
}

void AdaptiveNeighborhoodSelector::updateProbabilities() {
    // Calculate quality scores
    std::vector<double> scores;
    for (const auto& name : neighborhood_names) {
        double success_rate = stats[name].attempts > 0 ?
            (double)stats[name].improvements / stats[name].attempts : 0.5;
        double score = success_rate * (1 + stats[name].avg_improvement / 100);
        scores.push_back(score);
    }

    // Find best score
    double max_score = *std::max_element(scores.begin(), scores.end());

    // Update probabilities
    const double alpha = 0.1;  // Learning rate
    const double p_min = 0.05;  // Minimum probability

    for (size_t i = 0; i < neighborhood_names.size(); i++) {
        auto& prob = stats[neighborhood_names[i]].selection_probability;
        if (scores[i] == max_score) {
            prob = prob + alpha * (1 - prob);
        } else {
            prob = prob + alpha * (p_min - prob);
        }
    }
}
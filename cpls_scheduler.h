#ifndef CPLS_SCHEDULER_H
#define CPLS_SCHEDULER_H

#include <vector>
#include <random>

#include "structures.h"
#include "schedule.h"
#include "constraint_checker.h"
#include "adaptive_neighborhood_selector.h"


// Main CP-LS Hybrid Algorithm
class CPLSScheduler {
private:
    std::vector<Course> courses;
    std::vector<Room> rooms;
    std::vector<Lecturer> lecturers;
    std::vector<StudentGroup> groups;
    int days;
    int periods_per_day;
    std::mt19937 rng;
    ConstraintChecker checker;
    AdaptiveNeighborhoodSelector neighborhood_selector;

    // Memory structures for intensification
    std::vector<Schedule> elite_solutions;
    const size_t elite_size = 10;

public:
    CPLSScheduler(const std::vector<Course> &c, const std::vector<Room> &r,
                  const std::vector<Lecturer> &l, const std::vector<StudentGroup> &g,
                  int d, int p);

    // Initial solution using constraint propagation
    Schedule generateInitialSolution();

    // Local search with adaptive neighborhoods
    void localSearch(Schedule &schedule, int max_iterations);

    Schedule solve(int cp_iterations = 5, int ls_iterations = 10000);

private:
    void evaluateFitness(Schedule &schedule);

    bool swapRooms(Schedule &schedule);

    bool swapTimes(Schedule &schedule);

    bool moveAssignment(Schedule &schedule);

    bool chainSwap(Schedule &schedule);

    bool acceptWorse(double delta, double temperature);

    void perturbSolution(Schedule &schedule);

    void updateEliteSolutions(const Schedule &schedule);

    Schedule pathRelinking(const Schedule &source);
};

#endif //CPLS_SCHEDULER_H

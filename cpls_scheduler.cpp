#include <iostream>

#include "cpls_scheduler.h"

CPLSScheduler::CPLSScheduler(const std::vector<Course> &c, const std::vector<Room> &r,
                             const std::vector<Lecturer> &l, const std::vector<StudentGroup> &g,
                             int d, int p)
    : courses(c), rooms(r), lecturers(l), groups(g), days(d), periods_per_day(p),
      rng(std::chrono::steady_clock::now().time_since_epoch().count()),
      checker(c, r, l, g) {
}

// Initial solution using constraint propagation
Schedule CPLSScheduler::generateInitialSolution() {
    Schedule schedule;
    std::vector<int> course_indices(courses.size());
    std::iota(course_indices.begin(), course_indices.end(), 0);

    // Sort courses by difficulty (more groups and fewer valid assignments = harder)
    std::sort(course_indices.begin(), course_indices.end(), [this](int a, int b) {
        int students_a = courses[a].getTotalStudents(groups);
        int students_b = courses[b].getTotalStudents(groups);
        int groups_a = courses[a].group_ids.size();
        int groups_b = courses[b].group_ids.size();

        // Prioritize courses with more groups and more students
        return groups_a * students_a > groups_b * students_b;
    });

    for (int course_idx: course_indices) {
        const Course &course = courses[course_idx];

        for (int meeting = 0; meeting < course.weekly_meetings; meeting++) {
            std::vector<Assignment> valid_assignments;

            // Generate all possible assignments
            for (int r = 0; r < rooms.size(); r++) {
                for (int d = 0; d < days; d++) {
                    for (int p = 0; p <= periods_per_day - course.duration; p++) {
                        Assignment a(course_idx, r, TimeSlot(d, p));
                        if (checker.isValidAssignment(a, schedule)) {
                            valid_assignments.push_back(a);
                        }
                    }
                }
            }

            if (valid_assignments.empty()) {
                std::cerr << "No valid assignment for course " << course.name << std::endl;
                continue;
            }

            // Use value ordering heuristic: avoid undesirable slots
            std::sort(valid_assignments.begin(), valid_assignments.end(),
                      [this, &course](const Assignment &a, const Assignment &b) {
                          // Calculate desirability score
                          int score_a = 0, score_b = 0;

                          // Check lecturer preferences
                          if (lecturers[course.lecturer_id].isUndesirableSlot(a.time_slot)) score_a += 100;
                          if (lecturers[course.lecturer_id].isUndesirableSlot(b.time_slot)) score_b += 100;

                          // Check group preferences
                          for (int gid: course.group_ids) {
                              if (groups[gid].isUndesirableSlot(a.time_slot)) score_a += 50;
                              if (groups[gid].isUndesirableSlot(b.time_slot)) score_b += 50;
                          }

                          // Prefer morning slots
                          score_a += a.time_slot.period * 5;
                          score_b += b.time_slot.period * 5;

                          // Prefer larger rooms (more flexibility)
                          score_a -= rooms[a.room_id].capacity;
                          score_b -= rooms[b.room_id].capacity;

                          return score_a < score_b;
                      });

            schedule.addAssignment(valid_assignments[0]);
        }
    }

    evaluateFitness(schedule);
    return schedule;
}

// Local search with adaptive neighborhoods
void CPLSScheduler::localSearch(Schedule &schedule, int max_iterations) {
    Schedule best_schedule = schedule;
    int no_improvement_count = 0;
    const int max_no_improvement = 100;

    for (int iter = 0; iter < max_iterations; iter++) {
        std::string neighborhood = neighborhood_selector.selectNeighborhood();
        Schedule neighbor = schedule;

        bool valid = false;
        if (neighborhood == "swap_rooms") {
            valid = swapRooms(neighbor);
        } else if (neighborhood == "swap_times") {
            valid = swapTimes(neighbor);
        } else if (neighborhood == "move_assignment") {
            valid = moveAssignment(neighbor);
        } else if (neighborhood == "chain_swap") {
            valid = chainSwap(neighbor);
        }

        if (valid) {
            evaluateFitness(neighbor);
            double improvement = schedule.fitness - neighbor.fitness;
            bool improved = improvement > 0;

            // Simulated annealing acceptance
            double temperature = 100.0 * (1.0 - (double) iter / max_iterations);
            if (improved || acceptWorse(improvement, temperature)) {
                schedule = neighbor;
                if (improved) {
                    no_improvement_count = 0;
                    if (schedule.fitness < best_schedule.fitness) {
                        best_schedule = schedule;
                        updateEliteSolutions(schedule);
                    }
                }
            } else {
                no_improvement_count++;
            }

            neighborhood_selector.updateStats(neighborhood, improved, std::abs(improvement));
        }

        // Diversification
        if (no_improvement_count >= max_no_improvement) {
            perturbSolution(schedule);
            no_improvement_count = 0;
        }

        // Path relinking every 1000 iterations
        if (iter % 1000 == 0 && !elite_solutions.empty()) {
            Schedule relinked = pathRelinking(schedule);
            if (relinked.fitness < schedule.fitness) {
                schedule = relinked;
            }
        }
    }

    schedule = best_schedule;
}

Schedule CPLSScheduler::solve(int cp_iterations, int ls_iterations) {
    Schedule best_schedule;
    best_schedule.fitness = std::numeric_limits<double>::max();

    // Multi-start approach
    for (int i = 0; i < cp_iterations; i++) {
        std::cout << "CP iteration " << i + 1 << "/" << cp_iterations << std::endl;

        Schedule current = generateInitialSolution();
        std::cout << "Initial solution - Violations: " << current.hard_violations
                 << ", Fitness: " << current.fitness << std::endl;

        localSearch(current, ls_iterations);
        std::cout << "After LS - Violations: " << current.hard_violations
                 << ", Fitness: " << current.fitness << std::endl;

        if (current.fitness < best_schedule.fitness) {
            best_schedule = current;
        }

        // Shuffle courses for next iteration
        std::shuffle(courses.begin(), courses.end(), rng);
    }

    return best_schedule;
}

void CPLSScheduler::evaluateFitness(Schedule& schedule) {
    schedule.hard_violations = 0;
    schedule.soft_violations = 0;

    // Count hard constraint violations (should be 0 for valid solutions)
    for (const Assignment& a : schedule.assignments) {
        if (!checker.isValidAssignment(a, schedule)) {
            schedule.hard_violations++;
        }
    }

    // Soft constraints
    double soft_penalty = checker.evaluateSoftConstraints(schedule);
    schedule.soft_violations = (int)soft_penalty;

    // Combined fitness (hard constraints have much higher weight)
    schedule.fitness = schedule.hard_violations * 1000 + soft_penalty;
}

bool CPLSScheduler::swapRooms(Schedule& schedule) {
    if (schedule.assignments.size() < 2) return false;

    std::uniform_int_distribution<> dist(0, schedule.assignments.size() - 1);
    int idx1 = dist(rng);
    int idx2 = dist(rng);

    if (idx1 == idx2) return false;

    std::swap(schedule.assignments[idx1].room_id, schedule.assignments[idx2].room_id);

    // Check validity
    if (!checker.isValidAssignment(schedule.assignments[idx1], schedule) ||
        !checker.isValidAssignment(schedule.assignments[idx2], schedule)) {
        std::swap(schedule.assignments[idx1].room_id, schedule.assignments[idx2].room_id);
        return false;
        }

    return true;
}

bool CPLSScheduler::swapTimes(Schedule& schedule) {
    if (schedule.assignments.size() < 2) return false;

    std::uniform_int_distribution<> dist(0, schedule.assignments.size() - 1);
    int idx1 = dist(rng);
    int idx2 = dist(rng);

    if (idx1 == idx2) return false;

    std::swap(schedule.assignments[idx1].time_slot, schedule.assignments[idx2].time_slot);

    // Check validity
    if (!checker.isValidAssignment(schedule.assignments[idx1], schedule) ||
        !checker.isValidAssignment(schedule.assignments[idx2], schedule)) {
        std::swap(schedule.assignments[idx1].time_slot, schedule.assignments[idx2].time_slot);
        return false;
        }

    return true;
}

bool CPLSScheduler::moveAssignment(Schedule& schedule) {
    if (schedule.assignments.empty()) return false;

    std::uniform_int_distribution<> idx_dist(0, schedule.assignments.size() - 1);
    int idx = idx_dist(rng);

    Assignment old_assignment = schedule.assignments[idx];

    std::uniform_int_distribution<> room_dist(0, rooms.size() - 1);
    std::uniform_int_distribution<> day_dist(0, days - 1);
    std::uniform_int_distribution<> period_dist(0, periods_per_day - courses[old_assignment.course_id].duration);

    schedule.assignments[idx].room_id = room_dist(rng);
    schedule.assignments[idx].time_slot = TimeSlot(day_dist(rng), period_dist(rng));

    if (!checker.isValidAssignment(schedule.assignments[idx], schedule)) {
        schedule.assignments[idx] = old_assignment;
        return false;
    }

    return true;
}

bool CPLSScheduler::chainSwap(Schedule& schedule) {
    if (schedule.assignments.size() < 3) return false;

    // Select chain of 3-4 assignments
    std::uniform_int_distribution<> chain_size_dist(3, std::min(4, (int)schedule.assignments.size()));
    int chain_size = chain_size_dist(rng);

    std::vector<int> chain_indices;
    std::uniform_int_distribution<> idx_dist(0, schedule.assignments.size() - 1);
    std::unordered_set<int> used;

    while (chain_indices.size() < chain_size) {
        int idx = idx_dist(rng);
        if (used.find(idx) == used.end()) {
            chain_indices.push_back(idx);
            used.insert(idx);
        }
    }

    // Save original time slots
    std::vector<TimeSlot> original_slots;
    for (int idx : chain_indices) {
        original_slots.push_back(schedule.assignments[idx].time_slot);
    }

    // Rotate time slots
    for (size_t i = 0; i < chain_indices.size(); i++) {
        schedule.assignments[chain_indices[i]].time_slot =
            original_slots[(i + 1) % chain_indices.size()];
    }

    // Check validity
    for (int idx : chain_indices) {
        if (!checker.isValidAssignment(schedule.assignments[idx], schedule)) {
            // Restore
            for (size_t i = 0; i < chain_indices.size(); i++) {
                schedule.assignments[chain_indices[i]].time_slot = original_slots[i];
            }
            return false;
        }
    }

    return true;
}

bool CPLSScheduler::acceptWorse(double delta, double temperature) {
    if (temperature <= 0) return false;
    double probability = std::exp(-std::abs(delta) / temperature);
    std::uniform_real_distribution<> dist(0, 1);
    return dist(rng) < probability;
}

void CPLSScheduler::perturbSolution(Schedule& schedule) {
    // Strong perturbation: randomly reassign 10% of assignments
    int perturbation_size = std::max(1, (int)(schedule.assignments.size() * 0.1));
    std::uniform_int_distribution<> idx_dist(0, schedule.assignments.size() - 1);

    for (int i = 0; i < perturbation_size; i++) {
        moveAssignment(schedule);
    }

    evaluateFitness(schedule);
}

void CPLSScheduler::updateEliteSolutions(const Schedule& schedule) {
    if (elite_solutions.size() < elite_size) {
        elite_solutions.push_back(schedule);
    } else {
        // Replace worst elite solution if current is better
        auto worst_it = std::max_element(elite_solutions.begin(), elite_solutions.end(),
            [](const Schedule& a, const Schedule& b) { return a.fitness < b.fitness; });

        if (schedule.fitness < worst_it->fitness) {
            *worst_it = schedule;
        }
    }
}

Schedule CPLSScheduler::pathRelinking(const Schedule& source) {
    if (elite_solutions.empty()) return source;

    // Select random elite solution as target
    std::uniform_int_distribution<> dist(0, elite_solutions.size() - 1);
    const Schedule& target = elite_solutions[dist(rng)];

    Schedule current = source;
    Schedule best = source;

    // Find differences
    for (size_t i = 0; i < std::min(source.assignments.size(), target.assignments.size()); i++) {
        if (source.assignments[i].room_id != target.assignments[i].room_id ||
            !(source.assignments[i].time_slot == target.assignments[i].time_slot)) {

            // Move towards target
            Assignment old = current.assignments[i];
            current.assignments[i] = target.assignments[i];

            if (checker.isValidAssignment(current.assignments[i], current)) {
                evaluateFitness(current);
                if (current.fitness < best.fitness) {
                    best = current;
                }
            } else {
                current.assignments[i] = old;
            }
            }
    }

    return best;
}
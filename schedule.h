#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <vector>
#include <unordered_map>
#include "structures.h"

class Schedule {
public:
    std::vector<Assignment> assignments;
    std::unordered_map<int, std::vector<int>> course_assignments;  // course_id -> indices in `assignments`
    double fitness;
    int hard_violations;
    int soft_violations;

    Schedule();

    void addAssignment(const Assignment& a);
    void clear();
};

#endif //SCHEDULE_H

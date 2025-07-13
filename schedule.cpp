#include "schedule.h"

Schedule::Schedule()
    : fitness(0), hard_violations(0), soft_violations(0) {}

void Schedule::addAssignment(const Assignment& a) {
    assignments.push_back(a);
    course_assignments[a.course_id].push_back(assignments.size() - 1);
}

void Schedule::clear() {
    assignments.clear();
    course_assignments.clear();
    fitness = 0;
    hard_violations = 0;
    soft_violations = 0;
}
#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "structures.h"
#include "schedule.h"

class ConstraintChecker {
private:
    const std::vector<Course>& courses;
    const std::vector<Room>& rooms;
    const std::vector<Lecturer>& lecturers;
    const std::vector<StudentGroup>& groups;
    std::unordered_map<std::string, bool> constraint_cache;

    std::string getCacheKey(int course_id, int room_id, const TimeSlot& ts) const;

public:
    ConstraintChecker(const std::vector<Course>& c, const std::vector<Room>& r,
                     const std::vector<Lecturer>& l, const std::vector<StudentGroup>& g);

    void clearCache();
    bool isValidAssignment(const Assignment& a, const Schedule& schedule);
    double evaluateSoftConstraints(const Schedule& schedule);
};
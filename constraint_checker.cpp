#include "constraint_checker.h"
#include <algorithm>
#include <unordered_set>

ConstraintChecker::ConstraintChecker(const std::vector<Course>& c, const std::vector<Room>& r,
                                     const std::vector<Lecturer>& l, const std::vector<StudentGroup>& g)
    : courses(c), rooms(r), lecturers(l), groups(g) {}

std::string ConstraintChecker::getCacheKey(int course_id, int room_id, const TimeSlot& ts) const {
    return std::to_string(course_id) + "_" + std::to_string(room_id) + "_" +
           std::to_string(ts.day) + "_" + std::to_string(ts.period);
}

void ConstraintChecker::clearCache() { 
    constraint_cache.clear(); 
}

bool ConstraintChecker::isValidAssignment(const Assignment& a, const Schedule& schedule) {
    std::string key = getCacheKey(a.course_id, a.room_id, a.time_slot);
    auto it = constraint_cache.find(key);
    if (it != constraint_cache.end()) return it->second;

    const Course& course = courses[a.course_id];
    const Room& room = rooms[a.room_id];

    // Room capacity constraint
    int total_students = course.getTotalStudents(groups);
    if (room.capacity < total_students) {
        constraint_cache[key] = false;
        return false;
    }

    // Room features constraint
    if (!room.hasFeatures(course.required_features)) {
        constraint_cache[key] = false;
        return false;
    }

    // Time slot conflicts
    for (const Assignment& other : schedule.assignments) {
        if (other.course_id == a.course_id) continue;

        // Room conflict
        if (other.room_id == a.room_id && other.time_slot == a.time_slot) {
            constraint_cache[key] = false;
            return false;
        }

        // Lecturer conflict
        if (courses[other.course_id].lecturer_id == course.lecturer_id &&
            other.time_slot == a.time_slot) {
            constraint_cache[key] = false;
            return false;
        }

        // Student group conflicts - check if any group overlaps
        for (int group_id : course.group_ids) {
            for (int other_group_id : courses[other.course_id].group_ids) {
                if (group_id == other_group_id && other.time_slot == a.time_slot) {
                    constraint_cache[key] = false;
                    return false;
                }
            }
        }
    }

    constraint_cache[key] = true;
    return true;
}

double ConstraintChecker::evaluateSoftConstraints(const Schedule& schedule) {
    double penalty = 0;

    // 1. Minimize gaps for lecturers
    std::unordered_map<int, std::vector<TimeSlot>> lecturer_slots;
    for (const Assignment& a : schedule.assignments) {
        lecturer_slots[courses[a.course_id].lecturer_id].push_back(a.time_slot);
    }

    for (auto& [lecturer_id, slots] : lecturer_slots) {
        std::sort(slots.begin(), slots.end());

        // Penalty for gaps between classes on the same day
        for (size_t i = 1; i < slots.size(); i++) {
            if (slots[i].day == slots[i-1].day) {
                int gap = slots[i].period - slots[i-1].period - 1;
                penalty += gap * 10;  // 10 points per gap period for lecturers
            }
        }
    }

    // 2. Minimize gaps for student groups
    std::unordered_map<int, std::vector<TimeSlot>> group_slots;
    for (const Assignment& a : schedule.assignments) {
        for (int group_id : courses[a.course_id].group_ids) {
            group_slots[group_id].push_back(a.time_slot);
        }
    }

    for (auto& [group_id, slots] : group_slots) {
        std::sort(slots.begin(), slots.end());

        // Penalty for gaps between classes on the same day
        for (size_t i = 1; i < slots.size(); i++) {
            if (slots[i].day == slots[i-1].day) {
                int gap = slots[i].period - slots[i-1].period - 1;
                penalty += gap * 8;  // 8 points per gap period for students
            }
        }
    }

    // 3. Penalties for undesirable time slots
    for (const Assignment& a : schedule.assignments) {
        const Course& course = courses[a.course_id];

        // Check lecturer preferences
        const Lecturer& lecturer = lecturers[course.lecturer_id];
        if (lecturer.isUndesirableSlot(a.time_slot)) {
            penalty += lecturer.undesirable_penalty;
        }

        // Check group preferences
        for (int group_id : course.group_ids) {
            const StudentGroup& group = groups[group_id];
            if (group.isUndesirableSlot(a.time_slot)) {
                penalty += group.undesirable_penalty;
            }
        }
    }

    // 4. Prefer morning slots (general preference)
    for (const Assignment& a : schedule.assignments) {
        if (a.time_slot.period > 4) {  // Afternoon penalty
            penalty += 3;  // Reduced since we now have specific undesirable slots
        }
    }

    // 5. Distribute course meetings throughout the week
    for (const auto& [course_id, indices] : schedule.course_assignments) {
        std::unordered_set<int> days;
        for (int idx : indices) {
            days.insert(schedule.assignments[idx].time_slot.day);
        }

        // Penalty for same-day meetings
        if (days.size() < indices.size()) {
            penalty += (indices.size() - days.size()) * 20;
        }
    }

    // 6. Penalty for too many classes per day for groups
    for (auto& [group_id, slots] : group_slots) {
        std::unordered_map<int, int> classes_per_day;
        for (const TimeSlot& ts : slots) {
            classes_per_day[ts.day]++;
        }

        for (auto& [day, count] : classes_per_day) {
            if (count > 4) {  // More than 4 classes per day is undesirable
                penalty += (count - 4) * 15;
            }
        }
    }

    return penalty;
}
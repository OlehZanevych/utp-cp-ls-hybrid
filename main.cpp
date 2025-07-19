#include <iostream>
#include <vector>
#include <chrono>

#include "schedule.h"
#include "scheduling_data_generator.h"
#include "cpls_scheduler.h"
#include "scheduling_data_serialization.h"

// Example usage
int main() {
    std::string file_name = "../data/scheduling-data-1.json";

    SchedulingData data;
    loadFromFile(data, file_name);

    // Create scheduler
    CPLSScheduler scheduler(data.courses, data.rooms, data.lecturers, data.groups, 5, 8);  // 5 days, 8 periods per day

    std::cout << "Starting CP-LS Hybrid Algorithm for University Scheduling\n";
    std::cout << "Courses: " << data.courses.size() << ", Rooms: " << data.rooms.size()
              << ", Lecturers: " << data.lecturers.size() << ", Groups: " << data.groups.size() << "\n\n";

    auto start = std::chrono::high_resolution_clock::now();
    Schedule solution = scheduler.solve(3, 5000);  // 3 CP iterations, 5000 LS iterations
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\n=== Final Solution ===\n";
    std::cout << "Hard violations: " << solution.hard_violations << "\n";
    std::cout << "Soft violations: " << solution.soft_violations << "\n";
    std::cout << "Total fitness: " << solution.fitness << "\n";
    std::cout << "Time taken: " << duration.count() << " ms\n\n";

    // // Print schedule by group
    // std::cout << "\n=== Schedule by Student Group ===\n";
    // for (const StudentGroup& group : data.groups) {
    //     std::cout << "\n" << group.name << " (" << group.size << " students):\n";
    //     std::cout << "Course\t\t\tRoom\tDay\tPeriod\tLecturer\n";
    //     std::cout << "--------------------------------------------------------\n";
    //
    //     std::vector<std::pair<TimeSlot, std::string>> group_schedule;
    //
    //     for (const Assignment& a : solution.assignments) {
    //         const Course& course = data.courses[a.course_id];
    //         bool has_group = false;
    //         for (int gid : course.group_ids) {
    //             if (gid == group.id) {
    //                 has_group = true;
    //                 break;
    //             }
    //         }
    //
    //         if (has_group) {
    //             std::string entry = course.name + "\t";
    //             if (course.name.length() < 8) entry += "\t";
    //             if (course.name.length() < 16) entry += "\t";
    //             entry += data.rooms[a.room_id].name + "\t";
    //             entry += "Day " + std::to_string(a.time_slot.day + 1) + "\t";
    //             entry += "P" + std::to_string(a.time_slot.period + 1) + "-P" +
    //                     std::to_string(a.time_slot.period + course.duration) + "\t";
    //             entry += data.lecturers[course.lecturer_id].name;
    //
    //             group_schedule.push_back({a.time_slot, entry});
    //         }
    //     }
    //
    //     // Sort by day and period
    //     std::sort(group_schedule.begin(), group_schedule.end());
    //
    //     for (const auto& [ts, entry] : group_schedule) {
    //         std::cout << entry << "\n";
    //     }
    // }
    //
    // // Print schedule by lecturer
    // std::cout << "\n=== Schedule by Lecturer ===\n";
    // for (const Lecturer& lecturer : data.lecturers) {
    //     std::cout << "\n" << lecturer.name << ":\n";
    //     std::cout << "Course\t\t\tRoom\tDay\tPeriod\tGroups\n";
    //     std::cout << "--------------------------------------------------------\n";
    //
    //     std::vector<std::pair<TimeSlot, std::string>> lecturer_schedule;
    //
    //     for (const Assignment& a : solution.assignments) {
    //         const Course& course = data.courses[a.course_id];
    //         if (course.lecturer_id == lecturer.id) {
    //             std::string entry = course.name + "\t";
    //             if (course.name.length() < 8) entry += "\t";
    //             if (course.name.length() < 16) entry += "\t";
    //             entry += data.rooms[a.room_id].name + "\t";
    //             entry += "Day " + std::to_string(a.time_slot.day + 1) + "\t";
    //             entry += "P" + std::to_string(a.time_slot.period + 1) + "-P" +
    //                     std::to_string(a.time_slot.period + course.duration) + "\t";
    //
    //             // List groups
    //             for (size_t i = 0; i < course.group_ids.size(); i++) {
    //                 entry += data.groups[course.group_ids[i]].name;
    //                 if (i < course.group_ids.size() - 1) entry += ", ";
    //             }
    //
    //             lecturer_schedule.push_back({a.time_slot, entry});
    //         }
    //     }
    //
    //     // Sort by day and period
    //     std::sort(lecturer_schedule.begin(), lecturer_schedule.end());
    //
    //     for (const auto& [ts, entry] : lecturer_schedule) {
    //         std::cout << entry << "\n";
    //     }
    // }
    //
    // // Analyze constraint satisfaction
    // std::cout << "\n=== Constraint Analysis ===\n";
    //
    // // Check undesirable slots
    // int lecturer_undesirable_count = 0;
    // int group_undesirable_count = 0;
    //
    // for (const Assignment& a : solution.assignments) {
    //     const Course& course = data.courses[a.course_id];
    //
    //     if (data.lecturers[course.lecturer_id].isUndesirableSlot(a.time_slot)) {
    //         lecturer_undesirable_count++;
    //         std::cout << "Lecturer " << data.lecturers[course.lecturer_id].name
    //                  << " has class in undesirable slot: Day " << a.time_slot.day + 1
    //                  << " Period " << a.time_slot.period + 1 << "\n";
    //     }
    //
    //     for (int gid : course.group_ids) {
    //         if (data.groups[gid].isUndesirableSlot(a.time_slot)) {
    //             group_undesirable_count++;
    //             std::cout << "Group " << data.groups[gid].name
    //                      << " has class in undesirable slot: Day " << a.time_slot.day + 1
    //                      << " Period " << a.time_slot.period + 1 << "\n";
    //         }
    //     }
    // }
    //
    // std::cout << "\nTotal undesirable slots for lecturers: " << lecturer_undesirable_count << "\n";
    // std::cout << "Total undesirable slots for groups: " << group_undesirable_count << "\n";

    return 0;
}
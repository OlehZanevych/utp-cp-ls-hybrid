#ifndef SCHEDULING_JSON_H
#define SCHEDULING_JSON_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include "structures.h"
#include "schedule.h"

using json = nlohmann::json;

// JSON conversion functions for each structure

// TimeSlot serialization
void to_json(json& j, const TimeSlot& ts) {
    j = json{{"day", ts.day}, {"period", ts.period}};
}

void from_json(const json& j, TimeSlot& ts) {
    j.at("day").get_to(ts.day);
    j.at("period").get_to(ts.period);
}

// Lecturer serialization
void to_json(json& j, const Lecturer& lecturer) {
    json undesirable_slots_array = json::array();
    for (const auto& slot : lecturer.undesirable_slots) {
        undesirable_slots_array.push_back(slot);
    }

    j = json{
        {"id", lecturer.id},
        {"name", lecturer.name},
        {"undesirable_slots", undesirable_slots_array},
        {"undesirable_penalty", lecturer.undesirable_penalty}
    };
}

void from_json(const json& j, Lecturer& lecturer) {
    // Create a new lecturer with required constructor parameters
    int id = j.at("id").get<int>();
    std::string name = j.at("name").get<std::string>();
    lecturer = Lecturer(id, name);

    // Set optional fields
    lecturer.undesirable_penalty = j.at("undesirable_penalty").get<double>();

    // Clear and populate undesirable_slots
    lecturer.undesirable_slots.clear();
    for (const auto& slot_json : j.at("undesirable_slots")) {
        TimeSlot slot = slot_json.get<TimeSlot>();
        lecturer.undesirable_slots.insert(slot);
    }
}

// StudentGroup serialization
void to_json(json& j, const StudentGroup& group) {
    json undesirable_slots_array = json::array();
    for (const auto& slot : group.undesirable_slots) {
        undesirable_slots_array.push_back(slot);
    }

    j = json{
        {"id", group.id},
        {"name", group.name},
        {"size", group.size},
        {"undesirable_slots", undesirable_slots_array},
        {"undesirable_penalty", group.undesirable_penalty}
    };
}

void from_json(const json& j, StudentGroup& group) {
    // Create a new group with required constructor parameters
    int id = j.at("id").get<int>();
    std::string name = j.at("name").get<std::string>();
    int size = j.at("size").get<int>();
    group = StudentGroup(id, name, size);

    // Set optional fields
    group.undesirable_penalty = j.at("undesirable_penalty").get<double>();

    // Clear and populate undesirable_slots
    group.undesirable_slots.clear();
    for (const auto& slot_json : j.at("undesirable_slots")) {
        TimeSlot slot = slot_json.get<TimeSlot>();
        group.undesirable_slots.insert(slot);
    }
}

// Course serialization
void to_json(json& j, const Course& course) {
    j = json{
        {"id", course.id},
        {"name", course.name},
        {"lecturer_id", course.lecturer_id},
        {"group_ids", course.group_ids},
        {"duration", course.duration},
        {"required_features", course.required_features},
        {"weekly_meetings", course.weekly_meetings}
    };
}

void from_json(const json& j, Course& course) {
    // Create a new course with required constructor parameters
    int id = j.at("id").get<int>();
    std::string name = j.at("name").get<std::string>();
    int lecturer_id = j.at("lecturer_id").get<int>();
    int duration = j.at("duration").get<int>();
    int weekly_meetings = j.at("weekly_meetings").get<int>();

    course = Course(id, name, lecturer_id, duration, weekly_meetings);

    // Set vectors
    course.group_ids = j.at("group_ids").get<std::vector<int>>();
    course.required_features = j.at("required_features").get<std::vector<int>>();
}

// Room serialization
void to_json(json& j, const Room& room) {
    json features_array = json::array();
    for (const auto& feature : room.features) {
        features_array.push_back(feature);
    }

    j = json{
        {"id", room.id},
        {"name", room.name},
        {"capacity", room.capacity},
        {"features", features_array}
    };
}

void from_json(const json& j, Room& room) {
    // Create a new room with required constructor parameters
    int id = j.at("id").get<int>();
    std::string name = j.at("name").get<std::string>();
    int capacity = j.at("capacity").get<int>();

    room = Room(id, name, capacity);

    // Clear and populate features
    room.features.clear();
    for (const auto& feature : j.at("features")) {
        room.features.insert(feature.get<int>());
    }
}

// Assignment serialization
void to_json(json& j, const Assignment& assignment) {
    j = json{
        {"course_id", assignment.course_id},
        {"room_id", assignment.room_id},
        {"time_slot", assignment.time_slot}
    };
}

void from_json(const json& j, Assignment& assignment) {
    assignment.course_id = j.at("course_id").get<int>();
    assignment.room_id = j.at("room_id").get<int>();
    assignment.time_slot = j.at("time_slot").get<TimeSlot>();
}

// SchedulingData serialization
void to_json(json& j, const SchedulingData& data) {
    j = json{
        {"lecturers", data.lecturers},
        {"groups", data.groups},
        {"rooms", data.rooms},
        {"courses", data.courses}
    };
}

void from_json(const json& j, SchedulingData& data) {
    data.lecturers = j.at("lecturers").get<std::vector<Lecturer>>();
    data.groups = j.at("groups").get<std::vector<StudentGroup>>();
    data.rooms = j.at("rooms").get<std::vector<Room>>();
    data.courses = j.at("courses").get<std::vector<Course>>();
}

// // Utility class for JSON file operations
// class SchedulingDataSerializer {
// public:
//     // Save SchedulingData to JSON file
//     static bool saveToFile(const SchedulingData& data, const std::string& filename) {
//         try {
//             json j = data;
//             std::ofstream file(filename);
//             if (!file.is_open()) {
//                 std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
//                 return false;
//             }
//             file << j.dump(4); // Pretty print with 4 spaces indentation
//             file.close();
//             std::cout << "Successfully saved scheduling data to " << filename << std::endl;
//             return true;
//         } catch (const std::exception& e) {
//             std::cerr << "Error saving to JSON: " << e.what() << std::endl;
//             return false;
//         }
//     }
//
//     // Load SchedulingData from JSON file
//     static bool loadFromFile(SchedulingData& data, const std::string& filename) {
//         try {
//             std::ifstream file(filename);
//             if (!file.is_open()) {
//                 std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
//                 return false;
//             }
//             json j;
//             file >> j;
//             file.close();
//             data = j.get<SchedulingData>();
//             std::cout << "Successfully loaded scheduling data from " << filename << std::endl;
//             return true;
//         } catch (const std::exception& e) {
//             std::cerr << "Error loading from JSON: " << e.what() << std::endl;
//             return false;
//         }
//     }
//
//     // Convert SchedulingData to JSON string
//     static std::string toJsonString(const SchedulingData& data, bool pretty = true) {
//         try {
//             json j = data;
//             return pretty ? j.dump(4) : j.dump();
//         } catch (const std::exception& e) {
//             std::cerr << "Error converting to JSON string: " << e.what() << std::endl;
//             return "";
//         }
//     }
//
//     // Parse SchedulingData from JSON string
//     static bool fromJsonString(SchedulingData& data, const std::string& jsonStr) {
//         try {
//             json j = json::parse(jsonStr);
//             data = j.get<SchedulingData>();
//             return true;
//         } catch (const std::exception& e) {
//             std::cerr << "Error parsing JSON string: " << e.what() << std::endl;
//             return false;
//         }
//     }
//
//     // Validate loaded data
//     static bool validateData(const SchedulingData& data) {
//         // Check for valid IDs
//         std::unordered_set<int> lecturer_ids, group_ids, room_ids, course_ids;
//
//         for (const auto& lecturer : data.lecturers) {
//             if (lecturer_ids.count(lecturer.id) > 0) {
//                 std::cerr << "Duplicate lecturer ID: " << lecturer.id << std::endl;
//                 return false;
//             }
//             lecturer_ids.insert(lecturer.id);
//         }
//
//         for (const auto& group : data.groups) {
//             if (group_ids.count(group.id) > 0) {
//                 std::cerr << "Duplicate group ID: " << group.id << std::endl;
//                 return false;
//             }
//             group_ids.insert(group.id);
//         }
//
//         for (const auto& room : data.rooms) {
//             if (room_ids.count(room.id) > 0) {
//                 std::cerr << "Duplicate room ID: " << room.id << std::endl;
//                 return false;
//             }
//             room_ids.insert(room.id);
//         }
//
//         for (const auto& course : data.courses) {
//             if (course_ids.count(course.id) > 0) {
//                 std::cerr << "Duplicate course ID: " << course.id << std::endl;
//                 return false;
//             }
//             course_ids.insert(course.id);
//
//             // Validate foreign keys
//             if (lecturer_ids.count(course.lecturer_id) == 0) {
//                 std::cerr << "Invalid lecturer ID " << course.lecturer_id
//                          << " in course " << course.name << std::endl;
//                 return false;
//             }
//
//             for (int group_id : course.group_ids) {
//                 if (group_ids.count(group_id) == 0) {
//                     std::cerr << "Invalid group ID " << group_id
//                              << " in course " << course.name << std::endl;
//                     return false;
//                 }
//             }
//         }
//
//         return true;
//     }
// };
//
// // Additional utility for saving/loading schedules (solutions)
// class ScheduleSerializer {
// public:
//     struct SerializedSchedule {
//         std::vector<Assignment> assignments;
//         double fitness;
//         int hard_violations;
//         int soft_violations;
//     };
//
//     static json scheduleToJson(const Schedule& schedule) {
//         return json{
//             {"assignments", schedule.assignments},
//             {"fitness", schedule.fitness},
//             {"hard_violations", schedule.hard_violations},
//             {"soft_violations", schedule.soft_violations}
//         };
//     }
//
//     static bool saveScheduleToFile(const Schedule& schedule, const std::string& filename) {
//         try {
//             json j = scheduleToJson(schedule);
//             std::ofstream file(filename);
//             if (!file.is_open()) {
//                 std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
//                 return false;
//             }
//             file << j.dump(4);
//             file.close();
//             std::cout << "Successfully saved schedule to " << filename << std::endl;
//             return true;
//         } catch (const std::exception& e) {
//             std::cerr << "Error saving schedule: " << e.what() << std::endl;
//             return false;
//         }
//     }
//
//     static bool loadScheduleFromFile(Schedule& schedule, const std::string& filename) {
//         try {
//             std::ifstream file(filename);
//             if (!file.is_open()) {
//                 std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
//                 return false;
//             }
//             json j;
//             file >> j;
//             file.close();
//
//             schedule.clear();
//             for (const auto& assignment_json : j.at("assignments")) {
//                 Assignment a = assignment_json.get<Assignment>();
//                 schedule.addAssignment(a);
//             }
//             schedule.fitness = j.at("fitness").get<double>();
//             schedule.hard_violations = j.at("hard_violations").get<int>();
//             schedule.soft_violations = j.at("soft_violations").get<int>();
//
//             std::cout << "Successfully loaded schedule from " << filename << std::endl;
//             return true;
//         } catch (const std::exception& e) {
//             std::cerr << "Error loading schedule: " << e.what() << std::endl;
//             return false;
//         }
//     }
// };

#endif // SCHEDULING_JSON_H
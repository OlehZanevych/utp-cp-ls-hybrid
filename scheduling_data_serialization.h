#ifndef SCHEDULING_DATA_SERIALIZATION_H
#define SCHEDULING_DATA_SERIALIZATION_H

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "structures.h"
#include "schedule.h"

using json = nlohmann::json;

// JSON conversion functions for each structure

// TimeSlot serialization
void to_json(json &j, const TimeSlot &ts) {
    j = json{{"day", ts.day}, {"period", ts.period}};
}

void from_json(const json &j, TimeSlot &ts) {
    j.at("day").get_to(ts.day);
    j.at("period").get_to(ts.period);
}

// Lecturer serialization
void to_json(json &j, const Lecturer &lecturer) {
    json undesirable_slots_array = json::array();
    for (const auto &slot: lecturer.undesirable_slots) {
        undesirable_slots_array.push_back(slot);
    }

    j = json{
        {"id", lecturer.id},
        {"name", lecturer.name},
        {"undesirable_slots", undesirable_slots_array},
        {"undesirable_penalty", lecturer.undesirable_penalty}
    };
}

void from_json(const json &j, Lecturer &lecturer) {
    // Create a new lecturer with required constructor parameters
    int id = j.at("id").get<int>();
    std::string name = j.at("name").get<std::string>();
    lecturer = Lecturer(id, name);

    // Set optional fields
    lecturer.undesirable_penalty = j.at("undesirable_penalty").get<double>();

    // Clear and populate undesirable_slots
    lecturer.undesirable_slots.clear();
    for (const auto &slot_json: j.at("undesirable_slots")) {
        TimeSlot slot = slot_json.get<TimeSlot>();
        lecturer.undesirable_slots.insert(slot);
    }
}

// StudentGroup serialization
void to_json(json &j, const StudentGroup &group) {
    json undesirable_slots_array = json::array();
    for (const auto &slot: group.undesirable_slots) {
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

void from_json(const json &j, StudentGroup &group) {
    // Create a new group with required constructor parameters
    int id = j.at("id").get<int>();
    std::string name = j.at("name").get<std::string>();
    int size = j.at("size").get<int>();
    group = StudentGroup(id, name, size);

    // Set optional fields
    group.undesirable_penalty = j.at("undesirable_penalty").get<double>();

    // Clear and populate undesirable_slots
    group.undesirable_slots.clear();
    for (const auto &slot_json: j.at("undesirable_slots")) {
        TimeSlot slot = slot_json.get<TimeSlot>();
        group.undesirable_slots.insert(slot);
    }
}

// Course serialization
void to_json(json &j, const Course &course) {
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

void from_json(const json &j, Course &course) {
    // Create a new course with required constructor parameters
    int id = j.at("id").get<int>();
    std::string name = j.at("name").get<std::string>();
    int lecturer_id = j.at("lecturer_id").get<int>();
    int duration = j.at("duration").get<int>();
    int weekly_meetings = j.at("weekly_meetings").get<int>();

    course = Course(id, name, lecturer_id, duration, weekly_meetings);

    // Set vectors
    course.group_ids = j.at("group_ids").get<std::vector<int> >();
    course.required_features = j.at("required_features").get<std::vector<int> >();
}

// Room serialization
void to_json(json &j, const Room &room) {
    json features_array = json::array();
    for (const auto &feature: room.features) {
        features_array.push_back(feature);
    }

    j = json{
        {"id", room.id},
        {"name", room.name},
        {"capacity", room.capacity},
        {"features", features_array}
    };
}

void from_json(const json &j, Room &room) {
    // Create a new room with required constructor parameters
    int id = j.at("id").get<int>();
    std::string name = j.at("name").get<std::string>();
    int capacity = j.at("capacity").get<int>();

    room = Room(id, name, capacity);

    // Clear and populate features
    room.features.clear();
    for (const auto &feature: j.at("features")) {
        room.features.insert(feature.get<int>());
    }
}

// Assignment serialization
void to_json(json &j, const Assignment &assignment) {
    j = json{
        {"course_id", assignment.course_id},
        {"room_id", assignment.room_id},
        {"time_slot", assignment.time_slot}
    };
}

void from_json(const json &j, Assignment &assignment) {
    assignment.course_id = j.at("course_id").get<int>();
    assignment.room_id = j.at("room_id").get<int>();
    assignment.time_slot = j.at("time_slot").get<TimeSlot>();
}

// SchedulingData serialization
void to_json(json &j, const SchedulingData &data) {
    j = json{
        {"lecturers", data.lecturers},
        {"groups", data.groups},
        {"rooms", data.rooms},
        {"courses", data.courses}
    };
}

void from_json(const json &j, SchedulingData &data) {
    data.lecturers = j.at("lecturers").get<std::vector<Lecturer> >();
    data.groups = j.at("groups").get<std::vector<StudentGroup> >();
    data.rooms = j.at("rooms").get<std::vector<Room> >();
    data.courses = j.at("courses").get<std::vector<Course> >();
}

bool saveToFile(const SchedulingData &data, const std::string &filename) {
    try {
        json j = data;
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
            return false;
        }
        file << j.dump(4); // Pretty print with 4 spaces indentation
        file.close();
        std::cout << "Successfully saved scheduling data to " << filename << std::endl;
        return true;
    } catch (const std::exception &e) {
        printf("Error saving to JSON: %s\n", e.what());
        return false;
    }
}

bool loadFromFile(SchedulingData &data, const std::string &filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            printf("Error: Could not open file %s for reading\n", filename.c_str());
            return false;
        }
        json j;
        file >> j;
        file.close();
        data = j.get<SchedulingData>();

        printf("Successfully loaded scheduling data from %s\n", filename.c_str());
        return true;
    } catch (const std::exception &e) {
        printf("Error loading from JSON: %s\n", e.what());
        return false;
    }
}

#endif // SCHEDULING_DATA_SERIALIZATION_H

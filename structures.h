#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <vector>
#include <unordered_set>

// Core data structures

struct TimeSlot {
    int day;
    int period;

    TimeSlot(int d = 0, int p = 0);

    bool operator==(const TimeSlot& other) const;
    bool operator<(const TimeSlot& other) const;
    size_t hash() const;
};

namespace std {
    template<> struct hash<TimeSlot> {
        size_t operator()(const TimeSlot& ts) const {
            return ts.hash();
        }
    };
}

struct Lecturer {
    int id;
    std::string name;
    std::unordered_set<TimeSlot> undesirable_slots;
    double undesirable_penalty = 20.0;

    Lecturer(int id, const std::string& name);

    void addUndesirableSlot(const TimeSlot& ts);
    bool isUndesirableSlot(const TimeSlot& ts) const;
};

struct StudentGroup {
    int id;
    std::string name;
    int size;
    std::unordered_set<TimeSlot> undesirable_slots;
    double undesirable_penalty = 15.0;

    StudentGroup(int id, const std::string& name, int size);

    void addUndesirableSlot(const TimeSlot& ts);
    bool isUndesirableSlot(const TimeSlot& ts) const;
};

struct Course {
    int id;
    std::string name;
    int lecturer_id;
    std::vector<int> group_ids;
    int duration;
    std::vector<int> required_features;
    int weekly_meetings;

    Course(int id, const std::string& name, int lecturer, int dur, int meetings = 1);

    void addGroup(int group_id);
    int getTotalStudents(const std::vector<StudentGroup>& groups) const;
};

struct Room {
    int id;
    std::string name;
    int capacity;
    std::unordered_set<int> features;

    Room(int id, const std::string& name, int cap);

    bool hasFeatures(const std::vector<int>& required) const;
};

struct Assignment {
    int course_id;
    int room_id;
    TimeSlot time_slot;

    Assignment(int c = -1, int r = -1, TimeSlot ts = TimeSlot());
};

#endif //STRUCTURES_H

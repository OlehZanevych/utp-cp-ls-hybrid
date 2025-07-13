#include "structures.h"

// ---------- TimeSlot ----------

TimeSlot::TimeSlot(int d, int p) : day(d), period(p) {}

bool TimeSlot::operator==(const TimeSlot& other) const {
    return day == other.day && period == other.period;
}

bool TimeSlot::operator<(const TimeSlot& other) const {
    return day < other.day || (day == other.day && period < other.period);
}

size_t TimeSlot::hash() const {
    return std::hash<int>()(day) ^ (std::hash<int>()(period) << 1);
}

// ---------- Lecturer ----------

Lecturer::Lecturer(int id, const std::string& name)
    : id(id), name(name) {}

void Lecturer::addUndesirableSlot(const TimeSlot& ts) {
    undesirable_slots.insert(ts);
}

bool Lecturer::isUndesirableSlot(const TimeSlot& ts) const {
    return undesirable_slots.find(ts) != undesirable_slots.end();
}

// ---------- StudentGroup ----------

StudentGroup::StudentGroup(int id, const std::string& name, int size)
    : id(id), name(name), size(size) {}

void StudentGroup::addUndesirableSlot(const TimeSlot& ts) {
    undesirable_slots.insert(ts);
}

bool StudentGroup::isUndesirableSlot(const TimeSlot& ts) const {
    return undesirable_slots.find(ts) != undesirable_slots.end();
}

// ---------- Course ----------

Course::Course(int id, const std::string& name, int lecturer, int dur, int meetings)
    : id(id), name(name), lecturer_id(lecturer), duration(dur), weekly_meetings(meetings) {}

void Course::addGroup(int group_id) {
    group_ids.push_back(group_id);
}

int Course::getTotalStudents(const std::vector<StudentGroup>& groups) const {
    int total = 0;
    for (int gid : group_ids) {
        if (gid >= 0 && gid < static_cast<int>(groups.size())) {
            total += groups[gid].size;
        }
    }
    return total;
}

// ---------- Room ----------

Room::Room(int id, const std::string& name, int cap)
    : id(id), name(name), capacity(cap) {}

bool Room::hasFeatures(const std::vector<int>& required) const {
    for (int f : required) {
        if (features.find(f) == features.end())
            return false;
    }
    return true;
}

// ---------- Assignment ----------

Assignment::Assignment(int c, int r, TimeSlot ts)
    : course_id(c), room_id(r), time_slot(ts) {}

#include <random>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include "structures.h"
#include "schedule.h"
#include "constraint_checker.h"
#include "adaptive_neighborhood_selector.h"

#include "scheduling_data_generator.h"

SchedulingDataGenerator::SchedulingDataGenerator(unsigned int seed)
    : rng(seed) {
}

SchedulingData SchedulingDataGenerator::generateData(
    int num_lecturers,
    int num_groups,
    int num_rooms,
    int num_courses,
    int days,
    int periods_per_day,
    double undesirable_slot_probability,
    double course_feature_probability,
    double room_feature_probability
) {
    SchedulingData data;

    // Generate lecturers
    data.lecturers = generateLecturers(num_lecturers, days, periods_per_day, undesirable_slot_probability);

    // Generate student groups
    data.groups = generateStudentGroups(num_groups, days, periods_per_day, undesirable_slot_probability);

    // Generate rooms
    data.rooms = generateRooms(num_rooms, num_groups, room_feature_probability);

    // Generate courses
    data.courses = generateCourses(num_courses, num_lecturers, num_groups, course_feature_probability);

    return data;
}

std::vector<Lecturer> SchedulingDataGenerator::generateLecturers(int count, int days, int periods_per_day,
                                                                 double undesirable_prob) {
    std::vector<Lecturer> lecturers;
    std::uniform_int_distribution<> name_dist(0, first_names.size() - 1);
    std::uniform_int_distribution<> title_dist(0, titles.size() - 1);
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<> day_dist(0, days - 1);
    std::uniform_int_distribution<> period_dist(0, periods_per_day - 1);
    std::uniform_int_distribution<> num_undesirable_dist(2, 6); // 2-6 undesirable slots

    std::unordered_set<std::string> used_names;

    for (int i = 0; i < count; i++) {
        // Generate unique name
        std::string full_name;
        do {
            std::string first = first_names[name_dist(rng)];
            std::string last = last_names[name_dist(rng)];
            std::string title = titles[title_dist(rng)];
            full_name = title + " " + first + " " + last;
        } while (used_names.find(full_name) != used_names.end());

        used_names.insert(full_name);
        Lecturer lecturer(i, full_name);

        // Add undesirable slots
        if (prob_dist(rng) < undesirable_prob) {
            int num_slots = num_undesirable_dist(rng);
            std::unordered_set<TimeSlot> added_slots;

            for (int j = 0; j < num_slots; j++) {
                TimeSlot slot(day_dist(rng), period_dist(rng));
                if (added_slots.find(slot) == added_slots.end()) {
                    lecturer.addUndesirableSlot(slot);
                    added_slots.insert(slot);
                }
            }

            // Set penalty based on seniority (Prof. has higher penalty)
            if (lecturer.name.find("Prof.") != std::string::npos) {
                lecturer.undesirable_penalty = 25.0;
            } else {
                lecturer.undesirable_penalty = 20.0;
            }
        }

        lecturers.push_back(lecturer);
    }

    return lecturers;
}

std::vector<StudentGroup> SchedulingDataGenerator::generateStudentGroups(
    int count, int days, int periods_per_day, double undesirable_prob) {
    std::vector<StudentGroup> groups;
    std::uniform_int_distribution<> size_dist(15, 35); // Group sizes between 15-35
    std::uniform_int_distribution<> year_dist(1, 4); // Years 1-4
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<> day_dist(0, days - 1);
    std::uniform_int_distribution<> period_dist(0, periods_per_day - 1);

    char section = 'A';
    int current_year = 1;
    int groups_per_year = (count + 3) / 4; // Distribute evenly across 4 years

    for (int i = 0; i < count; i++) {
        // Generate group name (e.g., "CS-1A", "CS-2B")
        std::string name = "CS-" + std::to_string(current_year) + section;
        int size = size_dist(rng);

        StudentGroup group(i, name, size);

        // Add undesirable slots based on year
        if (prob_dist(rng) < undesirable_prob) {
            if (current_year == 1) {
                // First years avoid late classes
                for (int p = periods_per_day - 2; p < periods_per_day; p++) {
                    group.addUndesirableSlot(TimeSlot(days - 1, p)); // Late Friday
                }
            } else if (current_year >= 3) {
                // Senior years might avoid early classes
                group.addUndesirableSlot(TimeSlot(0, 0)); // Early Monday
                group.addUndesirableSlot(TimeSlot(0, 1));
            }

            // Add some random undesirable slots
            std::uniform_int_distribution<> num_random_dist(1, 3);
            int num_random = num_random_dist(rng);
            for (int j = 0; j < num_random; j++) {
                group.addUndesirableSlot(TimeSlot(day_dist(rng), period_dist(rng)));
            }
        }

        groups.push_back(group);

        // Update section and year
        section++;
        if ((i + 1) % groups_per_year == 0 && current_year < 4) {
            current_year++;
            section = 'A';
        }
    }

    return groups;
}

std::vector<Room> SchedulingDataGenerator::generateRooms(int count, int num_groups, double feature_prob) {
    std::vector<Room> rooms;
    std::uniform_int_distribution<> type_dist(0, room_types.size() - 1);
    std::uniform_int_distribution<> capacity_dist(20, 100);
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<> feature_dist(1, 3); // Features 1-3

    // Ensure we have enough total capacity
    int min_total_capacity = num_groups * 25; // Assume average group size of 25
    int current_capacity = 0;

    for (int i = 0; i < count; i++) {
        std::string type = room_types[type_dist(rng)];
        std::string name = type + " " + std::string(1, 'A' + (i % 26));
        if (i >= 26) name += std::to_string(i / 26 + 1);

        // Adjust capacity based on room type
        int capacity;
        if (type == "Lecture Hall") {
            capacity = std::uniform_int_distribution<>(60, 120)(rng);
        } else if (type == "Lab") {
            capacity = std::uniform_int_distribution<>(20, 30)(rng);
        } else if (type == "Seminar Room") {
            capacity = std::uniform_int_distribution<>(15, 25)(rng);
        } else {
            capacity = capacity_dist(rng);
        }

        // Ensure minimum total capacity
        if (i == count - 1 && current_capacity < min_total_capacity) {
            capacity = std::max(capacity, min_total_capacity - current_capacity);
        }
        current_capacity += capacity;

        Room room(i, name, capacity);

        // Add features
        if (prob_dist(rng) < feature_prob) {
            // Feature 1: Projector (common)
            if (prob_dist(rng) < 0.7) {
                room.features.insert(1);
            }

            // Feature 2: Lab equipment (for labs)
            if (type == "Lab" || prob_dist(rng) < 0.3) {
                room.features.insert(2);
            }

            // Feature 3: Special equipment (rare)
            if (prob_dist(rng) < 0.1) {
                room.features.insert(3);
            }
        }

        rooms.push_back(room);
    }

    return rooms;
}

std::vector<Course> SchedulingDataGenerator::generateCourses(int count, int num_lecturers, int num_groups,
                                                             double feature_prob) {
    std::vector<Course> courses;
    std::uniform_int_distribution<> lecturer_dist(0, num_lecturers - 1);
    std::uniform_int_distribution<> duration_dist(1, 3); // 1-3 period duration
    std::uniform_int_distribution<> meetings_dist(1, 3); // 1-3 meetings per week
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<> num_groups_dist(1, 3); // 1-3 groups per course

    std::unordered_set<std::string> used_names;

    // Track course load per lecturer
    std::vector<int> lecturer_load(num_lecturers, 0);

    for (int i = 0; i < count; i++) {
        // Generate unique course name
        std::string name;
        do {
            std::string prefix = course_prefixes[std::uniform_int_distribution<>(0, course_prefixes.size() - 1)(rng)];
            std::string subject = course_subjects[std::uniform_int_distribution<>(0, course_subjects.size() - 1)(rng)];
            name = prefix + " " + subject;
        } while (used_names.find(name) != used_names.end());

        used_names.insert(name);

        // Assign to lecturer with lower load
        int lecturer_id = lecturer_dist(rng);
        // 30% chance to reassign to lecturer with lowest load
        if (prob_dist(rng) < 0.3) {
            auto min_it = std::min_element(lecturer_load.begin(), lecturer_load.end());
            lecturer_id = std::distance(lecturer_load.begin(), min_it);
        }

        int duration = duration_dist(rng);
        int meetings = meetings_dist(rng);

        // Longer courses typically meet less frequently
        if (duration == 3) {
            meetings = std::min(meetings, 2);
        }

        Course course(i, name, lecturer_id, duration, meetings);
        lecturer_load[lecturer_id] += duration * meetings;

        // Add features
        if (prob_dist(rng) < feature_prob) {
            // Certain subjects more likely to need projector
            if (name.find("Graphics") != std::string::npos ||
                name.find("Vision") != std::string::npos ||
                name.find("AI") != std::string::npos ||
                prob_dist(rng) < 0.5) {
                course.required_features.push_back(1); // Projector
            }

            // Lab courses need lab equipment
            if (name.find("Programming") != std::string::npos ||
                name.find("Networks") != std::string::npos ||
                name.find("Operating") != std::string::npos ||
                prob_dist(rng) < 0.2) {
                course.required_features.push_back(2); // Lab equipment
            }
        }

        // Assign groups (try to keep same-year groups together)
        int num_groups_for_course = std::min(num_groups_dist(rng), num_groups);
        std::unordered_set<int> assigned_groups;

        // First, try to assign groups from the same year
        int start_group = std::uniform_int_distribution<>(0, num_groups - 1)(rng);
        int groups_per_year = (num_groups + 3) / 4;
        int year_start = (start_group / groups_per_year) * groups_per_year;

        for (int j = 0; j < num_groups_for_course && assigned_groups.size() < num_groups_for_course; j++) {
            int group_id = year_start + (j % groups_per_year);
            if (group_id < num_groups) {
                course.addGroup(group_id);
                assigned_groups.insert(group_id);
            }
        }

        // If we need more groups, add random ones
        while (assigned_groups.size() < num_groups_for_course) {
            int group_id = std::uniform_int_distribution<>(0, num_groups - 1)(rng);
            if (assigned_groups.find(group_id) == assigned_groups.end()) {
                course.addGroup(group_id);
                assigned_groups.insert(group_id);
            }
        }

        courses.push_back(course);
    }

    return courses;
}

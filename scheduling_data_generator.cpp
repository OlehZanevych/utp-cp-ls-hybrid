#include <random>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include "structures.h"

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
        // std::string name = "CS-" + std::to_string(current_year) + section;
        std::string name = "CS-" + std::to_string(current_year) + std::to_string(i);
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
    courses.reserve(count);

    // Pre-generate all course names at once
    auto generateUniqueNames = [this](int needed) -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(needed);

        // Create index arrays for combinations
        std::vector<std::pair<int, int>> combinations;
        for (int p = 0; p < course_prefixes.size(); p++) {
            for (int s = 0; s < course_subjects.size(); s++) {
                combinations.push_back({p, s});
            }
        }

        // Shuffle combinations
        std::shuffle(combinations.begin(), combinations.end(), rng);

        // Generate names from shuffled combinations
        for (int i = 0; i < needed && i < combinations.size(); i++) {
            names.push_back(course_prefixes[combinations[i].first] + " " +
                          course_subjects[combinations[i].second]);
        }

        // Add numbered variants if needed
        if (needed > combinations.size()) {
            int suffix = 2;
            size_t base_size = names.size();
            while (names.size() < needed) {
                for (size_t i = 0; i < base_size && names.size() < needed; i++) {
                    names.push_back(names[i] + " " + std::to_string(suffix));
                }
                suffix++;
            }
        }

        return names;
    };

    auto names = generateUniqueNames(count);

    // Initialize distributions
    std::uniform_int_distribution<> lecturer_dist(0, num_lecturers - 1);
    std::uniform_int_distribution<> duration_dist(1, 3);
    std::uniform_int_distribution<> meetings_dist(1, 3);
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<> num_groups_dist(1, std::min(3, num_groups));

    // Track lecturer loads
    std::vector<int> lecturer_load(num_lecturers, 0);

    // Pre-calculate groups per year
    const int groups_per_year = (num_groups + 3) / 4;

    // Batch process courses
    for (int i = 0; i < count; i++) {
        // Select lecturer (with load balancing)
        int lecturer_id = lecturer_dist(rng);
        if (i % 10 == 0) { // Check load balance every 10 courses
            auto min_it = std::min_element(lecturer_load.begin(), lecturer_load.end());
            if (lecturer_load[lecturer_id] > *min_it + 5) {
                lecturer_id = std::distance(lecturer_load.begin(), min_it);
            }
        }

        int duration = duration_dist(rng);
        int meetings = (duration == 3) ? std::min(meetings_dist(rng), 2) : meetings_dist(rng);

        Course course(i, names[i], lecturer_id, duration, meetings);
        lecturer_load[lecturer_id] += duration * meetings;

        // Simplified feature assignment based on name patterns
        if (prob_dist(rng) < feature_prob) {
            const std::string& name_lower = names[i];
            if (name_lower.find("Graphics") != std::string::npos ||
                name_lower.find("Vision") != std::string::npos ||
                name_lower.find("AI") != std::string::npos ||
                prob_dist(rng) < 0.5) {
                course.required_features.push_back(1);
            }

            if (name_lower.find("Programming") != std::string::npos ||
                name_lower.find("Networks") != std::string::npos ||
                name_lower.find("Operating") != std::string::npos ||
                prob_dist(rng) < 0.2) {
                course.required_features.push_back(2);
            }
        }

        // Fast group assignment
        int num_groups_for_course = num_groups_dist(rng);
        if (num_groups_for_course > 0 && num_groups > 0) {
            // Direct random selection without complex logic
            std::uniform_int_distribution<> group_select(0, num_groups - 1);
            std::unordered_set<int> selected;

            course.group_ids.reserve(num_groups_for_course);
            while (selected.size() < num_groups_for_course) {
                int group_id = group_select(rng);
                if (selected.insert(group_id).second) {
                    course.addGroup(group_id);
                }
            }
        }

        courses.push_back(std::move(course));
    }

    return courses;
}

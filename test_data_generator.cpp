#include <string>

#include "scheduling_data_generator.h"
#include "scheduling_data_serialization.h"

SchedulingData generateRandomSchedulingData(
    int num_lecturers = 10,
    int num_groups = 12,
    int num_rooms = 8,
    int num_courses = 20,
    unsigned int seed = 0  // 0 means use time-based seed
) {
    SchedulingDataGenerator generator(seed == 0 ? std::chrono::steady_clock::now().time_since_epoch().count() : seed);
    return generator.generateData(num_lecturers, num_groups, num_rooms, num_courses);
}

int main() {
    std::string fileName = "scheduling-data-1.json";

    const SchedulingData schedulingData = generateRandomSchedulingData(
            150,  // lecturers
            200,  // student groups
            120,  // rooms
            300   // courses
        );

    // serializeSchedulingData(schedulingData, fileName);

    return 0;
}

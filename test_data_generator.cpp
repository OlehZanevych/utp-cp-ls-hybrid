#include <string>

#include "scheduling_data_generator.h"
#include "scheduling_data_serialization.h"

SchedulingData generateRandomSchedulingData(
    int num_lecturers = 10,
    int num_groups = 12,
    int num_rooms = 8,
    int num_courses = 20,
    unsigned int seed = 0 // 0 means use time-based seed
) {
    SchedulingDataGenerator generator(seed == 0 ? std::chrono::steady_clock::now().time_since_epoch().count() : seed);
    return generator.generateData(num_lecturers, num_groups, num_rooms, num_courses);
}

int main() {
    std::string file_name = "../data/scheduling-data-1.json";
    const int coefficient = 1;

    int num_lecturers = coefficient * 150;
    int num_student_groups = coefficient * 200;
    int num_rooms = coefficient * 120;
    int num_courses = coefficient * 300;

    const SchedulingData schedulingData = generateRandomSchedulingData(
        num_lecturers,
        num_student_groups,
        num_rooms,
        num_courses
    );

    saveToFile(schedulingData, file_name);

    SchedulingData loadedData;
    loadFromFile(loadedData, file_name);

    return 0;
}

#ifndef SCHEDULING_DATA_GENERATOR_H
#define SCHEDULING_DATA_GENERATOR_H

#include <random>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

// Data generation structure to hold all generated entities
struct SchedulingData {
    std::vector<Lecturer> lecturers;
    std::vector<StudentGroup> groups;
    std::vector<Room> rooms;
    std::vector<Course> courses;
};

class SchedulingDataGenerator {
private:
    std::mt19937 rng;

    // Name generation helpers
    std::vector<std::string> first_names = {
        "James", "Mary", "John", "Patricia", "Robert", "Jennifer", "Michael", "Linda",
        "William", "Elizabeth", "David", "Barbara", "Richard", "Susan", "Joseph", "Jessica",
        "Thomas", "Sarah", "Charles", "Karen", "Christopher", "Nancy", "Daniel", "Lisa"
    };

    std::vector<std::string> last_names = {
        "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis",
        "Rodriguez", "Martinez", "Hernandez", "Lopez", "Gonzalez", "Wilson", "Anderson",
        "Thomas", "Taylor", "Moore", "Jackson", "Martin", "Lee", "Thompson", "White"
    };

    std::vector<std::string> titles = {"Dr.", "Prof.", "Dr.", "Prof.", "Dr."}; // More Dr. than Prof.

    std::vector<std::string> course_prefixes = {
        "Introduction to", "Advanced", "Fundamentals of", "Applied", "Theoretical",
        "Practical", "Modern", "Contemporary", "Principles of", "Topics in"
    };

    std::vector<std::string> course_subjects = {
        "Algorithms", "Data Structures", "Database Systems", "Computer Networks",
        "Operating Systems", "Software Engineering", "Artificial Intelligence",
        "Machine Learning", "Computer Graphics", "Web Development", "Mobile Computing",
        "Cloud Computing", "Cybersecurity", "Distributed Systems", "Compiler Design",
        "Computer Architecture", "Human-Computer Interaction", "Data Mining",
        "Natural Language Processing", "Computer Vision", "Robotics", "Game Development",
        "Quantum Computing", "Blockchain", "Internet of Things", "Parallel Computing",
        "Discrete Mathematics", "Linear Algebra", "Calculus", "Statistics"
    };

    std::vector<std::string> room_types = {"Room", "Lab", "Lecture Hall", "Seminar Room", "Tutorial Room"};

public:
    SchedulingDataGenerator(unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count());

    SchedulingData generateData(
        int num_lecturers,
        int num_groups,
        int num_rooms,
        int num_courses,
        int days = 5,
        int periods_per_day = 8,
        double undesirable_slot_probability = 0.15,  // Probability a lecturer/group has undesirable slots
        double course_feature_probability = 0.3,      // Probability a course needs special features
        double room_feature_probability = 0.4         // Probability a room has special features
    );

private:
    std::vector<Lecturer> generateLecturers(int count, int days, int periods_per_day, double undesirable_prob);

    std::vector<StudentGroup> generateStudentGroups(int count, int days, int periods_per_day, double undesirable_prob);

    std::vector<Room> generateRooms(int count, int num_groups, double feature_prob);

    std::vector<Course> generateCourses(int count, int num_lecturers, int num_groups, double feature_prob);
};

#endif //SCHEDULING_DATA_GENERATOR_H

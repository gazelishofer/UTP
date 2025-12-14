// Header file defining the Student structure used throughout the program.
#ifndef UTP_STUDENT_H // Header guard start to prevent multiple inclusion.
#define UTP_STUDENT_H // Define guard macro when file is included.

#include <string> // Include std::string so we can store names and other text fields.

struct Student { // Structure representing one student and their data.
    std::string name; // Student's first name.
    std::string surname; // Student's last name.
    std::string middleName; // Student's middle name / patronymic.
    int year; // Birth year of the student.
    int course; // Course number (academic year) of the student.
    std::string subjects[3]; // Names of the three subjects the student takes.
    std::string grades[3]; // Grades for each subject, stored as comma-separated string.

    Student() // Default constructor initializes fields to safe defaults.
        : name(""), surname(""), middleName(""), year(0), course(0) {} // Initialize strings empty and numeric fields to zero.
}; // End of Student structure definition.


#endif // End of header guard.

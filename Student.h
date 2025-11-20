#ifndef UTP_STUDENT_H
#define UTP_STUDENT_H

#include <string>

struct Student {
    std::string name;
    std::string surname;
    std::string middleName;
    int year;
    int course;
    std::string subjects[3];
    std::string grades[3];

    Student()
        : name(""), surname(""), middleName(""), year(0), course(0) {}
};



#endif // UTP_STUDENT_H


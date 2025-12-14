// Simple test file showing table output; comments explain each line.

#include <iostream> // For console output.
#include <string> // For std::string type.
#include <vector> // Included though not used heavily here.
using namespace std; // Use standard namespace for brevity.

struct Student { // Local Student struct for test file demonstration.
    string name; // First name.
    string surname; // Last name.
    string middleName; // Middle name.
    int year; // Birth year.
    int course; // Course number.
    string subjects[3]; // Three subjects.
    string grades[3]; // Grades for each subject as strings.
}; // End of Student struct.

Student testStudents[] = { // Array of test students with sample data.
    {"e", "d", "f", 2007, 1, {"math", "Rus", "Geo"}, {"5,5,5", "1,1,1", "3,3,3"}}, // First student record.
    {"ГАГООООО", "ГАГООООООО�ОООООООООБОК", "ГГАГИИКККЙКЙКЛДСЙКСДБКЙ", 2024, 2, {"меатем�атикка", "англ", "русс"}, {"4,4,4", "5,5,5", "5,4,3"}}, // Second student with Cyrillic data.
    {"fgjfjfopsjpojgpojspgs;gkfkpjgpj", "jfjfljpodfjgopjpdfjgpodfjgopjdg", "rirriririririr", 2025, 1, {"angl", "math", "istoria"}, {"4,4,4", "2,3,4", "5,5,5"}} // Third student with long names.
}; // End of test data array.

int main() { // Entry point for test program.
    cout << "Testing console width: "; // Print description text.
    cout << "120 chars" << endl; // Print example width and newline.
    
    cout << "Testing table format:" << endl; // Indicate the next output is a table preview.
    cout << "|№ |Год  |Курс |Имя                  |Фамилия              |" << endl; // Example table header row.
    cout << string(55, '-') << endl; // Print a separator line of 55 dashes.
    cout << "|1 |2007 |1    |e                    |d                    |" << endl; // Example data row.
    cout << string(55, '-') << endl; // Final separator to close table.
    
    return 0; // Indicate successful execution.
}

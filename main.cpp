#include <vector>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <windows.h>
#include <locale>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "Student.h"
#include <string>
#include <codecvt>

using namespace std;

Student *students = new Student[10];
int studentCount = 0;
int capacity = 10;

const string FILE1_PATH = "C:/Users/user/CLionProjects/UTP/forStudents.txt";
const string FILE2_PATH = "C:/Users/user/CLionProjects/UTP/forStudents.bin";

void processChoice(int choice);


string toLowerUtf8(const string& s);
int utf8_width(const std::string& s);
void printPadded(const string& s, int width);
void expandArray();
void editStudent(int index);
void deleteStudent(int index);
void printArray();
void addStudentToArray(const Student &student);
bool checkAvailability();
void saveToFile();
void loadFromFile();
void saveToBinaryFile();
void loadFromBinaryFile();
bool askPermission1();
bool askPermission2();
bool isNumber(string s);
bool checkGrades(string s);

bool isNumber(string s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool checkGrades(string s) {
    if (s.empty()) return false;
    string num = "";
    for (int i = 0; i < s.size(); i++) {
        char c = s[i];
        if (isdigit(c)) {
            num += c;
        } else if (c != ',') {
            return false;
        }
        if (c == ',' || i == s.size() - 1) {
            if (!num.empty()) {
                int g = stoi(num);
                if (g < 1 || g > 5) return false;
                num = "";
            }
        }
    }
    return true;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::locale::global(std::locale(""));

    while (true) {
        int choice;
        cout << "\nМеню:\n";
        cout << "1) Добавить студента\n";
        cout << "2) Редактировать студента\n";
        cout << "3) Удалить студента\n";
        cout << "4) Сохранить в текстовый файл\n";
        cout << "5) Сохранить бинарный файл\n";
        cout << "6) Загрузить данные\n";
        cout << "7) Показать список\n";
        cout << "8) Выход\n";
        cout << "Выберите пункт: ";
        cin >> choice;

        if (choice == 8) break;
        processChoice(choice);
    }
    return 0;
}

void processChoice(int choice) {
    switch (choice) {

        case 1: {
            cout << "Добавление студента\n";
            Student student;
            string input;

            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            while (true) {
                cout << "Введите год поступления: ";
                getline(cin, input);
                if (isNumber(input)) { student.year = stoi(input); break; }
                cout << "Введите число!\n";
            }

            while (true) {
                cout << "Введите курс: ";
                getline(cin, input);
                if (isNumber(input)) { student.course = stoi(input); break; }
                cout << "Введите число!\n";
            }

            cout << "Введите имя: ";
            getline(cin, student.name);

            cout << "Введите фамилию: ";
            getline(cin, student.surname);

            cout << "Введите отчество: ";
            getline(cin, student.middleName);

            for (int i = 0; i < 3; i++) {
                cout << "Введите предмет " << i + 1 << ": ";
                getline(cin, student.subjects[i]);

                while (true) {
                    cout << "Введите оценки по предмету (1–5, через запятую): ";
                    getline(cin, student.grades[i]);
                    if (checkGrades(student.grades[i])) break;
                    cout << "Ошибка: допустимы только числа 1–5 через запятую.\n";
                }
            }

            addStudentToArray(student);
            saveToFile();
            break;
        }

        case 2: {
            printArray();
            cout << "Введите номер студента для редактирования: ";
            int index;
            cin >> index;
            editStudent(index);
            break;
        }

        case 3: {
            printArray();
            cout << "Введите номер студента для удаления: ";
            int index;
            cin >> index;
            deleteStudent(index - 1);
            break;
        }

        case 4:
            saveToFile();
            break;

        case 5:
            saveToBinaryFile();
            break;

        case 6:
            loadFromFile();
            break;

        case 7:
            printArray();
            break;

        default:
            cout << "Неверный пункт меню.\n";
            break;
    }
}



// UTF-8 CHARACTER WIDTH (counts characters, not bytes)
// ------------------------------------------------
int utf8_width(const string& s) {
    int count = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = s[i];
        int len = 1;

        if ((c & 0x80) == 0) len = 1;           // ASCII
        else if ((c & 0xE0) == 0xC0) len = 2;  // 2-byte UTF-8
        else if ((c & 0xF0) == 0xE0) len = 3;  // 3-byte UTF-8
        else if ((c & 0xF8) == 0xF0) len = 4;  // 4-byte UTF-8

        i += len;
        count++;
    }
    return count;
}

// ------------------------------------------------
// PRINT UTF-8 STRING WITH PADDING
// ------------------------------------------------
void printPadded(const string& s, int width) {
    int w = utf8_width(s);
    cout << s;
    for (int i = 0; i < width - w; i++)
        cout << ' ';
}

// ------------------------------------------------
// FULL DYNAMIC UTF-8 SAFE TABLE PRINTER
// ------------------------------------------------
void printArray() {
    if (studentCount == 0) {
        cout << "Нет студентов.\n";
        return;
    }

    // ----- Column widths -----
    int wNo        = 2;
    int wYear      = 4;
    int wCourse    = 6;
    int wName      = 4;
    int wSurname   = 7;
    int wMidName   = 11;
    int wSubject   = 8;
    int wGrades    = 6;

    for (int i = 0; i < studentCount; i++) {

        wName      = max(wName,      utf8_width(students[i].name));
        wSurname   = max(wSurname,   utf8_width(students[i].surname));
        wMidName   = max(wMidName,   utf8_width(students[i].middleName));

        wYear      = max(wYear,      (int)to_string(students[i].year).size());
        wCourse    = max(wCourse,    (int)to_string(students[i].course).size());

        for (int j = 0; j < 3; j++) {
            wSubject = max(wSubject, utf8_width(students[i].subjects[j]));
            wGrades  = max(wGrades,  utf8_width(students[i].grades[j]));
        }
    }

    // ----- HEADER -----
    cout << "|"; printPadded("No", wNo + 2);
    cout << "|"; printPadded("Year", wYear + 2);
    cout << "|"; printPadded("Course", wCourse + 2);
    cout << "|"; printPadded("Name", wName + 2);
    cout << "|"; printPadded("Surname", wSurname + 2);
    cout << "|"; printPadded("Middle Name", wMidName + 2);

    for (int i = 1; i <= 3; i++) {
        string s = "Subject " + to_string(i);
        cout << "|"; printPadded(s, wSubject + 2);
        cout << "|"; printPadded("Grades", wGrades + 2);
    }
    cout << "|\n";

    // ----- SEPARATOR -----
    int totalWidth =
        1 + (wNo + 2) +
        1 + (wYear + 2) +
        1 + (wCourse + 2) +
        1 + (wName + 2) +
        1 + (wSurname + 2) +
        1 + (wMidName + 2) +
        3 * (1 + (wSubject + 2) + 1 + (wGrades + 2)) +
        1;

    cout << string(totalWidth, '-') << "\n";

    // ----- ROWS -----
    for (int i = 0; i < studentCount; i++) {
        cout << "|"; printPadded(to_string(i + 1), wNo + 2);
        cout << "|"; printPadded(to_string(students[i].year),   wYear + 2);
        cout << "|"; printPadded(to_string(students[i].course), wCourse + 2);
        cout << "|"; printPadded(students[i].name,              wName + 2);
        cout << "|"; printPadded(students[i].surname,           wSurname + 2);
        cout << "|"; printPadded(students[i].middleName,        wMidName + 2);

        for (int j = 0; j < 3; j++) {
            cout << "|"; printPadded(students[i].subjects[j], wSubject + 2);
            cout << "|"; printPadded(students[i].grades[j],   wGrades + 2);
        }

        cout << "|\n";
    }
}
bool checkAvailability() {
    if (studentCount + 1 > capacity) expandArray();
    return true;
}

void addStudentToArray(const Student &student) {
    checkAvailability();
    students[studentCount] = student;
    studentCount++;
    cout << "Студент успешно добавлен.\n";
}

void deleteStudent(int index) {
    if (index < 0 || index >= studentCount) {
        cout << "Неверный номер студента.\n";
        return;
    }
    for (int i = index; i < studentCount - 1; i++)
        students[i] = students[i + 1];

    studentCount--;
    cout << "Студент удалён.\n";
    saveToFile();
}

void expandArray() {
    int newCapacity = capacity * 2;
    Student *newStudents = new Student[newCapacity];
    for (int i = 0; i < capacity; i++)
        newStudents[i] = students[i];

    delete[] students;
    students = newStudents;
    capacity = newCapacity;

    cout << "Размер массива увеличен: " << capacity << "\n";
}

void editStudent(int index) {
    cout << "Редактирование пока не реализовано.\n";
}

void saveToFile() {
    ofstream fout(FILE1_PATH);
    if (!fout) {
        cout << "Ошибка: не удалось открыть файл для записи.\n";
        return;
    }
    for (int i = 0; i < studentCount; i++) {
        fout << students[i].year << " "
             << students[i].course << " "
             << students[i].name << " "
             << students[i].surname << " "
             << students[i].middleName << " ";
        for (int j = 0; j < 3; j++)
            fout << students[i].subjects[j] << " " << students[i].grades[j] << " ";
        fout << "\n";
    }
    fout.close();
    cout << "Текстовый файл сохранён.\n";
}

void loadFromFile() {
    ifstream fin(FILE1_PATH);
    if (!fin) {
        cout << "Текстовый файл не найден. Загрузка бинарного...\n";
        loadFromBinaryFile();
        return;
    }
    studentCount = 0;
    while (fin >> students[studentCount].year
               >> students[studentCount].course
               >> students[studentCount].name
               >> students[studentCount].surname
               >> students[studentCount].middleName) {
        for (int j = 0; j < 3; j++)
            fin >> students[studentCount].subjects[j] >> students[studentCount].grades[j];

        studentCount++;
        if (studentCount >= capacity) expandArray();
    }
    fin.close();
    cout << "Текстовый файл загружен.\n";
}

string toLowerUtf8(const string& s) {
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    wstring ws = conv.from_bytes(s);

    for (auto& c : ws)
        c = towlower(c);

    return conv.to_bytes(ws);
}

bool askPermission2() {
    string choice;
    cout << "Хотите продолжить? (y/n, д/н): ";
    cin >> choice;

    choice = toLowerUtf8(choice);

    return (choice == "y" || choice == "д");
}


void saveToBinaryFile() {
    ofstream fout(FILE2_PATH, ios::binary);
    if (!fout) {
        cout << "Ошибка записи бинарного файла.\n";
        return;
    }
    fout.write((char*)&studentCount, sizeof(studentCount));
    fout.write((char*)students, sizeof(Student) * studentCount);
    fout.close();
    cout << "Бинарный файл сохранён.\n";
}

void loadFromBinaryFile() {
    ifstream fin(FILE2_PATH, ios::binary);
    if (!fin) {
        cout << "Бинарный файл не найден.\n";
        return;
    }
    int countFromFile = 0;
    fin.read((char*)&countFromFile, sizeof(countFromFile));

    if (countFromFile > capacity) {
        delete[] students;
        students = new Student[countFromFile];
        capacity = countFromFile;
    }

    fin.read((char*)students, sizeof(Student) * countFromFile);
    studentCount = countFromFile;
    fin.close();
    cout << "Бинарный файл загружен.\n";
}

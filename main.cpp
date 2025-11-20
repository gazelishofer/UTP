#include <vector>
#include <filesystem>
#include <iomanip>
#include <limits>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif
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
int getConsoleWidth();
void addStudentToArray(const Student &student);
void sortStudentsByYear();
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
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
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



int getConsoleWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 120; // Default fallback for Windows
#else
    return 120; // Default for non-Windows
#endif
}

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

// UTF-8 SUBSTRING BY VISIBLE CHARACTER INDICES
// ------------------------------------------------
string utf8_substr(const string& s, int start, int length) {
    if (start < 0 || length <= 0) return "";
    
    string result = "";
    int charCount = 0;
    int collected = 0;
    
    for (size_t i = 0; i < s.size() && collected < length; ) {
        unsigned char c = s[i];
        int len = 1;

        if ((c & 0x80) == 0) len = 1;           // ASCII
        else if ((c & 0xE0) == 0xC0) len = 2;  // 2-byte UTF-8
        else if ((c & 0xF0) == 0xE0) len = 3;  // 3-byte UTF-8
        else if ((c & 0xF8) == 0xF0) len = 4;  // 4-byte UTF-8

        if (charCount >= start) {
            result += s.substr(i, len);
            collected++;
        }
        
        i += len;
        charCount++;
    }
    
    return result;
}

vector<string> wrapUtf8(const string& s, int maxWidth) {
    vector<string> lines;
    if (maxWidth <= 0) return lines;
    
    int totalChars = utf8_width(s);
    if (totalChars <= maxWidth) {
        lines.push_back(s);
        return lines;
    }
    
    int start = 0;
    while (start < totalChars) {
        int chunkSize = min(maxWidth, totalChars - start);
        string chunk = utf8_substr(s, start, chunkSize);
        lines.push_back(chunk);
        start += chunkSize;
    }
    
    return lines;
}

void printSeparatorLine(const vector<int>& colWidths) {
    int totalWidth = 1; // Start with 1 for the first |
    for (size_t i = 0; i < colWidths.size(); i++) {
        totalWidth += colWidths[i] + 2; // content + space + |
    }
    cout << string(totalWidth, '-') << "\n";
}

void printWrappedRow(const vector<vector<string>>& wrappedCols, const vector<int>& colWidths) {
    int maxLines = 0;
    for (const auto& col : wrappedCols) {
        maxLines = max(maxLines, (int)col.size());
    }
    
    for (int line = 0; line < maxLines; line++) {
        cout << "|";
        for (size_t col = 0; col < wrappedCols.size(); col++) {
            string cellContent = "";
            if (line < wrappedCols[col].size()) {
                cellContent = wrappedCols[col][line];
            }
            
            int contentWidth = utf8_width(cellContent);
            int padding = colWidths[col] - contentWidth;
            
            cout << cellContent;
            cout << string(padding + 1, ' ');
            cout << "|";
        }
        cout << "\n";
    }
}

void printArray() {
    if (studentCount == 0) {
        cout << "Нет студентов.\n";
        return;
    }
    
    sortStudentsByYear();

    vector<string> headers = {
        "№", "Год", "Курс", "Имя", "Фамилия", "Отчество",
        "Предмет 1", "Оценки 1", "Предмет 2", "Оценки 2", "Предмет 3", "Оценки 3"
    };
    
    // Adjust max cell width for Windows console compatibility
    int consoleWidth = getConsoleWidth();
    int estimatedCols = headers.size();
    int maxTableWidth = max(80, consoleWidth - 5); // Reserve some margin
    int maxCellWidth = min(20, max(8, (maxTableWidth - estimatedCols * 3) / estimatedCols));
    const int MAX_CELL_WIDTH = maxCellWidth;
    vector<int> colWidths(headers.size(), 0);
    
    for (size_t i = 0; i < headers.size(); i++) {
        colWidths[i] = min(MAX_CELL_WIDTH, utf8_width(headers[i]));
    }
    
    for (int i = 0; i < studentCount; i++) {
        vector<string> rowData = {
            to_string(i + 1),                    // No
            to_string(students[i].year),         // Year  
            to_string(students[i].course),       // Course
            students[i].name,                    // Name
            students[i].surname,                 // Surname
            students[i].middleName,              // Middle Name
            students[i].subjects[0],             // Subject 1
            students[i].grades[0],               // Grades 1
            students[i].subjects[1],             // Subject 2
            students[i].grades[1],               // Grades 2
            students[i].subjects[2],             // Subject 3
            students[i].grades[2]                // Grades 3
        };
        
        for (size_t j = 0; j < rowData.size(); j++) {
            int contentWidth = utf8_width(rowData[j]);
            int requiredWidth = min(MAX_CELL_WIDTH, contentWidth);
            colWidths[j] = max(colWidths[j], requiredWidth);
        }
    }
    
    vector<vector<string>> headerWrapped(headers.size());
    for (size_t i = 0; i < headers.size(); i++) {
        headerWrapped[i] = wrapUtf8(headers[i], colWidths[i]);
    }
    printWrappedRow(headerWrapped, colWidths);
    
    printSeparatorLine(colWidths);
    
    for (int i = 0; i < studentCount; i++) {
        vector<string> rowData = {
            to_string(i + 1),                    // No
            to_string(students[i].year),         // Year  
            to_string(students[i].course),       // Course
            students[i].name,                    // Name
            students[i].surname,                 // Surname
            students[i].middleName,              // Middle Name
            students[i].subjects[0],             // Subject 1
            students[i].grades[0],               // Grades 1
            students[i].subjects[1],             // Subject 2
            students[i].grades[1],               // Grades 2
            students[i].subjects[2],             // Subject 3
            students[i].grades[2]                // Grades 3
        };
        
        vector<vector<string>> wrappedRow(rowData.size());
        for (size_t j = 0; j < rowData.size(); j++) {
            wrappedRow[j] = wrapUtf8(rowData[j], colWidths[j]);
        }
        
        printWrappedRow(wrappedRow, colWidths);
        
        printSeparatorLine(colWidths);
    }
}

void sortStudentsByYear() {
    for (int i = 0; i < studentCount - 1; i++) {
        for (int j = 0; j < studentCount - i - 1; j++) {
            if (students[j].year > students[j + 1].year) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
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

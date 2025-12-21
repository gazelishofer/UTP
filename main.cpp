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

const string FILE1_PATH = "forStudents.txt";
const string FILE2_PATH = "forStudents.bin";

void processChoice(int choice);

string toLowerUtf8(const string &s);
int utf8_width(const std::string &s);
void printPadded(const string &s, int width);
void expandArray();
void editStudent(int index);
void deleteStudent(int index);
void printArray();
int getConsoleWidth();
void addStudentToArray(const Student &student);
void sortStudentsByYear();
void sortStudents(int sortBy, bool ascending = true);
bool checkAvailability();
void saveToFile();
void loadFromFile();
void saveToBinaryFile();
void loadFromBinaryFile();
bool askPermission1();
bool askPermission2();
bool isNumber(string s);
bool checkGrades(string s);
bool parseIntWithLimit(const string &s, int maxLen, int &value);
bool isValidName(const string &s);
bool isValidCourse(int course);
bool isValidYear(int year);
bool isValidSubject(const string &s);
bool splitLine(const string &line, char delimiter, vector<string> &fields, int expectedFields);
bool parseGradesSumCount(const string &grades, int &sum, int &count);
double calcAverageGrade(const Student &student);

bool isNumber(string s)
{
    if (s.empty())
        return false;
    for (char c : s)
    {
        if (!isdigit(c))
            return false;
    }
    return true;
}

bool checkGrades(string s)
{
    if (s.empty())
        return false;

    int gradeCount = 0;
    string current = "";

    for (size_t i = 0; i < s.size(); i++)
    {
        char c = s[i];

        if (isdigit(c))
        {
            current.push_back(c);
        }
        else if (c == ',')
        {
            if (current.empty())
                return false;
            int g = 0;
            if (!parseIntWithLimit(current, 1, g))
                return false;
            if (g < 1 || g > 5)
                return false;
            gradeCount++;
            current.clear();
        }
        else
        {
            return false;
        }
    }

    if (current.empty())
        return false;
    int g = 0;
    if (!parseIntWithLimit(current, 1, g))
        return false;
    if (g < 1 || g > 5)
        return false;
    gradeCount++;

    return gradeCount == 3;
}

bool parseIntWithLimit(const string &s, int maxLen, int &value)
{
    if (s.empty() || (int)s.size() > maxLen)
        return false;
    for (char c : s)
    {
        if (!isdigit(c))
            return false;
    }
    value = stoi(s);
    return true;
}

bool isValidName(const string &s)
{
    if (s.empty())
        return false;

    for (size_t i = 0; i < s.size();)
    {
        unsigned char c = s[i];

        if (isdigit(c))
            return false;

        if (c == ' ')
        {
            i++;
            continue;
        }

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        {
            i++;
            continue;
        }

        if ((c & 0x80) == 0)
        {
            return false;
        }
        else if ((c & 0xC0) != 0x80)
        {
            int len = 1;
            if ((c & 0xE0) == 0xC0)
                len = 2;
            else if ((c & 0xF0) == 0xE0)
                len = 3;
            else if ((c & 0xF8) == 0xF0)
                len = 4;

            if (i + len > s.size())
                return false;

            i += len;
            continue;
        }

        return false;
    }
    return true;
}

bool isValidCourse(int course)
{
    return course >= 1 && course <= 6;
}

bool isValidYear(int year)
{
    return year >= 1930 && year <= 2010;
}

bool isValidSubject(const string &s)
{
    if (s.empty())
        return false;

    for (size_t i = 0; i < s.size();)
    {
        unsigned char c = s[i];

        if (isdigit(c))
            return false;

        if (c == ' ' || c == '.')
        {
            i++;
            continue;
        }

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        {
            i++;
            continue;
        }

        if ((c & 0x80) == 0)
        {
            return false;
        }
        else if ((c & 0xC0) != 0x80)
        {
            int len = 1;
            if ((c & 0xE0) == 0xC0)
                len = 2;
            else if ((c & 0xF0) == 0xE0)
                len = 3;
            else if ((c & 0xF8) == 0xF0)
                len = 4;

            if (i + len > s.size())
                return false;

            i += len;
            continue;
        }

        return false;
    }
    return true;
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::locale::global(std::locale(""));

    while (true)
    {
        int choice;
        cout << "\nМеню:\n";
        cout << "1) Добавить студента\n";
        cout << "2) Редактировать студента\n";
        cout << "3) Удалить студента\n";
        cout << "4) Сохранить в текстовый файл\n";
        cout << "5) Сохранить бинарный файл\n";
        cout << "6) Загрузить данные\n";
        cout << "7) Показать список\n";
        cout << "8) Сортировать студентов\n";
        cout << "9) Выход\n";
        cout << "Выберите пункт: ";
        cin >> choice;

        if (choice == 9)
            break;
        processChoice(choice);
    }
    return 0;
}

void processChoice(int choice)
{
    switch (choice)
    {

    case 1:
    {
        cout << "Добавление студента\n";
        Student student;
        string input;

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        while (true)
        {
            cout << "Введите имя (только русские или английские буквы): ";
            getline(cin, student.name);
            if (isValidName(student.name))
                break;
            cout << "Ошибка: имя может содержать только русские или английские буквы.\n";
        }

        while (true)
        {
            cout << "Введите фамилию (только русские или английские буквы): ";
            getline(cin, student.surname);
            if (isValidName(student.surname))
                break;
            cout << "Ошибка: фамилия может содержать только русские или английские буквы.\n";
        }

        while (true)
        {
            cout << "Введите отчество (только русские или английские буквы): ";
            getline(cin, student.middleName);
            if (isValidName(student.middleName))
                break;
            cout << "Ошибка: отчество может содержать только русские или английские буквы.\n";
        }

        while (true)
        {
            cout << "Введите год рождения (1930-2010): ";
            getline(cin, input);
            if (isNumber(input))
            {
                if ((int)input.size() > 4)
                {
                    cout << "Ошибка: год должен быть не больше 4 цифр.\n";
                    continue;
                }
                int year = stoi(input);
                if (isValidYear(year))
                {
                    student.year = year;
                    break;
                }
                else
                {
                    cout << "Ошибка: год должен быть в диапазоне 1930-2010.\n";
                }
            }
            else
            {
                cout << "Ошибка: введите число!\n";
            }
        }

        while (true)
        {
            cout << "Введите курс (1-6): ";
            getline(cin, input);
            if (isNumber(input))
            {
                if ((int)input.size() > 1)
                {
                    cout << "Ошибка: курс должен быть одной цифрой (1-6).\n";
                    continue;
                }
                int course = stoi(input);
                if (isValidCourse(course))
                {
                    student.course = course;
                    break;
                }
                else
                {
                    cout << "Ошибка: курс должен быть в диапазоне 1-6.\n";
                }
            }
            else
            {
                cout << "Ошибка: введите число!\n";
            }
        }

        for (int i = 0; i < 3; i++)
        {
            while (true)
            {
                cout << "Введите предмет " << i + 1 << " (только русские или английские буквы): ";
                getline(cin, student.subjects[i]);
                if (isValidSubject(student.subjects[i]))
                    break;
                cout << "Ошибка: название предмета может содержать только русские или английские буквы.\n";
            }

            while (true)
            {
                cout << "Введите 3 оценки (1–5, через запятую, например: 5,4,3): ";
                getline(cin, student.grades[i]);
                if (checkGrades(student.grades[i]))
                    break;
                cout << "Ошибка: введите ровно 3 оценки (числа 1–5 через запятую).\n";
            }
        }

        addStudentToArray(student);
        saveToFile();
        break;
    }

    case 2:
    {
        printArray();
        cout << "Введите номер студента для редактирования: ";
        int index;
        cin >> index;
        editStudent(index);
        break;
    }

    case 3:
    {
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
    {
        cout << "\nЗагрузить из:\n";
        cout << "1) Текстового файла\n";
        cout << "2) Бинарного файла\n";
        cout << "Выберите источник: ";

        int loadChoice;
        cin >> loadChoice;

        if (loadChoice == 1)
        {
            loadFromFile();
        }
        else if (loadChoice == 2)
        {
            loadFromBinaryFile();
        }
        else
        {
            cout << "Неверный выбор.\n";
        }
        break;
    }

    case 7:
        printArray();
        break;

    case 8:
    {
        cout << "\nСортировать по:\n";
        cout << "1) Году рождения\n";
        cout << "2) Курсу\n";
        cout << "3) Имени\n";
        cout << "4) Фамилии\n";
        cout << "5) Отчеству\n";
        cout << "6) Оценкам\n";
        cout << "Выберите поле для сортировки: ";

        int sortChoice;
        cin >> sortChoice;

        if (sortChoice >= 1 && sortChoice <= 6)
        {
            cout << "Порядок:\n";
            cout << "1) По возрастанию\n";
            cout << "2) По убыванию\n";
            cout << "Выберите порядок: ";

            int orderChoice;
            cin >> orderChoice;

            bool ascending = (orderChoice == 1);
            sortStudents(sortChoice, ascending);
            cout << "Студенты отсортированы.\n\n";
            printArray();
        }
        else
        {
            cout << "Неверный выбор.\n";
        }
        break;
    }

    default:
        cout << "Неверный пункт меню.\n";
        break;
    }
}

int getConsoleWidth()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 120;
#else
    return 120;
#endif
}

int utf8_width(const string &s)
{
    int count = 0;
    for (size_t i = 0; i < s.size();)
    {
        unsigned char c = s[i];
        int len = 1;

        if ((c & 0x80) == 0)
            len = 1;
        else if ((c & 0xE0) == 0xC0)
            len = 2;
        else if ((c & 0xF0) == 0xE0)
            len = 3;
        else if ((c & 0xF8) == 0xF0)
            len = 4;

        i += len;
        count++;
    }
    return count;
}

string utf8_substr(const string &s, int start, int length)
{
    if (start < 0 || length <= 0)
        return "";

    string result = "";
    int charCount = 0;
    int collected = 0;

    for (size_t i = 0; i < s.size() && collected < length;)
    {
        unsigned char c = s[i];
        int len = 1;

        if ((c & 0x80) == 0)
            len = 1;
        else if ((c & 0xE0) == 0xC0)
            len = 2;
        else if ((c & 0xF0) == 0xE0)
            len = 3;
        else if ((c & 0xF8) == 0xF0)
            len = 4;

        if (charCount >= start)
        {
            result += s.substr(i, len);
            collected++;
        }

        i += len;
        charCount++;
    }

    return result;
}

vector<string> wrapUtf8(const string &s, int maxWidth)
{
    vector<string> lines;
    if (maxWidth <= 0)
        return lines;

    int totalChars = utf8_width(s);
    if (totalChars <= maxWidth)
    {
        lines.push_back(s);
        return lines;
    }

    int start = 0;
    while (start < totalChars)
    {
        int chunkSize = min(maxWidth, totalChars - start);
        string chunk = utf8_substr(s, start, chunkSize);
        lines.push_back(chunk);
        start += chunkSize;
    }

    return lines;
}

void printSeparatorLine(const vector<int> &colWidths)
{
    int totalWidth = 1;
    for (size_t i = 0; i < colWidths.size(); i++)
    {
        totalWidth += colWidths[i] + 2;
    }
    cout << string(totalWidth, '-') << "\n";
}

void printWrappedRow(const vector<vector<string>> &wrappedCols, const vector<int> &colWidths)
{
    int maxLines = 0;
    for (const auto &col : wrappedCols)
    {
        maxLines = max(maxLines, (int)col.size());
    }

    for (int line = 0; line < maxLines; line++)
    {
        cout << "|";
        for (size_t col = 0; col < wrappedCols.size(); col++)
        {
            string cellContent = "";
            if (line < wrappedCols[col].size())
            {
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

void printArray()
{
    if (studentCount == 0)
    {
        cout << "Нет студентов.\n";
        return;
    }

    vector<string> headers = {
        "№", "Год", "Курс", "Имя", "Фамилия", "Отчество",
        "Предмет 1", "Оценки 1", "Предмет 2", "Оценки 2", "Предмет 3", "Оценки 3"};

    int consoleWidth = getConsoleWidth();
    int estimatedCols = headers.size();
    int maxTableWidth = max(80, consoleWidth - 5);
    int maxCellWidth = min(20, max(8, (maxTableWidth - estimatedCols * 3) / estimatedCols));
    const int MAX_CELL_WIDTH = maxCellWidth;
    vector<int> colWidths(headers.size(), 0);

    for (size_t i = 0; i < headers.size(); i++)
    {
        colWidths[i] = min(MAX_CELL_WIDTH, utf8_width(headers[i]));
    }

    for (int i = 0; i < studentCount; i++)
    {
        vector<string> rowData = {
            to_string(i + 1),
            to_string(students[i].year),
            to_string(students[i].course),
            students[i].name,
            students[i].surname,
            students[i].middleName,
            students[i].subjects[0],
            students[i].grades[0],
            students[i].subjects[1],
            students[i].grades[1],
            students[i].subjects[2],
            students[i].grades[2]};

        for (size_t j = 0; j < rowData.size(); j++)
        {
            int contentWidth = utf8_width(rowData[j]);
            int requiredWidth = min(MAX_CELL_WIDTH, contentWidth);
            colWidths[j] = max(colWidths[j], requiredWidth);
        }
    }

    vector<vector<string>> headerWrapped(headers.size());
    for (size_t i = 0; i < headers.size(); i++)
    {
        headerWrapped[i] = wrapUtf8(headers[i], colWidths[i]);
    }
    printWrappedRow(headerWrapped, colWidths);

    printSeparatorLine(colWidths);

    for (int i = 0; i < studentCount; i++)
    {
        vector<string> rowData = {
            to_string(i + 1),
            to_string(students[i].year),
            to_string(students[i].course),
            students[i].name,
            students[i].surname,
            students[i].middleName,
            students[i].subjects[0],
            students[i].grades[0],
            students[i].subjects[1],
            students[i].grades[1],
            students[i].subjects[2],
            students[i].grades[2]};

        vector<vector<string>> wrappedRow(rowData.size());
        for (size_t j = 0; j < rowData.size(); j++)
        {
            wrappedRow[j] = wrapUtf8(rowData[j], colWidths[j]);
        }

        printWrappedRow(wrappedRow, colWidths);

        printSeparatorLine(colWidths);
    }
}

void sortStudentsByYear()
{
    sortStudents(1);
}

void sortStudents(int sortBy, bool ascending)
{
    if (studentCount <= 1)
        return;

    for (int i = 0; i < studentCount - 1; i++)
    {
        for (int j = 0; j < studentCount - i - 1; j++)
        {
            bool shouldSwap = false;

            switch (sortBy)
            {
            case 1:
                shouldSwap = ascending ? (students[j].year > students[j + 1].year) : (students[j].year < students[j + 1].year);
                break;
            case 2:
                shouldSwap = ascending ? (students[j].course > students[j + 1].course) : (students[j].course < students[j + 1].course);
                break;
            case 3:
            {
                string name1 = toLowerUtf8(students[j].name);
                string name2 = toLowerUtf8(students[j + 1].name);
                shouldSwap = ascending ? (name1 > name2) : (name1 < name2);
                break;
            }
            case 4:
            {
                string surname1 = toLowerUtf8(students[j].surname);
                string surname2 = toLowerUtf8(students[j + 1].surname);
                shouldSwap = ascending ? (surname1 > surname2) : (surname1 < surname2);
                break;
            }
            case 5:
            {
                string middle1 = toLowerUtf8(students[j].middleName);
                string middle2 = toLowerUtf8(students[j + 1].middleName);
                shouldSwap = ascending ? (middle1 > middle2) : (middle1 < middle2);
                break;
            }
            case 6:
            {
                double avg1 = calcAverageGrade(students[j]);
                double avg2 = calcAverageGrade(students[j + 1]);
                shouldSwap = ascending ? (avg1 > avg2) : (avg1 < avg2);
                break;
            }
            default:
                cout << "Неверный параметр сортировки.\n";
                return;
            }

            if (shouldSwap)
            {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
}

bool checkAvailability()
{
    if (studentCount + 1 > capacity)
        expandArray();
    return true;
}

void addStudentToArray(const Student &student)
{
    checkAvailability();
    students[studentCount] = student;
    studentCount++;
    cout << "Студент успешно добавлен.\n";
}

void deleteStudent(int index)
{
    if (index < 0 || index >= studentCount)
    {
        cout << "Неверный номер студента.\n";
        return;
    }
    for (int i = index; i < studentCount - 1; i++)
        students[i] = students[i + 1];

    studentCount--;
    cout << "Студент удалён.\n";
    saveToFile();
}

void expandArray()
{
    int newCapacity = capacity * 2;
    Student *newStudents = new Student[newCapacity];
    for (int i = 0; i < capacity; i++)
        newStudents[i] = students[i];

    delete[] students;
    students = newStudents;
    capacity = newCapacity;

    cout << "Размер массива увеличен: " << capacity << "\n";
}

void editStudent(int index)
{
    if (index < 1 || index > studentCount)
    {
        cout << "Неверный номер студента.\n";
        return;
    }

    index--;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    while (true)
    {
        cout << "\nЧто хотите изменить?\n";
        cout << "1) Год рождения\n";
        cout << "2) Курс\n";
        cout << "3) Имя\n";
        cout << "4) Фамилия\n";
        cout << "5) Отчество\n";
        cout << "6) Предметы и оценки\n";
        cout << "7) Выход\n";
        cout << "Выберите пункт: ";

        int editChoice;
        cin >> editChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (editChoice)
        {

        case 1:
        {
            string input;
            while (true)
            {
                cout << "Введите новый год рождения (1930-2010): ";
                getline(cin, input);
                if (isNumber(input))
                {
                    if ((int)input.size() > 4)
                    {
                        cout << "Ошибка: год должен быть не больше 4 цифр.\n";
                        continue;
                    }
                    int year = stoi(input);
                    if (isValidYear(year))
                    {
                        break;
                    }
                    else
                    {
                        cout << "Ошибка: год должен быть в диапазоне 1930-2010.\n";
                    }
                }
                else
                {
                    cout << "Ошибка: введите число!\n";
                }
            }

            cout << "Изменить год? ";
            if (!askPermission1())
            {
                cout << "Отменено.\n";
                break;
            }

            students[index].year = stoi(input);
            saveToFile();
            cout << "Год рождения обновлён.\n";
            break;
        }

        case 2:
        {
            string input;
            while (true)
            {
                cout << "Введите новый курс (0-6): ";
                getline(cin, input);
                if (isNumber(input))
                {
                    if ((int)input.size() > 1)
                    {
                        cout << "Ошибка: курс должен быть одной цифрой (0-6).\n";
                        continue;
                    }
                    int course = stoi(input);
                    if (isValidCourse(course))
                    {
                        break;
                    }
                    else
                    {
                        cout << "Ошибка: курс должен быть в диапазоне 0-6.\n";
                    }
                }
                else
                {
                    cout << "Ошибка: введите число!\n";
                }
            }

            if (!askPermission1())
            {
                cout << "Отменено.\n";
                break;
            }

            students[index].course = stoi(input);
            saveToFile();
            cout << "Курс обновлён.\n";
            break;
        }

        case 3:
        {
            string name;
            while (true)
            {
                cout << "Введите новое имя (только русские или английские буквы): ";
                getline(cin, name);
                if (isValidName(name))
                    break;
                cout << "Ошибка: имя может содержать только русские или английские буквы.\n";
            }

            if (!askPermission1())
                break;

            students[index].name = name;
            saveToFile();
            cout << "Имя обновлено.\n";
            break;
        }

        case 4:
        {
            string surname;
            while (true)
            {
                cout << "Введите новую фамилию (только русские или английские буквы): ";
                getline(cin, surname);
                if (isValidName(surname))
                    break;
                cout << "Ошибка: фамилия может содержать только русские или английские буквы.\n";
            }

            if (!askPermission1())
                break;

            students[index].surname = surname;
            saveToFile();
            cout << "Фамилия обновлена.\n";
            break;
        }

        case 5:
        {
            string middle;
            while (true)
            {
                cout << "Введите новое отчество (только русские или английские буквы): ";
                getline(cin, middle);
                if (isValidName(middle))
                    break;
                cout << "Ошибка: отчество может содержать только русские или английские буквы.\n";
            }

            if (!askPermission1())
                break;

            students[index].middleName = middle;
            saveToFile();
            cout << "Отчество обновлено.\n";
            break;
        }

        case 6:
        {
            Student backup = students[index];

            for (int i = 0; i < 3; i++)
            {
                while (true)
                {
                    cout << "Введите предмет " << i + 1 << " (только русские или английские буквы): ";
                    getline(cin, students[index].subjects[i]);
                    if (isValidSubject(students[index].subjects[i]))
                        break;
                    cout << "Ошибка: название предмета может содержать только русские или английские буквы.\n";
                }

                while (true)
                {
                    cout << "Введите 3 оценки (1–5, через запятую, например: 5,4,3): ";
                    getline(cin, students[index].grades[i]);

                    if (checkGrades(students[index].grades[i]))
                        break;

                    cout << "Ошибка: введите ровно 3 оценки (числа 1–5 через запятую).\n";
                }
            }

            if (!askPermission1())
            {
                students[index] = backup;
                cout << "Отменено.\n";
                break;
            }

            saveToFile();
            cout << "Предметы и оценки обновлены.\n";
            break;
        }

        case 7:
        {
            bool toMenu = askPermission2();
            if (toMenu)
                return;
            else
                break;
        }

        default:
            cout << "Неверный пункт.\n";
        }
    }
}

void saveToFile()
{
    ofstream fout(FILE1_PATH);
    if (!fout)
    {
        cout << "Ошибка: не удалось открыть файл для записи.\n";
        return;
    }
    for (int i = 0; i < studentCount; i++)
    {
        fout << students[i].year << "|"
             << students[i].course << "|"
             << students[i].name << "|"
             << students[i].surname << "|"
             << students[i].middleName << "|";
        for (int j = 0; j < 3; j++)
        {
            fout << students[i].subjects[j] << "|" << students[i].grades[j];
            if (j != 2)
                fout << "|";
        }
        fout << "\n";
    }
    fout.close();
    cout << "Текстовый файл сохранён.\n";
}

void loadFromFile()
{
    ifstream fin(FILE1_PATH);
    if (!fin)
    {
        cout << "Ошибка: текстовый файл не найден.\n";
        return;
    }
    studentCount = 0;
    string line;
    while (getline(fin, line))
    {
        if (line.empty())
            continue;

        vector<string> fields;
        if (!splitLine(line, '|', fields, 11))
        {
            cout << "Ошибка: неверный формат строки в файле.\n";
            continue;
        }

        students[studentCount].year = stoi(fields[0]);
        students[studentCount].course = stoi(fields[1]);
        students[studentCount].name = fields[2];
        students[studentCount].surname = fields[3];
        students[studentCount].middleName = fields[4];
        students[studentCount].subjects[0] = fields[5];
        students[studentCount].grades[0] = fields[6];
        students[studentCount].subjects[1] = fields[7];
        students[studentCount].grades[1] = fields[8];
        students[studentCount].subjects[2] = fields[9];
        students[studentCount].grades[2] = fields[10];

        studentCount++;
        if (studentCount >= capacity)
            expandArray();
    }
    fin.close();
    sortStudentsByYear();
    cout << "Текстовый файл загружен.\n";
}

bool splitLine(const string &line, char delimiter, vector<string> &fields, int expectedFields)
{
    fields.clear();
    string current;
    for (size_t i = 0; i < line.size(); i++)
    {
        char c = line[i];
        if (c == delimiter)
        {
            fields.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(c);
        }
    }
    fields.push_back(current);
    return (int)fields.size() == expectedFields;
}

bool parseGradesSumCount(const string &grades, int &sum, int &count)
{
    sum = 0;
    count = 0;
    string current;
    for (size_t i = 0; i < grades.size(); i++)
    {
        char c = grades[i];
        if (isdigit(c))
        {
            current.push_back(c);
            continue;
        }
        if (c == ',')
        {
            if (current.empty())
                return false;
            int value = stoi(current);
            sum += value;
            count++;
            current.clear();
            continue;
        }
        return false;
    }
    if (current.empty())
        return false;
    sum += stoi(current);
    count++;
    return count > 0;
}

double calcAverageGrade(const Student &student)
{
    int totalSum = 0;
    int totalCount = 0;
    for (int i = 0; i < 3; i++)
    {
        int sum = 0;
        int count = 0;
        if (parseGradesSumCount(student.grades[i], sum, count))
        {
            totalSum += sum;
            totalCount += count;
        }
    }
    if (totalCount == 0)
        return 0.0;
    return static_cast<double>(totalSum) / static_cast<double>(totalCount);
}

string toLowerUtf8(const string &s)
{
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    wstring ws = conv.from_bytes(s);

    for (auto &c : ws)
        c = towlower(c);

    return conv.to_bytes(ws);
}

bool askPermission1()
{
    while (true)
    {
        cout << "Вы уверены? (y/n): ";

        char answer;
        if (!(cin >> answer))
        {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        cin.ignore(10000, '\n');
        answer = tolower((unsigned char)answer);

        if (answer == 'y')
            return true;
        if (answer == 'n')
            return false;

        cout << "Введите только y или n.\n";
    }
}

bool askPermission2()
{
    cout << "Выйти на главное меню или на предыдущий шаг? (m/p): ";
    char answer;
    if (!(cin >> answer))
    {
        cin.clear();
        cin.ignore(10000, '\n');
        return false;
    }
    cin.ignore(10000, '\n');
    answer = tolower((unsigned char)answer);
    return answer == 'm';
}

void saveToBinaryFile()
{
    ofstream fout(FILE2_PATH, ios::binary);
    if (!fout)
    {
        cout << "Ошибка записи бинарного файла.\n";
        return;
    }
    fout.write((char *)&studentCount, sizeof(studentCount));
    for (int i = 0; i < studentCount; i++)
    {
        fout.write((char *)&students[i].year, sizeof(students[i].year));
        fout.write((char *)&students[i].course, sizeof(students[i].course));

        int nameLen = students[i].name.length();
        fout.write((char *)&nameLen, sizeof(nameLen));
        fout.write(students[i].name.c_str(), nameLen);

        int surnameLen = students[i].surname.length();
        fout.write((char *)&surnameLen, sizeof(surnameLen));
        fout.write(students[i].surname.c_str(), surnameLen);

        int middleNameLen = students[i].middleName.length();
        fout.write((char *)&middleNameLen, sizeof(middleNameLen));
        fout.write(students[i].middleName.c_str(), middleNameLen);

        for (int j = 0; j < 3; j++)
        {
            int subjectLen = students[i].subjects[j].length();
            fout.write((char *)&subjectLen, sizeof(subjectLen));
            fout.write(students[i].subjects[j].c_str(), subjectLen);

            int gradeLen = students[i].grades[j].length();
            fout.write((char *)&gradeLen, sizeof(gradeLen));
            fout.write(students[i].grades[j].c_str(), gradeLen);
        }
    }
    fout.close();
    cout << "Бинарный файл сохранён.\n";
}

void loadFromBinaryFile()
{
    ifstream fin(FILE2_PATH, ios::binary);
    if (!fin)
    {
        cout << "Бинарный файл не найден.\n";
        return;
    }
    int countFromFile = 0;
    fin.read((char *)&countFromFile, sizeof(countFromFile));

    if (countFromFile > capacity)
    {
        delete[] students;
        students = new Student[countFromFile];
        capacity = countFromFile;
    }

    for (int i = 0; i < countFromFile; i++)
    {
        fin.read((char *)&students[i].year, sizeof(students[i].year));
        fin.read((char *)&students[i].course, sizeof(students[i].course));

        int nameLen = 0;
        fin.read((char *)&nameLen, sizeof(nameLen));
        students[i].name.resize(nameLen);
        fin.read(&students[i].name[0], nameLen);

        int surnameLen = 0;
        fin.read((char *)&surnameLen, sizeof(surnameLen));
        students[i].surname.resize(surnameLen);
        fin.read(&students[i].surname[0], surnameLen);

        int middleNameLen = 0;
        fin.read((char *)&middleNameLen, sizeof(middleNameLen));
        students[i].middleName.resize(middleNameLen);
        fin.read(&students[i].middleName[0], middleNameLen);

        for (int j = 0; j < 3; j++)
        {
            int subjectLen = 0;
            fin.read((char *)&subjectLen, sizeof(subjectLen));
            students[i].subjects[j].resize(subjectLen);
            fin.read(&students[i].subjects[j][0], subjectLen);

            int gradeLen = 0;
            fin.read((char *)&gradeLen, sizeof(gradeLen));
            students[i].grades[j].resize(gradeLen);
            fin.read(&students[i].grades[j][0], gradeLen);
        }
    }

    studentCount = countFromFile;
    fin.close();
    sortStudentsByYear();
    cout << "Бинарный файл загружен.\n";
}

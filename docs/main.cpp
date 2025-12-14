// Main program implementing student management with text and binary storage.
// Every line is commented to explain purpose and flow for a first-year student.

#include <vector> // std::vector is used for column width calculations.
#include <filesystem> // Included for potential filesystem utilities (not heavily used).
#include <iomanip> // For formatting output if needed.
#include <limits> // Provides numeric_limits for input clearing.
#ifdef _WIN32 // Platform-specific includes for Windows console handling.
#include <windows.h> // Windows API for UTF-8 console setup.
#include <io.h> // Low-level I/O on Windows.
#endif
#include <locale> // To set global locale so Russian text displays correctly.
#include <iostream> // Standard I/O streams.
#include <algorithm> // For sorting students.
#include <fstream> // For file I/O (text and binary files).
#include "Student.h" // Include our Student struct definition.
#include <string> // For std::string throughout the program.
#include <codecvt> // For UTF-8 string conversions when sorting.

using namespace std; // Bring standard names into global namespace for brevity.

Student *students = new Student[10]; // Dynamic array to store students; starts with capacity 10.

int studentCount = 0; // How many students are currently stored.
int capacity = 10; // Current allocated size of the students array.

const string FILE1_PATH = "forStudents.txt"; // Path for plain text storage.
const string FILE2_PATH = "forStudents.bin"; // Path for binary storage.

void processChoice(int choice); // Forward declaration for handling menu selection.

string toLowerUtf8(const string& s); // Convert UTF-8 string to lowercase for comparisons.
int utf8_width(const std::string& s); // Count characters in UTF-8 string (not bytes).
void printPadded(const string& s, int width); // Declared but not used; would pad strings.
void expandArray(); // Double the capacity when we need more space for students.
void editStudent(int index); // Allow editing of a chosen student.
void deleteStudent(int index); // Remove a student at given index.
void printArray(); // Display the table of students.
int getConsoleWidth(); // Detect console width (roughly) for formatting.
void addStudentToArray(const Student &student); // Append a student to the array, resizing if needed.
void sortStudentsByYear(); // Helper to sort by birth year.
void sortStudents(int sortBy); // Sort based on a chosen field.
bool checkAvailability(); // Ensure there is capacity for another student.
void saveToFile(); // Save students to text file.
void loadFromFile(); // Load students from text file.
void saveToBinaryFile(); // Save students to binary file.
void loadFromBinaryFile(); // Load students from binary file.
bool askPermission1(); // Prompt for yes/no confirmation.
bool askPermission2(); // Prompt whether to go to menu or previous step.
bool isNumber(string s); // Validate that a string contains only digits.
bool checkGrades(string s); // Validate grade input format.
bool parseIntWithLimit(const string& s, int maxLen, int &value); // Convert to int only if length is within limit and digits only.
bool isValidName(const string& s); // Validate names (letters only, English/Russian).
bool isValidCourse(int course); // Validate course number range.
bool isValidYear(int year); // Validate birth year range.
bool isValidSubject(const string& s); // Validate subject names.

bool isNumber(string s) { // Check if string is purely numeric.
    if (s.empty()) return false; // Empty string is not a number.
    for (char c : s) { // Iterate through each character.
        if (!isdigit(c)) return false; // If any char is not digit, fail.
    }
    return true; // All digits => valid number.
}

bool checkGrades(string s) { // Validate grade string like "5,4,3".
    if (s.empty()) return false; // Empty input invalid.

    int gradeCount = 0; // Counts how many grades found.
    string current = ""; // Accumulates digits for current grade.

    for (size_t i = 0; i < s.size(); i++) { // Loop through characters.
        char c = s[i]; // Current character.

        if (isdigit(c)) { // If character is a digit...
            current.push_back(c); // Append to current grade string.
        } else if (c == ',') { // If we hit a comma delimiter...
            if (current.empty()) return false; // No digits before comma => invalid like ",3" or "3,,3".
            bool ok = false; // Track conversion success.
            int g = 0; // Parsed grade value.
            ok = parseIntWithLimit(current, 1, g); // Parse only if one digit and all digits.
            if (!ok || g < 1 || g > 5) return false; // Must be valid int and between 1 and 5.
            gradeCount++; // Count this completed grade.
            current.clear(); // Reset for next grade.
        } else {
            return false; // Any other character (spaces, letters) invalidates input.
        }
    }

    if (current.empty()) return false; // Trailing comma or missing last grade invalid.
    bool ok = false; // Track conversion success.
    int g = 0; // Parsed grade.
    ok = parseIntWithLimit(current, 1, g); // Parse with length check.
    if (!ok || g < 1 || g > 5) return false; // Must be valid and in range.
    gradeCount++; // Count final grade.

    return gradeCount == 3; // Require exactly three grades (two commas).
}

bool parseIntWithLimit(const string& s, int maxLen, int &value) { // Parse integer if length is within limit and digits-only.
    if (s.empty() || (int)s.size() > maxLen) return false; // Reject empty or too long strings.
    for (char c : s) { // Check every character.
        if (!isdigit(c)) return false; // Non-digit => invalid.
    }
    value = stoi(s); // Safe to convert because we enforced length and digits.
    return true; // Indicate success.
}

bool isValidName(const string& s) { // Validate names allowing ASCII letters or UTF-8 letters.
    if (s.empty()) return false; // Name cannot be empty.

    for (size_t i = 0; i < s.size(); ) { // Walk through bytes with manual UTF-8 detection.
        unsigned char c = s[i]; // Current byte.

        if (isdigit(c)) return false; // Digits are not allowed in names.

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) { // ASCII letters allowed.
            i++; // Move to next byte.
            continue; // Continue validation.
        }

        if ((c & 0x80) == 0) { // Single-byte non-ASCII (control/ punctuation) disallowed.
            return false; // Reject.
        } else if ((c & 0xC0) != 0x80) { // Start of multibyte UTF-8 sequence (for Cyrillic, etc.).
            int len = 1; // Length of multibyte sequence.
            if ((c & 0xE0) == 0xC0) len = 2; // Two-byte sequence.
            else if ((c & 0xF0) == 0xE0) len = 3; // Three-byte sequence.
            else if ((c & 0xF8) == 0xF0) len = 4; // Four-byte sequence.

            if (i + len > s.size()) return false; // If declared length exceeds string, invalid UTF-8.

            i += len; // Skip the full multibyte character.
            continue; // Continue scanning.
        }

        return false; // Continuation byte without start => invalid.
    }
    return true; // All characters valid.
}

bool isValidCourse(int course) { // Validate that course is between 0 and 6.
    return course >= 0 && course <= 6; // Return true only within range.
}

bool isValidYear(int year) { // Validate that birth year is reasonable.
    return year >= 1930 && year <= 2010; // Accept years from 1930 to 2010.
}

bool isValidSubject(const string& s) { // Validate subject names similarly to names.
    if (s.empty()) return false; // Empty subject invalid.

    for (size_t i = 0; i < s.size(); ) { // Traverse bytes.
        unsigned char c = s[i]; // Current byte.

        if (isdigit(c)) return false; // Digits not allowed.

        if (c == ' ') { // Allow spaces inside subject names (e.g., two-word courses).
            i++; // Move past space.
            continue; // Continue validation.
        }

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) { // ASCII letters allowed.
            i++; // Advance one byte.
            continue; // Continue.
        }

        if ((c & 0x80) == 0) { // Single-byte non-ASCII disallowed.
            return false; // Reject.
        } else if ((c & 0xC0) != 0x80) { // Start of UTF-8 multibyte.
            int len = 1; // Length placeholder.
            if ((c & 0xE0) == 0xC0) len = 2; // Two-byte character.
            else if ((c & 0xF0) == 0xE0) len = 3; // Three-byte character.
            else if ((c & 0xF8) == 0xF0) len = 4; // Four-byte character.

            if (i + len > s.size()) return false; // Invalid if length exceeds remaining bytes.

            i += len; // Skip full character.
            continue; // Keep scanning.
        }

        return false; // Continuation byte without start => invalid.
    }
    return true; // All characters valid.
}

int main() { // Program entry point.
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // On Windows, set console output code page to UTF-8.
    SetConsoleCP(CP_UTF8); // On Windows, set console input code page to UTF-8.
#endif
    std::locale::global(std::locale("")); // Set global locale to user default for proper text handling.

    while (true) { // Main menu loop runs until user chooses to exit.
        int choice; // Stores user menu selection.
        cout << "\nМеню:\n"; // Display menu title.
        cout << "1) Добавить студента\n"; // Option to add a student.
        cout << "2) Редактировать студента\n"; // Option to edit a student.
        cout << "3) Удалить студента\n"; // Option to delete a student.
        cout << "4) Сохранить в текстовый файл\n"; // Option to save to text file.
        cout << "5) Сохранить бинарный файл\n"; // Option to save to binary file.
        cout << "6) Загрузить данные\n"; // Option to load data from file.
        cout << "7) Показать список\n"; // Option to print student list.
        cout << "8) Сортировать студентов\n"; // Option to sort students.
        cout << "9) Выход\n"; // Option to exit program.
        cout << "Выберите пункт: "; // Prompt for user input.
        cin >> choice; // Read menu choice.

        if (choice == 9) break; // If user chooses 9, exit loop and program.
        processChoice(choice); // Handle the chosen action.
    }
    return 0; // End of program.
}

void processChoice(int choice) { // Dispatch actions based on menu selection.
    switch (choice) { // Evaluate the selected option.

        case 1: { // Add new student.
            cout << "Добавление студента\n"; // Inform user.
            Student student; // Temporary student object to fill.
            string input; // Helper string for numeric entries.

            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear leftover newline in input buffer.

            while (true) { // Loop until valid name.
                cout << "Введите имя (только русские или английские буквы): "; // Prompt for name.
                getline(cin, student.name); // Read full line for name.
                if (isValidName(student.name)) break; // Accept when valid.
                cout << "Ошибка: имя может содержать только русские или английские буквы.\n"; // Error message.
            }

            while (true) { // Loop until valid surname.
                cout << "Введите фамилию (только русские или английские буквы): "; // Prompt.
                getline(cin, student.surname); // Read surname.
                if (isValidName(student.surname)) break; // Validate.
                cout << "Ошибка: фамилия может содержать только русские или английские буквы.\n"; // Error.
            }

            while (true) { // Loop until valid middle name.
                cout << "Введите отчество (только русские или английские буквы): "; // Prompt.
                getline(cin, student.middleName); // Read middle name.
                if (isValidName(student.middleName)) break; // Validate.
                cout << "Ошибка: отчество может содержать только русские или английские буквы.\n"; // Error.
            }

            while (true) { // Loop until valid birth year.
                cout << "Введите год рождения (1930-2010): "; // Prompt for year.
                getline(cin, input); // Read as string to validate.
                if (isNumber(input)) { // Check digits only.
                    bool ok = false; // Track conversion success.
                    if ((int)input.size() > 4) { // Reject overly long year strings.
                        cout << "Ошибка: год должен быть не больше 4 цифр.\n"; // Explain limit.
                        continue; // Ask again.
                    }
                    int year = stoi(input); // Convert digits to int directly.
                    ok = true; // Consider conversion successful here.
                    if (ok && isValidYear(year)) { // Check range when conversion is fine.
                        student.year = year; // Store when valid.
                        break; // Exit loop.
                    } else {
                        cout << "Ошибка: год должен быть в диапазоне 1930-2010.\n"; // Out-of-range or invalid.
                    }
                } else {
                    cout << "Ошибка: введите число!\n"; // Non-digit characters.
                }
            }

            while (true) { // Loop until valid course.
                cout << "Введите курс (0-6): "; // Prompt for course number.
                getline(cin, input); // Read input as string.
                if (isNumber(input)) { // Validate digits.
                    bool ok = false; // Track conversion success.
                    if ((int)input.size() > 1) { // Course should be a single digit.
                        cout << "Ошибка: курс должен быть одной цифрой (0-6).\n"; // Explain limit.
                        continue; // Ask again.
                    }
                    int course = stoi(input); // Convert digits to int directly.
                    ok = true; // Consider conversion successful here.
                    if (ok && isValidCourse(course)) { // Range check.
                        student.course = course; // Store course.
                        break; // Exit loop.
                    } else {
                        cout << "Ошибка: курс должен быть в диапазоне 0-6.\n"; // Range or conversion error.
                    }
                } else {
                    cout << "Ошибка: введите число!\n"; // Non-digit input.
                }
            }

            for (int i = 0; i < 3; i++) { // Collect three subjects and grades.
                while (true) { // Validate subject name.
                    cout << "Введите предмет " << i + 1 << " (только русские или английские буквы): "; // Prompt with index.
                    getline(cin, student.subjects[i]); // Read subject.
                    if (isValidSubject(student.subjects[i])) break; // Accept when valid.
                    cout << "Ошибка: название предмета может содержать только русские или английские буквы.\n"; // Error message.
                }

                while (true) { // Validate grades for this subject.
                    cout << "Введите 3 оценки (1–5, через запятую, например: 5,4,3): "; // Prompt with example.
                    getline(cin, student.grades[i]); // Read grade string.
                    if (checkGrades(student.grades[i])) break; // Accept when format correct.
                    cout << "Ошибка: введите ровно 3 оценки (числа 1–5 через запятую).\n"; // Explain error.
                }
            }

            addStudentToArray(student); // Store new student into array.
            saveToFile(); // Immediately persist to text file.
            break; // Finish case 1.
        }

        case 2: { // Edit existing student.
            printArray(); // Show list so user can choose.
            cout << "Введите номер студента для редактирования: "; // Prompt for index.
            int index; // Variable to store choice.
            cin >> index; // Read index (1-based expected).
            editStudent(index); // Call edit routine.
            break; // End case.
        }

        case 3: { // Delete a student.
            printArray(); // Display list for reference.
            cout << "Введите номер студента для удаления: "; // Prompt.
            int index; // Store chosen index.
            cin >> index; // Read user input.
            deleteStudent(index - 1); // Convert to 0-based and delete.
            break; // End case.
        }

        case 4: // Save to text file.
            saveToFile(); // Call function to write text file.
            break; // End case.

        case 5: // Save to binary file.
            saveToBinaryFile(); // Call function to write binary file.
            break; // End case.

        case 6: { // Load data from chosen file type.
            cout << "\nЗагрузить из:\n"; // Prompt text.
            cout << "1) Текстового файла\n"; // Option 1 text file.
            cout << "2) Бинарного файла\n"; // Option 2 binary file.
            cout << "Выберите источник: "; // Prompt for choice.

            int loadChoice; // Variable to store selection.
            cin >> loadChoice; // Read choice.

            if (loadChoice == 1) { // If user chooses text file...
                loadFromFile(); // Load from text.
            } else if (loadChoice == 2) { // If chooses binary...
                loadFromBinaryFile(); // Load from binary.
            } else {
                cout << "Неверный выбор.\n"; // Invalid option feedback.
            }
            break; // End case.
        }

        case 7: // Print student list.
            printArray(); // Display table.
            break; // End case.

        case 8: { // Sort students.
            cout << "\nСортировать по:\n"; // Sorting menu header.
            cout << "1) Году рождения\n"; // Option: year.
            cout << "2) Курсу\n"; // Option: course.
            cout << "3) Имени\n"; // Option: name.
            cout << "4) Фамилии\n"; // Option: surname.
            cout << "5) Отчеству\n"; // Option: middle name.
            cout << "Выберите поле для сортировки: "; // Prompt.

            int sortChoice; // Store user selection.
            cin >> sortChoice; // Read selection.

            if (sortChoice >= 1 && sortChoice <= 5) { // Validate choice in range.
                sortStudents(sortChoice); // Perform sort based on choice.
                cout << "Студенты отсортированы.\n\n"; // Confirmation.
                printArray(); // Show sorted list.
            } else {
                cout << "Неверный выбор.\n"; // Error for invalid choice.
            }
            break; // End case.
        }

        default: // Handles any unexpected menu option.
            cout << "Неверный пункт меню.\n"; // Inform user.
            break; // End default.
    }
}



int getConsoleWidth() { // Attempt to get console width for formatting table.
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi; // Structure to hold console info.
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) { // If call succeeds...
        return csbi.srWindow.Right - csbi.srWindow.Left + 1; // Compute width.
    }
    return 120; // Fallback width if call fails.
#else
    return 120; // On non-Windows, use default width.
#endif
}

int utf8_width(const string &s) { // Count the number of characters in a UTF-8 string.
    int count = 0; // Character counter.
    for (size_t i = 0; i < s.size(); ) { // Iterate through bytes manually.
        unsigned char c = s[i]; // Current byte.
        int len = 1; // Default length for ASCII.

        if ((c & 0x80) == 0) len = 1; // ASCII character.
        else if ((c & 0xE0) == 0xC0) len = 2; // Two-byte character.
        else if ((c & 0xF0) == 0xE0) len = 3; // Three-byte character.
        else if ((c & 0xF8) == 0xF0) len = 4; // Four-byte character.

        i += len; // Skip the full character bytes.
        count++; // Count one character.
    }
    return count; // Return number of characters.
}


string utf8_substr(const string& s, int start, int length) { // Extract substring by character count in UTF-8.
    if (start < 0 || length <= 0) return ""; // Guard against invalid parameters.
    
    string result = ""; // Store extracted substring.
    int charCount = 0; // Count traversed characters.
    int collected = 0; // Count collected characters.
    
    for (size_t i = 0; i < s.size() && collected < length; ) { // Iterate until enough chars collected.
        unsigned char c = s[i]; // Current byte.
        int len = 1; // Default byte length.

        if ((c & 0x80) == 0) len = 1; // ASCII.
        else if ((c & 0xE0) == 0xC0) len = 2; // Two-byte.
        else if ((c & 0xF0) == 0xE0) len = 3; // Three-byte.
        else if ((c & 0xF8) == 0xF0) len = 4; // Four-byte.

        if (charCount >= start) { // If we are within desired start...
            result += s.substr(i, len); // Append this character bytes to result.
            collected++; // Count collected character.
        }
        
        i += len; // Move to next character.
        charCount++; // Increment traversed count.
    }
    
    return result; // Return constructed substring.
}

vector<string> wrapUtf8(const string& s, int maxWidth) { // Wrap a UTF-8 string into lines of at most maxWidth characters.
    vector<string> lines; // Resulting lines.
    if (maxWidth <= 0) return lines; // No width means no lines.
    
    int totalChars = utf8_width(s); // Count characters in input.
    if (totalChars <= maxWidth) { // If fits already...
        lines.push_back(s); // Store single line.
        return lines; // Return early.
    }
    
    int start = 0; // Starting character index for each chunk.
    while (start < totalChars) { // Continue until all characters consumed.
        int chunkSize = min(maxWidth, totalChars - start); // Determine next chunk size.
        string chunk = utf8_substr(s, start, chunkSize); // Extract substring for this line.
        lines.push_back(chunk); // Add to result lines.
        start += chunkSize; // Move start to next segment.
    }
    
    return lines; // Return wrapped lines.
}

void printSeparatorLine(const vector<int>& colWidths) { // Print a separator line using column widths.
    int totalWidth = 1; // Start with left border.
    for (size_t i = 0; i < colWidths.size(); i++) { // For each column...
        totalWidth += colWidths[i] + 2; // Add column width plus spaces and border.
    }
    cout << string(totalWidth, '-') << "\n"; // Output separator line of dashes.
}

void printWrappedRow(const vector<vector<string>>& wrappedCols, const vector<int>& colWidths) { // Print a row with wrapped cell contents.
    int maxLines = 0; // Track the maximum number of wrapped lines among columns.
    for (const auto& col : wrappedCols) { // Inspect each column's wrapped lines.
        maxLines = max(maxLines, (int)col.size()); // Update maxLines if needed.
    }
    
    for (int line = 0; line < maxLines; line++) { // Print each wrapped line layer.
        cout << "|"; // Left border.
        for (size_t col = 0; col < wrappedCols.size(); col++) { // For each column...
            string cellContent = ""; // Default empty content.
            if (line < wrappedCols[col].size()) { // If this column has content for this line...
                cellContent = wrappedCols[col][line]; // Use it.
            }
            
            int contentWidth = utf8_width(cellContent); // Compute displayed width.
            int padding = colWidths[col] - contentWidth; // Remaining space for padding.
            
            cout << cellContent; // Output the cell content.
            cout << string(padding + 1, ' '); // Pad with spaces (one extra for separation).
            cout << "|"; // Column border.
        }
        cout << "\n"; // End of line.
    }
}

void printArray() { // Display all students in a table with wrapping.
    if (studentCount == 0) { // If no students...
        cout << "Нет студентов.\n"; // Inform user.
        return; // Nothing to print.
    }

    vector<string> headers = { // Column headers for table.
        "№", "Год", "Курс", "Имя", "Фамилия", "Отчество",
        "Предмет 1", "Оценки 1", "Предмет 2", "Оценки 2", "Предмет 3", "Оценки 3"
    };
    

    int consoleWidth = getConsoleWidth(); // Determine available console width.
    int estimatedCols = headers.size(); // Number of columns.
    int maxTableWidth = max(80, consoleWidth - 5); // Choose a reasonable table width.
    int maxCellWidth = min(20, max(8, (maxTableWidth - estimatedCols * 3) / estimatedCols)); // Derive maximum width per cell.
    const int MAX_CELL_WIDTH = maxCellWidth; // Constant for clarity.
    vector<int> colWidths(headers.size(), 0); // Store calculated widths per column.
    
    for (size_t i = 0; i < headers.size(); i++) { // Initialize widths with header sizes.
        colWidths[i] = min(MAX_CELL_WIDTH, utf8_width(headers[i])); // Cap at max cell width.
    }
    
    for (int i = 0; i < studentCount; i++) { // Measure content widths for each student.
        vector<string> rowData = { // Gather row data for student.
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
            students[i].grades[2]
        };
        
        for (size_t j = 0; j < rowData.size(); j++) { // Update column widths based on content.
            int contentWidth = utf8_width(rowData[j]); // Character width of content.
            int requiredWidth = min(MAX_CELL_WIDTH, contentWidth); // Cap to max width.
            colWidths[j] = max(colWidths[j], requiredWidth); // Keep the maximum width observed.
        }
    }
    
    vector<vector<string>> headerWrapped(headers.size()); // Wrapped header lines.
    for (size_t i = 0; i < headers.size(); i++) { // Wrap each header if needed.
        headerWrapped[i] = wrapUtf8(headers[i], colWidths[i]); // Wrap based on width.
    }
    printWrappedRow(headerWrapped, colWidths); // Print header row.
    
    printSeparatorLine(colWidths); // Print separator line after headers.
    
    for (int i = 0; i < studentCount; i++) { // Print each student row.
        vector<string> rowData = { // Collect row data similar to before.
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
            students[i].grades[2]
        };
        
        vector<vector<string>> wrappedRow(rowData.size()); // Store wrapped lines for each cell.
        for (size_t j = 0; j < rowData.size(); j++) { // Wrap each cell to column width.
            wrappedRow[j] = wrapUtf8(rowData[j], colWidths[j]); // Wrap content.
        }
        
        printWrappedRow(wrappedRow, colWidths); // Print wrapped row.
        
        printSeparatorLine(colWidths); // Print separator after each row.
    }
}

void sortStudentsByYear() { // Helper to sort by birth year.
    sortStudents(1); // Call general sort with sortBy code 1.
}

void sortStudents(int sortBy) { // General sorting function based on requested field.
    if (studentCount <= 1) return; // Nothing to do with 0 or 1 student.

    for (int i = 0; i < studentCount - 1; i++) { // Outer bubble sort loop.
        for (int j = 0; j < studentCount - i - 1; j++) { // Inner loop compares adjacent elements.
            bool shouldSwap = false; // Flag to decide if swap needed.

            switch (sortBy) { // Choose comparison based on sortBy code.
                case 1:
                    shouldSwap = students[j].year > students[j + 1].year; // Swap if out of order by year.
                    break;
                case 2:
                    shouldSwap = students[j].course > students[j + 1].course; // Swap by course.
                    break;
                case 3: { // Compare names case-insensitively.
                    string name1 = toLowerUtf8(students[j].name); // Lowercase first name.
                    string name2 = toLowerUtf8(students[j + 1].name); // Lowercase second name.
                    shouldSwap = name1 > name2; // Swap if first is greater alphabetically.
                    break;
                }
                case 4: { // Compare surnames.
                    string surname1 = toLowerUtf8(students[j].surname); // Lowercase first surname.
                    string surname2 = toLowerUtf8(students[j + 1].surname); // Lowercase second surname.
                    shouldSwap = surname1 > surname2; // Decide swap.
                    break;
                }
                case 5: { // Compare middle names.
                    string middle1 = toLowerUtf8(students[j].middleName); // Lowercase first middle name.
                    string middle2 = toLowerUtf8(students[j + 1].middleName); // Lowercase second middle name.
                    shouldSwap = middle1 > middle2; // Determine swap.
                    break;
                }
                default:
                    cout << "Неверный параметр сортировки.\n"; // Invalid sort field.
                    return; // Exit without sorting.
            }

            if (shouldSwap) { // If comparison indicates wrong order...
                Student temp = students[j]; // Temporary storage.
                students[j] = students[j + 1]; // Move next student back.
                students[j + 1] = temp; // Place temp forward.
            }
        }
    }
}

bool checkAvailability() { // Ensure array has space for another student.
    if (studentCount + 1 > capacity) expandArray(); // If not, enlarge the array.
    return true; // After ensuring space, return true.
}

void addStudentToArray(const Student &student) { // Add a student to the array.
    checkAvailability(); // Make sure capacity is enough.
    students[studentCount] = student; // Copy student into array.
    studentCount++; // Increase count.
    cout << "Студент успешно добавлен.\n"; // Feedback message.
}

void deleteStudent(int index) { // Remove a student by index.
    if (index < 0 || index >= studentCount) { // Validate index bounds.
        cout << "Неверный номер студента.\n"; // Error message.
        return; // Exit without deletion.
    }
    for (int i = index; i < studentCount - 1; i++) // Shift all following students left by one.
        students[i] = students[i + 1]; // Copy next student into current position.

    studentCount--; // Decrement count after removal.
    cout << "Студент удалён.\n"; // Inform user.
    saveToFile(); // Persist updated list to text file.
}

void expandArray() { // Double the capacity when array is full.
    int newCapacity = capacity * 2; // Compute new size.
    Student *newStudents = new Student[newCapacity]; // Allocate new array.
    for (int i = 0; i < capacity; i++) // Copy existing students to new array.
        newStudents[i] = students[i]; // Perform element copy.

    delete[] students; // Free old array memory.
    students = newStudents; // Point to new array.
    capacity = newCapacity; // Update capacity value.

    cout << "Размер массива увеличен: " << capacity << "\n"; // Notify user of new size.
}

void editStudent(int index) { // Allow user to edit fields of a student.
    if (index < 1 || index > studentCount) { // Validate input (1-based indexing expected).
        cout << "Неверный номер студента.\n"; // Error message.
        return; // Exit if invalid.
    }

    index--; // Convert to 0-based index for internal use.

    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear newline left in buffer.

    while (true) { // Loop editing options until user exits.
        cout << "\nЧто хотите изменить?\n"; // Editing menu header.
        cout << "1) Год рождения\n"; // Option to change birth year.
        cout << "2) Курс\n"; // Option to change course.
        cout << "3) Имя\n"; // Option to change name.
        cout << "4) Фамилия\n"; // Option to change surname.
        cout << "5) Отчество\n"; // Option to change middle name.
        cout << "6) Предметы и оценки\n"; // Option to change subjects/grades.
        cout << "7) Выход\n"; // Option to exit editing.
        cout << "Выберите пункт: "; // Prompt for choice.

        int editChoice; // Store user choice.
        cin >> editChoice; // Read choice.
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear newline.

        switch (editChoice) { // Handle each editing option.


            case 1: { // Edit birth year.
                string input; // Hold user input.
                while (true) { // Validate year.
                    cout << "Введите новый год рождения (1930-2010): "; // Prompt.
                    getline(cin, input); // Read input.
                    if (isNumber(input)) { // Ensure numeric.
                        bool ok = false; // Track conversion success.
                        if ((int)input.size() > 4) { // Enforce reasonable length.
                            cout << "Ошибка: год должен быть не больше 4 цифр.\n"; // Explain.
                            continue; // Prompt again.
                        }
                        int year = stoi(input); // Convert digits to int directly.
                        ok = true; // Consider conversion successful.
                        if (ok && isValidYear(year)) { // Check range.
                            break; // Accept.
                        } else {
                            cout << "Ошибка: год должен быть в диапазоне 1930-2010.\n"; // Range or invalid.
                        }
                    } else {
                        cout << "Ошибка: введите число!\n"; // Non-digit characters.
                    }
                }

                cout << "Изменить год? "; // Ask for confirmation.
                if (!askPermission1()) { // If user cancels...
                    cout << "Отменено.\n"; // Inform and exit case.
                    break; // Break out of case.
                }

                students[index].year = stoi(input); // Apply new year directly (length already checked).
                saveToFile(); // Save changes.
                cout << "Год рождения обновлён.\n"; // Confirm.
                break; // End case.
            }


            case 2: { // Edit course.
                string input; // User input.
                while (true) { // Validate course.
                    cout << "Введите новый курс (0-6): "; // Prompt.
                    getline(cin, input); // Read input.
                    if (isNumber(input)) { // Ensure numeric.
                        bool ok = false; // Track conversion.
                        if ((int)input.size() > 1) { // Enforce single-digit course.
                            cout << "Ошибка: курс должен быть одной цифрой (0-6).\n"; // Explain limit.
                            continue; // Ask again.
                        }
                        int course = stoi(input); // Convert directly.
                        ok = true; // Consider conversion successful.
                        if (ok && isValidCourse(course)) { // Range check.
                            break; // Accept.
                        } else {
                            cout << "Ошибка: курс должен быть в диапазоне 0-6.\n"; // Range or invalid.
                        }
                    } else {
                        cout << "Ошибка: введите число!\n"; // Non-digit input.
                    }
                }

                if (!askPermission1()) { // Confirm change.
                    cout << "Отменено.\n"; // Cancel message.
                    break; // Exit case.
                }

                students[index].course = stoi(input); // Apply new course directly (length already checked).
                saveToFile(); // Save changes.
                cout << "Курс обновлён.\n"; // Confirm.
                break; // End case.
            }


            case 3: { // Edit name.
                string name; // New name.
                while (true) { // Validate.
                    cout << "Введите новое имя (только русские или английские буквы): "; // Prompt.
                    getline(cin, name); // Read.
                    if (isValidName(name)) break; // Accept when valid.
                    cout << "Ошибка: имя может содержать только русские или английские буквы.\n"; // Error.
                }

                if (!askPermission1()) break; // Cancel if user says no.

                students[index].name = name; // Apply new name.
                saveToFile(); // Save.
                cout << "Имя обновлено.\n"; // Confirm.
                break; // End case.
            }


            case 4: { // Edit surname.
                string surname; // New surname.
                while (true) { // Validate.
                    cout << "Введите новую фамилию (только русские или английские буквы): "; // Prompt.
                    getline(cin, surname); // Read.
                    if (isValidName(surname)) break; // Accept.
                    cout << "Ошибка: фамилия может содержать только русские или английские буквы.\n"; // Error.
                }

                if (!askPermission1()) break; // Cancel if user declines.

                students[index].surname = surname; // Apply change.
                saveToFile(); // Save.
                cout << "Фамилия обновлена.\n"; // Confirm.
                break; // End case.
            }


            case 5: { // Edit middle name.
                string middle; // New middle name.
                while (true) { // Validate.
                    cout << "Введите новое отчество (только русские или английские буквы): "; // Prompt.
                    getline(cin, middle); // Read.
                    if (isValidName(middle)) break; // Accept.
                    cout << "Ошибка: отчество может содержать только русские или английские буквы.\n"; // Error.
                }

                if (!askPermission1()) break; // Cancel if needed.

                students[index].middleName = middle; // Apply.
                saveToFile(); // Save.
                cout << "Отчество обновлено.\n"; // Confirm.
                break; // End case.
            }


            case 6: { // Edit subjects and grades.
                Student backup = students[index]; // Keep backup in case of cancel.

                for (int i = 0; i < 3; i++) { // Loop through each subject/grade pair.
                    while (true) { // Validate subject.
                        cout << "Введите предмет " << i+1 << " (только русские или английские буквы): "; // Prompt.
                        getline(cin, students[index].subjects[i]); // Read subject.
                        if (isValidSubject(students[index].subjects[i])) break; // Accept when valid.
                        cout << "Ошибка: название предмета может содержать только русские или английские буквы.\n"; // Error.
                    }

                    while (true) { // Validate grades.
                        cout << "Введите 3 оценки (1–5, через запятую, например: 5,4,3): "; // Prompt.
                        getline(cin, students[index].grades[i]); // Read grades string.

                        if (checkGrades(students[index].grades[i])) break; // Accept when valid format.

                        cout << "Ошибка: введите ровно 3 оценки (числа 1–5 через запятую).\n"; // Error message.
                    }
                }

                if (!askPermission1()) { // Confirm changes.
                    students[index] = backup; // Restore backup if canceled.
                    cout << "Отменено.\n"; // Inform user.
                    break; // Exit case.
                }

                saveToFile(); // Save changes.
                cout << "Предметы и оценки обновлены.\n"; // Confirm.
                break; // End case.
            }


            case 7: { // Exit editing.
                bool toMenu = askPermission2(); // Ask whether to go to main menu.
                if (toMenu) return; // Leave editStudent and go back to menu.
                else break; // Otherwise continue loop.
            }

            default:
                cout << "Неверный пункт.\n"; // Handle invalid selection.
        }
    }
}


void saveToFile() { // Write students to a plain text file.
    ofstream fout(FILE1_PATH); // Open file for writing (overwrite).
    if (!fout) { // Check if file opened.
        cout << "Ошибка: не удалось открыть файл для записи.\n"; // Error message.
        return; // Abort saving.
    }
    for (int i = 0; i < studentCount; i++) { // Write each student.
        fout << students[i].year << " "
             << students[i].course << " "
             << students[i].name << " "
             << students[i].surname << " "
             << students[i].middleName << " ";
        for (int j = 0; j < 3; j++) // Loop through three subjects/grades.
            fout << students[i].subjects[j] << " " << students[i].grades[j] << " "; // Write subject and grade.
        fout << "\n"; // Newline after each student.
    }
    fout.close(); // Close file.
    cout << "Текстовый файл сохранён.\n"; // Confirm to user.
}

void loadFromFile() { // Load students from a plain text file.
    ifstream fin(FILE1_PATH); // Open file for reading.
    if (!fin) { // If file not found...
        cout << "Ошибка: текстовый файл не найден.\n"; // Inform user.
        return; // Exit.
    }
    studentCount = 0; // Reset current count.
    while (fin >> students[studentCount].year
               >> students[studentCount].course
               >> students[studentCount].name
               >> students[studentCount].surname
               >> students[studentCount].middleName) { // Read basic fields.
        for (int j = 0; j < 3; j++) // Read subjects and grades.
            fin >> students[studentCount].subjects[j] >> students[studentCount].grades[j];

        studentCount++; // Increment count after reading one student.
        if (studentCount >= capacity) expandArray(); // Expand if needed.
    }
    fin.close(); // Close file.
    sortStudentsByYear(); // Sort after loading for consistency.
    cout << "Текстовый файл загружен.\n"; // Confirmation message.
}

string toLowerUtf8(const string& s) { // Convert UTF-8 string to lowercase using wide characters.
    wstring_convert<codecvt_utf8<wchar_t>> conv; // Converter between UTF-8 and wide strings.
    wstring ws = conv.from_bytes(s); // Convert to wide string.

    for (auto& c : ws) // Iterate through wide characters.
        c = towlower(c); // Convert each to lowercase.

    return conv.to_bytes(ws); // Convert back to UTF-8 string.
}

bool askPermission1() { // Ask user for yes/no confirmation.
    while (true) { // Loop until valid response.
        cout << "Вы уверены? (y/n): "; // Prompt.

        char answer; // Store answer.
        if (!(cin >> answer)) { // If input fails (e.g., EOF)...
            cin.clear(); // Clear error flags.
            cin.ignore(10000, '\n'); // Discard invalid input.
            continue; // Ask again.
        }

        cin.ignore(10000, '\n'); // Clear rest of line.
        answer = tolower((unsigned char)answer); // Normalize to lowercase.

        if (answer == 'y') return true; // Yes => proceed.
        if (answer == 'n') return false; // No => cancel.

        cout << "Введите только y или n.\n"; // Prompt for correct input.
    }
}

bool askPermission2() { // Ask whether to go to main menu or previous step.
    cout << "Выйти на главное меню или на предыдущий шаг? (m/p): "; // Prompt.
    char answer; // Store response.
    if (!(cin >> answer)) { // If input fails...
        cin.clear(); // Clear error state.
        cin.ignore(10000, '\n'); // Discard line.
        return false; // Default to stay.
    }
    cin.ignore(10000, '\n'); // Clear remainder of line.
    answer = tolower((unsigned char)answer); // Normalize.
    return answer == 'm'; // Return true if user chose 'm' (menu).
}

void saveToBinaryFile() { // Save student data to binary file for compact storage.
    ofstream fout(FILE2_PATH, ios::binary); // Open binary file for writing.
    if (!fout) { // Check for failure to open.
        cout << "Ошибка записи бинарного файла.\n"; // Error message.
        return; // Abort.
    }
    fout.write((char*)&studentCount, sizeof(studentCount)); // Write number of students first.
    for (int i = 0; i < studentCount; i++) { // Write each student.
        fout.write((char*)&students[i].year, sizeof(students[i].year)); // Write birth year.
        fout.write((char*)&students[i].course, sizeof(students[i].course)); // Write course.

        int nameLen = students[i].name.length(); // Length of name.
        fout.write((char*)&nameLen, sizeof(nameLen)); // Write length.
        fout.write(students[i].name.c_str(), nameLen); // Write name characters.

        int surnameLen = students[i].surname.length(); // Length of surname.
        fout.write((char*)&surnameLen, sizeof(surnameLen)); // Write length.
        fout.write(students[i].surname.c_str(), surnameLen); // Write surname characters.

        int middleNameLen = students[i].middleName.length(); // Length of middle name.
        fout.write((char*)&middleNameLen, sizeof(middleNameLen)); // Write length.
        fout.write(students[i].middleName.c_str(), middleNameLen); // Write middle name characters.

        for (int j = 0; j < 3; j++) { // Write each subject and grade.
            int subjectLen = students[i].subjects[j].length(); // Length of subject string.
            fout.write((char*)&subjectLen, sizeof(subjectLen)); // Write length.
            fout.write(students[i].subjects[j].c_str(), subjectLen); // Write subject text.

            int gradeLen = students[i].grades[j].length(); // Length of grade string.
            fout.write((char*)&gradeLen, sizeof(gradeLen)); // Write length.
            fout.write(students[i].grades[j].c_str(), gradeLen); // Write grade text.
        }
    }
    fout.close(); // Close file.
    cout << "Бинарный файл сохранён.\n"; // Confirm to user.
}

void loadFromBinaryFile() { // Load student data from binary file.
    ifstream fin(FILE2_PATH, ios::binary); // Open binary file for reading.
    if (!fin) { // If not found...
        cout << "Бинарный файл не найден.\n"; // Inform user.
        return; // Exit.
    }
    int countFromFile = 0; // Will hold number of students in file.
    fin.read((char*)&countFromFile, sizeof(countFromFile)); // Read count.

    if (countFromFile > capacity) { // If file has more students than current capacity...
        delete[] students; // Free current array.
        students = new Student[countFromFile]; // Allocate new array of needed size.
        capacity = countFromFile; // Update capacity.
    }

    for (int i = 0; i < countFromFile; i++) { // Read each student from file.
        fin.read((char*)&students[i].year, sizeof(students[i].year)); // Read year.
        fin.read((char*)&students[i].course, sizeof(students[i].course)); // Read course.

        int nameLen = 0; // Length holder for name.
        fin.read((char*)&nameLen, sizeof(nameLen)); // Read length.
        students[i].name.resize(nameLen); // Resize string to hold name.
        fin.read(&students[i].name[0], nameLen); // Read name characters.

        int surnameLen = 0; // Length for surname.
        fin.read((char*)&surnameLen, sizeof(surnameLen)); // Read length.
        students[i].surname.resize(surnameLen); // Resize string.
        fin.read(&students[i].surname[0], surnameLen); // Read surname.

        int middleNameLen = 0; // Length for middle name.
        fin.read((char*)&middleNameLen, sizeof(middleNameLen)); // Read length.
        students[i].middleName.resize(middleNameLen); // Resize string.
        fin.read(&students[i].middleName[0], middleNameLen); // Read middle name.

        for (int j = 0; j < 3; j++) { // Read three subjects and grades.
            int subjectLen = 0; // Length of subject string.
            fin.read((char*)&subjectLen, sizeof(subjectLen)); // Read length.
            students[i].subjects[j].resize(subjectLen); // Resize storage.
            fin.read(&students[i].subjects[j][0], subjectLen); // Read subject text.

            int gradeLen = 0; // Length of grade string.
            fin.read((char*)&gradeLen, sizeof(gradeLen)); // Read length.
            students[i].grades[j].resize(gradeLen); // Resize string.
            fin.read(&students[i].grades[j][0], gradeLen); // Read grade text.
        }
    }

    studentCount = countFromFile; // Update current count with loaded value.
    fin.close(); // Close file.
    sortStudentsByYear(); // Sort data after loading for consistent order.
    cout << "Бинарный файл загружен.\n"; // Confirm to user.
}

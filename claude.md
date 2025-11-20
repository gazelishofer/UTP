---

# 📄 **TASK: Rewrite `printArray()` into a fully dynamic UTF-8 safe table printer**

## 🎯 **Goal**

Rewrite the existing `printArray()` function into a **fully dynamic**, **UTF-8-aware**, **multi-line horizontal table** printer with:

* Dynamic column widths (no fixed sizes except max line width per cell)
* UTF-8 safe width calculation
* Word-wrapping / multi-line cell support (per column)
* Row height equalized across all columns
* Clear delimiters:

    * Between **columns**
    * Between **rows**
    * Between header and body
* Nicely formatted ASCII table output
* No vertical format — table must remain horizontal
* No truncation — wrap instead

The system will run inside CLion on Windows, so console width is not an issue.

---

# 📌 **Table Requirements (Mandatory)**

### ✔ Horizontal Table Only

The table must remain fully horizontal — **not vertical card layout**.

### ✔ Dynamic Column Widths

Column width automatically grows based on actual content (UTF-8 safe), but:

### ✔ Max Visible Characters Per Line in Any Cell = **20 UTF-8 characters**

This means:

* If a cell contains more than 20 visible characters → break into multiple lines (wrap).
* Wrapping must be UTF-8 safe (you cannot split multi-byte chars).

### ✔ Equal Row Height

If ANY column in a row wraps to N lines,
→ **The entire row must be printed over N lines**.

Example:

```
|Name           |Surname |
|Ivan           |Petrov  |
|LongLongLongNa |Petrov  |
|mePart2        |        |
```

### ✔ Column Delimiters

Columns must be separated using:

```
| <cell> |
```

Including left and right borders.

Example:

```
|Ivan           |Petrov        |Math     |5,5,5 |
```

### ✔ Row Delimiters

After EACH row, print a separator line:

```
---------------------------------------------------------
```

The separator must match the table width exactly.

### ✔ Header + Header Separator

Print column names dynamically with same logic.

After the header, print a separator line.

---

# 🌍 **UTF-8 Requirements**

The function **must** correctly measure the width of UTF-8 strings in terms of *visible characters*, not byte count.

Implement or re-use:

* `utf8_width()` – count visible characters
* `utf8_substr()` – extract substring by visible character index
* `wrapUtf8()` – wrap a UTF-8 string into chunks of ≤20 characters

All functions must be UTF-8 safe.

### 🛑 No truncation

Do NOT cut data permanently.
Only wrap into multiple lines.

---

# 🗂 **Data Model (for reference)**

Assume this struct:

```cpp
struct Student {
    string name;
    string surname;
    string middleName;
    int year;
    int course;
    string subjects[3];
    string grades[3];
};
extern Student students[];
extern int studentCount;
```

---

# 🧩 **Required Output Structure**

### 1. Header row

Dynamic widths apply here as well.

### 2. Header separator line

Works like row separator.

### 3. For each student:

* Calculate wrapped lines per column
* Determine max row height
* Print row line by line
* Then print a horizontal separator

### 4. End of table

---

# 📐 **Column Definitions**

Columns to print, in order:

1. `No`
2. `Year`
3. `Course`
4. `Name`
5. `Surname`
6. `Middle Name`
7. `Subject 1`
8. `Grades 1`
9. `Subject 2`
10. `Grades 2`
11. `Subject 3`
12. `Grades 3`

---

# 🔎 **Example of required behavior**

Long values should wrap:

Input (Name column):

```
"fgjfjfopsjpojgpojspgs;gkfkpjgpj"
```

Output:

```
|fgjfjfopsjpojgpojgp |  
|osjpgs;gkfkpjgpj    |  
```

If ANY column wraps:

```
|Name part 1 |Surname |
|Name part 2 |        |
```

---

# 📏 **Column Width Calculation Rules**

1. Base width = min( max( longest-visible-content ), 20 )
2. Width is still padded by spaces inside the cell to keep alignment
3. Column width includes cell content only; borders do not count
4. UTF-8 visible characters must be counted — not byte size

---

# 🔨 **Implementation Requirements**

Claude should:

### ✔ Rewrite `printArray()` entirely

### ✔ Produce clean, readable, well-commented C++ code

### ✔ Split logic into helper functions:

* `utf8_width()`
* `utf8_substr()`
* `wrapUtf8(const string&, int maxWidth)`
* `printWrappedRow(...)`
* `printSeparatorLine()`
* etc.

### ✔ Ensure printed table is stable and readable in a wide Windows console

### ✔ Handle any UTF-8 data including Russian

---

# 💬 **Final Deliverable**

Claude must return:

* A complete, ready-to-compile C++ implementation of the **entire** updated `printArray()` function
* All helper functions used for UTF-8 processing
* An example of output using test data
* No pseudocode — must be fully functional

---

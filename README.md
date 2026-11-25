# Student Academic Records and Performance Analytics System

A menu-driven C program that stores student records in memory, saves them to `students.txt` and produces academic reports.

## Build and run

From the root of the repository:

```
cd academic_records_analysis
gcc main.c helpers.c academic_record_operations.c search_sort.c -o main.exe
.\main.exe
```

Run it from inside `academic_records_analysis`, because the program looks for `students.txt` in the current folder. All the files below are in that folder.

| File | Purpose |
|---|---|
| `main.c` | Course catalog, global student list and menu |
| `academic_record_operations.c/.h` | Add, display, update, view, remove and report |
| `search_sort.c/.h` | Searching and sorting |
| `helpers.c/.h` | Structures, input validation, file handling and average calculation |
| `students.txt` | Sample student records and academic data |

## Structure definitions and relationships

| Structure | Fields | Role |
|---|---|---|
| `students` | ID, first name, last name, programme, year, number of courses, `course *courses` | One student |
| `course` | Code, name, marks, maximum marks | One course taken by one student |
| `CourseInfo` | Code, name, maximum marks | One course in the fixed catalog |

One student has many courses. Each `students` record holds a pointer to its own array of `course` records, and `number_of_courses` gives the length of that array. Course codes, names and maximum marks are copied from `CATALOG`, so a student can only register for a course that exists.

## Pointer usage

- `students *s` points to the first record in the student list. `s[i]` accesses a student.
- `courses` inside each student points to that student's courses: `s[i].courses[j]`.
- `students *st = &s[i]` is used to work with one record without copying it, as in `calculate_average(const students *st)`. The `const` means the function cannot change the record.
- `get_number` takes a `void *` so that it can fill either an `int` or a `float`.
- The menu uses an array of function pointers, so the chosen number selects the function to call.

## Dynamic memory allocation and deallocation

| Where | Function | Purpose |
|---|---|---|
| Loading | `malloc`, `calloc` | Create the student list and each student's courses |
| Adding a student | `malloc`, `realloc` | Create the courses, and double the list when it is full (starting at 4) |
| Adding a course | `realloc` | Make room for the new courses |
| Removing a student | `free`, `realloc` | Free the courses, then halve the list when half of it is empty |
| Sorting | `malloc`, `free` | Hold a temporary copy of the records and their averages |
| Exit or failed load | `free` | `free_records` frees each student's courses, then the list |

`count` is the number of students and `capacity` is the number that fit. Every `realloc` result is stored in a temporary pointer first, so the old data is not lost if it fails. Every allocation is checked for `NULL`.

## Searching and sorting algorithms

**Linear search.** `search_student` compares every record with the ID, the first or last name, or the programme, and prints each match. It ignores capital letters and is O(n). An ID search stops at the first match because IDs are unique.

**Insertion sort.** `sort_students` takes one student at a time, compares them with the students already in the `sorted` array from right to left, moves larger ones one place right, and puts the student in the gap. It is O(n²) and `qsort()` is not used. Records can be sorted by ID, first name, last name, average or programme, in ascending or descending order. The sort works on a copy, so the saved order does not change. Students with no courses come last when sorting by average.

## File handling implementation

Records are stored in `students.txt`. The first line is the number of students. Each student line is followed by that student's course lines:

```
1
1001|Amina|Uwase|Software Engineering|2|1
SE101|Introduction to C|78.00|100.00
```

- `load_records` runs at start-up and rebuilds the list and each course array. Every line is checked before it is used.
- A missing file starts an empty list. An invalid file stops the program with the line number, and the file is left unchanged.
- `save_records` runs after every add, update and removal, and on exit. If saving fails on exit, the user can go back to the menu instead of losing the records.

The loader checks that:

- the number on line 1 matches the number of student records in the file
- IDs, names, programmes, years and marks follow the same rules as typed input
- no ID or full name is used twice, and no student has the same course twice
- each course code, name and maximum mark matches the catalog
- no line has extra text after its last field

## Academic performance calculations

| Calculation | Formula |
|---|---|
| Course percentage | marks / maximum marks × 100 |
| Student average | Sum of the student's course percentages / number of courses |
| Overall average | Sum of the student averages / number of students with courses |
| Course average | Sum of the percentages in that course / number of students taking it |

A course is passed at 50% or more, and a student is counted as passing when their average is 50% or more. `calculate_average` returns -1 for a student with no courses, which prevents division by zero. These students are left out of the averages and the passing and failing counts, and are reported separately.

## Error handling and testing

Numbers are read as text and then converted, so letters, negative numbers and numbers that do not fit are rejected. Text is trimmed and checked for length and allowed characters. IDs and full names must be unique, marks must be between 0 and the course maximum, and a course can only be registered once per student.

| Test | Expected and actual result |
|---|---|
| Add, view, update and remove a student | Each change is shown and saved |
| Search by ID, name and programme | Correct matches, or a "no matching" message |
| Sort every way in both directions | Correct order |
| Academic report | Correct totals and averages |
| Restart after changes | Records are loaded with the changes |
| Missing, invalid and read-only file | Handled with a message, and an invalid or read-only file is not changed |
| Wrong number of students on line 1 (too low, too high or 0) | File is rejected and not changed |
| Invalid menu choices, IDs, names, years, courses and marks | Rejected with a message |
| Duplicate ID or name, or a student that does not exist | Rejected with a message |
| Student with no courses | No division by zero |

This repository has three C projects and one shell script:

- [Student Academic Records and Performance Analytics System](#student-academic-records-and-performance-analytics-system), in `academic_records_analysis`
- [Sales Transaction Management System](#sales-transaction-management-system), in `sales_transaction`
- [Multi-threaded Document Analyzer](#multi-threaded-document-analyzer), in `document_analyzer`
- [Linux System Health and Resource Monitoring Manager](#linux-system-health-and-resource-monitoring-manager), in `system_health`

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

# Sales Transaction Management System

A menu-driven C program that records sales, processes them with function pointers and callbacks, and saves them to `transactions.txt`.

## Building and running the sales program

From the root of the repository:

```
cd sales_transaction
gcc main.c transactions.c processing.c -o main.exe
.\main.exe
```

Run it from inside `sales_transaction`, because the program looks for `transactions.txt` in the current folder.

## Program structure

| File | Purpose |
|---|---|
| `main.c` | Menu, and the array, count and capacity |
| `transactions.c/.h` | Structures, input validation, add, display, update, delete, save and load |
| `processing.c/.h` | Function pointer operations, condition callbacks, filters, search, sort and sales report |
| `transactions.txt` | Sample transactions |

A `Transaction` holds an ID, product name, quantity, unit price, total amount and a `Date`. Money is stored as whole cents in a `long long`, so adding up totals has no rounding errors. Add and update set the total whenever the quantity or price can change. The ID never changes after a transaction is added.

There are no global variables. `main` owns the data and passes it to each function.

## Function pointers

`process_sales` keeps the five operations in an array and uses the number the user picks to call one:

```c
void (*ops[5])(const Transaction *, int) = {total_revenue, average_value, highest_transaction,
                                            lowest_transaction, total_units};
ops[choice - 1](list, count);
```

All five have the same parameters and return type, which is what lets them share one array. The choice is checked first, so the program never reads outside the array.

## Callback functions

A callback is passed to another function, which then calls it. Each condition callback answers yes (1) or no (0) for one transaction:

| Callbacks | Passed to | Used for |
|---|---|---|
| `above_amount`, `above_quantity`, `is_product` | `filter_transactions`, `count_transactions` | Filter menu |
| `is_product`, `equals_amount` | `filter_transactions`, `count_transactions` | Search |
| `is_product` | `count_transactions`, `sum_transactions` | Revenue of one product |
| `compare_id`, `compare_product`, `compare_total` | `sort_transactions` | Sort |

The value to compare with is passed as a `const void *`, so one shape works for amounts, quantities and names. Adding a condition needs only a new callback and a menu option. The functions that loop over the records do not change.

## Pointers and dynamic memory

- Functions that may move the array, such as `add_transaction`, take `Transaction **list`, so they can change the pointer in `main`. Functions that only change records take `Transaction *list`, and functions that only read take `const Transaction *list`.
- Loading uses `malloc` to make an array, and `realloc` doubles it when it is full. Adding also doubles the array with `realloc`.
- Deleting halves the array when only a quarter is used. It never goes below 4, so `realloc` is never asked for 0 bytes.
- Every `malloc` and `realloc` is checked. `realloc` results go into a temporary pointer first, so the old array is not lost if it fails.
- Sorting uses a copy made with `malloc`, which is freed afterwards. `main` frees the array on exit.

## Searching and sorting transactions

**Linear search.** An ID search stops at the first match because IDs are unique. Product and amount searches check every record, because several can match. Product names ignore capital letters.

**Insertion sort.** `sort_transactions` moves each record left until the one before it should stay in front. The compare callback decides the order, and the result is flipped for descending order. It is O(n²) and `qsort()` is not used. The saved order does not change.

## Saving and loading transactions

Each line is `id|product|quantity|price in cents|day|month|year`:

```
1001|Cooking oil 1L|2|32500|2|1|2025
```

- The total is not saved. It is worked out when loading, so it always matches the quantity and price.
- There is no record count in the file, so none can be wrong.
- Saving writes to `transactions.tmp` first, and replaces `transactions.txt` only if every write worked.
- Loading fills a separate array, and replaces the records in memory only if every line is valid.
- The program saves after every add, update and delete, and from the menu. If the last save failed, it tries again on exit.
- A missing file starts an empty list. An invalid file stops the program with the line number, and the file is left unchanged.

## Validation and testing of the sales program

| Test | Result |
|---|---|
| Duplicate, zero, negative or too large ID | Rejected |
| Invalid quantity, price, product name, date or menu choice | Rejected |
| Transaction that does not exist, or no transactions at all | Shown as a message, with no division by zero |
| All five operations, filters and searches | Correct results |
| 3,600 random sorts | All in the right order, with no lost records |
| Add, update and delete, then restart | Records are loaded with the changes |
| Missing, invalid and read-only file, and a full disk | Handled with a message and no saved records lost |
| `malloc` and `realloc` failing | Handled with a message and no records lost |
| Input ending in any menu option | The program stops cleanly |

# Multi-threaded Document Analyzer

A C program that searches several text files for a keyword at the same time, using one POSIX thread for each file, and writes a report for each one.

## Building and running the analyzer

From the root of the repository:

```
cd document_analyzer
gcc main.c analyzer.c report.c -o document_analyzer -pthread
./document_analyzer "IoT" document1.txt document2.txt document3.txt
```

| File | Purpose |
|---|---|
| `main.c` | Checks the arguments, creates and joins the threads, and frees the memory |
| `analyzer.c/.h` | Thread structure, setting it up, the keyword search and the thread function |
| `report.c/.h` | Report files and the summary |
| `document1.txt` to `document3.txt` | Sample documents, the third one does not contain `IoT` |

## Threads

- `main` makes an array with one `ThreadArgs` structure per file. Each structure holds the file name, the keyword, the results and the thread ID.
- `pthread_create` gives each thread the address of its own structure, `&args[i]`. The thread function `void *process_file(void *arg)` turns it back into a `ThreadArgs *`.
- All the threads are created in one loop and joined in a second loop. Joining in the first loop would run them one after another.
- `created` is set only if `pthread_create` returned 0, so a thread that never started is never joined. If one fails, the others carry on.
- A thread never calls `exit`, because that would stop the other threads. It records the failure and returns.
- `success` starts at 0 and is set to 1 last, after the file is read and the report is written.

**Why no lock is needed.** Each thread writes only to its own structure and its own report. The keyword is shared, but no thread changes it. `main` reads the results only after joining. Status lines from different threads can appear in any order, and each one is a single `printf`.

## Searching

- `count_keyword` compares the keyword with the text at every position using `strncasecmp`, so capitals are ignored.
- The keyword can be part of a word. Matches do not overlap, so `aa` is found twice in `aaaa`.
- Lines can be any length. Each thread reads into its own buffer, which doubles with `realloc` when it is full.
- Text after the last newline counts as a line.

## Memory

| Memory | Made by | Freed by |
|---|---|---|
| Array of structures | `main` | `main` |
| Report name | `init_thread_args` | `main` |
| Matching line numbers | `process_file` | `main`, after joining |
| Line buffer | `process_file` | `process_file` |

Every allocation is checked, and `realloc` results go into a temporary pointer first.

## Reports

The report of `document1.txt` is `document1_report.txt`, in the same folder:

```
===== DOCUMENT ANALYSIS REPORT =====
Input File: document1.txt
Search Keyword: IoT
Total Lines Processed: 7
Total Keyword Occurrences: 5
Number of Matching Lines: 4
Matching Line Numbers: 1, 3, 5, 7
Processing Status: Successful
```

A file that cannot be read gets a report that says why it failed. Running the program again replaces the old reports.

## Errors and testing of the analyzer

The program returns 0 if every file succeeded and 1 if not.

| Test | Result |
|---|---|
| Too few arguments, or an empty keyword | Message, and no threads are started |
| Missing file, or a file that is not plain text | That file fails and the others complete |
| Keyword not found | Successful, with 0 occurrences |
| Empty file, no newline at the end, Windows line endings | Correct counts |
| A 5 MB line | Correct counts |
| Report cannot be created or written | That file fails, and no half-written report is left |
| Two files with the same report name, or a report that would replace an input file | Refused before any thread starts |
| `pthread_create` failing for some or all threads | Only the created threads are joined |
| `malloc` and `realloc` failing | Handled with a message and no leaks |
| 60 files at once, 10 times | Correct reports every time |

The check for clashing names compares the names as they are typed, so `a.txt` and `./a.txt` are not seen as the same file.

# Linux System Health and Resource Monitoring Manager

An interactive Bash script that shows CPU, memory, disk and process information, warns when usage passes a threshold, terminates processes safely, saves health reports and keeps an activity log.

## Running the script

It needs Linux and Bash 4 or newer. From the root of the repository:

```
cd system_health
chmod +x system_health.sh
./system_health.sh
```

On Windows, run it inside WSL. In Git Bash it stops with `Error: this script must be run on Linux.`, because Git Bash does not have `free`, `uptime` or the Linux `/proc` files.

`.gitattributes` keeps `.sh` files in Linux line endings. Windows line endings would make Bash fail with `$'\r': command not found`.

## Menu

| Option | Function | What it does |
|---|---|---|
| 1. Display system information | `display_system_info` | Date and time, hostname, operating system, kernel, architecture and uptime |
| 2. Monitor CPU utilization | `monitor_cpu` | Number of cores, load average, CPU usage, the 5 busiest processes and warnings |
| 3. Monitor memory utilization | `monitor_memory` | Total, used and available memory, usage as a percentage and a warning |
| 4. Monitor disk utilization | `monitor_disk` | Size, used and available space of each filesystem, and the ones above the threshold |
| 5. View and search running processes | `view_processes` | Number of processes, the 15 busiest, and a search by name or PID |
| 6. Terminate a process | `terminate_process` | Checks the process can be stopped, asks for confirmation, then stops it |
| 7. Generate a system health report | `generate_report` | Saves options 1 to 4 in a file with a timestamp |
| 8. View activity history | `view_log` | Shows the activity log |
| 9. Clear activity log | `clear_log` | Empties the log after confirmation |
| 10. Exit | `main` | Logs the end of the session and exits |

`main` checks that it is running on Linux, then shows the menu in a loop until the user picks 10 or the input ends.

## Helper functions

Each helper is used in at least three places.

| Helper | Purpose |
|---|---|
| `log_action` | Adds a line with the date, time and level to the activity log |
| `ask` | Reads one line, and returns 1 when the input ends so the script can stop cleanly |
| `confirm` | Returns success only if the user types `yes`, used before terminating, clearing the log and creating a folder |
| `require_command` | Checks that a command exists and prints a clear error if it does not |
| `read_threshold` | Asks for a threshold until it is a whole number from 1 to 100 |
| `is_above` | Compares two numbers with `awk`, because Bash cannot compare decimals |

The report runs the same functions as options 1 to 4, with the output sent to the report file, so no monitoring code is repeated. When a monitoring function is given a threshold, it does not ask for one.

## Commands used

| Information | Command or file |
|---|---|
| Hostname, kernel, architecture | `uname -n`, `uname -r`, `uname -m` |
| Operating system | `PRETTY_NAME` in `/etc/os-release` |
| Uptime | `uptime -p`, `uptime -s` |
| CPU usage | `/proc/stat`, read twice one second apart |
| Load average and cores | `/proc/loadavg`, `/proc/cpuinfo` |
| Memory | `free -b` |
| Disk | `df -h --output=...` |
| Processes | `ps -eo pid,user,%cpu,%mem,stat,etime,comm --sort=-%cpu` |
| Terminating | `kill -TERM`, and `kill -0` to check if it stopped |
| Calculations | `awk` |

- **CPU usage.** Usage is the share of time the CPU was not idle between the two readings.
- **Process CPU.** `ps` shows each process's average CPU since it started, so a process can show more than 100% when it uses more than one core.
- **Memory.** Used memory is total minus available, so caches that Linux can free do not count as used.
- **Disk.** It leaves out `tmpfs`, `devtmpfs`, `squashfs` and snap filesystems, because they live in memory or are read-only images that always show 100%.

## Thresholds

The default thresholds are 80% for CPU, 80% for memory and 90% for disk. Options 2, 3 and 4 ask for a new value, and pressing Enter keeps the current one. Only whole numbers from 1 to 100 are accepted, so text, decimals, negative numbers and `0` are rejected. A value such as `08` is read as 8, not as an octal number.

A warning is shown when usage is above the threshold. The CPU check also warns about each process above it. Reports use the current thresholds.

## Terminating processes

`terminate_process` refuses to stop:

- PID 1, the script itself and the shell that started it
- processes owned by system accounts, with a user ID below 1000, such as `root`
- processes of another user, unless the script is run as root

It then shows the process and asks the user to type `yes`. It sends `SIGTERM`, which lets the process close properly, and waits up to 3 seconds. If the process is still running, it says so instead of reporting success.

## Reports

The user picks a folder, and pressing Enter uses `system_health/reports`. A `~` at the start is turned into the home folder. If the folder does not exist, the script asks before creating it. The file is named with the date and time, such as `health_report_20261126_045733.txt`.

The report contains the date and time it was made, the thresholds, and the output of options 1 to 4.

## Activity log

The log is `system_health/activity.log`, next to the script, wherever it is run from. Each line has a date, time and level:

```
2026-11-26 05:00:14 [INFO] Session started by wsl
2026-11-26 05:00:18 [INFO] Memory usage checked: 24.4%
2026-11-26 05:00:20 [INFO] Session ended with Ctrl+C
```

| Level | Used for |
|---|---|
| `INFO` | Checks, searches, thresholds, viewing and the start and end of a session |
| `WARNING` | Usage above a threshold, and refused terminations |
| `ACTION` | Terminated processes, saved reports and clearing the log |
| `ERROR` | Missing commands, files or processes, and failed saves |

Clearing the log leaves one entry that says it was cleared. The log and the reports are in `.gitignore`.

## Error handling

| Problem | What happens |
|---|---|
| Invalid menu choice or threshold | Message, then the question is asked again |
| Input ends (Ctrl+D) at any question | The script exits cleanly |
| Ctrl+C | The session end is logged, and the script exits |
| A command such as `free`, `ps`, `df` or `uptime` is missing | That section shows an error, and the other options still work |
| Invalid PID, or a PID that is not running | Message, and nothing is terminated |
| Protected process, or no permission | Refused with a message, and logged |
| A process that ignores `SIGTERM` | Reported as still running |
| Report folder cannot be created, is not writable, or is a file | Message, and no report is saved |
| Log file cannot be written | One warning, then the script carries on without logging |
| Not running on Linux | Message, and the script exits |

## Testing the script

These tests ran on Ubuntu 22.04 in WSL, and all 47 checks passed.

| Test | Result |
|---|---|
| Options 1 to 4 | Correct information, and the memory percentage matches `free` |
| Thresholds `abc`, `0`, `101`, `50.5`, `-5`, `1000` | Rejected; `08` is accepted as 8 |
| A busy process (`yes`) with a low CPU threshold | A warning for that process |
| Memory and disk thresholds of 1 and 100 | Warnings at 1, and none at 100 |
| Search by part of a name, any capitals, PID, unknown name, `[` and a PID that is too long | Correct matches or a message |
| Terminate: cancel, invalid PID, PID 1, a `root` process, a PID that is not running | Nothing is terminated |
| Terminate the user's own process | It stops, and the action is logged |
| A process that ignores `SIGTERM` | Reported as still running |
| Report in the default folder, a `~` folder, `/root`, a file, and a folder the user chose not to create | Saved, or an error with no report |
| View, cancel clearing, clear | Correct log contents |
| Input ending after every option and question | Clean exit |
| `free`, `ps`, `df` and `uptime` missing | Errors in those sections only |
| Read-only log file | One warning |
| Run from another folder | The log stays next to the script |
| Ctrl+C at the menu | Logged, and exit code 130 |
| Run in Git Bash | Stops with the Linux message |

Stopping another normal user's process was not tested, because the test system had only one normal user.

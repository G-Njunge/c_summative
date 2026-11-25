// Shows the menu and runs the option the user picks

#include <stdio.h>
#include <strings.h>
#include "helpers.h"
#include "academic_record_operations.h"
#include "search_sort.h"

// All the courses a student can register for
const CourseInfo CATALOG[] = {
    {"SE101",  "Introduction to C",         100},
    {"SE102",  "Data Structures",           100},
    {"SE103",  "Discrete Mathematics",       50},
    {"BUS101", "Principles of Management",  100},
    {"BUS102", "Business Statistics",        80},
    {"ENT101", "Entrepreneurship",          100}
};
int catalog_size = (int)(sizeof(CATALOG) / sizeof(CATALOG[0]));

// The student list, how many students it has and how many it can hold
students *s;
int count = 0;
int capacity = 0;

// Loads the records, then shows the menu until the user exits
int main(void)
{
    // Stop if loading fails, so we do not save an empty list over the file
    if (!load_records())
    {
        printf("Fix students.txt and run the program again. No records were changed.\n");
        return 1;
    }

    // Array of function pointers, one for each menu option
    void (*fun_ptr[])(void) = {add_student, display_students, update_student_info, remove_student, detailed_student_info, search_student, sort_students, academic_report};
    int choice;
    char answer[50];

    while (1)
    {
        printf("\nMenu:\n");
        printf("0. Add Student\n");
        printf("1. Display all registered students\n");
        printf("2. Update Student Information\n");
        printf("3. Remove student\n");
        printf("4. View student details and performance\n");
        printf("5. Search for a Student\n");
        printf("6. Sort Students\n");
        printf("7. Generate academic report\n");
        printf("8. Exit\n");
        if (get_number("Enter your choice: ", &choice, 'i')!=1)
        {
            free_records();
            return 1;
        }

        if (choice == 8)
        {
            if (save_records())
            {
                printf("Exiting...\n");
                break;
            }

            // Let the user stay and fix the file instead of losing the records
            get_string("Records were not saved. Type 'yes' to exit anyway or any other word to go back: ", answer, sizeof(answer));
            if (strcasecmp(answer, "yes") == 0)
                break;

            continue;
        }
        if (choice >= 0 && choice < 8)
            fun_ptr[choice]();
        else
            printf("Invalid choice.\n");
    }

    free_records();
    return 0;
}

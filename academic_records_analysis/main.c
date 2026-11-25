// Create a file where student records will be stored
// load the file details to our program whenver it starts
// present the menu to the user
// student info in structs + course info as structs with a student struct

#include <stdio.h>
#include "helpers.h"
#include "academic_record_operations.h"

const CourseInfo CATALOG[] = {
    {"SE101",  "Introduction to C",         100},
    {"SE102",  "Data Structures",           100},
    {"SE103",  "Discrete Mathematics",       50},
    {"BUS101", "Principles of Management",  100},
    {"BUS102", "Business Statistics",        80},
    {"ENT101", "Entrepreneurship",          100}
};
int catalog_size = (int)(sizeof(CATALOG) / sizeof(CATALOG[0]));

students *s;
int count = 0;
int capacity = 0;

int main(void)
{

    /* Stop here if loading fails, otherwise exiting would save an empty list over the file */
    if (!load_records())
    {
        printf("Fix students.txt and run the program again. No records were changed.\n");
        return 1;
    }

    //initialize an array of funtion pointers
    void (*fun_ptr[])(void) = {add_student, display_students, update_student_info, remove_student, detailed_student_info}; //search_student, sort_students, display_students, avg_grade, top_student, remove_student, risk_index_calc, summary_report};
    int choice;
    do
    {
        printf("\nMenu:\n");
        printf("0. Add Student\n");
        printf("1. Display all registered students\n");
        printf("2. Update Student Information\n");
        // printf("2. Search for a Student\n");
        // printf("3. Sort Students\n");
        // printf("4. Calculate Average Grade\n");
        // printf("5. Identify the top-performing student\n");
        printf("3. Remove student\n");
        printf("4. View student details and performance\n");
        //printf("5. Display a student's academic performance");
        // printf("8. Generate summary report\n");
        printf("9. Exit\n");
        if (get_number("Enter your choice: ", &choice, 'i')!=1)
        {
            free_records();
            return 1;
        }

        if (choice == 9)
        {
            printf("Exiting...\n");
            break;
        }
        if (choice >= 0 && choice < 9)
            fun_ptr[choice]();
        else
            printf("Invalid choice.\n");

    } while (choice != 9);

    save_records();
    free_records();
    return 0;
}

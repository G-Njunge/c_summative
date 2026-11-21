// Create a file where student records will be stored
// load the file details to our program whenver it starts
// present the menu to the user
// student info in structs + course info as structs with a student struct

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include "helpers.h"

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

void add_student()
{
    students new_student;
    course new_course;
    int id;

    if (get_number("Student ID: ", &id, 'i') != 1)
        return;

    if (id < 0)
    {
        printf("Student ID cannot be negative.\n");
        return;
    }

    if (id_search(id) != -1)
    {
        printf("Student already exists in our system\n");
        return;
    }
    new_student.id = id;

    /* Names are unique, so a name search can never be ambiguous */
    while (1)
    {
        get_name("Student First Name: ", new_student.first_name, sizeof(new_student.first_name), 0);
        get_name("Student Last Name: ", new_student.last_name, sizeof(new_student.last_name), 0);


        if (name_search(new_student.first_name, new_student.last_name) != -1)
                printf("A student with this name already exists in our system.\n");

        break;

    }

    get_name("Programme of study: ", new_student.programme, sizeof(new_student.programme), 1);

    /* Ask for the grade again instead of throwing away the whole record */
    while (1)
    {
        if (get_number("Year of study: ", &new_student.year, 'i') != 1)
            return;

        if (new_student.year >= 1 && new_student.year <= 4)
            break;

        printf("Input a year between 1 and 4\n");
    }

    while (1)
    {
        if (get_number("Number of courses registered: ", &new_student.number_of_courses, 'i') != 1)
            return;

        if (new_student.number_of_courses <= catalog_size && new_student.number_of_courses > 0)
            break;

        printf("Input a valid number of courses\n");
    }

    new_student.courses = NULL;
    if (new_student.number_of_courses > 0)
    {
        new_student.courses = malloc(new_student.number_of_courses * sizeof(course));
        if (new_student.courses == NULL)
        {
            printf("Memory allocation failed.\n");
            return;
        }
    }

    printf("\nAvailable courses:\n");
    for (int i = 0; i < catalog_size; i++)
        printf("%d. %-8s %-28s (max %.0f)\n", i, CATALOG[i].code, CATALOG[i].name, CATALOG[i].maxMarks);

    int course_choice;
    int duplicate;
    printf("Pick a number between 0 and %d to select your course\n", catalog_size - 1);
    for (int o = 0; o < new_student.number_of_courses; o++)
    {
        while (1)
        {
            if (get_number("Pick a number to select your course: ", &course_choice, 'i') != 1)
            {
                free(new_student.courses);
                return;
            }

            if (course_choice >= catalog_size || course_choice < 0)
            {
                printf("Input a valid course number\n");
                continue;
            }

            duplicate = 0;
            for (int k = 0; k < o; k++)
            {
                if (strcmp(new_student.courses[k].course_code, CATALOG[course_choice].code) == 0)
                {
                    duplicate = 1;
                    break;
                }
            }

            if (duplicate)
            {
                printf("Course already picked, choose another\n");
                continue;
            }
            break;
        }

        strcpy(new_course.course_code, CATALOG[course_choice].code);
        strcpy(new_course.course_name, CATALOG[course_choice].name);
        new_course.maxMarks = CATALOG[course_choice].maxMarks;

        while (1)
        {
            if (get_number("Enter course score: ", &new_course.marks, 'f') != 1)
            {
                free(new_student.courses);
                return;
            }

            if (new_course.marks >= 0 && new_course.marks <= new_course.maxMarks)
                break;

            printf("Input a valid course mark between 0 and %.0f\n", new_course.maxMarks);
        }

        new_student.courses[o] = new_course;
    }

    /* Only grow the array now that the record is valid and will be kept */
        if (count >= capacity)
        {
            students *temp;
            int new_capacity = (capacity == 0) ? 4 : capacity * 2;

            temp = realloc(s, new_capacity * sizeof(students));
            if (temp == NULL)
            {
                printf("Memory allocation failed.\n");
                free(new_student.courses);
                return;
            }
            s = temp;
            capacity = new_capacity;
        }

    s[count] = new_student;
    printf("\nNew student added!\nStudent ID: %d\nStudent name: %s %s\n", s[count].id, s[count].first_name, s[count].last_name);

    count++;
    save_records();
}

int main(void)
{

    load_records();
    //initialize an array of funtion pointers
    void (*fun_ptr[])(void) = {add_student}; //search_student, sort_students, display_students, avg_grade, top_student, remove_student, risk_index_calc, summary_report};
    int choice;
    do
    {
        printf("Menu:\n");
        printf("0. Add Student\n");
        printf("1. Search for a Student\n");
        printf("2. Sort Students\n");
        printf("3. Display Student Records\n");
        printf("4. Calculate Average Grade\n");
        printf("5. Identify the top-performing student\n");
        printf("6. Remove student\n");
        printf("7. Academic risk analysis and course comparison\n");
        printf("8. Generate summary report\n");
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

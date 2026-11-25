#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "helpers.h"
#include "academic_record_operations.h"

void add_student()
{
    students new_student;
    course new_course;
    int id;

    /* The loader rejects IDs below 1, so never let one be saved */
    while (1)
    {
        if (get_number("Student ID: ", &id, 'i') != 1)
            return;

        if (id > 0)
            break;

        printf("Student ID must be greater than 0\n");
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

        if (name_search(new_student.first_name, new_student.last_name) == -1)
            break;

        printf("A student with this name already exists in our system.\n");
    }

    get_name("Programme of study: ", new_student.programme, sizeof(new_student.programme), 1);

    /* Ask for the year again instead of throwing away the whole record */
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
void display_students(void)
{
    if (count == 0)
    {
        printf("No student records available.\n");
        return;
    }

    printf("\n---------------------------------------------------------\n");
    printf("%-10s %-25s %-25s\n",
           "ID", "FULL NAME", "STUDY PROGRAMME");
    printf("---------------------------------------------------------\n");

    for (int i = 0; i < count; i++)
        printf("%-10d %s %-18s %-25s\n",s[i].id,s[i].first_name,s[i].last_name,s[i].programme);
    printf("---------------------------------------------------------\n");
    printf("Total students: %d\n", count);
}

void update_student_info()
{
    course new_course;
    int id, user_choice;

    if (get_number("Student ID: ", &id, 'i') != 1)
        return;

    if (id_search(id) == -1)
    {
        printf("Student doesnt exist in our system\n");
        return;
    }

    int index = id_search(id);

    if (get_number("\nWhat would you like to update?\n"
                "0. First name\n"
                "1. Last name\n"
                "2. Year of study\n"
                "3. Course marks\n"
                "4. Register a new course\n"
                "5. Cancel\n"
                "Enter your choice: ", &user_choice, 'i') != 1)
    return;

    switch (user_choice)
    {
    case 0:
    {
        char f_name[50];
        printf("Current first name: %s\n", s[index].first_name);
        get_name("Student's new first Name: ", f_name, sizeof(f_name), 0);
        strcpy(s[index].first_name, f_name);
        break;
    }
    case 1:
    {
        char l_name[50];
        printf("Current last name: %s\n", s[index].last_name);
        get_name("Student's new last Name: ", l_name, sizeof(l_name), 0);
        strcpy(s[index].last_name, l_name);
        break;
    }
    case 2:
    {
        int year;
        printf("Current year: %d\n", s[index].year);
        while (1)
        {
            if (get_number("Year of study: ", &year, 'i') != 1)
                return;

            if (year >= 1 && year <= 4)
                break;

            printf("Input a year between 1 and 4\n");
        }
        s[index].year = year;
        break;
    }
    case 3:
    {
        int marks;
        char course_code[15];
        get_string("Course code: ", course_code, sizeof(course_code));
        int match = -1;
        for (int i = 0; i < s[index].number_of_courses; i++)
        {
            if (strcasecmp(course_code, s[index].courses[i].course_code) == 0)
            {
                match = i;
                break;
            }
        }

        if (match == -1)
        {
            printf("This student isnt taking the course you just entered\n");
            break;
        }

        while (1)
        {
            if (get_number("New marks: ", &marks, 'i') != 1)
                return;

            if (marks >= 0 && marks <= s[index].courses[match].maxMarks)
                break;

            printf("Input a valid score\n");
        }

        s[index].courses[match].marks = marks;
        break;
    }
    case 4:
    {
        int new_count;
        int course_choice;
        int duplicate;
        course *temp;

        if (s[index].number_of_courses == catalog_size)
        {
            printf("Student is already registered for every course.\n");
            break;
        }

        while (1)
        {
            if (get_number("Number of new courses you want to register: ", &new_count, 'i') != 1)
                return;

            if (new_count > 0 && new_count <= catalog_size - s[index].number_of_courses)
                break;

            printf("Input a number between 1 and %d\n", catalog_size - s[index].number_of_courses);
        }

        temp = realloc(s[index].courses,
                       (s[index].number_of_courses + new_count) * sizeof(course));
        if (temp == NULL)
        {
            printf("Memory allocation failed.\n");
            return;
        }
        s[index].courses = temp;

        printf("\nAvailable courses:\n");
        for (int i = 0; i < catalog_size; i++)
            printf("%d. %-8s %-28s (max %.0f)\n", i, CATALOG[i].code, CATALOG[i].name, CATALOG[i].maxMarks);

        printf("Pick a number between 0 and %d to select your course\n", catalog_size - 1);
        for (int o = 0; o < new_count; o++)
        {
            while (1)
            {
                if (get_number("Pick a number to select your course: ", &course_choice, 'i') != 1)
                    return;

                if (course_choice >= catalog_size)
                {
                    printf("Input a valid course number\n");
                    continue;
                }

                duplicate = 0;
                for (int k = 0; k < s[index].number_of_courses; k++)
                {
                    if (strcmp(s[index].courses[k].course_code, CATALOG[course_choice].code) == 0)
                    {
                        duplicate = 1;
                        break;
                    }
                }

                if (duplicate)
                {
                    printf("Student is already doing this course\n");
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
                    return;

                if (new_course.marks <= new_course.maxMarks)
                    break;

                printf("Input a valid course mark between 0 and %.0f\n", new_course.maxMarks);
            }

            s[index].courses[s[index].number_of_courses] = new_course;
            s[index].number_of_courses++;
        }
        printf("Courses registered.\n");
        break;
    }
    }
}

void detailed_student_info()
{
    int id;
    if (get_number("Student ID: ", &id, 'i') != 1)
        return;

    if (id_search(id) == -1)
    {
        printf("Student doesnt exist in our system\n");
        return;
    }

    int index = id_search(id);
    students *st = &s[index];
    float total_percent = 0;
    int best = 0;
    int worst = 0;
    float best_percent, worst_percent;

    printf("\n===== STUDENT DETAILS =====\n");
    printf("ID:            %d\n", st->id);
    printf("Name:          %s %s\n", st->first_name, st->last_name);
    printf("Programme:     %s\n", st->programme);
    printf("Year of study: %d\n", st->year);
    printf("Courses:       %d\n", st->number_of_courses);

    if (st->number_of_courses == 0)
    {
        printf("\nNo courses registered yet; performance not available.\n");
        return;
    }

    printf("\n===== ACADEMIC PERFORMANCE =====\n");
    printf("%-8s %-28s %8s %6s %7s  %s\n", "Code", "Course", "Marks", "Max", "%", "Result");

    best_percent = st->courses[0].marks / st->courses[0].maxMarks * 100;
    worst_percent = best_percent;

    for (int j = 0; j < st->number_of_courses; j++)
    {
        course *c = &st->courses[j];
        float percent = c->marks / c->maxMarks * 100;

        total_percent += percent;

        if (percent > best_percent)
        {
            best_percent = percent;
            best = j;
        }
        if (percent < worst_percent)
        {
            worst_percent = percent;
            worst = j;
        }

        printf("%-8s %-28s %8.2f %6.0f %6.2f%%  %s\n",
               c->course_code, c->course_name, c->marks, c->maxMarks,
               percent, percent >= 50 ? "PASS" : "FAIL");
    }

    float average = total_percent / st->number_of_courses;

    printf("\nOverall average: %.2f%% (%s)\n", average, average >= 50 ? "PASS" : "FAIL");
    printf("Best course:     %s %s (%.2f%%)\n",
           st->courses[best].course_code, st->courses[best].course_name, best_percent);
    printf("Weakest course:  %s %s (%.2f%%)\n",
           st->courses[worst].course_code, st->courses[worst].course_name, worst_percent);
}

void remove_student()
{
    int id,pos,index;
    char choice[7];

    if (get_number("ID of the student you would love to remove: ", &id, 'i') != 1)
        return;

    /* Check the ID first, so the user is only asked to confirm a student that exists */
    index = id_search(id);
    if (index == -1)
    {
        printf("Student ID doesnt exists in our system\n");
        return;
    }

    get_string("Are you sure you want to proceed?\nType in 'yes' to proceed and any other letter to go back to the main menu: ", choice, sizeof(choice));

    if (strcasecmp(choice, "yes") != 0)
        return;

    /* Free the courses before the shift overwrites the only pointer to them */
    free(s[index].courses);

    /* Shift every record after the removed one back by one place */
    for (pos = index; pos < count - 1; pos ++)
        s[pos] = s[pos+1];

    count --;
    printf("\nStudent removed succesfully!\n");

    /* Shrink to exactly half. count <= capacity/2 means the records still fit.
       The capacity > 1 check stops it reaching 0, because realloc(s, 0) would free the array */
    if (capacity > 1 && count <= capacity/2)
    {
        students *temp;
        temp = realloc(s, (capacity / 2) * sizeof(students));
        if (temp == NULL)
        {
            printf("Memory allocation failed.\n");
            return;
        }
        s = temp;
        capacity /= 2;
    }
}

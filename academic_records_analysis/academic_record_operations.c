#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "helpers.h"
#include "academic_record_operations.h"

// Registers a new student with their courses and marks
void add_student()
{
    students new_student;
    course new_course;
    int id;

    // The file cannot be loaded with an ID below 1, so do not allow it
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

    get_name("Student First Name: ", new_student.first_name, sizeof(new_student.first_name), 0);
    get_name("Student Last Name: ", new_student.last_name, sizeof(new_student.last_name), 0);

    // Two students cannot have the same name
    if (name_search(new_student.first_name, new_student.last_name) != -1)
    {
        printf("A student with this name already exists in our system.\n");
        return;
    }

    get_name("Programme of study: ", new_student.programme, sizeof(new_student.programme), 1);

    // Keep asking until the year is valid
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

    // Make the student list bigger if it is full
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

// Shows a table of all students
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
    {
        char full_name[101];

        // Join the names first so the columns line up
        snprintf(full_name, sizeof(full_name), "%s %s", s[i].first_name, s[i].last_name);
        printf("%-10d %-25s %-25s\n", s[i].id, full_name, s[i].programme);
    }
    printf("---------------------------------------------------------\n");
    printf("Total students: %d\n", count);
}

// Updates the name, year, marks or courses of a student
void update_student_info()
{
    course new_course;
    int id, user_choice;
    int match;

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

        // Two students cannot have the same name
        match = name_search(f_name, s[index].last_name);
        if (match != -1 && match != index)
        {
            printf("A student with this name already exists in our system.\n");
            return;
        }

        strcpy(s[index].first_name, f_name);
        printf("First name updated.\n");
        break;
    }
    case 1:
    {
        char l_name[50];
        printf("Current last name: %s\n", s[index].last_name);
        get_name("Student's new last Name: ", l_name, sizeof(l_name), 0);

        // Two students cannot have the same name
        match = name_search(s[index].first_name, l_name);
        if (match != -1 && match != index)
        {
            printf("A student with this name already exists in our system.\n");
            return;
        }

        strcpy(s[index].last_name, l_name);
        printf("Last name updated.\n");
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
        printf("Year of study updated.\n");
        break;
    }
    case 3:
    {
        float marks;
        char course_code[15];
        get_string("Course code: ", course_code, sizeof(course_code));

        // Find the course the user typed
        match = -1;
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
            return;
        }

        while (1)
        {
            if (get_number("New marks: ", &marks, 'f') != 1)
                return;

            if (marks >= 0 && marks <= s[index].courses[match].maxMarks)
                break;

            printf("Input a valid course mark between 0 and %.0f\n", s[index].courses[match].maxMarks);
        }

        s[index].courses[match].marks = marks;
        printf("Marks updated.\n");
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
            return;
        }

        while (1)
        {
            if (get_number("Number of new courses you want to register: ", &new_count, 'i') != 1)
                return;

            if (new_count > 0 && new_count <= catalog_size - s[index].number_of_courses)
                break;

            printf("Input a number between 1 and %d\n", catalog_size - s[index].number_of_courses);
        }

        // Make room for the new courses
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
    case 5:
        printf("Update cancelled.\n");
        return;
    default:
        printf("Invalid choice.\n");
        return;
    }

    // Save now so the change is not lost if the program closes
    save_records();
}

// Shows the details and performance of one student
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

    float average = calculate_average(st);

    printf("\nOverall average: %.2f%% (%s)\n", average, average >= 50 ? "PASS" : "FAIL");
    printf("Best course:     %s %s (%.2f%%)\n",
           st->courses[best].course_code, st->courses[best].course_name, best_percent);
    printf("Weakest course:  %s %s (%.2f%%)\n",
           st->courses[worst].course_code, st->courses[worst].course_name, worst_percent);
}

// Removes a student after the user confirms
void remove_student()
{
    int id,pos,index;
    char choice[50];

    if (get_number("ID of the student you would love to remove: ", &id, 'i') != 1)
        return;

    // Check that the student exists before asking to confirm
    index = id_search(id);
    if (index == -1)
    {
        printf("Student ID doesnt exists in our system\n");
        return;
    }

    get_string("Are you sure you want to proceed?\nType in 'yes' to proceed and any other word to go back to the main menu: ", choice, sizeof(choice));

    if (strcasecmp(choice, "yes") != 0)
        return;

    // Free the courses before the record is overwritten
    free(s[index].courses);

    // Move every record after the removed one back by one place
    for (pos = index; pos < count - 1; pos ++)
        s[pos] = s[pos+1];

    count --;
    printf("\nStudent removed succesfully!\n");

    // Save now so the change is not lost if the program closes
    save_records();

    // Make the list half the size if it is half empty, but never 0
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

// Shows a report of all students, those with no courses are left out of the averages
void academic_report(void)
{
    int total_courses = 0;
    int graded = 0;
    int passing = 0;
    int highest = -1;
    int lowest = -1;
    float sum_of_averages = 0;
    float highest_average = 0;
    float lowest_average = 0;

    if (count == 0)
    {
        printf("No student records available.\n");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        float average;

        total_courses += s[i].number_of_courses;

        if (s[i].number_of_courses == 0)
            continue;

        average = calculate_average(&s[i]);
        sum_of_averages += average;
        graded++;

        if (average >= 50)
            passing++;

        // -1 means this is the first student with an average
        if (highest == -1 || average > highest_average)
        {
            highest = i;
            highest_average = average;
        }
        if (lowest == -1 || average < lowest_average)
        {
            lowest = i;
            lowest_average = average;
        }
    }

    printf("\n===== ACADEMIC REPORT =====\n");
    printf("Total registered students:  %d\n", count);
    printf("Total courses registered:   %d\n", total_courses);

    if (graded == 0)
        printf("No student has a course yet; performance not available.\n");
    else
    {
        printf("Overall average:            %.2f%%\n", sum_of_averages / graded);
        printf("Highest average:            %s %s, ID %d (%.2f%%)\n",
               s[highest].first_name, s[highest].last_name, s[highest].id, highest_average);
        printf("Lowest average:             %s %s, ID %d (%.2f%%)\n",
               s[lowest].first_name, s[lowest].last_name, s[lowest].id, lowest_average);
        printf("Students passing:           %d\n", passing);
        printf("Students failing:           %d\n", graded - passing);
        printf("Students with no courses:   %d\n", count - graded);
    }

    printf("\n===== PERFORMANCE PER COURSE =====\n");
    printf("%-8s %-28s %8s %7s %7s %9s\n", "Code", "Course", "Students", "Passed", "Failed", "Average");

    // Go through each course and add up the marks of its students
    for (int i = 0; i < catalog_size; i++)
    {
        int enrolled = 0;
        int passed = 0;
        float course_total = 0;

        for (int j = 0; j < count; j++)
        {
            for (int k = 0; k < s[j].number_of_courses; k++)
            {
                if (strcmp(s[j].courses[k].course_code, CATALOG[i].code) == 0)
                {
                    float percent = s[j].courses[k].marks / s[j].courses[k].maxMarks * 100;

                    course_total += percent;
                    enrolled++;
                    if (percent >= 50)
                        passed++;

                    // A student takes each course only once
                    break;
                }
            }
        }

        printf("%-8s %-28s %8d %7d %7d ", CATALOG[i].code, CATALOG[i].name,
               enrolled, passed, enrolled - passed);

        if (enrolled == 0)
            printf("%9s\n", "N/A");
        else
            printf("%8.2f%%\n", course_total / enrolled);
    }

    printf("\n===== STUDENTS PER PROGRAMME =====\n");

    for (int i = 0; i < count; i++)
    {
        int counted = 0;
        int in_programme = 0;

        // Skip the programme if it was already printed
        for (int j = 0; j < i; j++)
        {
            if (strcasecmp(s[j].programme, s[i].programme) == 0)
            {
                counted = 1;
                break;
            }
        }

        if (counted)
            continue;

        for (int j = i; j < count; j++)
        {
            if (strcasecmp(s[j].programme, s[i].programme) == 0)
                in_programme++;
        }

        printf("%-28s %d\n", s[i].programme, in_programme);
    }
}

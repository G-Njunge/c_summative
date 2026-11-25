#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "helpers.h"
#include "search_sort.h"

void search_student()
{
    int search_type, id = 0, found = 0, match;
    char text[50];

    if (get_number("\nSearch students by:\n"
                   "0. Student ID\n"
                   "1. First or last name\n"
                   "2. Programme of study\n"
                   "Enter your choice: ", &search_type, 'i') != 1)
        return;

    if (search_type == 0)
    {
        if (get_number("Student ID: ", &id, 'i') != 1)
            return;
    }
    else if (search_type == 1)
        get_name("First or last name: ", text, sizeof(text), 0);
    else if (search_type == 2)
        get_string("Programme of study: ", text, sizeof(text));
    else
    {
        printf("Input a value between 0 and 2\n");
        return;   
    }
    printf("\n%-8s %-25s %-25s %-5s %s\n", "ID", "Name", "Programme", "Year", "Courses");

    for (int i = 0; i < count; i++)
    {
        if (search_type == 0)
            match = (s[i].id == id);
        else if (search_type == 1)
            match = (strcasecmp(s[i].first_name, text) == 0 ||
                     strcasecmp(s[i].last_name, text) == 0);
        else
            match = (strcasecmp(s[i].programme, text) == 0);

        if (match)
        {
            char full_name[101];

            snprintf(full_name, sizeof(full_name), "%s %s", s[i].first_name, s[i].last_name);
            printf("%-8d %-25s %-25s %-5d %d\n",
                   s[i].id, full_name, s[i].programme, s[i].year, s[i].number_of_courses);
            found++;

            if (search_type == 0)
                break;
        }
    }

    if (found == 0)
        printf("No matching students found.\n");
    else
        printf("\n%d matching student(s) found.\n", found);
}

/* Insertion sort into a separate array, so the order of the saved records never changes */
void sort_students(void)
{
    int sort_type, pos, result;
    int name_type = 0;
    int descending;
    students *sorted;
    float *averages;

    if (count == 0)
    {
        printf("No student records available.\n");
        return;
    }

    if (get_number("\nSort students by:\n"
                   "0. Student ID\n"
                   "1. Student name\n"
                   "2. Overall average\n"
                   "3. Programme of study\n"
                   "Enter your choice: ", &sort_type, 'i') != 1)
        return;

    if (sort_type > 3)
    {
        printf("Input a value between 0 and 3\n");
        return;
    }

    if (sort_type == 1)
    {
        if (get_number("\nSort names by:\n"
                       "0. First name\n"
                       "1. Last name\n"
                       "Enter your choice: ", &name_type, 'i') != 1)
            return;

        if (name_type > 1)
        {
            printf("Input either 0 or 1\n");
            return;
        }
    }

    if (get_number("\nDisplay in:\n"
                   "0. Ascending order\n"
                   "1. Descending order\n"
                   "Enter your choice: ", &descending, 'i') != 1)
        return;

    if (descending > 1)
    {
        printf("Input either 0 or 1\n");
        return;
    }

    /* averages[i] always belongs to the student in sorted[i] */
    sorted = malloc(count * sizeof(students));
    averages = malloc(count * sizeof(float));
    if (sorted == NULL || averages == NULL)
    {
        printf("Memory allocation failed.\n");
        free(sorted);
        free(averages);
        return;
    }

    for (int i = 0; i < count; i++)
    {
        /* A student with no courses gets an impossible average that puts them last in either order */
        float average = descending ? -1 : 1000;

        if (s[i].number_of_courses > 0)
        {
            float total = 0;

            for (int j = 0; j < s[i].number_of_courses; j++)
                total += s[i].courses[j].marks / s[i].courses[j].maxMarks * 100;

            average = total / s[i].number_of_courses;
        }

        /* Start at the end of the sorted part and move left until the student's place is found */
        for (pos = i; pos > 0; pos--)
        {
            /* result > 0: left student is bigger, result < 0: left student is smaller, 0: equal */
            if (sort_type == 0)
                result = (sorted[pos - 1].id > s[i].id) - (sorted[pos - 1].id < s[i].id);
            else if (sort_type == 1 && name_type == 0)
            {
                result = strcasecmp(sorted[pos - 1].first_name, s[i].first_name);
                if (result == 0)
                    result = strcasecmp(sorted[pos - 1].last_name, s[i].last_name);
            }
            else if (sort_type == 1)
            {
                result = strcasecmp(sorted[pos - 1].last_name, s[i].last_name);
                if (result == 0)
                    result = strcasecmp(sorted[pos - 1].first_name, s[i].first_name);
            }
            else if (sort_type == 2)
                result = (averages[pos - 1] > average) - (averages[pos - 1] < average);
            else
                result = strcasecmp(sorted[pos - 1].programme, s[i].programme);

            /* Descending order is the opposite comparison */
            if (descending)
                result = -result;

            /* The left student already belongs before this one, so the place is found */
            if (result <= 0)
                break;

            /* Shift the student on the left one place right to make room */
            sorted[pos] = sorted[pos - 1];
            averages[pos] = averages[pos - 1];
        }

        sorted[pos] = s[i];
        averages[pos] = average;
    }

    printf("\n%-8s %-25s %-25s %s\n", "ID", "Name", "Programme", "Average");

    for (int i = 0; i < count; i++)
    {
        char full_name[101];

        snprintf(full_name, sizeof(full_name), "%s %s", sorted[i].first_name, sorted[i].last_name);
        printf("%-8d %-25s %-25s ", sorted[i].id, full_name, sorted[i].programme);

        if (sorted[i].number_of_courses == 0)
            printf("N/A\n");
        else
            printf("%.2f%%\n", averages[i]);
    }

    /* The copies share their courses with s, so only the two arrays are freed here */
    free(sorted);
    free(averages);
}

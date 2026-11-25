#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "helpers.h"
#include "search_sort.h"

// Searches for students by ID, name or programme using linear search
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

    // Check every student one by one
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

// Sorts a copy of the students using insertion sort, so the saved order does not change
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

    // averages[i] is the average of the student in sorted[i]
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
        float average = calculate_average(&s[i]);

        // Students with no courses always come last, so use a very big average here
        if (average < 0 && !descending)
            average = 1000;

        // Move left through the sorted students until the right place is found
        for (pos = i; pos > 0; pos--)
        {
            // Positive means the left student is bigger and negative means smaller
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

            // Flip the result for descending order
            if (descending)
                result = -result;

            // Stop if the left student should stay before this one
            if (result <= 0)
                break;

            // Move the left student one place right to make room
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

    // Do not free the courses here because they still belong to s
    free(sorted);
    free(averages);
}

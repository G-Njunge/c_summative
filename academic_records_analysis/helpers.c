#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <strings.h>
#include "helpers.h"

// Asks for a number until it is valid, type 'i' is for int and 'f' is for float
int get_number(const char *prompt, void *value, char type)
{
    char line[64];
    char *end;
    int ch;
    long whole = 0;
    float decimal = 0;

    if (type != 'i' && type != 'f')
        return 0;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(line, sizeof(line), stdin) == NULL)
            return EOF;

        // Clear the rest of the line if it was too long
        if (strchr(line, '\n') == NULL && !feof(stdin))
        {
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            printf("Input too long. Try again.\n");
            continue;
        }

        errno = 0;
        if (type == 'i')
            whole = strtol(line, &end, 10);
        else
            decimal = strtof(line, &end);

        // Nothing was read, so the line has no number
        if (end == line)
        {
            printf("Invalid input. Try again.\n");
            continue;
        }

        // Only spaces are allowed after the number
        while (isspace((unsigned char)*end))
            end++;

        if (*end != '\0' || (type == 'f' && isnan(decimal)))
        {
            printf("Invalid input. Try again.\n");
            continue;
        }

        // Reject negative numbers
        if ((type == 'i' && whole < 0) || (type == 'f' && decimal < 0))
        {
            printf("Negative numbers are not allowed. Try again.\n");
            continue;
        }

        // Reject numbers that do not fit
        if (errno == ERANGE || (type == 'i' && whole > INT_MAX) || (type == 'f' && isinf(decimal)))
        {
            printf("Number is out of range. Try again.\n");
            continue;
        }

        if (type == 'i')
            *(int *)value = (int)whole;
        else
            *(float *)value = decimal;

        return 1;
    }
}

// Asks for text until it is not empty, not too long and has a letter
void get_string(const char *prompt, char *value, size_t size)
{
    int ch;
    int has_letter;
    size_t len;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(value, size, stdin) == NULL)
        {
            free_records();
            exit(0);
        }

        len = strlen(value);

        if (len > 0 && value[len - 1] != '\n')
        {
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            printf("Input too long. Try again.\n");
            continue;
        }

        // Remove the newline and any spaces at the end
        while (len > 0 && isspace((unsigned char)value[len - 1]))
            value[--len] = '\0';

        // Remove any spaces at the start, the + 1 also moves the '\0'
        int start = 0;
        while (isspace((unsigned char)value[start]))
            start++;
        memmove(value, value + start, len - start + 1);

        if (value[0] == '\0')
        {
            printf("Input cannot be empty.\n");
            continue;
        }

        has_letter = 0;

        for (int i = 0; value[i] != '\0'; i++)
        {
            if (isalpha((unsigned char)value[i]))
            {
                has_letter = 1;
                break;
            }
        }

        if (!has_letter)
        {
            printf("Input must contain at least one letter.\n");
            continue;
        }

        return;
    }
}

// Checks that text has two or more letters and only allowed characters
int valid_text(const char *value, int allow_digits)
{
    int letters = 0;

    for (int i = 0; value[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)value[i];

        if (isalpha(c))
            letters++;
        else if (c == ' ' || c == '-' || c == '\'' || c == '.')
            continue;
        else if (allow_digits && isdigit(c))
            continue;
        else
            return 0;
    }

    return letters >= 2;
}

// Asks for text until it looks like a real name or programme
void get_name(const char *prompt, char *value, size_t size, int allow_digits)
{
    while (1)
    {
        get_string(prompt, value, size);

        if (valid_text(value, allow_digits))
            return;

        if (allow_digits)
            printf("Use at least two letters, and only letters, digits, spaces, - ' and . characters.\n");
        else
            printf("Use at least two letters, and only letters, spaces, - ' and . characters.\n");
    }
}

// Returns the position of the student with this ID, or -1 if not found
int id_search(int id)
{
    for (int i = 0; i < count; i++)
    {
        if (id == s[i].id)
            return i;
    }

    return -1;
}

// Returns the position of the student with this name, or -1 if not found
int name_search(const char *first_name, const char *last_name )
{
    for (int i = 0; i < count; i++)
    {
        if (strcasecmp(first_name, s[i].first_name) == 0 && strcasecmp(last_name, s[i].last_name) == 0)
            return i;
    }

    return -1;
}

// Saves all students and their courses to the file, returns 1 if it worked and 0 if not
int save_records(void)
{
    FILE *fp;
    students *st;
    course *c;
    int i, j;

    fp = fopen("students.txt", "w");
    if (fp == NULL)
    {
        printf("Error: cannot open students.txt for writing.\n");
        return (0);
    }

    fprintf(fp, "%d\n", count);

    for (i = 0; i < count; i++)
    {
        st = &s[i];
        fprintf(fp, "%d|%s|%s|%s|%d|%d\n",
                st->id, st->first_name, st->last_name,
                st->programme, st->year, st->number_of_courses);

        for (j = 0; j < st->number_of_courses; j++)
        {
            c = &st->courses[j];
            fprintf(fp, "%s|%s|%.2f|%.2f\n",
                    c->course_code, c->course_name, c->marks, c->maxMarks);
        }
    }

    if (fclose(fp) != 0)
    {
        printf("Error: could not finish writing students.txt.\n");
        return (0);
    }
    printf("Saved %d students to students.txt.\n", count);
    return (1);
}

// Loads the students from the file, returns 1 if it worked or there is no file, and 0 if it failed
int load_records(void)
{
    FILE *fp;
    char line[256];
    int lineNum = 0;
    int total, i, j;
    int end = 0;
    students *st;

    fp = fopen("students.txt", "r");
    if (fp == NULL)
    {
        if (errno == ENOENT)
        {
            printf("No saved records found. Starting with an empty list.\n");
            return (1);
        }
        printf("Error: cannot open students.txt for reading.\n");
        return (0);
    }

    lineNum++;
    // Line 1 must have only the number of students, the 9 stops numbers that are too big
    if (fgets(line, sizeof(line), fp) == NULL ||
        sscanf(line, "%9d %n", &total, &end) != 1 || line[end] != '\0' || total < 0)
        return (load_failed(fp, lineNum));

    if (total > 0)
    {
        s = malloc(total * sizeof(students));
        if (s == NULL)
        {
            printf("Error: not enough memory to load records.\n");
            fclose(fp);
            return (0);
        }
    }
    capacity = total;
    count = 0;

    for (i = 0; i < total; i++)
    {
        st = &s[count];
        if (!read_student_line(fp, st, &lineNum))
            return (load_failed(fp, lineNum));

        st->courses = NULL;
        if (st->number_of_courses > 0)
        {
            st->courses = calloc(st->number_of_courses, sizeof(course));
            if (st->courses == NULL)
            {
                printf("Error: not enough memory to load records.\n");
                free_records();
                fclose(fp);
                return (0);
            }
        }
        count++;

        for (j = 0; j < st->number_of_courses; j++)
            if (!read_course_line(fp, st, j, &lineNum))
                return (load_failed(fp, lineNum));
    }

    // Anything left means line 1 is wrong, so stop or the extra records would be lost
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        lineNum++;
        if (strspn(line, " \t\r\n") != strlen(line))
        {
            printf("Error: there are more records than the number on line 1.\n");
            return (load_failed(fp, lineNum));
        }
    }

    fclose(fp);
    printf("Loaded %d students from students.txt.\n", count);
    return (1);
}

// Reads and checks one student line, returns 1 if it is valid and 0 if not
int read_student_line(FILE *fp, students *st, int *lineNum)
{
    char line[256];
    long long id;
    int end = 0;
    int i;

    (*lineNum)++;
    if (fgets(line, sizeof(line), fp) == NULL)
        return (0);

    // The sizes stop numbers that are too big, and end checks for extra text on the line
    if (sscanf(line, "%10lld|%49[^|]|%49[^|]|%49[^|]|%4d|%4d %n",
               &id, st->first_name, st->last_name,
               st->programme, &st->year, &st->number_of_courses, &end) != 6 || line[end] != '\0')
        return (0);

    if (id > INT_MAX)
        return (0);
    st->id = (int)id;

    // Names and programmes follow the same rules as when the user types them
    if (!valid_text(st->first_name, 0) || !valid_text(st->last_name, 0) ||
        !valid_text(st->programme, 1) || name_search(st->first_name, st->last_name) != -1)
        return (0);

    if (st->id <= 0 || st->year < 1 || st->year > 4 ||
        st->number_of_courses < 0 || st->number_of_courses > catalog_size)
        return (0);

    for (i = 0; i < count; i++)
        if (s[i].id == st->id)
            return (0);

    return (1);
}

// Reads and checks one course line, returns 1 if it is valid and 0 if not
int read_course_line(FILE *fp, students *st, int index, int *lineNum)
{
    char line[256];
    course *c = &st->courses[index];
    const CourseInfo *info;
    int end = 0;
    int k;

    (*lineNum)++;
    if (fgets(line, sizeof(line), fp) == NULL)
        return (0);

    if (sscanf(line, "%14[^|]|%49[^|]|%f|%f %n",
               c->course_code, c->course_name, &c->marks, &c->maxMarks, &end) != 4 || line[end] != '\0')
        return (0);

    info = find_catalog_course(c->course_code);
    if (info == NULL)
        return (0);

    // The course name and maximum marks must match the catalog
    if (strcmp(c->course_name, info->name) != 0 || c->maxMarks != info->maxMarks)
        return (0);

    // Written this way so that a mark of nan is also rejected
    if (!(c->marks >= 0 && c->marks <= c->maxMarks))
        return (0);

    for (k = 0; k < index; k++)
        if (strcmp(st->courses[k].course_code, c->course_code) == 0)
            return (0);

    return (1);
}

// Shows which line is bad, frees the data and closes the file, always returns 0
int load_failed(FILE *fp, int lineNum)
{
    printf("Error: invalid record on line %d of students.txt.\n", lineNum);
    free_records();
    fclose(fp);
    return (0);
}

// Frees the courses of each student first, then the student list
void free_records(void)
{
    int i;

    for (i = 0; i < count; i++)
        free(s[i].courses);
    free(s);
    s = NULL;
    count = 0;
    capacity = 0;
}

// Returns the course with this code, or NULL if it does not exist
const CourseInfo *find_catalog_course(const char *code)
{
    int i;

    for (i = 0; i < catalog_size; i++)
        if (strcmp(CATALOG[i].code, code) == 0)
            return (&CATALOG[i]);
    return (NULL);
}

// Returns the average percentage of a student, or -1 if they have no courses
float calculate_average(const students *st)
{
    float total = 0;

    // Avoid dividing by zero
    if (st->number_of_courses == 0)
        return -1;

    for (int i = 0; i < st->number_of_courses; i++)
        total += st->courses[i].marks / st->courses[i].maxMarks * 100;

    return total / st->number_of_courses;
}

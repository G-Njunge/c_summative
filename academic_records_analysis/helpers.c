#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include "helpers.h"

int get_number(const char *prompt, void *value, char type)
{
    int result;
    int ch;
    int invalid;

    while (1)
    {
        printf("%s", prompt);

        if (type == 'i')
            result = scanf("%d", (int *)value);
        else if (type == 'f')
            result = scanf("%f", (float *)value);
        else
            return 0;

        if (result == EOF)
            return EOF;

        if (result != 1)
        {
            printf("Invalid input. Try again.\n");

            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            continue;
        }

        /* Read the rest of the line. Anything other than spaces or tabs makes it invalid */
        invalid = 0;

        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            if (ch != ' ' && ch != '\t')
                invalid = 1;
        }

        if (invalid)
        {
            printf("Invalid input. Try again.\n");
            continue;
        }

        /* Reject negative numbers */
        if ((type == 'i' && *(int *)value < 0) ||
            (type == 'f' && *(float *)value < 0))
        {
            printf("Negative numbers are not allowed. Try again.\n");
            continue;
        }

        return 1;
    }
}
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

        /* Remove the newline and any spaces or tabs at the end */
        while (len > 0 && isspace((unsigned char)value[len - 1]))
            value[--len] = '\0';

        /* Remove any spaces or tabs at the start. The + 1 moves the '\0' as well */
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

/* A valid name or course has at least two letters and only letters, spaces, - ' . (plus digits for a course) */
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

/* Same as get_string, but keeps asking until the text looks like a real name or course */
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

int id_search(int id)
{
    for (int i = 0; i < count; i++)
    {
        if (id == s[i].id)
            return i;
    }

    return -1;
}

/* Case-insensitive, the same way names are compared everywhere else */
int name_search(const char *first_name, const char *last_name )
{
    for (int i = 0; i < count; i++)
    {
        if (strcasecmp(first_name, s[i].first_name) == 0 && strcasecmp(last_name, s[i].last_name) == 0)
            return i;
    }

    return -1;
}

/* Writes all students and their courses to the file; returns 1 on success, 0 on failure */
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

/* Rebuilds the student list from the file at start-up; returns 1 on success or missing file, 0 on failure */
int load_records(void)
{
    FILE *fp;
    char line[256];
    int lineNum = 0;
    int total, i, j;
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
    if (fgets(line, sizeof(line), fp) == NULL ||
        sscanf(line, "%d", &total) != 1 || total < 0)
        return (load_failed(fp, lineNum));

    if (total == 0)
    {
        fclose(fp);
        printf("Loaded 0 students from students.txt.\n");
        return (1);
    }

    s = malloc(total * sizeof(students));
    if (s == NULL)
    {
        printf("Error: not enough memory to load records.\n");
        fclose(fp);
        return (0);
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

    fclose(fp);
    printf("Loaded %d students from students.txt.\n", count);
    return (1);
}

/* Reads and validates one student line; returns 1 if valid, 0 otherwise */
int read_student_line(FILE *fp, students *st, int *lineNum)
{
    char line[256];
    int i;

    (*lineNum)++;
    if (fgets(line, sizeof(line), fp) == NULL)
        return (0);

    if (sscanf(line, "%d|%49[^|]|%49[^|]|%49[^|]|%d|%d",
               &st->id, st->first_name, st->last_name,
               st->programme, &st->year, &st->number_of_courses) != 6)
        return (0);

    if (st->id <= 0 || st->year < 1 || st->year > 6 ||
        st->number_of_courses < 0 || st->number_of_courses > catalog_size)
        return (0);

    for (i = 0; i < count; i++)
        if (s[i].id == st->id)
            return (0);

    return (1);
}

/* Reads and validates one course line against the catalog; returns 1 if valid, 0 otherwise */
int read_course_line(FILE *fp, students *st, int index, int *lineNum)
{
    char line[256];
    course *c = &st->courses[index];
    const CourseInfo *info;
    int k;

    (*lineNum)++;
    if (fgets(line, sizeof(line), fp) == NULL)
        return (0);

    if (sscanf(line, "%14[^|]|%49[^|]|%f|%f",
               c->course_code, c->course_name, &c->marks, &c->maxMarks) != 4)
        return (0);

    info = find_catalog_course(c->course_code);
    if (info == NULL)
        return (0);

    strcpy(c->course_name, info->name);
    c->maxMarks = info->maxMarks;

    if (c->marks < 0 || c->marks > c->maxMarks)
        return (0);

    for (k = 0; k < index; k++)
        if (strcmp(st->courses[k].course_code, c->course_code) == 0)
            return (0);

    return (1);
}

/* Reports the bad line, frees loaded data and closes the file; always returns 0 */
int load_failed(FILE *fp, int lineNum)
{
    printf("Error: invalid record on line %d of students.txt.\n", lineNum);
    free_records();
    fclose(fp);
    return (0);
}

/* Frees each student's courses first, then the student array */
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

/* Returns the catalog entry for a course code, or NULL if it doesn't exist */
const CourseInfo *find_catalog_course(const char *code)
{
    int i;

    for (i = 0; i < catalog_size; i++)
        if (strcmp(CATALOG[i].code, code) == 0)
            return (&CATALOG[i]);
    return (NULL);
}

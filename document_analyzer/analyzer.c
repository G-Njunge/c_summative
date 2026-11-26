#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "analyzer.h"
#include "report.h"

// Fills in a thread's struct before the thread starts, returns 1 if it worked and 0 if not
int init_thread_args(ThreadArgs *args, int thread_number, const char *file_name, const char *keyword)
{
    size_t length = strlen(file_name);

    args->created = 0;
    args->thread_number = thread_number;
    args->file_name = file_name;
    args->keyword = keyword;
    args->total_lines = 0;
    args->total_occurrences = 0;
    args->matching_lines = 0;
    args->capacity = 0;
    args->line_numbers = NULL;

    // Starts as failed, only a thread that finishes everything sets it to 1
    args->success = 0;

    // Look backwards for the last dot, but stop at a slash so folder names are not cut
    for (size_t i = length; i > 0 && file_name[i - 1] != '/' && file_name[i - 1] != '\\'; i--)
    {
        if (file_name[i - 1] == '.')
        {
            length = i - 1;
            break;
        }
    }

    // The size of the ending includes its '\0'
    args->report_name = malloc(length + sizeof("_report.txt"));
    if (args->report_name == NULL)
        return 0;

    memcpy(args->report_name, file_name, length);
    strcpy(args->report_name + length, "_report.txt");
    return 1;
}

// Counts how many times the keyword is in the line, ignoring capitals, main checks that the keyword is not empty
int count_keyword(const char *line, const char *keyword)
{
    size_t length = strlen(keyword);
    size_t i = 0;
    int count = 0;

    while (line[i] != '\0')
    {
        if (strncasecmp(line + i, keyword, length) == 0)
        {
            count++;

            // Skip past this match so matches cannot overlap
            i += length;
        }
        else
        {
            i++;
        }
    }

    return count;
}

// Thread function that reads one file, puts the results in the thread's struct and writes the report
void *process_file(void *arg)
{
    ThreadArgs *args = arg;
    const char *error = NULL;
    char *buffer = NULL;
    size_t length = 0;
    size_t buffer_size = 128;
    int ch = 0;
    FILE *fp;

    printf("[Thread %d] Processing %s ...\n", args->thread_number, args->file_name);

    fp = fopen(args->file_name, "rb");
    if (fp == NULL)
    {
        error = "cannot open";
    }
    else
    {
        buffer = malloc(buffer_size);
        if (buffer == NULL)
            error = "memory allocation failed for";
    }

    // Read one character at a time until the end of the file or an error
    while (error == NULL && ch != EOF)
    {
        ch = fgetc(fp);

        // Text files do not have zero bytes
        if (ch == '\0')
        {
            error = "not a plain text file:";
        }
        else if (ch != '\n' && ch != EOF)
        {
            // Double the buffer when it is full, leaving room for the '\0'
            if (length + 1 >= buffer_size)
            {
                char *temp = realloc(buffer, buffer_size * 2);

                if (temp == NULL)
                {
                    error = "memory allocation failed for";
                    continue;
                }

                buffer = temp;
                buffer_size *= 2;
            }

            buffer[length] = (char)ch;
            length++;
        }
        else if (ch == '\n' || length > 0)
        {
            // The line is complete, the last line may not end with a newline
            int found;

            buffer[length] = '\0';
            length = 0;
            args->total_lines++;

            found = count_keyword(buffer, args->keyword);
            if (found == 0)
                continue;

            // Double the list of line numbers when it is full
            if (args->matching_lines >= args->capacity)
            {
                int new_capacity = args->capacity == 0 ? 4 : args->capacity * 2;
                int *temp = realloc(args->line_numbers, new_capacity * sizeof(int));

                if (temp == NULL)
                {
                    error = "memory allocation failed for";
                    continue;
                }

                args->line_numbers = temp;
                args->capacity = new_capacity;
            }

            args->line_numbers[args->matching_lines] = args->total_lines;
            args->matching_lines++;
            args->total_occurrences += found;
        }
    }

    // Check if reading stopped because of an error and not the end of the file
    if (error == NULL && ferror(fp))
        error = "cannot read";

    // Only this function knows about the buffer, so it must free it
    free(buffer);
    if (fp != NULL)
        fclose(fp);

    if (error != NULL)
        printf("[Thread %d] Error: %s %s.\n", args->thread_number, error, args->file_name);
    else if (args->total_occurrences == 0)
        printf("[Thread %d] Keyword not found in %s\n", args->thread_number, args->file_name);

    // A failed file still gets a report that says it failed
    if (!generate_report(args, error) || error != NULL)
        return NULL;

    // Set last, so it is only 1 if everything worked
    args->success = 1;
    printf("[Thread %d] Successfully processed %s\n", args->thread_number, args->file_name);
    return NULL;
}

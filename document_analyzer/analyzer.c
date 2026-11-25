#include <stddef.h>
#include <string.h>
#include <strings.h>
#include "analyzer.h"

// Fills in a thread's struct before the thread starts
void init_thread_args(ThreadArgs *args, int thread_number, const char *file_name, const char *keyword)
{
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

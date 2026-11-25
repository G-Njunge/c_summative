#ifndef ANALYZER_H
#define ANALYZER_H

// Each thread gets its own struct, so no two threads write to the same memory
typedef struct {
    int thread_number;
    // These point to text in argv, so they are not freed
    const char *file_name;
    const char *keyword;
    int total_lines;
    int total_occurrences;
    // Number of matching lines, which is also the number of line numbers stored
    int matching_lines;
    int capacity;
    // Grows with realloc, main frees it after joining
    int *line_numbers;
    int success;
} ThreadArgs;

void init_thread_args(ThreadArgs *args, int thread_number, const char *file_name, const char *keyword);
int count_keyword(const char *line, const char *keyword);

#endif

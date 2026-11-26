#ifndef ANALYZER_H
#define ANALYZER_H

#include <pthread.h>

// Each thread gets its own struct, so no two threads write to the same memory
typedef struct {
    pthread_t thread;
    // Set to 1 by main if pthread_create worked, only then can it be joined
    int created;
    int thread_number;
    // These point to text in argv, so they are not freed
    const char *file_name;
    const char *keyword;
    // Made by init_thread_args with malloc, main frees it
    char *report_name;
    int total_lines;
    int total_occurrences;
    // Number of matching lines, which is also the number of line numbers stored
    int matching_lines;
    int capacity;
    // Grows with realloc, main frees it after joining
    int *line_numbers;
    int success;
} ThreadArgs;

int init_thread_args(ThreadArgs *args, int thread_number, const char *file_name, const char *keyword);
int count_keyword(const char *line, const char *keyword);

// Thread function, pthread_create needs exactly this shape
void *process_file(void *arg);

#endif

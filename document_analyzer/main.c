#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <pthread.h>
#include "analyzer.h"
#include "report.h"

// Starts one thread for each file, waits for all of them, then shows the results
int main(int argc, char *argv[])
{
    ThreadArgs *args;
    int count = argc - 2;
    int ready = 0;
    int failed = 0;

    if (argc < 3)
    {
        printf("Usage: ./document_analyzer \"keyword\" file1 file2 ...\n");
        return 1;
    }

    // An empty keyword would make the search loop forever
    if (argv[1][0] == '\0')
    {
        printf("Error: the keyword cannot be empty.\n");
        return 1;
    }

    args = malloc(count * sizeof(ThreadArgs));
    if (args == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // ready counts the structs that are set up, so cleanup knows where to stop
    while (ready < count && init_thread_args(&args[ready], ready + 1, argv[ready + 2], argv[1]))
        ready++;

    if (ready < count)
    {
        printf("Memory allocation failed.\n");
        failed = 1;
    }

    // A report must not replace another report or an input file, Windows ignores capitals in file names
    for (int i = 0; i < count && !failed; i++)
    {
        for (int j = 0; j < count && !failed; j++)
        {
            if (j > i && strcasecmp(args[i].report_name, args[j].report_name) == 0)
            {
                printf("Error: %s and %s would both use the report %s.\n", args[i].file_name, args[j].file_name, args[i].report_name);
                failed = 1;
            }
            else if (strcasecmp(args[i].report_name, args[j].file_name) == 0)
            {
                printf("Error: the report of %s would replace the input file %s.\n", args[i].file_name, args[j].file_name);
                failed = 1;
            }
        }
    }

    if (!failed)
    {
        printf("===== MULTI-THREADED DOCUMENT ANALYZER =====\n");
        printf("Search Keyword: %s\n", argv[1]);
        printf("Number of Input Files: %d\n\n", count);

        for (int i = 0; i < count; i++)
        {
            printf("[Main] Creating thread %d for %s\n", i + 1, args[i].file_name);

            // 0 means it worked, anything else is an error
            if (pthread_create(&args[i].thread, NULL, process_file, &args[i]) == 0)
            {
                args[i].created = 1;
            }
            else
            {
                printf("[Main] Error: cannot create thread %d.\n", i + 1);
                generate_report(&args[i], "cannot create a thread for");
            }
        }

        // Join in a second loop so the threads run at the same time
        for (int i = 0; i < count; i++)
        {
            if (args[i].created && pthread_join(args[i].thread, NULL) != 0)
                printf("[Main] Error: cannot join thread %d.\n", i + 1);
        }

        failed = display_results(args, count);
    }

    // All threads are finished, so free what is inside each struct before the array
    for (int i = 0; i < ready; i++)
    {
        free(args[i].line_numbers);
        free(args[i].report_name);
    }
    free(args);

    return failed > 0 ? 1 : 0;
}

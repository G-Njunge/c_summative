#include <stdio.h>
#include "analyzer.h"
#include "report.h"

// Writes the report of one file, returns 1 if it worked and 0 if not
int generate_report(const ThreadArgs *args, const char *error)
{
    FILE *fp = fopen(args->report_name, "w");
    int failed = 0;

    if (fp == NULL)
    {
        printf("[Thread %d] Error: cannot create %s.\n", args->thread_number, args->report_name);
        return 0;
    }

    fprintf(fp, "===== DOCUMENT ANALYSIS REPORT =====\n");
    fprintf(fp, "Input File: %s\n", args->file_name);
    fprintf(fp, "Search Keyword: %s\n", args->keyword);
    fprintf(fp, "Total Lines Processed: %d\n", args->total_lines);
    fprintf(fp, "Total Keyword Occurrences: %d\n", args->total_occurrences);
    fprintf(fp, "Number of Matching Lines: %d\n", args->matching_lines);

    if (args->matching_lines == 0)
    {
        fprintf(fp, "Matching Line Numbers: None\n");
    }
    else
    {
        fprintf(fp, "Matching Line Numbers: ");

        // Put a comma before every number except the first
        for (int i = 0; i < args->matching_lines; i++)
            fprintf(fp, "%s%d", i == 0 ? "" : ", ", args->line_numbers[i]);

        fprintf(fp, "\n");
    }

    if (error != NULL)
        fprintf(fp, "Processing Status: Failed, %s %s\n", error, args->file_name);
    else if (args->matching_lines == 0)
        fprintf(fp, "Processing Status: Successful, keyword not found\n");
    else
        fprintf(fp, "Processing Status: Successful\n");

    // fclose can fail too, because the last text is only written then
    if (ferror(fp))
        failed = 1;
    if (fclose(fp) != 0)
        failed = 1;

    if (failed)
    {
        // Do not leave half a report behind
        remove(args->report_name);
        printf("[Thread %d] Error: cannot write to %s.\n", args->thread_number, args->report_name);
        return 0;
    }

    return 1;
}

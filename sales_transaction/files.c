#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "helpers.h"
#include "files.h"

#define FILE_NAME "transactions.txt"
#define TEMP_NAME "transactions.tmp"

// Saves all transactions, returns 1 if it worked and 0 if not
int save_transactions(const Transaction *list, int count)
{
    FILE *fp = fopen(TEMP_NAME, "w");
    int failed = 0;

    if (fp == NULL)
    {
        printf("Error: cannot open %s for writing.\n", TEMP_NAME);
        return 0;
    }

    // Write to a temporary file first so the real file is safe if this fails
    for (int i = 0; i < count; i++)
    {
        if (fprintf(fp, "%d|%s|%d|%lld|%d|%d|%d\n", list[i].id, list[i].product_name, list[i].quantity,
                    list[i].price, list[i].date.day, list[i].date.month, list[i].date.year) < 0)
            failed = 1;
    }

    if (fclose(fp) != 0)
        failed = 1;

    if (failed)
    {
        printf("Error: cannot write to %s. The old file was not changed.\n", TEMP_NAME);
        remove(TEMP_NAME);
        return 0;
    }

    // The old file must be removed first because rename cannot replace it on Windows
    if (remove(FILE_NAME) != 0 && errno != ENOENT)
    {
        printf("Error: cannot replace %s. The old file was not changed.\n", FILE_NAME);
        remove(TEMP_NAME);
        return 0;
    }

    if (rename(TEMP_NAME, FILE_NAME) != 0)
    {
        printf("Error: cannot rename %s. Your records are in that file.\n", TEMP_NAME);
        return 0;
    }

    printf("Saved %d transaction(s) to %s.\n", count, FILE_NAME);
    return 1;
}

// Loads the transactions from the file, returns 1 if it worked or there is no file, and 0 if it failed
int load_transactions(Transaction **list, int *count, int *capacity)
{
    FILE *fp;
    char line[256];
    int line_number = 0;
    int new_count = 0;
    int new_capacity = 4;
    Transaction *loaded;
    Transaction t;

    fp = fopen(FILE_NAME, "r");
    if (fp == NULL)
    {
        if (errno == ENOENT)
        {
            printf("No saved transactions found.\n");
            return 1;
        }

        printf("Error: cannot open %s for reading.\n", FILE_NAME);
        return 0;
    }

    // Load into a separate array so the records in memory are safe if this fails
    loaded = malloc(new_capacity * sizeof(Transaction));
    if (loaded == NULL)
    {
        printf("Memory allocation failed.\n");
        fclose(fp);
        return 0;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line_number++;

        // Skip empty lines
        if (strspn(line, " \t\r\n") == strlen(line))
            continue;

        if (!read_transaction_line(line, &t) || find_by_id(loaded, new_count, t.id) != -1)
        {
            printf("Error: invalid record on line %d of %s.\n", line_number, FILE_NAME);
            free(loaded);
            fclose(fp);
            return 0;
        }

        if (new_count >= new_capacity)
        {
            Transaction *temp = realloc(loaded, new_capacity * 2 * sizeof(Transaction));

            if (temp == NULL)
            {
                printf("Memory allocation failed.\n");
                free(loaded);
                fclose(fp);
                return 0;
            }

            loaded = temp;
            new_capacity *= 2;
        }

        loaded[new_count] = t;
        new_count++;
    }

    // Check if reading stopped because of an error and not the end of the file
    if (ferror(fp))
    {
        printf("Error: cannot read %s.\n", FILE_NAME);
        free(loaded);
        fclose(fp);
        return 0;
    }

    fclose(fp);

    // Everything is valid, so free the old array and use the new one
    free(*list);
    *list = loaded;
    *count = new_count;
    *capacity = new_capacity;
    printf("Loaded %d transaction(s) from %s.\n", new_count, FILE_NAME);
    return 1;
}

// Reads and checks one line of the file, returns 1 if it is valid and 0 if not
int read_transaction_line(const char *line, Transaction *t)
{
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    long long id;
    int end = 0;
    size_t len;

    // The sizes stop numbers that are too big, and end checks for extra text on the line
    if (sscanf(line, "%10lld|%49[^|]|%7d|%10lld|%2d|%2d|%4d %n", &id, t->product_name, &t->quantity,
               &t->price, &t->date.day, &t->date.month, &t->date.year, &end) != 7 || line[end] != '\0')
        return 0;

    if (id < 1 || id > INT_MAX)
        return 0;
    t->id = (int)id;

    // Names cannot start or end with a space
    len = strlen(t->product_name);
    if (!valid_text(t->product_name, 1) || t->product_name[0] == ' ' || t->product_name[len - 1] == ' ')
        return 0;

    if (t->quantity < 1 || t->quantity > 1000000 || t->price < 1 || t->price > 100000000)
        return 0;

    // February has 29 days in a leap year
    if ((t->date.year % 4 == 0 && t->date.year % 100 != 0) || t->date.year % 400 == 0)
        days[1] = 29;

    if (t->date.year < 2000 || t->date.year > 2100 || t->date.month < 1 || t->date.month > 12 ||
        t->date.day < 1 || t->date.day > days[t->date.month - 1])
        return 0;

    t->total_amount = t->quantity * t->price;
    return 1;
}

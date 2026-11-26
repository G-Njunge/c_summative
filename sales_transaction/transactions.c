#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include "transactions.h"

#define FILE_NAME "transactions.txt"
#define TEMP_NAME "transactions.tmp"

// Asks for a number until it is valid, type 'i' is for int and 'd' is for double
int get_number(const char *prompt, void *value, char type)
{
    char line[64];
    char *end;
    int ch;
    long whole = 0;
    double decimal = 0;

    if (type != 'i' && type != 'd')
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
            decimal = strtod(line, &end);

        // Nothing was read, so the line has no number
        if (end == line)
        {
            printf("Invalid input. Try again.\n");
            continue;
        }

        // Only spaces are allowed after the number
        while (isspace((unsigned char)*end))
            end++;

        if (*end != '\0' || (type == 'd' && isnan(decimal)))
        {
            printf("Invalid input. Try again.\n");
            continue;
        }

        // Reject negative numbers
        if ((type == 'i' && whole < 0) || (type == 'd' && decimal < 0))
        {
            printf("Negative numbers are not allowed. Try again.\n");
            continue;
        }

        // Reject numbers that do not fit
        if (errno == ERANGE || (type == 'i' && whole > INT_MAX) || (type == 'd' && isinf(decimal)))
        {
            printf("Number is out of range. Try again.\n");
            continue;
        }

        if (type == 'i')
            *(int *)value = (int)whole;
        else
            *(double *)value = decimal;

        return 1;
    }
}

// Asks for text with a letter in it, returns 1 if it worked and 0 if input ended
int get_string(const char *prompt, char *value, size_t size)
{
    int ch;
    int has_letter;
    size_t len;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(value, (int)size, stdin) == NULL)
            return 0;

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

        return 1;
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

// Asks for text until it looks like a real name, returns 0 if input ended
int get_name(const char *prompt, char *value, size_t size, int allow_digits)
{
    while (1)
    {
        if (!get_string(prompt, value, size))
            return 0;

        if (valid_text(value, allow_digits))
            return 1;

        if (allow_digits)
            printf("Use at least two letters, and only letters, digits, spaces, - ' and . characters.\n");
        else
            printf("Use at least two letters, and only letters, spaces, - ' and . characters.\n");
    }
}

// Returns the position of the transaction with this ID, or -1 if not found
int find_by_id(const Transaction *list, int count, int id)
{
    for (int i = 0; i < count; i++)
    {
        if (list[i].id == id)
            return i;
    }

    return -1;
}

// Asks for a date until it is a real one, returns 1 if it worked and EOF if input ended
int read_date(const char *prompt, Date *date)
{
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    Date d;

    while (1)
    {
        printf("%s\n", prompt);

        if (get_number("Day: ", &d.day, 'i') != 1 ||
            get_number("Month: ", &d.month, 'i') != 1 ||
            get_number("Year: ", &d.year, 'i') != 1)
            return EOF;

        // February has 29 days in a leap year
        if ((d.year % 4 == 0 && d.year % 100 != 0) || d.year % 400 == 0)
            days[1] = 29;
        else
            days[1] = 28;

        if (d.year >= 2000 && d.year <= 2100 && d.month >= 1 && d.month <= 12 &&
            d.day >= 1 && d.day <= days[d.month - 1])
        {
            *date = d;
            return 1;
        }

        printf("Input a real date with a year between 2000 and 2100\n");
    }
}

// Asks for a quantity until it is valid, returns 1 if it worked and EOF if input ended
int read_quantity(const char *prompt, int *quantity)
{
    int number;

    while (1)
    {
        if (get_number(prompt, &number, 'i') != 1)
            return EOF;

        if (number >= 1 && number <= 1000000)
        {
            *quantity = number;
            return 1;
        }

        printf("Input a quantity between 1 and 1000000\n");
    }
}

// Asks for a price until it is valid and stores it in cents, returns EOF if input ended
int read_price(const char *prompt, long long *cents)
{
    double price;
    long long rounded;

    while (1)
    {
        if (get_number(prompt, &price, 'd') != 1)
            return EOF;

        // Change the price to cents and round to the nearest cent
        if (price <= 1000000)
        {
            rounded = llround(price * 100);
            if (rounded >= 1)
            {
                *cents = rounded;
                return 1;
            }
        }

        printf("Input a price between 0.01 and 1000000\n");
    }
}

// Adds a new transaction and makes the array bigger if it is full, returns 1 if it was added
int add_transaction(Transaction **list, int *count, int *capacity)
{
    Transaction t;

    while (1)
    {
        if (get_number("Transaction ID: ", &t.id, 'i') != 1)
            return 0;

        if (t.id > 0)
            break;

        printf("Transaction ID must be greater than 0\n");
    }

    if (find_by_id(*list, *count, t.id) != -1)
    {
        printf("This transaction ID already exists.\n");
        return 0;
    }

    if (!get_name("Product name: ", t.product_name, sizeof(t.product_name), 1))
        return 0;

    if (read_quantity("Quantity sold: ", &t.quantity) != 1)
        return 0;

    if (read_price("Unit price: ", &t.price) != 1)
        return 0;

    if (read_date("Transaction date", &t.date) != 1)
        return 0;

    t.total_amount = t.quantity * t.price;

    // Make the array bigger only now that all the input is valid
    if (*count >= *capacity)
    {
        int new_capacity = (*capacity == 0) ? 4 : *capacity * 2;
        Transaction *temp = realloc(*list, new_capacity * sizeof(Transaction));

        // The old array is still safe in *list if realloc fails
        if (temp == NULL)
        {
            printf("Memory allocation failed.\n");
            return 0;
        }

        *list = temp;
        *capacity = new_capacity;
    }

    (*list)[*count] = t;
    (*count)++;
    printf("Transaction %d added.\n", t.id);

    return 1;
}

// Prints the heading of the table
void print_header(void)
{
    printf("\n%-10s  %-25s  %8s  %12s  %18s  %-10s\n",
           "ID", "Product", "Quantity", "Unit price", "Total", "Date");
}

// Prints one transaction as a row of the table
void print_transaction(const Transaction *t)
{
    printf("%-10d  %-25s  %8d  ", t->id, t->product_name, t->quantity);

    // The last two digits are the cents, 02 keeps the zero in 2.05
    printf("%9lld.%02lld  ", t->price / 100, t->price % 100);
    printf("%15lld.%02lld  ", t->total_amount / 100, t->total_amount % 100);
    printf("%02d/%02d/%d\n", t->date.day, t->date.month, t->date.year);
}

// Shows a table of all transactions
void display_transactions(const Transaction *list, int count)
{
    if (count == 0)
    {
        printf("There are currently no transactions.\n");
        return;
    }

    print_header();

    for (int i = 0; i < count; i++)
        print_transaction(&list[i]);

    printf("\nTotal transactions: %d\n", count);
}

// Updates one field of a transaction but never the ID, returns 1 if it changed
int update_transaction(Transaction *list, int count)
{
    int id, index, choice;
    Transaction t;

    if (count == 0)
    {
        printf("There are currently no transactions.\n");
        return 0;
    }

    if (get_number("Transaction ID: ", &id, 'i') != 1)
        return 0;

    index = find_by_id(list, count, id);
    if (index == -1)
    {
        printf("This transaction does not exist.\n");
        return 0;
    }

    // Work on a copy so the record only changes if everything is valid
    t = list[index];

    if (get_number("\nWhat would you like to update?\n"
                   "1. Product name\n"
                   "2. Quantity sold\n"
                   "3. Unit price\n"
                   "4. Transaction date\n"
                   "0. Cancel\n"
                   "Enter your choice: ", &choice, 'i') != 1)
        return 0;

    switch (choice)
    {
    case 0:
        printf("Update cancelled.\n");
        return 0;
    case 1:
        if (!get_name("New product name: ", t.product_name, sizeof(t.product_name), 1))
            return 0;
        break;
    case 2:
        if (read_quantity("New quantity sold: ", &t.quantity) != 1)
            return 0;
        break;
    case 3:
        if (read_price("New unit price: ", &t.price) != 1)
            return 0;
        break;
    case 4:
        if (read_date("New transaction date", &t.date) != 1)
            return 0;
        break;
    default:
        printf("Invalid choice.\n");
        return 0;
    }

    // Set the total every time so it always matches the quantity and price
    t.total_amount = t.quantity * t.price;
    list[index] = t;
    printf("Transaction %d updated.\n", t.id);

    return 1;
}

// Deletes a transaction after the user confirms, returns 1 if it was deleted
int delete_transaction(Transaction **list, int *count, int *capacity)
{
    int id, index;
    char answer[50];

    if (*count == 0)
    {
        printf("There are currently no transactions.\n");
        return 0;
    }

    if (get_number("Transaction ID: ", &id, 'i') != 1)
        return 0;

    index = find_by_id(*list, *count, id);
    if (index == -1)
    {
        printf("This transaction does not exist.\n");
        return 0;
    }

    if (!get_string("Type 'yes' to delete or any other word to go back: ", answer, sizeof(answer)))
        return 0;

    if (strcasecmp(answer, "yes") != 0)
    {
        printf("Delete cancelled.\n");
        return 0;
    }

    // Move every record after the deleted one back by one place
    for (int i = index; i < *count - 1; i++)
        (*list)[i] = (*list)[i + 1];

    (*count)--;
    printf("Transaction %d deleted.\n", id);

    // Halve the array when only a quarter is used, but never go below 4
    if (*capacity > 4 && *count <= *capacity / 4)
    {
        int new_capacity = *capacity / 2;
        Transaction *temp = realloc(*list, new_capacity * sizeof(Transaction));

        // If this fails the bigger array still works, so the delete is still fine
        if (temp != NULL)
        {
            *list = temp;
            *capacity = new_capacity;
        }
    }

    return 1;
}

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include "helpers.h"

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

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include "helpers.h"
#include "transactions.h"
#include "filtering.h"

// Callback that checks if the total is above an amount in cents
int above_amount(const Transaction *t, const void *value)
{
    return t->total_amount > *(const long long *)value;
}

// Callback that checks if the total is exactly an amount in cents
int equals_amount(const Transaction *t, const void *value)
{
    return t->total_amount == *(const long long *)value;
}

// Callback that checks if the product has this name
int is_product(const Transaction *t, const void *value)
{
    return strcasecmp(t->product_name, (const char *)value) == 0;
}

// Callback that checks if the quantity is above a number
int above_quantity(const Transaction *t, const void *value)
{
    return t->quantity > *(const int *)value;
}

// Prints every transaction that the callback says yes to
void filter_transactions(const Transaction *list, int count,
                         int (*matches)(const Transaction *, const void *), const void *value)
{
    for (int i = 0; i < count; i++)
    {
        if (matches(&list[i], value))
            print_transaction(&list[i]);
    }
}

// Counts the transactions that the callback says yes to
int count_transactions(const Transaction *list, int count,
                       int (*matches)(const Transaction *, const void *), const void *value)
{
    int total = 0;

    for (int i = 0; i < count; i++)
    {
        if (matches(&list[i], value))
            total++;
    }

    return total;
}

// Adds up the totals of the transactions that the callback says yes to
long long sum_transactions(const Transaction *list, int count,
                           int (*matches)(const Transaction *, const void *), const void *value)
{
    long long total = 0;

    for (int i = 0; i < count; i++)
    {
        if (matches(&list[i], value))
            total += list[i].total_amount;
    }

    return total;
}

// Asks for a condition, then passes its callback to the count and filter functions
void filter_menu(const Transaction *list, int count)
{
    int choice, quantity, found;
    double amount;
    long long cents;
    char product[50];
    int (*matches)(const Transaction *, const void *);
    const void *value;

    if (count == 0)
    {
        printf("There are currently no transactions.\n");
        return;
    }

    if (get_number("\nShow transactions:\n"
                   "1. With a total above an amount\n"
                   "2. For a product\n"
                   "3. With a quantity above a number\n"
                   "Enter your choice: ", &choice, 'i') != 1)
        return;

    if (choice == 1)
    {
        if (get_number("Amount: ", &amount, 'd') != 1)
            return;

        // No total can be bigger than this
        if (amount > 1000000000000.0)
            amount = 1000000000000.0;

        cents = llround(amount * 100);
        matches = above_amount;
        value = &cents;
    }
    else if (choice == 2)
    {
        if (!get_name("Product name: ", product, sizeof(product), 1))
            return;

        matches = is_product;
        value = product;
    }
    else if (choice == 3)
    {
        if (get_number("Quantity: ", &quantity, 'i') != 1)
            return;

        matches = above_quantity;
        value = &quantity;
    }
    else
    {
        printf("Invalid choice.\n");
        return;
    }

    found = count_transactions(list, count, matches, value);
    if (found == 0)
    {
        printf("No matching transactions found.\n");
        return;
    }

    print_header();
    filter_transactions(list, count, matches, value);
    printf("\nMatching transactions: %d\n", found);
}

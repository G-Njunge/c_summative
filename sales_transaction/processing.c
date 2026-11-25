#include <stdio.h>
#include "helpers.h"
#include "processing.h"

// Adds up the totals of all transactions
void total_revenue(const Transaction *list, int count)
{
    long long total = 0;

    for (int i = 0; i < count; i++)
        total += list[i].total_amount;

    printf("Total revenue: %lld.%02lld\n", total / 100, total % 100);
}

// Works out the average value of a transaction
void average_value(const Transaction *list, int count)
{
    long long total = 0;
    long long average;

    // Avoid dividing by zero
    if (count == 0)
        return;

    for (int i = 0; i < count; i++)
        total += list[i].total_amount;

    // Adding half of count first rounds to the nearest cent
    average = (total + count / 2) / count;
    printf("Average transaction value: %lld.%02lld\n", average / 100, average % 100);
}

// Finds the transaction with the biggest total
void highest_transaction(const Transaction *list, int count)
{
    int best = 0;

    if (count == 0)
        return;

    for (int i = 1; i < count; i++)
    {
        if (list[i].total_amount > list[best].total_amount)
            best = i;
    }

    printf("Highest transaction: ID %d, %s, %lld.%02lld\n", list[best].id, list[best].product_name,
           list[best].total_amount / 100, list[best].total_amount % 100);
}

// Finds the transaction with the smallest total
void lowest_transaction(const Transaction *list, int count)
{
    int best = 0;

    if (count == 0)
        return;

    for (int i = 1; i < count; i++)
    {
        if (list[i].total_amount < list[best].total_amount)
            best = i;
    }

    printf("Lowest transaction: ID %d, %s, %lld.%02lld\n", list[best].id, list[best].product_name,
           list[best].total_amount / 100, list[best].total_amount % 100);
}

// Adds up the quantities of all transactions
void total_units(const Transaction *list, int count)
{
    long long total = 0;

    for (int i = 0; i < count; i++)
        total += list[i].quantity;

    printf("Total units sold: %lld\n", total);
}

// Runs the operation the user picks using an array of function pointers
void process_sales(const Transaction *list, int count)
{
    void (*ops[5])(const Transaction *, int) = {total_revenue, average_value, highest_transaction,
                                                lowest_transaction, total_units};
    int choice;

    if (count == 0)
    {
        printf("There are currently no transactions.\n");
        return;
    }

    if (get_number("\nWhat would you like to calculate?\n"
                   "1. Total revenue\n"
                   "2. Average transaction value\n"
                   "3. Highest transaction\n"
                   "4. Lowest transaction\n"
                   "5. Total units sold\n"
                   "Enter your choice: ", &choice, 'i') != 1)
        return;

    // Check the choice first so we never go outside the array
    if (choice < 1 || choice > 5)
    {
        printf("Invalid choice.\n");
        return;
    }

    ops[choice - 1](list, count);
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include "helpers.h"
#include "transactions.h"
#include "filtering.h"
#include "search_sort.h"

// Searches for transactions by ID, product name or total amount
void search_transactions(const Transaction *list, int count)
{
    int choice, id, index, found;
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

    if (get_number("\nSearch by:\n"
                   "1. Transaction ID\n"
                   "2. Product name\n"
                   "3. Transaction amount\n"
                   "Enter your choice: ", &choice, 'i') != 1)
        return;

    if (choice == 1)
    {
        if (get_number("Transaction ID: ", &id, 'i') != 1)
            return;

        // IDs are unique so the search stops at the first match
        index = find_by_id(list, count, id);
        if (index == -1)
        {
            printf("No matching transactions found.\n");
            return;
        }

        print_header();
        print_transaction(&list[index]);
        return;
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
        if (get_number("Transaction amount: ", &amount, 'd') != 1)
            return;

        // No total can be bigger than this
        if (amount > 1000000000000.0)
        {
            printf("No matching transactions found.\n");
            return;
        }

        cents = llround(amount * 100);
        matches = equals_amount;
        value = &cents;
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

// Callback that compares two IDs
int compare_id(const Transaction *a, const Transaction *b)
{
    return (a->id > b->id) - (a->id < b->id);
}

// Callback that compares two product names
int compare_product(const Transaction *a, const Transaction *b)
{
    return strcasecmp(a->product_name, b->product_name);
}

// Callback that compares two totals, subtracting them could be too big for an int
int compare_total(const Transaction *a, const Transaction *b)
{
    return (a->total_amount > b->total_amount) - (a->total_amount < b->total_amount);
}

// Sorts the records using insertion sort and the compare callback
void sort_transactions(Transaction *list, int count,
                       int (*compare)(const Transaction *, const Transaction *), int descending)
{
    for (int i = 1; i < count; i++)
    {
        Transaction current = list[i];
        int pos;

        for (pos = i; pos > 0; pos--)
        {
            int result = compare(&list[pos - 1], &current);

            // Flip the result for descending order
            if (descending)
                result = -result;

            // Stop if the record on the left should stay before this one
            if (result <= 0)
                break;

            // Move the record on the left one place right
            list[pos] = list[pos - 1];
        }

        list[pos] = current;
    }
}

// Asks how to sort, then sorts and shows a copy so the saved order does not change
void sort_menu(const Transaction *list, int count)
{
    int choice, order;
    Transaction *copy;
    int (*compare)(const Transaction *, const Transaction *);

    if (count == 0)
    {
        printf("There are currently no transactions.\n");
        return;
    }

    if (get_number("\nSort by:\n"
                   "1. Transaction ID\n"
                   "2. Product name\n"
                   "3. Total amount\n"
                   "Enter your choice: ", &choice, 'i') != 1)
        return;

    if (choice == 1)
        compare = compare_id;
    else if (choice == 2)
        compare = compare_product;
    else if (choice == 3)
        compare = compare_total;
    else
    {
        printf("Invalid choice.\n");
        return;
    }

    if (get_number("\nOrder:\n"
                   "1. Ascending\n"
                   "2. Descending\n"
                   "Enter your choice: ", &order, 'i') != 1)
        return;

    if (order != 1 && order != 2)
    {
        printf("Invalid choice.\n");
        return;
    }

    copy = malloc(count * sizeof(Transaction));
    if (copy == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    for (int i = 0; i < count; i++)
        copy[i] = list[i];

    sort_transactions(copy, count, compare, order == 2);
    display_transactions(copy, count);
    free(copy);
}

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "transactions.h"
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

// Shows a report of all sales, then the revenue of one product
void sales_report(const Transaction *list, int count)
{
    char product[50];
    int found;
    long long revenue;

    // Check first because the operations need at least one transaction
    if (count == 0)
    {
        printf("There are currently no transactions.\n");
        return;
    }

    printf("\n===== SALES REPORT =====\n");
    printf("Total transactions: %d\n", count);
    total_revenue(list, count);
    average_value(list, count);
    highest_transaction(list, count);
    lowest_transaction(list, count);
    total_units(list, count);

    if (!get_name("\nProduct to show revenue for: ", product, sizeof(product), 1))
        return;

    found = count_transactions(list, count, is_product, product);
    if (found == 0)
    {
        printf("There are no transactions for %s.\n", product);
        return;
    }

    revenue = sum_transactions(list, count, is_product, product);
    printf("Revenue for %s: %lld.%02lld from %d transaction(s)\n", product, revenue / 100, revenue % 100, found);
}

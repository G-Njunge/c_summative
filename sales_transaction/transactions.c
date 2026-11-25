#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "helpers.h"
#include "transactions.h"

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

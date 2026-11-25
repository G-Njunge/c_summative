#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"
#include "transactions.h"
#include "processing.h"
#include "filtering.h"
#include "search_sort.h"
#include "report.h"

// Shows the menu until the user exits
int main(void)
{
    Transaction *list = NULL;
    int count = 0;
    int capacity = 0;
    int choice;

    while (1)
    {
        printf("\n===== SALES TRANSACTION MANAGEMENT SYSTEM =====\n");
        printf("1. Add New Transaction\n");
        printf("2. Display All Transactions\n");
        printf("3. Update Transaction\n");
        printf("4. Delete Transaction\n");
        printf("5. Process Sales Data\n");
        printf("6. Filter Transactions\n");
        printf("7. Search Transactions\n");
        printf("8. Sort Transactions\n");
        printf("9. Generate Sales Report\n");
        printf("0. Exit\n");

        // Stop if input ends
        if (get_number("Enter your choice: ", &choice, 'i') != 1 || choice == 0)
            break;

        if (choice == 1)
            add_transaction(&list, &count, &capacity);
        else if (choice == 2)
            display_transactions(list, count);
        else if (choice == 3)
            update_transaction(list, count);
        else if (choice == 4)
            delete_transaction(&list, &count, &capacity);
        else if (choice == 5)
            process_sales(list, count);
        else if (choice == 6)
            filter_menu(list, count);
        else if (choice == 7)
            search_transactions(list, count);
        else if (choice == 8)
            sort_menu(list, count);
        else if (choice == 9)
            sales_report(list, count);
        else
            printf("Invalid choice.\n");
    }

    free(list);
    return 0;
}

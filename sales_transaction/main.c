#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "transactions.h"
#include "processing.h"

// Loads the transactions, then shows the menu until the user exits
int main(void)
{
    Transaction *list = NULL;
    int count = 0;
    int capacity = 0;
    int choice;
    int running = 1;
    int saved = 1;
    char answer[50];

    // Stop if loading fails, or the next save would replace the bad file
    if (!load_transactions(&list, &count, &capacity))
    {
        printf("Fix the file and run the program again. No records were changed.\n");
        return 1;
    }

    while (running)
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
        printf("10. Save Transactions\n");
        printf("11. Load Transactions\n");
        printf("0. Exit\n");

        // Stop if input ends
        if (get_number("Enter your choice: ", &choice, 'i') != 1)
            break;

        switch (choice)
        {
        case 1:
            // Only save when something changed
            if (add_transaction(&list, &count, &capacity))
                saved = save_transactions(list, count);
            break;
        case 2:
            display_transactions(list, count);
            break;
        case 3:
            if (update_transaction(list, count))
                saved = save_transactions(list, count);
            break;
        case 4:
            if (delete_transaction(&list, &count, &capacity))
                saved = save_transactions(list, count);
            break;
        case 5:
            process_sales(list, count);
            break;
        case 6:
            filter_menu(list, count);
            break;
        case 7:
            search_transactions(list, count);
            break;
        case 8:
            sort_menu(list, count);
            break;
        case 9:
            sales_report(list, count);
            break;
        case 10:
            saved = save_transactions(list, count);
            break;
        case 11:
            // Loading would lose changes that were not saved
            if (!saved)
            {
                if (!get_string("You have unsaved changes. Type 'yes' to lose them and load: ", answer, sizeof(answer)) ||
                    strcasecmp(answer, "yes") != 0)
                    break;
            }

            if (load_transactions(&list, &count, &capacity))
                saved = 1;
            break;
        case 0:
            // Try again if the last save failed
            if (!saved)
                saved = save_transactions(list, count);

            if (!saved)
            {
                if (get_string("Type 'yes' to exit without saving or any other word to go back: ", answer, sizeof(answer)) &&
                    strcasecmp(answer, "yes") != 0)
                    break;
            }

            printf("Exiting...\n");
            running = 0;
            break;
        default:
            printf("Invalid choice.\n");
        }
    }

    free(list);
    return 0;
}

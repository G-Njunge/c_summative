#include <stdio.h>
#include "helpers.h"
#include "processing.h"
#include "filtering.h"
#include "report.h"

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

#ifndef PROCESSING_H
#define PROCESSING_H

#include "transactions.h"

// All five operations have the same shape so they fit in one array
void total_revenue(const Transaction *list, int count);
void average_value(const Transaction *list, int count);
void highest_transaction(const Transaction *list, int count);
void lowest_transaction(const Transaction *list, int count);
void total_units(const Transaction *list, int count);

void process_sales(const Transaction *list, int count);

// Callbacks, each one answers yes (1) or no (0) for one transaction
int above_amount(const Transaction *t, const void *value);
int equals_amount(const Transaction *t, const void *value);
int is_product(const Transaction *t, const void *value);
int above_quantity(const Transaction *t, const void *value);

// These take a callback and call it for every transaction
void filter_transactions(const Transaction *list, int count,
                         int (*matches)(const Transaction *, const void *), const void *value);
int count_transactions(const Transaction *list, int count,
                       int (*matches)(const Transaction *, const void *), const void *value);
long long sum_transactions(const Transaction *list, int count,
                           int (*matches)(const Transaction *, const void *), const void *value);

void filter_menu(const Transaction *list, int count);

void search_transactions(const Transaction *list, int count);

// Callbacks, negative means a comes first, positive means b comes first, 0 means the same
int compare_id(const Transaction *a, const Transaction *b);
int compare_product(const Transaction *a, const Transaction *b);
int compare_total(const Transaction *a, const Transaction *b);

// Takes a compare callback and uses it to put the records in order
void sort_transactions(Transaction *list, int count,
                       int (*compare)(const Transaction *, const Transaction *), int descending);
void sort_menu(const Transaction *list, int count);

void sales_report(const Transaction *list, int count);

#endif

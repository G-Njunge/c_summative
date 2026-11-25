#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

#include "helpers.h"

void search_transactions(const Transaction *list, int count);

// Callbacks, negative means a comes first, positive means b comes first, 0 means the same
int compare_id(const Transaction *a, const Transaction *b);
int compare_product(const Transaction *a, const Transaction *b);
int compare_total(const Transaction *a, const Transaction *b);

// Takes a compare callback and uses it to put the records in order
void sort_transactions(Transaction *list, int count,
                       int (*compare)(const Transaction *, const Transaction *), int descending);
void sort_menu(const Transaction *list, int count);

#endif

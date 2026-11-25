#ifndef FILTERING_H
#define FILTERING_H

#include "helpers.h"

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

#endif

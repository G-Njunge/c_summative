#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "helpers.h"

// These can move the array, so they need the address of the pointer
void add_transaction(Transaction **list, int *count, int *capacity);
void delete_transaction(Transaction **list, int *count, int *capacity);

// These do not move the array, so a pointer to it is enough
void print_header(void);
void print_transaction(const Transaction *t);
void display_transactions(const Transaction *list, int count);
void update_transaction(Transaction *list, int count);

#endif

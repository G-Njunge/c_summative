#ifndef FILES_H
#define FILES_H

#include "helpers.h"

int save_transactions(const Transaction *list, int count);
int load_transactions(Transaction **list, int *count, int *capacity);
int read_transaction_line(const char *line, Transaction *t);

#endif

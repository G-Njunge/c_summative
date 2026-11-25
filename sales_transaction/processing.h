#ifndef PROCESSING_H
#define PROCESSING_H

#include "helpers.h"

// All five operations have the same shape so they fit in one array
void total_revenue(const Transaction *list, int count);
void average_value(const Transaction *list, int count);
void highest_transaction(const Transaction *list, int count);
void lowest_transaction(const Transaction *list, int count);
void total_units(const Transaction *list, int count);

void process_sales(const Transaction *list, int count);

#endif

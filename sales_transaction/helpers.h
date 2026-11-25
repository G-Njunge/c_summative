#ifndef HELPERS_H
#define HELPERS_H

#include <stddef.h>

typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct{
    int id;
    char product_name[50];
    int quantity;
    // Money is stored in cents
    long long price;
    // quantity * price, add and update must set this
    long long total_amount;
    Date date;
} Transaction;

// Function prototypes
int get_number(const char *prompt, void *value, char type);
int get_string(const char *prompt, char *value, size_t size);
int valid_text(const char *value, int allow_digits);
int get_name(const char *prompt, char *value, size_t size, int allow_digits);
int find_by_id(const Transaction *list, int count, int id);
int read_date(const char *prompt, Date *date);
int read_quantity(const char *prompt, int *quantity);
int read_price(const char *prompt, long long *cents);

#endif

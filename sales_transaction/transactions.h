#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

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

// Input and validation
int get_number(const char *prompt, void *value, char type);
int get_string(const char *prompt, char *value, size_t size);
int valid_text(const char *value, int allow_digits);
int get_name(const char *prompt, char *value, size_t size, int allow_digits);
int find_by_id(const Transaction *list, int count, int id);
int read_date(const char *prompt, Date *date);
int read_quantity(const char *prompt, int *quantity);
int read_price(const char *prompt, long long *cents);

// These can move the array, so they need the address of the pointer, they return 1 if something changed
int add_transaction(Transaction **list, int *count, int *capacity);
int delete_transaction(Transaction **list, int *count, int *capacity);

// These do not move the array, so a pointer to it is enough
void print_header(void);
void print_transaction(const Transaction *t);
void display_transactions(const Transaction *list, int count);
int update_transaction(Transaction *list, int count);

// Saving and loading
int save_transactions(const Transaction *list, int count);
int load_transactions(Transaction **list, int *count, int *capacity);
int read_transaction_line(const char *line, Transaction *t);

#endif

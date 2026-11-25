#ifndef REPORT_H
#define REPORT_H

#include "analyzer.h"

// error is NULL if the file was read, or says what went wrong
int generate_report(const ThreadArgs *args, const char *error);

#endif

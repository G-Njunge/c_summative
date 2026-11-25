#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>

//catalog entry including all the courses
typedef struct {
    char code[15];
    char name[50];
    float maxMarks;
} CourseInfo;

//course struct
typedef struct {
    char course_code[15];
    char course_name[50];
    float  marks;
    float maxMarks;
}course;

//student struct
typedef struct{
    int id;
    char first_name[50];
    char last_name[50];
    char programme[50];
    int year;
    int number_of_courses;
    course *courses;
} students;

extern const CourseInfo CATALOG[];
extern int catalog_size;

extern students *s;
extern int count;
extern int capacity;

//function prototypes
int save_records(void);
int load_records(void);
int read_student_line(FILE *fp, students *st, int *lineNum);
int read_course_line(FILE *fp, students *st, int index, int *lineNum);
int load_failed(FILE *fp, int lineNum);
void free_records(void);
const CourseInfo *find_catalog_course(const char *code);
int get_number(const char *prompt, void *value, char type);
void get_string(const char *prompt, char *value, size_t size);
int valid_text(const char *value, int allow_digits);
void get_name(const char *prompt, char *value, size_t size, int allow_digits);
int id_search(int id);
int name_search(const char *first_name,const char *last_name);
float calculate_average(const students *st);

#endif

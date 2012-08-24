#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#define PROGRAM_NAME "cloud_evaluation_center"


/* use select ---> sleep */
int select_sleep(const long sec, const long usec);

/* juage string is number */
int is_number(const char *str);

/* juage pathname is regular file */
int is_regularfile(const char *pathname);

/* create dir */
int create_dir(const char *dir, mode_t mode);

/* get string time now */
int get_time_now(char *timebuf, size_t len, const char *format);

/* get time utc string time ---> time_t */
time_t get_time_utc(const char *str_time, const char *format);

void get_time_str(const char *seconds,  char *ret_str);	

int get_table_name(char * table_name, int offset);


#endif


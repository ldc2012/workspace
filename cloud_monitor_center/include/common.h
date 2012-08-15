#ifndef COMMON_H_
#define COMMON_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <dirent.h>

#define PROGRAM_NAME "cloud_monitor_center"


struct listen_param_stu {
	char			*addr;
	unsigned short	port;
	unsigned short	timeout;
};

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

/* get filename from path */
char *get_filename(char *path);

/* get file size */
unsigned long get_file_size(const char *filename);

void get_time_str(const char *seconds,  char *ret_str);

#endif // COMMON_H_

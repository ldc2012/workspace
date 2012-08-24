#include "common.h"

int select_sleep(const long sec, const long usec)
{
	struct timeval timeout = {sec, usec};
	int ret = select(0, NULL, NULL, NULL, &timeout);

	return ret;
}

int is_number(const char *str)
{
	if ('\0' == *str) {
		return -1;
	}

	for (; '\0' != *str; ++str) {
		if (*str < '0' || *str > '9') {
			return -1;
		}
	}

	return 0;
}

int is_regularfile(const char *pathname)
{
	struct stat stat_tmp;
	
	if (0 == lstat(pathname, &stat_tmp)) {
		if (S_ISREG(stat_tmp.st_mode)) {
			return 0;
		}
	}

	return -1;
}

int create_dir(const char *dir, mode_t mode)
{
	char dirname[256];
	
	if (NULL == dir || (0 == strlen(dir))) {
		return -1;
	}

	memset(dirname, 0x00, sizeof(dirname));
	if ('/' == dirname[strlen(dir) - 1]) 
		snprintf(dirname, sizeof(dirname), "%s", dir);
	else
		snprintf(dirname, sizeof(dirname), "%s/", dir);

	int len = strlen(dirname);
	for (int i = 1; i < len; ++i) {
		if ('/' == dirname[i]) {
			dirname[i] = '\0';  // block 
			if (0 != access(dirname, F_OK))	{	//F_OK 0
				if (0 != mkdir(dirname, mode)) {   
					fprintf(stderr, "mkdir fail: %s\n", strerror(errno));
					return -1;
				}
			}
			dirname[i] = '/';  // revert
		}
	}

	return 0;
}

int get_time_now(char *timebuf, size_t len, const char *format)
{
	time_t t;
	struct tm tm;

	if (!timebuf || !format) {
		return -1;
	}

	t = time(NULL);
	localtime_r(&t, &tm); // threadd-safe
	strftime(timebuf, len, format, &tm);

	return 0;
}

time_t get_time_utc(const char *str_time, const char *format)
{
	struct tm ptm;
	time_t t;
	
	strptime(str_time, format, &ptm);
	t = mktime(&ptm);
	
	return t;
}

char *get_filename(char *path)
{
	DIR *dp;
	struct dirent *dirp;

	if ((dp=opendir(path)) == NULL)
		return NULL;
	    
	if(!access(path, 0)) { // path is exist
		while (((dirp=readdir(dp))!=NULL)) {
			if (dirp->d_name[0] != '.')
				return dirp->d_name;
		}
		closedir(dp);
	}

	return NULL;
}

unsigned long get_file_size(const char *filename) {
	struct stat buf;

	if(stat(filename, &buf)<0) {
		return 0;
	}
	
	return (unsigned long)buf.st_size;
}

void get_time_str(const char *seconds, char *ret_str)
{
	time_t lt;

	if (NULL != seconds) {
		lt = atol(seconds);
		strftime(ret_str, 64, "%Y-%m-%d %H:%M:%S", localtime(&lt));
	}

	return;
}

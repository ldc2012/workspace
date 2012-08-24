#include "log.h"

char g_log_path[256];
static pthread_mutex_t mutex_logrun = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_logerr = PTHREAD_MUTEX_INITIALIZER;

int write_logrun(const char *format, ...)
{
	char pathname_logrun[512];
	char hhmmss[256];
	char yyyymmdd[256];
	va_list ap;

	bzero(hhmmss, sizeof(hhmmss));
	bzero(yyyymmdd, sizeof(yyyymmdd));
	get_time_now(yyyymmdd, sizeof(yyyymmdd)-1, "%Y%m%d");
	get_time_now(hhmmss, sizeof(hhmmss)-1, "%H:%M:%S");
	if ('/' == g_log_path[strlen(g_log_path)-1])
		snprintf(pathname_logrun, sizeof(pathname_logrun), "%s%s_%s.log", 
				g_log_path, PROGRAM_NAME, yyyymmdd);
	else
		snprintf(pathname_logrun, sizeof(pathname_logrun), "%s/%s_%s.log", 
				g_log_path, PROGRAM_NAME, yyyymmdd);

	pthread_mutex_lock(&mutex_logrun);
	FILE *stream = fopen(pathname_logrun, "a");
	if (!stream) {
		va_start(ap, format);
		vfprintf(stderr, format, ap);	// stderr not flush line
		va_end(ap);
	} else {
		fprintf(stream, "%s:", hhmmss);
		va_start(ap, format);
		vfprintf(stream, format, ap);
		va_end(ap);
		fflush(stream);				// fflush(NULL) flush all
		fclose(stream);
	}
	pthread_mutex_unlock(&mutex_logrun);

	return 0;
}

int write_logerr(const char *format, ...)
{	
	char pathname_logrun[512];
	char pathname_logerr[512];
	char hhmmss[256];
	char yyyymmdd[256];
	va_list ap;

	bzero(hhmmss, sizeof(hhmmss));
	bzero(yyyymmdd, sizeof(yyyymmdd));
	get_time_now(yyyymmdd, sizeof(yyyymmdd)-1, "%Y%m%d");
	get_time_now(hhmmss, sizeof(hhmmss)-1, "%H:%M:%S");
	if ('/' == g_log_path[strlen(g_log_path)-1]) {
		snprintf(pathname_logerr, sizeof(pathname_logrun), "%s%s_%s.log", 
				g_log_path, PROGRAM_NAME, yyyymmdd);
		snprintf(pathname_logerr, sizeof(pathname_logerr), "%s%s_%s.logerr", 
				g_log_path, PROGRAM_NAME, yyyymmdd);
	} else {
		snprintf(pathname_logrun, sizeof(pathname_logrun), "%s/%s_%s.log", 
				g_log_path, PROGRAM_NAME, yyyymmdd);
		snprintf(pathname_logerr, sizeof(pathname_logerr), "%s/%s_%s.logerr", 
				g_log_path, PROGRAM_NAME, yyyymmdd);
	}

	pthread_mutex_lock(&mutex_logerr);
	FILE *stream = fopen(pathname_logerr, "a");  // file is not exist, create
	if (!stream) {
		va_start(ap, format);
		vfprintf(stderr, format, ap);	// stderr not flush line
		va_end(ap);
	} else {
		fprintf(stream, "%s:", hhmmss);
		va_start(ap, format);
		vfprintf(stream, format, ap);
		va_end(ap);
		fflush(stream);				// fflush(NULL) flush all
		fclose(stream);
	}
	pthread_mutex_unlock(&mutex_logerr);
    /*	
	pthread_mutex_lock(&mutex_logrun);
	stream = fopen(pathname_logrun, "a");  // file is not exist, create
	if (!stream) {
		va_start(ap, format);
		vfprintf(stderr, format, ap);	// stderr not flush line
		va_end(ap);
	} else {
		fprintf(stream, "%s:", hhmmss);
		va_start(ap, format);
		vfprintf(stream, format, ap);
		va_end(ap);
		fflush(stream);				// fflush(NULL) flush all
		fclose(stream);
	}
	pthread_mutex_unlock(&mutex_logrun);
    */
	return 0;
}
int write_logdata(const char *type, const char *format,...)
{
	char pathname_logdata[512];
	char yyyymmdd[256];
	va_list ap;
	float argvalue = 0;

	bzero(yyyymmdd, sizeof(yyyymmdd));
	get_time_now(yyyymmdd, sizeof(yyyymmdd)-1, "%Y%m%d");
	if ('/' == g_log_path[strlen(g_log_path)-1])
		snprintf(pathname_logdata, sizeof(pathname_logdata), "%s%s_%s_%s.log", 
				g_log_path, PROGRAM_NAME, type, yyyymmdd);
	else
		snprintf(pathname_logdata, sizeof(pathname_logdata), "%s/%s_%s_%s.log", 
				g_log_path, PROGRAM_NAME, type, yyyymmdd);
	
	FILE *stream = fopen(pathname_logdata, "a"); // file is not exist, create
	if (!stream) {
		va_start(ap, format);
		vfprintf(stderr, format, ap);   // stderr not flush line
		va_end(ap);
	} else {
		va_start(ap, format);
		vfprintf(stream, format, ap);
		va_end(ap);
		fflush(stream);         // fflush(NULL) flush all
		fclose(stream);
	}
	
	return 0;
}


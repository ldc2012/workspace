#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <pthread.h>

#include "common.h"

/*
 * Makefile OPEN_LOGDEBUG OPEN_LOG_TO_TERMINAL
 */

#ifdef OPEN_LOG_TO_TERMINAL
#define logrun(format, args...) fprintf(stderr, "%s:%d "format"\n", \
		__FILE__, __LINE__, ##args)

#define logdata(format, args...) fprintf(stderr, ""format"\n", ##args)

#define logbuf(format, args...) fprintf(stderr, ""format"\n", ##args)

#define logwarning(format, args...) fprintf(stderr, "%s:%d:WARNING "format"\n", \
		__FILE__, __LINE__, ##args)

#define logerr(format, args...) fprintf(stderr, "%s:%d:ERROR "format"\n", \
		__FILE__, __LINE__, ##args)

#define logdebug(format, args...) fprintf(stderr, "%s:%d:DEBUG "format"\n", \
		__FILE__, __LINE__, ##args)

#else

#define logrun(format, args...) write_logrun("%s:%d "format"\n", \
		__FILE__, __LINE__, ##args)

#define loghost(format, args...) write_logdata("host", ""format"\n", ##args)

#define logvm(format, args...) write_logdata("vm", ""format"\n", ##args)

#define logbuf(format, args...) write_logdata("buf", ""format"\n", ##args)

#define logwarning(format, args...) write_logrun("%s:%d:WARNING "format"\n", \
		__FILE__, __LINE__, ##args)

#define logerr(format, args...) write_logerr("%s:%d:ERROR "format"\n", \
		__FILE__, __LINE__, ##args)

#ifdef OPEN_LOGDEBUG
#define logdebug(format, args...) write_logrun("%s:%d:DEBUG "format"\n", \
		__FILE__, __LINE__, ##args)
#else
#define logdebug(format, args...) fprintf(stderr, "%s:%d:DEBUG "format"\n", \
		__FILE__, __LINE__, ##args)
#endif

#endif

/* write to run file */
int write_logrun(const char *format, ...);

/* write to data file */
int write_logdata(const char *type, const char *format, ...);

/* writ to err file */
int write_logerr(const char *format, ...);

#endif // LOG_H_

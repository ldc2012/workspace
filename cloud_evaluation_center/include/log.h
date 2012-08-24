#ifndef LOG_H
#define LOG_H

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

#define logwarning(format, args...) fprintf(stderr, "%s:%d:WARNING "format"\n", \
		__FILE__, __LINE__, ##args)

#define logerr(format, args...) fprintf(stderr, "%s:%d:ERROR "format"\n", \
		__FILE__, __LINE__, ##args)

#define logdebug(format, args...) fprintf(stderr, "%s:%d:DEBUG "format"\n", \
		__FILE__, __LINE__, ##args)

#else

#define logrun(format, args...) write_logrun("%s:%d "format"\n", \
		__FILE__, __LINE__, ##args)

#define logwarning(format, args...) write_logrun("%s:%d:WARNING "format"\n", \
		__FILE__, __LINE__, ##args)

#define logerr(format, args...) write_logerr("%s:%d:ERROR "format"\n", \
		__FILE__, __LINE__, ##args)

#define loghost(format, args...) write_logdata("host", ""format"\n", ##args)

#define logvm(format, args...) write_logdata("vm", ""format"\n", ##args)

#define logcloud(args...) write_logdata("cloud", "%d, %s;\n", ##args)
	//best_iaas_id, data_time
				
#define logrunlevel(args...) write_logdata("run_level", "%d, %d, %s, %s;\n", ##args)
	//vm_id, run_level, lack_resource, data_time



#ifdef OPEN_LOGDEBUG
#define logdebug(format, args...) write_logrun("%s:%d:DEBUG "format"\n", \
		__FILE__, __LINE__, ##args)
#else
#define logdebug(format, args...) fprintf(stderr, "%s:%d:DEBUG "format"\n", \
		__FILE__, __LINE__, ##args)
#endif

#endif

/* write to file */
int write_logrun(const char *format, ...);

/* writ to file */
int write_logerr(const char *format, ...);


int write_logdata(const char *type, const char *format, ...);


#endif


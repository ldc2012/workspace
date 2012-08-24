#ifndef CONF_H
#define CONF_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/stat.h>

#include <vector>
using namespace std;

#include "log.h"
#include "common.h"

struct listen_stu {
	char            ip[256];
	unsigned short  port;
};

struct conf_value_stu
{
	char			work_path[256];
	char			log_path[256];
	char            url_path[128];
	
	struct listen_stu	http_listen;
};

int load_profile(const char *profile);
void print_param(const struct conf_value_stu *pconf);

#endif

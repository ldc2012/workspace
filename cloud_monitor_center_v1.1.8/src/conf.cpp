#include "conf.h"

struct conf_value_stu g_confvalue;

static int match_param(const char *key, const char *value);

int load_profile(const char *pathname)
{
	int i, ret;
	FILE *fp;
	char line[512], key[256], value[256];
	
	fp = fopen(pathname, "r");
	if (NULL == fp) {
		fprintf(stderr, "fopen %s file fail: %s\n", pathname, strerror(errno));
		return -1;
	}
	
	while (1) {
		*line = '\0'; 
		if (EOF == (ret = fscanf(fp, "%[^\n]s", line))) {
			// fprintf(stderr, "read conf file end\n");
			break;
		}
		//printf("line: %s\n", line);
		fgetc(fp);						// read enter
		for (i = 0; line[i] != '\0'; ++i) {
			if (!isspace(line[i])) {	//not space 
				if ('#' == line[i]) {	
					// note
				} else {
					sscanf(line, "%s %s", key, value);
					match_param(key, value);
					//printf("key: [%s], value: [%s]\n", key, value);
				}
				break;
			}
		}
	}

	return 0;
}

static int match_param(const char *key, const char *value)
{
	if (0 == strcasecmp(key, "work_path")) {
		snprintf(g_confvalue.work_path, sizeof(g_confvalue.work_path), "%s", value);
	} else if (0 == strcasecmp(key, "log_path")) {
		snprintf(g_confvalue.log_path, sizeof(g_confvalue.log_path), "%s", value);
	} else if (0 == strcasecmp(key, "http_listen")) {
		char ip[64], port[64];
		const char *p;

		memset(ip, 0x00, sizeof(ip));
		memset(port, 0x00, sizeof(port));
		p = strchr(value, ':');
		if (NULL == p) 
			return -1;
		memcpy(ip, value, p-value);
		strncpy(port, p+1, sizeof(port)-1);
		snprintf(g_confvalue.http_listen.ip, sizeof(g_confvalue.http_listen.ip), "%s", ip);	
		if (0 == is_number(port)) {
			g_confvalue.http_listen.port = atoi(port);
		} else {
			g_confvalue.http_listen.port = 8080;
			fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
			return -1;
		}
	} else if ((0 == strcasecmp(key, "url_path"))) {
		snprintf(g_confvalue.url_path, sizeof(g_confvalue.url_path), "%s", value);
	}
	else {
		fprintf(stderr, "this key:%s not cmp, value:%s\n", key, value);
		return -1;
	}

	return 0;
}

void print_param(const struct conf_value_stu *pconf)
{
#ifdef DEBUG
	printf("work_path:%s\nlog_path:%s\n"
			"http_listen_ip:%s\nhttp_listen_port:%u\n",
			pconf->work_path, pconf->log_path,
			pconf->http_listen.ip);
#else
	logrun("work_path:%s\nlog_path:%s\n"
			"http_listen_ip:%s\nhttp_listen_port:%u\n",
			pconf->work_path, pconf->log_path,
			pconf->http_listen.ip);
#endif 
}

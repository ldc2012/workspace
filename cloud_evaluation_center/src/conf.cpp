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
	if (0 == strncasecmp(key, "#", 1)) {
		snprintf(g_confvalue.work_path, sizeof(g_confvalue.work_path), "%s", value);
	} else if (0 == strcasecmp(key, "work_path")) {
		snprintf(g_confvalue.work_path, sizeof(g_confvalue.work_path), "%s", value);
	} else if (0 == strcasecmp(key, "log_path")) {
		snprintf(g_confvalue.log_path, sizeof(g_confvalue.log_path), "%s", value);	
	}else if (0 == strcasecmp(key, "interval")) {
		if (0 == is_number(value)) {
			g_confvalue.interval = atoi(value);
		} else {
			g_confvalue.interval = 60;
			fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
			return -1;
		}
	}else if (0 == strcasecmp(key, "db_retry")) {
		if (0 == is_number(value)) {
			g_confvalue.db_retry = atoi(value);
		} else {
			g_confvalue.db_retry = 10;
			fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
			return -1;
		}
	}else if (0 == strcasecmp(key, "http_listen")) {
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
	} else if (0 == strcasecmp(key, "db_server")) {
		snprintf(g_confvalue.db_server, sizeof(g_confvalue.db_server), "%s", value);		
	}else if (0 == strcasecmp(key, "db_user")) {
		snprintf(g_confvalue.db_user, sizeof(g_confvalue.db_user), "%s", value);		
	}else if (0 == strcasecmp(key, "db_passwd")) {
		snprintf(g_confvalue.db_passwd, sizeof(g_confvalue.db_passwd), "%s", value);		
	}else if (0 == strcasecmp(key, "db_name")) {
		snprintf(g_confvalue.db_name, sizeof(g_confvalue.db_name), "%s", value);		
	}else if (0 == strcasecmp(key, "db_port")) {
		if (0 == is_number(value)) {
			g_confvalue.db_port= atoi(value);
		} else {
			g_confvalue.db_port = 3306;
			fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
			return -1;
		}
	}
	//运行等级参数
	else if (0 == strcasecmp(key, "run_level")) {
			if (0 == is_number(value)) {
				g_confvalue.run_level = atoi(value);
			} else {
				g_confvalue.run_level = 2;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "run_interval")) {
			if (0 == is_number(value)) {
				g_confvalue.run_interval = atoi(value);
			} else {
				g_confvalue.run_interval = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "cpu_avg_1")) {
			if (0 == is_number(value)) {
				g_confvalue.cpu_avg_1 = atof(value);
			}else{				
				g_confvalue.cpu_avg_1 = 60;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "cpu_weight_1")) {
			if (0 == is_number(value)) {
				g_confvalue.cpu_weight_1 = atof(value);
			}else{				
				g_confvalue.cpu_weight_1 = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "cpu_section_1")) {
			if (0 == is_number(value)) {
				g_confvalue.cpu_section_1 = atof(value);
			}else{				
				g_confvalue.cpu_section_1 = 10;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_avg_1")) {
			if (0 == is_number(value)) {
				g_confvalue.mem_avg_1 = atof(value);
			}else{				
				g_confvalue.mem_avg_1 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_weight_1")) {
			if (0 == is_number(value)) {
				g_confvalue.mem_weight_1 = atof(value);
			}else{				
				g_confvalue.mem_weight_1 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_section_1")) {
			if (0 == is_number(value)) {
				g_confvalue.mem_section_1 = atof(value);
			}else{				
				g_confvalue.mem_section_1 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_avg_1")) {
			if (0 == is_number(value)) {
				g_confvalue.io_avg_1 = atof(value);
			}else{				
				g_confvalue.io_avg_1 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_weight_1")) {
			if (0 == is_number(value)) {
				g_confvalue.io_weight_1 = atof(value);
			}else{				
				g_confvalue.io_weight_1 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_section_1")) {
			if (0 == is_number(value)) {
				g_confvalue.io_section_1 = atof(value);
			}else{				
				g_confvalue.io_section_1 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "cpu_avg_2")) {
			if (0 == is_number(value)) {
				g_confvalue.cpu_avg_2 = atof(value);
			}else{				
				g_confvalue.cpu_avg_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "cpu_weight_2")) {
			if (0 == is_number(value)) {
				g_confvalue.cpu_weight_2 = atof(value);
			}else{				
				g_confvalue.cpu_weight_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "cpu_section_2")) {
			if (0 == is_number(value)) {
				g_confvalue.cpu_section_2 = atof(value);
			}else{				
				g_confvalue.cpu_section_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_avg_2")) {
			if (0 == is_number(value)) {
				g_confvalue.mem_avg_2 = atof(value);
			}else{				
				g_confvalue.mem_avg_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_weight_2")) {
			if (0 == is_number(value)) {
				g_confvalue.mem_weight_2 = atof(value);
			}else{				
				g_confvalue.mem_weight_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_section_2")) {
			if (0 == is_number(value)) {
				g_confvalue.mem_section_2 = atof(value);
			}else{				
				g_confvalue.mem_section_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_avg_2")) {
			if (0 == is_number(value)) {
				g_confvalue.io_avg_2 = atof(value);
			}else{				
				g_confvalue.io_avg_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_weight_2")) {
			if (0 == is_number(value)) {
				g_confvalue.io_weight_2 = atof(value);
			}else{				
				g_confvalue.io_weight_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_section_2")) {
			if (0 == is_number(value)) {
				g_confvalue.io_section_2 = atof(value);
			}else{				
				g_confvalue.io_section_2 = 5;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}
	//评估算法参数
	else if (0 == strcasecmp(key, "rule_num")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_num = atoi(value);
			}else{				
				g_confvalue.rule_num = 1;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_1")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[0] = atoi(value);
			}else{				
				g_confvalue.rule_clause[0] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_2")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[1] = atoi(value);
			}else{				
				g_confvalue.rule_clause[1] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_3")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[2] = atoi(value);
			}else{				
				g_confvalue.rule_clause[2] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_4")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[3] = atoi(value);
			}else{				
				g_confvalue.rule_clause[3] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_5")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[4] = atoi(value);
			}else{				
				g_confvalue.rule_clause[4] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_6")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[5] = atoi(value);
			}else{				
				g_confvalue.rule_clause[5] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_7")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[6] = atoi(value);
			}else{				
				g_confvalue.rule_clause[6] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_8")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[7] = atoi(value);
			}else{				
				g_confvalue.rule_clause[7] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_9")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[8] = atoi(value);
			}else{				
				g_confvalue.rule_clause[8] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_10")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[9] = atoi(value);
			}else{				
				g_confvalue.rule_clause[9] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_11")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[10] = atoi(value);
			}else{				
				g_confvalue.rule_clause[10] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_12")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[11] = atoi(value);
			}else{				
				g_confvalue.rule_clause[11] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_13")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[12] = atoi(value);
			}else{				
				g_confvalue.rule_clause[12] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_14")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[13] = atoi(value);
			}else{				
				g_confvalue.rule_clause[13] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_15")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[14] = atoi(value);
			}else{				
				g_confvalue.rule_clause[14] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "rule_clause_16")) {
			if (0 == is_number(value)) {
				g_confvalue.rule_clause[15] = atoi(value);
			}else{				
				g_confvalue.rule_clause[15] = 0;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}
	//平均值差值
	else if (0 == strcasecmp(key, "cpu_age_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.avg_dif.cpu = atof(value);
			}else{				
				g_confvalue.avg_dif.cpu = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_age_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.avg_dif.mem = atof(value);
			}else{				
				g_confvalue.avg_dif.mem = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_age_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.avg_dif.io = atof(value);
			}else{				
				g_confvalue.avg_dif.io = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}
	//方差值差值
	else if (0 == strcasecmp(key, "cpu_assign_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.assign_dif.cpu = atof(value);
			}else{				
				g_confvalue.assign_dif.cpu = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_assign_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.assign_dif.mem = atof(value);
			}else{				
				g_confvalue.assign_dif.mem = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_assign_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.assign_dif.io = atof(value);
			}else{				
				g_confvalue.assign_dif.io = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}
	//虚拟机售卖率差值
	else if (0 == strcasecmp(key, "domain_sell_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.domain_sell_dif= atof(value);
			}else{				
				g_confvalue.domain_sell_dif= 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}
	//资源转化利用率差值
	else if (0 == strcasecmp(key, "cpu_available_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.available_dif.cpu = atof(value);
			}else{				
				g_confvalue.available_dif.cpu = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_available_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.available_dif.mem = atof(value);
			}else{				
				g_confvalue.available_dif.mem = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_available_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.available_dif.io = atof(value);
			}else{				
				g_confvalue.available_dif.io = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}
	//资源最大值差值
	else if (0 == strcasecmp(key, "cpu_max_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.max_dif.cpu = atof(value);
			}else{				
				g_confvalue.max_dif.cpu = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "mem_max_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.max_dif.mem = atof(value);
			}else{				
				g_confvalue.max_dif.mem = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}else if (0 == strcasecmp(key, "io_max_dif")) {
			if (0 == is_number(value)) {
				g_confvalue.max_dif.io = atof(value);
			}else{				
				g_confvalue.max_dif.io = 30;
				fprintf(stderr, "this param key:%s value:%s configuration error\n", key, value);
				return -1;
			}
	}
	
	//异常
	else {
		fprintf(stderr, "this key:%s not cmp, value:%s\n", key, value);
		return -1;
	}

	return 0;
}

void print_param(const struct conf_value_stu *pconf)
{
	fprintf(stderr, "work_path:%s\nlog_path:%s\n"
			"db_server:%s\ndb_port:%u\n"
			"interval:%u\nrun_level:%d\nrun_interval:%d\n"
			"rule_clause_1:%d\nrule_clause_2:%d\n",
			pconf->work_path, pconf->log_path,
			pconf->db_server, pconf->db_port,
			pconf->interval,pconf->run_level,pconf->run_interval,
			pconf->rule_clause[0],pconf->rule_clause[1]);
}

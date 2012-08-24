#ifndef CONF_H_
#define CONF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <sys/stat.h>

#include "common.h"

enum rule
{	
	CPU_OVERLOAD = 1,
	MEM_OVERLOAD,
	IO_OVERLOAD,
	CPU_AGE,
	MEM_AGE,
	IO_AGE,
	CPU_ASSIGN,
	MEM_ASSIGN,
	IO_ASSIGN,
	DOMAIN_SELL,
	CPU_AVAILABLE,
	MEM_AVAILABLE,
	IO_AVAILABLE,
	CPU_MAX,
	MEM_MAX,
	IO_MAX,
};

struct values
{
	float cpu;
	float io;
	float mem;
};

struct listen_stu {
	char            ip[256];
	unsigned short  port;
};

struct conf_value_stu
{
	char			work_path[256];
	char			log_path[256];
	unsigned		interval;
	
	//http server
	struct listen_stu	http_listen;

	//数据库信息
	int				db_retry;
	char			db_server[256];	
	char			db_user[256];	
	char			db_passwd[256];	
	char			db_name[256];	
	unsigned short	db_port;

	//运行等级
	int 			run_level;
	int				run_interval;
	float			cpu_avg_1;
	float			cpu_weight_1;	
	float			cpu_section_1;
	float			mem_avg_1;	
	float			mem_weight_1;
	float			mem_section_1;
	float			io_avg_1;	
	float			io_weight_1; 
	float			io_section_1;
	float			cpu_avg_2;	
	float			cpu_weight_2;
	float			cpu_section_2;
	float			mem_avg_2;	
	float			mem_weight_2;
	float			mem_section_2;
	float			io_avg_2;	
	float			io_weight_2;
	float			io_section_2;
	struct values	avg_1;
	struct values	avg_2;
	struct values	weight_1;
	struct values	weight_2;
	struct values	section_1;
	struct values	section_2;

	//评估算法
	int 			rule_num;
	int				rule_clause[16];
	struct values	avg_dif;
	struct values	assign_dif;
	float			domain_sell_dif; 			
	struct values	available_dif;
	struct values	max_dif;
	
};

int load_profile(const char *profile);
void print_param(const struct conf_value_stu *pconf);

#endif // CONF_H_


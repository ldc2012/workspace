#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#include "log.h"
#include "conf.h"
#include "common.h"
#include "http_server.h"
#include "evaluation_server.h"

#define VERSION_INFO	"cloud_evaluation_center_v1.3.1"


/* 全局变量 */
extern struct conf_value_stu g_confvalue;
extern char g_log_path[256];

static void print_usage();
static void daemon_server();
static void process();


int main(int argc, char *argv[])
{
	char ch;
	char pathname[256];

	if (1 == argc) {
		print_usage();
		return -1;
	} else {
		while (-1 != (ch = getopt(argc, argv, "c:hv"))) {
			switch (ch) {
				case 'c':
						snprintf(pathname, sizeof(pathname), "%s", optarg);
						break;
				case 'h':
						//print_usage(); 
						break;
				case 'v':
						fprintf(stderr, "%s\n", VERSION_INFO); 
						break;
				default:
						break;
			}
		}
	}
    
	if (-1 == load_profile(pathname))
		return -1;
	print_param(&g_confvalue);
	if (-1 == create_dir(g_confvalue.log_path, 0644))
		return -1;
	snprintf(g_log_path, sizeof(g_log_path), "%s", g_confvalue.log_path);

    //process();
    daemon_server();

	return 0;
}

void print_usage()
{
	fprintf(stderr, "%s Usage:\n\
				-c config file name\n\
				-h help\n\
				-v program version\n", PROGRAM_NAME);
}


void daemon_server()
{
	if (0 != daemon(0, 0)) {
		logerr("daemon(0, 0) fail: %s\n", strerror(errno));
		exit(100);
	}    
    	
	if (0 != chdir(g_confvalue.work_path)) {
		logerr("chdir() %s fail: %s\n", g_confvalue.work_path);
		exit(101);
	}
 
	while (1) {
		pid_t pid = fork();
		if (-1 == pid) {
			logerr("fork fail");
			exit(102);
		} else if (0 == pid) {
			process();
			exit(103); 
		}

		int stat;
		waitpid(pid, &stat, 0);
		if (WIFEXITED(stat)) {
			logrun("the child process:%u terminated normally, exit status:%d", 
					(unsigned int)(pid), WEXITSTATUS(stat));
		} else if (WIFSIGNALED(stat)) {
			logerr("process: %u is killded singal: %d, and program is restarting", 
					(unsigned int)(pid), WTERMSIG(stat));
		}
	}

	logerr("call abort()");
	abort();
}

void process()
{

	/* 启动httpserver监听服务 */
	unsigned short port(g_confvalue.http_listen.port);
	string ip(g_confvalue.http_listen.ip);
	short timeout(5);
	CHttpServer httpserver(port, ip.c_str(), timeout);
	if (!httpserver.work()) {
		logerr("httpserver start fail");
		return;
	}

	/* 启动evaluationserver评估服务 */
	CEvaluationServer evaluationserver;
    if (!evaluationserver.work()) {
		logerr("evaluationserver start fail");
		return;
	}

	while(1) {
		select_sleep(1000, 0);
	}
}



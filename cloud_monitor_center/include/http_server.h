#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <stdlib.h>
#include <pthread.h>

#include <sys/queue.h>
#include <event.h>
#include <evhttp.h>

#include <json.h>

#include <string>

#include "common.h"
#include "conf.h"
#include "log.h"
#include "http_post.h"

using namespace std;

#define         SHORT_BUF_LEN    256

/* 云物理主机运行性能评估指标 */
#define         IAAS_CODE        "cloudcode"
#define         HOST_ID          "machinecode"
#define         TIMESTAMP        "timestamp"
#define         PM_CPU           "pm_cpu"
#define         PM_MEM           "pm_mem"
#define         PM_IO            "pm_io"

/* 虚拟机运行性能评估指标 */
#define         VM_ID            "vm_code"
#define         VM_CPU           "vm_cpu"
#define         VM_MEM           "vm_mem"
#define         VM_IO            "vm_io"

/* http request uri params */
struct param_stu {
	char *task_id;
	char *start_time;
	char *end_time;
};

/* 物理主机运行性能数据 */
struct host_running_performance_data {
	char     iaas_code[SHORT_BUF_LEN];      // 云环境代码 
	char     iaas_host_id[SHORT_BUF_LEN];   // 云环境内部物理机编号
	char     timestamp[SHORT_BUF_LEN];      // 数据时间

	float    cpu_use_rate;                  // CPU平均使用率
	float    memory_use_rate;               // 内存平均使用率
	float    io_use_rate;                   // IO平均使用率
};
typedef host_running_performance_data host_data_st;

/* 虚拟机运行性能数据 */
struct vm_running_performance_data {
		char     iaas_code[SHORT_BUF_LEN];      // 云环境代码
		char     iaas_host_id[SHORT_BUF_LEN];   // 云环境内部物理机编号
		char     iaas_vm_id[SHORT_BUF_LEN];     // 云环境内部虚拟机编号
		char     timestamp[SHORT_BUF_LEN];      // 数据时间  
					 
		float    cpu_use_rate;                  // CPU平均使用率
		float    memory_use_rate;               // 内存平均使用率
		float    io_use_rate;                   // IO平均使用率
};
typedef vm_running_performance_data vm_data_st;


class CHttpServer {
public:
		CHttpServer(const unsigned short http_port, const char *http_addr, const short timeout);
		~CHttpServer();
		bool work();
		
private:
		/* http server */
		static void *http_start_server(void *arg);
		static void http_handle_postdata(struct evhttp_request *req, void *arg);
		static void parse_openstack_data(void *arg,
				                         struct evhttp_request *req, 
				                         struct evkeyvalq params, 
										 char *post_data);
		static void http_reponse(struct evhttp_request *req, 
				                 struct evkeyvalq *params, 
								 int statcode, 
								 char *statreason);
        /* check log buffer */	
		static void *check_buffer(void *arg);
		
private:
	const unsigned short  m_http_port;
	const char  *m_http_addr;
	const short  m_timeout;

	pthread_mutex_t m_mutex;
};

#endif// HTTP_SERVER_H_

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <stdlib.h>

#include <sys/queue.h>
#include <pthread.h>

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include <event.h>
#include <evhttp.h>

#include <json.h>

#include "mysql_db.h"
#include "common.h"
#include "conf.h"
#include "log.h"

using namespace std;


#define     SHORT_BUF_LEN    256

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

struct listen_param_stu {
		char			*ip;
		unsigned short	port;
		unsigned short	timeout;
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
	/**
	 * @brief 启动httpserver listen，注册url
	 * @param arg
	 * @return 
	 */
	static void *start_http_server(void *arg);
	static void receive_cloud_data(struct evhttp_request *req, void *arg);
	static void parse_cloud_data(MYSQL *conn, void *arg, struct evhttp_request *req, struct evkeyvalq params, char *post_data);
	static void http_reponse(struct evhttp_request *req, struct evkeyvalq *params, int statcode, char *statreason);
	/**
	 * @brief 获得OpenStack物理主机运行性能数据，并写入数据库表
	 * @param conn MYSQL
	 * @param arg this指针
	 * @param host
	 * @return
	 */
	static void pmdata_to_db(MYSQL *conn, void *arg, host_data_st *host);
	/**
	 * @brief 获得OpenStack虚拟机运行性能数据，并写入数据库表
	 * @param conn MYSQL
	 * @arg this指针
	 * @param vm
	* @return
	 */
	static void vmdata_to_db(MYSQL *conn, void *arg, vm_data_st *vm);
	/**
	 * @brief auto create table by hour
	 * @param arg
	 * @return 
	 */
	static void *auto_create_table(void *arg);
	/**
	  * @brief create host table 
	  * @param conn MYSQL
	  * @return
	  */
	static void create_host_table(MYSQL *conn);
	/**
	  * @brief create vm table
	  * @param conn MYSQL
	  * @return 
	  */
	static void create_vm_table(MYSQL *conn);
	/**
	  * @brief create vm running level data
	  * @param conn MYSQL
	  * @return
	  */
	static void create_vm_level(MYSQL *conn);
	/**
	  * @brief 检查当前时间表是否存在
	  * @param check_table_name 表名,当前时间15时，则表：host_running_performance_data_15
	  * @return 存在返回true，不存在返回false
	  */
	static bool check_table_exists(char *check_table_name);

private:
	const unsigned short  m_http_port;
	const char  *m_http_addr;
	const short  m_timeout;

	//pthread_mutex_t m_mutex;
};

#endif // HTTP_SERVER_H_ 

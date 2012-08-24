#include "http_server.h"

extern struct conf_value_stu  g_confvalue;


CHttpServer::CHttpServer(const unsigned short http_port, const char *http_addr, const short
		timeout) 
			:m_http_port(http_port), m_http_addr(http_addr), m_timeout(timeout) {	
	//pthread_mutex_init(&m_mutex, NULL);
	/*
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&m_mutex, &attr);
	pthread_mutexattr_destroy (&attr);
	*/
	logrun("constructor function ... ...");
}

CHttpServer::~CHttpServer() {	
	//pthread_mutex_destroy(&m_mutex);
	logrun("destructor function ... ...");
}

bool CHttpServer::work() {

	int err;
    pthread_t tid;
	pthread_attr_t tattr;
	pthread_attr_init(&tattr);

	if (0 != (err = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED))) {
		logerr("pthread_attr_setdetachstate fail: %s", strerror(err));
		return false;
	}
	if (0 != (err = pthread_create(&tid, &tattr, auto_create_table, this))) {
		logerr("pthread_create auto_create_table fail: %s", strerror(err));
		return false;
	}
	if (0 != (err = pthread_create(&tid, &tattr, start_http_server, this))) {
		logerr("pthread_create http_start_server fail: %s", strerror(err));
		return false;
	}

	return true;
}

void *CHttpServer::start_http_server(void *arg)
{
	CHttpServer *pthis = (CHttpServer *)arg;

	event_init();
	struct evhttp *httpd = NULL;
	httpd = evhttp_start(pthis->m_http_addr, pthis->m_http_port);
	evhttp_set_timeout(httpd, pthis->m_timeout);
	logrun("http server start:%s:%d timeout:%u",
			pthis->m_http_addr, 
			pthis->m_http_port, 
			pthis->m_timeout);
	evhttp_set_gencb(httpd, receive_cloud_data, arg);
	//evhttp_set_cb(httpd, "/openstack", receive_openstack_data, NULL); // post不支持
	event_dispatch();
	evhttp_free(httpd);

	return NULL;
}

void CHttpServer::receive_cloud_data(struct evhttp_request *req, void *arg)
{
	char *decode_uri = strdup((char*) evhttp_request_uri(req));
	struct evkeyvalq params;
	evhttp_parse_query(decode_uri, &params);
	free(decode_uri);

	//POST
	int buffer_data_len;
	buffer_data_len = EVBUFFER_LENGTH(req->input_buffer);

	if (buffer_data_len) { /* 接收POST正文信息 */
		char *buffer_data = (char *) malloc(buffer_data_len + 1);
		memset(buffer_data, '\0', buffer_data_len + 1);
		memcpy(buffer_data, EVBUFFER_DATA(req->input_buffer), buffer_data_len);	
//printf("--------------------------------Start-----------------------------------------------------\n");
//printf("%s\n", buffer_data);
//printf("--------------------------------End-------------------------------------------------------\n");
		
		MYSQL conn_mysql;
		if (NULL == mysql_conn(&conn_mysql, g_confvalue.db_server, 
					g_confvalue.db_user, g_confvalue.db_passwd, 
					g_confvalue.db_name, g_confvalue.db_port)) {
			logerr("receive_cloud_data mysql connect fail!!!");
			http_reponse(req, &params, 300, "FAIL");
			free(buffer_data);
			select_sleep(60, 0); // 1mins
			return;
		}

		http_reponse(req, &params, 200, "OK");
		parse_cloud_data(&conn_mysql, arg, req, params, buffer_data);
		free(buffer_data);
		/* close mysql conn */
		logrun("receive_cloud_data close mysql");
		mysql_close(&conn_mysql);
		mysql_library_end();
	} else { /* POST正文无内容 */
		logrun("rece post data nothing.");
		http_reponse(req, &params, 400, "ERROR");
	}

	return;
}

void CHttpServer::http_reponse(struct evhttp_request *req, 
							   struct evkeyvalq *params, 
						       int statcode, 
						       char *statreason)

{
	struct evbuffer *buff = evbuffer_new();

	evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
	evhttp_add_header(req->output_headers, "Connection", "close");
	evhttp_send_reply(req, statcode, statreason, buff);
	evhttp_clear_headers(params);
	evbuffer_free(buff);

	return;
}

void CHttpServer::parse_cloud_data(MYSQL *conn,
		                           void *arg,
		                           struct evhttp_request *req, 
								   struct evkeyvalq params, 
								   char *post_data)
{
	struct json_object *fst_obj = NULL, *snd_obj = NULL, *trd_obj = NULL;

	fst_obj = json_tokener_parse(post_data);
	if (is_error(fst_obj)) {
#ifdef DEBUG		
		printf("parse json cloud data fail!!! data:(%s)\n", post_data);
#else
	    logerr("parse json cloud data fail!!! data:(%s)", post_data );
#endif 
		
		return;
	}

	char g_timestamp[32]={0};
	host_data_st host_data;
	vm_data_st vm_data;
	memset((void*)&host_data, 0, sizeof(host_data));
	memset((void*)&vm_data, 0, sizeof(vm_data));
	
	if (json_object_get_type(fst_obj) == json_type_array) {
		for (int i=0 ; i<json_object_array_length(fst_obj) ; i++ ) {
			snd_obj = json_object_array_get_idx(fst_obj, i );
			if (NULL == snd_obj) {
				logerr("parse cloud data get snd_obj == NULL!\n");
				json_object_put(fst_obj);
				return;
			}
			json_object_object_foreach(snd_obj, key, val) {
				if (json_object_get_type(val) == json_type_array) {
					for (int j=0; j<json_object_array_length(val); j++) {
						trd_obj = json_object_array_get_idx(val, j);
						if (NULL == trd_obj) {
							logerr("parse cloud data get trd_obj == NULL!\n");
							json_object_put(fst_obj);
							json_object_put(snd_obj);
							return;
						}
						json_object_object_foreach(trd_obj, key, val) {
							/* padding vm data info */
							if (NULL == val) {
								logerr("json parse vm data as NULL!!!");
								json_object_put(fst_obj);
								json_object_put(snd_obj);
								json_object_put(trd_obj);
								return;
							}
							strcpy(vm_data.iaas_code, host_data.iaas_code);
							strcpy(vm_data.iaas_host_id, host_data.iaas_host_id);
							strcpy(vm_data.timestamp, host_data.timestamp);
							if (strcmp(key, VM_ID)==0)
								strcpy(vm_data.iaas_vm_id, json_object_get_string(val));
							else if (strcmp(key, VM_CPU)==0)
								vm_data.cpu_use_rate = json_object_get_double(val);
							else if (strcmp(key, VM_MEM)==0)
								vm_data.memory_use_rate = json_object_get_double(val);
							else if (strcmp(key, VM_IO)==0)
								vm_data.io_use_rate = json_object_get_double(val);
						}
#ifdef DEBUG			
						printf("虚拟机运行数据 [%s][%s][%s][%s][%f][%f][%f] \n",
								vm_data.iaas_code,
								vm_data.iaas_host_id,
								vm_data.iaas_vm_id,
								vm_data.timestamp,
                                vm_data.cpu_use_rate,
                                vm_data.memory_use_rate,
								vm_data.io_use_rate);
#endif	
						/* 虚拟机数据插入数据库 */
						vmdata_to_db(conn, arg, &vm_data);
					}
				} else {
					/* padding host data info */
					if (NULL == val) {
						logerr("json parse host data as NULL.");
						json_object_put(fst_obj);
						json_object_put(snd_obj);
						json_object_put(trd_obj);
						return;
					}
					time_t tt = get_time_utc(json_object_get_string(val),
							"%Y-%m-%d %H:%M:%S");
					sprintf(g_timestamp, "%ld", tt);
					if (strcmp(key, IAAS_CODE)==0)
						strcpy(host_data.iaas_code, json_object_get_string(val));
					else if (strcmp(key, HOST_ID)==0)
						strcpy(host_data.iaas_host_id, json_object_get_string(val));
					else if (strcmp(key, TIMESTAMP)==0)
						strcpy(host_data.timestamp, g_timestamp);
					else if (strcmp(key, PM_CPU)==0)
						host_data.cpu_use_rate = json_object_get_double(val);
					else if (strcmp(key, PM_MEM)==0)
						host_data.memory_use_rate = json_object_get_double(val);
					else if (strcmp(key, PM_IO)==0)
						host_data.io_use_rate = json_object_get_double(val);
				}
			}
#ifdef DEBUG           
			printf("物理主机运行数据 [%s][%s][%s][%f][%f][%f] \n",
					host_data.iaas_code,
					host_data.iaas_host_id,
					host_data.timestamp,
					host_data.cpu_use_rate,
					host_data.memory_use_rate,
					host_data.io_use_rate);
#endif		
			/* 物理主机数据插入数据库 */
			pmdata_to_db(conn, arg, &host_data);
		}	
	}
    
	if (NULL != fst_obj)
		json_object_put(fst_obj);
	if (NULL != snd_obj)
		json_object_put(snd_obj);
	if (NULL != trd_obj)
		json_object_put(trd_obj);
    
	return;
}

void CHttpServer::pmdata_to_db(MYSQL *conn, void *arg, host_data_st *host)
{
	CHttpServer *pthis = (CHttpServer *)arg;

#ifdef DEBUG
	printf("物理主机运行数据入库:[%s][%s][%s][%f][%f][%f]\n",
			host->iaas_code,
			host->iaas_host_id,
			host->timestamp,
			host->cpu_use_rate,
			host->memory_use_rate,
			host->io_use_rate);
#else
	logrun("物理主机运行数据入库:[%s][%s][%s][%f][%f][%f]",
			host->iaas_code,
			host->iaas_host_id,
			host->timestamp,
			host->cpu_use_rate,
			host->memory_use_rate,
			host->io_use_rate);
#endif	
    	
	char tm_buf[64];
	bzero(&tm_buf, 0);
	get_time_str(host->timestamp, tm_buf);

	//pthread_mutex_lock(&pthis->m_mutex);
	loghost("%s %s %s %f %f %f",
			tm_buf,
			host->iaas_code,
			host->iaas_host_id,
			host->cpu_use_rate,
			host->memory_use_rate,
			host->io_use_rate);
	//pthread_mutex_unlock(&pthis->m_mutex);

	// host data insert into MySql
    int iaas_id; /* 云环境统一编号 */
	int host_id; /* 物理主机统一编号 */

	list<vector<string> > lst;
	vector<string> vec;
	char select_sql[1024];
    sprintf(select_sql, "select iaas_id, host_id from host_info where (iaas_id in (select iaas_id from iaas_info where iaas_code = '%s')) and iaas_host_id = '%s';", host->iaas_code, host->iaas_host_id);
    int ret = mysql_exec_table(conn, select_sql, lst);
	list<vector<string> >::iterator iter;
	vector<string>::iterator it;
	if (ret > 0) {
		for(iter = lst.begin(); iter != lst.end(); iter++) { // 行
			vec = *iter;
			for(int i = 0; i < vec.size(); i++) { // 列
				iaas_id = atoi(vec[0].c_str());
				host_id = atoi(vec[1].c_str());
			}
		}
	} else {
#ifdef DEBUG
		printf("mysql_exec_table ret [%d] select sql:%s\n", ret, select_sql);
#else
		logrun("mysql_exec_table ret [%d] select sql:%s", ret, select_sql);
#endif
		return;
	}

	/* 检查当前时间表是否存在 */
	char tbl_name[64];
	bzero(&tbl_name, 0);
	sprintf(tbl_name, "%s", "host_running_performance_data");
	get_table_name(tbl_name, 0);
	if (!check_table_exists(tbl_name)) {
		create_host_table(conn);
	}

	char insert_sql[512];
	sprintf(insert_sql, "INSERT INTO %s values(NULL,'%d', '%s','%s','%d', '%s','%f','%f','%f')",
			tbl_name,
			iaas_id,
			host->iaas_host_id,
			host->iaas_code, 
			host_id,
            host->timestamp,
			host->cpu_use_rate, 
			host->memory_use_rate, 
			host->io_use_rate);

	int ret_insert = mysql_exec_single(conn, insert_sql);
	if (-1 == ret_insert) {
#ifdef DEBUG
		printf("pmdata_to_db fail, sql: %s\n", insert_sql);
#else
		logerr("pmdata_to_db fail, sql: %s", insert_sql);
#endif
	} else {
#ifdef DEBUG
		printf("pmdata_to_db success, ret [%d]\n", ret_insert);
#else
		logrun("pmdata_to_db success, ret [%d]", ret_insert);
#endif
	}

	return;
}

void CHttpServer::vmdata_to_db(MYSQL *conn, void *arg, vm_data_st *vm)
{
	CHttpServer *pthis = (CHttpServer *)arg;

#ifdef DEBUG
	printf("虚拟机运行数据入库:[%s][%s][%s][%s][%f][%f][%f]\n",
			vm->iaas_code, /* 云环境代码 */
			vm->iaas_host_id, /* 云环境内部物理机编号 */ 
			vm->iaas_vm_id, /* 云环境内部虚拟机编号 */
			vm->timestamp, 
			vm->cpu_use_rate,
			vm->memory_use_rate,
			vm->io_use_rate);
#else
	logrun("虚拟机运行数据入库:[%s][%s][%s][%s][%f][%f][%f]",
			vm->iaas_code,
			vm->iaas_host_id,
			vm->iaas_vm_id,
			vm->timestamp,
			vm->cpu_use_rate,
			vm->memory_use_rate,
			vm->io_use_rate);
#endif

	char tm_buf[64];
	bzero(&tm_buf, 0);
	get_time_str(vm->timestamp, tm_buf);

	//pthread_mutex_lock(&pthis->m_mutex);
	logvm("%s %s %s %s %f %f %f",
			tm_buf,
			vm->iaas_code,
			vm->iaas_host_id,
			vm->iaas_vm_id,
			vm->cpu_use_rate,
			vm->memory_use_rate,
			vm->io_use_rate);
	//pthread_mutex_unlock(&pthis->m_mutex);

	// vm data insert into MySql
	int iaas_id; /* 云环境统一编号 */
	int host_id; /* 物理主机统一编号 */
    int vm_id; /* 虚拟机机统一编号 */
	
	list<vector<string> > lst;
	vector<string> vec;
	char select_sql[1024];
    sprintf(select_sql, "select iaas_id,host_id,vm_id from vm_info where (iaas_id in (select iaas_id from iaas_info where iaas_code = '%s')) and iaas_host_id = '%s' and iaas_vm_id = '%s';", vm->iaas_code, vm->iaas_host_id, vm->iaas_vm_id);
    int ret = mysql_exec_table(conn, select_sql, lst);
	list<vector<string> >::iterator iter;
	vector<string>::iterator it;
	if (ret > 0) {
		for(iter = lst.begin(); iter != lst.end(); iter++) { // 行
			vec = *iter;
			//for(int i = 0; i < vec.size(); i++) { // 列 
			    iaas_id = atoi(vec[0].c_str());
				host_id = atoi(vec[1].c_str());
				vm_id = atoi(vec[2].c_str());
			//}
		}
	} else {
#ifdef DEBUG
		printf("mysql_exec_table ret [%d] select sql:%s\n", ret, select_sql);
#else
		logrun("mysql_exec_table ret [%d] select sql:%s", ret, select_sql);
#endif
		return;
	}

	/* 检查当前时间表是否存在 */
	char tbl_name[64];
	bzero(&tbl_name, 0);
	sprintf(tbl_name, "%s", "vm_running_performance_data");
	get_table_name(tbl_name, 0);
	if (!check_table_exists(tbl_name)) {
		create_vm_table(conn);
	}

	char insert_sql[512];
	bzero(&insert_sql, 0);
	sprintf(insert_sql, "INSERT INTO %s values(NULL,'%d', '%s', '%d','%s', '%d', '%s', '%s', '%f','%f','%f')",
			tbl_name,
			iaas_id,
			vm->iaas_code, 
			host_id, 
			vm->iaas_host_id,
			vm_id,
			vm->iaas_vm_id,
			vm->timestamp,
			vm->cpu_use_rate,
			vm->memory_use_rate,
			vm->io_use_rate);
	
	int ret_insert = mysql_exec_single(conn, insert_sql);
	if (-1 == ret_insert) {
#ifdef DEBUG
		printf("vmdata_to_db fail, sql: %s\n", insert_sql);
#else
		logerr("vmdata_to_db fail, sql: %s", insert_sql);
#endif
	} else {
#ifdef DEBUG
		printf("vmdata_to_db success, ret [%d]\n", ret_insert);
#else
		logrun("vmdata_to_db success, ret [%d]", ret_insert);
#endif
	}

	return;
}

void *CHttpServer::auto_create_table(void *arg) {

	char drop_host_sql[64];
	char drop_vm_sql[64];
	char drop_level_sql[64];
	char drop_host_table[64];
	char drop_vm_table[64];
	char drop_level_table[64];

	while(1) {
		/* init mysql connect */
		MYSQL conn_mysql;
		if (NULL == mysql_conn(&conn_mysql, g_confvalue.db_server,
				g_confvalue.db_user, g_confvalue.db_passwd,
				g_confvalue.db_name, g_confvalue.db_port)) {
			logerr("auto_create_table connect mysql fail!!!");
			select_sleep(120, 0); // 2mins
			continue;
		}
		//------------------------drop table----------------------------------------------
		bzero(&drop_host_sql, 0);
		bzero(&drop_vm_sql, 0);
		bzero(&drop_level_sql, 0);
		bzero(&drop_host_table, 0);
		bzero(&drop_vm_table, 0);
		bzero(&drop_level_table, 0);

		sprintf(drop_host_table, "%s", "host_running_performance_data");
		sprintf(drop_vm_table, "%s", "vm_running_performance_data");
		sprintf(drop_level_table, "%s", "vm_running_level_data");
		
		get_table_name(drop_host_table, -12);
		get_table_name(drop_vm_table, -12); 
		get_table_name(drop_level_table, -12);
		sprintf(drop_host_sql, "drop table %s;", drop_host_table);
		sprintf(drop_vm_sql, "drop table %s;", drop_vm_table);
		sprintf(drop_level_sql, "drop table %s;", drop_level_table);
        
		int ret;
		ret = mysql_exec_single(&conn_mysql, drop_host_sql);
		if (ret < 0) {
			logerr("drop host table %s fail, ret:[%d], sql:[%s]\n", drop_host_table, ret, drop_host_sql);
		} else {
			logrun("drop host table %s success, ret:[%d], sql:[%s]\n", drop_host_table, ret, drop_host_sql);
		}
        
		ret = mysql_exec_single(&conn_mysql, drop_vm_sql);
		if (ret < 0) {
			logerr("drop vm table %s fail, ret:[%d], sql:[%s]\n", drop_vm_table, ret, drop_vm_sql);
		} else {
			logrun("drop vm table %s success, ret:[%d], sql:[%s]\n", drop_vm_table, ret, drop_vm_sql);
		}
		
		ret = mysql_exec_single(&conn_mysql, drop_level_sql);
		if (ret < 0) {
			logerr("drop level table %s fail, ret:[%d], sql:[%s]\n", drop_level_table, ret, drop_level_sql);
		} else {
			logrun("drop level table %s success, ret:[%d], sql:[%s]\n", drop_level_table, ret, drop_level_sql);
		}

		//----------------create table------------------------------------------------------
        create_host_table(&conn_mysql);
		create_vm_table(&conn_mysql);
		create_vm_level(&conn_mysql);

		/* close mysql conn */
		logrun("auto_create_table close mysql");
		mysql_close(&conn_mysql);
		mysql_library_end();
		/* 半小时检查一次 */
		select_sleep(1800, 0);
	}

	return NULL;
}
 
void CHttpServer::create_host_table(MYSQL *conn) {

		char create_host_sql[512];
		bzero(&create_host_sql, 0);

		for(int i=0; i<4; i++) {
			char host_table_name[64];
			bzero(&host_table_name, 0);
			sprintf(host_table_name, "%s", "host_running_performance_data");

			get_table_name(host_table_name, i-1);
	
			sprintf(create_host_sql, "CREATE TABLE IF NOT EXISTS `%s` (`id` BIGINT(20) NOT NULL AUTO_INCREMENT,`iaas_id` BIGINT(20) NULL DEFAULT NULL,`iaas_host_id` VARCHAR(32) NOT NULL,`iaas_code` VARCHAR(16) NOT NULL,`host_id` BIGINT(20) NULL DEFAULT NULL,`data_time` INT(11) NOT NULL,`cpu_use_rate` FLOAT NOT NULL,`memory_use_rate` FLOAT NOT NULL,`io_use_rate` FLOAT NOT NULL,PRIMARY KEY (`id`))COLLATE='utf8_general_ci' ENGINE=MyISAM AUTO_INCREMENT=1;", host_table_name);
	
			int ret_host = mysql_exec_single(conn, create_host_sql);
			if (ret_host < 0) {
#ifdef DEBUG
			printf("create host table %s fail, ret:[%d], sql:[%s]\n", host_table_name, ret_host, create_host_sql);
#else
			logerr("create host table %s fail, ret:[%d], sql:[%s]", host_table_name, ret_host, create_host_sql);
#endif
			} else {
#ifdef DEBUG
			printf("create host table %s success, ret:[%d], sql:[%s]\n", host_table_name, ret_host, create_host_sql);
#else
			logrun("create host talbe %s success, ret:[%d], sql:[%s]", host_table_name, ret_host, create_host_sql);
#endif
			}
		}
		
	    return;
}

void CHttpServer::create_vm_table(MYSQL *conn) {

		char create_vm_sql[1024];
		bzero(&create_vm_sql, 0);

		for(int i=0; i<4; i++) {
			char vm_table_name[64];
			bzero(&vm_table_name, 0);
			sprintf(vm_table_name, "%s", "vm_running_performance_data");

			get_table_name(vm_table_name, i-1);
			sprintf(create_vm_sql, "CREATE TABLE IF NOT EXISTS `%s` (`id` BIGINT(20) NOT NULL AUTO_INCREMENT,`iaas_id` BIGINT(20) NULL DEFAULT NULL,`iaas_code` VARCHAR(16) NULL DEFAULT NULL,`host_id` BIGINT(20) NULL DEFAULT NULL,`iaas_host_id` VARCHAR(32) NULL DEFAULT NULL,`vm_id` BIGINT(20) NULL DEFAULT NULL,`iaas_vm_id` VARCHAR(64) NOT NULL,`data_time` INT(11) NOT NULL,`cpu_use_rate` FLOAT NOT NULL,`memory_use_rate` FLOAT NOT NULL,`io_use_rate` FLOAT NOT NULL,PRIMARY KEY (`id`))COLLATE='utf8_general_ci' ENGINE=MyISAM AUTO_INCREMENT=1;", vm_table_name);
			int ret_vm = mysql_exec_single(conn, create_vm_sql);
			if (ret_vm < 0) {
#ifdef DEBUG
			printf("create vm table %s fail, ret:[%d], sql:[%s]\n", vm_table_name, ret_vm, create_vm_sql);
#else
			logerr("create vm table %s fail, ret:[%d], sql:[%s]", vm_table_name, ret_vm, create_vm_sql);
#endif
			} else {
#ifdef DEBUG
			printf("create vm table %s success, ret:[%d], sql:[%s]\n", vm_table_name, ret_vm, create_vm_sql);
#else
			logrun("create vm talbe %s success, ret:[%d], sql:[%s]", vm_table_name, ret_vm, create_vm_sql);
#endif
			}
		}

	    return;
}

void CHttpServer::create_vm_level(MYSQL *conn) {
		
		char create_level_sql[512];
		bzero(&create_level_sql, 0);

		for(int i=0; i<4; i++) {
			char level_table_name[64];
			bzero(&level_table_name, 0);
			sprintf(level_table_name, "%s", "vm_running_level_data");

			get_table_name(level_table_name, i-1);
			sprintf(create_level_sql, "CREATE TABLE IF NOT EXISTS `%s` (`id` BIGINT(20) NOT NULL AUTO_INCREMENT,`data_time` INT(11) NOT NULL,`vm_id` BIGINT(20) NOT NULL,`running_level` VARCHAR(8) NOT NULL,`lack_resource` VARCHAR(8) NOT NULL,PRIMARY KEY (`id`))COLLATE='utf8_general_ci' ENGINE=MyISAM AUTO_INCREMENT=1;", level_table_name);
	
			int ret_level = mysql_exec_single(conn, create_level_sql);
			if (ret_level < 0) {
#ifdef DEBUG
			printf("create vm running level table %s fail, ret:[%d], sql:[%s]\n", level_table_name, ret_level, create_level_sql);
#else
			logerr("create vm running level table %s fail, ret:[%d], sql:[%s]", level_table_name, ret_level, create_level_sql);
#endif
			} else {
#ifdef DEBUG
			printf("create vm running level table %s success, ret:[%d], sql:[%s]\n", level_table_name, ret_level, create_level_sql);
#else
			logrun("create vm running level talbe %s success, ret:[%d], sql:[%s]", level_table_name, ret_level, create_level_sql);
#endif
			}
		}

		return;
}

bool CHttpServer::check_table_exists(char *table_name) {
		/* init mysql connect */
		MYSQL conn_mysql;
		if (NULL == mysql_conn(&conn_mysql, g_confvalue.db_server,
			g_confvalue.db_user, g_confvalue.db_passwd,
			g_confvalue.db_name, g_confvalue.db_port)) {
			logerr("check_table_exists error !!!");
			return false;
		}
	
		char check_sql[256];
		bzero(&check_sql, 0);
		
	    sprintf(check_sql, "select count(*) from `INFORMATION_SCHEMA`.`TABLES` where `TABLE_SCHEMA`='%s' and `TABLE_NAME`='%s';", g_confvalue.db_name, table_name);
		int select_count;
		int ret = mysql_select_count(&conn_mysql, check_sql, &select_count);
        if (ret < 0) {
#ifdef DEBUG
		printf("select table %s fail, ret:[%d], sql:[%s]\n", table_name, ret, check_sql);
#else
		logerr("select table %s fail, ret:[%d], sql:[%s]", table_name, ret, check_sql);
#endif
		} else {
#ifdef DEBUG
		printf("select table %s  success, ret:[%d], sql:[%s]\n", table_name, ret, check_sql);
#else
		logrun("select talbe %s success, ret:[%d], sql:[%s]", table_name, ret, check_sql);
#endif
		}

		/* close mysql conn */
        logrun("check_table_exists close mysql");
		mysql_close(&conn_mysql);
        mysql_library_end(); 
#ifdef DEBUG
		printf("check table : [%s], select_count: [%d]\n", table_name, select_count);
#else
		logrun("check table : [%s], select_count: [%d]\n", table_name, select_count);
#endif		
		if (0 == select_count)
			return false;
		
		return true;
}


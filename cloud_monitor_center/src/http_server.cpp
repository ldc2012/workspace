#include "http_server.h"


extern struct conf_value_stu g_confvalue;

CHttpServer::CHttpServer(const unsigned short http_port, const char *http_addr, const short timeout) :m_http_port(http_port), m_http_addr(http_addr), m_timeout(timeout) {
	pthread_mutex_init(&m_mutex, NULL);
}

CHttpServer::~CHttpServer() {
	pthread_mutex_destroy(&m_mutex);
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
	// start http server
	if (0 != (err = pthread_create(&tid, &tattr, http_start_server, this))) {
			logerr("pthread_create http_start_server fail: %s", strerror(err));
			return false;
	}	
	// check cache data log buffer
	if (0 != (err = pthread_create(&tid, &tattr, check_buffer, this))) {
			logerr("pthread_create check_buffer fail: %s", strerror(err));
			return false;
	}
	
	return true;
}

void *CHttpServer::http_start_server(void *arg)
{
	CHttpServer *pthis = (CHttpServer *)arg;
	struct evhttp *httpd = NULL;
    	
	event_init();
	httpd = evhttp_start(pthis->m_http_addr, pthis->m_http_port);
	if (NULL == httpd) {
           printf("http server unable to listen on %s:%d\n\n", pthis->m_http_addr, pthis->m_http_port);
	   return NULL; 
    }
    evhttp_set_timeout(httpd, pthis->m_timeout);       
    evhttp_set_gencb(httpd, http_handle_postdata, arg);

#ifdef DEBUG
	printf("http_server start:%s:%d timeout:%u\n", 
			pthis->m_http_addr, 
			pthis->m_http_port, 
			pthis->m_timeout);
#else
	logrun("http_server start:%s:%d timeout:%u", 
			pthis->m_http_addr, 
			pthis->m_http_port, 
			pthis->m_timeout);
#endif
	event_dispatch();
	evhttp_free(httpd);
	
	return NULL;
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

void CHttpServer::http_handle_postdata(struct evhttp_request *req, void *arg)
{
	CHttpServer *pthis = (CHttpServer *)arg;
    	
    // 请求是用POST发送的，下面的evhttp_request_uri和evhttp_parse_query函数
	// 不能正确解析uri中的参数，Libevent库的BUG！
	char *decode_uri;
	struct evkeyvalq params;
	decode_uri = strdup((char *)evhttp_request_uri(req));
	evhttp_parse_query(decode_uri, &params);
	free(decode_uri);
    
	// POST
	int buffer_data_len;
	buffer_data_len = EVBUFFER_LENGTH(req->input_buffer);
	//char *post_data;
	if (buffer_data_len) {
		char *buffer_data = (char *) malloc(buffer_data_len + 1);
		memset(buffer_data, '\0', buffer_data_len + 1);
		memcpy(buffer_data, EVBUFFER_DATA(req->input_buffer), buffer_data_len);
		//post_data = (char *) EVBUFFER_DATA(req->input_buffer);
printf("------------------------start---------------------------------------------\n");
printf("%s\n", buffer_data);
printf("------------------------end-----------------------------------------------\n");
		//if(NULL == buffer_data)
			//return;
		http_reponse(req, &params, 200, "OK");	
		// 转发给云评估系统
		size_t stat_code;
		char *uri = g_confvalue.url_path;
		stat_code = httpPostAsyn(uri, buffer_data, strlen(buffer_data)+1, NULL);	
		//logrun("http trans response code:[%d]", stat_code);
		// 缓存数据等待重发
		if (stat_code != 200) { //200:OK
#ifdef DEBUG
			printf("forward datas fail, datas to buffer.\n");
			//pthread_mutex_lock(&pthis->m_mutex);
			logbuf("%s", buffer_data);
			//pthread_mutex_unlock(&pthis->m_mutex);
#else
			logrun("forward datas fail, datas to buffer.");
			//pthread_mutex_lock(&pthis->m_mutex);
			logbuf("%s", buffer_data);
			//pthread_mutex_unlock(&pthis->m_mutex);
#endif
		}
		// 解析收到的json格式的OpenStack数据
		parse_openstack_data(arg, req, params, buffer_data);
		free(buffer_data);
	} else {
		logrun("rece post data nothing.");
		http_reponse(req, &params, 400, "ERROR");
	}
    
	return;
}

void CHttpServer::parse_openstack_data(void *arg,
		                               struct evhttp_request *req, 
		                               struct evkeyvalq params, 
									   char *post_data)
{
	CHttpServer *pthis = (CHttpServer *)arg;

	struct json_object *fst_obj = 0, *snd_obj = 0, *trd_obj = 0;
	fst_obj = json_tokener_parse(post_data);
	
	if (is_error(fst_obj)){
		logerr("parse json cloud data fail!!! data:(%s)", post_data);
		return;
	}

	char tm_buf[64];
	char g_timestamp[32]={0};
	host_data_st host_data;
	vm_data_st vm_data;
	memset((void*)&host_data, 0, sizeof(host_data));
	memset((void*)&vm_data, 0, sizeof(vm_data));

   	if (json_object_get_type(fst_obj) == json_type_array) {
		for (int i=0 ; i<json_object_array_length(fst_obj) ; i++ ){
			snd_obj = json_object_array_get_idx(fst_obj, i );
			json_object_object_foreach(snd_obj, key, val) {
				if (json_object_get_type(val) == json_type_array) {
					for (int j=0; j<json_object_array_length(val); j++) {
						trd_obj = json_object_array_get_idx(val, j);
						json_object_object_foreach(trd_obj, key, val) {
							
                            /* padding vm data info */
                            if (NULL == val) {
					        logerr("json parse data as NULL!!!");
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
						/* 打印虚拟机运行数据日志 */                    
						bzero(&tm_buf, 0);                  
						get_time_str(vm_data.timestamp, tm_buf);
								                                        
					    //pthread_mutex_lock(&pthis->m_mutex);
						logvm("%s %s %s %s %f %f %f",       
							tm_buf,                     
							vm_data.iaas_code,              
							vm_data.iaas_host_id,       								                                vm_data.iaas_vm_id,										                                    vm_data.cpu_use_rate,									                                    vm_data.memory_use_rate,
  							vm_data.io_use_rate);
						//pthread_mutex_unlock(&pthis->m_mutex);
					}
				} else { 
					/* padding host data info */
					if (NULL == val) {
						logerr("json parse host data as NULL!!!");
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
			/* 打印物理主机运行数据 */
			bzero(&tm_buf, 0);
			get_time_str(host_data.timestamp, tm_buf);

			//pthread_mutex_lock(&pthis->m_mutex);  
			loghost("%s %s %s %f %f %f",          
					tm_buf,                       
					host_data.iaas_code,              
				    host_data.iaas_host_id,           
					host_data.cpu_use_rate,           									                        host_data.memory_use_rate,        
					host_data.io_use_rate);           
			//pthread_mutex_unlock(&pthis->m_mutex);
		}

	}

	json_object_put(fst_obj);
	json_object_put(snd_obj);
	json_object_put(trd_obj);

	return;
}

void *CHttpServer::check_buffer(void *arg)
{
	CHttpServer *pthis = (CHttpServer *)arg;

	char *buflog_path = g_confvalue.log_path;
	char *uri = g_confvalue.url_path;
	
    char filename[256];
    char yyyymmdd[256];
    char pathname[256];

	while(true) {
	    bzero(yyyymmdd, sizeof(yyyymmdd));
	    get_time_now(yyyymmdd, sizeof(yyyymmdd)-1, "%Y%m%d");
	    sprintf(filename, "%s_buf_%s.log", PROGRAM_NAME, yyyymmdd);
    
	    memset(pathname, 0, sizeof(pathname));
	    sprintf(pathname, "%s", buflog_path);
	    strcat(pathname, filename);
		
        // 读取缓存数据
	    const unsigned int ret_size = get_file_size(pathname);
		if (ret_size == 0) break;
	    char buf[ret_size];
	    bzero(&buf, 0);
            
	    FILE *fp;
	    size_t stat_code;
        
	    //pthread_mutex_lock(&pthis->m_mutex);
	    fp = fopen(pathname, "r");
	    fseek(fp, SEEK_SET, 0);
		while(fgets(buf, sizeof(buf), fp)) {
#ifdef DEBUG
	    	printf("datas buffer to evaluation center.\n");
#else
			logrun("datas buffer to evaluation center.");
#endif
	   	    // 重新发给评估系统
	    	stat_code = httpPostAsyn(uri, buf, strlen(buf)+1, NULL);
	    	if (stat_code != 200) {
#ifdef DEBUG
				printf("forward datas to http server fail,stat code:[%d]", stat_code);
#else
				logrun("forward datas to http server fail,stat code:[%d]", stat_code);
#endif
	    	}	
	   }
	   fclose(fp);

	   const unsigned int file_size = get_file_size(pathname);
	   // 删除缓存文件
	   if (stat_code == 200) {
#ifdef DEBUG
			printf("delete cache file: %s\n", pathname);
#else
			logrun("delete cache file: %s", pathname);
#endif
			if (remove(pathname) < 0)
				fprintf(stderr, "remove status: %s\n", strerror(errno));
	   } 
	   //pthread_mutex_unlock(&pthis->m_mutex);
	   select_sleep(300, 0); // sleep 5minutes   
	}
	return NULL;
}

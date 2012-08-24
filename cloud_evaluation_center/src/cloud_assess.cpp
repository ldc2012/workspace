#include <mysql/mysql.h>
#include <math.h>


#include "conf.h"
#include "common.h"
#include "mysql_db.h"
#include "log.h"

extern struct conf_value_stu g_confvalue;

static const char *rate[3] = {"cpu_use_rate", "memory_use_rate", "io_use_rate"};
static const char *resource[3] = {"CPU", "MEM", "IO"};


enum run_level_type 
{
	cpu_avg2 = 0,
	cpu_weight2,
	cpu_section2,
	cpu_avg1 ,
	cpu_weight1,
	cpu_section1,
	cpu_level2,
	cpu_level1,
	cpu_level0,
	
	mem_avg2 ,
	mem_weight2,
	mem_section2,
	mem_avg1 ,
	mem_weight1,
	mem_section1,
	mem_level2,
	mem_level1,
	mem_level0,

	io_avg2 ,
	io_weight2,
	io_section2,
	io_avg1 ,
	io_weight1,
	io_section1,
	io_level2,
	io_level1,
	io_level0
};
int comup(const void *a,const void *b)
{
	//����С����˳������
	return ((float *)a)[1]-((float *)b)[1];
}
int comdown(const void *a,const void *b)
{
	printf("comdown");
	//���Ӵ�С˳������
	return ((float *)b)[1]-((float *)a)[1];
}

int avg_level(MYSQL * mysql_conn, const char * args, int host_id, time_t datatime, float * avg)
{
	logrun("avg levle %d %s", host_id, args);
	char sql[512], table_name[256];
	int count1, count2;
	float avg1, avg2;
	
	count1 = count2 = 0;
	avg1 = avg2 = 0.0;
	//��һ�ű�
	sprintf(table_name, "host_running_performance_data");
	get_table_name(table_name , 0);
	// ȡ��¼��
	sprintf(sql,"select count(%s) from %s where host_id = %d and data_time <= %ld", args, table_name, host_id, datatime);
	mysql_select_count(mysql_conn, sql, &count1);
	//ȡƽ����
	sprintf(sql,"select AVG(%s) from %s where host_id = %d and data_time <= %ld", args, table_name, host_id, datatime);
	mysql_select_count(mysql_conn, sql, &avg1);
	//ʱ��ת��
	datatime = datatime - (g_confvalue.interval * 60);
	//��һСʱ�ı�
	sprintf(table_name, "host_running_performance_data");
	get_table_name(table_name , -1);
	// ȡ��¼��
	sprintf(sql,"select count(%s) from %s where host_id = %d and data_time >= %ld", args, table_name, host_id, datatime);
	mysql_select_count(mysql_conn, sql, &count2);
	//ȡƽ����
	sprintf(sql,"select AVG(%s) from %s where host_id = %d and data_time >= %ld", args, table_name, host_id, datatime);
	mysql_select_count(mysql_conn, sql, &avg2);

	if((count1 ==0)&&(count2 == 0)){
		logerr("%d get avg level faile.",host_id);
		return -1;
	}
	if(count2 == 0)
		*avg = avg1;
	else
		*avg = ((avg1 * count1) + (avg2 * count2)) / (count1 + count2);
//	printf("host %d  %s  avg: %f\n", host_id, args, *avg);
	return 1;	
}
int weigth_level(MYSQL * mysql_conn, run_level_type runlevel, int host_id, time_t datatime, float * weigth)
{
	logrun("weight levle %d %d", host_id, runlevel);
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	char sql[512], table_name[256];
	float valve = 0.0;
	const char * args;
	int count, sum1, sum2;

	count = sum1 = sum2 = 0;
	//������ֵ
	switch(runlevel)
	{
		case cpu_weight1:
			args = rate[0];
			valve = g_confvalue.cpu_avg_1;			
			break;
		case cpu_weight2:
			args = rate[0];
			valve = g_confvalue.cpu_avg_2;
			break;
		case mem_weight1:
			args = rate[1];
			valve = g_confvalue.mem_avg_1;
			break;
		case mem_weight2:
			args = rate[1];
			valve = g_confvalue.mem_avg_2;
			break;
		case io_weight1:
			args = rate[2];
			valve = g_confvalue.io_avg_1;
			break;
		case io_weight2:
			args = rate[2];
			valve = g_confvalue.io_avg_2;
			break;	
		default:
			logerr("Run_level_type error.");
			return -1;
	}
	//��һ�ű�
	sprintf(table_name, "host_running_performance_data");
	get_table_name(table_name , 0);
	//��ȡ��¼��
	sprintf(sql,"select count(%s) from %s where host_id = %d and data_time <= %ld", args, table_name, host_id, datatime);
	mysql_select_count(mysql_conn, sql, &sum1);
	//ͳ�Ƹ���	
	sprintf(sql,"select %s from %s where host_id = %d and data_time <= %ld", args, table_name, host_id, datatime);
	if (0 == mysql_real_query(mysql_conn, sql, strlen(sql))) {
		mysql_result = mysql_store_result(mysql_conn); 
		while((mysql_row = mysql_fetch_row(mysql_result))){
			if(atof(mysql_row[0]) >=  valve)
				count += 1;			
		}
	} else 
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
	//ʱ��ת��
	datatime = datatime - (g_confvalue.interval * 60);
	//��һСʱ�ı�
	sprintf(table_name, "host_running_performance_data");
	get_table_name(table_name , -1);
	//��ȡ��¼��
	sprintf(sql,"select count(%s) from %s where host_id = %d and data_time >= %ld", args, table_name, host_id, datatime);
	mysql_select_count(mysql_conn, sql, &sum2);
	//ͳ�Ƹ���
	mysql_free_result(mysql_result);
	sprintf(sql,"select %s from %s where host_id = %d and data_time >= %ld", args, table_name, host_id, datatime);
	if (0 == mysql_real_query(mysql_conn, sql, strlen(sql))) {
		mysql_result = mysql_store_result(mysql_conn); 
		while((mysql_row = mysql_fetch_row(mysql_result))){
			if(atof(mysql_row[0]) >=  valve)
				count += 1;			
		}
	} else 
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));

	if((sum1 == 0)&&(sum2 == 0)){
		logerr("%d get weight level faile.",host_id);
//		mysql_free_result(mysql_result);
		return -1;
	}
	//����Ȩ��
	*weigth = ((float)count / (float)(sum1 + sum2)) * 100;
//	printf("host_id: %d  %s  weigth: %f\n", host_id, args, *weigth);
	mysql_free_result(mysql_result);
	return 1;
	
}

int section_level(MYSQL * mysql_conn, run_level_type runlevel, int host_id, time_t datatime, float * section)
{
	logrun("section levle %d %d", host_id, runlevel);
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	char sql[512] ,table_name[256];
	float  valve, last;
	const char * args;	
	int count, sum1, sum2;

	count = sum1 = sum2 = 0;
	valve = last =0.0;
	//������ֵ
	switch(runlevel)
	{
		case cpu_section1:
			args = rate[0];
			valve = g_confvalue.cpu_avg_1;			
			break;
		case cpu_section2:
			args = rate[0];
			valve = g_confvalue.cpu_avg_2;
			break;
		case mem_section1:
			args = rate[1];
			valve = g_confvalue.mem_avg_1;
			break;
		case mem_section2:
			args = rate[1];
			valve = g_confvalue.mem_avg_2;
			break;
		case io_section1:
			args = rate[2];
			valve = g_confvalue.io_avg_1;
			break;
		case io_section2:
			args = rate[2];
			valve = g_confvalue.io_avg_2;
			break;	
		default:
			logerr("Run_level_type error.");
			return -1;
	}
	//ʱ��ת��
	datatime = datatime - (g_confvalue.interval * 60);
	//��һСʱ�ı�
	sprintf(table_name, "host_running_performance_data");
	get_table_name(table_name , -1);
	//��ȡ��¼��
	sprintf(sql,"select count(%s) from %s where host_id = %d and data_time >= %ld", args, table_name, host_id, datatime);
//	printf("sql:%s\n",sql);
	mysql_select_count(mysql_conn, sql, &sum2);
	//ͳ���������	
	sprintf(sql,"select %s from %s where host_id = %d and data_time >= %ld", args, table_name, host_id, datatime);
	if (0 == mysql_real_query(mysql_conn, sql, strlen(sql))) {
		mysql_result = mysql_store_result(mysql_conn); 
		while((mysql_row = mysql_fetch_row(mysql_result))){
			if(atof(mysql_row[0]) >=  valve){
				if(last >= valve)
					count += 1;
			}
			last = atof(mysql_row[0]);
		}
	} else 
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
	//ʱ��ת��
	datatime = datatime + (g_confvalue.interval * 60);
	//��ǰʱ��ı�
	sprintf(table_name, "host_running_performance_data");
	get_table_name(table_name , 0);	
	//��ȡ��¼��
	sprintf(sql,"select count(%s) from %s where host_id = %d and data_time <= %ld", args, table_name, host_id, datatime);
	mysql_select_count(mysql_conn, sql, &sum1);
	//ͳ���������
	mysql_free_result(mysql_result);
	sprintf(sql,"select %s from %s where host_id = %d and data_time <= %ld", args, table_name, host_id, datatime);
	if (0 == mysql_real_query(mysql_conn, sql, strlen(sql))) {
		mysql_result = mysql_store_result(mysql_conn); 
		while((mysql_row = mysql_fetch_row(mysql_result))){
			if(atof(mysql_row[0]) >=  valve){
				if(last >= valve)
					count += 1;
			}
			last = atof(mysql_row[0]);
		}
	} else 
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
	//�����������
	if((sum1 <= 1)&&(sum2 <= 1)){
		logerr("%d get section level faile.",host_id);
		mysql_free_result(mysql_result);
		return -1;
	}		
	*section = ((float)count / (float)(sum1 + sum2 - 1)) * 100;
//	printf("host_id: %d  %s  section: %f\n", host_id, args, *section);
	mysql_free_result(mysql_result);
	return 1;
}
int insert_run_level(MYSQL *mysql_conn, int hostid, run_level_type run )
{
	//vm_info vm_running_performance_data vm_running_level_data
	int domain_num,run_level;
	char sql[512], table_name[256],time_str[64];
	time_t datatime;

	const char * cloud_rate,* cloud_resource;
	float avg1, avg2;
	int count1, count2;
	printf("insert run level  %d,%d\n",hostid,run);
	datatime = time(NULL);
	gmtime(&datatime);

	//������ֵ
	switch(run)
	{
		case cpu_level2:
			cloud_rate = rate[0];
			cloud_resource = resource[0];
			run_level = 2;
			break;
		case cpu_level1:
			cloud_rate = rate[0];
			cloud_resource = resource[0];
			run_level = 1;
			break;
		case cpu_level0:
			cloud_rate = rate[0];
			cloud_resource = resource[0];
			run_level = 0;
			break;
		case mem_level2:
			cloud_rate = rate[1];
			cloud_resource = resource[1];
			run_level = 2;
			break;
		case mem_level1:
			cloud_rate = rate[1];
			cloud_resource = resource[1];
			run_level = 1;
			break;
		case mem_level0:
			cloud_rate = rate[1];
			cloud_resource = resource[1];
			run_level = 0;
			break;
		case io_level2:
			cloud_rate = rate[2];
			cloud_resource = resource[2];
			run_level = 2;
			break;
		case io_level1:
			cloud_rate = rate[2];
			cloud_resource = resource[2];
			run_level = 1;
			break;
		case io_level0:
			cloud_rate = rate[2];
			cloud_resource = resource[2];
			run_level = 0;
			break;
		default:
			break;
	}
	//ȡ���������
	sprintf(sql,"select count(*) from vm_info where host_id = %d ", hostid);
//	printf("sql: %s \n",sql);
	if(-1 == mysql_select_count(mysql_conn, sql, &domain_num)){
		logerr("get 0 record: %s\n",sql);
		return -1;
	}
//	printf("domain numbers: %d \n",domain_num);
	//ȡ�����ID
//	float domain_info[domain_num][2];
	if(0 == domain_num){
		logerr("get 0 record: %s\n",sql);
		return -1;
	}
	printf("malloc %d\n",domain_num);
	float **domain_info;
	domain_info = (float **)malloc(sizeof(float *) * domain_num);

	for(int i = 0; i < domain_num ; ++i)
		domain_info[i] = (float *)malloc(sizeof(float) * 2);
	
	sprintf(sql,"select vm_id from vm_info where host_id = %d ", hostid);
	if(-1 == mysql_select_array(mysql_conn, sql, domain_info, 0)){
		logerr("get 0 record: %s\n",sql);
		for(int i = 0; i < domain_num ; i++)
			free(domain_info[i]);
		free(domain_info);
		logrun("domain_info %d free", hostid);
		return -1;
	}
	//ȡ���������ƽ��ֵ
	for(int i = 0; i < domain_num; ++i)
	{	
		printf("______________________________________\n");
		printf("get avg %f\n",domain_info[i][0]);
		count1 = count2 = 0 ;
		avg1 = avg2 = 0.0 ; 
		//��ǰʱ��ı�
		sprintf(table_name, "vm_running_performance_data");
		get_table_name(table_name , 0);	
		sprintf(sql,"select count(%s) from %s where vm_id = %f and data_time <= %ld", cloud_rate, table_name, domain_info[i][0], datatime);
		mysql_select_count(mysql_conn, sql, &count1);
		sprintf(sql,"select AVG(%s) from %s where vm_id = %f and data_time <= %ld", cloud_rate, table_name, domain_info[i][0], datatime);
		mysql_select_count(mysql_conn, sql, &avg1);

		//ʱ��ת��
		datatime = datatime - (g_confvalue.interval * 60);
		//��һСʱ�ı�
		sprintf(table_name, "vm_running_performance_data");
		get_table_name(table_name , -1);
		sprintf(sql,"select count(%s) from %s where vm_id = %f and data_time >= %ld", cloud_rate, table_name, domain_info[i][0], datatime);
		mysql_select_count(mysql_conn, sql, &count2);
		sprintf(sql,"select AVG(%s) from %s where vm_id = %f and data_time >= %ld", cloud_rate, table_name, domain_info[i][0], datatime);
		mysql_select_count(mysql_conn, sql, &avg2);

		if((count1 ==0)&&(count2 == 0))
			logerr("vm %f get avg faile:\n", domain_info[i][0]);
		else
			domain_info[i][1] = ((avg1 * count1) + (avg2 * count2)) / (float)(count1 + count2);
		//ʱ��ת��
		datatime = datatime + (g_confvalue.interval * 60);
		printf("%f avg: %f \n",domain_info[i][0],domain_info[i][1]);
	}
	printf("qsort\n");
	qsort(domain_info, domain_num, sizeof(float)*2, comdown);
	for(int i = 0; i < domain_num; i++)
	{
		printf("insert %f\n",domain_info[i][0]);
		//��ǰʱ��ı�
		sprintf(table_name, "vm_running_level_data");
		get_table_name(table_name , 0);		
	
		sprintf(sql,"INSERT INTO %s (data_time, vm_id, running_level, lack_resource) VALUES ('%ld', '%f', '%d', '%s')", table_name, datatime, domain_info[i][0], run_level, cloud_resource);
		printf("sql:%s\n",sql);
		if(0 >= mysql_exec_single(mysql_conn, sql))
			logerr("DB INSERT FAILE: %s\n",sql);
		//��¼��־
//		bzero(&time_str, 0);
		strftime(time_str, 64, "%Y-%m-%d %H:%M:%S", localtime(&datatime));
//		printf("%d, %d, %s, %s;\n",(int)domain_info[i][0], run_level, resource, time_str);
		logrunlevel((int)domain_info[i][0], run_level, cloud_resource, time_str);
		if(run_level > 0)
			run_level--;
	}
	for(int i = 0; i < domain_num ; i++)
		free(domain_info[i]);
	free(domain_info);
	logrun("domain_info %d free", hostid);
	return 0;
}



//��̭�㷨
int eliminate(float **p, int * remain_number, float dif, int sort)
{
	float last;
	
	if(sort ==1)
		//���Ӵ�С˳�����ж�ά����
		qsort(p, *remain_number, sizeof(float)*2, comdown);
	else
		//����С����˳�����ж�ά����
		qsort(p, *remain_number, sizeof(float)*2, comup);
/*	printf("new order:\n");
	for(int j = 0; j < *remain_number; j++){
		printf("%f %f \n", p[j][0], p[j][1]);
	}
*/	//�жϲ�ֵ
	last = p[0][1];
	for(int j = 1; j < *remain_number; j++){
		if((p[j][1] - last) >=  dif){
			*remain_number = j;
			return 1;
		}
		last = p[j][1];
	}
	return 0;
}

//��ֵ�㷨
int valve_value(float **p, int * remain_number, float dif, int sort)
{
	if(sort ==1)
		//���Ӵ�С˳�����ж�ά����
		qsort(p, *remain_number, sizeof(float)*2, comdown);
	else
		//����С����˳�����ж�ά����
		qsort(p, *remain_number, sizeof(float)*2, comup);
/*	printf("new order:\n");
	for(int j = 0; j < *remain_number; j++){
		printf("%f %f \n", p[j][0], p[j][1]);
	}
*/	//�жϷ�ֵ
	for(int j = 1; j < *remain_number; j++){
		if(p[j][1] >=  dif){
			*remain_number = j;
			return 1;
		}
	}
	return 0;
}

int get_variance(MYSQL *my, float **arr, int remain_number, const char * args, time_t datatime)
{
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	char sql[512], table_name[256] ;
	float variance_sum, avg, avg1, avg2;
	int count1, count2;
	//���������ƻ���
	for(int j = 0; j < remain_number; j++){						
		variance_sum = avg = avg1 = avg2 = 0.0;
		count1 = count2 = 0;
		//��һ�ű�
		sprintf(table_name, "host_running_performance_data");
		get_table_name(table_name , 0);	
		//ȡ����
		sprintf(sql,"select count(%s) from %s where iaas_id = %f and data_time <= %ld", args, table_name, arr[j][0], datatime);
		mysql_select_count(my, sql, &count1);
		//ȡƽ��ֵ
		sprintf(sql,"select AVG(%s) from %s where iaas_id = %f and data_time <= %ld", args, table_name, arr[j][0], datatime);
		mysql_select_count(my, sql, &avg1);


		//ʱ��ת��
		datatime = datatime - (g_confvalue.interval * 60);
		//��һСʱ�ı�
		sprintf(table_name, "host_running_performance_data");
		get_table_name(table_name , -1);
		//ȡ����
		sprintf(sql,"select count(%s) from %s where iaas_id = %f and data_time >= %ld", args, table_name, arr[j][0], datatime);
		mysql_select_count(my, sql, &count2);
		//ȡƽ��ֵ
		sprintf(sql,"select AVG(%s) from %s where iaas_id = %f and data_time >= %ld", args, table_name, arr[j][0], datatime);
		mysql_select_count(my, sql, &avg2);
	
		if((count1 == 0)&&(count2 == 0)){
			logerr("get avg faile: %s",sql);
			return -1;
		}
		else
			avg = ((avg1 * count1) + (avg2 *count2)) / (count1 + count2);
		//ʱ��ת��
		datatime = datatime + (g_confvalue.interval * 60);

		
		//��һ�ű�
		sprintf(table_name, "host_running_performance_data");
		get_table_name(table_name , 0);		
		//�ۼ�
		sprintf(sql,"select %s from %s where iaas_id = %f and data_time <= %ld ", args, table_name, arr[j][0], datatime);
		if (0 == mysql_real_query(my, sql, strlen(sql))){
			mysql_result = mysql_store_result(my);
			while((mysql_row = mysql_fetch_row(mysql_result))){
				variance_sum += (atof(mysql_row[0]) - avg)*(atof(mysql_row[0]) - avg);								
			}
		}else
			logerr("mysql_real_query fail: %s", mysql_error(my));
		//ʱ��ת��
		datatime = datatime - (g_confvalue.interval * 60);
		//��һСʱ�ı�
		sprintf(table_name, "host_running_performance_data");
		get_table_name(table_name , -1);
		//�ۼ�
		sprintf(sql,"select %s from %s where iaas_id = %f and data_time >= %ld ", args, table_name, arr[j][0], datatime);
		if (0 == mysql_real_query(my, sql, strlen(sql))){
			mysql_result = mysql_store_result(my);
			while((mysql_row = mysql_fetch_row(mysql_result))){
				variance_sum += (atof(mysql_row[0]) - avg)*(atof(mysql_row[0]) - avg);								
			}
		}else
			logerr("mysql_real_query fail: %s", mysql_error(my));		
	
		//��׼��
		arr[j][1] = sqrt(variance_sum / (float)(count1 + count2));
		//ʱ��ת��
		datatime = datatime + (g_confvalue.interval * 60);
	}	
	mysql_free_result(mysql_result);
	return 1;
}


int get_avg(MYSQL *my, float **arr, int remain_number, const char * args, time_t  datatime)
{
	char sql[512], table_name[256];
	float avg1, avg2;
	int count1, count2;
	for(int j = 0; j < remain_number; j++){
		avg1 = avg2 = 0.0;
		count1 = count2 =0;
		//��һ�ű�
		sprintf(table_name, "host_running_performance_data");
		get_table_name(table_name , 0);	
		//ȡ����
		sprintf(sql,"select count(%s) from %s where iaas_id = %f and data_time <= %ld", args, table_name, arr[j][0], datatime);
		mysql_select_count(my, sql, &count1);
		//ȡƽ��ֵ
		sprintf(sql,"select AVG(%s) from %s where iaas_id = %f and data_time <= %ld", args, table_name, arr[j][0], datatime);
		mysql_select_count(my, sql, &avg1);
		
		//ʱ��ת��
		datatime = datatime - (g_confvalue.interval * 60);
		//��һСʱ�ı�
		sprintf(table_name, "host_running_performance_data");
		get_table_name(table_name , -1);
		//ȡ����
		sprintf(sql,"select count(%s) from %s where iaas_id = %f and data_time >= %ld", args, table_name, arr[j][0], datatime);
		mysql_select_count(my, sql, &count2);
		//ȡƽ��ֵ
		sprintf(sql,"select AVG(%s) from %s where iaas_id = %f and data_time >= %ld", args, table_name, arr[j][0], datatime);
		mysql_select_count(my, sql, &avg2);
		
//		printf("sql:%s\n", sql);
		if((count1 == 0)&&(count2 == 0))
			logerr("get avg array faile: %s",sql);
		else		
			arr[j][1] = ((avg1 * count1) + (avg2 * count2)) / (float)(count1 + count2);
		//ʱ��ת��
		datatime = datatime + (g_confvalue.interval * 60);
			
	}
	return 1;
}
int get_max(MYSQL *my, float **arr, int remain_number, const char * args, time_t  datatime)
{
	char sql[512], table_name[256];
	float max ;
	for(int j = 0; j < remain_number; j++){
		max = 0;
		//��һ�ű�
		sprintf(table_name, "host_running_performance_data");
		get_table_name(table_name, 0);	
		sprintf(sql,"select MAX(%s) from %s where iaas_id = %f and data_time <= %ld", args, table_name, arr[j][0], datatime);
		if(1 == (mysql_select_count(my, sql, &max)))
			arr[j][1] = max;	

		//ʱ��ת��
		datatime = datatime - (g_confvalue.interval * 60);
		//��һСʱ�ı�
		sprintf(table_name, "host_running_performance_data");
		get_table_name(table_name , -1);
		sprintf(sql,"select MAX(%s) from %s where iaas_id = %f and data_time >= %ld", args, table_name, arr[j][0], datatime);
		if(1 == (mysql_select_count(my, sql, &max))){
			if(max > arr[j][1])
				arr[j][1] = max;
		}		
		//ʱ��ת��
		datatime = datatime + (g_confvalue.interval * 60);			
	}
	return 1;
}
void level(MYSQL *my)
{
	char sql[512];
	int cloud_number, host_number;;
	time_t datatime;
	int *host_id,*iaas_id;
	float avg, weigth, section;
	bool finish;
	run_level_type runleveltype;

	datatime = time(NULL);
	gmtime(&datatime);
	//ȡ�ƻ�������
	sprintf(sql,"select count(*) from iaas_info");
	if(-1 == mysql_select_count(my, sql, &cloud_number)){
		return;
	}
	//��ȡ�ƻ���ID

	iaas_id = (int *)malloc(sizeof(int) * cloud_number);
	sprintf(sql,"select iaas_id from iaas_info");
	if(-1 == mysql_select_array(my, sql, iaas_id)){
		logerr("get iaas_id filed");
		free(iaas_id);
		logrun("iaas_id free");
		return;
	}	
	//�����ƻ���
	for(int i = 0; i < cloud_number; i++)
	{		
		//��ȡ���������
		sprintf(sql,"select count(*) from host_info where iaas_id = %d", iaas_id[i]);
		if(-1 == mysql_select_count(my, sql, &host_number)){
			logerr("get host_number filed");
			free(iaas_id);
			logrun("iaas_id free get node count faile");
			return;
		}		
//			printf("host_number: %d\n", host_number);
		//��ȡ�����ID

		host_id = (int *)malloc(sizeof(int) * host_number);
		sprintf(sql,"select host_id from host_info where iaas_id = %d", iaas_id[i]);
		if(-1 == mysql_select_array(my, sql, host_id)){
			logerr("get host_id filed");
			free(iaas_id);
			logrun("iaas_id free get node id falie");						
			free(host_id);
			logrun("host_id free runlevel success");
			return;
		}
		//���������
		for(int j = 0; j < host_number; j++)
		{
			finish = false;
			runleveltype = cpu_avg2;
//				printf("start cmp\n");

			while(!finish)
			{
//					printf("%d\n", runleveltype);
				avg = weigth = section = 0.0;
				switch (runleveltype)
				{
					//cpu
					case cpu_avg2:							
						if(-1 == avg_level(my,rate[0],host_id[j],datatime,&avg))
							logerr("get avg faild");
						else if (avg >= g_confvalue.cpu_avg_2){ 						
							runleveltype = cpu_level2;
							break;
						}
						runleveltype = cpu_weight2;
						break;
					case cpu_weight2:
						if(-1 == weigth_level(my, runleveltype, host_id[j], datatime, &weigth))
							logerr("get weigth faild");
						else if (weigth >= g_confvalue.cpu_weight_2){
							runleveltype = cpu_level2;
							break;
						}					
						runleveltype = cpu_section2;
						break;
					case cpu_section2:
						if(-1 == section_level(my, runleveltype, host_id[j], datatime, &section))
							logerr("get section faild");
						else if (section >= g_confvalue.cpu_section_2){
							runleveltype = cpu_level2;
							break;
						}					
						runleveltype = cpu_avg1;
						break;
					case cpu_avg1:
						if(-1 == avg_level(my,rate[0],host_id[j],datatime,&avg))
							logerr("get avg faild");
						else if (avg >= g_confvalue.cpu_avg_1){ 						
							runleveltype = cpu_level1;
							break;
						}
						runleveltype = cpu_weight1;
						break;						
					case cpu_weight1:
						if(-1 == weigth_level(my, runleveltype, host_id[j], datatime, &weigth))
							logerr("get weigth faild");
						else if (weigth >= g_confvalue.cpu_weight_1){
							runleveltype = cpu_level1;
							break;
						}					
						runleveltype = cpu_section1;
						break;
					case cpu_section1:
						if(-1 == section_level(my, runleveltype, host_id[j], datatime, &section))
							logerr("get section faild");
						else if (section >= g_confvalue.cpu_section_1){
							runleveltype = cpu_level1;
							break;
						}					
						runleveltype = cpu_level0;
						break;
					case cpu_level2:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						runleveltype = mem_avg2;
						break;
					case cpu_level1:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						runleveltype = mem_avg2;
						break;
					case cpu_level0:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						runleveltype = mem_avg2;
						break;

					//mem
					case mem_avg2:
						if(-1 == avg_level(my,rate[1],host_id[j],datatime,&avg))
							logerr("get avg faild");
						else if (avg >= g_confvalue.mem_avg_2){ 						
							runleveltype = mem_level2;
							break;
						}
						runleveltype = mem_weight2;
						break;				
					case mem_weight2:
						if(-1 == weigth_level(my, runleveltype, host_id[j], datatime, &weigth))
							logerr("get weigth faild");
						else if (weigth >= g_confvalue.mem_weight_2){
							runleveltype = mem_level2;
							break;
						}					
						runleveltype = mem_section2;
						break;
					case mem_section2:
						if(-1 == section_level(my, runleveltype, host_id[j], datatime, &section))
							logerr("get section faild");
						else if (section >= g_confvalue.mem_section_2){
							runleveltype = mem_level2;
							break;
						}					
						runleveltype = mem_avg1;
						break;
					case mem_avg1:
						if(-1 == avg_level(my,rate[1],host_id[j],datatime,&avg))
							logerr("get avg faild");
						else if (avg >= g_confvalue.mem_avg_1){ 						
							runleveltype = mem_level1;
							break;
						}
						runleveltype = mem_weight1;
						break;								
					case mem_weight1:
						if(-1 == weigth_level(my, runleveltype, host_id[j], datatime, &weigth))
							logerr("get weigth faild");
						else if (weigth >= g_confvalue.mem_weight_1){
							runleveltype = mem_level1;
							break;
						}					
						runleveltype = mem_section1;
						break;
					case mem_section1:
						if(-1 == section_level(my, runleveltype, host_id[j], datatime, &section))
							logerr("get section faild");
						else if (section >= g_confvalue.mem_section_1){
							runleveltype = mem_level1;
							break;
						}					
						runleveltype = mem_level0;
						break;
					case mem_level2:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						runleveltype = io_avg2;
						break;
					case mem_level1:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						runleveltype = io_avg2;
						break;
					case mem_level0:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						runleveltype = io_avg2;
						break;

					//io
					case io_avg2:
						if(-1 == avg_level(my,rate[2],host_id[j],datatime,&avg))
							logerr("get avg faild");
						else if (avg >= g_confvalue.io_avg_2){							
							runleveltype = io_level2;
							break;
						}
						runleveltype = io_weight2;
						break;
					case io_weight2:
						if(-1 == weigth_level(my, runleveltype, host_id[j], datatime, &weigth))
							logerr("get weigth faild");
						else if (weigth >= g_confvalue.io_weight_2){
							runleveltype = io_level2;
							break;
						}					
						runleveltype = io_section2;
						break;
					case io_section2:
						if(-1 == section_level(my, runleveltype, host_id[j], datatime, &section))
							logerr("get section faild");
						else if (section >= g_confvalue.io_section_2){
							runleveltype = io_level2;
							break;
						}					
						runleveltype = io_avg1;
						break;
					case io_avg1:
						if(-1 == avg_level(my,rate[2],host_id[j],datatime,&avg))
							logerr("get avg faild");
						else if (avg >= g_confvalue.io_avg_1){							
							runleveltype = io_level1;
							break;
						}
						runleveltype = io_weight1;
						break;
					case io_weight1:
						if(-1 == weigth_level(my, runleveltype, host_id[j], datatime, &weigth))
							logerr("get weigth faild");
						else if (weigth >= g_confvalue.io_weight_1){
							runleveltype = io_level1;
							break;
						}					
						runleveltype = io_section1;
						break;
					case io_section1:
						if(-1 == section_level(my, runleveltype, host_id[j], datatime, &section))
							logerr("get section faild");
						else if (section >= g_confvalue.io_section_1){
							runleveltype = io_level1;
							break;
						}					
						runleveltype = io_level0;
						break;
					case io_level2:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))								
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						finish = true;
						break;
					case io_level1:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))							
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						finish = true;
						break;
					case io_level0:
						if(-1 == insert_run_level(my,host_id[j], runleveltype))
							logerr("insert filed:host_id %d run_level %d datatime %ld\n", host_id[j], runleveltype, datatime);
						finish = true;
						break;
					
					default:
						logerr("Run_level_type error.");
						break;
				}
			}
		}
		free(host_id);
		logrun("host_id free runlevel success");	
	}
	free(iaas_id);
	logrun("iaas_id free runlevel success");
	return;
	
}
void *run_level(void *args)
{	
	MYSQL my;	

	while(1)
	{
		

		//�������ݿ�
		
		if (NULL == mysql_conn(&my, g_confvalue.db_server, g_confvalue.db_user, 
			g_confvalue.db_passwd, g_confvalue.db_name, g_confvalue.db_port)) { 
		logerr("connect mysql fail, db_server: %s, db_user: %s, db_passwd: %s, db_name: %s, db_port: %u",
				g_confvalue.db_server, g_confvalue.db_user,
				g_confvalue.db_passwd, g_confvalue.db_name, g_confvalue.db_port);
			mysql_close(&my);
			mysql_library_end();
			sleep(g_confvalue.db_retry);
			continue;
			}

		level(&my);
		mysql_close(&my);
		//mysql_library_end();
		sleep(g_confvalue.run_interval * 60);
	}	
	
}
void assess(MYSQL *my)
{
	rule rule_type;
	int remain_number, cloud_number, host_number, domain_vcpu;;
	char sql[512],time_str[64];
	time_t datatime;
	float **iaas_info;

	datatime = time(NULL);
	gmtime(&datatime);
//		printf("datatime: %ld", datatime);
	remain_number = g_confvalue.rule_num;
		//ȡ�ƻ�������
	sprintf(sql,"select count(*) from iaas_info");
	if(-1 == mysql_select_count(my, sql, &cloud_number)){
		logerr("get cloud_number filed");
		return;
	}
	if(cloud_number == 0){
		logerr("there is no iaas in iass_info");
		return;
	}
	remain_number = cloud_number;
//		printf("cloud_number: %d\n", cloud_number);
	//ȡ�����ƻ���id
//		float (*iaas_info)[2];
//		iaas_info = (float (*)[2])malloc((sizeof(float) * 2) * cloud_number);
	iaas_info = (float **)malloc(sizeof(float *) * cloud_number);

	for(int i = 0; i < cloud_number ; i++)
		iaas_info[i] = (float *)malloc(sizeof(float) * 2);
	
	sprintf(sql,"select iaas_id from iaas_info");
	mysql_select_array(my, sql, iaas_info, 0);
	//����ƥ�����
	for(int i = 0; i < g_confvalue.rule_num; i++){
		rule_type = (rule)g_confvalue.rule_clause[i];
			printf("rule_type: %d  remain_number: %d\n", rule_type, remain_number);
		if(remain_number <= 1)
			break;
		switch(rule_type)
		{
			case CPU_OVERLOAD:
//					printf("CPU_OVERLOAD \n");
				//ȡ�����ƻ���CPUƽ��ֵ
				if(-1 != get_avg(my, iaas_info, remain_number, rate[0],datatime))
				//��ֵ�ж�
					valve_value(iaas_info, &remain_number, g_confvalue.cpu_avg_2, 0);
				else 
					logerr("get AVG(cpu_use_rate) of iaas filed.");
				break;
			case MEM_OVERLOAD:
//					printf("MEM_OVERLOAD \n");	
				//ȡ�����ƻ���MEMƽ��ֵ
				if(-1 != get_avg(my, iaas_info, remain_number, rate[1],datatime))
				//��ֵ�ж�
					valve_value(iaas_info, &remain_number, g_confvalue.mem_avg_2, 0);
				else 
					logerr("get AVG(memory_use_rate) of iaas filed.");
				break;
			case IO_OVERLOAD:
//					printf("IO_OVERLOAD \n");
				//ȡ�����ƻ���IOƽ��ֵ
				if(-1 != get_avg(my, iaas_info, remain_number, rate[2],datatime))
				//��ֵ�ж�
					valve_value(iaas_info, &remain_number, g_confvalue.io_avg_2, 0);
				else 
					logerr("get AVG(io_use_rate) of iaas filed.");
				break;
			case CPU_AGE:
//					printf("CPU_AGE \n");
				//ȡ�����ƻ���CPUƽ��ֵ
				if(-1 != get_avg(my, iaas_info, remain_number, rate[0],datatime))
				//��̭�㷨
					eliminate(iaas_info, &remain_number,g_confvalue.avg_dif.cpu, 0);
				else 
					logerr("get AVG(cpu_use_rate) of iaas filed.");
				break;
			case MEM_AGE:
//					printf("MEM_AGE \n");
				//ȡ�����ƻ���MEMƽ��ֵ
				if(-1 != get_avg(my, iaas_info, remain_number, rate[1],datatime))
				//��̭�㷨
					eliminate(iaas_info, &remain_number,g_confvalue.avg_dif.mem, 0);
				else 
					logerr("get AVG(memory_use_rate) of iaas filed.");
				break;
			case IO_AGE:
//					printf("IO_AGE \n");
				//ȡ�����ƻ���IOƽ��ֵ
				if(-1 != get_avg(my, iaas_info, remain_number, rate[2],datatime))
				//��̭�㷨
					eliminate(iaas_info, &remain_number,g_confvalue.avg_dif.io, 0);
				else 
					logerr("get AVG(io_use_rate) of iaas filed.");
				break;
			case CPU_ASSIGN:
//					printf("CPU_ASSIGN \n");
				//���������ƻ���
				if(-1 != get_variance(my, iaas_info, remain_number, rate[0], datatime))
				//��̭�㷨
					eliminate(iaas_info, &remain_number,g_confvalue.assign_dif.cpu, 0);
				else 
					logerr("get CPU variance of iaas filed.");
				break;
			case MEM_ASSIGN:
//					printf("MEM_ASSIGN \n");
				//ȡ�ڴ�ı�׼��
				if(-1 != get_variance(my, iaas_info, remain_number, rate[1], datatime))
				//��̭�㷨
					eliminate(iaas_info, &remain_number,g_confvalue.assign_dif.mem, 0);
				else 
					logerr("get money variance of iaas filed.");
				break;
			case IO_ASSIGN:
//					printf("IO_ASSIGN \n");
				//ȡIO�ı�׼��
				if(-1 != get_variance(my, iaas_info, remain_number, rate[2], datatime))
				//��̭�㷨
					eliminate(iaas_info, &remain_number,g_confvalue.assign_dif.io, 0);
				else 
					logerr("get IO variance of iaas filed.");
				break;
			case DOMAIN_SELL:
//					printf("DOMAIN_SELL \n");
				// �����ƻ���
				for(int j = 0; j < remain_number; j++){
					
					//ȡ���ƻ������������
					sprintf(sql,"select count(*) from host_info where iaas_id = %f", iaas_info[j][0]);
					if(-1 == mysql_select_count(my, sql, &host_number)){
						logerr("get host count of iaas failed: %s",sql);
						break;
					}
					//ȥ���ƻ����������������
					sprintf(sql,"select SUM(cpu_core_number) from vm_info where iaas_id = %f", iaas_info[j][0]);
					if(-1 == mysql_select_count(my, sql, &domain_vcpu)){
						logerr("get v_cpu count of iaas failed: %s",sql);
						break;
					}
					iaas_info[j][1] = (float)((float)domain_vcpu / (float)host_number) * 100;				
				}
				//��̭�㷨
				eliminate(iaas_info, &remain_number,g_confvalue.domain_sell_dif, 0);
				break;
			case CPU_AVAILABLE:
				break;
			case MEM_AVAILABLE:
				break;
			case IO_AVAILABLE:
				break;
			case CPU_MAX:
//					printf("CPU_MAX \n");
				// �����ƻ���CPU���ֵ
				if(-1 != get_max(my, iaas_info, remain_number, rate[0],datatime))	
				//��̭�㷨
					eliminate(iaas_info, &remain_number, g_confvalue.max_dif.cpu, 0);
				else 
					logerr("get MAX(cpu_use_rate) of iaas filed.");
				break;
			case MEM_MAX:
//					printf("MEM_MAX \n");
				// �����ƻ���MEM���ֵ
				if(-1 != get_max(my, iaas_info, remain_number, rate[1],datatime))
				//��̭�㷨
					eliminate(iaas_info, &remain_number,g_confvalue.max_dif.mem, 0);
				else 
					logerr("get MAX(memory_use_rate) of iaas filed.");
				break;
			case IO_MAX:
//					printf("IO_MAX \n");
				// �����ƻ���IO���ֵ
				if(-1 != get_max(my, iaas_info, remain_number, rate[2],datatime))
				//��̭�㷨
					eliminate(iaas_info, &remain_number,g_confvalue.max_dif.io, 0);	
				else 
					logerr("get MAX(io_use_rate) of iaas filed.");
				break;
			default:
				logerr("rule_type error");
				break;					
		}
	}
	//����������ݿ�
	sprintf(sql,"INSERT INTO iaas_performance_evaluation_data (iaas_id, data_time) VALUES ('%f', '%ld')", iaas_info[0][0], datatime);
	if(0 >= mysql_exec_single(my, sql))
		logerr("DB INSERT FAILE: %s\n",sql);
	//��¼��־
//		bzero(&time_str, 0);
	strftime(time_str, 64, "%Y-%m-%d %H:%M:%S", localtime(&datatime));
	logcloud((int)iaas_info[0][0], time_str);
	for(int i = 0; i < cloud_number ; i++)
		free((void *)iaas_info[i]);
	free((void *)iaas_info);
	logrun("iaas_info free assess success");
	return;
}
void *cloud_assess(void *args)
{
	//iaas_info //host_running_performance_data //host_info//vm_info//iaas_performance_evaluation_data
	MYSQL my;
		
	while(1)
	{
		//�������ݿ�
		if (NULL == mysql_conn(&my, g_confvalue.db_server, g_confvalue.db_user, 
				g_confvalue.db_passwd, g_confvalue.db_name, g_confvalue.db_port)) {	
			logerr("connect mysql fail, db_server: %s, db_user: %s, db_passwd: %s, db_name: %s, db_port: %u",
					g_confvalue.db_server, g_confvalue.db_user,
					g_confvalue.db_passwd, g_confvalue.db_name, g_confvalue.db_port);
			mysql_close(&my);
			mysql_library_end();
			sleep(g_confvalue.db_retry);
			continue;
		}
		assess(&my);
		mysql_close(&my);
		//mysql_library_end();
		sleep(g_confvalue.run_interval * 60);
	}
	
}
 



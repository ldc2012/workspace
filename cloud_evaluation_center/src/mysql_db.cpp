#include <stdlib.h>

#include "mysql_db.h"
#include "log.h"


MYSQL *mysql_conn(MYSQL *mysql_handle, const char *db_server, const char *db_user, 
		const char *db_passwd, const char *db_name, const unsigned short db_port) 
{
	mysql_server_init(0,NULL,NULL);
	mysql_init(mysql_handle);
	if (!mysql_real_connect(mysql_handle, db_server, db_user, db_passwd, 
				db_name, db_port, NULL, 0)) {
		logerr("failed to connect to mysql database: %s\n", mysql_error(mysql_handle));
		return NULL;
	}
	return mysql_handle;
}

int mysql_exec_single(MYSQL *mysql_conn, const char *sqlstr)
{
	if (0 != mysql_real_query(mysql_conn, sqlstr, strlen(sqlstr))) {
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
		return -1;
	}
	int i = mysql_affected_rows(mysql_conn);
	
	return i;
}

int mysql_select_count(MYSQL *mysql_conn, const char *sqlstr, int * count) 
{
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	if (0 == mysql_real_query(mysql_conn, sqlstr, strlen(sqlstr))) {
		mysql_result = mysql_store_result(mysql_conn);
		mysql_row = mysql_fetch_row(mysql_result); 
		if(NULL == mysql_row[0]){					
			//logerr("get 0 record: %s", mysql_error(mysql_conn));
			mysql_free_result(mysql_result);
			return -1;
		}
		*count = atoi(mysql_row[0]);
	} else {
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
		mysql_free_result(mysql_result);
		return -1;
	}
	mysql_free_result(mysql_result);
	
	return 1;

}
int mysql_select_count(MYSQL *mysql_conn, const char *sqlstr, float * count) 
{
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	if (0 == mysql_real_query(mysql_conn, sqlstr, strlen(sqlstr))) {
		mysql_result = mysql_store_result(mysql_conn);
		mysql_row = mysql_fetch_row(mysql_result); 
		if(NULL == mysql_row[0]){		
			//logerr("get 0 record: %s", mysql_error(mysql_conn));
			mysql_free_result(mysql_result);
			return -1;
		}
		*count = atof(mysql_row[0]);
	} else {
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
		mysql_free_result(mysql_result);
		return -1;
	}
	mysql_free_result(mysql_result);

	return 1;

}
int mysql_exec_table(MYSQL *mysql_conn, const char *sqlstr, list<vector<string> > &list_table) 
{
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;   // lxl 2011
	int num_row, num_col;
	vector<string> vec_row;

	if (0 == mysql_real_query(mysql_conn, sqlstr, strlen(sqlstr))) {
		mysql_result = mysql_store_result(mysql_conn);
		num_row = mysql_num_rows(mysql_result);
		num_col = mysql_num_fields(mysql_result);
		//logrun("select row %d, col %d\n", num_row, num_col);
		for (int i = 0; i < num_row; ++i) {
			mysql_row = mysql_fetch_row(mysql_result);
			for (int j = 0; j < num_col; ++j) {
				if (NULL != mysql_row[j])
				vec_row.push_back(mysql_row[j]);
			}
			list_table.push_back(vec_row);
			vec_row.clear();
		}
	} else {
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
		mysql_free_result(mysql_result);
		return -1;
	}
	mysql_free_result(mysql_result);

	return (int)list_table.size();
}

int mysql_select_array(MYSQL * mysql_conn, const char * sqlstr, float **arr, int num)
{
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	int i = 0;
	if (0 == mysql_real_query(mysql_conn, sqlstr, strlen(sqlstr))) {
		mysql_result = mysql_store_result(mysql_conn); 
		while((mysql_row = mysql_fetch_row(mysql_result))){
			if(NULL == mysql_row[0]){		
				//logerr("get 0 row: %s", mysql_error(mysql_conn));
				mysql_free_result(mysql_result);
				return -1;
			}
			arr[i][num] = atof(mysql_row[0]);
			i++;
		}
	} else {
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
		mysql_free_result(mysql_result);
		return -1;
	}
	mysql_free_result(mysql_result);

	return 1;
}
int mysql_select_array(MYSQL * mysql_conn, const char * sqlstr, int (*arr))
{
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	int i = 0;
	if (0 == mysql_real_query(mysql_conn, sqlstr, strlen(sqlstr))) {
		mysql_result = mysql_store_result(mysql_conn); 
		while((mysql_row = mysql_fetch_row(mysql_result))){
			if(NULL == mysql_row[0]){		
				//logerr("get 0 row: %s", mysql_error(mysql_conn));
				mysql_free_result(mysql_result);
				return -1;
			}
			arr[i] = atoi(mysql_row[0]);
			i++;
		}
	} else {
		logerr("mysql_real_query fail: %s", mysql_error(mysql_conn));
		mysql_free_result(mysql_result);
		return -1;
	}
	mysql_free_result(mysql_result);

	return 1;
}

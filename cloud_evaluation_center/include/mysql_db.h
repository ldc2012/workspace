#ifndef MYSQL_DB_H_
#define MYSQL_DB_H_

#include <string.h>

#include <vector>
#include <list>
#include <string>

#include <mysql/mysql.h>

using namespace std;

/* mysql connect  */
MYSQL *mysql_conn(MYSQL *mysql_handle, const char *db_server, const char *db_urer, 
		const char *db_passwd, const char *db_name, const unsigned short db_port);

/* execute sql, not return data */
int mysql_exec_single(MYSQL *mysql_conn, const char *sqlstr);

/* 返回单一结果 */
int mysql_select_count(MYSQL *mysql_conn, const char *sqlstr, int * count);

int mysql_select_count(MYSQL *mysql_conn, const char *sqlstr, float * count) ;

int mysql_exec_table(MYSQL *mysql_conn, const char *sqlstr, list<vector<string> > &list_table);

int mysql_select_array(MYSQL * mysql_conn, const char * sqlstr, float **arr, int num);

int mysql_select_array(MYSQL * mysql_conn, const char * sqlstr, int (*arr));



#endif // MYSQL_DB_H_

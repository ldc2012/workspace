#ifndef EVALUATION_SERVER_H_
#define EVALUATION_SERVER_H_

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "mysql_db.h"
#include "conf.h"
#include "log.h"
#include "cloud_assess.h"

using namespace std;

class CEvaluationServer {
public:
	CEvaluationServer();
	~CEvaluationServer();
	bool work();

private:
	/**
	 * @brief 计算虚拟机运行等级并将结果入库
	 * @param arg
	 * @return
	 */
	 static void *do_running_level(void *arg);
	/**
	 * @brief 获得云环境性能评估数据并将结果入库
	 * @param arg
	 * @return
	 */
	static void *do_cloud_evalution(void *arg);
	
private:

};

#endif // EVALUATION_SERVER_H__

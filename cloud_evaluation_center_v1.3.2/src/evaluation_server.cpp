#include "evaluation_server.h"

CEvaluationServer::CEvaluationServer()
{
}

CEvaluationServer::~CEvaluationServer()
{
}

 bool CEvaluationServer::work() {
	 int err;
	 pthread_t tid;
	 pthread_attr_t tattr;
	 pthread_attr_init(&tattr);
	 if (0 != (err = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED))) {
		 logerr("pthread_attr_setdetachstate fail: %s", strerror(err));
		 return false;
	 }
	 if (0 != (err = pthread_create(&tid, &tattr, run_level, NULL))) {
		 logerr("pthread_create get_running_level fail: %s", strerror(err));
		 return false;
	 }
	 if (0 != (err = pthread_create(&tid, &tattr, cloud_assess, NULL))) {
		 logerr("pthread_create get_cloud_evalution fail: %s", strerror(err));
		 return false;
	 }

	 return true;
 }

void *CEvaluationServer::do_running_level(void *arg)
{
#ifdef DEBUG
	printf("do_running_level\n");
#else
	logrun("do_running_level");
#endif	

	// TODO: running_level algorithm and result insert into DB

	return NULL;
}

void *CEvaluationServer::do_cloud_evalution(void *arg)
{
#ifdef DEBUG	
	printf("do_cloud_evalution\n");
#else
	logrun("do_cloud_evalution");
#endif

	// TODO: cloud_evalution algorithem and result insert into DB

	return NULL;
}

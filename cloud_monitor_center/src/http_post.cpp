#include "http_post.h"


/*
*Function: implemented asynchronous http client post 
*request for http server.
*/
/*
int 
httpPostAsyn(char *uri, char *data, size_t size, char *enc_type)
{
    ghttp_request *request = NULL;
    ghttp_status stat;
	int retstat = 0;
	    
	if ((request = ghttp_request_new())==NULL) {
		logerr("Could not create an http request! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}
	
	if (retstat != -1 && (ghttp_set_uri (request, uri)) != 0) {
		logerr("Invalid uri! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}

	if (retstat != -1 && (ghttp_set_type (request, ghttp_type_post)) != 0) {
		logerr("Request type is invalid or unsupported! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}

	if (retstat != -1) {
		ghttp_set_header(request, http_hdr_Connection, "close");
		ghttp_set_header(request, "Content-Type", "text/json");
	}

	if (retstat != -1 && (ghttp_set_body(request,data,size)) != 0) {
		logerr("Request type doesn't support it! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}

	//ghttp_set_chunksize(request,65535);

	if (retstat != -1 && (ghttp_prepare (request) != 0)) {
		logerr("Could not prepare http request! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}

	if (retstat != -1 && ghttp_set_sync (request, ghttp_async)) {
		logerr("Couldn't get async mode! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}
    
    while (1) {
		if (retstat == -1)
			break;
        stat = ghttp_process(request);
        if(stat == ghttp_error){
			return -2;
            break;
		}

        if(stat == ghttp_done){
			retstat = ghttp_status_code(request);
			logrun("Request status code -> [%d] Line %d File %s\n", retstat,  __LINE__,  __FILE__);
            break;
        }
    }
   
	//ghttp_clean(request);
	if (request)
		ghttp_request_destroy(request);

	return retstat;
}
*/
/*
*Function: implemented asynchronous http client post request
*for http server
*/
int 
httpPostAsyn(char *uri, char *data, size_t size, char *enc_type)
{
    ghttp_request *request = NULL;
    ghttp_status status;
	int retstat = 0;
	    
	do {
		if ((request = ghttp_request_new())==NULL)
			break;
		if ((ghttp_set_uri (request, uri)) != 0) 
			break;
		if ((ghttp_set_type (request, ghttp_type_post)) != 0)
			break;
		ghttp_set_header(request, http_hdr_Connection, "close");
		ghttp_set_header(request, "Content-Type", "text/json");
		if ((ghttp_set_body(request,data,size)) != 0)
			break;
		if ((ghttp_prepare (request) != 0))
			break;
		if (ghttp_set_sync (request, ghttp_async))
			break;
	} while(0);
    
    while (1) {

		status = ghttp_process(request);
        if(status == ghttp_error){
			return -2;
            break;
		}

        if(status == ghttp_done){
			retstat = ghttp_status_code(request);
            break;
        }
    }
   
	if (request)
		ghttp_request_destroy(request);

	return retstat;
}
/*
*Function: implemented synchronous http client post 
*request for http server.
*/
/*
int 
httpPost( char *uri, char *data, size_t size, char *enc_type )
{
	ghttp_request *request = NULL;
	ghttp_status stat;
	int retstat = 0;

	if ((request = ghttp_request_new())==NULL) {
		logerr("Could not create an http request! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}
	
	if (retstat != -1 && (ghttp_set_uri (request, uri)) != 0) {
		logerr("Invalid uri! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}

	if (retstat != -1 && (ghttp_set_type (request, ghttp_type_post)) != 0) {
		logerr("Request type is invalid or unsupported! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}

	if (retstat != -1) {
		ghttp_set_header(request, http_hdr_Connection, "close");
		ghttp_set_header(request, "Content-Type", "text/json");
	}

	if (retstat != -1 && (ghttp_set_body(request,data,size)) != 0) {
		logerr("Request type doesn't support it! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}

	if (retstat != -1 && (ghttp_prepare (request) != 0)) {
		logerr("Could not prepare http request! Line %d File %s\n", __LINE__, __FILE__);
		retstat = -1;
	}
	
	if (retstat != -1) {
	    stat = ghttp_process(request);

        retstat = ghttp_status_code(request);
	    logrun("Request status code -> [%d] Line %d File %s\n", retstat, __LINE__,__FILE__);
	}
	
	if (request)
		ghttp_request_destroy(request);

	return retstat;
}
*/

/*
*Function: implemented synchronous http client post
*request for http server.
*/
int 
httpPost( char *uri, char *data, size_t size, char *enc_type )
{
	ghttp_request *request = NULL;
	ghttp_status stat;
	int retstat = 0;

	do {
		if ((request = ghttp_request_new())==NULL)
			break;
		if ((ghttp_set_uri (request, uri)) != 0)
			break;
		if ((ghttp_set_type (request, ghttp_type_post)) != 0)
			break;
		ghttp_set_header(request, http_hdr_Connection, "close");
		ghttp_set_header(request, "Content-Type", "text/json");
		if ((ghttp_set_body(request,data,size)) != 0)
			break;
		if ((ghttp_prepare (request) != 0))
			break;
	} while(0);
	
	stat = ghttp_process(request);
    retstat = ghttp_status_code(request);
	logrun("Request status code -> [%d] Line %d File %s\n", retstat, __LINE__,__FILE__);
	
	if (request)
		ghttp_request_destroy(request);

	return retstat;
}

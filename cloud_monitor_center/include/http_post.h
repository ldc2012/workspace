#ifndef HTTP_POST_H
#define HTTP_POST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ghttp.h>

#include "log.h"


/*
*@brief send the synchronous http post request.
*@param uri
*@param data
*@param size
*@param enc_type
*/
int 
httpPost(char *uri, char *data, size_t size, char *enc_type);

/*
*@brief send the asynchronous http post request.
*@param uri
*@param data
*@param size
*@param enc_type
*/
int
httpPostAsyn(char *uri, char *data, size_t size, char *enc_type);


#endif /* HTTP_POST_H */

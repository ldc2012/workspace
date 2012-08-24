#include <net-snmp/net-snmp-config.h>  
  
#if HAVE_STDLIB_H  
#include <stdlib.h>  
#endif  
#if HAVE_UNISTD_H  
#include <unistd.h>  
#endif  
#if HAVE_STRING_H  
#include <string.h>  
#else  
#include <strings.h>  
#endif  
#include <sys/types.h>  
#if HAVE_NETINET_IN_H  
#include <netinet/in.h>  
#endif  
#include <stdio.h>  
#include <ctype.h>  
#if TIME_WITH_SYS_TIME  
# ifdef WIN32  
#  include <sys/timeb.h>  
# else  
#  include <sys/time.h>  
# endif  
# include <time.h>  
#else  
# if HAVE_SYS_TIME_H  
#  include <sys/time.h>  
# else  
#  include <time.h>  
# endif  
#endif  
#if HAVE_SYS_SELECT_H  
#include <sys/select.h>  
#endif  
#if HAVE_WINSOCK_H  
#include <winsock.h>  
#endif  
#if HAVE_NETDB_H  
#include <netdb.h>  
#endif  
#if HAVE_ARPA_INET_H  
#include <arpa/inet.h>  
#endif  
  
#include <net-snmp/utilities.h>  
  
#include <net-snmp/net-snmp-includes.h>  
#include <vector>  
#include <map>  
#include <iostream>  
using namespace  std;  
  
#define NETSNMP_DS_APP_DONT_FIX_PDUS 0  
  
   
  
string fprint_variable_1(const oid * objid,size_t objidlen, const netsnmp_variable_list * variable)  
{  
    u_char         *buf = NULL;  
    size_t          buf_len = 256, out_len = 0;  
    if ((buf = (u_char *) calloc(buf_len, 1)) == NULL) {  
          
        return "";  
    } else {  
        if (sprint_realloc_variable(&buf, &buf_len, &out_len, 1,  
                                    objid, objidlen, variable)) {  
            string strTemp((char*)buf);  
              
            int iFirst=strTemp.find_first_of(":");  
            string strInfo=strTemp.substr(iFirst,strTemp.length()-1);  
              
            return strInfo;  
              
        } else {  
            fprintf(stdout, "%s [TRUNCATED]\n", buf);  
        }  
    }  
    SNMP_FREE(buf);  
}  
  
string fprint_variable_2(const oid * objid,size_t objidlen, const netsnmp_variable_list * variable)  
{  
    u_char         *buf = NULL;  
    size_t          buf_len = 256, out_len = 0;  
    if ((buf = (u_char *) calloc(buf_len, 1)) == NULL) {  
          
        return "";  
    } else {  
        if (sprint_realloc_variable(&buf, &buf_len, &out_len, 1,  
                                    objid, objidlen, variable)) {  
            string strTemp((char*)buf);  
              
            return strTemp;  
              
        } else {  
            fprintf(stdout, "%s [TRUNCATED]\n", buf);  
        }  
    }  
    SNMP_FREE(buf);  
}  
  
string SnmpGet(char *community,char *ip,char *oid_array)//snmpget -v2c -c public 10.0.0.11 1.3.6.1.2.1.1.5.0  
{  
    netsnmp_session session, *ss=(netsnmp_session*)malloc(sizeof(netsnmp_session));  
    netsnmp_pdu    *response=(netsnmp_pdu*)malloc(sizeof(netsnmp_pdu));  
      
    netsnmp_variable_list *vars=NULL;  
    netsnmp_pdu    *pdu;  
      
    if(!ss)  
        ss = (netsnmp_session*)malloc(sizeof(netsnmp_session));  
    if(!response)  
        response=(netsnmp_pdu*)malloc(sizeof(netsnmp_pdu));  
      
      
    int             count;  
    int             current_name = 0;  
    char           *names[SNMP_MAX_CMDLINE_OIDS];  
    oid             name[MAX_OID_LEN];  
    size_t          name_length;  
    int             status;  
    int             failures = 0;  
    int             exitval = 0;  
      
    snmp_sess_init(&session);  
      
    session.version = SNMP_VERSION_2c;  
      
    session.peername = ip;  
    session.community = (unsigned char*)community;  
    session.community_len = strlen(community);  
      
      
    names[0] = oid_array;  
    current_name = 1;  
      
    SOCK_STARTUP;  
      
    snmp_close(ss);  
    ss = snmp_open(&session);  
      
    if (ss == NULL) {  
        snmp_sess_perror("snmpget", &session);  
        SOCK_CLEANUP;  
        return "";  
    }  
      
    pdu = snmp_pdu_create(SNMP_MSG_GET);  
    for (count = 0; count < current_name; count++) {  
        name_length = MAX_OID_LEN;  
        if (!snmp_parse_oid(names[count], name, &name_length)) {  
            snmp_perror(names[count]);  
            failures++;  
        }  
        else  
        {  
            snmp_add_null_var(pdu, name, name_length);  
        }  
    }  
    if (failures) {  
        snmp_close(ss);  
        SOCK_CLEANUP;  
        return "";  
    }  
      
    retry:  
    status = snmp_synch_response(ss, pdu, &response);  
    if (status == STAT_SUCCESS) {  
        if (response->errstat == SNMP_ERR_NOERROR)  
        {     
            vars = response->variables;  
              
            string strTemp=fprint_variable_1(vars->name, vars->name_length, vars);  
              
            return strTemp;  
        }  
        else  
        {  
            printf("2\n");  
            fprintf(stderr, "Error in packet\nReason: %s\n",  
                    snmp_errstring(response->errstat));  
              
            if (response->errindex != 0) {  
                fprintf(stderr, "Failed object: ");  
                for (count = 1, vars = response->variables;  
                     vars && count != response->errindex;  
                     vars = vars->next_variable, count++)  
                    /*EMPTY*/;  
                if (vars) {  
                    fprint_objid(stderr, vars->name, vars->name_length);  
                }  
                fprintf(stderr, "\n");  
            }  
            exitval = 2;  
              
              
            if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,  
                                        NETSNMP_DS_APP_DONT_FIX_PDUS)) {  
                pdu = snmp_fix_pdu(response, SNMP_MSG_GET);  
                snmp_free_pdu(response);  
                response = NULL;  
                if (pdu != NULL) {  
                    goto retry;  
                }  
            }  
        }                       /* endif -- SNMP_ERR_NOERROR */  
          
    } else if (status == STAT_TIMEOUT) {  
        fprintf(stderr, "Timeout: No Response from %s.\n",  
                session.peername);  
        exitval = 1;  
          
    } else {                    /* status == STAT_ERROR */  
        snmp_sess_perror("snmpget", ss);  
        exitval = 1;  
          
    }                           /* endif -- STAT_SUCCESS */      
      
    if (response)  
        snmp_free_pdu(response);  
    snmp_close(ss);  
      
    SOCK_CLEANUP;  
      
    return "";  
}  
  
   int main()  
{  
    string strSnmpInfo;  
    //    strSnmpInfo=SnmpGet("2c","public","192.168.1.241",".1.3.6.1.4.1.11.2.3.9.1.1.7.0");  
    //    strSnmpInfo=SnmpGet("2c","public","192.168.1.241",".1.3.6.1.2.1.1.1.0");  
    strSnmpInfo=SnmpGet("public","192.168.1.2","1.3.6.1.2.1.1.1.0");  
        
    cout<<"wyz---------------snmpInfo::"<<strSnmpInfo<<endl;  
    return 0;  
}

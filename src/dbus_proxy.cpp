#include <dbus/dbus.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>
#include <queue>
#include <map>

#include "dbus_proxy.h"

#include "dbus_proxy_private.h"
#include "dbus_proxy_utility.h"


DBusConnection* pDbusConn = NULL; 
pthread_mutex_t mutex4Conn = PTHREAD_MUTEX_INITIALIZER;

bool bListenerThreadAlive = FALSE;
pthread_mutex_t mutex4ListenerThread = PTHREAD_MUTEX_INITIALIZER;

std::string strBusName;
pthread_mutex_t mutex4Busname = PTHREAD_MUTEX_INITIALIZER;

bool bAllowReplaceBusName = FALSE;
pthread_mutex_t mutex4ReplaceBusName = PTHREAD_MUTEX_INITIALIZER;

std::list<regSignal_t> regSignalList;
pthread_mutex_t mutex4RegSigList = PTHREAD_MUTEX_INITIALIZER;

std::list<regMethod_t> regMethodList;
pthread_mutex_t mutex4RegMetList = PTHREAD_MUTEX_INITIALIZER;

const int nDbusMethodTimeoutMillisecond = 5000;
/// defines how many processing thread can be active at the same time, for Signal and Method, respectively.
const int nSignalThreadPoolCapacity = 1;
const int nMethodThreadPoolCapacity = 1;

int nSignalThreadPoolDepth = 0;
int nMethodThreadPoolDepth = 0;


pthread_t ListenerThread;


static const std::string dbus_proxy_error_msg[] =
{
    "Succeeded.", 
    "Failed.", 
    "Invalid argument.", 
    "Insufficient memory.", 
    "D-Bus daemon is not running or configured properly.", 
    "Application already owns the bus name.", 
    "The Signal subscription is duplicated with existing entry.", 
    "D-Bus method call time-out.", 
    "D-Bus method call error.", 
    "D-Bus Adapter operation failed due to un-identified reason.", 
};

DBUS_PROXY_RET_T dbus_proxy_init_conn()
{
    pthread_mutex_lock(&mutex4Conn);
    if (pDbusConn == NULL)
    {
        // declare and initialise the error
        DBusError err;
        dbus_error_init(&err);
        // connect to the bus
        
        pDbusConn = dbus_bus_get(DBUS_BUS_SESSION, &err);
        if (dbus_error_is_set(&err)) {
            fprintf(stderr, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
            pthread_mutex_unlock(&mutex4Conn);
            return  DBUS_PROXY_RET_CONNECTION_ERROR;
        }
        /// The following settings must be done each time after the connection is re-established.
        // "exit_on_disconnect" was automatically set TRUE by dbus_bus_get(), need to reverse it.
        dbus_connection_set_exit_on_disconnect(pDbusConn, FALSE);
    }
    pthread_mutex_unlock(&mutex4Conn);
    return DBUS_PROXY_RET_SUCCESS;
}

void dbus_proxy_close_conn()
{
    pthread_mutex_lock(&mutex4Conn);
    if ( pDbusConn != NULL )  dbus_connection_unref(pDbusConn);
    pDbusConn = NULL;
    pthread_mutex_unlock(&mutex4Conn);
    dbus_shutdown();
}

DBUS_PROXY_RET_T dbus_proxy_init_listener_thread()
{
    pthread_mutex_lock(&mutex4Conn);
    DBusConnection* pConnection = pDbusConn;
    pthread_mutex_unlock(&mutex4Conn);
	
    pthread_mutex_lock(&mutex4ListenerThread);
    if (bListenerThreadAlive == FALSE)
    {
        dbus_connection_add_filter(pConnection, DbusMessageFilter, NULL, NULL);
        
        //initialize err.
        DBusError err;
        dbus_error_init(&err);
        
        // add a rule for which kind of signal messages we want to process.
        char szMatchRule[128]; 
        snprintf(szMatchRule, sizeof(szMatchRule), "type='signal',interface='%s'",  DBUS_SIGNALINTERFACE);
        
        // see signals from the given interface
        dbus_bus_add_match(pConnection, szMatchRule, &err);


        if (CheckDbusError(&err, "Listener Thread"))
        {
            pthread_mutex_unlock(&mutex4ListenerThread);
            return DBUS_PROXY_RET_LISTENER_ERROR;
        }
        
        // flush the connection.
        dbus_connection_flush(pConnection);

        
        // in case this a re-connect due to previous conenction lost, and a bus name was already specified,
        // need to reclaim it if possbile.
        // don't return error code if bus name request fails though
        if (!strBusName.empty() && pConnection == NULL)
        {
            dbus_bus_request_name(pConnection, strBusName.c_str(), \
                                  ((bAllowReplaceBusName)?DBUS_NAME_FLAG_ALLOW_REPLACEMENT:0)| \
                                  DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
        }

        // Start the thread
        int ret = pthread_create(&ListenerThread, NULL, DbusListenThreadProc, NULL);
        pthread_detach(ListenerThread);
        if (ret != 0){
            fprintf(stderr, "Start Thread Error.\n");
            pthread_mutex_unlock(&mutex4ListenerThread);
            return DBUS_PROXY_RET_LISTENER_ERROR;
        }
        
    }
    pthread_mutex_unlock(&mutex4ListenerThread);
    return DBUS_PROXY_RET_SUCCESS;
}

#ifdef __cplusplus
extern "C"
{
#endif
char * dbus_proxy_GetErrorMessage(DBUS_PROXY_RET_T eReturnCode)
{
    std::string szResult;
    if (DBUS_PROXY_RET_SUCCESS <= eReturnCode && DBUS_PROXY_RET_UNKNOWN >= eReturnCode)
        szResult = dbus_proxy_error_msg[eReturnCode];
    return const_cast<char*>(szResult.c_str());
}







DBUS_PROXY_RET_T dbus_proxy_signal_emit(const char * szSenderName, const char * szSignalName, const void * pSignalContent, int iContentSize)
{
    if (szSenderName == NULL || szSignalName == NULL)
        return DBUS_PROXY_RET_INVALID_ARGUMENT;

    DBUS_PROXY_RET_T errorCode = dbus_proxy_init_conn();
    if (errorCode != DBUS_PROXY_RET_SUCCESS)
        return errorCode;
    //cast the szSenderName to valid form.
    std::string strSenderName = DbusGetValidObjectPath(szSenderName);
    std::string strSignalName = DbusGetValidMemberName(szSignalName);
    //create a new signal
    DBusMessage* pDbusMsg = dbus_message_new_signal(strSenderName.c_str(),\
                                                    DBUS_SIGNALINTERFACE,strSignalName.c_str());
    
    //add the signal content .
    if (pDbusMsg != NULL)
    {
        pthread_mutex_lock(&mutex4Conn);
        DBusConnection* pConnection = pDbusConn;
        pthread_mutex_unlock(&mutex4Conn);
        
        if (pConnection != NULL)
        {
            DBusMessageIter msg_iter;
            dbus_message_iter_init_append(pDbusMsg, &msg_iter);
            if (iContentSize > 0)
            {
                //initialize the contained_signature and append the signal content to container. 
                char signature[2]=""; 
                signature[0] = (char) DBUS_TYPE_BYTE, signature[1] = '\0';
                DBusMessageIter sub_iter;
                dbus_message_iter_open_container(&msg_iter, DBUS_TYPE_ARRAY, signature, &sub_iter);
                dbus_message_iter_append_fixed_array(&sub_iter, DBUS_TYPE_BYTE, &pSignalContent, \
                                                     iContentSize);
                dbus_message_iter_close_container(&msg_iter, &sub_iter);
            }
            //place the signal in outgoing queue
            dbus_uint32_t serial = 0;
            dbus_connection_send(pConnection, pDbusMsg, &serial);
            //flush the connection
            dbus_connection_flush(pConnection);
            //unref the pDbusMsg.
            dbus_message_unref(pDbusMsg);
            return DBUS_PROXY_RET_SUCCESS;
        }
        else
        {
            dbus_message_unref(pDbusMsg);
            return DBUS_PROXY_RET_FAILED;
        }
    }
    else
        return DBUS_PROXY_RET_INSUFFICIENT_MEMORY;
}

DBUS_PROXY_RET_T dbus_proxy_signal_reg(const char* szSignalName, DBUS_PROXY_SIGNAL_CALLBACK pfnSignalCallback, void * pContext)
{
    if (szSignalName == NULL || pfnSignalCallback == NULL)
        return DBUS_PROXY_RET_INVALID_ARGUMENT;
	
    DBUS_PROXY_RET_T errorCode = dbus_proxy_init_conn();
    if (errorCode != DBUS_PROXY_RET_SUCCESS)
        return errorCode;
	
    //start listener thread.
    errorCode = dbus_proxy_init_listener_thread();
    if (errorCode != DBUS_PROXY_RET_SUCCESS)
        return errorCode;

    std::string strSignalName = szSignalName;
	
    //cast the argument to valid form.
    if (!strSignalName.empty())
        std::string strSignalName = DbusGetValidMemberName(szSignalName);
	
	
    //check if the signal be re-registered.
    //a signal can be registered at most once.
    pthread_mutex_lock(&mutex4RegSigList);
    for (std::list<regSignal_t>::const_iterator it=regSignalList.begin(); it!=regSignalList.end(); ++it)
    {
        if (it->strSignalName == strSignalName)
        {
            pthread_mutex_unlock(&mutex4RegSigList);
            return DBUS_PROXY_RET_DUPLICATE_REGISTERED;
        }
    }
    //frist time register
    regSignal_t reg_signal;
    reg_signal.strSignalName = strSignalName;
    reg_signal.pfnSignalCallback = (void*)pfnSignalCallback;
    reg_signal.pContext = pContext;
    regSignalList.push_back(reg_signal);
    pthread_mutex_unlock(&mutex4RegSigList);
    return DBUS_PROXY_RET_SUCCESS;
}

DBUS_PROXY_RET_T dbus_proxy_signal_unreg(const char* szSignalName)
{
    if (szSignalName == NULL)
        return DBUS_PROXY_RET_INVALID_ARGUMENT;
	
    std::string strSignalName = DbusGetValidMemberName(szSignalName);
    
    pthread_mutex_lock(&mutex4RegSigList);
    for (std::list<regSignal_t>::iterator it=regSignalList.begin(); it!=regSignalList.end(); ++it)
    {
        if (it->strSignalName == strSignalName)
        {
            //remove the regSignal from the regSignalList.
            std::list<regSignal_t>::iterator it_to_del = it;
            regSignalList.erase(it_to_del);
            pthread_mutex_unlock(&mutex4RegSigList);
            // if there is no any registered message, we need to notify the Listener Thread to exit.
            if (regSignalList.empty() && regMethodList.empty())
            {
                fprintf(stderr,"********* There is no signal/method registered, " \
                        "ListenerThread may be exit. *********\n");
                pthread_mutex_lock(&mutex4ListenerThread);
                bListenerThreadAlive = FALSE;
                pthread_mutex_unlock(&mutex4ListenerThread);
                
            }
            return   DBUS_PROXY_RET_SUCCESS;
        }
    }
    pthread_mutex_unlock(&mutex4RegSigList);
    return DBUS_PROXY_RET_FAILED;// not find.
}

DBUS_PROXY_RET_T dbus_proxy_request_busname(const char * szBusName, bool bAllowReplacement)
{
    if (szBusName == NULL)
        return DBUS_PROXY_RET_INVALID_ARGUMENT;
    
    DBUS_PROXY_RET_T errorCode = dbus_proxy_init_conn();
    if (errorCode != DBUS_PROXY_RET_SUCCESS)
        return errorCode;

    // declare and initialise the error
    DBusError err;
    dbus_error_init(&err);

    pthread_mutex_lock(&mutex4Busname);
    strBusName = DbusGetValidBusName(szBusName);
    pthread_mutex_unlock(&mutex4Busname);

    pthread_mutex_lock(&mutex4ReplaceBusName);
    bAllowReplaceBusName = bAllowReplacement;
    pthread_mutex_unlock(&mutex4ReplaceBusName);

    pthread_mutex_lock(&mutex4Conn);
    DBusConnection* pConnection = pDbusConn;
    pthread_mutex_unlock(&mutex4Conn);

    if (pConnection == NULL)
        return DBUS_PROXY_RET_FAILED;

    int ret = dbus_bus_request_name(pConnection, strBusName.c_str(),    \
                                    ((bAllowReplacement)?DBUS_NAME_FLAG_ALLOW_REPLACEMENT:0)| \
                                    DBUS_NAME_FLAG_REPLACE_EXISTING, &err);

    if(CheckDbusError(&err, "Request Name"))
    {
        return DBUS_PROXY_RET_FAILED;
    }

    switch (ret)
    {
    case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
        return DBUS_PROXY_RET_SUCCESS;
    case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
        return DBUS_PROXY_RET_ALREADY_BUSOWNER;
    default:
        return DBUS_PROXY_RET_FAILED;
    }
}

DBUS_PROXY_RET_T dbus_proxy_method_call(const char * szCallerName, const char * szServiceBusName, const char * szMethodName,const void * pParameters, int iParameterSize, char *szOutputData, int *iOutputDataSize)
{
    if (szCallerName == NULL || szServiceBusName == NULL)
        return DBUS_PROXY_RET_INVALID_ARGUMENT;
    
    DBUS_PROXY_RET_T errorCode = dbus_proxy_init_conn();
    if (errorCode != DBUS_PROXY_RET_SUCCESS)
        return errorCode;

    std::string strBusName =  DbusGetValidBusName(szServiceBusName);
    std::string strCallerName =  DbusGetValidObjectPath(szCallerName);
    std::string strMethodName =  DbusGetValidMemberName(szMethodName);

    // create a new method call and check for errors
    DBusMessage* pDbusMsg = dbus_message_new_method_call(strBusName.c_str(), strCallerName.c_str(),  DBUS_METHODINTERFACE, strMethodName.c_str());

    if ( pDbusMsg == NULL )
        return  DBUS_PROXY_RET_INSUFFICIENT_MEMORY;
    
    // append the parameters buffer onto signal message
    if ( iParameterSize > 0 )
    {
        dbus_message_append_args(pDbusMsg, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &pParameters, \
                                 iParameterSize, DBUS_TYPE_INVALID);
    }
    // send message and get a handle for a reply

    DBusPendingCall *pPendingCall;
    {
    pthread_mutex_lock(&mutex4Conn);
    DBusConnection* pConnection = pDbusConn;
    pthread_mutex_unlock(&mutex4Conn);
    if ( pConnection == NULL )
        return  DBUS_PROXY_RET_FAILED;
    
    dbus_bool_t bResult = dbus_connection_send_with_reply(pConnection, pDbusMsg, &pPendingCall, \
                                                          nDbusMethodTimeoutMillisecond);
    // free message
    dbus_message_unref(pDbusMsg);
    
    if ( !bResult )
        return  DBUS_PROXY_RET_INSUFFICIENT_MEMORY;
    if ( pPendingCall == NULL )
        return  DBUS_PROXY_RET_DAEMON_ERROR;
    }

    //dbus_connection_flush(pConnection);
    // block until we recieve a reply
    dbus_pending_call_block(pPendingCall);

    //get the reply message.
    DBusMessage* pDbusReply = dbus_pending_call_steal_reply(pPendingCall);
    // free the pending message handle
    dbus_pending_call_unref(pPendingCall);

    if ( pDbusReply == NULL )
        return  DBUS_PROXY_RET_METHODCALL_TIMEOUT;

    //check error.
    DBusError err;
    dbus_error_init(&err);
    if ( dbus_set_error_from_message(&err, pDbusReply) )
    {
        CheckDbusError(&err, "Method return");
        return  DBUS_PROXY_RET_METHODCALL_ERROR;
    }
    

    DBusMessageIter msg_iter;
    if ( dbus_message_iter_init(pDbusReply, &msg_iter) )
    {
        if ( dbus_message_iter_get_arg_type(&msg_iter) == DBUS_TYPE_ARRAY )
        {// Get the response result content
            DBusMessageIter sub_msg_iter;
            dbus_message_iter_recurse(&msg_iter, &sub_msg_iter);
            char *pBuffer;
            dbus_message_iter_get_fixed_array(&sub_msg_iter, &pBuffer, iOutputDataSize);
            memcpy(szOutputData, pBuffer, *iOutputDataSize);
        }
    }

    // free reply 
    dbus_message_unref(pDbusReply);
    return DBUS_PROXY_RET_SUCCESS;
}

DBUS_PROXY_RET_T dbus_proxy_method_reg(const char* szMethodName,DBUS_PROXY_METHOD_CALLBACK pfnMethodCallback, void * pContext)
{
    if (szMethodName == NULL || pfnMethodCallback == NULL)
        return DBUS_PROXY_RET_INVALID_ARGUMENT;
	
    DBUS_PROXY_RET_T errorCode = dbus_proxy_init_conn();
    if (errorCode != DBUS_PROXY_RET_SUCCESS)
        return errorCode;
	
    //start listener thread.
    errorCode = dbus_proxy_init_listener_thread();
    if (errorCode != DBUS_PROXY_RET_SUCCESS)
        return errorCode;

    std::string strMethodName =  DbusGetValidMemberName(szMethodName);

    //check if the method be re-registered.
    pthread_mutex_lock(&mutex4RegMetList);
    for (std::list<regMethod_t>::const_iterator it=regMethodList.begin(); it!=regMethodList.end(); ++it)
    {
        if (it->strMethodName == strMethodName)
        {
            pthread_mutex_unlock(&mutex4RegMetList);
            return DBUS_PROXY_RET_DUPLICATE_REGISTERED;
        }
    }
    //frist time register the method
    regMethod_t reg_method;
    reg_method.strMethodName = strMethodName;
    reg_method.pfnMethodCallback = (void*)pfnMethodCallback;
    reg_method.pContext = pContext;
    regMethodList.push_back(reg_method);
    pthread_mutex_unlock(&mutex4RegMetList);
    return DBUS_PROXY_RET_SUCCESS;
}

DBUS_PROXY_RET_T dbus_proxy_method_unreg(const char* szMethodName)
{
    if (szMethodName == NULL)
        return DBUS_PROXY_RET_INVALID_ARGUMENT;
	
    std::string strMethodName = DbusGetValidMemberName(szMethodName);
    
    pthread_mutex_lock(&mutex4RegMetList);
    for (std::list<regMethod_t>::iterator it=regMethodList.begin(); it!=regMethodList.end(); ++it)
    {
        if (it->strMethodName == strMethodName)
        {
            //remove the regMethod and check if the reglist empty.
            std::list<regMethod_t>::iterator it_to_del = it;
            regMethodList.erase(it_to_del);
            pthread_mutex_unlock(&mutex4RegMetList);
            
            if (regMethodList.empty() && regSignalList.empty())
            {
                fprintf(stderr,"********* There is no method/signal registered, " \
                        "ListenerThread may be exit. *********\n");
                pthread_mutex_lock(&mutex4ListenerThread);
                bListenerThreadAlive = FALSE;
                pthread_mutex_unlock(&mutex4ListenerThread);

            }
            return   DBUS_PROXY_RET_SUCCESS;
        }
    }
    pthread_mutex_unlock(&mutex4RegMetList);
    return DBUS_PROXY_RET_FAILED;// not find.
}
#ifdef __cplusplus
}
#endif

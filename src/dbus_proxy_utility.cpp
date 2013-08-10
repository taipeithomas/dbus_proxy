#include "dbus_proxy_utility.h"
#include "syncQueue.h"

SyncQueue <DBUS_PROXY_SIGNAL_ENTRY_T> SignalQueue;

SyncQueue <DBUS_PROXY_METHOD_ENTRY_T> MethodQueue;

std::string  DbusGetValidMemberName(const char* szInput)
{
    std::string strInput = szInput;
    std::string strResult(strInput);
    // remove all invalid characters (non alpha-numeric-'_')
    size_t index = 0;
    while (index < strResult.size())
    {
        if ( isalnum(strResult[index]) || strResult[index] == '_' )
            ++ index;
        else
            strResult.erase(index, 1);
    }
    // If Input name is empty, it is for Match All signal name
    if ( strResult.size() == 0 ) strResult =  DBUS_MEMBERNAME_DEFAULT; 
    // Make sure the first character isn't digit
    if ( isdigit(strResult[0]) ) strResult = std::string( DBUS_MEMBERNAME_PREFIX) + strResult;
    // A valid member name has at most 255 characters.
    return strResult.substr(0, 255);
}


std::string  DbusGetValidObjectPath(const char* szInput)
{
    std::string strInput = szInput;
    std::string strResult(strInput);
    
    // Make sure the name starts with '/'
    if ( strInput[0] != '/' )   strResult = std::string("/") + strResult;
    // remove all invalid characters (non alpha-numeric-'_'-'/')
    size_t index = 1;
    while (index < strResult.size())
    {
        if ( isalnum(strResult[index]) || strResult[index] == '/' || strResult[index] == '_' )
            ++ index;
        else
            strResult.erase(index, 1);
    }
    // shrink all "//" into "/"
    while ( (index=strResult.find("//")) != std::string::npos )
        strResult.erase(index, 1);
    // remove the trailing '/' if exists (unless the whole name is "/")
    if ( strResult.size() > 1 && strResult[strResult.size()-1] == '/' )
        strResult.erase(strResult.size()-1);
    return strResult;
}


std::string  DbusGetValidBusName(const char* szInput)
{
    std::string strInput = szInput;
    std::string strResult(strInput);
    // remove all invalid characters (non alpha-numeric-'_-.')
    size_t index = 0;
    while (index < strResult.size())
    {
        if ( isalnum(strResult[index]) || strResult[index] == '_' || strResult[index] == '-' || strResult[index] == '.' )
            ++ index;
        else
            strResult.erase(index, 1);
    }
    // shrink all ".." into "."
    while ( (index=strResult.find("..")) != std::string::npos )
        strResult.erase(index, 1);
    // On the other hand, there must be at least 1 '.' inside
    if ( strResult.find(".") == std::string::npos )   strResult += '.';
    // Make sure the first character isn't '.'
    if ( strResult[0] == '.' ) strResult = std::string( DBUS_BUSNAME_PREFIX) + strResult;
    // Make sure the last character isn't '.'
    if ( strResult[strResult.size()-1] == '.' )   strResult += "bus";

    return strResult.substr(0, 255);
}


bool  CheckDbusError(DBusError *pError, const std::string& strText)
{
    if ( pError != NULL && dbus_error_is_set(pError) )
    {
        fprintf(stderr, "D-Bus %s error - %s", strText.c_str(), pError->message); 
        if ( strstr(pError->message, "\n") == NULL )    fprintf(stderr, "\n"); 
        dbus_error_free(pError); 
        return true;
    }
    return false;
}

DBUS_PROXY_SIGNAL_DATA_T CastSignalData(const DBUS_PROXY_SIGNAL_DATA_INTER_T& signalDataInter)
{
    DBUS_PROXY_SIGNAL_DATA_T signalData;
    signalData.szSignalName = signalDataInter.strSignalName.c_str();
    signalData.szSenderName = signalDataInter.strSenderName.c_str();
    signalData.pSignalContent = signalDataInter.strSignalContent.data();
    signalData.nContentSize = signalDataInter.nContentSize;
    return signalData;
}

DBUS_PROXY_METHOD_DATA_T CastMethodData(const DBUS_PROXY_METHOD_DATA_INTER_T& methodDataInter)
{
    DBUS_PROXY_METHOD_DATA_T methodData;
    methodData.szMethodName = methodDataInter.strMethodName.c_str();
    methodData.szCallerName = methodDataInter.strCallerName.c_str();
    methodData.pMethodParameters = methodDataInter.strMethodParameters.data();
    methodData.nParameterSize = methodDataInter.nParameterSize;
    return methodData;
}

bool isCanceledSignal(const DBUS_PROXY_SIGNAL_ENTRY_T& signal_entry)
{
    pthread_mutex_lock(&mutex4RegSigList);
    for (std::list<regSignal_t>::const_iterator it=regSignalList.begin(); it!=regSignalList.end(); ++it )
    {
        if (it->strSignalName == signal_entry.signalData.strSignalName)
        {
            pthread_mutex_unlock(&mutex4RegSigList);
            return false;
        }
    }
    pthread_mutex_unlock(&mutex4RegSigList);
    return true;
}

bool isCanceledMethod(const DBUS_PROXY_METHOD_ENTRY_T& method_entry)
{
    pthread_mutex_lock(&mutex4RegMetList);
    for (std::list<regMethod_t>::const_iterator it=regMethodList.begin(); it!=regMethodList.end(); ++it )
    {
        if (it->strMethodName == method_entry.methodData.strMethodName)
        {
            pthread_mutex_unlock(&mutex4RegMetList);
            return false;
        }
    }
    pthread_mutex_unlock(&mutex4RegMetList);
    return true;
}

void *DbusListenThreadProc(void * data)
{
    bListenerThreadAlive = TRUE;
    while (bListenerThreadAlive == TRUE)
    {
        pthread_mutex_lock(&mutex4Conn);
        DBusConnection* pConnection = pDbusConn;
        pthread_mutex_unlock(&mutex4Conn);
        
        if ( pConnection == NULL )
        {
            if ( dbus_proxy_init_conn() !=  DBUS_PROXY_RET_SUCCESS )
                sleep(1); 
            else
                fprintf(stderr, "  *********  dbus_proxy Re-connected  *********\n");
            continue;
        }
        
        // check read/write status
        while ( bListenerThreadAlive == TRUE && dbus_connection_read_write_dispatch(pConnection, 1) )
        {   // Instead of empty loop body
            if ( dbus_connection_get_dispatch_status(pConnection) == DBUS_DISPATCH_DATA_REMAINS )
                dbus_connection_dispatch(pConnection);
            else
                usleep(20);
        }
        
        if ( bListenerThreadAlive == FALSE )
        {
            fprintf(stderr, "*********  dbus_proxy ListenerThread exit. *********\n");
            break;
        }
        fprintf(stderr, "  *********  dbus_proxy Disconnected  *********\n");
        dbus_proxy_close_conn();
    }
    return ((void *)0);
}

//if the signal we have registered, push it in the SignalQueue which would be processed.
bool DbusProcessSignal(DBUS_PROXY_SIGNAL_ENTRY_T& signal_entry)
{
    bool bProcessed = FALSE;
    pthread_mutex_lock(&mutex4RegSigList);
    for (std::list<regSignal_t>::const_iterator it=regSignalList.begin(); it!=regSignalList.end(); ++it )
    {
        if (it->strSignalName == signal_entry.signalData.strSignalName)
        {
            signal_entry.pfnSignalCallback = (DBUS_PROXY_SIGNAL_CALLBACK)it->pfnSignalCallback;
            signal_entry.pContext = it->pContext;
            SignalQueue.push(signal_entry);
            bProcessed = TRUE;
            break;
        }
    }
    pthread_mutex_unlock(&mutex4RegSigList);
    LaunchSignalProcessThread();
    return bProcessed;
}

// Create the SignalProcessThread, depending on our setting.
void LaunchSignalProcessThread(void)
{
    for ( ; nSignalThreadPoolDepth < nSignalThreadPoolCapacity; ++nSignalThreadPoolDepth )
    {
        pthread_t id;
        int ret = pthread_create(&id, NULL, SignalThreadProc, NULL);
        if (ret != 0){
            fprintf(stderr, "Start SignalThreadProc Error = %d, %s\n", ret, strerror(ret));
        }
        else
            pthread_detach(id);
    }
}

//pop the singal from SignalQueue and call the signal Callback func.
void *SignalThreadProc(void * data)
{
    DBUS_PROXY_SIGNAL_ENTRY signal_entry;
    while (bListenerThreadAlive == TRUE || !SignalQueue.empty())
    {
        SignalQueue.pop(signal_entry);
        if (bListenerThreadAlive == TRUE && !isCanceledSignal(signal_entry))
        {
            try{
                signal_entry.pfnSignalCallback(CastSignalData(signal_entry.signalData), \
                                               signal_entry.pContext);

            }catch(...){}
        }
    }
    // SignalThreadProc will exit, we need to change the pooldepth, if not,
    // it may not restart any more.
    nSignalThreadPoolDepth--;
    return ((void *)0);
}

bool DbusProcessMethod(DBUS_PROXY_METHOD_ENTRY_T& method_entry)
{
    bool bProcessed = FALSE;
    pthread_mutex_lock(&mutex4RegMetList);
    for  (std::list<regMethod_t>::const_iterator it=regMethodList.begin(); it !=regMethodList.end(); ++it )
    {
        if ( it->strMethodName == method_entry.methodData.strMethodName)
        {
            method_entry.pfnMethodCallback = (DBUS_PROXY_METHOD_CALLBACK)it->pfnMethodCallback;
            method_entry.pContext = it->pContext;
            method_entry.pDbusMsg = dbus_message_new_method_return(method_entry.pDbusMsg);
            if ( method_entry.pDbusMsg != NULL )
            {
                MethodQueue.push(method_entry);
            }
            bProcessed = TRUE;
            break;
        }
    }
    pthread_mutex_unlock(&mutex4RegMetList);
    LaunchMethodProcessThread();
    return bProcessed;
}

void LaunchMethodProcessThread(void)
{
    for ( ; nMethodThreadPoolDepth < nMethodThreadPoolCapacity; ++nMethodThreadPoolDepth )
    {
        pthread_t id;
        int ret = pthread_create(&id, NULL, MethodThreadProc, NULL);
        if (ret != 0){
            fprintf(stderr, "Start SignalThreadProc Error = %d, %s\n", ret, strerror(ret));
        }
        else
            pthread_detach(id);
    }
}

void *MethodThreadProc (void * data)
{
    DBUS_PROXY_METHOD_ENTRY_T method_entry;
    while (bListenerThreadAlive == TRUE || !MethodQueue.empty())
    {
        MethodQueue.pop(method_entry);
        if (bListenerThreadAlive == TRUE && !isCanceledMethod(method_entry))
        {
            std::string strResult;
            try{
                DBUS_PROXY_METHOD_CALLBACK_RET_VALUE_T Result;
                printf("pthread id = %lu\n",pthread_self());
                Result = method_entry.pfnMethodCallback(CastMethodData(method_entry.methodData), \
                                                        method_entry.pContext);
                strResult = std::string(Result.pCallbackRetValue, Result.nCallbackRetValueSize);
                

            }catch(...){}
            if ( strResult.size() > 0 )
            {
                const char *pResult = strResult.data();
                dbus_message_append_args(method_entry.pDbusMsg, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, \
                                         &pResult, strResult.size(), DBUS_TYPE_INVALID);
            }
            dbus_uint32_t serial = 0;
            
            pthread_mutex_lock(&mutex4Conn);
            DBusConnection* pConnection = pDbusConn;
            pthread_mutex_unlock(&mutex4Conn);


            if (pConnection != NULL && dbus_connection_send(pConnection, method_entry.pDbusMsg, &serial))
                ;
            dbus_connection_flush(pConnection);
            dbus_message_unref(method_entry.pDbusMsg);
        }
    }
    nMethodThreadPoolDepth--;
    return ((void *)0);
	
}

DBusHandlerResult  DbusMessageFilter(DBusConnection *connection, DBusMessage *pDbusMsg, void *user_data)
{
    
    int message_type = dbus_message_get_type(pDbusMsg);
    // check if the message is a signal from the correct interface
    if ( message_type == DBUS_MESSAGE_TYPE_SIGNAL && \
        dbus_message_has_interface(pDbusMsg,  DBUS_SIGNALINTERFACE) ) 
    {
        DBUS_PROXY_SIGNAL_ENTRY_T signal_entry;
        signal_entry.signalData.strSignalName = dbus_message_get_member(pDbusMsg);
        signal_entry.signalData.strSenderName = dbus_message_get_path(pDbusMsg);
        // read the signal content
        DBusMessageIter msg_iter;
        if ( dbus_message_iter_init(pDbusMsg, &msg_iter) )
        {
            if ( dbus_message_iter_get_arg_type(&msg_iter) == DBUS_TYPE_ARRAY )
            {
                DBusMessageIter sub_msg_iter;
                dbus_message_iter_recurse(&msg_iter, &sub_msg_iter);
                char * pBuffer; int nSize;
                dbus_message_iter_get_fixed_array(&sub_msg_iter, &pBuffer, &nSize);
                if ( nSize > 0 ){
                    signal_entry.signalData.strSignalContent = std::string(pBuffer, nSize);
                    signal_entry.signalData.nContentSize = nSize;
                }
            }
        }
        //dispatch the signal to be processed, if it had been registered.
        if (DbusProcessSignal(signal_entry))
            return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (dbus_message_is_signal(pDbusMsg, DBUS_INTERFACE_LOCAL, "Disconnected"))
    {
        dbus_proxy_close_conn();
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if ( message_type == DBUS_MESSAGE_TYPE_METHOD_CALL \
              && dbus_message_has_interface(pDbusMsg,  DBUS_METHODINTERFACE) )
    {
        DBUS_PROXY_METHOD_ENTRY_T method_entry;
        method_entry.methodData.strMethodName = dbus_message_get_member(pDbusMsg);
        method_entry.methodData.strCallerName = dbus_message_get_path(pDbusMsg);
			
        // read the parameters
        DBusMessageIter msg_iter;
        if ( dbus_message_iter_init(pDbusMsg, &msg_iter) )
        {
            if ( dbus_message_iter_get_arg_type(&msg_iter) == DBUS_TYPE_ARRAY )
            {
                DBusMessageIter sub_msg_iter;
                dbus_message_iter_recurse(&msg_iter, &sub_msg_iter);
                char * pBuffer; int nSize;
                dbus_message_iter_get_fixed_array(&sub_msg_iter, &pBuffer, &nSize);
                if ( nSize > 0 ){
                    method_entry.methodData.strMethodParameters = std::string(pBuffer, nSize);
                    method_entry.methodData.nParameterSize = nSize;
                }
            }
        }
        method_entry.pDbusMsg = pDbusMsg;
        /// Try dispatch the method entry to make sure it match the service registration
	if ( DbusProcessMethod(method_entry) )
            return DBUS_HANDLER_RESULT_HANDLED;
        
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}



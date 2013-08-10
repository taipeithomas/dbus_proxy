#ifndef __DBUS_PROXY_BASE_H__
#define __DBUS_PROXY_BASE_H__



//define the interface
#define DBUS_SIGNALINTERFACE     "Dbusproxy.Signal"
#define DBUS_METHODINTERFACE     "Dbusproxy.Method"

#define DBUS_MEMBERNAME_PREFIX   "Dbusproxy_"
#define DBUS_MEMBERNAME_DEFAULT   DBUS_MEMBERNAME_PREFIX "DefaultMember"

#define DBUS_BUSNAME_PREFIX      "Dbusproxy"

#ifndef __cplusplus
//bool is not a base type in C
typedef enum
{
    FALSE = 0,
    TRUE = 1,
} BOOL;

#endif

//RET CODE OF DBUS_PROXY
typedef enum
{
    // the function exec successful.
    DBUS_PROXY_RET_SUCCESS,
    //the function failed.
    DBUS_PROXY_RET_FAILED,
    //the argument is not valid. 
    DBUS_PROXY_RET_INVALID_ARGUMENT, 
    DBUS_PROXY_RET_INSUFFICIENT_MEMORY, 
    DBUS_PROXY_RET_DAEMON_ERROR, 
    DBUS_PROXY_RET_ALREADY_BUSOWNER, 
    DBUS_PROXY_RET_DUPLICATE_REGISTERED, 
    DBUS_PROXY_RET_METHODCALL_TIMEOUT, 
    DBUS_PROXY_RET_METHODCALL_ERROR,
    DBUS_PROXY_RET_CONNECTION_ERROR,
    DBUS_PROXY_RET_LISTENER_ERROR,
    DBUS_PROXY_RET_UNKNOWN,
}DBUS_PROXY_RET_T;


typedef struct DBUS_PROXY_SIGNAL_DATA
{
    const char *szSignalName;

    const char *szSenderName;

    const void *pSignalContent;

    int nContentSize;
    
} DBUS_PROXY_SIGNAL_DATA_T;


typedef struct DBUS_PROXY_METHOD_DATA
{
    const char *szMethodName;
    
    const char *szCallerName;

    const void *pMethodParameters;

    int nParameterSize;

} DBUS_PROXY_METHOD_DATA_T;


typedef struct DBUS_PROXY_METHOD_CALLBACK_RET_VALUE
{
    const char *pCallbackRetValue;
    
    int nCallbackRetValueSize;
    
} DBUS_PROXY_METHOD_CALLBACK_RET_VALUE_T;

typedef void (*DBUS_PROXY_SIGNAL_CALLBACK)(const DBUS_PROXY_SIGNAL_DATA_T signal_data, void *pContext);

typedef DBUS_PROXY_METHOD_CALLBACK_RET_VALUE_T (*DBUS_PROXY_METHOD_CALLBACK)(const DBUS_PROXY_METHOD_DATA_T method_data, void *pContext);

#endif //__DBUS_PROXY_BASE_H__

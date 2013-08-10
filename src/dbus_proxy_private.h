#ifndef __DBUS_PROXY_PRIVATE_H__
#define __DBUS_PROXY_PRIVATE_H__

#include <dbus_proxy_base.h>

typedef struct regSingal
{
	std::string strSignalName;
	void* pfnSignalCallback;
	void* pContext;
	
} regSignal_t;

typedef struct regMethod
{
	std::string strMethodName;
	void* pfnMethodCallback;
	void* pContext;
} regMethod_t;


typedef struct DBUS_PROXY_SIGNAL_DATA_INTER
{
    std::string strSignalName;

    std::string strSenderName;

    std::string strSignalContent;

    int nContentSize;
    
} DBUS_PROXY_SIGNAL_DATA_INTER_T;

typedef struct DBUS_PROXY_SIGNAL_ENTRY
{
	DBUS_PROXY_SIGNAL_DATA_INTER_T signalData;
	DBUS_PROXY_SIGNAL_CALLBACK pfnSignalCallback;
	void* pContext; 
} DBUS_PROXY_SIGNAL_ENTRY_T;



typedef struct DBUS_PROXY_METHOD_DATA_INTER
{
    std::string strMethodName;
    std::string strCallerName;
    std::string strMethodParameters;
    int nParameterSize;
}DBUS_PROXY_METHOD_DATA_INTER_T;

typedef struct DBUS_PROXY_METHOD_ENTRY
{
	DBUS_PROXY_METHOD_DATA_INTER_T methodData;
	DBusMessage * pDbusMsg;
	DBUS_PROXY_METHOD_CALLBACK pfnMethodCallback;
	void* pContext;
} DBUS_PROXY_METHOD_ENTRY_T;

#endif

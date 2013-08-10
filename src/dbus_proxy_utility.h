#ifndef __DBUS_PROXY_UTILITY_H__
#define __DBUS_PROXY_UTILITY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>
#include <queue>
#include <dbus/dbus.h>

#include "dbus_proxy.h"
#include "dbus_proxy_private.h"


extern DBusConnection* pDbusConn; 
extern pthread_mutex_t mutex4Conn;

extern bool bListenerThreadAlive;
extern pthread_mutex_t mutex4ListenerThread;

extern std::string strBusName;
extern pthread_mutex_t mutex4Busname;

extern bool bAllowReplaceBusName;
extern pthread_mutex_t mutex4ReplaceBusName;

extern std::list<regSignal_t> regSignalList;
extern pthread_mutex_t mutex4RegSigList;

extern std::list<regMethod_t> regMethodList;
extern pthread_mutex_t mutex4RegMetList;



extern pthread_mutex_t mutex4SignalQueue;



extern pthread_mutex_t mutex4MethodQueue;

extern pthread_mutex_t mutex4SignalThreadStarted;
extern pthread_mutex_t mutex4MethodThreadStarted;

extern DBUS_PROXY_RET_T dbus_proxy_init_conn();
extern void dbus_proxy_close_conn();
extern DBUS_PROXY_RET_T dbus_proxy_init_listener_thread();

//------------------------------------------------------------

extern const int nDbusMethodTimeoutMillisecond;
extern const int nSignalThreadPoolCapacity;
extern const int nMethodThreadPoolCapacity;
extern int nSignalThreadPoolDepth;
extern int nMethodThreadPoolDepth;
extern int nSignalThreadStarted;
extern int nMethodThreadStarted;

std::string  DbusGetValidObjectPath(const char* szInput);

std::string  DbusGetValidMemberName(const char* szInput);

std::string  DbusGetValidBusName(const char* szInput);

bool  CheckDbusError(DBusError *pError, const std::string& strText);


DBUS_PROXY_SIGNAL_DATA_T CastSignalData(const DBUS_PROXY_SIGNAL_DATA_INTER_T& signalDataInter);

DBUS_PROXY_METHOD_DATA_T CastSignalData(const DBUS_PROXY_METHOD_DATA_INTER_T& methodDataInter);

bool isCanceledSignal(const DBUS_PROXY_SIGNAL_ENTRY_T& signal_entry);

bool isCanceledMethod(const DBUS_PROXY_METHOD_ENTRY_T& signal_entry);

void *SignalThreadProc(void * data);

void LaunchSignalProcessThread(void);

void *MethodThreadProc(void * data);

void LaunchMethodProcessThread(void);

void *DbusListenThreadProc(void * data);

bool DbusProcessSignal(DBUS_PROXY_SIGNAL_ENTRY_T& signal_entry);

bool DbusProcessMethod(DBUS_PROXY_METHOD_ENTRY_T& method_entry);

DBusHandlerResult  DbusMessageFilter(DBusConnection *connection, DBusMessage *pDbusMsg, void *user_data);





#endif //__DBUS_PROXY_UTILITY_H__

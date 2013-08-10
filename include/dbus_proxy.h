#ifndef __DBUS_PROXY_H__
#define __DBUS_PROXY_H__
//export the C interface.

#ifdef __cplusplus
#define BOOL bool  
extern "C"
{
#endif
    
#include "dbus_proxy_base.h"
    /*!
     * \brief
     * return the message string of eReturnCode..
     * 
     * \param eReturnCode
     * every dbus_proxy interface would return a errorCode to identify what error happened.

     * \returns
     * return the pointer of the message text.

     * \remarks
     * suggest that invoke this function after call the dbus_proxy function.
     * 
     */
    char * dbus_proxy_GetErrorMessage(DBUS_PROXY_RET_T eReturnCode);
        /*!
     * \brief
     * Send a simple signal message.
     * 
     * \param szSenderName
     * Reference to a string indicating what entity (Process/Thread/module) is sending this signal.
     * This name is only for informational purpose, therefore, has no impact/control towards the actual signal delivery.
     * 
     * \param szSignalName
     * Reference to a string object indicating the name of this Signal, which can be used by any register
     * who is interested in this signal.  Any other Process/Thread/Module in the system may register to this signal
     * by name beforehand, then will receive notification via Callback function everytime the signal with this name is sent. <br>
     * The signal name would be converted into a valid "Signal" name based on D-Bus Specification if not already so, 
     * which is then presented to all registers of the signal upon delivery.
     * 
     * \param pSignalContent
     * point to a object indicating an arbitrary length of binary payload to be sent with this Signal.
     * This can be the the serialized data of any information, which can be either empty.
     *
     * \returns
     * DBUS_PROXY_RET_T you can get the error message text through the dbus_proxy_GetErrorMessage function.
     * 
     * Applications may use this API at the sender (aka client) side of Inter-Process Communication (IPC) 
     * by specifying the Signal name and an optional payload. A non-empty module name is also necessary to identify the sender.
     * The receiving side needs to register the same signal name beforehand, then is guaranteed 
     * to receive the notificaiton of this signal with its paylaod and some other auxillary information. <br>
     * No intialization or clean up is necessary for the sender of IPC.
     * 
     * \remarks
     * It's guaranteed by D-Bus mechanisms that all signals are delivered in order, therefore,
     * all signal callback invocations are scheduled within the same order.
     * However, DbusAdapter utilizes a fixed-size thread-pool to invoke callback function(s), 
     * which may be scheduled out of order by Operation System.  Therefore, 
     * callback function may encounter, in rare occassions, out-of-order serial_number for 
     * the same signal name, even if all signals are sent by the same Process.
     * (The thread-pool initialized by one.)
     * 
     * \see
     * dbus_proxy_signal_reg
     */
    
    DBUS_PROXY_RET_T dbus_proxy_signal_emit(const char * szSenderName, const char * szSignalName, const void * pSignalContent, int iContentSize);
    /*!
     * \brief
     * register a simple signal message, by associating the signal name with 
     * a callback function pointer and an optional context pointer.
     * 
     * \param szSignalName
     * Reference to a string indicating the Signal name of interest.  <br>
     * The signal name would be converted into a valid "Signal" name based on D-Bus Specification if not already so, 
     * which is then used to filter signals being sent to D-Bus. <br>
     * 
     * \param pfnSignalCallback
     * The callback function pointer to be associated with the signal name.  This can not be NULL.
     *
     * \param pContext
     * The optional context pointer to be used when invoking the callback function associated with any matching signal, 
     * in case the same callback function is used (shared) by multiple registered and requires differentiation.
     *
     * \returns
     * DBUS_PROXY_RET_T you can get the error message text through the dbus_proxy_GetErrorMessage function.
     * 
     * Applications may use this API at the receiver (aka server) side of Inter-Process Communication (IPC),  
     * by specifying the Signal name of interest and the callback function pointer.
     
     * It's the responsibility of callback function to interpret the context if not NULL.  <br>
     
     * Upon succesful completion, any future signal by the same name sent from any Process/Thread/Module 
     * in the system is guaranteed to be delivered via callback function 'pfnSignalCallback'. <br>
     * register may be cleaned up by corresponding dbus_proxy_signal_unreg() call.
     * 
     * \remarks
     * In common practice, Applications may use the address of a user-defined object as context pointer. 
     * With this approach, the callback function casts the context pointer back into object pointer, then dereference it accordingly.
     * 
     * \see
     * dbus_proxy_signal_emit | dbus_proxy_signal_unreg
     */
    DBUS_PROXY_RET_T dbus_proxy_signal_reg(const char* szSignalName, DBUS_PROXY_SIGNAL_CALLBACK pfnSignalCallback, void * pContext);

    /*!
     * \brief
     * unregister a simple signal message, by matching signal name
     * 
     * \param szSignalName
     * Reference to a string indicating the Signal name of interest.  
     * The signal name would be converted into a valid "Signal" name based on D-Bus Specification if not already so, 
     * which is then used to match with existing registered.<br>
     * 
     *
     * \returns
     * DBUS_PROXY_RET_T you can get the error message text through the dbus_proxy_GetErrorMessage function.
     * 
     * Applications may use this API at the receiver (aka server) side of Inter-Process Communication (IPC) 
     * to un-register signal previously subscribed. Upon succesful completion, 
     * 
     * \remarks
     * Keep in mind, though, that all entities (thread/module) inside a Process share the same dbus_proxy resources.
     * That means, it's possible for one entity to (either deliberatly or unintentionally) "un-register" 
     * the register(s) belonging to other entities.
     * 
     * \see
     * dbus_proxy_signal_reg
     */
    DBUS_PROXY_RET_T dbus_proxy_signal_unreg(const char* szSignalName);
        /*!
     * \brief
     * Request the bus name to be associated with current process.
     * 
     * \param szBusName
     * Reference to a string object indicating what bus name the current process wants. <br>
     * The content must obey D-Bus specification, which is in the form of "element1.element2....elementN", 
     * must have at least one '.', namely two elements minimum. 
     * Each element must be non-empty (i.e., no ".." anywhere"), has only characters of "[A-Z][a-z][0-9]_-",
     * and cannot start with a digit.  The maximum length of a bus name is 255. <br>
     * If API users provide an invalid bus name, the path is converted into a valid form, 
     * which is then used by D-Bus API dbus_bus_request_name().
     * 
     * \param bAllowReplacement
     * The boolean flag to indicate that once the bus name is successfully requested, whether or not it's possible for 
     * another process requesting the same bus name to replace the current bus-name association.
     * use "FALSE", which is appropriate if at most one instance of the application is expected to exist in the system.
     * On the other hand, use "TRUE" if multiple instances of the same application can co-exist, and only one
     * (the latest started) instance is expected to maintain the bus name (providing method call servce, etc.).
     *
     * \returns
     * DBUS_PROXY_RET_T you can get the error message text through the dbus_proxy_GetErrorMessage function.
     * 
     * Applications may use this API to request for a designated bus name, which is necessary if it supports
     * service via method call dbus_proxy_method_reg().
     * The clients of such service method need to speicify the 
     * "bus name" as well as the "method name" to invoke the corresponding method.
     * 
     * \remarks
     * Depends on how application was designed, the value of 'bAllowReplacement' should be used accordingly.
     * "FALSE" value ensures that the current process cannot be FORCED out of the bus name it requested
     * if another application requests the same name.  Use "TRUE" if it expects the requested bus name to be replaced,
     * for example, mutliple instances of the same application may run concurrently with whichever instance requested the latest wins.
     * 
     * \see
     *  dbus_proxy_method_call | dbus_proxy_method_reg
     */

    DBUS_PROXY_RET_T dbus_proxy_request_busname(const char * szBusName, BOOL bAllowReplacement);

    /*!
     * \brief
     * Invoke a method call service.
     * 
     * \param szCallerName
     * Reference to a string object indicating what entity (Process/Thread/module) is calling this method.
     * The caller name would be converted into a valid "Object Path" name based on D-Bus Specification if not already so, 
     * which is then presented to the method service provider upon invocation.
     * This name is only for informational purpose, therefore, has no impact/control towards the actual method invocation.
     * 
     * \param szServiceBusName
     * Reference to a string object indicating what bus name the method service provider is located at. <br>
     * The content must obey D-Bus specification, which is in the form of "element1.element2....elementN", 
     * must have at least one '.', namely two elements minimum. 
     * Each element must be non-empty (i.e., no ".." anywhere"), has only characters of "[A-Z][a-z][0-9]_-",
     * and cannot start with a digit.  The maximum length of a bus name is 255. <br>
     * The name would be converted into a valid "bus name" based on D-Bus Specification if not already so, 
     * which is then used to locate the method call service provider.
     * 
     * \param szMethodName
     * Reference to a string object indicating the name of the method, which can be used by any caller
     * who is interested in the service of this method call.  Any other Process/Thread/Module in the system 
     * may invoke the method by name, and expect to receive the result data upon successful return. <br>
     * The method name would be converted into a valid "Method" name based on D-Bus Specification if not already so, 
     * which is then presented to the callback function registered by the service provider.
     *
     * \param pParameters
     * Reference to a  object indicating an arbitrary length of parameter data to be sent with this method call.
     * This can be the the serialized data of any information.
     * \param iParameterSize
     *
     * The length of the parameters.
     *
     * \param szOutputData
     * Reference to a  object to retrieve the result data of the method upon successful return.
     * This can be the the serialized data of any information.
     * \param iOutputDataSize
     * Reference to a integer to receive the length of szOutpuData.

     * \returns
     * DBUS_PROXY_RET_T you can get the error message text through the dbus_proxy_GetErrorMessage function.

     * Applications may use this API at the client side of Inter-Process Communication (IPC), 
     * by invoking a method service provided at designated bus name, with optional parameters and receives optional return data.
     * The same method name must be registered by another (or the same) process with the specified bus name, 
     * otherwise the API returns error.
     * 
     * \remarks
     * If the method service takes too long to finish and return, the method caller 
     * may time-out (after 5 seconds) and return timeout error.
     * 
     * \see
     * dbus_proxy_request_busname | dbus_proxy_method_reg
     */

    DBUS_PROXY_RET_T dbus_proxy_method_call(const char * szCallerName, const char * szServiceBusName, const char * szMethodName,const void * pParameters, int iParameterSize, char *szOutputData, int *iOutputDataSize);

        /*!
     * \brief
     * Register a method servcie, by associating the method name with 
     * a callback function pointer and an optional context pointer.
     * 
     * \param szMethodName
     * Reference to a string object indicating the Method name of interest.  <br>
     * The method name would be converted into a valid "method" name based on D-Bus Specification if not already so, 
     * which is then used to filter method calls presented to D-Bus.
     * 
     * \param pfnMethodCallback
     * The callback function pointer to be associated with the method name.  This can not be NULL.
     *
     * \param pContext
     * The optional context pointer to be used when invoking the callback function associated with any matching method name, 
     * in case the same callback function is used (shared) by multiple method registrations and requires differentiation.
     *
     * \returns
     * DBUS_PROXY_RET_T you can get the error message text through the dbus_proxy_GetErrorMessage function.
     * 
     * Applications may use this API at the server side of Inter-Process Communication (IPC), 
     * by specifying the Method name of interest and the callback function pointer.
     * If a method with the same name has already been registered in current process.
     * it would be return a duplicated register error.
     * 
     * \remarks
     * The callback function must be thread-safe, since it's likely that multiple clients invoke the same method call
     * around the same time.
     * 
     * \see
     * dbus_proxy_method_call | dbus_proxy_method_unreg
     */

    DBUS_PROXY_RET_T dbus_proxy_method_reg(const char* szMethodName,DBUS_PROXY_METHOD_CALLBACK pfnMethodCallback, void * pContext);
    
        /*!
     * \brief
     * UnRegister a method servcie, by matching method name.
     * 
     * \param szMethodName
     * Reference to a string object indicating the Method name of interest.  <br>
     * The method name would be converted into a valid "method" name based on D-Bus Specification if not already so, 
     * which is then used to match with existing registrtion.
     * 
     * \returns
     * DBUS_PROXY_RET_T you can get the error message text through the dbus_proxy_GetErrorMessage function.
     * 
     * Applications may use this API at the receiver (aka server) side of Inter-Process Communication (IPC) 
     * to un-register method previously registered. Upon succesful completion, the method service become
     * un-avaiable if any caller tries invoke the same method name. <br>
     * 
     * 
     * \see
     * dbus_proxy_method_call | dbus_proxy_method_unreg
     */
    DBUS_PROXY_RET_T dbus_proxy_method_unreg(const char* szMethodName);


#ifdef __cplusplus
}
#endif

#endif

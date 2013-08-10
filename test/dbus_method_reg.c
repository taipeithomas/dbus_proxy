#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus_proxy.h>

#define DBUS_PROXY_BUS_NAME "plustest.plus"

typedef struct argu
{
    int a;
    int b;
}argu_t;

int temp;
DBUS_PROXY_METHOD_CALLBACK_RET_VALUE_T remote_plus_callback(const DBUS_PROXY_METHOD_DATA_T method_data, void * pContext)
{

    DBUS_PROXY_METHOD_CALLBACK_RET_VALUE_T result;
    argu_t *parameters;
    
    parameters = (argu_t *)method_data.pMethodParameters;
    printf("received parameters, a = %d, b = %d\n", parameters->a, parameters->b);
    temp = parameters->a + parameters->b;
    
    result.pCallbackRetValue = (void *)&temp;
    result.nCallbackRetValueSize = sizeof(int);

    printf("result of remot call is %d\n", temp);
   
    return result;
}

int main(int argc, char *argv[])
{
    DBUS_PROXY_RET_T eReturnCode;
    eReturnCode = dbus_proxy_request_busname(DBUS_PROXY_BUS_NAME, FALSE);
    
    if ( eReturnCode != DBUS_PROXY_RET_SUCCESS )
    {
            printf("A error occured: %s\n", dbus_proxy_GetErrorMessage(eReturnCode));
            return 0;
    }
    
    eReturnCode = dbus_proxy_method_reg("remote_plus", remote_plus_callback, NULL);
    
    if ( eReturnCode != DBUS_PROXY_RET_SUCCESS )
    {
            printf("A error occured: %s\n", dbus_proxy_GetErrorMessage(eReturnCode));
            return 0;
    }
    printf("Press Ctrl+C to exit ...\n");
    while (1) sleep(1);
    
    return 0;
}

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus_proxy.h>

#define DBUS_PROXY_BUS_NAME  "plustest.plus"

typedef struct argu
{
    int a;
    int b;
}argu_t;



int main(int argc, char *argv[])
{
    int i,j;
    argu_t parameters;
    int nRsize ;
    DBUS_PROXY_RET_T eReturnCode;
    char *result ;

    if (argv[1] == NULL || argv[2] == NULL)
    {
        printf("Usage:./dbus_method_call [num] [num].\n" \
               "example:./dbus_method_call 10 10.\n");
        exit(0);
    }
    #if 1
    for ( i = 0; i < atoi(argv[1]); i++)
    {
        parameters.a = i;
        for ( j = 0; j < atoi(argv[2]); j++)
        {
            parameters.b = j;
	    result = (char*)malloc(sizeof(int));
            eReturnCode = dbus_proxy_method_call("Caller", DBUS_PROXY_BUS_NAME, \
                                                 "remote_plus", &parameters, sizeof(parameters),\
                                                 result, &nRsize);
            
            if ( eReturnCode != DBUS_PROXY_RET_SUCCESS ) 
                printf("A error occured: %s\n", dbus_proxy_GetErrorMessage(eReturnCode));
            else
		{
		int* retvalue = (int *)result;
		printf("%2d, ", *retvalue);
		}
		free(result);
        }
        printf("\n");
    }
    #endif
    return 0;
}

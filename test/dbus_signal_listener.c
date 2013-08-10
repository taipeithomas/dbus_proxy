#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <dbus_proxy.h>

typedef struct aa
{
    int a;
    int b;
    int c;
}aa_t;
typedef struct student_info
{
    int id;
    char name[32];
    int score;
    aa_t aa;
} student_info_t;

void dbus_siganl_callback(const DBUS_PROXY_SIGNAL_DATA_T signal_data, void * pContext)
{
    static int i = 0;
    printf("signal callback called times = %d\n", ++i);
    student_info_t *ss;
    ss = (student_info_t *)signal_data.pSignalContent;
    printf("Received Signal Content: %d, %s, %d\n", ss->id, ss->name,ss->score);
    printf("%d,%d,%d\n", ss->aa.a, ss->aa.b, ss->aa.c);
    //sleep(5);
}

int main (int argc, char* argv[])
{
    int i = 0;
    if ( argc <= 1 || strcasecmp(argv[1], "-h") == 0 || strcasecmp(argv[1], "help") == 0 )
    {// print out help information
        // find out the executable file name (without PATH) from the command line
        char *pExecutableName = strrchr(argv[0], '/');
        if ( pExecutableName == NULL )  pExecutableName = argv[0];
        else pExecutableName++;
        printf("'%s' monitors the signals sent out via DbusAdapter, specified by signal names\n", pExecutableName);
        printf("Usage:\t%s <signal name> [<signal name> ...] [-h] \n\n", pExecutableName);
    }
    else
    {// subscribe with each signal names.
        for ( i=1; i<argc; i++ )
        {
            DBUS_PROXY_RET_T eReturnCode = dbus_proxy_signal_reg(argv[i], dbus_siganl_callback, NULL);
            if ( eReturnCode != DBUS_PROXY_RET_SUCCESS ) 
                printf("For signal '%s': %s\n", argv[i], dbus_proxy_GetErrorMessage(eReturnCode));
        }
        printf("Press Ctrl+C to exit ...\n");
        while (1)   sleep(1);
    }
    return 0;
}


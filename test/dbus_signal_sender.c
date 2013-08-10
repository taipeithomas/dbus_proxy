#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


int main(int argc, char * argv[])
{
    struct timeval tpstart,tpend;
    student_info_t student;
    int i, j;
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        printf("Usage:./Dbus_signal_sender [num] [string] [num] [num]\n" \
               "example:./Dbus_signal_sender 100 \"test\" 100 100\n");
        exit(0);
    }
    
    student.id = atoi(argv[1]);
    strcpy(student.name, argv[2]);
    student.score = atoi(argv[3]);
    j = atoi(argv[4]);

    float timeuse; 
    gettimeofday(&tpstart,NULL); 
    float datacount = 0;

    
    
    for (i = 0; i < j; i++)
    {
        
        student.score = student.score + 1;
        student.aa.a = i;
        student.aa.b = i;
        student.aa.c = i;
        DBUS_PROXY_RET_T eReturnCode = dbus_proxy_signal_emit("Sender", "Signal", &student, sizeof(student));
        if ( eReturnCode != DBUS_PROXY_RET_SUCCESS ) 
            printf("For signal %s\n", dbus_proxy_GetErrorMessage(eReturnCode));
        
        eReturnCode = dbus_proxy_signal_emit("Sender", "Signal2", &student, sizeof(student));
        datacount += sizeof(student);
        if ( eReturnCode != DBUS_PROXY_RET_SUCCESS ) 
            printf("For signal %s\n", dbus_proxy_GetErrorMessage(eReturnCode));
    }
    gettimeofday(&tpend,NULL); 
    timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec; 
    timeuse/=1000000; 
    printf("Send used Time:%f, Send data size = %f KByte.\n",timeuse, datacount / 1024.0); 

    return 0;
}

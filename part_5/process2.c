#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extra_file.h"


const true = 1;

int main()
{
    pid_t pid;
    char *message;
    int n;
    printf("process2 program starting\n");

    message = "(process 2): Message 2";

    char buffer[256];

    int times = 4;

    char *check = "2";

    int userinput;

    int shmid = getShmId();

    char *shrbuffer;

    shrbuffer = (char *) attachToSharedMemory(shmid);

    int test;

    printf("it works");

    int semID = getSemId();

    while(times > 0){

        SemaphoreWait(semID, BLOCK);

        if(strncmp(shrbuffer,"x",1)== 0){
            break;
        }

        //fgets(shrbuffer, 256, stdin);

        userinput = atoi(shrbuffer);

        //puts(buffer);

        if(userinput != 0 && userinput == 2){
            puts(message);
            //times--;

            SemaphoreSignal(semID);
        }else if(strncmp(shrbuffer,"x",1)== 0){

            SemaphoreSignal(semID);

            break;
        }else{
            
            SemaphoreSignal(semID);

        }
    }
    
    detachSharedMemory(shrbuffer);

    exit(0);
}
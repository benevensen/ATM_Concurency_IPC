#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "extra_file.h"

int main()
{
    pid_t pid;
    char *message;
    int n;
    //printf("fork program starting\n");
    
    pid = fork();
    message = "fork program starting";
    puts(message);

    n = 5;
    switch (pid)
    {
    case -1:
        perror("fork failed");
        exit(1);
    case 0: //child process

        execlp("./process2","", NULL);

        break;
    default: //parent process

        //wait(NULL); //waits for child to end
        //message = "forking program ended";
        //puts(message);
        //n = 3;
        break;
    }

    message = "(Process 1): message 1";

    int times = 4;
    char *shrbuffer;
    int userinput;

    int shmid = getShmId();

    //sharedVar* var = (sharedVar *) attachToSharedMemory();

    shrbuffer = (char *) attachToSharedMemory(shmid);

    int semID = getSemId();

    semInit(semID);

    while(times > 0){

        SemaphoreWait(semID, BLOCK);

        fgets(shrbuffer, 256, stdin);

        userinput = atoi(shrbuffer);

        if(userinput != 0 && userinput == 1){
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

    wait(NULL);

    detachSharedMemory(shrbuffer);

    deleteSharedMemory(shmid);

    semDelete(semID);


    exit(0);
}
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "extra_file.h"


void start_child_process(const char * programFilePath, int msgID);

int main(int argc, char *argv[])
{
    pid_t pid;
    char *message;
    int n;
    //printf("fork program starting\n");
    
    message = "Process1 starting";
    puts("Process1 starting\n");
   


    int times = 4;
    char *shrbuffer;
    //int userinput;

    //int shmid = getShmId();

    int msgid = getmsgQueueID();
    int semID = getSemId();

    start_child_process("./Process2", msgid);
    start_child_process("./Process3", msgid);


    printf("(Process 1): msgId is %d \n", msgid);

    semInit(semID);

    int x = 0;

    my_message sendingMessage;

    my_message receivingMessage;

    char userinput[10];

    while(1){

        puts("Press x to quit or anything to continue\n");
        fgets(userinput, 10, stdin);

        /* printf(&c);
        printf("%s", &c); */

        x++;

        printf("(Process 1): %d \n", x);

        sendingMessage.data = x;
        sendingMessage.message_type = (long) 2;

        sendMessage(msgid, sendingMessage, NOBLOCK);

        receivingMessage.message_type = 1;
        receiveMessage(msgid, &receivingMessage, BLOCK);

        if(strncmp(userinput,"x",1) == 0){

            sendingMessage.data = -1;
            sendingMessage.message_type = (long) 2;

            sendMessage(msgid, sendingMessage, NOBLOCK);

            break;
        }
        
        /* 
        //SemaphoreWait(semID, BLOCK);

        fgets(shrbuffer, 256, stdin);

        userinput = atoi(shrbuffer);
        if(userinput != 0 && userinput == 1){
            puts(message);
            //times--;

        //    SemaphoreSignal(semID);
        }else if(strncmp(shrbuffer,"x",1)== 0){
            
        //    SemaphoreSignal(semID);
            break;
        }else{
            
        //    SemaphoreSignal(semID);

        } */

    } 

    wait(NULL);

    //detachSharedMemory(shrbuffer);

    //deleteSharedMemory(shmid);

    semDelete(semID);

    deleteMessageQueue(msgid);

    exit(0);
}

void start_child_process(const char * programFilePath, int msgID){

    pid_t pid;

    pid = fork();

    if(pid < 0){
        perror("fork failed");
        exit(1);
    }else if (pid == 0){ //child process

        char numArg[8];

        sprintf(numArg, "%d", msgKey);

        execlp(programFilePath,numArg, NULL);

    }

}
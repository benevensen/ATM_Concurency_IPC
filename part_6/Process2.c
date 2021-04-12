#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extra_file.h"


const true = 1;

int main(int argc, char *argv[])
{

    int msgId;

    if(argc > 1 ){
        msgId = atoi(argv[1]);
    }else{
        msgId = getmsgQueueID();
    }


   // pid_t pid;
    char *message;
   // int n;
    printf("(process 2): program starting\n");

    message = "(process 2): Message 2";

  //  char buffer[256];

   // int times = 4;

    //char *check = "2";

   // int userinput;

    //int shmid = getShmId();

    //char *shrbuffer;

    //shrbuffer = (char *) attachToSharedMemory(shmid);

    //int test;

    //printf("it works");

    //int semID = getSemId();

    printf("(Process 2): msgId is %d \n", msgId);

    
    my_message sendingMessage;

    my_message receivingMessage;

    char c;

    int current_x;

    int times = 1;

    //assignment stated to multiple the received number by 2 and print the total
    // I will assume that the "total" is a typo and is meant to be the product
    while(times == 1){

        //puts("Press x to quit or anything to continue\n");
        //c = getchar();

        //x++;

        puts("(Process 2) trying to receive\n");
        //printf("(Process 2) trying to receive");
        receivingMessage.message_type = 2;
        receiveMessage(msgId, &receivingMessage, BLOCK);

        puts("(Process 2) received message\n");

        if(receivingMessage.data == -1){

            sendingMessage.data = -1;
            sendingMessage.message_type = (long) 3;

            sendMessage(msgId, sendingMessage, NOBLOCK);

            break;
        }

        current_x = receivingMessage.data;

        current_x *= 2;

        printf("(Process 2): %d\n ", current_x);

        sendingMessage.data = current_x;
        sendingMessage.message_type = (long) 3;

        sendMessage(msgId, sendingMessage, NOBLOCK);



    }



        /* 
    while(1){
        SemaphoreWait(semID, BLOCK);

        if(strncmp(shrbuffer,"x",1)== 0){
            break;
        }

        //fgets(shrbuffer, 256, stdin);

       // userinput = atoi(shrbuffer);

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
        } */
    
   // detachSharedMemory(shrbuffer);

    exit(0);
}
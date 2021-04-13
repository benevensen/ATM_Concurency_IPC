#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "extra_file.h"
#include "DBserver.h"


void start_child_process(const char * programFilePath, int msgID);

int main(int argc, char *argv[])
{

    int times = 4;
    Mailbox *mailbox;
    int userinput;

    if( argc == 1 ){

    }


    int msgID = getmsgQueueID();

    int x = 0;

    int counter = 0;

    GenericMessage sendingMessage;
    GenericMessage receivingMessage;

    sendingMessage.message_type = clientMessageType;
    receivingMessage.message_type = serverMessageType;


    resetDataBundle(&sendingMessage.data);
    resetDataBundle(&receivingMessage.data);


    while(times > 0){

        //SemaphoreWait(shmid, BLOCK);
        resetDataBundle(&receivingMessage.data);

        receiveMessage(msgID, &receivingMessage, BLOCK);

        //SemaphoreWaitUntilZeroNthSemInSet(semID, mailboxSemaphore);

        puts("----------------------------------------------------");

        puts("finished waiting");

        // if error or exit command then exit the loop
        if(receivingMessage.data.response == -1){

            //SemaphoreSignalNthSemInSet(semID, mailboxSemaphore);
            sendingMessage.data.response = -1;

            sendMessage(msgID, sendingMessage, NOBLOCK);
            sendMessage(msgID, sendingMessage, NOBLOCK);

            break;
        }

        int message_received_type = receivingMessage.data.type.responseType;

        printMessageType( (MessageType) message_received_type );

        x++;

        resetDataBundle(&sendingMessage.data);

        sendingMessage.data.response = x;

        puts("sent something to the client");

        printf("x is currently %d \n", x);

        sendMessage(msgID, sendingMessage, NOBLOCK);

        puts("sent a message outwards");



    }

    wait(NULL);



    if( argc == 1 ){

        /*  semDeleteNthSemInSet(semID , clientSemaphore);
         semDeleteNthSemInSet(semID , mailboxSemaphore); */

        deleteMessageQueue(msgID);
    }

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
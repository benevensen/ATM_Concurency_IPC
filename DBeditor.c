#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "extra_file.h"

#include "DBserver.h"

/**
 * Function that gets user input in the form of an integer. Returns -1 if there is an error, otherwise returns an integer. 
 * 
 * paramater is a char array which will be displayed to the user.
 */
int getUserInput(char *message){
    char buffer[10];
    int result;

    puts(message);

    fgets(buffer,10,stdin);

    result = atoi(buffer);

    if((result == -1)){
        return -1;
    }

    return result;

};

/**
 * Function that gets user input in the form of an float. Returns -1 if there is an error, otherwise returns an integer. 
 * 
 * paramater is a char array which will be displayed to the user.
 */
float getUserInputFloat(char *message){
    char buffer[20];
    float result;

    puts(message);

    fgets(buffer,20,stdin);

    result = atof(buffer);

    if(( result == -1)){
        return -1;
    }

    return result;
    
};


int main(int argc, char *argv[])
{

    int semID = getSemId(clientSemKey);

    //semInit(semID);
    int msgID = getmsgQueueID();

    start_child_process("./DBserver", 0);   // Forking the child process

    GenericMessage sendingMessage;
    GenericMessage receivingMessage;

    sendingMessage.message_type = serverMessageType;
    receivingMessage.message_type = clientMessageType;

    resetDataBundle(&sendingMessage.data);
    resetDataBundle(&receivingMessage.data);

    //puts("At any time, enter \"x\" to quit the program");

    signal(SIGHUP, exit_program);


//-------------------------------------------------------------------------------------------------------------
// Sign in process to register the client with the server so that the server can shut down the system
// Sign in process also limits the system to only have 1 interest calculator, 5 atms, and 1 DB editor

    //register the client with the server
    SemaphoreWait(semID, BLOCK);

    resetDataBundle(&sendingMessage.data);

    sendingMessage.data.type.message = SIGNIN;
    sendingMessage.data.pid = getpid();
    sendMessage(msgID, sendingMessage, NOBLOCK);           // Sending the SEND struct; contains PID of the current process

    resetDataBundle(&sendingMessage.data);

    receiveMessage(msgID, &receivingMessage, BLOCK);     // Receiving the DB struct; contains response

    SemaphoreSignal(semID);

    if(receivingMessage.data.response == NOSPACE){
        puts("The maximum number of clients has been reached! Quiting program....");
        exit(0);
    }else if(receivingMessage.data.response != OK){
        perror("(DBeditor) Error in sign in process: ");
        exit(0);
    }


//-------------------------------------------------------------------------------------------------------------

    int account_number = -1;
    int pin = -1;
    float funds = -1.0;

    int response;

    while (1)
    {
        //fgets(buffer, 10, stdin);

        account_number = getUserInput("What is the account number?");
        pin = getUserInput("What is the PIN?");
        funds = getUserInputFloat("What is the amount of funds? Please answer with a decimal number");

        if ( account_number == -1 || pin == -1 || funds == -1.0){
            puts("Error in one of the inputs, please enter the information again");
            continue;
        }

        SemaphoreWait(semID, BLOCK );


        resetDataBundle(&sendingMessage.data);

        sendingMessage.data.type.message = UPDATE_DB;

        sendingMessage.data.account.accountNumber = account_number;
        sendingMessage.data.account.pin = pin;
        sendingMessage.data.account.funds = funds;
        sendingMessage.data.account.isLocked = 0;




        sendMessage(msgID, sendingMessage, NOBLOCK);

        

        resetDataBundle(&receivingMessage.data);
        
        receiveMessage(msgID, &receivingMessage, BLOCK);

        response = receivingMessage.data.response;

        if( response == -1){
            break;
        }

        SemaphoreSignal(semID);


    }

    exit(0);
}
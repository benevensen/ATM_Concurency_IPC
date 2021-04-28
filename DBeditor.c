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

    //get client semaphore id
    int semID = getSemId(clientSemKey);

    //get message queue id
    int msgID = getmsgQueueID();

    //variable for flagging if deadlock mode is on
    int currentCase = -1;

    //checks if there is 2 arguments, if there is 2 arguements and the second argument is 1 : the system will attempt to deadlock
    if(argc > 1){
        if(atoi(argv[1]) == 1){
            currentCase = 1;
        }
    }

    //setting up messages for sending and receiving
    GenericMessage sendingMessage;
    GenericMessage receivingMessage;

    sendingMessage.message_type = serverMessageType;
    receivingMessage.message_type = clientMessageType;

    resetDataBundle(&sendingMessage.data);
    resetDataBundle(&receivingMessage.data);

 
    signal(SIGHUP, exit_program);  //registers the SIGHUP signal with the exit_program function as the handler function


//-------------------------------------------------------------------------------------------------------------
// Sign in process to register the client with the server so that the server can shut down the system
// Sign in process also limits the system to only have 1 interest calculator, 5 atms, and 1 DB editor

    if(currentCase == 1){
        printToLogFile("(DBeditor) SIGNIN: acquiring semaphores and sending message");
    }  

    //register the client with the server

    //enter the critical section
    SemaphoreWait(semID, BLOCK);

    resetDataBundle(&sendingMessage.data);

    sendingMessage.data.type.message = SIGNIN;
    sendingMessage.data.pid = getpid();

    sendMessage(msgID, sendingMessage, NOBLOCK);           // Sending the SEND struct; contains PID of the current process

    resetDataBundle(&sendingMessage.data);

    receiveMessage(msgID, &receivingMessage, BLOCK);     // Receiving the DB struct; contains response

    //exit the critical section
    SemaphoreSignal(semID);

    //if deadlock mode, output state to log file
    if(currentCase == 1){
        printToLogFile("(DBeditor) SIGNIN: released semaphores and received message");
    }
    
    //check response from server to see if client signed in properly
    if(receivingMessage.data.response == NOSPACE){
        puts("The maximum number of clients has been reached! Quiting program....");
        exit(0);
    }else if(receivingMessage.data.response != OK){
        perror("(DBeditor) Error in sign in process: ");
        exit(0);
    }


//-------------------------------------------------------------------------------------------------------------

    //initalize some instance variables

    int account_number = -1;
    int pin = -1;
    float funds = -1.0;

    int response;

    //infinite loop
    while (1)
    {
        
        //gets input from user
        account_number = getUserInput("What is the account number?");
        pin = getUserInput("What is the PIN?");
        funds = getUserInputFloat("What is the amount of funds? Please answer with a decimal number");

        //if invalid input, get input again
        if ( account_number == -1 || pin == -1 || funds == -1.0){
            puts("Error in one of the inputs, please enter the information again");
            continue;
        }

        //if deadlock mode, log state to log file
        if(currentCase == 1){
            printToLogFile("(DBeditor): acquiring semaphores and sending message");
        }

        //enter critical section
        SemaphoreWait(semID, BLOCK );

        //reset databundle
        resetDataBundle(&sendingMessage.data);


        // create message for server
        sendingMessage.data.type.message = UPDATE_DB;

        sendingMessage.data.account.accountNumber = account_number;
        sendingMessage.data.account.pin = pin;
        sendingMessage.data.account.funds = funds;
        sendingMessage.data.account.isLocked = 0;

        //send message to server
        sendMessage(msgID, sendingMessage, NOBLOCK);

        resetDataBundle(&receivingMessage.data);
        
        //receive message from server
        receiveMessage(msgID, &receivingMessage, BLOCK);

        response = receivingMessage.data.response;

        //exit critical section
        SemaphoreSignal(semID);

        //if response is -1, end loop
        if( response == -1){
            break;
        }
        
        //if deadlock mode, log state to log file
        if(currentCase == 1){
            printToLogFile("(DBeditor): released semaphores and received message");
        }

    }

    exit(0);
}
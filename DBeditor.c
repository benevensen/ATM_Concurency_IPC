#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extra_file.h"

#include "DBserver.h"

/**
 * Function that gets user input in the form of an integer. Returns -1 if there is an error, otherwise returns an integer. 
 * Special case: if user inputs x, returns -2.
 * 
 * paramater is a char array which will be displayed to the user.
 */
int getUserInput(char *message){
    char buffer[10];
    int result;

    puts(message);

    fgets(buffer,10,stdin);

    if(strncmp(buffer,"x",1) == 0){
        return -2;
    }else if((result = atoi(buffer) == -1)){
        return -1;
    }

    return result;

};

/**
 * Function that gets user input in the form of an float. Returns -1 if there is an error, otherwise returns an integer. 
 * Special case: if user inputs x, returns -2.
 * 
 * paramater is a char array which will be displayed to the user.
 */
float getUserInputFloat(char *message){
    char buffer[10];
    int result;

    puts(message);

    fgets(buffer,10,stdin);

    if(strncmp(buffer,"x",1) == 0){
        return -2;
    }else if((result = atof(buffer) == -1)){
        return -1;
    }

    return result;
    
};


int main(int argc, char *argv[])
{

    int semID = getSemId();

    semInit(semID);

    int msgID = getmsgQueueID();

    GenericMessage sendingMessage;
    GenericMessage receivingMessage;

    sendingMessage.message_type = serverMessageType;
    receivingMessage.message_type = clientMessageType;

    resetDataBundle(&sendingMessage.data);
    resetDataBundle(&receivingMessage.data);

    puts("At any time, enter \"x\" to quit the program");

    int account_number = -1;
    int pin = -1;
    float funds = -1.0;

    int userinput;
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
        puts("got input");

        SemaphoreWait(semID, BLOCK );
        puts("got the client semaphore");


        resetDataBundle(&sendingMessage.data);

        if( account_number == -2 || pin == -2 || funds == -2.0){
            

            sendingMessage.data.response = -1;
            sendMessage(msgID, sendingMessage, NOBLOCK);

            SemaphoreSignal(semID);
            break;
        }

        sendingMessage.data.type.message = UPDATE_DB;

        sendingMessage.data.account.accountNumber = account_number;
        sendingMessage.data.account.pin = pin;
        sendingMessage.data.account.funds = funds;


        sendMessage(msgID, sendingMessage, NOBLOCK);

        
        puts("put something in the mailbox");



        resetDataBundle(&receivingMessage.data);
        receiveMessage(msgID, &receivingMessage, BLOCK);

        response = receivingMessage.data.response;

        if( response == -1){
            break;
        }
        printf("first response was %d \n", response);   

        SemaphoreSignal(semID);

        puts("released the client semaphore");

    }

    exit(0);
}
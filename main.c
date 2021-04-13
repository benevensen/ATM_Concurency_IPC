#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<mqueue.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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


/* ========================== RESOURCE PREREQUISITES & OTHER ========================== */
/*

/*
 * Method to check the equality of two arrays; 1 if equal, 0 otherwise
 * */
int EQUALITY_CHECK(char *ARR1, char *ARR2, int LENGTH){
    for (int i = 0; i < LENGTH; ++i) {      // Iterate through each array argument
        if (ARR1[i] !=  ARR2[i]){
            return 0;                       // They are not equal; return 0
        }
    }
    return 1;                               // They are equal; return 1
}

/*
 * Method to start the ATM system
 * */
void ATM_START() {

    GenericMessage sendingMessage;
    GenericMessage receivingMessage;

    Mailbox *mailbox;

    int msgID = getmsgQueueID();     //
    int attempts = 3;               // How many attempts the user has left
    int status;                     // Represents the status when sending and receiving on the message queue

    int accountNumber = -1;          // User given account number
    int PIN = 1;                    // User given PIN
    int balance = -1.0;

    char response;

    sendingMessage.message_type = clientMessageType;
    receivingMessage.message_type = serverMessageType;

    resetDataBundle(&sendingMessage.data);
    resetDataBundle(&receivingMessage.data);



    printf("<=====================> ATM <=====================>");

    /* Keeping the ATM running */
    do {
        /* Getting the account number and account number from the user*/
        printf("Kindly enter your account number:- %d\n", accountNumber);
        printf("Kindly enter you PIN:- %d\n", PIN);

        /* Making the account number in the struct equal to the user-given account number */
        sendingMessage.data.account.accountNumber = accountNumber;

        /* Making the PIN in the struct equal to the user-given PIN */
        sendingMessage.data.account.pin = PIN;

        status = sendMessage(msgID, sendingMessage, NOBLOCK);           // Sending the SEND struct; contains PIN and account number

        /* If the returned status is -1 there is an error in sending the message to the DB*/
        if (status < 0) {
            printf("Message cannot send");
            exit(EXIT_FAILURE);
        }

        status = receiveMessage(msgID, &receivingMessage, NOBLOCK);     // Receiving the DB struct; contains response

        /* If the returned status is -1 there is an error in receiving the message from the DB */
        if (status < 0) {
            printf("Message cannot be received");
            exit(EXIT_FAILURE);
        }

        response = receivingMessage.data.response;
        /* If we receive the "OK" from the DB we may proceed to selection */
        if (EQUALITY_CHECK(&response, "OK", 2)) {
            char selection[8];                                                                          // The selection (can either be "BALANCE", or "WITHDRAW")

            /* If the user wishes to have multiple requests we can do this with a loop */
            do {
                printf("Kindly enter your service; \"BALANCE\" or \"WITHDRAW\":- %s\n", selection);

                /* If its a BALANCE request */
                if (EQUALITY_CHECK(selection, "BALANCE", 7)) {
                    sendingMessage.message_type = BALANCE;
                    sendMessage(msgID, sendingMessage, NOBLOCK);
                    receiveMessage(msgID, &receivingMessage, NOBLOCK);
                    printf("\nCurrent balance:- $%f", receivingMessage.data.account.funds);
                    break;
                }

                /* If its a WITHDRAW request */
                else if (EQUALITY_CHECK(selection, "WITHDRAW", 8)) {
                    double AMOUNT;                                                                      // The amount to withdraw
                    sendingMessage.message_type = WITHDRAW;
                    printf("\nKindly enter the amount to withdraw (CAD assumed):- ");
                    scanf("%lf", &AMOUNT);
                    sendMessage(msgID, sendingMessage, NOBLOCK);
                    receiveMessage(msgID, &receivingMessage, NOBLOCK);
                    printf("\nCurrent balance:- $%f", receivingMessage.data.account.funds);       // Receives the response from the DB

                    response = receivingMessage.data.response;
                    /* If there is insufficient funds in the account */
                    if (EQUALITY_CHECK(&response, "NSF", 3)) {
                        printf("\nInsufficient funds\n");
                    }
                    /* If there is sufficient funds in the account */
                    else if (EQUALITY_CHECK(&response, "FUNDS_OK", 8)) {
                        printf("\nCurrent balance:- $%f", receivingMessage.data.account.funds);        // Print balance minus withdrawal
                    }
                    break;
                }
            } while (0);
        }

        /* If the user inputs the wrong account or PIN; dock them an attempt */
        else {
            printf("\nIncorrect account or PIN\n");
            attempts -= 1;

            /* If they are out of attempts; exit (what we chose to do) */
            if (attempts <= 0){
                printf("\nAccount is locked\n");
                exit(EXIT_FAILURE);                         // Not specified if should exit
            }
        }
    } while (0);
}


int main() {
    ATM_START();

}


/*
 * REFERENCES:-
 * 1. https://man7.org/linux/man-pages/man3/pthread_create.3.html
 * 2. https://www.geeksforgeeks.org/thread-functions-in-c-c/
 * 3. https://www.thegeekstuff.com/2012/05/c-mutex-examples/
 * 4. https://www.thegeekstuff.com/2012/05/c-mutex-examples/
 * 5. https://gist.github.com/gustavorv86/9e98621b44222114524399a3b4302ddb
 * 6. https://pubs.opengroup.org/onlinepubs/009695399/basedefs/mqueue.h.html
 * */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
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



/* ========================== RESOURCE PREREQUISITES & OTHER ========================== */


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


/*
 * Method to start the ATM system
 * */
void ATM_START() {

    GenericMessage sendingMessage;
    GenericMessage receivingMessage;

    int semID = getSemId();
    semInit(semID);


    int msgID = getmsgQueueID();     // Getting the message queue id
    int attempts = 3;               // How many attempts the user has left
    int status;                     // Represents the status when sending and receiving on the message queue

    start_child_process("./DBserver", msgID);   // Forking the child process

    int accountNumber = -1;          // User given account number
    int accountPIN = 1;              // User given PIN
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
        accountNumber = getUserInput("Kindly enter your account number:\n");
        accountPIN = getUserInput("Kindly enter you PIN:\n");

        //enter the critical section
        SemaphoreWait(semID, BLOCK );

        resetDataBundle(&sendingMessage.data);

        /* Making the account number in the struct equal to the user-given account number */
        sendingMessage.data.account.accountNumber = accountNumber;

        /* Making the PIN in the struct equal to the user-given PIN */
        sendingMessage.data.account.pin = accountPIN;

        sendingMessage.data.type.message = PIN;

        status = sendMessage(msgID, sendingMessage, NOBLOCK);           // Sending the SEND struct; contains PIN and account number
        /* If the returned status is -1 there is an error in sending the message to the DB*/
        if (status < 0) {
            printf("Message cannot send");
            exit(EXIT_FAILURE);
        }

        resetDataBundle(&sendingMessage.data);


        status = receiveMessage(msgID, &receivingMessage, BLOCK);     // Receiving the DB struct; contains response

        /* If the returned status is -1 there is an error in receiving the message from the DB */
        if (status < 0) {
            printf("Message cannot be received");
            exit(EXIT_FAILURE);
        }

        //exit the critical section
        SemaphoreSignal(semID);

        response = receivingMessage.data.response;
        
        /* If we receive the "OK" from the DB we may proceed to selection */
        
        if(response == PIN_WRONG){

            // if the pin or account is wrong, it will request information from the user again
            continue;
        }else if (response == OK) {
            char selection[10];                                                                          // The selection (can either be "BALANCE", or "WITHDRAW")
            int choice = 0;
            /* users will only be able to do 1 banking operation*/ 

            // if the user enters invalid input, the system will keep requesting a proper operation           
            do
            {
                puts("Kindly enter your service; \"BALANCE (b) \" or \"WITHDRAW (w)\":\n");
                fgets(selection,10,stdin);
            } while (strncmp(selection,"b",1) != 0 && strncmp(selection,"w",1));
            
            resetDataBundle(&sendingMessage.data);

            /* If its a BALANCE request */
            if (strncmp(selection,"b",1)) {

                choice = 1;
                sendingMessage.data.type.message = BALANCE;
            }

            /* If its a WITHDRAW request */
            else if (strncmp(selection,"w",1)) {

                choice = 2;

                float AMOUNT;                                                                      // The amount to withdraw
                printf("\nKindly enter the amount to withdraw (CAD assumed):- ");
                scanf("%f", &AMOUNT);

                sendingMessage.data.type.message = WITHDRAW;
                sendingMessage.data.account.funds = AMOUNT;
               
            }

            resetDataBundle(&receivingMessage.data);
            
            //enter the critical section
            SemaphoreWait(semID, BLOCK);

            sendMessage(msgID, sendingMessage, NOBLOCK);
            receiveMessage(msgID, &receivingMessage, BLOCK);

            //enter the critical section
            SemaphoreSignal(semID);


            if(choice == 1){
                printf("\nCurrent balance:- $%f", receivingMessage.data.account.funds);
            }else{

                if(receivingMessage.data.response == NSF){
                    printf("\nInsufficient funds\n");
                }else if(receivingMessage.data.response == FUNDS_OK){
                    printf("\nNew balance:- $%f", receivingMessage.data.account.funds);       // Receives the response from the DB
                }else{
                    perror("Unknown message received from the DB Server");
                }

            }

        }else{
            perror("Received unknown message from DB");
        }

        /* If the user inputs the wrong account or PIN; dock them an attempt */
        /* else {
            printf("\nIncorrect account or PIN\n");
            attempts -= 1; */

            /* If they are out of attempts; exit (what we chose to do) */
           /*  if (attempts <= 0){
                printf("\nAccount is locked\n");
                exit(EXIT_FAILURE);                         // Not specified if should exit
            }
        } */

    } while (0);
    wait(NULL);
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
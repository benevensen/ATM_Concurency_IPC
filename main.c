#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<mqueue.h>


/* ========================== QUEUE PREREQUISITES ========================== */
#define ACCOUNT_NUMBER_SIZE 5
#define PIN_NUMBER_SIZE 3
#define DB_RESPONSE_SIZE 8
#define MAX_QUEUE_MSG_SIZE 16


/* ========================== STRUCT PREREQUISITES ========================== */
/*
 * The PIN and account number struct to be transmitted to the
 * DB upon submission from the client.
 * */
typedef struct SEND_MSG_STRUCT{
    char PIN_NUMBER[3];         // The client's PIN
    char ACCOUNT_NUMBER[5];     // The client's account number
} SEND_MSG_STRUCT;
static mqd_t SEND_DESTINATION = -1;
static mqd_t RCV_DESTINATION = -1;

/*
 * The banking request struct to be transmitted to the
 * DB when the user selects their needed service
 * */
typedef struct BANKING_MSG_STRUCT{
    char BALANCE_REQUEST[15];
    char WITHDRAW_REQUEST[16];     // Only the requested banking service will have a corresponding value of 1
    char WITHDRAW_AMOUNT;                      // For a withdrawal service; the user-provided withdrawal amount is a float
} BANKING_MSG_STRUCT;

/*
 * The response struct returned by the database when it is
 * sent one of the above MSG_STRUCT's
 * */
typedef struct RCV_MSG_STRUCT{
    char RESPONSE[DB_RESPONSE_SIZE];                       // The response must be no more than 9 chars
}RCV_MSG_STRUCT;


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

void ATM_START() {

    int ATTEMPTS_REMAINING = 3;         // How many attempts the user has left
    int status;                         // Represents the status when sending and receiving on the message queue
    char ACCOUNT_NUM[5];                // User given account number
    char PIN[3];                        // User given PIN

    SEND_MSG_STRUCT SEND_MSG;            // Sends PIN and account number to DB
    RCV_MSG_STRUCT RCV_MSG;             // Receives the response from the DB

    printf("<=====================> ATM <=====================>");

    do {
        printf("Kindly enter your account number:- ");
        scanf("%c", ACCOUNT_NUM);
        printf("\nKindly enter you PIN:- ");
        scanf("%c", PIN);

        for (int i = 0; i < 5; ++i) {                       // Making the account number in the struct equal to the user-given account number
            SEND_MSG.ACCOUNT_NUMBER[i] = ACCOUNT_NUM[i];
        }
        for (int i = 0; i < 3; ++i) {                       // Making the PIN in the struct equal to the user-given PIN
            SEND_MSG.PIN_NUMBER[i] = PIN[i];
        }

        status = mq_send(SEND_DESTINATION, (const char*) &ACCOUNT_NUM, sizeof(ACCOUNT_NUM),
                         1);    // Sending the SEND struct; contains PIN and account number
        if (status < 0) {               // If the returned status is -1 there is an error
            printf("PIN message cannot send");
            exit(EXIT_FAILURE);
        }

        status = mq_receive(RCV_DESTINATION, (char *) &RCV_MSG, sizeof(RCV_MSG), 0);       // Receiving the DB struct; contains response
        if (status < 0) {           // If the returned status is -1 there is an error
            printf("DB message cannot be received");
            exit(EXIT_FAILURE);
        }

        // If we receive the "OK" from the DB we may proceed to selection
        if (EQUALITY_CHECK(RCV_MSG.RESPONSE, "OK", 4)) {
            char SELECTION[8];                  // Ther user-given operation selection
            BANKING_MSG_STRUCT BANKING_MSG;     // Struct containing the information regarding the selection ("BALANCE", "WITHDRAW" and the amount).

            printf("Kindly type your service:- \"BALANCE\" or \"WITHDRAW\"\n");
            do {
                scanf("%c", SELECTION);

                // If its a BALANCE request
                if (EQUALITY_CHECK(SELECTION, "BALANCE", 7)) {
                    memcpy(BANKING_MSG.BALANCE_REQUEST, "BALANCE_REQUEST", 15);                         // Copies the request into the BALANCE_REQUEST field
                    mq_send(SEND_DESTINATION, (const char*) &BANKING_MSG, sizeof(BANKING_MSG), 1);      // Sends the BALANCE request to the DB
                    mq_receive(SEND_DESTINATION, (char*) &BANKING_MSG, sizeof(BANKING_MSG), 0);         // Receives the response from the DB
                    printf("\nCurrent balance:- $%s", BANKING_MSG.BALANCE_REQUEST);
                    break;
                }

                // If its a WITHDRAW request
                else if (EQUALITY_CHECK(SELECTION, "WITHDRAW", 8)) {
                    double AMOUNT;                                                                              // The amount to withdraw
                    printf("\nKindly enter the amount to withdraw (CAD assumed):- ");
                    scanf("%lf", &AMOUNT);
                    memcpy(BANKING_MSG.WITHDRAW_REQUEST, "WITHDRAW_REQUEST", 16);                       // Copies the request into the WITHDRAW_REQUEST
                    memcpy(BANKING_MSG.WITHDRAW_AMOUNT, &AMOUNT, 16);                                       // Copies the amount in the WITHDRAW_AMOUNT
                    mq_send(SEND_DESTINATION, (const char*) &BANKING_MSG, sizeof(BANKING_MSG), 1);      // Sends the WITHDRAW request to the DB
                    mq_receive(SEND_DESTINATION, (char*) &BANKING_MSG, sizeof(BANKING_MSG), 0);         // Receives the response from the DB

                    // If there is insufficient funds in the account
                    if (EQUALITY_CHECK(RCV_MSG.RESPONSE, "NSF", 3)) {
                        printf("\nInsufficient funds\n");
                    }

                    // If there is sufficient funds in the account
                    else if (EQUALITY_CHECK(RCV_MSG.RESPONSE, "FUNDS_OK", 8)) {
                        printf("\nCurrent balance:- $%f",
                               BANKING_MSG.BALANCE_REQUEST);        // Print balance - withdrawal
                    }
                    break;
                }

            } while (0);
        }

        // If the user inputs the wrong accont or PIN; dock them an attempt
        else {
            printf("\nIncorrect account or PIN\n");
            ATTEMPTS_REMAINING -= 1;

            // If they are out of attempts; exit (what we chose to do)
            if (ATTEMPTS_REMAINING <= 0){
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
 * 7.
 * */

/*
 * NOTES:-
 * - Still need a message queue, not sure how to implement
 * */

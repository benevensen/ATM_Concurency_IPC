#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<mqueue.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<signal.h>
#include<stdbool.h>
#include<time.h>


/* ========================== QUEUE PREREQUISITES ========================== */
#define ACCOUNT_NUMBER_SIZE 5
#define PIN_NUMBER_SIZE 3
#define DB_RESPONSE_SIZE 8
#define MAX_QUEUE_MSG_SIZE 16

/* ========================== THREAD PREREQUISITES ========================== */
#define ERROR_HANDLER(MSG) \
    do { perror(MSG); exit(EXIT_FAILURE); } while(0)

/*
 * Struct for information about the thread; succeeds thread_start()
 * */
typedef struct THREAD_INFO{
    pthread_t THREAD_ID;        // The ID returned by the pthread_create()
    int THREAD_NUMBER;          // The number assigned to the thread by the system
}THREAD_INFO;

THREAD_INFO DB_SERVER_THREAD_INFO;      // Housing info on the DB server thread
THREAD_INFO DB_EDITOR_THREAD_INFO;      // Housing the info on the DB editor thread
THREAD_INFO ATM_THREAD_INFO;            // Housing the info on the ATM thread

pthread_t ATM_THREAD;                           // Thread for the ATM
pthread_t DB_EDITOR_THREAD;                     // DB editor thread
pthread_t DB_SERVER_THREAD;                     // DB server thread

pthread_mutex_t RESOURCE_LOCK;                   // Lock for resources

pthread_mutex_t PRINT_LOCK;                     // Lock for printing
pthread_mutex_t READWRITE_LOCK;                 // Lock for read/write processes
pthread_mutex_t DB_EDITOR_LOCK;                 // Lock for the DB starting


/* ========================== STRUCT PREREQUISITES ========================== */
/*
 * The PIN and account number struct to be transmitted to the
 * DB upon submission from the client.
 * */
typedef struct PIN_MSG_STRUCT{
    char PIN_NUMBER[3];         // The client's PIN
    char ACCOUNT_NUMBER[5];     // The client's account number
} PIN_MSG_STRUCT;

/*
 * The banking request struct to be transmitted to the
 * DB when the user selects their needed service
 * */
typedef struct BANKING_MSG_STRUCT{
    bool BALANCE_REQUEST, WITHDRAW_REQUEST;     // Only the requested banking service will have a corresponding value of 1
    float WITHDRAW_AMOUNT;                      // For a withdrawal service; the user-provided withdrawal amount is a float
} BANKING_MSG_STRUCT;

/*
 * The response struct returned by the database when it is
 * sent one of the above MSG_STRUCT's
 * */
typedef struct RCV_MSG_STRUCT{
    char RESPONSE[DB_RESPONSE_SIZE];                       // The response must be no more than 9 chars
}RCV_MSG_STRUCT;
#define RCV_MSG_STRUCT_SIZE sizeof(RCV_MSG_STRUCT);




/* ========================== RESOURCE PREREQUISITES & OTHER ========================== */
/*
 * Method to facilitate synchronization between threads on shared resources (reference 4.)
 * */
void RESOURCE_LOCK_PRINT(char* MSG){
    pthread_mutex_lock(&RESOURCE_LOCK);
    printf(MSG);
    pthread_mutex_unlock(&RESOURCE_LOCK);
}

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
 * Method for handling the signal inputs from the user. The user wishes to quit
 * the application
 * */
void SIGNAL_HANDLER(int SIGNAL_NUMBER){
    printf("\nSIGNAL RECEIVED:- %d \nSTOPPING THREADS...", SIGNAL_NUMBER);

//    pthread_cancel(ATM_THREAD);              // Stopping the ATM thread           NEED TO FIRST RUN ATM; DB_EDITOR; AND DB_SERVER
//    pthread_cancel(DB_EDITOR_THREAD);        // Stopping the DB editor thread
//    pthread_cancel(DB_SERVER_THREAD);        // Stopping the DB server thread

//    mq_close(PIN_MSG);
//    mq_unlink(PIN_MSG_NAME);
//    mq_close(DB_MSG);
//    mq_unlink(DB_MSG_NAME);
};

bool ACCOUNT_VALIDATE(){
    char GIVEN_ACCOUNT_NUMBER[5];
    char DB_ACCOUNT_NUMBER[5];
    bool status = false;

    RESOURCE_LOCK_PRINT("Kindly enter your account number:- ");       // Prompt user to enter account number
    scanf("%s", GIVEN_ACCOUNT_NUMBER);                              // Scan for user submissison

    // Opening the database with a read request
    FILE *fp = fopen("atm_database.txt", "r");
    if (fp == NULL) {
        ERROR_HANDLER("File cannot be opened");
    }

    // Looking for the account number provided by the user
    while (fgets(DB_ACCOUNT_NUMBER, sizeof(DB_ACCOUNT_NUMBER), fp)) {
        if (strstr(DB_ACCOUNT_NUMBER, GIVEN_ACCOUNT_NUMBER)) {
            status = true;
        }
    }
    fclose(fp);
    return status;
}

bool PIN_VALIDATE(){
    char GIVEN_PIN[3];
    char DB_PIN[3];
    bool status = false;

    RESOURCE_LOCK_PRINT("Kindly enter your PIN:- ");       // Prompt user to enter PIN
    scanf("%s", GIVEN_PIN);                              // Scan for user submissison

    // Opening the database with a read request
    FILE *fp = fopen("atm_database.txt", "r");
    if (fp == NULL) {
        ERROR_HANDLER("File cannot be opened");
    }

    // Looking for the account number provided by the user
    while (fgets(DB_PIN, sizeof(DB_PIN), fp)) {
        if (strstr(DB_PIN, GIVEN_PIN)) {
            status = true;
        }
    }
    fclose(fp);
    return status;
}


int main() {

}

void ATM_START(){
    PIN_MSG_STRUCT SEND_MSG, RECEIVE_MSG;   // Declaring the message structs for the message to be sent and received

    bool SIGNAL;                            // Signal represents the feedback from the message queue
    char ACCOUNT_NUMBER[5];                 // The account number given by the user
    char PIN[3];                            // The PIN given by the user

    int REMAINING_PIN_TRIES = 3;                      // Number of invalid PIN entries by the user

    RESOURCE_LOCK_PRINT("<=======================> ATM <=======================>");

    do {
        switch (ACCOUNT_VALIDATE()) {
            case true:
                RESOURCE_LOCK_PRINT("\nAccount found\n");
                SIGNAL = PIN_VALIDATE();
                while(!SIGNAL){                // If the PIN is not found
                    if (REMAINING_PIN_TRIES == 0) {
                        ERROR_HANDLER("PIN wrong three times; exiting");
                    }

                    RESOURCE_LOCK_PRINT("\n\nIncorrect PIN ");                      // May be a source of error (passing REMAINING_... into RLP)
                    RESOURCE_LOCK_PRINT(REMAINING_PIN_TRIES + " tries left.\n");

                    REMAINING_PIN_TRIES -= 1;
                }

        }
    } while (0);

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

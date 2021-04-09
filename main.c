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


/* ========================== THREAD PREREQUISITES ========================== */
#define ERROR_HANDLER(MSG) \
    do { perror(MSG); exit(EXIT_FAILURE); } while(0)

///*
// * Struct for information about the thread; succeeds thread_start()
// * */
//typedef struct THREAD_INFO{
//    pthread_t THREAD_ID;        // The ID returned by the pthread_create()
//    int THREAD_NUMBER;          // The number assigned to the thread by the system
//}THREAD_INFO;
//
//THREAD_INFO DB_SERVER_THREAD_INFO;      // Housing info on the DB server thread
//THREAD_INFO DB_EDITOR_THREAD_INFO;      // Housing the info on the DB editor thread
//THREAD_INFO ATM_THREAD_INFO;            // Housing the info on the ATM thread

pthread_t ATM_THREAD;
pthread_t DB_EDITOR_THREAD;
pthread_t DB_SERVER_THREAD;

pthread_mutex_t LOCK;                   // Lock for resources


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
typedef struct DATABASE_RESPONSE_STRUCT{
    char RESPONSE[9];                       // The response must be no more than 9 chars
}DATABASE_RESPONSE_STRUCT;


/* ========================== QUEUE PREREQUISITES ========================== */
#define DATABASE "/database.db";                        // Database file name (reference 5.)
#define MOCK_DATABASE "/mock_database.db";              // Temporary database file name

#define PIN_MSG_QUEUE "/pin_msg_queue";                 // The queue for the PIN's (reference 5.)
#define PIN_MSG_QUEUE_MAX 16;                           // Maximum number of PIN messages
#define PIN_MSG_QUEUE_SIZE sizeof(PIN_MSG_STRUCT);      // The length of a PIN message

#define DB_MSG_QUEUE "/database_msg_queue";             // The queue for the database response messages (reference 5.)
#define DB_MSG_QUEUE_MAX 16;                            // Maximum number of DB messages
#define DB_MSG_QUEUE_SIZE sizeof(DB_RESPONSE_STRUCT)    // The length of a DB message



/* ========================== RESOURCE PREREQUISITES & OTHER ========================== */
/*
 * Method to facilitate synchronization between threads on shared resources (reference 4.)
 * */
void RESOURCE_LOCK_PRINT(char MSG[]){
    pthread_mutex_lock(&LOCK);
    printf(MSG);
    pthread_mutex_unlock(&LOCK);
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
SIGNAL_HANDLER(int SIGNAL_NUMBER){
    printf("\nSIGNAL RECEIVED:- %d \nSTOPPING THREADS...", SIGNAL_NUMBER);

//    pthread_cancel(ATM_THREAD);              // Stopping the ATM thread           NEED TO FIRST RUN ATM; DB_EDITOR; AND DB_SERVER
//    pthread_cancel(DB_EDITOR_THREAD);        // Stopping the DB editor thread
//    pthread_cancel(DB_SERVER_THREAD);        // Stopping the DB server thread
};



int main() {
    printf("Hello world!");
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

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

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
    int BALANCE_REQUEST, WITHDRAW_REQUEST;  // Only the requested banking service will have a corresponding value of 1
    float WITHDRAW_AMOUNT;                  // For a withdrawal service; the user-provided withdrawal amount is a float
} BANKING_MSG_STRUCT;

/*
 * The response struct returned by the database when it is
 * sent one of the above MSG_STRUCT's
 * */
typedef struct DATABASE_RESPONSE_STRUCT{
    char RESPONSE[9];                       // The response must be no more than 9 chars
}DATABASE_RESPONSE_STRUCT;


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



int main() {

}

/*
 * REFERENCES:-
 * 1. https://man7.org/linux/man-pages/man3/pthread_create.3.html
 * 2. https://www.geeksforgeeks.org/thread-functions-in-c-c/
 * 3. https://www.thegeekstuff.com/2012/05/c-mutex-examples/
 * 4. https://www.thegeekstuff.com/2012/05/c-mutex-examples/
 * */

/*
 * NOTES:-
 * - Still need a message queue, not sure how to implement
 * */

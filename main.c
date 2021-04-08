#include<stdio.h>
#include<string.h>
//#include<pthread.h> not working
#include<stdlib.h>
#include<unistd.h>


int main() {
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


}

/*
 * REFERENCES:-
 * 1. https://man7.org/linux/man-pages/man3/pthread_create.3.html
 * 2. https://www.geeksforgeeks.org/thread-functions-in-c-c/
 * 3. https://www.thegeekstuff.com/2012/05/c-mutex-examples/
 * 4.
 * */

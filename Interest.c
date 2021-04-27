/**
 * Interest calculator functionality, attached to the database server
 * Every 1000 seconds, 10% (exaggerated figure) is removed from a users funds as
 * a service-charge
 * 
 * Problems:-
 *  - Repeated code from parent (DBserver.c) because I am not sure if we should do #include "DBserver.c", i.e., query_balance_in_db()
 *  - Not sure if we need the interest calculator to communicate with the DBserver
 * */
#include <unistd.h>

#include "DBserver.h"
#include "extra_file.h"

const char filename[20] = "DB_file.txt";

DataBundle Handle_INTEREST(DataBundle message, Account* accounts, int numberOfAccounts, int currentAccount);

float query_balance_in_db(Account *accounts,int numberOfAccounts,int account_number);
int interest_remover_in_db(Account *accounts,int numberOfAccounts,int account_number, float funds_being_withdrawn);


int main(int argc, char *argv[]){
    /*
    * Getting all of the required information for the interest functionality
    */
    int semID = getSemId();

    int msgID = getmsgQueueID();

    GenericMessage sendingMessage;
    GenericMessage receivingMessage;

    sendingMessage.message_type = clientMessageType;
    receivingMessage.message_type = serverMessageType;

    resetDataBundle(&sendingMessage.data);
    resetDataBundle(&receivingMessage.data);

    int message_received_message_type;
    DataBundle sendingData;

    int current_account = -1;

    int numberOfAccounts = 0;

    Account * accounts;

    /* Initializing the database */
    accounts = DB_INIT(filename, &numberOfAccounts);

    if(accounts == (Account*) -1){ // If DB file is empty
        accounts = (Account*) malloc(sizeof(Account));
    }

    /* Every 1000 seconds handle an interest request */
    while (1)
    {
        sendingData = Handle_INTEREST(receivingMessage.data, accounts, numberOfAccounts, current_account);
        sendingMessage.data = sendingData;
        sendMessage(msgID, sendingMessage, NOBLOCK);
        sleep(1000);
    }
}

DataBundle Handle_INTEREST(DataBundle message, Account* accounts, int numberOfAccounts, int currentAccount){

    DataBundle responseData;

    int account = currentAccount;
    float interest_amount = message.account.funds / 10.00;

    //query the database 

    float current_funds = query_balance_in_db(accounts, numberOfAccounts,account); 

    if(current_funds == 0.00){
        responseData.response = NSF;
    }else{

        //decrememt the funds in the account
        int result = interest_remover_in_db(accounts, numberOfAccounts, account, interest_amount);
        DB_UPDATE_FILE(accounts,numberOfAccounts,filename);
        if(result == -1){
            perror("Interest.c: error in handle interest");
            exit(1);
        }

        current_funds = query_balance_in_db(accounts, numberOfAccounts,account); 

        responseData.account.funds = current_funds;
        responseData.response = FUNDS_OK;
    }
    return responseData;
}

/**
 * Method to interact with DB and removed specified interest amount
 * */
int interest_remover_in_db(Account *accounts,int numberOfAccounts,int account_number, float interest_funds){
    /* Iterate through all the accounts */ 
    for(int x = 0; x < numberOfAccounts; x++){

        /* Checks if the account numbers exist in the array of accounts */
        if(accounts[x].accountNumber == account_number){
            accounts[x].funds -= interest_funds;

            return 1;
            break;
        }
    }

    /* If the account cannot be found */
    return -1;
}

//function for checking the balance of an account in the system
//parameters are an array of accounts , the number of accounts , account number of the account being queried
//returns the current funds within the account, otherwise returns -1 if the account could not be found
float query_balance_in_db(Account *accounts,int numberOfAccounts,int account_number){
      //iterate through all the accounts 
    for(int x = 0; x < numberOfAccounts; x++){
        

        //checks if the account numbers exist in the array of accounts
        if(accounts[x].accountNumber == account_number){

            return accounts[x].funds;
            break;
        }
    }

    //no accounts with the account number specified
    return -1;

}


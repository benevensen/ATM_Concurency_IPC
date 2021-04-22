#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "extra_file.h"
#include "DBserver.h"


const char filename[20] = "DB_file.txt";

DataBundle Handle_PIN(int *attempts, int *last_account,int *current_account ,DataBundle message, Account** accounts, int numberOfAccounts);

DataBundle Handle_BALANCE( DataBundle message, Account* accounts, int numberOfAccounts, int currentAccount);

DataBundle Handle_WITHDRAW(DataBundle message, Account* accounts, int numberOfAccounts, int currentAccount);

void Handle_UPDATE_DB(DataBundle message,  Account** accounts, int* numberOfAccounts);



// Database related functions

int getCountOfLinesInDBFile();

Account* add_account_to_db(Account *accounts,int *numberOfAccounts, Account account);
void lock_account_in_db(Account *accounts,int numberOfAccounts, int accountToLock);


int query_account_and_pin_in_db(Account *accounts,int numberOfAccounts,int account_number, int pin);
float query_balance_in_db(Account *accounts,int numberOfAccounts,int account_number);
int withdrawal_within_in_db(Account *accounts,int numberOfAccounts,int account_number, float funds_being_withdrawn);
int query_account_in_db(Account *accounts,int numberOfAccounts,int account_number);
void update_account_in_db(Account *accounts,int numberOfAccounts,Account account);


Account* DB_INIT(const char *inputFile, int *numberOfAccounts);

void DB_UPDATE_FILE(Account *accounts,int numberOfAccounts,const char *inputFile); // after a withdrawal, new account, or lock

void print_current_Accounts_DB(Account *accounts,int numberOfAccounts, char *message);

void print_current_Accounts_DB_pointer(Account *accounts, char *message);

int main(int argc, char *argv[])
{

    int semID = getSemId();

    int msgID = getmsgQueueID();

    

    semInit(semID);
    
    
    int x = 0;

    GenericMessage sendingMessage;
    GenericMessage receivingMessage;

    sendingMessage.message_type = clientMessageType;
    receivingMessage.message_type = serverMessageType;

    resetDataBundle(&sendingMessage.data);
    resetDataBundle(&receivingMessage.data);

    int message_received_message_type;
    DataBundle sendingData;

    int attempts = 3;
    int last_account = -1;
    int current_account = -1;

    int numberOfAccounts = 0;

    Account * accounts;

    accounts = DB_INIT(filename, &numberOfAccounts);

    if(accounts == (Account*) -1){ // If DB file is empty
        accounts = (Account*) malloc(sizeof(Account));
    }

    //print_current_Accounts_DB(accounts,numberOfAccounts, "Initial DB");

    while(1){

        resetDataBundle(&receivingMessage.data);

        receiveMessage(msgID, &receivingMessage, BLOCK);


        // if error or exit command then exit the loop
        if(receivingMessage.data.account.accountNumber == -1 || receivingMessage.data.response == -1){

            sendingMessage.data.response = -1;

            sendMessage(msgID, sendingMessage, NOBLOCK);

            break;
        }

        resetDataBundle(&sendingMessage.data);

        message_received_message_type = receivingMessage.data.type.message;

       /*  print_current_Accounts_DB(accounts,numberOfAccounts, "DB After receiving message"); */

        switch (message_received_message_type)
        {
        case PIN:
            sendingData = Handle_PIN(&attempts, &last_account, &current_account, receivingMessage.data, &accounts, numberOfAccounts);
            break;
        case BALANCE:

            sendingData = Handle_BALANCE(receivingMessage.data, accounts, numberOfAccounts, current_account);
            break;
        case WITHDRAW:

            sendingData = Handle_WITHDRAW(receivingMessage.data, accounts, numberOfAccounts, current_account);
            break;
        case UPDATE_DB:

            Handle_UPDATE_DB(receivingMessage.data, &accounts, &numberOfAccounts);

            print_current_Accounts_DB_pointer(accounts, "DB ptr After print to db");

            break;
        default:
            perror("DBserver: Invalid message type received at DBserver");
            exit(1);
            break;
        }


       /*  print_current_Accounts_DB(accounts,numberOfAccounts, "DB After handling message"); */

      /*   print_current_Accounts_DB_pointer(accounts, "DB ptr  After handling message"); */

        sendingMessage.data = sendingData;

     /*    printMessageType( (MessageType) message_received_message_type );
 */
        x++;

        sendMessage(msgID, sendingMessage, NOBLOCK);

    } 

    wait(NULL);

    free(accounts);


    semDelete(semID);

    deleteMessageQueue(msgID);
    
}


// this function subtracts 1 from the pin to compare encrypted pin numbers
DataBundle Handle_PIN(int *attempts, int *last_account,int *current_account ,DataBundle message, Account** accounts, int numberOfAccounts){
    
    DataBundle responseData;

    int account_number = message.account.accountNumber;
    int pin = message.account.pin - 1;

    //reset attempts
    if(*last_account != account_number){
        *attempts = 3;
    }

    int result = query_account_and_pin_in_db(*accounts, numberOfAccounts, account_number, pin);

    if( result == -1 ){  // account exists but incorrect pin
        *attempts = *attempts - 1;
        
    

        //if 3 attempts, lock the account
        if(*attempts == 0){
            
            lock_account_in_db(*accounts, numberOfAccounts, account_number);
            DB_UPDATE_FILE(*accounts,numberOfAccounts, filename);

            responseData.account.isLocked = 1;

        }

        responseData.response = PIN_WRONG;
        *current_account = -1;

    }else if(result == -2){ //account does not exist

         responseData.response = PIN_WRONG;
        *current_account = -1;
    }else{ //account exists and correct pin

        responseData.response = OK;
        *current_account = account_number;

    }

    *last_account = account_number;

    return responseData;
}


DataBundle Handle_BALANCE( DataBundle message, Account* accounts, int numberOfAccounts, int currentAccount){

    DataBundle responseData;


     //query the database 
    float current_funds = query_balance_in_db(accounts, numberOfAccounts, currentAccount);  //to be changed to a query

    if(current_funds == -1){
        perror("DBServer: error in Handle_Balance");
    }
    //int current_funds = 0;


    responseData.response = current_funds;
    responseData.account.funds = current_funds;

    return responseData;
    
}

DataBundle Handle_WITHDRAW(DataBundle message, Account* accounts, int numberOfAccounts, int currentAccount){
    
    DataBundle responseData;

    int account = currentAccount;
    float requested_withdrawal_amount = message.account.funds;

    //query the database 

    float current_funds = query_balance_in_db(accounts, numberOfAccounts,account); 

    if(current_funds < requested_withdrawal_amount){
        responseData.response = NSF;
    }else{

        //decrememt the funds in the account
        int result = withdrawal_within_in_db(accounts, numberOfAccounts, account, requested_withdrawal_amount);
        DB_UPDATE_FILE(accounts,numberOfAccounts,filename);
        if(result == -1){
            perror("DBserver : error in handle withdraw");
            exit(1);
        }

        current_funds = query_balance_in_db(accounts, numberOfAccounts,account); 

        responseData.account.funds = current_funds;
        responseData.response = FUNDS_OK;
    }

    return responseData;
}

void Handle_UPDATE_DB(DataBundle message,  Account** accounts, int* numberOfAccounts){

    Account account = message.account;

    if(query_account_in_db(*accounts, *numberOfAccounts, account.accountNumber) == -1){
        *accounts = add_account_to_db(*accounts, numberOfAccounts, account);
    }else{
        update_account_in_db(*accounts, *numberOfAccounts, account);
    }
    
    DB_UPDATE_FILE(*accounts, *numberOfAccounts, filename);
    
}

//Function to count the number of accounts in the database file
//parameter is a string of the name of the inputFile
//returns the number of accounts found in the input file (assuming each line represents a process)
//ASSUMES : there is no blank lines in the input file
int countNumberOfAccounts(const char *inputFile)
{
    int accountCtr = 0; // initializes account counter

    FILE *file = fopen(inputFile, "r"); // opens/creates database file in current working directory in read mode

    if (file == NULL)
    { //Checks for file if file opened properly
        perror("DBServer: Could not open file.\n");
        exit(1);
    }

    char input_line[100]; // input buffer for reading from the file

    //while loop that reads the input file (line by line) until the end of file is
    while (fgets(input_line, sizeof(input_line), file))
    {

        /* if(strncmp(&input_line[0],"x",1) == 0){  // if the account is blocked then don't include it in the count
            continue;
        } */
        accountCtr++; //Increment process counter for each line read
    }

    //closes input file when finished
    fclose(file);

    //returns the number of processes found in the input file
    return accountCtr;
}


//function for reading and parsing the database file
//parameters are an array of accounts , a string for the name of the input file, and the memory scheme
//returns nothing
//ASSUMES : there is no blank lines in the input file
//If memory scheme is 0, ASSUMES no memory requirement for the processes
void readDBFile(Account *accounts, const char *inputFile)
{
    char str[100];                //To store the text contained in each line
    const char truncate[2] = ","; //In-line separator
    char *token;                  //To store the token for each line
    int input_field = 0;      //To traverse through the different output parameters (i.e., execution time, etc.) Set to zero for first token
    int account_position = 0;     //To traverse through the words in each line

    FILE *file = fopen(inputFile, "r"); //Opens the input file within current working directory in read mode

    // If file could not open, prints the error and exits
    if (file == NULL)
    {
        perror("DBServer: Could not open file.");
        exit(1);
    }

    //while loop that keeps reading the file (line by line) until the end is reached, stores the contents of the line in str
    while (fgets(str, sizeof(str), file))
    {
        
        if(strncmp(&str[0],"x",1) == 0){  // if the account is blocked then don't include it in the count
           /*  continue; */
            accounts[account_position].isLocked = 1;
        }else{
            accounts[account_position].isLocked = 0;
        }

        token = strtok(str, truncate); //Break input into a series of tokens

        while (token != NULL)
        {
            //int int_token = atoi(token); //Converts string to integer representation

            // if statements to set the members of the process struct according to the input from the file
            if (input_field == 0)
            {
                if(strncmp(&str[0],"x",1) == 0){ // account is currently locked
                    accounts[account_position].accountNumber = atoi(token+1); // skips the x and reads the numbers after the x
                }else{
                    accounts[account_position].accountNumber = atoi(token);
                }
            }
            else if (input_field == 1)
            {
                accounts[account_position].pin = atoi(token);
            }
            else if (input_field == 2)
            {
                accounts[account_position].funds = atof(token);
            }

            token = strtok(NULL, truncate); //Reset token
            input_field++;              //Increment input parameter position counter
        }

        input_field = 0; //Reset input parameter counter
        account_position++;  //Increment the process position counter to point to the next process in the array of processes
    }

    //closes the input file
    fclose(file);
}

//function for initializing the database in the system
//parameter is the filename of the database file
//returns a pointer to an Account array if the DB file isn't empty, otherwise returns -1
Account* DB_INIT(const char *inputFile, int *numberOfAccounts){

    *numberOfAccounts = countNumberOfAccounts(inputFile);

    Account *Accounts;

    if(*numberOfAccounts > 0){
        Accounts = (Account *) malloc(*numberOfAccounts * sizeof(Account));

        readDBFile(Accounts, inputFile);
    }else{
        Accounts = (Account *) -1;
    }

    return Accounts;
}

//function for updating the file after a change
//parameters are an Account array, the current number of accounts, account number of locked account (set to --1 if there is no account to lock) , 
//and the filename of the DB
//returns nothing
void DB_UPDATE_FILE(Account *accounts,int numberOfAccounts,const char *inputFile){ // after a withdrawal, new account, or lock
    
    
    FILE *file = fopen(inputFile, "w"); //Opens the input file within current working directory in read mode

    // If file could not open, prints the error and exits
    if (file == NULL)
    {
        perror("DBServer: Could not open file.");
        exit(1);
    }

    //iterate through all the accounts 
    for(int x = 0; x < numberOfAccounts; x++){

        //print the accounts to the DBFile
        if(accounts[x].isLocked == 1){
            fprintf(file,"x%04d,%03d,%.2f", accounts[x].accountNumber,accounts[x].pin,accounts[x].funds);
        }else{
            fprintf(file,"%05d,%03d,%.2f", accounts[x].accountNumber,accounts[x].pin,accounts[x].funds);
        }

        if(x < numberOfAccounts -1){
            fprintf(file,"\n");
        }
    }

    //closes the input file
    fclose(file);

}

//function for adding an account to the system
//parameters are an array of accounts , account number of the accounts, and the account being added
//returns a pointer to a newly sized array with the updated information
Account* add_account_to_db(Account *accounts,int* numberOfAccounts, Account account){
    Account newAccount;

    newAccount.accountNumber = account.accountNumber;
    newAccount.pin = account.pin;
    newAccount.funds = account.funds;
    newAccount.isLocked = account.isLocked;

    int newArrayLength = *numberOfAccounts + 1; 

    Account *newArray;
    newArray =  (Account *) malloc( (newArrayLength) * sizeof(Account) );

    for(int x = 0; x < newArrayLength; x++){

        if(x == newArrayLength -1){
            newArray[*numberOfAccounts].accountNumber = newAccount.accountNumber;
            newArray[*numberOfAccounts].pin = newAccount.pin;
            newArray[*numberOfAccounts].funds = newAccount.funds;
            newArray[*numberOfAccounts].isLocked = newAccount.isLocked;

        }else{

            newArray[x].accountNumber = accounts[x].accountNumber;
            newArray[x].pin = accounts[x].pin;
            newArray[x].funds = accounts[x].funds;
            newArray[x].isLocked = accounts[x].isLocked;

        }

    }

    *numberOfAccounts = newArrayLength;

    free(accounts);

    return newArray;
}

//function for removing a locked account in the system
//parameters are an array of accounts , account number of the accounts, and the account being removed
//returns a pointer to a newly sized array with the updated information
void lock_account_in_db(Account *accounts,int numberOfAccounts, int accountToLock){

    
    for(int x = 0; x < numberOfAccounts; x++){

        if(accounts[x].accountNumber == accountToLock){
            accounts[x].isLocked = 1;
            break;
        }

    }


}

//function for checking an account with a specific pin exists in the system
//parameters are an array of accounts , account number of the account being queried, and the (encrypted) pin being queried
//returns 1 if the account exists and the pin is correct, -1 if the account exists but the pin is wrong, and returns -2 if the account does not exist
//encryption is chosen to be subtracting 1 from the pin, because the assignment specified 2 conflicting encryption methods
int query_account_and_pin_in_db(Account *accounts,int numberOfAccounts,int account_number, int pin){

    //iterate through all the accounts 
    for(int x = 0; x < numberOfAccounts; x++){

        //checks if the account numbers exist in the array of accounts
        if( accounts[x].isLocked == 0  && accounts[x].accountNumber == account_number){

           

            if(accounts[x].pin == pin){ // if the encrypted pin matchs, return 1
                return 1;
            }else{                      //otherwise return -1
                return -1;
            }

            break;
        }
    }

    //no accounts with the account number specified
    return -2;

}

//function for checking an account with a specific account exists in the system
//parameters are an array of accounts , account number of the account being queried
//returns 1 if the account exists , otherwise returns -1
int query_account_in_db(Account *accounts,int numberOfAccounts,int account_number){

    //iterate through all the accounts 
    for(int x = 0; x < numberOfAccounts; x++){

        //checks if the account numbers exist in the array of accounts
        if(accounts[x].accountNumber == account_number){
            return 1;
        }
    }

    //no accounts with the account number specified
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

//function for conducting a withdrawal of funds from an account in the system
//parameters are an array of accounts , the number of accounts , account number of the account being used, the amount of funds being withdrawn
//returns 1 if successful, otherwise returns -1 if the account could not be found
int withdrawal_within_in_db(Account *accounts,int numberOfAccounts,int account_number, float funds_being_withdrawn){
       //iterate through all the accounts 
    for(int x = 0; x < numberOfAccounts; x++){

        //checks if the account numbers exist in the array of accounts
        if(accounts[x].accountNumber == account_number){

            accounts[x].funds -= funds_being_withdrawn;

            return 1;
            break;
        }
    }

    //no accounts with the account number specified
    return -1;
}

//function for updating an account within the system
//parameters are an array of accounts , the number of accounts , and the account information being updated
//returns nothing
void update_account_in_db(Account *accounts,int numberOfAccounts,Account account){
         //iterate through all the accounts 
    for(int x = 0; x < numberOfAccounts; x++){

        //checks if the account numbers exist in the array of accounts
        if(accounts[x].accountNumber == account.accountNumber){

            accounts[x].pin = account.pin;
            accounts[x].funds = account.funds;
            accounts[x].isLocked = account.isLocked;

            return;
        }
    }

}


//For testing and debugging
void print_current_Accounts_DB(Account *accounts,int numberOfAccounts, char *message){

    puts("<--------------------------Start------------------------------>");

    puts(message);

   //iterate through all the accounts 
    for(int x = 0; x < numberOfAccounts; x++){

        printf("Position: %d, Account Number:%d, Account PIN: %d, Account Funds: %f, isLocked: %d\n", x,  accounts[x].accountNumber,  accounts[x].pin,  accounts[x].funds, accounts[x].isLocked );
    
    }

    puts("--------------------------------------------------------");
}

void print_current_Accounts_DB_pointer(Account *accounts, char *message){

    puts("<--------------------------Start------------------------------>");

    puts(message);

    printf("Current Account pointer is %p\n", (void *) accounts);

    puts("--------------------------------------------------------");
}
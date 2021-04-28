#include <stdlib.h>
#include <stdio.h>

//Enum types and structs for message passing

//enum for message types for requests to server
typedef enum MessageType{
    SIGNIN,
    PIN,
    BALANCE,
    TRANSFER,
    WITHDRAW,
    UPDATE_DB,
    INTEREST
} MessageType;


//struct for passing information about an account
typedef struct Account{
    int accountNumber;
    int pin;
    float funds;
    int isReceivingTransfer; // May not be needed, we the recepient may not need to accept the transfer
    int isLocked;
}Account;

//response types the server can send to a client 
typedef enum ResponseType{
    NOSPACE,
    OK,
    PIN_WRONG,
    BALANCE_RESPONSE,
    FUNDS_OK,
    NSF,
    RECIPIENT_DOES_NOT_EXIST,
    INVALID_AMOUNT
} ResponseType;

//union between message type and response type so they can be used in the same place
typedef union message_response_types{
    MessageType message;
    ResponseType responseType;
} message_response_types;

//data bundle that is send with a message
typedef struct DataBundle{
    message_response_types type;
    Account account;
    pid_t pid; //PID for sign in process
    int response;
} DataBundle;

//generic message which contains a message type used to send data between server and clients
//and a databundle which contains information for requests from the client and responses from the server
typedef struct genericMessage {
    long int message_type;
    DataBundle data;
} GenericMessage;

//struct for managing system and storing pid's for shutting down the processes within the system
typedef struct System_Memory {
    int process_count;
    pid_t process_IDs[7];
} System_Memory;

//database file name
const char filename[20] = "DB_file.txt";

//initialization function for system memory
void initSystemMemory(System_Memory *sys_mem){

    sys_mem->process_count = 0;

    for(int x = 0; x < 7; x++){
        sys_mem->process_IDs[x] = -1;
    }

}

//function for adding a process to the system
//limits system to 8 processes including the server its self (5 atms, 1 DBeditor, 1 interest calculator)
int addProcessToSystem(System_Memory *sys_mem, pid_t pid){

    if(sys_mem->process_count > 6){
        return -1;
    }else{
        sys_mem->process_IDs[sys_mem->process_count] = pid;
        sys_mem->process_count++;
    }

    return 1;

}

//resets databundle so that it can be used to send or receive data
void resetDataBundle( DataBundle * DataBundle){

    DataBundle->account.accountNumber = 0;
    DataBundle->account.pin = 0;
    DataBundle->account.funds = 0.0;

    DataBundle->type.message = 0;

    DataBundle->response = 0;

}

//print message type for debugging
void printMessageType(MessageType messageType){
    char *message;

    switch (messageType)
    {
    case PIN:
        message = " Message was PIN";
        break;
     case BALANCE:
        message = " Message was BALANCE";
        break;
     case WITHDRAW:
        message = " Message was WITHDRAW";
        break;
     case UPDATE_DB:
        message = " Message was UPDATE_DB";
        break;
    default:
        message = "Error in printMessageType";
        break;
    }

    puts(message);

}

//print response type for debugging
void printResponseType(ResponseType responseType){
    char *message;

    switch (responseType)
    {
    case OK:
        message = " Response was OK";
        break;
     case PIN_WRONG:
        message = " Response was PIN_WRONG";
        break;
     case BALANCE_RESPONSE:
        message = " Response was BALANCE_RESPONSE";
        break;
     case FUNDS_OK:
        message = " Response was FUNDS_OK";
        break;
     case NSF:
        message = " Response was NSF";
        break;
    default:
        message = "Error in printResponseType";
        break;
    }

    puts(message);

}


//function for sending a message using a message queue
int sendMessage(int messageQueueId, GenericMessage message, int shouldIBlockIfFull){

    int flag;

    if(shouldIBlockIfFull > 0){
        flag = 0;
    }else{
        flag = IPC_NOWAIT;
    }

    int msgLength = sizeof(GenericMessage) - sizeof(long);

    int result;

    if ( (result = msgsnd(messageQueueId, &message, (size_t) msgLength, flag) == -1))
    {
        perror("msgsnd: msgsnd failed");
        exit(1);
    }
    
    return result;
}

//function for receiving a message using a message queue
int receiveMessage(int messageQueueId, GenericMessage *message, int shouldIBlockIfFull){


    int flag;

    if(shouldIBlockIfFull > 0){
        flag = 0;
    }else{
        flag = IPC_NOWAIT;
    }

    int msgLength = sizeof(GenericMessage) - sizeof(long);

    long type = message->message_type;

    int result;

    if ( (result = msgrcv(messageQueueId, message,(size_t) msgLength,type , flag) == -1))
    {
        perror("msgsnd: msgsnd failed");
        exit(1);
    }
   
   return result;
}


//function for starting a child process
void start_child_process(const char * programFilePath, int msgID){

    pid_t pid;

    pid = fork();

    if(pid < 0){
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    { //child process

        if (msgID == 0)
        {
            execlp(programFilePath,"", NULL);
        }
        else
        {
            char numArg[8];

            sprintf(numArg, "%d", msgKey);

            execlp(programFilePath, numArg, NULL);
        }
    }

}

//function for exiting a program
void exit_program(){
    exit(0);
}

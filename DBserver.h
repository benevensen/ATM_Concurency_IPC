#include <stdlib.h>
#include <stdio.h>


typedef enum MessageType{
    SIGNIN,
    PIN,
    BALANCE,
    TRANSFER,
    WITHDRAW,
    UPDATE_DB,
    INTEREST
} MessageType;

typedef struct Account{
    int accountNumber;
    int pin;
    float funds;
    int isReceivingTransfer; // May not be needed, we the recepient may not need to accept the transfer
    int isLocked;
}Account;

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

typedef struct ServerMailbox{
    MessageType message;
    Account account;
} ServerMailbox;

typedef struct ClientMailbox{
    ResponseType responseType;
    int response;
} ClientMailbox;

typedef struct Mailbox {    
    ServerMailbox serverMailbox;
    ClientMailbox clientMailbox;
} Mailbox;

typedef union message_response_types{
    MessageType message;
    ResponseType responseType;
} message_response_types;

typedef struct DataBundle{
    message_response_types type;
    Account account;
    pid_t pid; //PID for sign in process
    int response;
} DataBundle;


typedef struct genericMessage {
    long int message_type;

    DataBundle data;
} GenericMessage;


typedef struct System_Memory {
    int process_count;
    pid_t process_IDs[7];
} System_Memory;

const char filename[20] = "DB_file.txt";

void initSystemMemory(System_Memory *sys_mem){

    sys_mem->process_count = 0;

    for(int x = 0; x < 7; x++){
        sys_mem->process_IDs[x] = -1;
    }

}

int addProcessToSystem(System_Memory *sys_mem, pid_t pid){

    if(sys_mem->process_count > 6){
        return -1;
    }else{
        sys_mem->process_IDs[sys_mem->process_count] = pid;
        sys_mem->process_count++;
    }

    return 1;

}

void resetDataBundle( DataBundle * DataBundle){

    DataBundle->account.accountNumber = 0;
    DataBundle->account.pin = 0;
    DataBundle->account.funds = 0.0;

    DataBundle->type.message = 0;

    DataBundle->response = 0;

}

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


void exit_program(){
    exit(0);
}

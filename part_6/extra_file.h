
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/msg.h>


const int shmKey = 99999;

const int semKey = 99999;

const int msgKey = 99999;


const int BLOCK = 1;
const int NOBLOCK = -1;


typedef union sharedVar
{
    char character;
    int number;
} sharedVar;



// Shared Memory Interface

const char buffer[256];

int getShmId(){
    
    int shmid_1;

    if ((shmid_1 = shmget(shmKey, sizeof(buffer), 0666 | IPC_CREAT)) == -1)
    {
        perror( "Error in shmget");
    }

    return shmid_1;
}

void *attachToSharedMemory(int shmid){

    void * ptr;

    if ( ( ptr = (int * ) shmat (shmid, (void *)0, 0 )) == (int *)-1){
        perror( "Error in shmat");
    }

    return ptr;

}

void detachSharedMemory(void * SharedMemoryPointer){
    if(shmdt(SharedMemoryPointer) == -1){
        perror( "Error in shmdt");
    }
}



void deleteSharedMemory(int shmID){
    if(shmctl(shmID, IPC_RMID, (struct shmid_ds *) NULL ) == -1){
        perror( "Error in shmdt");
    }
}

// Semaphore Interface

int getSemId(){
    int semId_1;

    if ((semId_1 = semget(semKey, 1, 0666 | IPC_CREAT)) == -1)
    {
        perror( "Error in shmget");
    }

    return semId_1;
}

int getSemIdForNSems(int sems){
    int semId_1;

    if ((semId_1 = semget(semKey, sems, 0666 | IPC_CREAT)) == -1)
    {
        perror( "Error in shmget");
    }

    return semId_1;
}

int semInit(int semId){
    int semId_1;

    if ( semId != -1 && (semId_1 = semctl(semId, 0, SETVAL, 1)) == -1)
    {
        perror( "Error in semctl (SETVAL)");
    }

    return semId_1;
}

int semInitForNthSemInSet(int semId, int n){
    int semId_1;

    if ( semId != -1 && (semId_1 = semctl(semId, n, SETVAL, 1)) == -1)
    {
        perror( "Error in semctl (SETVAL)");
    }

    return semId_1;
}


int semDelete(int semId){
    int semId_1;

    if (  semId != -1 && (semId_1 = semctl(semId, 0, IPC_RMID, 0)) == -1)
    {
        perror( "Error in semctl (IPC_RMID)");
    }

    return semId_1;
}


int semDeleteNthSemInSet(int semId, int n){
    int semId_1;

    if (  semId != -1 && (semId_1 = semctl(semId, n, IPC_RMID, 0)) == -1)
    {
        perror( "Error in semctl (IPC_RMID)");
    }

    return semId_1;
}

int SemaphoreWait(int semid, int iMayBlock)
{
    int flag;

    if(iMayBlock > 0){
        flag = SEM_UNDO;
    }else{
        flag = IPC_NOWAIT;
    }

    struct sembuf sbOperation;
    sbOperation.sem_num = 0;
    sbOperation.sem_op = -1;
    sbOperation.sem_flg = flag;
    return semop(semid, &sbOperation, 1);
}

int SemaphoreWaitNthSemInSet(int semid, int iMayBlock, int n)
{
    int flag;

    if(iMayBlock > 0){
        flag = SEM_UNDO;
    }else{
        flag = IPC_NOWAIT;
    }

    struct sembuf sbOperation;
    sbOperation.sem_num = n;
    sbOperation.sem_op = -1;
    sbOperation.sem_flg = iMayBlock;
    return semop(semid, &sbOperation, 1);
}

int SemaphoreWaitUntilZero(int semid)
{
    struct sembuf sbOperation;
    sbOperation.sem_num = 0;
    sbOperation.sem_op = 0;
    sbOperation.sem_flg = 0;
    return semop(semid, &sbOperation, 1);
}


int SemaphoreWaitUntilZeroNthSemInSet(int semid, int n)
{
    struct sembuf sbOperation;
    sbOperation.sem_num = n;
    sbOperation.sem_op = 0;
    sbOperation.sem_flg = 0;
    return semop(semid, &sbOperation, 1);
}

int SemaphoreSignal(int semid)
{
    struct sembuf sbOperation;
    sbOperation.sem_num = 0;
    sbOperation.sem_op = +1;
    sbOperation.sem_flg = 0;
    
    return semop(semid, &sbOperation, 1);
}

int SemaphoreSignalNthSemInSet(int semid, int n)
{
    struct sembuf sbOperation;
    sbOperation.sem_num = n;
    sbOperation.sem_op = +1;
    sbOperation.sem_flg = 0;
    
    return semop(semid, &sbOperation, 1);
}

//Message queue interface

typedef struct my_message {
    long int message_type;

    int data;
}my_message;

int getmsgQueueID(){
    int messageQueueId;

    if( (messageQueueId = msgget((key_t) msgKey, IPC_CREAT| 0600))  == -1 ){
        perror( "Error in msgget");
        exit(1);
    }

    return messageQueueId;
}

/* int getNewMsgQueueID(){
    int messageQueueId;

    if( (messageQueueId = msgget((key_t) IPC_PRIVATE, IPC_CREAT| 0600))  == -1 ){
        perror( "Error in msgget");
        exit(1);
    }

    return messageQueueId;
} */

int sendMessage(int messageQueueId, my_message message, int shouldIBlockIfFull){

    int flag;

    if(shouldIBlockIfFull > 0){
        flag = 0;
    }else{
        flag = IPC_NOWAIT;
    }

    int msgLength = sizeof(my_message) - sizeof(long);

    int result;

    if ( (result = msgsnd(messageQueueId, &message, (size_t) msgLength, flag) == -1))
    {
        perror("msgsnd: msgsnd failed");
        exit(1);
    }
   
   return result;
}

int receiveMessage(int messageQueueId, my_message *message, int shouldIBlockIfFull){


    int flag;

    if(shouldIBlockIfFull > 0){
        flag = 0;
    }else{
        flag = IPC_NOWAIT;
    }

    int msgLength = sizeof(my_message) - sizeof(long);

    long type = message->message_type;

    int result;

    if ( (result = msgrcv(messageQueueId, message,(size_t) msgLength,type , flag) == -1))
    {
        perror("msgsnd: msgsnd failed");
        exit(1);
    }
   
   return result;
}

int deleteMessageQueue(int messageQueueId){

    int result; 
    if (( result = msgctl(messageQueueId, IPC_RMID, (struct msqid_ds *) NULL)) == -1)
    {
        perror("msgctl: msgctl failed");
        exit(1);
    }

    return result;
}
#include <stdlib.h>     
#include <stdio.h>     
#include <stdbool.h>   
#include <stdint.h>    
#include <string.h>     
#include <unistd.h>     
#include <stdarg.h>     
#include <errno.h>      
#include <signal.h>    
#include <sys/ipc.h>   
#include <sys/msg.h>
#include <sys/shm.h>    
#include <sys/sem.h>    
#include <sys/time.h>   
#include <sys/types.h>  
#include <sys/wait.h>  
#include <time.h>      
#include <math.h>
#include "shared.h"

#define BUFFER_LENGTH 4096
#define MAX_FILE_LINE 100000

#define TERMINATION_TIME 20
#define MAX_PROCESS 18
#define TOTAL_PROCESS 100

#define PROCESS_SIZE 32000
#define PAGE_SIZE 1000
#define MAX_PAGE (PROCESS_SIZE / PAGE_SIZE)

#define MEMORY_SIZE 256000
#define FRAME_SIZE PAGE_SIZE
#define MAX_FRAME (MEMORY_SIZE / FRAME_SIZE)

//global variables
static char *exe_name;
static int exe_index;
static key_t key;

static int m_queue_id = -1;
static Message user_message;
static int shm_clock_shm_id = -1;
static SharedClock *shm_clock_shm_ptr = NULL;
static int sem_id = -1;
static struct sembuf sema_operation;
static int pcbt_shm_id = -1;
static ProcessControlBlock *pcbt_shm_ptr = NULL;

void processInterrupt();
void processHandler(int signum);
void resumeHandler(int signum);
void discardShm(void *shmaddr, char *shm_name , char *exe_name, char *process_type);
void cleanUp();
void semaLock(int sem_index);
void semaRelease(int sem_index);
void getSharedMemory();

int main(int argc, char *argv[]) 
{
	//signal handling
	processInterrupt();

	exe_name = argv[0];
	exe_index = atoi(argv[1]);
	srand(getpid());


	//shared memory 
	getSharedMemory();
	
	bool is_terminate = false;
	int memory_reference = 0;
	unsigned int address = 0;
	unsigned int request_page = 0;
	while(1)
	{
		//Waiting for master signal to get resources
		msgrcv(m_queue_id, &user_message, (sizeof(Message) - sizeof(long)), getpid(), 0);
		//DEBUG fprintf(stderr, "%s (%d): my index [%d]\n", exe_name, getpid(), user_message.index);

		if(memory_reference <= 1000)
		{
			//- Requesting Memory -//
			address = rand() % 32768 + 0;
			request_page = address >> 10;
			memory_reference++;
		}
		else
		{
			is_terminate = true;
		}
		
			
		//Send a message to master that I got the signal and master should invoke an action base on my data
		user_message.mtype = 1;
		user_message.flag = (is_terminate) ? 0 : 1;
		user_message.address = address;
		user_message.requestPage = request_page;
		msgsnd(m_queue_id, &user_message, (sizeof(Message) - sizeof(long)), 0);

		if(is_terminate)
		{
			break;
		}
	}

	cleanUp();
	exit(exe_index);
}

void processInterrupt()
{
	struct sigaction sa1;
	sigemptyset(&sa1.sa_mask);
	sa1.sa_handler = &processHandler;
	sa1.sa_flags = SA_RESTART;
	if(sigaction(SIGUSR1, &sa1, NULL) == -1)
	{
		perror("ERROR");
	}

	struct sigaction sa2;
	sigemptyset(&sa2.sa_mask);
	sa2.sa_handler = &processHandler;
	sa2.sa_flags = SA_RESTART;
	if(sigaction(SIGINT, &sa2, NULL) == -1)
	{
		perror("ERROR");
	}
}
void processHandler(int signum)
{
	printf("%d: Terminated!\n", getpid());
	cleanUp();
	exit(2);
}

void discardShm(void *shmaddr, char *shm_name , char *exe_name, char *process_type)
{
	//Detaching...
	if(shmaddr != NULL)
	{
		if((shmdt(shmaddr)) << 0)
		{
			fprintf(stderr, "%s (%s) ERROR: could not detach [%s] shared memory!\n", exe_name, process_type, shm_name);
		}
	}
}

void cleanUp()
{
	//Release shmclock shared memory
	discardShm(shm_clock_shm_ptr, "shmclock", exe_name, "Child");

	//Release pcbt shared memory
	discardShm(pcbt_shm_ptr, "pcbt", exe_name, "Child");
}

void semaLock(int sem_index)
{
	sema_operation.sem_num = sem_index;
	sema_operation.sem_op = -1;
	sema_operation.sem_flg = 0;
	semop(sem_id, &sema_operation, 1);
}

void semaRelease(int sem_index)
{	
	sema_operation.sem_num = sem_index;
	sema_operation.sem_op = 1;
	sema_operation.sem_flg = 0;
	semop(sem_id, &sema_operation, 1);
}

void getSharedMemory()
{
	key = ftok("./oss.c", 1);
	m_queue_id = msgget(key, 0600);
	if(m_queue_id < 0)
	{
		fprintf(stderr, "%s ERROR: could not get [message queue] shared memory! Exiting...\n", exe_name);
		cleanUp();
		exit(EXIT_FAILURE);
	}

	key = ftok("./oss.c", 2);
	shm_clock_shm_id = shmget(key, sizeof(SharedClock), 0600);
	if(shm_clock_shm_id < 0)
	{
		fprintf(stderr, "%s ERROR: could not get [shmclock] shared memory! Exiting...\n", exe_name);
		cleanUp();
		exit(EXIT_FAILURE);
	}

	//Attaching shared memory and check if can attach it. 
	shm_clock_shm+ptr = shmat(shm_clock_shm_id, NULL, 0);
	if(shm_clock_shm_ptr == (void *)( -1 ))
	{
		fprintf(stderr, "%s ERROR: fail to attach [shmclock] shared memory! Exiting...\n", exe_name);
		cleanUp();
		exit(EXIT_FAILURE);	
	}

	//semaphore
	key = ftok("./oss.c", 3);
	sem_id = semget(key, 1, 0600);
	if(sem_id == -1)
	{
		fprintf(stderr, "%s ERROR: fail to attach a private semaphore! Exiting...\n", exe_name);
		cleanUp();
		exit(EXIT_FAILURE);
	}

	//process control block table
	key = ftok("./oss.c", 4);
	size_t process_table_size = sizeof(ProcessControlBlock) * MAX_PROCESS;
	pcbt_shm_id = shmget(key, process_table_size, 0600);
	if(pcbt_shm_id < 0)
	{
		fprintf(stderr, "%s ERROR: could not get [pcbt] shared memory! Exiting...\n", exe_name);
		cleanUp();
		exit(EXIT_FAILURE);
	}

	//Attaching shared memory and check if can attach it.
	pcbt_shm_ptr = shmat(pcbt_shm_id, NULL, 0);
	if(pcbt_shm_ptr == (void *)( -1 ))
	{
		fprintf(stderr, "%s ERROR: fail to attach [pcbt] shared memory! Exiting...\n", exe_name);
		cleanUp();
		exit(EXIT_FAILURE);	
	}
}


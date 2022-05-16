#define MY_SHARED_H
#include <stdbool.h>
#include "constant.h"


/* New type variable */
typedef unsigned int uint;


/* SharedClock */
typedef struct 
{
	unsigned int second;
	unsigned int nanosecond;
}SharedClock;


/* Message */
typedef struct
{
	long mtype;
	int index;
	pid_t childPid;
	int flag;	//0 : isDone | 1 : isQueue
	unsigned int address;
	unsigned int requestPage;
	char message[BUFFER_LENGTH];
}Message;


/* PageTableEntry */
typedef struct
{
	uint frameNo;
	uint address: 8;
	uint protection: 1; 
	uint dirty: 1;     
	uint valid: 1;      
}PageTableEntry; 

/* ProcessControlBlock */
typedef struct
{
	int pidIndex;
	pid_t actualPid;
	PageTableEntry page_table[MAX_PAGE];
}ProcessControlBlock;


#endif


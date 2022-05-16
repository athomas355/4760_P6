#include <stdlib.h>     
#include <stdio.h>      
#include <stdbool.h>    
#include <stdint.h>     
#include <string.h>     
#include <unistd.h>     
#include "helper.h"
#include "queue.h"

Queue *createQueue()
{
	Queue *q = (Queue *)malloc(sizeof(Queue));
	q->front = NULL;
	q->rear = NULL;
	q->count = 0;
	return q;
}

QNode *newQNode(int index)
{ 
    QNode *temp = (QNode *)malloc(sizeof(QNode));
    temp->index = index;
    temp->next = NULL;
    return temp;
} 

void enQueue(Queue *q, int index) 
{ 
	//Create a new LL node
	QNode *temp = newQNode(index);

	//Increase queue count
	q->count = q->count + 1;

	//If queue is empty, then new node is front and rear both
	if(q->rear == NULL)
	{
		q->front = q->rear = temp;
		return;
	}

	//Add the new node at the end of queue and change rear 
	q->rear->next = temp;
	q->rear = temp;
}

QNode *deQueue(Queue *q) 
{
	//If queue is empty, return NULL
	if(q->front == NULL) 
	{
		return NULL;
	}

	//Store previous front and move front one node ahead
	QNode *temp = q->front;
	free(temp);
	q->front = q->front->next;

	//If front becomes NULL, then change rear also as NULL
	if(q->front == NULL)
	{
		q->rear = NULL;
	}

	//Decrease queue count
	q->count = q->count - 1;
	return temp;
} 


bool isQueueEmpty(Queue *q)
{
	if(q->rear == NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}


int getQueueCount(Queue *q)
{
	return (q->count);	
}

char *getQueue(const Queue *q)
{
	char buf[4096];
	QNode next;
	next.next = q->front;

	sprintf(buf, "Queue: ");
	while(next.next != NULL)
	{
		sprintf(buf, "%s%d", buf, next.next->index);
		
		next.next = (next.next->next != NULL) ? next.next->next : NULL;
		if(next.next != NULL)
		{
			sprintf(buf, "%s, ", buf);
		}
	}
	sprintf(buf, "%s\n", buf);

	return strduplicate(buf);
}


#include <stdlib.h>     
#include <stdio.h>      
#include <stdbool.h>    
#include <stdint.h>     
#include <string.h>     
#include <unistd.h>     
#include "helper.h"
#include "linklist.h"

List *createList()
{
	List *l = (List *)malloc(sizeof(List));
	l->front = NULL;
	return l;
}

LNode *newLNode(int index, int page, int frame)
{ 
    LNode *temp = (LNode *)malloc(sizeof(LNode));
    temp->index = index;
	temp->page = page;
	temp->frame = frame;
    temp->next = NULL;
    return temp;
}

void addListElement(List *l, int index, int page, int frame)
{
	LNode *temp = newLNode(index, page, frame);

	if(l->front == NULL)
	{
		l->front = temp;
		return;
	}
	
	LNode *next = l->front;
	while(next->next != NULL)
	{
		next = next->next;
	}
	next->next = temp;
}

void deleteListFirst(List *l) 
{
    if(l->front == NULL)
    {
        return;
    }
    
    LNode *temp = l->front;
    l->front = l->front->next;
    free(temp);
}

int deleteListElement(List *l, int index, int page, int frame)
{
	LNode *current = l->front;
    LNode *previous = NULL;
    
    if(current == NULL)
    {
        return -1;
    }
    
    while(current->index != index || current->page != page || current->frame != frame)
    {
        if(current->next == NULL)
        {
            return -1;
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }
    
    if(current == l->front)
    {
		int x = current->frame;
		free(current);
        l->front = l->front->next;
		return x;
    }
    else
    {
		int x = previous->next->frame;
		free(previous->next);
        previous->next = current->next;
		return x;
    }
}

bool isInList(List *l, int key) 
{
    LNode next;
    next.next = l->front;

    if(next.next == NULL) 
    {
        return false;
    }

    while(next.next->frame != key) 
    {
        if(next.next->next == NULL) 
        {
            return false;
        }
        else 
        {
            next.next = next.next->next;
        }
    }      
	
    return true;
}

char *getList(const List *l) 
{
	char buf[4096];
    LNode next;
    next.next = l->front;
    
    if(next.next == NULL) 
    {
        return strduplicate(buf);
    }
    
	sprintf(buf, "Linked List: ");
    while(next.next != NULL) 
    {
        sprintf(buf, "%s(%d | %d| %d)", buf, next.next->index, next.next->page, next.next->frame);
        
        next.next = (next.next->next != NULL) ? next.next->next : NULL;
		if(next.next != NULL)
		{
			sprintf(buf, "%s, ", buf);
		}
    }
	sprintf(buf, "%s\n", buf);

	return strduplicate(buf);
}


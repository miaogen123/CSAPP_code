#pragma once
#include<stdio.h>
#include"Mem.h"
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

#define MAX_HEAP 100

static char *mem_heap; //the first byte of heap
static char *mem_brk;  //the last byte plus 1
static char *mem_max_addr;   //Max legal heap addr plus 1

void mem_init(void)
{
	mem_heap=(char*)Malloc(MAX_HEAP);
	mem_brk=mem_heap;
	mem_max_addr=(mem_heap+MAX_HEAP);
}

//extend the heap by incr, and return the pointer point to the first bytes
void *mem_sbrk(int incr)
{
	char *old_brk=mem_brk;
	if((incr<0)||((mem_brk+incr)>mem_max_addr)){
		errno=ENOMEM;
		fprintf(stderr, "ERROR :mem_sbrk failed. ran out of memory\n");
		return (void*)-1;
	}
	mem_brk+=incr;
	return (void*)old_brk;
}
inline const void* Return_Head()
{
	return mem_heap;
}
inline const void* Return_Tail()
{
	return  mem_brk;
}
inline const void* Return_MaxAddr()
{
	return  mem_max_addr;
}

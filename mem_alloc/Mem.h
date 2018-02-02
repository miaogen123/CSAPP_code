#pragma once

#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<stdio.h>

inline void unix_error(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}

void* Malloc(size_t size)
{
	void *temp;
	temp=malloc(size);
	if(temp==NULL)
	{
		unix_error("malloc error");
		exit(1);
	}
	return temp;
}

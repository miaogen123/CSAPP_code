#pragma once

#include<errno.h>
#include<string.h>
#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>

pid_t Fork();
void unix_error(char *msg);

pid_t Fork()
{
	int pid_t;
	if((pid_t=fork())<0)
	{
		unix_error("Fork error");
	}
	return pid_t;
}

inline void unix_error(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}

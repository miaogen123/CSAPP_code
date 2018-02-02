#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/types.h>

int main(void)
{
	int i=100;
	while(i>0)
	{

		printf("i'm  here\n");
		sleep(1);
		i--;

	}

}

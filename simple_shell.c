
#include<stdio.h>
#include"Fork.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>

#define MAXCHAR 40
#define MAXARGS  20

void eval (char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

char **g_environ;

int main(int argc, char *argv[], char *envp[])
{
	g_environ=envp;
	//while(*envp!=NULL)
	//	printf("the envp:%s\n", *envp++);
	char cmdline[MAXCHAR];
	while(1){
		printf("> ");
		fgets(cmdline, MAXCHAR, stdin);
		printf("%s\n", cmdline);
		if(feof(stdin))
			exit(0);
		eval(cmdline);
	}
}


void eval (char *cmdline)
{
	char  *argv[MAXARGS];
	char buf[MAXCHAR];
	int bg;
	pid_t pid;
	//strncpy(buf, cmdline, sizeof(*cmdline));
	strcpy(buf, cmdline);
	printf("%s\n", buf);
	bg=parseline(buf, argv);
	if(argv[0]==NULL)
		return ;
	if(!builtin_command(argv)){
		if((pid=Fork())==0){
			printf("the cmd is in 50:%s\n", argv[0]);
			if(execve(argv[0], argv, g_environ)<0)
			{
				printf("%s:cmd not found\n", argv[0]);
				exit(0);
			}
		}
		if(!bg){
			int status;
			if(waitpid(pid, &status, 0)<0)
				unix_error("waitfg:waitpid error\n");
		}
		else
			printf("%d %s", pid, cmdline);
	}
	return;
}
int builtin_command(char **argv)
{
	if(!strcmp(argv[0], "quit"))
		exit(0);
	if(!strcmp(argv[0], "&"))
		return 1;
	return 0;
}

int parseline(char *buf, char **argv)
{
	char *delim;
	int argc;
	int bg;
	buf[strlen(buf)-1]=' ';
	while(*buf &&(*buf==' '))
		buf++;
	argc =0;
	while((delim=strchr(buf, ' ')))
	{
		argv[argc++]=buf;
		*delim='\0';
		buf=delim +1;
		while(*buf&&(*buf==' '))
				buf++;
	}
	argv[argc]=NULL;
	if(argc==0)
		return 1;
	if((bg=(*argv[argc-1]=='&'))!=0)
		argv[--argc]=NULL;
	return bg;

}




#include<stdio.h>
#include"Fork.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>



//the max chars a cmd can have.
#define MAXCHAR 40
//the max argvs . .   .   have.
#define MAXARGS  20

//DEBUG MODE
#define  DEBUG



//the proc info struct
typedef struct sim_pcb{
	pid_t pid;
	pid_t ppid;
	//1:the bg proc, 2:the fg proc
	unsigned short stat;
	//1:running or 0:sleeping
	unsigned short running;
	char name[MAXCHAR];
	
	//to be added
	//short priority;

	struct sim_pcb *next;
}Pcb;

//pass to the execve
char **g_environ;
Pcb *g_pcb;

//signal handler
typedef void (*sighandler)(int);

#ifdef DEBUG
void test_handler(int signum);
#endif

//attention: here return a void * but when using should convert it to a Pcb *
inline void * Assign_to_Simpcb( pid_t pid, pid_t ppid, unsigned short stat, unsigned short \
		running, char *name);
void eval (char *cmdline,struct sim_pcb* pcb);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void View_Jobs();


int main(int argc, char *argv[], char *envp[])
{
	g_environ=envp;
	char cmdline[MAXCHAR];
	struct sim_pcb *pcb;

	//initialize the head
	//can be simlified by Assign_to_Simpcb;
	pcb=(struct sim_pcb*)malloc(sizeof(struct sim_pcb));
	pcb->pid=getpid();
	pcb->ppid=0;
	pcb->stat=1;
	pcb->running=1;
	strncpy(pcb->name, "bash", 5);
	pcb->next=NULL;
	g_pcb=pcb;
	
	if(signal(SIGTSTP,test_handler)==SIG_ERR)
		unix_error("signal error");

	while(1){
		printf("> ");
		fgets(cmdline, MAXCHAR, stdin);
		if(feof(stdin))
			exit(0);
		eval(cmdline, pcb);
		printf("\n");
	}
}


void eval (char *cmdline, struct sim_pcb* pcb)
{
	char  *argv[MAXARGS];
	char buf[MAXCHAR];
	int bg;
	pid_t pid;
	//strncpy(buf, cmdline, sizeof(*cmdline));
	strcpy(buf, cmdline);
	bg=parseline(buf, argv);

#ifdef DEBUG
	char **temp_envir=g_environ;
	printf("\n");
	printf("test g_environ\n");
//	while(*temp_envir!=NULL)
//		printf("the temp_envir:%s\n", *temp_envir++);
	printf("end\n");
	printf("\n");
#endif

	if(argv[0]==NULL)
		return ;
	if(!builtin_command(argv)){
		if((pid=Fork())==0){
			if(execve(argv[0], argv, g_environ)<0)
			{
				printf("%s:cmd not found\n", argv[0]);
				exit(0);
			}
			else
				exit(1);
		}
		else if(pid>0){
			//add the proc into the linklist pcb
			Pcb *new_pcb=(Pcb*)Assign_to_Simpcb(pid, getpid(),0, 0, argv[0]);
			new_pcb->next=pcb->next;
			pcb->next=new_pcb;
			if(!bg){
				pcb->next->stat=1;
				int status;
				if(waitpid(pid, &status, 0)<0)
					unix_error("waitfg:waitpid error\n");
				else
				{
					Pcb *temp_pcb=pcb->next;
					pcb->next=pcb->next->next;
					free(temp_pcb);
				}
			}
			else{
				pcb->next->stat=1;
				printf("%d %s", pid, cmdline);
			}

		}
	}
	return;
}
int builtin_command(char **argv)
{
	if(!strcmp(argv[0], "quit"))
		exit(0);
	if(!strcmp(argv[0], "&"))
		return 1;
	if(!strcmp(argv[0], "jobs"))
	{
		//View_Jobs();
		return 1;
	}
	if(!strcmp(argv[0], "bg"))
	{
		//TODO
	}
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


inline void * Assign_to_Simpcb( pid_t pid, pid_t ppid, unsigned short stat, unsigned short \
		running, char *name)
{
	Pcb * pcb;
	pcb=(struct sim_pcb*)malloc(sizeof(struct sim_pcb));
	pcb->pid=pid;
	pcb->ppid=ppid;
	pcb->stat=stat;
	pcb->running=running;
	strncpy(pcb->name,name, 5);
	pcb->next=NULL;
	return (void*)pcb;
	
}

void test_handler(int signum)
{
	Pcb* temp_pcb=g_pcb->next;
	while(temp_pcb!=NULL){
		printf("pid=%d, ppid=%d, name=%s, stat=%d, running=%d\n", temp_pcb->pid, temp_pcb->ppid, temp_pcb->name \ 
				,temp_pcb->stat, temp_pcb->running);
			temp_pcb=temp_pcb->next;
	}

}

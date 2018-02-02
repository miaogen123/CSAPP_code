
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
//#define  DEBUG



//the proc info struct
typedef struct sim_pcb{
	pid_t pid;
	pid_t ppid;
	//1:the bg proc, 2:the fg proc
	unsigned short stat;
	//1:running or 0:sleeping(stop) 2:end
	unsigned short running;
	char name[MAXCHAR];
	
	//to be added
	//short priority;

	struct sim_pcb *next;
}Pcb;

//signal handler
typedef void (*sighandler)(int);



//pass to the execve
char **g_environ;
Pcb *g_pcb;


#ifdef DEBUG
void test_handler(int signum);
#endif

//to override a part of signals
void SIGCHLD_handler(int signum);
void SIGTSTP_handler(int signum);
void SIGINT_handler(int signum);

//attention: here return a void * but when using should convert it to a Pcb *
inline void * Assign_to_Simpcb( pid_t pid, pid_t ppid, unsigned short stat, unsigned short \
		running, char *name);
void eval (char *cmdline,struct sim_pcb* pcb);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void View_Jobs();
void Put_Proc_Front(char *pid);
void Put_Proc_Back(char *pid);


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
	
#ifdef DEBUG
if(signal(SIGTSTP,test_handler)==SIG_ERR)
		unix_error("signal error");
#endif

	if(signal(SIGTSTP,SIGTSTP_handler)==SIG_ERR)
		unix_error("signal error");
	if(signal(SIGINT,SIGINT_handler)==SIG_ERR)
		unix_error("signal error");
	if(signal(SIGCHLD,SIGCHLD_handler)==SIG_ERR)
		unix_error("signal error");

	while(1){
		printf("mg> ");
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
			Pcb *new_pcb=(Pcb*)Assign_to_Simpcb(pid, getpid(),1, 1, argv[0]);
			new_pcb->next=pcb->next;
			pcb->next=new_pcb;
			if(!bg){
				pcb->next->stat=1;
				int status;
				int temp_pid;
#ifdef DEBUG
				printf("here\n");
#endif 
				temp_pid=waitpid(pid, &status, WUNTRACED);
				//temp_pid=waitpid(pid, &status, 0);
#ifdef DEBUG
				printf("after here:temp_pid=%d\n", temp_pid);
#endif 
				if(temp_pid<0)
					unix_error("waitfg:waitpid error\n");
				else
				{
					if(pcb->next->running!=0){
						Pcb *temp_pcb=pcb->next;
						pcb->next=pcb->next->next;
						free(temp_pcb);
					}
				}
			}
			else{
				pcb->next->stat=0;
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
		View_Jobs();
		return 1;
	}
	if(!strcmp(argv[0], "bg"))
	{
		Put_Proc_Back(argv[1]);
		return 1;
	}
	if(!strcmp(argv[0], "fg"))
	{
		Put_Proc_Front(argv[1]);
		return 1;
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
	size_t size=(strlen(name)>20)?20:strlen(name);
	strncpy(pcb->name,name, size);
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

//view the jobs in the bg.
void View_Jobs()
{
	Pcb * temp_pcb=g_pcb->next;
	while(temp_pcb!=NULL){

#ifdef DEBUG
		printf("the stat=%d\t and the running=%d\n", temp_pcb->stat, temp_pcb->running );
#endif
		if(0==temp_pcb->stat)
		{
			if(kill(temp_pcb->pid, 0)<0)
					temp_pcb->running=2;
			printf("pid=%d, ppid=%d, name=%s, stat=%d, ", temp_pcb->pid, temp_pcb->ppid, temp_pcb->name \ 
				,temp_pcb->stat);
			switch(temp_pcb->running)
			{
				case 0:printf("sleeping(stopped)\n");break;
				case 1:printf("running\n");break;
				case 2:printf("end\n");break;
				default:break;
			}
			
		}
		temp_pcb=temp_pcb->next;
	}
}

//to override a part of signals
void SIGCHLD_handler(int signum)
{
	pid_t pid;
	int status;
    while((pid=waitpid(-1, &status, WNOHANG))>0)
		printf("pid %d is end \n", pid);
	return;
}
//to override a part of signals
void SIGTSTP_handler(int signum)
{
	Pcb* temp_pcb=g_pcb->next;
	while(temp_pcb!=NULL)
	{
		kill(temp_pcb->pid, SIGTSTP);
		temp_pcb->running =0;
		temp_pcb->stat=0;
		printf("job %d fell asleep by signal SIGTSTP caused by ctrl+Z\n", temp_pcb->pid);
		temp_pcb=temp_pcb->next;
#ifdef DEBUG 
	printf("IN THE  SIGTSTP handler\n");
#endif
	}
#ifdef DEBUG 
	printf("end SIGTSTP handler\n");
#endif
}
//to override a part of signals
void SIGINT_handler(int signum)
{
	Pcb* temp_pcb=g_pcb->next,*pre_pcb=g_pcb;
	while(temp_pcb!=NULL)
	{
		kill(temp_pcb->pid, SIGINT);
		printf("job %d end by signal SIGINT caused by ctrl+C\n", temp_pcb->pid);
		pre_pcb->next=temp_pcb->next;
		free(temp_pcb);
		temp_pcb=pre_pcb->next;
#ifdef DEBUG 
	printf("IN THE  SIGINT handler\n");
#endif
	}
#ifdef DEBUG 
	printf("end SIGINT handler\n");
#endif

}
//to "fg" the proc
void Put_Proc_Front(char *pid)
{
	int pid_tru=atoi(pid);
	Pcb *temp_pcb=g_pcb->next;
	int status, flag=0;
	while(temp_pcb!=NULL)
	{
		if(temp_pcb->pid==pid_tru)
		{
			kill(pid_tru, SIGCONT );
			temp_pcb->stat=1;
			temp_pcb->running=1;
			waitpid(pid_tru,&status, WUNTRACED );
			flag=1;
			break;
		}
		temp_pcb=temp_pcb->next; 
	}
	if(!flag){
		printf("no pid %d  match, please check your input\n", pid_tru );
	}
}


//to "bg" the proc
void Put_Proc_Back(char *pid)
{
	int pid_tru=atoi(pid);
	Pcb *temp_pcb=g_pcb->next;
	int status, flag=0;
	while(temp_pcb!=NULL)
	{
		if(temp_pcb->pid==pid_tru)
		{
			kill(pid_tru, SIGCONT );
			temp_pcb->running=1;
			temp_pcb->stat=0;
			flag=1;
			break;
		}
		temp_pcb=temp_pcb->next; 
	}
	if(!flag){
		printf("no pid %d  match, please check your input\n", pid_tru );
	}

}

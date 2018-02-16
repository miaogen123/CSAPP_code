#include<stdio.h>
#include<sys/mman.h> 
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<netdb.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include"RIO.h"
#include"Fork.h"
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h> 

//#define DEBUG

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filetype);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


int main(int argc, char **argv)
{
	int listenfd, connfd;
	char hostname[MAXLINE], port[MAXLINE];
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;

	if (argc != 2) {
		fprintf(stderr, "usage:%s<port>\n", argv[0]);
		exit(1);
	}

	listenfd = open_listenfd(argv[1]);
	if (listenfd == -1) {
		fprintf(stderr, "error happened in tiny.c in listenfd\n");
		exit(1);
	}
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
#ifdef DEBUG
		printf("accept success: %d\n", connfd);
#endif
		if (connfd == -1) {
			fprintf(stderr, "error happened in tiny.c in accept\n");
			exit(1);
		}
		getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		printf("accept from (%s %s)\n", hostname, port);

#ifdef DEBUG
		printf("doit ing...: %d\n", connfd);
#endif
		doit(connfd);
#ifdef DEBUG
		printf("close ing...: %d\n", connfd);
#endif

		close(connfd);
	}
}

void doit(int fd)
{
#ifdef DEBUG
		printf("inside  doit ing...: \n");
#endif
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	printf("the size of MAXLINE is %d\n", MAXLINE);

	rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	printf("Request: headers:\n");
	printf("%s", buf);
	sscanf(buf, "%s %s %s", method, uri, version);
	if (strcasecmp(method, "GET")) {
		clienterror(fd, method, "501", "not implement", "tiny does not implement this method");
		return;
	}
	read_requesthdrs(&rio);

	is_static = parse_uri(uri, filename, cgiargs);
	if (stat(filename, &sbuf) < 0) {
		clienterror(fd, filename, "404", "not found", "tiny couldn't find this fiel");
		return;
	}
	if (is_static){
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR&sbuf.st_mode)) {
			clienterror(fd, filename, "403", "forbidden", "tiny couldn't read the file");
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else {
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR &sbuf.st_mode)) {
			clienterror(fd, filename, "403", "forbidden", "tiny couldn't run the cgifile");
			return;
		}
		serve_dynamic(fd, filename, cgiargs);
	}

}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXBUF];
	sprintf(body, "<html><title>tiny error/<title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s:%s\r\n", body, longmsg, shortmsg);
	sprintf(body, "%s<p>%s:%s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>the tiny web server</em>\r\n", body);

	// confused: why it write every time.
	sprintf(buf, "http/1.0 %s %s \r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "content-type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "content-length:%d\r\n\r\n", (int)strlen(body));
	Rio_writen(fd, buf, strlen(buf));
	Rio_writen(fd, body, strlen(buf));

}

void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];
	Rio_readlineb(rp, buf, MAXLINE);
	while (strcmp(buf, "\r\n")) {
		Rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);
	}
	return;
}

int parse_uri(char * uri, char *filename, char *cgiargs)
{
	char *ptr;

	if (!strstr(uri, "cgi-bin")) {
		strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		if (uri[strlen(uri) - 1] == '/')
			strcat(filename, "home.html");
		return 1;
	}
	else {
		ptr = index(uri, '?');
		if (ptr) {
			strcpy(cgiargs, ptr + 1);
			*ptr = '\0';
		}
		else
			strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}
}

void serve_static(int fd, char *filename, int filesize)
{
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXLINE];

	get_filetype(filename, filetype);
	sprintf(buf, "http/1.0 200 ok\r\n");
	sprintf(buf, "%sServer:tiny web server\r\n", buf);
	sprintf(buf,"%sConnection:close\r\n",buf);
	sprintf(buf,"%sContent-length:%d",buf,filesize);
	sprintf(buf,"%sContent-type:%s\r\n\r",buf,filetype);
	Rio_writen(fd, buf, strlen(buf));
	printf("Response headers:\n");
	printf("%s", buf);

	srcfd = open(filename, O_RDONLY, 0);
	if (srcfd == -1) {
		fprintf(stderr, " srcfd error\n");
		exit(1);
	}
	srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);
	Rio_writen(fd, srcp, filesize);
	munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype)
{
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".png"))
		strcpy(filetype, "image/png");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else
		strcpy(filetype, "text/plain");
	return;
}


void serve_dynamic(int fd, char *filename, char *cgiargs)
{
	char buf[MAXLINE], *emptylist[] = { NULL };
	sprintf(buf, "http/1.0 200 OK\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server:tiny web server\r\n");
	printf("hello\n");
	Rio_writen(fd, buf, strlen(buf));
#ifdef DEBUG
	printf("in dynamic\n");
#endif
	if (Fork() == 0) {
		setenv("QUERY_STRING", cgiargs, 1);
		printf("hill\n");
		dup2(fd, STDOUT_FILENO);
		execve(filename, emptylist, NULL);
	}
	wait(NULL);
}

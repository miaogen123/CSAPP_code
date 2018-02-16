
#include<sys/types.h>
#include<string.h> 
#include<errno.h>
#include<stdlib.h>
#include"RIO.h"
#include<stdio.h>
#include<unistd.h>

 ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
	size_t nleft=n;
	ssize_t nread;
	char *bufp=(char *)usrbuf;

	while(nleft>0){
		if(nread=read(fd, bufp, nleft)<0){
			if(errno=EINTR)
				nread=0;
			else 
				return -1;
		}
		else if(nread==0)
			break;
		nleft-=nread;
		bufp+=nread;
	}
	return n-nleft;
}
 ssize_t rio_writen( int fd, void *usrbuf, size_t n )
{
	size_t nleft=n;
	ssize_t nwritten;
	char *bufp=(char*)usrbuf;
	while(nleft>0){
		if((nwritten =write(fd, bufp, nleft))<=0){
			if(errno==EINTR)
				nwritten=0;
			else
				return -1;
		}
		nleft=nleft-nwritten;
		bufp+=nwritten;

	}
	return n;
}

void rio_readinitb(rio_t *rp, int fd)
{
	rp->rio_fd=fd;
	rp->rio_cnt=0;
	rp->rio_bufptr=rp->rio_buf;
}

 static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
	int cnt;
	while(rp->rio_cnt<=0){
		rp->rio_cnt =read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if(rp->rio_cnt<0){
			if(errno!=EINTR)
				return -1;
		}
		else if(rp->rio_cnt==0)
			return 0;
		else
			rp->rio_bufptr=rp->rio_buf;
	}
	cnt=(rp->rio_cnt<n)?rp->rio_cnt:n;
	memcpy(usrbuf, rp->rio_bufptr, cnt);
	rp->rio_bufptr+=cnt;
	rp->rio_cnt-=cnt;
	return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void*usrbuf, size_t maxlen)
{
	int n, rc;
	char *bufp=(char*)usrbuf, c;
	for(n=1;n<maxlen;++n){
		if((rc=rio_read(rp, &c, 1))==1){
			*bufp++=c;
			if(c=='\n'){
				n++;
				break;
			}
		}
		else if(rc==0){
			if(n==1)
				return 0;
			else
				break;
		}
		else 
			return -1;
	}
	*bufp=0;
	return n-1;


}

 ssize_t rio_readnb(int fd, void *usrbuf, size_t n)
{
	size_t nleft=n;
	ssize_t nread;
	char *bufp=(char *)usrbuf;

	while(nleft>0){
		if(nread=read(fd, bufp, nleft)<0){
			return -1;
		}
		else if(nread==0)
			break;
		nleft-=nread;
		bufp+=nread;
	}
	return n-nleft;
}

ssize_t Rio_readlineb(rio_t *rp, void* usrbuf, size_t maxlen)
{
	ssize_t temp;
	if((temp=(rio_readlineb(rp, usrbuf, maxlen)))==-1){
		fprintf(stderr, "rio_readlineb error");
		exit(1);
	}
	else
		return temp;
}
ssize_t Rio_readnb(int fd, void *usrbuf, size_t n)
{
	ssize_t temp;
	if(temp=(rio_readnb(fd, usrbuf, n))==-1){
		fprintf(stderr, "rio_readnb error");
		exit(1);
	}
	else
		return temp;
}
ssize_t Rio_readn(int fd, void *usrbuf, size_t n)
{
	ssize_t temp;
	if(temp=(rio_readn(fd, usrbuf, n))==-1){
		fprintf(stderr, "rio_readn error");
		exit(1);
	}
	else
		return temp;
}
ssize_t Rio_writen( int fd, void *usrbuf, size_t n )
{
	ssize_t temp;
	if(temp=(rio_writen(fd, usrbuf, n))==-1){
		fprintf(stderr, "rio_written  error");
		exit(1);
	}
	else
		return temp;
}

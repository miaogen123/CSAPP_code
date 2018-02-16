#ifndef ROBUST_IO
#define ROBUST_IO

#define RIO_BUFSIZE 8192
typedef struct{
	int rio_fd;
	int rio_cnt;
	char *rio_bufptr;
	char rio_buf[RIO_BUFSIZE]; 
}rio_t;
#define MAXLINE 100
#define MAXBUF 1000

typedef struct sockaddr SA;

void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readn(int fg, void *usrbuf, size_t n);
ssize_t rio_writen( int fd, void *usrbuf, size_t n );
ssize_t rio_readlineb(rio_t *rp, void*usrbuf, size_t maxlen);
ssize_t rio_readnb(int fd, void *usrbuf, size_t n);

ssize_t Rio_readlineb(rio_t *rp, void*usrbuf, size_t maxlen);
ssize_t Rio_readnb(int fd, void *usrbuf, size_t n);
ssize_t Rio_readn(int fg, void *usrbuf, size_t n);
ssize_t Rio_writen( int fd, void *usrbuf, size_t n );

#endif //ROBUST_IO

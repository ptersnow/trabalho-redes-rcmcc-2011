/***************************************************************************
             httpd.h

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#ifndef __HTTPD_H
#define __HTTPD_H

#define PENDING 10
#define MAXBUF 256
#define SERVER_PORT 8080
#define PROTOCOL "HTTP/1.1"
#define SERVER "FACOM-MCC-RC-2011/1.0"

#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <regex.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct request {
	int fd;
	int rfd;			/* serve requests on fd */
  int post_fd;
  int do_cache;
	int keepalive;
  int content_length;

	char *path;
	char *program;
	char method[5];
	char version[10];
	char buffer[8192];
	
	time_t last_time;
	
	char *url;
	char *host;
	int port;

} request;

typedef struct thread {
	int sock;
	sem_t sem;
	short busy;
	request *req;
} thread;

int n;
fd_set readfd;
thread *threads;
pthread_t *pool;


extern int sigint_flag;

/*                                  request.c                                 */

int do_cgi(request *req);
void handle_reques(int socket);
request *get_request(int socket);
void process_request(request *req);


/*                                  server.c                                  */

void usage(char *exec);
char * get_mime_type(char *name);
void perror_exit(const char *msg);
void send_error(int sock, int status, char *title, char *text);
void send_file(int sock, char *method, char *path, struct stat *statbuf);
void send_headers(int sock, int status, char *title, char *mime, int length);


/*                                  signals.c                                 */

void initSignals();


/*                                  post.c                                    */

int post(request *req);
int create_temporary_file();

#endif

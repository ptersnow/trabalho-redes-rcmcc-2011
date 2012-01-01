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

#include "network.h"
#include "my_socket.h"

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

typedef struct list {
	char *string;
	regex_t regexp;
	struct list *prox;
} list;

list *deny;
list *allow;

typedef struct lruc_item {
  int age;
  char *value;
  char *key;
  int value_length;
  int key_length;
  int access_count;
  struct lruc_item *next;
} lruc_item;

typedef struct lruc {
  sem_t lock;
  time_t seed;
  int free_memory;
  int access_count;
  lruc_item **items;
  int hash_table_size;
  lruc_item *free_items;
} lruc;

lruc *cache;

int n;
int proxy;
int total_memory;
char *access_list;
char *cache_directory;

sem_t sem_deny;
sem_t sem_allow;

fd_set readfd;
thread *threads;
pthread_t *pool;

extern int msock;
extern int mport;
extern int sigint_flag;

/*                                  request.c                                 */

int do_cgi(request *req);
request *get_request(int socket);
void process_request(request *req);


/*                                  server.c                                  */

void usage(char *exec);
char * get_mime_type(char *name);
int openProxy(char *host, int port);
void send_error(int sock, int status, char *title, char *text);
void proxy_error(int sock, char *title, char *head, char *body);
void send_file(int sock, char *method, char *path, struct stat *statbuf);
void send_headers(int sock, int status, char *title, char *mime, int length);


/*                                  signals.c                                 */

void initSignals();


/*                                  list.c                                    */

void lopen();
void free_list();
void print(list *ini);
void inicializa_lista();
int search(char *string);
void insert_list(FILE *file);
void search_insert(char *x, list *ini);
char *get_regerror(int errcode, regex_t *compiled);


/*                                  post.c                                    */

int post(request *req);
int create_temporary_file();


/*                                  config.c                                  */

int config(char *file);


/*                                  cache.c                                  */

void lruc_free();
void lruc_remove_lru_item();
void lruc_new(int average_length);
char *lruc_get(char *key, int key_length);
void lruc_delete(char *key, int key_length);
int lruc_set(char *key, int key_length, char *value, int value_length);

#endif

/***************************************************************************
             httpd.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"

int msock;
int mport;
int sigint_flag = 0;

void *work(void *argv);
static void openSock();
static void process(int socket);
static void runThread(int socket, int n);

static void
process(int socket)
{
	int fd;
	request *req;
	struct sockaddr_in sin;
	socklen_t size = sizeof(struct sockaddr_in);

	printf("process\n");

	while(!sigint_flag) {
		if((fd = my_accept(socket, &sin, size)) < 0) {
			perror("accept");
			break;
		}
		
		if(!fork()) {
			printf("forked: ");
			
			req = get_request(fd);
			process_request(req);
			if(req)
				free(req);
			_exit(0);
		}
	}
}

void *
work(void *argv)
{
	thread *th = (thread *) argv;
	
	while(!sigint_flag) {
		sem_wait(&th->sem);
		if(th->busy == 0) {
			sem_post(&th->sem);
			sleep(1);
			continue;
		}
		sem_post(&th->sem);
		
		th->req = get_request(th->sock);
		process_request(th->req);
		
		sem_wait(&th->sem);
		th->busy = 0;
		if(th->req)
			free(th->req);
		sem_post(&th->sem);
	}
	
	pthread_exit(0);
}

static void
runThread(int socket, int n)
{
	int i, j;
	socklen_t size;
	struct timeval tv;
	struct sockaddr_in sin;
	
	threads = (thread *) malloc(n * sizeof(thread));
	pool = (pthread_t*) malloc(n * sizeof(pthread_t));
	
	printf("%d threads\n", n);

	for(i = 0; i < n; i++) {
		threads[i].busy = 0;
		sem_init(&threads[i].sem, 0, 1);
		pthread_create(&pool[i], NULL, work, (void*) &threads[i]);
	}

	while(!sigint_flag) {

		tv.tv_sec = 10;
		tv.tv_usec = 0;

		FD_ZERO(&readfd);
		FD_SET(socket, &readfd);

		if(!sigint_flag && (select(FD_SETSIZE, &readfd, NULL, NULL, &tv) < 0)) {
			if(errno == EAGAIN || errno == EINTR)
				continue;
			perror("th->select");
		}

		for(j = 0; j < FD_SETSIZE; j++)
		if(FD_ISSET(j, &readfd)) {
			for(i = 0; i < n; i++) {
				sem_wait(&threads[i].sem);
				if(threads[i].busy == 0) {
					if(j == socket) {
						size = sizeof(sin);
						if(!sigint_flag && ((threads[i].sock = accept(j, (struct sockaddr *)&sin, &size)) < 0))
							perror("th->accept");
						else {
							FD_SET(threads[i].sock, &readfd);
							threads[i].busy = 1;
						}
					}
					else {
						threads[i].sock = j;
						threads[i].busy = 1;
					}
				}
				sem_post(&threads[i].sem);
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	char c;
	short fflag = 0;
  
	n = 0;
	while((c = getopt(argc, argv, "ft:")) != -1)
		switch(c) {
		case 'f':
			if(n != 0)
				usage(argv[0]);
			fflag = 1;
			break;
		case 't':
			if(fflag == 1)
				usage(argv[0]);
			n = atoi(optarg);
			break;
		case '?':
			if(optopt == 't')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if(isprint (optopt))
				fprintf(stderr, "Unknown option ‘-%c’.\n", optopt);
			else fprintf(stderr,"Unknown option character ‘\\x%x’.\n", optopt);
			return 1;
			break;
		default:
			usage(argv[0]);
		}

	initSignals();
	
	proxy = 0;
	 
	if(fflag || n)
		if(optind != argc)
			mport = atoi(argv[optind]);
		else mport = SERVER_PORT;
	else { 
		proxy = 1;
		
		if(optind != argc)
			mport = config(argv[optind]);
		else usage(argv[0]);
		
		lopen();
		lruc_new(1024);
	}

	openSock();
	printf("%s is listening on port %d \n", argv[0], mport);
	
	if(fflag == 1)
		process(msock);
	else runThread(msock, n);
	
	return 0;
}

static void
openSock()
{
	int yes = 1;
	struct sockaddr_in sin;

	if(proxy) {
	
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(mport);
	
		if((msock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			_exit(1);
		}

		setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if(bind(msock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
			perror("bind");
			_exit(1);
		}
		
		if(listen(msock, PENDING) < 0) {
			perror("listen");
			_exit(1);
		}
	}
	else {
		init_socket(mport, 0);
		
		printf("my_socket\n");
		if((msock = my_socket()) < 0)
			printf("error\n");
	
		printf("my_bind\n");
		if(my_bind(msock, mport) < 0)
			printf("error\n");
	
		printf("my_listen\n");
		if(my_listen(msock, PENDING) < 0)
			printf("error\n");
	}
}


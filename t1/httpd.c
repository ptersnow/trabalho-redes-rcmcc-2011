/***************************************************************************
             httpd.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"

int sigint_flag = 0;

void *work(void *argv);
static int openSock(int port);
static void process(int socket);
static void runThread(int socket, int n);

static void
process(int socket)
{
	int i, fd;
	socklen_t size;
	struct sockaddr_in sin;

	printf("process\n");

	while(!sigint_flag) {
	
		if((fd = accept(socket, (struct sockaddr *)&sin, &size)) < 0) {
			perror("accept");
			continue;
		}
			
		if(!fork()) {
			close(socket);
			
			handle_request(fd);			
			_exit(0);
		}
		close(fd);
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
		
		handle_request(th->sock);
		
		sem_wait(&th->sem);
		th->busy = 0;
		sem_post(&th->sem);
	}
	
	pthread_exit(0);
}

static void
runThread(int socket, int n)
{
	int fd, i;
	socklen_t size;
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

		if((fd = accept(socket, (struct sockaddr *)&sin, &size)) < 0) {
			perror("accept");
			continue;
		}

		for(i = 0; i < n; i++) {
			sem_wait(&threads[i].sem);
			if(threads[i].busy == 0) {
				threads[i].sock = fd;
				threads[i].busy = 1;
				sem_post(&threads[i].sem);
				break;
			}
			sem_post(&threads[i].sem);
		}
	}
}

int
main(int argc, char *argv[])
{
	char c;
	short fflag = 0;
  int port, msock;
  
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
	
	
	if(optind != argc)
		port = atoi(argv[optind]);
	else port = SERVER_PORT;
	
	msock = openSock(port);

	printf("%s is listening on port %d \n", argv[0], port);

	if(fflag == 1)
		process(msock);
	else runThread(msock, n);
	
	return 0;
}

int
openSock(int port)
{
	int msock;
	int yes = 1;
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	
	if((msock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror_exit("socket");

	setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	setsockopt(msock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));

	if(bind(msock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		perror_exit("bind");

	if(listen(msock, PENDING) < 0)
		perror_exit("listen");
	
	return msock;
}


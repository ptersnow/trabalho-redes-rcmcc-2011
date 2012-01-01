/***************************************************************************
             signals.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"
#include <sys/wait.h>

void sigint_handler(int s);
void sigchld_handler(int s);

void
initSignals()
{

	struct sigaction sa;
	
	sa.sa_flags = 0;
	
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGCHLD);
  
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);

	sa.sa_handler = sigchld_handler;
	sigaction(SIGCHLD, &sa, NULL);
}

void
sigchld_handler(int s)
{
	int pid, status;
	
	while((pid = waitpid(WAIT_ANY, &status, WNOHANG)) > 0) {
		if(WIFEXITED(status))
			printf("child[%d] exited normally with status %d\n", pid, WEXITSTATUS(status));
	}
}

void
sigint_handler(int s)
{
	int i;
	
	sigint_flag = 1;
	printf("SIGINT\n");
	
	if(threads) {
		printf("Ending threads... \n");
		for(i = 0; i < n; i++)
			sem_destroy(&threads[i].sem);
		free(threads);
		free(pool);
	}
	
	printf("Closing sockets... \n");
	for(i = 0; i < FD_SETSIZE; i++) {
		close(i);
		FD_CLR(i, &readfd);
	}
	
	printf("Exiting... \n");
	_exit(0);
}

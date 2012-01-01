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
void sighup_handler(int s);
void sigchld_handler(int s);

void
initSignals()
{

	struct sigaction sa;
	
	sa.sa_flags = 0;
	
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGCHLD);
  
	sa.sa_handler = sighup_handler;
	sigaction(SIGHUP, &sa, NULL);

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
	
	if(proxy) {
		printf("Removing cache directory... \n");
		rmdir(cache_directory);
		free(cache_directory);
		free(access_list);
		free_list();
		if(cache) {
			printf("Access count: %d\n", cache->access_count);
			lruc_free();
		}
	}
	
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
	
	if(usocket)
		my_close(usocket);
	
	printf("Exiting... \n");
	_exit(0);
}

void
sighup_handler(int s)
{
	printf("Sinal de HUP (Hang up) capturado...\n");
    
    /* Quando o usuário atualiza a lista, removo a lista e insiro novamente,
     * pois alguma permissão pode ter sido removida da lista de acesso 
     * e não pensei num bom jeito de se atualizá-la */
     
	free_list();	
  lopen();
  
  signal(SIGHUP, sighup_handler);
}

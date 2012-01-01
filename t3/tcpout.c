/* tcpout.c - tcpout */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpout - handle events affecting TCP output processing
 *------------------------------------------------------------------------
 */
void *
tcpout(void)
{
	int i;
	struct tcb *ptcb;
	char buffer[TCPPKTLEN + 1];
	struct mq_attr attr;

	/* initialize the queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = TCPQLEN;
  attr.mq_msgsize = TCPPKTLEN;
  attr.mq_curmsgs = 0;

	tcps_oport = mq_open(MQ_OUTPUT, O_RDWR | O_CREAT, 0644, &attr);
	mq_unlink(MQ_OUTPUT);
	if(tcps_oport != -1)
		printf("thread tcpout criada %d....\n", tcps_oport);
	else {
		perror("tcps_oport");
		exit(1);
	}
	
	while (1) {
		memset(&buffer, 0, sizeof(buffer));
		mq_receive(tcps_oport, buffer, TCPPKTLEN, NULL);
		printf("\nout -> preceive");
		
		i = atoi(buffer);
		ptcb = &tcbtab[TCB(i)];
		if(ptcb->tcb_state <= TCPS_CLOSED)
			continue;		/* a rogue; ignore it	*/

		sem_wait(&ptcb->tcb_mutex);
		if(ptcb->tcb_state <= TCPS_CLOSED)
			continue;		/* TCB deallocated	*/
		
		if(EVENT(i) == DELETE)		/* same for all states	*/
			tcbdealloc(ptcb);
		else {
			printf("\nout -> tcposwitch\n");
			tcposwitch[ptcb->tcb_ostate](TCB(i), EVENT(i));
		}
		
		if(ptcb->tcb_state != TCPS_FREE)
			sem_post(&ptcb->tcb_mutex);
	}
	
	pthread_exit(0);
}

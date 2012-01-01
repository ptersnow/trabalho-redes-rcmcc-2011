/* tcpinp.c - tcpinp */

#include "network.h"

int tcps_lqsize;
mqd_t tcps_oport, tcps_iport;
sem_t tcps_tmutex;

/*------------------------------------------------------------------------
 *  tcpinp  -  handle TCP segment coming in from IP
 *------------------------------------------------------------------------
 */
void *
tcpinp(void)
{
	struct udp *pudp;
	struct tcp *ptcp;
	struct tcb *ptcb;
	char buffer[TCPPKTLEN + 1];
	struct mq_attr attr;

	/* initialize the queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = TCPQLEN;
  attr.mq_msgsize = TCPPKTLEN;
  attr.mq_curmsgs = 0;

	tcps_iport = mq_open(MQ_INPUT, O_RDWR | O_CREAT, 0644, &attr);
	mq_unlink(MQ_INPUT);
	if(tcps_iport != -1)
		printf("thread tcpinp criada %d....\n", tcps_iport);
	else {
		perror("tcps_iport");
		exit(1);
	}
	
	while (1) {
		memset(&buffer, 0, sizeof(buffer));
		mq_receive(tcps_iport, buffer, TCPPKTLEN, NULL);
		printf("in -> preceive\n");
		
		pudp = (struct udp *)buffer;
		if(!pudp)
			break;
			
		ptcp = (struct tcp *)pudp->data;
		if(ptcp->tcp_cksum != tcpcksum(pudp, pudp->length - UDPMHLEN)) {
			printf("TCP input Errors: %d\n", ++TcpInErrs);
			continue;
		}
		
		printf("in -> ");
		tcpnet2h(ptcp); /* convert all fields to host order */
		ptcb = tcpdemux(pudp);

		if(!ptcb) {
			printf("TCP input Errors: %d\n", ++TcpInErrs);
			tcpreset(pudp);
			continue;
		}
		
		printf("in -> ");
		if(!tcpok(ptcb, pudp))
			tcpackit(ptcb, pudp);
		else {
			printf("tcb_state=%d\n", ptcb->tcb_state);
			tcpopts(ptcb, pudp);
			tcpswitch[ptcb->tcb_state](ptcb, pudp);
		}
		
		if(ptcb->tcb_state != TCPS_FREE)
			sem_post(&ptcb->tcb_mutex);
	}
	
	pthread_exit(0);
}

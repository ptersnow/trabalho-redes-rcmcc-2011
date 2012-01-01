/* tcpsynrcvd.c - tcpsynrcvd */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpsynrcvd -  do SYN_RCVD state input processing
 *------------------------------------------------------------------------
 */
int
tcpsynrcvd(struct tcb *ptcb, struct udp *pudp)
{
	char buffer[10];
	struct tcb *pptcb;
	struct mq_attr attr;
	struct tcp *ptcp = (struct tcp *)pudp->data;

	printf("tcpsynrcvd: ");
	if(ptcp->tcp_code & TCPF_RST) {
		printf("ptcp->tcp_code & TCPF_RST\n");
		TcpAttemptFails++;
		if(!ptcb->tcb_pptcb)
			return tcbdealloc(ptcb);
		else return tcpabort(ptcb, TCPE_REFUSED);
	}
	
	if(ptcp->tcp_code & TCPF_SYN) {
		printf("ptcp->tcp_code & TCPF_SYN\n");
		TcpAttemptFails++;
		tcpreset(pudp);
		return tcpabort(ptcb, TCPE_RESET);
	}
	
	if(tcpacked(ptcb, pudp) == -1) {
		printf("tcpacked == -1\n");
		return 0;
	}
		
	if(ptcb->tcb_pptcb) {		/* from a passive open	*/
		pptcb = ptcb->tcb_pptcb;
		sem_wait(&pptcb->tcb_mutex);
		if(pptcb->tcb_state != TCPS_LISTEN) {
			TcpAttemptFails++;
			tcpreset(pudp);
			sem_post(&pptcb->tcb_mutex);
			return tcbdealloc(ptcb);
		}
		
		mq_getattr(pptcb->tcb_listenq, &attr);
	  if(attr.mq_curmsgs >= pptcb->tcb_lqsize) {
			TcpAttemptFails++;
			tcpreset(pudp);
			sem_post(&pptcb->tcb_mutex);
			return tcbdealloc(ptcb);
		}
		
		sprintf(buffer, "%d", ptcb->tcb_dvnum);
		if(mq_send(pptcb->tcb_listenq, buffer, strlen(buffer), 0) < 0)
			perror("tcpsynrcvd: mq_send");
		sem_post(&pptcb->tcb_mutex);
	}
	else sem_post(&ptcb->tcb_ocsem); /* from an active open	*/
	
	TcpCurrEstab++;
	ptcb->tcb_state = TCPS_ESTABLISHED;
	tcpdata(ptcb, pudp);
	
	if(ptcb->tcb_flags & TCBF_RDONE)
		ptcb->tcb_state = TCPS_CLOSEWAIT;
		
	return 0;
}

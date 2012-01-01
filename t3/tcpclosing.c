#include "network.h"

/*------------------------------------------------------------------------
 *  tcpclosing -  do CLOSING state input processing
 *------------------------------------------------------------------------
 */
int
tcpclosing(struct tcb *ptcb, struct udp *pudp)
{
	struct	tcp	*ptcp = (struct tcp *)pudp->data;

	printf("tcpclosing\n");
	if(ptcp->tcp_code & TCPF_RST)
		return tcbdealloc(ptcb);
	if(ptcp->tcp_code & TCPF_SYN) {
		tcpreset(pudp);
		return tcbdealloc(ptcb);
	}
	tcpacked(ptcb, pudp);
	if((ptcb->tcb_code & TCPF_FIN) == 0) {
		ptcb->tcb_state = TCPS_TIMEWAIT;
		sem_post(&ptcb->tcb_ocsem);		/* wake closer	*/
		tcpwait(ptcb);
	}
	return 0;
}

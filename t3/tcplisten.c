/* tcplisten.c - tcplisten */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcplisten -  do LISTEN state processing
 *------------------------------------------------------------------------
 */

int
tcplisten(struct tcb *ptcb, struct udp *pudp)
{
	struct	tcb	*newptcb;
	struct	tcp	*ptcp = (struct tcp*)(pudp->data);

	printf("tcplisten ");
	if(ptcp->tcp_code & TCPF_RST) {
		printf("return 0\n");
		return 0;		/* "parent" TCB still in LISTEN */
	}
	if((ptcp->tcp_code & TCPF_ACK) || (ptcp->tcp_code & TCPF_SYN) == 0)
		return tcpreset(pudp);
		
	newptcb = &tcbtab[tcballoc()];
	if(!newptcb || tcpsync(newptcb) == -1) {
		printf("return -1\n");
		return -1;
	}
	
	newptcb->tcb_state = TCPS_SYNRCVD;
	newptcb->tcb_ostate = TCPO_IDLE;
	newptcb->tcb_error = 0;
	newptcb->tcb_pptcb = ptcb;			/* for ACCEPT	*/

	newptcb->tcb_rip = pudp->src;
	newptcb->tcb_rport = ptcp->tcp_sport;
	newptcb->tcb_lip = pudp->dst;
	newptcb->tcb_lport = ptcp->tcp_dport;

	tcpwinit(ptcb, newptcb, pudp);	/* initialize window data	*/

	newptcb->tcb_finseq = newptcb->tcb_pushseq = 0;
	newptcb->tcb_flags = TCBF_NEEDOUT;
	TcpPassiveOpens++;
	tcpdata(newptcb, pudp);
	sem_post(&(newptcb->tcb_mutex));
  
	return 0;
}

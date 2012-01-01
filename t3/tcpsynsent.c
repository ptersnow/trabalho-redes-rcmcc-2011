/* tcpsynsent.c - tcpsynsent */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpsynsent -  do SYN_SENT state processing
 *------------------------------------------------------------------------
 */
int
tcpsynsent(struct tcb *ptcb, struct udp *pudp)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;

	printf("tcpsynsent: ");
	if((ptcp->tcp_code & TCPF_ACK) &&
	   (((ptcp->tcp_ack - ptcb->tcb_iss) <= 0) ||
	   ((ptcp->tcp_ack - ptcb->tcb_snext) > 0)))
		return tcpreset(pudp);
		
	if(ptcp->tcp_code & TCPF_RST) {
		ptcb->tcb_state = TCPS_CLOSED;
		ptcb->tcb_error = TCPE_RESET;
		TcpAttemptFails++;
		tcpkilltimers(ptcb);
		sem_post(&ptcb->tcb_ocsem);
		printf("tcpkilltimers 0\n");
		return 0;
	}
	
	if((ptcp->tcp_code & TCPF_SYN) == 0) {
		printf("ptcp->tcp_code & TCPF_SYN\n");
		return 0;
	}
		
	ptcb->tcb_swindow = ptcp->tcp_window;
	ptcb->tcb_lwseq = ptcp->tcp_seq;
	ptcb->tcb_rnext = ptcp->tcp_seq;
	ptcb->tcb_cwin = ptcb->tcb_rnext + ptcb->tcb_rbsize;
	tcpacked(ptcb, pudp);
	tcpdata(ptcb, pudp);
	ptcp->tcp_code &= ~TCPF_FIN;
	
	if(ptcb->tcb_code & TCPF_SYN)		/* our SYN not ACKed	*/
		ptcb->tcb_state = TCPS_SYNRCVD;
	else {
		TcpCurrEstab++;
		ptcb->tcb_state = TCPS_ESTABLISHED;
		sem_post(&ptcb->tcb_ocsem);	/* return in open	*/
	}
	return 0;
}

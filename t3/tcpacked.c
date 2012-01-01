/* tcpacked.c - tcpacked */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpacked - handle in-bound ACKs and do round trip estimates
 *------------------------------------------------------------------------
 */
int
tcpacked(struct tcb *ptcb, struct udp *pudp)
{
	struct	tcp	*ptcp = (struct tcp *)pudp->data;
	int acked, value, cacked;

	if(!(ptcp->tcp_code & TCPF_ACK))
		return -1;
	acked = ptcp->tcp_ack - ptcb->tcb_suna;
	cacked = 0;
	if(acked <= 0)
		return 0;	/* duplicate ACK */
	if(SEQCMP(ptcp->tcp_ack, ptcb->tcb_snext) > 0) {
		if(ptcb->tcb_state == TCPS_SYNRCVD)
			return tcpreset(pudp);
		else
			return tcpackit(ptcb, pudp);
	}
	tcprtt(ptcb);
	ptcb->tcb_suna = ptcp->tcp_ack;
	if(acked && ptcb->tcb_code & TCPF_SYN) {
		acked--;
		cacked++;
		ptcb->tcb_code &= ~TCPF_SYN;
		ptcb->tcb_flags &= ~TCBF_FIRSTSEND;
	}
	if((ptcb->tcb_code & TCPF_FIN) &&
	    SEQCMP(ptcp->tcp_ack, ptcb->tcb_snext) == 0) {
		acked--;
		cacked++;
		ptcb->tcb_code &= ~TCPF_FIN;
		ptcb->tcb_flags &= ~TCBF_SNDFIN;
	}
	if((ptcb->tcb_flags & TCBF_SUPOK) &&
	    SEQCMP(ptcp->tcp_ack, ptcb->tcb_supseq) >= 0)
		ptcb->tcb_flags &= ~TCBF_SUPOK;
	ptcb->tcb_sbstart = (ptcb->tcb_sbstart+acked) % ptcb->tcb_sbsize;
	ptcb->tcb_sbcount -= acked;
	if(acked) {
		sem_getvalue(&ptcb->tcb_ssema, &value);
		if(value <= 0)
			sem_post(&ptcb->tcb_ssema);
	}
	
	printf("tcpostate acked=%d\n", acked);
	tcpostate(ptcb, acked+cacked);
	return acked;
}

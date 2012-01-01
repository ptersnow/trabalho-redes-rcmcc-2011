/* tcpok.c - tcpok */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpok -  determine if a received segment is acceptable
 *------------------------------------------------------------------------
 */
int
tcpok(struct tcb *ptcb, struct udp *pudp)
{
	int seglen, rwindow, rv;
	long wlast, slast;
	struct tcp *ptcp = (struct tcp *)pudp->data;

	printf("tcpok: ");
	if(ptcb->tcb_state < TCPS_SYNRCVD) {
		printf("return 1 ");
		return 1;
	}
	seglen = pudp->length - UDPMHLEN - TCP_HLEN(ptcp);
		
	/* add SYN and FIN */
	if(ptcp->tcp_code & TCPF_SYN)
		++seglen;
	if(ptcp->tcp_code & TCPF_FIN)
		++seglen;
	rwindow = ptcb->tcb_rbsize - ptcb->tcb_rbcount; 
	if(rwindow == 0 && seglen == 0) {
		printf("ptcp->tcp_seq=%ld ptcb->tcb_rnext=%ld ", ptcp->tcp_seq, ptcb->tcb_rnext);
		return ptcp->tcp_seq == ptcb->tcb_rnext;
	}
		
	wlast = ptcb->tcb_rnext + rwindow - 1;
	rv = (((ptcp->tcp_seq - ptcb->tcb_rnext) >= 0) && ((ptcp->tcp_seq - wlast) <= 0));
		
	if(seglen == 0) {
		printf("rv=%d ", rv);
		return rv;
	}
	slast = ptcp->tcp_seq + seglen - 1;
	rv |= (((slast - ptcb->tcb_rnext) >= 0) && ((slast - wlast) <= 0));

	/* If no window, strip data but keep ACK, RST and URG */
  if(rwindow == 0)
  	pudp->length = UDPMHLEN + TCP_HLEN(ptcp);

	printf("return rv=%d ", rv);
	return rv;
}

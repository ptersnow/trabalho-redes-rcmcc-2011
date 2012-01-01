/* tcpswindow.c - tcpswindow */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpswindow -  handle send window updates from remote
 *------------------------------------------------------------------------
 */
int
tcpswindow(struct tcb *ptcb, struct udp *pudp)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;
	long wlast, owlast;

	printf("tcpswindow\n");
	if(SEQCMP(ptcp->tcp_seq, ptcb->tcb_lwseq) < 0)
		return 0;
		
	if (SEQCMP(ptcp->tcp_seq, ptcb->tcb_lwseq) == 0 &&
	    SEQCMP(ptcp->tcp_ack, ptcb->tcb_lwack) < 0)
		return 0;
	/* else, we have a send window update */

	/* compute the last sequences of the new and old windows */

	owlast = ptcb->tcb_lwack + ptcb->tcb_swindow;
	wlast = ptcp->tcp_ack + ptcp->tcp_window;

	ptcb->tcb_swindow = ptcp->tcp_window;
	ptcb->tcb_lwseq = ptcp->tcp_seq;
	ptcb->tcb_lwack = ptcp->tcp_ack;
	if(SEQCMP(wlast, owlast) <= 0)
		return 0;
	/* else,  window increased */
	if(ptcb->tcb_ostate == TCPO_PERSIST) {
		tmclear(tcps_oport, (int) MKEVENT(PERSIST, ptcb-&tcbtab[0]));
		ptcb->tcb_ostate = TCPO_XMIT;
	}
	tcpkick(ptcb);			/* do something with it */
	return 0;
}

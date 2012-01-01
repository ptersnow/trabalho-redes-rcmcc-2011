/* tcpwinit.c - tcpwinit */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpwinit - initialize window data for a new connection
 *------------------------------------------------------------------------
 */
int
tcpwinit(struct tcb *ptcb, struct tcb *newptcb, struct udp *pudp)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;
	unsigned mss;

	newptcb->tcb_swindow = ptcp->tcp_window;
	newptcb->tcb_lwseq = ptcp->tcp_seq;
	newptcb->tcb_lwack = newptcb->tcb_iss;	/* set in tcpsync()	*/

	mss = 536;	/* RFC 1122 */
	if(ptcb->tcb_smss) {
		newptcb->tcb_smss = min(ptcb->tcb_smss, mss);
		ptcb->tcb_smss = 0;		/* reset server smss	*/
	}
	else newptcb->tcb_smss = mss;
	
	newptcb->tcb_rmss = mss;		/* receive mss		*/
	newptcb->tcb_cwnd = newptcb->tcb_smss;	/* 1 segment		*/
	newptcb->tcb_ssthresh = 65535;		/* IP max window	*/
	newptcb->tcb_rnext = ptcp->tcp_seq;
	newptcb->tcb_cwin = newptcb->tcb_rnext + newptcb->tcb_rbsize;
	
	return 0;
}

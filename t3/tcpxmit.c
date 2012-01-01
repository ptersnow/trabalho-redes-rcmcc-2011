/* tcpxmit.c - tcpxmit */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpxmit - handle TCP output events while we are transmitting
 *------------------------------------------------------------------------
 */
int
tcpxmit(int tcbnum, int event)
{
	int tosend, pending, window, tv;
	struct tcb *ptcb = &tcbtab[tcbnum];

	printf("tcpxmit: ");
	if(event == RETRANSMIT) {
		tmclear(tcps_oport, (int) MKEVENT(SEND, tcbnum));
		tcprexmt(tcbnum, event);
		ptcb->tcb_ostate = TCPO_REXMT;
		return 0;
	} /* else SEND */
	
	tosend = tcphowmuch(ptcb);
	if(tosend == 0) {
		if(ptcb->tcb_flags & TCBF_NEEDOUT)
			tcpsend(tcbnum, TSF_NEWDATA);	/* just an ACK */
		if(ptcb->tcb_snext == ptcb->tcb_suna)
			return 0;
		/* still unacked data; restart transmit timer	*/
		tv = (int) MKEVENT(RETRANSMIT, tcbnum);
		if(!tmleft(tcps_oport, tv))
			tmset(tcps_oport, TCPQLEN, tv, ptcb->tcb_rexmt);
		return 0;
	}
	else if(ptcb->tcb_swindow == 0) {
		printf("tcpxmit: swindow == 0\n");
		ptcb->tcb_ostate = TCPO_PERSIST;
		ptcb->tcb_persist = ptcb->tcb_rexmt;
		tcpsend(tcbnum, TSF_NEWDATA);
		tmset(tcps_oport, TCPQLEN, (int) MKEVENT(PERSIST,tcbnum), ptcb->tcb_persist);
		return 0;
	}	/* else, we have data and window */
	
	ptcb->tcb_ostate = TCPO_XMIT;
	window = min(ptcb->tcb_swindow, ptcb->tcb_cwnd);
	pending = ptcb->tcb_snext - ptcb->tcb_suna;
	printf("tcpxmit: window=%d pending=%d ", window, pending);
	while((tcphowmuch(ptcb) > 0) && (pending < window)) {
		tcpsend(tcbnum, TSF_NEWDATA);
		pending = ptcb->tcb_snext - ptcb->tcb_suna;
	}
	
	tv = (int) MKEVENT(RETRANSMIT, tcbnum);
	if(!tmleft(tcps_oport, tv))
		tmset(tcps_oport, TCPQLEN, tv, ptcb->tcb_rexmt);

	return 0;
}

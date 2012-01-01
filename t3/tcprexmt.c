/* tcprexmt.c - tcprexmt */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcprexmt - handle TCP output events while we are retransmitting
 *------------------------------------------------------------------------
 */
int
tcprexmt(int tcbnum, int event)
{
	struct tcb *ptcb = &tcbtab[tcbnum];

	printf("tcprexmt\n");
	if(event != RETRANSMIT)
		return 0;	/* ignore others while retransmitting	*/
	if(++ptcb->tcb_rexmtcount > TCP_MAXRETRIES) {
		tcpabort(ptcb, TCPE_TIMEDOUT);
		return 0;
	}
	tcpsend(tcbnum, TSF_REXMT);
	tmset(tcps_oport, TCPQLEN, (int) MKEVENT(RETRANSMIT, tcbnum), min(ptcb->tcb_rexmt << ptcb->tcb_rexmtcount, TCP_MAXRXT));

	if(ptcb->tcb_ostate != TCPO_REXMT)
		ptcb->tcb_ssthresh = ptcb->tcb_cwnd;	/* first drop	*/
	ptcb->tcb_ssthresh = min(ptcb->tcb_swindow,ptcb->tcb_ssthresh) / 2;
	if(ptcb->tcb_ssthresh < ptcb->tcb_smss)
		ptcb->tcb_ssthresh = ptcb->tcb_smss;
	ptcb->tcb_cwnd = ptcb->tcb_smss;
	return 0;
}

/* tcpostate.c - tcpostate */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpostate - do TCP output state processing after an ACK
 *------------------------------------------------------------------------
 */
int
tcpostate(struct tcb *ptcb, int acked)
{
	if(acked <= 0)
		return 0;	/* no state change */
	if(ptcb->tcb_ostate == TCPO_REXMT) {
		ptcb->tcb_rexmtcount = 0;
		ptcb->tcb_ostate = TCPO_XMIT;
	}
	if(ptcb->tcb_sbcount == 0) {
		ptcb->tcb_ostate = TCPO_IDLE;
		return 0;
	}
	tcpkick(ptcb);
	return 0;
}

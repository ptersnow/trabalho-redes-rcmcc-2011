/* tcppersist.c - tcppersist */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcppersist - handle events while the send window is closed
 *------------------------------------------------------------------------
 */
int
tcppersist(int tcbnum, int event)
{
	struct	tcb	*ptcb = &tcbtab[tcbnum];

	printf("tcppersist\n");
	if(event != PERSIST && event != SEND)
		return 0;	/* ignore everything else */
		
	tcpsend(tcbnum, TSF_REXMT);
	ptcb->tcb_persist = min(ptcb->tcb_persist << 1, TCP_MAXPRS);
	tmset(tcps_oport, TCPQLEN, (int) MKEVENT(PERSIST, tcbnum), ptcb->tcb_persist);
	return 0;
}

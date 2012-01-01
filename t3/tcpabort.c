#include "network.h"

/*------------------------------------------------------------------------
 *  tcpabort -  abort an active TCP connection
 *------------------------------------------------------------------------
 */
int
tcpabort(struct tcb *ptcb, int error)
{
	
	printf("tcpabort\n");
	tcpkilltimers(ptcb);
	ptcb->tcb_flags |= (TCBF_RDONE|TCBF_SDONE);
	ptcb->tcb_error = error;
	tcpwakeup(READERS|WRITERS, ptcb);
	
	return 0;
}

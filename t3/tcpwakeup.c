/* tcpwakeup.c - tcpwakeup */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpwakeup -  wake up processes sleeping for TCP, if necessary
 *	NB: Called with tcb_mutex HELD
 *------------------------------------------------------------------------
 */
void
tcpwakeup(int type, struct tcb *ptcb)
{
	int freelen, value;

	printf("tcpwakeup: ");
	if(type & READERS) {
		sem_getvalue(&ptcb->tcb_rsema, &value);
		if(((ptcb->tcb_flags & TCBF_RDONE) ||
				(ptcb->tcb_rbcount > 0) ||
		    (ptcb->tcb_flags & TCBF_RUPOK)) &&
		    (value <= 0)) {
		  
			sem_post(&ptcb->tcb_rsema);
			printf("READERS->sem_post tcb=%d rsema=%d ", ptcb->tcb_dvnum, (value + 1));
		}
	}
	if(type & WRITERS) {
		freelen = ptcb->tcb_sbsize - ptcb->tcb_sbcount;
		sem_getvalue(&ptcb->tcb_ssema, &value);
		if(((ptcb->tcb_flags & TCBF_SDONE) || freelen > 0) && (value <= 0)) {
			
			sem_post(&ptcb->tcb_ssema);
			printf("WRITERS->sem_post tcb=%d ssema=%d ", ptcb->tcb_dvnum, (value + 1));
		}
		
		/* special for abort */
		sem_getvalue(&ptcb->tcb_ocsem, &value);
		if(ptcb->tcb_error && value > 0) {
			
			sem_post(&ptcb->tcb_ocsem);
			printf("WRITERS->sem_post tcb=%d ocsem=%d ", ptcb->tcb_dvnum, (value + 1));
		}
	}
	printf("done\n");
}

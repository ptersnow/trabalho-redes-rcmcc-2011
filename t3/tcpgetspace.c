/* tcpgetspace.c - tcpgetspace */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpgetspace  -  wait for space in the send buffer
 *	N.B. - returns with tcb_mutex HELD
 *------------------------------------------------------------------------
 */
int
tcpgetspace(struct tcb *ptcb, unsigned len)
{
	printf("tcpgetspace: ");
	if(len > ptcb->tcb_sbsize) {
		printf("return error=%d\n", TCPE_TOOBIG);
		return TCPE_TOOBIG;	/* we'll never have this much	*/
	}
		
	while (1) {
		sem_wait(&ptcb->tcb_ssema);
		sem_wait(&ptcb->tcb_mutex);
		if (ptcb->tcb_state == TCPS_FREE) {
			printf("return -1\n");
			return -1;			/* gone		*/
		}
			
		if(ptcb->tcb_error) {
			tcpwakeup(WRITERS, ptcb);	/* propagate it */
			sem_post(&ptcb->tcb_mutex);
			printf("return tcb_error=%d\n", ptcb->tcb_error);
			return ptcb->tcb_error;
		}
		
		if(len <= ptcb->tcb_sbsize - ptcb->tcb_sbcount) {
			printf("return len=%d\n", len);
			return len;
		}
		sem_post(&ptcb->tcb_mutex);
	}
}

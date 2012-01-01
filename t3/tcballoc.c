/* tcballoc.c - tcballoc */

#include "network.h"

/*------------------------------------------------------------------------
 * tcballoc - allocate a Transmission Control Block
 *------------------------------------------------------------------------
 */
int
tcballoc(void)
{
	int i;

	sem_wait(&tcps_tmutex);
	/* look for a free TCB */

	for(i = 1; i <= Ntcp; i++)
		if(tcbtab[i].tcb_state == TCPS_FREE)
			break;
			
	tcbtab[i].tcb_state = TCPS_CLOSED;
	sem_init(&tcbtab[i].tcb_mutex, 0, 0);

	sem_post(&tcps_tmutex);
	return i;
}

/*------------------------------------------------------------------------
 *  tcbdealloc - deallocate a TCB and free its resources
 *	ASSUMES ptcb->tcb_mutex HELD
 *------------------------------------------------------------------------
 */
int
tcbdealloc(struct tcb *ptcb)
{
	if (ptcb->tcb_state == TCPS_FREE)
		return 0;
	switch (ptcb->tcb_type) {
	case TCPT_CONNECTION:
		tcpkilltimers(ptcb);
		sem_destroy(&ptcb->tcb_ocsem);
		sem_destroy(&ptcb->tcb_ssema);
		sem_destroy(&ptcb->tcb_rsema);
		if(ptcb->tcb_sndbuf)
			free(ptcb->tcb_sndbuf);
		if(ptcb->tcb_rcvbuf)
			free(ptcb->tcb_rcvbuf);
		if (ptcb->tcb_rsegq >= 0)
			freeq(ptcb->tcb_rsegq);
		break;
	case TCPT_SERVER:
		mq_close(ptcb->tcb_listenq);
		break;
	default:
		sem_post(&ptcb->tcb_mutex);
		return -1;
	}
	ptcb->tcb_state = TCPS_FREE;
	sem_destroy(&ptcb->tcb_mutex);
	return 0;
}

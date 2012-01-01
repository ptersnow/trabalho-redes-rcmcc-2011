/* tcpcon.c - tcpcon */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpcon - initiate a connection
 *------------------------------------------------------------------------
 */
int
tcpcon(struct tcb *ptcb)
{
	int		error;

	ptcb->tcb_smss = 536;	/* RFC 1122 */
	ptcb->tcb_rmss = ptcb->tcb_smss;
	ptcb->tcb_swindow = ptcb->tcb_smss;
	ptcb->tcb_cwnd = ptcb->tcb_smss;
	ptcb->tcb_ssthresh = 65535;
	ptcb->tcb_rnext = 0;
	ptcb->tcb_finseq = 0;
	ptcb->tcb_pushseq = 0;
	ptcb->tcb_flags = (TCBF_NEEDOUT | TCBF_FIRSTSEND);
	ptcb->tcb_ostate = TCPO_IDLE;
	ptcb->tcb_state = TCPS_SYNSENT;
	tcpkick(ptcb);
	ptcb->tcb_listenq = -1;
	TcpActiveOpens++;
	
	sem_post(&(ptcb->tcb_mutex));
	sem_wait(&(ptcb->tcb_ocsem));
	
	if((error = ptcb->tcb_error))
  	tcbdealloc(ptcb);
		  
	return error;
}

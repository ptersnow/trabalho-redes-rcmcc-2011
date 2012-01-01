/* tfcoalesce.c - tfcoalesce */

#include "network.h"

/*------------------------------------------------------------------------
 *  tfcoalesce -  join TCP fragments
 *------------------------------------------------------------------------
 */
int
tfcoalesce(struct tcb *ptcb, unsigned datalen, struct tcp *ptcp)
{
	struct tcpfrag *tf = NULL;
	int newcount;

	ptcb->tcb_rnext += datalen;
	ptcb->tcb_rbcount += datalen;
	if (ptcb->tcb_rnext == ptcb->tcb_finseq)
		goto alldone;
	if ((ptcb->tcb_rnext - ptcb->tcb_pushseq) >= 0) {
		ptcp->tcp_code |= TCPF_PSH;
		ptcb->tcb_pushseq = 0;
	}
	if (ptcb->tcb_rsegq < 0)	/* see if this closed a hole */
		return 0;
	tf = (struct tcpfrag *)deq(ptcb->tcb_rsegq);
	if(!tf)
		return 0;
	while ((tf->tf_seq - ptcb->tcb_rnext) <= 0) {
		newcount = tf->tf_len - (ptcb->tcb_rnext - tf->tf_seq);
		if (newcount > 0) {
			ptcb->tcb_rnext += newcount;
			ptcb->tcb_rbcount += newcount;
		}
		if (ptcb->tcb_rnext == ptcb->tcb_finseq)
			goto alldone;
		if ((ptcb->tcb_rnext - ptcb->tcb_pushseq) >= 0) {
			ptcp->tcp_code |= TCPF_PSH;
			ptcb->tcb_pushseq = 0;
		}
		if(tf)
			free(tf);
		tf = (struct tcpfrag *)deq(ptcb->tcb_rsegq);
		if(tf) {
			freeq(ptcb->tcb_rsegq);
			ptcb->tcb_rsegq = -1;
			return 0;
		}
	}
	enq(ptcb->tcb_rsegq, tf, -tf->tf_seq); /* got one too many	*/
	return 0;
alldone:
	if(tf)
		free(tf);
	while((tf = (struct tcpfrag *) deq(ptcb->tcb_rsegq)))
		free(tf);
	freeq(ptcb->tcb_rsegq);
	ptcb->tcb_rsegq = -1;
	ptcp->tcp_code |= TCPF_FIN;
	return 0;
}

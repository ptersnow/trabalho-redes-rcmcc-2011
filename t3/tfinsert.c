/* tfinsert.c - tfinsert */

#include "network.h"

/*------------------------------------------------------------------------
 *  tfinsert - add a new TCP segment fragment to a TCB sequence queue
 *------------------------------------------------------------------------
 */
int
tfinsert(struct tcb *ptcb, long seq, unsigned datalen)
{
	struct tcpfrag *tf;

	if (datalen == 0)
		return 0;
	tf = (struct tcpfrag *) malloc(sizeof(struct tcpfrag));
	tf->tf_seq = seq;
	tf->tf_len = datalen;
	if (ptcb->tcb_rsegq < 0)
		ptcb->tcb_rsegq = newq(NTCPFRAG);
	if(enq(ptcb->tcb_rsegq, tf, -tf->tf_seq) < 0)
		if(tf)
			free(tf);
		
	return 0;
}

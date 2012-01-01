/* tcpsndlen.c - tcpsndlen */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpsndlen - compute the packet length and offset in sndbuf
 *------------------------------------------------------------------------
 */
int
tcpsndlen(struct tcb *ptcb, int rexmt, unsigned int *poff)
{
	unsigned	datalen;

	printf("tcpsndlen: ");
	if(rexmt || (ptcb->tcb_code & TCPF_SYN))
		*poff = 0;
	else *poff = ptcb->tcb_snext - ptcb->tcb_suna;
	
	datalen = ptcb->tcb_sbcount - *poff;
	datalen = min(datalen, ptcb->tcb_swindow);
	datalen = min(datalen, ptcb->tcb_smss);
	
	printf("return %d\n", datalen);
	return datalen;
}

/* tcptimewait.c - tcptimewait */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcptimewait -  do TIME_WAIT state input processing
 *------------------------------------------------------------------------
 */
int
tcptimewait(struct tcb *ptcb, struct udp *pudp)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;

	printf("tcptimewait\n");
	if (ptcp->tcp_code & TCPF_RST)
		return tcbdealloc(ptcb);
	if (ptcp->tcp_code & TCPF_SYN) {
		tcpreset(pudp);
		return tcbdealloc(ptcb);
	}
	tcpacked(ptcb, pudp);
	tcpdata(ptcb, pudp);		/* just ACK any packets */
	tcpwait(ptcb);
	return 0;
}

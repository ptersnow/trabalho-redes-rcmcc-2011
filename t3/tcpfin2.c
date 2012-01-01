#include "network.h"

/*------------------------------------------------------------------------
 *  tcpfin2 -  do FIN_WAIT_2 state input processing
 *------------------------------------------------------------------------
 */
int
tcpfin2(struct tcb *ptcb, struct udp *pudp)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;

	printf("tcpfin2\n");
	if (ptcp->tcp_code & TCPF_RST)
		return tcpabort(ptcb, TCPE_RESET);
	if (ptcp->tcp_code & TCPF_SYN) {
		tcpreset(pudp);
		return tcpabort(ptcb, TCPE_RESET);
	}
	if (tcpacked(ptcb, pudp) == -1)
		return 0;
	tcpdata(ptcb, pudp);	/* for data + FIN ACKing */

	if (ptcb->tcb_flags & TCBF_RDONE) {
		ptcb->tcb_state = TCPS_TIMEWAIT;
		tcpwait(ptcb);
	}
	return 0;
}

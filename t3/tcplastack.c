#include "network.h"

/*------------------------------------------------------------------------
 *  tcplastack -  do LAST_ACK state input processing
 *------------------------------------------------------------------------
 */
int
tcplastack(struct tcb *ptcb, struct udp *pudp)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;

	printf("tcplastack\n");
	if (ptcp->tcp_code & TCPF_RST)
		return tcpabort(ptcb, TCPE_RESET);
	if (ptcp->tcp_code & TCPF_SYN) {
		tcpreset(pudp);
		return tcpabort(ptcb, TCPE_RESET);
	}
	tcpacked(ptcb, pudp);
	if ((ptcb->tcb_code & TCPF_FIN) == 0)
		sem_post(&ptcb->tcb_ocsem);	/* close() deallocs	*/
	return 0;
}

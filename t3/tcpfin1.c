#include "network.h"

/*------------------------------------------------------------------------
 *  tcpfin1 -  do FIN_WAIT_1 state input processing
 *------------------------------------------------------------------------
 */
int
tcpfin1(struct tcb *ptcb, struct udp *pudp)
{
	struct tcp *ptcp	= (struct tcp *)pudp->data;

	printf("tcpfin1\n");
	if (ptcp->tcp_code & TCPF_RST)
		return tcpabort(ptcb, TCPE_RESET);
	if (ptcp->tcp_code & TCPF_SYN) {
		tcpreset(pudp);
		return tcpabort(ptcb, TCPE_RESET);
	}
	if (tcpacked(ptcb, pudp) == -1)
		return 0;
	tcpdata(ptcb, pudp);
	tcpswindow(ptcb, pudp);

	if (ptcb->tcb_flags & TCBF_RDONE) {
		if (ptcb->tcb_code & TCPF_FIN)		/* FIN not ACKed*/
			ptcb->tcb_state = TCPS_CLOSING;
		else {
			ptcb->tcb_state = TCPS_TIMEWAIT;
			sem_post(&ptcb->tcb_ocsem);	/* wake closer	*/
			tcpwait(ptcb);
		}
	} else if ((ptcb->tcb_code & TCPF_FIN) == 0) {
		sem_post(&ptcb->tcb_ocsem);		/* wake closer	*/
		ptcb->tcb_state = TCPS_FINWAIT2;
	}
	return 0;
}

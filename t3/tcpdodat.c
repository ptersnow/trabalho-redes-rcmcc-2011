#include "network.h"

/*------------------------------------------------------------------------
 *  tcpdodat -  do input data processing
 *------------------------------------------------------------------------
 */
int
tcpdodat(struct tcb *ptcb, struct tcp *ptcp, long first, unsigned datalen)
{
	int	wakeup = 0;

	if(ptcb->tcb_rnext == first) {
		printf("tcpdodat: if\n");
		if(datalen > 0) {
			tfcoalesce(ptcb, datalen, ptcp);
			ptcb->tcb_flags |= TCBF_NEEDOUT;
			wakeup++;
		}
		
		if(ptcp->tcp_code & TCPF_FIN) {
			ptcb->tcb_flags |= TCBF_RDONE|TCBF_NEEDOUT;
			ptcb->tcb_rnext++;
			wakeup++;
		}
		
		if(ptcp->tcp_code & (TCPF_PSH | TCPF_URG)) {
			ptcb->tcb_flags |= TCBF_PUSH;
			wakeup++;
		}
		
		if(wakeup) {
			printf("tcpdodat: wakeup ");
			tcpwakeup(READERS, ptcb);
		}
	} 
	else {
		printf("tcpdodat: else\n");
		/* process delayed controls */
		if(ptcp->tcp_code & TCPF_FIN)
			ptcb->tcb_finseq = ptcp->tcp_seq + datalen;
			
		if(ptcp->tcp_code & (TCPF_PSH | TCPF_URG))
			ptcb->tcb_pushseq = ptcp->tcp_seq + datalen;
			
		ptcp->tcp_code &= ~(TCPF_FIN|TCPF_PSH);
		tfinsert(ptcb, first, datalen);
	}
	
	return 0;
}

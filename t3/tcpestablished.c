#include "network.h"

/*------------------------------------------------------------------------
 *  tcpestablished -  do ESTABLISHED state input processing
 *------------------------------------------------------------------------
 */
int
tcpestablished(struct tcb *ptcb, struct udp *pudp)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;

	printf("tcpestabilished: ");
	if(ptcp->tcp_code & TCPF_RST) {
		printf("ptcp->tcp_code & TCPF_RST ");
		TcpEstabResets++;
		TcpCurrEstab--;
		return tcpabort(ptcb, TCPE_RESET);
	}
	
	if(ptcp->tcp_code & TCPF_SYN) {
		printf("ptcp->tcp_code & TCPF_SYN ");
		TcpEstabResets++;
		TcpCurrEstab--;
		tcpreset(pudp);
		return tcpabort(ptcb, TCPE_RESET);
	}
	
	if(tcpacked(ptcb, pudp) == -1) {
		printf("return tcpacked==-1\n");
		return 0;
	}
	
	tcpdata(ptcb, pudp);
	tcpswindow(ptcb, pudp);
	
	if (ptcb->tcb_flags & TCBF_RDONE)
		ptcb->tcb_state = TCPS_CLOSEWAIT;
		
	return 0;
}

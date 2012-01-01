#include "network.h"

/*------------------------------------------------------------------------
 *  tcpclosewait -  do CLOSE_WAIT state input processing
 *------------------------------------------------------------------------
 */
int
tcpclosewait(struct tcb *ptcb, struct udp *pudp)
{
	struct	tcp	*ptcp = (struct tcp *)pudp->data;

	printf("tcpclosewait\n");
	if(ptcp->tcp_code & TCPF_RST) {
		TcpEstabResets++;
		TcpCurrEstab--;
		return tcpabort(ptcb, TCPE_RESET);
	}
	
	if(ptcp->tcp_code & TCPF_SYN) {
		TcpEstabResets++;
		TcpCurrEstab--;
		tcpreset(pudp);
		return tcpabort(ptcb, TCPE_RESET);
	}
	tcpacked(ptcb, pudp);
	tcpswindow(ptcb, pudp);
	return 0;
}

/* tcprmss.c - tcprmss */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcprmss - set receive MSS option
 *------------------------------------------------------------------------
 */
int
tcprmss(struct tcb *ptcb, struct udp *pudp)
{
	struct	tcp	*ptcp = (struct tcp *)pudp->data;
	int		mss, hlen, olen, i;

	printf("tcprmss\n");
	hlen = TCP_HLEN(ptcp);
	olen = 2 + sizeof(short);
	pudp->data[hlen] = TPO_MSS;
	pudp->data[hlen+1] = olen;
	mss = ptcb->tcb_rmss;
	for(i = olen - 1; i > 1; i--) {
		pudp->data[hlen+i] = mss & 0377;
		mss >>= 8;
	}
	hlen += olen + 3;
	ptcp->tcp_offset = ((hlen<<2) & 0xf0) | (ptcp->tcp_offset & 0xf);

	return 0;
}

/* tcpsmss.c - tcpsmss */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpsmss - set sender MSS from option in incoming segment
 *------------------------------------------------------------------------
 */
int
tcpsmss(struct tcb *ptcb, struct tcp *ptcp, u_char *popt)
{
	unsigned	mss, len;

	printf("tcpsmss\n");
	len = *++popt;
	++popt;		/* skip length field */
	if ((ptcp->tcp_code & TCPF_SYN) == 0)
		return len;
		
	switch (len-2) {	/* subtract kind & len	*/
	case sizeof(char):
		mss = *popt;
		break;
	case sizeof(short):
		mss = ntohs(*(unsigned short *)popt);
		break;
	case sizeof(long):
		mss = ntohl(*(unsigned long *)popt);
		break;
	default:
		mss = ptcb->tcb_smss;
		break;
	}
	
	mss -= TCPMHLEN;	/* save just the data buffer size */
	if (ptcb->tcb_smss)
		ptcb->tcb_smss = min(mss, ptcb->tcb_smss);
	else ptcb->tcb_smss = mss;
	
	return len;
}

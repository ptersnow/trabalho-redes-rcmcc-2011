/* tcpopts.c - tcpopts */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpopts - handle TCP options for an inbound segment
 *------------------------------------------------------------------------
 */
int
tcpopts(struct tcb *ptcb, struct udp *pudp)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;
	u_char *popt, *popend;
	int len;
	
	if(TCP_HLEN(ptcp) == TCPMHLEN)
		return 0;
		
	popt = ptcp->tcp_data;
	popend = &(pudp->data[TCP_HLEN(ptcp)]);
	do {
		switch (*popt) {
		case TPO_NOOP:
			popt++;	/* fall through */
		case TPO_EOOL:
			break;
		case TPO_MSS:
			popt += tcpsmss(ptcb, ptcp, popt);
			break;
		default:
			popt++;	/* skip option code */
			if(*popt > 0 && *popt <= popend - popt - 1)
				popt += *popt - 1;
			else popt = popend;	/* bogus option length */
			
			break;
		}
	} while(*popt != TPO_EOOL && popt < popend);

	/* delete the options */
	len = pudp->length - UDPMHLEN - TCP_HLEN(ptcp);
	if(len)
		memcpy(ptcp->tcp_data, &(pudp->data[TCP_HLEN(ptcp)]), len); 
	
	pudp->length = UDPMHLEN + TCPMHLEN + len;
	ptcp->tcp_offset = TCPHOFFSET;
	return 0;
}

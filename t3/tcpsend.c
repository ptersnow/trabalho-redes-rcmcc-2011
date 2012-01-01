/* tcpsend.c - tcpsend */

#include "network.h"

static void
print_buffer(struct udp *pudp)
{
	struct tcp *ptcp;
	
	if(pudp) {
		ptcp = (struct tcp *) pudp->data;

		printf("lip=%s lport=%d length=%d ", inet_ntoa(pudp->src), ntohs(ptcp->tcp_sport), pudp->length);
  	printf("rip=%s rport=%d cksum=%d code=%d\n", 
  		inet_ntoa(pudp->dst), ntohs(ptcp->tcp_dport), ntohs(ptcp->tcp_cksum), ptcp->tcp_code);
	}
} 

/*------------------------------------------------------------------------
 *  tcpsend -  compute and send a TCP segment for the given TCB
 *------------------------------------------------------------------------
 */
int
tcpsend(int tcbnum, int rexmt)
{
	struct sockaddr_in sin;
	struct tcb *ptcb = &tcbtab[tcbnum];
	struct udp *pudp;
	struct tcp *ptcp;
	unsigned	i, off, tocopy, datalen;
	int		newdata;
	short up;
	u_char *pch;

	printf("tcpsend: ");
	pudp = (struct udp *) malloc(UDPMHLEN + 1);
	pudp->src = ptcb->tcb_lip;
	pudp->dst = ptcb->tcb_rip;
	datalen = tcpsndlen(ptcb, rexmt, &off);	/* get length & offset	*/
	
	ptcp = (struct tcp *)pudp->data;
	ptcp->tcp_sport = ptcb->tcb_lport;
	ptcp->tcp_dport = ptcb->tcb_rport;
	
	if(!rexmt)
		ptcp->tcp_seq = ptcb->tcb_snext;
	else ptcp->tcp_seq = ptcb->tcb_suna;
	
	ptcp->tcp_ack = ptcb->tcb_rnext;

	if((ptcb->tcb_flags & TCBF_SNDFIN) && (SEQCMP(ptcp->tcp_seq + datalen, ptcb->tcb_slast) == 0))
		ptcb->tcb_code |= TCPF_FIN;
		
	ptcp->tcp_code = ptcb->tcb_code;
	ptcp->tcp_offset = TCPHOFFSET;
	if((ptcb->tcb_flags & TCBF_FIRSTSEND) == 0)
		ptcp->tcp_code |= TCPF_ACK;
	
	if(ptcp->tcp_code & TCPF_SYN)
		tcprmss(ptcb, pudp);
		
	if(datalen > 0)
		ptcp->tcp_code |= TCPF_PSH;
		
	ptcp->tcp_window = tcprwindow(ptcb);
	if (ptcb->tcb_flags & TCBF_SUPOK) {
		up = ptcb->tcb_supseq - ptcp->tcp_seq;

		if (up >= 0) {
			ptcp->tcp_urgptr = up;
			ptcp->tcp_code |= TCPF_URG;
		}
		else ptcp->tcp_urgptr = 0;
	}
	else ptcp->tcp_urgptr = 0;
	
	pch = (u_char *) &ptcp->tcp_data;
	i = (ptcb->tcb_sbstart + off) % ptcb->tcb_sbsize;
	for(tocopy = datalen; tocopy > 0; --tocopy) {
		*pch++ = ptcb->tcb_sndbuf[i];
		if(++i >= ptcb->tcb_sbsize)
			i = 0;
	}
	
	ptcb->tcb_flags &= ~TCBF_NEEDOUT;	/* we're doing it	*/
	if(rexmt) {
		newdata = ptcb->tcb_suna + datalen - ptcb->tcb_snext;
		if(newdata < 0)
			newdata = 0;
		TcpRetransSegs++;
	}
	else {
		newdata = datalen;
		if(ptcb->tcb_code & TCPF_SYN)
			newdata++; /* SYN is part of the sequence	*/
		if(ptcb->tcb_code & TCPF_FIN)
			newdata++; /* FIN is part of the sequence	*/
	}
	
	ptcb->tcb_snext += newdata;
	if(newdata >= 0)
		TcpOutSegs++;
	if(ptcb->tcb_state == TCPS_TIMEWAIT)	/* final ACK		*/
		tcpwait(ptcb);
	datalen += TCP_HLEN(ptcp);
	pudp->length = datalen + UDPMHLEN;
	
	tcph2net(ptcp);
	ptcp->tcp_cksum = 0;
	ptcp->tcp_cksum = tcpcksum(pudp, datalen);
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = ptcb->tcb_rip;
	sin.sin_port = uport;
	
	printf("tcpsend: ");
	print_buffer(pudp);
	newdata = sendto(usocket, pudp, pudp->length, 0, (struct sockaddr*)&sin, sizeof(sin));
	
	return newdata;
}

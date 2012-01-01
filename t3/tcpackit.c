/* tcpackit.c - tcpackit */

#include "network.h"

static void
print_buffer(struct udp *pudp)
{
	struct tcp *ptcp;
	
	if(pudp) {
		ptcp = (struct tcp *) pudp->data;

		printf("lip=%s lport=%d length=%d ", inet_ntoa(pudp->src), ntohs(ptcp->tcp_sport), pudp->length);
  	printf("rip=%s rport=%d cksum=%d code=%d\n", inet_ntoa(pudp->dst), ntohs(ptcp->tcp_dport), ntohs(ptcp->tcp_cksum), ptcp->tcp_code);
	}
} 

/*------------------------------------------------------------------------
 *  tcpackit -  generate an ACK for a received TCP packet
 *------------------------------------------------------------------------
 */
int
tcpackit(struct tcb *ptcb, struct udp *pudpin)
{
	int rv;
	struct sockaddr_in sin;
	struct udp *pudp;
	struct tcp *ptcpin = (struct tcp *)pudpin->data, *ptcpout;

	printf("tcpackit: ");

	if(ptcpin->tcp_code & TCPF_RST) {
		printf("TCPF_RST\n");
		return 0;
	}
	if((pudpin->length <= UDPMHLEN + TCP_HLEN(ptcpin)) && !(ptcpin->tcp_code & (TCPF_SYN|TCPF_FIN))) {
		printf("duplicate ACK\n");
		return 0;	/* duplicate ACK */
	}

	pudp = (struct udp *) malloc(pudpin->length + 1);
	pudp->src = ptcb->tcb_rip;
	pudp->dst = ptcb->tcb_lip;
	pudp->length = pudpin->length;
	ptcpout = (struct tcp *)pudp->data;
	ptcpout->tcp_sport = ptcpin->tcp_dport;
	ptcpout->tcp_dport = ptcpin->tcp_sport;
	ptcpout->tcp_seq = ptcb->tcb_snext;
	ptcpout->tcp_ack = ptcb->tcb_rnext;
	ptcpout->tcp_code = TCPF_ACK;
	ptcpout->tcp_offset = TCPHOFFSET;
	ptcpout->tcp_window = tcprwindow(ptcb);
	ptcpout->tcp_urgptr = 0;
	ptcpout->tcp_cksum = 0;
	
	tcph2net(ptcpout);
	ptcpout->tcp_cksum = tcpcksum(pudp, TCPMHLEN);
	
	TcpOutSegs++;
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = pudp->dst;
	sin.sin_port = uport;
	
	printf("tcpackit: ");
	print_buffer(pudp);
	rv = sendto(usocket, pudp, pudp->length, 0, (struct sockaddr*)&sin, sizeof(sin));
	
	return rv;
}

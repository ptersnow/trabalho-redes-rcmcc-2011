/* tcpreset.c - tcpreset */

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
 *  tcpreset -  generate a reset in response to a bad packet
 *------------------------------------------------------------------------
 */
int
tcpreset(struct udp *pudp)
{
	struct sockaddr_in sin;
	struct udp *pudpout;
	struct tcp *ptcpin = (struct tcp *)pudp->data, *ptcpout;
	int		datalen;

	printf("tcpreset: ");
	if(ptcpin->tcp_code & TCPF_RST) {
		printf("return 0 \n");
		return 0;		/* no RESETs on RESETs */
	}
		
	pudpout = (struct udp *) malloc(pudp->length + 1);
	pudpout->src = pudp->dst;
	pudpout->dst = pudp->src;

	ptcpout = (struct tcp *)pudpout->data;
	ptcpout->tcp_sport = ptcpin->tcp_dport;
	ptcpout->tcp_dport = ptcpin->tcp_sport;
	
	if(ptcpin->tcp_code & TCPF_ACK) {
		ptcpout->tcp_seq = ptcpin->tcp_ack;
		ptcpout->tcp_code = TCPF_RST;
	}
	else {
		ptcpout->tcp_seq = 0;
		ptcpout->tcp_code = (TCPF_RST|TCPF_ACK);
	}
	
	datalen = pudp->length - UDPMHLEN - TCP_HLEN(ptcpin);
	if(ptcpin->tcp_code & TCPF_SYN)
		datalen++;
	if(ptcpin->tcp_code & TCPF_FIN)
		datalen++;
		
	ptcpout->tcp_ack = ptcpin->tcp_seq + datalen;
	ptcpout->tcp_offset = TCPHOFFSET;
	ptcpout->tcp_window = ptcpout->tcp_urgptr = 0;
	
	tcph2net(ptcpout);
	ptcpout->tcp_cksum = 0;
	ptcpout->tcp_cksum = tcpcksum(pudpout, TCPMHLEN);
	
	TcpOutSegs++;
	TcpOutRsts++;
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = pudp->dst;
	sin.sin_port = uport;
	
	pudpout->length = datalen + UDPMHLEN;
	print_buffer(pudpout);
	datalen = sendto(usocket, pudpout, pudpout->length, 0, (struct sockaddr*)&sin, sizeof(sin));
	
	return datalen;
}

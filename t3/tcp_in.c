/* tcp_in.c - tcp_in */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcp_in - deliver an inbound TCP packet to the TCP process
 *------------------------------------------------------------------------
 */

static void
print_buf(char *buffer)
{
	struct udp *pudp = (struct udp *) buffer;
	struct tcp *ptcp = (pudp ? (struct tcp *) pudp->data : NULL);
	
	printf("lip=%s lport=%d length=%d ", inet_ntoa(pudp->src), ntohs(ptcp->tcp_sport), pudp->length);
	printf("rip=%s rport=%d cksum=%d code=%d\n", 
  		inet_ntoa(pudp->dst), ntohs(ptcp->tcp_dport), ntohs(ptcp->tcp_cksum), ptcp->tcp_code);
} 

void *
tcp_in(void *param)
{
	int rv;
	struct mq_attr attr;
	char buf[TCPPKTLEN + 1];
	socklen_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr *sin = (struct sockaddr *) &param;
  
  printf("thread tcp_in criada....\n");
	
	while(1) {
		memset(&buf, 0, sizeof(buf));
		rv = recvfrom(usocket, buf, TCPPKTLEN, 0, sin, &addrlen);
		
		printf("tcp_in -> recvfrom=%d ", rv);
		print_buf(buf);
		if(uport <= 0)
			uport = ((struct sockaddr_in *) sin)->sin_port;
		
		mq_getattr(tcps_iport, &attr);
		if(attr.mq_curmsgs < TCPQLEN)
			if(mq_send(tcps_iport, buf, TCPPKTLEN, 0) < 0)
				perror("tcp_in: mq_send");
	}
	
	pthread_exit(0);
}

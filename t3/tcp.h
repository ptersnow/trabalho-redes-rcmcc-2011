/* tcp.h - TCP_HLEN, SEQCMP */

#include <arpa/inet.h>

/*
 * SEQCMP - sequence space comparator
 *	This handles sequence space wrap-around. Overlow/Underflow makes
 * the result below correct ( -, 0, + ) for any a, b in the sequence
 * space. Results:	result	implies
 *			  - 	 a < b
 *			  0 	 a = b
 *			  + 	 a > b
 */
#define	SEQCMP(a, b)	((a) - (b))
#define TCPPKTLEN 4288

/* tcp packet format */

struct tcp {
	unsigned short	tcp_sport;	/* source port			*/
	unsigned short	tcp_dport;	/* destination port		*/
		 long	tcp_seq;	/* sequence			*/
		 long	tcp_ack;	/* acknowledged sequence	*/
	unsigned char	tcp_offset;
	unsigned char	tcp_code;	/* control flags		*/
	unsigned short	tcp_window;	/* window advertisement		*/
	unsigned short	tcp_cksum;	/* check sum			*/
	unsigned short	tcp_urgptr;	/* urgent pointer		*/
	unsigned char	tcp_data[1];
};

struct udp {
	struct in_addr src;
	struct in_addr dst;
	unsigned short length;
	unsigned char	data[1];
};

extern void *tcp_in(void *);

/* TCP Control Bits */

#define	TCPF_URG	0x20	/* urgent pointer is valid		*/
#define	TCPF_ACK	0x10	/* acknowledgement field is valid	*/
#define	TCPF_PSH	0x08	/* this segment requests a push		*/
#define	TCPF_RST	0x04	/* reset the connection			*/
#define	TCPF_SYN	0x02	/* synchronize sequence numbers		*/
#define	TCPF_FIN	0x01	/* sender has reached end of its stream	*/

#define UDPMHLEN		80
#define	TCPMHLEN	  20	/* minimum TCP header length		*/
#define	TCPHOFFSET	0x50	/* tcp_offset value for TCPMHLEN	*/
#define	TCP_HLEN(ptcp)		(((ptcp)->tcp_offset & 0xf0)>>2)

/* TCP Options */

#define	TPO_EOOL	0	/* end Of Option List			*/
#define	TPO_NOOP	1	/* no Operation				*/
#define	TPO_MSS		2	/* maximum Segment Size			*/

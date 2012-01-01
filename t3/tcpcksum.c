/* tcpcksum.c - tcpcksum */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpcksum -  compute a TCP pseudo-header checksum
 *------------------------------------------------------------------------
 */
unsigned short
tcpcksum(struct udp *pudp, unsigned len)
{
	struct tcp *ptcp = (struct tcp *)pudp->data;
	unsigned short *sptr;
	unsigned long tcksum;
	unsigned i;

	tcksum = 0;
	sptr = (unsigned short *) &pudp->src;
	/* 2*IP_ALEN octets = IP_ALEN shorts... */
	/* they are in net order.		*/

	for(i = 0; i < 4; ++i)
		tcksum += *sptr++;
	sptr = (unsigned short *)ptcp;
	tcksum += htons(6 + len);
	
	if (len % 2) {
		((char *)ptcp)[len] = 0;	/* pad */
		len += 1;	/* for the following division */
	}
	len >>= 1;	/* convert to length in shorts */

	for(i = 0; i < len; ++i)
		tcksum += *sptr++;
	tcksum = (tcksum >> 16) + (tcksum & 0xffff);
	tcksum += (tcksum >> 16);
	
	return (short)(~tcksum & 0xffff);
}

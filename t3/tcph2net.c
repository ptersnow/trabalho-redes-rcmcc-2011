/* tcph2net.c - tcph2net */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcph2net -  convert TCP header fields from host to net byte order
 *------------------------------------------------------------------------
 */
struct tcp *
tcph2net(struct tcp *ptcp)
{
	/* NOTE: does not include TCP options */

	ptcp->tcp_sport = htons(ptcp->tcp_sport);
	ptcp->tcp_dport = htons(ptcp->tcp_dport);
	ptcp->tcp_seq = htonl(ptcp->tcp_seq);
	ptcp->tcp_ack = htonl(ptcp->tcp_ack);
	ptcp->tcp_window = htons(ptcp->tcp_window);
	ptcp->tcp_urgptr = htons(ptcp->tcp_urgptr);
	return ptcp;
}

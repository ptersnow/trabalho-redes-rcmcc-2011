/* tcpwait.c - tcpwait */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpwait - (re)schedule a DELETE event for 2MSL from now
 *------------------------------------------------------------------------
 */
int
tcpwait(struct tcb *ptcb)
{
	int	tcbnum = ptcb - &tcbtab[0];

	printf("tcpwait\n");
	tcpkilltimers(ptcb);
	tmset(tcps_oport, TCPQLEN, (int) MKEVENT(DELETE, tcbnum), TCP_TWOMSL);
	return 0;
}

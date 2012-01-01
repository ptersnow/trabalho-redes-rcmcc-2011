#include "network.h"

/*------------------------------------------------------------------------
 *  tcpclosed -  do CLOSED state processing
 *------------------------------------------------------------------------
 */
int
tcpclosed(struct tcb *ptcb, struct udp *pudp)
{
	printf("tcpclosed\n");
	tcpreset(pudp);
	return	-1;
}

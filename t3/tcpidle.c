/* tcpidle.c - tcpidle */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpidle - handle events while a connection is idle
 *------------------------------------------------------------------------
 */
int
tcpidle(int tcbnum, int event)
{
	printf("tcpidle: ");
	if(event == SEND)
		tcpxmit(tcbnum, event);
	printf("return\n");
	return 0;
}

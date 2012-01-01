/* tcpnxtp.c - tcpnxtp */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpnxtp  -  return the next available TCP local "port" number
 *------------------------------------------------------------------------
 */
u_short
tcpnxtp()
{
	static u_short lastport = 1026;
	u_short	start;
	int i;

	sem_wait(&tcps_tmutex);
	for(start = lastport++; start != lastport; ++lastport) {
		for(i = 1; i <= Ntcp; ++i)
			if((tcbtab[i].tcb_state != TCPS_FREE) && (tcbtab[i].tcb_lport == lastport))
				break;
				
		if(i == Ntcp + 1)
			break;
	}
	
	if(lastport == start)
		printf("out of TCP ports\n");
	sem_post(&tcps_tmutex);
	
	return lastport;
}

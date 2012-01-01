/* tcpdemux.c - tcpdemux */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpdemux -  do TCP port demultiplexing
 *------------------------------------------------------------------------
 */
struct tcb *
tcpdemux(struct udp *pudp)
{
	struct	tcp	*ptcp = (struct tcp *)pudp->data;
	int		tcbn, lstcbn;

	sem_wait(&tcps_tmutex);
	for(tcbn = 1, lstcbn = -1; tcbn <= Ntcp; ++tcbn) {
		if(tcbtab[tcbn].tcb_state == TCPS_FREE)
			continue;
				
		if((ptcp->tcp_dport == tcbtab[tcbn].tcb_lport) &&
		   (ptcp->tcp_sport == tcbtab[tcbn].tcb_rport) &&
		   (pudp->src.s_addr == tcbtab[tcbn].tcb_rip.s_addr) &&
		   (pudp->dst.s_addr == tcbtab[tcbn].tcb_lip.s_addr))
			break;

		if((tcbtab[tcbn].tcb_state == TCPS_LISTEN) && (ptcp->tcp_dport == tcbtab[tcbn].tcb_lport))
			lstcbn = tcbn;
	}
	
	if(tcbn >= Ntcp + 1) {
		if(ptcp->tcp_code & TCPF_SYN)
			tcbn = lstcbn;
		else tcbn = -1;
	}
	
	sem_post(&tcps_tmutex);
	if(tcbn < 0)
		return 0;
		
	sem_wait(&tcbtab[tcbn].tcb_mutex);
	if(tcbtab[tcbn].tcb_state == TCPS_FREE)
		return 0;			/* OOPS! Lost it... */
	
	printf("tcpdemux: return %d\n", tcbn);
	return &tcbtab[tcbn];
}

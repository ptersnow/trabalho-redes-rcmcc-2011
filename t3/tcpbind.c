/* tcpbind.c - tcpbind */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpbind - bind a TCP pseudo device to its addresses and port
 *------------------------------------------------------------------------
 */
int
tcpbind(struct tcb *ptcb, struct in_addr fip, u_short fport)
{	
  inet_pton(AF_INET, "127.0.0.1", &(ptcb->tcb_lip));
  ptcb->tcb_rip = fip;
  ptcb->tcb_lport = tcpnxtp();
  ptcb->tcb_rport = ntohs(fport);
  
  printf("tcpbind: lip=%s lport=%d ", inet_ntoa(ptcb->tcb_lip), ptcb->tcb_lport);
  printf("rip=%s rport=%d \n", inet_ntoa(ptcb->tcb_rip), ptcb->tcb_rport);
  
  return (ptcb->tcb_lport > 0);
}

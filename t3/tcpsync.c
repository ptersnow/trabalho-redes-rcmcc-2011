/* tcpsync.c - tcpsync */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcpsync - inicializa TCB para uma nova requisicao de conexao
 *------------------------------------------------------------------------
 */
int
tcpsync(struct tcb *ptcb)
{
	ptcb->tcb_state = TCPS_CLOSED;
	ptcb->tcb_type = TCPT_CONNECTION;

	ptcb->tcb_iss = ptcb->tcb_suna = ptcb->tcb_snext = tcpiss();
	ptcb->tcb_lwack = ptcb->tcb_iss;

	ptcb->tcb_sndbuf = (u_char *)malloc(TCPSBS);
	ptcb->tcb_sbsize = TCPSBS;
	ptcb->tcb_sbstart = ptcb->tcb_sbcount = 0;
  sem_init(&ptcb->tcb_ssema, 0, 1);

	ptcb->tcb_rcvbuf = (u_char *)malloc(TCPRBS);
	ptcb->tcb_rbsize = TCPRBS;
	ptcb->tcb_rbstart = ptcb->tcb_rbcount = 0;
	ptcb->tcb_rsegq = -1;

  sem_init(&ptcb->tcb_rsema, 0, 0);
  sem_init(&ptcb->tcb_ocsem, 0, 0);

	/* timer stuff */

	ptcb->tcb_srt = 0;		/* in sec/100	*/
	ptcb->tcb_rtde = 0;		/* in sec/100	*/
	ptcb->tcb_rexmt = 50;		/* in sec/100	*/
	ptcb->tcb_rexmtcount = 0;
	ptcb->tcb_keep = 12000;		/* in sec/100	*/

	ptcb->tcb_code = TCPF_SYN;
	ptcb->tcb_flags = 0;
	
	return 0;
}

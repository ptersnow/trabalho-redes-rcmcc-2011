/* tcprwindow.c - tcprwindow */

#include "network.h"

/*------------------------------------------------------------------------
 *  tcprwindow - do receive window processing for a TCB
 *------------------------------------------------------------------------
 */
int
tcprwindow(struct tcb *ptcb)
{
	int	window;

	printf("tcprwindow: ");
	window = ptcb->tcb_rbsize - ptcb->tcb_rbcount;
	if(ptcb->tcb_state < TCPS_ESTABLISHED) {
		printf("if window=%d\n", window);
		return window;
	}
	/*
	 *	Receiver-Side Silly Window Syndrome Avoidance:
	 *  Never shrink an already-advertised window, but wait for at
	 *  least 1/4 receiver buffer and 1 max-sized segment before
	 *  opening a zero window.
	 */
	if((window * 4 < (int)ptcb->tcb_rbsize) || (window < (int)ptcb->tcb_rmss))
		window = 0;
		
	window = max(window, ptcb->tcb_cwin - ptcb->tcb_rnext);
	ptcb->tcb_cwin = ptcb->tcb_rnext + window;
	
	printf("return: window=%d\n", window);
	return window;
}

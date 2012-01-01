/* tcpiss.c - tcpiss */

#include <time.h>

#define	TCPINCR	904

/*------------------------------------------------------------------------
*  tcpiss -  set the ISS for a new connection
*-------------------------------------------------------------------------
*/
int
tcpiss(void)
{
	static int	seq = 0;

	if (seq == 0)
		seq = time(0);
	seq += TCPINCR;
	return seq;
}

/* tcptimer.h */

#ifndef _TCPTIMER_H_
#define _TCPTIMER_H_

#define TIMERGRAN 1
/* A timer delta list entry */

struct	tqent {
	int	tq_timeleft;		/* time to expire (1/100 secs)	*/
	long	tq_time;		/* time this entry was queued	*/
	mqd_t	tq_port;		/* port to send the event	*/
	int	tq_portlen;		/* length of "tq_port"		*/
	int tq_msg;		/* data to send when expired	*/
	struct	tqent	*tq_next;	/* next in the list		*/
};
/* timer process declarations and definitions */

extern	void *tcptimer();

extern	sem_t	tqmutex;
extern	struct	tqent	*tqhead;

int tmclear(int, int), tmleft(int, int);
int tmset(int, int, int, int);
int tcpkilltimers(struct tcb *);

#endif /* _TCPTIMER_H_ */

/* tcptimer.c - tcptimer */

#include "network.h"

long ctr100;
sem_t	tqmutex;
struct tqent *tqhead;

/*------------------------------------------------------------------------
 *  tmclear -  clear the indicated timer
 *------------------------------------------------------------------------
 */
int
tmclear(mqd_t port, int msg)
{
	struct tqent *prev, *ptq;
	int	timespent;

	printf("tmclear: ");
	sem_wait(&tqmutex);
	prev = 0;
	for(ptq = tqhead; ptq != NULL; ptq = ptq->tq_next) {
		if(ptq->tq_port == port && ptq->tq_msg == msg) {
			timespent = ctr100 - ptq->tq_time;
			
			if(prev)
				prev->tq_next = ptq->tq_next;
			else tqhead = ptq->tq_next;
			
			if(ptq->tq_next)
				ptq->tq_next->tq_timeleft += ptq->tq_timeleft;
			
			printf("timespent=%d\n", timespent);
			sem_post(&tqmutex);
			if(ptq)
				free(ptq);
			return timespent;
		}
		prev = ptq;
	}
	
	sem_post(&tqmutex);
	printf("return -1\n");
	return -1;
}


/*------------------------------------------------------------------------
 *  tcpkilltimers -  kill all outstanding timers for a TCB
 *------------------------------------------------------------------------
 */
int
tcpkilltimers(struct tcb *ptcb)
{
	int	tcbnum = ptcb - &tcbtab[0];

	/* clear all possible pending timers */

	tmclear(tcps_oport, (int) MKEVENT(SEND, tcbnum));
	tmclear(tcps_oport, (int) MKEVENT(RETRANSMIT, tcbnum));
	tmclear(tcps_oport, (int) MKEVENT(PERSIST, tcbnum));
	return 0;
}


/*------------------------------------------------------------------------
 *  tmleft -  how much time left for this timer?
 *------------------------------------------------------------------------
 */
int
tmleft(int port, int msg)
{
	struct tqent *tq;
	int timeleft = 0;

	if (tqhead == NULL)
		return 0;
	
	printf("tmleft: ");
	sem_wait(&tqmutex);
	for (tq = tqhead; tq != NULL; tq = tq->tq_next) {
		timeleft += tq->tq_timeleft;
		if (tq->tq_port == port && tq->tq_msg == msg) {
			sem_post(&tqmutex);
			printf("timeleft=%d\n", timeleft);
			return timeleft;
		}
	}
	
	sem_post(&tqmutex);
	printf("return 0\n");
	return 0;
}


/*------------------------------------------------------------------------
 *  tmset -  set a fast timer
 *------------------------------------------------------------------------
 */
int
tmset(mqd_t port, int portlen, int msg, int time)
{
	struct	tqent	*ptq, *newtq, *tq;

	printf("tmset: ");
	newtq = (struct tqent *) malloc(sizeof(struct tqent));
	newtq->tq_timeleft = time;
	newtq->tq_time = ctr100;
	newtq->tq_port = port;
	newtq->tq_portlen = portlen;
	newtq->tq_msg = msg;
	newtq->tq_next = NULL;

	/* clear duplicates */
	printf("tmset: ");
	tmclear(port, msg);

	sem_wait(&tqmutex);
	if(tqhead == NULL) {
		tqhead = newtq;
		sem_post(&tqmutex);
		printf("tqhead == NULL\n");
		return 0;
	}
	/* search the list for our spot */

	for(ptq = 0, tq = tqhead; tq; tq = tq->tq_next) {
		if(newtq->tq_timeleft < tq->tq_timeleft)
			break;
		newtq->tq_timeleft -= tq->tq_timeleft;
		ptq = tq;
	}

	newtq->tq_next = tq;
	if(ptq)
		ptq->tq_next = newtq;
	else tqhead = newtq;
	
	if(tq)
		tq->tq_timeleft -= newtq->tq_timeleft;
	
	sem_post(&tqmutex);
	printf("return 0\n");
	return 0;
}


/*------------------------------------------------------------------------
 *  tcpkick -  make sure we send a packet soon
 *------------------------------------------------------------------------
 */
int
tcpkick(struct tcb *ptcb)
{
	int tv;
	char buffer[10];
	struct mq_attr attr;
	int	tcbnum = ptcb - &tcbtab[0];	/* for MKEVENT() */

	tv = (int) MKEVENT(SEND, tcbnum);
	
	printf("tcpkick: ");
	if(ptcb->tcb_flags & TCBF_DELACK && !tmleft(tcps_oport, tv)) {
		printf("tcpkick: ");
		tmset(tcps_oport, TCPQLEN, tv, TCP_ACKDELAY);
	}
	else {
		mq_getattr(tcps_oport, &attr);
		if(attr.mq_curmsgs < TCPQLEN) {
			sprintf(buffer, "%d", tv);
			if(mq_send(tcps_oport, buffer, strlen(buffer), 0) < 0)
				perror("tcpkick: mq_send");
		}
	}
	
	return 0;
}

/*------------------------------------------------------------------------
 *  tcptimer -  TCP timer process
 *------------------------------------------------------------------------
 */ 
void *
tcptimer(void)
{
	char buffer[10];
	struct mq_attr attr;
	long now, lastrun;		/* times from system clock	*/
	long delta;			/* time since last iteration	*/
	struct tqent *tq;		/* temporary delta list ptr	*/
	ctr100 = time(NULL);
	
	lastrun = ctr100;  /* initialize to "now"		*/
	sem_init(&tqmutex, 0, 1);		/* mutual exclusion semaphore	*/

	printf("thread tcptimer criada, ctr100==%ld....\n", ctr100);
	while (1) {
		usleep(TIMERGRAN * 100000);	/* real-time delay		*/
		
		if(!tqhead)
			continue;
		
		sem_wait(&tqmutex);
		now = ctr100 = time(NULL);
		delta = (now - lastrun) * 10;	/* compute elapsed time		*/

		if(delta < 0 || delta > TIMERGRAN * 100)
			delta = TIMERGRAN * 10;	/* estimate the delay	*/
		lastrun = now;
		
		while(tqhead  &&  (tqhead->tq_timeleft - delta <= 0)) {
			delta -= tqhead->tq_timeleft;
			mq_getattr(tqhead->tq_port, &attr);
			if(attr.mq_curmsgs <= tqhead->tq_portlen) {
				sprintf(buffer, "%d", tqhead->tq_msg);
				printf("manda %s para %d\n", buffer, tqhead->tq_port);
				if(mq_send(tqhead->tq_port, buffer, strlen(buffer), 0) < 0)
					perror("tcptimer: mq_send");
			}
				
			tq = tqhead;
			tqhead = tqhead->tq_next;
			if(tq)
				free(tq);
		}
		
		if(tqhead)
			tqhead->tq_timeleft -=delta;
			
		sem_post(&tqmutex);
	}
	
	pthread_exit(0);
}

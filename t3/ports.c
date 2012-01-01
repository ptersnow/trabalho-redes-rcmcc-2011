#ifndef __PORTS_H
#include "ports.h"
#endif

struct ptnode *ptfree = NULL;		/* list of free queue nodes	*/
struct pt ports[NPORTS];
int	ptnextp;

/*------------------------------------------------------------------------
*  pinit  --  initialize all ports
*-------------------------------------------------------------------------
*/
void
pinit(int maxmsgs)
{
	int	i;
	struct	ptnode	*next, *prev;

	if(!(ptfree = (struct ptnode *) malloc(maxmsgs * sizeof(struct ptnode)))) {
		perror("pinit");
		_exit(1);
	}
	
	for(i = 0; i < NPORTS; i++)
		ports[i].ptstate = PTFREE;
	ptnextp = NPORTS - 1;

	/* link up free list of message pointer nodes */
	for(prev = next = ptfree; --maxmsgs > 0; prev = next)
		prev->ptnext = ++next;
	prev->ptnext = NULL;
}

/*------------------------------------------------------------------------
*  pcreate  --  create a port that allows "count" outstanding messages
*-------------------------------------------------------------------------
*/
int
pcreate(unsigned int count)
{
	int	i, p;
	struct pt *ptptr;

	if(!ptfree)
		pinit(MAXMSGS);

	for(i = 0; i < NPORTS; i++) {
		if((p = ptnextp--) <= 0)
			ptnextp = NPORTS - 1;
		if((ptptr = &ports[p])->ptstate == PTFREE) {
			ptptr->ptstate = PTALLOC;
			sem_init(&ptptr->ptssem, 0, count);
			sem_init(&ptptr->ptrsem, 0, 0);
			ptptr->pthead = ptptr->pttail = NULL;
			ptptr->ptseq++;
			ptptr->ptmaxcnt = count;
			return(p);
		}
	}
	
	return -1;
}

/*------------------------------------------------------------------------
*  pdelete  --  delete a port, freeing waiting processes and messages
*-------------------------------------------------------------------------
*/
int
pdelete(int portid)
{
	struct	pt *ptptr;

	if(isbadport(portid) ||
	     (ptptr= &ports[portid])->ptstate != PTALLOC ) {
		return -1;
	}
	_ptclear(ptptr, PTFREE);
	
	return 0;
}

/*------------------------------------------------------------------------
*  preset  --  reset a port, freeing waiting processes and messages
*-------------------------------------------------------------------------
*/
int
preset(int portid)
{
	struct	pt *ptptr;

	if(isbadport(portid) ||
	     (ptptr = &ports[portid])->ptstate != PTALLOC ) {
		return -1;
	}
	_ptclear(ptptr, PTALLOC);
	return 0;
}

/*------------------------------------------------------------------------
*  _ptclear  --  used by pdelete and preset to clear a port
*-------------------------------------------------------------------------
*/
void
_ptclear(struct pt *ppt, int newstate)
{
	struct	ptnode	*p;

	/* put port in limbo until done freeing processes */
	ppt->ptstate = PTLIMBO;
	ppt->ptseq++;
	if(!(p = ppt->pthead)) {
		for(; p != NULL; p = p->ptnext)
			p->ptmsg = 0;
		(ppt->pttail)->ptnext = ptfree;
		ptfree = ppt->pthead;
	}
	if(newstate == PTALLOC) {
		ppt->pttail = ppt->pthead = NULL;
		sem_destroy(&ppt->ptssem);
		sem_destroy(&ppt->ptrsem);
		sem_init(&ppt->ptssem, ppt->ptmaxcnt, 0);
		sem_init(&ppt->ptrsem, 0, 0);
	} else {
		sem_destroy(&ppt->ptssem);
		sem_destroy(&ppt->ptrsem);
	}
	ppt->ptstate = newstate;
}

/*------------------------------------------------------------------------
*  pcount  --  return the count of current messages in a port
*-------------------------------------------------------------------------
*/
int
pcount(int	portid)
{
	int	scnt;
	int	count;
	struct pt *ptptr;

	if(isbadport(portid) ||
		(ptptr = &ports[portid])->ptstate != PTALLOC ) {
			return -1;
	}
	sem_getvalue(&ptptr->ptrsem, &count);
	sem_getvalue(&ptptr->ptssem, &scnt);
	if(scnt < 0 )
		count -= scnt;			/* add number waiting	*/
	return(count);
}

/*------------------------------------------------------------------------
 *  psend  --  send a message to a port by enqueuing it
 *------------------------------------------------------------------------
 */
int
psend(int portid, char *msg)
{
	int	seq;
	struct pt *ptptr;
	struct ptnode *freenode;
	
	if(isbadport(portid) ||
    (ptptr = &ports[portid])->ptstate != PTALLOC) {
		return -1;
	}

	printf("FUCK\n");
	printf("%s\n", msg);
	/* wait for space and verify port is still allocated */
	seq = ptptr->ptseq;
	sem_wait(&ptptr->ptssem);

	if(ptptr->ptstate != PTALLOC || ptptr->ptseq != seq)
		return -1;
	
	if(!ptfree) {
		perror("psend");
		_exit(1);
	}
	
	freenode = ptfree;
	ptfree  = freenode->ptnext;
	freenode->ptnext = NULL;
	freenode->ptmsg  = msg;
	
	if(!ptptr->pttail)	/* empty queue */
		ptptr->pttail = ptptr->pthead = freenode;
	else {
		(ptptr->pttail)->ptnext = freenode;
		ptptr->pttail = freenode;
	}
	
	sem_post(&ptptr->ptrsem);
	return 0;
}

/*------------------------------------------------------------------------
*  preceive  --  receive a message from a port, blocking if port empty
*-------------------------------------------------------------------------
*/
char *
preceive(int portid)
{
	int	seq;
	char *msg;
	struct	pt	*ptptr;
	struct	ptnode	*nxtnode;

	if(isbadport(portid) || ((ptptr = &ports[portid])->ptstate != PTALLOC))
		return NULL;

	/* wait for message and verify that the port is still allocated */
	seq = ptptr->ptseq;
	sem_wait(&ptptr->ptrsem);
	if((ptptr->ptstate != PTALLOC) || (ptptr->ptseq != seq))
		return NULL;

	/* dequeue first message that is waiting in the port */
	nxtnode = ptptr->pthead;
	msg = nxtnode->ptmsg;
	if(ptptr->pthead == ptptr->pttail)	/* delete last item	*/
		ptptr->pthead = ptptr->pttail = NULL;
	else ptptr->pthead = nxtnode->ptnext;
	
	nxtnode->ptnext = ptfree;		/* return to free list	*/
	ptfree = nxtnode;
	sem_post(&ptptr->ptssem);
	
	return msg;
}


#ifndef __PORTS_H
#define __PORTS_H

#include "network.h"

#define NPORTS 20

#define	MAXMSGS		50		/* maximum messages on all ports*/
#define	PTFREE		1		/* port is Free			*/
#define	PTLIMBO		2		/* port is being deleted/reset	*/
#define	PTALLOC		3		/* port is allocated		*/
#define	PTEMPTY		-1		/* initial semaphore entries	*/

#define	isbadport(portid)	( (portid)<0 || (portid)>=NPORTS )

struct	ptnode	{			/* node on list of message ptrs	*/
	char *ptmsg;			/* a one-word message		*/
	struct	ptnode	*ptnext;	/* address of next node on list	*/
};

struct	pt	{			/* entry in the port table	*/
	int	ptstate;		/* port state (FREE/LIMBO/ALLOC)*/
	sem_t	ptssem;			/* sender semaphore		*/
	sem_t	ptrsem;			/* receiver semaphore		*/
	int	ptmaxcnt;		/* max messages to be queued	*/
	int	ptseq;			/* sequence changed at creation	*/
	struct	ptnode	*pthead;	/* list of message pointers	*/
	struct	ptnode	*pttail;	/* tail of message list		*/
};

extern	struct	ptnode	*ptfree;	/* list of free nodes		*/
extern	struct	pt	ports[];	/* port table			*/
extern	int	ptnextp;		/* next port to examine when	*/
					/*   looking for a free one	*/

void pinit(int maxmsgs);
int preset(int portid);
int pcount(int portid);
int pdelete(int portid);
int recvtim(int maxwait);
char *preceive(int portid);
int pcreate(unsigned int count);
int psend(int portid, char *msg);
void _ptclear(struct pt *ppt, int newstate);

#endif

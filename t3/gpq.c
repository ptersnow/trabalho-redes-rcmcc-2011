/* gpq.c - newq, enq, headq, deq, seeq, freeq, initq */

/* generic priority queue processing functions */

#include "network.h"

static int qinit = 0;
static void initq();

static struct qinfo Q[MAXNQ];

/*------------------------------------------------------------------------
 * enq  --	insert an item at the tail of a list, based on priority
 *	Returns the number of slots available; -1, if full
 *------------------------------------------------------------------------
 */
int
enq(int q, void *elt, int key)
{
	struct qinfo *qp;
	int i, j, left;

	printf("Chamo isso???\n\n");
	if(q < 0 || q >= MAXNQ)
		return -1;
		
	if(!Q[q].q_valid || Q[q].q_count >= Q[q].q_max)
		return -1;

	qp = &Q[q];

	sem_wait(&qp->q_mutex);

	if(qp->q_count < 0)
		qp->q_count = 0;
	i = qp->q_count-1;

	while(i >= 0 && key > qp->q_key[i])
		--i;

	for(j = qp->q_count-1; j > i; --j) {
		qp->q_key[j+1] = qp->q_key[j];
		qp->q_elt[j+1] = qp->q_elt[j];
	}
	
	qp->q_key[i+1] = key;
	qp->q_elt[i+1] = elt;
	qp->q_count++;
	left = qp->q_max - qp->q_count;
	
	sem_post(&qp->q_mutex);
	return left;
}


/*------------------------------------------------------------------------
 *  deq --  remove an item from the head of a list and return it
 *------------------------------------------------------------------------
 */
void *
deq(int q)
{
	struct qinfo	*qp;
	char *elt;
	int	i;

	if(q < 0 || q >= MAXNQ)
		return NULL;
	if(!Q[q].q_valid || Q[q].q_count <= 0)
		return NULL;

	qp = &Q[q];

	sem_wait(&qp->q_mutex);

	elt = qp->q_elt[0];

	for (i=1; i < qp->q_count; ++i) {
		qp->q_elt[i-1] = qp->q_elt[i];
		qp->q_key[i-1] = qp->q_key[i];
	}
	qp->q_count--;

	sem_post(&qp->q_mutex);
	return elt;
}


/*------------------------------------------------------------------------
 *  newq --  allocate a new queue, return the queue's index
 *------------------------------------------------------------------------
 */
int
newq(unsigned size)
{
	struct qinfo *qp;
	int i;

	if(!qinit)
		initq();
	for(i = 0; i < MAXNQ; ++i) {
		if(!Q[i].q_valid)
			break;
	}
	if(i == MAXNQ)
		return -1;
	qp = &Q[i];
	qp->q_valid = 1;
	qp->q_max = size;
	qp->q_count = 0;
	qp->q_seen = -1;
	sem_init(&qp->q_mutex, 0, 1);
	qp->q_elt = (char **) malloc(sizeof(char *) * size);
	qp->q_key = (int *) malloc(sizeof(int) * size);
	if(!qp->q_key || !qp->q_elt)
		return -1;
		
	return i;
}

int
freeq(int q)
{
	struct qinfo *qp;

	if(q < 0 || q >= MAXNQ)
		return 0;
	if(!Q[q].q_valid || Q[q].q_count != 0)		/* user frees elts */
		return 0;

	qp = &Q[q];
	/* free resources */
	if(qp->q_key)
		free(qp->q_key);
	if(qp->q_elt)
		free(qp->q_elt);
	sem_destroy(&qp->q_mutex);
	qp->q_valid = 0;
	return 1;
}

static void
initq()
{
	int i;

	for(i = 0; i < MAXNQ; ++i)
		Q[i].q_valid = 0;
	qinit = 1;
}

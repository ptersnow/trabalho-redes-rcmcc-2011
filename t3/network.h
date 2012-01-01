#ifndef __NETWORK_H
#define __NETWORK_H

#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcp.h"
#include "tcb.h"
#include "tcpfsm.h"
#include "ports.h"
/*#include "tcpstat.h"*/
#include "tcptimer.h"

#define	MAXNQ	100

#define TCPSBS 4096
#define TCPRBS 8192

#define MQ_INPUT "/mqinput"
#define MQ_OUTPUT "/mqoutput"

#define	max(a,b)	( (a) > (b) ? (a) : (b) )
#define	min(a,b)	( (a) < (b) ? (a) : (b) )

struct	qinfo {
	int q_valid;
	int q_max;
	int q_count;
	int q_seen;
	sem_t q_mutex;
	int *q_key;
	char **q_elt;
};

extern int usocket;
extern short uport;

extern unsigned TcpInErrs, TcpOutSegs, TcpOutRsts, TcpCurrEstab, TcpRetransSegs, TcpEstabResets, TcpPassiveOpens, TcpAttemptFails, TcpActiveOpens;

int enq(int q, void *elt, int key);
void *deq(int q);
int newq(unsigned size);
int freeq(int q);

u_short tcpnxtp();
struct tcp *tcpnet2h(struct tcp *);
struct tcp *tcph2net(struct tcp *);
int tcpxmit(int, int);
int tcpreset(struct udp *);
int tcpsend(int, int);
int tcpiss(void);
int tcprexmt(int, int);

int tcballoc(void);
struct tcb *tcpdemux(struct udp *);
int tfcoalesce(struct tcb *, unsigned, struct tcp *);
int tfinsert(struct tcb *, long, unsigned);
int tcpsync(struct tcb *);
int tcpcon(struct tcb *);
int tcpbind(struct tcb *, struct in_addr, u_short);
int tcbdealloc(struct tcb *);
int tcpok(struct tcb *, struct udp *);
int tcpackit(struct tcb *, struct udp *);
int tcpwait(struct tcb *);
int tcpdata(struct tcb *, struct udp *);
int tcpacked(struct tcb *, struct udp *);
int tcpabort(struct tcb *, int);
int tcpopts(struct tcb *, struct udp *);
int tcpswindow(struct tcb *, struct udp *);
int tcpdodat(struct tcb *, struct tcp *, long, unsigned);
int tcpkick(struct tcb *);
void tcpwakeup(int, struct tcb *);
int tcpostate(struct tcb *, int);
int tcprwindow(struct tcb *);
int tcprtt(struct tcb *);
int tcpwinit(struct tcb *, struct tcb *, struct udp *);
int tcpsmss(struct tcb *, struct tcp *, u_char *);
int tcprmss(struct tcb *, struct udp *);
int tcpsndlen(struct tcb *, int, unsigned int *);
int tcphowmuch(struct tcb *);
int tcpgetdata(struct tcb *, u_char *, unsigned);
int tcpgetspace(struct tcb *, unsigned);
unsigned short tcpcksum(struct udp *, unsigned);

#endif

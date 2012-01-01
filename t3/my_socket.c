#ifndef __MY_SOCKET_H
#include "my_socket.h"
#endif

unsigned TcpInErrs = 0;
unsigned TcpOutSegs = 0;
unsigned TcpOutRsts = 0;
unsigned TcpCurrEstab = 0;
unsigned TcpRetransSegs = 0;
unsigned TcpEstabResets = 0;
unsigned TcpPassiveOpens = 0;
unsigned TcpAttemptFails = 0;
unsigned TcpActiveOpens = 0;

void
init_socket(int rport, int sport)
{
	int i, yes = 1;
	static int created = 0;
	struct sockaddr_in sin;

	if(!created) {
		pthread_create(&input, NULL, tcpinp, NULL);
		pthread_create(&output, NULL, tcpout, NULL);
		pthread_create(&timer, NULL, tcptimer, NULL);

		uport = htons(sport);
		if((usocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("socket");
				_exit(1);
		}

		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(rport);

		pthread_create(&udprecv, NULL, tcp_in, (void*) &sin);
		setsockopt(usocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if(bind(usocket, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
			perror("bind");
			_exit(1);
		}
		
		sem_init(&tcps_tmutex, 0 , 1);
		
		for(i = 1; i <= Ntcp; i++) {
			tcbtab[i].tcb_state = TCPS_FREE;
			tcbtab[i].tcb_dvnum = i;
		}
		
		created = 1;
		sleep(2);
	}
}

int
my_socket()
{
	struct tcb *ptcb;
	
	ptcb = &tcbtab[tcballoc()];
	ptcb->tcb_type = TCPT_SERVER;
	sem_post(&ptcb->tcb_mutex);
	
	return ptcb->tcb_dvnum;
}

int
my_bind(int socket, int lport)
{
	struct tcb *ptcb;

	ptcb = &tcbtab[socket];
	ptcb->tcb_lip.s_addr = INADDR_ANY;
	ptcb->tcb_lport = lport;
	ptcb->tcb_state = TCPS_LISTEN;
	ptcb->tcb_smss = 0;
	
	sem_post(&ptcb->tcb_mutex);
		
	return 0;
}

int
my_listen(int socket, int queuelen)
{
	char name[10];
	struct tcb *ptcb;
	struct mq_attr attr;

	/* initialize the queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = queuelen;
  attr.mq_msgsize = TCPPKTLEN;
  attr.mq_curmsgs = 0;

	printf("my_accept\n");
	ptcb = &tcbtab[socket];
	ptcb->tcb_lqsize = tcps_lqsize = queuelen;
	
	sprintf(name, "/%d", socket);
	ptcb->tcb_listenq = mq_open(name, O_CREAT | O_RDWR, 0644, &attr);
	mq_unlink(name);
	
	if(!ptcb->tcb_listenq) {
		printf("mq_open error\n");
		exit(1);
	}
	
	return 0;
}

int
my_accept(int socket, struct sockaddr_in *raddress, socklen_t length)
{
	struct tcb *ptcb;
	char msg[TCPPKTLEN + 1];

	ptcb = &tcbtab[socket];
	sem_wait(&ptcb->tcb_mutex);

	printf("my_accept ");
	if(ptcb->tcb_type != TCPT_SERVER)
		return -1;
	
	printf("mq_receive\n");
	mq_receive(ptcb->tcb_listenq, msg, TCPPKTLEN, NULL);
	ptcb->tcb_rip.s_addr = raddress->sin_addr.s_addr;
	ptcb->tcb_rport = raddress->sin_port;
	
	printf("my_accept: return %d\n", atoi(msg));
	return atoi(msg);
}

int
my_connect(int socket, const struct sockaddr_in *address, socklen_t length)
{
	int	error;
	struct tcb *ptcb;
  u_short fport = address->sin_port;
  struct in_addr fip = address->sin_addr;
  
  ptcb = &tcbtab[socket];
  ptcb->tcb_error = 0;
	
	printf("my_connect: \n");
  if(tcpbind(ptcb, fip, fport) != 1 || tcpsync(ptcb) != 0) {
    ptcb->tcb_state = TCPS_FREE;
    sem_destroy(&(ptcb->tcb_mutex));
    printf("erro na porra do bind\n");
    return -1;
  }
  
  printf("my_connect: ");
  if((error = tcpcon(ptcb))) {
    printf("erro na porra do connect\n");
    return error;
  }
  
  printf("tcb_dvnum==%d\n", ptcb->tcb_dvnum);
  
  return ptcb->tcb_dvnum;
}

int
my_read(int socket, void *buffer, int length)
{
	int cc;
	u_char *pch = (u_char *) &buffer;
	struct tcb *ptcb;

	ptcb = &tcbtab[socket];

	if(ptcb->tcb_state != TCPS_ESTABLISHED && ptcb->tcb_state != TCPS_CLOSEWAIT)
		return -1;
		
retry:
	sem_getvalue(&ptcb->tcb_rsema, &cc);
	printf("my_read: tcb=%d rsema=%d\n", ptcb->tcb_dvnum, cc);
	sem_wait(&ptcb->tcb_rsema);
	sem_getvalue(&ptcb->tcb_mutex, &cc);
	printf("my_read: tcb=%d mutex=%d\n", ptcb->tcb_dvnum, cc);
	sem_wait(&ptcb->tcb_mutex);

	if(ptcb->tcb_state == TCPS_FREE) {
		printf("my_read: gone\n");
		return -1;			/* gone		*/
	}
	
	if(ptcb->tcb_error) {
		printf("my_read: error\n");
		tcpwakeup(READERS, ptcb);
		sem_post(&ptcb->tcb_mutex);
		return ptcb->tcb_error;
	}
	
	if(ptcb->tcb_flags & TCBF_RUPOK) {
		printf("my_read: TCBF_RUPOK ");
		cc = tcpgetdata(ptcb, pch, length);
		printf("tcpgetdata: buffer=%s\n", pch);
	}
	else {
		if((length > ptcb->tcb_rbcount) &&
		    (ptcb->tcb_flags & TCBF_BUFFER) &&
		    ((ptcb->tcb_flags & (TCBF_PUSH|TCBF_RDONE)) == 0)) {
			sem_post(&ptcb->tcb_mutex);
			printf("again ");
			goto retry;
		}
		else {
			printf("my_read: ");
			cc = tcpgetdata(ptcb, pch, length);
			printf("tcpgetdata: buffer=%s\n", pch);
		}
	}
	
	printf("my_read: ");
	tcpwakeup(READERS, ptcb);
	sem_post(&ptcb->tcb_mutex);
	
	
	printf("my_read: cc=%d\n", cc);
	return cc;
}

int
my_write(int socket, void *buffer, int length)
{
	unsigned sboff;
  struct tcb *ptcb;
	int state, tocopy;
	u_char *pch = (u_char *)buffer;

	printf("my_write: ");
	ptcb = &tcbtab[socket];
	state = ptcb->tcb_state;	

	if(state != TCPS_ESTABLISHED && state != TCPS_CLOSEWAIT)
		return -1;
	
	tocopy = tcpgetspace(ptcb, length);	/* acquires tcb_mutex	*/
	if(tocopy <= 0)
		return tocopy;
		
	sboff = (ptcb->tcb_sbstart + ptcb->tcb_sbcount) % ptcb->tcb_sbsize;

	while(tocopy--) {
		ptcb->tcb_sndbuf[sboff] = *pch++;
		++ptcb->tcb_sbcount;
		if(++sboff >= ptcb->tcb_sbsize)
			sboff = 0;
	}
	
	printf("my_write: %d == %d - %s\n", length, ptcb->tcb_sbcount, ptcb->tcb_sndbuf);
	
	ptcb->tcb_flags |= TCBF_NEEDOUT;
	tcpwakeup(WRITERS, ptcb);
	sem_post(&ptcb->tcb_mutex);

	if(ptcb->tcb_snext == ptcb->tcb_suna) {
		printf("my_write: tcpkick\n");
		tcpkick(ptcb);
	}
		
	printf("my_write: length=%d\n", length);
	return length;
}

int
my_close(int socket)
{
	int error;
	struct tcb *ptcb;

	printf("my_close\n");
	ptcb = &tcbtab[socket];
	
	sem_getvalue(&ptcb->tcb_mutex, &error);
	if(error)
		sem_wait(&ptcb->tcb_mutex);
	
	switch (ptcb->tcb_state) {
	case TCPS_LISTEN:
	case TCPS_ESTABLISHED:
	case TCPS_CLOSEWAIT:
		break;
	case TCPS_FREE:
		return -1;
	default:
		sem_post(&ptcb->tcb_mutex);
		return -1;
	}
	printf("my_close: if\n");
	if(ptcb->tcb_error || ptcb->tcb_state == TCPS_LISTEN)
		return tcbdealloc(ptcb);
	/* to get here, we must be in ESTABLISHED or CLOSE_WAIT */

	TcpCurrEstab--;
	ptcb->tcb_flags |= TCBF_SNDFIN;
	ptcb->tcb_slast = ptcb->tcb_suna + ptcb->tcb_sbcount;
	if(ptcb->tcb_state == TCPS_ESTABLISHED)
		ptcb->tcb_state = TCPS_FINWAIT1;
	else	/* CLOSE_WAIT */
		ptcb->tcb_state = TCPS_LASTACK;
		
	printf("my_close: tcpkick\n");
	ptcb->tcb_flags |= TCBF_NEEDOUT;
	tcpkick(ptcb);
	sem_post(&ptcb->tcb_mutex);
	sem_wait(&ptcb->tcb_ocsem);		/* wait for FIN to be ACKed	*/
	error = ptcb->tcb_error;
	if(ptcb->tcb_state == TCPS_LASTACK)
		tcbdealloc(ptcb);

	return error;
}


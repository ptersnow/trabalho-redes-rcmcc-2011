#ifndef __MY_SOCKET_H
#define __MY_SOCKET_H

#include "network.h"

short uport;
int usocket;
int tcps_lqsize;	/* default SERVER queue size			*/
sem_t tcps_tmutex;	/* tcb table mutex				*/
pthread_t input, output, timer, udprecv;

void init_socket(int rport, int sport);

int my_socket();
int my_bind(int socket, int lport);
int my_listen(int socket, int queuelen);
int my_accept(int socket, struct sockaddr_in *raddress, socklen_t length);
int my_connect(int socket, const struct sockaddr_in *address, socklen_t length);
int my_read(int socket, void *buffer, int length);
int my_write(int socket, void *buffer, int length);
int my_close(int socket);

#endif

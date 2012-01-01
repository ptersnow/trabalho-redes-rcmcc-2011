/***************************************************************************
             request.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"

int
recvtimeout(int fd, char *buf)
{
	int n;
	fd_set fds;
	struct timeval tv;
	
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	n = select(fd + 1, &fds, NULL, NULL, &tv); 
	/*	n == 0  -> timeout
			n == -1 -> error */
	if(n == 0)
		return -2;
		
	if(n == -1)
		return -1;

	return recv(fd, buf, MAXBUF, 0);
}

void
handle(int socket)
{
	int parsed;
	request *req;
	ssize_t bytes;
	char buf[MAXBUF];
	
	req = (request *) malloc(sizeof(request));
	
	do {
	
		req->fd = socket;
		req->do_cache = 0;
		req->keepalive = 0;
		
		req->url = NULL;
		req->path = NULL;
		req->host = NULL;
		req->program = NULL;
		memset(&req->buffer, 0, sizeof(req->buffer));
		parsed = 0;
	
		while(1) {
			memset(&buf, 0, sizeof(buf));
			bytes = recvtimeout(req->fd, buf);
			if((bytes == -2) && !parsed) {
				printf("Timeout\n");
				break;
			}
			
			if(bytes == -1) {
				send_error(req->fd, 400, "Bad Request", "Can't parse request.");
				break;
			}
			
			if(bytes > 0) {
				buf[bytes] = '\0';
				strcat(req->buffer, buf);
			
				if(strstr(req->buffer, "\r\n\r\n") != NULL)
					break;
			}
			else break;
		}
		
		if((bytes != -1) && (parsed = parse_request(req)))
			process_request(req);
		
		shutdown(req->fd, 1);
	} while(req->keepalive && (bytes > 0) && parsed);
	
	printf("Close\n");
	shutdown(req->fd, 2);
	free(req);
}

int
parse_request(request *req)
{
	char *line, *value;
	char c, buf[MAXBUF];

	sscanf(req->buffer, "%s %s %s\r\n", req->method, buf, req->version);
	if(memcmp(req->method, "GET", 3) && memcmp(req->method, "HEAD", 4) && memcmp(req->method, "POST", 4)) {
		send_error(req->fd, 501, "Not Implemented", "The server does not support \
		 the functionality required to fulfill the request");
		return 0;
	}
	
	if(!memcmp(req->version, "HTTP/1.1", 8))
		req->keepalive = 1;
	
	if(buf[0] == '/') {
		req->path = (char *) malloc(strlen(buf) + 1);
		strcpy(req->path, ".");
		strcat(req->path, buf);
		
		printf("PATH --> %s %s %s\n", req->method, req->path, req->version);
	}
	else if(!memcmp(buf, "http", 4))
	{
		value = strchr(buf, ':');
		*value++ = '\0';            /* overwrite the : */
		while((c = *value) && (c == '/'))
			value++;
		req->host = (char *) malloc(strlen(value) + 1);
		strcpy(req->host, value);
				
		value = strchr(req->host, ':');
		if(value != NULL)
			req->port = atoi(++value);
		else req->port = 80;

		value = strchr(req->host, '/');
		if(value != NULL) {
			req->url = (char *) malloc(strlen(value) + 1);
			strcpy(req->url, value);
			req->host[strlen(req->host) - strlen(value)] = '\0';
		}
		else {
			req->url = (char *) malloc(sizeof(char) + 1);
			strcpy(req->url, "/");
		}
		
		printf("HTTP --> %s:%d%s\n", req->host, req->port, req->url);
	}
	
	line = strtok(req->buffer, "\n");
	while((line = strtok(NULL, "\n")) != NULL) {
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
				break;
		value = strchr(line, ':');
		
		if(value == NULL) {
			send_error(req->fd, 400, "Bad Request", "Can't parse request.");
			return 0;
		}
			
		*value++ = '\0';
		while((c = *value) && (c == ' ' || c == '\t'))
			value++;
		
		if(!strncasecmp(value, "No-Cache", 8) || !strncasecmp(value, "max-age=0", 9))
			req->do_cache = 0;
		else if(!memcmp(line, "Connection", 11))
			req->keepalive = (!strncasecmp(value, "Keep-Alive", 10) ?	1 : 0);
    else if(!memcmp(line, "Content-Length", 14))
			req->content_length = atoi(value);
	}
	
	return 1;
}

void
process_request(request *req)
{
	DIR *dir;
	
	fd_set rfds;
	int fd, bytes;
	char *aux, *link;
	struct timeval tv;
	struct dirent *fhdl;
	struct stat statbuf;
	
	if(req) {
		if(!proxy && req->path) {
			printf("req->path\n");
		  if(!memcmp(req->method, "POST", 4)) {
		  	printf("POST\n");
				if(req->content_length > 0) {
					req->post_fd = create_temporary_file();
					if(!post(req))
						send_error(req->fd, 500, "Server Error", "The server encountered an internal error and could not complete your request.");
				}
				else send_error(req->fd, 400, "Bad Request", "Can't parse request.");
		  }
		
		  else if((stat(req->path, &statbuf) < 0) && (strchr(req->path, '?') == NULL))
				send_error(req->fd, 404, "Not Found", "File not found.");
			else if(S_ISDIR(statbuf.st_mode)) {
				printf("DIR\n");

				asprintf(&aux, "%sindex.html", req->path);
				if(stat(aux, &statbuf) >= 0)
					send_file(req->fd, req->method, aux, &statbuf);
				else
				{
					send_headers(req->fd, 200, "OK", "text/html", -1);

					if(!memcmp(req->method, "GET", 3)) {
						printf("OPENDIR\n");
						dir = opendir(req->path);
						while((fhdl = readdir(dir))) {
							strcpy(aux, req->path);
							strcat(aux, fhdl->d_name);

							stat(aux, &statbuf);
							asprintf(&link, "<a href=\"%s%s\"> %s%s </a> <br />", fhdl->d_name, S_ISDIR(statbuf.st_mode) ? "/" : "",
											 fhdl->d_name, S_ISDIR(statbuf.st_mode) ? "/" : "");
							send(req->fd, link, strlen(link), 0);
							printf("%s\n", link);
						}
						closedir(dir);
					}
				}
			}
			else if(strncmp(req->path, "./cgi-bin/", 10) == 0)
		    do_cgi(req);
			else send_file(req->fd, req->method, req->path, &statbuf);
		}
		else if(req->host) {
			asprintf(&link, "%s%s", req->host, req->url);
			if(!search(link)) {
	
				FD_ZERO(&rfds);
				FD_SET(req->fd,&rfds);
		
				if((fd = lruc_get(link, strlen(link))) != NULL) {
					printf("cache\n");
					do {
						bytes = read(fd, req->buffer, sizeof(req->buffer));
						if(bytes > 0)
							send(req->fd, req->buffer, strlen(req->buffer), 0);
					} while(bytes > 0);
					
					close(fd);
				}
				else if((req->rfd = openProxy(req->host, req->port)) > 0) {
	
					printf("Connected to %s on port %d -- ", req->host, req->port);

					asprintf(&aux,"%s %s %s\nHost: %s\nProxy-Connection: Keep-Alive\nAgent: Proxy-%s\n\n",
						req->method, req->url, req->version, req->host, SERVER);
					send(req->rfd, aux, strlen(aux), 0);
					printf("%s", aux);
					
					if(req->do_cache)
						lruc_set(link, strlen(link), NULL, 0);
													
					while(!sigint_flag) {
						tv.tv_sec = 10;
						tv.tv_usec = 0;
				
						FD_ZERO(&rfds);
						FD_SET(req->rfd, &rfds);
						FD_SET(req->fd, &rfds);
				
						if(select(FD_SETSIZE, &rfds, NULL, NULL, &tv) < 0)  {
							if(errno == EAGAIN)
								continue;
							perror("select");
							break;
						}
					
						if(FD_ISSET(req->rfd, &rfds)) {
							if((bytes = recv(req->rfd, req->buffer, sizeof(req->buffer), 0)) < 1) {
								perror("recv_proxy");
								break;
							}
						
							if(req->do_cache)
								lruc_set(link, strlen(link), req->buffer, strlen(req->buffer));
						
							if(send(req->fd, req->buffer, bytes, 0) < bytes) {
								perror("send_toproxy");
								break;
							}
						}
						else if(FD_ISSET(req->fd, &rfds)) {
							if((bytes = recv(req->fd, req->buffer, sizeof(req->buffer), 0)) < 1) {
								perror("recv_fromproxy");
								break;
							}
						
							if(send(req->rfd, req->buffer, bytes, 0) < bytes) {
								perror("send_proxy");
								break;
							}
						}
					}
				
					free(aux);
					close(req->rfd);
				}
			}
			else send_error(req->fd, 403, "Forbidden", "Access denied by Admin.");

			free(link);
		}
	}
}

int
do_cgi(request *req)
{
	FILE *fp;

	int i;
	char *aux = (char *) malloc(strlen(req->path) + 1);
	
	/* Se não for um diretório, verifica se é um arquivo dentro do cgi-bin  */
	printf("CGI-BIN\n");
	send_headers(req->fd, 200, "OK", "text/plain", -1);

	req->program = (char *) malloc(strlen(req->path) + 1);

	/* Copia o caminho até o executável */
	i = 0;
	while(req->path[i] != '?' && req->path[i] != ' ' && i < strlen(req->path)) {
		req->program[i] = req->path[i];
		i++;
	}

	/* Se tiver argumentos, copia para a variável de ambiente QUERY_STRING */
	if(req->path[i] == '?')
		setenv("QUERY_STRING", &req->path[++i], 1);

	if(!memcmp(req->method, "POST", 4)) {
		lseek(req->post_fd, SEEK_SET, 0);
		dup2(req->post_fd, STDIN_FILENO);
		close(req->post_fd);
	}

	/* Executa o programa */
	if((fp = popen(req->program, "r")) != NULL) {
		while(fgets(aux, sizeof(aux), fp) != NULL)
			send(req->fd, aux, strlen(aux), 0);
		pclose(fp);
	}
	
	return 1;
}


/***************************************************************************
             request.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"

void
handle_request(int fd)
{
	do {
	} while(req->keepalive);
}

request *
get_request(int fd)
{
	request *req;
	ssize_t bytes;
	char c, buf[8192];
	char *line, *value;
	
	req = (request *) malloc(sizeof(request));
	if(!req) {
		perror("new");
		return NULL;
	}
	
	req->fd = fd;
	req->do_cache = 1;
	req->keepalive = 0;
	memset(&req->buffer, 0, sizeof(req->buffer));
	
	while(1) {
		memset(&buf, 0, sizeof(buf));
		if((bytes = recv(req->fd, buf, MAXBUF, 0)) < 1) {
			perror("recv");
			free(req);
			return NULL;
		}

		buf[bytes] = '\0';
		strcat(req->buffer, buf);
		if(strstr(req->buffer, "\r\n\r\n") != NULL)
			break;
	}
	
	sscanf(req->buffer, "%s %s %s\r\n", req->method, buf, req->version);
	
	if(memcmp(req->method, "GET", 3) && memcmp(req->method, "HEAD", 4) && memcmp(req->method, "POST", 4)) {
		send_error(req->fd, 501, "Not Implemented", "The server does not support \
		 the functionality required to fulfill the request");
		free(req);
		return NULL;
	}
	
	if(!memcmp(req->version, "HTTP/1.1", 8))
		req->keepalive = 1;
	
	if(buf[0] == '/') {
		req->path = (char *) malloc(strlen(buf) + 1);
		strcpy(req->path, ".");
		strcat(req->path, buf);
		
		printf("PATH --> %s %s %s\n", req->method, req->path, req->version);
	}
	
	line = strtok(req->buffer, "\n");
	while((line = strtok(NULL, "\n")) != NULL) {
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
				break;
		value = strchr(line, ':');
		
		if(value == NULL) {
			send_error(req->fd, 400, "Bad Request", "Can't parse request.");
			free(req);
			return NULL;
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

	return req;
}

void
process_request(request *req)
{
	DIR *dir;

	char *aux, *link;
	struct dirent *fhdl;
	struct stat statbuf;
	
	if(req) {
		if(req->path) {
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
	
		if(!req->keepalive) {
			printf("Close\n");
			shutdown(req->fd, 2);
			FD_CLR(req->fd, &readfd);
		}
		else {
			printf("Keep-Alive\n");
			shutdown(req->fd, 1);
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


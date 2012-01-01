/***************************************************************************
             server.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"

char *
get_mime_type(char *name)
{
  char *ext = strrchr(name, '.');
  
  if(!ext) {
  	free(ext);
  	return "application/octet-stream";
  }
  	
  if(strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
  	free(ext);
  	return "text/html";
  }
  	
  if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
  	free(ext);
  	return "image/jpeg";
  }

  if(strcmp(ext, ".css") == 0) {
  	free(ext);
  	return "text/css";
  }

  if(strcmp(ext, ".pdf") == 0) {
  	free(ext);
  	return "application/pdf";
  }
  
	free(ext);
  return "application/octet-stream";
}

void
send_headers(int sock, int status, char *title, char *mime, int length)
{
	char *aux;
  char header[4096];

  sprintf(header, "%s %d %s\r\nServer: %s\r\n \
  	Content-Type: %s\r\n", PROTOCOL, status, title, SERVER,
  	(mime != NULL) ? mime : "text/plain");
  
  if(length >= 0) {
	  asprintf(&aux, "Content-Length: %d\r\n", length);
	  strcat(header, aux);
	  free(aux);
	}
  
  strcat(header, "\r\n");
  send(sock, header, strlen(header), 0);
}

void
send_error(int sock, int status, char *title, char *text)
{
	char *error;

  send_headers(sock, status, title, "text/html", -1);
  asprintf(&error, "<html>\n<head><title>%d %s</title></head>\n \
  	<body><h4>%d %s</h4>%s</body>\n</html>\n", status, title, 
  	status, title, text);
  
  send(sock, error, strlen(error), 0);
  free(error);
}

void
send_file(int sock, char *method, char *path, struct stat *statbuf)
{
	FILE *fp = fopen(path, "r");
  
  int n, length;
  char data[4096];
  
  printf("OPEN\n");
  if (!fp)
    send_error(sock, 403, "Forbidden", "Access denied.");
  else
  {				
    length = S_ISREG(statbuf->st_mode) ? statbuf->st_size : -1;
    send_headers(sock, 200, "OK", get_mime_type(path), length);

		if(!memcmp(method, "GET", 3))
			while((n = fread(data, 1, sizeof(data), fp)) > 0)
	    	send(sock, data, sizeof(char) * n, 0);
    fclose(fp);
  }
}

void
perror_exit(const char *msg)
{
	perror(msg);
	_exit(1);
}

void
usage(char *exec)
{
	printf("\nUsage: %s <options> \
	\nwhere possible options include:\n \
	\r -f <port>     \t\t Specify the port where the server will run.\n \
	\r -t <N> <port> \t\t Generate <N> threads to run.\n\n", exec);
	
	_exit(1);
}

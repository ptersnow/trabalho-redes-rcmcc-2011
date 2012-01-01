#include "httpd.h"

int
create_temporary_file()
{
	static char temp[2047];
	int fd;

	snprintf(temp, 2047, "/tmp/post.XXXXXX");

	fd = mkstemp(temp);
	if(fd == -1) {
		perror("mkstemp");
		return 0;
	}

	if(unlink(temp) == -1) {
		close(fd);
		fd = 0;
		perror("unlink");
	}

	return(fd);
}

int
post(request *req)
{
    int aux;
    int bytes = req->content_length;
    char *buf = (char *) malloc(strlen(req->buffer) - bytes + 1);

    strcpy(buf, &req->buffer[strlen(req->buffer) - bytes]);
    printf("buf--> %s\n", buf);

    aux = write(req->post_fd, buf, bytes);
    if(aux < bytes) {
    	perror("post write");
      return 0;
    }
    
    return do_cgi(req);
}

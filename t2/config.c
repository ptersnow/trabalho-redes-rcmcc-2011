/***************************************************************************
             config.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"

int
config(char *file)
{
	FILE *fp;
	
	int port;
	size_t len = 0;
	char *aux, *ptr, *line;
	
	struct stat statbuf;

	fp = fopen(file, "r");
	
	if(fp)
	{
		printf("Reading config file: %s\n", file);

		while(!feof(fp))
		{
			len = getline(&line, &len, fp);
			line[len-1] = '\0';
			
			ptr = strchr(line, '=');
			if(ptr) {
				*ptr++ = '\0';
			
				if(!strncmp("PORT", line, 4))
					port = atoi(ptr);
				else if(!strncmp("THREADS", line, 7))
					n = atoi(ptr);
				else if(!strncmp("CACHE SIZE", line, 10)) {
					total_memory = atoi(ptr) * 1024;
					if(strstr(ptr, "Mb") != NULL)
						total_memory *= 1024;
					else if(strstr(ptr, "Gb") != NULL)
						total_memory *= 1048576;
				}
				else if(!strncmp("CACHE DIRECTORY", line, 15)) {
					cache_directory = (char *) malloc(len + 1);
					strcpy(cache_directory, ptr);
					
					if((stat(cache_directory, &statbuf) < 0)) {
						if(mkdir(cache_directory, S_IRWXU) < 0)
							perror_exit("mkdir");
					}
					else if(!S_ISDIR(statbuf.st_mode)) {
						aux = (char *) malloc(len + 5);
						strcpy(aux, ptr);
						strcat(aux, "_old");
					
						if(rename(cache_directory, aux) < 0) {
							free(aux);
							perror_exit("rename");
						}
							
						if(mkdir(cache_directory, S_IRWXU) < 0)
							perror_exit("mkdir");
					}
				}
				else if(!strncmp("ACCESS LIST", line, 15)) {
					access_list = (char *) malloc(len + 1);
					strcpy(access_list, ptr);
				}
			}
		}
	}
	else perror_exit("config");

	fclose(fp);
	free(ptr);
	free(line);
	
	return port;
}


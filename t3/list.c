/***************************************************************************
             list.c

		Version:	2.25 
  	Wed Aug 31 10:33:22 2011
   Copyright  2011  Karolina, Patricia, Pedro
   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com

****************************************************************************/

#include "httpd.h"
#include <ctype.h>

static void
perror_exit(const char *msg)
{
	perror(msg);
	_exit(1);
}

char *get_regerror(int errcode, regex_t *compiled)
{
	size_t length = regerror (errcode, compiled, NULL, 0);
	char *buffer = (char *) malloc(length);
	
	(void) regerror (errcode, compiled, buffer, length);
	return buffer;
}

void lopen()
{
  FILE *file = fopen(access_list, "r");
  
  if (!file)
		perror_exit("access list");

	sem_init(&sem_deny, 0, 1);
	/*Inicializa a lista vazia: DENY */
	deny = (list *) malloc(sizeof(list));
	deny->prox = NULL;
	
	sem_init(&sem_allow, 0, 1);
	/*Inicializa a lista vazia: ALLOW */
	allow = (list *) malloc(sizeof(list));
	allow->prox = NULL;
			
	printf("Reading access list file: %s\n", access_list);
  insert_list(file);
}

void search_insert(char *x, list *ini)
{
	int error;
	list *p, *q, *new;
	p = ini;
	q = ini ? ini->prox : NULL;
	
	while((q != NULL) && (strcmp(q->string, x) != 0))
	{
		p = q;
		q = q->prox;
	}
	
	if(q == NULL)
	{
		new = (list *) malloc(sizeof(list));
		new->string = (char *) malloc(strlen(x) + 1);
		strcpy(new->string, x);
		
		if(x[0] == '*')
			*x++ = '\0';
		
		if((error = regcomp(&new->regexp, x, REG_ICASE | REG_EXTENDED)) != 0)
			printf("%s\n", get_regerror(error, &new->regexp));
		new->prox = NULL;

		p->prox = new;
	}	
}

void free_list()
{
	list *p, *q;

	p = allow;
	q = allow->prox;
	
	sem_wait(&sem_allow);
	while(q != NULL)
	{
		p = q;
		q = q->prox;
		
		regfree(&p->regexp);
		free(p->string);
		free(p);
	}	
	sem_post(&sem_allow);
	free(allow);
	sem_destroy(&sem_allow);
	
	p = deny;
	q = deny->prox;
	
	sem_wait(&sem_deny);
	while(q != NULL)
	{
		p = q;
		q = q->prox;
		
		regfree(&p->regexp);
		free(p->string);
		free(p);
	}
	sem_post(&sem_deny);
	free(deny);
	sem_destroy(&sem_deny);
}

void print(list *ini)
{
   list *p;
   for(p = ini->prox; p != NULL; p = p->prox)
      printf ("%s\n", p->string);
}
        
void insert_list(FILE *file)
{
	size_t len = 0;
  ssize_t read;
  char *line = NULL;
  char *al, *pch;
  size_t pos, line_size;
  
  sem_wait(&sem_deny);
  sem_wait(&sem_allow);
  while ((read = getline(&line, &len, file)) != -1) 
  {
  	if(strncmp ("DENY=", line, 5) == 0)
  	{
  			pch = (char*) memchr(line, ' ', strlen(line)); /*encontra a posicao onde ocorre o primeiro espaco */
  			pos =  pch-line;
  			line_size = strlen(line);
  			al = (char *) malloc (line_size);
  			memset(al, 0, line_size);
				if (pch != NULL)
				{
					memcpy(al, line, pos); /* copia até o primeiro espaco */
					memmove(al, al+5, pos); /* retira: DENY= */
				}
				else
				{
					memcpy(al, line, line_size-1); /* copia até o fim da lista sem o \n */
					memmove(al, al+5, line_size);
				}
				search_insert(al, deny);
				
				free(al);
				free(pch);
  	}
  	else if(strncmp ("ALLOW=", line, 6) == 0)
  	{
  			pch = (char*) memchr (line, ' ', strlen(line));
  			pos =  pch-line;
  			line_size = strlen(line);
				al = (char *) malloc (line_size);
				memset(al, 0, line_size);
				if (pch != NULL)
				{	
					memcpy(al, line, pos); /* copia até o primeiro espaco */
					memmove(al, al+6, pos); /* retira: ALLOW= */
				}
				else
				{
					memcpy(al, line, line_size-1); /* strcpy(al, line); */
					memmove(al, al+6, line_size);
				}
				search_insert(al, allow);
				
				free(al);
				free(pch);
  	}
	}
	
	free(line);
	fclose(file);
	printf("Lista Encadeada DENY:\n");
	print(deny);
	printf("Lista Encadeada ALLOW:\n");
	print(allow);
	
	sem_post(&sem_deny);
  sem_post(&sem_allow);
}

int
search(char *string)
{
	list *p;
	
	sem_wait(&sem_allow);
	for(p = allow->prox; p != NULL; p = p->prox) {
		if(!regexec(&p->regexp, string, 0, NULL, 0)) {
			sem_post(&sem_allow);
			return 0;
		}
	}
	sem_post(&sem_allow);
	
	sem_wait(&sem_deny);
	for(p = deny->prox; p != NULL; p = p->prox) {
		if(!regexec(&p->regexp, string, 0, NULL, 0)) {
			sem_post(&sem_deny);
			return 1;
		}
	}
	sem_post(&sem_deny);
	
	return 0;
}


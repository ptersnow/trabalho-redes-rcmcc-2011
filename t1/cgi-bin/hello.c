#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	char *env = getenv("QUERY_STRING");

	printf("\nHello World\n");
	printf("env == %s\n", env);
	
	return 0;
}

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char string[10];

	printf("argc == %d\n", argc);
	scanf("%s", string);
	
	printf("%s\n", string);
	return 0;
}

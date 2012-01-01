#include<stdio.h>

char buf[100];

int main(int argc, char *argv[], char *envp[])
{
	FILE *ph;

	//putenv("QUERY_STRING=par1&par2");
	if ((ph=popen("./cgi-bin/test", "r"))) {
		while(fgets(buf, 40, ph))
			printf("%s", buf);
		pclose(ph);
	}
	else printf("Error executing test\n");
}

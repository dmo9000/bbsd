#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char buffer[80];
int main(int argc, char *argv[])
{

		/* we do this to disable buffering of stdout, which mitigates
		   the need to call fflush() after every message */ 

		setvbuf(stdout, NULL, _IONBF, 0);

    printf("What is your name?\n");
    memset(&buffer, 0, 80);
    fgets((char *) &buffer, 79, stdin);
    printf("Your name is: %s\n", buffer);

    exit(0);

}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char myhostname[256];

int main(int argc, char *argv[])
{
    char buffer[80];
    memset(&myhostname, 0, 256);
    gethostname((char *) &myhostname, 255);

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\n\nBMI Technology Menu\n");
    printf("You are connected on node [%s]\n\n", myhostname);

    printf("\t1)	Download latest software release\n");
    printf("\t2)	Disconnect\n");

    printf("\n");
    printf("Enter your choice: ");
    memset(&buffer, 0, 80);
    fgets((char *) &buffer, 79, stdin);
    printf("\n\n");
    printf("Your choice was: %s\n", buffer);

    printf("\n");
	exit(1);

}



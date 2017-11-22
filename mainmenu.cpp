#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char buffer[80];
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\n\nBMI Technology Menu\n\n");

    printf("\t1)	Download latest software release\n");
    printf("\t2)	Disconnect\n");

    printf("\n");
    printf("Enter your choice: ");
    memset(&buffer, 0, 80);
    fgets((char *) &buffer, 79, stdin);
    printf("Your choice was: %s\n", buffer);

		exit(1);

}



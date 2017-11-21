#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char buffer[80];
int main(int argc, char *argv[])
{

    printf("What is your name?\n");
    fflush(NULL);
    memset(&buffer, 0, 80);
    fgets((char *) &buffer, 79, stdin);
    printf("Your name is: %s\n", buffer);
    fflush(NULL);

    exit(0);

}

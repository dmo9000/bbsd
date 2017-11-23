#include <cstdio>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "subprocess.h"

using std::cout;
using std::endl;
using std::vector;

char myhostname[256];

int main(int argc, char *argv[])
{
		Subprocess *shell = NULL;
    char buffer[80];
		char *myargv[64];
		int r = 0, w = 0;
		pid_t child_process = 0;
		int rd =0, wr = 0;
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

    myargv[0] = (char *) "/usr/bin/fortune";
    myargv[1] = (char *) "-l";
		myargv[2] = NULL;
    shell = new Subprocess();
    child_process = shell->StartProcess(myargv[0], myargv);
    r = shell->GetPipeFD(PARENT_READ_PIPE, READ_FD);
    w = shell->GetPipeFD(PARENT_WRITE_PIPE, WRITE_FD);
		shell->RegisterSocket(r, w);
		rd = shell->pRead();	

		while (rd > 0 || (rd < 0 && errno == EAGAIN)) {
				if (rd > 0) {
					wr = write(STDOUT_FILENO, shell->GetReadBuffer(), rd); 
					if (wr != rd) {
						cout << "wr != rd" << endl;
						exit(1);
						}
					shell->SetRbufsize(0);
					}
				rd = shell->pRead();	
				}

 	cout << endl;

	exit(1);

}



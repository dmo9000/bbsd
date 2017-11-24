#include <cstdio>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include "subprocess.h"

using std::cout;
using std::endl;
using std::vector;

#define CHAR_ESCAPE	0x1B

char myhostname[256];

/*

    This code will probably never need to run on Windows, but if it ever does, bear
    in mind that Windows treats "text" and "binary" file modes differently, but UNIX/Linux
    don't. Windows opens stdout, stderr etc. in text mode by default.

      https://msdn.microsoft.com/en-us/library/ktss1a9b.aspx

    So to avoid translating CRLF to CRCRLF when in text mode, you'd want to call _setmode()
    on the filehandle to switch it to binary.

*/

int RunSubprocess(char *argv[]);

int main(int argc, char *argv[])
{
    Subprocess *shell = NULL;
    char buffer[80];
    char *myargv[64];
    int r = 0, w = 0;
    pid_t child_process = 0;
    int rd =0, wr = 0;
    int choice = 0;
    bool logoff_requested = false;

    setvbuf(stdout, NULL, _IONBF, 0);

    memset(&myhostname, 0, 256);
    gethostname((char *) &myhostname, 255);

    printf ("%c[H%c[2J", CHAR_ESCAPE, CHAR_ESCAPE);
    printf ("%c[1;1H", CHAR_ESCAPE);
		printf ("\n");

    myargv[0] = (char *) "/usr/bin/cat";
    myargv[1] = (char *) "/usr/local/bbsd/data/fruit.ans";
    myargv[2] = NULL;
    if (!RunSubprocess(myargv)) {
        cout << endl << "Error: couldn't start process" << endl;
				exit(1);
        };

    printf("\n\nBMI Technology Menu\n");
    printf("You are connected on node [%s]\n\n", myhostname);

    while (!logoff_requested) {

        printf("\t1)	Receive a fortune cookie\n");
        printf("\t2)	Download latest software release\n");
        printf("\t3)	Disconnect\n");

        printf("\n");
        printf("Enter your choice: ");
        memset(&buffer, 0, 80);
        fgets((char *) &buffer, 79, stdin);
        printf("\n\n");
        printf("Your choice was: %s\n", buffer);
        printf("\n");

        buffer[1] = '\0';
        choice = atoi(buffer);

        switch (choice) {
        case 1:

						/* FIXME: vt100 specific clear screen */	

            printf ("%c[H%c[2J", CHAR_ESCAPE, CHAR_ESCAPE);
            printf ("%c[1;1H", CHAR_ESCAPE);
						printf ("\n");

            myargv[0] = (char *) "/usr/bin/fortune";
            myargv[1] = (char *) "-l";
            myargv[2] = NULL;
            if (!RunSubprocess(myargv)) {
                cout << endl << "Error: couldn't start process" << endl;
            };
            cout << endl << endl << endl << endl;
            break;
        case 2:
            cout << endl << endl << endl << endl;
            myargv[0] = (char *) "/usr/bin/sz";
            myargv[1] = (char *) "/usr/local/bbsd/data/oempkg.arc";
            myargv[2] = NULL;
            if (!RunSubprocess(myargv)) {
                cout << endl << "Error: couldn't start process" << endl;
            };
            cout << endl << endl << endl << endl;
            break;
        case 3:
            cout << endl << endl << "Thanks for visiting BMI Technology." << endl << endl;
            logoff_requested = true;
            exit(0);
            break;
        default:
            cout << endl << endl << "That menu choice is invalid." << endl;
            break;
        }

    }

    /* never reached */

}


int RunSubprocess(char *myargv[])
{
    Subprocess *shell = NULL;
    int r = 0, w = 0;
    pid_t child_process = 0;
    int rd =0, wr = 0;

    shell = new Subprocess();
    child_process = shell->StartProcess(myargv[0], myargv);

    if (child_process < 1) {
        cout << "[child_process = " << child_process << "]" << endl ;
        return 0;
    }

    r = shell->GetPipeFD(PARENT_READ_PIPE, READ_FD);
    w = shell->GetPipeFD(PARENT_WRITE_PIPE, WRITE_FD);
    shell->RegisterSocket(r, w);
    rd = shell->pRead();

    while (rd > 0 || (rd < 0 && errno == EAGAIN)) {
        if (rd > 0) {
            wr = write(STDOUT_FILENO, shell->GetReadBuffer(), rd);
            if (wr != rd) {
                cout << "+++ wr != rd" << endl;
                return(0);
            }
            shell->SetRbufsize(0);
        }
        rd = shell->pRead();
    }
    return 1;

}



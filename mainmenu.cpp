#include <cstdio>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "subprocess.h"
#include "build-id.h"

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
char terminal_type[80];

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
    int flags = 0;
    bool logoff_requested = false;
    char *fgc = NULL;
    int sp = 0;
    char *env_v = NULL;

    /* FIXME: we should refuse to run unless we are running as user nobody */

    setvbuf(stdout, NULL, _IONBF, 0);
    sleep(1);

    memset(&terminal_type, 0, 80);
    setenv("TERM", "ansi", 0);
    setenv("HOME", "/home/bootstrap", 1);
    env_v = secure_getenv("TERM");

    if (env_v  != NULL) {
        strncpy((char *) &terminal_type,  env_v, strlen(env_v));
        printf("[Auto-detected terminal type]\n");
    }

    /* set stdin to be non blocking */

    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    memset(&myhostname, 0, 256);
    gethostname((char *) &myhostname, 255);

    printf ("%c[H%c[2J", CHAR_ESCAPE, CHAR_ESCAPE);
    printf ("%c[1;1H", CHAR_ESCAPE);
    printf ("\n");

    myargv[0] = (char *) "/usr/bin/cat";
    myargv[1] = (char *) "/usr/local/bbsd/data/k1shack.ans";
    //myargv[1] = (char *) "/usr/local/bbsd/data/bmilogo.ans";
    myargv[2] = NULL;
    if (!RunSubprocess(myargv)) {
        cout << endl << "Error: couldn't start process" << endl;
        exit(1);
    };


    /* reset terminal pen & paper colors */
    printf ("%c[0m", CHAR_ESCAPE);


    while (!logoff_requested) {
        sleep(1);
        printf("\n\nBMI Technology Menu\n");
        printf("Services build id #%s\n", BUILD_ID);
        printf("You are connected on node [%s]\n", myhostname);
        struct winsize size;

        size.ws_col = 80;
        size.ws_row = 24;
        size.ws_xpixel = 0;
        size.ws_ypixel = 0;

        ioctl(STDOUT_FILENO,TIOCSWINSZ,&size);
        ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
        printf("Terminal type is %s:%dx%d\n\n", (const char *) terminal_type,
               size.ws_col, size.ws_row, size.ws_xpixel, size.ws_ypixel);

        printf("\n");

        printf("\t1)    Receive a fortune cookie\n");
        printf("\t2)    Play ZORK I\n");
        printf("\t3)    Download OEMPKG.ARC   (ZMODEM)\n");
        printf("\t4)    Download OEMFONTS.ARC (ZMODEM)\n");
        printf("\t5)    Download OEMIMAGE.ARC (ZMODEM)\n");
        printf("\t6)    Disconnect\n");

        printf("\n");
        printf("Enter your choice: ");
        memset(&buffer, 0, 80);

        fgc = fgets((char *) &buffer, 79, stdin);
        while (!fgc) {
        usleep(20000);
            fgc = fgets((char *) &buffer, 79, stdin);
        }


        printf("\n\n");
        printf("Your choice was: %s\n", buffer);
        printf("\n");

        buffer[1] = '\0';
        choice = atoi(buffer);

        switch (choice) {
    case 1:

        /* FIXME: VT100 specific clear screen */

        printf ("%c[H%c[2J", CHAR_ESCAPE, CHAR_ESCAPE);
            printf ("%c[1;1H", CHAR_ESCAPE);
            /* reset terminal pen & paper colors */
            printf ("%c[0m", CHAR_ESCAPE);
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

            cout << endl << endl ;
            chdir("/usr/local/bbsd/data");
            //myargv[0] = (char *) "/usr/bin/epic";
            //myargv[1] = (char *) "irc.freenode.net";
            myargv[0] = (char *) "/usr/bin/frotz"; 
            myargv[1] = (char *) "/usr/local/bbsd/data/zork1.z3"; 
            myargv[2] = NULL;
            sp = RunSubprocess(myargv);

            if (!sp) {
                cout << endl << "Error: couldn't start process" << endl;
            };
            cout << endl << endl ;
            printf("Subprocess returned %d\n", sp);
            break;

        case 3:
            cout << endl << endl ;
            printf ("%c[H%c[2J", CHAR_ESCAPE, CHAR_ESCAPE);
            printf ("%c[1;1H", CHAR_ESCAPE);

            chdir("/usr/local/bbsd/data");
            myargv[0] = (char *) "/usr/bin/sz";
            myargv[1] = (char *) "-vv";
            myargv[2] = (char *) "oempkg.arc";
            //myargv[2] = (char *) "testdata.bin";
            myargv[3] = NULL;
            sp = RunSubprocess(myargv);

            if (!sp) {
                cout << endl << "Error: couldn't start process" << endl;
            };
            cout << endl << endl ;
            printf("Subprocess returned %d\n", sp);
            break;

        case 4:
            cout << endl << endl ;
            printf ("%c[H%c[2J", CHAR_ESCAPE, CHAR_ESCAPE);
            printf ("%c[1;1H", CHAR_ESCAPE);


            chdir("/usr/local/bbsd/data");
            myargv[0] = (char *) "/usr/bin/sz";
            myargv[1] = (char *) "-vv";
            myargv[2] = (char *) "oemfonts.arc";
            myargv[3] = NULL;
            sp = RunSubprocess(myargv);

            if (!sp) {
                cout << endl << "Error: couldn't start process" << endl;
            };
            cout << endl << endl ;
            printf("Subprocess returned %d\n", sp);
            break;

        case 5:
            printf ("%c[H%c[2J", CHAR_ESCAPE, CHAR_ESCAPE);
            printf ("%c[1;1H", CHAR_ESCAPE);

            cout << endl << endl ;
            chdir("/usr/local/bbsd/data");
            myargv[0] = (char *) "/usr/bin/sz";
            myargv[1] = (char *) "-vv";
            myargv[2] = (char *) "oemimage.arc";
            myargv[3] = NULL;
            sp = RunSubprocess(myargv);
            if (!sp) {
                cout << endl << "Error: couldn't start process" << endl;
            };
            cout << endl << endl ;
            printf("Subprocess returned %d\n", sp);
            break;

        case 6:
            cout << "Thanks for visiting BMI Technology." << endl << endl;
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
    uint8_t stdio_buf[BUFSIZE];
    Subprocess *shell = NULL;
    int r = 0, w = 0;
    pid_t child_process = 0;
    int rd =0, wr = 0;
    uint16_t wsize = 0;
    int flags = 0;
    bool process_completed = false;
    pid_t c = 0;
    int wstatus;
    int options = WNOHANG;

    shell = new Subprocess();
    child_process = shell->StartProcess(myargv[0], myargv);

    if (child_process < 1) {
        cout << "[child_process = " << child_process << "]" << endl ;
        return 0;
    }

    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);


    r = shell->GetPipeFD(PARENT_READ_PIPE, READ_FD);
    w = shell->GetPipeFD(PARENT_WRITE_PIPE, WRITE_FD);
    shell->RegisterSocket(r, w);
    c =  waitpid(shell->GetPID(), &wstatus, options);


    while (c == 0) {
        rd = shell->pRead();
        //   printf("read_backend = %d\n", rd);

        if (rd > 0) {
            wr = write(STDOUT_FILENO, shell->GetReadBuffer(), rd);
            if (wr != rd) {
                cout << "+++ wr != rd" << endl;
                exit(1);
            }
            shell->SetRbufsize(0);
        }

        /* read from stdio */

        rd = read(STDIN_FILENO, &stdio_buf, BUFSIZE);
        //  printf("read_frontend = %d\n", rd);

        if (rd == -1 && errno != EAGAIN) {
            printf ("A serious I/O error occured; %s\n", strerror(errno));
            exit(1);
        }

        if (rd > 0) {
            wsize = shell->GetWbufsize();
            memcpy(shell->GetWriteBuffer(), &stdio_buf, rd);
            shell->SetWbufsize(rd);
            wr = shell->pWrite();
            if (wr != rd) {
                cout << "++ couldn't transfer " << rd << " bytes from stdin to subprocess, only " << wr << endl;
                printf ("A serious I/O error occured; %s\n", strerror(errno));
                exit(1);
            }
        }

        c =  waitpid(shell->GetPID(), &wstatus, options);
    }
    return 1;

}



#include <cstdio>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/ioctl.h>
#include "subprocess.h"
#include "build-id.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using std::cout;
using std::endl;
using std::vector;

#define JUSTIFY_BUFSIZE 1024
#define CHAR_ESCAPE	0x1B

void prompt_enter();

char myhostname[256];
struct winsize size;


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


int justify_text(int width, char *text)
{
    int l = 0;
    int ii = 0;
    assert(text);
    assert(strlen(text));
    assert(strlen(text) < width);

    l = strlen(text);
    l = (width - l) / 2;
    for (ii = 0; ii < l ; ii++) {
        putchar(' ');
    }
    printf("%s", text);
    return 0;

}

int main(int argc, char *argv[])
{
    unsigned char justify_buf[JUSTIFY_BUFSIZE];
    int s;
    struct sockaddr_in peer;
    int peer_len;
    Subprocess *shell = NULL;
    static char buffer[80];
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
    char *router_hostname = NULL;
    int ii = 0;

    /* FIXME: we should refuse to run unless we are running as user nobody */

    setvbuf(stdout, NULL, _IONBF, 0);
    sleep(1);

    memset(&terminal_type, 0, 80);
    setenv("TERM", "ansi", 0);
    setenv("HOME", "/home/bootstrap", 1);
    env_v = secure_getenv("TERM");
    router_hostname = secure_getenv("TRANSPORT_PATH");

    if (env_v  != NULL) {
        strncpy((char *) &terminal_type,  env_v, strlen(env_v));
        //printf("[Auto-detected terminal type]\n");
    }

    /* set stdin to be non blocking */

    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    memset(&myhostname, 0, 256);
    gethostname((char *) &myhostname, 255);

    printf ("%c[H%c[2J", CHAR_ESCAPE, CHAR_ESCAPE);
    printf ("%c[1;1H", CHAR_ESCAPE);

    /* reset terminal pen & paper colors */
    printf ("%c[0m", CHAR_ESCAPE);

    size.ws_col = 80;
    size.ws_row = 24;
    size.ws_xpixel = 0;
    size.ws_ypixel = 0;
    ioctl(STDOUT_FILENO,TIOCSWINSZ,&size);
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);

    printf("TERMINAL INPUT TESTER\n\n");
    cout << endl << endl;

    while (!logoff_requested) {
        sleep(1);

        printf("Enter some input: ");
        memset(&buffer, 0, 80);
        fgc = fgets((char *) &buffer, 79, stdin);
        while (!fgc) {
            usleep(20000);
            fgc = fgets((char *) &buffer, 79, stdin);
            if (strlen(buffer)) {
                printf("Length was: %d\n", strlen(buffer));
                for (int i = 0; i < strlen(buffer); i++) {
                    printf("[%02x:%c] ", buffer[i], ( buffer[i] >=32 && buffer[i] <= 127 ? buffer[i] : '.'));
                }
                printf("\n");
                if (strlen(buffer) == 1 && buffer[0] == 0x0a) {
                    goto finished;
                }
            }
            memset(&buffer, 0, 80);
        }
    }

    /* never reached */

finished:
    printf("Finished.\n");
    exit(0);

}



#define _POSIX_SOURCE
#include <algorithm>
#include <iostream>
#include <csignal>
#include <vector>
#include <sys/types.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>         // for exit()
#include "nvt.h"
#include "subprocess.h"
#include "build-id.h"

using std::cout;
using std::endl;
using std::vector;

//#define DEBUG_TRANSFERIO

/*

		This code will probably never need to run on Windows, but if it ever does, bear
		in mind that Windows treats "text" and "binary" file modes differently, but UNIX/Linux
		don't. Windows opens stdout, stderr etc. in text mode by default.

			https://msdn.microsoft.com/en-us/library/ktss1a9b.aspx

		So to avoid translating CRLF to CRCRLF when in text mode, you'd want to call _setmode()
		on the filehandle to switch it to binary.

*/

static void myerror(const char *msg);

#define LISTEN_PORT     8024
#define MAX_PIPELINES   128

/* this can all go into a controller class when more mature */

int PerformShutdown(Pipeline *p);
Pipeline* telnet_acceptor = NULL;

vector<Pipeline*> SelectablePipelines;
fd_set socketset;
struct timeval timeout;
int sockfd = -1;         /* socket for TCP listener */
bool terminated = false;


uid_t name_to_uid(char const *name)
{
    if (!name) {
        return -1;
    }

    long const buflen = sysconf(_SC_GETPW_R_SIZE_MAX);

    if (buflen == -1) {
        return -1;
    }
    // requires c99
    char buf[buflen];
    struct passwd pwbuf, *pwbufp;
    if (0 != getpwnam_r(name, &pwbuf, buf, buflen, &pwbufp)
            || !pwbufp) {
        return -1;
    }
    return pwbufp->pw_uid;
}

static void myerror(const char *msg)
{
    perror(msg);
    exit(1);
}

void int_handler(int x)
{
    cout << "Terminating ..." << endl;
    terminated = true;
}


int RegisterForIO(Pipeline* p)
{

    Pipeline* myPipeline = p;
    int r = 0;
    cout << "Registering for Pipeline IO" << endl ;

    r = p->GetRsockfd();

    if (r >= sockfd && r < MAX_PIPELINES) {
        SelectablePipelines.push_back(p);
    } else {
        cout << "+++ fd " << r << " is out of range" << endl;
        exit(1);
    }
    return 1;
}

int UnregisterForIO(Pipeline *p)
{


}

int BuildIOSelectSet()
{

    std::vector<Pipeline*>::iterator iter, end;

//    cout << "BuildIOSelectSet()" << endl;
//    cout << "There are " << SelectablePipelines.size() << " pipelines registered for IO" << endl;

    FD_ZERO (&socketset);
    for (iter = SelectablePipelines.begin() ; iter != SelectablePipelines.end(); ++iter) {
        if (!(*iter)->GetSelected()) {
            std::cout << "Adding fd " << (*iter)->GetRsockfd() << " to select set" << std::endl;
            (*iter)->SetSelected();
        }

        if ((*iter)->GetState() == STATE_DISCONNECTED && (*iter)->IsReadyForDeletion()) {
            cout << "BuildIOSelectSet(): found Pipeline for deletion" << endl;
            PerformShutdown(*iter);
            delete(*iter);
            *iter = (Pipeline*) NULL;

        } else {
            FD_SET ((*iter)->GetRsockfd(), &socketset);
        }
    }
    SelectablePipelines.erase(std::remove(SelectablePipelines.begin(), SelectablePipelines.end(), (Pipeline*) NULL), SelectablePipelines.end());

    return 1;

}

void ShutdownIO()
{
    std::vector<Pipeline*>::iterator iter, end, iter2;
    for(iter = SelectablePipelines.begin(), end = SelectablePipelines.end() ; iter != end; iter++) {
        //(*iter)->Shutdown();
        PerformShutdown(*iter);
    }

}


int PerformShutdown(Pipeline *p)
{
    NVT *nvt_ptr = NULL;
    Subprocess *sub_ptr = NULL;
    enum Pipeline_Type t;
    int r = 0;
    t = p->GetPipelineType();


    switch(t) {
    case Pipeline_Type::PIPELINE_RAW:
        p->Shutdown();
        return 1;
        break;
    case Pipeline_Type::PIPELINE_NVT:
        nvt_ptr = (NVT*) p;
        nvt_ptr->Shutdown();
        return 1;
        break;
    case Pipeline_Type::PIPELINE_SUBPROCESS:
        sub_ptr = (Subprocess*) p;
        sub_ptr->Shutdown();
        return 1;
        break;
    default:
        cout << "+++ Unknown Pipeline_Type" << endl;
        ShutdownIO();
        exit(1);
        break;
    }

    return -1;

}


int PerformReadIO(Pipeline *p)
{
    NVT *nvt_ptr = NULL;
    Subprocess *sub_ptr = NULL;
    enum Pipeline_Type t;
    int r = 0;
    t = p->GetPipelineType();

    switch(t) {
    case Pipeline_Type::PIPELINE_RAW:
        p->pRead();
        break;
    case Pipeline_Type::PIPELINE_NVT:
        nvt_ptr = (NVT*) p;
        nvt_ptr->UpdateConnectTime();
        r =  nvt_ptr->pRead();
        if (!r) {
            cout << "--> EOF on NVT" << endl;
            if (nvt_ptr->GetNextPipeline()) {
                cout << "--> Marking connected peer for deletion" << endl;
                nvt_ptr->GetNextPipeline()->SetReadyForDeletion();
                nvt_ptr->GetNextPipeline()->SetState(STATE_DISCONNECTED);
                //PerformShutdown(nvt_ptr->GetNextPipeline());
            } else {
                cout << "--> EOF on NVT, nothing attached" << endl;
            }
            nvt_ptr->SetReadyForDeletion();
            nvt_ptr->SetState(STATE_DISCONNECTED);
        }
        //nvt_ptr->Debug_Read();
        return r;
        break;
    case Pipeline_Type::PIPELINE_SUBPROCESS:
        sub_ptr = (Subprocess*) p;
        r = sub_ptr->pRead();
        if (!r) {
            cout << "--> EOF on subprocess" << endl;
            sub_ptr->GetNextPipeline()->SetReadyForDeletion();
            sub_ptr->GetNextPipeline()->SetState(STATE_DISCONNECTED);
            sub_ptr->SetReadyForDeletion();
            sub_ptr->SetState(STATE_DISCONNECTED);
        }
        //sub_ptr->Debug_Read();
        return r;
        break;
    default:
        cout << "+++ Unknown Pipeline_Type" << endl;
        break;
    }

    return -1;
}

int PerformWriteIO(Pipeline *p)
{
    NVT *nvt_ptr = NULL;
    Subprocess *sub_ptr = NULL;
    enum Pipeline_Type t;
    int w = 0;
    t = p->GetPipelineType();

    switch(t) {
    case Pipeline_Type::PIPELINE_RAW:
        p->pWrite();
        break;
    case Pipeline_Type::PIPELINE_NVT:
        nvt_ptr = (NVT*) p;
        nvt_ptr->UpdateConnectTime();
        //nvt_ptr->Debug_Write();
        w = nvt_ptr->pWrite();
        return w;
        break;
    case Pipeline_Type::PIPELINE_SUBPROCESS:
        sub_ptr = (Subprocess*) p;
        //sub_ptr->Debug_Write();
        w = sub_ptr->pWrite();
        return w;
        break;
    default:
        cout << "+++ Unknown Pipeline_Type" << endl;
        break;
    }

    return -1;
}



int RunIOSelectSet()
{
    std::vector<Pipeline*>::iterator iter, end, iter2;
//    struct sockaddr_in serv_addr, cli_addr;
    struct sockaddr_storage serv_addr, cli_addr;
    socklen_t clilen = 0;
    int newsockfd = -1;
    NVT *new_nvt = NULL;
    char *myargv[64];
    Subprocess *shell = NULL;
    pid_t child_process = 0;
    int r, w;
//    char ipstr[INET6_ADDRSTRLEN];
//    int port;

    int s = 0;
//    cout << "RunIOSelectSet()" << endl;
    s = select (FD_SETSIZE, &socketset, NULL, NULL, &timeout);
    switch (s) {
    case -1:
        /* I/O error */
        perror("select()");
        exit(1);
        break;
    case 0:
        /* timeout, no I/O - that is okay */
//        cout << "Timeout" << endl;
        usleep(20000);
        return 1;
        break;
    default:
        /* input received on fd */
        //cout << "Input was received via select()" << endl;
        for(iter = SelectablePipelines.begin(), end = SelectablePipelines.end() ; iter != end; iter++) {
            int r = (*iter)->GetRsockfd();
            if (FD_ISSET(r, &socketset)) {
                if (r == sockfd) {
                    cout << "[" << r << "] connection received" << endl;
                    clilen = sizeof cli_addr;
                    newsockfd = accept(r, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen);

                    /* FIXME: fill in client information address somewhere I suppose */
                    new_nvt = new NVT();
                    if (cli_addr.ss_family == AF_INET) {
                        struct sockaddr_in *s = (struct sockaddr_in *)&cli_addr;
                        new_nvt->port = ntohs(s->sin_port);
                        inet_ntop(AF_INET, &s->sin_addr, new_nvt->ipstr, sizeof(new_nvt->ipstr));
                    } else { // AF_INET6
                        printf("IPV6\n");
                        exit(1);
                        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&cli_addr;
                        new_nvt->port = ntohs(s->sin6_port);
                        inet_ntop(AF_INET6, &s->sin6_addr, new_nvt->ipstr, sizeof(new_nvt->ipstr));
                    }
                    //printf("+++ CLIENT IP address: %s:%u\n", new_nvt->ipstr, new_nvt->port);
                    // telnet connection is bi-direction TCP communication via a single socket
                    new_nvt->RegisterSocket(newsockfd, newsockfd);
                    if (!RegisterForIO((Pipeline*)(new_nvt))) {
                        cout << "Couldn't Register NVT for I/O" << endl;
                        exit(1);
                    };

                    if (!new_nvt->NegotiateOptions()) {
                        cout << "+++ Failed to negotation telnet options.\n";
                        new_nvt->SetState(STATE_DISCONNECTED);
                        break;
                    };

                    myargv[0] = (char *) "./mainmenu";
                    myargv[1] = NULL;
                    shell = new Subprocess();

                    /* start the child process */
                    strncpy((char *) &shell->ipstr, (char *) &new_nvt->ipstr, INET6_ADDRSTRLEN);
                    shell->port  = new_nvt->port;

                    child_process = shell->StartProcess(myargv[0], myargv);
                    //cout << "  child process pid is " << child_process << endl;
                    r = shell->GetPipeFD(PARENT_READ_PIPE, READ_FD);
                    w = shell->GetPipeFD(PARENT_WRITE_PIPE, WRITE_FD);
                    shell->RegisterSocket(r, w);
                    if (!RegisterForIO((Pipeline*)(shell))) {
                        cout << "Couldn't Register Subprocess for I/O" << endl;
                        exit(1);
                    };
                    new_nvt->SetNextPipeline(shell);
                    new_nvt->SetState(STATE_CONNECTED);
                    shell->SetNextPipeline(new_nvt);
                    shell->SetState(STATE_CONNECTED);

                } else {
                    //cout << "[" << r << "] input received" << endl;
                    PerformReadIO(*iter);
                    /* we have a connection, transfer the data */

                    Pipeline *s = (*iter);
                    Pipeline *d = (*iter)->GetNextPipeline();
                    if (!s->GetNextPipeline() || ! d->GetNextPipeline()) {
                        cout << "Unterminated circuit!" << endl;
                        exit(1);
                    }
                    int r = s->GetRbufsize();
                    int w = 0;
                    if (r) {
                        //cout << "Pipeline write! " << r << " bytes" << endl;
                        if (d->GetWbufsize() == 0) {
                            memcpy(d->GetWriteBuffer(), s->GetReadBuffer(),  r);
                            d->SetWbufsize(d->GetWbufsize() + r);
                            w = PerformWriteIO(d);
#ifdef DEBUG_TRANSFERIO
                            cout << "Transferred " << w << " bytes" << endl;
#endif /* DEBUG_TRANSFERIO */
                            s->SetRbufsize(0);
                        } else {
                            w = d->pWrite();
                            if (w <= 0) {
                                if (errno != EAGAIN) {
                                    cout << "+++ Error on write pipeline flush! w = " << w << endl;
                                    cout << "Error: " << strerror(errno) << endl;
                                    exit(1);
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
        break;
    }

    /* all okay */

    return 1;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = 0;
    int portno = -1;
    int newsockfd = -1;
    int i = 0;
    int flags = 0;
    pid_t child_process = 0;
    Subprocess *shell = NULL;
    char *myargv[64];
    int r =0, w = 0;
    int optval = 1 ;
    uid_t bootstrap_uid = 0;

    if (getuid() != 0) {
        cout << "Error: this program must be started as root.\n";
        exit(1);
    }

    bootstrap_uid = name_to_uid("bootstrap");

    if (bootstrap_uid == -1) {
        cout << "Error: couldn't get UID for bootstrap user\n";
        exit(1);
    }

    if (setuid(bootstrap_uid) != 0) {
        cout << "Error: getting bootstrap_uid ; " << strerror(errno);
        exit(1);
    }


    /* probably need some handlers for more signals ... */

    signal(SIGINT,int_handler);

    chdir("/usr/local/bbsd");


    cout << "BBSD build-id #" << BUILD_ID << endl;
    cout << "Initializing listener socket on port " << LISTEN_PORT << endl;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)) == -1) {
        perror("setsockopt") ;
        exit(-1) ;
    }

    if (sockfd < 0) {
        myerror("ERROR opening socket");
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = LISTEN_PORT;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0) {
        myerror("ERROR on binding");
    }

    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    cout << "There are " << SelectablePipelines.size() << " pipelines registered for IO" << endl;


    /* setup select() frequency */

    timeout.tv_sec =  0;
    timeout.tv_usec = 10;

    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    telnet_acceptor = new Pipeline();
    telnet_acceptor->RegisterSocket(sockfd, sockfd);

    if (!RegisterForIO(telnet_acceptor)) {
        cout << "++ Couldn't register telnet_acceptor for IO" << endl;
        exit(1);
    }


    while (!terminated) {

        /* unsure whether to do a seperate pass for an output set - for now I'll have the pipeline push the write if the write buffer is full*/

        if (!BuildIOSelectSet()) {
            cout << "+++ Error building IO select set" << endl;
            exit(1);
        }
        if (!RunIOSelectSet()) {
            cout << "RunIOSelectSet() failed" << endl;
            exit(1);
        }

    }

    cout << "Shutting down sockets ..." << endl;
    ShutdownIO();
    exit(0);

}


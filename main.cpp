#include <iostream>
#include <csignal>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "nvt.h"
#include "subprocess.h"

using std::cout;
using std::endl;
using std::vector;


static void myerror(const char *msg);

#define LISTEN_PORT     8023
#define MAX_PIPELINES     16

/* this can all go into a controller class when more mature */

Pipeline* telnet_acceptor = NULL;

vector<Pipeline*> SelectablePipelines;
fd_set socketset;
struct timeval timeout;
int sockfd = -1;         /* socket for TCP listener */
bool terminated = false;

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

int UnregisterForIO(Pipeline *p, bool do_shutdown)
{



}

int BuildIOSelectSet()
{

    std::vector<Pipeline*>::iterator iter, end, iter2;

//    cout << "BuildIOSelectSet()" << endl;
//    cout << "There are " << SelectablePipelines.size() << " pipelines registered for IO" << endl;

    FD_ZERO (&socketset);
    for(iter = SelectablePipelines.begin(), end = SelectablePipelines.end() ; iter != end; iter++) {
        if (!(*iter)->GetSelected()) {
            std::cout << "Adding fd " << (*iter)->GetRsockfd() << " to select set" << std::endl;
            (*iter)->SetSelected();
        }

        if ((*iter)->GetState() == STATE_DISCONNECTED && (*iter)->IsReadyForDeletion()) {
            cout << "BuildIOSelectSet(): found Pipeline for deletion" << endl;
            (*iter)->Shutdown();
            /* remove from the vector of open pipelines */
            for ( iter2 = SelectablePipelines.begin(); iter2 != SelectablePipelines.end(); )
                if(*iter2 == *iter) {
                    cout << "Removing pipeline from vector" << endl;
                    delete * iter2;
                    iter2 = SelectablePipelines.erase(iter2);
                }
                else {
                    ++iter2;
                }
        } else {
            FD_SET ((*iter)->GetRsockfd(), &socketset);
        }
    }

    return 1;

}

void ShutdownIO()
{
    std::vector<Pipeline*>::iterator iter, end, iter2;
    for(iter = SelectablePipelines.begin(), end = SelectablePipelines.end() ; iter != end; iter++) {
        (*iter)->Shutdown();
    }

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
        r =  nvt_ptr->pRead();
        if (!r) {
            cout << "--> EOF on NVT" << endl;
            nvt_ptr->GetNextPipeline()->SetReadyForDeletion();
            nvt_ptr->GetNextPipeline()->SetState(STATE_DISCONNECTED);
            nvt_ptr->SetReadyForDeletion();
            nvt_ptr->SetState(STATE_DISCONNECTED);
        }
        nvt_ptr->Debug_Read();
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
        sub_ptr->Debug_Read();
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
        w = nvt_ptr->pWrite();
        nvt_ptr->Debug_Write();
        return w;
        break;
    case Pipeline_Type::PIPELINE_SUBPROCESS:
        sub_ptr = (Subprocess*) p;
        w = sub_ptr->pWrite();
        sub_ptr->Debug_Write();
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
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = 0;
    int newsockfd = -1;
    NVT *new_nvt = NULL;
    char *myargv[64];
    Subprocess *shell = NULL;
    pid_t child_process = 0;
    int r, w;

    int s = 0;
//    cout << "RunIOSelectSet()" << endl;
    s = select (FD_SETSIZE, &socketset, NULL, NULL, &timeout);
    switch (s) {
    case 0:
        /* timeout, no I/O - that is okay */
//        cout << "Timeout" << endl;
        usleep(1000);
        return 1;
        break;
    case 1:
        /* input received on fd */
        //cout << "Input was received via select()" << endl;
        for(iter = SelectablePipelines.begin(), end = SelectablePipelines.end() ; iter != end; iter++) {
            int r = (*iter)->GetRsockfd();
            if (FD_ISSET(r, &socketset)) {
                if (r == sockfd) {
                    cout << "[" << r << "] connection received" << endl;
                    newsockfd = accept(r, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen);
                    /* FIXME: fill in client information address somewhere I suppose */
                    new_nvt = new NVT();
                    // telnet connection is bi-direction TCP communication via a single socket
                    new_nvt->RegisterSocket(newsockfd, newsockfd);
                    if (!RegisterForIO((Pipeline*)(new_nvt))) {
                        cout << "Couldn't Register NVT for I/O" << endl;
                        exit(1);
                    };


                    myargv[0] = (char *) "/bin/bash";
                    myargv[1] = (char *) "-i";
                    myargv[2] = NULL;
                    shell = new Subprocess();
                    child_process = shell->StartProcess(myargv[0], myargv);
                    cout << "child process pid is " << child_process << endl;
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
                    cout << "[" << r << "] input received" << endl;
                    PerformReadIO(*iter);
                    if ((*iter)->GetState() == STATE_DISCONNECTED && (*iter)->IsReadyForDeletion()) {
                        cout << "Pipeline is unconnected, and marked for deletion, closing" << endl;
                        (*iter)->Shutdown();
                        /* remove from the vector of open pipelines */
                        for ( iter2 = SelectablePipelines.begin(); iter2 != SelectablePipelines.end(); )
                            if(*iter2 == *iter) {
                                cout << "Removing pipeline from vector" << endl;
                                delete * iter2;
                                iter2 = SelectablePipelines.erase(iter2);
                            }
                            else {
                                ++iter2;
                            }
                    } else {
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
                            cout << "Pipeline write! " << r << " bytes" << endl;
                            memcpy(d->GetWriteBuffer(), s->GetReadBuffer(),  r);
                            d->SetWbufsize(d->GetWbufsize() + r);
                            w = PerformWriteIO(d);
                            cout << "Transferred " << w << " bytes" << endl;
                            s->SetRbufsize(0);
                        }
                    }
                }
                break;
            }
        }
        break;
    case -1:
        /* I/O error */
        perror("select()");
        exit(1);
        break;
    default:
        cout << "Unexpected return code " << s << " received from select()" << endl;
        exit(1);
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

    signal(SIGINT,int_handler);

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


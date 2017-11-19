#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "nvt.h"

using std::cout;
using std::endl;
using std::vector;


static void myerror(const char *msg);

#define LISTEN_PORT     8023
#define MAX_PIPELINES     16

/* this can all go into a controller class when more mature */

Pipeline* telnet_acceptor = NULL;
Pipeline* IOPipelines[MAX_PIPELINES];

vector<Pipeline*> SelectablePipelines;
fd_set socketset;
struct timeval timeout;
int sockfd = -1;         /* socket for TCP listener */

static void myerror(const char *msg)
{
    perror(msg);
    exit(1);
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

int BuildIOSelectSet()
{

    std::vector<Pipeline*>::iterator iter, end, iter2;

//    cout << "BuildIOSelectSet()" << endl;
//    cout << "There are " << SelectablePipelines.size() << " pipelines registered for IO" << endl;

    FD_ZERO (&socketset);
    for(iter = SelectablePipelines.begin(), end = SelectablePipelines.end() ; iter != end; iter++) {
        //std::cout << "Adding fd " << (*iter)->GetRsockfd() << " to select set" << std::endl;
        FD_SET ((*iter)->GetRsockfd(), &socketset);
    }

    return 1;

}


int PerformReadIO(Pipeline *p)
{
    NVT *nvt_ptr = NULL;
    enum Pipeline_Type t;
    t = p->GetPipelineType();

    switch(t) {
    case Pipeline_Type::PIPELINE_RAW:
        p->pRead();
        break;
    case Pipeline_Type::PIPELINE_NVT:
        nvt_ptr = (NVT*) p;
        nvt_ptr->pRead();
        break;
    case Pipeline_Type::PIPELINE_SUBPROCESS:
        cout << "+++ Subprocess read: no handler" << endl;
        exit(1);
        break;
    default:
        cout << "+++ Unknown Pipeline_Type" << endl;
        break;
    }

    return 1;
}


int RunIOSelectSet()
{
    std::vector<Pipeline*>::iterator iter, end, iter2;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = 0;
    int newsockfd = -1;
    NVT *new_nvt = NULL;

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
                        cout << "Couldn't Register NVT for I/O\n" << endl;
                    };
                } else {
                    cout << "[" << r << "] input received" << endl;
                    PerformReadIO(*iter);
                    if ((*iter)->GetState() == STATE_DISCONNECTED) {
                        cout << "Pipeline is unconnected, closing" << endl;
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

    /* initalize pipeline table */

    for (i = 0; i < MAX_PIPELINES; i++) {
        IOPipelines[i] = NULL;
    }

    cout << "Initializing listener socket on port " << LISTEN_PORT << endl;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

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

    while (1) {
        if (!BuildIOSelectSet()) {
            cout << "+++ Error building IO select set" << endl;
            exit(1);
        }
        if (!RunIOSelectSet()) {
            cout << "RunIOSelectSet() failed" << endl;
            exit(1);
        }
    }

    exit(0);

}

#include <iostream>
#include <fcntl.h>
#include <sys/wait.h>
#include "subprocess.h"

using std::cout;
using std::endl;

//#define WRITE_FD 1

/*
#define PARENT_READ_FD   pipes[PARENT_READ_PIPE][READ_FD]   
#define PARENT_WRITE_FD  pipes[PARENT_WRITE_PIPE][WRITE_FD] 
#define CHILD_READ_FD    pipes[PARENT_WRITE_PIPE][READ_FD]  
#define CHILD_WRITE_FD   pipes[PARENT_READ_PIPE][WRITE_FD]  
*/

Subprocess::Subprocess()
{
//    cout << "Subprocess created" << endl;
}

Subprocess::~Subprocess()
{
 //   cout << "Subprocess destroyed" << endl;
}

int Subprocess::GetPipeFD(int pair, int channel)
{
    return pipes[pair][channel];
}

int Subprocess::RegisterSocket(int r, int w)
{
    int flags = 0;
//    cout << "Subprocess::RegisterSocket" << "(read=" << r << ", write=" << w << ")" << endl;
    /* for an Subprocess we use non-blocking I/O, since telnet protocol can be a little adhoc */
    flags = fcntl(r, F_GETFL, 0);
    fcntl(r, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(w, F_GETFL, 0);
    fcntl(w, F_SETFL, flags | O_NONBLOCK);
//    cout << "Subprocess set sockets to non-blocking mode" << endl;
    SetPipelineType(Pipeline_Type::PIPELINE_SUBPROCESS);
    return Pipeline::RegisterSocket(r, w);
}

int Subprocess::pRead()
{
    int r = 0;
//    cout << "Subprocess::pRead()" << endl;
    r = Pipeline::pRead();
    return r;
}

int Subprocess::pWrite()
{
    int w = 0;
//    cout << "Subprocess::pWrite()" << endl;
    //Debug_Write();
    w = Pipeline::pWrite();
    return w;
}

pid_t Subprocess::StartProcess(const char *path, char **argv)
{
    int flags = 0;
    int p = 0;
    int i = 0, j = 0;

//    cout << "StartProcess(" << path << ")" << endl;
    p = pipe(pipes[PARENT_READ_PIPE]);
//    printf("Create parent read pipe = %d\n", p);
    p = pipe(pipes[PARENT_WRITE_PIPE]);
//    printf("Create parent write pipe = %d\n", p);
//

    child_pid = fork();
    if(!child_pid) {
        /* we are within the child process here */

        p = dup2(CHILD_READ_FD, STDIN_FILENO);      
        //cout << "child: read_fd = "  << p << endl;
        p = dup2(CHILD_WRITE_FD, STDOUT_FILENO);
        //cout << "child: write_fd = "  << p << endl;
        /* Close fds not required by child. Also, we don't
           want the exec'ed program to know these existed */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);

				/* close stderr  - we may want to do something else with this later */
        close(2);

        /* wait until connect_time >= 5 */

        if (execv(argv[0], argv) == -1) {
            cout << "Couldn't start subprocess" << endl;
            perror("execv");
            GetNextPipeline()->SetState(STATE_DISCONNECTED);
            GetNextPipeline()->SetReadyForDeletion();
            SetState(STATE_DISCONNECTED);
            SetReadyForDeletion();
            exit(255);
            };
        /* never return */
    } else {
        /* we are in the parent here */
        /* close fds not required by parent */

        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        flags = fcntl(PARENT_READ_FD, F_GETFL, 0);
        fcntl(PARENT_READ_FD, F_SETFL, flags | O_NONBLOCK);
    }
//    cout << "+PARENT_WRITE_FD:  " << PARENT_WRITE_FD << endl;
//    cout << "+PARENT_READ_FD:   " << PARENT_READ_FD << endl;
//    for (i = 0; i < 2; i++) {
//        for (j = 0; j < 2; j++) {
 //           printf("pipes[%d][%d]=%d\n", i, j, pipes[i][j]);
 //           }
  //      }

    return child_pid;
}


void Subprocess::Shutdown()
{
    pid_t c = 0;
    int wstatus;
    int options = WNOHANG; 
    cout << "Subprocess:Shutdown()" << endl;
    Pipeline::Shutdown();
    cout << "Reaping child pid " << child_pid;
    c =  waitpid(child_pid, &wstatus, options);  
    cout << "Reap status: " << c << endl;
    return;
}

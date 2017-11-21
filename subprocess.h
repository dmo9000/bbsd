#pragma once
#include <unistd.h>
#include <sys/types.h>
#include "pipeline.h"

#define NUM_PIPES          2

#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1

/* always in a pipe[], pipe[0] is for read and
   pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1

/*
#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )
#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )
*/


class Subprocess : public Pipeline  
{

protected:

private:
    int pipes[NUM_PIPES][2];
    pid_t child_pid = -1;

public:
    Subprocess();
    ~Subprocess();
	//pid_t StartProcess(int *pipes, const char *path, char *const *const argv);	
    pid_t StartProcess(const char *path, char **argv);
    int RegisterSocket(int r, int w);
    int pRead();
    int pWrite();
    int GetPipeFD(int pair, int channel);
    void Shutdown();
    
};

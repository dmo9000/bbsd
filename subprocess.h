#pragma once
#include <sys/types.h>
#include "pipeline.h"


class Subprocess : public Pipeline  
{

protected:

private:
		int pipes[2];

public:
    Subprocess();
    ~Subprocess();
		pid_t StartProcess(int *pipes, const char *path, char *const *const argv);	
    int RegisterSocket(int r, int w);
    int pRead();
    int pWrite();
};

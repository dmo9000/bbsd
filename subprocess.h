#pragma once
#include "pipeline.h"


class Subprocess : public Pipeline  
{

protected:

private:

public:
    Subprocess();
    ~Subprocess();
    int RegisterSocket(int r, int w);
    int pRead();
    int pWrite();
};

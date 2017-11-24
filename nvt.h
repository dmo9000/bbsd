#pragma once
#include <string.h>
#include "pipeline.h"


class NVT : public Pipeline  
{

protected:

private:

public:
    NVT();
    ~NVT();
    int RegisterSocket(int r, int w);
    int pRead();
    int pWrite();
		int LineDiscipline();
    void Shutdown();
};

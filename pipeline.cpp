#include <iostream>
#include "nvt.h"
using std::cout;
using std::endl;


Pipeline::Pipeline()
{

    cout << "Pipeline created" << endl;
    pType = Pipeline_Type::PIPELINE_RAW;

}

Pipeline::~Pipeline()
{
    cout << "Pipeline destroyed" << endl;

}

Pipeline *Pipeline::GetNextPipeline()
{
    return next_pipeline;
}


Pipeline_Type Pipeline::GetPipelineType()
{
    return pType;
}


int Pipeline::SetPipelineType(Pipeline_Type a)
{
    pType = a;
}

int Pipeline::RegisterSocket(int r, int w)
{
    cout << "Pipeline::RegisterSocket" << "(read=" << r << ", write=" << w << ")" << endl;

    if (r > 0) {
        rsock = r;
    } else {
        return 0;
    }

    if (w > 0) {
        wsock = w;
    } else {
        return 0;
    }

    /* okay */
    return 1;
}

int Pipeline::GetRsockfd()
{
    return rsock;
}

int Pipeline::pRead()
{
    int r = 0;
    cout << "Pipeline::pRead()" << endl;
    r = read(rsock, &rbuf, BUFSIZE - rsize); 
    cout << "read " << r << " bytes" << endl;
    return r;
}

int Pipeline::pWrite()
{
    cout << "Pipeline::pWrite()" << endl;
    return 0;
}

Pipeline_State Pipeline::GetState()
{
    return pState;
}

int Pipeline::SetState(Pipeline_State s)
{
    pState = s;
    return 1;
}

int Pipeline::Shutdown()
{
    close(rsock);
    if (rsock != wsock) {
        /* also close write socket in case of unidirectional fd's */
        close(wsock);
        }
    return 1;
}

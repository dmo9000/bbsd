#include <iostream>
#include <fcntl.h>
#include "nvt.h"
using std::cout;
using std::endl;


NVT::NVT()
{
    cout << "NVT created" << endl;

}

NVT::~NVT()
{
    cout << "NVT destroyed" << endl;
}

int NVT::RegisterSocket(int r, int w)
{
    int flags = 0;
    cout << "NVT::RegisterSocket" << "(read=" << r << ", write=" << w << ")" << endl;

    /* for an NVT we use non-blocking I/O, since telnet protocol can be a little adhoc */

    flags = fcntl(r, F_GETFL, 0);
    fcntl(r, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(w, F_GETFL, 0);
    fcntl(w, F_SETFL, flags | O_NONBLOCK);
    cout << "NVT set sockets to non-blocking mode" << endl;

    SetPipelineType(Pipeline_Type::PIPELINE_NVT);

    return Pipeline::RegisterSocket(r, w);
}

int NVT::pRead()
{
    int r = 0;
    cout << "NVT::pRead()" << endl;
    if (!GetNextPipeline()) {
        /* there is data available, but we have nowhere to send it */
        SetState(STATE_DISCONNECTED); 
        }

    r = Pipeline::pRead();
    
    return r;
}

int NVT::pWrite()
{
    int w = 0;
    cout << "NVT::pWrite()" << endl;
    w = Pipeline::pWrite();
    return w;
}

void NVT::Shutdown()
{
    cout << "NVT::Shutdown()" << endl;
    Pipeline::Shutdown();
    return;

}



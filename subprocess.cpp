#include <iostream>
#include <fcntl.h>
#include "subprocess.h"
using std::cout;
using std::endl;


Subprocess::Subprocess()
{
    cout << "Subprocess created" << endl;

}

Subprocess::~Subprocess()
{
    cout << "Subprocess destroyed" << endl;
}

int Subprocess::RegisterSocket(int r, int w)
{
    int flags = 0;
    cout << "Subprocess::RegisterSocket" << "(read=" << r << ", write=" << w << ")" << endl;

    /* for an Subprocess we use non-blocking I/O, since telnet protocol can be a little adhoc */

    flags = fcntl(r, F_GETFL, 0);
    fcntl(r, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(w, F_GETFL, 0);
    fcntl(w, F_SETFL, flags | O_NONBLOCK);
    cout << "Subprocess set sockets to non-blocking mode" << endl;

    SetPipelineType(Pipeline_Type::PIPELINE_SUBPROCESS);

    return Pipeline::RegisterSocket(r, w);
}

int Subprocess::pRead()
{
    int r = 0;
    cout << "Subprocess::pRead()" << endl;
    r = Pipeline::pRead();
    return r;
}

int Subprocess::pWrite()
{
    int w = 0;
    cout << "Subprocess::pWrite()" << endl;
    w = Pipeline::pWrite();
    return w;
}

pid_t Subprocess::StartProcess(int *pipes, const char *path, char *const *const argv)
{
	cout << "StartProcess(%s)" << path << endl;	

}




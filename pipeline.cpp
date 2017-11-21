#include <iostream>
#include <sys/socket.h>
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

int Pipeline::SetNextPipeline(Pipeline *p)
{
    next_pipeline = p;
    return 1;
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
    cout << "Pipeline::pRead(" << rsock << ")" << endl;
    r = read(rsock, &rbuf, BUFSIZE - rsize); 
    cout << "read " << r << " bytes" << endl;

    
    if (r == 0) {
            cout << "+++EOF on socket " << rsock << endl;
            }

    rsize += r;
    return r;
}

void Pipeline::Debug_Read()
{
    int i = 0;
    char buffer[17];
    int b = 0;
    //int j = (rsize / 16) + ((rsize % 16) ? 16 : 0);
    int j = 256; 

    for (i = 0; i < j ; i++) {
        printf("%02x ", rbuf[i]);
        buffer[b] = (rbuf[i] >= 32 && rbuf[i] <=127 ? rbuf[i] : '.');
        buffer[16] = '\0';
        b++;
        if (b == 16) {
            printf(" | %s\n", buffer);
            b = 0;
            }
        }

    printf("\n");

}

void Pipeline::Debug_Write()
{
    int i = 0;
    char buffer[17];
    int b = 0;
    //int j = (wsize / 16) + ((wsize % 16) ? 16 : 0);
    int j = 256; 
    
    for (i = 0; i < j ; i++) {
        printf("%02x ", wbuf[i]);
        buffer[b] = (wbuf[i] >= 32 && wbuf[i] <=127 ? wbuf[i] : '.');
        buffer[16] = '\0'; 
        b++;
        if (b == 16) {
            printf(" | %s\n", buffer);
            b = 0;
            }
        }

    printf("\n");
}

int Pipeline::pWrite()
{
    int w = 0;
    cout << "Pipeline::pWrite(" << wsock << ")" << endl;
    cout << "wsize = " << wsize << endl;
    cout << "wsock = " << wsock << endl;
    w = write(wsock, &wbuf, wsize);
    cout << "wrote " << w << " bytes" << endl;
    if (w != wsize) {
        cout << "Short write!" << endl;
        exit(1);
        }
    wsize -= w;
    return w;
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
    shutdown (rsock, SHUT_RDWR);
    close(rsock);
    if (rsock != wsock) {
        /* also close write socket in case of unidirectional fd's */
        shutdown (rsock, SHUT_RDWR);
        close(wsock);
        }
    return 1;
}

int Pipeline::GetRbufsize()
{
    return rsize;
}

int Pipeline::GetWbufsize()
{
    return wsize;
}

int Pipeline::SetRbufsize(uint16_t s)
{
    rsize = s;
}

int Pipeline::SetWbufsize(uint16_t s)
{
    wsize = s;
}



uint8_t *Pipeline::GetReadBuffer()
{
    return (uint8_t *) &rbuf;
}

uint8_t *Pipeline::GetWriteBuffer()
{
    return (uint8_t*) &wbuf;
}

bool Pipeline::GetSelected()
{
    return selected;
}

void Pipeline::SetSelected()
{
    selected = true;
}


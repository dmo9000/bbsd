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
    uint8_t nvt_wbuf[BUFSIZE];
    int w = -1;
    int wr = 0;
    int i = 0;
    uint32_t out_idx = 0;
    uint16_t remain = GetWbufsize();
    uint8_t *ptr = GetWriteBuffer();

    cout << "NVT::pWrite()" << endl;


    if (!line_discipline) {
        /* RAW path */
        return Pipeline::pWrite();
        }

    w = GetWsockfd();
    memset((char *) &nvt_wbuf, 0, BUFSIZE);

    for (i = 0; i < remain; i++) {
        switch (ptr[i]) {
        case 0xff:
            nvt_wbuf[out_idx] = 0xff;
            nvt_wbuf[out_idx+1] = 0xff;
            out_idx+=2;
            break;
        case '\n':
            nvt_wbuf[out_idx] = '\r';
            nvt_wbuf[out_idx+1] = '\n';
            out_idx+=2;
            break;
        default:
            nvt_wbuf[out_idx] = ptr[i];
            out_idx++;
            break;
        }
    }

    /* copy the altered buffer back to the output buffer, and write it out */

    if (out_idx > 0x0000ffff) {
        cout << "+++ NVT: output buffer is too large\n";
        exit(1);
    }

    if (out_idx > GetWbufsize()) {
        cout << "(NVT expanded buffer by " << (out_idx - GetWbufsize()) << " bytes)\n";
    }

    memcpy(ptr, &nvt_wbuf, out_idx);
    SetWbufsize(out_idx);
    wr = Pipeline::pWrite();
    return wr;
}

void NVT::Shutdown()
{
    cout << "NVT::Shutdown()" << endl;
    Pipeline::Shutdown();
    return;

}

int NVT::LineDiscipline()
{
    cout << "NVT::LineDiscipline()" << endl;
    int lfcount = 0;
    char *ptr = (char *) GetWriteBuffer();
    char *endptr = (char *) &wbuf;
    uint16_t buflen = 0;
    endptr += 65536;
    buflen = endptr - ptr;

    cout << "++ LF check: " << GetWbufsize() << endl;
    Debug_Write();
    ptr = memchr((const void *) ptr, '\n', (size_t) buflen);
    if (!ptr) {
        cout << "++ No linefeeds in stream" << endl;
        return 1;
    }

    while (ptr < endptr) {
        ptr = memchr((const void *) ptr, '\n', (size_t) buflen);
        if (ptr) {
            ptr++;
            lfcount++;
        }
    }

    cout << "++ " << lfcount << " linefeeds were found" << endl;
    return 1;
}

#include <iostream>
#include <fcntl.h>
#include <time.h>
#include "nvt.h"
using std::cout;
using std::endl;


NVT::NVT()
{
    cout << "NVT created" << endl;
    SetStartTime(time(NULL));

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
    uint8_t nvt_rbuf[BUFSIZE];
    int r = 0;
    uint8_t *ptr = NULL;
    uint16_t rsize = 0;
    uint16_t i = 0;
    uint16_t o = 0;
    int optsize = 0;

    ptr = GetReadBuffer();
    cout << "NVT::pRead()" << endl;
    if (!GetNextPipeline()) {
        /* there is data available, but we have nowhere to send it */
        SetState(STATE_DISCONNECTED);
        return 0;
    }

    r = Pipeline::pRead();

    rsize = GetRbufsize();

    /* scan for IAC */

    for (i = 0 ; i < rsize ; i++) {
        ptr = GetReadBuffer() + i;
        switch (ptr[0]) {
        case IAC:
            cout << "+++ IAC received in stream at offset " << ((uint8_t*) ptr - GetReadBuffer()) << endl;
            //Debug_Read();
            optsize = IAC_Process(ptr);
            if (!optsize) {
                cout << "+++ Error; option processing return size 0\n";
                exit(1);
                }
            cout << "option size: " << optsize << endl;
            i += (optsize - 1); /* we subtract one to line up with the next byte in the stream */
            cout << "i now equals: " << i << endl;
            break;
        default:
            nvt_rbuf[o] = ptr[0];
            o++;
            break;
        }
    }

    if (rsize > o) {
        cout << "+++ options processing reduced buffer from " << rsize << " to " << o << endl;
        }

    memset(GetReadBuffer(), 0, BUFSIZE);
    memcpy(GetReadBuffer(), &nvt_rbuf, o);
    SetRbufsize(o);
    Debug_Read();

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

int NVT::NegotiateOptions()
{
    int w = -1;
    uint8_t options_out[64];
    uint8_t *ptr = (uint8_t*) &options_out;
    uint16_t write_size = 0;
    cout << "NVT::NegotiateOptions()" << endl;

    w = GetWsockfd();

    memset(&options_out, 0, 64);
    snprintf((char *) ptr, 64, "%c%c%c", IAC, IAC_DO, TERMINALTYPE);
    ptr +=3;
    snprintf((char *) ptr, 64, "%c%c%c", IAC, IAC_DO, TSPEED);
    ptr +=3;
    snprintf((char *) ptr, 64, "%c%c%c", IAC, IAC_DO, XDISPLAYLOCATION);
    ptr +=3;
    snprintf((char *) ptr, 64, "%c%c%c", IAC, IAC_DO, NEWENVIRON);
    ptr +=3;


    /* set wsize */
    SetWbufsize(ptr - ((uint8_t*) &options_out));
    write_size = GetWbufsize();
    memcpy(GetWriteBuffer(), &options_out, write_size);
    if (Pipeline::pWrite() != write_size) {
        cout << "+++ couldn't write telnet options set 1\n";
        return 0;
    }

    /* now we want to wait until we have the responses */

    return (1);

}

void NVT::SetStartTime(time_t t)
{
    start_time = t;
}

void NVT::UpdateConnectTime()
{
    connect_time = time(NULL) - start_time;

}

time_t NVT::GetConnectTime()
{
    return connect_time;
}


int NVT::IAC_Process(uint8_t *buf)
{
    int l = 0;
    int rc = 0;

    cout << "NVT::IAC_Process()\n";
    if (buf[0] != IAC) {
        cout << "+++ IAC_Process: error!\n";
        exit(1);
    }

    l++;

    switch(buf[1]) {
    case IAC_WILL:
        rc =  IAC_Will(buf[2]);
        break;
    case IAC_WONT:
        rc =  IAC_Wont(buf[2]);
        break;
    case IAC_DO:
        rc = IAC_Do(buf[2]);
        break;
    case IAC_DONT:
        rc = IAC_Dont(buf[2]);
        break;
    default:
        cout << "+++ Unknown telnet option command!\n";
        exit(1);
    }

    if (!rc) {
        cout  << "++ error in option processing\n";
        exit(1);
    } else {
        l+=rc;
    }

    return l;
}

int NVT::IAC_Will(uint8_t opt)
{
    printf("NVT::IAC_Will(0x%02x)\n", opt);
    switch(opt) {
    case TERMINALTYPE:
        client_will_terminal_type = true;
        cout << ">RCVD WILL TERMINAL TYPE\n";
        return 2;
        break;
    case TSPEED:
        client_will_tspeed = true;
        cout << ">RCVD WILL TSPEED\n";
        return 2;
        break;
    case XDISPLAYLOCATION:
        client_will_xdisplaylocation = true;
        cout << ">RCVD WILL XDISPLAYLOCATION\n";
        return 2;
        break;
    case NEWENVIRON:
        client_will_newenviron = true;
        cout << ">RCVD WILL NEWENVIRON\n";
        return 2;
    default:
        cout << "++ Unhandled\n";
        exit(1);
        break;
    }
    return (0);
}


int NVT::IAC_Wont(uint8_t opt)
{
    printf("NVT::IAC_Wont(0x%02x)\n", opt);
    switch(opt) {
    case TERMINALTYPE:
        client_will_terminal_type = false;
        cout << ">RCVD WONT TERMINAL TYPE\n";
        return 2;
        break;
    case TSPEED:
        client_will_tspeed = false;
        cout << ">RCVD WONT TSPEED\n";
        return 2;
        break;
    case XDISPLAYLOCATION:
        client_will_xdisplaylocation = false;
        cout << ">RCVD WONT XDISPLAYLOCATION\n";
        return 2;
        break;
    case NEWENVIRON:
        client_will_newenviron = false;
        cout << ">RCVD WONT NEWENVIRON\n";
        return 2;
    default: 
        cout << "++ Unhandled\n";
        exit(1);
        break;
    }
    return (0);

}

int NVT::IAC_Do(uint8_t opt)
{
    printf("NVT::IAC_Do(0x%02x)\n", opt);
    exit(1);
}

int NVT::IAC_Dont(uint8_t opt)
{
    printf("NVT::IAC_Dont(0x%02x)\n", opt);
    exit(1);

}



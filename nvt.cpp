#include <iostream>
#include <fcntl.h>
#include <time.h>
#include "nvt.h"
using std::cout;
using std::endl;

//#define DEBUG_LIFECYCLE
//#define DEBUG_TELNETOPTIONS
//#define DEBUG_IO


NVT::NVT()
{
#ifdef DEBUG_LIFECYCLE
    cout << "NVT created" << endl;
#endif /* DEBUG_LIFECYCLE */
    SetStartTime(time(NULL));
}

NVT::~NVT()
{
#ifdef DEBUG_LIFECYCLE
    cout << "NVT destroyed" << endl;
#endif /* DEBUG_LIFECYCLE */
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
    //cout << "NVT set sockets to non-blocking mode" << endl;

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
#ifdef DEBUG_IO
    cout << "NVT::pRead()" << endl;
#endif /* DEBUG_IO */
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
#ifdef DEBUG_TELNETOPTIONS
            cout << "+++ IAC received in stream at offset " << ((uint8_t*) ptr - GetReadBuffer()) << endl;
#endif /* DEBUG_TELNETOPTIONS */
            //Debug_Read();
            optsize = IAC_Process(ptr);
            if (!optsize) {
                cout << "+++ Error; option processing return size 0\n";
                exit(1);
            }
#ifdef DEBUG_TELNETOPTIONS
            cout << "option size: " << optsize << endl;
#endif /* DEBUG_TELNETOPTIONS */

            i += (optsize - 1);
            /* we subtract one to line up with the next byte in the stream */


#ifdef DEBUG_TELNETOPTIONS
            cout << "i now equals: " << i << endl;
#endif /* DEBUG_TELNETOPTIONS */
            break;
        default:
            nvt_rbuf[o] = ptr[0];
            o++;
            break;
        }
    }

#ifdef DEBUG_TELNETOPTIONS
    if (rsize > o) {
        cout << "+++ options processing reduced buffer from " << rsize << " to " << o << endl;
    }
#endif /* DEBUG_TELNETOPTIONS */

    memset(GetReadBuffer(), 0, BUFSIZE);
    memcpy(GetReadBuffer(), &nvt_rbuf, o);
    SetRbufsize(o);
    //Debug_Read();

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

#ifdef DEBUG_IO
    cout << "NVT::pWrite()" << endl;
#endif /* DEBUG_IO */

    /*
            DEPRECATED! DON'T USE!

    */
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

            /*
            When the Binary option has been successfully negotiated, arbitrary 8-bit characters are allowed. However, the data stream MUST still be
            scanned for IAC characters, any embedded Telnet commands MUST be obeyed, and data bytes equal to IAC MUST be doubled. Other character processing
             (e.g., replacing CR by CR NUL or by CR LF) MUST NOT be done. In particular, there is no end-of-line convention (see Section 3.3.1) in binary mode. */

            if (!server_do_binary) {
                /* manage lines - FIXME: should check for just CR here as well as LF. Check what should happens in the case of a lone CR */
                nvt_wbuf[out_idx] = '\r';
                nvt_wbuf[out_idx+1] = '\n';
                out_idx+=2;
            } else {
                /* pass through */
                nvt_wbuf[out_idx] = ptr[i];
                out_idx++;
            }
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
    cout << "+++ NVT::Shutdown()" << endl;
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

#ifdef DEBUG_TELNETOPTIONS
    cout << "NVT::IAC_Process()\n";
#endif /* DEBUG_TELNETOPTIONS */
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
        printf("+++ Unknown or malformed telnet option command! -> %u (0x%02x), ignoring \n", buf[2], buf[2]);
        rc = 1;
        l++;
        break;
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
#ifdef DEBUG_TELNETOPTIONS
    printf("NVT::IAC_Will(0x%02x)\n", opt);
#endif /* DEBUG_TELNETOPTIONS */

    switch(opt) {
    case BINARY:
        client_will_binary = true;
        cout << ">RCVD WILL BINARY";
        return 2;
        break;
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
        break;
    case SUPPRESSGOAHEAD:
        client_will_suppressgoahead=true;
        cout << ">RCVD WILL SUPPRESSGOAHEAD\n";
        return 2;
        break;
    default:
        /* send IAC_DONT for anything not explicitly supported */
        printf("+++ SENDING IAC_DONT(0x%02x) [UNSUPPORTED]\n", opt);
        IAC_Dont(opt);
        return 2;
        printf("+++ UNHANDLED NVT::IAC_Will(0x%02x)\n", opt);
        exit(1);
        break;
    }
    return (0);
}


int NVT::IAC_Wont(uint8_t opt)
{
#ifdef DEBUG_TELNETOPTIONS
    printf("NVT::IAC_Wont(0x%02x)\n", opt);
#endif /* DEBUG_TELNETOPTIONS */
    switch(opt) {
    case BINARY:
        client_will_binary = false;
        cout << ">RCVD WONT BINARY\n";
        return 2;
        break;
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
        printf("+++ RCVD IAC_WONT(0x%02x)\n", opt);
        return 2;
        break;
        printf("+++ UNHANDLED NVT::IAC_Wont(0x%02x)\n", opt);
        exit(1);
        break;
    }
    return (0);

}

int NVT::IAC_Do(uint8_t opt)
{
#ifdef DEBUG_TELNETOPTIONS
    printf("NVT::IAC_Do(0x%02x)\n", opt);
#endif /* DEBUG_TELNETOPTIONS */

    switch(opt) {

    case BINARY:
        server_do_binary = true;
        cout << ">RCVD DO BINARY\n";
        return 2;
        break;
    /*
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
    */
    case WINDOWSIZE:
        cout << ">RCVD DO WINDOWSIZE\n";
        IAC_Wont(WINDOWSIZE);
        return 2;
        break;
    case SUPPRESSGOAHEAD:
        server_will_suppressgoahead = true;
        cout << ">RCVD DO SUPPRESS GOAHEAD\n";
        IAC_Will(SUPPRESSGOAHEAD);
        return 2;
        break;
    default:
        printf("+++ SENDING IAC_DONT(0x%02x) [UNSUPPORTED]\n", opt);
        IAC_Wont(opt);
        return 2;
        break;
        printf("+++ UNHANDLED NVT::IAC_Do(0x%02x)\n", opt);
        exit(1);
        break;
    }
    return (0);

}

int NVT::IAC_Dont(uint8_t opt)
{
#ifdef DEBUG_TELNETOPTIONS
    printf("NVT::IAC_Dont(0x%02x)\n", opt);
#endif /* DEBUG_TELNETOPTIONS */
    switch(opt) {
    case BINARY:
        server_do_binary = false;
        cout << ">RCVD DONT BINARY\n";
        return 2;
        break;
    /*
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
    */
    default:
        printf("+++ SENDING IAC_WONT(0x%02x) [UNSUPPORTED]\n", opt);
        IAC_Wont(opt);
        return 2;
        break;
        printf("+++ UNHANDLED NVT::IAC_Dont(0x%02x)\n", opt);
        exit(1);
        break;
    }
    return (0);

}



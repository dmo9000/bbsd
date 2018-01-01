#pragma once
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pipeline.h"

#define IAC_SE               0xF0
#define IAC_SB               0xFA
#define IAC_WILL             0xFB
#define IAC_WONT             0xFC
#define IAC_DO               0xFD
#define IAC_DONT             0xFE
#define IAC                  0xFF

#define BINARY              0
#define ECHO                1
#define SUPPRESSGOAHEAD     3
#define STATUS              5           /* RFC859 */
#define LOCATION            23
#define TERMINALTYPE        0x18        /* RFC1091 & RFC1010 */
#define WINDOWSIZE          31          /* RFC1073 */
#define TSPEED              0x20        /* RFC1079 */
#define TOGGLEFLOWCONTROL   33          /* RFC1372 */
#define LINEMODE            34          /* RFC1184 */
#define XDISPLAYLOCATION    0x23        /* RFC1096 */
#define NEWENVIRON          0x27        /* RFC1572 */
#define CHARSET             42          /* RFC2066 */


class NVT : public Pipeline  
{

protected:

private:
    bool line_discipline = true;
    time_t start_time = 0;
    time_t connect_time = 0;
    bool client_will_terminal_type = false;
    bool client_will_tspeed = false;
    bool client_will_xdisplaylocation = false;
    bool client_will_newenviron = false;
    bool client_will_binary = false;
    bool server_do_binary = false;
    bool option_neg_pass_1 = false;

public:

    struct sockaddr_in peer;
    int peer_len;

    NVT();
    ~NVT();
    int RegisterSocket(int r, int w);
    int pRead();
    int pWrite();
	int LineDiscipline();
    int IAC_Decode();
    void Shutdown();
    int NegotiateOptions();
    void SetStartTime(time_t t);
    void UpdateConnectTime();
    time_t GetConnectTime();
    int IAC_Process(uint8_t *buf);
    int IAC_Will(uint8_t);
    int IAC_Wont(uint8_t);
    int IAC_Do(uint8_t);
    int IAC_Dont(uint8_t);
};

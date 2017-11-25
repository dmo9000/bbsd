#pragma once

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define BUFSIZE 65536

enum Pipeline_Type
   {
        PIPELINE_RAW,
        PIPELINE_NVT,
        PIPELINE_SUBPROCESS
   };

enum Pipeline_State
    {
        STATE_DISCONNECTED,
        STATE_CONNECTED
    };


class Pipeline  
{

protected:
    uint8_t rbuf[BUFSIZE];
    uint8_t wbuf[BUFSIZE];

private:
    Pipeline* next_pipeline = NULL;
    Pipeline_Type pType = Pipeline_Type::PIPELINE_RAW;
    Pipeline_State pState = Pipeline_State::STATE_DISCONNECTED;
    int rsock = -1;
    int wsock = -1;
    uint16_t rsize = 0;
    uint16_t wsize = 0;
//    uint8_t rbuf[BUFSIZE];
//    uint8_t wbuf[BUFSIZE];
    bool selected = false; 
    bool ready_for_deletion = false;
    bool debugging = false;

public:
    Pipeline();
    ~Pipeline();
    int RegisterSocket(int r, int w);
    int GetRsockfd();
    int GetWsockfd();
    int pRead();
    int pWrite();
    Pipeline_Type GetPipelineType();
    int SetPipelineType(Pipeline_Type a);
    Pipeline* GetNextPipeline();
    int SetNextPipeline(Pipeline *p);
    int Shutdown();
    int  SetState(Pipeline_State s);
    Pipeline_State  GetState();
    int GetRbufsize();
    int GetWbufsize();
    int SetRbufsize(uint16_t s);
    int SetWbufsize(uint16_t s);
    uint8_t *GetReadBuffer();
    uint8_t *GetWriteBuffer();
    void Debug_Read();
    void Debug_Write();
    bool GetSelected();
    void SetSelected();
    bool IsReadyForDeletion();
    void SetReadyForDeletion();


};

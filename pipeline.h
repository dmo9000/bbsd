#pragma once

#include <unistd.h>
#include <stdint.h>

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

private:
    Pipeline* next_pipeline = NULL;
    Pipeline_Type pType = Pipeline_Type::PIPELINE_RAW;
    Pipeline_State pState = Pipeline_State::STATE_DISCONNECTED;
    int rsock = -1;
    int wsock = -1;
    uint16_t rsize = 0;
    uint16_t wsize = 0;
    uint8_t rbuf[BUFSIZE];
    uint8_t wbuf[BUFSIZE];
    


public:
    Pipeline();
    ~Pipeline();
    int RegisterSocket(int r, int w);
    int GetRsockfd();
    int pRead();
    int pWrite();
    Pipeline_Type GetPipelineType();
    int SetPipelineType(Pipeline_Type a);
    Pipeline* GetNextPipeline();
    int Shutdown();
    int  SetState(Pipeline_State s);
    Pipeline_State  GetState();
};

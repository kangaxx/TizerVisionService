#include "delegateDllLibrary.h"

HImage cameraWorker(int argc, char* in[])
{
    halconFunction("from  delegate library");
    while (true) {
        Sleep(100);
    }
    return HImage();
}

void setHalconFunction(callHalconFunc func)
{
    halconFunction = func;
}

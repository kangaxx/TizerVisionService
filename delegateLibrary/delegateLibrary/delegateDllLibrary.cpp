#include "delegateDllLibrary.h"

HImage cameraWorker(int argc, char* in[])
{
    g_handle = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 4096, 4096);
    if (g_handle == 0)
        return HImage();
    g_shareMemory = HeapAlloc(g_handle, HEAP_ZERO_MEMORY, 100);
    g_shareMemory = LPVOID(1);
    halconFunction(g_shareMemory);
    while (true) {
        Sleep(100);
    }
    return HImage();
}

void setHalconFunction(callHalconFunc func)
{
    halconFunction = func;
}

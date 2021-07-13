#pragma once
//send burrs calculate result in thread
//add by gxx 20210508
#include <zmq.hpp>

class ZmqServer
{
public:
    static HANDLE hMutex;
    static DWORD WINAPI ThreadProc(PVOID pParam);
    static HANDLE m_hThread;
    static DWORD  m_dwThreadID;
};


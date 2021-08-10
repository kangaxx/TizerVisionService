#include "pch.h"
#include "ZmqServer.h"
#include "../../../hds/Fixed2WayList.h"
#include "../../../hds/common.h"

#include "BurrsInfoString.h"


using namespace commonfunction_c;

DWORD __stdcall ZmqServer::ThreadProc(PVOID pParam)
{
    Fixed2WayList<BurrsInfoString>* burrsInfoList = (Fixed2WayList<BurrsInfoString>*) pParam;
    void* context = zmq_ctx_new();
    void* responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(responder, "tcp://*:5555");
    while (true) {
        char recv[100] = { '\0' };
        rc = zmq_recv(responder, recv, 100, 0);
        WaitForSingleObject(hMutex, INFINITE);
        if (burrsInfoList != NULL && burrsInfoList->size() > 0) {
            zmq_send(responder, burrsInfoList->getElement(0).getBuffer(), INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE, 0);
            Sleep(300);
        }
        ReleaseMutex(hMutex);
    }
    zmq_close(responder);
    zmq_ctx_destroy(context);
	return 0;
}

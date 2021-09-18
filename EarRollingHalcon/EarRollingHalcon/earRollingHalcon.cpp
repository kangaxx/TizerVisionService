#include "easywsclient.hpp"
//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include "earRollingHalcon.h"
using easywsclient::WebSocket;
#define SEND_NO_IMAGE //如果需要发送图片请屏蔽此项
void handle_message(const std::string& message)
{
    printf(">>> %s\n", message.c_str());
    //if (message == "world") { ws->close(); }
}

char** halconAction(int argc, char* in[], HImage image, char** out)
{
	//**out = 'h';
	//image.WriteImage("jpg", 0, "d:/grabs/libImageTest");
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return NULL;
    }
#endif
    WebSocket::pointer ws = NULL;

    //HObject ho_ImageLoad;
    //ReadImage(&ho_ImageLoad, "d:/images/22_1.bmp");
    //HImage ho_Image;
   // ho_Image = ho_ImageLoad;
   // HString type;
   // Hlong width, height;
   // unsigned char* ptr = (unsigned char*)ho_Image.GetImagePointer1(&type, &width, &height);
   // std::string imageStr = (char*)ptr;
    //ws = WebSocket::from_url("ws://114.55.169.91:8126/foo");
    ws = WebSocket::from_url("ws://127.0.0.1:5555/winding");
    if (!ws)
        return NULL;
    //ws->send("goodbye");
    std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"time\":\"%s\"}";
#ifndef SEND_NO_IMAGE
    char message[5020000];
#else
    char message[2048];
    string imageStr = "0000";
#endif
    int id = 0;

    while (true) {
        float width = 142.0 + ((float)(rand() % 30)) / 10.0;
        float ll = 21 + ((float)(rand() % 30)) / 10.0;
        float lr = 46 + ((float)(rand() % 30)) / 10.0;
        float rl = 94 + ((float)(rand() % 30)) / 10.0;
        float rr = 122 + ((float)(rand() % 30)) / 10.0;
        sprintf_s(message, 2048, messageFmt.c_str(), id++, imageStr.c_str(), width, ll, lr, rl, rr, "2021-01-01 12:00:01");
        ws->send(message);
        if (ws->getReadyState() != WebSocket::CLOSED) {
            ws->poll();
            ws->dispatch(handle_message);
            Sleep(4);
        }
    }
    delete ws;
#ifdef _WIN32
    WSACleanup();
#endif

	return out;
}

#pragma once
#include "stdafx.h"
#include <winsock2.h>
#include <iostream>
#include "ModbusCom.h"

#pragma comment(lib,"ws2_32.lib")

ModbusCom g_mb_obj;
int com_num_;
extern "C" {
	__declspec(dllexport) void initial(int argc, char* in[]);
	__declspec(dllexport) void open(int);
	__declspec(dllexport) void close();
	__declspec(dllexport) bool trigger_complete(int);
}

class ModBusLibrary
{
};


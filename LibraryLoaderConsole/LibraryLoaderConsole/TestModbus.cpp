// TestModbus.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <winsock2.h>
#include "ModbusCom.h"

#pragma comment(lib,"ws2_32.lib")
int _tmain(int argc, _TCHAR* argv[])
{
	//≥ı ºªØWSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	ModbusCom modbusObj;
	int temp;
	if (modbusObj.Connect("192.168.0.80", 10123, 1) == 0)
	{
		modbusObj.CloseAllCol(4);
		modbusObj.Open(1);
		std::cout << "Com 1 opened, press any key continue ...";
		std::cin >> temp;
		modbusObj.Close(1);
		std::cout << "Com 1 closed, press any key exit!";
		std::cin >> temp;
	}

	return 0;
}


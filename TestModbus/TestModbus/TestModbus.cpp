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
	try {
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
			modbusObj.Open(3);
			std::cout << "Com 1 opened, press any key continue ...";
			std::cin >> temp;
			modbusObj.Close(3);
			std::cout << "Com 1 closed, press any key exit!";
			std::cin >> temp;                                                   
		}
		else
		{
			std::cout << "network not connect" << std::endl;
		}
	} 
	catch (...)
	{
		int x;
		std::cin >> x;
	}
	int y;
	std::cin >> y;
	return 0;
}


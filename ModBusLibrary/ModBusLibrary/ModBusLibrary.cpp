#include "ModBusLibrary.h"

void initial(int argc, char* in[])
{
	WORD sock_version = MAKEWORD(2, 2);
	WSADATA wsa_data;
	if (WSAStartup(sock_version, &wsa_data) != 0)
	{
		return;
	}
	if (g_mb_obj.Connect("192.168.0.80", 10123, 1) == 0)
		return;
	else
		std::cout << "Error: connect fail" << std::endl;
}

void open(int v)
{
	com_num_ = v;
	g_mb_obj.Open(com_num_);
	return;
}

void close()
{
	g_mb_obj.Close(com_num_);
	return;
}

bool trigger_complete(int v)
{
	try {
		ModbusCom mb_obj;
		WORD sock_version = MAKEWORD(2, 2);
		WSADATA wsa_data;
		if (WSAStartup(sock_version, &wsa_data) != 0)
		{
			return false;
		}
		if (mb_obj.Connect("192.168.0.80", 10123, 1) == 0) {
			com_num_ = v;
			if (mb_obj.Open(v) == -1)
				return false;
			Sleep(5000);
			if (mb_obj.Close(v) == -1) {
				return false;
			}
			return true;
		}
		else
			return false;

	}
	catch (...) {
		return false;
	}

}

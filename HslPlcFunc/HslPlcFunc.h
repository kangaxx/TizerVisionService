#pragma once
#include <iostream>


extern "C" {
	__declspec(dllexport) bool write_plc_bool(const char*, bool, bool);
	__declspec(dllexport) bool connect_plc();
	__declspec(dllexport) bool disconnect_plc();
}

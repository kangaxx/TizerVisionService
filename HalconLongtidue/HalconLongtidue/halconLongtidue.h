#pragma once
#include <iostream>

extern "C" {
	__declspec(dllexport) char** halconLongtitudeAction(int argc, char** out, char* in[]);
}
char** halconLongtitudeAction(int argc, char** out, char* in[]);
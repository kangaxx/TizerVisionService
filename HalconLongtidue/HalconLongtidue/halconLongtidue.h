#pragma once
#include <iostream>

extern "C" {
	__declspec(dllexport) char** halconAction(int argc, char** out, char* in[]);
}

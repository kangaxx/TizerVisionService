#pragma once
#include "HalconCpp.h"
#include "HDevThread.h"

using namespace std;
using namespace HalconCpp;
extern "C" {
	__declspec(dllexport) char** halconAction(int argc, char* in[], char* image, char** out);
}

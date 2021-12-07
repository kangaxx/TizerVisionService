#pragma once
#include "HalconCpp.h"
using namespace HalconCpp;
#define DEBUG_MODE
extern "C" {
	__declspec(dllexport) bool calibrationWorker(int argc, char* in[]);

}
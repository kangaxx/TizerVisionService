#pragma once
#include "../../../hds/FastDelegate.h"
#include "HalconCpp.h"
#include "HDevThread.h"

using namespace HalconCpp;
using namespace fastdelegate;
typedef void (*callHalconFunc)(int, char* [], const HBYTE*);
static callHalconFunc halconFunction = nullptr;
extern "C" {
	__declspec(dllexport) char** cameraWorker(int argc, char* in[], char** out);
	__declspec(dllexport) void setHalconFunction(callHalconFunc func);
}

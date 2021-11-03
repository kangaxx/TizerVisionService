#pragma once
#include "HalconCpp.h"
#include "HDevThread.h"
#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>
using namespace HalconCpp;


typedef void (*callHalconFunc)(LPVOID);
static callHalconFunc halconFunction = nullptr;
static LPVOID g_shareMemory;
static HANDLE g_handle;
extern "C" {
	__declspec(dllexport) HImage cameraWorker(int argc, char* in[]);
	__declspec(dllexport) void setHalconFunction(callHalconFunc func);
}


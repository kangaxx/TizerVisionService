#pragma once
#include "HalconCpp.h"
#include "HDevThread.h"
#include <iostream>
#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>
using namespace HalconCpp;


typedef void (*callHalconFunc)(std::string);
static callHalconFunc halconFunction = nullptr;
static HGLOBAL g_hGlobal;
extern "C" {
	__declspec(dllexport) HImage cameraWorker(int argc, char* in[]);
	__declspec(dllexport) void setHalconFunction(callHalconFunc func);
}


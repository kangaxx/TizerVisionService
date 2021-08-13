#pragma once
#include <UserEnv.h>
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
#include "HalconCpp.h"
#include "HDevThread.h"
#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>


#ifdef PYLON_WIN_BUILD
#   include <pylon/PylonGUI.h>

// Include file to use pylon universal instant camera parameters.
#	include <pylon/BaslerUniversalInstantCamera.h>
#endif

using namespace HalconCpp;
using namespace fastdelegate;
using namespace Pylon;
using namespace commonfunction_c;
#define INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE 512


typedef void (*callHalconFunc)(int, char* [], HBYTE []);
static callHalconFunc halconFunction = nullptr;
class HalconData {
public:
	HalconData() {
		//to do list
		_image = NULL;
	}

	void setImage(const HBYTE* source, size_t size) {
		try {
			_image = (HBYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size * sizeof(HBYTE));
			memcpy_s(_image, size, source, size);
			_size = size;
		}
		catch (...) {
			//to do list
		}
	}

	HBYTE* getImage() {
		return _image;
	}

	void freeImage() {
		if (_image != NULL)
			HeapFree(GetProcessHeap(), 0, _image);
	}
private:
	size_t _size;
	HBYTE* _image;
};

extern "C" {
	__declspec(dllexport) void cameraWorker(int argc, char* in[], HalconData& ho_data);
	__declspec(dllexport) void setHalconFunction(callHalconFunc func);
}

#pragma once
#include <Windows.h>
#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/serialization_c11.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/halconUtils.h"
#include "../../../hds/Logger.h"
#include "../../../hds/configHelper.h"

using namespace fastdelegate;
class LibraryManager;
typedef FastDelegate3<int, char* [], const HBYTE*> SIGNAL_INT_CHAR_BYTE;
typedef char** (*halconFunc)(int, char* [], char**);
typedef char** (*cameraWork)(int, char* [], char**);
typedef void (*callHalconFunc)(int, char* [], const HBYTE*);
typedef void (*setHalconFunctionDelegate)(void (LibraryManager::*)(int, char* [], const HBYTE*));
using namespace commonfunction_c;

class LibraryManager
{
public:
	LibraryManager();
	void runCameraLib();
	void runHalconLib(int argc, char* in[], const HBYTE* image);
};


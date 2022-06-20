/******************************************************/
/*                    create by gxx                   */
/*                     22��06��16��                   */
/*                 ������sdk����ģ��                */
/******************************************************/

#pragma once
#include "../../../hds/common.h"
#include "../../../hds/CameraHelper.h"
#include "../../../hds/Logger.h"
#include "version.h"

#define _VERTOSTRING(arg) #arg
#define VERTOSTRING(arg) _VERTOSTRING(arg)
#define VER_MAIN    1
#define VER_SUB     0
#define VER_FULL    VER_MAIN.VER_SUB.VER_BUILD
#define VER_FULL_RC VER_MAIN,VER_SUB,VER_BUILD
#define STR_VER_FULL    _T(VERTOSTRING(VER_FULL))
#define STR_VER_FULL_RC VERTOSTRING(VER_FULL_RC)

using namespace commonfunction_c;
extern "C" {
	__declspec(dllexport) CameraDevicesParent* get_camera_devices(); //�ô��sdk��ʼ��g_device
	__declspec(dllexport) void free_camera_devices(); //�ͷ�g_device
}

class DahengCameraDevice : CameraDevicesParent
{
public:
	DahengCameraDevice() {}
	bool do_capture(int index, char** result);
private:
	Logger* log = nullptr;
};

static DahengCameraDevice g_devices;
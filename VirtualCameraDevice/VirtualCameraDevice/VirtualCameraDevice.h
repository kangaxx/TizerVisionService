/******************************************************/
/*                    create by gxx                   */
/*                     22年06月16日                   */
/*                 虚拟相机调试功能模块               */
/******************************************************/
#pragma once
#include "HalconCpp.h"

#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/CameraHelper.h"
#include "../../../hds/Logger.h"
#include "../../../hds/halconUtils.h"
#include "../../../hds/JsonHelper.h"
#include "version.h"

//#define DEBUG_WORK_PATH "Z:\\TizerVisionService\\LibraryLoaderConsole\\x64\\Debug\\" //代码调试模式下工作目录
#define _VERTOSTRING(arg) #arg
#define VERTOSTRING(arg) _VERTOSTRING(arg)
#define VER_MAIN    1 //系统结构级别
#define VER_SUB     1 //整体模块，主要通讯级别，兼容性变动
#define VER_MODI    1 //bug批量修正，不影响兼容
#define VER_FULL    VER_MAIN.VER_SUB.VER_MODI.VER_BUILD
#define VER_FULL_RC VER_MAIN.VER_SUB.VER_MODI.VER_BUILD
#define STR_VER_FULL    _T(VERTOSTRING(VER_FULL))
#define STR_VER_FULL_RC VERTOSTRING(VER_FULL_RC)


using namespace commonfunction_c;

typedef void (*cameraCaputredDelegate)(const char*, HImage&);
extern "C" {
	__declspec(dllexport) CameraDevicesBase* get_camera_devices(const char* config); //用大恒sdk初始化g_device
	__declspec(dllexport) void* set_camera_captured_delegate(cameraCaputredDelegate);
	__declspec(dllexport) void free_camera_devices(); //释放g_device
}

cameraCaputredDelegate g_delegate_function = NULL;
class VirtualCameraDevice : CameraDevicesBase
{
public:
	VirtualCameraDevice(const char* config);
	VirtualCameraDevice& operator =(VirtualCameraDevice right) {
		//wait to do
	}
	VirtualCameraDevice(const VirtualCameraDevice& right) {}
	~VirtualCameraDevice() {}
	bool do_capture(int index, HalconCpp::HImage& image);
	string get_camera_tag(int index);
	int get_devices_num() {
		return _image_count;
	}
	void set_capture_type(bool value){ //true表示回调模式
		_is_delegate_capture = value;
	}
	bool is_delegate_capture() {
		return _is_delegate_capture;
	}
private:
	VirtualCameraDevice() {} //暂时不允许无参数的构建函数
	Logger* _log = nullptr;
	string _config;
	int _image_count = 0;
	int _camera_count = 0;
	bool _is_delegate_capture; //拍照是回调触发还是主动触发
	vector<string> _image_files;
	vector<string> _camera_tags;
	HImage img1, img2, img3, img4, img5, img6, img7, img8;
	HImage image_list[MAX_VIRTUAL_CAMERA_COUNT] = { img1, img2, img3, img4, img5, img6, img7, img8 };
};

static VirtualCameraDevice* g_devices = NULL;
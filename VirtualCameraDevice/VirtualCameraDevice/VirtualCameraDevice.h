/******************************************************/
/*                    create by gxx                   */
/*                     22��06��16��                   */
/*                 ����������Թ���ģ��               */
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

//#define DEBUG_WORK_PATH "Z:\\TizerVisionService\\LibraryLoaderConsole\\x64\\Debug\\" //�������ģʽ�¹���Ŀ¼
#define _VERTOSTRING(arg) #arg
#define VERTOSTRING(arg) _VERTOSTRING(arg)
#define VER_MAIN    1 //ϵͳ�ṹ����
#define VER_SUB     1 //����ģ�飬��ҪͨѶ���𣬼����Ա䶯
#define VER_MODI    1 //bug������������Ӱ�����
#define VER_FULL    VER_MAIN.VER_SUB.VER_MODI.VER_BUILD
#define VER_FULL_RC VER_MAIN.VER_SUB.VER_MODI.VER_BUILD
#define STR_VER_FULL    _T(VERTOSTRING(VER_FULL))
#define STR_VER_FULL_RC VERTOSTRING(VER_FULL_RC)


using namespace commonfunction_c;

typedef void (*cameraCaputredDelegate)(const char*, HImage&);
extern "C" {
	__declspec(dllexport) CameraDevicesBase* get_camera_devices(const char* config); //�ô��sdk��ʼ��g_device
	__declspec(dllexport) void* set_camera_captured_delegate(cameraCaputredDelegate);
	__declspec(dllexport) void free_camera_devices(); //�ͷ�g_device
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
	void set_capture_type(bool value){ //true��ʾ�ص�ģʽ
		_is_delegate_capture = value;
	}
	bool is_delegate_capture() {
		return _is_delegate_capture;
	}
private:
	VirtualCameraDevice() {} //��ʱ�������޲����Ĺ�������
	Logger* _log = nullptr;
	string _config;
	int _image_count = 0;
	int _camera_count = 0;
	bool _is_delegate_capture; //�����ǻص�����������������
	vector<string> _image_files;
	vector<string> _camera_tags;
	HImage img1, img2, img3, img4, img5, img6, img7, img8, img9, img10, img11, img12;
	HImage image_list[MAX_VIRTUAL_CAMERA_COUNT] = { img1, img2, img3, img4, img5, img6, img7, img8, img9, img10, img11, img12 };
};

static VirtualCameraDevice* g_devices = NULL;
#pragma once

#include "HalconCpp.h"

#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/CameraHelper.h"
#include "../../../hds/Logger.h"
#include "../../../hds/JsonHelper.h"
#include "../../../hds/halconUtils.h"

#include "version.h"
#include "MvCamera.h"

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
	__declspec(dllexport) void free_camera_devices(); //释放g_device
	__declspec(dllexport) void set_camera_captured_delegate(cameraCaputredDelegate);
}

cameraCaputredDelegate g_delegate_function = NULL;
//相机的属性记录
class HikCameraInfo
{
public:
	HikCameraInfo(string name) : _name(name) {

	}
	HikCameraInfo() {
		//wait to do
	}
	string get_searial_num() { return _searial_num; }
	void set_searial_num(string value) { _searial_num = value; }
	void set_position(int value) { _position = CAMERA_POSITION(value); }
	void set_heart_beat_time(int value) { _heart_beat_time_out = value; }
	int get_heart_beat_time() { return _heart_beat_time_out; }
	void do_capture(HalconCpp::HImage& image);
	void set_logger(Logger& log);
	void initial();
	void set_camera_id(int id) { _camera_id = id; }
	int get_camera_id() { return _camera_id; }
	void set_trigger_mode(bool value) { _is_trigger_mode = value; }
	bool is_trigger_mode() { return _is_trigger_mode; }
	void inc_captured_id() { _captured_id++; }
	int get_captured_id() { return _captured_id; }
	void set_image(HImage& image) { _image = image; _image_captured = true; _captured_id++; }
	HImage get_image() { return _image; }
	bool is_captured() { return _image_captured; }
private:
	string _name;
	string _searial_num; //序列号
	int _heart_beat_time_out; //心跳包时间
	int _camera_id = -1;
	int _captured_id = 0;
	CAMERA_POSITION _position;
	Logger* _log = nullptr;
	HImage _image;
	bool _image_captured = false;
	bool _is_trigger_mode = true; //触发模式， true表示信号出发模式， false表示程序控制
};

class HikCameraDevice : CameraDevicesBase
{
public:
	HikCameraDevice(const char* config);
	HikCameraDevice& operator =(HikCameraDevice right) {
		//wait to do
	}
	HikCameraDevice(const HikCameraDevice& right) {}
	~HikCameraDevice() {}
	bool do_capture(int index, HalconCpp::HImage& image);
	string get_camera_tag(int index);
	int get_devices_num();
	bool is_trigger_mode() { return _trigger_mode; }
	void set_trigger_mode(bool value) { _trigger_mode = value; }
private:
	void camera_capture(int camera_num_in_list, int camera_id); //触发拍摄模式，该模式下一次性启动全部相机，并且不在capture方法内返回图像
	HikCameraDevice() {} //暂时不允许无参数的构建函数
	Logger* _log = nullptr;
	HikCameraInfo* _camera_infos;
	string _config;
	bool _trigger_mode;
	int _camera_count = 0;
	MV_CC_DEVICE_INFO_LIST _device_list;
	vector<string> _camera_names;
};

static HikCameraDevice* g_devices = NULL;
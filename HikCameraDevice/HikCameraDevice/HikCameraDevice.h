#pragma once

#include "HalconCpp.h"

#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/CameraHelper.h"
#include "../../../hds/Logger.h"
#include "../../../hds/JsonHelper.h"

#include "version.h"
#include "MvCamera.h"

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

extern "C" {
	__declspec(dllexport) CameraDevicesBase* get_camera_devices(const char* config); //�ô��sdk��ʼ��g_device
	__declspec(dllexport) void free_camera_devices(); //�ͷ�g_device
}

//��������Լ�¼
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
private:
	string _name;
	string _searial_num; //���к�
	int _heart_beat_time_out; //������ʱ��
	CAMERA_POSITION _position;
	Logger* _log = nullptr;
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
private:
	HikCameraDevice() {} //��ʱ�������޲����Ĺ�������
	Logger* _log = nullptr;
	HikCameraInfo* _camera_infos;
	string _config;
	int _camera_count = 0;
	MV_CC_DEVICE_INFO_LIST _device_list;
};

static HikCameraDevice* g_devices = NULL;

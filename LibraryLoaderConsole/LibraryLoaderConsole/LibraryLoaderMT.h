#pragma once
#include <redis++.h>
#include <cstdio>
#include <unordered_set>
#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <UserEnv.h>
#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/JsonHelper.h"
#include "../../../hds/ConcurrentLogger.h"
using namespace commonfunction_c;
typedef bool (*write_plc_bool)(const char*, bool, bool);
typedef bool (*connect_plc)();
typedef bool (*disconnect_plc)();

class LibraryLoaderMT
{
public:
	LibraryLoaderMT(LOADER_MODE mode): _mode(mode) {
		initial();
	}

	LibraryLoaderMT() : _mode(LOADER_MODE::LM_CONSOLE_MODE) {
		initial();
	}

	void call_library_by_num(int num); //命令入口函数

	void log(const char*);
	void log(string);
private:
	LOADER_MODE _mode;
	JsonHelper _system_config;
	ConcurrentLogger* _logger;
	bool _initialed = false;
	void initial();
	bool call_lib_loader_plc_debug(); //调试plc库功
	void call_heart_beat();
	void heart_beat_thread(void* lpParameter);
	HImage call_camera_no_delegate();
	unsigned long readRedisProc(void* lpParameter);

	//读取相机动态链接库 备用
	//HImage run_msa_test() {
	//	HINSTANCE hDllInst;
	//	configHelper ch("c:\\tizer\\config.ini", CT_JSON);
	//	Logger l("d:");
	//	param_num_ = MSA_NO_TRIGGER_CAMERA_MODE;

	//	l.Log(ch.findValue("cameraLibrary", string("string")));
	//	hDllInst = LoadLibrary(BaseFunctions::s2ws(ch.findValue("cameraLibrary", string("string"))).c_str());
	//	if (hDllInst == 0) {
	//		throw "Load camera library failed!";
	//	}
	//	cameraWork cameraWorkFunc = NULL;
	//	cameraWorkFunc = (cameraWork)GetProcAddress(hDllInst, "cameraWorker");
	//	if (cameraWorkFunc == 0) {
	//		FreeLibrary(hDllInst);
	//		throw "Load camera library function failed!";
	//	}
	//	setHalconFunction setFunc = NULL;
	//	setFunc = (setHalconFunction)GetProcAddress(hDllInst, "setHalconFunction");
	//	if (setFunc == 0) {
	//		FreeLibrary(hDllInst);
	//		throw "Load camera library set halcon function failed!";
	//	}
	//	//测试委托
	//	setFunc(delegateFunction);
	//	string cameraLeftName = ch.findValue("cameraLeftName", string("string"));
	//	string cameraMidName = ch.findValue("cameraMidName", string("string"));
	//	string cameraRightName = ch.findValue("cameraRightName", string("string"));
	//	char* in[9];
	//	char leftCamera[50], midCamera[50], rightCamera[50], camera_width[10], camera_height[10], package_delay[10],
	//		package_size[10], exposure_time[10], center[10], param_num[5];
	//	strcpy_s(leftCamera, cameraLeftName.c_str());
	//	strcpy_s(midCamera, cameraMidName.c_str());
	//	strcpy_s(rightCamera, cameraRightName.c_str());

	//	commonfunction_c::BaseFunctions::Int2Chars(param_num_, param_num);
	//	in[0] = &param_num[0];
	//	in[1] = &leftCamera[0];
	//	in[2] = &midCamera[0];
	//	in[3] = &rightCamera[0];
	//	HImage image = cameraWorkFunc(0, in);
	//	return image;
	//}

	//MSA_NO_TRIGGER_CAMERA_MODE;国轩特定情况下使用 备用
	//void run_area_camera_function() {
	//	HINSTANCE hDllInst;
	//	param_num_ = MSA_NO_TRIGGER_CAMERA_MODE;
	//	_log->Log(_ch->findValue("cameraLibrary", string("string")));
	//	hDllInst = LoadLibrary(BaseFunctions::s2ws(_ch->findValue("cameraLibrary", string("string"))).c_str());
	//	if (hDllInst == 0) {
	//		throw "Load camera library failed!";
	//	}
	//	get_camera_devices get_camera_devices_func = NULL;
	//	get_camera_devices_func = (get_camera_devices)GetProcAddress(hDllInst, "get_camera_devices");
	//	if (get_camera_devices_func == 0) {
	//		FreeLibrary(hDllInst);
	//		throw "Load camera library function [get_camera_devices_func] failed!";
	//	}
	//	string camera_devices_info = _ch->findValue("camera_devices_info", string("string"));
	//	_log->Log(camera_devices_info);
	//	CameraDevicesBase* devices = get_camera_devices_func(camera_devices_info.c_str());
	//	//test do capture
	//	HImage image_test;
	//	int num = devices->get_devices_num();
	//	for (int i = 0; i < num; ++i) {
	//		devices->do_capture(0, image_test);
	//	}
	//	return;
	//}
};


/****************************************************
				四角十字定位算法程序
					22年07月13日
				   create by gxx 
****************************************************/
#pragma once
#include "HalconCpp.h"
#include "HDevThread.h"
#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/CameraHelper.h"
#include "../../../hds/Logger.h"
#include "../../../hds/halconUtils.h"
#include "../../../hds/JsonHelper.h"
#include "version.h"


#define _VERTOSTRING(arg) #arg
#define VERTOSTRING(arg) _VERTOSTRING(arg)
#define VER_MAIN    1 //系统结构级别
#define VER_SUB     1 //整体模块，主要通讯级别，兼容性变动
#define VER_MODI    1 //bug批量修正，不影响兼容
#define VER_FULL    VER_MAIN.VER_SUB.VER_MODI.VER_BUILD
#define VER_FULL_RC VER_MAIN.VER_SUB.VER_MODI.VER_BUILD
#define STR_VER_FULL    _T(VERTOSTRING(VER_FULL))
#define STR_VER_FULL_RC VERTOSTRING(VER_FULL_RC)

using namespace HalconCpp;
extern "C" {
	__declspec(dllexport) char** halcon_program(int argc, char* in[], HImage& image, char** out);
}
using namespace std;
using namespace commonfunction_c;



class JmjCornerAlignmentHalcon
{
public:
	JmjCornerAlignmentHalcon(int argc, char* in[], HImage& image) {
		char* json_input = in[0];
		JsonHelper jh(json_input);
		_camera_tag = jh.search("camera_tag");
#ifndef DEBUG_WORK_PATH
		_log_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + jh.search("log_dir");
		_calib_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + jh.search("calib_dir") + "\\" + jh.search("id");
#else
		_log_dir = DEBUG_WORK_PATH + "\\" + jh.search("log_dir");
		_calib_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + jh.search("calib_dir");
#endif
		_log = new Logger(_log_dir);
		_image = &image;
		load_halcon_calib_data();
	}
	void log(string words) { _log->Log(words); }
	void log(wstring words) { _log->Log(words); }
	Logger get_log() { return *_log; }
	void do_find_cross(int id, char** result);

private:
	HTuple _image_zoom_value = 0.2;
	Logger* _log;
	HImage* _image;
	string _log_dir, _calib_dir, _camera_tag;
	HTuple _camera_parameters, _camera_poses;
	string _calibration_file_name, _plate_desc;
	void load_halcon_calib_data();
};

 
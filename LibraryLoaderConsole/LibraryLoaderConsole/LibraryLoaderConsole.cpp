// LibraryLoaderConsole.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "LibraryLoaderMT.h"
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")
#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/serialization_c11.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
#include "../../../hds/configHelper.h"
#include "../../../hds/JsonHelper.h"
#include "../../../hds/CameraHelper.h"
#include "../../../hds/MeasureSize.h"
#include "../../../hds/tz_project_common.h"
#include "SerialPort.h"

using namespace fastdelegate;
using namespace HalconCpp;
using namespace sw::redis;
using namespace commonfunction_c;
class LibraryLoader;
typedef void (*halconFunc)(int, char* [], const char*, char**);
typedef HImage(*cameraWork)(int, char* []);
typedef bool (*calibrationWork)(int, char* []);
typedef void (*callHalconFunc)(char*);
typedef void (*cameraCaputredDelegate)(const char*, HImage&);
typedef void (*setHalconFunctionDelegate)(void (LibraryLoader::*)(int, char* [], HBYTE[]));
typedef void (*setHalconFunction)(callHalconFunc);
typedef void (*setCameraCapturedDelegate)(cameraCaputredDelegate);
typedef void (*call_image_concat)();
typedef bool (*trigger_complete)(int);
typedef char** (*halcon_program)(int, char* [], HImage&, char**);
typedef void (*halcon_program_image_list)(int, char* [], vector<HImage*>&, char*, MeasureSize&, int& result); //多相机列表类
typedef bool (*write_plc_bool)(const char*, bool, bool);
typedef bool (*connect_plc)();
typedef bool (*disconnect_plc)();
typedef CameraDevicesBase* (*get_camera_devices)(const char*); //初始化相机设备
typedef void (*free_camera_devices)(); //释放相机
static int index;
std::mutex log_mtx;
std::mutex mtx;
static halconUtils::HImageExtraInfo* _temp_img0 = NULL;
static halconUtils::HImageExtraInfo* _temp_img1 = NULL;
static HTuple system_time_hour, system_time_min, system_time_sec, system_time_msec;
//#define DEBUG_WORK_PATH "Z:\\TizerVisionService\\LibraryLoaderConsole\\x64\\Debug\\" //代码调试模式下工作目录
#define PROGRAM_COMPLIRE_VERSION "Console program, version 1.20920.17"

#ifdef PYLON_WIN_BUILD
#   include <pylon/PylonGUI.h>

// Include file to use pylon universal instant camera parameters.
#	include <pylon/BaslerUniversalInstantCamera.h>
#endif

using namespace fastdelegate;
using namespace HalconCpp;
using namespace sw::redis;
//using namespace boost::asio;

void delegateFunction(char*);
void camera_caputred_delegate(const char* json, HImage& image);



using namespace commonfunction_c;



//如果电芯不合格，需要触发485继电器来剔除，下面的功能就是触发485
void switchTrigger485(int port)
{
	/*com口继电器， 暂时不用了，改用modbus网线版本
	CSerialPort sp;
	TCHAR szPort[MAX_PATH];
	_stprintf_s(szPort, MAX_PATH, _T("COM%d"), port);
	sp.Open(szPort, 9600UL);
	if (sp.IsOpen()) {
		std::cout << "sp is open" << std::endl;
		unsigned char value[8];
		value[0] = 0x01;
		value[1] = 0x06;
		value[2] = 0x00;
		value[3] = 0x00;
		value[4] = 0x00;
		value[5] = 0x01;
		value[6] = 0x48;
		value[7] = 0x0A;
		//out 1 开启
		sp.Write((void*)(&value[0]), 8);

		Sleep(300);
		value[0] = 0x01;
		value[1] = 0x06;
		value[2] = 0x00;
		value[3] = 0x01;
		value[4] = 0x00;
		value[5] = 0x01;
		value[6] = 0x19;
		value[7] = 0xCA;
		//out 1 关闭
		sp.Write((void*)(&value[0]), 8);
	}
	else
		std::cout << "sp is close" << std::endl;
	sp.Close();
	*/

	HINSTANCE hDllInst;

	configHelper ch("c:\\tizer\\config.ini", CT_JSON);
	hDllInst = LoadLibrary(LPCTSTR(L"ModBusLibrary"));
	if (hDllInst == 0) {
		throw "Calibration library failed!";
	}
	trigger_complete trigger_complete_function = NULL;
	trigger_complete_function = (trigger_complete)GetProcAddress(hDllInst, "trigger_complete");
	if (trigger_complete_function == 0) {
		FreeLibrary(hDllInst);
		throw "Load camera library function failed!";
	}
	char* in[5];
	Logger l;
	if (!trigger_complete_function(port))
		l.Log("Electry relay trigger failed!");
	else
		l.Log("Electry relay trigger successed!");
	return;
}

//循环读取redis列表看看前台有没有数据返回


unsigned long thread_save_log(void* lpParameter) {
	int old_msec = 0;
	int used_time;
	int log_type = 0;
	int old_sys_time_msec = 0;
	char message[200];
	Logger l("c:/tz_log");
	while (true) {
		if (system_time_msec.I() != old_sys_time_msec && (4 == g_log_msec_type || 7 == g_log_msec_type || 8 == g_log_msec_type)) {


			old_sys_time_msec = system_time_msec.I();
			//l.linkable_log("log_time_type:").linkable_log(BaseFunctions::Int2Str(log_type)).linkable_log("time:").linkable_log(BaseFunctions::Int2Str(g_msec)).linkable_log(" used time: ").Log(used_time);
			
			//cout << "local time :" << system_time_hour.I() << ":" << system_time_min.I() << ":" << system_time_sec.I() << "  " << system_time_msec.I() << "  ,";

			log_mtx.lock();
			switch (g_log_msec_type) {
			case 1:
				sprintf_s(message, 200, "local time : %d : %d : %d + % d ,desc : [%s] ", system_time_hour.I()
					, system_time_min.I(), system_time_sec.I(), system_time_msec.I(), "start calc");
				break;
			case 2:
				sprintf_s(message, 200, "local time : %d : %d : %d + % d ,desc : [%s] ", system_time_hour.I()
					, system_time_min.I(), system_time_sec.I(), system_time_msec.I(), "calc finished");
				break;
			case 3:
				sprintf_s(message, 200, "local time : %d : %d : %d + % d ,desc : [%s] ", system_time_hour.I()
					, system_time_min.I(), system_time_sec.I(), system_time_msec.I(), "redis updated");
				break;
			case 4:
				sprintf_s(message, 200, "local time : %d : %d : %d + % d ,desc : [%s] ", system_time_hour.I()
					, system_time_min.I(), system_time_sec.I(), system_time_msec.I(), "get image from camera");
				break;
			case 7:
				sprintf_s(message, 200, "local time : %d : %d : %d + % d ,desc : [%s] ", system_time_hour.I()
					, system_time_min.I(), system_time_sec.I(), system_time_msec.I(), "write plc ng finished");

				break;
			case 8:
				sprintf_s(message, 200, "local time : %d : %d : %d + % d ,desc : [%s] ", system_time_hour.I()
					, system_time_min.I(), system_time_sec.I(), system_time_msec.I(), "write plc ok finished");

				break;
			default:
				sprintf_s(message, 200, "local time : %d : %d : %d + % d ,desc : [%s] ", system_time_hour.I()
					, system_time_min.I(), system_time_sec.I(), system_time_msec.I(), "others");

				break;
			}
			l.Log(message);
			log_mtx.unlock();
		}
		else {
			timeBeginPeriod(1);  // 设置精度为1毫秒
			::Sleep(3);          // 当前线程挂起一毫秒
			timeEndPeriod(1);    // 结束精度设置
		}

	}
	return 0;
}
unsigned long thd_call_halcon_camera_array_with_delegate(void* lpParameter);
class LibraryLoader {
public:
	LibraryLoader() {
		_ch = new configHelper("c:\\tizer\\config.ini", CT_JSON);

#ifndef DEBUG_WORK_PATH
		string log_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + _ch->findValue("log_dir", string("string"));
#else
		string log_dir = DEBUG_WORK_PATH + "\\" + _ch->findValue("log_dir", string("string"));
#endif
		_log = new Logger(log_dir);
	}

	void log(const char* msg) {
		_log->Log(msg);
	}

	void log(string msg) {
		_log->Log(msg);
	}
	//读取算法动态链接库（非回调模式）
	void run_halcon_lib_no_delegate(int argc, char* in[], HImage& image) {
		_log->Log("run_halcon_lib_no_delegate start");
		Redis redis = Redis("tcp://127.0.0.1:6379");
		HINSTANCE hDllInst;
		hDllInst = LoadLibrary(LPCTSTR(BaseFunctions::s2ws(_ch->findValue("halconLibraryND", string("string"))).c_str()));
		if (hDllInst == 0) {
			_log->Log("Halcon library (ND) load error!");
			return;
		}
		halcon_program hp = NULL;
		hp = (halcon_program)GetProcAddress(hDllInst, "halcon_program");
		if (hp == 0) {
			_log->Log("Halcon function load error!");
			return;
		}
		char buffer[INT_HALCON_BURR_RESULT_SIZE] = { '\0' };
		char** out = new char* ();
		*out = &buffer[0];
		char* source[8];
		source[0] = in[0];
		try {
			hp(argc, source, image, out);
			lPush(REDIS_WRITE_KEY, string(*out));
		}
		catch (...) {
		}
	}

	//读取算法动态链接库（回调模式+HImage）
	void run_halcon_lib_with_delegate_image(int argc, char* in[], HImage& image) {
		strncpy_s(temp_in_0, in[0], 256);
		log_mtx.lock();
		update_current_msec(4);
		HalconCpp::GetSystemTime(&system_time_msec, &system_time_sec, &system_time_min, &system_time_hour, 0, 0, 0, 0);
		log_mtx.unlock();
		JsonHelper jh;
		jh.initial(string(in[0]));
		int camera_id = BaseFunctions::Str2Int(jh.search("camera_id")); //min camera id is 0
		int job_id = BaseFunctions::Str2Int(jh.search("job_id"));
		halconUtils::HImageExtraInfo* ex_image_info = new halconUtils::HImageExtraInfo(image);
		ex_image_info->set_camera_id(camera_id);
		ex_image_info->set_job_id(job_id);
		ex_image_info->set_total_count(_camera_count);
		mtx.lock();
		switch (camera_id)
		{
		case 0:
			_temp_img0 = ex_image_info;
			break;
		case 1:
			_temp_img1 = ex_image_info;
			break;
		default:
			break;
		}
		//_captured_images.at(camera_id) = &image
		mtx.unlock();
		return;
	}

	void runHalconLib(int argc, char* in[], string image) {
		Logger l("d:");
		l.Log("Console program #runHalconLib start"); //test log
		HINSTANCE hDllInst;

		Redis redis = Redis("tcp://127.0.0.1:6379");
		StringView work_status_key = REDIS_WORK_STATUS_KEY;
		auto work_status_value = redis.get(work_status_key);
		if (work_status_value.value()._Equal(REDIS_WORK_STATUS_STOP) > 0) {
			//客户停止检测程序
			l.Log("Measure program stopped by client user!");

			return;
		}
		else {
			l.Log("Measure program set to run by client user!"); // test log
		}

		configHelper ch("c:\\tizer\\config.ini", CT_JSON);
		hDllInst = LoadLibrary(LPCTSTR(BaseFunctions::s2ws(ch.findValue("halconLibrary", string("string"))).c_str()));
		if (hDllInst == 0) {
			l.Log("Halcon library load error!");
			return;
		}
		halconFunc func = NULL;
		func = (halconFunc)GetProcAddress(hDllInst, "halconAction");
		if (func == 0) {
			l.Log("Halcon function load error!");
			return;
		}
		int burr_limit = 15;
		int localImage = ch.findValue("localImage", 1);
		char* source[8];
		//设置输入参数
		//
		int width = 1920; //实际参数需要参看相机情况，读取本地文件时设置为0
		int height = 1080; // 同上
		int polesWidth = 10;
		source[0] = in[0]; //status
		source[1] = (char*)(&calibration_line_top_);
		source[2] = (char*)(&calibration_line_bottom_);
		source[3] = NULL;
		source[4] = NULL;
		source[5] = NULL;
		source[6] = NULL;
		source[7] = NULL;
		//初始化输出参数
		char buffer[INT_HALCON_BURR_RESULT_SIZE] = { '\0' };
		char** out = new char* ();
		*out = &buffer[0];
		try {
			func(param_num_, source, image.c_str(), out);
			lPush(REDIS_WRITE_KEY, string(*out));
		}
		catch (...) {
		}
		//std::cout << "get taichi result : " << **out << std::endl;
		if (hDllInst > 0)
			FreeLibrary(hDllInst);
		return;
	}


	HImage runCalibrationCameraLib() {
		//CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&readRedisProc, NULL, 0, 0);
		param_num_ = STANDARD_CAMERA_MODE;
		HINSTANCE hDllInst;
		configHelper ch("c:\\tizer\\config.ini", CT_JSON);
		Logger l("d:");
		hDllInst = LoadLibrary(BaseFunctions::s2ws(ch.findValue("cameraLibrary", string("string"))).c_str());
		if (hDllInst == 0) {
			throw "Load camera library failed!";
		}
		cameraWork cameraWorkFunc = NULL;
		cameraWorkFunc = (cameraWork)GetProcAddress(hDllInst, "cameraWorker");
		if (cameraWorkFunc == 0) {
			FreeLibrary(hDllInst);
			throw "Load camera library function failed!";
		}

		string cameraLeftName = ch.findValue("cameraLeftName", string("string"));
		string cameraMidName = ch.findValue("cameraMidName", string("string"));
		string cameraRightName = ch.findValue("cameraRightName", string("string"));
		char* in[5];
		char leftCamera[50], midCamera[50], rightCamera[50], param_num[5];
		commonfunction_c::BaseFunctions::Int2Chars(param_num_, param_num);
		strcpy_s(leftCamera, cameraLeftName.c_str());
		strcpy_s(midCamera, cameraMidName.c_str());
		strcpy_s(rightCamera, cameraRightName.c_str());
		in[0] = &param_num[0];
		in[1] = &leftCamera[0];
		in[2] = &midCamera[0];
		in[3] = &rightCamera[0];
		HImage image = cameraWorkFunc(0, in);
		return image;
	}

	void runCalibration() {
		HINSTANCE hDllInst;

		configHelper ch("c:\\tizer\\config.ini", CT_JSON);
		hDllInst = LoadLibrary(LPCTSTR(BaseFunctions::s2ws(ch.findValue("calibrationLibrary", string("string"))).c_str()));
		if (hDllInst == 0) {
			throw "Calibration library failed!";
		}
		calibrationWork calibrationWorkFunc = NULL;
		calibrationWorkFunc = (calibrationWork)GetProcAddress(hDllInst, "calibrationWorker");
		if (calibrationWorkFunc == 0) {
			FreeLibrary(hDllInst);
			throw "Load camera library function failed!";
		}
		char* in[5];
		Logger l;
		if (!calibrationWorkFunc(0, in))
			l.Log("Calibration failed!");
		else
			l.Log("Calibration successed!");
		return;
	}

	void run_manual_concat_image() {
		Logger l("d:");
		HINSTANCE hDllInst;
		configHelper ch("c:\\tizer\\config.ini", CT_JSON);
		l.Log(ch.findValue("cameraLibrary", string("string")));
		hDllInst = LoadLibrary(BaseFunctions::s2ws(ch.findValue("cameraLibrary", string("string"))).c_str());
		if (hDllInst == 0) {
			throw "Load camera library failed!";
		}
		call_image_concat call_image_concat_function = NULL;
		call_image_concat_function = (call_image_concat)GetProcAddress(hDllInst, "call_image_concat");
		if (call_image_concat_function == 0) {
			FreeLibrary(hDllInst);
			throw "Load camera library function call_image_concat failed!";
		}
		call_image_concat_function();
		return;
	}

	void lPush(string k, string v) {
		Redis redis = Redis("tcp://127.0.0.1:6379");
		StringView key = k.c_str();
		StringView value = v.c_str();
		redis.lpush(key, value);
		return;
	}

	string rPop(string k) {
		Redis redis = Redis("tcp://127.0.0.1:6379");
		StringView key = k.c_str();

		auto value = redis.rpop("list");
		return value.value();
	}

	string get(string k) {
		Redis redis = Redis("tcp://127.0.0.1:6379");
		StringView key = k.c_str();

		auto value = redis.get(key);
		return value.value();
	}

	bool set(string k, string v) {
		Redis redis = Redis("tcp://127.0.0.1:6379");
		StringView key = k.c_str();

		return redis.set(key, v);
	}

	void run_camera_with_delegate_no_processing() {
		HINSTANCE hDllInst;
		_log->Log(_ch->findValue("cameraLibrary", string("string")));
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&thd_call_halcon_camera_array_with_delegate, NULL, 0, 0);
		hDllInst = LoadLibrary(BaseFunctions::s2ws(_ch->findValue("cameraLibrary", string("string"))).c_str());
		if (hDllInst == 0) {
			throw "Load camera library failed!";
		}
		setCameraCapturedDelegate set_camera_capture_delegate;
		set_camera_capture_delegate = (setCameraCapturedDelegate)GetProcAddress(hDllInst, "set_camera_captured_delegate");
		if (set_camera_capture_delegate == 0) {
			FreeLibrary(hDllInst);
			throw "Load camera library function [set_camera_captured_delegate] failed!";
		}
		set_camera_capture_delegate(camera_caputred_delegate);
		get_camera_devices get_camera_devices_func = NULL;
		get_camera_devices_func = (get_camera_devices)GetProcAddress(hDllInst, "get_camera_devices");
		if (get_camera_devices_func == 0) {
			FreeLibrary(hDllInst);
			throw "Load camera library function [get_camera_devices_func] failed!";
		}
		string camera_devices_info = _ch->findValue("camera_devices_info", string("string"));
		CameraDevicesBase* devices = get_camera_devices_func(camera_devices_info.c_str());
		if ((_camera_count = devices->get_devices_num()) < 0) {
			_log->Log("Error, no camera!");
			throw "Error , no camera!";
		}

		HImage no_image_return; //委托模式下himage在回调方法里返回
		if (devices->do_capture(-1, no_image_return))
			set("camera_ok", "0");
		else
		{
			_log->Log("Error, camera setup fail");
			throw "Error, camera setup fail!";
		}
	}

	void run_camera_no_delegate_no_processing() {
		HINSTANCE hDllInst;
		_log->Log(_ch->findValue("cameraLibrary", string("string")));
		hDllInst = LoadLibrary(BaseFunctions::s2ws(_ch->findValue("cameraLibrary", string("string"))).c_str());
		if (hDllInst == 0) {
			throw "Load camera library failed!";
		}
		get_camera_devices get_camera_devices_func = NULL;
		get_camera_devices_func = (get_camera_devices)GetProcAddress(hDllInst, "get_camera_devices");
		if (get_camera_devices_func == 0) {
			FreeLibrary(hDllInst);
			throw "Load camera library function [get_camera_devices_func] failed!";
		}
		string camera_devices_info = _ch->findValue("camera_devices_info", string("string"));
		CameraDevicesBase* devices = get_camera_devices_func(camera_devices_info.c_str());
		//test do capture
		HImage image_test;
		int num = devices->get_devices_num();
		for (int i = 0; i < num; ++i) {
			char* in[5];
			string fmt = "{\"id\": %d, \"camera_tag\":\"%s\", \"log_dir\":\"%s\", \"calib_dir\":\"%s\"}";
			string log_dir = _ch->findValue("log_dir", string("string"));
			string calib_dir = _ch->findValue("calib_dir", string("string"));
			char message[100];
			string camera_tag = devices->get_camera_tag(i);
			sprintf_s(message, 100, fmt.c_str(), _camera_jobs_id++, camera_tag.c_str(),
				log_dir.c_str(), calib_dir.c_str());
			in[0] = &message[0];
			if (devices->do_capture(i, image_test))
				run_halcon_lib_no_delegate(0, in, image_test);
			_halcon_jobs_id++;
		}
		return;
	}
	char temp_in_0[256];
private:
	configHelper* _ch;
	Logger* _log;
	int calibration_line_top_, calibration_line_bottom_, param_num_;
	int camera_width_, camera_height_, package_size_, package_delay_, center_, exposure_time_;
	int _halcon_jobs_id = 0;
	int _camera_jobs_id = 0;
	int _camera_count = 0;
	base_calc_std* _calc_std;
	commonfunction_c::DuLink<halconUtils::HImageExtraInfo*> _captured_images;
};
static LibraryLoader ll;
void delegateFunction(char* msg) {
	Logger l("d:");
	ll.lPush("to_test", msg);
	JsonHelper jh;
	jh.initial(string(msg));
	string key = "image";
	//cout << "json initial by string , string : \"" << msg << "\" , key is : \"" << key << "\", value is :\"" << jh.search(key) << "\"" << endl;
	//调用算法
	try {
		char* args[8];
		char status[400];
		strcpy_s(status, 400, msg);
		args[0] = &status[0];
		ll.runHalconLib(1, args, jh.search(key));
	}
	catch (...) {

	}
	return;
}
void camera_caputred_delegate(const char* json, HImage& image) {
	//ll.log(string(json)); //test log
	char* args[8];
	char status[400];
	strcpy_s(status, 400, json);
	args[0] = &status[0];
	ll.run_halcon_lib_with_delegate_image(1, args, image);
}
//调用算法程序（通过相机回调）
unsigned long thd_call_halcon_camera_array_with_delegate(void* lpParameter) {
	try {
		HImage img0, img1;
		bool is_get_image = false;
		JsonHelper jh;
		jh.initialByFile("c:\\tizer\\config.ini");
		HINSTANCE hDllInst;
		string lib_name = jh.search("halconLibrary");
		hDllInst = LoadLibrary(LPCTSTR(BaseFunctions::s2ws(lib_name).c_str()));
		if (hDllInst == 0) {
			ll.log("Halcon library load error!");
			return 0;
		}
		halcon_program_image_list func = NULL;
		func = (halcon_program_image_list)GetProcAddress(hDllInst, "halconActionWithImageList");
		if (func == 0) {
			ll.log("Halcon function load error!");
			return 0;
		}
		string setup = ll.get("Setup");
		vector<double> params;
		int params_count = jh.read_array("camera_parameters", params);
		if (params_count != 16) {
			ll.log("Error: halcon params count out of range!");
			return 0;
		}
		MeasureSize ms(params.at(0), params.at(1), params.at(2), params.at(3),
			params.at(4), params.at(5), params.at(6), params.at(7), params.at(8),
			params.at(9), params.at(10), params.at(11), params.at(12), params.at(13), params.at(14), params.at(15));
		//MeasureSize ms;
		F0SIZE_PIXEL c0Pos;
		F2SIZE_PIXEL c2Pos;
		FSIZE stdSize;
		vector<double> fmt_value;
		int fmt_value_count = jh.read_array("std_size", fmt_value);
		/*
		if (0 == BaseFunctions::Str2Int(jh.search("node_type")))
			fmt_value = &anode_std_size[0];
		else if (1 == BaseFunctions::Str2Int(jh.search("node_type")))
			fmt_value = &cathode_std_size[0];
		else {
			ll.log("Read config node_type error!");
			return 0L;
		}
		*/
		stdSize.W = fmt_value[0];
		stdSize.L = fmt_value[1];
		stdSize.H = fmt_value[2];
		stdSize.H1 = fmt_value[3];
		stdSize.W1 = fmt_value[4];
		stdSize.W2 = fmt_value[5];
		stdSize.Ra[0] = 3;
		stdSize.Ra[1] = 3;
		stdSize.Rb[0] = 3;
		stdSize.Rb[1] = 3;
		stdSize.Rb[2] = 3;
		stdSize.Rb[3] = 3;
		int used_mesc = 0;
		//调用plc， connect代码 //暂时没有
		HINSTANCE dll_hsl_plc;
		dll_hsl_plc = LoadLibrary(BaseFunctions::s2ws(jh.search("hslLibrary")).c_str());
		if (dll_hsl_plc == 0) {
			ll.log("Load hsl library fail!");
			return 0;
		}
		connect_plc connect_plc_func = (connect_plc)GetProcAddress(dll_hsl_plc, "connect_plc");
		if (connect_plc_func == 0) {
			ll.log("Load hsl library function connect_plc fail!");
			return 0;
		}
		write_plc_bool write_plc_func = (write_plc_bool)GetProcAddress(dll_hsl_plc, "write_plc_bool");
		if (write_plc_func == 0) {
			ll.log("Load hsl library function write_plc_bool fail!");
			return 0;
		}
		disconnect_plc disconnect_plc_func = (disconnect_plc)GetProcAddress(dll_hsl_plc, "disconnect_plc");
		if (disconnect_plc_func == 0) {
			ll.log("Load hsl library function write_plc_bool fail!");
			return 0;
		}

		const char* ng_addr = "w100.07";
		const char* ok_addr = "w100.09";
		if (!connect_plc_func()) {
			ll.log("connect to plc fail!");
			return 0;
		}

		HImage img_fmt_0, img_fmt_2;
		mtx.lock();
		img_fmt_0.ReadImage("C:/tizer/fmt_0.jpg");
		img_fmt_2.ReadImage("C:/tizer/fmt_2.jpg");
		if (0 == ms.CalcCamera0(img_fmt_0, c0Pos)
			&& 0 == ms.CalcCamera2(img_fmt_2, c2Pos)
			)
		{
			if (0 == ms.SetStdSize(stdSize)) {
				ll.log("Format image setted success!");
			}
			else {
				ll.log("Format image setted fail!");
			}
		}
		else {
			ll.log("Format image setted fail!");
		}
		mtx.unlock();
		string offsets = ll.get("Offsets");
		while (true) {
			mtx.lock();
			if (_temp_img0 != NULL && _temp_img1 != NULL) {
				img0 = _temp_img0->get_image();
				img1 = _temp_img1->get_image();
				_temp_img0 = NULL;
				_temp_img1 = NULL;
				is_get_image = true;
			}
			else {
				is_get_image = false;
			}
			mtx.unlock();
			if (!is_get_image) {
				//不要再锁里sleep那样会一直锁
				timeBeginPeriod(1);  // 设置精度为1毫秒
				::Sleep(2);          // 当前线程挂起一毫秒
				timeEndPeriod(1);    // 结束精度设置
				continue;
			}

			int burr_limit = 15;
			char* source[8];
			//设置输入参数
			//
			int width = 1920; //实际参数需要参看相机情况，读取本地文件时设置为0
			int height = 1080; // 同上
			int polesWidth = 10;
			source[0] = ll.temp_in_0; //status
			source[1] = &setup[0];
			source[2] = &offsets[0];
			source[3] = NULL;
			source[4] = NULL;
			source[5] = NULL;
			source[6] = NULL;
			source[7] = NULL;
			//初始化输出参数
			try {
				char test[INT_HALCON_BURR_RESULT_SIZE] = { "\0" };
				vector<HImage*> _captured_images;
				_captured_images.push_back(&img0);
				_captured_images.push_back(&img1);
				//log_mtx.lock();
				//update_current_msec(1);
				//HalconCpp::GetSystemTime(&system_time_msec, &system_time_sec, &system_time_min, &system_time_hour, 0, 0, 0, 0);
				//log_mtx.unlock();
				int result_status;
				func(2, source, _captured_images, &test[0], ms, result_status);
				if (result_status == 1 || result_status == 2) {
					//需要完成plc功能 w100.07表示ng
					write_plc_func(ng_addr, true, false);
					log_mtx.lock();
					update_current_msec(7);
					HalconCpp::GetSystemTime(&system_time_msec, &system_time_sec, &system_time_min, &system_time_hour, 0, 0, 0, 0);
					log_mtx.unlock();
					timeBeginPeriod(1);  // 设置精度为1毫秒
					::Sleep(15);          // 当前线程挂起一毫秒
					timeEndPeriod(1);    // 结束精度设置
					write_plc_func(ng_addr, false, false);
					
				}
				else if (result_status == 0) {
					write_plc_func(ok_addr, true, false);
					log_mtx.lock();
					update_current_msec(8);
					HalconCpp::GetSystemTime(&system_time_msec, &system_time_sec, &system_time_min, &system_time_hour, 0, 0, 0, 0);
					log_mtx.unlock();
					timeBeginPeriod(1);  // 设置精度为1毫秒
					::Sleep(15);          // 当前线程挂起一毫秒
					timeEndPeriod(1);    // 结束精度设置
					write_plc_func(ok_addr, false, false);
				}
				//log_mtx.lock();
				//update_current_msec(2);
				//HalconCpp::GetSystemTime(&system_time_msec, &system_time_sec, &system_time_min, &system_time_hour, 0, 0, 0, 0);
				//log_mtx.unlock();
				ll.lPush(REDIS_WRITE_KEY, string(test));

			}
			catch (exception e) {
				cout << e.what() << endl;
			}
			//std::cout << "get taichi result : " << **out << std::endl;
		}
		if (hDllInst > 0)
			FreeLibrary(hDllInst);
		//调用plc disconnect代码 //暂时没有
		disconnect_plc_func();
		return 0;
	}
	catch (HException& exception)
	{
		fprintf(stderr, "  Error #%u in %s: %s\n", exception.ErrorCode(),
			exception.ProcName().TextA(),
			exception.ErrorMessage().TextA());
	}
	catch (exception e) {
		ll.log(e.what());
		return 1;
	}
}

int main(int argc, char** argv)
{
	LibraryLoaderMT llmt;
	HalconCpp::GetSystemTime(&system_time_msec, &system_time_sec, &system_time_min, &system_time_hour, 0, 0, 0, 0);
	update_current_msec(0);
	int get_cmd_num_from_redis;
	try {
		Redis redis = Redis("tcp://127.0.0.1:6379");
		StringView cmd_num_key = REDIS_COMMAND_NUMBER;
		auto cmd_num = redis.get(cmd_num_key);
		get_cmd_num_from_redis = BaseFunctions::Str2Int(cmd_num.value());
	}
	catch (...) {
		get_cmd_num_from_redis = -1;
	}
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&thread_save_log, NULL, 0, 0);
	ll.log(PROGRAM_COMPLIRE_VERSION);
	std::cout << "Hello World! Welcome to library loader, pls select library by num\n";
	std::cout << "[0] exit program \n";
	std::cout << "[1] winding test! \n";
	std::cout << "[2] Image test! \n";
	std::cout << "[3] create calibration file! \n";
	std::cout << "[4] grab calibration image! \n";
	std::cout << "[5] concat image manual, ONLY IN DEBUG MODE! \n";
	std::cout << "[8] jmj test system function! \n";
	std::cout << "[9] xy test system function! \n";
	while (true) {
		if (get_cmd_num_from_redis > 0 && get_cmd_num_from_redis < 100)
			index = get_cmd_num_from_redis;
		else
			std::cin >> index;
		ll.log("index : " + BaseFunctions::Int2Str(index));
		if (index == 0) return 0;
		HImage image;
		try {
			switch (index)
			{
			case 1:
				llmt.call_library_by_num(index);
				break;
			case 2:
				char* args[8];
				char status[30];
				strcpy_s(status, 30, "{\"status\": 0,\"mode\" : 0}");
				args[0] = &status[0];

				ll.runHalconLib(2, args, "d:/images/trigger_concat_400");
				break;
			case 3:
				ll.runCalibration();
				break;
			case 4:
				ll.runCalibrationCameraLib();
				break;
			case 5:
				ll.run_manual_concat_image();
				break;
			case 8:
				ll.run_camera_no_delegate_no_processing();
				break;
			case 9:
				ll.run_camera_with_delegate_no_processing();
				break;
			case CMD_NUM_FOR_LIB_LOADER_PLC_DEBUG:
				llmt.call_library_by_num(index);
				break;
			default:
				llmt.call_library_by_num(index);
				break;
			}
		}
		catch (HException& exception)
		{
			fprintf(stderr, "  Error #%u in %s: %s\n", exception.ErrorCode(),
				exception.ProcName().TextA(),
				exception.ErrorMessage().TextA());
		}
		catch (const char* err) {
			fprintf(stderr, "  Error #%s\n", err);
		}
		catch (...) {
			//to do list
		}

		get_cmd_num_from_redis = -1;
		/* ↓↓↓↓↓↓以下只是测试相机采集 ， 可以屏蔽 ↓↓↓↓↓↓
		HObject ho_Image;
		GenImage1Extern(&ho_Image, "byte", 1920, 1080, (Hlong)(ho_data.getImage()), NULL);
		HImage saveImage = ho_Image;
		saveImage.WriteImage("jpg", 0, "d:/grabs/libTest.jpg");
		 ↑↑↑↑↑↑以上只是测试相机采集 ， 可以屏蔽 ↑↑↑↑↑↑*/

	}
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

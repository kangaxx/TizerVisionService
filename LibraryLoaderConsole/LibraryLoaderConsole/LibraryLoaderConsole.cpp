// LibraryLoaderConsole.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <redis++.h>
#include <cstdio>
#include <unordered_set>
#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <UserEnv.h>

#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/serialization_c11.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/halconUtils.h"
#include "../../../hds/Logger.h"
#include "../../../hds/configHelper.h"
#include "../../../hds/JsonHelper.h"

#include "SerialPort.h"

#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>
#define PROGRAM_COMPLIRE_VERSION "Console program, version 1.20218.09"

#ifdef PYLON_WIN_BUILD
#   include <pylon/PylonGUI.h>

// Include file to use pylon universal instant camera parameters.
#	include <pylon/BaslerUniversalInstantCamera.h>
#endif
#define WINDING_NG_RESULT_KEY "IsNg"
#define WINDING_NG_RESULT_ISNG "0"
using namespace fastdelegate;
using namespace HalconCpp;
using namespace sw::redis;
class LibraryLoader;

void delegateFunction(char*);
typedef void (*halconFunc)(int, char* [], const char*, char**);
typedef HImage(*cameraWork)(int, char* []);
typedef bool (*calibrationWork)(int, char* []);
typedef void (*callHalconFunc)(char*);
typedef void (*setHalconFunctionDelegate)(void (LibraryLoader::*)(int, char* [], HBYTE[]));
typedef void (*setHalconFunction)(callHalconFunc);
typedef void (*call_image_concat)();

using namespace commonfunction_c;
static int index;

//如果电芯不合格，需要触发485继电器来剔除，下面的功能就是触发485
void switchTrigger485(int port)
{
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
	return;
}

//循环读取redis列表看看前台有没有数据返回


unsigned long readRedisProc(void* lpParameter) {
	Logger l("d:");
	l.Log("start read Redis proc");
	Redis redis = Redis("tcp://127.0.0.1:6379");
	StringView key = REDIS_READ_KEY;
	while (true) {
		try {
			auto value = redis.rpop(key);
			l.Log(value.value());
			JsonHelper jh(value.value());
			cout << "is ng pos :" << jh.search(WINDING_NG_RESULT_KEY).find(WINDING_NG_RESULT_ISNG) << endl;
			if (jh.search(WINDING_NG_RESULT_KEY).find(WINDING_NG_RESULT_ISNG) == 0)
				switchTrigger485(1);
		}
		catch (char* err) {
			l.Log(err);
		}
		catch (...) {
			//do nothing
		}
		Sleep(2000);
	}//while
	return 0;
}

class LibraryLoader {
public:
	//读取算法动态链接库
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
			l.Log("Console try to run halcon function!");
			func(param_num_, source, image.c_str(), out);
			l.Log(string(*out));
			lPush(REDIS_WRITE_KEY, string(*out));
		}
		catch (...) {
		}
		//std::cout << "get taichi result : " << **out << std::endl;
		if (hDllInst > 0)
			FreeLibrary(hDllInst);
		return;
	}

	//读取相机动态链接库
	HImage runCameraLib() {
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&readRedisProc, NULL, 0, 0);
		HINSTANCE hDllInst;
		configHelper ch("c:\\tizer\\config.ini", CT_JSON);
		Logger l("d:");
		StringView key = REDIS_LIST_CALIBRATION_KEY;
		Redis redis = Redis("tcp://127.0.0.1:6379");
		auto value = redis.rpop(key);
		int param_num = 0;
		try {
			JsonHelper jh(value.value());
			calibration_line_top_ = BaseFunctions::Str2Int(jh.search(JSON_CALIBRATION_TOP_KEY), 0);
			calibration_line_bottom_ = BaseFunctions::Str2Int(jh.search(JSON_CALIBRATION_BOTTOM_KEY), 1);
			camera_width_ = BaseFunctions::Str2Int(jh.search(JSON_CAMERA_PARAM_WIDTH), 1920);
			camera_height_ = BaseFunctions::Str2Int(jh.search(JSON_CAMERA_PARAM_WIDTH), 1080);
			package_delay_ = BaseFunctions::Str2Int(jh.search(JSON_CAMERA_PARAM_PACKAGE_DELAY), 1000);
			package_size_ = BaseFunctions::Str2Int(jh.search(JSON_CAMERA_PARAM_PACKAGE_SIZE), 7000);
			exposure_time_ = BaseFunctions::Str2Int(jh.search(JSON_CAMERA_PARAM_EXPOSURE_TIME), 3000);
			center_ = BaseFunctions::Str2Int(jh.search(JSON_CAMERA_PARAM_CENTER), 3000);
			param_num_ = 3;
		}
		catch (...) {
			calibration_line_top_ = 0;
			calibration_line_bottom_ = 0;
			l.Log("Calibarton line value read error!"); //test log
			param_num_ = 1;
		}
		l.Log(ch.findValue("cameraLibrary", string("string")));
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
		setHalconFunction setFunc = NULL;
		setFunc = (setHalconFunction)GetProcAddress(hDllInst, "setHalconFunction");
		if (setFunc == 0) {
			FreeLibrary(hDllInst);
			throw "Load camera library set halcon function failed!";
		}
		//测试委托
		setFunc(delegateFunction);
		string cameraLeftName = ch.findValue("cameraLeftName", string("string"));
		string cameraMidName = ch.findValue("cameraMidName", string("string"));
		string cameraRightName = ch.findValue("cameraRightName", string("string"));
		char* in[9];
		char leftCamera[50], midCamera[50], rightCamera[50], camera_width[10], camera_height[10], package_delay[10], 
			package_size[10], exposure_time[10], center[10];
		strcpy_s(leftCamera, cameraLeftName.c_str());
		strcpy_s(midCamera, cameraMidName.c_str());
		strcpy_s(rightCamera, cameraRightName.c_str());
		commonfunction_c::BaseFunctions::Int2Chars(camera_width_, camera_width);
		commonfunction_c::BaseFunctions::Int2Chars(camera_height_, camera_height);
		commonfunction_c::BaseFunctions::Int2Chars(package_delay_, package_delay);
		commonfunction_c::BaseFunctions::Int2Chars(package_size_, package_size);
		commonfunction_c::BaseFunctions::Int2Chars(center_, center);
		commonfunction_c::BaseFunctions::Int2Chars(exposure_time_, exposure_time);
		in[0] = &leftCamera[0];
		in[1] = &midCamera[0];
		in[2] = &rightCamera[0];
		in[3] = &camera_width[0];
		in[4] = &camera_height[0];
		in[5] = &package_delay[0];
		in[6] = &package_size[0];
		in[7] = &center[0];
		in[8] = &exposure_time[0];
		HImage image = cameraWorkFunc(0, in);
		return image;
	}


	HImage runCalibrationCameraLib() {
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&readRedisProc, NULL, 0, 0);
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
		cout << "cameraLeftName: " << cameraLeftName << endl;
		char* in[5];
		char leftCamera[50], midCamera[50], rightCamera[50];
		strcpy_s(leftCamera, cameraLeftName.c_str());
		strcpy_s(midCamera, cameraMidName.c_str());
		strcpy_s(rightCamera, cameraRightName.c_str());
		in[0] = &leftCamera[0];
		in[1] = &midCamera[0];
		in[2] = &rightCamera[0];
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
private:
	int calibration_line_top_, calibration_line_bottom_, param_num_;
	int camera_width_, camera_height_, package_size_, package_delay_, center_, exposure_time_;
};

static LibraryLoader ll;

int main()
{
	Logger l("d:");
	l.Log(PROGRAM_COMPLIRE_VERSION);
	l.Log(BaseFunctions::ws2s(BaseFunctions::GetWorkPath()));
	std::cout << "Hello World! Welcome to library loader, pls select library by num\n";
	std::cout << "[0] exit program \n";
	std::cout << "[1] winding test! \n";
	std::cout << "[2] Image test! \n";
	std::cout << "[3] create calibration file! \n";
	std::cout << "[4] grab calibration image! \n";
	std::cout << "[5] concat image manual, ONLY IN DEBUG MODE! \n";
	while (true) {
		std::cin >> index;
		std::cout << "selected index :" << index << std::endl;
		if (index == 0) return 0;


		HImage image;
		try {
			switch (index)
			{
			case 1:
				image = ll.runCameraLib();
				Pylon::PylonTerminate();
				break;
			case 2:
				char* args[8];
				char status[10];
				strcpy_s(status, 10, BaseFunctions::Int2Str(CONCAT_IMAGE_SUCCESS).c_str());
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
			default:
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

		/* ↓↓↓↓↓↓以下只是测试相机采集 ， 可以屏蔽 ↓↓↓↓↓↓
		HObject ho_Image;
		GenImage1Extern(&ho_Image, "byte", 1920, 1080, (Hlong)(ho_data.getImage()), NULL);
		HImage saveImage = ho_Image;
		saveImage.WriteImage("jpg", 0, "d:/grabs/libTest.jpg");
		 ↑↑↑↑↑↑以上只是测试相机采集 ， 可以屏蔽 ↑↑↑↑↑↑*/

	}

	return 0;
}

void delegateFunction(char* msg) {
	Logger l("d:");
	l.Log("Delegate function start!"); //test log
	l.Log(string(msg)); //test log
	ll.lPush("to_test", msg);
	JsonHelper jh;
	jh.initial(string(msg));
	string key = "image";
	//cout << "json initial by string , string : \"" << msg << "\" , key is : \"" << key << "\", value is :\"" << jh.search(key) << "\"" << endl;
	//调用算法
	try {
		char* args[8];
		char status[10];
		strcpy_s(status, 10, jh.search("status").c_str());
		args[0] = &status[0];
		ll.runHalconLib(1, args, jh.search(key));
	}
	catch (...) {

	}
	return;
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

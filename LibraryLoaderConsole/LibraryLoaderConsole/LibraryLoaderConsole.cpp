// LibraryLoaderConsole.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <redis++.h>
#include <cstdio>
#include <unordered_set>

#include <iostream>
#include <Windows.h>

#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/serialization_c11.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/halconUtils.h"
#include "../../../hds/Logger.h"
#include "../../../hds/configHelper.h"
#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>
#define PROGRAM_COMPLIRE_VERSION "camera worker, version 2111011630"

#ifdef PYLON_WIN_BUILD
#   include <pylon/PylonGUI.h>

// Include file to use pylon universal instant camera parameters.
#	include <pylon/BaslerUniversalInstantCamera.h>
#endif
using namespace fastdelegate;
using namespace HalconCpp;
using namespace sw::redis;
class LibraryLoader;
 
void delegateFunction(char*);
typedef char** (*halconFunc)(int, char*[], HImage, char**);
typedef HImage (*cameraWork)(int, char* []);
typedef void (*callHalconFunc)(char*);
typedef void (*setHalconFunctionDelegate)(void (LibraryLoader::*)(int, char* [], HBYTE[]));
typedef void (*setHalconFunction)(callHalconFunc);

using namespace commonfunction_c;


class LibraryLoader {
public:
	//读取算法动态链接库
	void runHalconLib(int argc, char* in[], HImage image){
		int x = argc;
		HINSTANCE hDllInst;
		configHelper ch("c:\\tizer\\config.ini", CT_JSON);
		hDllInst = LoadLibrary(LPCTSTR(BaseFunctions::s2ws(ch.findValue("halconLibrary", string("string"))).c_str()));
		if (hDllInst == 0)
			return;
		halconFunc func = NULL;
		func = (halconFunc)GetProcAddress(hDllInst, "halconAction");
		if (func == 0)
			return;
		int burr_limit = 15;
		int grayMin = 140;
		int grayMax = 255;
		int localImage = ch.findValue("localImage", 1);
		char* source[8];
		//设置输入参数
		//
		int width = 1920; //实际参数需要参看相机情况，读取本地文件时设置为0
		int height = 1080; // 同上
		int polesWidth = 10;
		source[0] = (char*)(&burr_limit);
		source[1] = (char*)(&grayMin);
		source[2] = (char*)(&grayMax);
		source[3] = (char*)(&width);
		source[4] = (char*)(&height);
		source[5] = NULL;
		source[6] = (char*)(&polesWidth);
		source[7] = (char*)(&localImage);
		//初始化输出参数
		char buffer[INT_HALCON_BURR_RESULT_SIZE] = { '\0' };
		char** out = new char* ();
		*out = &buffer[0];
		
		func(6, source, image, out);
		std::cout << "get taichi result : " << **out << std::endl;
		if (hDllInst > 0)
			FreeLibrary(hDllInst);
		return;
	}

	//读取相机动态链接库

	HImage runCameraLib() {
		HINSTANCE hDllInst;
		configHelper ch("c:\\tizer\\config.ini", CT_JSON);
		hDllInst = LoadLibrary(LPCTSTR(BaseFunctions::s2ws(ch.findValue("cameraLibrary", string("string"))).c_str()));
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
};

static LibraryLoader ll;

int main()
{

	//创建共享内存
	HANDLE hFile = CreateFile(L"Recv1.zip",
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE,
		0, 0x4000000, NULL);
	// 释放文件内核对象
	CloseHandle(hFile);
	// 设定大小、偏移量等参数
	__int64 qwFileSize = 0x4000000;
	__int64 qwFileOffset = 0;
	__int64 T = 600;
	DWORD dwBytesInBlock = 1000;

	// 将文件数据映射到进程的地址空间
	PBYTE pbFile = (PBYTE)MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		(DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF), dwBytesInBlock);
	//

	std::cout << "Hello World! Welcome to library loader, pls select library by num\n";
	std::cout << "[0] exit program \n";
	std::cout << "[1] halcon taichi! \n";
	int index;
	while (true) {
		std::cin >> index;
		std::cout << "selected index :" << index << std::endl;
		if (index == 0) return 0;

		
		HImage image;
		try {
			image = ll.runCameraLib();
			Pylon::PylonTerminate();
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
		//调用算法
		try {
			ll.runHalconLib(index, NULL, image);
		}
		catch (...) {

		}
	}

	// 从进程的地址空间撤消文件数据映像
	UnmapViewOfFile(pbFile);
	return 0;
}



void delegateFunction(char* msg) {
	ll.lPush("some key", msg);
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

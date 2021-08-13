#include "pch.h"
#include "libraryManager.h"

LibraryManager::LibraryManager()
{
}

void LibraryManager::runCameraLib()
{
	HINSTANCE hDllInst;
	configHelper ch("c:\\tizer\\config.ini", CT_JSON);
	hDllInst = LoadLibrary(LPCTSTR(BaseFunctions::s2ws(ch.findValue("cameraLibrary", string("string"))).c_str()));
	if (hDllInst == 0)
		return;
	cameraWork cameraWorkFunc = NULL;
	cameraWorkFunc = (cameraWork)GetProcAddress(hDllInst, "cameraWorker");
	if (cameraWorkFunc == 0) {
		FreeLibrary(hDllInst);
		return;
	}
	setHalconFunctionDelegate setDelegate = NULL;
	setDelegate = (setHalconFunctionDelegate)GetProcAddress(hDllInst, "setHalconFunction");
	if (setDelegate == 0) {
		FreeLibrary(hDllInst);
		return;
	}
	setDelegate(&LibraryManager::runHalconLib);
	char* out = new char[10];
	cameraWorkFunc(0, NULL, &out);
	delete[] out;
}

void LibraryManager::runHalconLib(int argc, char* in[], const HBYTE* image)
{
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
	int width = 0; //实际参数需要参看相机情况，读取本地文件时设置为0
	int height = 0; // 同上
	int polesWidth = 10;
	source[0] = (char*)(&burr_limit);
	source[1] = (char*)(&grayMin);
	source[2] = (char*)(&grayMax);
	source[3] = (char*)(&width);
	source[4] = (char*)(&height);
	source[5] = (char*)(image);
	source[6] = (char*)(&polesWidth);
	source[7] = (char*)(&localImage);
	//初始化输出参数
	char buffer[INT_HALCON_BURR_RESULT_SIZE] = { '\0' };
	char** out = new char* ();
	*out = &buffer[0];
	func(6, source, out);
	if (hDllInst > 0)
		FreeLibrary(hDllInst);
	return;
}

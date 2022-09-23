#include "LibraryLoaderMT.h"
using namespace sw::redis;
typedef HImage(*cameraWork)(int, char* []);
typedef void (*callHalconFunc)(char*);
typedef void (*setHalconFunction)(callHalconFunc);
void LibraryLoaderMT::call_library_by_num(int num)
{
	switch (num)
	{
	case CMD_NUM_FOR_LIB_LOADER_CAMERA_NO_DELEGATE:
		call_camera_no_delegate();
		break;
	case CMD_NUM_FOR_LIB_LOADER_PLC_DEBUG:
		//调试plc通信
		if (!call_lib_loader_plc_debug())
			cout << "will change to log" << endl;
		break;
	case CMD_NUM_FOR_LIB_LOADER_HEART_BEAT:
		//启动心跳进程
		call_heart_beat();
		break;
	default:
		break;
	}
	return;
}

HImage LibraryLoaderMT::call_camera_no_delegate()
{
	//读取相机动态链接库

		//CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&readRedisProc, NULL, 0, 0);
		HINSTANCE hDllInst;
		//configHelper ch("c:\\tizer\\config.ini", CT_JSON);
		StringView key = REDIS_LIST_CALIBRATION_KEY;
		Redis redis = Redis("tcp://127.0.0.1:6379");
		auto value = redis.get(key);
		_logger->Log(value.value());
		char* in[10];
		char leftCamera[50], midCamera[50], rightCamera[50], camera_width[50], camera_height[50], package_delay[50],
			package_size[50], exposure_time[50], center[50], param_num[50];
		try {
			JsonHelper jh(value.value());
			strncpy_s(camera_width, 50, jh.search(JSON_CAMERA_PARAM_WIDTH).c_str(), 49);
			strncpy_s(camera_height, 50, jh.search(JSON_CAMERA_PARAM_HEIGHT).c_str(), 49);
			strncpy_s(package_delay, 50, jh.search(JSON_CAMERA_PARAM_PACKAGE_DELAY).c_str(), 49);
			strncpy_s(package_size, 50, jh.search(JSON_CAMERA_PARAM_PACKAGE_SIZE).c_str(), 49);
			strncpy_s(center, 50, jh.search(JSON_CAMERA_PARAM_CENTER).c_str(), 49);
			strncpy_s(exposure_time, 50, jh.search(JSON_CAMERA_PARAM_EXPOSURE_TIME).c_str(), 49);
			strncpy_s(param_num, 50, BaseFunctions::Int2Str(STANDARD_CAMERA_MODE).c_str(), 49);
			in[0] = &param_num[0];
			in[1] = &leftCamera[0];
			in[2] = &midCamera[0];
			in[3] = &rightCamera[0];
			in[4] = &camera_width[0];
			in[5] = &camera_height[0];
			in[6] = &package_delay[0];
			in[7] = &package_size[0];
			in[8] = &center[0];
			in[9] = &exposure_time[0];
		}
		catch (...) {
			_logger->append_log("Calibarton line value read error!"); //test log
		}
		hDllInst = LoadLibrary(BaseFunctions::s2ws(_system_config.search("cameraLibrary")).c_str());
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
		//setFunc(delegateFunction);
		string cameraLeftName = _system_config.search("cameraLeftName");
		string cameraMidName = _system_config.search("cameraMidName");
		string cameraRightName = _system_config.search("cameraRightName");


		strcpy_s(leftCamera, cameraLeftName.c_str());
		strcpy_s(midCamera, cameraMidName.c_str());
		strcpy_s(rightCamera, cameraRightName.c_str());
		HImage image = cameraWorkFunc(0, in);
		return image;
}

void LibraryLoaderMT::log(const char* l)
{
	_logger->append_log(l);
}

void LibraryLoaderMT::log(string l)
{
	log(l.c_str());
}

bool LibraryLoaderMT::call_lib_loader_plc_debug()
{
	HINSTANCE hDllInst;
	hDllInst = LoadLibrary(BaseFunctions::s2ws(_system_config.search("hslLibrary")).c_str());
	if (hDllInst == 0) {
		//_log->Log("Load hsl library fail!");
		return false;
	}
	connect_plc connect_plc_func = (connect_plc)GetProcAddress(hDllInst, "connect_plc");
	if (connect_plc_func == 0) {
		//_log->Log("Load hsl library function connect_plc fail!");
		return false;
	}
	write_plc_bool write_plc_func = (write_plc_bool)GetProcAddress(hDllInst, "write_plc_bool");
	if (write_plc_func == 0) {
		//_log->Log("Load hsl library function write_plc_bool fail!");
		return false;
	}
	disconnect_plc disconnect_plc_func = (disconnect_plc)GetProcAddress(hDllInst, "disconnect_plc");
	if (disconnect_plc_func == 0) {
		//_log->Log("Load hsl library function write_plc_bool fail!");
		return false;
	}

	const char* addr = "w100.07";
	if (!connect_plc_func()) {
		//_log->Log("connect to plc fail!");
		return false;
	}

	int end_cmd = 0;
	while (0 == end_cmd) {
		if (!write_plc_func(addr, true, true)) {
			//_log->Log("write to plc fail!");
			return false;
		}
		Sleep(10);
		if (!write_plc_func(addr, false, true)) {
			//_log->Log("write to plc fail!");
			return false;
		}
		cin >> end_cmd;
	}
	if (!disconnect_plc_func()) {
		//_log->Log("disconnect fail!");
		return false;
	}
	return true;
}

void LibraryLoaderMT::heart_beat_thread(void* lpParameter)
{
	int i = 0;
	Redis redis = Redis("tcp://127.0.0.1:6379");
	while (true) {
		if (i > 9999)
			i = 0;
		StringView key = "heartbeat";
		redis.set(key, BaseFunctions::Int2Str(i++));
		Sleep(2000);
	}
	return;
}

void LibraryLoaderMT::call_heart_beat()
{
	std::thread heart_beat_thd(&LibraryLoaderMT::heart_beat_thread, this, (void*)NULL);
	heart_beat_thd.detach();
}

void LibraryLoaderMT::initial()
{
	if (LOADER_MODE::LM_CONSOLE_MODE == _mode) {
		_system_config.initialByFile("c:\\tizer\\config.ini");
		_initialed = true;
#ifndef DEBUG_WORK_PATH
		string log_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + _system_config.search("log_dir");
#else
		string log_dir = DEBUG_WORK_PATH + "\\" + _ch->findValue("log_dir", string("string"));
#endif
		_logger = ConcurrentLogger::get_instance(log_dir.c_str());
	}
}

unsigned long LibraryLoaderMT::readRedisProc(void* lpParameter) {
	Redis redis = Redis("tcp://127.0.0.1:6379");
	StringView key = REDIS_READ_KEY;
	while (true) {
		try {
			auto value = redis.rpop(key);
			JsonHelper jh(value.value());
			cout << "is ng pos :" << jh.search(WINDING_NG_RESULT_KEY).find(WINDING_NG_RESULT_ISNG) << endl;
			//if (jh.search(WINDING_NG_RESULT_KEY).find(WINDING_NG_RESULT_ISNG) == 0)
				//switchTrigger485(ELECTRIC_RELAY_COM_NUM);
		}
		catch (char* err) {
			cout << err << endl;
		}
		catch (...) {
			//do nothing
		}
		Sleep(2000);
	}//while
	return 0;
}

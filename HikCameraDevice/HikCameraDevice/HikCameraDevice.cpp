#include "HikCameraDevice.h"

void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	if (pFrameInfo)
	{
		unsigned short width, height;
		width = pFrameInfo->nWidth;
		height = pFrameInfo->nHeight;
		printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
			width, height, pFrameInfo->nFrameNum);
		HObject ho_image = halconUtils::HalconUtils::char_to_halcon_image((char*)pData, width, height);
		HImage image = (HImage)ho_image;
		char json[256];
		sprintf_s(json, 256, "{\"job_id\":%d, \"camera_id\":%d, \"msec\":%d}",
			((HikCameraInfo*)pUser)->get_captured_id(),
			((HikCameraInfo*)pUser)->get_camera_id(),
			g_msec);
		((HikCameraInfo*)pUser)->inc_captured_id();
		g_delegate_function(json, image);
	}
}

CameraDevicesBase* get_camera_devices(const char* config)
{
	if (g_devices == NULL)
		g_devices = new HikCameraDevice(config);

	return (CameraDevicesBase*)g_devices;
}

void free_camera_devices()
{

}

void set_camera_captured_delegate(cameraCaputredDelegate delegate)
{
	g_delegate_function = delegate;
}

void HikCameraInfo::do_capture(HalconCpp::HImage& image)
{

}

void HikCameraInfo::set_logger(Logger& log)
{
	_log = &log;
}

void HikCameraInfo::initial()
{
}

HikCameraDevice::HikCameraDevice(const char* config)
{
	_config = string(config);
	JsonHelper jh(config);
#ifndef DEBUG_WORK_PATH
	string log_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + jh.search("log_dir");
#else
	string log_dir = DEBUG_WORK_PATH + jh.search("log_dir");
#endif
   	_log = new Logger(log_dir);
	_log->Log("HikRobot camera library version " + string(STR_VER_FULL_RC));
	string mode = jh.search("trigger_mode");
	if (mode.find("true") != mode.npos)
		set_trigger_mode(true);
	else
		set_trigger_mode(false);
	_camera_count = jh.read_array<string>("camera_name", _camera_names);	
	memset(&_device_list, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

	// ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
	int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &_device_list);
	if (MV_OK != nRet)
	{
		_log->Log("EnumDevicesByGenTL fail");
		return;
	}
}

void HikCameraDevice::camera_capture(int camera_num_in_list, int camera_id) {
	try {
		int nRet;
		void* handle = NULL;
		nRet = MV_CC_CreateHandle(&handle, _device_list.pDeviceInfo[camera_num_in_list]);
		if (MV_OK != nRet)
		{
			printf("Create Handle fail! nRet [0x%x]\n", nRet);
		}
		nRet = MV_CC_OpenDevice(handle);
		if (MV_OK != nRet)
		{
			printf("Open Device fail! nRet [0x%x]\n", nRet);
		}

		// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
		if (_device_list.pDeviceInfo[camera_num_in_list]->nTLayerType == MV_GIGE_DEVICE)
		{
			int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
			if (nPacketSize > 0)
			{
				nRet = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
				if (nRet != MV_OK)
				{
					printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
				}
			}
			else
			{
				printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
			}
		}

		// ch:设置触发模式为on | eb:Set trigger mode as on
		if (is_trigger_mode()) {
			nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_ON);
			MV_CC_SetEnumValue(handle, "TriggerSource", MV_TRIGGER_SOURCE_LINE0);
		}
		else {
			nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
			MV_CC_SetEnumValue(handle, "TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
		}
		if (MV_OK != nRet)
		{
			printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
		}

		// ch:注册抓图回调 | en:Register image callback
		HikCameraInfo* camera_info = new HikCameraInfo(_camera_names.at(camera_id));
		camera_info->set_camera_id(camera_id);
		nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, (void*)camera_info);
		if (MV_OK != nRet)
		{
			printf("Register Image CallBack fail! nRet [0x%x]\n", nRet);
		}

		// ch:开始取流 | en:Start grab image
		nRet = MV_CC_StartGrabbing(handle);
		if (MV_OK != nRet)
		{
			printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
		}
		//使用回调函数时停止后续操作，否则无法开发多相机同时回调
		return; 
		//回调函数保存到himage后，会修改captured 状态
		while (!camera_info->is_captured()) {
			//没有获取到照片就会一直等待
		}
		// ch:停止取流 | en:Stop grab image
		nRet = MV_CC_StopGrabbing(handle);
		if (MV_OK != nRet)
		{
			printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
		}

		// ch:注销抓图回调 | en:Unregister image callback
		nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL);
		if (MV_OK != nRet)
		{
			printf("Unregister Image CallBack fail! nRet [0x%x]\n", nRet);
		}

		// ch:关闭设备 | en:Close device
		nRet = MV_CC_CloseDevice(handle);
		if (MV_OK != nRet)
		{
			printf("Close Device fail! nRet [0x%x]\n", nRet);
		}

		// ch:销毁句柄 | en:Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
		}
		return;
	}
	catch (...) {
		_log->Log("Error ,catched by camera capture.");
		return;
	}
}

bool HikCameraDevice::do_capture(int index, HalconCpp::HImage& image)
{
	try {
		int camera_num_in_list = -1;
		int nRet;
		for (unsigned int j = 0; j < _camera_count; j++) {
			camera_num_in_list = -1;
			for (unsigned int i = 0; i < _device_list.nDeviceNum; i++)
			{
				char strUserName[256];
				if (strcmp(_camera_names.at(j).c_str(), (char*)_device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber) != 0)
				{
					sprintf_s(strUserName, 256, "Dev[%d]:%s %s (%s)", i, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chModelName, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);

				}
				else
				{
					sprintf_s(strUserName, 256, "Dev[%d]:%s %s (%s)", i, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chModelName, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);
					camera_num_in_list = i;
					_log->Log(strUserName);
					camera_capture(camera_num_in_list, j);

				}
			}
			if (camera_num_in_list < 0 || camera_num_in_list >= _camera_count) {
				_log->Log("Camera not found , pls check config file!");
			}
		}
		return true;
	}
	catch (...) {
		return false;
	}
}

string HikCameraDevice::get_camera_tag(int index)
{
	return _camera_names.at(index);
}

int HikCameraDevice::get_devices_num()
{
	return _camera_count;
}

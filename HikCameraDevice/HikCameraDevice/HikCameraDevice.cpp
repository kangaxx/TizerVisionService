#include "HikCameraDevice.h"

void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	if (pFrameInfo)
	{
		printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
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
	/*     ���ղ��Դ���     */
	memset(&_device_list, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

	// ch:ö�������������豸 | en:Enumerate all devices within subnet
	int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &_device_list);
	if (MV_OK != nRet)
	{
		_log->Log("EnumDevicesByGenTL fail");
		return;
	}

	for (unsigned int i = 0; i < _device_list.nDeviceNum; i++)
	{
		char strUserName[256];
		if (strcmp("", (char*)_device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName) != 0)
		{
			sprintf_s(strUserName, 256, "Dev[%d]:%s %s (%s)", i, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chModelName, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);
		}
		else
		{
			sprintf_s(strUserName, 256, "Dev[%d]:%s %s (%s)", i, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chModelName, _device_list.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);
		}
		_log->Log(strUserName);
	}
	void* handle = NULL;
	nRet = MV_CC_CreateHandle(&handle, _device_list.pDeviceInfo[0]);
	if (MV_OK != nRet)
	{
		printf("Create Handle fail! nRet [0x%x]\n", nRet);
	}
	nRet = MV_CC_OpenDevice(handle);
	if (MV_OK != nRet)
	{
		printf("Open Device fail! nRet [0x%x]\n", nRet);
	}

	// ch:̽��������Ѱ���С(ֻ��GigE�����Ч) | en:Detection network optimal package size(It only works for the GigE camera)
	if (_device_list.pDeviceInfo[0]->nTLayerType == MV_GIGE_DEVICE)
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

	// ch:���ô���ģʽΪon | eb:Set trigger mode as on
	nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_ON);
	if (MV_OK != nRet)
	{
		printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
	}

	// ch:ע��ץͼ�ص� | en:Register image callback
	nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle);
	if (MV_OK != nRet)
	{
		printf("Register Image CallBack fail! nRet [0x%x]\n", nRet);
	}

	// ch:��ʼȡ�� | en:Start grab image
	nRet = MV_CC_StartGrabbing(handle);
	if (MV_OK != nRet)
	{
		printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
	}

	//�����У�ȡ��������Ҫ���밴��
	std::cin >> nRet;
	// ch:ֹͣȡ�� | en:Stop grab image
	nRet = MV_CC_StopGrabbing(handle);
	if (MV_OK != nRet)
	{
		printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
	}

	// ch:ע��ץͼ�ص� | en:Unregister image callback
	nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL);
	if (MV_OK != nRet)
	{
		printf("Unregister Image CallBack fail! nRet [0x%x]\n", nRet);
	}

	// ch:�ر��豸 | en:Close device
	nRet = MV_CC_CloseDevice(handle);
	if (MV_OK != nRet)
	{
		printf("Close Device fail! nRet [0x%x]\n", nRet);
	}

	// ch:���پ�� | en:Destroy handle
	nRet = MV_CC_DestroyHandle(handle);
	if (MV_OK != nRet)
	{
		printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
	}
	/*  ���ղ��Դ���  end   */
}

bool HikCameraDevice::do_capture(int index, HalconCpp::HImage& image)
{
	return false;
}

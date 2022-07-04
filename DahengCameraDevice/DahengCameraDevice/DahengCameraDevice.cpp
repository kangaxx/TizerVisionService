#include "DahengCameraDevice.h"
#define DEBUG_WORK_PATH "Z:\\TizerVisionService\\LibraryLoaderConsole\\x64\\Debug\\" //代码调试模式下工作目录
CameraDevicesBase* get_camera_devices(const char* config)
{
	if (g_devices == NULL)
		g_devices = new DahengCameraDevice(config);

	return (CameraDevicesBase*)g_devices;
}

void free_camera_devices()
{

}

DahengCameraDevice::DahengCameraDevice(const char* config)
{

	/**************************************************************/
	/*相机sdk代码基本拍摄功能测试
	gxdeviceinfo_vector gx_devices_info;
	IGXFactory::GetInstance().UpdateDeviceList(1000, gx_devices_info);
	if (0 == gx_devices_info.size())
		cout << "ERROR: no device found!" << endl;
	else {
		//"FAK22040242"
		cout << gx_devices_info[0].GetSN().c_str() << endl;
		CGXDevicePointer ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(
			gx_devices_info[0].GetSN(),
			GX_ACCESS_EXCLUSIVE);
		CGXStreamPointer ObjStreamPtr = ObjDevicePtr->OpenStream(0);
		CGXFeatureControlPointer ObjFeatureControlPtr =
			ObjDevicePtr->GetRemoteFeatureControl();
		//获取流层属性控制器
		CGXFeatureControlPointer objStreamFeatureControlPtr =
			ObjStreamPtr->GetFeatureControl();
		//提高网络相机的采集性能,设置方法参考以下代码（目前只有千兆网系列相机支持设置最优包长）。
		GX_DEVICE_CLASS_LIST objDeviceClass =
			ObjDevicePtr->GetDeviceInfo().GetDeviceClass();
		if (GX_DEVICE_CLASS_GEV == objDeviceClass) {
			//判断设备是否支持流通道数据包功能
			if (true == ObjFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
			{
				//获取当前网络环境的最优包长值
				int nPacketSize = ObjStreamPtr->GetOptimalPacketSize();
				if (true == ObjFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
				{
					//获取当前网络环境的最优包长值
					int nPacketSize = ObjStreamPtr->GetOptimalPacketSize();

					//将最优包长值设置为当前设备的流通道包长值
					ObjFeatureControlPtr->GetIntFeature(
						"GevSCPSPacketSize")->SetValue(nPacketSize);
				}
			}
		}

		//设置 Buffer 处理模式
		objStreamFeatureControlPtr->GetEnumFeature(
			"StreamBufferHandlingMode")->SetValue("OldestFirst");
		ObjStreamPtr->StartGrab();
		//给设备发送开采命令
		ObjDevicePtr->GetRemoteFeatureControl()->GetCommandFeature("AcquisitionStart")->Execute();
		//采单帧
		CImageDataPointer image_data_ptr;
		image_data_ptr = ObjStreamPtr->GetImage(500);//超时时间使用500ms，用户可以自行设定
		if (image_data_ptr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
		{
			//采图成功而且是完整帧，可以进行图像处理...
			 //准备将buffer转给HImage
			HalconCpp::HImage image;
			image.GenImage1("byte", image_data_ptr->GetWidth(), image_data_ptr->GetHeight(), image_data_ptr->GetBuffer());
			image.WriteImage("jpg", 0, "d:\\1.jpg");
		}
		//停采
		ObjDevicePtr->GetRemoteFeatureControl()->GetCommandFeature("AcquisitionStop")->Execute();
		ObjStreamPtr->StopGrab();
		ObjStreamPtr->Close();
	}

	*/
	/**************************************************************/

	_config = string(config);
	JsonHelper jh(config);
#ifndef DEBUG_WORK_PATH
	string log_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + jh.search("log_dir");
#else
	string log_dir = DEBUG_WORK_PATH + jh.search("log_dir");
#endif
	_log = new Logger(log_dir);
	_log->Log("Daheng camera library version " + string(STR_VER_FULL_RC));

	vector<string> device_sn;
	_camera_count = jh.read_array<string>("device_sn", device_sn);
	_camera_infos = new DahengCameraInfo[_camera_count];
	int heart_beat_time = BaseFunctions::Str2Int(jh.search("hear_beat_time"));

	for (int i = 0; i < _camera_count; ++i) {
		_camera_infos[i].set_searial_num(device_sn.at(i));
		//_camera_infos[i].set_searial_num("FAK22040242");
		_camera_infos[i].set_position(i + 1);
		_camera_infos[i].set_heart_beat_time(heart_beat_time);
		_camera_infos[i].initial();
		_camera_infos[i].set_logger(*_log);
	}

	//设置心跳超时时间 5 分钟
	//object_feature_control->GetIntFeature("GevHeartbeatTimeout")->SetValue(300000);
}

bool DahengCameraDevice::do_capture(int index, HalconCpp::HImage &image)
{
	if (index < _camera_count) {
		_camera_infos[index].do_capture(image);
		return true;
	}
	return false;
}

void DahengCameraInfo::do_capture(HalconCpp::HImage& image)
{
	IGXFactory::GetInstance().Init(); //初始化相机驱动

	gxdeviceinfo_vector gx_devices_info;
	IGXFactory::GetInstance().UpdateDeviceList(1000, gx_devices_info);
	if (0 == gx_devices_info.size())
		cout << "ERROR: no device found!" << endl;
	else {
		cout << "FAK22040242 || " << gx_devices_info[0].GetSN().c_str() << endl;
		CGXDevicePointer ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(
			get_searial_num().c_str(),
			GX_ACCESS_EXCLUSIVE);
		CGXStreamPointer ObjStreamPtr = ObjDevicePtr->OpenStream(0);
		CGXFeatureControlPointer ObjFeatureControlPtr =
			ObjDevicePtr->GetRemoteFeatureControl();
		//获取流层属性控制器
		CGXFeatureControlPointer objStreamFeatureControlPtr =
			ObjStreamPtr->GetFeatureControl();
		//提高网络相机的采集性能,设置方法参考以下代码（目前只有千兆网系列相机支持设置最优包长）。
		GX_DEVICE_CLASS_LIST objDeviceClass =
			ObjDevicePtr->GetDeviceInfo().GetDeviceClass();
		if (GX_DEVICE_CLASS_GEV == objDeviceClass) {
			//判断设备是否支持流通道数据包功能
			if (true == ObjFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
			{
				//获取当前网络环境的最优包长值
				int nPacketSize = ObjStreamPtr->GetOptimalPacketSize();
				if (true == ObjFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
				{
					//获取当前网络环境的最优包长值
					int nPacketSize = ObjStreamPtr->GetOptimalPacketSize();

					//将最优包长值设置为当前设备的流通道包长值
					ObjFeatureControlPtr->GetIntFeature(
						"GevSCPSPacketSize")->SetValue(nPacketSize);
				}
			}
		}

		//设置 Buffer 处理模式
		objStreamFeatureControlPtr->GetEnumFeature(
			"StreamBufferHandlingMode")->SetValue("OldestFirst");
		ObjStreamPtr->StartGrab();
		//给设备发送开采命令
		ObjDevicePtr->GetRemoteFeatureControl()->GetCommandFeature("AcquisitionStart")->Execute();
		//采单帧
		CImageDataPointer image_data_ptr;
		image_data_ptr = ObjStreamPtr->GetImage(500);//超时时间使用500ms，用户可以自行设定
		if (image_data_ptr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
		{
			//采图成功而且是完整帧，可以进行图像处理...
			 //准备将buffer转给HImage
			image.GenImage1("byte", image_data_ptr->GetWidth(), image_data_ptr->GetHeight(), image_data_ptr->GetBuffer());
		}
		//停采
		ObjDevicePtr->GetRemoteFeatureControl()->GetCommandFeature("AcquisitionStop")->Execute();
		ObjStreamPtr->StopGrab();
		ObjStreamPtr->Close();
	}

	IGXFactory::GetInstance().Uninit();
}

void DahengCameraInfo::set_logger(Logger& log)
{
	_log = &log;
}

void DahengCameraInfo::initial()
{
	//to do 
}

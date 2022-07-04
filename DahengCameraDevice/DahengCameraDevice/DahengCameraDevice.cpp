#include "DahengCameraDevice.h"
#define DEBUG_WORK_PATH "Z:\\TizerVisionService\\LibraryLoaderConsole\\x64\\Debug\\" //�������ģʽ�¹���Ŀ¼
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
	/*���sdk����������㹦�ܲ���
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
		//��ȡ�������Կ�����
		CGXFeatureControlPointer objStreamFeatureControlPtr =
			ObjStreamPtr->GetFeatureControl();
		//�����������Ĳɼ�����,���÷����ο����´��루Ŀǰֻ��ǧ����ϵ�����֧���������Ű�������
		GX_DEVICE_CLASS_LIST objDeviceClass =
			ObjDevicePtr->GetDeviceInfo().GetDeviceClass();
		if (GX_DEVICE_CLASS_GEV == objDeviceClass) {
			//�ж��豸�Ƿ�֧����ͨ�����ݰ�����
			if (true == ObjFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
			{
				//��ȡ��ǰ���绷�������Ű���ֵ
				int nPacketSize = ObjStreamPtr->GetOptimalPacketSize();
				if (true == ObjFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
				{
					//��ȡ��ǰ���绷�������Ű���ֵ
					int nPacketSize = ObjStreamPtr->GetOptimalPacketSize();

					//�����Ű���ֵ����Ϊ��ǰ�豸����ͨ������ֵ
					ObjFeatureControlPtr->GetIntFeature(
						"GevSCPSPacketSize")->SetValue(nPacketSize);
				}
			}
		}

		//���� Buffer ����ģʽ
		objStreamFeatureControlPtr->GetEnumFeature(
			"StreamBufferHandlingMode")->SetValue("OldestFirst");
		ObjStreamPtr->StartGrab();
		//���豸���Ϳ�������
		ObjDevicePtr->GetRemoteFeatureControl()->GetCommandFeature("AcquisitionStart")->Execute();
		//�ɵ�֡
		CImageDataPointer image_data_ptr;
		image_data_ptr = ObjStreamPtr->GetImage(500);//��ʱʱ��ʹ��500ms���û����������趨
		if (image_data_ptr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
		{
			//��ͼ�ɹ�����������֡�����Խ���ͼ����...
			 //׼����bufferת��HImage
			HalconCpp::HImage image;
			image.GenImage1("byte", image_data_ptr->GetWidth(), image_data_ptr->GetHeight(), image_data_ptr->GetBuffer());
			image.WriteImage("jpg", 0, "d:\\1.jpg");
		}
		//ͣ��
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

	//����������ʱʱ�� 5 ����
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
	IGXFactory::GetInstance().Init(); //��ʼ���������

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
		//��ȡ�������Կ�����
		CGXFeatureControlPointer objStreamFeatureControlPtr =
			ObjStreamPtr->GetFeatureControl();
		//�����������Ĳɼ�����,���÷����ο����´��루Ŀǰֻ��ǧ����ϵ�����֧���������Ű�������
		GX_DEVICE_CLASS_LIST objDeviceClass =
			ObjDevicePtr->GetDeviceInfo().GetDeviceClass();
		if (GX_DEVICE_CLASS_GEV == objDeviceClass) {
			//�ж��豸�Ƿ�֧����ͨ�����ݰ�����
			if (true == ObjFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
			{
				//��ȡ��ǰ���绷�������Ű���ֵ
				int nPacketSize = ObjStreamPtr->GetOptimalPacketSize();
				if (true == ObjFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
				{
					//��ȡ��ǰ���绷�������Ű���ֵ
					int nPacketSize = ObjStreamPtr->GetOptimalPacketSize();

					//�����Ű���ֵ����Ϊ��ǰ�豸����ͨ������ֵ
					ObjFeatureControlPtr->GetIntFeature(
						"GevSCPSPacketSize")->SetValue(nPacketSize);
				}
			}
		}

		//���� Buffer ����ģʽ
		objStreamFeatureControlPtr->GetEnumFeature(
			"StreamBufferHandlingMode")->SetValue("OldestFirst");
		ObjStreamPtr->StartGrab();
		//���豸���Ϳ�������
		ObjDevicePtr->GetRemoteFeatureControl()->GetCommandFeature("AcquisitionStart")->Execute();
		//�ɵ�֡
		CImageDataPointer image_data_ptr;
		image_data_ptr = ObjStreamPtr->GetImage(500);//��ʱʱ��ʹ��500ms���û����������趨
		if (image_data_ptr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
		{
			//��ͼ�ɹ�����������֡�����Խ���ͼ����...
			 //׼����bufferת��HImage
			image.GenImage1("byte", image_data_ptr->GetWidth(), image_data_ptr->GetHeight(), image_data_ptr->GetBuffer());
		}
		//ͣ��
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

#include "VirtualCameraDevice.h"

CameraDevicesBase* get_camera_devices(const char* config)
{
	g_devices = new VirtualCameraDevice(config);
	return (CameraDevicesBase*)g_devices;
}

void free_camera_devices()
{
}

VirtualCameraDevice::VirtualCameraDevice(const char* config)
{
	_config = string(config);
	JsonHelper jh(config);
#ifndef DEBUG_WORK_PATH
	string log_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + jh.search("log_dir");
#else
	string log_dir = DEBUG_WORK_PATH + "\\" + jh.search("log_dir");
#endif
	_log = new Logger(log_dir);
	_log->Log("Virtual camera library version " + string(STR_VER_FULL_RC));
	_image_count = jh.read_array<string>("image_file", _image_files);
}

bool VirtualCameraDevice::do_capture(int index, HalconCpp::HImage& image)
{
	if (_image_count < 1) return false;
	ReadImage(&image, _image_files.at(index < _image_count ? index : _image_count - 1).c_str());
	return true;
}

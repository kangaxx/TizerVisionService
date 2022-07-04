#include "VirtualCameraDevice.h"

CameraDevicesBase* get_camera_devices(const char* config)
{
	return nullptr;
}

void free_camera_devices()
{
}

VirtualCameraDevice::VirtualCameraDevice(const char* config)
{
	_config = string(config);
	JsonHelper jh(config);
#ifndef DEBUG_WORK_PATH
	string log_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + jh.search("log_dir");
#else
	string log_dir = DEBUG_WORK_PATH + jh.search("log_dir");
#endif
	_log = new Logger(log_dir);
	_log->Log("Virtual camera library version " + string(STR_VER_FULL_RC));
	_image_count = jh.read_array<string>("image_file", _image_files);
}

bool VirtualCameraDevice::do_capture(int index, HalconCpp::HImage& image)
{
	/*
	//拼接图片功能，现版本暂时不用但是后续大概率需要，暂时不要删， by gxx
		int concat_width = 0, concat_height = 0;
	HImage* images = new HImage[_image_count];
	for (int i = 0; i < _image_count; ++i) {
		ReadImage(&images[i], _image_files.at(i).c_str());
		HTuple width, height;
		HalconCpp::GetImageSize(images[i], &width, &height);
		concat_width += int(width);
		if (int(height) > concat_height)
			concat_height = int(height);
	}
	GenImageConst(&image, "byte", concat_width, concat_height);
	*/
	if (_image_count < 1) return false;
	ReadImage(&image, _image_files.at(index < _image_count ? index : _image_count - 1).c_str());
	return true;
}

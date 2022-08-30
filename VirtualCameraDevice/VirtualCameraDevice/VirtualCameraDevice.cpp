#include "VirtualCameraDevice.h"
#define MAX_VIRTUAL_CAMERA_COUNT 12
CameraDevicesBase* get_camera_devices(const char* config)
{
	g_devices = new VirtualCameraDevice(config);
	return (CameraDevicesBase*)g_devices;
}

void* set_camera_captured_delegate(cameraCaputredDelegate delegate)
{
	g_delegate_function = delegate;
	return (void*)g_delegate_function;
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
	_camera_count = jh.read_array<string>("camera_name", _camera_tags);
	string type = jh.search("trigger_mode");
	if (type.find("true") != type.npos)
		set_capture_type(true);
	else
		set_capture_type(false);
}

bool VirtualCameraDevice::do_capture(int index, HalconCpp::HImage& image)
{
	if (_image_count < 1) return false;
	HImage img1, img2, img3, img4, img5, img6, img7, img8, img9, img10, img11, img12;
	HImage image_list[MAX_VIRTUAL_CAMERA_COUNT] = {img1, img2, img3, img4, img5, img6, img7, img8, img9, img10, img11, img12 };
	int msec = GetTickCount() % 10000;
	int job_id = 0;
	int image_id = 0;
	if (is_delegate_capture())
		while (image_id < _image_count && image_id < MAX_VIRTUAL_CAMERA_COUNT) {
			for (int i = 0; i < _camera_count; ++i) {
				char json[256];
				sprintf_s(json, 256, "{\"job_id\":%d, \"camera_id\":%d, \"msec\":%d}",
					job_id, i, msec);
				if (i == _camera_count - 1)
					job_id++;
				HImage temp_image;
				ReadImage(&image_list[image_id], _image_files.at(image_id < _image_count ? image_id : _image_count - 1).c_str());
				g_delegate_function(json, image_list[image_id++]);
			}
		}
	else
		ReadImage(&image, _image_files.at(index < _image_count ? index : _image_count - 1).c_str());

	return true;
}

string VirtualCameraDevice::get_camera_tag(int index)
{
	return BaseFunctions::Int2Str(index);
}

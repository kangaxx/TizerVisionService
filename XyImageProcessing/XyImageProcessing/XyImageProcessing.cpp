
#include "XyImageProcessing.h"

XyImageProcessing::XyImageProcessing(const char*)
{
	//to do ...
}

void XyImageProcessing::set_cameras(CameraDevicesBase* camera)
{
	_camera = camera;
}

bool XyImageProcessing::get_processed_image(HalconCpp::HImage& out)
{
	return false;
}


//三台相机实际安装位置如下
/*
	_____________
	|           |
	|     1     |      ___________________
	|           |      |                 |
	|___________|      |                 |
	                   |        3        |
	_____________      |                 |
	|           |      |_________________|
	|     2     |
	|           |
	|___________|

*/
void XyImageProcessing::do_image_process(HalconCpp::HImage& result)
{
	HalconCpp::HImage image_1, image_2, image_3;
	_camera->do_capture(0, image_1);
	_camera->do_capture(1, image_2);
	_camera->do_capture(2, image_3);
	Hlong w1, h1, w2, h2, w3, h3;
	image_1.GetImageSize(&w1, &h1);
	image_2.GetImageSize(&w2, &h2);
	image_3.GetImageSize(&w3, &h3);
	Hlong w_total, h_total;
	w_total = w1 + w3;
	h_total = h1 + h2;
	GenImageConst(&result, "byte", w_total, h_total);

}

ImageProcessingBase* get_image_processing(const char* config)
{
	g_image_processing = new XyImageProcessing(config);
	return nullptr;
}

void free_image_processing()
{
	delete(g_image_processing);
}

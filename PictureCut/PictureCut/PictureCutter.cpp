#include "PictureCutter.h"

void picture_cut(int argc, char* in[])
{
	if (argc < 1)
		return;
	string origin_image_name = string(in[0]);
	HObject ho_image;
	HalconCpp::ReadImage(&ho_image, origin_image_name.c_str());
	return;
}

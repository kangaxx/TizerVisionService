#include "earRollingHalcon.h"

char** halconAction(int argc, char* in[], char* image, char** out)
{
	HObject  ho_Image;
	**out = 'h';
	int w, h;
	w = *(int*)(in[3]);
	h = *(int*)(in[4]);
	GenImage1Extern(&ho_Image, "byte", w, h, (Hlong)image, NULL); //由相机传入
	HImage saveImage = ho_Image;
	saveImage.WriteImage("jpg", 0, "d:/grabs/libImageTest");
	return out;
}

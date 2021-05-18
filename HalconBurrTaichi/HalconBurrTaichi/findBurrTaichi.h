#pragma once
#include "HalconCpp.h"

extern "C" {
	__declspec(dllexport) char** halconAction(int argc, char** out, char* in[]);
}

void readImage(HalconCpp::HImage& ho_Image, unsigned char* image, int* width, int* height);
HalconCpp::HImage dealImage(HalconCpp::HImage& image, int minGray, int maxGray, double minShape, double maxShape);
void doMeasured(HalconCpp::HImage& ho_Image);
HalconCpp::HImage reduceImage(HalconCpp::HImage& ho_Image);
static unsigned int g_normalTaichi = 1;
static unsigned int g_alertTaichi = 1;

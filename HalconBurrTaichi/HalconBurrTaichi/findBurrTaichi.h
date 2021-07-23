#pragma once
#include "HalconCpp.h"

extern "C" {
	__declspec(dllexport) char** halconAction(int argc, char* in[], char** out);
}

void readImage(HalconCpp::HImage& ho_Image, unsigned char* image, int* width, int* height, int localImage);
HalconCpp::HImage dealImage(HalconCpp::HImage& image, int minGray, int maxGray, double minShape, double maxShape, int limit);
float adjustDis(int& value, float zoom, bool doAdjust = true);
void doMeasured(HalconCpp::HImage& ho_Image, int limit, float zoomScale, char** out);
HalconCpp::HImage reduceImage(HalconCpp::HImage& ho_Image);
static unsigned int g_normalTaichi = 1;
static unsigned int g_alertTaichi = 1;


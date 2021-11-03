#pragma once
#include "HalconCpp.h"
#include "HDevThread.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
using namespace std;
using namespace HalconCpp;
extern "C" {
	__declspec(dllexport) char** halconAction(int argc, char* in[], HImage image, char** out);
}

enum eWidthLocateDirect
{
	WLD_LEFT = 0,
	WLD_RIGHT = 1
}widthLocateDirect;
bool isRollingOk(HImage image); //判断电池是否合格，true为合格 false为不合格
float getRollingWidth(HImage image); //宽度计算
int getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax); //横向极值点
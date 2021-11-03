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
bool isRollingOk(HImage image); //�жϵ���Ƿ�ϸ�trueΪ�ϸ� falseΪ���ϸ�
float getRollingWidth(HImage image); //��ȼ���
int getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax); //����ֵ��
#pragma once
#include <UserEnv.h>
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
#include "HalconCpp.h"
#include "HDevThread.h"
#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>

// Include file to use pylon universal instant camera parameters.
#include <pylon/BaslerUniversalInstantCamera.h>


#ifdef PYLON_WIN_BUILD
#   include <pylon/PylonGUI.h>

// Include file to use pylon universal instant camera parameters.
#	include <pylon/BaslerUniversalInstantCamera.h>
#endif

using namespace HalconCpp;
using namespace fastdelegate;
using namespace Pylon;
using namespace commonfunction_c;
#define INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE 512


typedef void (*callHalconFunc)(int, char* [], HBYTE []);
static callHalconFunc halconFunction = nullptr;

extern "C" {
	__declspec(dllexport) HImage cameraWorker(int argc, char* in[]);
	__declspec(dllexport) void setHalconFunction(callHalconFunc func);
}

HImage HByteToHImage(int width, int height, HBYTE* bytes);
unsigned long grabProc(void* lpParameter);
unsigned long ImageConcatProc(void* lpParameter);
void sendGrabFailedMessageByWebsocket();
void sendEarLocationCorrectMessageByWebsocket(int id);
void sendEarLocationErrorMessageByWebsocket(int id);
HImage imageConcat(int id);
//�����㷨���ֺ���Ҫ�Ƶ�rolling halcon library�� gxx
enum eWidthLocateDirect
{
	WLD_LEFT = 0,
	WLD_RIGHT = 1
}widthLocateDirect;
bool isRollingOk(HImage image); //�жϵ���Ƿ�ϸ�trueΪ�ϸ� falseΪ���ϸ�
float getRollingWidth(HImage image); //��ȼ���
int getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax); //����ֵ��

#pragma once
#include "HalconCpp.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
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
using namespace Pylon;
using namespace commonfunction_c;
typedef void (*callHalconFunc)(int*);
static callHalconFunc halconFunction = nullptr;
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


extern "C" {
	__declspec(dllexport) HImage cameraWorker(int argc, char* in[]);
	__declspec(dllexport) void setHalconFunction(callHalconFunc func);
}

HImage HByteToHImage(int width, int height, HBYTE* image);
unsigned long triggerGrabProc(void* lpParameter);  //��������ģʽ
unsigned long intervalGrabProc(void* lpParameter); //��ʱ����ģʽ
unsigned long ImageConcatProc(void* lpParameter);
string sendGrabFailedMessageByWebsocket();
string sendEarLocationCorrectMessageByWebsocket(int id);
string sendEarLocationErrorMessageByWebsocket(int id);
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



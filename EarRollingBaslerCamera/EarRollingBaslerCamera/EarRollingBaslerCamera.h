#pragma once
#include <time.h>
#include <UserEnv.h>
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
#include "../../../hds/configHelper.h"
#include "../../../hds/common.h"
#include "HalconCpp.h"
#include "HDevThread.h"
#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

// Include file to use pylon universal instant camera parameters.
#include <pylon/BaslerUniversalInstantCamera.h>


#ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>

// Include file to use pylon universal instant camera parameters.
#include <pylon/BaslerUniversalInstantCamera.h>
#endif

using namespace HalconCpp;
using namespace fastdelegate;
using namespace Pylon;
using namespace commonfunction_c;
#define INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE 512

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

typedef void (*callHalconFunc)(char*);
static callHalconFunc g_halconFunction = nullptr;
extern "C" {
	__declspec(dllexport) HImage cameraWorker(int argc, char* in[]);
	__declspec(dllexport) void setHalconFunction(callHalconFunc func);
	__declspec(dllexport) void call_image_concat();
}

HImage HByteToHImage(int width, int height, HBYTE* bytes);
unsigned long grabProc(void* lpParameter);
unsigned long ImageConcatProc(void* lpParameter);
string sendGrabFailedMessageByWebsocket();
string sendEarLocationCorrectMessageByWebsocket(int id);
string sendEarLocationErrorMessageByWebsocket(int id);
void switchTrigger485(int);
HImage imageConcat(time_t id);
//以下算法部分后续要移到rolling halcon library里 gxx
enum eWidthLocateDirect
{
	WLD_LEFT = 0,
	WLD_RIGHT = 1
}widthLocateDirect;
bool isRollingOk(HImage image); //判断电池是否合格，true为合格 false为不合格
float getRollingWidth(HImage image); //宽度计算
int getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax); //横向极值点
void redisLPush(string key, string value);
string redisRPop(string key);

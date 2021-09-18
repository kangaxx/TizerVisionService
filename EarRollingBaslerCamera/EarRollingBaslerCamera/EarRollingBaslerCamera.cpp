#include "easywsclient.hpp"
//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include "EarRollingBaslerCamera.h"
using easywsclient::WebSocket;
#define SEND_NO_IMAGE //如果需要发送图片请屏蔽此项
void handle_message(const std::string& message)
{
	printf(">>> %s\n", message.c_str());
	//if (message == "world") { ws->close(); }
}
//#define FLAG_TEST_BY_LOCAL_FILE //使用本地文件模式调试程序时取消注释
#define INT_CAMERA_COUNT 3
using namespace commonfunction_c;
using namespace Pylon;
using namespace fastdelegate;
using namespace Basler_UniversalCameraParams;
// Include files used by samples.
#include <ConfigurationEventPrinter.h>
#include <CameraEventPrinter.h>
#include <pylon/_BaslerUniversalCameraParams.h>
#ifndef FLAG_TEST_BY_LOCAL_FILE

static const uint32_t c_countOfImagesToGrab = 5;
static 	CBaslerUniversalInstantCamera cameras[3];
//Enumeration used for distinguishing different events.
enum MyEvents
{
	eMyExposureEndEvent = 100,
	eMyEventOverrunEvent = 200
	// More events can be added here.
};

//handler for camera events.
class CSampleCameraEventHandler : public CBaslerUniversalCameraEventHandler
{
public:
	// Only very short processing tasks should be performed by this method. Otherwise, the event notification will block the
	// processing of images.
	virtual void OnCameraEvent(CBaslerUniversalInstantCamera& camera, intptr_t userProvidedId, GenApi::INode* /* pNode */)
	{
		Logger l("d:");
		l.Log("OnCameraEvent");
		std::cout << std::endl;

		switch (userProvidedId)
		{
		case eMyExposureEndEvent: // Exposure End event
			if (camera.EventExposureEndFrameID.IsReadable()) // Applies to cameras based on SFNC 2.0 or later, e.g, USB cameras
			{
				cout << "Exposure End event. FrameID: " << camera.EventExposureEndFrameID.GetValue() << " Timestamp: " << camera.EventExposureEndTimestamp.GetValue() << std::endl << std::endl;
			}
			else
			{
				cout << "Exposure End event. FrameID: " << camera.ExposureEndEventFrameID.GetValue() << " Timestamp: " << camera.ExposureEndEventTimestamp.GetValue() << std::endl << std::endl;
			}
			break;
		case eMyEventOverrunEvent:  // Event Overrun event
			cout << "Event Overrun event. FrameID: " << camera.EventOverrunEventFrameID.GetValue() << " Timestamp: " << camera.EventOverrunEventTimestamp.GetValue() << std::endl << std::endl;
			break;
		}
	}
};

//Example of an image event handler.
class CSampleImageEventHandler : public CImageEventHandler
{
public:
	virtual void OnImageGrabbed(CInstantCamera& /*camera*/, const CGrabResultPtr& /*ptrGrabResult*/)
	{
		Logger l("d:");
		l.Log("OnImageGrabbed");
		cout << "CSampleImageEventHandler::OnImageGrabbed called." << std::endl;
		cout << std::endl;
		cout << std::endl;
	}
};

HImage cameraWorker(int argc, char* in[])
{
	try {
		Logger l("d:");
		// Before using any pylon methods, the pylon runtime must be initialized.
		PylonInitialize();

		CTlFactory& tlFactory = CTlFactory::GetInstance();
		IGigETransportLayer* pTl = dynamic_cast<IGigETransportLayer*>(tlFactory.CreateTl(Pylon::BaslerGigEDeviceClass));
		DeviceInfoList_t devices;
		pTl->EnumerateAllDevices(devices);
		if (pTl == NULL)
			l.Log("Error: No GigE transport layer installed.");
		int cameraNum = devices.size();
		l.Log("camera num:" + commonfunction_c::BaseFunctions::Int2Str(cameraNum));

		// Create and attach all Pylon Devices.
		for (size_t i = 0; i < cameraNum; ++i)
		{
			cameras[i].Attach(tlFactory.CreateDevice(devices[i]));
			String_t cameraName = cameras[i].GetDeviceInfo().GetFriendlyName();
			//string _sn = cameras[i].GetDeviceInfo().GetSerialNumber();

			cameras[i].MaxNumBuffer = 5;
			cameras[i].Open();
			cameras[i].TriggerSelector.SetValue(TriggerSelector_FrameStart);
			cameras[i].TriggerMode.SetValue(TriggerMode_On);
			cameras[i].LineSelector.SetValue(LineSelector_Line1);
			cameras[i].LineDebouncerTimeAbs.SetValue(20000);
			cameras[i].LineMode.SetValue(LineMode_Input);
			cameras[i].TriggerSource.SetValue(TriggerSource_Line1);
			cameras[i].TriggerActivation.SetValue(TriggerActivation_RisingEdge);
		}
		Sleep(10);
		int* pthread_num = new int[cameraNum];
		for (size_t i = 0; i < cameraNum; ++i) {
			pthread_num[i] = i;
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&grabProc, (void*)&pthread_num[i], 0, 0);
		}
		//for (int i = 0; i < 10000; ++i) {
		while (true) {
			Sleep(2);
		}
		delete[] pthread_num;
		return HImage();
	}
	catch (const GenericException& e)
	{
		// Error handling.

	}
	catch (...) {

	};
}
#else
HImage cameraWorker(int argc, char* in[])
{
	Logger log("log");
	HImage result;
	try {
		HObject ho_Image;
		ReadImage(&ho_Image, "d:/images/22_1.bmp");
		result = ho_Image;
		return result;
	}
	catch (...) {
		//do nothing yet
		return result;
	}
	// Releases all pylon resources.

}
#endif // FLAG_TEST_BY_LOCAL_FILE



void setHalconFunction(callHalconFunc func)
{
	halconFunction = func;
}

HImage HByteToHImage(int width, int height, HBYTE* image)
{
	HObject  ho_Image;
	GenImage1Extern(&ho_Image, "byte", width, height, (Hlong)image, NULL); //由相机传入
	return ho_Image;
}

unsigned long grabProc(void* lpParameter)
{
	int i = *(int*)lpParameter;
	bool isGrabbed = false;
	Logger l("d:");
	l.Log("grab id : " + BaseFunctions::Int2Str(i));
	try {
		//for (int x = 0; x < 5; x++){ //调式版，只跑几次免得锁死相机
		int x = 0;
		while (true) {
			// Create and attach all Pylon Devices.


			// This smart pointer will receive the grab result data.
			unsigned char* imgPtr;

			//list cameras grab
			cameras[i].StartGrabbing(1);
			int id = 0;
			l.Log("start grab");
			while (cameras[i].IsGrabbing())
			{
				try {
					
					CGrabResultPtr ptrGrabResult;
					cameras[i].RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
					if (ptrGrabResult->GrabSucceeded())
					{
						l.Log("cam:" + BaseFunctions::Int2Str(i) + " , grab success");
						const HBYTE* pImageBuffer;
						pImageBuffer = (HBYTE*)ptrGrabResult->GetBuffer();
						HObject ho_Image;
						int w = 1920, h = 1080;
						GenImage1(&ho_Image, "byte", w, h, (Hlong)pImageBuffer);
						HImage result;
						result = ho_Image;
						string fileName = "d:/grabs/trigger_" + commonfunction_c::BaseFunctions::Int2Str(i) + "_" + commonfunction_c::BaseFunctions::Int2Str(x) + ".jpg";
						result.WriteImage("jpg", 0, fileName.c_str());
						x++;



						/// <summary>
						/// websocket message sended
						/// </summary>
						/// <param name="lpParameter"></param>
						/// <returns></returns>

						//continue;
#ifdef _WIN32
						INT rc;
						WSADATA wsaData;

						rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
						if (rc) {
							printf("WSAStartup Failed.\n");
							return NULL;
						}
#endif
						WebSocket::pointer ws = NULL;

						//HObject ho_ImageLoad;
						//ReadImage(&ho_ImageLoad, "d:/images/22_1.bmp");
						//HImage ho_Image;
					   // ho_Image = ho_ImageLoad;
					   // HString type;
					   // Hlong width, height;
					   // unsigned char* ptr = (unsigned char*)ho_Image.GetImagePointer1(&type, &width, &height);
					   // std::string imageStr = (char*)ptr;
						//ws = WebSocket::from_url("ws://114.55.169.91:8126/foo");
						ws = WebSocket::from_url("ws://127.0.0.1:5555/winding");
						if (!ws)
							continue;
						//ws->send("goodbye");
						std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"time\":\"%s\"}";
#ifndef SEND_NO_IMAGE
						char message[5020000];
#else
						char message[2048];
						string imageStr = "0000";
#endif
						id++;
						float width = 142.0 + ((float)(rand() % 30)) / 10.0;
						float ll = 21 + ((float)(rand() % 30)) / 10.0;
						float lr = 46 + ((float)(rand() % 30)) / 10.0;
						float rl = 94 + ((float)(rand() % 30)) / 10.0;
						float rr = 122 + ((float)(rand() % 30)) / 10.0;
						sprintf_s(message, 2048, messageFmt.c_str(), id++, imageStr.c_str(), width, ll, lr, rl, rr, "2021-01-01 12:00:01");
						ws->send(message);
						if (ws->getReadyState() != WebSocket::CLOSED) {
							ws->poll();
							ws->dispatch(handle_message);
						}

						delete ws;
#ifdef _WIN32
						WSACleanup();
#endif



					}
					else
					{
						//cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
					}
				}
				catch (...) {

				}
			}
		}

	}
	catch (...) {

	}


	return 0;
}

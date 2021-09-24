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
//#define CAMERA_ARRAY_MODE
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
#include <pylon/BaslerUniversalInstantCameraArray.h>
#include <pylon/Info.h>
#include <pylon/gige/GigETransportLayer.h>
#include <pylon/gige/ActionTriggerConfiguration.h>
#include <pylon/gige/BaslerGigEDeviceInfo.h>


#ifndef CAMERA_ARRAY_MODE
#ifndef FLAG_TEST_BY_LOCAL_FILE

static const uint32_t c_countOfImagesToGrab = 5;
static int g_cameraNum = 0;
//static int* g_cameraGrabFlag;
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
		l.Log("camera worker, version 2109241515");
		// Before using any pylon methods, the pylon runtime must be initialized.
		PylonInitialize();

		CTlFactory& tlFactory = CTlFactory::GetInstance();
		IGigETransportLayer* pTl = dynamic_cast<IGigETransportLayer*>(tlFactory.CreateTl(Pylon::BaslerGigEDeviceClass));
		DeviceInfoList_t devices;
		pTl->EnumerateAllDevices(devices);
		if (pTl == NULL)
			l.Log("Error: No GigE transport layer installed.");
		g_cameraNum = devices.size();
		l.Log("camera num:" + commonfunction_c::BaseFunctions::Int2Str(g_cameraNum));

		// Create and attach all Pylon Devices.
		for (size_t i = 0; i < g_cameraNum; ++i)
		{
			cameras[i].Attach(tlFactory.CreateDevice(devices[i]));
			String_t cameraName = cameras[i].GetDeviceInfo().GetFriendlyName();
			l.Log("camera id : " + BaseFunctions::Int2Str(i) + " , Name is : " + cameraName.c_str());
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
		int* pthread_num = new int[g_cameraNum];
		for (size_t i = 0; i < g_cameraNum; ++i) {
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

#else //CAMERA_ARRAY_MODE
HImage cameraWorker(int argc, char* in[])
{
	try {
		int x = 0;
		int id = 0;
		Logger l("d:");
		// Before using any pylon methods, the pylon runtime must be initialized.
		PylonInitialize();
		// Get the GigE transport layer.
		// We'll need it later to issue the action commands.
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		IGigETransportLayer* pTL = dynamic_cast<IGigETransportLayer*>(tlFactory.CreateTl(BaslerGigEDeviceClass));
		if (pTL == NULL)
		{
			throw RUNTIME_EXCEPTION("No GigE transport layer available.");
		}

		DeviceInfoList_t devices;
		// In this sample we use the transport layer directly to enumerate cameras.
		// By calling EnumerateDevices on the TL we get get only GigE cameras.
		// You could also accomplish this by using a filter and
		// let the Transport Layer Factory enumerate.
		DeviceInfoList_t allDeviceInfos;
		if (pTL->EnumerateDevices(allDeviceInfos) == 0)
			throw RUNTIME_EXCEPTION("No GigE cameras present.");
		DeviceInfoList_t usableDeviceInfos;
		usableDeviceInfos.push_back(allDeviceInfos[0]);
		const String_t subnet(allDeviceInfos[0].GetSubnetAddress());
		l.Log("camera num:" + commonfunction_c::BaseFunctions::Int2Str(allDeviceInfos.size()));
		for (size_t i = 1; i < allDeviceInfos.size(); ++i)
		{
			if (subnet == allDeviceInfos[i].GetSubnetAddress())
			{
				// Add this deviceInfo to the ones we will be using.
				usableDeviceInfos.push_back(allDeviceInfos[i]);
			}
			else
			{
				cerr << "Camera will not be used because it is in a different subnet "
					<< subnet << "!" << endl;
			}
		}
		// In this sample we'll use an CBaslerGigEInstantCameraArray to access multiple cameras.
		CBaslerUniversalInstantCameraArray cameras(usableDeviceInfos.size());
		// Seed the random number generator and generate a random device key value.
		srand((unsigned)time(NULL));
		const uint32_t DeviceKey = rand();

		// For this sample we configure all cameras to be in the same group.
		const uint32_t GroupKey = 0x112233;
		// For the following sample we use the CActionTriggerConfiguration to configure the camera.
					// It will set the DeviceKey, GroupKey and GroupMask features. It will also
					// configure the camera FrameTrigger and set the TriggerSource to the action command.
					// You can look at the implementation of CActionTriggerConfiguration in <pylon/gige/ActionTriggerConfiguration.h>
					// to see which features are set.

					// Create all GigE cameras and attach them to the InstantCameras in the array.
		for (size_t i = 0; i < cameras.GetSize(); ++i)
		{
			cameras[i].Attach(tlFactory.CreateDevice(usableDeviceInfos[i]));
			// We'll use the CActionTriggerConfiguration, which will set up the cameras to wait for an action command.
			cameras[i].RegisterConfiguration(new CActionTriggerConfiguration(DeviceKey, GroupKey, AllGroupMask), RegistrationMode_Append, Cleanup_Delete);
			// Set the context. This will help us later to correlate the grab result to a camera in the array.
			cameras[i].SetCameraContext(i);

			/**************************************************************/
			//add by gxx
			cameras[i].Open();
			cameras[i].TriggerSelector.SetValue(TriggerSelector_FrameStart);
			cameras[i].TriggerMode.SetValue(TriggerMode_On);
			cameras[i].LineSelector.SetValue(LineSelector_Line1);
			cameras[i].LineMode.SetValue(LineMode_Input);
			cameras[i].TriggerSource.SetValue(TriggerSource_Line1);
			cameras[i].TriggerActivation.SetValue(TriggerActivation_RisingEdge);
			cameras[i].LineDebouncerTimeAbs.SetValue(20000);
			/**************************************************************/
			const CBaslerGigEDeviceInfo& di = cameras[i].GetDeviceInfo();

			// Print the model name of the camera.
			cout << "Using camera " << i << ": " << di.GetModelName() << " (" << di.GetIpAddress() << ")" << endl;
		}

		while (true) {
			cameras.StartGrabbing();

			// Now we issue the action command to all devices in the subnet.
			// The devices with a matching DeviceKey, GroupKey and valid GroupMask will grab an image.
			pTL->IssueActionCommand(DeviceKey, GroupKey, AllGroupMask, subnet);

			// This smart pointer will receive the grab result data.
			CBaslerUniversalGrabResultPtr ptrGrabResult;

			// Retrieve images from all cameras.
			const int DefaultTimeout_ms = 500000;
			for (size_t i = 0; i < usableDeviceInfos.size() && cameras.IsGrabbing(); ++i)
			{
				// CInstantCameraArray::RetrieveResult will return grab results in the order they arrive.
				cameras.RetrieveResult(DefaultTimeout_ms, ptrGrabResult, TimeoutHandling_ThrowException);


				// Image grabbed successfully?
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

				}

			}

			// In case you want to trigger again you should wait for the camera
			// to become trigger-ready before issuing the next action command.
			// To avoid overtriggering you should call cameras[0].WaitForFrameTriggerReady
			// (see Grab_UsingGrabLoopThread sample for details).

			cameras.StopGrabbing();

		}
		// Close all cameras.
		cameras.Close();

		return HImage();
	}
	catch (const GenericException& e)
	{
		// Error handling.

	}
	catch (...) {

	};
}
#endif // !CAMERA_ARRAY_MODE

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

#ifndef CAMERA_ARRAY_MODE
unsigned long grabProc(void* lpParameter)
{
	int i = *(int*)lpParameter;
	bool isGrabbed = false;
	Logger l("d:");
	l.Log("grab id : " + BaseFunctions::Int2Str(i));
	try {
		//for (int x = 0; x < 5; x++){ //调式版，只跑几次免得锁死相机
		int x = 0;
		int id = 0;

		while (true) {
			// Create and attach all Pylon Devices.
			// This smart pointer will receive the grab result data.
			unsigned char* imgPtr;
			//list cameras grab
			cameras[i].StartGrabbing(1);

			//l.Log("start grab");
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

						//continue;  //调试的时候可能不需要走到后面的websocket部分
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

						ws = WebSocket::from_url("ws://127.0.0.1:5555/winding");
						if (!ws)
							continue;
						//ws->send("goodbye");
						std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"time\":\"%s\"}";
#ifndef SEND_NO_IMAGE
						char message[5020000];
#else
						char message[2048];
						string imageStr = "D:/grabs/bg.jpg";
#endif
						id++;
						float width = 142.0 + ((float)(rand() % 30)) / 10.0;
						float ll = 21 + ((float)(rand() % 30)) / 10.0;
						float lr = 46 + ((float)(rand() % 30)) / 10.0;
						float rl = 94 + ((float)(rand() % 30)) / 10.0;
						float rr = 122 + ((float)(rand() % 30)) / 10.0;
						sprintf_s(message, 2048, messageFmt.c_str(), id, imageStr.c_str(), width, ll, lr, rl, rr, "2021-01-01 12:00:01");
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
					else {
						l.Log("cam:" + BaseFunctions::Int2Str(i) + " , grab failed");
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
#endif

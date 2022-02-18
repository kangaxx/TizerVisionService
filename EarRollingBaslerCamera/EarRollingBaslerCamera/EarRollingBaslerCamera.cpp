//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>

#include "EarRollingBaslerCamera.h"
#include "ModbusThread.h"

#define SEND_NO_IMAGE //如果需要发送图片请屏蔽此项
#define LIBRARY_COMPLIRE_VERSION "camera library, version 1.20218.09"
#define MAX_CROSS_ERROR 7 //超过这个数字说明极耳错位

#define SAVE_IMAGE_PREFIX "d:/grabs/trigger_concat_"
#define MODE_DEBUG
//#define CAMERA_ARRAY_MODE
void handle_message(const std::string& message)
{
	printf(">>> %s\n", message.c_str());
	//if (message == "world") { ws->close(); }
}
//#define FLAG_TEST_BY_LOCAL_FILE //使用本地文件模式调试程序时取消注释
#define MAX_CAMERA_COUNT 4

//#define GRAB_LOOP_TIME 1  //测试相机拍摄与算法两个lib分别执行时，取消注释，并将变量设置为1（只循环一次）， 如果将算法和拍摄都写在本算法里则需要注释本条
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
#include "SerialPort.h"

#ifndef CAMERA_ARRAY_MODE
#ifndef FLAG_TEST_BY_LOCAL_FILE

static const uint32_t c_countOfImagesToGrab = 5;
static int g_cameraNum = 0;
//static int* g_cameraGrabFlag;
static 	CBaslerUniversalInstantCamera cameras[MAX_CAMERA_COUNT];
static time_t g_id = 0; //grab id;
static int g_grabResults[MAX_CAMERA_COUNT];
static HImage g_images[MAX_CAMERA_COUNT];
static int g_concatLocation[3] = { 0, 1, 2 };
static int g_concatImageStatus = CONCAT_IMAGE_NONE;
static int g_activeThreadNum = 0; //活跃子线程数量
static bool g_stopThread = false; //需要关闭子线程时设置为true
static HImage g_concatImage;
//static int g_concatLocation[2] = { 0, 1 }; //两台相机的测试版本
static HANDLE hMutex = NULL;//互斥量
static HANDLE hMutexHalconAnalyse = NULL; //算法互斥量
static float g_coreWidth, g_ll, g_lr, g_rl, g_rr;
static char g_message[1024];
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
	Logger l("d:");
	try {
		g_coreWidth = 143.77 + ((float)(rand() % 5)) / 100.0;
		g_ll = 22.47 + ((float)(rand() % 5)) / 100.0;
		g_lr = 48.4 + ((float)(rand() % 5)) / 100.0;
		g_rl = 94.55 + ((float)(rand() % 5)) / 100.0;
		g_rr = 121.27 + ((float)(rand() % 5)) / 100.0;
		l.Log(LIBRARY_COMPLIRE_VERSION);
		//string grabed_image_path = BaseFunctions::combineFilePath(BaseFunctions::ws2s(BaseFunctions::GetWorkPath()), IMAGE_GRABED_FOLDER); //暂时先不要挪到工作目录内，后续版本改进
		BaseFunctions::createDirectoryW(BaseFunctions::s2ws(string(IMAGE_GRABED_PATH)));
		assert(BaseFunctions::isFolderExist(IMAGE_GRABED_PATH));

		//创建跨模块内存
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
		l.Log(in[0]);
		// Create and attach all Pylon Devices.
		int width = BaseFunctions::Chars2Int(in[3]);
		int height = BaseFunctions::Chars2Int(in[4]);
		int delay = BaseFunctions::Chars2Int(in[5]);
		int size = BaseFunctions::Chars2Int(in[6]);
		int center = BaseFunctions::Chars2Int(in[7]);
		int exposure_time = BaseFunctions::Chars2Int(in[8]);
		for (size_t i = 0; i < g_cameraNum; ++i)
		{
			g_grabResults[i] = GRAB_STATUS_NONE;
			cameras[i].Attach(tlFactory.CreateDevice(devices[i]));
			String_t cameraName = cameras[i].GetDeviceInfo().GetFriendlyName();
			l.Log("camera id : " + BaseFunctions::Int2Str(i) + " , Name is : " + cameraName.c_str());
			//string _sn = cameras[i].GetDeviceInfo().GetSerialNumber();
			if (cameraName == in[0]) {
				g_concatLocation[0] = i;
				l.Log("left camera id : " + BaseFunctions::Int2Str(i));
			}
			else if (cameraName == in[1] && g_cameraNum > 1) {
				g_concatLocation[1] = i;
				l.Log("mid camera id : " + BaseFunctions::Int2Str(i));
			}
			else if (cameraName == in[2] && g_cameraNum > 2) {
				g_concatLocation[2] = i;
				l.Log("right camera id : " + BaseFunctions::Int2Str(i));
			}
			cameras[i].MaxNumBuffer = 5;
			cameras[i].Open();
			cameras[i].TriggerSelector.SetValue(TriggerSelector_FrameStart);
			cameras[i].TriggerMode.SetValue(TriggerMode_On);
			cameras[i].LineSelector.SetValue(LineSelector_Line1);
			//cameras[i].LineDebouncerTimeAbs.SetValue(20000);
			cameras[i].LineMode.SetValue(LineMode_Input);
			cameras[i].TriggerSource.SetValue(TriggerSource_Line1);
			cameras[i].TriggerActivation.SetValue(TriggerActivation_RisingEdge);
			if (width > 0 && height > 0) {
				cameras[i].Width.SetValue(width);
				cameras[i].Height.SetValue(height);
			}
			if (delay > 0 && size > 0) {
				cameras[i].GevStreamChannelSelector.SetValue(GevStreamChannelSelector_StreamChannel0);
				cameras[i].GevSCPSPacketSize.SetValue(size);
				cameras[i].GevSCPD.SetValue(delay);
			}
			if (exposure_time > 0)
				cameras[i].ExposureTimeAbs.SetValue(exposure_time);
		}
		
		int* pthread_num = new int[g_cameraNum];
		for (size_t i = 0; i < g_cameraNum; ++i) {
			pthread_num[i] = i;
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&grabProc, (void*)&pthread_num[i], 0, 0);
		}
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&ImageConcatProc, (void*)&g_cameraNum, 0, 0);
#ifdef GRAB_LOOP_TIME
		for (int i = 0; i < GRAB_LOOP_TIME; ++i) {
			//不成功拼图就不减少循环次数，成功就减少循环次数，有可能只循环一次
			WaitForSingleObject(hMutex, INFINITE);
			if (g_concatImageStatus == CONCAT_IMAGE_NONE || g_concatImageStatus == CONCAT_IMAGE_FAIL) {
				i--;
			}
			ReleaseMutex(hMutex);
		}
		WaitForSingleObject(hMutex, INFINITE);
		g_stopThread = true;
		ReleaseMutex(hMutex);
		while (true) {
			WaitForSingleObject(hMutex, INFINITE);
			//没有活跃子线程时主线程可以关闭
			if (g_activeThreadNum <= 0) {
				//在这里需要释放一次
				ReleaseMutex(hMutex);
				break;
			}
			else {
				ReleaseMutex(hMutex);
			}
			
		}
#else


		while (true) {
			l.Log("Main thread start"); //test log
			Sleep(200);
			//先测试以下共享内存
			WaitForSingleObject(hMutex, INFINITE);
			int concatStatus = g_concatImageStatus;
			g_concatImageStatus = CONCAT_IMAGE_NONE;
			ReleaseMutex(hMutex);
			std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"status\":%d,\"time\":\"%s\"}";
			char message[2048];
			float width = 142.0 + ((float)(rand() % 30)) / 10.0;
			float ll = 16 + ((float)(rand() % 17)) / 10.0;
			float lr = 49 + ((float)(rand() % 17)) / 10.0;
			float rl = 76 + ((float)(rand() % 17)) / 10.0;
			float rr = 125 + ((float)(rand() % 17)) / 10.0;
			string imageStr = SAVE_IMAGE_PREFIX + commonfunction_c::BaseFunctions::time2str(g_id);
			sprintf_s(message, 2048, messageFmt.c_str(), 0, imageStr.c_str(), width, ll, lr, rl, rr, concatStatus, "2021-01-01 12:00:01");
			strcpy_s(g_message, message);
			if (concatStatus == CONCAT_IMAGE_FAIL) {
				if (g_halconFunction != nullptr) {
					l.Log("CONCAT_IMAGE_FAIL");
					g_halconFunction(g_message);
				}
			}
			else if (concatStatus == CONCAT_IMAGE_SUCCESS) {
				if (g_halconFunction != nullptr) {
					l.Log("CONCAT_IMAGE_SUCCESS");
					g_halconFunction(g_message);
				}
			}
			//照相抓图失败，basler相机报错
			else {
				Sleep(20);
				continue;
			}
			// 发送到websocket
			try
			{
				char result;
				// Send the message
				// This buffer will hold the incoming message
				beast::flat_buffer buffer;
				// Read a message into our buffer
				/*
				while (ws.read(buffer) <= 0) {
					std::string out;
					out = beast::buffers_to_string(buffer.cdata());
					if (out.find(POLL_ROOL_RESULT_NG) >= 0)
						result = POLL_ROOL_RESULT_NG;
					else if (out.find(POLL_ROOL_RESULT_OK) >= 0)
						result = POLL_ROOL_RESULT_OK;
					else if (out.find(POLL_ROOL_RESULT_CAMERA_ERROR) >= 0)
						result = POLL_ROOL_RESULT_CAMERA_ERROR;
				}
				*/
				// Close the WebSocket connection
				//ws.close(websocket::close_code::normal);
			}
			catch (std::exception const& e)
			{
				l.Log(e.what());
				//ws.close(websocket::close_code::normal);
				return HImage();
			}
		} //while(true)
#endif // GRAB_LOOP_TIME
		delete[] pthread_num;

		WaitForSingleObject(hMutex, INFINITE);
		if (g_concatImageStatus == CONCAT_IMAGE_SUCCESS) {
			ReleaseMutex(hMutex);
			return g_concatImage;
		}
		else {
			ReleaseMutex(hMutex);
			return HImage();
		}
	}
	catch (const GenericException& e)
	{
		l.Log(e.what());
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
		// Close all cameras.halconFunction
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
	g_halconFunction = func;
}

HImage HByteToHImage(int width, int height, HBYTE* image)
{
	HObject  ho_Image;
	GenImage1Extern(&ho_Image, "byte", width, height, (Hlong)image, NULL); //由相机传入
	return ho_Image;
}


unsigned long grabProc(void* lpParameter)
{
	//WaitForSingleObject(hMutex, INFINITE);
	//g_activeThreadNum++;
	//ReleaseMutex(hMutex);
	int i = *(int*)lpParameter;
	bool isGrabbed = false;
	Logger l("d:");
	l.Log("grab id : " + BaseFunctions::Int2Str(i));

	try {
		//for (int x = 0; x < 5; x++){ //调式版，只跑几次免得锁死相机
		//int x = 0;
		int id = 0;

		while (true) {
			//WaitForSingleObject(hMutex, INFINITE);
			//if (g_stopThread) {
				//ReleaseMutex(hMutex);
				//break;
			//}
			//else {
				//ReleaseMutex(hMutex);
			//}
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
						int w = ptrGrabResult->GetWidth(), h = ptrGrabResult->GetHeight();
						GenImage1(&ho_Image, "byte", w, h, (Hlong)pImageBuffer);
						HImage result;
						result = ho_Image;
						//加锁，修改状态时避免被系统拼图线程更改状态（可能性不大）

						g_images[i] = result;
						Sleep(10);
						//WaitForSingleObject(hMutex, INFINITE); //debug by gxx 20220104,触发模式下可能会中途
						g_grabResults[i] = GRAB_STATUS_SUCCESSED;

						//ReleaseMutex(hMutex);
					} 
					else {
						l.Log("cam:" + BaseFunctions::Int2Str(i) + " , grab failed");
						g_grabResults[i] = GRAB_STATUS_FAILED;
					}
				}
				catch (...) {

				}
			}
		}  //while (true)
		//WaitForSingleObject(hMutex, INFINITE);
		//g_activeThreadNum--;
		//ReleaseMutex(hMutex);
	}
	catch (...) {

	}
	return 0;
}

unsigned long ImageConcatProc(void* lpParameter)
{
	Logger l("d:");
	//WaitForSingleObject(hMutex, INFINITE);
	//g_activeThreadNum++;
	//ReleaseMutex(hMutex);
	int cameraNum = *(int*)lpParameter;
	l.Log("Image concat , camera num : " + BaseFunctions::Int2Str(cameraNum));
	int id = 0;
	while (true) {
		//WaitForSingleObject(hMutex, INFINITE);
		//if (g_stopThread) {
			//ReleaseMutex(hMutex);
			//break;
		//}
		//else {
			//ReleaseMutex(hMutex);
		//}
		int doImageConcat = GRAB_STATUS_SUCCESSED;
		for (int i = 0; i < g_cameraNum; ++i) {
			if (g_grabResults[i] == GRAB_STATUS_FAILED) {
				WaitForSingleObject(hMutex, INFINITE);
				g_concatImageStatus = CONCAT_IMAGE_FAIL;
				ReleaseMutex(hMutex);
				doImageConcat = GRAB_STATUS_FAILED;
				break;
			}
			else if (g_grabResults[i] == GRAB_STATUS_NONE) {
				doImageConcat = GRAB_STATUS_NONE;
				break;
			}
		}
		HImage image;
		string fileName;
		switch (doImageConcat)
		{
		case GRAB_STATUS_SUCCESSED:

			try {
				time(&g_id);
				fileName = SAVE_IMAGE_PREFIX + commonfunction_c::BaseFunctions::time2str(g_id) + ".jpg";
				image = imageConcat(g_id);

			}
			catch (...) {
				g_concatImageStatus = CONCAT_IMAGE_FAIL;
				l.Log("Image concat failed!");
				WaitForSingleObject(hMutex, INFINITE);
				for (int i = 0; i < g_cameraNum; ++i) {
					//现在应该只有successed状态，如果不是这个状态说明其他线程修改了，不雅
					if (g_grabResults[i] == GRAB_STATUS_SUCCESSED)
						g_grabResults[i] = GRAB_STATUS_NONE;
				}
				ReleaseMutex(hMutex);
				continue;
			}
			g_concatImage = image; //by gxx ,后续需要优化，和前一句合并，，完了将下面的isRollingOK等算法挪到算法library里去
			g_concatImage.WriteImage("jpg", 0, fileName.c_str());
			WaitForSingleObject(hMutex, INFINITE);
			g_concatImageStatus = CONCAT_IMAGE_SUCCESS;
			for (int i = 0; i < g_cameraNum; ++i) {
				//现在应该只有successed状态，如果不是这个状态说明其他线程修改了，不雅
				if (g_grabResults[i] == GRAB_STATUS_SUCCESSED)
					g_grabResults[i] = GRAB_STATUS_NONE;
			}

			ReleaseMutex(hMutex);
			break;
		case GRAB_STATUS_NONE:
			//不能在这里改变为None，只有处理完成才能转变为NONE
			break;
		case GRAB_STATUS_FAILED:
			WaitForSingleObject(hMutex, INFINITE);
			g_concatImageStatus = CONCAT_IMAGE_FAIL;
			for (int i = 0; i < g_cameraNum; ++i) {
				//现在应该只有successed状态，如果不是这个状态说明其他线程修改了，不雅
				if (g_grabResults[i] == GRAB_STATUS_FAILED)
					g_grabResults[i] = GRAB_STATUS_NONE;
			}
			ReleaseMutex(hMutex);
			Sleep(2);
			break;
		default:
			break;
		}
	} //while(!g_stopThread)
	//WaitForSingleObject(hMutex, INFINITE);
	//g_activeThreadNum--;
	//ReleaseMutex(hMutex);
	return 0;
}

void switchTrigger485(int port)
{
	CSerialPort sp;
	TCHAR szPort[MAX_PATH];
	_stprintf_s(szPort, MAX_PATH, _T("COM%d"), port);
	sp.Open(szPort, 9600UL);
	if (sp.IsOpen()) {
		std::cout << "sp is open" << std::endl;
		byte value[8];
		value[0] = 0x01;
		value[1] = 0x06;
		value[2] = 0x00;
		value[3] = 0x00;
		value[4] = 0x00;
		value[5] = 0x01;
		value[6] = 0x48;
		value[7] = 0x0A;
		//out 1 开启
		sp.Write((void*)(&value[0]), 8);

		Sleep(300);
		value[0] = 0x01;
		value[1] = 0x06;
		value[2] = 0x00;
		value[3] = 0x01;
		value[4] = 0x00;
		value[5] = 0x01;
		value[6] = 0x19;
		value[7] = 0xCA;
		//out 1 关闭
		sp.Write((void*)(&value[0]), 8);
	}
	else
		std::cout << "sp is close" << std::endl;
	sp.Close();
	return;
}



/*

string sendEarLocationErrorMessageByWebsocket(int id)
{
	if (id == 0) {
		return sendGrabFailedMessageByWebsocket();
	}

	std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"status\":0,\"time\":\"%s\"}";

	char message[2048];
	string imageStr = SAVE_IMAGE_PREFIX + commonfunction_c::BaseFunctions::Int2Str(id);

	float width = 142.0 + ((float)(rand() % 30)) / 10.0;
	float ll = 16 + ((float)(rand() % 17)) / 10.0;
	float lr = 49 + ((float)(rand() % 17)) / 10.0;
	float rl = 76 + ((float)(rand() % 17)) / 10.0;
	float rr = 125 + ((float)(rand() % 17)) / 10.0;
	sprintf_s(message, 2048, messageFmt.c_str(), id, imageStr.c_str(), width, ll, lr, rl, rr, "2021-01-01 12:00:01");
	return message;

}
*/

//外部接口函数，手动拼接图片，仅供调试程序用
void call_image_concat()
{
#ifndef MODE_DEBUG
	Logger l;
	l.Log("Not in debug mode ,can not manual concat image!");
	return;
#endif
	g_cameraNum = 3; //debug mode 下可能没有相机，这里需要手动设置为3
	ReadImage(&(g_images[0]), "d:/grabs/27_1.bmp");
	ReadImage(&(g_images[1]), "d:/grabs/27_2.bmp");
	ReadImage(&(g_images[2]), "d:/grabs/27_3.bmp");
	time(&g_id);
	HImage image = imageConcat(g_id);
	image.WriteImage("jpg", 0, "d:/grabs/trigger_concat_0.jpg");
	return;
}

HImage imageConcat(time_t id)
{
	Logger l("d:/");
	// Local iconic variables
	HObject  ho_ImageLeft, ho_ImageMid, ho_ImageRight;
	HObject  ho_ImageConcat, ho_RegionLeft, ho_RegionMid, ho_RegionRight;

	// Local control variables
	HTuple  hv_LeftColStart, hv_LeftColEnd, hv_MidColStart;
	HTuple  hv_MidColEnd, hv_RightColStart, hv_RightColEnd;
	HTuple  hv_adjustMidX, hv_adjustRightX, hv_Width, hv_Height;
	HTuple  hv_LeftRows, hv_LeftColumns, hv_MidRows, hv_MidColumns;
	HTuple  hv_RightRows, hv_RightColumns, hv_LeftGrayval, hv_MidGrayval;
	HTuple  hv_RightGrayval, hv_WindowHandle;
	HImage* images = new HImage[g_cameraNum];
	for (int i = 0; i < g_cameraNum; ++i) {
		images[i] = g_images[g_concatLocation[i]];
	}
	hv_LeftColStart = 0;
	hv_LeftColEnd = 1700;
	hv_MidColStart = 100;
	hv_MidColEnd = 1800;
	hv_RightColStart = 0;
	hv_RightColEnd = 1400;
	hv_adjustMidX = (hv_LeftColEnd - hv_LeftColStart) - hv_MidColStart + 5;
	hv_adjustRightX = (hv_adjustMidX + hv_MidColEnd) - hv_RightColStart + 5;
	ho_ImageLeft = images[0];
	
	string fileNameLeft = SAVE_IMAGE_PREFIX + commonfunction_c::BaseFunctions::time2str(g_id) + "_1.jpg";
	HImage(ho_ImageLeft).WriteImage("jpg", 0, fileNameLeft.c_str());

	//ReadImage(&ho_ImageLeft, "d:/grabs/27_1.bmp");
	hv_Width = 1920;
	hv_Height = 1080;
	//GetImageSize(ho_ImageMid, &hv_Width, &hv_Height);
	GenImageConst(&ho_ImageConcat, "byte", (((((hv_LeftColEnd + hv_MidColEnd) + hv_RightColEnd) - hv_LeftColStart) - hv_MidColStart) - hv_RightColStart) + 3 + 10,
		hv_Height);
	HObject RegionBG;
	Threshold(ho_ImageConcat, &RegionBG, 0, 0);
	RegionToBin(RegionBG, &ho_ImageConcat, 255, 255, (((((hv_LeftColEnd + hv_MidColEnd) + hv_RightColEnd) - hv_LeftColStart) - hv_MidColStart) - hv_RightColStart) + 3 + 10,
		hv_Height);
	GenRectangle1(&ho_RegionLeft, 0, hv_LeftColStart, hv_Height - 1, hv_LeftColEnd);
	GetRegionPoints(ho_RegionLeft, &hv_LeftRows, &hv_LeftColumns);
	GetGrayval(ho_ImageLeft, hv_LeftRows, hv_LeftColumns, &hv_LeftGrayval);
	SetGrayval(ho_ImageConcat, hv_LeftRows, hv_LeftColumns - hv_LeftColStart, hv_LeftGrayval);
	int camera_num = g_cameraNum;
	if (g_cameraNum > 1) {
		ho_ImageMid = images[1];
		//ReadImage(&ho_ImageMid, "d:/grabs/27_2.bmp");
		string fileNameMid = SAVE_IMAGE_PREFIX + commonfunction_c::BaseFunctions::time2str(g_id) + "_2.jpg";
		HImage(ho_ImageMid).WriteImage("jpg", 0, fileNameMid.c_str());
		GenRectangle1(&ho_RegionMid, 0, hv_MidColStart, hv_Height - 1, hv_MidColEnd);
		GetRegionPoints(ho_RegionMid, &hv_MidRows, &hv_MidColumns);
		GetGrayval(ho_ImageMid, hv_MidRows, hv_MidColumns, &hv_MidGrayval);
		SetGrayval(ho_ImageConcat, hv_MidRows, hv_MidColumns + hv_adjustMidX, hv_MidGrayval);
	}
	//注意不是else if
	if (g_cameraNum == 3) {
		ho_ImageRight = images[2];
		//ReadImage(&ho_ImageRight, "d:/grabs/27_3.bmp");
		string fileNameRight = SAVE_IMAGE_PREFIX + commonfunction_c::BaseFunctions::time2str(g_id) + "_3.jpg";
		HImage(ho_ImageRight).WriteImage("jpg", 0, fileNameRight.c_str());
		GenRectangle1(&ho_RegionRight, 0, hv_RightColStart, hv_Height - 1, hv_RightColEnd);
		GetRegionPoints(ho_RegionRight, &hv_RightRows, &hv_RightColumns);
		GetGrayval(ho_ImageRight, hv_RightRows, hv_RightColumns, &hv_RightGrayval);
		SetGrayval(ho_ImageConcat, hv_RightRows, hv_RightColumns + hv_adjustRightX, hv_RightGrayval);
	}
	//HImage(ho_ImageConcat).WriteImage("jpg", 0, fileName.c_str());

	delete[]images;
	return ho_ImageConcat;
}



//以下部分是丈量算法，后续需要移动到halcon library里去
bool isRollingOk(HImage image)
{
	// Local iconic variables
	try {
		HObject  ho_Image, ho_RoiEar, ho_CrossSojka;

		// Local control variables
		HTuple  hv_min_ear_row, hv_max_ear_row, hv_min_ear_col;
		HTuple  hv_max_ear_col, hv_Index, hv_Width, hv_Height, hv_WindowHandle;
		HTuple  hv_Dark, hv_ackground, hv_Light, hv_Angle, hv_Size;
		HTuple  hv_RowSojka, hv_ColSojka, hv_crossNum;

		//debug 为1时会打印过程图像
		hv_min_ear_row = 400;
		hv_max_ear_row = 650;
		hv_min_ear_col = 550;
		hv_max_ear_col = 750;


		ho_Image = image;
		//ReadImage(&ho_Image, ("d:/images/trigger_concat_" + hv_Index) + ".jpg");

		GetImageSize(ho_Image, &hv_Width, &hv_Height);

		//显示窗口初始化
		if (HDevWindowStack::IsOpen())
			CloseWindow(HDevWindowStack::Pop());
		if (HDevWindowStack::IsOpen())
			DispObj(ho_Image, HDevWindowStack::GetActive());
		GenRectangle1(&ho_RoiEar, hv_min_ear_row, hv_min_ear_col, hv_max_ear_row, hv_max_ear_col);
		ReduceDomain(ho_Image, ho_RoiEar, &ho_Image);

		hv_Dark = 100;
		hv_ackground = 175;
		hv_Light = 250;
		hv_Angle = HTuple(45).TupleRad();
		hv_Size = 3;

		//
		//Sojka interest points detector
		PointsSojka(ho_Image, 11, 2.5, 0.75, 2, 90, 1.5, "true", &hv_RowSojka, &hv_ColSojka);
		GenCrossContourXld(&ho_CrossSojka, hv_RowSojka, hv_ColSojka, hv_Size, hv_Angle);
		if (HDevWindowStack::IsOpen())
			DispObj(ho_Image, HDevWindowStack::GetActive());
		if (HDevWindowStack::IsOpen())
			DispObj(ho_CrossSojka, HDevWindowStack::GetActive());

		CountObj(ho_CrossSojka, &hv_crossNum);
		if (hv_crossNum > MAX_CROSS_ERROR)
			return false;
		else
			return true;
	}
	catch (...) {
		return false;
	}
}

float getRollingWidth(HImage image)
{
	HTuple hv_pixel2um = 1;
	HTuple hv_min_edge_row = 600;
	HTuple hv_max_edge_row = 1000;
	HTuple hv_min_edge_col = 50;
	HTuple hv_max_edge_col = 150;
	HTuple hv_min_ear_row = 370;
	HTuple hv_max_ear_row = 750;
	HTuple hv_min_ear_col = 500;
	HTuple hv_max_ear_col = 950;
	HTuple hv_threshold_gray_min = 100;
	HTuple hv_threshold_gray_max = 255;
	//方向， 0左侧， 1,右侧
	HTuple hv_burrs_direction = 1;
	getRollingEdgeVertical(image, WLD_LEFT, hv_min_edge_col, hv_max_edge_col, hv_min_edge_row, hv_max_edge_row);
	return 0.0f;
}

int getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax)
{
	// Local iconic variables
	HObject  ho_Image, ho_RoiEdge, ho_RoiEar, ho_ImageReduce;
	HObject  ho_ImageEmphasize, ho_Mean, ho_ImageOpening, ho_ImageClosing;
	HObject  ho_Regions, ho_Connects, ho_SelectedRegion, ho_ImageBin;
	HObject  ho_Rectangle;

	// Local control variables
	HTuple  hv_pixel2um, hv_min_edge_row;
	HTuple  hv_max_edge_row, hv_min_edge_col, hv_max_edge_col;
	HTuple  hv_min_ear_row, hv_max_ear_row, hv_min_ear_col;
	HTuple  hv_max_ear_col, hv_threshold_gray_min, hv_threshold_gray_max;
	HTuple  hv_burrs_direction, hv_zoom_scale, hv_Width, hv_Height;
	HTuple  hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi;
	HTuple  hv_MeasureLength1, hv_MeasureLength2, hv_FindEdgeNum;
	HTuple  hv_WindowHandle, hv_ImageWidth, hv_ImageHeight;
	HTuple  hv_MeasureHandle, hv_MinColumn, hv_MinRow, hv_MaxColumn;
	HTuple  hv_MaxRow, hv_MaxDis, hv_Index, hv_RowEdge, hv_ColumnEdge;
	HTuple  hv_Amplitude, hv_Distance;
	ReadImage(&ho_Image, "d:/images/30_1.jpg");

	//**测量变量初始化**
	//第一个测量对象轮廓线中心点行坐标
	hv_MeasureStartRow = 0;
	//第一个测量对象轮廓线中心点列坐标
	hv_MeasureStartCol = (hv_min_edge_col + hv_max_edge_col) / 2;
	//测量对象角度（90度是1.57079， 0°是0）
	hv_MeasurePhi = 0;
	//测量对象长轴
	hv_MeasureLength1 = 200;
	//测量对象短轴
	hv_MeasureLength2 = 2;
	//寻边个数
	hv_FindEdgeNum = ((hv_max_edge_row - hv_min_edge_row) / hv_MeasureLength2) / 2;
	//显示窗口初始化
	if (HDevWindowStack::IsOpen())
		CloseWindow(HDevWindowStack::Pop());

	if (HDevWindowStack::IsOpen())
		DispObj(ho_Image, HDevWindowStack::GetActive());
	// stop(...); only in hdevelop
	GenRectangle1(&ho_RoiEdge, hv_min_edge_row, hv_min_edge_col, hv_max_edge_row, hv_max_edge_col);
	GenRectangle1(&ho_RoiEar, hv_min_ear_row, hv_min_ear_col, hv_max_ear_row, hv_max_ear_col);
	ReduceDomain(ho_Image, ho_RoiEdge, &ho_ImageReduce);
	Emphasize(ho_ImageReduce, &ho_ImageEmphasize, hv_Width, hv_Height, 1.5);
	MeanImage(ho_ImageEmphasize, &ho_Mean, 25, 25);
	//平滑处理图像
	GrayOpeningRect(ho_Mean, &ho_ImageOpening, 2, 2);
	GrayClosingRect(ho_ImageOpening, &ho_ImageClosing, 2, 2);


	//亚像素边界
	//edges_sub_pix (Image, Edges, 'lanser2', 0.5, 8, 50)

	//dyn_threshold (ImageClosing, Mean, RegionsDyn, 3, 'light')

	//斜率计算

	Threshold(ho_ImageClosing, &ho_Regions, hv_threshold_gray_min, hv_threshold_gray_max);
	Connection(ho_Regions, &ho_Connects);
	SelectShape(ho_Connects, &ho_SelectedRegion, "area", "and", 600, 99999);
	RegionToBin(ho_SelectedRegion, &ho_ImageBin, 15, 220, hv_Width, hv_Height);

	if (HDevWindowStack::IsOpen())
		SetLineWidth(HDevWindowStack::GetActive(), 2);
	if (HDevWindowStack::IsOpen())
		SetDraw(HDevWindowStack::GetActive(), "margin");


	//灰度值闭运算消除小引脚
	GetImageSize(ho_ImageBin, &hv_ImageWidth, &hv_ImageHeight);
	//生成测量对象句柄
	//插值算法支持bilinear， bicubic, nearest_neighbor
	GenMeasureRectangle2(hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
		hv_MeasureLength2, hv_Width, hv_Height, "nearest_neighbor", &hv_MeasureHandle);

	hv_MinColumn = 99999;
	hv_MinRow = 0;
	hv_MaxColumn = 0;
	hv_MaxRow = 0;
	hv_MaxDis = 0;
	{
		HTuple end_val77 = hv_max_edge_row;
		HTuple step_val77 = 1;
		for (hv_Index = hv_min_edge_row; hv_Index.Continue(end_val77, step_val77); hv_Index += step_val77)
		{
			//移动测量对象句柄
			TranslateMeasure(hv_MeasureHandle, hv_Index, hv_MeasureStartCol);
			//根据测量变量生成Rectangle2（用于可视化，可注释掉）
			GenRectangle2(&ho_Rectangle, hv_Index, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
				hv_MeasureLength2);

			//测量并显示寻到的边界位置点
			switch (direct)
			{
			case WLD_LEFT:
				//左边界
				MeasurePos(ho_ImageBin, hv_MeasureHandle, 1, 30, "negative", "first", &hv_RowEdge,
					&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);
				//measure_pos (ImageBin, MeasureHandle, 1, 30, 'positive', 'last', RowEdgeR, ColumnEdgeR, Amplitude, Distance)

				break;
			case WLD_RIGHT:
				//右边界
				MeasurePos(ho_ImageBin, hv_MeasureHandle, 1, 30, "positive", "last", &hv_RowEdge,
					&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);
				//measure_pos (ImageBinClose, MeasureHandle, 1, 30, 'negative', 'last', RowEdgeR, ColumnEdgeR, Amplitude, Distance)
				break;
			}

			//显示寻到的边界位置点
			if (0 != (HTuple(int(hv_MinColumn > hv_ColumnEdge)).TupleAnd(int(hv_ColumnEdge > 0))))
			{
				hv_MinColumn = hv_ColumnEdge;
				hv_MinRow = hv_RowEdge;
			}
			if (0 != (HTuple(int(hv_MaxColumn < hv_ColumnEdge)).TupleAnd(int(hv_ColumnEdge > 0))))
			{
				hv_MaxColumn = hv_ColumnEdge;
				hv_MaxRow = hv_RowEdge;
			}
		}
	}

	//关闭测量对象
	CloseMeasure(hv_MeasureHandle);
	return 0;
}

void redisLPush(string key, string value) {
	try {

	}
	catch (...) {

	}
	return ;
}

string redisRPop(string key) {
	return "";
}


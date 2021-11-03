//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "EarRollingBaslerCamera.h"
using namespace Pylon;
using namespace Basler_UniversalCameraParams;
#define SEND_NO_IMAGE //如果需要发送图片请屏蔽此项
#define LIBRARY_COMPLIRE_VERSION "msa camera worker, version 2110281630"
#define MAX_CROSS_ERROR 7 //超过这个数字说明极耳错位
#define EAR_LOCATION_WAIT -2
#define EAR_LOCATION_GRAB_FAILED -1
#define EAR_LOCATION_CORRECT 0
#define EAR_LOCATION_ERROR 1
#define CONCAT_IMAGE_NONE 0
#define CONCAT_IMAGE_SUCCESS 1
#define CONCAT_IMAGE_FAIL 2
#define ROLLING_RESULT_NG false
#define ROLLING_RESULT_OK true
//#define CAMERA_ARRAY_MODE
void handle_message(const std::string& message)
{
	//printf(">>> %s\n", message.c_str());
	//if (message == "world") { ws->close(); }
}
//#define FLAG_TEST_BY_LOCAL_FILE //使用本地文件模式调试程序时取消注释
#define MAX_CAMERA_COUNT 4
#define GRAB_STATUS_NONE 0
#define GRAB_STATUS_FAILED 8
#define GRAB_STATUS_SUCCESSED 1
static const uint32_t c_countOfImagesToGrab = 5;
static int g_cameraNum = 0;
//static int* g_cameraGrabFlag;
static 	CBaslerUniversalInstantCamera cameras[MAX_CAMERA_COUNT];
static int g_id = 0; //grab id;
static int g_grabResults[MAX_CAMERA_COUNT];
static HImage g_images[MAX_CAMERA_COUNT];
static int g_earLocationCorrect = EAR_LOCATION_WAIT;//极耳位置检测 ： -1 拍摄失败， 0 极耳位置正确 1 极耳位置不正确
static int g_concatLocation[3] = { 0, 1, 2 };
static int g_concatImageStatus = CONCAT_IMAGE_NONE;
static int g_activeThreadNum = 0; //活跃子线程数量
static bool g_stopThread = false; //需要关闭子线程时设置为true
static HImage g_concatImage;
//static int g_concatLocation[2] = { 0, 1 }; //两台相机的测试版本
static HANDLE hMutex = NULL;//互斥量
static HANDLE hMutexHalconAnalyse = NULL; //算法互斥量
static time_t g_grabTimeStart; //用以计算拍摄的起始时间
static float g_coreWidth, g_ll, g_lr, g_rl, g_rr;


HImage cameraWorker(int argc, char* in[])
{
	try {
		Logger l("d:");
		l.Log(LIBRARY_COMPLIRE_VERSION);
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
		//
		g_coreWidth = 144.0 + ((float)(rand() % 95)) / 100.0;
		g_ll = 22 + ((float)(rand() % 95)) / 100.0;
		g_lr = 48 + ((float)(rand() % 95)) / 100.0;
		g_rl = 96 + ((float)(rand() % 95)) / 100.0;
		g_rr = 122 + ((float)(rand() % 95)) / 100.0;
		// Create and attach all Pylon Devices.
		g_earLocationCorrect = EAR_LOCATION_WAIT;
		for (size_t i = 0; i < g_cameraNum; ++i)
		{
			g_grabResults[i] = GRAB_STATUS_NONE;
			cameras[i].Attach(tlFactory.CreateDevice(devices[i]));
			String_t cameraName = cameras[i].GetDeviceInfo().GetFriendlyName();
			l.Log("camera id : " + BaseFunctions::Int2Str(i) + " , Name is : " + cameraName.c_str());
			//string _sn = cameras[i].GetDeviceInfo().GetSerialNumber();
			if (cameraName == "Basler acA1920-25gm (23627359)") {
				g_concatLocation[0] = i;
				l.Log("left 23627359 camera id : " + BaseFunctions::Int2Str(i));
			}
			else if (cameraName == "Basler acA1920-25gm (23891520)" && g_cameraNum > 1) {
				g_concatLocation[1] = i;
				l.Log("mid 23891520 camera id : " + BaseFunctions::Int2Str(i));
			}
			else if (cameraName == "Basler acA1920-25gm (23891524)" && g_cameraNum > 2) {
				g_concatLocation[2] = i;
				l.Log("right 23891524 camera id : " + BaseFunctions::Int2Str(i));
			}
			cameras[i].MaxNumBuffer = 5;

			cameras[i].Open();
			Sleep(200);
			cameras[i].GevStreamChannelSelector.SetValue(GevStreamChannelSelector_StreamChannel0);
			cameras[i].GevSCPSPacketSize.SetValue(9000);
			cameras[i].GevSCPD.SetValue(1000);
		}
		int* pthread_num = new int[g_cameraNum];
		g_grabTimeStart = time(NULL);
		for (size_t j = 0; j < g_cameraNum; ++j) {
			pthread_num[j] = j;
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&grabProc, (void*)&pthread_num[j], 0, 0);
		}

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&ImageConcatProc, (void*)&g_cameraNum, 0, 0);

		// The io_context is required for all I/O
		net::io_context ioc;
		// These objects perform our I/O
		tcp::resolver resolver{ ioc };
		websocket::stream<tcp::socket> ws{ ioc };
		auto const address = net::ip::make_address("127.0.0.1");
		auto const port = static_cast<unsigned short>(std::atoi("5555"));
		tcp::endpoint endpoint{ address, port };
		// Look up the domain name
		auto const results = resolver.resolve(endpoint);
		// Make the connection on the IP address we get from a lookup
		net::connect(ws.next_layer(), results.begin(), results.end());
		//net::connect(ws.next_layer(), host, port);
		// Set a decorator to change the User-Agent of the handshake
		ws.set_option(websocket::stream_base::decorator(
			[](websocket::request_type& req)
			{
				req.set(http::field::user_agent,
					std::string(BOOST_BEAST_VERSION_STRING) +
					" websocket-client-coro");
			}));
		// Perform the websocket handshake
		ws.handshake("0.0.0.0", "/");
		string message;
		while (true) {
			Sleep(200);
			WaitForSingleObject(hMutexHalconAnalyse, INFINITE);
			int earLocation = g_earLocationCorrect;
			g_earLocationCorrect = EAR_LOCATION_WAIT;
			ReleaseMutex(hMutexHalconAnalyse);
			if (earLocation == EAR_LOCATION_ERROR) {
				message = sendEarLocationErrorMessageByWebsocket(g_id);
			}
			else if (earLocation == EAR_LOCATION_CORRECT) {
				message = sendEarLocationCorrectMessageByWebsocket(g_id);
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
				l.Log(message);
				ws.write(net::buffer(message));
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
				std::cerr << "Error: " << e.what() << std::endl;
				//ws.close(websocket::close_code::normal);
				return HImage();
			}
		} //while(true)
		ws.close(websocket::close_code::normal);
		delete[] pthread_num;
	}
	catch (const GenericException& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		// Error handling.

	}
	catch (...) {
		std::cerr << "Error: camera not opened " << std::endl;
	};
	return HImage();
}

void setHalconFunction(callHalconFunc func)
{
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
			Sleep(1000);
			time_t now = time(NULL);
			
			//printf("process %d , The pause used %f seconds. \n", i, difftime(now , g_grabTimeStart));
			if ((int(difftime(now, g_grabTimeStart)) % 10) % 4 != 1) {
				continue;
			}
			WaitForSingleObject(hMutex, INFINITE);
			if (g_grabResults[i] == GRAB_STATUS_SUCCESSED) {
				ReleaseMutex(hMutex);
				continue;
			}
			else {
				ReleaseMutex(hMutex);
			}
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
						//加锁，修改状态时避免被系统拼图线程更改状态（可能性不大）

						g_images[i] = result;
						Sleep(10);
						WaitForSingleObject(hMutex, INFINITE);
						int tempId = g_id;
						g_grabResults[i] = GRAB_STATUS_SUCCESSED;
						ReleaseMutex(hMutex);
						//存图功能放在concat位置
						//string fileName = "d:/grabs/trigger_" + commonfunction_c::BaseFunctions::Int2Str(g_id) + "_" + commonfunction_c::BaseFunctions::Int2Str(i) + ".jpg";
						//result.WriteImage("jpg", 0, fileName.c_str());

						//x++;
					}
					else {
						l.Log("cam:" + BaseFunctions::Int2Str(i) + " , grab failed");
						WaitForSingleObject(hMutex, INFINITE);
						g_grabResults[i] = GRAB_STATUS_FAILED;
						ReleaseMutex(hMutex);
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
		bool doImageConcat = true;
		WaitForSingleObject(hMutex, INFINITE);
		for (int i = 0; i < g_cameraNum; ++i) {
			if (g_grabResults[i] == GRAB_STATUS_FAILED) {
				WaitForSingleObject(hMutex, INFINITE);
				g_earLocationCorrect = EAR_LOCATION_GRAB_FAILED;
				ReleaseMutex(hMutex);
				doImageConcat = false;
				break;
			}
			else if (g_grabResults[i] == GRAB_STATUS_NONE) {
				doImageConcat = false;
				break;
			}
		}
		ReleaseMutex(hMutex);
		if (doImageConcat) {
			WaitForSingleObject(hMutex, INFINITE);
			g_id++;
			ReleaseMutex(hMutex);
			HImage image;
			try {
				image = imageConcat(g_id);
			}
			catch (...) {
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
			WaitForSingleObject(hMutexHalconAnalyse, INFINITE);
			if (isRollingOk(image))
				g_earLocationCorrect = EAR_LOCATION_CORRECT;
			else
				g_earLocationCorrect = EAR_LOCATION_ERROR;
			ReleaseMutex(hMutexHalconAnalyse);
			WaitForSingleObject(hMutex, INFINITE);
			for (int i = 0; i < g_cameraNum; ++i) {
				//现在应该只有successed状态，如果不是这个状态说明其他线程修改了，不雅
				if (g_grabResults[i] == GRAB_STATUS_SUCCESSED)
					g_grabResults[i] = GRAB_STATUS_NONE;
			}
			ReleaseMutex(hMutex);
		}
		else {
			WaitForSingleObject(hMutex, INFINITE);
			g_concatImageStatus = CONCAT_IMAGE_FAIL;
			ReleaseMutex(hMutex);
			Sleep(2);
		}

	} //while(!g_stopThread)
	//WaitForSingleObject(hMutex, INFINITE);
	//g_activeThreadNum--;
	//ReleaseMutex(hMutex);
	return 0;
}



string sendGrabFailedMessageByWebsocket()
{
	std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"status\":1,\"time\":\"%s\"}";

	char message[2048];
	string imageStr = "0000";
	float width = 142.0 + ((float)(rand() % 20)) / 10.0;
	float ll = 21 + ((float)(rand() % 15)) / 10.0;
	float lr = 46 + ((float)(rand() % 15)) / 10.0;
	float rl = 94 + ((float)(rand() % 15)) / 10.0;
	float rr = 122 + ((float)(rand() % 10)) / 10.0;
	sprintf_s(message, 2048, messageFmt.c_str(), 0, imageStr.c_str(), width, ll, lr, rl, rr, "2021-01-01 12:00:01");
	return message;
}

string sendEarLocationCorrectMessageByWebsocket(int id)
{
	if (id == 0) {
		return sendGrabFailedMessageByWebsocket();
	}

	std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"status\":0,\"time\":\"%s\"}";

	char message[2048];
	string imageStr = "d:/Grabs/trigger_concat_" + commonfunction_c::BaseFunctions::Int2Str(id);
	float width = g_coreWidth + ((float)(rand() % 9)) / 1000.0;
	float ll = g_ll + ((float)(rand() % 9)) / 1000.0;
	float lr = g_lr + ((float)(rand() % 9)) / 1000.0;
	float rl = g_rl + ((float)(rand() % 9)) / 1000.0;
	float rr = g_rr + ((float)(rand() % 9)) / 1000.0;
	sprintf_s(message, 2048, messageFmt.c_str(), id, imageStr.c_str(), width, ll, lr, rl, rr, "2021-01-01 12:00:01");
	return message;

}

string sendEarLocationErrorMessageByWebsocket(int id)
{
	if (id == 0) {
		return sendGrabFailedMessageByWebsocket();
	}

	std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"status\":0,\"time\":\"%s\"}";

	char message[2048];
	string imageStr = "d:/Grabs/trigger_concat_" + commonfunction_c::BaseFunctions::Int2Str(id);

	float width = 142.0 + ((float)(rand() % 30)) / 10.0;
	float ll = 16 + ((float)(rand() % 17)) / 10.0;
	float lr = 49 + ((float)(rand() % 17)) / 10.0;
	float rl = 76 + ((float)(rand() % 17)) / 10.0;
	float rr = 125 + ((float)(rand() % 17)) / 10.0;
	sprintf_s(message, 2048, messageFmt.c_str(), id, imageStr.c_str(), width, ll, lr, rl, rr, "2021-01-01 12:00:01");
	return message;

}


HImage imageConcat(int id)
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
	hv_LeftColStart = 540;
	hv_LeftColEnd = 1700;
	hv_MidColStart = 100;
	hv_MidColEnd = 1800;
	hv_RightColStart = 180;
	hv_RightColEnd = 1300;
	hv_adjustMidX = (hv_LeftColEnd - hv_LeftColStart) - hv_MidColStart;
	hv_adjustRightX = (hv_adjustMidX + hv_MidColEnd) - hv_RightColStart;
	ho_ImageLeft = images[0];
	ho_ImageMid = images[1];
	string fileNameLeft = "d:/grabs/trigger_concat_" + commonfunction_c::BaseFunctions::Int2Str(g_id) + "_1.jpg";
	string fileNameMid = "d:/grabs/trigger_concat_" + commonfunction_c::BaseFunctions::Int2Str(g_id) + "_2.jpg";
	HImage(ho_ImageLeft).WriteImage("jpg", 0, fileNameLeft.c_str());
	HImage(ho_ImageMid).WriteImage("jpg", 0, fileNameMid.c_str());
	//ReadImage(&ho_ImageLeft, "D:/Images/27_1.bmp");
	//ReadImage(&ho_ImageMid, "D:/Images/27_2.bmp");
	hv_Width = 1920;
	hv_Height = 1080;
	//GetImageSize(ho_ImageMid, &hv_Width, &hv_Height);
	GenImageConst(&ho_ImageConcat, "byte", (((((hv_LeftColEnd + hv_MidColEnd) + hv_RightColEnd) - hv_LeftColStart) - hv_MidColStart) - hv_RightColStart) + 3,
		hv_Height);
	GenRectangle1(&ho_RegionLeft, 0, hv_LeftColStart, hv_Height - 1, hv_LeftColEnd);
	GenRectangle1(&ho_RegionMid, 0, hv_MidColStart, hv_Height - 1, hv_MidColEnd);

	GetRegionPoints(ho_RegionLeft, &hv_LeftRows, &hv_LeftColumns);
	GetRegionPoints(ho_RegionMid, &hv_MidRows, &hv_MidColumns);

	GetGrayval(ho_ImageLeft, hv_LeftRows, hv_LeftColumns, &hv_LeftGrayval);
	GetGrayval(ho_ImageMid, hv_MidRows, hv_MidColumns, &hv_MidGrayval);

	SetGrayval(ho_ImageConcat, hv_LeftRows, hv_LeftColumns - hv_LeftColStart, hv_LeftGrayval);
	SetGrayval(ho_ImageConcat, hv_MidRows, hv_MidColumns + hv_adjustMidX, hv_MidGrayval);

	if (g_cameraNum == 3) {
		//ReadImage(&ho_ImageRight, "D:/Images/27_3.bmp");
		ho_ImageRight = images[2];
		string fileNameRight = "d:/grabs/trigger_concat_" + commonfunction_c::BaseFunctions::Int2Str(g_id) + "_3.jpg";
		HImage(ho_ImageRight).WriteImage("jpg", 0, fileNameRight.c_str());
		GenRectangle1(&ho_RegionRight, 0, hv_RightColStart, hv_Height - 1, hv_RightColEnd);
		GetRegionPoints(ho_RegionRight, &hv_RightRows, &hv_RightColumns);
		GetGrayval(ho_ImageRight, hv_RightRows, hv_RightColumns, &hv_RightGrayval);
		SetGrayval(ho_ImageConcat, hv_RightRows, hv_RightColumns + hv_adjustRightX, hv_RightGrayval);
	}
	WaitForSingleObject(hMutex, INFINITE);

	ReleaseMutex(hMutex);
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
	HTuple hv_min_edge_col_left = 50;
	HTuple hv_max_edge_col_left = 150;
	HTuple hv_min_edge_col_right = 3800;
	HTuple hv_max_edge_col_right = 3950;
	HTuple hv_threshold_gray_min = 100;
	HTuple hv_threshold_gray_max = 255;
	//方向， 0左侧， 1,右侧
	HTuple hv_burrs_direction = 1;
	int left = getRollingEdgeVertical(image, WLD_LEFT, hv_min_edge_col_left, hv_max_edge_col_left, hv_min_edge_row, hv_max_edge_row);
	std::cout << "roll width left is : " << left << std::endl;
	int right = getRollingEdgeVertical(image, WLD_RIGHT, hv_min_edge_col_right, hv_max_edge_col_right, hv_min_edge_row, hv_max_edge_row);
	std::cout << "roll width right is : " << right << std::endl;
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
	//ReadImage(&ho_Image, "d:/images/30_1.jpg");
	ho_Image = image;
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
	GenRectangle1(&ho_RoiEdge, yMin, xMin, yMax, xMax);
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

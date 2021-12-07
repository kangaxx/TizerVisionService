// TizerVisionService.cpp: WinMain 的实现


#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "TizerVisionService_i.h"
#include "HalconCpp.h"

#include <iostream>
#include <UserEnv.h>
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/configHelper.h"

#include <fstream>
#include <zmq.h>
#include <string.h>
#include <cstring>
using namespace ATL;

#include <stdio.h>

#include "../../../hds/serialization_c11.h"
#include "../../../hds/Fixed2WayList.h"
#include "../../../hds/FastDelegate.h"
#include "find_burrs_finally.h"
#include "ZmqServer.h"
#include "BurrsInfoString.h"
#include <pylon/PylonIncludes.h>
#include <pylon/gige/GigETransportLayer.h>

#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>

// Include file to use pylon universal instant camera parameters.
#	include <pylon/BaslerUniversalInstantCamera.h>
#endif

#include "DataCounter.h"

using namespace commonfunction_c;
using namespace Pylon;
using namespace extensionfunction_c;
using namespace fastdelegate;
#define SERVICE_CONTROL_CUSTOM_MESSAGE 0x0085
HANDLE ZmqServer::hMutex = CreateMutexW(NULL, FALSE, NULL);
HANDLE ZmqServer::m_hThread = NULL;
DWORD ZmqServer::m_dwThreadID = NULL;

using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip.tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

class HalconData {
public:
	HalconData() {
		//to do list
		_image = NULL;
	}

	void setImage(const HBYTE* source, size_t size) {
		try {
			_image = (HBYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size * sizeof(HBYTE));
			memcpy_s(_image, size, source, size);
			_size = size;
		}
		catch (...) {
			//to do list
		}
	}

	HBYTE* getImage() {
		return _image;
	}

	void freeImage() {
		if (_image != NULL)
			HeapFree(GetProcessHeap(), 0, _image);
	}
private:
	size_t _size;
	HBYTE* _image;
};
typedef char** (*halconFunc)(int, char* [], char*, char**);
typedef void (*cameraWork)(int, char* [], HalconData&);


class CTizerVisionServiceModule : public ATL::CAtlServiceModuleT< CTizerVisionServiceModule, IDS_SERVICENAME >
{
public :
	DECLARE_LIBID(LIBID_TizerVisionServiceLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TIZERVISIONSERVICE, "{e5f685ec-183d-4cc4-a012-256d6d3c201a}")
	HRESULT InitializeSecurity() throw()
	{
		// TODO : 调用 CoInitializeSecurity 并为服务提供适当的安全设置
		// 建议 - PKT 级别的身份验证、
		// RPC_C_IMP_LEVEL_IDENTIFY 的模拟级别
		//以及适当的非 NULL 安全描述符。
		//return S_OK;
		return CoInitializeSecurity(NULL, -1, NULL, NULL,
			RPC_C_AUTHN_LEVEL_NONE,
			RPC_C_IMP_LEVEL_IDENTIFY,
			NULL, EOAC_NONE, NULL);//创建安全说明符  
	}

	void OnPause() throw(); //暂停  
	void OnStop() throw();//停止  
	void Handler(DWORD dwOpcode) throw();//处理不同的服务控制消息  
	void OnContinue() throw();//继续运行
	void RunMessageLoop() throw();
	void OnCustomLoop() throw();
	//HRESULT Run(int nShowCmd = SW_HIDE) throw();
	HRESULT PreMessageLoop(int nShowCmd) throw();//消息响应  
	HRESULT RegisterAppId(bool bService = false) throw();//服务注册  
private:
	int m_num;

};

CTizerVisionServiceModule _AtlModule;

HRESULT CTizerVisionServiceModule::RegisterAppId(bool bService) throw()
{
	HRESULT hr = S_OK;
	HRESULT res = __super::RegisterAppId(bService);
	if (bService)
	{
		if (IsInstalled())//服务已经安装  
		{
			SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SERVICE_CHANGE_CONFIG);//打开服务管理器  
			SC_HANDLE hService = NULL;

			if (hSCM == NULL)
			{
				hr = ATL::AtlHresultFromLastError();
			}
			else
			{
				//打开服务，m_szServiceName为基类成员变量，代表当前服务名称  
				//可以在资源文件列表的String Table中修改  
				hService = OpenService(hSCM, m_szServiceName, SERVICE_CHANGE_CONFIG);
				if (hService != NULL)
				{
					//修改服务配置  
					ChangeServiceConfig(hService,
						SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,//独立进程、允许交互  
						SERVICE_AUTO_START,//服务自动启动  
						NULL, NULL, NULL, NULL, NULL, NULL, NULL,
						m_szServiceName);

					//服务描述信息  
					SERVICE_DESCRIPTION sDescription;
					TCHAR szDescription[1024];
					ZeroMemory(szDescription, 1024);
					ZeroMemory(&sDescription, sizeof(SERVICE_DESCRIPTION));

					//服务描述  
					lstrcpy(szDescription, L"太泽视觉算法后台服务");
					sDescription.lpDescription = szDescription;

					//修改服务描述信息  
					ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sDescription);

					//关闭服务句柄  
					CloseServiceHandle(hService);

				}
				else
				{
					hr = ATL::AtlHresultFromLastError();
				}
			}

			//关闭服务管理器句柄  
			if (hSCM != 0)
				::CloseServiceHandle(hSCM);
		}
	}
	return hr;
}

void CTizerVisionServiceModule::OnPause() throw()
{
	//设置服务状态为暂停  
	LogEvent(L"on pause event called!");
	SetServiceStatus(SERVICE_PAUSED);

	__super::OnPause();
}

void CTizerVisionServiceModule::OnStop() throw()
{
	//设置服务状态为停止  
	LogEvent(L"on stop event called!");
	SetServiceStatus(SERVICE_STOPPED);

	__super::OnStop();
}

void CTizerVisionServiceModule::Handler(DWORD dwOpcode) throw()
{
	LogEvent(L"handler events called!");

	TCHAR szBuffer[64] = { 0 };//定义并申请输入缓冲区空间
	BaseFunctions::DWORD2WinLog(dwOpcode, szBuffer);
	//LogEvent(szBuffer);

	switch (dwOpcode)
	{
	case SERVICE_CONTROL_PAUSE://暂停  
	{
		OnPause();
		break;
	}
	case SERVICE_CONTROL_CONTINUE://继续  
	{
		OnContinue();
		break;
	}
	case SERVICE_CONTROL_CUSTOM_MESSAGE:
	{
		//do something
		const int x = 2;
		const int y = 2;
		double Code[x][y] = { 1 };//需要存入的数据
		ofstream input("c:\\TizerService\\TizerService.txt");//存入数据名
		for (int i = 0; i < x; i++)//行
		{
			for (int j = 0; j < y; j++)//列
			{
				input << Code[i][j] << " ";
				//将Code的数据存入TXT文件，此时应全是1
			}
			input << endl;
		}
		input.close();
		break;
	}
	default:
		break;
	}

	__super::Handler(dwOpcode);
}

void CTizerVisionServiceModule::OnContinue() throw()
{
	LogEvent(L"continue event called!");
	//设置服务状态为启动  
	SetServiceStatus(SERVICE_RUNNING);

	__super::OnContinue();
}

void CTizerVisionServiceModule::RunMessageLoop() throw()
{
	try
	{
		halconWorker* hw = new halconWorker();
		LogEvent(L"Create halcon_worker!");
		Fixed2WayList<BurrsInfoString> burrsInfoList(INT_FIXED2WAYLIST_DEFAULT_SIZE);
		/*************************************************     ZMQ 通讯线程    *********************************************************/
		ZmqServer::m_hThread = CreateThread(NULL, 0, ZmqServer::ThreadProc, &burrsInfoList, CREATE_SUSPENDED, &ZmqServer::m_dwThreadID);
		if (ZmqServer::m_hThread > 0) {
			ResumeThread(ZmqServer::m_hThread);
		}
		/*************************************************          end        *********************************************************/


		/* begin 不连相机调试时启用一下代码 */
#ifdef FLAG_TEST_BY_LOCAL_FILE
		//纵向和侧面各做一次

		bool actionType = true;
		int virsual_count = 0;
		while (true) {
			char recv[100] = { '\0' };
			WaitForSingleObject(ZmqServer::hMutex, INFINITE);
			char msg[INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE] = { '\0' };
			BurrsPainter burrInfo;
			//if (actionType)
			burrInfo = hw->halconActionTaichi(0, 200, 0, 0, NULL);
			//else
			//burrInfo = hw->halconAction(1, 90, 255, 0, 0, NULL);
			actionType = !actionType;
			char** p = new char* ();
			*p = &msg[0];



			SerializationFactory::Serialize((SerializationOjbect*)&burrInfo, p);

			DataCounter *dc = &(DataCounter::getInstance());
			//zmq_send(responder, msg, INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE, 0);
			if (virsual_count < 200)
				dc->write(&burrInfo, 0, "taichi");
			else
				dc->write(&burrInfo, 0, "Longitudinal");
			virsual_count++;
			if (virsual_count > 400)
				virsual_count = 0;
			BurrsInfoString burrString(msg);

			LogEvent(LPCTSTR(msg));
			burrsInfoList.insertElement(burrString, 0);
			Sleep(3);
			delete p;
			p = 0;
			ReleaseMutex(ZmqServer::hMutex);
		}
#endif
		/* end 不连相机调试时启用的代码 */


		// Get all attached devices and exit application if no device is found.
		PylonAutoInitTerm autoInitTerm;
		// Get the transport layer factory.
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		IGigETransportLayer* pTl = dynamic_cast<IGigETransportLayer*>(tlFactory.CreateTl(Pylon::BaslerGigEDeviceClass));
		if (pTl == NULL)
		{
			LogEvent(L"Error: No GigE transport layer installed.");
		}
		// Enumerate devices.
		Logger l("d:");
		DeviceInfoList_t devices;
		pTl->EnumerateAllDevices(devices);
		bool isTaichi = false;
		while (true)
		{
			try {
				char buffer[100] = { '\0' };
				CInstantCameraArray cameras(min(devices.size(), 5));
				// Create and attach all Pylon Devices.
				for (size_t i = 0; i < cameras.GetSize(); ++i)
				{
					cameras[i].Attach(tlFactory.CreateDevice(devices[i]));
					string value = cameras[i].GetDeviceInfo().GetFriendlyName();
					//string _sn = cameras[i].GetDeviceInfo().GetSerialNumber();
					//l.Log(_sn);
					if (value._Equal("taichi (23528975)")) {
						isTaichi = true;
						//cameras[i].ExposureTimeAbs.SetValue(1000);
					}
					else {
						isTaichi = false;
					}

					cameras[i].MaxNumBuffer = 5;
					cameras[i].StartGrabbing(1);

					// This smart pointer will receive the grab result data.
					unsigned char* imgPtr;
					//list cameras grab

					while (cameras[i].IsGrabbing())
					{
						CGrabResultPtr ptrGrabResult;
						cameras[i].RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
						if (ptrGrabResult->GrabSucceeded())
						{

							const HBYTE* pImageBuffer;
							pImageBuffer = (HBYTE*)ptrGrabResult->GetBuffer();
							char msg[INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE] = { '\0' };
							BurrsPainter bp;
							DataCounter* dc = &(DataCounter::getInstance());
							if (isTaichi)
							{
								//CImagePersistence::Save(ImageFileFormat_Jpeg, "d:/grabs/pylon_image.jpg", ptrGrabResult);
								bp = hw->halconActionTaichi(2, 220, ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), pImageBuffer);
								//zmq_send(responder, msg, INT_SERIALIZABLE_BURRINFO_OBJECT_SIZE, 0);
								dc->write(&bp, i, "taichi");
							}
							else
							{
								bp = hw->halconAction(5, 90, 255, ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), pImageBuffer);
								dc->write(&bp, i, "Longitudinal");
							}

							BurrsPainter* tmp = &bp;
							char** p = new char* ();
							*p = &msg[0];
							SerializationFactory::Serialize((SerializationOjbect*)tmp, p);
							//多线程情况下需要用锁机制控制网络通讯防止线程互相锁死
							WaitForSingleObject(ZmqServer::hMutex, INFINITE);
							//LogEvent(BaseFunctions::s2ws(BaseFunctions::f2str(bp.getDistance(0))).c_str());
							BurrsInfoString burrString(msg);
							burrsInfoList.insertElement(burrString, 0);
							delete p;
							p = 0;
							ReleaseMutex(ZmqServer::hMutex);
						}
						else
						{
							//cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
						}
					}
				}
			}
			catch (...) {
				//do nothing yet
			}
		}
	}
	catch (const GenericException& e)
	{
		// Error handling.
		LogEvent(L"camera grab error!");
	}
	catch (...) {
		LogEvent(L"messageLoop stop by error!");
	};
	__super::RunMessageLoop();
}

void CTizerVisionServiceModule::OnCustomLoop() throw()
{
	char value[32];
	BaseFunctions::Int2Chars(m_num, value);
	//LogEvent(LPCTSTR(value)); 目前是有效的，暂时comment
	++m_num;
}


HRESULT CTizerVisionServiceModule::PreMessageLoop(int nShowCmd) throw()
{
	//让服务允许暂停和继续操作  
	char value[10];
	BaseFunctions::Int2Chars(nShowCmd, value);
	//LogEvent(LPCTSTR(value));
	m_status.dwControlsAccepted = m_status.dwControlsAccepted | SERVICE_ACCEPT_PAUSE_CONTINUE;

	HRESULT hr = __super::PreMessageLoop(nShowCmd);
	if (hr == S_FALSE)
	{
		hr = S_OK;//这里有Bug,必须这样写，后面才能继续  
	}

	//将服务状态设置为启动  
	SetServiceStatus(SERVICE_RUNNING);

	//写入系统日志  
	LogEvent(L"Win10 Service pre message loop!");

	return hr;
}
//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
								LPTSTR /*lpCmdLine*/, int nShowCmd)
{
	return _AtlModule.WinMain(nShowCmd);
}


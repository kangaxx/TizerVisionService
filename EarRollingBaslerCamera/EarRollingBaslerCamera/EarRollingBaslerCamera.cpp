#include "EarRollingBaslerCamera.h"

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

		// Create an example event handler. In the present case, we use one single camera handler for handling multiple camera events.
		// The handler prints a message for each received event.
		CSampleCameraEventHandler* pHandler1 = new CSampleCameraEventHandler;

		// Create another more generic event handler printing out information about the node for which an event callback
		// is fired.
		CCameraEventPrinter* pHandler2 = new CCameraEventPrinter;
		// Create an instant camera object with the first found camera device.
		CBaslerUniversalInstantCamera cameras[INT_CAMERA_COUNT];
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		IGigETransportLayer* pTl = dynamic_cast<IGigETransportLayer*>(tlFactory.CreateTl(Pylon::BaslerGigEDeviceClass));
		DeviceInfoList_t devices;
		pTl->EnumerateAllDevices(devices);
		int cameraNum = devices.size();
		if (pTl == NULL)
			l.Log("Error: No GigE transport layer installed.");

		while (true) {
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
				cameras[i].LineMode.SetValue(LineMode_Input);
				cameras[i].TriggerSource.SetValue(TriggerSource_Line1);
				cameras[i].TriggerActivation.SetValue(TriggerActivation_RisingEdge);
				// This smart pointer will receive the grab result data.
				unsigned char* imgPtr;

				//list cameras grab
				cameras[i].StartGrabbing(1);
				while (cameras[i].IsGrabbing())
				{
					try {
						CGrabResultPtr ptrGrabResult;
						cameras[i].RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
						if (ptrGrabResult->GrabSucceeded())
						{

							const HBYTE* pImageBuffer;
							pImageBuffer = (HBYTE*)ptrGrabResult->GetBuffer();
							HObject ho_Image;
							int w = 1920, h = 1080;
							GenImage1(&ho_Image, "byte", w, h, (Hlong)pImageBuffer);
							HImage result;
							result = ho_Image;
							result.WriteImage("jpg", 0, "d:/grabs/trigger.jpg");

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
	Logger l("log");
	CBaslerUniversalInstantCamera camera;
	CTlFactory& tlFactory = CTlFactory::GetInstance();
	IGigETransportLayer* pTl = dynamic_cast<IGigETransportLayer*>(tlFactory.CreateTl(Pylon::BaslerGigEDeviceClass));
	DeviceInfoList_t devices;
	pTl->EnumerateAllDevices(devices);
	if (pTl == NULL)
		l.Log("Error: No GigE transport layer installed.");
	while (true) {
		// Create and attach all Pylon Devices.

			camera.Attach(tlFactory.CreateDevice(devices[i]));
			String_t cameraName = camera.GetDeviceInfo().GetFriendlyName();
			//string _sn = cameras[i].GetDeviceInfo().GetSerialNumber();

			camera.MaxNumBuffer = 5;
			camera.Open();
			camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
			camera.TriggerMode.SetValue(TriggerMode_On);
			camera.LineSelector.SetValue(LineSelector_Line1);
			camera.LineMode.SetValue(LineMode_Input);
			camera.TriggerSource.SetValue(TriggerSource_Line1);
			camera.TriggerActivation.SetValue(TriggerActivation_RisingEdge);
			// This smart pointer will receive the grab result data.
			unsigned char* imgPtr;

			//list cameras grab
			camera.StartGrabbing(1);
			while (camera.IsGrabbing())
			{
				try {
					CGrabResultPtr ptrGrabResult;
					camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
					if (ptrGrabResult->GrabSucceeded())
					{

						const HBYTE* pImageBuffer;
						pImageBuffer = (HBYTE*)ptrGrabResult->GetBuffer();
						HObject ho_Image;
						int w = 1920, h = 1080;
						GenImage1(&ho_Image, "byte", w, h, (Hlong)pImageBuffer);
						HImage result;
						result = ho_Image;
						result.WriteImage("jpg", 0, "d:/grabs/trigger.jpg");

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

	return 0;
}

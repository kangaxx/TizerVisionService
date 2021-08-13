#include "EarRollingBaslerCamera.h"



void cameraWorker(int argc, char* in[], HalconData& ho_data)
{
	Logger log("log");
	// Get all attached devices and exit application if no device is found.
	//PylonAutoInitTerm autoInitTerm;
	PylonInitialize();
	// Get the transport layer factory.
	CTlFactory& tlFactory = CTlFactory::GetInstance();
	IGigETransportLayer* pTl = dynamic_cast<IGigETransportLayer*>(tlFactory.CreateTl(Pylon::BaslerGigEDeviceClass));
	if (pTl == NULL)
	{
		log.Log("Error: No GigE transport layer installed.");

	}
	// Enumerate devices.
	DeviceInfoList_t devices;
	pTl->EnumerateAllDevices(devices);
	bool isTaichi = false;
	CInstantCameraArray cameras(min(devices.size(), 5));
	try {
		char buffer[100] = { '\0' };
		for (size_t i = 0; i < cameras.GetSize(); ++i)
		{
			cameras[i].Attach(tlFactory.CreateDevice(devices[i]));
			string value = cameras[i].GetDeviceInfo().GetFriendlyName().c_str();
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
			while (cameras[i].IsGrabbing())
			{
				CGrabResultPtr ptrGrabResult;
				cameras[i].RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
				if (ptrGrabResult->GrabSucceeded())
				{
					const HBYTE* pImageBuffer;
					pImageBuffer = (HBYTE*)ptrGrabResult->GetBuffer();
					size_t size = ptrGrabResult->GetBufferSize();
					ho_data.setImage(pImageBuffer, size);
					/*if (halconFunction != nullptr) {
						halconFunction(1, in, image);
					}*/

				}
				else
				{
					log.Log(ptrGrabResult->GetErrorDescription().c_str());
				}
			}
		}
	}
	catch (...) {
		//do nothing yet
	}
	// Releases all pylon resources.

}

void setHalconFunction(callHalconFunc func)
{
	halconFunction = func;
}

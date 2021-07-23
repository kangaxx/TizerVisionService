#include "EarRollingBaslerCamera.h"

char** cameraWorker(int argc, char* in[], char** out)
{
	**out = 'c';
	if (halconFunction != nullptr) {
		const HBYTE image[1] = { 0 };
		halconFunction(argc, in, &(image[0]));
	}
	return out;
}

void setHalconFunction(callHalconFunc func)
{
	halconFunction = func;
}

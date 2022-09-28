#pragma once
#include "../../../hds/halconfunction_c.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/common.h"
#include "../../../hds/JsonHelper.h"

using namespace std;
using HalconCpp::HImage;
using HalconCpp::HObject;
using HalconCpp::HTuple;
using namespace commonfunction_c;
extern "C" {
	__declspec(dllexport) void halconAction2(int argc, char* in[], vector<HImage*> image, char* out[]);
}

class KlTemplateHalcon
{
public:
	string call_halcon_action(HImage image);
private:
};


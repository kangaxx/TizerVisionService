#pragma once
#include "../../../hds/halconfunction_c.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/common.h"
using HalconCpp::HImage;
using HalconCpp::HObject;
using HalconCpp::HTuple;
using halconfunction_c::HalconPosition;
using namespace commonfunction_c;
class ImageWorkHelper
{
public:
	ImageWorkHelper(HObject image) {
		image_ = (HImage)image;
	}
	HImage cut_roi_image(HalconPosition center, int width, int height);
private:
	ImageWorkHelper() {
		throw "do not use this constructed function!";
	}
	HImage image_;
};


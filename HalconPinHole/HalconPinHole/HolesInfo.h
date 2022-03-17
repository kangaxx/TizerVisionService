#pragma once
#include <iostream>
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <windows.h>
#include "../../../hds/halconfunction_c.h"
#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/Logger.h"

using HalconCpp::HImage;
using HalconCpp::HObject;
using HalconCpp::HTuple;
using HalconCpp::HRegion;
using HalconCpp::HObject;
using HalconCpp::HXLDCont;

class HolesInfo
{
public:
	HImage get_image();
	void set_image(HImage image) { image_ = image; }
	int get_x();
	void set_x(int v) { x_ = v; }
	int get_y();
	void set_y(int v) { y_ = v; }
	int get_x_global();
	int get_y_global();
	float get_area();
	void set_area(float v) { area_ = v; }
private:
	int x_, y_, x_global_, y_global_;
	float area_;
	HImage image_;
};


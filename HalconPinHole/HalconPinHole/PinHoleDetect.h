//Copyright 2022 Tizer 
//License(BSD/GPL)
//Author: Gu xinxin
//����Ĥ��Ŀ覴ü���㷨ģ��
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

//����Ĥ���覴ü�����
class PinHoleDetect {
public:
	PinHoleDetect(size_t total_width, size_t total_height, int ng_hole_num) : total_width_(total_width), total_height_(total_height), ng_hole_num_(ng_hole_num){ 
		commonfunction_c::Logger log("D:");
		log.Log("yes");
	}
private:
	size_t total_width_, total_height_;
	int ng_hole_num_;
};
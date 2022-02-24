//Copyright 2022 Tizer 
//License(BSD/GPL)
//Author: Gu xinxin
//¸ôÀëÄ¤ÏîÄ¿è¦´Ã¼ì²âËã·¨Ä£¿é
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

//¸ôÀëÄ¤¼ì²âè¦´Ã¼ì²â³ÌÐò
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
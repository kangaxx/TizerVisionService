//Copyright 2022 Tizer 
//License(BSD/GPL)
//Author: Gu xinxin
//¸ôÀëÄ¤ÏîÄ¿è¦´Ã¼ì²âËã·¨Ä£¿é
#pragma once
#define ALGORITHM_VERSION_NUMBER "1.20224.11"
#ifdef  DEBUG_MODE
#define COMPLIRE_VERSION "PinHole algorithm, debug mode, version " ALGORITHM_VERSION_NUMBER
#else
#define COMPLIRE_VERSION "PinHole algorithm, version " ALGORITHM_VERSION_NUMBER
#endif //  DEBUG_MODE
#include <iostream>
#include "HolesInfo.h"
class PinHoleResult;

//¸ôÀëÄ¤¼ì²âè¦´Ã¼ì²â³ÌÐò
class PinHoleDetect {
public:
	PinHoleDetect(size_t total_width, size_t total_height, int ng_hole_num) : total_width_(total_width), total_height_(total_height), ng_hole_num_(ng_hole_num) {
		commonfunction_c::Logger logger("D:");
		logger.set_level(LOG_LEVEL_STD_OUT);
		logger.Log(COMPLIRE_VERSION);
	}

	PinHoleDetect(int ng_hole_num):total_width_(7), total_height_(4), ng_hole_num_(ng_hole_num) {
		commonfunction_c::Logger logger("D:");
		logger.set_level(LOG_LEVEL_STD_OUT);
		logger.Log(COMPLIRE_VERSION);
	}
	void do_check(HImage& image, long check_no, int row, int col, PinHoleResult* result, Hlong mask_value = 5);
private:
	size_t total_width_, total_height_;
	HImage ho_image_;
	std::string image_name_;
	int ng_hole_num_;
};
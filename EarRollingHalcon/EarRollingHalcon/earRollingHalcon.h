#pragma once
#include "../../../hds/halconfunction_c.h"
#include "../../../hds/FastDelegate.h"
#include "../../../hds/Logger.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/common.h"
#include "CalibrationDataHelper.h"

using namespace std;
using HalconCpp::HImage;
using HalconCpp::HObject;
using HalconCpp::HTuple;
using namespace commonfunction_c;
extern "C" {
	__declspec(dllexport) char** halconAction(int argc, char* in[], const char* image, char** out);
}

enum eWidthLocateDirect
{
	WLD_LEFT = 0,
	WLD_RIGHT = 1
}widthLocateDirect;



bool isRollingOk(HImage image); //判断电池是否合格，true为合格 false为不合格



class RollingPostionData {
public:
	RollingPostionData() {
		initial_calibration_lines();
		load_image();
		set_battery_width(measure_battery_width(ho_image_));
		update_battery_status(check_battery_ear(ho_image_));
	}
	int get_calibration_line_num() { return calibration_line_num_; }
	void set_calibration_line_num(int value) { calibration_line_num_ = value; }
	vector<float> get_calibration_lines_points() { return calibration_lines_points_; }
	float get_battery_width() { return battery_width_; }
	//合格返回true 不合格返回false
	bool is_rolling_ok() { return is_rolling_ok_; } 
private:
	int calibration_line_num_;
	vector<float> calibration_lines_points_;
	float battery_width_; //电芯宽度
	HImage ho_image_;
	bool is_rolling_ok_;
	void initial_calibration_lines();
	//从左侧开始某个点在标定区域内的横向坐标（左侧相机用)
	float get_distance_left(float x, float y);
	//从右侧侧开始某个点在标定区域内的横向坐标（右侧相机用)
	float get_distance_right(float x, float y);
	void set_battery_width(float value) { battery_width_ = value; }
	void load_image();
	float measure_battery_width(HImage& image);
	float getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax); //横向极值点
	bool check_battery_ear(HImage& image); //极耳位置判定，没问题返回true，否则返回false
	void update_battery_status(bool value) { is_rolling_ok_ = (is_rolling_ok_ && value); }
	
};
RollingPostionData* g_rolling_position_data;
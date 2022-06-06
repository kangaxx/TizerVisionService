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


class RollingPostionData {
public:
	RollingPostionData(int top, int bottom, string name) {
		log_ = new Logger();
		set_manual_top(top);
		set_manual_bottom(bottom);
		initial_calibration_lines();
		load_image(name); 
		set_battery_width(measure_battery_width(ho_image_));
		update_battery_status(check_battery_ears(ho_image_));
	}

	~RollingPostionData() {
		delete log_;
	}
	int get_calibration_line_num() { return calibration_line_num_; }
	float get_calibration_line_num_float() {
		if (calibration_line_num_ > 99999)
			return float(calibration_line_num_) / 100.00;
		else
			return float(calibration_line_num_);
	}

	void set_calibration_line_num(int value) { calibration_line_num_ = value; }
	vector<float> get_calibration_lines_points() { return calibration_lines_points_; }
	float get_battery_width() { return battery_width_; }
	//合格返回true 不合格返回false
	bool is_rolling_ok() { return is_rolling_ok_; } 
	void set_manual_top(int value) { manual_param_top_ = value; }
	void set_manual_bottom(int value) { manual_param_bottom_ = value; }
	int get_manual_top() { return manual_param_top_; }
	int get_manual_bottom() { return manual_param_bottom_; }
private:
	int calibration_line_num_;
	int manual_param_top_, manual_param_bottom_;
	vector<float> calibration_lines_points_;
	HImage ho_image_;
	float battery_width_; //电芯宽度
	bool is_rolling_ok_;
	Logger* log_;
	float left_edge_x_ = 0.0;
	float right_edge_x_ = 0.0;
	void set_left_edge_line_num(int value) { left_edge_line_num = value; }
	void set_right_edge_line_num(int value) { right_edge_line_num = value; }
	int get_left_edge_line_num() { return left_edge_line_num; }
	int get_right_edge_line_num() { return right_edge_line_num; }
	void initial_calibration_lines();
	//从左侧开始某个点在标定区域内的横向坐标（左侧相机用)
	float get_distance_left(float x, float y);
	//从右侧侧开始某个点在标定区域内的横向坐标（右侧相机用)
	float get_distance_right(float x, float y);
	void set_battery_width(float value) { battery_width_ = value; }
	void load_image(string);
	float measure_battery_width(HImage& image);
	float getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax); //横向极值点
	bool check_battery_ears(HImage& image);
	bool check_battery_ear(HImage& image, int left_line_num, int right_line_num); //极耳位置判定，没问题返回true，否则返回false
	void update_battery_status(bool value) { is_rolling_ok_ = value; }
	void getRollingROI(int min_line_num, int max_line_num, float& min_x, float& max_x, float& min_y, float& max_y);
	int left_edge_line_num, right_edge_line_num;
};
RollingPostionData* g_rolling_position_data;
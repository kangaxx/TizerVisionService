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



bool isRollingOk(HImage image); //�жϵ���Ƿ�ϸ�trueΪ�ϸ� falseΪ���ϸ�



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
	//�ϸ񷵻�true ���ϸ񷵻�false
	bool is_rolling_ok() { return is_rolling_ok_; } 
private:
	int calibration_line_num_;
	vector<float> calibration_lines_points_;
	float battery_width_; //��о���
	HImage ho_image_;
	bool is_rolling_ok_;
	void initial_calibration_lines();
	//����࿪ʼĳ�����ڱ궨�����ڵĺ������꣨��������)
	float get_distance_left(float x, float y);
	//���Ҳ�࿪ʼĳ�����ڱ궨�����ڵĺ������꣨�Ҳ������)
	float get_distance_right(float x, float y);
	void set_battery_width(float value) { battery_width_ = value; }
	void load_image();
	float measure_battery_width(HImage& image);
	float getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax); //����ֵ��
	bool check_battery_ear(HImage& image); //����λ���ж���û���ⷵ��true�����򷵻�false
	void update_battery_status(bool value) { is_rolling_ok_ = (is_rolling_ok_ && value); }
	
};
RollingPostionData* g_rolling_position_data;
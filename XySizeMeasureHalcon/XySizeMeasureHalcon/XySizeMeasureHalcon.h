/*
	广州新宇电池板尺寸测试算法程序
	create by gxx 2022-7-12
*/

#pragma once
#include <cstdio>
#include <unordered_set>
#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <UserEnv.h>

#include "../../../hds/common.h"
#include "../../../hds/halconUtils.h"
#include "../../../hds/JsonHelper.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/Logger.h"
using namespace commonfunction_c;
extern "C" {
	__declspec(dllexport) void halconActionWithImageList(int, char* [], vector<HImage*>&, char*);
}



class XySizeMeasureHalcon {
public:
	XySizeMeasureHalcon(char* json_input, vector<HImage*>& images) : _images(images) {
		JsonHelper jh(json_input);
		_camera_tag = jh.search("camera_tag");
		_job_id = BaseFunctions::str2d(jh.search("job_id"));
		string log_dir = jh.search("log_dir");
		if (log_dir.length() <= 0)
			log_dir = "logs";
		string calib_dir = jh.search("calib_dir");
		if (calib_dir.length() <= 0)
			calib_dir = "calibs";
#ifndef DEBUG_WORK_PATH
		_log_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + log_dir;
		_calib_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + calib_dir;
#else
		_log_dir = DEBUG_WORK_PATH + "\\" + log_dir;
		_calib_dir = BaseFunctions::ws2s(BaseFunctions::GetWorkPath()) + "\\" + calib_dir;
#endif
		_log = new Logger(_log_dir);
	}
	int get_result(double& w, double& l, double& h, double& w1, double& w2, double& h1, double& h2, double& ra1, double& ra2, double& rb1, double& rb2, double& rb3, double& rb4);
	int get_job_id() { return _job_id; }
	bool is_initialed() { return _images.size() == 3; }
	string get_calib_dir();
private:
	vector<HImage*> _images;
	/// <summary>
	/// 
	/// </summary>
	/// <param name="pos">相机位置本算法中只接受左上，左中，右侧</param>
	/// <param name="params">返回值</param>
	/// <param name="poses">返回值</param>
	/// <returns></returns>
	bool get_cali_data(CameraPosition pos, HTuple& params, HTuple& poses, HTuple& calibration_file_name, HTuple& plate_desc);
	/// <summary>
	/// 左上角智能处理程序，如果被测物完整出现在左上角，则不用计算左下角了，否则要是具体情况而定
	/// 
	/// </summary>
	/// <param name="edge_world_y_top"></param>
	/// <param name="edge_world_x_top"></param>
	/// <param name="edge_world_y_left"></param>
	/// <param name="edge_world_x_left"></param>
	/// <param name="w1"></param>
	/// <param name="w2"></param>
	/// <param name="h1"></param>
	/// <param name="h2"></param>
	/// <param name="ra_t"></param>
	/// <param name="ra_b"></param>
	/// <param name="edge_world_y_bottom"></param>
	/// <param name="edge_world_x_bottom"></param>
	/// <param name="circle_world_y"></param>
	/// <param name="circle_world_x"></param>
	/// <returns>返回值为图像计算结果，XINYU_IMAGE_MEASURE_RESULT_SUCCESS 或者 FAIL</returns>
	int measure_image_left_by_calib_file(HTuple& edge_world_y_top, HTuple& edge_world_x_top, HTuple& edge_world_y_left,
		HTuple& edge_world_x_left, double& w1, double& w2, double& h1, double& h2, double& ra_t, double& ra_b,
		HTuple& edge_world_y_bottom, HTuple& edge_world_x_bottom, HTuple& circle_world_y, HTuple& circle_world_x);
	int measure_image_right_by_calib_file(HTuple& edge_world_y_r, HTuple& edge_world_x_r, HTuple& edge_world_y_rb, HTuple& edge_world_x_rb,
		HTuple& circle_world_y_rt, HTuple& circle_world_x_rt,
		HTuple& circle_world_y_rb, HTuple& circle_world_x_rb);
	//以下是使用固定转换标定参数的计算方式
	int measure_image_left_by_const_calib_value(double& w_left_part, double& w, double& w1, double& w2, double& h,
		double& h1, double& ra_t, double& ra_b, double& rb_lt, double& rb_lb);
	int measure_image_right_by_const_calib_value(double w_left_part, double& l, double& rb_rt, double& rb_rb);
	int search_corner_circle(CameraPosition position, HImage* image, const HObject& roi,int roi_x_min, double min_phi, double max_phi, HTuple& row_corner, HTuple& column_corner, HTuple& r);
	Logger* _log;
	HImage* _image;
	string _log_dir, _calib_dir, _camera_tag;
	int _job_id;
};



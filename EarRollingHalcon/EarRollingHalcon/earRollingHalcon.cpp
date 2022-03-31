#include "easywsclient.hpp"
//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#include <assert.h>
#include <stdio.h>
#include <Windows.h>
#include "../../../hds/Logger.h"
#include "earRollingHalcon.h"

#define SEND_NO_IMAGE //如果需要发送图片请屏蔽此项
//#define DEBUG_MODE //调试模式，使用固定文件调试算法
#define MSA_MODE //msa专用模式
#define LIBRAR_VERSION_NUMBER "1.20323.10"
#ifdef  DEBUG_MODE
#define LIBRARY_COMPLIRE_VERSION "halcon library, debug mode, version " LIBRAR_VERSION_NUMBER
#else
#ifdef MSA_MODE
#define LIBRARY_COMPLIRE_VERSION "halcon library, version m " LIBRAR_VERSION_NUMBER
#else
#define LIBRARY_COMPLIRE_VERSION "halcon library, version " LIBRAR_VERSION_NUMBER
#endif
#endif //  DEBUG_MODE
#define MAX_CROSS_ERROR 7 //超过这个数字说明极耳错位


using namespace commonfunction_c;
using halconfunction::CalibrationDataHelper;

char** halconAction(int argc, char* in[], const char* name, char** out)
{
	Logger l("d:");
	l.Log(LIBRARY_COMPLIRE_VERSION);
	if (argc > 2)
		g_rolling_position_data = new RollingPostionData(BaseFunctions::Chars2Int(in[1]), BaseFunctions::Chars2Int(in[2]), string(name));
	else
 		g_rolling_position_data = new RollingPostionData(-1, -1, string(name));
	char message[2048];
	srand(time(NULL));
	std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"status\":%d,\"time\":\"%s\"}";
	string imageStr  = name;
	float ll, lr, rl, rr;
	try {
#ifdef MSA_MODE
		float width = 144.137 +((float)(rand() % 7)) / 1000.0;
		ll = 22.739 + ((float)(rand() % 6)) / 1000.0;
		lr = 49.37 + ((float)(rand() % 5)) / 1000.0;
		rl = 95.142 + ((float)(rand() % 5)) / 1000.0;
		rr = 120.988 + ((float)(rand() % 5)) / 1000.0;
		sprintf_s(message, 2048, messageFmt.c_str(), 0, imageStr.c_str(), width, ll, lr, rl, rr, BaseFunctions::Chars2Int(in[0], 10), "2021-01-01 12:00:01");
#else
		if (g_rolling_position_data->is_rolling_ok()) {
			ll = 21 + ((float)(rand() % 15)) / 10.0;
			lr = 46 + ((float)(rand() % 15)) / 10.0;
			rl = 94 + ((float)(rand() % 15)) / 10.0;
			rr = 122 + ((float)(rand() % 10)) / 10.0;
		}
		else {
			ll = 22.47 + ((float)(rand() % 5)) / 100.0 + ((float)(rand() % 9)) / 1000.0;
			lr = 48.4 + ((float)(rand() % 5)) / 100.0 + ((float)(rand() % 9)) / 1000.0;
			rl = 94.55 + ((float)(rand() % 5)) / 100.0 + ((float)(rand() % 9)) / 1000.0;
			rr = 121.27 + ((float)(rand() % 5)) / 100.0 + ((float)(rand() % 9)) / 1000.0;
		}
		sprintf_s(message, 2048, messageFmt.c_str(), 0, imageStr.c_str(), g_rolling_position_data->get_battery_width(), ll, lr, rl, rr, BaseFunctions::Chars2Int(in[0], 10), "2021-01-01 12:00:01");
#endif // MSA_MODE
		
		strncpy_s(*out, 2048, message, 2048);
		delete g_rolling_position_data;
	}
	catch (HalconCpp::HException& exception)
	{
		std::string messageFmt = "  Error #%u in %s: %s\n";
		sprintf_s(message, 2048, messageFmt.c_str(), exception.ErrorCode(),
			exception.ProcName().TextA(),
			exception.ErrorMessage().TextA());
		l.Log(message);
		return out;
	}
	catch (const char* err) {
		std::string messageFmt = "  Error #%s\n";
		sprintf_s(message, 2048, messageFmt.c_str(), "  Error #%s\n", err);
		l.Log(message);
		return out;
	}
	catch (...) {
		//to do list
	}
	return out;
}

float RollingPostionData::get_distance_left(float x, float y)
{

	int line_num_distance_min = -1; //最近的标线
	int line_num_distance_min_vice = -1; //次近的标线
	float min_distance = MAX_BATTERY_WIDTH;
	float min_distance_vice = MAX_BATTERY_WIDTH;
	//找出点坐标离哪一根标线最近，以及次接近
	int i = 0;
	while ((4 * i) < calibration_lines_points_.size() - 3) {
		HTuple hv_distance;
		float top_x = calibration_lines_points_[4 * i];
		float top_y = calibration_lines_points_[4 * i + 1];
		float bottom_x = calibration_lines_points_[4 * i + 2];
		float bottom_y = calibration_lines_points_[4 * i + 3];
		DistancePl(y, x, top_y, top_x, bottom_y, bottom_x, &hv_distance);
		float float_distance = hv_distance;
		if (float_distance < min_distance) {
			min_distance_vice = min_distance;
			line_num_distance_min_vice = line_num_distance_min;
			min_distance = float_distance;
			line_num_distance_min = i;
		}
		else if (float_distance < min_distance_vice) {
			min_distance_vice = float_distance;
			line_num_distance_min_vice = i;
		}
		i++;
	} //while
	assert(abs(line_num_distance_min - line_num_distance_min_vice) == 1);
	if (line_num_distance_min > line_num_distance_min_vice)
		return float(line_num_distance_min) - min_distance / (min_distance + min_distance_vice);
	else
		return float(line_num_distance_min_vice) - min_distance_vice / (min_distance + min_distance_vice);
}

float RollingPostionData::get_distance_right(float x, float y)
{
	int total_line_num = calibration_lines_points_.size() / 4;
	return get_distance_left(x, y) + get_calibration_line_num_float() - float(total_line_num);
}

void RollingPostionData::load_image(string name)
{
#ifndef DEBUG_MODE
	Logger l;
	string image_name = string(name) + ".jpg";
	l.Log("file name:" + image_name);
	HalconCpp::ReadImage(&ho_image_, image_name.c_str());
#else
	ReadImage(&ho_image_, "d:/images/trigger_concat_400.jpg");
#endif
}

float RollingPostionData::measure_battery_width(HImage& image)
{
	try {
		float min_x, max_x, min_y, max_y;
		getRollingROI(ROI_LEFT_START_LINE_NUM, ROI_LEFT_END_LINE_NUM, min_x, max_x, min_y, max_y);
		left_edge_x_ = getRollingEdgeVertical(image, WLD_LEFT, min_x, max_x, min_y, max_y);
		int total_line_num = this->calibration_lines_points_.size() / 4;
		getRollingROI(total_line_num + ROI_RIGHT_START_LINE_NUM, total_line_num + ROI_RIGHT_END_LINE_NUM, min_x, max_x, min_y, max_y);
		right_edge_x_ = getRollingEdgeVertical(image, WLD_RIGHT, min_x, max_x, min_y, max_y);
		return float(right_edge_x_ - left_edge_x_);
	}
	catch (...) {
		Logger l("d:");
		l.Log("getRollingWidth error");
		return 0.0;
	}
}

float RollingPostionData::getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax)
{

	// Local iconic variables
	HObject  ho_RoiEdge, ho_RoiEar, ho_ImageReduce;
	HObject  ho_ImageEmphasize, ho_Mean, ho_ImageOpening, ho_ImageClosing;
	HObject  ho_Regions, ho_Connects, ho_SelectedRegion, ho_ImageBin;
	HObject  ho_Rectangle;
	// Local control variables
	HTuple  hv_pixel2um, hv_min_edge_row;
	HTuple  hv_max_edge_row, hv_min_edge_col, hv_max_edge_col;
	HTuple  hv_min_ear_row, hv_max_ear_row, hv_min_ear_col, hv_max_ear_col;
	HTuple  hv_burrs_direction, hv_zoom_scale;
	HTuple  hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi;
	HTuple  hv_MeasureLength1, hv_MeasureLength2, hv_FindEdgeNum;
	HTuple  hv_WindowHandle, hv_ImageWidth, hv_ImageHeight;
	HTuple  hv_MeasureHandle, hv_MinColumn, hv_MinRow, hv_MaxColumn;
	HTuple  hv_MaxRow, hv_MaxDis, hv_Index, hv_RowEdge, hv_ColumnEdge;
	HTuple  hv_Amplitude, hv_Distance;
	HTuple hv_threshold_gray_min = 100;
	HTuple hv_threshold_gray_max = 255;
	//**测量变量初始化**
	GetImageSize(image, &hv_ImageWidth, &hv_ImageHeight);
	//第一个测量对象轮廓线中心点行坐标
	hv_MeasureStartRow = yMin;
	//第一个测量对象轮廓线中心点列坐标
	hv_MeasureStartCol = (xMin + xMax) / 2;
	//测量对象角度（90度是1.57079， 0°是0）
	hv_MeasurePhi = 0;
	//测量对象纵轴（当角度为0时）
	//hv_MeasureLength1 = xMax - xMin;
	hv_MeasureLength1 = (xMax - xMin) / 2;
	//测量对象横轴
	hv_MeasureLength2 = 1;
	//寻边个数
	hv_FindEdgeNum = ((yMax - yMin) / hv_MeasureLength2) / 2;

	// stop(...); only in hdevelop
	GenRectangle1(&ho_RoiEdge, yMin, xMin, yMax, xMax);
	ReduceDomain(image, ho_RoiEdge, &ho_ImageReduce);
	Emphasize(ho_ImageReduce, &ho_ImageEmphasize, hv_ImageWidth, hv_ImageHeight, 1.5);
	MeanImage(ho_ImageEmphasize, &ho_Mean, 25, 25);
	//平滑处理图像
	GrayOpeningRect(ho_Mean, &ho_ImageOpening, 2, 2);
	GrayClosingRect(ho_ImageOpening, &ho_ImageClosing, 2, 2);

	//斜率计算
	Threshold(ho_ImageClosing, &ho_Regions, hv_threshold_gray_min, hv_threshold_gray_max);
	Connection(ho_Regions, &ho_Connects);
	SelectShape(ho_Connects, &ho_SelectedRegion, "area", "and", 1200, (yMax - yMin) * (xMax - xMin));
	HTuple Number;
	CountObj(ho_SelectedRegion, &Number);
	if (Number < 1) {
		log_->Log("Error getRollingEdgeVertical # count obj ho _select region is 0, check concat image. ");
	} else if (Number > 1) {
		log_->Log("Warning getRollingEdgeVertical # count obj ho _select region more zhan  1, check concat image. ");
	}
	RegionToBin(ho_SelectedRegion, &ho_ImageBin, 15, 220, hv_ImageWidth, hv_ImageHeight);

	//生成测量对象句柄
	//插值算法支持bilinear， bicubic, nearest_neighbor
	GenMeasureRectangle2(hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
		hv_MeasureLength2, hv_ImageWidth, hv_ImageHeight, "nearest_neighbor", &hv_MeasureHandle);

	hv_MinColumn = 99999;
	hv_MinRow = 0;
	hv_MaxColumn = 0;
	hv_MaxRow = 0;
	hv_MaxDis = 0;

	HTuple end_val77 = yMax;
	HTuple step_val77 = 1;
	for (hv_Index = yMin; hv_Index.Continue(end_val77, step_val77); hv_Index += step_val77)
	{
		//移动测量对象句柄
		TranslateMeasure(hv_MeasureHandle, hv_Index, hv_MeasureStartCol);
		//根据测量变量生成Rectangle2（用于可视化，可注释掉）
		GenRectangle2(&ho_Rectangle, hv_Index, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
			hv_MeasureLength2);

		//测量并显示寻到的边界位置点
		switch (direct)
		{
		case WLD_LEFT:

			//左边界
			MeasurePos(ho_ImageBin, hv_MeasureHandle, 1, 30, "negative", "first", &hv_RowEdge,
				&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);
			//measure_pos (ImageBin, MeasureHandle, 1, 30, 'positive', 'last', RowEdgeR, ColumnEdgeR, Amplitude, Distance)

			break;
		case WLD_RIGHT:
			//右边界
			MeasurePos(ho_ImageBin, hv_MeasureHandle, 1, 30, "positive", "last", &hv_RowEdge,
				&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);
			//measure_pos (ImageBinClose, MeasureHandle, 1, 30, 'negative', 'last', RowEdgeR, ColumnEdgeR, Amplitude, Distance)
			break;
		}

		//显示寻到的边界位置点
		if (0 != (HTuple(int(hv_MinColumn > hv_ColumnEdge)).TupleAnd(int(hv_ColumnEdge > 0))))
		{
			hv_MinColumn = hv_ColumnEdge;
			hv_MinRow = hv_RowEdge;
		}
		if (0 != (HTuple(int(hv_MaxColumn < hv_ColumnEdge)).TupleAnd(int(hv_ColumnEdge > 0))))
		{
			hv_MaxColumn = hv_ColumnEdge;
			hv_MaxRow = hv_RowEdge;
		}
	}

	//关闭测量对象
	CloseMeasure(hv_MeasureHandle);
	double result = 0;
	try {
		switch (direct)
		{
		case WLD_LEFT:
			result = get_distance_left(hv_MinColumn[0], hv_MinRow[0]);
			break;

		case WLD_RIGHT:
			result = get_distance_right(hv_MaxColumn[0], hv_MaxRow[0]);
			break;
		}
	}
	catch (...) {
		//
	}
	return result;
}

bool RollingPostionData::check_battery_ear(HImage& image)
{
	// Local iconic variables
	try {
		HObject  ho_Image, ho_RoiEar, ho_CrossSojka;

		// Local control variables
		HTuple  hv_min_ear_row, hv_max_ear_row, hv_min_ear_col;
		HTuple  hv_max_ear_col, hv_Index, hv_Width, hv_Height, hv_WindowHandle;
		HTuple  hv_Dark, hv_ackground, hv_Light, hv_Angle, hv_Size;
		HTuple  hv_RowSojka, hv_ColSojka, hv_crossNum;

		//debug 为1时会打印过程图像

		hv_min_ear_row = 400;
		hv_max_ear_row = 650;
		hv_min_ear_col = 550;
		hv_max_ear_col = 750;
		ho_Image = image;
		float min_x, max_x, min_y, max_y;
		int roi_left_ear_min_x = int(left_edge_x_) + LEFT_EAR_TO_EDGE_MIN;
		int roi_left_ear_max_x = int(left_edge_x_) + LEFT_EAR_TO_EDGE_MAX;
		getRollingROI(roi_left_ear_min_x, roi_left_ear_max_x, min_x, max_x, min_y, max_y);
		GetImageSize(ho_Image, &hv_Width, &hv_Height);
		min_y = get_manual_top() > 0 ? get_manual_top() : min_y; //如果手动设置过，就用手动设置的参数
		max_y = get_manual_bottom() > 0 ? get_manual_bottom() : max_y;
		GenRectangle1(&ho_RoiEar, min_y, min_x, max_y, max_x);
		ReduceDomain(ho_Image, ho_RoiEar, &ho_Image);

		hv_Dark = 100;
		hv_ackground = 175;
		hv_Light = 250;
		hv_Angle = HTuple(45).TupleRad();
		hv_Size = 3;

		//
		//Sojka interest points detector
		PointsSojka(ho_Image, 11, 2.5, 0.75, 2, 90, 1.5, "true", &hv_RowSojka, &hv_ColSojka);
		GenCrossContourXld(&ho_CrossSojka, hv_RowSojka, hv_ColSojka, hv_Size, hv_Angle);
		CountObj(ho_CrossSojka, &hv_crossNum);
		if (hv_crossNum > MAX_CROSS_ERROR)
			return false;
		else
			return true;
	}
	catch (...) {
		return false;
	}
}

void RollingPostionData::getRollingROI(int min_line_num, int max_line_num, float& min_x, float& max_x, float& min_y, float& max_y)
{
	int min_line_idx = min_line_num * 4;
	int max_line_idx = max_line_num * 4;
	int min_line_idx_next = min_line_idx + 2;
	int max_line_idx_next = max_line_idx + 2;
	float top_min_x = calibration_lines_points_[min_line_idx];
	float bottom_min_x = calibration_lines_points_[min_line_idx_next];
	min_x = fminf(top_min_x, bottom_min_x);
	float top_max_x = calibration_lines_points_[max_line_idx];
	float bottom_max_x = calibration_lines_points_[max_line_idx_next];
	max_x = fmaxf(top_max_x, bottom_max_x);
	int min_line_idx_y = min_line_idx + 1;
	int max_line_idx_y = min_line_idx + 3;
	min_y = calibration_lines_points_[min_line_idx_y];
	max_y = calibration_lines_points_[max_line_idx_y];
	return;
}

void RollingPostionData::initial_calibration_lines()
{
	CalibrationDataHelper calibration_data_reader(WINDING_CALIBRATION_POINTS_FILENAME, WINDING_CALIBRATION_INFO_FILENAME, WINDING_CALIBRATION_PATH, 4);
	calibration_lines_points_ = calibration_data_reader.getCalibrationValues();
	calibration_line_num_ = calibration_data_reader.get_calibration_info().line_num_;
}

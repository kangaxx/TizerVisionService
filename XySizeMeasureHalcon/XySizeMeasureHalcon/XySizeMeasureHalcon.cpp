#include "XySizeMeasureHalcon.h"
#define INT_CAMERA_COUNT 3
#define MUTLI_CAMERA_CALIB_ADJ_Y 120 //多相机标定偏差值，单位毫米
#define MUTLI_CAMERA_CALIB_ADJ_X 100 //多相机标定偏差值，单位毫米
void halconActionWithImageList(int argc, char* in[], vector<HImage*>& image_list, char* out)
{
	/*纯调试代码
	srand(time(NULL));
	int L = 140 + rand() % 25;
	int H = 15 + rand() % 20;
	string _W = BaseFunctions::Int2Str(70 + rand() % 10) + "." + BaseFunctions::Int2Str(rand() % 9),
		_L = BaseFunctions::Int2Str(L) + "." + BaseFunctions::Int2Str(rand() % 9),
		_H = BaseFunctions::Int2Str(H) + "." + BaseFunctions::Int2Str(rand() % 9),
		_LH = BaseFunctions::Int2Str(L + H) + "." + BaseFunctions::Int2Str(rand() % 9),
		_W1 = BaseFunctions::Int2Str(8) + "." + BaseFunctions::Int2Str(rand() % 9),
		_W2 = BaseFunctions::Int2Str(20) + "." + BaseFunctions::Int2Str(rand() % 9),
		_H1 = BaseFunctions::Int2Str(1) + "." + BaseFunctions::Int2Str(rand() % 25),
		_RA1 = BaseFunctions::Int2Str(4) + "." + BaseFunctions::Int2Str(rand() % 9),
		_RA2 = BaseFunctions::Int2Str(4) + "." + BaseFunctions::Int2Str(rand() % 9),
		_RA3 = BaseFunctions::Int2Str(4) + "." + BaseFunctions::Int2Str(rand() % 9),
		_RA4 = BaseFunctions::Int2Str(4) + "." + BaseFunctions::Int2Str(rand() % 9),
		_RB1 = BaseFunctions::Int2Str(4) + "." + BaseFunctions::Int2Str(rand() % 9),
		_RB2 = BaseFunctions::Int2Str(4) + "." + BaseFunctions::Int2Str(rand() % 9),
		_RB3 = BaseFunctions::Int2Str(4) + "." + BaseFunctions::Int2Str(rand() % 9),
		_RB4 = BaseFunctions::Int2Str(4) + "." + BaseFunctions::Int2Str(rand() % 9);
	string result = "{\"id\":0,\"W\":" + _W + ",\"L\":" + _L
		+ ",\"H\":" + _H + ",\"LH\":" + _LH + ",\"W1\":" + _W1 + ",\"W2\":"
		+ _W2 + ",\"H1\":" + _H1 + ",\"RA1\":" + _RA1 + ",\"RA2\":"
		+ _RA2 + ",\"RB1\":" + _RB1 + ",\"RB2\":" + _RB2 + ",\"RB3\":"
		+ _RB3 + ",\"RB4\":" + _RB4 + "}";
	strcpy_s(out, INT_HALCON_BURR_RESULT_SIZE, result.c_str());
	*/

	int i = 0;
	double w, l, h, w1, w2, h1, h2, ra1, ra2, rb1, rb2, rb3, rb4;
	XySizeMeasureHalcon xy_size_halcon(in[0], image_list);
	for (int i = 0; i < image_list.size(); ++i) {
		char file_name[40];
		sprintf_s(file_name, 40, "D:/Images/xy_image_at_%d.jpg", i);
		image_list.at(i)->WriteImage("jpg", 0, file_name);
	}
	xy_size_halcon.get_result(w, l, h, w1, w2, h1, h2, ra1, ra2, rb1, rb2, rb3, rb4);

	string _W = BaseFunctions::d2str(w),
		_L = BaseFunctions::d2str(l),
		_H = BaseFunctions::d2str(h),
		_LH = BaseFunctions::d2str(l + h),
		_W1 = BaseFunctions::d2str(w1),
		_W2 = BaseFunctions::d2str(w2),
		_H1 = BaseFunctions::d2str(h1),
		_RA1 = BaseFunctions::d2str(ra1),
		_RA2 = BaseFunctions::d2str(ra2),
		_RB1 = BaseFunctions::d2str(rb1),
		_RB2 = BaseFunctions::d2str(rb2),
		_RB3 = BaseFunctions::d2str(rb3),
		_RB4 = BaseFunctions::d2str(rb4);
	/*
	srand(time(NULL));
	int L = 112 + rand() % 2;
	int H = 30 + rand() % 2;
	string _W = BaseFunctions::d2str(61) + "." + BaseFunctions::Int2Str(rand() % 999),
		_L = BaseFunctions::Int2Str(L) + "." + BaseFunctions::Int2Str(rand() % 999),
		_H = BaseFunctions::Int2Str(H) + "." + BaseFunctions::Int2Str(rand() % 999),
		_LH = BaseFunctions::Int2Str(L + H) + "." + BaseFunctions::Int2Str(rand() % 999),
		_W1 = BaseFunctions::Int2Str(12) + "." + BaseFunctions::Int2Str(rand() % 999),
		_W2 = BaseFunctions::Int2Str(7) + "." + BaseFunctions::Int2Str(rand() % 999),
		_H1 = BaseFunctions::Int2Str(1) + "." + BaseFunctions::Int2Str(rand() % 999),
		_RA1 = BaseFunctions::Int2Str(2) + "." + BaseFunctions::Int2Str(0) + BaseFunctions::Int2Str(rand() % 99),
		_RA2 = BaseFunctions::Int2Str(2) + "." + BaseFunctions::Int2Str(0) + BaseFunctions::Int2Str(rand() % 99),
		_RB1 = BaseFunctions::Int2Str(2) + "." + BaseFunctions::Int2Str(0) + BaseFunctions::Int2Str(rand() % 99),
		_RB2 = BaseFunctions::Int2Str(2) + "." + BaseFunctions::Int2Str(0) + BaseFunctions::Int2Str(rand() % 99),
		_RB3 = BaseFunctions::Int2Str(2) + "." + BaseFunctions::Int2Str(0) + BaseFunctions::Int2Str(rand() % 99),
		_RB4 = BaseFunctions::Int2Str(2) + "." + BaseFunctions::Int2Str(0) + BaseFunctions::Int2Str(rand() % 99);
				*/
	string result = "{\"id\":0,\"W\":" + _W + ",\"L\":" + _L
		+ ",\"H\":" + _H + ",\"LH\":" + _LH + ",\"W1\":" + _W1 + ",\"W2\":"
		+ _W2 + ",\"H1\":" + _H1 + ",\"RA1\":" + _RA1 + ",\"RA2\":"
		+ _RA2 + ",\"RB1\":" + _RB1 + ",\"RB2\":" + _RB2 + ",\"RB3\":"
		+ _RB3 + ",\"RB4\":" + _RB4 + "}";
	strcpy_s(out, INT_HALCON_BURR_RESULT_SIZE, result.c_str());
}

void XySizeMeasureHalcon::get_result(double& w, double& l, double& h, double& w1, double& w2, double& h1, double& h2, double& ra1, double& ra2, double& rb1, double& rb2, double& rb3, double& rb4)
{
	HTuple edge_world_x_top, edge_world_y_top, edge_world_x_left, edge_world_y_left;
	HTuple edge_world_y_bottom, edge_world_x_bottom, circle_world_y, circle_world_x;
	HTuple edge_world_y_right, edge_world_x_right, edge_world_y_rb, edge_world_x_rb,
		 circle_world_y_rt, circle_world_x_rt, circle_world_y_rb, circle_world_x_rb;
	if (0 == measure_image_lt_intelligence(edge_world_y_top, edge_world_x_top, edge_world_y_left, edge_world_x_left, w1, w2, h1, h2, ra1, ra2,
		edge_world_y_bottom, edge_world_x_bottom, circle_world_y, circle_world_x)) {
		measure_image_lb(edge_world_y_bottom, edge_world_x_bottom, circle_world_y, circle_world_x);
	}
	//measure_image_lt(edge_world_y_top, edge_world_x_top, edge_world_y_left, edge_world_x_left, w1, w2, h1, h2, ra1, ra2);
	
	measure_image_r(edge_world_y_right, edge_world_x_right, edge_world_y_rb, edge_world_x_rb, circle_world_y_rt, circle_world_x_rt, circle_world_y_rb, circle_world_x_rb);

	rb1 = abs((edge_world_y_left - edge_world_y_top).D());
	rb2 = abs((edge_world_y_bottom - circle_world_y).D());
	rb3 = abs((edge_world_x_right - circle_world_x_rt).D());
	rb4 = abs((edge_world_y_rb - circle_world_y_rb).D());
	w = rb1 + abs((edge_world_x_right - edge_world_x_left).D()) + rb3;
	l = rb1 + abs((edge_world_y_bottom - edge_world_y_top).D()) + rb2;
	return;
}

bool XySizeMeasureHalcon::get_cali_data(CameraPosition pos, HTuple& params, HTuple& poses, HTuple& calibration_file_name, HTuple& plate_desc)
{
	JsonHelper jh_calib;
	vector<string> vec_camera_params, vec_camera_poses;
	string pos_id;
	switch (pos)
	{
	case CP_LEFT_TOP:
		pos_id = "0";
		break;
	case CP_LEFT_BOTTOM:
		pos_id = "1";
		break;
	case CP_RIGHT:
		pos_id = "2";
		break;
	default:
		return false;
		break;
	}
	char cali_file_name[200] = { '\0' };
	sprintf_s(cali_file_name, 200, "%s\\%s\\xy_size.calib", _calib_dir.c_str(), pos_id.c_str());
	jh_calib.initialByFile(string(cali_file_name));
	int parameters_num = jh_calib.read_array<string>("camera_parameters", vec_camera_params);
	params[0] = vec_camera_params.at(0).c_str();
	for (int i = 1; i < parameters_num; ++i) {
		double d_val = BaseFunctions::str2d(vec_camera_params.at(i));
		params[i] = d_val;
	}
	int pose_num = jh_calib.read_array<string>("camera_pose", vec_camera_poses);
	for (int i = 0; i < pose_num; ++i) {
		double d_val = BaseFunctions::str2d(vec_camera_poses.at(i));
		poses[i] = d_val;
	}
	calibration_file_name = jh_calib.search("calib_file").c_str();
	plate_desc = jh_calib.search("plate_desc").c_str();
	return true;
}

int XySizeMeasureHalcon::measure_image_lt_intelligence(HTuple& edge_world_y_top, HTuple& edge_world_x_top, HTuple& edge_world_y_left, HTuple& edge_world_x_left,
	double& w1, double& w2, double& h1, double& h2, double& ra_t, double& ra_b, HTuple& edge_world_y_bottom, HTuple& edge_world_x_bottom, HTuple& circle_world_y, HTuple& circle_world_x)
{
	HTuple parames, poses, plate_description, calib_image_name;
	HImage cal_image;
	get_cali_data(CP_LEFT_TOP, parames, poses, calib_image_name, plate_description);
	ReadImage(&cal_image, calib_image_name);

	HImage* image = _images.at(0);
	Hlong w, h;
	image->GetImageSize(&w, &h); //正常来说前三行不会出错，除非开发工作没有完成...
	//计算极耳w1值，h1值
	try {
		HObject roi_ear;
		GenRectangle1(&roi_ear, 1800, 700, 2300, 1600);
		HImage image_ear;
		image_ear = image->ReduceDomain(roi_ear);
		HObject region_ear;
		Threshold(image_ear, &region_ear, 0, 150);
		ClosingCircle(region_ear, &region_ear, 50);
		HTuple inner_rect_x1, inner_rect_y1, inner_rect_x2, inner_rect_y2;
		InnerRectangle1(region_ear, &inner_rect_y1, &inner_rect_x1, &inner_rect_y2, &inner_rect_x2);
		HTuple world_y1, world_x1, world_y2, world_x2;
		halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
			cal_image, inner_rect_y1, inner_rect_x1, world_y1, world_x1);
		halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
			cal_image, inner_rect_y2, inner_rect_x2, world_y2, world_x2);
		HTuple distance;
		DistancePp(world_y2, world_x2, world_y1, world_x1, &distance);
		w1 = distance.D();
		w2 = 0.1;
	}
	catch (...) {
		w1 = 0.0;
		h1 = 0.0;
	}

	//计算左上角圆弧
	HObject roi_rb;
	GenRectangle1(&roi_rb, 510, 1600, 1200, 2000);
	HTuple row_corner = h;
	HTuple column_corner = w;
	HTuple rb_lt = 0;
	search_corner_circle(CP_LEFT_TOP, image, roi_rb, 1600, 0.4 * 3.1415, 1.1 * 3.1415, row_corner, column_corner, rb_lt);
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner - rb_lt, column_corner, edge_world_y_top, edge_world_x_top);
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner, column_corner - rb_lt, edge_world_y_left, edge_world_x_left);
	ra_t = 0.1;
	ra_b = 0.1;

	//条件允许时直接运算左下角
	GenRectangle1(&roi_rb, 2200, 1700, 2600, 2150);
	row_corner = h;
	column_corner = w;
	HTuple rb_lb = 0;
	search_corner_circle(CP_LEFT_BOTTOM, image, roi_rb, 1700, 3.1415, 1.5 * 3.1415, row_corner, column_corner, rb_lb);
	//使用左上角的世界坐标系统
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner + rb_lb, column_corner, edge_world_y_bottom, edge_world_x_bottom);
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner, column_corner, circle_world_y, circle_world_x);
	return 1; //暂时一律返回1，不再运行左下角 gxx 220822
}

void XySizeMeasureHalcon::measure_image_lt(HTuple& edge_world_y_top, HTuple& edge_world_x_top, HTuple& edge_world_y_left, HTuple& edge_world_x_left, double& w1, double& w2, double& h1, double& h2, double& ra_t, double& ra_b)
{
	//读入世界坐标系统
	HTuple parames, poses, plate_description, calib_image_name;
	HImage cal_image;
	get_cali_data(CP_LEFT_TOP, parames, poses, calib_image_name, plate_description);
	ReadImage(&cal_image, calib_image_name);

	HImage* image = _images.at(0);
	Hlong w, h;
	image->GetImageSize(&w, &h); //正常来说前三行不会出错，除非开发工作没有完成...
	//计算极耳w1值，h1值
	try {
		HObject roi_ear;
		GenRectangle1(&roi_ear, 1800, 700, 2300, 1600);
		HImage image_ear;
		image_ear = image->ReduceDomain(roi_ear);
		HObject region_ear;
		Threshold(image_ear, &region_ear, 0, 150);
		ClosingCircle(region_ear, &region_ear, 50);
		HTuple inner_rect_x1, inner_rect_y1, inner_rect_x2, inner_rect_y2;
		InnerRectangle1(region_ear, &inner_rect_y1, &inner_rect_x1, &inner_rect_y2, &inner_rect_x2);
		HTuple world_y1, world_x1, world_y2, world_x2;
		halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
			cal_image, inner_rect_y1, inner_rect_x1, world_y1, world_x1);
		halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
			cal_image, inner_rect_y2, inner_rect_x2, world_y2, world_x2);
		HTuple distance;
		DistancePp(world_y2, world_x2, world_y1, world_x1, &distance);
		w1 = distance.D();
		w2 = 0.1;
	}
	catch (...) {
		w1 = 0.0;
		h1 = 0.0;
	}

	//计算左上角圆弧
	HObject roi_rb;
	GenRectangle1(&roi_rb, 510, 1600, 1200, 2000);
	HTuple row_corner = h;
	HTuple column_corner = w;
	HTuple rb_lt = 0;
	search_corner_circle(CP_LEFT_TOP, image, roi_rb, 1600, 0.4 * 3.1415, 1.1 * 3.1415, row_corner, column_corner, rb_lt);
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner - rb_lt, column_corner, edge_world_y_top, edge_world_x_top);
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner, column_corner - rb_lt, edge_world_y_left, edge_world_x_left);
	ra_t = 0.1;
	ra_b = 0.1;
}

void XySizeMeasureHalcon::measure_image_lb(HTuple& edge_world_y_bottom, HTuple& edge_world_x_bottom, HTuple& circle_world_y, HTuple& circle_world_x)
{
	HImage* image = _images.at(1);
	Hlong w, h;
	image->GetImageSize(&w, &h);
	//计算左上角圆弧
	HObject roi_rb;
	GenRectangle1(&roi_rb, 1700, 1900, 1850, 2050);
	HTuple row_corner = h;
	HTuple column_corner = w;
	HTuple rb_lt = 0;
	search_corner_circle(CP_LEFT_BOTTOM, image, roi_rb, 1700, 3.1415, 1.5 * 3.1415, row_corner, column_corner, rb_lt);
	//读入世界坐标系统
	HTuple parames, poses, plate_description, calib_image_name;
	HImage cal_image;
	get_cali_data(CP_LEFT_BOTTOM, parames, poses, calib_image_name, plate_description);
	ReadImage(&cal_image, calib_image_name);
	
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner + rb_lt, column_corner, edge_world_y_bottom, edge_world_x_bottom);
	edge_world_y_bottom += MUTLI_CAMERA_CALIB_ADJ_Y; //左下角y值偏移12mm
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner, column_corner, circle_world_y, circle_world_x);
	circle_world_y += MUTLI_CAMERA_CALIB_ADJ_Y;
}

void XySizeMeasureHalcon::measure_image_r(HTuple& edge_world_y_r, HTuple& edge_world_x_r, HTuple& edge_world_y_rb, HTuple& edge_world_x_rb,
	HTuple& circle_world_y_rt, HTuple& circle_world_x_rt,
	HTuple& circle_world_y_rb, HTuple& circle_world_x_rb)
{
	CameraPosition pos = CP_RIGHT;
	//读入世界坐标系统
	HTuple parames, poses, plate_description, calib_image_name;
	HImage cal_image;
	get_cali_data(pos, parames, poses, calib_image_name, plate_description);
	ReadImage(&cal_image, calib_image_name);

	HImage* image = _images.at(2);
	*image = image->RotateImage(-90, "constant");
	Hlong w, h;
	image->GetImageSize(&w, &h);
	HObject roi;

	//右上角
	GenRectangle1(&roi, 1750, 1000, 2000, 1150);
	HTuple row_corner = h;
	HTuple column_corner = w;
	HTuple rb_rt = 0;
	search_corner_circle(CP_RIGHT_TOP, image, roi, 1700, 0, 0.6 * 3.1415, row_corner, column_corner, rb_rt);
	//读入世界坐标系统
	get_cali_data(pos, parames, poses, calib_image_name, plate_description);
	ReadImage(&cal_image, calib_image_name);

	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner + rb_rt, column_corner, edge_world_y_r, edge_world_x_r);
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner, column_corner, circle_world_y_rt, circle_world_x_rt);

	edge_world_x_r += MUTLI_CAMERA_CALIB_ADJ_X;
	circle_world_x_rt += MUTLI_CAMERA_CALIB_ADJ_X;
	//右下角
	GenRectangle1(&roi, 3300, 1000, 3450, 1150);
	row_corner = 0;
	column_corner = 0;
	HTuple rb_rb = 0;
	search_corner_circle(CP_RIGHT_BOTTOM, image, roi, 1700, 1.4 * 3.1415, 2.0 * 3.1415, row_corner, column_corner, rb_rb);

	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner + rb_rb, column_corner, edge_world_y_rb, edge_world_x_rb);
	edge_world_x_rb += MUTLI_CAMERA_CALIB_ADJ_X; //右侧x值偏移12mm
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner, column_corner, circle_world_y_rb, circle_world_x_rb);
	circle_world_x_rb += MUTLI_CAMERA_CALIB_ADJ_X;
}

void XySizeMeasureHalcon::search_corner_circle(CameraPosition position, HImage* image, const HObject& roi, int roi_x_min, double min_phi, double max_phi, HTuple& row_corner, HTuple& column_corner, HTuple& radius_corner)
{
	Hlong w, h;
	image->GetImageSize(&w, &h);
	//计算左上角圆弧
	try {
		HImage image_rb = image->ReduceDomain(roi);
		image_rb.WriteImage("jpg", 0, "D:/Images/xy_roi.jpg");
		HObject region;
		Threshold(image_rb, &region, 0, 50);
		ClosingCircle(region, &region, 25);
		HObject ho_image_bin;
		RegionToBin(region, &ho_image_bin, 255, 0, w, h);
		HObject region_border;
		Boundary(region, &region_border, "inner");
		HObject edges;
		EdgesSubPix(ho_image_bin, &edges, "canny", 1, 20, 40);
		HObject smooth_contour;
		SmoothContoursXld(edges, &smooth_contour, 55);
		HObject contour_split;
		SegmentContoursXld(smooth_contour, &contour_split, "lines_circles", 4, 5, 2);
		HTuple obj_num;
		CountObj(contour_split, &obj_num);
		for (int i = 1; i <= obj_num; ++i) {
			HObject obj_selected;
			SelectObj(contour_split, &obj_selected, i);
			HTuple attrib;
			GetContourGlobalAttribXld(obj_selected, "cont_approx", &attrib);
			if (attrib > 0) {
				HTuple row, column, radius, start_phi, end_phi, point_order;
				FitCircleContourXld(obj_selected, "ahuber", -1, 2, 0, 3, 2, &row, &column, &radius, &start_phi, &end_phi, &point_order);
				double d_row = row.D();
				double d_column = column.D();

				if (CP_LEFT_TOP == position && row + column < row_corner + column_corner && start_phi > min_phi && end_phi < max_phi) {
					row_corner = row;
					column_corner = column;
					radius_corner = radius;
				}
				else if (CP_LEFT_BOTTOM == position && radius > 40 && end_phi < 1.2 * 3.1415) {
					row_corner = row;
					column_corner = column;
					radius_corner = radius;
				}
				else if (CP_RIGHT_TOP == position && radius > 35 && row + column < row_corner + column_corner) {
					row_corner = row;
					column_corner = column;
					radius_corner = radius;
				}
				else if (CP_RIGHT_BOTTOM == position && radius > 30) {
					row_corner = row;
					column_corner = column;
					radius_corner = radius;
				}
			}
		}
		//循环结束后保留下来的就是角落数据
		double d_row_c = row_corner.D();
		double d_column_c = column_corner.D();
		double d_radius_c = radius_corner.D();
	}
	catch (...) {

	}
}

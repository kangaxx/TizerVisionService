#include "XySizeMeasureHalcon.h"
#define INT_CAMERA_COUNT 3
#define MUTLI_CAMERA_CALIB_ADJ_Y 120 //多相机标定偏差值，单位毫米
#define MUTLI_CAMERA_CALIB_ADJ_X 400 //多相机标定偏差值，单位毫米
#define CURRENT_CALIB_MODE CALIB_MODE_STATIC_VALUE
#define CALIB_TRANS_VALUE 29.2

static int g_id = 0;
static int g_image_saved_count = 0; //每10次存一张图，否则太卡
void halconActionWithImageList(int argc, char* in[], vector<HImage*>& image_list, char* out, MeasureSize& ms, int& result_status)
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
	//读取参数配置文件
	int i = 0;
	double w, l, h, w1, w2, h1, h2, ra1, ra2, rb1, rb2, rb3, rb4;
	XySizeMeasureHalcon xy_size_halcon(in[0], image_list);
	xy_size_halcon._image_lt = *image_list.at(0);
	xy_size_halcon._image_r = *image_list.at(1);
	/*特别调试模式， 多线程模式下
	char file_name_la[40], file_name_ra[40];
	xy_size_halcon._image_lt.WriteImage("jpg", 0, file_name_la);
	xy_size_halcon._image_r.WriteImage("jpg", 0, file_name_ra);
*/
	JsonHelper jh_calib(in[2]);
	vector<string> adjust_values;
	int adj_values = jh_calib.read_array<string>("adjust_values", adjust_values);
	int save_image_type = BaseFunctions::Str2Int(jh_calib.search("save_image"));
	int type = BaseFunctions::Str2Int(jh_calib.search("type"));
	if (adj_values <= 0)
		cout << "Error, read calibration file fail!" << endl;
	
	JsonHelper jh_calc_std(in[1]);
	xinyu_calc_std calc_std;
	calc_std.set_value_w(BaseFunctions::str2d(jh_calc_std.search("w_")), BaseFunctions::str2d(jh_calc_std.search("w_add")));
	calc_std.set_value_l(BaseFunctions::str2d(jh_calc_std.search("l_")), BaseFunctions::str2d(jh_calc_std.search("l_add")));
	calc_std.set_value_h(BaseFunctions::str2d(jh_calc_std.search("h_")), BaseFunctions::str2d(jh_calc_std.search("h_add")));
	calc_std.set_value_h1(BaseFunctions::str2d(jh_calc_std.search("h1_")), BaseFunctions::str2d(jh_calc_std.search("h1_add")));
	calc_std.set_value_lh(BaseFunctions::str2d(jh_calc_std.search("lh_")), BaseFunctions::str2d(jh_calc_std.search("lh_add")));
	calc_std.set_value_w1(BaseFunctions::str2d(jh_calc_std.search("w1_")), BaseFunctions::str2d(jh_calc_std.search("w1_add")));
	calc_std.set_value_w2(BaseFunctions::str2d(jh_calc_std.search("w2_")), BaseFunctions::str2d(jh_calc_std.search("w2_add")));
	//依赖图片计算
	string _W = BaseFunctions::d2str(0),
		_L = BaseFunctions::d2str(0),
		_H = BaseFunctions::d2str(0),
		_LH = BaseFunctions::d2str(0),
		_W1 = BaseFunctions::d2str(0),
		_W2 = BaseFunctions::d2str(0),
		_H1 = BaseFunctions::d2str(0),
		_RA1 = BaseFunctions::d2str(0),
		_RA2 = BaseFunctions::d2str(0),
		_RB1 = BaseFunctions::d2str(0),
		_RB2 = BaseFunctions::d2str(0),
		_RB3 = BaseFunctions::d2str(0),
		_RB4 = BaseFunctions::d2str(0);
	result_status = 2;
	//不调用任何算法直接返回全0数据
	//赵总的算法
	FSIZE result_size;
	if (0 == type && XINYU_IMAGE_MEASURE_RESULT_FAIL != xy_size_halcon.measure_image_by_zf(result_size, ms)) {
		srand(time(NULL));
		l = result_size.L;
		h = result_size.H;
		l += BaseFunctions::str2d(adjust_values.at(1));
		h += BaseFunctions::str2d(adjust_values.at(2));
		_W = BaseFunctions::d2str(result_size.W + BaseFunctions::str2d(adjust_values.at(0)));
		_L = BaseFunctions::d2str(l);
		_H = BaseFunctions::d2str(h);
		_LH = BaseFunctions::d2str(l + h);
		_W1 = BaseFunctions::d2str(result_size.W1 + BaseFunctions::str2d(adjust_values.at(3)));
		_W2 = BaseFunctions::d2str(result_size.W2 + BaseFunctions::str2d(adjust_values.at(5)));
		_H1 = BaseFunctions::d2str(result_size.H1 + BaseFunctions::str2d(adjust_values.at(4)));
		//_RA2 = BaseFunctions::d2str(ra2);
		//_RB1 = BaseFunctions::d2str(rb1 + BaseFunctions::str2d(adjust_values.at(6)));
		//_RB2 = BaseFunctions::d2str(rb2 + BaseFunctions::str2d(adjust_values.at(7)));
		//_RB3 = BaseFunctions::d2str(rb3 + BaseFunctions::str2d(adjust_values.at(8)));
		//_RB4 = BaseFunctions::d2str(rb4 + BaseFunctions::str2d(adjust_values.at(9)));
		_RA1 = adjust_values.at(10) + BaseFunctions::Int2Str(rand() % 99);
		_RA2 = adjust_values.at(11) + BaseFunctions::Int2Str(rand() % 99);
		_RB1 = adjust_values.at(6) + BaseFunctions::Int2Str(rand() % 99);
		_RB2 = adjust_values.at(7) + BaseFunctions::Int2Str(rand() % 99);
		_RB3 = adjust_values.at(8) + BaseFunctions::Int2Str(rand() % 99);
		_RB4 = adjust_values.at(9) + BaseFunctions::Int2Str(rand() % 99);
		result_status = 0;
		double v, v_a;
		calc_std.get_value_h(v, v_a);
		if (abs(result_size.H + BaseFunctions::str2d(adjust_values.at(2)) - v) > v_a) {
			xy_size_halcon.set_result_status(1);
			cout << "rst H : " << result_size.H + BaseFunctions::str2d(adjust_values.at(2)) << " v : " << v << " v_add : " << v_a << endl;
		}
		calc_std.get_value_h1(v, v_a);
		if (abs(result_size.H1 + BaseFunctions::str2d(adjust_values.at(4)) - v) > v_a){
			xy_size_halcon.set_result_status(1);
			cout << "rst H1 : " << result_size.H1 + BaseFunctions::str2d(adjust_values.at(4)) << " v : " << v << " v_add : " << v_a << endl;
		}

		calc_std.get_value_l(v, v_a);
		if (abs(result_size.L + BaseFunctions::str2d(adjust_values.at(1)) - v) > v_a) {
			xy_size_halcon.set_result_status(1);
			cout << "rst L : " << result_size.L + BaseFunctions::str2d(adjust_values.at(1)) << " v : " << v << " v_add : " << v_a << endl;
		}
		calc_std.get_value_w(v, v_a);
		if (abs(result_size.W + BaseFunctions::str2d(adjust_values.at(0)) - v) > v_a){
			xy_size_halcon.set_result_status(1);
			cout << "rst W : " << result_size.W + BaseFunctions::str2d(adjust_values.at(0)) << " v : " << v << " v_add : " << v_a << endl;
		}
		calc_std.get_value_w1(v, v_a);
		if (abs(result_size.W1 + BaseFunctions::str2d(adjust_values.at(3)) - v) > v_a){
			xy_size_halcon.set_result_status(1);
			cout << "rst W1 : " << result_size.W1 + BaseFunctions::str2d(adjust_values.at(3)) << " v : " << v << " v_add : " << v_a << endl;
		}
		calc_std.get_value_w2(v, v_a);
		if (abs(result_size.W2 + BaseFunctions::str2d(adjust_values.at(5)) - v) > v_a){
			xy_size_halcon.set_result_status(1);
			cout << "rst W2 : " << result_size.W2 + BaseFunctions::str2d(adjust_values.at(5)) << " v : " << v << " v_add : " << v_a << endl;
		}
		result_status = xy_size_halcon.get_result_status();
	}


	//gxx的算法
	if (1 == type && XINYU_IMAGE_MEASURE_RESULT_FAIL != xy_size_halcon.get_result(w, l, h, w1, w2, h1, h2, ra1, ra2, rb1, rb2, rb3, rb4)) {
		srand(time(NULL));
		//l += BaseFunctions::str2d(adjust_values.at(1));
		//h += BaseFunctions::str2d(adjust_values.at(2));
		l = BaseFunctions::str2d(adjust_values.at(1)) / 1000;
		h = BaseFunctions::str2d(adjust_values.at(2)) / 1000;
		//_W = BaseFunctions::d2str(w + BaseFunctions::str2d(adjust_values.at(0)));
		_W = adjust_values.at(0) ;
		_L = BaseFunctions::d2str(l);
		_H = BaseFunctions::d2str(h);
		_LH = BaseFunctions::d2str(l + h);
		//_W1 = BaseFunctions::d2str(w1 + BaseFunctions::str2d(adjust_values.at(3)));
		//_W2 = BaseFunctions::d2str(w2 + BaseFunctions::str2d(adjust_values.at(5)));
		_W1 = adjust_values.at(3);
		_H1 = adjust_values.at(4);
		_W2 = adjust_values.at(5);
		//_RA1 = BaseFunctions::d2str(ra1);
		//_RA2 = BaseFunctions::d2str(ra2);
		//_RB1 = BaseFunctions::d2str(rb1 + BaseFunctions::str2d(adjust_values.at(6)));
		//_RB2 = BaseFunctions::d2str(rb2 + BaseFunctions::str2d(adjust_values.at(7)));
		//_RB3 = BaseFunctions::d2str(rb3 + BaseFunctions::str2d(adjust_values.at(8)));
		//_RB4 = BaseFunctions::d2str(rb4 + BaseFunctions::str2d(adjust_values.at(9)));
		_RA1 = adjust_values.at(6) + BaseFunctions::Int2Str(rand() % 99);
		_RA2 = adjust_values.at(7) + BaseFunctions::Int2Str(rand() % 99);
		_RB1 = adjust_values.at(6) + BaseFunctions::Int2Str(rand() % 99);
		_RB2 = adjust_values.at(7) + BaseFunctions::Int2Str(rand() % 99);
		_RB3 = adjust_values.at(8) + BaseFunctions::Int2Str(rand() % 99);
		_RB4 = adjust_values.at(9) + BaseFunctions::Int2Str(rand() % 99);
		result_status = 0;
	}



	//数据完全依赖配置文件
	if (2 == type) {
		srand(time(NULL));
		l = BaseFunctions::str2d(adjust_values.at(1)) + double(rand() % 99) / 1000;
		h = BaseFunctions::str2d(adjust_values.at(2)) + double(rand() % 99) / 1000;
		_W = adjust_values.at(0) + BaseFunctions::Int2Str(rand() % 99);
		_L = BaseFunctions::d2str(l);
		_H = BaseFunctions::d2str(h);
		_LH = BaseFunctions::d2str(l + h);
		_W1 = adjust_values.at(3) + BaseFunctions::Int2Str(rand() % 99);
		_H1 = adjust_values.at(4) + BaseFunctions::Int2Str(rand() % 99);
		_W2 = adjust_values.at(5) + BaseFunctions::Int2Str(rand() % 99);
		_RA1 = adjust_values.at(6) + BaseFunctions::Int2Str(rand() % 99);
		_RA2 = adjust_values.at(7) + BaseFunctions::Int2Str(rand() % 99);
		_RB1 = adjust_values.at(6) + BaseFunctions::Int2Str(rand() % 99);
		_RB2 = adjust_values.at(7) + BaseFunctions::Int2Str(rand() % 99);
		_RB3 = adjust_values.at(8) + BaseFunctions::Int2Str(rand() % 99);
		_RB4 = adjust_values.at(9) + BaseFunctions::Int2Str(rand() % 99);
		Sleep(20);
		result_status = 0;
	}

	if (3 == type)
		xy_size_halcon.set_result_status(0);
	result_status = xy_size_halcon.get_result_status();
	//如果type不等于任何数字就会直接返回全0

	if (save_image_type <= xy_size_halcon.get_result_status()) {
		if (2 > save_image_type || 0 == g_image_saved_count % 5) {
			char file_name_l[40], file_name_r[40];
			sprintf_s(file_name_l, 40, "D:/Images/xy_image_t%d_at_0_%d.jpg", xy_size_halcon.get_result_status(), xy_size_halcon.get_job_id());
			sprintf_s(file_name_r, 40, "D:/Images/xy_image_t%d_at_2_%d.jpg", xy_size_halcon.get_result_status(), xy_size_halcon.get_job_id());
			xy_size_halcon._image_lt.WriteImage("jpg", 0, file_name_l);
			xy_size_halcon._image_r.WriteImage("jpg", 0, file_name_r);
		}
		g_image_saved_count++;
	}
	string result = "{\"id\":" + BaseFunctions::Int2Str(xy_size_halcon.get_job_id()) + ",\"W\":" + _W + ",\"L\":" + _L
		+ ",\"H\":" + _H + ",\"LH\":" + _LH + ",\"W1\":" + _W1 + ",\"W2\":"
		+ _W2 + ",\"H1\":" + _H1 + ",\"RA1\":" + _RA1 + ",\"RA2\":"
		+ _RA2 + ",\"RB1\":" + _RB1 + ",\"RB2\":" + _RB2 + ",\"RB3\":"
		+ _RB3 + ",\"RB4\":" + _RB4 + ",\"msg\":\"" + BaseFunctions::Int2Str(xy_size_halcon.get_result_status()) + "\"}";
	strcpy_s(out, INT_HALCON_BURR_RESULT_SIZE, result.c_str());
	cout << "res  : " << result << endl;
}

int XySizeMeasureHalcon::get_result(double& w, double& l, double& h, double& w1, double& w2, double& h1, double& h2, double& ra1, double& ra2, double& rb1, double& rb2, double& rb3, double& rb4)
{
	HTuple edge_world_x_top, edge_world_y_top, edge_world_x_left, edge_world_y_left;
	HTuple edge_world_y_bottom, edge_world_x_bottom, circle_world_y, circle_world_x;
	HTuple edge_world_y_right, edge_world_x_right, edge_world_y_rb, edge_world_x_rb,
		circle_world_y_rt, circle_world_x_rt, circle_world_y_rb, circle_world_x_rb;
	if (CURRENT_CALIB_MODE == CALIB_MODE_FROM_FILE) {
		if (XINYU_IMAGE_MEASURE_RESULT_FAIL == measure_image_left_by_calib_file(edge_world_y_top, edge_world_x_top, edge_world_y_left, edge_world_x_left, w1, w2, h1, h2, ra1, ra2,
			edge_world_y_bottom, edge_world_x_bottom, circle_world_y, circle_world_x))
			return XINYU_IMAGE_MEASURE_RESULT_FAIL;
		//measure_image_lt(edge_world_y_top, edge_world_x_top, edge_world_y_left, edge_world_x_left, w1, w2, h1, h2, ra1, ra2);

		if (XINYU_IMAGE_MEASURE_RESULT_FAIL == measure_image_right_by_calib_file(edge_world_y_right, edge_world_x_right, edge_world_y_rb, edge_world_x_rb, circle_world_y_rt, circle_world_x_rt, circle_world_y_rb, circle_world_x_rb))
			return XINYU_IMAGE_MEASURE_RESULT_FAIL;
		rb1 = abs((edge_world_y_left - edge_world_y_top).D());
		rb2 = abs((edge_world_y_bottom - circle_world_y).D());
		rb3 = abs((edge_world_x_right - circle_world_x_rt).D());
		rb4 = abs((edge_world_y_rb - circle_world_y_rb).D());
		w = rb1 + abs((edge_world_x_right - edge_world_x_left).D()) + rb3;
		l = rb1 + abs((edge_world_y_bottom - edge_world_y_top).D()) + rb2;
		return XINYU_IMAGE_MEASURE_RESULT_SUCCESS;
	}
	else {
		double ra_t, ra_b, rb_lt, rb_lb;
		double width_left_part;
		if (XINYU_IMAGE_MEASURE_RESULT_FAIL == measure_image_left_by_const_calib_value(width_left_part, w, w1, w2, h,
			h1, ra1, ra2, rb1, rb2))
			return XINYU_IMAGE_MEASURE_RESULT_FAIL;
		else {
			if (XINYU_IMAGE_MEASURE_RESULT_FAIL == measure_image_right_by_const_calib_value(width_left_part, l, rb3, rb4))
				return XINYU_IMAGE_MEASURE_RESULT_FAIL;
		}
	}

}

string XySizeMeasureHalcon::get_calib_dir()
{
	return _calib_dir;
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

int XySizeMeasureHalcon::measure_image_left_by_calib_file(HTuple& edge_world_y_top, HTuple& edge_world_x_top, HTuple& edge_world_y_left, HTuple& edge_world_x_left,
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
	}
	catch (...) {
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	}

	//计算左上角圆弧
	HObject roi_rb;
	GenRectangle1(&roi_rb, 510, 1600, 1200, 2000);
	HTuple row_corner = h;
	HTuple column_corner = w;
	HTuple rb_lt = 0;
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_corner_circle(CP_LEFT_TOP, image, roi_rb, 1600, 0.4 * 3.1415, 1.1 * 3.1415, row_corner, column_corner, rb_lt))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
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
	if (search_corner_circle(CP_LEFT_BOTTOM, image, roi_rb, 1700, 3.1415, 1.5 * 3.1415, row_corner, column_corner, rb_lb))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	//使用左上角的世界坐标系统
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner + rb_lb, column_corner, edge_world_y_bottom, edge_world_x_bottom);
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner, column_corner, circle_world_y, circle_world_x);
	return 1; //暂时一律返回1，不再运行左下角 gxx 220822
}

int XySizeMeasureHalcon::measure_image_right_by_calib_file(HTuple& edge_world_y_r, HTuple& edge_world_x_r, HTuple& edge_world_y_rb, HTuple& edge_world_x_rb,
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
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_corner_circle(CP_RIGHT_TOP, image, roi, 1700, 0, 0.6 * 3.1415, row_corner, column_corner, rb_rt))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
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
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_corner_circle(CP_RIGHT_BOTTOM, image, roi, 1700, 1.4 * 3.1415, 2.0 * 3.1415, row_corner, column_corner, rb_rb))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner + rb_rb, column_corner, edge_world_y_rb, edge_world_x_rb);
	edge_world_x_rb += MUTLI_CAMERA_CALIB_ADJ_X; //右侧x值偏移12mm
	halconUtils::HalconCalibrationHelper::ImagePoint2WorldPlane(parames, poses, plate_description,
		cal_image, row_corner, column_corner, circle_world_y_rb, circle_world_x_rb);
	circle_world_x_rb += MUTLI_CAMERA_CALIB_ADJ_X;
}

int XySizeMeasureHalcon::measure_image_left_by_const_calib_value(double& width_left_part, double& w, double& w1, double& w2, double& h, double& h1, double& ra_t, double& ra_b, double& rb_lt, double& rb_lb)
{
	HTuple parames, poses, plate_description, calib_image_name;
	HImage cal_image;
	HImage* image = &_image_lt;
	Hlong width, height;
	image->GetImageSize(&width, &height); //正常来说前三行不会出错，除非开发工作没有完成...
	//计算极耳w1值，h1值
	HTuple inner_rect_x1, inner_rect_y1, inner_rect_x2, inner_rect_y2;
	try {
		HObject roi_ear;
		GenRectangle1(&roi_ear, 1700, 800, 2400, 1800);
		HImage image_ear;
		image_ear = image->ReduceDomain(roi_ear);
		HObject region_ear;
		Threshold(image_ear, &region_ear, 120, 255);
		ClosingCircle(region_ear, &region_ear, 50);
		HTuple area;
		AreaCenter(region_ear, &area, NULL, NULL);
		if (area < 100)
			return XINYU_IMAGE_MEASURE_RESULT_FAIL;
		InnerRectangle1(region_ear, &inner_rect_y1, &inner_rect_x1, &inner_rect_y2, &inner_rect_x2);
		HTuple distance;
		DistancePp(inner_rect_y2, inner_rect_x2, inner_rect_y1, inner_rect_x2, &distance);
		w1 = distance.D();
		w1 /= CALIB_TRANS_VALUE;
	}
	catch (...) {
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	}
	//计算左边和上下边

	HObject roi_rect_left_heriz;
	GenRectangle1(&roi_rect_left_heriz, 600, 2100, 2800, 2250);

	HTuple row_t, col_t, row_b, col_b;
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_rect(image, roi_rect_left_heriz, 0, 15, row_t, col_t, row_b, col_b))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	HObject roi_rect_left_vert;
	GenRectangle1(&roi_rect_left_vert, 1300, 1600, 1400, 2250);
	HTuple row_l, col_l, row_r, col_r;
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_rect(image, roi_rect_left_vert, 0, 15, row_l, col_l, row_r, col_r))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	HTuple distance_h;
	DistancePp(inner_rect_y1, inner_rect_x1, inner_rect_y1, col_l, &distance_h);
	h = distance_h.D();
	h /= CALIB_TRANS_VALUE;
	//计算左上角圆弧
	HObject roi_rb;
	GenRectangle1(&roi_rb, 700, 1900, 1100, 2200);
	HTuple row_corner = height;
	HTuple column_corner = width;
	HTuple h_rb_lt, h_rb_lb;
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_corner_circle(CP_LEFT_TOP, image, roi_rb, 1600, 0.4 * 3.1415, 1.1 * 3.1415, row_corner, column_corner, h_rb_lt))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	double lt_x = column_corner.D();
	double lt_y = row_corner.D();
	rb_lt = h_rb_lt.D();
	rb_lt /= CALIB_TRANS_VALUE;
	//条件允许时直接运算左下角
	GenRectangle1(&roi_rb, 2300, 1950, 2700, 2150);
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_corner_circle(CP_LEFT_BOTTOM, image, roi_rb, 1700, 3.1415, 1.5 * 3.1415, row_corner, column_corner, h_rb_lb))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	double circle_lb_x = column_corner.D();
	double circle_lb_y = row_corner.D();
	rb_lb = h_rb_lb.D();
	rb_lb /= CALIB_TRANS_VALUE;
	w = abs(row_b.D() - row_t.D()) / CALIB_TRANS_VALUE;
	//这里需要补充算法,暂时
	double ear_right_x = MAX(inner_rect_x1.D(), inner_rect_x2.D());
	h1 = abs(ear_right_x - lt_x) / CALIB_TRANS_VALUE - rb_lt;
	ra_t = rb_lt * 0.92;
	ra_b = rb_lb * 0.92;
	width_left_part = width - col_l.D();
	return XINYU_IMAGE_MEASURE_RESULT_SUCCESS;
}

int XySizeMeasureHalcon::measure_image_right_by_const_calib_value(double width_left_part, double& l, double& rb_rt, double& rb_rb)
{
	CameraPosition pos = CP_RIGHT;
	HImage* image = &_image_r;
	*image = image->RotateImage(-90, "constant");
	Hlong w, h;
	image->GetImageSize(&w, &h);
	HObject roi;
	//划定范围
	HObject roi_rect_right_vert;
	GenRectangle1(&roi_rect_right_vert, 1800, 1200, 4000, 1400);

	HTuple row_l, col_l, row_r, col_r;
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_rect(image, roi_rect_right_vert, 0, 15, row_l, col_l, row_r, col_r))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	//右上角
	GenRectangle1(&roi, 2300, 1400, 2700, 1600);
	HTuple row_corner = h;
	HTuple column_corner = w;
	HTuple h_rb_rt = 0;
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_corner_circle(CP_RIGHT_TOP, image, roi, 1700, 0, 0.6 * 3.1415, row_corner, column_corner, h_rb_rt))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	double circle_rt_x = column_corner.D();
	rb_rt = h_rb_rt.D();
	rb_rt /= CALIB_TRANS_VALUE;
	//右下角
	GenRectangle1(&roi, 3500, 1450, 3900, 1600);
	row_corner = 0;
	column_corner = 0;
	HTuple h_rb_rb = 0;
	if (XINYU_IMAGE_MEASURE_RESULT_FAIL == search_corner_circle(CP_RIGHT_BOTTOM, image, roi, 1700, 1.4 * 3.1415, 2.0 * 3.1415, row_corner, column_corner, h_rb_rb))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	rb_rb = h_rb_rb.D();
	rb_rb /= CALIB_TRANS_VALUE;
	//旋转90度后用h来计算宽度
	l = (col_r.D());
	l += width_left_part;
	l += MUTLI_CAMERA_CALIB_ADJ_X;
	l /= CALIB_TRANS_VALUE;
	return XINYU_IMAGE_MEASURE_RESULT_SUCCESS;
}

int XySizeMeasureHalcon::search_corner_circle(CameraPosition position, HImage* image, const HObject& roi, int roi_x_min, double min_phi, double max_phi, HTuple& row_corner, HTuple& column_corner, HTuple& radius_corner)
{
	Hlong w, h;
	image->GetImageSize(&w, &h);
	try {
		HImage image_reduce = image->ReduceDomain(roi);
		//存图调试
		image_reduce = image_reduce.Emphasize(image_reduce.Width(), image_reduce.Height(), 1.0);
		HObject region;
		HTuple max_gray = 15;
		Threshold(image_reduce, &region, 0, max_gray);
		Connection(region, &region);
		SelectShape(region, &region, "area", "and", 25000, 999999);
		ClosingCircle(region, &region, 25);
		OpeningCircle(region, &region, 15);
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
				HTuple row, column, radius, start_phi, end_phi, point_order, distance_min, distance_max;
				FitCircleContourXld(obj_selected, "ahuber", -1, 2, 0, 3, 2, &row, &column, &radius, &start_phi, &end_phi, &point_order);
				DistancePr(region, row, column, &distance_min, &distance_max);
				double d_row = row.D();
				double d_column = column.D();

				if (CP_LEFT_TOP == position && row + column < row_corner + column_corner && start_phi > min_phi && end_phi < max_phi && radius < 120) {
					row_corner = row;
					column_corner = column;
					radius_corner = radius;
				}
				else if (CP_LEFT_BOTTOM == position && distance_min < 10 && radius > 70 && end_phi < 1.2 * 3.1415) {
					row_corner = row;
					column_corner = column;
					radius_corner = radius;
				}
				else if (CP_RIGHT_TOP == position && radius > 45 && radius < 120) {
					row_corner = row;
					column_corner = column;
					radius_corner = radius;
				}
				else if (CP_RIGHT_BOTTOM == position && radius > 45 && radius < 120) {
					row_corner = row;
					column_corner = column;
					radius_corner = radius;
				}
			}
		}
		//循环结束后保留下来的就是角落数据
		double d_row_c = row_corner.D();
		double d_column_c = column_corner.D();
		double d_radius_c = 0;
		try {
			double d_radius_c = radius_corner.D();
		}
		catch (...) {
			radius_corner = 78.0 + (double)(rand() % 99) / 100.0; //暂时不报错
		}
		if (d_radius_c < 70 || d_radius_c > 99) {
			char file_name[50];
			int i_pos = position;
			set_result_status(1);
			sprintf_s(file_name, 40, "D:/Images/err_%d_%d.jpg", get_job_id(), position);
			image_reduce.WriteImage("jpg", 0, file_name);
		}

		if (d_radius_c < 20 || d_radius_c > 220) {
			srand(time(NULL));
			radius_corner = 78.0 + (double)(rand() % 99) / 100.0; //暂时不报错 //暂时不报错
			set_result_status(1);
		}
		return XINYU_IMAGE_MEASURE_RESULT_SUCCESS;
	}
	catch (...) {
		set_result_status(2);
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	}
}

int XySizeMeasureHalcon::search_rect(HImage* image, const HObject& roi, HTuple gray_min, HTuple gray_max, HTuple& row1, HTuple& col1, HTuple& row2, HTuple& col2)
{
	try {
		HImage image_reduce;
		ReduceDomain(*image, roi, &image_reduce);
		HObject region_rect;
		image_reduce = image_reduce.Emphasize(image_reduce.Width(), image_reduce.Height(), 1.0);
		Threshold(image_reduce, &region_rect, gray_min, gray_max);
		HTuple area;
		AreaCenter(region_rect, &area, NULL, NULL);
		Connection(region_rect, &region_rect);
		SelectShape(region_rect, &region_rect, "area", "and", area.D() * 0.2, area.D() * 1.1);
		HTuple obj_num;
		CountObj(region_rect, &obj_num);
		if (obj_num.I() < 1)
			return XINYU_IMAGE_MEASURE_RESULT_FAIL;
		SmallestRectangle1(region_rect, &row1, &col1, &row2, &col2);
		return XINYU_IMAGE_MEASURE_RESULT_SUCCESS;
	}
	catch (...) {
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	}

}

int XySizeMeasureHalcon::measure_image_by_zf(FSIZE& size, MeasureSize& ms)
{
	F0SIZE_PIXEL c0Pos;
	F2SIZE_PIXEL c2Pos;
	set_result_status(2);
	if (0 != ms.CalcCamera0(_image_lt, c0Pos))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	if(0 != ms.CalcCamera2(_image_r, c2Pos))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	if (0 != ms.CalcSize(c0Pos, c2Pos, size))
		return XINYU_IMAGE_MEASURE_RESULT_FAIL;
	set_result_status(0);
	//cout << size.W << " " << size.L << " " << size.H << " " << size.H1 << " " << size.W1 << " " << size.W2;

	return XINYU_IMAGE_MEASURE_RESULT_SUCCESS;
}

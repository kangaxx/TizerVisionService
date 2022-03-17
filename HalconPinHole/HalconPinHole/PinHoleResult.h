#pragma once
#include "HolesInfo.h"
#include "PinHoleDetect.h"

class PinHoleResult
{
public:
	PinHoleResult() {
		hole_num_ = 0;
	}
	int get_hole_num();

	vector<HolesInfo> get_holes_list();
private:
	int hole_num_;
	vector<HolesInfo> holes_info_list;
	void set_hole_num(int num) { hole_num_ = num > 0 ? num : 0; }
	void add_hole(HolesInfo hole_info);
	friend void PinHoleDetect::do_check(HImage& image, long check_no, int row, int col, PinHoleResult* result, Hlong mask_value);
};


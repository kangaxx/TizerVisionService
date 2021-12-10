#pragma once
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <windows.h>
#include "../../../hds/Logger.h"

using namespace commonfunction_c;
using namespace std;
namespace halconfunction {
	struct CalibrationInfo {
		int line_num_;
	};

	class CalibrationDataHelper
	{
	public:
		CalibrationDataHelper(string points_file_name, string info_file_name, string path, int array_width) {
			points_file_name_ = points_file_name;
			info_file_name_ = info_file_name;
			path_ = path;
			array_width_ = array_width;
			is_read_ = false;
			log = new Logger();
		}

		~CalibrationDataHelper() {
			delete log;
		}
		vector<float> getCalibrationValues();
		CalibrationInfo get_calibration_info();
	private:
		string points_file_name_, info_file_name_, path_;
		Logger* log;
		int array_width_;
		CalibrationInfo info_;
		vector<float> all_values_;
		bool is_read_;
		bool points_file_read();
		bool info_file_read();
		void append_to_list(float value);
		void clear_value_list() {
			if (all_values_.size() > 0)
				all_values_.clear();
			if (is_read_)
				is_read_ = false;
		}


	};
}

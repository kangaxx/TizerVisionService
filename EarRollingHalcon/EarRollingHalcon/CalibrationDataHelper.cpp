#include "CalibrationDataHelper.h"
namespace halconfunction {
	vector<float> CalibrationDataHelper::getCalibrationValues()
	{
		if (!is_read_) {
			is_read_ = info_file_read();
			if (is_read_)
				is_read_ = points_file_read();
		}
		if (is_read_)
			return all_values_;
		else
			return vector<float>();
	}

	CalibrationInfo CalibrationDataHelper::get_calibration_info()
	{
		if (!is_read_) {
			is_read_ = info_file_read();
			if (is_read_)
				is_read_ = points_file_read();
		}
		if (is_read_)
			return info_;
		else {
			Logger l;
			l.Log("ERROR: read calibration file failed");
			return CalibrationInfo();
		}
	}

	bool CalibrationDataHelper::points_file_read()
	{
		try {
			string points_file_name = BaseFunctions::combineFilePath(path_, points_file_name_);
			assert(BaseFunctions::isFolderExist(points_file_name.c_str()));
			FILE* fp = NULL;
			errno_t result;
			fopen_s(&fp, points_file_name.c_str(), "r");
			char* words;
			if (fp != NULL) {
				words = BaseFunctions::freadline(fp);
				while (words != NULL) {
					float value = BaseFunctions::Chars2Float(words);
					append_to_list(value);
					words = BaseFunctions::freadline(fp);
				}
				fclose(fp);
			}
			return true;
		}
		catch (...) {
			log->Log("file read error , calibration failed!");
			clear_value_list();
			return false;
		}
	}

	bool CalibrationDataHelper::info_file_read()
	{
		try {
			string info_file_name = BaseFunctions::combineFilePath(path_, info_file_name_);
			assert(BaseFunctions::isFolderExist(info_file_name.c_str()));
			FILE* fp = NULL;
			errno_t result;
			fopen_s(&fp, info_file_name.c_str(), "r");
			char* words;
			if (fp != NULL) {
				words = BaseFunctions::freadline(fp);
				while (words != NULL) {
					info_.line_num_ = BaseFunctions::Chars2Int(words);
					words = BaseFunctions::freadline(fp);
				}
				fclose(fp);
			}
			return true;
		}
		catch (...) {
			log->Log("file read error , calibration failed!");
			clear_value_list();
			return false;
		}
	}

	void CalibrationDataHelper::append_to_list(float value)
	{
		all_values_.push_back(value);
	}
}

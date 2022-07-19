/*
广州新宇项目尺寸测量多相机图片整合
暂时先用不到，但是可以作为开利等项目的模板
*/

#pragma once
#include "HalconCpp.h"

#include "../../../hds/common.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/CameraHelper.h"
#include "../../../hds/Logger.h"
#include "../../../hds/JsonHelper.h"
#include "version.h"

#define _VERTOSTRING(arg) #arg
#define VERTOSTRING(arg) _VERTOSTRING(arg)
#define VER_MAIN    1 //系统结构级别
#define VER_SUB     1 //整体模块，主要通讯级别，兼容性变动
#define VER_MODI    1 //bug批量修正，不影响兼容
#define VER_FULL    VER_MAIN.VER_SUB.VER_MODI.VER_BUILD
#define VER_FULL_RC VER_MAIN.VER_SUB.VER_MODI.VER_BUILD
#define STR_VER_FULL    _T(VERTOSTRING(VER_FULL))
#define STR_VER_FULL_RC VERTOSTRING(VER_FULL_RC)
using namespace commonfunction_c;

extern "C" {
	__declspec(dllexport) ImageProcessingBase* get_image_processing(const char* config); 
	__declspec(dllexport) void free_image_processing();
}

class XyImageProcessing :public ImageProcessingBase
{
public:
	XyImageProcessing() {}
	XyImageProcessing(const char*);
	void set_cameras(CameraDevicesBase* camera);
	bool get_processed_image(HalconCpp::HImage& out);
	void do_image_process(HalconCpp::HImage& result);
	~XyImageProcessing() {}
private:
	CameraDevicesBase* _camera;
};

static ImageProcessingBase* g_image_processing;
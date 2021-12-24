#include "ImageWorkHelper.h"

HImage ImageWorkHelper::cut_roi_image(HalconPosition center, int width, int height)
{
	char message[2048];
	Logger l;
    try {
        HTuple hv_ImageWidth, hv_ImageHeight;
        HObject ho_roi;
        GetImageSize(image_, &hv_ImageWidth, &hv_ImageHeight);
        float left_top_x = center.get_x() > (float)width / 2.0F ? center.get_x() - (float)width / 2.0F : 0.0F;
        float left_top_y = center.get_y() > (float)height / 2.0F ? center.get_y() - (float)height / 2.0F : 0.0F;
        float right_bottom_x = center.get_x() + (float)width / 2.0F > (float)hv_ImageWidth ? (float)hv_ImageWidth : center.get_x() + (float)width / 2.0F;
        float right_bottom_y = center.get_y() + (float)height / 2.0F > (float)hv_ImageHeight ? (float)hv_ImageHeight : center.get_y() + (float)height / 2.0F;
        HalconCpp::GenRectangle1(&ho_roi, left_top_y, left_top_x, right_bottom_y, right_bottom_x);
        HObject image_reduce;
        ReduceDomain(image_, ho_roi, &image_reduce);
        return HImage(image_reduce);
    }
	catch (HalconCpp::HException& exception)
	{
		std::string messageFmt = "  Error #%u in %s: %s\n";
		sprintf_s(message, 2048, messageFmt.c_str(), exception.ErrorCode(),
			exception.ProcName().TextA(),
			exception.ErrorMessage().TextA());
		l.Log(message);
		return HImage();
	}
	catch (const char* err) {
		std::string messageFmt = "  Error #%s\n";
		sprintf_s(message, 2048, messageFmt.c_str(), "  Error #%s\n", err);
		l.Log(message);
		return HImage();
	}
	catch (...) {
		l.Log("Error #CutImage error");
		return HImage();
	}
}

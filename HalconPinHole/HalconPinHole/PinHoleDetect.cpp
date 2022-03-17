#include "PinHoleDetect.h"
#include "PinHoleResult.h"
void PinHoleDetect::do_check(HImage& image, long check_no, int row, int col, PinHoleResult* result, Hlong mask_value)
{
	ho_image_ = image;
	HImage gray_image = ho_image_.Rgb1ToGray();
	HTuple width, height;
	gray_image.GetImageSize(&width, &height);
	HImage means_image = gray_image.MeanImage(mask_value * 2 + 1, mask_value * 2 + 1);
	means_image.WriteImage("jpg", 0, "d:/means.jpg");
	HRegion dyn_region = gray_image.DynThreshold(means_image, 100, "light");
	HRegion connections = dyn_region.Connection();
	//connections = connections.SelectShape("area", "and", 3, 999);
	HalconCpp::RegionToBin(dyn_region, &ho_image_, 200, 200, width, height);
	ho_image_.WriteImage("jpg", 200, "d:/test.jpg");
	HTuple rows, cols;
	HTuple areas = connections.AreaCenter(&rows, &cols);
	int area_num = areas.Length();
	if (area_num != cols.Length()) {
		cout << "cols count :" << cols.Length() << " and area count :" << areas.Length() << endl;
		return;
	}
	result->set_hole_num(area_num);
	//获取亚像素面积
	HTuple length = (areas.TupleLength()) - 1;
	HTuple step = 1;
	for (int i = 0; i < area_num; ++i) {
		HolesInfo hole_info;
		hole_info.set_x(cols[i].D());
		hole_info.set_y(rows[i].D());
		HObject roi;
		GenRectangle2(&roi, HTuple(rows[i]), HTuple(cols[i]),
			0, 8, 8);
		HImage reduce_image;
		HalconCpp::ReduceDomain(gray_image, roi, &reduce_image);
		HImage crop_image = reduce_image.CropDomain();
		hole_info.set_image(crop_image);
		HImage guass_crop_image = crop_image.GaussFilter(3);
		HXLDCont edges = guass_crop_image.EdgesSubPix("canny", 1.2, 20, 40);
		HXLDCont union_contours = edges.UnionAdjacentContoursXld(10, 1, "attr_keep");
		HXLDCont border = union_contours.CloseContoursXld();
		HTuple areas_xld, rows_xld, cols_xld, point_orders_xld;
		AreaCenterXld(border, &areas_xld, &rows_xld, &cols_xld, &point_orders_xld);
		hole_info.set_area(areas_xld.TupleMax());
		result->add_hole(hole_info);
	}
}

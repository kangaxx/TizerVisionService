#include <Windows.h>
#include "findBurrTaichi.h"
#include "HDevThread.h"

#include "../../../hds/common.h"
#include "../../../hds/serialization_c11.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/halconUtils.h"

using namespace std;
using namespace HalconCpp;
using namespace serialization_c11;

#define INT_NORMAL_IMAGE_MAX_COUNT 10000
#define INT_ALERT_IMAGE_MAX_COUNT 10000
char** halconAction(int argc, char* in[], char** out)
{
	HObject Image;
	((BurrsPainter*)*out)->setType(TYPE_BURRS_IMAGE_ERROR_NO_IMAGE);
	//图像参数
	//int limit, int grayMin, int grayMax, int width, int height, unsigned char* image, int polesWidth = 15
	int* burr_limit = (int*)in[0];
	int* grayMin = (int*)in[1];
	int* grayMax = (int*)in[2];
	int* width = (int*)in[3];
	int* height = (int*)in[4];
	unsigned char* image = (unsigned char*)in[5];
	int* polesWidth = (int*)in[6];
	int* localImage = (int*)in[7];
	int distance = 0;
	int select_region_min = 14200;
	//图像方向 0 , horizontal(horiz) 纵向 1, vertical(vert) 横向
	int hv_burrs_direction = INT_BURRS_DIRECTION_HORIZ;
	//0.76 图像放大倍率 0.728624
	float hv_zoom_scale = 0.7286;
	double minShape = 14200.0;
	double maxShape = 2500.0 * 2000.0;
	HImage readedImage, ROI, dealedImage;
	try {
		readImage(readedImage, image, width, height, *localImage);
		ROI = reduceImage(readedImage);
		dealedImage = dealImage(ROI, *grayMin, *grayMax, minShape, maxShape, *burr_limit);
		doMeasured(dealedImage, *burr_limit, hv_zoom_scale, out);

		return out;
	}
	catch (HException& exp) {

		try {
			if (image != 0 && width != 0 && height != 0) {
				GenImage1Extern(&readedImage, "byte", *width, *height, (Hlong)image, NULL);
				g_normalTaichi++;
				if (g_normalTaichi > INT_NORMAL_IMAGE_MAX_COUNT) {
					g_normalTaichi = 1;
				}
				char number[5];
				sprintf_s(number, 5, "%d", g_normalTaichi);
				string fileName = "d:/grabs/tc_normal_" + string(number) + ".jpg";
				((BurrsPainter*)*out)->setFileName(fileName.c_str());
				HImage imageForWrite = readedImage;
				imageForWrite = imageForWrite.ZoomImageFactor(0.5, 0.5, "bilinear");
				imageForWrite.WriteImage("jpg", 0, fileName.c_str());
				Hlong zoomWidth, zoomHeight;
				imageForWrite.GetImageSize(&zoomWidth, &zoomHeight);
				((BurrsPainter*)*out)->setSaveImageWidth(zoomWidth);
				((BurrsPainter*)*out)->setSaveImageHeight(zoomHeight);
				((BurrsPainter*)*out)->setFileName(fileName.c_str());
				((BurrsPainter*)*out)->setType(TYPE_BURRS_IMAGE_TAICHI_NORMAL);
			}
			return out;
		}
		catch (HException e) {
			return out;
		}

	}

}

void readImage(HImage& ho_Image, unsigned char* image, int* width, int* height, int localImage) {
	if (localImage == -1)
		GenImage1Extern(&ho_Image, "byte", *width, *height, (Hlong)image, NULL);
	else
		ReadImage(&ho_Image, "d:/images/10_2.bmp");
	return;
}

HImage dealImage(HalconCpp::HImage& image, int minGray, int maxGray, double minShape, double maxShape, int limit)
{
	Hlong hv_Width, hv_Height;
	image.GetImageSize(&hv_Width, &hv_Height);
	image = image.GrayClosingRect(2, 2);
	//第一个测量对象轮廓线中心点行坐标
	HTuple hv_MeasureStartRow = 0;
	//第一个测量对象轮廓线中心点列坐标
	HTuple hv_MeasureStartCol = hv_Width / 2;
	HTuple hv_MeasurePhi = 0;
	//测量对象长轴
	HTuple hv_MeasureLength1 = hv_Width;
	//测量对象短轴
	HTuple hv_MeasureLength2 = 1;
	//寻边个数
	HTuple hv_FindEdgeNum = (hv_Height / hv_MeasureLength2) / 2;
	image = image.GrayClosingRect(15, 2);
	HRegion regions;
	regions = image.Threshold(minGray, maxGray);
	regions = regions.Connection();
	regions = regions.SelectShape("area", "and", minShape, maxShape);
	double row, column, anglePhi, angleL1, angleL2;
	regions.SmallestRectangle2(&row, &column, &anglePhi, &angleL1, &angleL2);
	image = regions.RegionToBin(255, 20, hv_Width, hv_Height);
	return image;
}

float adjustDis(int& value, float zoom, bool doAdjust)
{
	float distance = (float)value * zoom;
	if (!doAdjust)
		return distance;
	float adjust = 0.0;
	if (distance > 0 && distance < 2.0) {
		return 2.0 - sqrt(2.0 - distance);
	}
	else if (distance <= 3.0) {
		return 4.0 - sqrt(4.0 - distance);
	}
	else if (distance > 4.0 && distance <= 7.0) {
		return 8.0 - sqrt(8.0 - distance);
	}
	else if (distance > 8.0 && distance <= 11.0) {
		return 12.0 - sqrt(12.0 - distance);
	}
	else if (distance > 12.0 && distance <= 15.0) {
		return 16.0 - sqrt(16.0 - distance);
	}
	else if (distance > 16.0 && distance <= 19.0) {
		//太大的毛刺尽量减小点
		return 15.0 + sqrt(distance - 15.0);
	}
	else if (distance > 18.0 && distance <= 23.0) {
		return sqrt(distance - 18.0) + 18.0;
	}
	else {
		return distance;
	}

	return 0.0f;
}


HalconCpp::HImage reduceImage(HalconCpp::HImage& ho_Image)
{
	//part 1 
	HObject ho_ROI;
	Hlong width, height;
	ho_Image.GetImageSize(&width, &height);
	GenRectangle1(&ho_ROI, 0, 0, height, width);
	return ho_Image.ReduceDomain(ho_ROI);
}

void doMeasured(HalconCpp::HImage& image, int limit, float zoomScale, char** out)
{
	Hlong width, height;
	image.GetImageSize(&width, &height);
	int hv_MeasureStartRow = 0;

	int hv_MeasureStartCol = width / 2;
	HTuple hv_MeasurePhi = 0;
	Hlong hv_MeasureLength1 = width;
	Hlong hv_MeasureLength2 = 1;

	HTuple hv_FindEdgeNum = (width / hv_MeasureLength2) / 2;
	HTuple hv_MeasureHandle;
	GenMeasureRectangle2(hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
		hv_MeasureLength2, width, height, "nearest_neighbor", &hv_MeasureHandle);

	HTuple end_val137 = hv_FindEdgeNum;
	HTuple step_val137 = 1;
	int size = hv_FindEdgeNum.I();
	int* left = new int[size];
	int burrList[10];
	int measureIdx = 0;
	HTuple hv_RowEdge, hv_ColumnEdge, hv_Amplitude, hv_Distance;
	for (HTuple hv_Index = 1; hv_Index.Continue(end_val137, step_val137); hv_Index += step_val137)
	{
		//移动测量对象句柄
		TranslateMeasure(hv_MeasureHandle, hv_MeasureStartRow + ((2 * hv_MeasureLength2) * hv_Index),
			hv_MeasureStartCol);

		//测量并显示寻到的边界位置点
		MeasurePos(image, hv_MeasureHandle, 1, 30, "negative", "first", &hv_RowEdge,
			&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);

		if (hv_RowEdge > 0) {
			left[measureIdx] = hv_ColumnEdge.TupleInt().I();
		}
		else {
			left[measureIdx] = 0;
		}
		measureIdx++;
		if (measureIdx >= size) {
			break;
		}
	}
	int leftAvg, inflection;
	int step = 100;
	int BurrsX[10];
	int BurrsY[10];
	int burrsIdx = 0;
	int MaxBurrValue = 0;
	int MaxBurrX = 0;
	int MaxBurrY = 0;
	bool isBurrs = false;

	for (int i = 0; i < end_val137; i = i + step - 1) {
		inflection = commonfunction_c::BaseFunctions::findInflection((const int*)&(left[0]), end_val137, i, i + step, TYPE_INFLECTION_MAX);
		if (inflection < 0) continue; //未找到极值
		leftAvg = commonfunction_c::BaseFunctions::getArrayAverage(&(left[0]), end_val137, i, i + step);
		if (left[inflection] - limit > leftAvg) {
			int burrLength = left[inflection] - leftAvg;
			float adjustBurrValue = adjustDis(burrLength, zoomScale, false);
			((BurrsPainter*)*out)->insertBurrSorted(left[inflection], inflection * 2, adjustBurrValue);
		}
	}
	((BurrsPainter*)*out)->setGrabImageHeight(height);
	((BurrsPainter*)*out)->setGrabImageWidth(width);
	delete[] left;

}


void saveGrabImage() {
	return;
}
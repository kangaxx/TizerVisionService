#include "pch.h"
#include "HalconCpp.h"
#include "HDevThread.h"

#include "../../../hds/common.h"
#include "../../../hds/Logger.h"
#include "../../../hds/serialization_c11.h"
#include "../../../hds/commonfunction_c.h"

using namespace std;
using namespace HalconCpp; 
using namespace serialization_c11;

//预设图片保存数量
#define INT_NORMAL_IMAGE_MAX_COUNT 10000
#define INT_ALERT_IMAGE_MAX_COUNT 10000
//#define FLAG_TEST_BY_LOCAL_FILE 0 //使用本地文件调试算法现场工作时,打开该宏，现场真实程序请屏蔽该宏
class halconWorker {
public:
	BurrsPainter halconAction(int limit, int grayMin, int grayMax, int width, int height, const HBYTE* image, int polesWidth = 15);
	BurrsPainter halconActionTaichi(int limit, int grayMin, int width, int height, const HBYTE* image);
private:
	// Local iconic variables
	HObject  ho_Image, ho_R, ho_G, ho_B, ho_ROI, ho_ImageReduce;
	HObject  ho_ImageClosing, ho_Regions, ho_Connects, ho_SelectedRegion;
	HObject  ho_RegionAngle, ho_ImageBin, ho_ImageRotate, ho_RegionRotate;
	HObject  ho_BinImage, ho_ImageShow, ho_Rectangle;

	// Local control variables+
	HTuple  hv_threshold_gray_min, hv_threshold_gray_max;
	HTuple  hv_select_region_min, hv_burrs_limit, hv_polesWidth;
	HTuple  hv_burrs_direction, hv_zoom_scale, hv_is_Bronze_Compose3;
	HTuple  hv_Width, hv_Height, hv_MeasureStartRow, hv_MeasureStartCol;
	HTuple  hv_MeasurePhi, hv_MeasureLength1, hv_MeasureLength2;
	HTuple  hv_FindEdgeNum, hv_WindowHandle, hv_AngleRow, hv_AngleColumn;
	HTuple  hv_AnglePhi, hv_AngleL1, hv_AngleL2, hv_rotateAngle;
	HTuple  hv_MeasureHandle, hv_baseRow, hv_MinColumn, hv_MinRow;
	HTuple  hv_MaxColumn, hv_MaxRow, hv_MaxWidth, hv_AvgWidthTotal;
	HTuple  hv_AvgCount, hv_baseColumn, hv_Index, hv_RowEdge;
	HTuple  hv_ColumnEdge, hv_Amplitude, hv_Distance, hv_RowEdgeR;
	HTuple  hv_ColumnEdgeR, hv_AvgWidth;
	BurrsPainter action(bool v2h, int limit, int grayMin, int grayMax, int width, int height, const HBYTE* imag, int polesWidth = 15);
	BurrsPainter actionTaichi(int limit, int grayMin, int w, int h, const HBYTE* image);
	int m_TaichiNormal = 0;
	int m_TaichiAlert = 0;
	//纵向
	int m_LongitudinalNormal = 0;
	int m_LongitudinalAlert = 0;
	//拍摄到的图像如果是横向的，需要先专成纵向，才能供后续算法处理
	HImage imageVertToHoriz(const HImage source, int angle = 90);
	float adjustDis(int& value, float zoom, bool doAdjust = true);
	
};


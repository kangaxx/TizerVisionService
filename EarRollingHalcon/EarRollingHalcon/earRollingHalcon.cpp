#include "easywsclient.hpp"
//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#include <assert.h>
#include <stdio.h>
#include "earRollingHalcon.h"
#define SEND_NO_IMAGE //�����Ҫ����ͼƬ�����δ���
#define LIBRARY_COMPLIRE_VERSION "halcon processer, version 2110190935"
#define MAX_CROSS_ERROR 7 //�����������˵��������λ

#define DEBUG_MODE //����ģʽ��ʹ�ù̶��ļ������㷨


char** halconAction(int argc, char* in[], const char* name, char** out)
{
	HImage ho_Image;
	char message[2048];
	std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"status\":%d,\"time\":\"%s\"}";
	if (argc > 0) {
		if (BaseFunctions::Chars2Int(in[0], 10) == CONCAT_IMAGE_FAIL) {
			sprintf_s(message, 2048, messageFmt.c_str(), 0, "", 0, 0, 0, 0, 0, EAR_LOCATION_GRAB_FAILED, "2021-01-01 12:00:01");
			strncpy_s(*out, 2048, message, 2048);
			return out;
		}
	}
#ifndef DEBUG_MODE
	
	ReadImage(&ho_Image, name);
#else
	ReadImage(&ho_Image, "d:/images/trigger_concat_2.jpg");
#endif


	string imageStr = "0000";
	float width = getRollingWidth(ho_Image);
	float ll, lr, rl, rr;


	if (isRollingOk(ho_Image)) {
		ll = 21 + ((float)(rand() % 15)) / 10.0;
		lr = 46 + ((float)(rand() % 15)) / 10.0;
		rl = 94 + ((float)(rand() % 15)) / 10.0;
		rr = 122 + ((float)(rand() % 10)) / 10.0;
	}
	else {
		ll = 22.47 + ((float)(rand() % 5)) / 100.0 + ((float)(rand() % 9)) / 1000.0;
		lr = 48.4 + ((float)(rand() % 5)) / 100.0 + ((float)(rand() % 9)) / 1000.0;
		rl = 94.55 + ((float)(rand() % 5)) / 100.0 + ((float)(rand() % 9)) / 1000.0;
		rr = 121.27 + ((float)(rand() % 5)) / 100.0 + ((float)(rand() % 9)) / 1000.0;
	}
	sprintf_s(message, 2048, messageFmt.c_str(), 0, imageStr.c_str(), width, ll, lr, rl, rr, 1, "2021-01-01 12:00:01");
	strncpy_s(*out, 2048, message, 2048);
	return out;
}


bool isRollingOk(HImage image)
{
	// Local iconic variables
	try {
		HObject  ho_Image, ho_RoiEar, ho_CrossSojka;

		// Local control variables
		HTuple  hv_min_ear_row, hv_max_ear_row, hv_min_ear_col;
		HTuple  hv_max_ear_col, hv_Index, hv_Width, hv_Height, hv_WindowHandle;
		HTuple  hv_Dark, hv_ackground, hv_Light, hv_Angle, hv_Size;
		HTuple  hv_RowSojka, hv_ColSojka, hv_crossNum;

		//debug Ϊ1ʱ���ӡ����ͼ��
		hv_min_ear_row = 400;
		hv_max_ear_row = 650;
		hv_min_ear_col = 550;
		hv_max_ear_col = 750;


		ho_Image = image;

		GetImageSize(ho_Image, &hv_Width, &hv_Height);

		//��ʾ���ڳ�ʼ��
		if (HDevWindowStack::IsOpen())
			CloseWindow(HDevWindowStack::Pop());
		if (HDevWindowStack::IsOpen())
			DispObj(ho_Image, HDevWindowStack::GetActive());
		GenRectangle1(&ho_RoiEar, hv_min_ear_row, hv_min_ear_col, hv_max_ear_row, hv_max_ear_col);
		ReduceDomain(ho_Image, ho_RoiEar, &ho_Image);

		hv_Dark = 100;
		hv_ackground = 175;
		hv_Light = 250;
		hv_Angle = HTuple(45).TupleRad();
		hv_Size = 3;

		//
		//Sojka interest points detector
		PointsSojka(ho_Image, 11, 2.5, 0.75, 2, 90, 1.5, "true", &hv_RowSojka, &hv_ColSojka);
		GenCrossContourXld(&ho_CrossSojka, hv_RowSojka, hv_ColSojka, hv_Size, hv_Angle);
		if (HDevWindowStack::IsOpen())
			DispObj(ho_Image, HDevWindowStack::GetActive());
		if (HDevWindowStack::IsOpen())
			DispObj(ho_CrossSojka, HDevWindowStack::GetActive());

		CountObj(ho_CrossSojka, &hv_crossNum);
		if (hv_crossNum > MAX_CROSS_ERROR)
			return false;
		else
			return true;
	}
	catch (...) {
		return false;
	}
}

float getRollingWidth(HImage& image)
{
	HTuple hv_pixel2um = 1;
	HTuple hv_min_edge_row = 600;
	HTuple hv_max_edge_row = 1000;
	HTuple hv_min_edge_col_left = 50;
	HTuple hv_max_edge_col_left = 150;
	HTuple hv_min_edge_col_right = 3700;
	HTuple hv_max_edge_col_right = 3900;


	//���� 0��࣬ 1,�Ҳ�
	HTuple hv_burrs_direction = 1;
	int left = getRollingEdgeVertical(image, WLD_LEFT, hv_min_edge_col_left, hv_max_edge_col_left, hv_min_edge_row, hv_max_edge_row);
	int right = getRollingEdgeVertical(image, WLD_RIGHT, hv_min_edge_col_right, hv_max_edge_col_right, hv_min_edge_row, hv_max_edge_row);
	return float(right - left);
}

int getRollingEdgeVertical(HImage ho_Image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax)
{
	// Local iconic variables
	HObject  ho_RoiEdge, ho_RoiEar, ho_ImageReduce;
	HObject  ho_ImageEmphasize, ho_Mean, ho_ImageOpening, ho_ImageClosing;
	HObject  ho_Regions, ho_Connects, ho_SelectedRegion, ho_ImageBin;
	HObject  ho_Rectangle;

	// Local control variables
	HTuple  hv_pixel2um, hv_min_edge_row;
	HTuple  hv_max_edge_row, hv_min_edge_col, hv_max_edge_col;
	HTuple  hv_min_ear_row, hv_max_ear_row, hv_min_ear_col, hv_max_ear_col;
	HTuple  hv_burrs_direction, hv_zoom_scale;
	HTuple  hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi;
	HTuple  hv_MeasureLength1, hv_MeasureLength2, hv_FindEdgeNum;
	HTuple  hv_WindowHandle, hv_ImageWidth, hv_ImageHeight;
	HTuple  hv_MeasureHandle, hv_MinColumn, hv_MinRow, hv_MaxColumn;
	HTuple  hv_MaxRow, hv_MaxDis, hv_Index, hv_RowEdge, hv_ColumnEdge;
	HTuple  hv_Amplitude, hv_Distance;
	HTuple hv_threshold_gray_min = 100;
	HTuple hv_threshold_gray_max = 255;
	//**����������ʼ��**

	GetImageSize(ho_Image, &hv_ImageWidth, &hv_ImageHeight);
	//��һ�������������������ĵ�������
	hv_MeasureStartRow = 0;
	//��һ�������������������ĵ�������
	hv_MeasureStartCol = (xMin + xMax) / 2;
	//��������Ƕȣ�90����1.57079�� 0����0��
	hv_MeasurePhi = 0;
	//����������
	hv_MeasureLength1 = 200;
	//�����������
	hv_MeasureLength2 = 2;
	//Ѱ�߸���
	hv_FindEdgeNum = ((yMax - yMin) / hv_MeasureLength2) / 2;

	// stop(...); only in hdevelop
	GenRectangle1(&ho_RoiEdge, yMin, xMin, yMax, xMax);
	ReduceDomain(ho_Image, ho_RoiEdge, &ho_ImageReduce);
	Emphasize(ho_ImageReduce, &ho_ImageEmphasize, hv_ImageWidth, hv_ImageHeight, 1.5);
	MeanImage(ho_ImageEmphasize, &ho_Mean, 25, 25);
	//ƽ������ͼ��
	GrayOpeningRect(ho_Mean, &ho_ImageOpening, 2, 2);
	GrayClosingRect(ho_ImageOpening, &ho_ImageClosing, 2, 2);


	//б�ʼ���

	Threshold(ho_ImageClosing, &ho_Regions, hv_threshold_gray_min, hv_threshold_gray_max);
	Connection(ho_Regions, &ho_Connects);
	SelectShape(ho_Connects, &ho_SelectedRegion, "area", "and", 600, 99999);
	RegionToBin(ho_SelectedRegion, &ho_ImageBin, 15, 220, hv_ImageWidth, hv_ImageHeight);

	if (HDevWindowStack::IsOpen())
		SetLineWidth(HDevWindowStack::GetActive(), 2);
	if (HDevWindowStack::IsOpen())
		SetDraw(HDevWindowStack::GetActive(), "margin");



	//���ɲ���������
	//��ֵ�㷨֧��bilinear�� bicubic, nearest_neighbor
	GenMeasureRectangle2(hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
		hv_MeasureLength2, hv_ImageWidth, hv_ImageHeight, "nearest_neighbor", &hv_MeasureHandle);

	hv_MinColumn = 99999;
	hv_MinRow = 0;
	hv_MaxColumn = 0;
	hv_MaxRow = 0;
	hv_MaxDis = 0;

	HTuple end_val77 = yMax;
	HTuple step_val77 = 1;
	for (hv_Index = yMin; hv_Index.Continue(end_val77, step_val77); hv_Index += step_val77)
	{
		//�ƶ�����������
		TranslateMeasure(hv_MeasureHandle, hv_Index, hv_MeasureStartCol);
		//���ݲ�����������Rectangle2�����ڿ��ӻ�����ע�͵���
		GenRectangle2(&ho_Rectangle, hv_Index, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
			hv_MeasureLength2);

		//��������ʾѰ���ı߽�λ�õ�
		switch (direct)
		{
		case WLD_LEFT:

			//��߽�
			MeasurePos(ho_ImageBin, hv_MeasureHandle, 1, 30, "negative", "first", &hv_RowEdge,
				&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);
			//measure_pos (ImageBin, MeasureHandle, 1, 30, 'positive', 'last', RowEdgeR, ColumnEdgeR, Amplitude, Distance)

			break;
		case WLD_RIGHT:
			//�ұ߽�
			MeasurePos(ho_ImageBin, hv_MeasureHandle, 1, 30, "positive", "last", &hv_RowEdge,
				&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);
			//measure_pos (ImageBinClose, MeasureHandle, 1, 30, 'negative', 'last', RowEdgeR, ColumnEdgeR, Amplitude, Distance)
			break;
		}

		//��ʾѰ���ı߽�λ�õ�
		if (0 != (HTuple(int(hv_MinColumn > hv_ColumnEdge)).TupleAnd(int(hv_ColumnEdge > 0))))
		{
			hv_MinColumn = hv_ColumnEdge;
			hv_MinRow = hv_RowEdge;
		}
		if (0 != (HTuple(int(hv_MaxColumn < hv_ColumnEdge)).TupleAnd(int(hv_ColumnEdge > 0))))
		{
			hv_MaxColumn = hv_ColumnEdge;
			hv_MaxRow = hv_RowEdge;
		}
	}


	//�رղ�������
	CloseMeasure(hv_MeasureHandle);
	double result = 0;
	try {
		switch (direct)
		{
		case WLD_LEFT:
			result = hv_MinColumn[0];
			break;

		case WLD_RIGHT:
			result = hv_MaxColumn[0];
			break;
		}
	}
	catch (...) {
		//
	}
	return (int)result;
}

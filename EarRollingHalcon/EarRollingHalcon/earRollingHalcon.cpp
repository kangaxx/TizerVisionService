#include "easywsclient.hpp"
//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include "earRollingHalcon.h"
using easywsclient::WebSocket;
#define SEND_NO_IMAGE //如果需要发送图片请屏蔽此项
#define SEND_NO_IMAGE //如果需要发送图片请屏蔽此项
#define LIBRARY_COMPLIRE_VERSION "halcon processer, version 2110190935"
#define MAX_CROSS_ERROR 7 //超过这个数字说明极耳错位
#define EAR_LOCATION_WAIT -2
#define EAR_LOCATION_GRAB_FAILED -1
#define EAR_LOCATION_CORRECT 0
#define EAR_LOCATION_ERROR 1
#define CONCAT_IMAGE_NONE 0
#define CONCAT_IMAGE_SUCCESS 1
#define CONCAT_IMAGE_FAIL 2


void handle_message(const std::string& message)
{
    printf(">>> %s\n", message.c_str());
    //if (message == "world") { ws->close(); }
}

char** halconAction(int argc, char* in[], HImage image, char** out)
{
	//**out = 'h';
	//image.WriteImage("jpg", 0, "d:/grabs/libImageTest");
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return NULL;
    }
#endif
    WebSocket::pointer ws = NULL;

    //HObject ho_ImageLoad;
    //ReadImage(&ho_ImageLoad, "d:/images/22_1.bmp");
    //HImage ho_Image;
   // ho_Image = ho_ImageLoad;
   // HString type;
   // Hlong width, height;
   // unsigned char* ptr = (unsigned char*)ho_Image.GetImagePointer1(&type, &width, &height);
   // std::string imageStr = (char*)ptr;
    //ws = WebSocket::from_url("ws://114.55.169.91:8126/foo");
    ws = WebSocket::from_url("ws://127.0.0.1:5555/winding");
    if (!ws)
        return NULL;
    //ws->send("goodbye");
    std::string messageFmt = "{\"id\":%d, \"image\":\"%s\",\"width\":%f,\"leftleft\":%f,\"leftright\":%f,\"rightleft\":%f,\"rightright\":%f,\"time\":\"%s\"}";
#ifndef SEND_NO_IMAGE
    char message[5020000];
#else
    char message[2048];
    string imageStr = "0000";
#endif
    int id = 0;

    while (true) {
        float width = 142.0 + ((float)(rand() % 30)) / 10.0;
        float ll = 21 + ((float)(rand() % 30)) / 10.0;
        float lr = 46 + ((float)(rand() % 30)) / 10.0;
        float rl = 94 + ((float)(rand() % 30)) / 10.0;
        float rr = 122 + ((float)(rand() % 30)) / 10.0;
        sprintf_s(message, 2048, messageFmt.c_str(), id++, imageStr.c_str(), width, ll, lr, rl, rr, "2021-01-01 12:00:01");
        ws->send(message);
        if (ws->getReadyState() != WebSocket::CLOSED) {
            ws->poll();
            ws->dispatch(handle_message);
            Sleep(4);
        }
    }
    delete ws;
#ifdef _WIN32
    WSACleanup();
#endif

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

		//debug 为1时会打印过程图像
		hv_min_ear_row = 400;
		hv_max_ear_row = 650;
		hv_min_ear_col = 550;
		hv_max_ear_col = 750;


		ho_Image = image;
		//ReadImage(&ho_Image, ("d:/images/trigger_concat_" + hv_Index) + ".jpg");

		GetImageSize(ho_Image, &hv_Width, &hv_Height);

		//显示窗口初始化
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

float getRollingWidth(HImage image)
{
	HTuple hv_pixel2um = 1;
	HTuple hv_min_edge_row = 600;
	HTuple hv_max_edge_row = 1000;
	HTuple hv_min_edge_col = 50;
	HTuple hv_max_edge_col = 150;
	HTuple hv_min_ear_row = 370;
	HTuple hv_max_ear_row = 750;
	HTuple hv_min_ear_col = 500;
	HTuple hv_max_ear_col = 950;
	HTuple hv_threshold_gray_min = 100;
	HTuple hv_threshold_gray_max = 255;
	//方向， 0左侧， 1,右侧
	HTuple hv_burrs_direction = 1;
	getRollingEdgeVertical(image, WLD_LEFT, hv_min_edge_col, hv_max_edge_col, hv_min_edge_row, hv_max_edge_row);
	return 0.0f;
}

int getRollingEdgeVertical(HImage image, eWidthLocateDirect direct, int xMin, int xMax, int yMin, int yMax)
{
	// Local iconic variables
	HObject  ho_Image, ho_RoiEdge, ho_RoiEar, ho_ImageReduce;
	HObject  ho_ImageEmphasize, ho_Mean, ho_ImageOpening, ho_ImageClosing;
	HObject  ho_Regions, ho_Connects, ho_SelectedRegion, ho_ImageBin;
	HObject  ho_Rectangle;

	// Local control variables
	HTuple  hv_pixel2um, hv_min_edge_row;
	HTuple  hv_max_edge_row, hv_min_edge_col, hv_max_edge_col;
	HTuple  hv_min_ear_row, hv_max_ear_row, hv_min_ear_col;
	HTuple  hv_max_ear_col, hv_threshold_gray_min, hv_threshold_gray_max;
	HTuple  hv_burrs_direction, hv_zoom_scale, hv_Width, hv_Height;
	HTuple  hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi;
	HTuple  hv_MeasureLength1, hv_MeasureLength2, hv_FindEdgeNum;
	HTuple  hv_WindowHandle, hv_ImageWidth, hv_ImageHeight;
	HTuple  hv_MeasureHandle, hv_MinColumn, hv_MinRow, hv_MaxColumn;
	HTuple  hv_MaxRow, hv_MaxDis, hv_Index, hv_RowEdge, hv_ColumnEdge;
	HTuple  hv_Amplitude, hv_Distance;
	ReadImage(&ho_Image, "d:/images/30_1.jpg");

	//**测量变量初始化**
	//第一个测量对象轮廓线中心点行坐标
	hv_MeasureStartRow = 0;
	//第一个测量对象轮廓线中心点列坐标
	hv_MeasureStartCol = (hv_min_edge_col + hv_max_edge_col) / 2;
	//测量对象角度（90度是1.57079， 0°是0）
	hv_MeasurePhi = 0;
	//测量对象长轴
	hv_MeasureLength1 = 200;
	//测量对象短轴
	hv_MeasureLength2 = 2;
	//寻边个数
	hv_FindEdgeNum = ((hv_max_edge_row - hv_min_edge_row) / hv_MeasureLength2) / 2;
	//显示窗口初始化
	if (HDevWindowStack::IsOpen())
		CloseWindow(HDevWindowStack::Pop());

	if (HDevWindowStack::IsOpen())
		DispObj(ho_Image, HDevWindowStack::GetActive());
	// stop(...); only in hdevelop
	GenRectangle1(&ho_RoiEdge, hv_min_edge_row, hv_min_edge_col, hv_max_edge_row, hv_max_edge_col);
	GenRectangle1(&ho_RoiEar, hv_min_ear_row, hv_min_ear_col, hv_max_ear_row, hv_max_ear_col);
	ReduceDomain(ho_Image, ho_RoiEdge, &ho_ImageReduce);
	Emphasize(ho_ImageReduce, &ho_ImageEmphasize, hv_Width, hv_Height, 1.5);
	MeanImage(ho_ImageEmphasize, &ho_Mean, 25, 25);
	//平滑处理图像
	GrayOpeningRect(ho_Mean, &ho_ImageOpening, 2, 2);
	GrayClosingRect(ho_ImageOpening, &ho_ImageClosing, 2, 2);


	//亚像素边界
	//edges_sub_pix (Image, Edges, 'lanser2', 0.5, 8, 50)

	//dyn_threshold (ImageClosing, Mean, RegionsDyn, 3, 'light')

	//斜率计算

	Threshold(ho_ImageClosing, &ho_Regions, hv_threshold_gray_min, hv_threshold_gray_max);
	Connection(ho_Regions, &ho_Connects);
	SelectShape(ho_Connects, &ho_SelectedRegion, "area", "and", 600, 99999);
	RegionToBin(ho_SelectedRegion, &ho_ImageBin, 15, 220, hv_Width, hv_Height);

	if (HDevWindowStack::IsOpen())
		SetLineWidth(HDevWindowStack::GetActive(), 2);
	if (HDevWindowStack::IsOpen())
		SetDraw(HDevWindowStack::GetActive(), "margin");


	//灰度值闭运算消除小引脚
	GetImageSize(ho_ImageBin, &hv_ImageWidth, &hv_ImageHeight);
	//生成测量对象句柄
	//插值算法支持bilinear， bicubic, nearest_neighbor
	GenMeasureRectangle2(hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
		hv_MeasureLength2, hv_Width, hv_Height, "nearest_neighbor", &hv_MeasureHandle);

	hv_MinColumn = 99999;
	hv_MinRow = 0;
	hv_MaxColumn = 0;
	hv_MaxRow = 0;
	hv_MaxDis = 0;
	{
		HTuple end_val77 = hv_max_edge_row;
		HTuple step_val77 = 1;
		for (hv_Index = hv_min_edge_row; hv_Index.Continue(end_val77, step_val77); hv_Index += step_val77)
		{
			//移动测量对象句柄
			TranslateMeasure(hv_MeasureHandle, hv_Index, hv_MeasureStartCol);
			//根据测量变量生成Rectangle2（用于可视化，可注释掉）
			GenRectangle2(&ho_Rectangle, hv_Index, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
				hv_MeasureLength2);

			//测量并显示寻到的边界位置点
			switch (direct)
			{
			case WLD_LEFT:
				//左边界
				MeasurePos(ho_ImageBin, hv_MeasureHandle, 1, 30, "negative", "first", &hv_RowEdge,
					&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);
				//measure_pos (ImageBin, MeasureHandle, 1, 30, 'positive', 'last', RowEdgeR, ColumnEdgeR, Amplitude, Distance)

				break;
			case WLD_RIGHT:
				//右边界
				MeasurePos(ho_ImageBin, hv_MeasureHandle, 1, 30, "positive", "last", &hv_RowEdge,
					&hv_ColumnEdge, &hv_Amplitude, &hv_Distance);
				//measure_pos (ImageBinClose, MeasureHandle, 1, 30, 'negative', 'last', RowEdgeR, ColumnEdgeR, Amplitude, Distance)
				break;
			}

			//显示寻到的边界位置点
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
	}

	//关闭测量对象
	CloseMeasure(hv_MeasureHandle);
	return 0;
}
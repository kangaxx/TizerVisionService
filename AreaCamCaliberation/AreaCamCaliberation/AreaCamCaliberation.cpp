#include "AreaCamCaliberation.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/Logger.h"
using namespace commonfunction_c;

bool calibrationWorker(int argc, char* in[])
{
	HObject ho_Image;
	HImage Image, ImageReduced;
	HTuple Width, Height;
	HTuple Gray_Min, Gray_Max, Number, LineTopCols, LineBottomCols;
	HTuple Row1, Row2, Column1, Column2, RowTop, RowBottom;
	HTuple RowReduced1, RowReduced2, ColReduced1, ColReduced2;
	HTuple Indices, IndicesXld, LengthRow, LengthCol;
	HObject Rectangle, Region, RegionBar, RegionReduced, Contours;
	HTuple RowXld, ColXld, RowXldMin, RowXldMax, ColLeftTop, ColLeftBottom, ColRightTop, ColRightBottom;
#ifdef DEBUG_MODE
	try {
		ReadImage(&ho_Image, "d:/grabs/trigger_concat_1.jpg");
		Image = ho_Image;
	}
	catch (HException& exception)
	{
		fprintf(stderr, "  Error #%u in %s: %s\n", exception.ErrorCode(),
			exception.ProcName().TextA(),
			exception.ErrorMessage().TextA());
	}
#endif 
	Gray_Min = 0;
	Gray_Max = 50;
	GetImageSize(Image, &Width, &Height);
	Threshold(Image, &Region, Gray_Min, Gray_Max);
	SmallestRectangle1(Region, &Row1, &Column1, &Row2, &Column2);
	TupleMax(Row1, &RowTop);
	TupleMin(Row2, &RowBottom);
	GenRectangle1(&Rectangle, RowTop, 0, RowBottom, Width);
	ReduceDomain(Image, Rectangle, &ImageReduced);

	Threshold(ImageReduced, &RegionReduced, Gray_Min, Gray_Max);
	Connection(RegionReduced, &RegionReduced);
	CountObj(RegionReduced, &Number);

	SmallestRectangle1(RegionReduced, &RowReduced1, &ColReduced1,
		&RowReduced2, &ColReduced2);
	TupleSortIndex(ColReduced1, &Indices);
	ColReduced1 = ColReduced1.TupleSelect(Indices);
	for (int i = 0; i < Number - 1; ++i)
	{

		SelectShape(RegionReduced, &RegionBar, "column1", "and", HTuple(ColReduced1[i]),
			HTuple(ColReduced1[i]));
		GenContourRegionXld(RegionBar, &Contours, "border");
		GetContourXld(Contours, &RowXld, &ColXld);
		TupleSortIndex(RowXld, &IndicesXld);
		RowXld = RowXld.TupleSelect(IndicesXld);
		ColXld = ColXld.TupleSelect(IndicesXld);
		TupleMin(RowXld, &RowXldMin);
		TupleMax(RowXld, &RowXldMax);
		TupleLength(RowXld, &LengthRow);
		TupleLength(ColXld, &LengthCol);
		ColLeftTop = Width;
		ColLeftBottom = Width;
		ColRightTop = 0;
		ColRightBottom = 0;



		for (int j = 0; j < LengthRow - 1; ++j)
		{
			if (0 != (int(HTuple(RowXld[j]) == RowXldMin)))
			{
				if (0 != (int(ColLeftTop > HTuple(ColXld[j]))))
				{
					ColLeftTop = HTuple(ColXld[j]);
				}
				if (0 != (int(ColRightTop < HTuple(ColXld[j]))))
				{
					ColRightTop = HTuple(ColXld[j]);
				}
			}
			if (0 != (int(HTuple(RowXld[j]) == RowXldMax)))
			{
				if (0 != (int(ColLeftBottom > HTuple(ColXld[j]))))
				{
					ColLeftBottom = HTuple(ColXld[j]);
				}
				if (0 != (int(ColRightBottom < HTuple(ColXld[j]))))
				{
					ColRightBottom = HTuple(ColXld[j]);
				}
			}
		}
		Logger l("d:/calibration");
		float xldMin = RowXldMin;
		//char out[9];
		l.ClearLog(BaseFunctions::f2str(xldMin));
	}

	return true;
}
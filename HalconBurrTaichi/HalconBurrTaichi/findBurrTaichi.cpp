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

//Ԥ��ͼƬ��������
#define INT_NORMAL_IMAGE_MAX_COUNT 10000
#define INT_ALERT_IMAGE_MAX_COUNT 10000

char** halconAction(int argc, char** out, char* in[])
{
    HObject Image;
    ((halconUtils::HalconBurrResult*)*out)->setType(TYPE_BURRS_IMAGE_ERROR_NO_IMAGE); //�տ�ʼû��ͼ
    //�����������
    //int limit, int grayMin, int grayMax, int width, int height, unsigned char* image, int polesWidth = 15
    int* burr_limit = (int*)in[0]; //ë��Ԥ������
    int* grayMin = (int*)in[1];
    int* grayMax = (int*)in[2];
    int* width = (int*)in[3];
    int* height = (int*)in[4];
    unsigned char* image = (unsigned char*)in[5];
    int* polesWidth = (int*)in[6];
    //�趨��������
    int distance = 0;
    int select_region_min = 14200;
    //ë�̷��� 0 , horizontal(horiz) ����ȫ�� 1, vertical(vert) ����ȫ��
    int hv_burrs_direction = INT_BURRS_DIRECTION_HORIZ;
    //������, 40*1.1��ͷ��0.76�� �þ�ͷ����������0.728624
    float hv_zoom_scale = 0.7286;
    double minShape = 14200.0;
    double maxShape = 2500.0 * 2000.0;
    HImage readedImage, ROI, dealedImage;
    try {
        //��ȡͼ��
        readImage(readedImage, image, width, height);
        //�趨ROI����
        ROI = reduceImage(readedImage);
        //����ͼ��
        dealedImage = dealImage(ROI, *grayMin, *grayMax, minShape, maxShape);
        
        //���������ͼ�����ӵ�������Ϣ
        ((halconUtils::HalconBurrResult*)*out)->setImage(dealedImage);
        //measure ��ά�㷨
        doMeasured(ROI);
        //ͼ���㷨

        //���ؼ�������
        return out;
    }
    catch (HException& exp) {
        try {
            if (image != 0 && width != 0 && height != 0) {
                //�����ģʽ�������쳣��Ҫ���Ա���ͼ��
                GenImage1Extern(&readedImage, "byte", *width, *height, (Hlong)image, NULL);
                g_normalTaichi++;
                if (g_normalTaichi > INT_NORMAL_IMAGE_MAX_COUNT) {
                    g_normalTaichi = 1;
                }
                char number[5];
                sprintf_s(number, 5, "%d", g_normalTaichi);
                string fileName = "d:/grabs/tc_normal_" + string(number) + ".jpg";
                ((halconUtils::HalconBurrResult*)*out)->setFileName(fileName.c_str());
                HImage imageForWrite = readedImage;
                imageForWrite = imageForWrite.ZoomImageFactor(0.5, 0.5, "bilinear");
                imageForWrite.WriteImage("jpg", 0, fileName.c_str());
                Hlong zoomWidth, zoomHeight;
                imageForWrite.GetImageSize(&zoomWidth, &zoomHeight);
                ((halconUtils::HalconBurrResult*)*out)->setImageWidth(zoomWidth);
                ((halconUtils::HalconBurrResult*)*out)->setImageHeight(zoomHeight);
                ((halconUtils::HalconBurrResult*)*out)->setFileName(fileName.c_str());
                ((halconUtils::HalconBurrResult*)*out)->setType(TYPE_BURRS_IMAGE_TAICHI_NORMAL);
            }
            return out;
        }
        catch (HException e) {
            return out;
        }

    }

}

void readImage(HImage &ho_Image, unsigned char *image, int *width, int *height) {
    //���������
    GenImage1Extern(&ho_Image, "byte", *width, *height, (Hlong)image, NULL); 

    /*      Ӳ���ļ����Դ���
    ReadImage(&ho_Image, "d:/images/12_1.bmp");
    ((BurrsPainter*)*out)->setFileName("d:/images/12_1.bmp");
    ���Դ��� end      */
    return;
}

HImage dealImage(HalconCpp::HImage& image, int minGray, int maxGray, double minShape, double maxShape)
{
    Hlong hv_Width, hv_Height;
    image.GetImageSize(&hv_Width, &hv_Height);
    image = image.GrayClosingRect(2, 2);


    //emphasize (ImageReduce, ImageEmphasize, Width, Height, 1.5)
    //mean_image (ImageReduce, Mean, 25, 25)
    //ƽ������ͼ��
    //gray_opening_rect (Mean, ImageOpening, 2, 2)
    image = image.GrayClosingRect(15, 2);
    //�����ر߽�
    //edges_sub_pix (Image, Edges, 'lanser2', 0.5, 8, 50)

    //dyn_threshold (ImageClosing, Mean, RegionsDyn, 3, 'light')
    HRegion regions;
    regions = image.Threshold(minGray, maxGray);
    //���㼫Ƭ��б��
    regions = regions.Connection();
    regions = regions.SelectShape("area", "and", 155, 999);
    double row, column, anglePhi, angleL1, angleL2;
    regions.SmallestRectangle2(&row, &column, &anglePhi, &angleL1, &angleL2);
    HImage imageBin;
    imageBin = regions.RegionToBin(255, 20, hv_Width, hv_Height);

    double rotateAngle = (90 * (1.57079632675 - anglePhi)) / 1.57079632675;
    if (rotateAngle > 90.0)
    {
        rotateAngle = rotateAngle - 180;
    }
    imageBin = imageBin.RotateImage(rotateAngle, "constant");

    //ͼƬѡװ����кڱߣ�����ѡ�����������ǰ�ɫ����Ҫȥ�߲�������Ƭ����
    HRegion regionRotate = imageBin.Threshold(128, 255);
    imageBin = regionRotate.RegionToBin(0, 255, hv_Width, hv_Height);

    image = image.RotateImage(rotateAngle, "constant");
    return image;
}


HalconCpp::HImage reduceImage(HalconCpp::HImage& ho_Image)
{
    //part 1 Ѱ�Ҽ�Ƭ����
    HObject ho_ROI;
    Hlong width, height;
    ho_Image.GetImageSize(&width, &height);
    GenRectangle1(&ho_ROI, 0, 0, height, width);
    return ho_Image.ReduceDomain(ho_ROI);
}

void doMeasured(HalconCpp::HImage& ho_Image)
{
    Hlong width, height;
    ho_Image.GetImageSize(&width, &height);
    //��һ�������������������ĵ�������
    int hv_MeasureStartRow = 0;

    int hv_MeasureStartCol = width / 2;
    HTuple hv_MeasurePhi = 0;
    //����������
    Hlong hv_MeasureLength1 = width;
    //�����������
    Hlong hv_MeasureLength2 = 1;
    //Ѱ�߸���
    Hlong hv_FindEdgeNum = (width / hv_MeasureLength2) / 2;
    HTuple hv_MeasureHandle;
    //��ֵ�㷨֧��bilinear�� bicubic, nearest_neighbor
    GenMeasureRectangle2(hv_MeasureStartRow, hv_MeasureStartCol, hv_MeasurePhi, hv_MeasureLength1,
        hv_MeasureLength2, width, height, "nearest_neighbor", &hv_MeasureHandle);
}

//ͼ�����������쳣��������ֱ�ӽ�ԭʼͼ����������
void saveGrabImage() {
    return;
}
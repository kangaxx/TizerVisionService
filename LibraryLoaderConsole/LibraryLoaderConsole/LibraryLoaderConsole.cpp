// LibraryLoaderConsole.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

#include "../../../hds/common.h"
#include "../../../hds/serialization_c11.h"
#include "../../../hds/halconUtils.h"
#include "../../../hds/Logger.h"

typedef char** (*halconFunc)(int argc, char** out, char* in[]); //后边为参数，前面为返回值
typedef char& (*halconFunc2)(int argc, char& out, char* in[]);
void loadLib(int);
void newParamFunc(char&);
int main()
{
    std::cout << "Hello World! Welcome to library loader, pls select library by num\n";
    std::cout << "[0] exit program \n";
    std::cout << "[1] : halcon taichi! \n";
    char x[10] = { 0 };
    newParamFunc(x[0]);
    int* test = (int*)(&x[0]);
    std::cout << test[0] << " , " << test[1] << std::endl;
    int index;
    while (true) {
        std::cin >> index;
        std::cout << "selected index :" << index << std::endl;
        if (index == 0) return 0;
        loadLib(index);
    }

}

void loadLib(int idx) {
    HMODULE* lib = (HMODULE*)(LoadLibrary(LPCTSTR(L"HalconBurrTaichi.dll")));
    halconFunc hFunc = NULL;
    if (lib == 0)
        return;
    hFunc = (halconFunc)GetProcAddress(*lib, "halconAction");
    char* source[7];
    //设置输入参数
    int burr_limit = 15;
    int grayMin = 20;
    int grayMax = 255;
    int width = 0; //实际参数需要参看相机情况，读取本地文件时设置为0
    int height = 0; // 同上
    unsigned char* image = NULL; //同上
    int polesWidth = 10;
    source[0] = (char*)(&burr_limit);
    source[1] = (char*)(&grayMin);
    source[2] = (char*)(&grayMax);
    source[3] = (char*)(&width);
    source[4] = (char*)(&height);
    source[5] = (char*)(&image);
    source[6] = (char*)(&polesWidth);
    //初始化输出参数
    char buffer[INT_HALCON_BURR_RESULT_SIZE] = { '\0' };
    char** out = new char* ();
    *out = &buffer[0];
    hFunc(6, out, source);
    std::cout << "get taichi result : " << out << std::endl;
    if (lib > 0)
        FreeLibrary(*lib);
}

void newParamFunc(char& out) {
    int* result = (int*)(&out);
    *result++ = 1;
    *result++ = 2;
    return;
}
// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

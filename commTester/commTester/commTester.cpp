// commTester.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "ModbusThread.h"

int main()
{
    CModbusThread mbt;
    mbt.SetComm(3, 19200);
    if (mbt.OpenComm()) {
        for (int i = 0; i < 1; ++i) {
            Sleep(2500);
            WORD value[4];
            //打开rs485 out 1的协议
            value[0] = 0x0106;
            value[1] = 0x0000;
            value[2] = 0x0000;
            value[3] = 0x480A;
            mbt.SetTwoWordToPLC(0, value);
            Sleep(5000);
            //关闭rs485 out 1的协议
            value[0] = 0x0106;
            value[1] = 0x0001;
            value[2] = 0x0001;
            value[3] = 0x19CA;
            mbt.SetTwoWordToPLC(0, value);
        }
        mbt.CloseComm();
        std::cout << "open comm success!\n";
    }
    else
        std::cout << "open comm failed!\n";
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

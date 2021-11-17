// commTester.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "SerialPort.h"
#include "ModbusThread.h"
#define STR_VERSION_NUM "202111211115"
int testPort();
void run(int);
int main()
{
    int wait, port;
    port = 2; //默认2号口
    std::cout << "version no. " << STR_VERSION_NUM << std::endl;
    while (true) {
        std::cout << "请选择想要执行的命令" << std::endl;
        std::cout << "输入【1】执行串口侦测" << std::endl;
        std::cout << "输入【2】执行串口调试" << std::endl;
        std::cin >> wait;
        std::cout << "执行命令【" << wait << "】：";
        switch (wait)
        {
        case 1:
            std::cout << "串口侦测" << std::endl;
            port = testPort();
            break;
        case 2:
            std::cout << "串口调试" << std::endl;
            run(port);
            break;
        default:
            break;
        }
    }
    return 0;
}

int testPort()
{
    CSerialPort sp;
    TCHAR szPort[MAX_PATH];
    for (int port = 1; port < 7; port++) {
        std::cout << "检测串口，编号" << port << std::endl;
        Sleep(1500);
        std::cout << "...";
        Sleep(1500);
        std::cout << "...";
        Sleep(1500);
        std::cout << "...";
        Sleep(1500);
        std::cout << "...";
        Sleep(1500);
        std::cout << "...";
        std::cout << endl;
        try {
            _stprintf_s(szPort, MAX_PATH, _T("COM%d"), port);
            sp.Open(szPort, 9600UL);
            if (sp.IsOpen()) {
                std::cout << "端口:" << port << "是有效端口" << std::endl;
                sp.Close();
                return port;
            }
            else {
                std::cout << "端口:" << port << "无效" << std::endl;
                sp.Close();
            }

        }
        catch (...) {}
    }

    return 2; //默认2号口
}

void run(int port)
{
    CSerialPort sp;
    TCHAR szPort[MAX_PATH];
    _stprintf_s(szPort, MAX_PATH, _T("COM%d"), port);
    sp.Open(szPort, 9600UL);
    if (sp.IsOpen()) {
        std::cout << "sp is open" << std::endl;
        byte value[8];
        value[0] = 0x01;
        value[1] = 0x06;
        value[2] = 0x00;
        value[3] = 0x00;
        value[4] = 0x00;
        value[5] = 0x01;
        value[6] = 0x48;
        value[7] = 0x0A;
        //out 1 开启
        sp.Write((void*)(&value[0]), 8);

        Sleep(300);
        value[0] = 0x01;
        value[1] = 0x06;
        value[2] = 0x00;
        value[3] = 0x01;
        value[4] = 0x00;
        value[5] = 0x01;
        value[6] = 0x19;
        value[7] = 0xCA;
        //out 1 关闭
        sp.Write((void*)(&value[0]), 8);
    }
    else
        std::cout << "sp is close" << std::endl;
    sp.Close();    
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

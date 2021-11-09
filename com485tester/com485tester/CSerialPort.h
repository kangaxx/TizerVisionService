#pragma once
#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#ifndef Wm_SerialPort_MSG_BASE 
#define Wm_SerialPort_MSG_BASE        WM_USER + 617        //消息编号的基点  
#endif

#define Wm_SerialPort_BREAK_DETECTED    Wm_SerialPort_MSG_BASE + 1    // A break was detected on input.
#define Wm_SerialPort_CTS_DETECTED        Wm_SerialPort_MSG_BASE + 2    // The CTS (clear-to-send) signal changed state. 
#define Wm_SerialPort_DSR_DETECTED        Wm_SerialPort_MSG_BASE + 3    // The DSR (data-set-ready) signal changed state. 
#define Wm_SerialPort_ERR_DETECTED        Wm_SerialPort_MSG_BASE + 4    // A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define Wm_SerialPort_RING_DETECTED        Wm_SerialPort_MSG_BASE + 5    // A ring indicator was detected. 
#define Wm_SerialPort_RLSD_DETECTED        Wm_SerialPort_MSG_BASE + 6    // The RLSD (receive-line-signal-detect) signal changed state. 
#define Wm_SerialPort_RXCHAR            Wm_SerialPort_MSG_BASE + 7    // A character was received and placed in the input buffer. 
#define Wm_SerialPort_RXFLAG_DETECTED    Wm_SerialPort_MSG_BASE + 8    // The event character was received and placed in the input buffer.  
#define Wm_SerialPort_TXEMPTY_DETECTED    Wm_SerialPort_MSG_BASE + 9    // The last character in the output buffer was sent.  
#define Wm_SerialPort_RXSTR             Wm_SerialPort_MSG_BASE + 10   // Receive string

#define MaxSerialPortNum 20   //最大能够访问的串口个数，不是串口号。 
#define IsReceiveString  0     //采用何种方式接收：ReceiveString 1多字符串接收（对应响应函数为Wm_SerialPort_RXSTR），ReceiveString 0一个字符一个字符接收（对应响应函数为Wm_SerialPort_RXCHAR）


#include "stdio.h"
#include "stdafx.h"
#include<windows.h>

//自定义串口状态结构体，包含 portNr串口号，bytesRead读取到的字节数
struct serialPortInfo
{
	UINT portNr;//串口号
	DWORD bytesRead;//读取的字节数
};


class CSerialPort
{
public:
	// contruction and destruction
	int GetPortNO();
	CSerialPort();
	virtual        ~CSerialPort();

	// port initialisation        
	// UINT stopsbits = ONESTOPBIT   stop is index 0 = 1 1=1.5 2=2 
	// 切记：stopsbits = 1，不是停止位为1。
	// by itas109 20160506
	BOOL        InitPort(HWND pPortOwner, UINT portnr = 1, UINT baud = 9600,
		TCHAR parity = _T('N'), UINT databits = 8, UINT stopsbits = ONESTOPBIT,
		DWORD dwCommEvents = EV_RXCHAR | EV_CTS, UINT nBufferSize = 512,

		DWORD ReadIntervalTimeout = 1000,
		DWORD ReadTotalTimeoutMultiplier = 1000,
		DWORD ReadTotalTimeoutConstant = 1000,
		DWORD WriteTotalTimeoutMultiplier = 1000,
		DWORD WriteTotalTimeoutConstant = 1000);

	// start/stop comm watching
	//控制串口监视线程
	BOOL         StartMonitoring();//开始监听
	BOOL         ResumeMonitoring();//恢复监听
	BOOL         SuspendMonitoring();//挂起监听
	BOOL         IsThreadSuspend(HANDLE hThread);//判断线程是否挂起，hThread为要判断的线程

	DWORD         GetWriteBufferSize();//获取写缓冲大小
	DWORD         GetCommEvents();//获取事件
	DCB             GetDCB();//获取DCB

 //写数据到串口
	void        WriteToPort(char* string, size_t n);	// 
	void        WriteToPort(BYTE* Buffer, size_t n);	// 
	void		ClosetoPort();						// 
	BOOL        IsOpen();

	void QueryKey(HKEY hKey);///查询注册表的串口号，将值存于数组中

#ifdef _AFX
	void Hkey2ComboBox(CComboBox& m_PortNO);			//将QueryKey查询到的串口号添加到CComboBox控件中
#endif // _AFX

protected:
	// protected memberfunctions///
	void        ProcessErrorMessage(TCHAR* ErrorText);	//错误处理
	static DWORD WINAPI CommThread(LPVOID pParam);		//线程函数
	static void    ReceiveChar(CSerialPort* port);
	static void ReceiveStr(CSerialPort* port);			//
	static void    WriteChar(CSerialPort* port);

	//thread

	HANDLE              m_Thread;					//监视线程句柄
	BOOL                m_bIsSuspened;				//thread监视线程是否挂起

	//synchronisation objects///
	CRITICAL_SECTION    m_csCommunicationSync;		//临界资源
	BOOL                m_bThreadAlive;				//监视线程运行标志，1线程运行，0线程挂起

/// 定义handles///
	HANDLE              m_hShutdownEvent;			//关闭事件响应 句柄
	HANDLE              m_hComm;					//绑定的串口句柄 
	HANDLE              m_hWriteEvent;				//写事件句柄，如果利用SetEvent(m_hWriteEvent)调用，则表示开始串口写入。
	HANDLE				m_hEventArray[3];			//设定事件句柄数组，用于定义优先级

///定义结构体/
	OVERLAPPED          m_ov;						//声明异步I/O结构体OVERLAPPED，之后利用m_ov调用底层异步处理程序
	/*OVERLAPPED结构体详解
	typedef struct _OVERLAPPED {
		DWORD Internal;			//预留给操作系统使用。它指定一个独立于系统的状态,当GetOverlappedResult函数返回时没有设置扩展错误信息ERROR_IO_PENDING时有效。
		DWORD InternalHigh;		//预留给操作系统使用。它指定长度的数据转移,当GetOverlappedResult函数返回TRUE时有效。
		DWORD Offset;			//该文件的位置是从文件起始处的字节偏移量。调用进程设置这个成员之前调用ReadFile或WriteFile函数。当读取或写入命名管道和通信设备时这个成员被忽略设为零。
		DWORD OffsetHigh;		//指定文件传送的字节偏移量的高位字。当读取或写入命名管道和通信设备时这个成员被忽略设为零。
		HANDLE hEvent;			//在转移完成时该事件设置为有信号状态。调用进程集这个成员在调用ReadFile、 WriteFile、TransactNamedPipe、 ConnectNamedPipe函数之前。
	} OVERLAPPED*/
	COMMTIMEOUTS        m_SerialPortTimeouts;		//超时设置
	/*COMMTIMEOUTS 结构体详解
		typedef struct _COMMTIMEOUTS {
		DWORD ReadIntervalTimeout;			//两字符之间最大的延时，当读取串口数据时，一旦两个字符传输的时间差超过该时间，读取函数将返回现有的数据。
		DWORD ReadTotalTimeoutMultiplier;	//读取每字符间的超时。 指定以毫秒为单位的累积值。用于计算读操作时的超时总数。对于每次读操作，该值与所要读的字节数相乘。
		DWORD ReadTotalTimeoutConstant;		//一次读取串口数据的固定超时。
		DWORD WriteTotalTimeoutMultiplier;	//写入每字符间的超时
		DWORD WriteTotalTimeoutConstant;	//一次写入串口数据的固定超时。
	} COMMTIMEOUTS, *LPCOMMTIMEOUTS;*/

	DCB                 m_dcb;						//设备控制块。串口通信基本设置结构体
	/*DCB结构体定义
	typedef struct _DCB {
		DWORD DCBlength;				//DCB结构大小，即sizeof(DCB)，在调用SetCommState来更新DCB前必须作设置
		DWORD BaudRate;					//指定当前采用的波特率，应与所连接的通讯设备相匹配
		DWORD fBinary : 1;				//指定是否允许二进制模式。Win32 API不支持非二进制模式传输，应设置为true
		DWORD fParity : 1;				//指定奇偶校验是否允许，在为true时具体采用何种校验看Parity 设置
		DWORD fOutxCtsFlow : 1;			//是否监控CTS(clear-to-send)信号来做输出流控。当设置为true时： 若CTS为低电平，则数据发送将被挂起，直至CTS变为高。
		DWORD fOutxDsrFlow : 1;			//
		DWORD fDtrControl : 2;			//
		DWORD fDsrSensitivity : 1;		//
		DWORD fTXContinueOnXoff : 1;	//
		DWORD fOutX : 1;				// XON/XOFF 流量控制在发送时是否可用。
		DWORD fInX : 1;					//XON/XOFF 流量控制在接收时是否可用。
		DWORD fErrorChar : 1;			//该值为TRUE，则用ErrorChar指定的字符代替奇偶校验错误的接收字符
		DWORD fNull : 1;				//为TRUE时，接收时自动去掉空（0值）字节
		DWORD fRtsControl : 2;			//
		DWORD fAbortOnError : 1;		//读写操作发生错误时是否取消操作。若设置为true，则当发生读写错误时，将取消所有读写操作
		DWORD fDummy2 : 17;				//
		WORD wReserved;					//未启用，必须设置为0
		WORD XonLim;					//在XON字符发送前接收缓冲区内可允许的最小字节数
		WORD XoffLim;					//
		BYTE ByteSize;					//
		BYTE Parity;					//指定端口数据传输的校验方法。
		BYTE StopBits;					//指定端口当前使用的停止位数，可取值
		char XonChar;					//指定XON字符
		char XoffChar;					//指定XOFF字符
		char ErrorChar;					//指定ErrorChar字符（代替接收到的奇偶校验发生错误时的字节）
		char EofChar;					//指定用于标示数据结束的字符
		char EvtChar;					//
		WORD wReserved1;				//保留，未启用
	} DCB;*/


	// owner window//

	HWND                m_pOwner;				//串口绑定的窗口句柄

	// misc
	UINT                m_nPortNr;				//串口号
	PBYTE               m_szWriteBuffer;		//写缓冲区指针。
	DWORD               m_dwCommEvents;			//定义串口事件
	DWORD               m_nWriteBufferSize;		//写缓冲大小
	size_t              m_nWriteSize;			//写入字节数
};

#endif __SERIALPORT_H__


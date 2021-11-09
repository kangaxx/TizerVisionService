#include "stdafx.h"
#include "CSerialPort.h"
#include <assert.h>

int m_nComArray[20];//存放活跃的串口号

int CSerialPort::GetPortNO()
{
	return m_nPortNr;
}

//定义结构体
CSerialPort::CSerialPort()	//初始化设置
{
	m_hComm = NULL;			//初始化串口句柄

	// initialize overlapped structure members to zero PS：overlapped为串口通信的重叠模式，在createfile内为 FILE_FLAG_OVERLAPPED，区别分于同步模式的NULL
	///初始化异步结构体
	m_ov.Offset = 0;			//初始化传输文件的位置
	m_ov.OffsetHigh = 0;		//文件起始处的字节偏移量的高字位，PS：初始化指定传输文件的高度

	// create events
	m_ov.hEvent = NULL;			//初始化 传送完成时的事件响应函数，一般在Readfile(),Writefile()之前。
	m_hWriteEvent = NULL;		//初始化写串口事件
	m_hShutdownEvent = NULL;

	m_szWriteBuffer = NULL;

	m_bThreadAlive = FALSE;

	m_nWriteSize = 1;
	m_bIsSuspened = FALSE;
}


//SerialPort析构函数用于Delete dynamic memory
CSerialPort::~CSerialPort()
{
	MSG message;						//传递MSG结构体，建立消息对象，类似于线程消息队列
	/*MSG结构体详解
	背景：windows是通过监视各种输入设备，把发生的事件转化为消息的，并将消息保存在消息队列中。最后当前的应用程序从自己的消息队列中按顺序检索消息，并把每一个消息发送到所对应的窗口消息处理函数中去
	typedef struct tagMSG{
			HWND hwnd；		//消息所指向的窗口的句柄。
			UINT message；	//消息标识符
			WPARAM wparam；	//32位的“消息参数”，该参数的含义和取值取决于具体的消息
			LPARAM lparam；	//另外一个32位的“消息参数”，该参数的含义和取值同样取决于具体的消息
			DWORD time；	//消息进入消息队列的时间
			POINT pt；		//消息进入消息中的鼠标指针的位置坐标。
		}MSG,*PMSG;
	*/

	//增加线程挂起判断，解决由于线程挂起导致串口关闭死锁的问题
	if (IsThreadSuspend(m_Thread))
	{
		ResumeThread(m_Thread);
	}

	//若串口句柄无效,释放句柄
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);			//返回值Long型，!0 表示成功，零表示失败。会设置GetLastError
		m_hComm = NULL;
		return;
	}

	do												//do{} while{}循环
	{
		SetEvent(m_hShutdownEvent);					//将 关闭事件 变为有信号状态，防止死锁
		if (::PeekMessage(&message, m_pOwner, 0, 0, PM_REMOVE))	//PeekMessage用于读响应消息（消息队列地址的指针，窗口指向:该窗口消息被检索，消息下界，消息上界：NULL>检索所有消息，检索后如何处理:PM_REMOVE>处理后清除消息PM_NOREMOVE>处理后不清除消息）
		{
			::TranslateMessage(&message);	//将串口消息转换为字符消息，并存到MSG消息对象中（利用指针形式传递）。					//
											//若消息被转换，则返回WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, 或 WM_SYSKEYUP。若未转换，返回0.
			::DispatchMessage(&message);	//该函数调度含有消息的MSG指针指向的消息传递给窗口程序
		}
	} while (m_bThreadAlive);				//如果监视线程状态位为打开，则执行do{}内容

// if the port is still opened: close it 
	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	// Close Handles  
	//关闭句柄
	if (m_hShutdownEvent != NULL)			//关闭ShutdownEvent事件
		CloseHandle(m_hShutdownEvent);
	if (m_ov.hEvent != NULL)				//关闭异步通信信号事件
		CloseHandle(m_ov.hEvent);
	if (m_hWriteEvent != NULL)				//关闭写串口事件
		CloseHandle(m_hWriteEvent);

	//TRACE("Thread ended\n");

	if (m_szWriteBuffer != NULL)			//若写缓冲区不为NULL，则清空
	{
		delete[] m_szWriteBuffer;			//释放写缓冲区
		m_szWriteBuffer = NULL;
	}
}

//
// Initialize the port. This can be port 1 to MaxSerialPortNum.
//初始化串口。只能是1到MaxSerialPortNum

//初始化串口函数
BOOL CSerialPort::InitPort(HWND pPortOwner,    // 绑定串口响应消息MSG对应窗口的句柄，the owner (CWnd) of the port (receives message)
	UINT  portnr,			// 串口号 (1到MaxSerialPortNum)
	UINT  baud,				// 波特率
	TCHAR  parity,			// 校验位，不区分大小写 parity n=none,e=even,o=odd,m=mark,s=space
	UINT  databits,			// 数据位 databits 5,6,7,8
	UINT  stopbits,			// 停止位 stopbits 1,1.5,2
	DWORD dwCommEvents,		//串口响应事件，有EV_RXCHAR，EV_CTS， EV_DSR ，EV_RING等等。
							// EV_RXCHAR：设备返回数据时，第一个字符达到缓冲区时触发
	UINT  writebuffersize,	// size of the writebuffer

	DWORD   ReadIntervalTimeout, //读间隔超时
	DWORD   ReadTotalTimeoutMultiplier,//读时间系数
	DWORD   ReadTotalTimeoutConstant,//读时间常量
	DWORD   WriteTotalTimeoutMultiplier,//写时间系数
	DWORD   WriteTotalTimeoutConstant)//写时间常量

{
	assert(portnr > 0 && portnr < MaxSerialPortNum);		//断言：串口号必须是大于0小于MaxSerialPortNum
	assert(pPortOwner != NULL);								//断言：必须要为串口响应消息绑定一个窗口句柄

	MSG message;				//见前文

//增加线程挂起判断，解决由于线程挂起导致串口关闭死锁的问题
	if (IsThreadSuspend(m_Thread))				//若挂起
	{
		ResumeThread(m_Thread);					//恢复线程
	}

	//若线程运行，则挂起
	// if the thread is alive: Kill
	if (m_bThreadAlive)
	{
		do
		{
			SetEvent(m_hShutdownEvent);			//将 关闭事件 变为有信号状态
			//防止死锁，同上
			if (::PeekMessage(&message, m_pOwner, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		} while (m_bThreadAlive);
		//TRACE("Thread ended\n");
		Sleep(50);//此处的延时很重要，因为如果串口开着，发送关闭指令到彻底关闭需要一定的时间，这个延时应该跟电脑的性能相关
	}

	// 重置 events
	if (m_ov.hEvent != NULL)
		ResetEvent(m_ov.hEvent);								//将异步通信Event变为无信号状态
	else														//change by COMTOOL
		m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		//CreateEvent(安全属性:NULL，复位方式:TRUE>用ResetEvent复位FALSE被线程释放后自动复位，初始状态:TRUE>有信号FALSE》无信号，指定名字:NULL>无名事件对象)

	if (m_hWriteEvent != NULL)
		ResetEvent(m_hWriteEvent);								//清除WriteEvent
	else														//change by COMTOOL
		m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hShutdownEvent != NULL)
		ResetEvent(m_hShutdownEvent);							//清除ShutdownEvent
	else														//change by COMTOOL
		m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// initialize the event objects
	//事件数组初始化，设定优先级别
	m_hEventArray[0] = m_hShutdownEvent;    // highest priority
											//为避免有些串口设备无数据输入，但一直返回读事件，使监听线程阻塞，
											//可以将读写放在两个线程中，或者修改读写事件优先级
											//修改优先级有两个方案：
											//方案一为监听线程中WaitCommEvent()后，添加如下两条语句：
											//if (WAIT_OBJECT_O == WaitForSingleObject(port->m_hWriteEvent, 0))
											//    ResetEvent(port->m_ov.hEvent);
											//方案二为初始化时即修改，即下面两条语句：
	m_hEventArray[1] = m_hWriteEvent;
	m_hEventArray[2] = m_ov.hEvent;
	/*m_hEventArray[1] = m_ov.hEvent;
	m_hEventArray[2] = m_hWriteEvent;*/


	//初始化临界资源 initialize critical section
	/*
	临界区背景：
	单进程的各个线程可以使用临界资源对象来解决同步互斥问题，该对象不能保证哪个线程能够获得到临界资源对象，该系统能公平的对待每一个线程。（百度百科）
	在使用一个临界区对象以前，一些进程中的线程必须调用InitializeCriticalSection函数来初始化对象。一旦一个临界区对象已被初始化，该进程的线程可以在EnterCriticalSection或LeaveCriticalSection函数指定对象，提供对共享资源的相互独占式访问。
	在将临界区传递给 InitializeCriticalSection 时（或者更准确地说，是在传递其地址时），临界区即开始存在。初始化之后，代码即将临界区传递给 EnterCriticalSection 和 LeaveCriticalSection API。一个线程自 EnterCriticalSection 中返回后，所有其他调用 EnterCriticalSection 的线程都将被阻止，直到第一个线程调用 LeaveCriticalSection 为止。
	*/
	InitializeCriticalSection(&m_csCommunicationSync);

	// set buffersize for writing and save the owner
	//为读写缓冲区绑定窗口函数
	m_pOwner = pPortOwner;

	if (m_szWriteBuffer != NULL)					//释放写缓冲区
	{
		delete[] m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
	m_szWriteBuffer = new BYTE[writebuffersize];	//新建写缓冲区

	m_nPortNr = portnr;								//定义串口号

	m_nWriteBufferSize = writebuffersize;			//定义写缓冲区大小
	m_dwCommEvents = dwCommEvents;					//定义串口响应事件，例如EV_RXCHAR，EV_CTS， EV_DSR ，EV_RING等等。

	BOOL bResult = FALSE;							//
	TCHAR* szPort = new TCHAR[MAX_PATH];			//该数组用于存放“COM？”
	TCHAR* szBaud = new TCHAR[MAX_PATH];			//该数组用于BuildCommDCB函数，存放"baud="<bau>d parity=<parity> data=<databits> stop=<mystop>")


	// now it critical! 开始使用临界区
	EnterCriticalSection(&m_csCommunicationSync);			///临界区进入起点

// if the port is already opened: close it
//串口已打开就关掉
	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	// prepare port strings
	//将UINT的整数型串口号转换为string形式的"COM?"形式
	_stprintf_s(szPort, MAX_PATH, _T("\\\\.\\COM%d"), portnr);		//（\\\\.\\COM）可以显示COM10以上端口

	int mystop;			// 停止位：0 = 1 1=1.5 2=2。注意1不等于1.
	switch (stopbits)
	{
	case 0:
		mystop = ONESTOPBIT;
		break;
	case 1:
		mystop = ONE5STOPBITS;
		break;
	case 2:
		mystop = TWOSTOPBITS;
		break;
		//增加默认情况，因为stopbits=1.5时，SetCommState会报错。
		//一般的电脑串口不支持1.5停止位，这个1.5停止位似乎用在红外传输上的。
	default:
		mystop = ONESTOPBIT;
		break;
	}
	int myparity;		//校验位
	myparity = 0;
	parity = _totupper(parity);		//小写字母转换为大写字母
	switch (parity)
	{
	case _T('N'):
		myparity = 0;
		break;
	case _T('O'):
		myparity = 1;
		break;
	case _T('E'):
		myparity = 2;
		break;
	case _T('M'):
		myparity = 3;
		break;
	case _T('S'):
		myparity = 4;
		break;
		//增加默认情况。
	default:
		myparity = 0;
		break;
	}
	_stprintf_s(szBaud, MAX_PATH, _T("baud=%d parity=%c data=%d stop=%d"), baud, parity, databits, mystop);

	// get a handle to the port

	m_hComm = CreateFile(szPort,                        // 打开串口COMX，并指定串口句柄m_hComm
		GENERIC_READ | GENERIC_WRITE,    // 可读可写 read/write types
		0,                                // 独占方式，串口不支持共享 comm devices must be opened with exclusive access
		NULL,                            // 安全属性指针，默认值为NULL no security attributes
		OPEN_EXISTING,                    // 打开现有的串口文件 comm devices must use OPEN_EXISTING
		FILE_FLAG_OVERLAPPED,            // 0：同步方式，FILE_FLAG_OVERLAPPED：异步方式
		0);                           //用于复制文件句柄，默认值为NULL/0，对串口而言该参数必须置为NULL

//创建失败
	if (m_hComm == INVALID_HANDLE_VALUE)			//如果打开串口失败
	{

		//串口打开失败，增加提示信息
		switch (GetLastError())						//获取串口代码
		{
			//串口不存在
		case ERROR_FILE_NOT_FOUND:
		{
			TCHAR Temp[200] = { 0 };
			_stprintf_s(Temp, 200, _T("COM%d ERROR_FILE_NOT_FOUND,Error Code:%d"), portnr, GetLastError());
			MessageBox(NULL, Temp, _T("COM InitPort Error"), MB_ICONERROR);
			break;
		}
		//串口拒绝访问
		case ERROR_ACCESS_DENIED:
		{
			TCHAR Temp[200] = { 0 };
			_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED,Error Code:%d"), portnr, GetLastError());
			MessageBox(NULL, Temp, _T("COM InitPort Error"), MB_ICONERROR);
			break;
		}
		default:
			break;
		}
		// port not found
		delete[] szPort;				//释放串口号定义内存
		delete[] szBaud;				//释放波特率定义内存

		return FALSE;
	}

	// set the timeout values
	///设置超时
	m_SerialPortTimeouts.ReadIntervalTimeout = ReadIntervalTimeout * 1000;
	m_SerialPortTimeouts.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier * 1000;
	m_SerialPortTimeouts.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant * 1000;
	m_SerialPortTimeouts.WriteTotalTimeoutMultiplier = WriteTotalTimeoutMultiplier * 1000;
	m_SerialPortTimeouts.WriteTotalTimeoutConstant = WriteTotalTimeoutConstant * 1000;

	// configure
	///配置
	///分别调用Windows API设置串口参数
	if (SetCommTimeouts(m_hComm, &m_SerialPortTimeouts))//设置超时
		/*SetCommTimeouts函数的第一个参数hFile是由CreateFile函数返回指向已打开串行口的句柄。
			第二个参数指向拥有超时数据的控制块COMMTIMEOUTS。
				如果函数调用成功，则返回值为非0；若函数调用失败，则返回值为0。*/
	{

		if (SetCommMask(m_hComm, dwCommEvents))//SetCommMask用于设定需要接收通知的事件并将结果放入dwCommEvents，
		{

			if (GetCommState(m_hComm, &m_dcb))//获取当前DCB参数。GetCommState函数的第一个参数hFile是由CreateFile函数返回指向已打开串行口的句柄。第二个参数指向设备控制块DCB。如果函数调用成功，则返回值为非0；若函数调用失败，则返回值为0。
			{
				m_dcb.EvtChar = 'q';							//当接收到此字符时,会产生一个EV_RXFLAG事件，如果用SetCommMask函数中指定了EV_RXFLAG , 则可用WaitCommEvent 来监测该事件
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE;			// RTS(request-to-send)流控制设置。RTS_CONTROL_ENABLE为打开设备时置RTS信号为高电平，应用程序可通过调用 
				m_dcb.BaudRate = baud;							//指定当前采用的波特率，应与所连接的通讯设备相匹配 
				m_dcb.Parity = myparity;						//指定端口数据传输的校验方法
				m_dcb.ByteSize = databits;						//一个字节大小
				m_dcb.StopBits = mystop;						//停止位设置

				//if (BuildCommDCB(szBaud &m_dcb))///填写DCB结构，和SetCommState类似。填写形式为BuildCommDCB(〝COM1:2400,n,8,1〞,&dcb);
				//{
				if (SetCommState(m_hComm, &m_dcb))//配置该串口绑定串口的DCB
					; /*SetCommState，函数的第一个参数hFile是由CreateFile函数返回指向已打开串行口的句柄。
							第二个参数指向设备控制块DCB。如果函数调用成功，则返回值为非0；
									若函数调用失败，则返回值为0。*/
				else
					ProcessErrorMessage((TCHAR*)("SetCommState()"));
				//}
				//else
				//    ProcessErrorMessage("BuildCommDCB()");
			}
			else
				ProcessErrorMessage((TCHAR*)("GetCommState()"));
		}
		else
			ProcessErrorMessage((TCHAR*)("SetCommMask()"));
	}
	else
		ProcessErrorMessage((TCHAR*)("SetCommTimeouts()"));

	delete[] szPort;
	delete[] szBaud;

	// flush the port
	//终止读写并清空接收和发送
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	/*PurgeComm(HANDLE hFile,DWORD dwFlags )
		参数dwFlags指定要完成的操作，可以是下列值的组合：
		PURGE_TXABORT 终止所有正在进行的字符输出操作, 完成一个正处于等待状态的重叠i / o操作, 他将产生一个事件, 指明完成了写操作
		PURGE_RXABORT 终止所有正在进行的字符输入操作, 完成一个正在进行中的重叠i / o操作, 并带有已设置得适当事件
		PURGE_TXCLEAR 这个命令指导设备驱动程序清除输出缓冲区，经常与PURGE_TXABORT 命令标志一起使用
		PURGE_RXCLEAR 这个命令用于设备驱动程序清除输入缓冲区，经常与PURGE_RXABORT 命令标志一起使用*/

		// release critical section
		//释放临界资源
	LeaveCriticalSection(&m_csCommunicationSync);//解锁 到EnterCriticalSection之间代码资源已经释放了，其他线程可以进行操作   

	//TRACE("Initialisation for communicationport %d completed.\nUse Startmonitor to communicate.\n", portnr);

	return TRUE;
}

//监视线程的大致流程：
//检查串口-->进入循环{WaitCommEvent(不阻塞询问)询问事件-->如果有事件来到-->到相应处理(关闭\读\写)}
//监视线程函数：The CommThread Function
DWORD WINAPI CSerialPort::CommThread(LPVOID pParam)
{
	// Cast the void pointer passed to the thread back to a pointer of CSerialPort class
	CSerialPort* port = (CSerialPort*)pParam;

	// Set the status variable in the dialog class。
	// TRUE表示线程正在运行，TRUE to indicate the thread is running.
	port->m_bThreadAlive = TRUE;

	// Misc. variables
	DWORD BytesTransfered = 0;
	DWORD Event = 0;					//返回事件定义编号：0-关闭事件，1-发送数据，2-串口响应事件
	DWORD CommEvent = 0;				//串口事件标识符
	DWORD dwError = 0;					//获取失败状态函数，dwError = GetLastError()
	COMSTAT comstat;					//定义串口状态结构体

	BOOL  bResult = TRUE;				//通信事件出现与否判断标志位。

	// Clear comm buffers at startup
	///开始时清除串口缓冲
	if (port->m_hComm)        // 如果串口打开，check if the port is opened
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);		//清除串口，同上

	// begin forever loop.  This loop will run as long as the thread is alive.
	///只要线程存在就不断读取数据
	for (;;)				//开始死循环
	{
		// Make a call to WaitCommEvent().  This call will return immediatly
		// because our port was created as an async port (FILE_FLAG_OVERLAPPED
		// and an m_OverlappedStructerlapped structure specified).  This call will cause the 
		// m_OverlappedStructerlapped element m_OverlappedStruct.hEvent, which is part of the m_hEventArray to 
		// be placed in a non-signeled state if there are no bytes available to be read,
		// or to a signeled state if there are bytes available.  If this event handle 
		// is set to the non-signeled state, it will be set to signeled when a 
		// character arrives at the port.

		// we do this for each port!

				/*
						WaitCommEvent函数第3个参数1pOverlapped可以是一个OVERLAPPED结构的变量指针
								，也可以是NULL，当用NULL时，表示该函数是同步的，否则表示该函数是异步的。
										调用WaitCommEvent时，如果异步操作不能立即完成，会立即返回FALSE，系统在
												WaitCommEvent返回前将OVERLAPPED结构成员hEvent设为无信号状态，等到产生通信
														事件时，系统将其置有信号
																*/

		bResult = WaitCommEvent(port->m_hComm, &Event, &port->m_ov);//表示该函数是异步的
		/*BOOL WaitCommEvent								//判断用SetCommMask()函数设置的串口通信事件是否已发生。
			(HANDLE hFile,
			LPDWORD lpEvtMask,								//如果发生错误，pEvtMask指向0，否则指向某一事件
			LPOVERLAPPED lpOverlapped						//异步结构，用来保存异步操作结果。
		);*/
		if (!bResult)										//若WaitCommEvent失败
		{
			// If WaitCommEvent() returns FALSE, process the last error to determin the reason.
			//如果WaitCommEvent返回Error为FALSE，则查询错误信息
			switch (dwError = GetLastError())
			{
			case ERROR_IO_PENDING:     //正常情况，没有字符可读，操作悬挂 erroe code:997
			{
				// This is a normal return value if there are no bytes
				// to read at the port.
				// Do nothing and continue
				break;
			}
			case ERROR_INVALID_PARAMETER://系统错误 erroe code:87
			{
				// Under Windows NT, this value is returned for some reason.
				// I have not investigated why, but it is also a valid reply
				// Also do nothing and continue.
				break;
			}
			case ERROR_ACCESS_DENIED://拒绝访问 erroe code:5
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				TCHAR Temp[200] = { 0 };
				_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED，WaitCommEvent() Error Code:%d"), port->m_nPortNr, GetLastError());
				MessageBox(NULL, Temp, _T("COM WaitCommEvent Error"), MB_ICONERROR);
				break;
			}
			case ERROR_INVALID_HANDLE://打开串口失败 erroe code:6
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				break;
			}
			case ERROR_BAD_COMMAND://连接过程中非法断开 erroe code:22
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				TCHAR Temp[200] = { 0 };
				_stprintf_s(Temp, 200, _T("COM%d ERROR_BAD_COMMAND，WaitCommEvent() Error Code:%d"), port->m_nPortNr, GetLastError());
				MessageBox(NULL, Temp, _T("COM WaitCommEvent Error"), MB_ICONERROR);
				break;
			}
			default://发生其他错误，其中有串口读写中断开串口连接的错误（错误22）
			{
				// All other error codes indicate a serious error has
				port->m_hComm = INVALID_HANDLE_VALUE;					//发生错误时，将串口句柄置为无效句柄
				// occured.  Process this error.
				port->ProcessErrorMessage((TCHAR*)("WaitCommEvent()"));
				break;
			}
			}
		}
		else    //串口事件发生，WaitCommEvent()能正确返回
		{
			// If WaitCommEvent() returns TRUE, check to be sure there are
			// actually bytes in the buffer to read.  
			// If you are reading more than one byte at a time from the buffer 
			// (which this program does not do) you will have the situation occur 
			// where the first byte to arrive will cause the WaitForMultipleObjects() 
			// function to stop waiting.  The WaitForMultipleObjects() function 
			// resets the event handle in m_OverlappedStruct.hEvent to the non-signelead state
			// as it returns.  
			//
			// If in the time between the reset of this event and the call to 
			// ReadFile() more bytes arrive, the m_OverlappedStruct.hEvent handle will be set again
			// to the signeled state. When the call to ReadFile() occurs, it will 
			// read all of the bytes from the buffer, and the program will
			// loop back around to WaitCommEvent().
			// 
			// At this point you will be in the situation where m_OverlappedStruct.hEvent is set,
			// but there are no bytes available to read.  If you proceed and call
			// ReadFile(), it will return immediatly due to the async port setup, but
			// GetOverlappedResults() will not return until the next character arrives.
			//
			// It is not desirable for the GetOverlappedResults() function to be in 
			// this state.  The thread shutdown event (event 0) and the WriteFile()
			// event (Event2) will not work if the thread is blocked by GetOverlappedResults().
			//
			// The solution to this is to check the buffer with a call to ClearCommError().
			// This call will reset the event handle, and if there are no bytes to read
			// we can loop back through WaitCommEvent() again, then proceed.
			// If there are really bytes to read, do nothing and proceed.

			bResult = ClearCommError(port->m_hComm, &dwError, &comstat);		//清除串口错误事件
			/*BOOL ClearCommError(		//此函数清除硬件的通讯错误以及获取通讯设备的当前状态
				HANDLE hFile,			//由CreateFile函数返回指向已打开串行口的句柄
				LPDWORD lpErrors,		//错误类型
				LPCOMSTAT lpStat		//返回设备状态的控制块COMSTAT
			);*/

			if (comstat.cbInQue == 0)		//comstat.cbInQue输入缓冲区中的字节数,cbOutQue输出缓冲区中的字节数
				continue;
		}    // end if bResult

			 ///主等待函数，会阻塞线程
			 // Main wait function.  This function will normally block the thread
			 // until one of nine events occur that require action.
			 ///等待3个事件：关断/读/写，有一个事件发生就返回
		Event = WaitForMultipleObjects(3, //3个事件如果函数成功，返回值表示该事件导致该函数返回。
			port->m_hEventArray, //事件数组
			FALSE, //有一个事件发生就返回
			INFINITE);//超时时间间隔超时间隔，以毫秒为单位。
						/*如果指定了非零值，则该函数将一直等到指定的对象发出信号或经过间隔。
							如果dwMilliseconds为零，则如果未发出指示对象，则该函数不会进入等待状态;它总是立即返回。
								如果dwMilliseconds是INFINITE，则仅在发出指定对象信号时才返回该函数。*/

		switch (Event)
		{
		case 0:
		{
			// Shutdown event.  This is event zero so it will be
			// the higest priority and be serviced first.
			///关断事件，关闭串口
			CloseHandle(port->m_hComm);
			port->m_hComm = NULL;
			port->m_bThreadAlive = FALSE;

			// Kill this thread.  break is not needed, but makes me feel better.
			//AfxEndThread(100);				//COMTOOL change
//			::ExitThread(100);				//COMTOOL change

			break;
		}
		case 1: // write event 发送数据
		{
			// Write character event from port
			WriteChar(port);					//写字符串
			break;
		}
		case 2:    // read event 将定义的各种消息发送出去
		{
			GetCommMask(port->m_hComm, &CommEvent);			//获取串口事件
			if (CommEvent & EV_RXCHAR) //接收到字符，并置于输入缓冲区中
			{
				if (IsReceiveString == 1)
				{
					ReceiveStr(port);//多字符接收
				}
				else if (IsReceiveString == 0)
				{
					ReceiveChar(port);//单字符接收
				}
				else
				{
					//默认多字符接收
					ReceiveStr(port);//多字符接收
				}
			}

			if (CommEvent & EV_CTS) //CTS信号状态发生变化
				::SendMessage(port->m_pOwner, Wm_SerialPort_CTS_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			if (CommEvent & EV_RXFLAG) //接收到事件字符，并置于输入缓冲区中 
				::SendMessage(port->m_pOwner, Wm_SerialPort_RXFLAG_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			if (CommEvent & EV_BREAK)  //输入中发生中断
				::SendMessage(port->m_pOwner, Wm_SerialPort_BREAK_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			if (CommEvent & EV_ERR) //发生线路状态错误，线路状态错误包括CE_FRAME,CE_OVERRUN和CE_RXPARITY 
				::SendMessage(port->m_pOwner, Wm_SerialPort_ERR_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			if (CommEvent & EV_RING) //检测到振铃指示
				::SendMessage(port->m_pOwner, Wm_SerialPort_RING_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			break;
		}
		default:
		{
			MessageBox(NULL, _T("Receive Error!"), _T("COM Receive Error"), MB_ICONERROR);
			break;
		}

		} // end switch

	} // close forever loop

	return 0;
}

//
// start comm watching

//开启监视线程
BOOL CSerialPort::StartMonitoring()
{
	//if (!(m_Thread = AfxBeginThread(CommThread, this)))
	if (!(m_Thread = ::CreateThread(NULL, 0, CommThread, this, 0, NULL)))
		/*CreateThread 函数定义
		CreateThread(												//函数成功，返回线程句柄；函数失败返回false
			_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,		//线程安全属性.指向SECURITY_ATTRIBUTES型态的结构的指针,NULL使用默认安全性
			_In_ SIZE_T dwStackSize,								//堆栈大小.设置初始栈的大小，以字节为单位，如果为0，那么默认将使用与调用该函数的线程相同的栈空间大小。
			_In_ LPTHREAD_START_ROUTINE lpStartAddress,				//线程函数.指向线程函数的指针
			_In_opt_ __drv_aliasesMem LPVOID lpParameter,			//线程参数.向线程函数传递的参数，是一个指向结构的指针，不需传递参数时，为NULL。
			_In_ DWORD dwCreationFlags,								//线程创建属性,0-表示创建后立即激活
			_Out_opt_ LPDWORD lpThreadId							//线程的id
		);*/
		return FALSE;
	//TRACE("Thread started\n");
	return TRUE;
}

// Restart the comm thread
//从挂起恢复监视线程
BOOL CSerialPort::ResumeMonitoring()
{
	//TRACE("Thread resumed\n");
	//m_Thread->ResumeThread();
	::ResumeThread(m_Thread);
	return TRUE;
}

//
// Suspend the comm thread
//挂起监视线程
BOOL CSerialPort::SuspendMonitoring()
{
	//TRACE("Thread suspended\n");
	//m_Thread->SuspendThread();
	::SuspendThread(m_Thread);						//SuspendThread用于挂起线程如果返回-1(0xFFFFFFFF)，表示失败；如果返回整数，表示线程已经被挂起过的次数。
	return TRUE;
}

//判断线程是否挂起函数，如果挂起为1，否则为0. 
BOOL CSerialPort::IsThreadSuspend(HANDLE hThread)
{
	DWORD   count = SuspendThread(hThread);			//SuspendThread用于挂起线程，如果返回-1(0xFFFFFFFF)，表示失败；如果返回整数，表示线程已经被挂起过的次数。
	if (count == -1)
	{
		return FALSE;
	}
	ResumeThread(hThread);							//ResumeThread可以恢复被SuspendThread挂起的线程的执行。
	return (count != 0);							//检查count和0是否不等，不等为1，相等为0.
}

//
// If there is a error, give the right message
//提示函数：如果有错误，给出具体提示信息：ErrorText
void CSerialPort::ProcessErrorMessage(TCHAR* ErrorText)
{
	TCHAR Temp[200] = { 0 };

	LPVOID lpMsgBuf;
	/*FormatMessageW函数解析			//这个函数是用来格式化消息字符串，就是处理消息资源的。
	FormatMessageW(
		_In_     DWORD dwFlags,			//格式化选项，对lpSource参数值有指导作用。
		_In_opt_ LPCVOID lpSource,		//消息表资源来自哪里，这个值依靠dwFlags。
		_In_     DWORD dwMessageId,		//所需格式化消息的标识符。
		_In_     DWORD dwLanguageId,	//格式化消息语言标识符。
		_Out_    LPWSTR lpBuffer,		//一个缓冲区指针来接受格式化后的消息。
		_In_     DWORD nSize,			//指定了输出缓冲区的消息，以TCHARs为单位。
		_In_opt_ va_list *Arguments		//一个数组中的值在格式化消息中作为插入值。
	);*/
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,							//这个值是消息表资源来自哪里，这个值依靠dwFlags,
		GetLastError(),					//所需格式化消息的标识符。
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// Default language
		(LPTSTR)&lpMsgBuf,							//一个缓冲区指针来接受格式化后的消息。
		0,											//这个参数指定了输出缓冲区的消息，以TCHARs为单位
		NULL										//一个数组中的值在格式化消息中作为插入值
	);

	_stprintf_s(Temp, 200, _T("WARNING:  %s Failed with the following error: \n%s\nPort: %d\n"), ErrorText, (TCHAR*)lpMsgBuf, m_nPortNr);
	MessageBox(NULL, Temp, _T("Application Error"), MB_ICONSTOP);

	LocalFree(lpMsgBuf);				//由于lpBuffer这个参数的值是FormatMessage函数动态分配的缓冲区，所以在不使用的时候要LocalFree.
}

//
// Write a character.
//写一个字符串
void CSerialPort::WriteChar(CSerialPort* port)
{
	BOOL bWrite = TRUE;						//写入状态位，1-可以写入，0-无法写入
	BOOL bResult = TRUE;					//串口通讯成功与否标志位。1-成功，0-失败
	DWORD BytesSent = 0;					//写入字节数
	DWORD SendLen = port->m_nWriteSize;							//写入数据长度
	ResetEvent(port->m_hWriteEvent);							//设为无信号状态


	// Gain ownership of the critical section
	EnterCriticalSection(&port->m_csCommunicationSync);			//临界区线程锁，锁定m_csCommunicationSync

	if (bWrite)
	{
		// Initailize variables
		port->m_ov.Offset = 0;				//发送文件起始偏移量	
		port->m_ov.OffsetHigh = 0;			//发送文件高度

		// 终止收发区，功能同上
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

		bResult = WriteFile(port->m_hComm,                            // Handle to COMM Port
			port->m_szWriteBuffer,                    // Pointer to message buffer in calling finction
			SendLen,
			//strlen((char*)port->m_szWriteBuffer),    // Length of message to send
			&BytesSent,                                //实际写入字节数 Where to store the number of bytes sent
			&port->m_ov);                            // Overlapped structure

													 // deal with any error codes
		if (!bResult)
		{
			DWORD dwError = GetLastError();			//获取写入失败原因
			switch (dwError)
			{
			case ERROR_IO_PENDING: //正常情况，没有字符可读，操作悬挂 erroe code:997
			{
				// continue to GetOverlappedResults()
				BytesSent = 0;					//清除实际写入字节数
				bWrite = FALSE;					//写入状态位
				break;
			}
			case ERROR_ACCESS_DENIED://拒绝访问 erroe code:5
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				TCHAR Temp[200] = { 0 };
				_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED，WriteFile() Error Code:%d"), port->m_nPortNr, GetLastError());
				MessageBox(NULL, Temp, _T("COM WriteFile Error"), MB_ICONERROR);
				break;
			}
			case ERROR_INVALID_HANDLE://打开串口失败 erroe code:6
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				break;
			}
			case ERROR_BAD_COMMAND://连接过程中非法断开 erroe code:22
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				TCHAR Temp[200] = { 0 };
				_stprintf_s(Temp, 200, _T("COM%d ERROR_BAD_COMMAND，WriteFile() Error Code:%d"), port->m_nPortNr, GetLastError());
				MessageBox(NULL, Temp, _T("COM WriteFile Error"), MB_ICONERROR);
				break;
			}
			default:
			{
				// all other error codes
				port->ProcessErrorMessage((TCHAR*)("WriteFile()"));
			}
			}
		}
		else
		{
			LeaveCriticalSection(&port->m_csCommunicationSync);//解锁临界区 到EnterCriticalSection之间代码资源已经释放了，其他线程可以进行操作   
		}
	} // end if(bWrite)

//异步IO操作仍在进行，需要调用GetOverlappedResult查询
	if (!bWrite)
	{
		bWrite = TRUE;
		/*	GetOverlappedResult函数定义
		BOOL GetOverlappedResult(				//获取重叠输入输出结果
			HANDLE hFile,						// 串口的句柄，指向重叠操作开始时指定的OVERLAPPED结构
			LPOVERLAPPED lpOverlapped,			//重叠标志位
			LPDWORD lpNumberOfBytesTransferred,	// 指向一个32位变量，该变量的值返回实际读写操作传输的字节数。
			BOOL bWait							//该参数用于指定函数是否一直等到重叠操作结束。
												//如果该参数为TRUE，函数直到操作结束才返回。
												//如果该参数为FALSE，函数直接返回，这时如果操作没有完成，通过调用GetLastError()函数会返回ERROR_IO_INCOMPLETE。
			);*/
		bResult = GetOverlappedResult(port->m_hComm,    // Handle to COMM port 
			&port->m_ov,        // Overlapped structure
			&BytesSent,        // Stores number of bytes sent
			TRUE);             // Wait flag

		LeaveCriticalSection(&port->m_csCommunicationSync);//解锁 到EnterCriticalSection之间代码资源已经释放了，其他线程可以进行操作   

		// 如果写串口错误 
		if (!bResult)
		{
			port->ProcessErrorMessage((TCHAR*)("GetOverlappedResults() in WriteFile()"));
		}
	} // end if (!bWrite)

	  // 验证实际发送数据和理论发送数据是否相等 Verify that the data size send equals what we tried to send
	  //if (BytesSent != SendLen /*strlen((char*)port->m_szWriteBuffer)*/)  
	  //{
	  //TRACE(_T("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n"), BytesSent, _tcsclen((TCHAR*)port->m_szWriteBuffer));
	  //}
}

//
// Character received. Inform the owner
//接收字符
void CSerialPort::ReceiveChar(CSerialPort* port)
{
	BOOL  bRead = TRUE;					//能否读取状态位,1-可读，0-不可读
	BOOL  bResult = TRUE;				//串口通讯成功与否标识符
	DWORD dwError = 0;					//错误代码，=GetlastError()
	DWORD BytesRead = 0;				//实际读取到的字节数
	COMSTAT comstat;					//串口状态结构体
	unsigned char RXBuff;				//接收缓冲区

	for (;;)
	{
		//防止死锁
		if (WaitForSingleObject(port->m_hShutdownEvent, 0) == WAIT_OBJECT_0)
			return;
		/*WaitForSingleObject（HANDLE hHandle,DWORD dwMilliseconds）函数用来检测hHandle事件的信号状态，在某一线程中调用该函数时，线程暂时挂起，
			如果在挂起的dwMilliseconds毫秒内，线程所等待的对象变为有信号状态，则该函数立即返回；
				如果时间已经到达dwMilliseconds毫秒，但hHandle所指向的对象还没有变成有信号状态，函数照样返回*/

				// Gain ownership of the comm port critical section.
				// This process guarantees no other part of this program 
				// is using the port object. 

		EnterCriticalSection(&port->m_csCommunicationSync);//临界区线程锁，锁定m_csCommunicationSync

		// ClearCommError() will update the COMSTAT structure and clear any other errors.
		//更新COMSTAT
		bResult = ClearCommError(port->m_hComm, &dwError, &comstat);
		/*BOOL ClearCommError(		//此函数清除硬件的通讯错误以及获取通讯设备的当前状态
		HANDLE hFile,			//由CreateFile函数返回指向已打开串行口的句柄
		LPDWORD lpErrors,		//错误类型
		LPCOMSTAT lpStat		//返回设备状态的控制块COMSTAT
		);*/

		LeaveCriticalSection(&port->m_csCommunicationSync);//解锁 到EnterCriticalSection之间代码资源已经释放了，其他线程可以进行操作   

		// start forever loop.  I use this type of loop because I
		// do not know at runtime how many loops this will have to
		// run. My solution is to start a forever loop and to
		// break out of it when I have processed all of the
		// data available.  Be careful with this approach and
		// be sure your loop will exit.
		// My reasons for this are not as clear in this sample 
		// as it is in my production code, but I have found this 
		// solutiion to be the most efficient way to do this.

		//所有字符均被读出，中断循环
		if (comstat.cbInQue == 0)		//comstat.cbInQue输入缓冲区中的字节数,cbOutQue输出缓冲区中的字节数
		{
			// break out when all bytes have been read
			break;
		}

		EnterCriticalSection(&port->m_csCommunicationSync);//临界区线程锁，锁定m_csCommunicationSync

		if (bRead)
		{
			//读取串口，读出缓冲区中字节
			bResult = ReadFile(
				port->m_hComm,        // Handle to COMM port 
				&RXBuff,                // RX Buffer Pointer
				1,                    // Read one byte
				&BytesRead,            // Stores number of bytes read
				&port->m_ov);        // pointer to the m_ov structure
									 // deal with the error code 
			/*BOOL ReadFile(					//调用成功,返回非0.调用不成功,返回为0
												//会设置GetLastError。如启动的是一次异步读操作，则函数会返回零值并将ERROR_IO_PENDING设置成GetLastError的结果。
												//如结果不是零值，但读入的字节数小于nNumberOfBytesToRead参数指定的值，表明早已抵达了文件的结尾。
				HANDLE hFile,					//文件的句柄
				LPVOID lpBuffer,				//用于保存读入数据的一个缓冲区
				DWORD nNumberOfBytesToRead,		//要读入的字节数
				LPDWORD lpNumberOfBytesRead,	//指向实际读取字节数的指针
				LPOVERLAPPED lpOverlapped		//异步通信状态标识符
				//如文件打开时指定了FILE_FLAG_OVERLAPPED，那么必须，用这个参数引用一个特殊的结构。
				//该结构定义了一次异步读取操作。否则，应将这个参数设为NULL
			);*/

			//若返回错误，错误处理
			if (!bResult)
			{
				switch (dwError = GetLastError())
				{
				case ERROR_IO_PENDING:	//正常情况，没有字符可读，操作悬挂 erroe code:997
				{
					// asynchronous i/o is still in progress 
					// Proceed on to GetOverlappedResults();
					///异步IO仍在进行
					bRead = FALSE;
					break;
				}
				case ERROR_ACCESS_DENIED://拒绝访问 erroe code:5
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					TCHAR Temp[200] = { 0 };
					_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED，ReadFile() Error Code:%d"), port->m_nPortNr, GetLastError());
					MessageBox(NULL, Temp, _T("COM ReadFile Error"), MB_ICONERROR);
					break;
				}
				case ERROR_INVALID_HANDLE://打开串口失败 erroe code:6
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					break;
				}
				case ERROR_BAD_COMMAND://连接过程中非法断开 erroe code:22
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					TCHAR Temp[200] = { 0 };
					_stprintf_s(Temp, 200, _T("COM%d ERROR_BAD_COMMAND，ReadFile() Error Code:%d"), port->m_nPortNr, GetLastError());
					MessageBox(NULL, Temp, _T("COM ReadFile Error"), MB_ICONERROR);
					break;
				}
				default:
				{
					// Another error has occured.  Process this error.
					port->ProcessErrorMessage((TCHAR*)("ReadFile()"));
					break;
					//return;//防止读写数据时，串口非正常断开导致死循环一直执行。 与上面添加防死锁的代码差不多
				}
				}
			}
			else//ReadFile返回TRUE
			{
				// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
				bRead = TRUE;
			}
		}  // close if (bRead)

 //如果串口不可读，获取原因
//异步IO操作仍在进行，需要调用GetOverlappedResult查询
		if (!bRead)
		{
			bRead = TRUE;
			bResult = GetOverlappedResult(port->m_hComm,    // Handle to COMM port 
				&port->m_ov,        // Overlapped structure
				&BytesRead,        // Stores number of bytes read
				TRUE);             // Wait flag

								   // deal with the error code 
			if (!bResult)
			{
				port->ProcessErrorMessage((TCHAR*)("GetOverlappedResults() in ReadFile()"));
			}
		}  // close if (!bRead)

		LeaveCriticalSection(&port->m_csCommunicationSync);//解锁 到EnterCriticalSection之间代码资源已经释放了，其他线程可以进行操作   

		// notify parent that a byte was received
		//避免线程互相等待，产生死锁，使用PostMessage()代替SendMessage()
		PostMessage(port->m_pOwner, Wm_SerialPort_RXCHAR, (WPARAM)RXBuff, (LPARAM)port->m_nPortNr);			//绑定消息响应函数
		//::SendMessage((port->m_pOwner), Wm_SerialPort_RXCHAR, (WPARAM) RXBuff, (LPARAM) port->m_nPortNr);
	} // end forever loop

}

//
// str received. Inform the owner
//接收字符串
void CSerialPort::ReceiveStr(CSerialPort* port)
{
	BOOL  bRead = TRUE;				//能否读取状态位，1-可读，0-不可读
	BOOL  bResult = TRUE;			//串口通讯成功与否标识符
	DWORD dwError = 0;				//错误代码，dwError=GEtlastError();
	DWORD BytesRead = 0;			//实际读取到的字节数
	COMSTAT comstat;				//串口状态结构体
	serialPortInfo commInfo;		//传递串口状态位，包含 portNr串口号，bytesRead读取到的字节数,用于接收字符串

	for (;;)
	{
		//防止死锁
		if (WaitForSingleObject(port->m_hShutdownEvent, 0) == WAIT_OBJECT_0)
			/*WaitForSingleObject（HANDLE hHandle,DWORD dwMilliseconds）函数用来检测hHandle事件的信号状态，在某一线程中调用该函数时，线程暂时挂起，
			如果在挂起的dwMilliseconds毫秒内，线程所等待的对象变为有信号状态，则该函数立即返回；
			如果时间已经到达dwMilliseconds毫秒，但hHandle所指向的对象还没有变成有信号状态，函数照样返回*/
			return;

		// Gain ownership of the comm port critical section.
		// This process guarantees no other part of this program 
		// is using the port object. 

		EnterCriticalSection(&port->m_csCommunicationSync);//临界区线程锁，锁定m_csCommunicationSync

		// ClearCommError() will update the COMSTAT structure and
		// clear any other errors.
		///更新COMSTAT

		bResult = ClearCommError(port->m_hComm, &dwError, &comstat);		//清除串口错误并将串口错误代码保存到dwError，串口状态保存到comstat

		LeaveCriticalSection(&port->m_csCommunicationSync);//解锁 到EnterCriticalSection之间代码资源已经释放了，其他线程可以进行操作   

		// start forever loop.  I use this type of loop because I
		// do not know at runtime how many loops this will have to
		// run. My solution is to start a forever loop and to
		// break out of it when I have processed all of the
		// data available.  Be careful with this approach and
		// be sure your loop will exit.
		// My reasons for this are not as clear in this sample 
		// as it is in my production code, but I have found this 
		// solutiion to be the most efficient way to do this.

		//comstat.cbInQue输入缓冲区中的字节数,cbOutQue输出缓冲区中的字节数

		//所有字符均被读出，中断循环。
		//0xcccccccc表示串口异常了，会导致RXBuff指针初始化错误
		if (comstat.cbInQue == 0 || comstat.cbInQue == 0xcccccccc)			//如果串口没数据或串口异常
		{
			// break out when all bytes have been read
			break;
		}

		///如果遇到'\0'，那么数据会被截断，实际数据全部读取只是没有显示完全，这个时候使用memcpy才能全部获取
		//RXBuff为接收缓冲区
		unsigned char* RXBuff = new unsigned char[comstat.cbInQue + 1];
		if (RXBuff == NULL)
		{
			return;
		}
		RXBuff[comstat.cbInQue] = '\0';//附加字符串结束符

		EnterCriticalSection(&port->m_csCommunicationSync);//临界区线程锁，锁定m_csCommunicationSync

		if (bRead)
		{
			//串口读出，读出缓冲区中字节
			bResult = ReadFile(port->m_hComm,        // Handle to COMM port 
				RXBuff,                // RX Buffer Pointer
				comstat.cbInQue,                    // Read cbInQue len byte
				&BytesRead,            // Stores number of bytes read
				&port->m_ov);        // pointer to the m_ov structure
									 // deal with the error code 
			//若返回错误，错误处理
			if (!bResult)
			{
				switch (dwError = GetLastError())
				{
				case ERROR_IO_PENDING:			//正常情况，没有字符可读，操作悬挂 erroe code:997
				{
					// asynchronous i/o is still in progress 
					// Proceed on to GetOverlappedResults();
					//异步IO仍在进行
					bRead = FALSE;
					break;
				}
				case ERROR_ACCESS_DENIED://拒绝访问 erroe code:5
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					TCHAR Temp[200] = { 0 };
					_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED，ReadFile() Error Code:%d"), port->m_nPortNr, GetLastError());
					MessageBox(NULL, Temp, _T("COM ReadFile Error"), MB_ICONERROR);
					break;
				}
				case ERROR_INVALID_HANDLE://打开串口失败 erroe code:6
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					break;
				}
				case ERROR_BAD_COMMAND://连接过程中非法断开 erroe code:22
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					TCHAR Temp[200] = { 0 };
					_stprintf_s(Temp, 200, _T("COM%d ERROR_BAD_COMMAND，ReadFile() Error Code:%d"), port->m_nPortNr, GetLastError());
					MessageBox(NULL, Temp, _T("COM ReadFile Error"), MB_ICONERROR);
					break;
				}
				default:
				{
					// Another error has occured.  Process this error.
					port->ProcessErrorMessage((TCHAR*)("ReadFile()"));
					break;
					//return;///防止读写数据时，串口非正常断开导致死循环一直执行。与上面liquanhai添加防死锁的代码差不多
				}
				}
			}
			else//ReadFile成功，返回TRUE
			{
				// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
				bRead = TRUE;
			}
		}  // close if (bRead)

		//异步IO操作仍在进行，需要调用GetOverlappedResult查询
		//如果不能读取，则获取原因
		if (!bRead)
		{
			bRead = TRUE;
			bResult = GetOverlappedResult(port->m_hComm,    // Handle to COMM port 
				&port->m_ov,        // Overlapped structure
				&BytesRead,        // Stores number of bytes read
				TRUE);             // Wait flag

								   // deal with the error code 
			if (!bResult)
			{
				port->ProcessErrorMessage((TCHAR*)("GetOverlappedResults() in ReadFile()"));
			}
		}  // close if (!bRead)

		LeaveCriticalSection(&port->m_csCommunicationSync);//解锁 到EnterCriticalSection之间代码资源已经释放了，其他线程可以进行操作   

		commInfo.portNr = port->m_nPortNr;
		commInfo.bytesRead = BytesRead;
		// notify parent that some byte was received
		::SendMessage((port->m_pOwner), Wm_SerialPort_RXSTR, (WPARAM)RXBuff, (LPARAM)&commInfo);			//绑定串口响应函数

		//释放
		delete[] RXBuff;			//释放RXBuff
		RXBuff = NULL;

	} // end forever loop

}

//
// Return the device control block
//
DCB CSerialPort::GetDCB()
{
	return m_dcb;						//返回DCB设置状态
}

//
// Return the communication event masks
//
DWORD CSerialPort::GetCommEvents()
{
	return m_dwCommEvents;				//获取通信口通信事件，例如EV_RXCHAR，EV_CTS， EV_DSR ，EV_RING等等。
}

//
// Return the output buffer size
//返回输出缓冲区大小
DWORD CSerialPort::GetWriteBufferSize()
{
	return m_nWriteBufferSize;
}

//检测串口是否打开
BOOL CSerialPort::IsOpen()
{
	return m_hComm != NULL && m_hComm != INVALID_HANDLE_VALUE;//m_hComm增加INVALID_HANDLE_VALUE的情况
}

//关闭串口函数
void CSerialPort::ClosetoPort()
{
	MSG message;

	//增加线程挂起判断，解决由于线程挂起导致串口关闭死锁的问题
	if (IsThreadSuspend(m_Thread))
	{
		ResumeThread(m_Thread);
	}
	MessageBox(NULL, _T("1"), _T("COM ReadFile Error"), MB_ICONERROR);
	//串口句柄无效则清除句柄
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
		return;
	}
	MessageBox(NULL, _T("2"), _T("COM ReadFile Error"), MB_ICONERROR);
	do
	{
		SetEvent(m_hShutdownEvent);			//启动 关闭消息响应事件
		MessageBox(NULL, _T("3"), _T("COM ReadFile Error"), MB_ICONERROR);
		//防止死锁
		if (::PeekMessage(&message, m_pOwner, 0, 0, PM_REMOVE))			//提取消息响应事件
		{
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
	} while (m_bThreadAlive);			//若监视线程启用

	// if the port is still opened: close it 
	//若串口打开则关闭串口
	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}
	MessageBox(NULL, _T("4"), _T("COM ReadFile Error"), MB_ICONERROR);
	// Close Handles  
	//关闭句柄，同上
	if (m_hShutdownEvent != NULL)			//关闭 ShutdownEvent
	{
		ResetEvent(m_hShutdownEvent);
	}
	if (m_ov.hEvent != NULL)
	{
		ResetEvent(m_ov.hEvent);			//关闭窗口响应Event
	}
	if (m_hWriteEvent != NULL)
	{
		ResetEvent(m_hWriteEvent);			//关闭写Event
		MessageBox(NULL, _T("5"), _T("COM ReadFile Error"), MB_ICONERROR);
		//CloseHandle(m_hWriteEvent);//开发者反映，这里会导致多个串口工作时，重新打开串口异常
	}
	MessageBox(NULL, _T("6"), _T("COM ReadFile Error"), MB_ICONERROR);
	if (m_szWriteBuffer != NULL)			//释放缓冲区
	{
		delete[] m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
}

//写入char数组
void CSerialPort::WriteToPort(char* string, size_t n)
{
	assert(m_hComm != 0);										//串口必须打开成功
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));		/*void *memset(void *s, int ch, size_t n);
																将s中当前位置后面的n个字节用 ch 替换并返回 s 。*/

	memcpy(m_szWriteBuffer, string, n);							/*void *memcpy(void *destin, void *source, unsigned n)；
																函数的功能是从源source中拷贝n个字节到目标destin中。*/
	m_nWriteSize = n;

	// set event for write
	SetEvent(m_hWriteEvent);								//触发WriteEvent事件，开始进行串口写入
}

//写入BYTE数组
void CSerialPort::WriteToPort(BYTE* Buffer, size_t n)
{
	assert(m_hComm != 0);										//串口必须打开成功
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));		//初始化写缓冲
	memcpy(m_szWriteBuffer, Buffer, n);							//转移数据到写缓冲区
		/*另一种转换方式
			int i;
				for(i=0; i<n; i++)
					{
						m_szWriteBuffer[i] = Buffer[i];
							}
								*/
	m_nWriteSize = n;

	// set event for write
	SetEvent(m_hWriteEvent);
}



//查询注册表的串口号，将值存于数组中
//
void CSerialPort::QueryKey(HKEY hKey)
{
#define MAX_KEY_LENGTH 255					
#define MAX_VALUE_NAME 16383				
	//    TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	//    DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	for (i = 0; i < 20; i++)///存放串口号的数组初始化
	{
		m_nComArray[i] = -1;
	}

	// Enumerate the key values. 
	if (cValues > 0) {
		for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
		{
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';
			if (ERROR_SUCCESS == RegEnumValue(hKey, i, achValue, &cchValue, NULL, NULL, NULL, NULL))
			{
				TCHAR strDSName[10];
				memset(strDSName, 0, 10);
				DWORD nValueType = 0, nBuffLen = 10;
				if (ERROR_SUCCESS == RegQueryValueEx(hKey, (LPCTSTR)achValue, NULL, &nValueType, (LPBYTE)strDSName, &nBuffLen))
				{
					int nIndex = -1;
					while (++nIndex < MaxSerialPortNum)
					{
						if (-1 == m_nComArray[nIndex])
						{
							m_nComArray[nIndex] = _tstoi((TCHAR*)(strDSName + 3));
							break;
						}
					}
				}
			}
		}
	}
	else {
		MessageBox(NULL, _T("No Com In This Computer!"), _T("COM Query Error"), MB_ICONERROR);
	}

}

#ifdef _AFX
void CSerialPort::Hkey2ComboBox(CComboBox& m_PortNO)
{
	HKEY hTestKey;
	bool Flag = FALSE;

	//仅是XP系统的注册表位置，其他系统根据实际情况做修改
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hTestKey))
	{
		QueryKey(hTestKey);
	}
	RegCloseKey(hTestKey);

	int i = 0;
	m_PortNO.ResetContent();///刷新时，清空下拉列表内容
	while (i < MaxSerialPortNum && -1 != m_nComArray[i])
	{
		CString szCom;
		szCom.Format(_T("COM%d"), m_nComArray[i]);
		m_PortNO.InsertString(i, szCom.GetBuffer(5));
		++i;
		Flag = TRUE;
		if (Flag)///把第一个发现的串口设为下拉列表的默认值
			m_PortNO.SetCurSel(0);
	}
}
#endif // _AFX


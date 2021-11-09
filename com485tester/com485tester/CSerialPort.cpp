#include "stdafx.h"
#include "CSerialPort.h"
#include <assert.h>

int m_nComArray[20];//��Ż�Ծ�Ĵ��ں�

int CSerialPort::GetPortNO()
{
	return m_nPortNr;
}

//����ṹ��
CSerialPort::CSerialPort()	//��ʼ������
{
	m_hComm = NULL;			//��ʼ�����ھ��

	// initialize overlapped structure members to zero PS��overlappedΪ����ͨ�ŵ��ص�ģʽ����createfile��Ϊ FILE_FLAG_OVERLAPPED���������ͬ��ģʽ��NULL
	///��ʼ���첽�ṹ��
	m_ov.Offset = 0;			//��ʼ�������ļ���λ��
	m_ov.OffsetHigh = 0;		//�ļ���ʼ�����ֽ�ƫ�����ĸ���λ��PS����ʼ��ָ�������ļ��ĸ߶�

	// create events
	m_ov.hEvent = NULL;			//��ʼ�� �������ʱ���¼���Ӧ������һ����Readfile(),Writefile()֮ǰ��
	m_hWriteEvent = NULL;		//��ʼ��д�����¼�
	m_hShutdownEvent = NULL;

	m_szWriteBuffer = NULL;

	m_bThreadAlive = FALSE;

	m_nWriteSize = 1;
	m_bIsSuspened = FALSE;
}


//SerialPort������������Delete dynamic memory
CSerialPort::~CSerialPort()
{
	MSG message;						//����MSG�ṹ�壬������Ϣ�����������߳���Ϣ����
	/*MSG�ṹ�����
	������windows��ͨ�����Ӹ��������豸���ѷ������¼�ת��Ϊ��Ϣ�ģ�������Ϣ��������Ϣ�����С����ǰ��Ӧ�ó�����Լ�����Ϣ�����а�˳�������Ϣ������ÿһ����Ϣ���͵�����Ӧ�Ĵ�����Ϣ��������ȥ
	typedef struct tagMSG{
			HWND hwnd��		//��Ϣ��ָ��Ĵ��ڵľ����
			UINT message��	//��Ϣ��ʶ��
			WPARAM wparam��	//32λ�ġ���Ϣ���������ò����ĺ����ȡֵȡ���ھ������Ϣ
			LPARAM lparam��	//����һ��32λ�ġ���Ϣ���������ò����ĺ����ȡֵͬ��ȡ���ھ������Ϣ
			DWORD time��	//��Ϣ������Ϣ���е�ʱ��
			POINT pt��		//��Ϣ������Ϣ�е����ָ���λ�����ꡣ
		}MSG,*PMSG;
	*/

	//�����̹߳����жϣ���������̹߳����´��ڹر�����������
	if (IsThreadSuspend(m_Thread))
	{
		ResumeThread(m_Thread);
	}

	//�����ھ����Ч,�ͷž��
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);			//����ֵLong�ͣ�!0 ��ʾ�ɹ������ʾʧ�ܡ�������GetLastError
		m_hComm = NULL;
		return;
	}

	do												//do{} while{}ѭ��
	{
		SetEvent(m_hShutdownEvent);					//�� �ر��¼� ��Ϊ���ź�״̬����ֹ����
		if (::PeekMessage(&message, m_pOwner, 0, 0, PM_REMOVE))	//PeekMessage���ڶ���Ӧ��Ϣ����Ϣ���е�ַ��ָ�룬����ָ��:�ô�����Ϣ����������Ϣ�½磬��Ϣ�Ͻ磺NULL>����������Ϣ����������δ���:PM_REMOVE>����������ϢPM_NOREMOVE>����������Ϣ��
		{
			::TranslateMessage(&message);	//��������Ϣת��Ϊ�ַ���Ϣ�����浽MSG��Ϣ�����У�����ָ����ʽ���ݣ���					//
											//����Ϣ��ת�����򷵻�WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, �� WM_SYSKEYUP����δת��������0.
			::DispatchMessage(&message);	//�ú������Ⱥ�����Ϣ��MSGָ��ָ�����Ϣ���ݸ����ڳ���
		}
	} while (m_bThreadAlive);				//��������߳�״̬λΪ�򿪣���ִ��do{}����

// if the port is still opened: close it 
	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	// Close Handles  
	//�رվ��
	if (m_hShutdownEvent != NULL)			//�ر�ShutdownEvent�¼�
		CloseHandle(m_hShutdownEvent);
	if (m_ov.hEvent != NULL)				//�ر��첽ͨ���ź��¼�
		CloseHandle(m_ov.hEvent);
	if (m_hWriteEvent != NULL)				//�ر�д�����¼�
		CloseHandle(m_hWriteEvent);

	//TRACE("Thread ended\n");

	if (m_szWriteBuffer != NULL)			//��д��������ΪNULL�������
	{
		delete[] m_szWriteBuffer;			//�ͷ�д������
		m_szWriteBuffer = NULL;
	}
}

//
// Initialize the port. This can be port 1 to MaxSerialPortNum.
//��ʼ�����ڡ�ֻ����1��MaxSerialPortNum

//��ʼ�����ں���
BOOL CSerialPort::InitPort(HWND pPortOwner,    // �󶨴�����Ӧ��ϢMSG��Ӧ���ڵľ����the owner (CWnd) of the port (receives message)
	UINT  portnr,			// ���ں� (1��MaxSerialPortNum)
	UINT  baud,				// ������
	TCHAR  parity,			// У��λ�������ִ�Сд parity n=none,e=even,o=odd,m=mark,s=space
	UINT  databits,			// ����λ databits 5,6,7,8
	UINT  stopbits,			// ֹͣλ stopbits 1,1.5,2
	DWORD dwCommEvents,		//������Ӧ�¼�����EV_RXCHAR��EV_CTS�� EV_DSR ��EV_RING�ȵȡ�
							// EV_RXCHAR���豸��������ʱ����һ���ַ��ﵽ������ʱ����
	UINT  writebuffersize,	// size of the writebuffer

	DWORD   ReadIntervalTimeout, //�������ʱ
	DWORD   ReadTotalTimeoutMultiplier,//��ʱ��ϵ��
	DWORD   ReadTotalTimeoutConstant,//��ʱ�䳣��
	DWORD   WriteTotalTimeoutMultiplier,//дʱ��ϵ��
	DWORD   WriteTotalTimeoutConstant)//дʱ�䳣��

{
	assert(portnr > 0 && portnr < MaxSerialPortNum);		//���ԣ����ںű����Ǵ���0С��MaxSerialPortNum
	assert(pPortOwner != NULL);								//���ԣ�����ҪΪ������Ӧ��Ϣ��һ�����ھ��

	MSG message;				//��ǰ��

//�����̹߳����жϣ���������̹߳����´��ڹر�����������
	if (IsThreadSuspend(m_Thread))				//������
	{
		ResumeThread(m_Thread);					//�ָ��߳�
	}

	//���߳����У������
	// if the thread is alive: Kill
	if (m_bThreadAlive)
	{
		do
		{
			SetEvent(m_hShutdownEvent);			//�� �ر��¼� ��Ϊ���ź�״̬
			//��ֹ������ͬ��
			if (::PeekMessage(&message, m_pOwner, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		} while (m_bThreadAlive);
		//TRACE("Thread ended\n");
		Sleep(50);//�˴�����ʱ����Ҫ����Ϊ������ڿ��ţ����͹ر�ָ����׹ر���Ҫһ����ʱ�䣬�����ʱӦ�ø����Ե��������
	}

	// ���� events
	if (m_ov.hEvent != NULL)
		ResetEvent(m_ov.hEvent);								//���첽ͨ��Event��Ϊ���ź�״̬
	else														//change by COMTOOL
		m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		//CreateEvent(��ȫ����:NULL����λ��ʽ:TRUE>��ResetEvent��λFALSE���߳��ͷź��Զ���λ����ʼ״̬:TRUE>���ź�FALSE�����źţ�ָ������:NULL>�����¼�����)

	if (m_hWriteEvent != NULL)
		ResetEvent(m_hWriteEvent);								//���WriteEvent
	else														//change by COMTOOL
		m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hShutdownEvent != NULL)
		ResetEvent(m_hShutdownEvent);							//���ShutdownEvent
	else														//change by COMTOOL
		m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// initialize the event objects
	//�¼������ʼ�����趨���ȼ���
	m_hEventArray[0] = m_hShutdownEvent;    // highest priority
											//Ϊ������Щ�����豸���������룬��һֱ���ض��¼���ʹ�����߳�������
											//���Խ���д���������߳��У������޸Ķ�д�¼����ȼ�
											//�޸����ȼ�������������
											//����һΪ�����߳���WaitCommEvent()���������������䣺
											//if (WAIT_OBJECT_O == WaitForSingleObject(port->m_hWriteEvent, 0))
											//    ResetEvent(port->m_ov.hEvent);
											//������Ϊ��ʼ��ʱ���޸ģ�������������䣺
	m_hEventArray[1] = m_hWriteEvent;
	m_hEventArray[2] = m_ov.hEvent;
	/*m_hEventArray[1] = m_ov.hEvent;
	m_hEventArray[2] = m_hWriteEvent;*/


	//��ʼ���ٽ���Դ initialize critical section
	/*
	�ٽ���������
	�����̵ĸ����߳̿���ʹ���ٽ���Դ���������ͬ���������⣬�ö����ܱ�֤�ĸ��߳��ܹ���õ��ٽ���Դ���󣬸�ϵͳ�ܹ�ƽ�ĶԴ�ÿһ���̡߳����ٶȰٿƣ�
	��ʹ��һ���ٽ���������ǰ��һЩ�����е��̱߳������InitializeCriticalSection��������ʼ������һ��һ���ٽ��������ѱ���ʼ�����ý��̵��߳̿�����EnterCriticalSection��LeaveCriticalSection����ָ�������ṩ�Թ�����Դ���໥��ռʽ���ʡ�
	�ڽ��ٽ������ݸ� InitializeCriticalSection ʱ�����߸�׼ȷ��˵�����ڴ������ַʱ�����ٽ�������ʼ���ڡ���ʼ��֮�󣬴��뼴���ٽ������ݸ� EnterCriticalSection �� LeaveCriticalSection API��һ���߳��� EnterCriticalSection �з��غ������������� EnterCriticalSection ���̶߳�������ֹ��ֱ����һ���̵߳��� LeaveCriticalSection Ϊֹ��
	*/
	InitializeCriticalSection(&m_csCommunicationSync);

	// set buffersize for writing and save the owner
	//Ϊ��д�������󶨴��ں���
	m_pOwner = pPortOwner;

	if (m_szWriteBuffer != NULL)					//�ͷ�д������
	{
		delete[] m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
	m_szWriteBuffer = new BYTE[writebuffersize];	//�½�д������

	m_nPortNr = portnr;								//���崮�ں�

	m_nWriteBufferSize = writebuffersize;			//����д��������С
	m_dwCommEvents = dwCommEvents;					//���崮����Ӧ�¼�������EV_RXCHAR��EV_CTS�� EV_DSR ��EV_RING�ȵȡ�

	BOOL bResult = FALSE;							//
	TCHAR* szPort = new TCHAR[MAX_PATH];			//���������ڴ�š�COM����
	TCHAR* szBaud = new TCHAR[MAX_PATH];			//����������BuildCommDCB���������"baud="<bau>d parity=<parity> data=<databits> stop=<mystop>")


	// now it critical! ��ʼʹ���ٽ���
	EnterCriticalSection(&m_csCommunicationSync);			///�ٽ����������

// if the port is already opened: close it
//�����Ѵ򿪾͹ص�
	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	// prepare port strings
	//��UINT�������ʹ��ں�ת��Ϊstring��ʽ��"COM?"��ʽ
	_stprintf_s(szPort, MAX_PATH, _T("\\\\.\\COM%d"), portnr);		//��\\\\.\\COM��������ʾCOM10���϶˿�

	int mystop;			// ֹͣλ��0 = 1 1=1.5 2=2��ע��1������1.
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
		//����Ĭ���������Ϊstopbits=1.5ʱ��SetCommState�ᱨ��
		//һ��ĵ��Դ��ڲ�֧��1.5ֹͣλ�����1.5ֹͣλ�ƺ����ں��⴫���ϵġ�
	default:
		mystop = ONESTOPBIT;
		break;
	}
	int myparity;		//У��λ
	myparity = 0;
	parity = _totupper(parity);		//Сд��ĸת��Ϊ��д��ĸ
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
		//����Ĭ�������
	default:
		myparity = 0;
		break;
	}
	_stprintf_s(szBaud, MAX_PATH, _T("baud=%d parity=%c data=%d stop=%d"), baud, parity, databits, mystop);

	// get a handle to the port

	m_hComm = CreateFile(szPort,                        // �򿪴���COMX����ָ�����ھ��m_hComm
		GENERIC_READ | GENERIC_WRITE,    // �ɶ���д read/write types
		0,                                // ��ռ��ʽ�����ڲ�֧�ֹ��� comm devices must be opened with exclusive access
		NULL,                            // ��ȫ����ָ�룬Ĭ��ֵΪNULL no security attributes
		OPEN_EXISTING,                    // �����еĴ����ļ� comm devices must use OPEN_EXISTING
		FILE_FLAG_OVERLAPPED,            // 0��ͬ����ʽ��FILE_FLAG_OVERLAPPED���첽��ʽ
		0);                           //���ڸ����ļ������Ĭ��ֵΪNULL/0���Դ��ڶ��Ըò���������ΪNULL

//����ʧ��
	if (m_hComm == INVALID_HANDLE_VALUE)			//����򿪴���ʧ��
	{

		//���ڴ�ʧ�ܣ�������ʾ��Ϣ
		switch (GetLastError())						//��ȡ���ڴ���
		{
			//���ڲ�����
		case ERROR_FILE_NOT_FOUND:
		{
			TCHAR Temp[200] = { 0 };
			_stprintf_s(Temp, 200, _T("COM%d ERROR_FILE_NOT_FOUND,Error Code:%d"), portnr, GetLastError());
			MessageBox(NULL, Temp, _T("COM InitPort Error"), MB_ICONERROR);
			break;
		}
		//���ھܾ�����
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
		delete[] szPort;				//�ͷŴ��ںŶ����ڴ�
		delete[] szBaud;				//�ͷŲ����ʶ����ڴ�

		return FALSE;
	}

	// set the timeout values
	///���ó�ʱ
	m_SerialPortTimeouts.ReadIntervalTimeout = ReadIntervalTimeout * 1000;
	m_SerialPortTimeouts.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier * 1000;
	m_SerialPortTimeouts.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant * 1000;
	m_SerialPortTimeouts.WriteTotalTimeoutMultiplier = WriteTotalTimeoutMultiplier * 1000;
	m_SerialPortTimeouts.WriteTotalTimeoutConstant = WriteTotalTimeoutConstant * 1000;

	// configure
	///����
	///�ֱ����Windows API���ô��ڲ���
	if (SetCommTimeouts(m_hComm, &m_SerialPortTimeouts))//���ó�ʱ
		/*SetCommTimeouts�����ĵ�һ������hFile����CreateFile��������ָ���Ѵ򿪴��пڵľ����
			�ڶ�������ָ��ӵ�г�ʱ���ݵĿ��ƿ�COMMTIMEOUTS��
				����������óɹ����򷵻�ֵΪ��0������������ʧ�ܣ��򷵻�ֵΪ0��*/
	{

		if (SetCommMask(m_hComm, dwCommEvents))//SetCommMask�����趨��Ҫ����֪ͨ���¼������������dwCommEvents��
		{

			if (GetCommState(m_hComm, &m_dcb))//��ȡ��ǰDCB������GetCommState�����ĵ�һ������hFile����CreateFile��������ָ���Ѵ򿪴��пڵľ�����ڶ�������ָ���豸���ƿ�DCB������������óɹ����򷵻�ֵΪ��0������������ʧ�ܣ��򷵻�ֵΪ0��
			{
				m_dcb.EvtChar = 'q';							//�����յ����ַ�ʱ,�����һ��EV_RXFLAG�¼��������SetCommMask������ָ����EV_RXFLAG , �����WaitCommEvent �������¼�
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE;			// RTS(request-to-send)���������á�RTS_CONTROL_ENABLEΪ���豸ʱ��RTS�ź�Ϊ�ߵ�ƽ��Ӧ�ó����ͨ������ 
				m_dcb.BaudRate = baud;							//ָ����ǰ���õĲ����ʣ�Ӧ�������ӵ�ͨѶ�豸��ƥ�� 
				m_dcb.Parity = myparity;						//ָ���˿����ݴ����У�鷽��
				m_dcb.ByteSize = databits;						//һ���ֽڴ�С
				m_dcb.StopBits = mystop;						//ֹͣλ����

				//if (BuildCommDCB(szBaud &m_dcb))///��дDCB�ṹ����SetCommState���ơ���д��ʽΪBuildCommDCB(��COM1:2400,n,8,1��,&dcb);
				//{
				if (SetCommState(m_hComm, &m_dcb))//���øô��ڰ󶨴��ڵ�DCB
					; /*SetCommState�������ĵ�һ������hFile����CreateFile��������ָ���Ѵ򿪴��пڵľ����
							�ڶ�������ָ���豸���ƿ�DCB������������óɹ����򷵻�ֵΪ��0��
									����������ʧ�ܣ��򷵻�ֵΪ0��*/
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
	//��ֹ��д����ս��պͷ���
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	/*PurgeComm(HANDLE hFile,DWORD dwFlags )
		����dwFlagsָ��Ҫ��ɵĲ���������������ֵ����ϣ�
		PURGE_TXABORT ��ֹ�������ڽ��е��ַ��������, ���һ�������ڵȴ�״̬���ص�i / o����, ��������һ���¼�, ָ�������д����
		PURGE_RXABORT ��ֹ�������ڽ��е��ַ��������, ���һ�����ڽ����е��ص�i / o����, �����������õ��ʵ��¼�
		PURGE_TXCLEAR �������ָ���豸����������������������������PURGE_TXABORT �����־һ��ʹ��
		PURGE_RXCLEAR ������������豸��������������뻺������������PURGE_RXABORT �����־һ��ʹ��*/

		// release critical section
		//�ͷ��ٽ���Դ
	LeaveCriticalSection(&m_csCommunicationSync);//���� ��EnterCriticalSection֮�������Դ�Ѿ��ͷ��ˣ������߳̿��Խ��в���   

	//TRACE("Initialisation for communicationport %d completed.\nUse Startmonitor to communicate.\n", portnr);

	return TRUE;
}

//�����̵߳Ĵ������̣�
//��鴮��-->����ѭ��{WaitCommEvent(������ѯ��)ѯ���¼�-->������¼�����-->����Ӧ����(�ر�\��\д)}
//�����̺߳�����The CommThread Function
DWORD WINAPI CSerialPort::CommThread(LPVOID pParam)
{
	// Cast the void pointer passed to the thread back to a pointer of CSerialPort class
	CSerialPort* port = (CSerialPort*)pParam;

	// Set the status variable in the dialog class��
	// TRUE��ʾ�߳��������У�TRUE to indicate the thread is running.
	port->m_bThreadAlive = TRUE;

	// Misc. variables
	DWORD BytesTransfered = 0;
	DWORD Event = 0;					//�����¼������ţ�0-�ر��¼���1-�������ݣ�2-������Ӧ�¼�
	DWORD CommEvent = 0;				//�����¼���ʶ��
	DWORD dwError = 0;					//��ȡʧ��״̬������dwError = GetLastError()
	COMSTAT comstat;					//���崮��״̬�ṹ��

	BOOL  bResult = TRUE;				//ͨ���¼���������жϱ�־λ��

	// Clear comm buffers at startup
	///��ʼʱ������ڻ���
	if (port->m_hComm)        // ������ڴ򿪣�check if the port is opened
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);		//������ڣ�ͬ��

	// begin forever loop.  This loop will run as long as the thread is alive.
	///ֻҪ�̴߳��ھͲ��϶�ȡ����
	for (;;)				//��ʼ��ѭ��
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
						WaitCommEvent������3������1pOverlapped������һ��OVERLAPPED�ṹ�ı���ָ��
								��Ҳ������NULL������NULLʱ����ʾ�ú�����ͬ���ģ������ʾ�ú������첽�ġ�
										����WaitCommEventʱ������첽��������������ɣ�����������FALSE��ϵͳ��
												WaitCommEvent����ǰ��OVERLAPPED�ṹ��ԱhEvent��Ϊ���ź�״̬���ȵ�����ͨ��
														�¼�ʱ��ϵͳ���������ź�
																*/

		bResult = WaitCommEvent(port->m_hComm, &Event, &port->m_ov);//��ʾ�ú������첽��
		/*BOOL WaitCommEvent								//�ж���SetCommMask()�������õĴ���ͨ���¼��Ƿ��ѷ�����
			(HANDLE hFile,
			LPDWORD lpEvtMask,								//�����������pEvtMaskָ��0������ָ��ĳһ�¼�
			LPOVERLAPPED lpOverlapped						//�첽�ṹ�����������첽���������
		);*/
		if (!bResult)										//��WaitCommEventʧ��
		{
			// If WaitCommEvent() returns FALSE, process the last error to determin the reason.
			//���WaitCommEvent����ErrorΪFALSE�����ѯ������Ϣ
			switch (dwError = GetLastError())
			{
			case ERROR_IO_PENDING:     //���������û���ַ��ɶ����������� erroe code:997
			{
				// This is a normal return value if there are no bytes
				// to read at the port.
				// Do nothing and continue
				break;
			}
			case ERROR_INVALID_PARAMETER://ϵͳ���� erroe code:87
			{
				// Under Windows NT, this value is returned for some reason.
				// I have not investigated why, but it is also a valid reply
				// Also do nothing and continue.
				break;
			}
			case ERROR_ACCESS_DENIED://�ܾ����� erroe code:5
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				TCHAR Temp[200] = { 0 };
				_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED��WaitCommEvent() Error Code:%d"), port->m_nPortNr, GetLastError());
				MessageBox(NULL, Temp, _T("COM WaitCommEvent Error"), MB_ICONERROR);
				break;
			}
			case ERROR_INVALID_HANDLE://�򿪴���ʧ�� erroe code:6
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				break;
			}
			case ERROR_BAD_COMMAND://���ӹ����зǷ��Ͽ� erroe code:22
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				TCHAR Temp[200] = { 0 };
				_stprintf_s(Temp, 200, _T("COM%d ERROR_BAD_COMMAND��WaitCommEvent() Error Code:%d"), port->m_nPortNr, GetLastError());
				MessageBox(NULL, Temp, _T("COM WaitCommEvent Error"), MB_ICONERROR);
				break;
			}
			default://�����������������д��ڶ�д�жϿ��������ӵĴ��󣨴���22��
			{
				// All other error codes indicate a serious error has
				port->m_hComm = INVALID_HANDLE_VALUE;					//��������ʱ�������ھ����Ϊ��Ч���
				// occured.  Process this error.
				port->ProcessErrorMessage((TCHAR*)("WaitCommEvent()"));
				break;
			}
			}
		}
		else    //�����¼�������WaitCommEvent()����ȷ����
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

			bResult = ClearCommError(port->m_hComm, &dwError, &comstat);		//������ڴ����¼�
			/*BOOL ClearCommError(		//�˺������Ӳ����ͨѶ�����Լ���ȡͨѶ�豸�ĵ�ǰ״̬
				HANDLE hFile,			//��CreateFile��������ָ���Ѵ򿪴��пڵľ��
				LPDWORD lpErrors,		//��������
				LPCOMSTAT lpStat		//�����豸״̬�Ŀ��ƿ�COMSTAT
			);*/

			if (comstat.cbInQue == 0)		//comstat.cbInQue���뻺�����е��ֽ���,cbOutQue����������е��ֽ���
				continue;
		}    // end if bResult

			 ///���ȴ��������������߳�
			 // Main wait function.  This function will normally block the thread
			 // until one of nine events occur that require action.
			 ///�ȴ�3���¼����ض�/��/д����һ���¼������ͷ���
		Event = WaitForMultipleObjects(3, //3���¼���������ɹ�������ֵ��ʾ���¼����¸ú������ء�
			port->m_hEventArray, //�¼�����
			FALSE, //��һ���¼������ͷ���
			INFINITE);//��ʱʱ������ʱ������Ժ���Ϊ��λ��
						/*���ָ���˷���ֵ����ú�����һֱ�ȵ�ָ���Ķ��󷢳��źŻ򾭹������
							���dwMillisecondsΪ�㣬�����δ����ָʾ������ú����������ȴ�״̬;�������������ء�
								���dwMilliseconds��INFINITE������ڷ���ָ�������ź�ʱ�ŷ��ظú�����*/

		switch (Event)
		{
		case 0:
		{
			// Shutdown event.  This is event zero so it will be
			// the higest priority and be serviced first.
			///�ض��¼����رմ���
			CloseHandle(port->m_hComm);
			port->m_hComm = NULL;
			port->m_bThreadAlive = FALSE;

			// Kill this thread.  break is not needed, but makes me feel better.
			//AfxEndThread(100);				//COMTOOL change
//			::ExitThread(100);				//COMTOOL change

			break;
		}
		case 1: // write event ��������
		{
			// Write character event from port
			WriteChar(port);					//д�ַ���
			break;
		}
		case 2:    // read event ������ĸ�����Ϣ���ͳ�ȥ
		{
			GetCommMask(port->m_hComm, &CommEvent);			//��ȡ�����¼�
			if (CommEvent & EV_RXCHAR) //���յ��ַ������������뻺������
			{
				if (IsReceiveString == 1)
				{
					ReceiveStr(port);//���ַ�����
				}
				else if (IsReceiveString == 0)
				{
					ReceiveChar(port);//���ַ�����
				}
				else
				{
					//Ĭ�϶��ַ�����
					ReceiveStr(port);//���ַ�����
				}
			}

			if (CommEvent & EV_CTS) //CTS�ź�״̬�����仯
				::SendMessage(port->m_pOwner, Wm_SerialPort_CTS_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			if (CommEvent & EV_RXFLAG) //���յ��¼��ַ������������뻺������ 
				::SendMessage(port->m_pOwner, Wm_SerialPort_RXFLAG_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			if (CommEvent & EV_BREAK)  //�����з����ж�
				::SendMessage(port->m_pOwner, Wm_SerialPort_BREAK_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			if (CommEvent & EV_ERR) //������·״̬������·״̬�������CE_FRAME,CE_OVERRUN��CE_RXPARITY 
				::SendMessage(port->m_pOwner, Wm_SerialPort_ERR_DETECTED, (WPARAM)0, (LPARAM)port->m_nPortNr);
			if (CommEvent & EV_RING) //��⵽����ָʾ
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

//���������߳�
BOOL CSerialPort::StartMonitoring()
{
	//if (!(m_Thread = AfxBeginThread(CommThread, this)))
	if (!(m_Thread = ::CreateThread(NULL, 0, CommThread, this, 0, NULL)))
		/*CreateThread ��������
		CreateThread(												//�����ɹ��������߳̾��������ʧ�ܷ���false
			_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,		//�̰߳�ȫ����.ָ��SECURITY_ATTRIBUTES��̬�Ľṹ��ָ��,NULLʹ��Ĭ�ϰ�ȫ��
			_In_ SIZE_T dwStackSize,								//��ջ��С.���ó�ʼջ�Ĵ�С�����ֽ�Ϊ��λ�����Ϊ0����ôĬ�Ͻ�ʹ������øú������߳���ͬ��ջ�ռ��С��
			_In_ LPTHREAD_START_ROUTINE lpStartAddress,				//�̺߳���.ָ���̺߳�����ָ��
			_In_opt_ __drv_aliasesMem LPVOID lpParameter,			//�̲߳���.���̺߳������ݵĲ�������һ��ָ��ṹ��ָ�룬���贫�ݲ���ʱ��ΪNULL��
			_In_ DWORD dwCreationFlags,								//�̴߳�������,0-��ʾ��������������
			_Out_opt_ LPDWORD lpThreadId							//�̵߳�id
		);*/
		return FALSE;
	//TRACE("Thread started\n");
	return TRUE;
}

// Restart the comm thread
//�ӹ���ָ������߳�
BOOL CSerialPort::ResumeMonitoring()
{
	//TRACE("Thread resumed\n");
	//m_Thread->ResumeThread();
	::ResumeThread(m_Thread);
	return TRUE;
}

//
// Suspend the comm thread
//��������߳�
BOOL CSerialPort::SuspendMonitoring()
{
	//TRACE("Thread suspended\n");
	//m_Thread->SuspendThread();
	::SuspendThread(m_Thread);						//SuspendThread���ڹ����߳��������-1(0xFFFFFFFF)����ʾʧ�ܣ����������������ʾ�߳��Ѿ���������Ĵ�����
	return TRUE;
}

//�ж��߳��Ƿ���������������Ϊ1������Ϊ0. 
BOOL CSerialPort::IsThreadSuspend(HANDLE hThread)
{
	DWORD   count = SuspendThread(hThread);			//SuspendThread���ڹ����̣߳��������-1(0xFFFFFFFF)����ʾʧ�ܣ����������������ʾ�߳��Ѿ���������Ĵ�����
	if (count == -1)
	{
		return FALSE;
	}
	ResumeThread(hThread);							//ResumeThread���Իָ���SuspendThread������̵߳�ִ�С�
	return (count != 0);							//���count��0�Ƿ񲻵ȣ�����Ϊ1�����Ϊ0.
}

//
// If there is a error, give the right message
//��ʾ����������д��󣬸���������ʾ��Ϣ��ErrorText
void CSerialPort::ProcessErrorMessage(TCHAR* ErrorText)
{
	TCHAR Temp[200] = { 0 };

	LPVOID lpMsgBuf;
	/*FormatMessageW��������			//���������������ʽ����Ϣ�ַ��������Ǵ�����Ϣ��Դ�ġ�
	FormatMessageW(
		_In_     DWORD dwFlags,			//��ʽ��ѡ���lpSource����ֵ��ָ�����á�
		_In_opt_ LPCVOID lpSource,		//��Ϣ����Դ����������ֵ����dwFlags��
		_In_     DWORD dwMessageId,		//�����ʽ����Ϣ�ı�ʶ����
		_In_     DWORD dwLanguageId,	//��ʽ����Ϣ���Ա�ʶ����
		_Out_    LPWSTR lpBuffer,		//һ��������ָ�������ܸ�ʽ�������Ϣ��
		_In_     DWORD nSize,			//ָ�����������������Ϣ����TCHARsΪ��λ��
		_In_opt_ va_list *Arguments		//һ�������е�ֵ�ڸ�ʽ����Ϣ����Ϊ����ֵ��
	);*/
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,							//���ֵ����Ϣ����Դ����������ֵ����dwFlags,
		GetLastError(),					//�����ʽ����Ϣ�ı�ʶ����
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// Default language
		(LPTSTR)&lpMsgBuf,							//һ��������ָ�������ܸ�ʽ�������Ϣ��
		0,											//�������ָ�����������������Ϣ����TCHARsΪ��λ
		NULL										//һ�������е�ֵ�ڸ�ʽ����Ϣ����Ϊ����ֵ
	);

	_stprintf_s(Temp, 200, _T("WARNING:  %s Failed with the following error: \n%s\nPort: %d\n"), ErrorText, (TCHAR*)lpMsgBuf, m_nPortNr);
	MessageBox(NULL, Temp, _T("Application Error"), MB_ICONSTOP);

	LocalFree(lpMsgBuf);				//����lpBuffer���������ֵ��FormatMessage������̬����Ļ������������ڲ�ʹ�õ�ʱ��ҪLocalFree.
}

//
// Write a character.
//дһ���ַ���
void CSerialPort::WriteChar(CSerialPort* port)
{
	BOOL bWrite = TRUE;						//д��״̬λ��1-����д�룬0-�޷�д��
	BOOL bResult = TRUE;					//����ͨѶ�ɹ�����־λ��1-�ɹ���0-ʧ��
	DWORD BytesSent = 0;					//д���ֽ���
	DWORD SendLen = port->m_nWriteSize;							//д�����ݳ���
	ResetEvent(port->m_hWriteEvent);							//��Ϊ���ź�״̬


	// Gain ownership of the critical section
	EnterCriticalSection(&port->m_csCommunicationSync);			//�ٽ����߳���������m_csCommunicationSync

	if (bWrite)
	{
		// Initailize variables
		port->m_ov.Offset = 0;				//�����ļ���ʼƫ����	
		port->m_ov.OffsetHigh = 0;			//�����ļ��߶�

		// ��ֹ�շ���������ͬ��
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

		bResult = WriteFile(port->m_hComm,                            // Handle to COMM Port
			port->m_szWriteBuffer,                    // Pointer to message buffer in calling finction
			SendLen,
			//strlen((char*)port->m_szWriteBuffer),    // Length of message to send
			&BytesSent,                                //ʵ��д���ֽ��� Where to store the number of bytes sent
			&port->m_ov);                            // Overlapped structure

													 // deal with any error codes
		if (!bResult)
		{
			DWORD dwError = GetLastError();			//��ȡд��ʧ��ԭ��
			switch (dwError)
			{
			case ERROR_IO_PENDING: //���������û���ַ��ɶ����������� erroe code:997
			{
				// continue to GetOverlappedResults()
				BytesSent = 0;					//���ʵ��д���ֽ���
				bWrite = FALSE;					//д��״̬λ
				break;
			}
			case ERROR_ACCESS_DENIED://�ܾ����� erroe code:5
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				TCHAR Temp[200] = { 0 };
				_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED��WriteFile() Error Code:%d"), port->m_nPortNr, GetLastError());
				MessageBox(NULL, Temp, _T("COM WriteFile Error"), MB_ICONERROR);
				break;
			}
			case ERROR_INVALID_HANDLE://�򿪴���ʧ�� erroe code:6
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				break;
			}
			case ERROR_BAD_COMMAND://���ӹ����зǷ��Ͽ� erroe code:22
			{
				port->m_hComm = INVALID_HANDLE_VALUE;
				TCHAR Temp[200] = { 0 };
				_stprintf_s(Temp, 200, _T("COM%d ERROR_BAD_COMMAND��WriteFile() Error Code:%d"), port->m_nPortNr, GetLastError());
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
			LeaveCriticalSection(&port->m_csCommunicationSync);//�����ٽ��� ��EnterCriticalSection֮�������Դ�Ѿ��ͷ��ˣ������߳̿��Խ��в���   
		}
	} // end if(bWrite)

//�첽IO�������ڽ��У���Ҫ����GetOverlappedResult��ѯ
	if (!bWrite)
	{
		bWrite = TRUE;
		/*	GetOverlappedResult��������
		BOOL GetOverlappedResult(				//��ȡ�ص�����������
			HANDLE hFile,						// ���ڵľ����ָ���ص�������ʼʱָ����OVERLAPPED�ṹ
			LPOVERLAPPED lpOverlapped,			//�ص���־λ
			LPDWORD lpNumberOfBytesTransferred,	// ָ��һ��32λ�������ñ�����ֵ����ʵ�ʶ�д����������ֽ�����
			BOOL bWait							//�ò�������ָ�������Ƿ�һֱ�ȵ��ص�����������
												//����ò���ΪTRUE������ֱ�����������ŷ��ء�
												//����ò���ΪFALSE������ֱ�ӷ��أ���ʱ�������û����ɣ�ͨ������GetLastError()�����᷵��ERROR_IO_INCOMPLETE��
			);*/
		bResult = GetOverlappedResult(port->m_hComm,    // Handle to COMM port 
			&port->m_ov,        // Overlapped structure
			&BytesSent,        // Stores number of bytes sent
			TRUE);             // Wait flag

		LeaveCriticalSection(&port->m_csCommunicationSync);//���� ��EnterCriticalSection֮�������Դ�Ѿ��ͷ��ˣ������߳̿��Խ��в���   

		// ���д���ڴ��� 
		if (!bResult)
		{
			port->ProcessErrorMessage((TCHAR*)("GetOverlappedResults() in WriteFile()"));
		}
	} // end if (!bWrite)

	  // ��֤ʵ�ʷ������ݺ����۷��������Ƿ���� Verify that the data size send equals what we tried to send
	  //if (BytesSent != SendLen /*strlen((char*)port->m_szWriteBuffer)*/)  
	  //{
	  //TRACE(_T("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n"), BytesSent, _tcsclen((TCHAR*)port->m_szWriteBuffer));
	  //}
}

//
// Character received. Inform the owner
//�����ַ�
void CSerialPort::ReceiveChar(CSerialPort* port)
{
	BOOL  bRead = TRUE;					//�ܷ��ȡ״̬λ,1-�ɶ���0-���ɶ�
	BOOL  bResult = TRUE;				//����ͨѶ�ɹ�����ʶ��
	DWORD dwError = 0;					//������룬=GetlastError()
	DWORD BytesRead = 0;				//ʵ�ʶ�ȡ�����ֽ���
	COMSTAT comstat;					//����״̬�ṹ��
	unsigned char RXBuff;				//���ջ�����

	for (;;)
	{
		//��ֹ����
		if (WaitForSingleObject(port->m_hShutdownEvent, 0) == WAIT_OBJECT_0)
			return;
		/*WaitForSingleObject��HANDLE hHandle,DWORD dwMilliseconds�������������hHandle�¼����ź�״̬����ĳһ�߳��е��øú���ʱ���߳���ʱ����
			����ڹ����dwMilliseconds�����ڣ��߳����ȴ��Ķ����Ϊ���ź�״̬����ú����������أ�
				���ʱ���Ѿ�����dwMilliseconds���룬��hHandle��ָ��Ķ���û�б�����ź�״̬��������������*/

				// Gain ownership of the comm port critical section.
				// This process guarantees no other part of this program 
				// is using the port object. 

		EnterCriticalSection(&port->m_csCommunicationSync);//�ٽ����߳���������m_csCommunicationSync

		// ClearCommError() will update the COMSTAT structure and clear any other errors.
		//����COMSTAT
		bResult = ClearCommError(port->m_hComm, &dwError, &comstat);
		/*BOOL ClearCommError(		//�˺������Ӳ����ͨѶ�����Լ���ȡͨѶ�豸�ĵ�ǰ״̬
		HANDLE hFile,			//��CreateFile��������ָ���Ѵ򿪴��пڵľ��
		LPDWORD lpErrors,		//��������
		LPCOMSTAT lpStat		//�����豸״̬�Ŀ��ƿ�COMSTAT
		);*/

		LeaveCriticalSection(&port->m_csCommunicationSync);//���� ��EnterCriticalSection֮�������Դ�Ѿ��ͷ��ˣ������߳̿��Խ��в���   

		// start forever loop.  I use this type of loop because I
		// do not know at runtime how many loops this will have to
		// run. My solution is to start a forever loop and to
		// break out of it when I have processed all of the
		// data available.  Be careful with this approach and
		// be sure your loop will exit.
		// My reasons for this are not as clear in this sample 
		// as it is in my production code, but I have found this 
		// solutiion to be the most efficient way to do this.

		//�����ַ������������ж�ѭ��
		if (comstat.cbInQue == 0)		//comstat.cbInQue���뻺�����е��ֽ���,cbOutQue����������е��ֽ���
		{
			// break out when all bytes have been read
			break;
		}

		EnterCriticalSection(&port->m_csCommunicationSync);//�ٽ����߳���������m_csCommunicationSync

		if (bRead)
		{
			//��ȡ���ڣ��������������ֽ�
			bResult = ReadFile(
				port->m_hComm,        // Handle to COMM port 
				&RXBuff,                // RX Buffer Pointer
				1,                    // Read one byte
				&BytesRead,            // Stores number of bytes read
				&port->m_ov);        // pointer to the m_ov structure
									 // deal with the error code 
			/*BOOL ReadFile(					//���óɹ�,���ط�0.���ò��ɹ�,����Ϊ0
												//������GetLastError������������һ���첽�������������᷵����ֵ����ERROR_IO_PENDING���ó�GetLastError�Ľ����
												//����������ֵ����������ֽ���С��nNumberOfBytesToRead����ָ����ֵ���������ѵִ����ļ��Ľ�β��
				HANDLE hFile,					//�ļ��ľ��
				LPVOID lpBuffer,				//���ڱ���������ݵ�һ��������
				DWORD nNumberOfBytesToRead,		//Ҫ������ֽ���
				LPDWORD lpNumberOfBytesRead,	//ָ��ʵ�ʶ�ȡ�ֽ�����ָ��
				LPOVERLAPPED lpOverlapped		//�첽ͨ��״̬��ʶ��
				//���ļ���ʱָ����FILE_FLAG_OVERLAPPED����ô���룬�������������һ������Ľṹ��
				//�ýṹ������һ���첽��ȡ����������Ӧ�����������ΪNULL
			);*/

			//�����ش��󣬴�����
			if (!bResult)
			{
				switch (dwError = GetLastError())
				{
				case ERROR_IO_PENDING:	//���������û���ַ��ɶ����������� erroe code:997
				{
					// asynchronous i/o is still in progress 
					// Proceed on to GetOverlappedResults();
					///�첽IO���ڽ���
					bRead = FALSE;
					break;
				}
				case ERROR_ACCESS_DENIED://�ܾ����� erroe code:5
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					TCHAR Temp[200] = { 0 };
					_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED��ReadFile() Error Code:%d"), port->m_nPortNr, GetLastError());
					MessageBox(NULL, Temp, _T("COM ReadFile Error"), MB_ICONERROR);
					break;
				}
				case ERROR_INVALID_HANDLE://�򿪴���ʧ�� erroe code:6
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					break;
				}
				case ERROR_BAD_COMMAND://���ӹ����зǷ��Ͽ� erroe code:22
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					TCHAR Temp[200] = { 0 };
					_stprintf_s(Temp, 200, _T("COM%d ERROR_BAD_COMMAND��ReadFile() Error Code:%d"), port->m_nPortNr, GetLastError());
					MessageBox(NULL, Temp, _T("COM ReadFile Error"), MB_ICONERROR);
					break;
				}
				default:
				{
					// Another error has occured.  Process this error.
					port->ProcessErrorMessage((TCHAR*)("ReadFile()"));
					break;
					//return;//��ֹ��д����ʱ�����ڷ������Ͽ�������ѭ��һֱִ�С� ��������ӷ������Ĵ�����
				}
				}
			}
			else//ReadFile����TRUE
			{
				// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
				bRead = TRUE;
			}
		}  // close if (bRead)

 //������ڲ��ɶ�����ȡԭ��
//�첽IO�������ڽ��У���Ҫ����GetOverlappedResult��ѯ
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

		LeaveCriticalSection(&port->m_csCommunicationSync);//���� ��EnterCriticalSection֮�������Դ�Ѿ��ͷ��ˣ������߳̿��Խ��в���   

		// notify parent that a byte was received
		//�����̻߳���ȴ�������������ʹ��PostMessage()����SendMessage()
		PostMessage(port->m_pOwner, Wm_SerialPort_RXCHAR, (WPARAM)RXBuff, (LPARAM)port->m_nPortNr);			//����Ϣ��Ӧ����
		//::SendMessage((port->m_pOwner), Wm_SerialPort_RXCHAR, (WPARAM) RXBuff, (LPARAM) port->m_nPortNr);
	} // end forever loop

}

//
// str received. Inform the owner
//�����ַ���
void CSerialPort::ReceiveStr(CSerialPort* port)
{
	BOOL  bRead = TRUE;				//�ܷ��ȡ״̬λ��1-�ɶ���0-���ɶ�
	BOOL  bResult = TRUE;			//����ͨѶ�ɹ�����ʶ��
	DWORD dwError = 0;				//������룬dwError=GEtlastError();
	DWORD BytesRead = 0;			//ʵ�ʶ�ȡ�����ֽ���
	COMSTAT comstat;				//����״̬�ṹ��
	serialPortInfo commInfo;		//���ݴ���״̬λ������ portNr���ںţ�bytesRead��ȡ�����ֽ���,���ڽ����ַ���

	for (;;)
	{
		//��ֹ����
		if (WaitForSingleObject(port->m_hShutdownEvent, 0) == WAIT_OBJECT_0)
			/*WaitForSingleObject��HANDLE hHandle,DWORD dwMilliseconds�������������hHandle�¼����ź�״̬����ĳһ�߳��е��øú���ʱ���߳���ʱ����
			����ڹ����dwMilliseconds�����ڣ��߳����ȴ��Ķ����Ϊ���ź�״̬����ú����������أ�
			���ʱ���Ѿ�����dwMilliseconds���룬��hHandle��ָ��Ķ���û�б�����ź�״̬��������������*/
			return;

		// Gain ownership of the comm port critical section.
		// This process guarantees no other part of this program 
		// is using the port object. 

		EnterCriticalSection(&port->m_csCommunicationSync);//�ٽ����߳���������m_csCommunicationSync

		// ClearCommError() will update the COMSTAT structure and
		// clear any other errors.
		///����COMSTAT

		bResult = ClearCommError(port->m_hComm, &dwError, &comstat);		//������ڴ��󲢽����ڴ�����뱣�浽dwError������״̬���浽comstat

		LeaveCriticalSection(&port->m_csCommunicationSync);//���� ��EnterCriticalSection֮�������Դ�Ѿ��ͷ��ˣ������߳̿��Խ��в���   

		// start forever loop.  I use this type of loop because I
		// do not know at runtime how many loops this will have to
		// run. My solution is to start a forever loop and to
		// break out of it when I have processed all of the
		// data available.  Be careful with this approach and
		// be sure your loop will exit.
		// My reasons for this are not as clear in this sample 
		// as it is in my production code, but I have found this 
		// solutiion to be the most efficient way to do this.

		//comstat.cbInQue���뻺�����е��ֽ���,cbOutQue����������е��ֽ���

		//�����ַ������������ж�ѭ����
		//0xcccccccc��ʾ�����쳣�ˣ��ᵼ��RXBuffָ���ʼ������
		if (comstat.cbInQue == 0 || comstat.cbInQue == 0xcccccccc)			//�������û���ݻ򴮿��쳣
		{
			// break out when all bytes have been read
			break;
		}

		///�������'\0'����ô���ݻᱻ�ضϣ�ʵ������ȫ����ȡֻ��û����ʾ��ȫ�����ʱ��ʹ��memcpy����ȫ����ȡ
		//RXBuffΪ���ջ�����
		unsigned char* RXBuff = new unsigned char[comstat.cbInQue + 1];
		if (RXBuff == NULL)
		{
			return;
		}
		RXBuff[comstat.cbInQue] = '\0';//�����ַ���������

		EnterCriticalSection(&port->m_csCommunicationSync);//�ٽ����߳���������m_csCommunicationSync

		if (bRead)
		{
			//���ڶ������������������ֽ�
			bResult = ReadFile(port->m_hComm,        // Handle to COMM port 
				RXBuff,                // RX Buffer Pointer
				comstat.cbInQue,                    // Read cbInQue len byte
				&BytesRead,            // Stores number of bytes read
				&port->m_ov);        // pointer to the m_ov structure
									 // deal with the error code 
			//�����ش��󣬴�����
			if (!bResult)
			{
				switch (dwError = GetLastError())
				{
				case ERROR_IO_PENDING:			//���������û���ַ��ɶ����������� erroe code:997
				{
					// asynchronous i/o is still in progress 
					// Proceed on to GetOverlappedResults();
					//�첽IO���ڽ���
					bRead = FALSE;
					break;
				}
				case ERROR_ACCESS_DENIED://�ܾ����� erroe code:5
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					TCHAR Temp[200] = { 0 };
					_stprintf_s(Temp, 200, _T("COM%d ERROR_ACCESS_DENIED��ReadFile() Error Code:%d"), port->m_nPortNr, GetLastError());
					MessageBox(NULL, Temp, _T("COM ReadFile Error"), MB_ICONERROR);
					break;
				}
				case ERROR_INVALID_HANDLE://�򿪴���ʧ�� erroe code:6
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					break;
				}
				case ERROR_BAD_COMMAND://���ӹ����зǷ��Ͽ� erroe code:22
				{
					port->m_hComm = INVALID_HANDLE_VALUE;
					TCHAR Temp[200] = { 0 };
					_stprintf_s(Temp, 200, _T("COM%d ERROR_BAD_COMMAND��ReadFile() Error Code:%d"), port->m_nPortNr, GetLastError());
					MessageBox(NULL, Temp, _T("COM ReadFile Error"), MB_ICONERROR);
					break;
				}
				default:
				{
					// Another error has occured.  Process this error.
					port->ProcessErrorMessage((TCHAR*)("ReadFile()"));
					break;
					//return;///��ֹ��д����ʱ�����ڷ������Ͽ�������ѭ��һֱִ�С�������liquanhai��ӷ������Ĵ�����
				}
				}
			}
			else//ReadFile�ɹ�������TRUE
			{
				// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
				bRead = TRUE;
			}
		}  // close if (bRead)

		//�첽IO�������ڽ��У���Ҫ����GetOverlappedResult��ѯ
		//������ܶ�ȡ�����ȡԭ��
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

		LeaveCriticalSection(&port->m_csCommunicationSync);//���� ��EnterCriticalSection֮�������Դ�Ѿ��ͷ��ˣ������߳̿��Խ��в���   

		commInfo.portNr = port->m_nPortNr;
		commInfo.bytesRead = BytesRead;
		// notify parent that some byte was received
		::SendMessage((port->m_pOwner), Wm_SerialPort_RXSTR, (WPARAM)RXBuff, (LPARAM)&commInfo);			//�󶨴�����Ӧ����

		//�ͷ�
		delete[] RXBuff;			//�ͷ�RXBuff
		RXBuff = NULL;

	} // end forever loop

}

//
// Return the device control block
//
DCB CSerialPort::GetDCB()
{
	return m_dcb;						//����DCB����״̬
}

//
// Return the communication event masks
//
DWORD CSerialPort::GetCommEvents()
{
	return m_dwCommEvents;				//��ȡͨ�ſ�ͨ���¼�������EV_RXCHAR��EV_CTS�� EV_DSR ��EV_RING�ȵȡ�
}

//
// Return the output buffer size
//���������������С
DWORD CSerialPort::GetWriteBufferSize()
{
	return m_nWriteBufferSize;
}

//��⴮���Ƿ��
BOOL CSerialPort::IsOpen()
{
	return m_hComm != NULL && m_hComm != INVALID_HANDLE_VALUE;//m_hComm����INVALID_HANDLE_VALUE�����
}

//�رմ��ں���
void CSerialPort::ClosetoPort()
{
	MSG message;

	//�����̹߳����жϣ���������̹߳����´��ڹر�����������
	if (IsThreadSuspend(m_Thread))
	{
		ResumeThread(m_Thread);
	}
	MessageBox(NULL, _T("1"), _T("COM ReadFile Error"), MB_ICONERROR);
	//���ھ����Ч��������
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
		return;
	}
	MessageBox(NULL, _T("2"), _T("COM ReadFile Error"), MB_ICONERROR);
	do
	{
		SetEvent(m_hShutdownEvent);			//���� �ر���Ϣ��Ӧ�¼�
		MessageBox(NULL, _T("3"), _T("COM ReadFile Error"), MB_ICONERROR);
		//��ֹ����
		if (::PeekMessage(&message, m_pOwner, 0, 0, PM_REMOVE))			//��ȡ��Ϣ��Ӧ�¼�
		{
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
	} while (m_bThreadAlive);			//�������߳�����

	// if the port is still opened: close it 
	//�����ڴ���رմ���
	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}
	MessageBox(NULL, _T("4"), _T("COM ReadFile Error"), MB_ICONERROR);
	// Close Handles  
	//�رվ����ͬ��
	if (m_hShutdownEvent != NULL)			//�ر� ShutdownEvent
	{
		ResetEvent(m_hShutdownEvent);
	}
	if (m_ov.hEvent != NULL)
	{
		ResetEvent(m_ov.hEvent);			//�رմ�����ӦEvent
	}
	if (m_hWriteEvent != NULL)
	{
		ResetEvent(m_hWriteEvent);			//�ر�дEvent
		MessageBox(NULL, _T("5"), _T("COM ReadFile Error"), MB_ICONERROR);
		//CloseHandle(m_hWriteEvent);//�����߷�ӳ������ᵼ�¶�����ڹ���ʱ�����´򿪴����쳣
	}
	MessageBox(NULL, _T("6"), _T("COM ReadFile Error"), MB_ICONERROR);
	if (m_szWriteBuffer != NULL)			//�ͷŻ�����
	{
		delete[] m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
}

//д��char����
void CSerialPort::WriteToPort(char* string, size_t n)
{
	assert(m_hComm != 0);										//���ڱ���򿪳ɹ�
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));		/*void *memset(void *s, int ch, size_t n);
																��s�е�ǰλ�ú����n���ֽ��� ch �滻������ s ��*/

	memcpy(m_szWriteBuffer, string, n);							/*void *memcpy(void *destin, void *source, unsigned n)��
																�����Ĺ����Ǵ�Դsource�п���n���ֽڵ�Ŀ��destin�С�*/
	m_nWriteSize = n;

	// set event for write
	SetEvent(m_hWriteEvent);								//����WriteEvent�¼�����ʼ���д���д��
}

//д��BYTE����
void CSerialPort::WriteToPort(BYTE* Buffer, size_t n)
{
	assert(m_hComm != 0);										//���ڱ���򿪳ɹ�
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));		//��ʼ��д����
	memcpy(m_szWriteBuffer, Buffer, n);							//ת�����ݵ�д������
		/*��һ��ת����ʽ
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



//��ѯע���Ĵ��ںţ���ֵ����������
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

	for (i = 0; i < 20; i++)///��Ŵ��ںŵ������ʼ��
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

	//����XPϵͳ��ע���λ�ã�����ϵͳ����ʵ��������޸�
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hTestKey))
	{
		QueryKey(hTestKey);
	}
	RegCloseKey(hTestKey);

	int i = 0;
	m_PortNO.ResetContent();///ˢ��ʱ����������б�����
	while (i < MaxSerialPortNum && -1 != m_nComArray[i])
	{
		CString szCom;
		szCom.Format(_T("COM%d"), m_nComArray[i]);
		m_PortNO.InsertString(i, szCom.GetBuffer(5));
		++i;
		Flag = TRUE;
		if (Flag)///�ѵ�һ�����ֵĴ�����Ϊ�����б��Ĭ��ֵ
			m_PortNO.SetCurSel(0);
	}
}
#endif // _AFX


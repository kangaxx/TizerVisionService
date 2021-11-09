#pragma once
#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#ifndef Wm_SerialPort_MSG_BASE 
#define Wm_SerialPort_MSG_BASE        WM_USER + 617        //��Ϣ��ŵĻ���  
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

#define MaxSerialPortNum 20   //����ܹ����ʵĴ��ڸ��������Ǵ��ںš� 
#define IsReceiveString  0     //���ú��ַ�ʽ���գ�ReceiveString 1���ַ������գ���Ӧ��Ӧ����ΪWm_SerialPort_RXSTR����ReceiveString 0һ���ַ�һ���ַ����գ���Ӧ��Ӧ����ΪWm_SerialPort_RXCHAR��


#include "stdio.h"
#include "stdafx.h"
#include<windows.h>

//�Զ��崮��״̬�ṹ�壬���� portNr���ںţ�bytesRead��ȡ�����ֽ���
struct serialPortInfo
{
	UINT portNr;//���ں�
	DWORD bytesRead;//��ȡ���ֽ���
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
	// �мǣ�stopsbits = 1������ֹͣλΪ1��
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
	//���ƴ��ڼ����߳�
	BOOL         StartMonitoring();//��ʼ����
	BOOL         ResumeMonitoring();//�ָ�����
	BOOL         SuspendMonitoring();//�������
	BOOL         IsThreadSuspend(HANDLE hThread);//�ж��߳��Ƿ����hThreadΪҪ�жϵ��߳�

	DWORD         GetWriteBufferSize();//��ȡд�����С
	DWORD         GetCommEvents();//��ȡ�¼�
	DCB             GetDCB();//��ȡDCB

 //д���ݵ�����
	void        WriteToPort(char* string, size_t n);	// 
	void        WriteToPort(BYTE* Buffer, size_t n);	// 
	void		ClosetoPort();						// 
	BOOL        IsOpen();

	void QueryKey(HKEY hKey);///��ѯע���Ĵ��ںţ���ֵ����������

#ifdef _AFX
	void Hkey2ComboBox(CComboBox& m_PortNO);			//��QueryKey��ѯ���Ĵ��ں���ӵ�CComboBox�ؼ���
#endif // _AFX

protected:
	// protected memberfunctions///
	void        ProcessErrorMessage(TCHAR* ErrorText);	//������
	static DWORD WINAPI CommThread(LPVOID pParam);		//�̺߳���
	static void    ReceiveChar(CSerialPort* port);
	static void ReceiveStr(CSerialPort* port);			//
	static void    WriteChar(CSerialPort* port);

	//thread

	HANDLE              m_Thread;					//�����߳̾��
	BOOL                m_bIsSuspened;				//thread�����߳��Ƿ����

	//synchronisation objects///
	CRITICAL_SECTION    m_csCommunicationSync;		//�ٽ���Դ
	BOOL                m_bThreadAlive;				//�����߳����б�־��1�߳����У�0�̹߳���

/// ����handles///
	HANDLE              m_hShutdownEvent;			//�ر��¼���Ӧ ���
	HANDLE              m_hComm;					//�󶨵Ĵ��ھ�� 
	HANDLE              m_hWriteEvent;				//д�¼�������������SetEvent(m_hWriteEvent)���ã����ʾ��ʼ����д�롣
	HANDLE				m_hEventArray[3];			//�趨�¼�������飬���ڶ������ȼ�

///����ṹ��/
	OVERLAPPED          m_ov;						//�����첽I/O�ṹ��OVERLAPPED��֮������m_ov���õײ��첽�������
	/*OVERLAPPED�ṹ�����
	typedef struct _OVERLAPPED {
		DWORD Internal;			//Ԥ��������ϵͳʹ�á���ָ��һ��������ϵͳ��״̬,��GetOverlappedResult��������ʱû��������չ������ϢERROR_IO_PENDINGʱ��Ч��
		DWORD InternalHigh;		//Ԥ��������ϵͳʹ�á���ָ�����ȵ�����ת��,��GetOverlappedResult��������TRUEʱ��Ч��
		DWORD Offset;			//���ļ���λ���Ǵ��ļ���ʼ�����ֽ�ƫ���������ý������������Ա֮ǰ����ReadFile��WriteFile����������ȡ��д�������ܵ���ͨ���豸ʱ�����Ա��������Ϊ�㡣
		DWORD OffsetHigh;		//ָ���ļ����͵��ֽ�ƫ�����ĸ�λ�֡�����ȡ��д�������ܵ���ͨ���豸ʱ�����Ա��������Ϊ�㡣
		HANDLE hEvent;			//��ת�����ʱ���¼�����Ϊ���ź�״̬�����ý��̼������Ա�ڵ���ReadFile�� WriteFile��TransactNamedPipe�� ConnectNamedPipe����֮ǰ��
	} OVERLAPPED*/
	COMMTIMEOUTS        m_SerialPortTimeouts;		//��ʱ����
	/*COMMTIMEOUTS �ṹ�����
		typedef struct _COMMTIMEOUTS {
		DWORD ReadIntervalTimeout;			//���ַ�֮��������ʱ������ȡ��������ʱ��һ�������ַ������ʱ������ʱ�䣬��ȡ�������������е����ݡ�
		DWORD ReadTotalTimeoutMultiplier;	//��ȡÿ�ַ���ĳ�ʱ�� ָ���Ժ���Ϊ��λ���ۻ�ֵ�����ڼ��������ʱ�ĳ�ʱ����������ÿ�ζ���������ֵ����Ҫ�����ֽ�����ˡ�
		DWORD ReadTotalTimeoutConstant;		//һ�ζ�ȡ�������ݵĹ̶���ʱ��
		DWORD WriteTotalTimeoutMultiplier;	//д��ÿ�ַ���ĳ�ʱ
		DWORD WriteTotalTimeoutConstant;	//һ��д�봮�����ݵĹ̶���ʱ��
	} COMMTIMEOUTS, *LPCOMMTIMEOUTS;*/

	DCB                 m_dcb;						//�豸���ƿ顣����ͨ�Ż������ýṹ��
	/*DCB�ṹ�嶨��
	typedef struct _DCB {
		DWORD DCBlength;				//DCB�ṹ��С����sizeof(DCB)���ڵ���SetCommState������DCBǰ����������
		DWORD BaudRate;					//ָ����ǰ���õĲ����ʣ�Ӧ�������ӵ�ͨѶ�豸��ƥ��
		DWORD fBinary : 1;				//ָ���Ƿ����������ģʽ��Win32 API��֧�ַǶ�����ģʽ���䣬Ӧ����Ϊtrue
		DWORD fParity : 1;				//ָ����żУ���Ƿ�������Ϊtrueʱ������ú���У�鿴Parity ����
		DWORD fOutxCtsFlow : 1;			//�Ƿ���CTS(clear-to-send)�ź�����������ء�������Ϊtrueʱ�� ��CTSΪ�͵�ƽ�������ݷ��ͽ�������ֱ��CTS��Ϊ�ߡ�
		DWORD fOutxDsrFlow : 1;			//
		DWORD fDtrControl : 2;			//
		DWORD fDsrSensitivity : 1;		//
		DWORD fTXContinueOnXoff : 1;	//
		DWORD fOutX : 1;				// XON/XOFF ���������ڷ���ʱ�Ƿ���á�
		DWORD fInX : 1;					//XON/XOFF ���������ڽ���ʱ�Ƿ���á�
		DWORD fErrorChar : 1;			//��ֵΪTRUE������ErrorCharָ�����ַ�������żУ�����Ľ����ַ�
		DWORD fNull : 1;				//ΪTRUEʱ������ʱ�Զ�ȥ���գ�0ֵ���ֽ�
		DWORD fRtsControl : 2;			//
		DWORD fAbortOnError : 1;		//��д������������ʱ�Ƿ�ȡ��������������Ϊtrue���򵱷�����д����ʱ����ȡ�����ж�д����
		DWORD fDummy2 : 17;				//
		WORD wReserved;					//δ���ã���������Ϊ0
		WORD XonLim;					//��XON�ַ�����ǰ���ջ������ڿ��������С�ֽ���
		WORD XoffLim;					//
		BYTE ByteSize;					//
		BYTE Parity;					//ָ���˿����ݴ����У�鷽����
		BYTE StopBits;					//ָ���˿ڵ�ǰʹ�õ�ֹͣλ������ȡֵ
		char XonChar;					//ָ��XON�ַ�
		char XoffChar;					//ָ��XOFF�ַ�
		char ErrorChar;					//ָ��ErrorChar�ַ���������յ�����żУ�鷢������ʱ���ֽڣ�
		char EofChar;					//ָ�����ڱ�ʾ���ݽ������ַ�
		char EvtChar;					//
		WORD wReserved1;				//������δ����
	} DCB;*/


	// owner window//

	HWND                m_pOwner;				//���ڰ󶨵Ĵ��ھ��

	// misc
	UINT                m_nPortNr;				//���ں�
	PBYTE               m_szWriteBuffer;		//д������ָ�롣
	DWORD               m_dwCommEvents;			//���崮���¼�
	DWORD               m_nWriteBufferSize;		//д�����С
	size_t              m_nWriteSize;			//д���ֽ���
};

#endif __SERIALPORT_H__


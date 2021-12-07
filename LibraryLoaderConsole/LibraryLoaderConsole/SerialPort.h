/*
Module : SerialPort.h
Purpose: Interface for an C++ wrapper class for serial ports

Copyright (c) 1999 - 2016 by PJ Naughter.  

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////// Macros / Structs etc ////////////////////////////////////

#pragma once

#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#ifndef CSERIALPORT_EXT_CLASS
#define CSERIALPORT_EXT_CLASS
#endif //#ifndef CSERIALPORT_EXT_CLASS

#ifndef _Out_writes_bytes_
#define _Out_writes_bytes_(X)
#endif //#ifndef _Out_writes_bytes_

#ifndef __out_data_source
#define __out_data_source(X)
#endif //#ifndef __out_data_source

#ifndef _Out_writes_bytes_to_opt_
#define _Out_writes_bytes_to_opt_(X,Y)
#endif //#ifndef _Out_writes_bytes_to_opt_

#ifndef _Out_writes_bytes_opt_
#define _Out_writes_bytes_opt_(X)
#endif //#ifndef _Out_writes_bytes_opt_

#ifndef _In_reads_bytes_opt_
#define _In_reads_bytes_opt_(X)
#endif //#ifndef _In_reads_bytes_opt_

#ifndef _In_
#define _In_
#endif //#ifndef _In_

#ifndef _In_z_
#define _In_z_
#endif //#ifndef _In_z_

#ifndef _Inout_opt_
#define _Inout_opt_
#endif //#ifndef _Inout_opt_

#ifndef _Out_opt_
#define _Out_opt_
#endif //#ifndef _Out_opt_

#ifndef _Out_
#define _Out_
#endif //#ifndef _Out_

#ifndef _Inout_
#define _Inout_
#endif //#ifndef _Inout_

#ifndef _In_opt_
#define _In_opt_
#endif //#ifndef _In_opt_


////////////////////////// Includes ///////////////////////////////////////////

//#include <WinDef.h>
//#include <WinBase.h>

#include <windows.h>
#include <sal.h>
#include <time.h>
//
#ifndef CSERIALPORT_MFC_EXTENSIONS
#include <exception>
#include <string>
#endif //#ifndef CSERIALPORT_MFC_EXTENSIONS

//#include "commonfunction_c.h"




/////////////////////////// Classes ///////////////////////////////////////////

#ifdef CSERIALPORT_MFC_EXTENSIONS
class CSERIALPORT_EXT_CLASS CSerialException : public CException
#else
class CSERIALPORT_EXT_CLASS CSerialException : public std::exception
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS
{
public:
	//Constructors / Destructors
	CSerialException(unsigned long dwError);

	//Methods
#ifdef CSERIALPORT_MFC_EXTENSIONS
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif //#ifdef _DEBUG
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS

#if _MSC_VER >= 1700
	virtual bool GetErrorMessage(_Out_z_cap_(nMaxError)  char* lpszError, _In_ unsigned int nMaxError, _Out_opt_ unsigned int* pnHelpContext = NULL);
#else	
	virtual bool GetErrorMessage(__out_ecount_z(nMaxError) const char* lpszError, __in unsigned int nMaxError, __out_opt unsigned int* pnHelpContext = NULL);
#endif

#ifdef CSERIALPORT_MFC_EXTENSIONS
	CString GetErrorMessage();
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS

	//Data members
	unsigned long m_dwError;
};


class CSERIALPORT_EXT_CLASS CSerialPort
{
public:
	//Enums
	enum FlowControl
	{
		NoFlowControl,
		CtsRtsFlowControl,
		CtsDtrFlowControl,
		DsrRtsFlowControl,
		DsrDtrFlowControl,
		XonXoffFlowControl
	};

	enum Parity
	{
		NoParity = 0,
		OddParity = 1,
		EvenParity = 2,
		MarkParity = 3,
		SpaceParity = 4
	};

	enum StopBits
	{
		OneStopBit,
		OnePointFiveStopBits,
		TwoStopBits
	};

	//Constructors / Destructors
	CSerialPort();
	virtual ~CSerialPort();

	//General Methods
	void Open(_In_ int nPort, _In_ unsigned long dwBaud = 115200, _In_ Parity parity = NoParity, _In_ unsigned char DataBits = 8,
		_In_ StopBits stopBits = OneStopBit, _In_ FlowControl fc = NoFlowControl, _In_ bool bOverlapped = false);
	void Open(_In_z_ const TCHAR* pszPort, _In_ unsigned long dwBaud = 115200, _In_ Parity parity = NoParity, _In_ unsigned char DataBits = 8,
		_In_ StopBits stopBits = OneStopBit, _In_ FlowControl fc = NoFlowControl, _In_ bool bOverlapped = false);
	void Close();
	void Attach(_In_ HANDLE hComm);
	HANDLE Detach();
	operator HANDLE() const { return m_hComm; };
	bool IsOpen() const { return m_hComm != INVALID_HANDLE_VALUE; };

#ifdef CSERIALPORT_MFC_EXTENSIONS
#ifdef _DEBUG
	void Dump(_In_ CDumpContext& dc) const;
#endif //#ifdef _DEBUG
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS

	//Reading / Writing Methods
	unsigned long Read(_Out_writes_bytes_(dwNumberOfBytesToRead) __out_data_source(FILE) void* lpBuffer, _In_ unsigned long dwNumberOfBytesToRead);
	void  Read(_Out_writes_bytes_to_opt_(dwNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) void* lpBuffer, _In_ unsigned long dwNumberOfBytesToRead, _In_ OVERLAPPED& overlapped, _Inout_opt_ unsigned long* lpNumberOfBytesRead = NULL);
	void  ReadEx(_Out_writes_bytes_opt_(dwNumberOfBytesToRead) __out_data_source(FILE) LPVOID lpBuffer, _In_ unsigned long dwNumberOfBytesToRead, _Inout_ LPOVERLAPPED lpOverlapped, _In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	unsigned long Write(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) const void* lpBuffer, _In_ unsigned long dwNumberOfBytesToWrite);
	void  Write(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) const void* lpBuffer, _In_ unsigned long dwNumberOfBytesToWrite, _In_ OVERLAPPED& overlapped, _Out_opt_ unsigned long* lpNumberOfBytesWritten = NULL);
	void  WriteEx(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) LPCVOID lpBuffer, _In_ unsigned long dwNumberOfBytesToWrite, _Inout_ LPOVERLAPPED lpOverlapped, _In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	void  TransmitChar(_In_ char cChar);
	void  GetOverlappedResult(_In_ OVERLAPPED& overlapped, _Out_ unsigned long& dwBytesTransferred, _In_ bool bWait);
	void  CancelIo();
	unsigned long BytesWaiting();

	//Configuration Methods
	void GetConfig(_In_ COMMCONFIG& config);
	static void GetDefaultConfig(_In_ int nPort, _Out_ COMMCONFIG& config);
	static void GetDefaultConfig(_In_z_ const char* pszPort, _Out_ COMMCONFIG& config);
	void SetConfig(_In_ COMMCONFIG& Config);
	static void SetDefaultConfig(_In_ int nPort, _In_ COMMCONFIG& config);
	static void SetDefaultConfig(_In_z_ const char* pszPort, _In_ COMMCONFIG& config);

	//Misc RS232 Methods
	void ClearBreak();
	void SetBreak();
	void ClearError(_Out_ unsigned long& dwErrors);
	void GetStatus(_Out_ COMSTAT& stat);
	void GetState(_Out_ DCB& dcb);
	void SetState(_In_ DCB& dcb);
	void Escape(_In_ unsigned long dwFunc);
	void ClearDTR();
	void ClearRTS();
	void SetDTR();
	void SetRTS();
	void SetXOFF();
	void SetXON();
	void GetProperties(_Inout_ COMMPROP& properties);
	void GetModemStatus(_Out_ unsigned long& dwModemStatus);

	//Timeouts
	void SetTimeouts(_In_ COMMTIMEOUTS& timeouts);
	void GetTimeouts(_Out_ COMMTIMEOUTS& timeouts);
	void Set0Timeout();
	void Set0WriteTimeout();
	void Set0ReadTimeout();

	//Event Methods
	void SetMask(_In_ unsigned long dwMask);
	void GetMask(_Out_ unsigned long& dwMask);
	void WaitEvent(_Inout_ unsigned long& dwMask);
	bool WaitEvent(_Inout_ unsigned long& dwMask, _Inout_ OVERLAPPED& overlapped);

	//Queue Methods
	void Flush();
	void Purge(_In_ unsigned long dwFlags);
	void TerminateOutstandingWrites();
	void TerminateOutstandingReads();
	void ClearWriteBuffer();
	void ClearReadBuffer();
	void Setup(_In_ unsigned long dwInQueue, _In_ unsigned long dwOutQueue);

	//Static methods
	static void ThrowSerialException(_In_ unsigned long dwError = 0);

protected:
	//Member variables
	HANDLE m_hComm;  //Handle to the comms port

	//jiang.j.q add
	bool m_bOverlapped;
};

#endif //#ifndef __SERIALPORT_H__

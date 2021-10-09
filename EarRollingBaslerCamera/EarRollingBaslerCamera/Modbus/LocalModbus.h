// LocalModbus.h: interface for the CLocalModbus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCALMODBUS_H__C08841E2_BB2E_11D1_8FE9_E4CD05C10000__INCLUDED_)
#define AFX_LOCALMODBUS_H__C08841E2_BB2E_11D1_8FE9_E4CD05C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "modbus.h"
#include "windows.h"
#include "mmsystem.h"
#include "crc16.h"

class CLocalModbus;

class CLocalModbus : public CModbus  
{

public:
	void ByteSize(WORD wSize);
	BYTE HiLo4BitsToByte(BYTE byHi, BYTE byLo);
	BOOL VerifyRespLRC(BYTE* aResp, DWORD dwLength);
	BYTE Num2Ascii(BYTE byNum);
	void RTU2ASCII(const BYTE* aRtu ,int iSize , BYTE* aAscii );
	WORD TransmissionMode();
	void TransmissionMode(WORD wMode);
	CLocalModbus();
	virtual ~CLocalModbus();

	BOOL UpdateSerialConfig(BYTE byComPort=0,    //com port
		DWORD dwBaudRate=0, // Baudrate 9600,4800 actual values  
		BYTE byParity=INVALID_PARITY);   //Parity EVENPARITY or ODDPARITY or NOPARITY DCB definitions 

	//Set/Get  Comm Timetouts Use COMMTIMEOUTS structure from win32 API 
	BOOL SetTimeouts(COMMTIMEOUTS* timeouts_p );
	BOOL GetTimeouts( COMMTIMEOUTS& timeouts );

	//return string error message 
	//in:wErrorCode constant errro code 
	//out:string with error 
	string ErrorMessage(WORD wErrorCode); 

	BOOL CloseCommPort(); //close serial port 

	BOOL IsActive(); //return true if serial port opened

	//used for seralization the object to file 
	//void Serialize( CArchive& archive );

	//properties 

	//time-out 
	DWORD Timeout() const;
	void  Timeout(DWORD dwTimeout);

	//end message time 
	DWORD SilentInterval() const;
	void  SilentInterval(DWORD dwSilentInterval);

	//com port 
	BYTE ComPort(); //used com port 1, 2 ...

	WORD ByteSize(); //8,7


	//transmit a query to a modbus device and wait for a response
	//return ERR_XXX Code see Modbus.cpp file  
	WORD TxRxMessage(BYTE* abyQuery, //modbus query without CRC
		WORD wLengthQuery, //QueryLength without CRC
		BYTE* abyResponse, //modbus Response without CRC
		WORD wLengthResponse, //wLengthResponse Response without CRC
		WORD* pwNumOfBytesRead=NULL);

	//Transmit and Recive Raw Data to serial port 
	WORD TxRxRawData(const BYTE* pDataWrite,
		WORD wLengthData,
		BYTE* pRespBuffer,
		WORD  wMaxBufferSize,
		WORD& wNumOfBytesRead,
		DWORD dwTimeOut=0
		);

	void ComPort(BYTE byComPort);

	//flow contlol 
	/*  FC_NONE,      //None
	FC_DTRDSR     //DTR/DSR 
	FC_RTSCTS     //RTS/CTS
	FC_XONXOFF    XOn/Xoff
	*/
	BYTE FlowControl(); //flow contlol 

	void FlowControl(BYTE byFlowControl);


	/*ONESTOPBIT  1 stop bit 
	ONE5STOPBITS 1.5 stop bits 
	TWOSTOPBITS 2 stop bits 
	*/

	BYTE StopBits(); //flow contlol 

	void StopBits(BYTE byStopBits);

	//baudrate 
	DWORD BaudRate(); //actual baudrate value 
	void BaudRate(DWORD dwBaudRate);
	//parity 
	BYTE  Parity(); // windows dcb parity defitions 
	void   Parity(BYTE byParity); // windows dcb parity defitions  

	//constants 

	//flow control
	enum _FlowControl_ {
		FC_NONE=0,
		FC_DTRDSR=0x01,
		FC_RTSCTS=0x02,
		FC_XONXOFF=0x04,
		ASCII_XON=0x11,
		ASCII_XOFF=0x13
	};

	enum _general_coinstants_{
		MODE_RTU=0,
		MODE_ASCII=1,
		C_R=13,
		L_F=10,
		INVALID_PARITY=255
	};

private:
	WORD m_wByteSize;
	BYTE Ascii2Num(BYTE byChar);


	HANDLE m_hComHandle; //handle da porta com 
	BOOL  ClearBuffers(); //Clear RxTxBuffers 

	//return true if succses
	BOOL WriteQuery(BYTE* abyQuery, //Modbus Query with crc 
		DWORD dwQuerySize,  //Query Size 	
		DWORD& dwNumberOfBytesWritten //Number of bytes written to serial port 
		);

	BOOL ReadResponseASCII(BYTE* abyResponse, //Modbus Response with crc
		DWORD& dwNeedRead, //Number of bytes we need.
		DWORD& dwRead); //Number of bytes read from serial port

	BOOL ReadResponseRTU(BYTE* abyResponse, //Modbus Response with crc
		DWORD& dwNeedRead, //Number of bytes we need.
		DWORD& dwRead); //Number of bytes read from serial port

	WORD TxRxMessageAscii(const BYTE* abyQuery, //modbus query without CRC
		WORD wLengthQuery, //QueryLength without CRC
		BYTE* abyResponse, //modbus Response without CRC
		WORD wLengthResponse, //wLengthResponse Response without CRC
		WORD* pwNumOfBytesRead=NULL);


	//verify modbus response
	//return true if ok 
	//abyResp Modbus response 
	//dwLength Length of response without CRC 
	BOOL VerifyRespCRC(const BYTE* abyResp, WORD wLength);


	//Calc LRC of resp Message 
	BYTE LRCASCII(const BYTE *auchMsgASCII, WORD usDataLen);

	DWORD m_lTimeOut; //time-out miliseconds 
	DWORD m_dwSilentInterval; //modbus Silent Interval 4 charaters

	CRC16 m_crc;
	BYTE  m_ComPort;
	DWORD m_BaudRate;
	BYTE  m_byParity;
	BYTE  m_byFlowControl;
	BYTE  m_byStopBits;
	WORD  m_wTranmitionMode;
	//QMyByteArray m_abyBuffer;
	BYTE m_abyBuffer[RESP_BUFFER*2];
};



#endif // !defined(AFX_LOCALMODBUS_H__C08841E2_BB2E_11D1_8FE9_E4CD05C10000__INCLUDED_)

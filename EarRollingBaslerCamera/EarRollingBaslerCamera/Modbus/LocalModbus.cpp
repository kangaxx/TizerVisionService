// LocalModbus.cpp: implementation of the CLocalModbus class.
//
//////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "LocalModbus.h"
//#include "LogProgram.h"
//#include "Global.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLocalModbus::CLocalModbus()
{
	m_hComHandle=INVALID_HANDLE_VALUE;
	m_lTimeOut=1000;
	m_ComPort=2;
	m_byParity=NOPARITY; 
	m_BaudRate=9600;
	m_dwSilentInterval=0;
	FlowControl(FC_NONE);
	StopBits(ONESTOPBIT);
	m_wByteSize=8;
	m_wTranmitionMode=MODE_RTU;
	//m_abyBuffer.SetSize(RESP_BUFFER*2);

}

CLocalModbus::~CLocalModbus()
{
	CloseCommPort();
}

//properties 

BOOL CLocalModbus::SetTimeouts(COMMTIMEOUTS* timeouts_p )
{
	ASSERT( m_hComHandle!=INVALID_HANDLE_VALUE );

	// Choose to live

	if ( m_hComHandle ==INVALID_HANDLE_VALUE )
	{
		return( FALSE );
	}

	BOOL return_value = FALSE;

	COMMTIMEOUTS communications_timeouts;

	::ZeroMemory( &communications_timeouts, sizeof( communications_timeouts ) );

	if ( timeouts_p != NULL )
	{
		// We were passed a pointer, do not trust it

		try
		{
			communications_timeouts.ReadIntervalTimeout         = timeouts_p->ReadIntervalTimeout;
			communications_timeouts.ReadTotalTimeoutMultiplier  = timeouts_p->ReadTotalTimeoutMultiplier;
			communications_timeouts.ReadTotalTimeoutConstant    = timeouts_p->ReadTotalTimeoutConstant;
			communications_timeouts.WriteTotalTimeoutMultiplier = timeouts_p->WriteTotalTimeoutMultiplier;
			communications_timeouts.WriteTotalTimeoutConstant   = timeouts_p->WriteTotalTimeoutConstant;
		}
		catch( ... )
		{
			communications_timeouts.ReadIntervalTimeout         = MAXDWORD;
			communications_timeouts.ReadTotalTimeoutMultiplier  = 0;
			communications_timeouts.ReadTotalTimeoutConstant    = 0;
			communications_timeouts.WriteTotalTimeoutMultiplier = 0;
			communications_timeouts.WriteTotalTimeoutConstant   = 0;
		}
	}
	else
	{
		// Use our defaults
		communications_timeouts.ReadIntervalTimeout         = MAXDWORD;
		communications_timeouts.ReadTotalTimeoutMultiplier  = 0;
		communications_timeouts.ReadTotalTimeoutConstant    = 0;
		communications_timeouts.WriteTotalTimeoutMultiplier = 0;
		communications_timeouts.WriteTotalTimeoutConstant   = 0;
	}

	return_value = ::SetCommTimeouts( (HANDLE) m_hComHandle, &communications_timeouts );

	return( return_value );
}

BOOL CLocalModbus::GetTimeouts( COMMTIMEOUTS& timeouts )
{
	ASSERT( m_hComHandle != INVALID_HANDLE_VALUE );

	// Choose to live

	if ( m_hComHandle ==INVALID_HANDLE_VALUE )
	{
		::ZeroMemory( &timeouts, sizeof( COMMTIMEOUTS ) );
		return( FALSE );
	}

	BOOL return_value = FALSE;

	return_value = ::GetCommTimeouts(m_hComHandle, &timeouts );

	return( return_value );
}


DWORD CLocalModbus::Timeout() const {
	return m_lTimeOut;
}


void CLocalModbus::Timeout(DWORD dwTimeout){
	m_lTimeOut=dwTimeout;
}


DWORD CLocalModbus::SilentInterval() const {
	return m_dwSilentInterval;
}

void CLocalModbus::SilentInterval(DWORD dwSilentInterval ){
	if (dwSilentInterval>0){
		m_dwSilentInterval=dwSilentInterval;
	}
}

BYTE CLocalModbus::ComPort()
{
	return m_ComPort;  

}


void CLocalModbus::ComPort(BYTE byComPort)
{
	m_ComPort=byComPort;  

}

BYTE CLocalModbus::FlowControl() {

	return  m_byFlowControl;

}

void CLocalModbus::FlowControl(BYTE byFlowControl){

	m_byFlowControl= byFlowControl;

}

BYTE CLocalModbus::StopBits() {

	return  m_byStopBits;

}

void CLocalModbus::StopBits(BYTE byStopBits){

	m_byStopBits= byStopBits;

}


DWORD CLocalModbus::BaudRate()
{

	return m_BaudRate;
}

void CLocalModbus::BaudRate(DWORD dwBaudRate)
{

	m_BaudRate=dwBaudRate;
}

BYTE CLocalModbus::Parity()
{
	return m_byParity;

}

void  CLocalModbus::Parity(BYTE byParity) {

	m_byParity = byParity;
}

//close serial port 
BOOL CLocalModbus::CloseCommPort(){
	if (m_hComHandle!=INVALID_HANDLE_VALUE){
		if (::CloseHandle(m_hComHandle)) {
			m_hComHandle=INVALID_HANDLE_VALUE;
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return (TRUE);
	}
}

BOOL CLocalModbus::UpdateSerialConfig(BYTE byComPort,  //com port
									  DWORD dwBaudRate, // Baudrate 9600,4800 actual values  
									  BYTE byParity){    //Parity EVENPARITY or ODDPARITY or NOPARITY DCB definitions 				  	

										  // set timeouts
										  //CString sPort;
										  wchar_t sPort[20];
										  char txt[128];
										  COMMTIMEOUTS cto;
										  DCB dcb;
										  DWORD dwSilentInterval;
										  BYTE bSet;

										  Lock();

										  if (!CloseCommPort()){
											  LOG_PROGRAM(LL_ERROR, "Can't close comm port");
											  goto OpenCommError;
										  }

										  if (0==byComPort){
											  byComPort=m_ComPort;
										  }

										  if ((byComPort>=1)&&(byComPort<=MAX_COM_PORTS)){
											  m_ComPort=byComPort;
										  }
										  else {
											  goto OpenCommError;
										  }

										  swprintf_s(sPort ,L"COM%d", m_ComPort);

										  m_hComHandle = ::CreateFileW(sPort,
											  GENERIC_READ|GENERIC_WRITE,
											  0,
											  NULL,
											  OPEN_EXISTING,
											  0,
											  NULL);

										  if(m_hComHandle == INVALID_HANDLE_VALUE) {
											  DWORD err = GetLastError();
											  //LOG_PROGRAM(LL_ERROR, QString("Open COM%1 port error:%2\n").arg(m_ComPort).arg(err));
											  sprintf_s(txt, "Open COM%d port error:%d\n", m_ComPort, err);
											  LOG_PROGRAM(LL_INFO, txt);
											  //sPort.Format(_T("\\\\.\\COM%d"),m_ComPort); //Try NT Format 
											  swprintf_s(sPort, L"\\\\.\\COM%d", m_ComPort);

											  m_hComHandle = ::CreateFileW(sPort,
												  GENERIC_READ|GENERIC_WRITE,
												  0,
												  NULL,
												  OPEN_EXISTING,
												  0,
												  NULL);

											  if(m_hComHandle == INVALID_HANDLE_VALUE) {
												  LOG_PROGRAM(LL_ERROR, "CreateFile Failed");
												  goto OpenCommError;
											  } 
										  }

										  if(!::SetupComm(
											  m_hComHandle,	// handle of communications device
											  INPUT_BUFFER_SIZE,	// size of input buffer
											  OUTPUT_BUFFER_SIZE	// size of output buffer
											  )
											  ){
												  LOG_PROGRAM(LL_ERROR, "SetupComm failed");
												  goto OpenCommError;
										  }

										  //missinig verify baudrate and pairty *****

										  // set DCB
										  if (0==dwBaudRate) {
											  //m_byParity=dcb.Parity; 
											  dwBaudRate =m_BaudRate;
										  }

										  if (INVALID_PARITY==byParity) {
											  byParity=m_byParity;
										  }


										  /////////////////////////////////////////////////////////////////////////
										  // 	 BOOL fSuccess = GetCommState(m_hComHandle, &dcb);
										  // 	 
										  // 	 if (!fSuccess) {
										  // 		 // Handle the error.
										  // 		 printf ("GetCommState failed with error %d.\n", GetLastError());
										  // 		 goto OpenCommError;
										  // 	 }
										  ///////////////////////////////////////////////////////////////////////////

										  ::memset(&dcb,0,sizeof(dcb));
										  dcb.DCBlength = sizeof(dcb);
										  dcb.BaudRate = dwBaudRate;
										  dcb.fBinary = TRUE;
										  dcb.StopBits = StopBits();
										  dcb.Parity = byParity;
										  dcb.ByteSize = BYTE(m_wByteSize);

										  bSet=(BYTE) ((FlowControl()& FC_DTRDSR)!=0);
										  dcb.fOutxDsrFlow = bSet ;

										  if (bSet)
											  dcb.fDtrControl = DTR_CONTROL_HANDSHAKE ;
										  else
											  dcb.fDtrControl = DTR_CONTROL_ENABLE ;

										  bSet = (BYTE) ((FlowControl()& FC_RTSCTS) != 0) ;
										  dcb.fOutxCtsFlow = bSet ;

										  if (bSet)
											  dcb.fRtsControl = RTS_CONTROL_HANDSHAKE ;
										  else
											  dcb.fRtsControl = RTS_CONTROL_ENABLE ;

										  // setup software flow control

										  bSet = (BYTE) ((FlowControl()&FC_XONXOFF) != 0) ;

										  dcb.fInX = dcb.fOutX = bSet ;
										  dcb.XonChar = ASCII_XON ;
										  dcb.XoffChar = ASCII_XOFF ;
										  dcb.XonLim = 100 ;
										  dcb.XoffLim = 100 ;

										  dwSilentInterval= (DWORD)((dcb.ByteSize+4)*(4)*1000L)/(dcb.BaudRate);
										  m_byParity=dcb.Parity; 
										  m_BaudRate=dcb.BaudRate;

										  if (dwSilentInterval>m_dwSilentInterval) {
											  SilentInterval(dwSilentInterval);
										  }

										  cto.ReadIntervalTimeout=SilentInterval();//Max Char Interval 
										  cto.ReadTotalTimeoutMultiplier=(DWORD)((dcb.ByteSize+4)*(1)*1000L)/(dcb.BaudRate);

										  if (cto.ReadTotalTimeoutMultiplier<=0){ 
											  cto.ReadTotalTimeoutMultiplier=1;
										  }

										  cto.ReadTotalTimeoutConstant=m_lTimeOut;
										  cto.WriteTotalTimeoutMultiplier=SilentInterval();
										  cto.WriteTotalTimeoutConstant=m_lTimeOut;


										  if (MODE_RTU==m_wTranmitionMode){
											  if(!SetTimeouts(&cto)){
												  LOG_PROGRAM(LL_ERROR, "SetTimeouts failed ");
												  goto OpenCommError;
											  }
										  }
										  else {	
											  if(!SetTimeouts(NULL)){
												  LOG_PROGRAM(LL_ERROR, "SetTimeouts failed ");
												  goto OpenCommError;
											  }
										  }


										  if(!::SetCommState(m_hComHandle,&dcb)){
											  DWORD dwError = GetLastError();
											  LOG_PROGRAM(LL_ERROR, "SetCommState failed ");
											  goto OpenCommError;
										  }

										  Unlock();
										  return (TRUE);

OpenCommError:
										  Unlock();
										  return (FALSE);	 
}

BOOL CLocalModbus::IsActive() {
	if (m_hComHandle!=INVALID_HANDLE_VALUE) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}


//void CLocalModbus::Serialize( CArchive& archive )
//{
//	// call base class function first
//	// base class is CObject in this case
//	CModbus::Serialize( archive );
//
//	BOOL bIsActive;	
//	// now do the stuff for our specific class
//	if( archive.IsStoring() ){
//		archive << m_dwSilentInterval << m_lTimeOut;
//		archive <<m_ComPort<<m_byParity<<m_BaudRate<<IsActive();
//		archive <<m_byStopBits<<m_byFlowControl<<m_wTranmitionMode<<m_wByteSize;
//
//	} 
//	else {
//
//		archive >> m_dwSilentInterval >> m_lTimeOut >> m_ComPort >> m_byParity >> m_BaudRate >> bIsActive;
//		archive >>m_byStopBits >> m_byFlowControl>>m_wTranmitionMode>>m_wByteSize;
//
//		if (bIsActive) {
//			UpdateSerialConfig(m_ComPort,  //com port
//				m_BaudRate, // Baudrate 9600,4800 actual values  
//				m_byParity);
//
//		}
//
//	}
//
//}

WORD CLocalModbus::TxRxMessage(BYTE* abyQuery, //modbus query without CRC
	WORD wLengthQuery, //QueryLength without CRC
	BYTE* abyResponse, //modbus Response without CRC
	WORD wLengthResponse, WORD* pwNumOfBytesRead) { //wLengthResponse Response without CRC

	WORD wcrc;
	DWORD  dwNumberOfBytesWritten;

	//QMyByteArray m_abyBuffer;
	DWORD dwNumberOfBytesRead;
	WORD wRespSize;
	WORD wError;

	//move by gxx
	BYTE hi;
	BYTE low;
	DWORD dwNeed;
	//DWORD ticCountStart;

	//Fill response 
	/* liu.k.x
	if (abyResponse.GetSize()<wLengthResponse) {
		abyResponse.SetSize(wLengthResponse);
	}
	*/

	//lock com device  
	if (!Lock()) {
		LOG_PROGRAM(LL_ERROR, "Lock Failed");
		return(ERR_LOCK_TIME_OUT);
	}

	if (m_hComHandle == INVALID_HANDLE_VALUE) {
		LOG_PROGRAM(LL_ERROR, "Port Not opened");

		wError = (ERR_NOT_INT);
		goto TxRxError;
	}


	if (MODE_ASCII == m_wTranmitionMode) {
		wError = TxRxMessageAscii(abyQuery, wLengthQuery, abyResponse, wLengthResponse, pwNumOfBytesRead);
		Unlock();
		return wError;
	}


	//Add CRC16 to query 
	wcrc = m_crc.CalcCrcFast(abyQuery, wLengthQuery);

	/* liu.k.x
	if (abyQuery.GetSize()<(wLengthQuery+2)) {
		abyQuery.SetSize(wLengthQuery+2);
	}*/
	hi = HIBYTE(wcrc);
	low = LOBYTE(wcrc);

	abyQuery[wLengthQuery] = HIBYTE(wcrc);
	abyQuery[wLengthQuery + 1] = LOBYTE(wcrc);


	if (!ClearBuffers()) {
		LOG_PROGRAM(LL_ERROR, "ClearBuffers Error");
	};

	//ticCountStart = ::GetTickCount();
	//write query 
	if (!WriteQuery(abyQuery, wLengthQuery + 2, dwNumberOfBytesWritten)) {
		LOG_PROGRAM(LL_ERROR, "Write Query Error");
		wError = (ERR_WR_PORT);
		goto TxRxError;
	}
	// DWORD time = (::GetTickCount() - ticCountStart);
	// if(time>0)
	// {
		  //qDebug("WriteQuery:%dms\n", time);
	// }


	 //read response 
	 //m_abyBuffer.SetSize(RESP_BUFFER); //fill temp buffer   liu.k.x
	dwNeed = wLengthResponse + 2;
	//m_abyBuffer.SetSize(wLengthResponse + 2);


	//liu.k.x
	//for(int i=0; i<m_abyBuffer.size();i++)
	//{
	//   m_abyBuffer[i] = 0;
	//}
	::ZeroMemory(m_abyBuffer, dwNeed);

	//ticCountStart = ::GetTickCount();
	//if (!ReadResponse(m_abyBuffer,dwNumberOfBytesRead,wLengthResponse+2)) //using COMMTIMEOUTS
	if (!ReadResponseRTU(m_abyBuffer, dwNeed, dwNumberOfBytesRead))
	{
		//qDebug("ReadResponse Failed"); 
		wError = (ERR_RD_PORT);
		goto TxRxError;

	}
	//time = (::GetTickCount() - ticCountStart);
	//if(time>0)
	//{
	   // qDebug("ReadResponseRTU:%dms\n", time);
	//}

	if (dwNumberOfBytesRead == 0) {
		//m_PerfCounter.End();
		wError = (ERR_TIMEOUT);
		goto TxRxError;
	}


	//Verify if ocurred modbus exception 
	if (m_abyBuffer[1] > 0x80) {
		//slave message error 
		if (!VerifyRespCRC(m_abyBuffer, 3)) { //length modbus error message=3 
			wError = (ERR_CRC);
			goto TxRxError;
		}
		else {
			wError = (ERR_EXCPTION_CODE + m_abyBuffer[2]);
			goto TxRxError;

		}
	}

	//ResponseLength not defined 

	if (0 == wLengthResponse) {
		wRespSize = WORD(dwNumberOfBytesRead);
	}
	else {
		wRespSize = wLengthResponse;
	}

	//verify if response have a valid CRC 

	if (!VerifyRespCRC(m_abyBuffer, wRespSize)) { //length modbus error message=3 
		wError = (ERR_CRC);
		goto TxRxError;
	}


	if (0 == wLengthResponse) {
		if (wRespSize > 2) {
			wLengthResponse = (wRespSize - 2);
		}
	}

	//Discard possible garbage read from buffer 

	//m_abyBuffer.SetSize(wLengthResponse);
	//abyQuery.SetSize(wLengthQuery);

	//liu.k.x
	//abyResponse.Copy(m_abyBuffer);
	memcpy(abyResponse, m_abyBuffer, wLengthResponse);

	if (pwNumOfBytesRead != NULL) {
		*pwNumOfBytesRead = wLengthResponse;
	}

	Unlock();
	return(ERR_OK);

TxRxError:

	if (Delay() > 0) {
		::Sleep(Delay());
	}

	Unlock();
	return(wError);
}

string CLocalModbus::ErrorMessage(WORD wErrorCode){
	return CModbus::ErrorMessage(wErrorCode);
}

WORD CLocalModbus::TxRxMessageAscii(const BYTE* abyQuery, //modbus query without CRC
									WORD wLengthQuery, //QueryLength without CRC
									BYTE* abyResponse, //modbus Response without CRC
									WORD wLengthResponse, //wLengthResponse Response without CRC
									WORD* pwNumOfBytesRead){

										int iBufferSize;
										int i;
										int iDataSize;
										BYTE byLRC;
										DWORD dwNumberOfBytesWritten;
										DWORD dwNumberOfBytesRead;
										WORD wError;
										
										//m_abyBuffer.SetSize(wLengthQuery*2+5);

										iBufferSize=wLengthQuery*2+5;//m_abyBuffer.GetSize();
										//liu.k.x
										::ZeroMemory(m_abyBuffer, iBufferSize);
										
										//RTU2ASCII(abyQuery.GetData(),wLengthQuery,&m_abyBuffer[1]);
										RTU2ASCII(abyQuery,wLengthQuery,&m_abyBuffer[1]);


										//byLRC=CRC16::LRC(&m_abyBuffer[1],wLengthQuery*2);										
										//liu.k.x
										//byLRC=CRC16::LRC(abyQuery.GetData(),wLengthQuery);
										byLRC=CRC16::LRC(abyQuery,wLengthQuery);

										m_abyBuffer[0]=':';
										m_abyBuffer[wLengthQuery*2+1]=Num2Ascii(HI4BITS(byLRC));
										m_abyBuffer[wLengthQuery*2+2]=Num2Ascii(LO4BITS(byLRC));
										m_abyBuffer[wLengthQuery*2+3]=C_R;
										m_abyBuffer[wLengthQuery*2+4]=L_F;

										if(!ClearBuffers()){
											LOG_PROGRAM(LL_ERROR, "ClearBuffers Error");
										};
										DWORD bufferSize = RESP_BUFFER * 2;
										//m_abyBuffer.SetSize(RESP_BUFFER*2);

										//liu.k.x
										::ZeroMemory(m_abyBuffer, bufferSize);
										//write query 
										if (!WriteQuery(m_abyBuffer,wLengthQuery*2+5,dwNumberOfBytesWritten)){
											LOG_PROGRAM(LL_ERROR, "Write Query Error");
											wError=(ERR_WR_PORT);
											goto TxRxErrorAscii;
										} 

										//DWORD bufferSize = RESP_BUFFER*2;
										//m_abyBuffer.SetSize(RESP_BUFFER*2);

										//liu.k.x
										//::ZeroMemory(m_abyBuffer, bufferSize);

										if (!ReadResponseASCII(m_abyBuffer, bufferSize, dwNumberOfBytesRead)) //not using COMMTIMEOUTS
										{
											LOG_PROGRAM(LL_ERROR, "ReadResponse Failed"); 
											wError=(ERR_RD_PORT);
											goto TxRxErrorAscii;

										}


										if (dwNumberOfBytesRead == 0){
											//m_PerfCounter.End();
											wError=(ERR_TIMEOUT);
											goto TxRxErrorAscii;
										}


										if (!VerifyRespLRC(m_abyBuffer,dwNumberOfBytesRead)){

											wError=(ERR_CRC);
											goto TxRxErrorAscii;

										}

										if (HiLo4BitsToByte(Ascii2Num(m_abyBuffer[3]),Ascii2Num(m_abyBuffer[4]))>0x80) { 

											wError= (ERR_EXCPTION_CODE+HiLo4BitsToByte(Ascii2Num(m_abyBuffer[5]),Ascii2Num(m_abyBuffer[6]))
												);
											goto TxRxErrorAscii;

										}

										iDataSize = int((dwNumberOfBytesRead-5)/2);

										iDataSize= min(iDataSize,bufferSize/*m_abyBuffer.GetSize()*/);

										for (i=0;i<iDataSize;i++){
											abyResponse[i]=HiLo4BitsToByte(Ascii2Num(m_abyBuffer[1+i*2]),
												Ascii2Num(m_abyBuffer[1+i*2+1]));
										}

										if (pwNumOfBytesRead!=NULL) {
											*pwNumOfBytesRead=WORD((dwNumberOfBytesRead-5)/2);
										}
										return(ERR_OK);

TxRxErrorAscii:

										if (Delay()>0) {
											::Sleep(Delay());
										}
										return(wError);
}



//Transmit Raw Data to serial port 
WORD CLocalModbus::TxRxRawData(const BYTE* pDataWrite,
							   WORD wLengthData,
							   BYTE* pRespBuffer,
							   WORD  wMaxBufferSize,
							   WORD& wNumOfBytesRead,
							   DWORD dwTimeOut
							   ){ 

								   COMMTIMEOUTS cto;
								   WORD wError;
								   DWORD  dwTime_Out;


								   Lock();

								   wNumOfBytesRead=0;

								   if (m_hComHandle==INVALID_HANDLE_VALUE){
									   LOG_PROGRAM(LL_ERROR, "Port Not opened\n"); 

									   wError=(ERR_NOT_INT);
									   goto TxRxErrorRawData;
								   }

								   if (dwTimeOut!=0) {

									   if (!GetTimeouts(cto)){
										   wError=(ERR_NOT_INT);
										   goto TxRxErrorRawData;

									   }
									   dwTime_Out= dwTimeOut;
									   SetTimeouts(NULL);
								   }
								   else {

									   dwTime_Out=(DWORD)m_lTimeOut;
								   }


								   if(!ClearBuffers()){
									   LOG_PROGRAM(LL_ERROR, "ClearBuffers Error");
								   }

								   //Write Data 
								   DWORD  dwNumberOfBytesWritten;
								   DWORD  dwErrors;
								   DWORD  dwStart;
								   COMSTAT cstStatus;
								   char txt[128];

								   if( !::WriteFile(m_hComHandle,pDataWrite,wLengthData,&dwNumberOfBytesWritten,NULL) ) {
									   if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
										   //Verify Returned error 
										   //LOG_PROGRAM(LL_ERROR, QString("Writefile Error %1").arg(dwErrors));
										   sprintf_s(txt, "Writefile Error %d", dwErrors);
										   LOG_PROGRAM(LL_INFO, txt);
									   }
									   else{
										   LOG_PROGRAM(LL_ERROR, "ClearCommError Failed writing");
									   }
									   wError=(ERR_WR_PORT);
									   goto TxRxErrorRawData;
								   }



								   ::ClearCommError(m_hComHandle,&dwErrors,&cstStatus);

								   dwStart = ::timeGetTime(); 

								   while (((dwStart+dwTime_Out) >::timeGetTime())&&(cstStatus.cbOutQue>0)){
									   ::ClearCommError(m_hComHandle,&dwErrors,&cstStatus);
								   }


								   //Read Data
								   DWORD dwNumberOfBytesRead;
								   DWORD dwRead;

								   dwRead=0;
								   dwStart = ::timeGetTime(); 

								   do {
									   if( !::ReadFile(m_hComHandle,(pRespBuffer+dwRead),1,&dwNumberOfBytesRead,NULL) ) { //read one charcter 
										   if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
											   //LOG_PROGRAM(LL_ERROR, QString("ReadFile Errors 0x=%1").arg(dwErrors));
											   sprintf_s(txt, "ReadFile Errors 0x=%d", dwErrors);
											   LOG_PROGRAM(LL_INFO, txt);
										   }
										   else{
											   LOG_PROGRAM(LL_ERROR, " ClearCommError Failed reading");
										   }
										   wError=(ERR_RD_PORT);
										   goto TxRxErrorRawData;
									   }
									   dwRead+=dwNumberOfBytesRead;
								   }
								   while (((dwStart+dwTime_Out) >::timeGetTime())&&(dwRead<wMaxBufferSize));

								   wNumOfBytesRead=(WORD)dwRead;

								   //set timeout back to default values
								   if (dwTimeOut!=0) {

									   if (!SetTimeouts(&cto)){
										   wError=(ERR_NOT_INT);
										   goto TxRxErrorRawData;

									   }

								   }


								   Unlock();
								   return(ERR_OK);

TxRxErrorRawData:

								   if (Delay()>0) {
									   ::Sleep(Delay());
								   }

								   Unlock();
								   return(wError);
}

// private functions 

//verify modbus response
//return true if ok 
//abyResp Modbus response 
//dwLength Length of response without CRC 
BOOL CLocalModbus::VerifyRespCRC(const BYTE* abyResp, WORD wLength){ 
	WORD wcrc; 
	wcrc=m_crc.CalcCrcFast(abyResp,wLength);
	return( (abyResp[wLength]==HIBYTE(wcrc)) && (abyResp[wLength+1]==LOBYTE(wcrc)) );
}

BOOL CLocalModbus::WriteQuery(BYTE* abyQuery,DWORD dwQuerySize, DWORD& dwNumberOfBytesWritten) {

	DWORD  dwTimeOut;  
	DWORD  dwErrors;
	COMSTAT cstStatus;
	char txt[128];
	//Write Query 	
	if( !::WriteFile(m_hComHandle,abyQuery,dwQuerySize,&dwNumberOfBytesWritten,NULL) ) {
		if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
			//Verify Returned error 
			//LOG_PROGRAM(LL_ERROR, QString("Writefile Error %1").arg(dwErrors));
			sprintf_s(txt, "Writefile Error %d", dwErrors);
			LOG_PROGRAM(LL_INFO, txt);
		}
		else{
			LOG_PROGRAM(LL_ERROR, "ClearCommError Failed writing");
		}
		return (FALSE);
	}

	//conut time after send message to device
	//m_PerfCounter.Start();

	::ClearCommError(m_hComHandle,&dwErrors,&cstStatus);

	dwTimeOut = ::timeGetTime() + m_lTimeOut;

	while ((dwTimeOut <::timeGetTime())&&(cstStatus.cbOutQue>0)){
		::ClearCommError(m_hComHandle,&dwErrors,&cstStatus);
	}

	return (cstStatus.cbOutQue==0);


}

//read device response 
//return true if Read OK 
BOOL CLocalModbus::ReadResponseASCII(BYTE* abyResponse,//bytes read 
									 DWORD& dwNeedRead,
									 DWORD& dwRead) {    //num of bytes read 


										 DWORD dwNumberOfBytesRead=0;
										 DWORD dwTimeOut = m_lTimeOut;
										 DWORD bFirstNull=FALSE;
										 DWORD dwFirstNullTime=0;
										 DWORD dwNullTime=0;
										 DWORD  dwErrors;
										 COMSTAT cstStatus;
										 //int iRespMaxSize;
										 BOOL bAsciiCompleted;
										 char txt[128];

										 //iRespMaxSize = abyResponse.GetSize();

										 dwRead=0;

										 bAsciiCompleted=FALSE;

										 do {

											 if (dwRead>DWORD(dwNeedRead/*iRespMaxSize*/-1)) {
												 return FALSE;
											 }

											 if( !::ReadFile(m_hComHandle,&abyResponse[dwRead],1,&dwNumberOfBytesRead,NULL) ) { //read one charcter 
												 if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
													 //LOG_PROGRAM(LL_ERROR, QString("ReadFile Errors %1").arg(dwErrors));
													 sprintf_s(txt, "ReadFile Errors %d", dwErrors);
													 LOG_PROGRAM(LL_INFO, txt);
												 }
												 else{
													 LOG_PROGRAM(LL_ERROR, " ClearCommError Failed reading");
												 }

												 return (FALSE);
											 }


											 if (dwNumberOfBytesRead == 1){
												 dwRead += 1;
												 if (1==dwRead) {
													 //m_PerfCounter.End();
												 }
												 dwTimeOut=m_dwSilentInterval;
												 bFirstNull=FALSE;
												 dwNullTime=0;
												 if (dwRead>dwNeedRead/*(DWORD)abyResponse.GetSize()*/) {
													 return FALSE;
												 }
											 }

											 else{

												 if (!bFirstNull){
													 bFirstNull=TRUE;
													 dwFirstNullTime=::timeGetTime();
												 }
												 else {
													 dwNullTime=::timeGetTime();
												 }
												 //DoEvents();
												 ::Sleep(1);

											 }



											 if (!::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
												 LOG_PROGRAM(LL_ERROR, "ReadResponse ClearCommError Failed reading");
											 }

											 if (m_wTranmitionMode==MODE_ASCII){
												 if (dwRead>5){//Read at least  <:>  <Address> <Functioncode> 
													 if ((C_R==abyResponse[dwRead-2])&&(L_F==abyResponse[dwRead-1])){
														 bAsciiCompleted=TRUE;
													 }
												 }

											 }


										 } while	( ( (cstStatus.cbInQue>0)
											 ||(dwNullTime<(dwTimeOut+dwFirstNullTime))
											 )
											 &&(!bAsciiCompleted)
											 );

										 return(TRUE);

}

//read device response 
//return true if Read OK 
BOOL CLocalModbus::ReadResponseRTU(BYTE* abyResponse,//bytes read 
								   DWORD& dwNeedRead,
								   DWORD& dwRead) {    //num of bytes read 
									   

									   DWORD dwNumberOfBytesRead=0;
									   DWORD  dwErrors;
									   COMSTAT cstStatus;
									   //int iRespMaxSize;
									   char txt[128];


									   //iRespMaxSize = dwNeedRead;//abyResponse.GetSize();

									   dwRead=0;

									   //DWORD ticCountStart = ::GetTickCount();
									   if( !::ReadFile(m_hComHandle, &abyResponse[dwRead],INITIAL_READ,&dwNumberOfBytesRead,NULL) ) { //read one charcter 
										   if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
											   //LOG_PROGRAM(LL_ERROR, QString("ReadFile Errors %1").arg(dwErrors));
											   sprintf_s(txt, "ReadFile Errors %d", dwErrors);
											   LOG_PROGRAM(LL_INFO, txt);
										   }
										   else{
											   LOG_PROGRAM(LL_ERROR, " ClearCommError Failed reading");
										   }
										   return (FALSE);
									   }
									   //DWORD time = (::GetTickCount() - ticCountStart);
									   //if(time > 0)
									   //{
										  // qDebug("ReadFile:(%d) %dms\n",dwNumberOfBytesRead, time);
									   //}

									   dwRead=dwRead+dwNumberOfBytesRead;

									   if (INITIAL_READ==dwNumberOfBytesRead) {	   
										   if (dwRead>DWORD(dwNeedRead/*iRespMaxSize*/-INITIAL_READ)) {
											   return FALSE;
										   }

										   //ticCountStart = ::GetTickCount();
										   if( !::ReadFile(m_hComHandle,&abyResponse[dwRead],(dwNeedRead/*iRespMaxSize*/-INITIAL_READ),&dwNumberOfBytesRead,NULL) ) { //read one charcter 
											   if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
												   //LOG_PROGRAM(LL_ERROR, QString("ReadFile Errors %1").arg(dwErrors));
												   sprintf_s(txt, "ReadFile Errors %d", dwErrors);
												   LOG_PROGRAM(LL_INFO, txt);
											   }
											   else{
												   LOG_PROGRAM(LL_ERROR, " ClearCommError Failed reading");
											   }
											   return (FALSE);
										   }
										   //time = (::GetTickCount() - ticCountStart);
										   //if(time>0)
										   //{
											  // qDebug("ReadFile2 (%d):%dms\n", dwNumberOfBytesRead, time);
										   //}
										   dwRead=dwRead+dwNumberOfBytesRead;	   
									   }
									   return(TRUE);

}

//BOOL CLocalModbus::ReadResponse(QMyByteArray& abyResponse,DWORD& dwRead,DWORD dwBytesToRead) {
//
//	DWORD dwErrors;
//	COMSTAT cstStatus;
//	DWORD dwNumberOfBytesRead;
//	COMMTIMEOUTS timeouts;
//	DWORD dwCurrentReadTotalTimeoutConstant;
//
//	//Get Current Timeouts 
//	if(GetTimeouts(timeouts)) {
//
//		dwCurrentReadTotalTimeoutConstant=timeouts.ReadTotalTimeoutConstant;				 
//
//	}
//	else {
//		return FALSE;
//	}
//
//	dwRead=0;
//
//	do {
//		if( !::ReadFile(m_hComHandle,&abyResponse[dwRead],1,&dwNumberOfBytesRead,NULL) ) { //read one charcter 
//			if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
//				LOG_PROGRAM(LL_ERROR, "ReadFile Errors 0x=%xd",dwErrors);
//			}
//			else{
//				LOG_PROGRAM(LL_ERROR, " ClearCommError Failed reading");
//			}
//
//			return (FALSE);
//		}
//
//		if (dwNumberOfBytesRead == 1){
//			dwRead++;
//			if (1==dwRead) {
//				//m_PerfCounter.End();
//				timeouts.ReadTotalTimeoutConstant=SilentInterval();
//				if (!SetTimeouts(&timeouts)){
//					LOG_PROGRAM(LL_ERROR, "SetTimeouts Failed");
//					break;
//				}
//			}
//		}
//
//	} while (dwNumberOfBytesRead>0);
//
//	if (dwRead>0) {
//		timeouts.ReadTotalTimeoutConstant=dwCurrentReadTotalTimeoutConstant;
//		if (!SetTimeouts(&timeouts)){
//			LOG_PROGRAM(LL_ERROR, "SetTimeouts Failed\n");
//		}
//	}
//
//	return (TRUE);
//
//}


BOOL CLocalModbus::ClearBuffers(){
	//Clear RxTx Buffers 
	return(::PurgeComm(m_hComHandle,(PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR)));
}



void CLocalModbus::TransmissionMode(WORD wMode)
{

	m_wTranmitionMode=wMode;
}

WORD CLocalModbus::TransmissionMode()
{
	return m_wTranmitionMode;
}

void CLocalModbus::RTU2ASCII(const BYTE* aRtu, int iSize , BYTE* aAscii)
{

	int i=0;

	//Q_ASSERT((iSize*2)<aAscii.GetSize());

	for (i=0;i<iSize;i++){
		aAscii[i*2]=Num2Ascii(HI4BITS(aRtu[i]));
		aAscii[i*2+1]=Num2Ascii(LO4BITS(aRtu[i]));
	}

}

BYTE CLocalModbus::Num2Ascii(BYTE byNum)
{
	if (byNum<=9){
		return byNum+'0';
	}
	else if ((byNum>=0xA)&&(byNum<=0xF)){
		return ((byNum-0xA)+'A');
	}
	else {
		ASSERT(FALSE);
		return '0';
	}
}

BOOL CLocalModbus::VerifyRespLRC(BYTE* aResp, DWORD dwLength)
{

	BYTE byLRC;
	BYTE byMsgLRC;

	if (dwLength<5) {
		return FALSE;
	}

	//liu.k.x
	//byLRC=LRCASCII(aResp.GetData(),WORD(dwLength)); //calc crc 
	byLRC=LRCASCII(aResp,WORD(dwLength)); //calc crc 

	byMsgLRC=HiLo4BitsToByte(Ascii2Num(aResp[dwLength-4]),
		Ascii2Num(aResp[dwLength-3]));

	return (byLRC==byMsgLRC);
}

BYTE CLocalModbus::Ascii2Num(BYTE byChar)
{
	if ((byChar>='0')&&(byChar<='9')){
		return (byChar-'0');
	}
	else if ((byChar>='A')&&(byChar<='F')){
		return (byChar-'A')+0xA;
	}
	else {
		//Q_ASSERT(FALSE);
		return 0; 

	}
}

BYTE CLocalModbus::HiLo4BitsToByte(BYTE byHi, BYTE byLo)
{

	return ( (((0xF)&byHi)<<4)| ((0xF)&byLo)    );

}


BYTE CLocalModbus::LRCASCII(const BYTE *auchMsgASCII, WORD usDataLen) {

	BYTE uchLRC = 0 ;	/* LRC char initialized   */
	BYTE byTemp =0;
	int i;

	int iDataSize = int((usDataLen-5)/2);

	for (i=0;i<iDataSize;i++){
		byTemp=HiLo4BitsToByte(Ascii2Num(auchMsgASCII[1+i*2]),
			Ascii2Num(auchMsgASCII[1+i*2+1]));
		uchLRC +=byTemp;
	}

	//uchLRC = uchLRC ^ 0xffff;
	//uchLRC += 1;

	return BYTE(-uchLRC);
}


void CLocalModbus::ByteSize(WORD wSize)
{
	m_wByteSize= wSize;
}

WORD CLocalModbus::ByteSize()
{
	return m_wByteSize;
}

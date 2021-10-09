// modbu.h: interface for the CModbus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODBU_H__C08841E3_BB2E_11D1_8FE9_E4CD05C10000__INCLUDED_)
#define AFX_MODBU_H__C08841E3_BB2E_11D1_8FE9_E4CD05C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Common.h"
#include "windows.h"
#include <string>
using std::string;

#define ASSERT(x)

class CModbus;

class CModbus
{


public:

	CModbus();
	virtual ~CModbus();

	//Do a loopback test 
	// return ERR_XXX Code see Modbus.cpp file 
	WORD LoopbackTest(WORD nAddr);//addrres of modbus device 

	//Read Output registers 
	//return ERR_XXX Code see Modbus.cpp file  
	WORD ReadOutputRegisters(WORD nAddr,  //address of modbus device
		WORD nDataStart , //start address, first address is 0 "ZERO" Ex: 30005 -> nDaStart=4 
		WORD nQtd , //number of reg. to read 
		WORD* anRegValues); //read values if does not occur an error

	WORD ReadOutputRegisters(WORD nAddr,  //address of modbus device
		WORD nDataStart , //start address, first address is 0 "ZERO" Ex: 30005 -> nDaStart=4 
		WORD nQtd , //number of reg. to read 
		short* anRegValues); //read values if does not occur an error

	//WORD ReadOutputRegisters(WORD nAddr,
	//				WORD nDataStart , 
	//				WORD nQtd , 
	//				CDblArray& anRegValues,
	//				WORD wRealType = REAL_NORMAL);

	//WORD ReadOutputRegisters(WORD nAddr,
	//				WORD nDataStart , 
	//				WORD nQtd , 
	//				CFloatArray& anRegValues,
	//				WORD wRealType = REAL_NORMAL);

	//Read Input registers 
	//return ERR_XXX Code see Modbus.cpp file  
	WORD ReadInputRegisters( WORD nAddr,  //address of modbus device 
		WORD nDataStart , //start address, first address is 0 "ZERO" Ex: 40005 -> nDaStart=4 
		WORD nQtd , //number of reg. to read 
		WORD* anRegValues);//read values if does not occur an error

	WORD ReadInputRegisters( WORD nAddr,  //address of modbus device 
		WORD nDataStart , //start address, first address is 0 "ZERO" Ex: 40005 -> nDaStart=4 
		WORD nQtd , //number of reg. to read 
		short* anRegValues);//read values if does not occur an error

	//WORD ReadInputRegisters(WORD nAddr,  
	//							WORD nDataStart , 
	//							WORD nQtd , 
	//							CDblArray& anRegValues,
	//							WORD wRealType = REAL_NORMAL);

	//WORD ReadInputRegisters(WORD nAddr,  
	//							WORD nDataStart , 
	//							WORD nQtd , 
	//							CFloatArray& anRegValues,
	//							WORD wRealType = REAL_NORMAL);
	//Read Output Status  
	//return ERR_XXX Code see Modbus.cpp file 
	WORD ReadOutputStatus(WORD nAddr, //address of modbus device
		WORD nDataStart , //start address, first address is 0 "ZERO" 
		WORD nQtd , //number of coils. to read
		BYTE* abyCoilValues); //read coils if does not occur an error
	//use TRUE and FALSE windows defines 

	//Read Input Status  
	//return ERR_XXX Code see Modbus.cpp file 
	WORD ReadInputStatus(WORD nAddr, //address of modbus device
		WORD nDataStart , //start address, first address is 0 "ZERO" 
		WORD nQtd , //number of coils. to read
		BYTE* abCoilValues);//read coils if does not occur an error
	//use TRUE and FALSE windows defines 


	//Preset multiple Registers    
	//return ERR_XXX Code see Modbus.cpp file 
	WORD PresetMultipleRegisters(WORD nAddr, //address of modbus device 
		WORD nDataStart , //start address, first address is 0 "ZERO" 
		WORD nQtd , //number of reg . to write 
		const short* anRegValues); //regvalues that will be written

	WORD PresetMultipleRegisters(WORD nAddr, //address of modbus device 
		WORD nDataStart , //start address, first address is 0 "ZERO" 
		WORD nQtd , //number of reg . to write 
		const WORD* anRegValues); //regvalues that will be written

	//WORD PresetMultipleRegisters(WORD nAddr, //address of modbus device 
	//					   WORD nDataStart , //start address, first address is 0 "ZERO" 
	//					   WORD nQtd , //number of reg . to write 
	//					   CDblArray& anRegValues,
	//					   WORD wRealType = REAL_NORMAL); //regvalues that will be written

	//WORD PresetMultipleRegisters(WORD nAddr, //address of modbus device 
	//				   WORD nDataStart , //start address, first address is 0 "ZERO" 
	//				   WORD nQtd , //number of reg . to write 
	//				   CFloatArray& afRegValues,
	//				   WORD wRealType = REAL_NORMAL); //regvalues that will be written


	//Preset single Register    
	//return ERR_XXX Code see Modbus.cpp file
	WORD PresetSingleRegister(WORD nAddr, //address of modbus device
		WORD nRegister , //register start at 0 
		WORD nRegValue); //register value 

	WORD PresetSingleRegister(WORD nAddr,
		WORD nRegister , 
		short int nRegValue);

	WORD PresetSingleRegister(WORD nAddr,
		WORD nRegister , 
		float fRegValue,
		WORD wRealType = REAL_NORMAL);

	WORD PresetSingleRegister(WORD nAddr,
		WORD nRegister , 
		double dRegValue,
		WORD wRealType = REAL_NORMAL);


	//Force Single Coil 
	//return ERR_XXX Code see Modbus.cpp file
	WORD ForceSingleCoil(WORD nAddr,//address of modbus device
		WORD nRegister,  //register start at 0 
		BOOL bCoilValue); //register value 

	//Force multiple Coils 
	//return ERR_XXX Code see Modbus.cpp file
	WORD ForceMultipleCoils(WORD nAddr, //address of modbus device
		WORD nDataStart , //start address, first address is 0 "ZERO" 
		WORD nQtd ,  //number of reg . to write
		BYTE* abCoilValues); //coils values 



	//transmit a query to a modbus device and wait for a response
	//return ERR_XXX Code see Modbus.cpp file  
	virtual WORD TxRxMessage(BYTE* abyQuery, //modbus query without CRC
		WORD wLengthQuery, //QueryLength without CRC
		BYTE* abyResponse, //modbus Response without CRC
		WORD wLengthResponse, //wLengthResponse Response without CRC
		WORD* pwNumOfBytesRead=NULL)=0;

	//return string error message 
	//in:wErrorCode constant errro code 
	//out:string with error 
	virtual string ErrorMessage(WORD wErrorCode); 

	BOOL Lock(); //lock modbus object "External Critical Section"

	BOOL Unlock();

	//response time of last comunication
	float ResponseTime();

	//Retries
	WORD Retries() const ;
	void Retries(WORD wRetries);

	DWORD Delay();
	void  Delay(DWORD dwDelay);


	BOOL ThrowException() const;
	void ThrowException(BOOL bThrow);

	virtual BOOL IsActive()=0; //return  if the connection is open


	enum _general_constants_ {
		REAL_NORMAL=0,
		REAL_REVERSE=1
	};


protected:
	void ThrowModbusException(WORD wErrorcode);
	//void DoEvents();
	void ErrorDelay(WORD nError);

//	CPerformanceCounter m_PerfCounter;
	//QMutex m_csComm; //Critical Section when sending message to modbus device 
	CRITICAL_SECTION m_cs;


	BYTE abyQuery[RESP_BUFFER*2];
	BYTE abyReply[RESP_BUFFER*2];
private:

	//C++ boolean to modbus boolean 
	WORD CmodBool(BOOL bCoil);

	//copy short 2 word array 
	void Short2WordArray(const short* aiArray, const WORD size, WORD* anArray);

	void Word2ShortArray(const WORD* anArray, const WORD size, short* aiArray);

	void Dbl2Word(const double* padSrc, int iSrcSize,WORD* pawDest, int iDestSize,WORD wRealType = REAL_NORMAL);  

	void Word2Dbl(const WORD* pawSrc, int iSrcSize,double* padDest, int iDestSize,WORD wRealType = REAL_NORMAL);  

	void Word2Float(const WORD* pawSrc, int iSrcSize,float* padDest, int iDestSize,WORD wRealType= REAL_NORMAL);

	void Float2Word(const float* padSrc, int iSrcSize,WORD* pawDest, int iDestSize,WORD wRealType);

	WORD  m_wRetries;
	DWORD m_dwLockTimeout;
	DWORD m_dwDelay;

	BOOL m_bThrowModbusException;

	//modbus functions 
	static const BYTE READ_OUTPUT_REGISTERS; 
	static const BYTE READ_INPUT_REGISTERS;
	static const BYTE READ_INPUT_STATUS; 
	static const BYTE READ_OUTPUT_STATUS; 
	static const BYTE PRESET_MULTIPLE_REGISTERS;
	static const BYTE PRESET_SINGLE_REGISTER;
	static const BYTE LOOP_BACK_TEST;
	static const BYTE FORCE_SINGLE_COIL;
	static const BYTE FORCE_MULTIPLE_COILS;


};

#endif // !defined(AFX_MODBU_H__C08841E3_BB2E_11D1_8FE9_E4CD05C10000__INCLUDED_)

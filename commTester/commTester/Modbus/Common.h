#ifndef __COMMON_H__
#define __COMMON_H__

typedef unsigned char UCHAR;
typedef unsigned short WORD;
typedef unsigned char BYTE;

enum _CModbu_LIMITS_ {
	MAX_COM_PORTS=8,
	INPUT_BUFFER_SIZE=1024,
	OUTPUT_BUFFER_SIZE=1024,
	RESP_BUFFER=6,//24,
	READ_SIZE=8,
	INITIAL_READ=4
};

//error codes 
enum _error_codes_ {
	ERR_OK =    0,   // OK
	ERR_TIMEOUT=1,   // TIMEOUT
	ERR_INV_RESP=2,   // Invalid Response 
	ERR_CRC=    3,   // CRC Error
	ERR_WR_PORT= 8,   // Write Comm Error
	ERR_RD_PORT= 9,   // Read Comm Error
	ERR_NOT_INT=10,  // Open Comm error
	ERR_LOCK_TIME_OUT=11,  // Open Comm error
	ERR_EXCPTION_CODE=0x8000   //8001->Exp. code 1 8002->excep. code 2 
};

enum LOG_LEVEL
{
	LL_INFO,
	LL_ERROR
};

enum SYS_LOG_TYPE
{
	LT_LOGIN = 0,
	LT_LOGOUT,
	LT_CHANGE_CONFIG_VALUE
};

#define LOG_PROGRAM(x, y) //(g_LogProgram.Add((x), (y)))

#define		PACKAGE_TRY_TIMES		2	//发送Modbus包时,重试几次

//要写到PLC的数据包
typedef struct 
{
	WORD  wStartDT;
	WORD  wQuerySize;
	WORD  arrWordQuery[RESP_BUFFER*2];
}ModbusWritePackage;

#endif // __COMMON_H__
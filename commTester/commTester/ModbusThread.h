#ifndef __MODBUS_THREAD_H__
#define __MODBUS_THREAD_H__


#include "Modbus/localmodbus.h"
#include <list>
using std::list;

// QueryMessageFromPLC读取PLC中的寄存器的起始地址
#define REGISTER_BASE			100
#define READ_REG_NUM			5
#define WRITE_REG_NUM			3



class CModbusThread 
{
public:
	CModbusThread() {
		m_bCommIsOpen = false;
		m_bConnected = false;

		m_SiteAddress = 0x0001;
		m_ComPort = 1;
		m_dwBaudRate = 19200;
		m_Parity = EVENPARITY;

		long wByteSize = 8;
		long byStopBits = ONESTOPBIT;

		m_LocalModbus.StopBits(byStopBits);
		m_LocalModbus.ByteSize(wByteSize);
		m_LocalModbus.FlowControl(CLocalModbus::FC_NONE);

		m_LocalModbus.Timeout(300);
		m_LocalModbus.SilentInterval(20);
		m_LocalModbus.TransmissionMode(CLocalModbus::MODE_RTU);
	}
	~CModbusThread() {
		//
	}

public:
	void SetComm(int port, int baudRate, int parity = 2) {
		m_ComPort = port;
		m_dwBaudRate = baudRate;
		m_Parity = parity;
	}

	bool OpenComm() {
		if (m_bCommIsOpen)
			return true;

		if (!m_LocalModbus.UpdateSerialConfig(m_ComPort, m_dwBaudRate, m_Parity))
		{
			return false;
		}
		else
		{
			m_bCommIsOpen = true;
		}
		return true;
	}

	// 串口也会在本对象析构时自动关闭
	bool CloseComm() {
		return m_LocalModbus.CloseCommPort();
	}

	// 异步
	void SetOneWordToPLC(const WORD& dt, const WORD& value) {
		ModbusWritePackage package;
		ZeroMemory(&package, sizeof(ModbusWritePackage));
		package.wStartDT = dt;
		package.wQuerySize = 1;
		package.arrWordQuery[0] = value;
		//AppendModbusWritePackage(package);
		if (!WritePackageToPLC(package)) //发送失败，那么表示与PLC的连接断开
		{
			std::cout << "send word failed!\n";
		}
	}

	// 同步
	// 只能至少一次从PLC读取2个以上的寄存器
	//bool QueryTwoWordFromPLC(const WORD& dt, WORD* value);

	// 异步
	void SetTwoWordToPLC(const WORD& dt, const WORD* value) {
		//	FILE* fp = fopen("PLC.txt","a");
		ModbusWritePackage package;
		ZeroMemory(&package, sizeof(ModbusWritePackage));
		package.wStartDT = dt;
		package.wQuerySize = WRITE_REG_NUM;
		for (int i = 0; i < WRITE_REG_NUM; i++)
		{
			//	if(fp)
			//		fprintf(fp,"%d = %d\n",i,value[i]);
			package.arrWordQuery[i] = value[i];
		}
		//	if(fp)
		//		fclose(fp);
			//package.arrWordQuery[1] = value[1];
			//AppendModbusWritePackage(package);

		if (!WritePackageToPLC(package)) //发送失败，那么表示与PLC的连接断开
		{
		}
	}

	// 异步
	void SetFourWordToPLC(const WORD& dt, const WORD* value) {
		//	FILE* fp = fopen("PLC.txt","a");
		ModbusWritePackage package;
		ZeroMemory(&package, sizeof(ModbusWritePackage));
		package.wStartDT = dt;
		package.wQuerySize = 4;
		for (int i = 0; i < 4; i++)
		{
			//	if(fp)
			//		fprintf(fp,"%d = %d\n",i,value[i]);
			package.arrWordQuery[i] = value[i];
		}
		//	if(fp)
		//		fclose(fp);
			//package.arrWordQuery[1] = value[1];
			//AppendModbusWritePackage(package);

		if (!WritePackageToPLC(package)) //发送失败，那么表示与PLC的连接断开
		{
		}
	}
	// 在线程中读取和写入PLC的寄存器
	//virtual void run();

	bool  IsConnected() {return m_bConnected;}
private:
	bool m_bCommIsOpen;
	bool m_bConnected;

	CLocalModbus m_LocalModbus;

	list<ModbusWritePackage> m_ModbusPackageList;

	void AppendModbusWritePackage(ModbusWritePackage& package);
	bool GetFirstModbusWritePackage(ModbusWritePackage& package);
	bool WritePackageToPLC(ModbusWritePackage& package) {
		for (int i = 0; i < PACKAGE_TRY_TIMES; i++)
		{
			WORD nReturn = m_LocalModbus.PresetMultipleRegisters(m_SiteAddress, package.wStartDT, package.wQuerySize, package.arrWordQuery);
			if (nReturn == ERR_OK)
			{
				return true;
			}
			else
			{
				LOG_PROGRAM(LL_ERROR, QString("2>>Error:ReadOutputRegisters. code=%1, error message=%2").arg(nReturn).arg(m_LocalModbus.ErrorMessage(nReturn)));
			}
		}
		return false;
	}
	bool QueryMessageFromPLC() {
		WORD nDataStart = REGISTER_BASE;
		const WORD nQtd = READ_REG_NUM;
		WORD anRegValues[nQtd];

		for (int i = 0; i < PACKAGE_TRY_TIMES; i++)
		{
			WORD nReturn = m_LocalModbus.ReadOutputRegisters(m_SiteAddress, nDataStart, nQtd, anRegValues);
			if (nReturn == ERR_OK)
			{
				// 对读取到的数据anRegValues进行处理
				// ...
				//for(int j = 0; j < READ_REG_NUM; j++)
					//msg->_data[j] = anRegValues[j];
				//msg->_data[1] = anRegValues[1];
				return true;
			}
			else
			{
				LOG_PROGRAM(LL_ERROR, QString("1>>>Error:ReadOutputRegisters. code=%1, error message=%2").arg(nReturn).arg(m_LocalModbus.ErrorMessage(nReturn)));
			}
		}
		return false;
	}

	long m_SiteAddress;	// modbus site
	long m_ComPort;		// com port
	long m_dwBaudRate;	// Baudrate 9600,4800 actual values  
	long m_Parity;
};

#endif // __MODBUS_THREAD_H__
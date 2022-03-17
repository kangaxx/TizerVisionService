#include "ModbusCom.h"

ModbusCom::ModbusCom() 
{
	_ctx		= 0;
	m_bConnect = false;
}

int ModbusCom::Connect(const char* ip, int port,int nSlaveID)
{
	Close();

	// Step1: /* TCP */
	_ctx = modbus_new_tcp(ip, port);

	// Step2: set slave
	modbus_set_slave(_ctx, nSlaveID);

    // Step3: print or not print debug info
    modbus_set_debug(_ctx, TRUE);  // TRUE : print debug info, FALSE : do not print

	if (modbus_connect(_ctx) == -1)
	{
		fprintf(stderr, "Connection failed: %s\n",modbus_strerror(errno));
		modbus_free(_ctx);
		_ctx = 0;
		return -1;
	}
	m_bConnect = true;
	return 0;
}
ModbusCom::~ModbusCom()
{

	Close();
}
int ModbusCom::Close()
{
	if (_ctx != NULL)
	{
		modbus_close(_ctx);
		modbus_free(_ctx);
		_ctx = NULL;
	}

	return 0;
}
int ModbusCom::Open(int iCol)
{
	return modbus_write_bit(_ctx, iCol, 0xff);
}

int ModbusCom::Close(int iCol)
{
	return modbus_write_bit(_ctx, iCol, 0x00);
}

int ModbusCom::CloseAllCol(int num)
{
	uint8_t* data = new uint8_t[num];
	for (int i = 0; i < num; i++)
	{
		data[i] = 0x00;
	} 
	return modbus_write_bits(_ctx, 0x00, 4, data);

}

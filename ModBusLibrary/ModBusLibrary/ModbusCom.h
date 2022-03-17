#ifndef __MODBUS_COM_H__
#define __MODBUS_COM_H__

#include <modbus.h>
#include <list>

class ModbusCom
{
public:
	ModbusCom();
	~ModbusCom();

	// ������Ͽ�
	int Connect(const char* ip, int port, int nSlaveID);
	int Close();

	//�̵�������
	int CloseAllCol(int num);
	int Open(int iCol);
	int Close(int iCol);

	bool  m_bConnect;
private:
	modbus_t*				_ctx;
};

#endif // __MODBUS_COM_H__
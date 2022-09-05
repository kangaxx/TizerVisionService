#include "HslPlcFunc.h"

bool write_plc_bool(const char* addr, bool data, bool is_show_msg)
{
	if (is_show_msg)
		std::cout << addr << std::endl;
	//plc代码加在下面， to cy

	return true;
}

bool connect_plc()
{
	return true;
}

bool disconnect_plc()
{
	return true;
}

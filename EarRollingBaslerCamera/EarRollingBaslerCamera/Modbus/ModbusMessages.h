
#ifndef _ModbusMessages_h_
#define _ModbusMessages_h_


#define MSG_MODBUS_ERR_INV_RESP "invalid returned device response"
#define MSG_MODBUS_ERR_CRC      "crc error"
#define MSG_MODBUS_ERR_WR_PORT  "Write communication error"
#define MSG_MODBUS_ERR_RD_PORT  "Read communication error"
#define MSG_MODBUS_ERR_NOT_INT  "Comm port not opened"
#define MSG_MODBUS_ERR_LOCK_TIME_OUT "Critical section lock time-out error"
#define MSG_MODBUS_ERR_EXCPTION_CODE "Modbus device return an exception code"
#define MSG_MODBUS_ERR_OK       "Ok"
#define MSG_MODBUS_ERR_TIMEOUT  "Time Out Error"

#define MSG_REMOTE_MODBUS_ERR_OVERFLOW "Buffer Overflow"
#define MSG_REMOTE_MODBUS_ERR_SOCKET_ERROR "Socket Error"
#define MSG_REMOTE_MODBUS_ERR_WR_SOCKET_ERROR "Write Socket Error"
#define MSG_REMOTE_MODBUS_ERR_RD_SOCKET_ERROR "Read Socket Error"
#define MSG_REMOTE_MODBUS_ERR_SOCKET_NOT_CONNECTED "Socket not connected"

#define MSG_AUTO_MODBUS_ERR_AUTOMATION_ERROR "Automation Error" 
#define MSG_AUTO_MODBUS_ERR_BUILD_ARRAY_ERROR "Build Array Error"

#endif 
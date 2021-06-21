#pragma once
//记录毛刺数据，将统计信息写入数据库
//数据库操作部分使用hds里的SqlHelper类
//add by gxx 20210524
#include "pch.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/SqlHelper.h"
#include "../../../hds/serialization_c11.h"
#include <string.h>
using namespace serialization_c11;
using namespace commonfunction_c;
using namespace extensionfunction_c;

class DataCounter
{
public:
	static DataCounter& getInstance() { return _instance; }
	void write(BurrsPainter* bp){
		for (int i = 0; i < bp->getBurrsNum(); ++i) {
			if (bp->getDistance(i) < FLOAT_BURR_LEVEL_LOW) {
				m_LowBurrNumber++;
			}
			else if (bp->getDistance(i) < FLOAT_BURR_LEVEL_MID) {
				m_MidBurrNumber++;
			}
			else {
				m_HighBurrNumber++;
			}
		}
		if (isChangeDate()) {
			writeToMysql();
			setDate();
			m_LowBurrNumber = 0;
			m_MidBurrNumber = 0;
			m_HighBurrNumber = 0;
		}

	}
	int get_LowBurrNumber() { return m_LowBurrNumber; }
	int get_MidBurrNumber() { return m_MidBurrNumber; }
	int get_HighBurrNumber() { return m_HighBurrNumber; }
	~DataCounter() { sqlHelper.free(); }
private:
	int m_LowBurrNumber, m_MidBurrNumber, m_HighBurrNumber;
	void setDate() {
		char chTmp[20] = { '\0' };
		time_t t;
		time(&t);
		localtime_s(&m_date, &t);
		m_date.tm_year = m_date.tm_year + 1900;
		m_date.tm_mon = m_date.tm_mon + 1;
	}

	bool isChangeDate() {
		char chTmp[20] = { '\0' };
		time_t t;
		time(&t);
		struct tm p;
		localtime_s(&p, &t);
		p.tm_year = p.tm_year + 1900;
		p.tm_mon = p.tm_mon + 1;
		if (p.tm_year == m_date.tm_year && p.tm_mon == m_date.tm_mon && p.tm_mday == m_date.tm_mday && p.tm_hour == m_date.tm_hour)
			return false;
		else
			return true;
	}
	
	void writeToMysql(){
		char insertCommand[400] = { 0 };
		snprintf(insertCommand, sizeof(insertCommand), 
			"insert TB_BURR_COUNT_LOG(LOW_BURR_NUMBER, MID_BURR_NUMBER, HIGH_BURR_NUMBER, LOG_TIME) value(%d, %d, %d, '%04d-%02d-%02d %02d:%02d')",
			m_LowBurrNumber, m_MidBurrNumber, m_HighBurrNumber, m_date.tm_year, m_date.tm_mon, m_date.tm_mday, m_date.tm_hour, m_date.tm_min);
		sqlHelper.insert(insertCommand);
	}

	MysqlHelper sqlHelper;
	struct tm m_date;
	DataCounter() { 
		m_LowBurrNumber = 0;
		m_MidBurrNumber = 0;
		m_HighBurrNumber = 0;
		//wstring config = commonfunction_c::BaseFunctions::GetWorkPath();
		//wstring config = wstring(L"/config.ini");
		sqlHelper.connect("c:\\tizer\\config.ini");
		setDate(); 
	}

	DataCounter(const DataCounter& right) {
		//do nothing
	}

	void operator=(const DataCounter& right) { 
		//do nothing 
	}
	static DataCounter _instance;

};

DataCounter DataCounter::_instance;
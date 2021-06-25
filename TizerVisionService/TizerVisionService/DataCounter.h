#pragma once
//记录毛刺数据，将统计信息写入数据库
//数据库操作部分使用hds里的SqlHelper类
//add by gxx 20210524
#include "pch.h"
#include "../../../hds/commonfunction_c.h"
#include "../../../hds/SqlHelper.h"
#include "../../../hds/serialization_c11.h"
#include "../../../hds/Logger.h"
#include <string.h>
using namespace serialization_c11;
using namespace commonfunction_c;
using namespace extensionfunction_c;

class DataCounter
{
public:
	static DataCounter& getInstance() { return _instance; }
	void write(BurrsPainter* bp, int cameraIdx, string cameraName){
		for (int i = 0; i < bp->getBurrsNum(); ++i) {
			if (bp->getDistance(i) < FLOAT_BURR_LEVEL_LOW) 
				m_LowBurrNumber[cameraIdx]++;
			else if (bp->getDistance(i) < FLOAT_BURR_LEVEL_MID) 
				m_MidBurrNumber[cameraIdx]++;
			else if (bp->getDistance(i) < FLOAT_BURR_LEVEL_HIGH) 
				m_HighBurrNumber[cameraIdx]++;
			else if (bp->getDistance(i) < FLOAT_BURR_LEVEL_UPH) 
				m_UphBurrNumber[cameraIdx]++;
			else 
				m_TopBurrNumber[cameraIdx]++;
		}
		if (isChangeDate()) {
			writeToMysql(cameraIdx, cameraName);
			setDate();
			clearCameraData(cameraIdx);
		}

	}
	int get_LowBurrNumber(int camIdx) { return m_LowBurrNumber[camIdx]; }
	int get_MidBurrNumber(int camIdx) { return m_MidBurrNumber[camIdx]; }
	int get_HighBurrNumber(int camIdx) { return m_HighBurrNumber[camIdx]; }
	int get_UphBurrNumber(int camIdx) { return m_UphBurrNumber[camIdx]; }
	int get_TopBurrNumber(int camIdx) { return m_TopBurrNumber[camIdx]; }
	~DataCounter() { sqlHelper.free(); }
private:
	Logger *Log = new Logger("d:");
	int m_LowBurrNumber[MAX_CAMERA_NUM], m_MidBurrNumber[MAX_CAMERA_NUM], m_HighBurrNumber[MAX_CAMERA_NUM], m_UphBurrNumber[MAX_CAMERA_NUM], m_TopBurrNumber[MAX_CAMERA_NUM];
	void setDate() {
		char chTmp[20] = { '\0' };
		time_t t;
		time(&t);
		localtime_s(&m_date, &t);
		m_date.tm_year = m_date.tm_year + 1900;
		m_date.tm_mon = m_date.tm_mon + 1;
	}

	void clearData() {
		for (int i = 0; i < MAX_CAMERA_NUM; ++i) {
			m_LowBurrNumber[i] = 0;
			m_MidBurrNumber[i] = 0;
			m_HighBurrNumber[i] = 0;
			m_UphBurrNumber[i] = 0;
			m_TopBurrNumber[i] = 0;
		}
	}

	void clearCameraData(int cameraIdx) {
		m_LowBurrNumber[cameraIdx] = 0;
		m_MidBurrNumber[cameraIdx] = 0;
		m_HighBurrNumber[cameraIdx] = 0;
		m_UphBurrNumber[cameraIdx] = 0;
		m_TopBurrNumber[cameraIdx] = 0;
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
	
	void writeToMysql(int cameraIdx, string cameraName){
		try {
			Logger logger("d:");
			char insertCommand[400] = { 0 };
			snprintf(insertCommand, sizeof(insertCommand),
				"insert TB_BURR_COUNT_LOG(LOW_BURR_NUMBER, MID_BURR_NUMBER, HIGH_BURR_NUMBER, UPH_BURR_NUMBER, TOP_BURR_NUMBER, BURR_TYPE, LOG_TIME) value(%d, %d, %d, %d, %d, '%s', '%04d-%02d-%02d %02d:%02d')",
				m_LowBurrNumber[cameraIdx], m_MidBurrNumber[cameraIdx], m_HighBurrNumber[cameraIdx], m_UphBurrNumber[cameraIdx], m_TopBurrNumber[cameraIdx], cameraName.c_str(), m_date.tm_year, m_date.tm_mon, m_date.tm_mday, m_date.tm_hour, m_date.tm_min);
			logger.Log(insertCommand);
			sqlHelper.insert(insertCommand);
		}
		catch (...) {
			Log->Log("DataCounter WriteToMysql error");
		}
	}

	MysqlHelper sqlHelper;
	struct tm m_date;
	DataCounter() { 
		clearData();
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
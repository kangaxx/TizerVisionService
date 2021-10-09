#ifndef _CRC16_h_
#define _CRC16_h_

#include "Common.h"

#define HI4BITS(w)   ((BYTE) (((BYTE) (w) >> 4) & 0xF)) 
#define LO4BITS(w)   ((BYTE) ((w) & 0xF))  

class CRC16 {

protected:


private:
	static const UCHAR auchCRCHi[];
	static const UCHAR auchCRCLo[];
public:
	CRC16();
	~CRC16();
	WORD CalcCrcFast(const BYTE *puchMsg, WORD usDataLen);
	/* bytes in message      */
	static BYTE LRC(const BYTE *auchMsg, WORD usDataLen);
};

#endif  // _CRC16_h_
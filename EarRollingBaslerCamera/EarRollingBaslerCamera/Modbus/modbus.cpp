// modbus.cpp: implementation of the CModbus class.
//
//////////////////////////////////////////////////////////////////////

#include "modbus.h"
#include "ModbusMessages.h"
//#include "Global.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModbus::CModbus()
{
	m_wRetries=1;
	m_bThrowModbusException=FALSE;
	m_dwLockTimeout=1000;
	m_dwDelay=0;

	InitializeCriticalSection(&m_cs);
}

CModbus::~CModbus()
{
	DeleteCriticalSection(&m_cs);
}

#ifdef _SIMULATION_
WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , WORD* anRegValues) 
{
	if(NULL == g_pModbusThread)
	{
		return (ERR_INV_RESP);
	}
	switch(nDataStart)
	{
	case FUN_MODE:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.mode, sizeof(WORD) * nQtd);
		break;
	case FUN_UNSURE_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.unsure_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd1_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd2_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd3_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd4_address, sizeof(WORD) * nQtd);
		break;
	case FUN_OK1_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ok1_address, sizeof(WORD) * nQtd);
		break;
	case FUN_OK2_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ok2_address, sizeof(WORD) * nQtd);
		break;
	case FUN_FAIL_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.fail_address, sizeof(WORD) * nQtd);
		break;
	case FUN_LESS_MATERIAL_DELAY:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.less_material_delay, sizeof(WORD) * nQtd);
		break;
	case FUN_MATERIAL_SPACE:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.material_space, sizeof(WORD) * nQtd);
		break;
	case FUN_FAIL_NUMER_LIMIT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.fail_numer_limit, sizeof(WORD) * nQtd);
		break;
	case FUN_MATERIAL_LENGTH:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.material_length, sizeof(WORD) * nQtd);
		break;
	case FUN_DISCHARGE_TIME:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.discharge_time, sizeof(WORD) * nQtd);
		break;
	case FUN_VIBRATION_SPEED:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.vibration_speed, sizeof(WORD) * nQtd);
		break;
	case FUN_BELT_SPEED:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.belt_speed, sizeof(WORD) * nQtd);
		break;
	case FUN_GLASS_SPEED:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.glass_speed, sizeof(WORD) * nQtd);
		break;
	case FUN_PACKAGE_SIZE:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.package_size, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_RESULT_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd1_result_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_RESULT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd1_result, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_RESULT_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd2_result_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_RESULT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd2_result, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_RESULT_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd3_result_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_RESULT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd3_result, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_RESULT_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd4_result_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_RESULT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd4_result, sizeof(WORD) * nQtd);
		break;
	//case FUN_UNSURE_EMPTY:
	//	memcpy(anRegValues, &g_pModbusThread->m_CommonData.unsure_empty, sizeof(WORD) * nQtd);
	//	break;
	case FUN_GREEN_LIGHT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.green_light, sizeof(WORD) * nQtd);
		break;
	case FUN_RED_LIGHT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.red_light, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_BACK_LIGHT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd1_back_light, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_RING_LIGHT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd1_ring_light, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_BACK_LIGHT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd2_back_light, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_RING_LIGHT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd2_ring_light, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_BACK_LIGHT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd3_back_light, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_RING_LIGHT:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ccd4_ring_light, sizeof(WORD) * nQtd);
		break;
	case FUN_GLASS_ROTATION:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.glass_rotation, sizeof(WORD) * nQtd);
		break;
	case FUN_BELT_ROTATION:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.belt_rotation, sizeof(WORD) * nQtd);
		break;
	case FUN_SHOCKER_CONTROL:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.shocker_control, sizeof(WORD) * nQtd);
		break;
	case FUN_OK1_CONTROL:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ok1_control, sizeof(WORD) * nQtd);
		break;
	case FUN_OK2_CONTROL:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.ok2_control, sizeof(WORD) * nQtd);
		break;
	case FUN_UNSURE_DISCHARGE:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.unsure_discharge, sizeof(WORD) * nQtd);
		break;
	case FUN_POSITION_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.position_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_IMAGE_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd1_image_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_IMAGE_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd2_image_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_IMAGE_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd3_image_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_IMAGE_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.ccd4_image_address, sizeof(WORD) * nQtd);
		break;
	case FUN_ALARM:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.alarm, sizeof(WORD) * nQtd);
		break;
	case FUN_UNSURE_NUMBER:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.unsure_number, sizeof(WORD) * nQtd);
		break;
	case FUN_CURRENT_ADDRESS:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.current_address, sizeof(WORD) * nQtd);
		break;
	case FUN_CLEAR_DISK:
		memcpy(anRegValues, &g_pModbusThread->m_CommonData.clear_disk, sizeof(WORD) * nQtd);
		break;
	case FUN_MATERIAL_LENGTH_2_PC:
		memcpy(anRegValues, g_pModbusThread->m_CommonData.material_length_2_pc, sizeof(WORD) * nQtd);
		break;
	default:
		return (ERR_INV_RESP);
	}
	return(ERR_OK);
}//end ReadOutRegisters 

#else
WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , WORD* anRegValues) {

	
	short nReplyLength=(3 + 2 * nQtd);
	
	WORD nError;
	short nByte;
	short nRespByte;
	const short QUERY_LENGHT=6;
	int  iRetry=0;

	//BYTE abyQuery[QUERY_LENGHT + 2];
	//BYTE *abyReply = new BYTE[nReplyLength];// new int[nReplyLength];
	//Query 

	//modbus message 
	abyQuery[0]=(BYTE)nAddr; //ADDR	
	abyQuery[1]=READ_OUTPUT_REGISTERS; //Function 
	abyQuery[2]=HIBYTE(nDataStart);
	abyQuery[3]=LOBYTE(nDataStart);
	abyQuery[4]=HIBYTE(nQtd);
	abyQuery[5]=LOBYTE(nQtd);

	do {

		nError=TxRxMessage(abyQuery,6,abyReply, nReplyLength);

		if (nError==ERR_OK) {
			if ((abyReply[0]!=abyQuery[0])||(abyReply[1]!=abyQuery[1])) {
				nError=ERR_INV_RESP;
			}
			else {
				nByte=0;//return array 
				for(nRespByte=3;nRespByte<nReplyLength;nRespByte=(short)(nRespByte+2)){
					anRegValues[nByte] = (abyReply[nRespByte]);
					anRegValues[nByte]= ((anRegValues[nByte]<<8)|(abyReply[nRespByte+1]));
					nByte++;
				}
			}
		}//nerror=ERR_OK

		iRetry++;

		ErrorDelay(nError);

	} while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 


	ThrowModbusException(nError);

	//delete[] abyReply;
	return(nError);

}//end ReadOutRegisters 

#endif

WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , short* anRegValues) {


	//QMyWordArray anValues;
	WORD* anValues = new WORD[nQtd];
	WORD nError;
	//anValues.SetSize(anRegValues.GetSize());
	nError =ReadOutputRegisters(nAddr,nDataStart,nQtd,anValues);
	Word2ShortArray(anValues, nQtd, anRegValues);
	delete[] anValues;
	return nError;

}


WORD CModbus::ReadInputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , WORD* anRegValues) {

	//QMyByteArray abyQuery;
	short nReplyLength=(3 + 2 * nQtd);
	//QMyByteArray  abyReply;// new int[nReplyLength];
	WORD nError;
	short nByte;
	short nRespByte;
	const short QUERY_LENGHT=6;
	int  iRetry=0;


	//BYTE abyQuery[QUERY_LENGHT + 2];
	//BYTE* abyReply = new BYTE[nReplyLength];
	//abyQuery.SetSize(6);
	//abyReply.SetSize(nReplyLength);
	//Query 

	//modbus message 
	abyQuery[0]=(BYTE)nAddr; //ADDR	
	abyQuery[1]=READ_INPUT_REGISTERS; //Function 
	abyQuery[2]=HIBYTE(nDataStart);
	abyQuery[3]=LOBYTE(nDataStart);
	abyQuery[4]=HIBYTE(nQtd);
	abyQuery[5]=LOBYTE(nQtd);

	do {

		nError=TxRxMessage(abyQuery,6,abyReply, nReplyLength);


		if (nError==ERR_OK) {
			if ((abyReply[0]!=abyQuery[0])||(abyReply[1]!=abyQuery[1])) {
				nError=ERR_INV_RESP;
			}
			else {
				nByte=0;//return array 
				for(nRespByte=3;nRespByte<nReplyLength;nRespByte=(short)(nRespByte+2)){
					anRegValues[nByte] = (abyReply[nRespByte]);
					anRegValues[nByte]= ((anRegValues[nByte]<<8)|(abyReply[nRespByte+1]));
					nByte++;
				}

			}


		}//nerror=ERR_OK

		iRetry++;

		ErrorDelay(nError);

	} while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 


	ThrowModbusException(nError);

	//delete[] abyReply;
	return(nError);

}//end ReadInRegisters 


WORD CModbus::ReadInputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , short* anRegValues) {


	//QMyWordArray anValues;
	WORD* anValues = new WORD[nQtd];
	WORD nError;
	//anValues.SetSize(anRegValues.GetSize());
	nError =ReadInputRegisters(nAddr,nDataStart,nQtd,anValues);
	Word2ShortArray(anValues,nQtd, anRegValues);
	delete[] anValues;
	return nError;

}


//WORD CModbus::ReadInputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CDblArray& anRegValues,WORD wRealType) {
//

// QMyWordArray anValues;
// WORD nError;
// anValues.SetSize(4*anRegValues.GetSize());
// nError =ReadInputRegisters(nAddr,nDataStart,4*nQtd,anValues);
// Word2Dbl(anValues.GetData(),anValues.GetSize(),anRegValues.GetData(),anRegValues.GetSize(),wRealType);
// return nError;

//}

//WORD CModbus::ReadInputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CFloatArray& anRegValues,WORD wRealType) {
//

// QMyWordArray anValues;
// WORD nError;
// anValues.SetSize(2*anRegValues.GetSize());
// nError =ReadInputRegisters(nAddr,nDataStart,2*nQtd,anValues);
// Word2Float(anValues.GetData(),anValues.GetSize(),anRegValues.GetData(),anRegValues.GetSize(),wRealType);
// return nError;

//}

//WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CDblArray& anRegValues,WORD wRealType) {
//

// QMyWordArray anValues;
// WORD nError;
// anValues.SetSize(4*anRegValues.GetSize());
// nError =ReadOutputRegisters(nAddr,nDataStart,4*nQtd,anValues);
// Word2Dbl(anValues.GetData(),anValues.GetSize(),anRegValues.GetData(),anRegValues.GetSize(),wRealType);
// return nError;

//}

//WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CFloatArray& anRegValues,WORD wRealType) {
//

// QMyWordArray anValues;
// WORD nError;
// anValues.SetSize(2*anRegValues.GetSize());
// nError =ReadOutputRegisters(nAddr,nDataStart,2*nQtd,anValues);
// Word2Float(anValues.GetData(),anValues.GetSize(),anRegValues.GetData(),anRegValues.GetSize(),wRealType);
// return nError;

//}

//Read output status "coils" 
//return

WORD CModbus::ReadInputStatus(WORD nAddr, //Modbus device Address 
							  WORD nDataStart , //
							  WORD nQtd , 
							  BYTE* abCoilValues) {

								  static WORD const  QUERY_LENGHT=6;

								  //QMyByteArray abyQuery;
								  WORD nReplyLength=(3 + (nQtd + 7) / 8);
								  //QMyByteArray  abyReply;//[]  = new int[nReplyLength];
								  WORD nError;
								  WORD nRespByte;
								  WORD nCoil;
								  WORD nBit;
								  int  iRetry=0;

								  //set array sizes 
								  //BYTE abyQuery[QUERY_LENGHT + 2];
								  //BYTE* abyReply = new BYTE[nReplyLength];
								  //abyQuery.SetSize(QUERY_LENGHT);
								  //abyReply.SetSize(nReplyLength); 
								  //modbus message 
								  abyQuery[0]=(BYTE)nAddr; //ADDR	
								  abyQuery[1]=READ_INPUT_STATUS; //Function 
								  abyQuery[2]=HIBYTE(nDataStart);
								  abyQuery[3]=LOBYTE(nDataStart);
								  abyQuery[4]=HIBYTE(nQtd);
								  abyQuery[5]=LOBYTE(nQtd);

								  do {

									  nError=TxRxMessage(abyQuery,(QUERY_LENGHT),abyReply, nReplyLength);


									  if (nError==ERR_OK) {
										  if ((abyReply[0]!=abyQuery[0])||(abyReply[1]!=abyQuery[1])) {
											  nError=ERR_INV_RESP;
										  }
										  else {
											  nBit=0;
											  nRespByte=3;		
											  for(nCoil=0;nCoil<nQtd;nCoil++){

												  if (nBit>7){
													  nBit=0;
													  nRespByte++;
												  }

												  if ((abyReply[nRespByte] & (0x01<<nBit))>0){
													  abCoilValues[nCoil]= TRUE; 
												  }
												  else {
													  abCoilValues[nCoil]= FALSE; 
												  }
												  nBit++; //next bit 

											  }// end for 

										  }// else 


									  }//nerror=ERR_OK

									  iRetry++;

									  ErrorDelay(nError);

								  } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 


								  ThrowModbusException(nError);
								  //delete[] abyReply;
								  return(nError);

}//end ReadInputStatus 



//Read output status "coils" 
//return

WORD CModbus::ReadOutputStatus(WORD nAddr, //Modbus device Address 
							   WORD nDataStart , //
							   WORD nQtd , 
							   BYTE* abCoilValues) {

								   static WORD const  QUERY_LENGHT=6;

								   //QMyByteArray abyQuery;
								   WORD nReplyLength=(3 + (nQtd + 7) / 8);
								   //QMyByteArray  abyReply;//[]  = new int[nReplyLength];
								   WORD nError;
								   WORD nRespByte;
								   WORD nCoil;
								   WORD nBit;
								   int  iRetry=0;

								   //set array sizes 
								   //BYTE abyQuery[QUERY_LENGHT + 2];
								   //BYTE* abyReply = new BYTE[nReplyLength];
								   //abyQuery.SetSize(QUERY_LENGHT);
								   //abyReply.SetSize(nReplyLength); 
								   //modbus message 
								   abyQuery[0]=(BYTE)nAddr; //ADDR	
								   abyQuery[1]=READ_OUTPUT_STATUS; //Function 
								   abyQuery[2]=HIBYTE(nDataStart);
								   abyQuery[3]=LOBYTE(nDataStart);
								   abyQuery[4]=HIBYTE(nQtd);
								   abyQuery[5]=LOBYTE(nQtd);


								   do {

									   nError=TxRxMessage(abyQuery,(QUERY_LENGHT),abyReply, nReplyLength);


									   if (nError==ERR_OK) {
										   if ((abyReply[0]!=abyQuery[0])||(abyReply[1]!=abyQuery[1])) {
											   nError=ERR_INV_RESP;
										   }
										   else {
											   nBit=0;
											   nRespByte=3;		
											   for(nCoil=0;nCoil<nQtd;nCoil++){

												   if (nBit>7){
													   nBit=0;
													   nRespByte++;
												   }

												   if ((abyReply[nRespByte] & (0x01<<nBit))>0){
													   abCoilValues[nCoil]= TRUE; 
												   }
												   else {
													   abCoilValues[nCoil]= FALSE; 
												   }
												   nBit++; //next bit 

											   }// end for 

										   }// else 


									   }//nerror=ERR_OK

									   iRetry++;

									   ErrorDelay(nError);

								   } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 


								   ThrowModbusException(nError);
								   //delete[] abyReply;
								   return(nError);

}//end ReadOutStatus 

#ifdef _SIMULATION_
WORD CModbus::PresetMultipleRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , const WORD* anRegValues) 
{
	if(NULL == g_pModbusThread)
	{
		return (ERR_INV_RESP);
	}

	switch(nDataStart)
	{
	case FUN_MODE:
		memcpy(&g_pModbusThread->m_CommonData.mode, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_UNSURE_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.unsure_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd1_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd2_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd3_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd4_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_OK1_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ok1_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_OK2_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ok2_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_FAIL_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.fail_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_LESS_MATERIAL_DELAY:
		memcpy(&g_pModbusThread->m_CommonData.less_material_delay, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_MATERIAL_SPACE:
		memcpy(g_pModbusThread->m_CommonData.material_space, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_FAIL_NUMER_LIMIT:
		memcpy(&g_pModbusThread->m_CommonData.fail_numer_limit, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_MATERIAL_LENGTH:
		memcpy(g_pModbusThread->m_CommonData.material_length, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_DISCHARGE_TIME:
		memcpy(&g_pModbusThread->m_CommonData.discharge_time, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_VIBRATION_SPEED:
		memcpy(&g_pModbusThread->m_CommonData.vibration_speed, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_BELT_SPEED:
		memcpy(&g_pModbusThread->m_CommonData.belt_speed, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_GLASS_SPEED:
		memcpy(&g_pModbusThread->m_CommonData.glass_speed, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_PACKAGE_SIZE:
		memcpy(g_pModbusThread->m_CommonData.package_size, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_RESULT_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd1_result_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_RESULT:
		memcpy(&g_pModbusThread->m_CommonData.ccd1_result, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_RESULT_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd2_result_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_RESULT:
		memcpy(&g_pModbusThread->m_CommonData.ccd2_result, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_RESULT_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd3_result_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_RESULT:
		memcpy(&g_pModbusThread->m_CommonData.ccd3_result, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_RESULT_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd4_result_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_RESULT:
		memcpy(&g_pModbusThread->m_CommonData.ccd4_result, anRegValues, sizeof(WORD) * nQtd);
		break;
	//case FUN_UNSURE_EMPTY:
	//	memcpy(&g_pModbusThread->m_CommonData.unsure_empty, anRegValues, sizeof(WORD) * nQtd);
	//	break;
	case FUN_GREEN_LIGHT:
		memcpy(&g_pModbusThread->m_CommonData.green_light, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_RED_LIGHT:
		memcpy(&g_pModbusThread->m_CommonData.red_light, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_BACK_LIGHT:
		memcpy(&g_pModbusThread->m_CommonData.ccd1_back_light, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_RING_LIGHT:
		memcpy(&g_pModbusThread->m_CommonData.ccd1_ring_light, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_BACK_LIGHT:
		memcpy(&g_pModbusThread->m_CommonData.ccd2_back_light, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_RING_LIGHT:
		memcpy(&g_pModbusThread->m_CommonData.ccd2_ring_light, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_BACK_LIGHT:
		memcpy(&g_pModbusThread->m_CommonData.ccd3_back_light, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_RING_LIGHT:
		memcpy(&g_pModbusThread->m_CommonData.ccd4_ring_light, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_GLASS_ROTATION:
		memcpy(&g_pModbusThread->m_CommonData.glass_rotation, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_BELT_ROTATION:
		memcpy(&g_pModbusThread->m_CommonData.belt_rotation, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_SHOCKER_CONTROL:
		memcpy(&g_pModbusThread->m_CommonData.shocker_control, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_OK1_CONTROL:
		memcpy(&g_pModbusThread->m_CommonData.ok1_control, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_OK2_CONTROL:
		memcpy(&g_pModbusThread->m_CommonData.ok2_control, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_UNSURE_DISCHARGE:
		memcpy(&g_pModbusThread->m_CommonData.unsure_discharge, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_POSITION_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.position_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD1_IMAGE_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd1_image_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD2_IMAGE_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd2_image_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD3_IMAGE_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd3_image_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CCD4_IMAGE_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.ccd4_image_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_ALARM:
		memcpy(&g_pModbusThread->m_CommonData.alarm, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_UNSURE_NUMBER:
		memcpy(g_pModbusThread->m_CommonData.unsure_number, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CURRENT_ADDRESS:
		memcpy(g_pModbusThread->m_CommonData.current_address, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_CLEAR_DISK:
		memcpy(&g_pModbusThread->m_CommonData.clear_disk, anRegValues, sizeof(WORD) * nQtd);
		break;
	case FUN_MATERIAL_LENGTH_2_PC:
		memcpy(g_pModbusThread->m_CommonData.material_length_2_pc, anRegValues, sizeof(WORD) * nQtd);
		break;


	default:
		return (ERR_INV_RESP);
	}
	return(ERR_OK);
}//end ReadOutRegisters 

#else
WORD CModbus::PresetMultipleRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , const WORD* anRegValues) {


	const WORD QUERY_LENGHT=(7+2*nQtd);
	//QMyByteArray abyQuery;//[QUERY_LENGHT];
	const short nReplyLength = (6);
	//QMyByteArray  abyReply;
	short nError;
	int	  iReg; 	
	int  iRetry=0;


	//Query 
	//BYTE* abyQuery = new BYTE[QUERY_LENGHT + 2];
	//BYTE abyReply[nReplyLength];
	//abyQuery.SetSize(QUERY_LENGHT);
	//abyReply.SetSize(nReplyLength);
	//modbus message 
	abyQuery[0]=(BYTE)nAddr; //ADDR	
	abyQuery[1]=PRESET_MULTIPLE_REGISTERS; //Function 
	abyQuery[2]=HIBYTE(nDataStart);
	abyQuery[3]=LOBYTE(nDataStart);
	abyQuery[4]=HIBYTE(nQtd);
	abyQuery[5]=LOBYTE(nQtd);
	abyQuery[6]=(2*nQtd);

	for (iReg=0;iReg<(nQtd);iReg++){
		abyQuery[7+iReg*2]=HIBYTE(anRegValues[iReg]);
		abyQuery[7+iReg*2+1]=LOBYTE(anRegValues[iReg]);
	}

	do {
		nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply, nReplyLength);


		if (nError==ERR_OK) {

			for (iReg=0;iReg<6;iReg++){
				if (abyReply[iReg]!=abyQuery[iReg]) {
					nError=ERR_INV_RESP;
					break;
				}
			}

		}//nerror=ERR_OK

		iRetry++;

		ErrorDelay(nError);

	} while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 

	
	ThrowModbusException(nError);
	//delete[] abyQuery;
	return(nError);

}//end PresetMultipleRegisters 
#endif

WORD CModbus::PresetMultipleRegisters(WORD nAddr, //address of modbus device 
									  WORD nDataStart , //start address, first address is 0 "ZERO" 
									  WORD nQtd , //number of reg . to write 
									  const short* anRegValues) {

										  WORD* anValues = new WORD[nQtd];
										  Short2WordArray(anRegValues, nQtd, anValues);
										  WORD wReturn = PresetMultipleRegisters(nAddr,nDataStart,nQtd ,anValues);
										  delete[] anValues;
										  return wReturn;

} //end PresetMultipleRegisters //QMyShortArray


//WORD CModbus::PresetMultipleRegisters(WORD nAddr, //address of modbus device 
//						  WORD nDataStart , //start address, first address is 0 "ZERO" 
//						  WORD nQtd , //number of reg . to write 
//						  CDblArray& adRegValues,
//						  WORD wRealType) {
//   
// QMyWordArray anValues;
// anValues.SetSize(nQtd*4);
// ASSERT(nQtd<=adRegValues.GetSize());
// 
// Dbl2Word(adRegValues.GetData(),nQtd,anValues.GetData(),nQtd*4,wRealType);

// return PresetMultipleRegisters(nAddr,nDataStart,nQtd*4 ,anValues);
// 
//} //end PresetMultipleRegisters //QMyShortArray


//WORD CModbus::PresetMultipleRegisters(WORD nAddr, //address of modbus device 
//							  WORD nDataStart , //start address, first address is 0 "ZERO" 
//							  WORD nQtd , //number of reg . to write 
//							  CFloatArray& afRegValues,
//							  WORD wRealType) {
//    
//	 QMyWordArray anValues;
//	 anValues.SetSize(nQtd*2);
//	 ASSERT(nQtd<=afRegValues.GetSize());
//	 
//	 Float2Word(afRegValues.GetData(),nQtd,anValues.GetData(),nQtd*2,wRealType);
//
//	 return PresetMultipleRegisters(nAddr,nDataStart,nQtd*2 ,anValues);
//	 
// } //end PresetMultipleRegisters //CFloatArray



WORD CModbus::PresetSingleRegister(WORD nAddr,  WORD nRegister , WORD nRegValue) {


	const short QUERY_LENGHT=(6);
	//QMyByteArray abyQuery;
	const short nReplyLength=(6);
	//QMyByteArray  abyReply;
	short nError;
	int	  iReg; 
	int  iRetry=0;



	//Query 
	//BYTE abyQuery[QUERY_LENGHT + 2];
	//BYTE abyReply[nReplyLength];
	//abyQuery.SetSize(QUERY_LENGHT); 
	//abyReply.SetSize(nReplyLength);

	//modbus message 
	abyQuery[0]=(BYTE)nAddr; //ADDR	
	abyQuery[1]=PRESET_SINGLE_REGISTER; //Function 
	abyQuery[2]=HIBYTE(nRegister);
	abyQuery[3]=LOBYTE(nRegister);
	abyQuery[4]=HIBYTE(nRegValue);
	abyQuery[5]=LOBYTE(nRegValue);

	do {

		nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply, nReplyLength);


		if (nError==ERR_OK) {

			for (iReg=0;iReg<QUERY_LENGHT;iReg++){
				if (abyReply[iReg]!=abyQuery[iReg]) {
					nError=ERR_INV_RESP;
					break;
				}
			}

		}//nerror=ERR_OK

		iRetry++;

		ErrorDelay(nError);

	} while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 


	ThrowModbusException(nError);
	return(nError);

}//end PresetSingleRegister

WORD CModbus::PresetSingleRegister(WORD nAddr,  WORD nRegister , short int nRegValue) {

	return(PresetSingleRegister(nAddr,nRegister ,(WORD)nRegValue));
}


WORD CModbus::PresetSingleRegister(WORD nAddr,  WORD nRegister ,float fRegValue, WORD wRealType){

	//QMyWordArray anRegs;
	WORD* anRegs = new WORD[2];
	WORD* pRegValue= (WORD*)&fRegValue; 

	//anRegs.SetSize(2);

	if (REAL_NORMAL== wRealType) { 

		anRegs[0]= *(pRegValue+0); 
		anRegs[1]= *(pRegValue+1); 

	}
	else {

		anRegs[0]= *(pRegValue+1); 
		anRegs[1]= *(pRegValue+0); 

	}

	WORD wReturn = ( PresetMultipleRegisters(nAddr , nRegister , 2 ,anRegs));
	delete[] anRegs;
	return wReturn;
}

WORD CModbus::PresetSingleRegister(WORD nAddr,  WORD nRegister , double dRegValue, WORD wRealType){

	//QMyWordArray anRegs;
	WORD* anRegs = new WORD[4];
	WORD* pRegValue= (WORD*)&dRegValue; 

	//anRegs.SetSize(4);

	if (REAL_NORMAL== wRealType) { 

		anRegs[0]= *(pRegValue+0); 
		anRegs[1]= *(pRegValue+1); 
		anRegs[2]= *(pRegValue+2); 
		anRegs[3]= *(pRegValue+3); 

	}
	else {

		anRegs[0]= *(pRegValue+3); 
		anRegs[1]= *(pRegValue+2); 
		anRegs[2]= *(pRegValue+1); 
		anRegs[3]= *(pRegValue+0); 

	}

	WORD wReturn = ( PresetMultipleRegisters(nAddr , nRegister , 4 ,anRegs));
	delete[] anRegs;
	return wReturn;

}


WORD CModbus::ForceSingleCoil(WORD nAddr, WORD nRegister, BOOL bCoilValue) {


	const short QUERY_LENGHT=(6);
	//QMyByteArray abyQuery;
	const short nReplyLength=(6);
	//QMyByteArray  abyReply;
	short nError;
	int	  iReg;
	int iRetry=0;



	//Query 
	//BYTE abyQuery[QUERY_LENGHT + 2];
	//BYTE abyReply[nReplyLength];
	//abyQuery.SetSize(QUERY_LENGHT);
	//abyReply.SetSize(nReplyLength);
	//modbus message 
	abyQuery[0]=(BYTE)nAddr; //ADDR	
	abyQuery[1]=FORCE_SINGLE_COIL; //Function 
	abyQuery[2]=HIBYTE(nRegister); 
	abyQuery[3]=LOBYTE(nRegister);
	abyQuery[4]=HIBYTE(CmodBool(bCoilValue));
	abyQuery[5]=LOBYTE(CmodBool(bCoilValue));


	do {
		nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply, nReplyLength);


		if (nError==ERR_OK) {

			for (iReg=0;iReg<QUERY_LENGHT;iReg++){
				if (abyReply[iReg]!=abyQuery[iReg]) {
					nError=ERR_INV_RESP;
					break;
				}
			}

		}//nerror=ERR_OK

		iRetry++;

		ErrorDelay(nError);

	} while ((iRetry<m_wRetries)&&(ERR_OK!=nError));


	ThrowModbusException(nError);

	return(nError);

}//end ForceSingleCoil 


WORD CModbus::ForceMultipleCoils(WORD nAddr,  WORD nDataStart , WORD nQtd , BYTE* abCoilValues) {


	const short QUERY_LENGHT=(7+(nQtd + 7)/8);
	//QMyByteArray abyQuery;
	const short nReplyLength=(6);
	//QMyByteArray  abyReply;
	short nError;
	short nQueryByte;
	int   iCoil; 
	int   iBit;
	int   iReg;
	int  iRetry=0;



	//Query 
	//BYTE* abyQuery = new BYTE[QUERY_LENGHT + 2];
	//BYTE abyReply[nReplyLength];
	//abyQuery.SetSize(QUERY_LENGHT);
	//abyReply.SetSize(nReplyLength); 

	//modbus message 
	abyQuery[0]=(BYTE)nAddr; //ADDR	
	abyQuery[1]=FORCE_MULTIPLE_COILS; //Function 
	abyQuery[2]=HIBYTE(nDataStart);
	abyQuery[3]=LOBYTE(nDataStart);
	abyQuery[4]=HIBYTE(nQtd);
	abyQuery[5]=LOBYTE(nQtd);
	abyQuery[6]=(BYTE)((nQtd + 7)/8);
	nQueryByte=7;
	iBit=0;

	for (iCoil=0;iCoil<(nQtd);iCoil++){
		if (iBit>7){
			iBit=0;
			nQueryByte++;
		}

		if (abCoilValues[iCoil]) {
			abyQuery[nQueryByte]=(abyQuery[nQueryByte] | (0x01<<iBit));
		}
		iBit++;

	}

	do {
		nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply, nReplyLength);


		if (nError==ERR_OK) {

			for (iReg=0;iReg<6;iReg++){
				if (abyReply[iReg]!=abyQuery[iReg]) {
					nError=ERR_INV_RESP;
					break;
				}
			}

		}//nerror=ERR_OK
		iRetry++;

		ErrorDelay(nError);

	} while ((iRetry<m_wRetries)&&(ERR_OK!=nError));


	ThrowModbusException(nError);
	//delete[] abyQuery;
	return(nError);

}//end ForceMultipleCoils 



WORD CModbus::LoopbackTest(WORD nAddr) {

	static const WORD QUERY_LENGHT=(6);
	static const WORD nReplyLength=(6);
	//QMyByteArray abyQuery; 
	//QMyByteArray abyReply;
	WORD  nError;
	int  iReg;
	int  iRetry=0;

	//BYTE abyQuery[QUERY_LENGHT + 2];
	//BYTE abyReply[nReplyLength];
	//abyQuery.SetSize(QUERY_LENGHT);
	//abyReply.SetSize(nReplyLength);
	//modbus message 
	abyQuery[0]=(BYTE)nAddr; //ADDR	
	abyQuery[1]=LOOP_BACK_TEST; //Function 
	abyQuery[2]=HIBYTE(0); //diag. code=0
	abyQuery[3]=LOBYTE(0);
	abyQuery[4]=0xA5;
	abyQuery[5]=0x37;

	do {

		nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply,nReplyLength);

		if (nError==ERR_OK) {

			for (iReg=0;iReg<QUERY_LENGHT;iReg++){
				if (abyReply[iReg]!=abyQuery[iReg]) {
					nError=ERR_INV_RESP;
					break;
				}
			}

		}

		iRetry++;

		ErrorDelay(nError);

	} while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 

	ThrowModbusException(nError);

	return(nError);

}//end LoopBackTest



string CModbus::ErrorMessage(WORD wErrorCode){

	string sRetValue("");
	string sAux;

	if (wErrorCode>=ERR_EXCPTION_CODE)
	{
		//sAux = QString::number(wErrorCode-ERR_EXCPTION_CODE);
		char txt[128];
		sprintf_s(txt, "%d", wErrorCode-ERR_EXCPTION_CODE);
		sAux = txt;
		sRetValue=MSG_MODBUS_ERR_EXCPTION_CODE;
		sRetValue+=sAux;
	}
	else if(ERR_OK==wErrorCode)
	{ 
		sRetValue=MSG_MODBUS_ERR_OK;
	} 
	else if(ERR_TIMEOUT==wErrorCode) 
	{
		sRetValue=MSG_MODBUS_ERR_TIMEOUT;
	}
	else if (ERR_INV_RESP==wErrorCode) 
	{
		sRetValue=MSG_MODBUS_ERR_INV_RESP;
	}
	else if (ERR_CRC==wErrorCode) 
	{
		sRetValue=MSG_MODBUS_ERR_CRC;
	}
	else if (ERR_WR_PORT==wErrorCode) 
	{
		sRetValue=MSG_MODBUS_ERR_WR_PORT;
	}

	else if (ERR_RD_PORT==wErrorCode) 
	{
		sRetValue=MSG_MODBUS_ERR_RD_PORT;
	}

	else if (ERR_NOT_INT==wErrorCode) 
	{
		sRetValue=MSG_MODBUS_ERR_NOT_INT;
	}

	else if (ERR_LOCK_TIME_OUT==wErrorCode) 
	{
		sRetValue=MSG_MODBUS_ERR_LOCK_TIME_OUT;
	}


	return sRetValue;

}


float CModbus::ResponseTime() {

	float fRespTime = 0;	 

	if(Lock()){

		//fRespTime=m_PerfCounter.CountToMiliSec(m_PerfCounter.DiffCounts());
		Unlock();
	}
	return fRespTime;
}

//void CModbus::Serialize( CArchive& archive )
//{
//	// call base class function first
//	// base class is CObject in this case
//	CObject::Serialize( archive );
//
//
//	// now do the stuff for our specific class
//	if( archive.IsStoring() ){
//		archive << m_wRetries << m_bThrowModbusException << m_dwDelay;
//	} 
//	else {
//		archive >> m_wRetries >> m_bThrowModbusException >> m_dwDelay;
//	}
//
//}

//Delay Error 
void CModbus::ErrorDelay(WORD nError) {

	//if (ERR_OK!=nError) {
	//	 ::Sleep(m_dwDelay);
	//}

}


//lock com device
BOOL CModbus::Lock() {
	//return(m_csComm.Lock(m_dwLockTimeout));
	//if(m_csComm.tryLock(m_dwLockTimeout))
	{
		//m_csComm.lock();
		EnterCriticalSection(&m_cs);
		return TRUE;
	}
	//return FALSE;	
}


//unlock com device
BOOL CModbus::Unlock() {
	//return m_csComm.Unlock();
	//m_csComm.unlock();
	LeaveCriticalSection(&m_cs);
	return TRUE;
}




//private functions 

//C++ boolean to modbus boolean 
WORD CModbus::CmodBool(BOOL bCoil) {
	if (bCoil) {
		return(0xFF00);
	}
	else {
		return(0x0000);
	}

}


void CModbus::Short2WordArray(const short* aiArray, const WORD size, WORD* anArray){
	int i;
	//anArray.SetSize(aiArray.GetSize());
	for (i=0;i<size/*aiArray.GetSize()*/;++i){
		anArray[i]=(WORD)aiArray[i];
	}
}

void CModbus::Word2ShortArray(const WORD* anArray, const WORD size, short* aiArray){
	int i;

	//aiArray.SetSize(anArray.GetSize());
	for (i=0;i<size/*aiArray.GetSize()*/;++i){
		aiArray[i]=(short)anArray[i];

	}
}

void CModbus::Word2Dbl(const WORD* pawSrc, int iSrcSize,double* padDest, int iDestSize,WORD wRealType){
	int i;
	WORD* pwDest;

	ASSERT(iSrcSize==(4*iDestSize));

	for (i=0;i<iDestSize;++i){
		pwDest = (WORD*)(padDest+i);
		if (REAL_NORMAL==wRealType) {
			*(pwDest+0)= pawSrc[4*i+0];
			*(pwDest+1)= pawSrc[4*i+1];
			*(pwDest+2)= pawSrc[4*i+2];
			*(pwDest+3)= pawSrc[4*i+3];
		}
		else {
			*(pwDest+0)= pawSrc[4*i+3];
			*(pwDest+1)= pawSrc[4*i+2];
			*(pwDest+2)= pawSrc[4*i+1];
			*(pwDest+3)= pawSrc[4*i+0];
		}
	}
}


void CModbus::Word2Float(const WORD* pawSrc, int iSrcSize,float* padDest, int iDestSize,WORD wRealType){
	int i;
	WORD* pwDest;

	ASSERT(iSrcSize==(2*iDestSize));

	for (i=0;i<iDestSize;++i){
		pwDest = (WORD*)(padDest+i);
		if (REAL_NORMAL==wRealType) {
			*(pwDest+0)= pawSrc[2*i+0];
			*(pwDest+1)= pawSrc[2*i+1];
		}
		else {
			*(pwDest+0)= pawSrc[2*i+1];
			*(pwDest+1)= pawSrc[2*i+0];
		}
	}
}


void CModbus::Dbl2Word(const double* padSrc, int iSrcSize,WORD* pawDest, int iDestSize,WORD wRealType){


	WORD* pRegValue; 
	int i;

	ASSERT((4*iSrcSize)==(iDestSize));

	for(i=0;i<iSrcSize;i++) {

		pRegValue= (WORD*)(padSrc+i);

		if (REAL_NORMAL== wRealType) { 

			pawDest[4*i+0]= *(pRegValue+0); 
			pawDest[4*i+1]= *(pRegValue+1); 
			pawDest[4*i+2]= *(pRegValue+2); 
			pawDest[4*i+3]= *(pRegValue+3); 

		}
		else {

			pawDest[4*i+0]= *(pRegValue+3); 
			pawDest[4*i+1]= *(pRegValue+2); 
			pawDest[4*i+2]= *(pRegValue+1); 
			pawDest[4*i+3]= *(pRegValue+0); 

		}
	}
}


void CModbus::Float2Word(const float* padSrc, int iSrcSize,WORD* pawDest, int iDestSize,WORD wRealType){


	WORD* pRegValue; 
	int i;

	ASSERT((2*iSrcSize)==(iDestSize));

	for(i=0;i<iSrcSize;i++) {

		pRegValue= (WORD*)(padSrc+i);

		if (REAL_NORMAL== wRealType) { 

			pawDest[2*i+0]= *(pRegValue+0); 
			pawDest[2*i+1]= *(pRegValue+1); 
		}
		else {

			pawDest[2*i+0]= *(pRegValue+1); 
			pawDest[2*i+1]= *(pRegValue+0); 
		}
	}
}


WORD CModbus::Retries() const {
	return m_wRetries;
}

void CModbus::Retries(WORD wRetries){
	m_wRetries=wRetries;
}


DWORD  CModbus::Delay(){

	return m_dwDelay;

}


void  CModbus::Delay(DWORD dwDelay){

	m_dwDelay=dwDelay;

}


void CModbus::ThrowException(BOOL bThrow)
{
	m_bThrowModbusException=bThrow;
}

BOOL CModbus::ThrowException() const
{
	return m_bThrowModbusException;
}


void CModbus::ThrowModbusException(WORD wErrorcode)
{
	if ((ERR_OK!=wErrorcode)&&(m_bThrowModbusException)) {
		//throw new CModbusException(ErrorMessage(wErrorcode),wErrorcode);
		throw ErrorMessage(wErrorcode);
	}
}

//
//void CModbus::DoEvents() {
//
//	MSG message;
//
//	if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
//		::TranslateMessage(&message);
//		::DispatchMessage(&message);
//	}
//}

//Modbus Functions 
const BYTE CModbus::READ_OUTPUT_REGISTERS=3;
const BYTE CModbus::READ_INPUT_REGISTERS=4; 
const BYTE CModbus::READ_OUTPUT_STATUS=1; 
const BYTE CModbus::READ_INPUT_STATUS=2;
const BYTE CModbus::PRESET_MULTIPLE_REGISTERS=16;
const BYTE CModbus::PRESET_SINGLE_REGISTER=6;
const BYTE CModbus::LOOP_BACK_TEST=8;
const BYTE CModbus::FORCE_SINGLE_COIL=5;
const BYTE CModbus::FORCE_MULTIPLE_COILS=15;





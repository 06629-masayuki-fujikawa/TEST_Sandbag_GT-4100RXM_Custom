#ifndef _EX_I2CDATA_H_
#define _EX_I2CDATA_H_

#include "I2c_driver.h"
/****************************************/
/*	I2C_Arbitration_ManagerInfo			*/
/****************************************/
extern	I2C_ARBITRATION_MANAGER		I2cAM;			// I2C_Arbitration_ManagerInfo
extern	I2C_I2CDEV2_1_SIG_REQ			I2cDev2_1Signal;	// DTR, RTS, InputSignal
/****************************************************************************/
/*	Private global variables(Application)									*/
/****************************************************************************/
extern	unsigned char 	trm_adr_I2CDEV2_1[ADDR_LEN_I2CDEV2_1];
extern	unsigned char 	trm_adr_I2CDEV2_2[ADDR_LEN_I2CDEV2_2];
extern	unsigned char 	rcv_adr_I2CDEV2_1[ADDR_LEN_I2CDEV2_1];
extern	unsigned char 	rcv_adr_I2CDEV2_2[ADDR_LEN_I2CDEV2_2];
extern	unsigned char 	trm_adr_RP[ADDR_LEN_RP];
extern	unsigned char 	trm_adr_JP[ADDR_LEN_JP];
extern	unsigned char 	rcv_adr_RP[ADDR_LEN_RP];
extern	unsigned char 	rcv_adr_JP[ADDR_LEN_JP];
extern	unsigned char 	trm_adr_RTC[ADDR_LEN_RTC];
extern	unsigned char 	rcv_adr_RTC[ADDR_LEN_RTC];
/****************************************************************************/
/****************************************************************************/
extern	I2C_MESSAGE 	iic_buff_prm[2];			// Parameter of Write/Read Driver(0/1)
/****************************************************************************/
/*	Private global variables(Driver)										*/
/****************************************************************************/
extern	IIC_MILESTONE				IicMilestone;		// 
extern	I2C_MESSAGE 				iic_ActiveBuff;		// I2c_driverì‡ÇÃWrite/ReadópStructure
extern	enum RiicInternalMode_e 	iic_mode;			// I2c_driverì‡ÇÃMode(IIC_MODE_I2C_)
extern	enum RiicStatus_e 			iic_status;			// I2c_driverì‡ÇÃStatus(RIIC_STATUS_)
extern	unsigned long 				iic_trm_cnt;		// I2c_driverì‡ÇÃSendêîâ¡éZCounter
extern	unsigned long 				iic_rcv_cnt;		// I2c_driverì‡ÇÃRecvêîâ¡éZCounter(ä‹ÇﬁCPUÇ©ÇÁëóêMÇµÇΩSlaveAddress)
/****************************************************************************/
/*	I2C_SC16_Internal Registers												*/
/****************************************************************************/
extern	I2C_SC16_REGS	I2cSc16Regs[SC16_IC_CH_NUM];
/****************************************************************************/
extern	unsigned short	TickCnt_Start[8];
extern	unsigned short	TickCnt_Stop[8];
extern	unsigned short	TickCnt_RSLT_Cnt[8];
extern	unsigned short	TickCnt_RSLT_Value[8][128];
extern	unsigned short	TickCntBoxNo[8];
extern	unsigned short	TickCntValue[8][32];
extern	unsigned short	DebugSignalOut;

#endif	// _EX_I2CDATA_H_

#include "I2c_driver.h"
/****************************************/
/*	I2C_Arbitration_ManagerInfo			*/
/****************************************/
I2C_ARBITRATION_MANAGER		I2cAM;			// I2C_Arbitration_ManagerInfo
I2C_I2CDEV2_1_SIG_REQ			I2cDev2_1Signal;	// DTR, RTS, InputSignal
/****************************************************************************/
/*	Private global variables(Application)									*/
/****************************************************************************/
unsigned char 	trm_adr_I2CDEV2_1[ADDR_LEN_I2CDEV2_1];
unsigned char 	trm_adr_I2CDEV2_2[ADDR_LEN_I2CDEV2_2];
unsigned char 	rcv_adr_I2CDEV2_1[ADDR_LEN_I2CDEV2_1];
unsigned char 	rcv_adr_I2CDEV2_2[ADDR_LEN_I2CDEV2_2];
unsigned char 	trm_adr_RP[ADDR_LEN_RP];
unsigned char 	trm_adr_JP[ADDR_LEN_JP];
unsigned char 	rcv_adr_RP[ADDR_LEN_RP];
unsigned char 	rcv_adr_JP[ADDR_LEN_JP];
unsigned char 	trm_adr_RTC[ADDR_LEN_RTC];
unsigned char 	rcv_adr_RTC[ADDR_LEN_RTC];
/****************************************************************************/
I2C_MESSAGE 	iic_buff_prm[2];			// Parameter of Write/Read Driver(0/1)
/****************************************************************************/
/*	I2c_driver Variables													*/
/****************************************************************************/
I2C_MESSAGE 				iic_ActiveBuff;		// I2c_driverì‡ÇÃWrite/ReadópStructure
enum RiicInternalMode_e 	iic_mode;			// I2c_driverì‡ÇÃMode(IIC_MODE_I2C_)
enum RiicStatus_e 			iic_status;			// I2c_driverì‡ÇÃStatus(RIIC_STATUS_)
unsigned long 				iic_trm_cnt;		// I2c_driverì‡ÇÃSendêîâ¡éZCounter
unsigned long 				iic_rcv_cnt;		// I2c_driverì‡ÇÃRecvêîâ¡éZCounter(ä‹ÇﬁCPUÇ©ÇÁëóêMÇµÇΩSlaveAddress)
IIC_MILESTONE				IicMilestone;		// I2c_driverì‡ÇÃMilestoneèÓïÒ
/****************************************************************************/
/*	I2C_SC16_Internal Registers												*/
/****************************************************************************/
I2C_SC16_REGS	I2cSc16Regs[SC16_IC_CH_NUM];
/****************************************************************************/
unsigned short	TickCnt_Start[8];
unsigned short	TickCnt_Stop[8];
unsigned short	TickCnt_RSLT_Cnt[8];
unsigned short	TickCnt_RSLT_Value[8][128];
unsigned short	TickCntBoxNo[8];
unsigned short	TickCntValue[8][32];
unsigned short	DebugSignalOut;

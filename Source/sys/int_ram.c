/********************************************************************************************/
/*																							*/
/*	�����ݏ����̓��A����ROM,����RAM�œ��삳������݂̂̂������ɒu��							*/
/*		rx630_int_ram.c																		*/
/*			Excep_IRQ4			#RTC_IRQ/RTC���������M��(1min�Ԋu)							*/
/*			Excep_IRQ5			#EX_IRQ1/UARTcontroler1������								*/
/*			Excep_IRQ8			#EX_IRQ2/UARTcontroler2������								*/
/*			Excep_IRQ15			#ETHER_IRQ/Ethernetcontroler������							*/
/*			Excep_TPU0_TGI0A	20msTimer�ݒ�(MHz)		//	Tpu0TGIA						*/
/*			Excep_TPU1_TGI1A	 2msTimer�ݒ�(MHz)		//	Tpu1TGIA						*/
/*			Excep_TPU2_TGI2A	 1msTimer�ݒ�(MHz)		//	Tpu2TGIA						*/
/*			Excep_CAN1_RXM1		�@����ʐM(MailBox):��M									*/
/*			Excep_CAN1_TXM1		�@����ʐM(MailBox):���M									*/
/*			Excep_CANi_ERSi		�@����ʐM(MailBox):Error									*/
/*			Excep_RIIC1_EEI1	RS232C�ʐMDevice����(IC1,2,3): TMOF, AL, SP, Nack, ST		*/
/*			Excep_RIIC1_RXI1	RS232C�ʐMDevice����(IC1,2,3): ��M							*/
/*			Excep_RIIC1_TXI1	RS232C�ʐMDevice����(IC1,2,3): ���M							*/
/*			Excep_RIIC1_TEI1	RS232C�ʐMDevice����(IC1,2,3): Transmitt End				*/
/*			Excep_SCI2_RXI2		JVMA(���ү�,����ذ��):��M									*/
/*			Excep_SCI2_TXI2		JVMA(���ү�,����ذ��):���M									*/
/*			Excep_SCI2_TEI2		JVMA(���ү�,����ذ��):Error									*/
/*			Excep_SCI3_RXI3		���CReader����:��M											*/
/*			Excep_SCI3_TXI3		���CReader����:���M											*/
/*			Excep_SCI3_TEI3		���CReader����:Error										*/
/*			Excep_SCI4_RXI4		Flap����:��M												*/
/*			Excep_SCI4_TXI4		Flap����:���M												*/
/*			Excep_SCI4_TEI4		Flap����:Error												*/
/*			Excep_SCI6_RXI6		Bluetooth:��M												*/
/*			Excep_SCI6_TXI6		Bluetooth:���M												*/
/*			Excep_SCI6_TEI6		Bluetooth:Error												*/
/*			Excep_SCI7_RXI7		serverFOMA_Rau:��M											*/
/*			Excep_SCI7_TXI7		serverFOMA_Rau:���M											*/
/*			Excep_SCI7_TEI7		serverFOMA_Rau:Error										*/
/*			Excep_SCI9_RXI9		NT-NET�ʐM:��M												*/
/*			Excep_SCI9_TXI9		NT-NET�ʐM:���M												*/
/*			Excep_SCI9_TEI9		NT-NET�ʐM:Error											*/
/*			Excep_SCI10_RXI10	��ڐGIC:��M												*/
/*			Excep_SCI10_TXI10	��ڐGIC:���M												*/
/*			Excep_SCI10_TEI10	��ڐGIC:Error												*/
/*			Excep_SCI11_RXI11	MultiICcardR/W:��M											*/
/*			Excep_SCI11_TXI11	MultiICcardR/W:���M											*/
/*			Excep_SCI11_TEI11	MultiICcardR/W:Error										*/
/*		Unused_Variable_Interrupt															*/
/*		Project������intprg.c������Customize												*/
/*			IIC_EEI_Int																		*/
/*				IIC_EEI_IntTimeOut															*/
/*				IIC_EEI_IntAL																*/
/*					IIC_GenClkSP															*/
/*				IIC_EEI_IntNack																*/
/*				IIC_EEI_IntSP																*/
/*				IIC_EEI_IntST																*/
/*			IIC_RXI_Int																		*/
/*				IIC_RXI_IntI2cRead															*/
/*			IIC_TXI_Int																		*/
/*				IIC_TXI_IntI2cWrite															*/
/*				IIC_TXI_IntI2cRead															*/
/*			IIC_TEI_Int																		*/
/*				IIC_TEI_IntI2cWrite															*/
/*				IIC_TEI_IntI2cRead															*/
/*			IIC_Error																		*/
/********************************************************************************************/
#include	<string.h>
#include 	<machine.h>
#include	"iodefine.h"
#include	"vect.h"
#include	"extern_Debugdata.h"
#include 	"extern_I2Cdata.h"
#include 	"rtc_readwrite.h"
#include 	"extern_Etherdata.h"
#include	"system.h"
#include "FlashSerial.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"mdl_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"sysmnt_def.h"
#include	"can_api.h"
#include	"LCM.h"
#include	"toS.h"
#include	"can_api.h"
#include	"can_def.h"
#include 	"ksg_def.h"
#include	"bluetooth.h"
#include	"ftpctrl.h"
#include	"pip_def.h"
#include	"raudef.h"

extern		LK_Int_RXI( void );
extern		LK_Int_TXI( void );
extern		LK_Int_ERI( void );
extern		FB_Int_RXI( void );
extern		FB_Int_TXI( void );
extern		FB_Int_ERI( void );
extern		void	KSG_RauIntervalTimer( void );
extern		void	KSG_Rau_Int_RXI( void );
extern		void	KSG_Rau_Int_TXI( void );
extern		void	KSG_Rau_Int_ERI( void );
extern 		void	KSG_Rau_Int_TEI( void );
extern		void	CNM_Int_ERI( void );
extern		void	RAU_20msInt( void );

/****************************************************************************/
/*	Functions Prototypes for I2c											*/
/****************************************************************************/
void IIC_EEI_Int(void);
void IIC_RXI_Int(void);
void IIC_TXI_Int(void);
void IIC_TEI_Int(void);
void IIC_EEI_IntTimeOut(void);
void IIC_EEI_IntAL(void);
void IIC_EEI_IntSP(void);
void IIC_EEI_IntST(void);
void IIC_EEI_IntNack(void);
void IIC_RXI_IntI2cRead (void);
void IIC_TXI_IntI2cWrite(void);
void IIC_TXI_IntI2cRead(void);
void IIC_TEI_IntI2cWrite(void);
void IIC_TEI_IntI2cRead(void);
void IIC_GenClkSP(void);
void IIC_Error(RiicErrCode_e);

/**********************************************/
void I2c_RecvNotifyFromI2cDev2_1_IRQ8(void);
void I2c_RecvNotifyFromI2cDev2_2_IRQ8(void);
void I2c_RecvNotifyFromRp_IRQ5(void);
void I2c_RecvNotifyFromJp_IRQ5(void);
void I2c_SendEndNotifyToI2cDev2_1_IRQ8(void);
void I2c_SendEndNotifyToI2cDev2_2_IRQ8(void);
void I2c_SendEndNotifyToRp_IRQ5(void);
void I2c_SendEndNotifyToJp_IRQ5(void);

// Sc16RecvInSending_IRQ5_Level_L start
	unsigned char 	I2cSendDataBuff[256];
	unsigned short 	I2cSendDataCnt;
	unsigned char 	I2cSendSlaveAddr;
	unsigned char 	I2cSendLsiAddr;
// Sc16RecvInSending_IRQ5_Level_L end

#pragma section IntPRG
/****************************************************************************/
/*																			*/
/*	IRQ4																	*/
/*		VectorNo		68													*/
/*		Trigger			NegativeEdge/RTC���������M��(1min�Ԋu)				*/
/*		�M����			#RTC_IRQ											*/
/*		PinNo			43													*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_IRQ4 )
void	Excep_IRQ4( void )
{
	//�����i����
	//1.	CRW��g�p���Ɠ��l�ɖ{IC�g�p����1�����ɕ��i�̒ʒm��Main��֍s�Ȃ��B
	//2.	���vIC�͕������i�����/INT�����ݒ[�q�ɂ�Main��ɒʒm����B
	//		Main��͊����݂�Trigger�Ɏ��vIC�̎�����Control Register�̓Ǐo�����s�Ȃ��B
	//3.	�Ǐo����������CLK_REC(sys\memdata.c)�ɔ��f�����v�X�V��opeTask�ɒʒm����B
	//4.	���i���������̎�����IRQ1_SREQ_INTERRUPT�֐�(FT4800)��u��������`�ƂȂ�B
	//5.	���vData�Ǐo�����̎�����IICBus(Task)�̎�M���������Ŋ֐�Call�̌`�ōs�Ȃ��B
	//		�ڍ�Flow --->	���vIC����݌v����1-2-2���Q��
	//		IRQ1_SREQ_INTERRUPT --->	irq1_FT4800.c���Q��
	I2C_Event_Info.BIT.RTC_1_MIN_IRQ = 1;
}
/********************************************************************************************/
/*																							*/
/*	IRQ5																					*/
/*		VectorNo		69																	*/
/*		Trigger			NegativeEdge/UARTcontroler1������(RP & JP)							*/
/*		�M����			#EX_IRQ1															*/
/*		PinNo			42																	*/
/*		���L����:		RP��JP�̔r�������AppicationLayer�ōs���Ƃ̎�						*/
/*						FT4800�ł�RP, JP����CTS����, RTS�o�͐M����							*/
/*						Pin���݂���Ă��Ȃ��̂Ŗ��g�p�ł���									*/
/*																							*/
/*		����:	�{������Handler��Rp, Jp��Ώۂɂ���IC2���ݒ肵�������ݗv����				*/
/*				�����������ɓ��삵�܂�														*/
/*				�{������Handler�ł͈ȉ��̊����ݔ����v���ɑ΂��ď������s���܂�				*/
/*				1.	BIT_DATA_RECEIVED_LSR�������������ɑΏ�Device�̎�MData��				*/
/*					FIFO���Read���i�[/��͂��s��											*/
/*																							*/
/*				2.	BIT_THR_TSR_EMPTY_LSR��������������QueueingBuffer�̓��e�݂̂�			*/
/*					Drive���鏈���ł���Initialize���y�ы������M�ōs��ꂽ���M��				*/
/*					�ΏۂƂ��Ȃ��ז����Ƃ��鏈�����K�v�ł���								*/
/*																							*/
/*				3.	Error��� = I2C_Event_Info��Bit�o�^����									*/
/*								(�V�^�t���b�vNT-NET ErrorList_Rev110.xls)					*/
/*								IRQ5 -> I2C_Event_Analize()�֎󂯓n��						*/
/*								I2C_Event_Analize()����										*/
/*								err_chk(Error���, ErrorCode, ����/�폜/�o�^, 0, 0)��call	*/
/*	enum I2cResultFnc_e{																	*/
/*	RIIC_RSLT_OK = 0,			// ����I��													*/
/*	RIIC_RSLT_BUS_BUSY,			// I2C_Bus�オ���炩�̗��R�ɂ��Busy��Q���(���M�s��)		*/
/*	RIIC_RSLT_MODE_ERROR,		// ����I2C_Bus��ł�Wirte�܂���Read, ACK_Polling�����s��	*/
/*	RIIC_RSLT_PRM_ERROR,		// Application/ArbitrationM�����Parameter���s���ł�		*/
/*	RIIC_RSLT_OVER_DATA_ERROR,	// ApplicationM�����write/read��Data����Max�ȏ�			*/
/*	RIIC_RSLT_BUFF_OVER_ERROR,	// Write/Read���s���\��Buffer����t�ł�						*/
/*	RIIC_RSLT_PREV_TOUT,		// �O��X�e�[�^�X�ω��҂��^�C���A�E�g						*/
/*	RIIC_RSLT_MAX																			*/
/*	};																						*/
/*	RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR�ȊO = Error�o�^���s��	*/
/********************************************************************************************/
/*		Baudrate: 	400Kbps(2.5us/1bit)														*/
/*		write(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+D:8+A:1+P:1) =: 30bit(75us)					*/
/*		read(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+SlvAdr:7+R:1+A:1+D:8+A:1+P:1) =: 40bit(100us)	*/
/*																							*/
/********************************************************************************************/
//#pragma interrupt ( Excep_IRQ5 )
void	Excep_IRQ5( void )
{
	unsigned char 			ReadData[2];
	unsigned char 			WriteData[2];
	unsigned long			loopcnt;
	unsigned char			NextDeviceProcNo;
	unsigned long			ReadFifoCnt;
	unsigned char			LsrData_CH0;
	unsigned char			LsrData_CH1;
	int						ret;
	unsigned char			RHR_Read_Result;

	_ei();		// for I2cINT

	IicMilestone.I5cnt++;	// = DebugInfo
	di_I2cArbitration();
// Sc16RecvInSending_IRQ5_Level_L start
	WriteData[0] = 0x00;		// RHR_RCV <- DI
	ret = writeRegSC16(SC16_RP, SC16_IER_RW, 1, WriteData, BUSFREE_WAIT);
	ret = writeRegSC16(SC16_JP, SC16_IER_RW, 1, WriteData, BUSFREE_WAIT);
// Sc16RecvInSending_IRQ5_Level_L end
	/************************************************************************/
	/*	Initialize���ɂ�SC16_THR_W��Write���Ȃ��̂Ŕ������Ȃ��Ǝv���邪	*/
	/*	�{�����݂��������Ă��������鎖�B									*/
	/*	�܂�RTC�ւ�Write/Read���s���Ă��������܂���							*/
	/*	Initialize����ExeMode = EXE_MODE_FORCE��call����					*/
	/*	BUSFREE_WAIT�Ȃ̂�I2cAM.ActiveDevice == I2C_DN_NON�ł���			*/
	/*	* ActiveDevice��I2c_SendNewstart_Start_IRQ5_8()�֐��ł̂�set�����	*/
	/*																		*/
	/*	--->	�Ƃ肠�����v������Register��DummyRead����					*/
	/************************************************************************/
	if (I2cAM.I2c_ProcStatus == 0) {		// 0229
		// RP
		ret = readRegSC16(SC16_RP, SC16_LSR_R, 1, ReadData, BUSFREE_WAIT);
		LsrData_CH0 = ReadData[0];
		if (LsrData_CH0 & BIT_DATA_RECEIVED_LSR) {	// 1 = at least one character in RX FIFO
			ret = readRegSC16(SC16_RP, SC16_RXLVL_R, 1, ReadData, BUSFREE_WAIT);
			ReadFifoCnt = ReadData[0];
			for(loopcnt=0; loopcnt < ReadFifoCnt; loopcnt++){
				ret = readRegSC16(SC16_RP, SC16_RHR_R, 1, ReadData, BUSFREE_WAIT);
			}
		}
		// JP
		ret = readRegSC16(SC16_JP, SC16_LSR_R, 1, ReadData, BUSFREE_WAIT);
		LsrData_CH1 = ReadData[0];
		if (LsrData_CH1 & BIT_DATA_RECEIVED_LSR) {	// 1 = at least one character in RX FIFO
			ret = readRegSC16(SC16_JP, SC16_RXLVL_R, 1, ReadData, BUSFREE_WAIT);
			ReadFifoCnt = ReadData[0];
			for(loopcnt=0; loopcnt < ReadFifoCnt; loopcnt++){
				ret = readRegSC16(SC16_JP, SC16_RHR_R, 1, ReadData, BUSFREE_WAIT);
			}
		}
		return;
	}
	/********************************************************/
	/*	LSR, IIR��Check���s��								*/
	/********************************************************/
	/************************/
	/*	Rp��LSR��Read		*/
	/************************/
	ret = readRegSC16(SC16_RP, SC16_LSR_R, 1, ReadData, BUSFREE_WAIT);
	LsrData_CH0 = ReadData[0];
	ret = readRegSC16(SC16_JP, SC16_LSR_R, 1, ReadData, BUSFREE_WAIT);
	LsrData_CH1 = ReadData[0];
	
	if( LsrData_CH0 & (BIT_FRAMING_ERR_LSR|BIT_PARITY_ERR_LSR|BIT_OVERRUN_ERR_LSR)){
		RP_OV_PA_FR = LsrData_CH0;
	}
	/************************/
	/*	Jp��LSR��Check		*/
	/************************/
	if( LsrData_CH1 & (BIT_FRAMING_ERR_LSR|BIT_PARITY_ERR_LSR|BIT_OVERRUN_ERR_LSR)){
		JP_OV_PA_FR = LsrData_CH1;
	}
	
	/*	2.	CH0(Rp)��Recv���������݂�Check����									*/
	/*		Recv���� = LSR��BIT_DATA_RECEIVED_LSR�Ŕ��肷��						*/
	/*				   --->	�A��RXLVL��Read����RX_FIFO��Read���鎖				*/
	/*		Send���� = LSR��BIT_THR_TSR_EMPTY_LSR����BIT_THR_EMPTY_LSR�Ŕ��肷��*/
	/*				   --->														*/
	/****************************************************************************/
	if (LsrData_CH0 & BIT_DATA_RECEIVED_LSR) {	// 1 = at least one character in RX FIFO
		/********************************************/
		/*	������RX_FIFO��Data���z���グ�Ȃ���		*/
		/*	SC16��OverrunError���N�����̂�			*/
		/*	�S��Read���鎖 ---> ����1Byte			*/
		/********************************************/
		ret = readRegSC16(SC16_RP, SC16_RXLVL_R, 1, ReadData, BUSFREE_WAIT);
		ReadFifoCnt = ReadData[0];
		RHR_Read_Result = 0;
		for(loopcnt=0; loopcnt < ReadFifoCnt; loopcnt++){
			ret = readRegSC16(SC16_RP, SC16_RHR_R, 1, &I2cAM.DevBufRp[I2cAM.CtrRp.DevRecvP], BUSFREE_WAIT);
			if (ret != RIIC_RSLT_OK){
				RHR_Read_Result = 1;
				// �ǂݍ��ݎ��s����RX_FIFO���N���A���ăI�[�o�[�����ɂȂ�̂�h�~����B
				WriteData[0] = 0x02;
				ret = writeRegSC16(SC16_RP, SC16_FCR_W, 1, WriteData, BUSFREE_WAIT);
				if (ret == RIIC_RSLT_OK){			// �N���A�����������ꍇ�́A���[�v���痣�E
					break;
				}
				//else		FIFO�̃N���A�Ɏ��s�����ꍇ�͂Ƃ肠�������[�h���p������
			}
			IicMilestone.I5_RP_RHRbuf[IicMilestone.I5_RP_RHRcnt] = I2cAM.DevBufRp[I2cAM.CtrRp.DevRecvP];	// = DebugInfo
			++IicMilestone.I5_RP_RHRcnt;				// = DebugInfo
			if (IicMilestone.I5_RP_RHRcnt >= 16) {		// = DebugInfo
				IicMilestone.I5_RP_RHRcnt = 0;			// = DebugInfo
			}
			++I2cAM.CtrRp.DevRecvP;
		}
		if( !RHR_Read_Result ){				// Read���ɃG���[���������Ă��Ȃ�
			I2c_RecvNotifyFromRp_IRQ5();
		}
	}
	/****************************************************************************/
	/*	3.	CH0(Rp)��Send���������݂�Check����									*/
	/*		* Recv�����Ɠ����ɔ�������(�S2�d)�ꍇ������							*/
	/****************************************************************************/
// Sc16RecvInSending_IRQ5_Level_L start
#if 1
	if ((I2cAM.ActiveDevice == I2C_DN_RP) && (I2cAM.CtrRp.Status == DEVICE_STS_SENDING)) {
		if (LsrData_CH0 & BIT_THR_EMPTY_LSR) {		// 0/1 = THR not empty/empty(Pon=1)
			IicMilestone.I5_RP_THRcnt++;	// = DebugInfo
			WriteData[0] = 0x01;		// RHR_RCV <- EI
			ret = writeRegSC16(SC16_RP, SC16_IER_RW, 1, WriteData, BUSFREE_WAIT);
			/****************************************************************************/
			/*	�{�v�������������Ƃ�������I2C_Request()���͖{Handler����				*/
			/*	Rp��writeRegSC16/readRegSC16���s�����ׂł���B							*/
			/*		I2cAM.ActiveDevice == �{Device�̂͂�								*/
			/*		�{Device��I2cAM.Ctrxxx.Status == DEVICE_STS_SENDING�̂͂�			*/
			/*																			*/
			/*	�����I2c_Arbitration_Manager�ŊǗ����Ă��鑗�MDataResource�ɂ��		*/
			/*	����Device�̑��M�p��Check, 1Data���M�����ɂ�鎟Data�̑��M����			*/
			/*	(���M�\�񂳂ꂽ����Device���͑���Device�̍ĊJ/�V���M)					*/
			/*	���s���K�v�����邪														*/
			/*	���M����Device���PByte����RoundRobin������ׂ�							*/
			/*	��Device�̍ĊJ/�V���M��Check���s��,����Device�̑��M�\�񂪖������		*/
			/*	�{Device�̑��M�p��Check, 1Data���M�����ɂ��SDevice���M�����������s��	*/
			/*	* �A��RTC�͑��������̈�,Check�Ώۂ�Device�ɂ͊܂݂܂���					*/
			/*	------->	Device�̑��M�J�nCheck���s���O�ɖ{Device�̂PData�̏I����		*/
			/*				Check���ďI���Ȃ�ȉ��̏������s���B							*/
			/*				1.	I2cAM�̏�����											*/
			/*				2.	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)	*/
			/****************************************************************************/
			/****************************************************************************/
			/*	�@	Device�̑��M�J�nCheck���s���O�ɖ{Device�̂PData�̏I����				*/
			/*		Check���ďI���Ȃ�ȉ��̏������s���B									*/
			/*		�����ł͂PData�̏I��Check�����݂̂��s��Data�̑��M�͍s��Ȃ�			*/
			/*		1.	I2cAM�̏�����													*/
			/*			���YAplRegistedNum--;											*/
			/*			���YStatus = DEVICE_STS_IDLE;									*/
			/*			ActiveDevice = I2C_DN_NON;										*/
			/*		2.	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)			*/
			/****************************************************************************/
			I2cAM.CtrRp.AplActiveRWCnt++;
			if (I2cAM.CtrRp.AplActiveRWCnt >= 
				I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
				//	1Data�̏I������
				I2cAM.CtrRp.AplRegistedNum--;
				I2cAM.ActiveDevice = I2C_DN_NON;
				//	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)
				I2c_SendEndNotifyToRp_IRQ5();
			}
			I2cAM.CtrRp.Status = DEVICE_STS_IDLE;
			NextDeviceProcNo = I2cNextDeviceReNewStartCheck(SC16_RP);
			/************************/
			/*	�������f�v���L��	*/
			/************************/
			if (I2cAM.ForceRequest == 0xff) {
				I2cAM.ForceRequest = NextDeviceProcNo;
			} else {
				if (NextDeviceProcNo != 0xfe) {
					I2cNextDeviceProc(NextDeviceProcNo, I2C_INT_MODE_DI);
				}
				/********************************************/
				/*	ActiveDevice == I2C_DN_NON�ƂȂ��Ă���	*/
				/********************************************/
			}
		}
	}
#endif
// Sc16RecvInSending_IRQ5_Level_L end
/***********************************************************************************************************************/
	/****************************************************************************/
	/*	Rp��Jp��APLLayer�ŋ������Ȃ��悤�ɂ���Ƃ̎��Ȃ̂�						*/
	/*	Jp�̏��������̂܂ܖ{Handler���ōs���Ă�I2cBus��ɑ��M��					*/
	/*	�d�Ȃ鎖�͂Ȃ��̂�Jp�̏��������̂܂܋L�q����							*/
	/*	�A��Device����̔񓯊��̎�M�v���͏d�Ȃ鎖�͂���܂�					*/
	/****************************************************************************/
	if (LsrData_CH1 & BIT_DATA_RECEIVED_LSR) {	// 1 = at least one character in RX FIFO
		ret = readRegSC16(SC16_JP, SC16_RXLVL_R, 1, ReadData, BUSFREE_WAIT);
		ReadFifoCnt = ReadData[0];
		RHR_Read_Result = 0;
		for(loopcnt=0; loopcnt < ReadFifoCnt; loopcnt++){
			ret = readRegSC16(SC16_JP, SC16_RHR_R, 1, &I2cAM.DevBufJp[I2cAM.CtrJp.DevRecvP], BUSFREE_WAIT);
			if (ret != RIIC_RSLT_OK){
				// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR�ȊO = Error�o�^���s�� ---> ����Layer�ōs��
				RHR_Read_Result = 1;
				// �ǂݍ��ݎ��s����RX_FIFO���N���A���ăI�[�o�[�����ɂȂ�̂�h�~����B
				WriteData[0] = 0x02;
				ret = writeRegSC16(SC16_JP, SC16_FCR_W, 1, WriteData, BUSFREE_WAIT);
				if (ret == RIIC_RSLT_OK){			// �N���A�����������ꍇ�́A���[�v���痣�E
					break;
				}
				//else		FIFO�̃N���A�Ɏ��s�����ꍇ�͂Ƃ肠�������[�h���p������
			}
			IicMilestone.I5_JP_RHRbuf[IicMilestone.I5_JP_RHRcnt] = I2cAM.DevBufJp[I2cAM.CtrJp.DevRecvP];	// = DebugInfo
			++IicMilestone.I5_JP_RHRcnt;			// = DebugInfo
			if (IicMilestone.I5_JP_RHRcnt >= 16) {	// = DebugInfo
				IicMilestone.I5_JP_RHRcnt = 0;		// = DebugInfo
			}
			++I2cAM.CtrJp.DevRecvP;
		}
		if( !RHR_Read_Result ){				// Read���ɃG���[���������Ă��Ȃ�
			I2c_RecvNotifyFromJp_IRQ5();
		}
	}
// Sc16RecvInSending_IRQ5_Level_L start
#if 1
	if ((I2cAM.ActiveDevice == I2C_DN_JP) && (I2cAM.CtrJp.Status == DEVICE_STS_SENDING)) {
		if (LsrData_CH1 & BIT_THR_EMPTY_LSR) {		// 0/1 = THR not empty/empty(Pon=1)
			IicMilestone.I5_JP_THRcnt++;			// = DebugInfo
			WriteData[0] = 0x01;		// RHR_RCV <- EI
			ret = writeRegSC16(SC16_JP, SC16_IER_RW, 1, WriteData, BUSFREE_WAIT);
			I2cAM.CtrJp.AplActiveRWCnt++;
			if (I2cAM.CtrJp.AplActiveRWCnt >= 
				I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
				//	1Data�̏I������
				I2cAM.CtrJp.AplRegistedNum--;
				I2cAM.ActiveDevice = I2C_DN_NON;
				//	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)
				I2c_SendEndNotifyToJp_IRQ5();
			}
			I2cAM.CtrJp.Status = DEVICE_STS_IDLE;
			NextDeviceProcNo = I2cNextDeviceReNewStartCheck(SC16_JP);
			/************************/
			/*	�������f�v���L��	*/
			/************************/
			if (I2cAM.ForceRequest == 0xff) {
				I2cAM.ForceRequest = NextDeviceProcNo;
			} else {
				if (NextDeviceProcNo != 0xfe) {
					I2cNextDeviceProc(NextDeviceProcNo, I2C_INT_MODE_DI);
				}
				/********************************************/
				/*	ActiveDevice == I2C_DN_NON�ƂȂ��Ă���	*/
				/********************************************/
			}
		}
	}
#endif
// Sc16RecvInSending_IRQ5_Level_L end
	ei_I2cArbitration();
// Sc16RecvInSending_IRQ5_Level_L start
	WriteData[0] = 0x01;		// RHR_RCV <- EI
	ret = writeRegSC16(SC16_RP, SC16_IER_RW, 1, WriteData, BUSFREE_WAIT);
	ret = writeRegSC16(SC16_JP, SC16_IER_RW, 1, WriteData, BUSFREE_WAIT);
// Sc16RecvInSending_IRQ5_Level_L end
#ifdef	CPU_DEBUG
	DBG_irq5_cnt++;
#else
#endif
}
/************************************************/
/*												*/
/************************************************/
// Sc16RecvInSending_IRQ5_Level_L
void Sc16SendInI2C_RP(void)
{
	unsigned char			NextDeviceProcNo;

	if ((I2cAM.ActiveDevice == I2C_DN_RP) && (I2cAM.CtrRp.Status == DEVICE_STS_SENDING)) {
//		if (LsrData_CH0 & BIT_THR_EMPTY_LSR) {		// 0/1 = THR not empty/empty(Pon=1)
//			IicMilestone.I5_RP_THRcnt++;	// = DebugInfo
//			WriteData[0] = 0x01;		// RHR_RCV <- EI
//			ret = writeRegSC16(SC16_RP, SC16_IER_RW, 1, WriteData, BUSFREE_WAIT);
			I2cAM.CtrRp.AplActiveRWCnt++;
			if (I2cAM.CtrRp.AplActiveRWCnt >= 
				I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
				//	1Data�̏I������
				I2cAM.CtrRp.AplRegistedNum--;
				I2cAM.ActiveDevice = I2C_DN_NON;
				//	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)
				I2c_SendEndNotifyToRp_IRQ5();
			}
			I2cAM.CtrRp.Status = DEVICE_STS_IDLE;
			NextDeviceProcNo = I2cNextDeviceReNewStartCheck(SC16_RP);
			/************************/
			/*	�������f�v���L��	*/
			/************************/
			if (I2cAM.ForceRequest == 0xff) {
				I2cAM.ForceRequest = NextDeviceProcNo;
			} else {
				if (NextDeviceProcNo != 0xfe) {
					I2cNextDeviceProc(NextDeviceProcNo, I2C_INT_MODE_DI);
				}
				/********************************************/
				/*	ActiveDevice == I2C_DN_NON�ƂȂ��Ă���	*/
				/********************************************/
			}
//		}
	}
}
/************************************************/
/*												*/
/************************************************/
// Sc16RecvInSending_IRQ5_Level_L
void Sc16SendInI2C_JP(void)
{
	unsigned char			NextDeviceProcNo;

	if ((I2cAM.ActiveDevice == I2C_DN_JP) && (I2cAM.CtrJp.Status == DEVICE_STS_SENDING)) {
//		if (LsrData_CH1 & BIT_THR_EMPTY_LSR) {		// 0/1 = THR not empty/empty(Pon=1)
//			IicMilestone.I5_JP_THRcnt++;			// = DebugInfo
//			WriteData[0] = 0x01;		// RHR_RCV <- EI
//			ret = writeRegSC16(SC16_JP, SC16_IER_RW, 1, WriteData, BUSFREE_WAIT);
			I2cAM.CtrJp.AplActiveRWCnt++;
			if (I2cAM.CtrJp.AplActiveRWCnt >= 
				I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
				//	1Data�̏I������
				I2cAM.CtrJp.AplRegistedNum--;
				I2cAM.CtrJp.Status = DEVICE_STS_IDLE;
				I2cAM.ActiveDevice = I2C_DN_NON;
				//	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)
				I2c_SendEndNotifyToJp_IRQ5();
			}
			NextDeviceProcNo = I2cNextDeviceReNewStartCheck(SC16_JP);
			/************************/
			/*	�������f�v���L��	*/
			/************************/
			if (I2cAM.ForceRequest == 0xff) {
				I2cAM.ForceRequest = NextDeviceProcNo;
			} else {
				if (NextDeviceProcNo != 0xfe) {
					I2cNextDeviceProc(NextDeviceProcNo, I2C_INT_MODE_DI);
				}
				/********************************************/
				/*	ActiveDevice == I2C_DN_NON�ƂȂ��Ă���	*/
				/********************************************/
			}
//		}
	}
}
/********************************************************************************************/
/*																							*/
/*	IRQ8																					*/
/*		VectorNo		72																	*/
/*		Trigger			NegativeEdge/UARTcontroler2������									*/
/*		�M����			#EX_IRQ2															*/
/*		PinNo			141																	*/
/*																							*/
/********************************************************************************************/
/*		Baudrate: 	400Kbps(2.5us/1bit)														*/
/*		write(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+D:8+A:1+P:1) =: 30bit(75us)					*/
/*		read(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+SlvAdr:7+R:1+A:1+D:8+A:1+P:1) =: 40bit(100us)	*/
/*																							*/
/********************************************************************************************/
//#pragma interrupt ( Excep_IRQ8 )
void	Excep_IRQ8( void )
{
}
/****************************************************************************/
/*																			*/
/*	IRQ15																	*/
/*		VectorNo		79													*/
/*		Trigger			NegativeEdge/Ethernetcontroler������				*/
/*		�M����			#ETHER_IRQ											*/
/*		PinNo			98													*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_IRQ15 )
void	Excep_IRQ15( void )
{
	USHT				wEIE, wEIR, wECON1, wESTAT;
	USHT				wDummy;
	USHT				SendRecvFlag, DummyReadReq;
	int					numberRecvPackets;
	int					numberSendCompletePackets;
	ttUser32Bit			numberBytesSent;
	ttUser32Bit			flag;
	ETHER_TOP_8BYTE		EncPreamble;
	USHT 				wNewRXTail;		// REG_ERXTAIL�ɏ�����TailPointer
	USHT				ByteCount;
	USHT				NextRxPtr;
	USHT				FirstRecvPacket;
	USHT				PreambleDataERR;
	USHT				PacketCnt1Irq;		// Multi_RecvPackets

	wDummy = 0;
	/********************************/
	/*	�����ݔ���Flag�̎擾		*/
	/********************************/
	wESTAT 	= ReadReg(REG_ESTAT);	// INT(b15), RXBUSY(b13), PHYLNK(b8), PKTCNT7_0(b7-b0)
	wEIE 	= ReadReg(REG_EIE);		// 0x8058�̂͂�(INTIE(b15), PKTIE(b6), TXIE(b3))
	wEIR 	= ReadReg(REG_EIR);		// PKTIF(b6), TXIF(b3), TXABTIF(b2), RXABTIF(b1), PCFULIF(b0)
	wECON1 	= ReadReg(REG_ECON1);	// PKTDEC(b8), TXRTS(b1), RXEN(b0)
//	Ether_DTinit()��opetask-I2C1_init()�O��_ei()�̑O����call�����̂�
//	�����l�͕K��Initialize����Ă���͂�
	if (SendRecvInfo.k_OpenCnt == 0) {
		SendRecvInfo.I15_EIR_BeforeOpen[SendRecvInfo.I15cnt_BeforeOpen & 0x0f] = wEIR;
		SendRecvInfo.I15cnt_BeforeOpen++;
	} else {
		SendRecvInfo.I15_EIR[SendRecvInfo.I15cnt & (SIZE_I15_EIR-1)] = wEIR;
		SendRecvInfo.I15cnt++;
	}
	SendRecvFlag = 0;
	DummyReadReq = 0;
	PreambleDataERR = 0;
	/********************************************************************/
	/*	k_OpenCnt == 0�̎��͂܂�Open�v������Ă��Ȃ����ɓ����Ă���		*/
	/*	�����݂Ȃ̂Ŏ�MData�֘A��Pointe,�l�������������̂Ŗ����Ƃ���	*/
	/********************************************************************/

	/****************************************************************************/
	/*	IRQ15��Line��L�̂܂܂�PowerOn�����H�ɂȂ�܂ŉi�v��IRQ15����������		*/
	/*	System�������オ��Ȃ��̂�kasago����Open�����O�ɔ�������IRQ15��		*/
	/*	LSI����SPacket��Read����IRQ15��Line��H�ɂ���B							*/
	/*	* FTP�ɂ��PDL�͓�����M��Max5Packets��IRQ15���̏������Ԃ�Max3.6ms�ł�	*/
	/****************************************************************************/
	if ( SendRecvInfo.k_OpenCnt == 0 ){
		DummyReadReq |= 0x0008;
	}
	/********************************************************************************************************************/
	/*	EIR_CRYPTEN(b15)	1 = All cryptographic engine modules are enabled 											*/
	/*	EIR_MODEXIF(b14)	1 = Modular exponentiation calculation complete 											*/
	/*	EIR_HASHIF(b13)		1 = MD5/SHA-1 hash operation complete 														*/
	/*	EIR_AESIF(b12)		1 = AES encrypt/decrypt operation complete 													*/
	/*	EIR_LINKIF(b11)		1 = PHY Ethernet link status has changed.													*/
	/*							Read PHYLNK(ESTAT<8>) to determine the current state. 									*/
	/*	EIR_PRDYIF(b10)																									*/
	/*	EIR_PKTIF(b6)		1 = One or more RX packets have been saved and are ready for software processing.			*/
	/*							The PKTCNT<7:0> (ESTAT<7:0>) bits are non-zero. To clear this flag,						*/
	/*							decrement the PKTCNT bits to zero by setting PKTDEC (ECON1<8>). 						*/
	/*	EIR_DMAIF(b5)		1 = DMA copy or checksum operation complete 												*/
	/*	EIR_TXIF(b3)		1 = Packet transmission has completed. TXRTS (ECON1<1>) has been cleared by hardware. 		*/
	/*	EIR_TXABTIF(b2)		1 = Packet transmission has been aborted due to an error. 									*/
	/*							Read the ETXSTAT register to determine the cause.										*/
	/*							TXRTS (ECON1<1>) has been cleared by hardware. 											*/
	/*	EIR_RXABTIF(b1)		1 = An RX packet was dropped because there is insufficient space							*/
	/*							in the RX buffer to store the complete packet or the PKTCNT field is saturated at FFh 	*/
	/*	EIR_PCFULIF(b0)		1 = Packet Counter Full Interrupt Flag bit(RW:0)											*/
	/********************************************************************************************************************/
	/********************************************/
	/*	tfNotifyInterfaceIsr()��Parameter������	*/
	/********************************************/
	numberRecvPackets = 0;
	numberSendCompletePackets = 0;
	numberBytesSent = 0;
	flag = 0;
	_ei();				//	<---	_ei()���Ȃ���Uart�n(IPR==0x07�ȏ�)��OverrunError����������
	/************************************************************************************/
	/*	����MErrorCheck																*/
	/************************************************************************************/
	if(wEIR & EIR_TXABTIF) {	// 1 = Packet transmission has been aborted due to an error.
								// 		Read the ETXSTAT register to determine the cause.
								// 		TXRTS (ECON1<1>) has been cleared by hardware.
		SendRecvInfo.ERR_EIR_TXABTIF++;
		return;
	}
	if(wEIR & EIR_RXABTIF) {	// 1 = An RX packet was dropped because there is insufficient space
								// 		in the RX buffer to store the complete packet
								// 		or the PKTCNT field is saturated at FFh
		SendRecvInfo.ERR_EIR_RXABTIF++;
		DummyReadReq |= 0x0001;
	}
	if(wEIR & EIR_PCFULIF) {	// 1 = PKTCNT field has reached FFh.
								// Software must decrement the packet counter
								// to prevent the next RX packet from being dropped. 
		/****************************************/
		/* PKTCNT��0�ɂȂ�܂�Dummy Read����	*/
		/****************************************/
		SendRecvInfo.ERR_EIR_PCFULIF++;
		DummyReadReq |= 0x0002;
	}
	/************************************************************************************/
	/*	PHY Ethernet link status Check													*/
	/************************************************************************************/
	if(wEIR & EIR_LINKIF) {		// 1 = PHY Ethernet link status has changed
		BitClearReg(REG_EIR, EIR_LINKIF);		// LINKIF <- 0
		if(wESTAT & ESTAT_PHYLNK) {
			if (SendRecvInfo.PhylinkStatus == 0) {
				/****************************/
				/*	PHYLNK: 0 -> 1 Occer	*/
				/****************************/
				SendRecvInfo.PhylinkStatus = 1;
				// opetask�֒ʒm����(queueset)
			} else {
				// 1 -> 1 = ���̕ω����Ă��Ȃ��̂�EIR_LINKIF������̂�?
				wDummy = 1;
			}
		} else {
			if (SendRecvInfo.PhylinkStatus == 1) {
				/****************************/
				/*	PHYLNK: 1 -> 0 Occer	*/
				/****************************/
				SendRecvInfo.PhylinkStatus = 0;
				// opetask�֒ʒm����(queueset)
			} else {
				// 0 -> 0 = ���̕ω����Ă��Ȃ��̂�EIR_LINKIF������̂�?
				wDummy = 2;
			}
		}
	}
	/************************************************************************************/
	/*	���MPacket����Check																*/
	/*	The transmit complete interrupt occurs											*/
	/*	when the transmission of a frame has ended (whether or not it was successful).	*/
	/*	This flag is set when TXRTS(REG_ECON1<1>) is cleared.							*/
	/*	The interrupt should be cleared by software once it has been serviced. 			*/
	/*	To enable the transmit complete interrupt, set TXIE(b3)			 				*/
	/************************************************************************************/
	if(wEIR & EIR_TXIF) {
		BitClearReg(REG_EIR, EIR_TXIF);		// TXIF <- 0
		SendRecvInfo.SendPktInt++;
		/************************/
		/*	���MPacket��������	*/
		/************************/
		numberSendCompletePackets 	= 1;
		numberBytesSent = SendRecvInfo.SendByteCnt;	// ���񑗐M����Byte��
		SendRecvFlag = 1;
	}
	/************************************************************************************/
	/*	��MPacket��MCheck																*/
	/*	The received packet pending interrupt occurs									*/
	/*	when one or more frames have been received & are ready for software processing	*/
	/*	This flag is set when the PKTCNT bits are non-zero.								*/
	/*	This interrupt flag is read-only and will automatically clear 					*/
	/*	when the PKTCNT bits are decremented to zero.									*/
	/************************************************************************************/
	if(wEIR & EIR_PKTIF) {					// PKTIF��PKTCNT==0�ɂȂ�����Clear�����
		SendRecvInfo.RecvPktInt++;
		/****************************************************************/
		/*	��MPacket�擾����											*/
		/*		PKTCNT = ESTAT(b0-b7)	ESTAT_PKTCNT					*/
		/*		PKTDEC = ECON1(b8)		ECON1_PKTDEC					*/
		/*	��M����Byte��(����Preamble)��Preamble����ByteCount�ł킩��	*/
		/****************************************************************/
		/************************************************/
		/*	��M�̏ꍇ��1Irq��Max= 8Packets�܂Ŏ�M��	*/
		/*	Todo ---> ��M���͌����������K�v����		*/
		/************************************************/
		PacketCnt1Irq = (wESTAT & 0xff);
		/************************************/
		/*	1_Irq�̎�MPacket��Max����Save	*/
		/************************************/
		SendRecvInfo.RecvPktCnt += PacketCnt1Irq;
		if (DummyReadReq == 0) {
			if (SendRecvInfo.RecvPktMax < PacketCnt1Irq) {
				SendRecvInfo.RecvPktMax = PacketCnt1Irq;
			}
		}
		if (PacketCnt1Irq != 0) {
			FirstRecvPacket = 0;
			while (1) {
				/************************************************************************************************************/
				/*	ESTAT_INT(b15)			1 = One of the EIR bits is set and enabled by the EIE register.					*/
				/*								If INTIE (EIE<15>) is set, the INT pin is also driven low. 					*/
				/*	ESTAT_FCIDLE(b14)		1 = Internal flow control state machine is Idle.								*/
				/*								It is safe to change the FCOP (ECON1<7:6>) and FULDPX (MACON2<0>) bits. 	*/
				/*	ESTAT_RXBUSY(b13)		1 = Receive logic is currently receiving a packet.								*/
				/*								This packet may be discarded in the future									*/
				/*								if an RX buffer overflow occurs or a receive filter rejects it,				*/
				/*								so this bit does not necessarily indicate									*/
				/*								that an RX packet pending interrupt will occur. 							*/
				/*	ESTAT_CLKRDY(b12)		1 = Normal operation 															*/
				/*	ESTAT_RSTDONE(b11)																						*/
				/*	ESTAT_PHYDPX(b10)		1 = PHY is operating in Full-Duplex mode 										*/
				/*	ESTAT_PHYRDY(b9)		(R:0)																			*/
				/*	ESTAT_PHYLNK(b8)		1 = Ethernet link has been established with a remote Ethernet partner			*/
				/*	ESTAT_PKTCNT7_0(b7-b0)	Receive Packet Count bits<7>: (R:0)												*/
				/************************************************************************************************************/
				/********************************************************************************************/
				/*	Test if at least one packet has been received and is waiting							*/
				/*	EIR_PKTIF																				*/
				/*		1 = One or more RX packets have been saved and are ready for software processing.	*/
				/*			The PKTCNT<7:0> (ESTAT<7:0>) bits are non-zero. To clear this flag,				*/
				/*			decrement the PKTCNT bits to zero by setting PKTDEC (ECON1<8>). 				*/
				/********************************************************************************************/
				// MACDiscardRx()����
					/********************************************************************************************************/
					/*	Decrement the next packet pointer before writing it into the REG_ERXRDPT registers.					*/
					/*	RX buffer wrapping must be taken into account if the NextPacketLocation is precisely ADDR_RXSTART.	*/
					/********************************************************************************************************/
					wNewRXTail = wNextRxPacketPointer - 2;
					if(wNextRxPacketPointer == ADDR_RXSTART)
						wNewRXTail = SIZE_SRAM_MAX - 2;
					/****************************************************/
					/*	PKTDEC <- 1(start of decrement ESTAT_PKTCNT)	*/
					/*	������ <- 1����PKTCNT == 0�ɂȂ��				*/
					/*	PKTDEC�͎����I�� <- 0�����						*/
					/*	�܂�PKTIF�������I�� <- 0�����					*/
					/****************************************************/
					BitSetReg(REG_ECON1, ECON1_PKTDEC);			// ECON1_PKTDEC <- 1
					/********************************************************************************************/
					/*	Move the receive read pointer to unwrite-protect the memory used by the last packet.	*/
					/*	The writing order is important: set the low byte first, 								*/
					/*	high byte last (handled automatically in WriteReg()).									*/
					/********************************************************************************************/
					WriteReg(REG_ERXTAIL, wNewRXTail);	//	Ethernet_RX_Tail_Pointer(set/clr)(Reset = 0x5FFE)
				/********************************************************/
				/*	Set the RX Read Pointer								*/
				/*	to the beginning of the next unprocessed packet		*/
				/********************************************************/
				wCurrentRxPacketPointer = wNextRxPacketPointer;	// wCurrentRxPacketPointer <--- �O��{�֐���REG_ERXDATA�ɂ��Read������MData��NextPacketPointer
				WriteReg(REG_ERXRDPT, wCurrentRxPacketPointer);	// REG_ERXRDPT <--- wCurrentRxPacketPointer
				/********************************************************/
				/*	Obtain the MAC header <--- from the Ethernet buffer	*/
				/*	��L�Őݒ肵��REG_ERXRDPT����MData��Read����		*/
				/*		USHT			NextPacketPointer;				*/
				/*		RXSTATUS		StatusVector;					*/
				/*	Preamble��: 8Byte��Read�����̂�ByteCount�̒l��		*/
				/*	Preamble������1Packet��Byte���Ǝv����				*/
				/********************************************************/
				ReadMemWindow(	RX_WINDOW,				// REG_ERXDATA(8bit)
								(UCHAR*)&EncPreamble,	// ETHER_TOP_8BYTE�^�̊i�[��
								sizeof(EncPreamble),	// Byte��
								1);						// 
				ByteCount 	= EncPreamble.StatusVector.bits.ByteCount;
				NextRxPtr 	= EncPreamble.NextPacketPointer;
				/****************************************************************************/
				/*	The EtherType field,													*/
				/*	like most items transmitted on the Ethernet medium are in big endian.	*/
				/****************************************************************************/
				//	��M����StatusVector, NextPacketPointer��Error_Check���s��
				if (NextRxPtr > ADDR_RXSTOP) {
					SendRecvInfo.ERR_NextRxPtr_MAX++;
					/****************************************/
					/* PKTCNT��0�ɂȂ�܂�Dummy Read����	*/
					/****************************************/
					NextRxPtr = ADDR_RXSTART;
					PreambleDataERR = 1;
				}
				if (ByteCount > (1518)) {	// 6 + 6 + 2 + 1500 + 4(Preamble��������Byte��)
					SendRecvInfo.ERR_ByteCount_MAX++;
					/****************************************/
					/* PKTCNT��0�ɂȂ�܂�Dummy Read����	*/
					/****************************************/
					ByteCount = 1518;
					PreambleDataERR = 2;
				}
				/********************************************************************/
				/*	����ݒ肵��REG_ERXRDPT���Read������MData��NextPacketPointer	*/
				/*	����{�֐���ReadMemWindow()�Ŏ�M����ׂ�PacketPointer -> save	*/
				/********************************************************************/
				wNextRxPacketPointer = NextRxPtr;
				/********************************************************************/
				/*	ByteCount(Preamble������1Packet��Byte��)��Read����				*/
				/********************************************************************/
				EtheRecvInfo.PktCnt++;
				EtheRecvInfo.ByteCnt[(EtheRecvInfo.PktCnt-1)&ETHER_RECV_MASK]	= ByteCount;
				ReadMemWindow(	RX_WINDOW,
								(UCHAR*)&EtheRecvInfo.Buff[(EtheRecvInfo.PktCnt-1)&ETHER_RECV_MASK][0],
								ByteCount,
								0);
				if (PreambleDataERR == 0) {
					numberRecvPackets++;
					if ( FirstRecvPacket == 0) {
						FirstRecvPacket++;
					}
					SendRecvFlag = 2;
				}
				/****************************/
				/*	��MPacket�I��Check		*/
				/****************************/
				PacketCnt1Irq--;
				if (PacketCnt1Irq == 0) {
					break;		// escape while
				}
			}	// End of While
		}		// End of if (PacketCnt1Irq != 0)
	}			// End of if(wEIR & EIR_PKTIF)
	/********************************************************/
	/*	���LkasagoAPI��1_Interrupt��1�񂵂�call�ł��Ȃ�		*/
	/********************************************************/
	if (SendRecvFlag != 0) {
		if (DummyReadReq == 0) {
			/************************/
			/*	call kasago_API		*/
			/************************/
			tfNotifyInterfaceIsr(	OpenHandle,	// EtherOpen()��set����ttUserInterface�^Handle
									numberRecvPackets,
									numberSendCompletePackets,
									numberBytesSent,
									flag);
			SendRecvInfo.kasagoAPI_CallCnt++;
		}
	}
	if (wEIE != 0)			wDummy = 1;
	if (wESTAT != 0)		wDummy = 2;
	if (wECON1 != 0)		wDummy = 6;
	if (wDummy != 0)		wDummy = 9;

	wait2us( 300L );		//	wait until IRQ15_line == H for 10M
}
///****************************************************************************/
///*																			*/
///*	TPU0																	*/
///*		VectorNo		TGI0A = 126						 					*/
///*		ComName			20msTimer������										*/
///*		PCLK 			50MHz(20nsec/1clock)								*/
///*																			*/
///****************************************************************************/
//#pragma interrupt ( Excep_TPU0_TGI0A )
void	Excep_TPU0_TGI0A( void )
{
}

/****************************************************************************/
/*																			*/
/*	TPU1																	*/
/*		VectorNo		TGI1A = 130				 							*/
/*		ComName			2msTimer������										*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_TPU1_TGI1A )
void	Excep_TPU1_TGI1A( void )
{
	volatile unsigned char	wkb;
	volatile unsigned long	ist;			// ���݂̊�����t���

	TPU1.TIER.BIT.TGIEA = 0;				// TGIA������ <- DI
    _ei();

	/*** 10ms interval process ***/
	++Tim10msCount;							// 10ms�ώZ����UP
	++LifeTimer2ms;							// 2ms�ώZ����UP(Use IFcom)
	if( 5 <= Tim10msCount ){				// 10ms�o�� (Y)
		SCAN_INP_Exec();					// �g��I/O�|�[�g�X�L���������i�m�C�Y�΍�̂���10ms 3��Ƃ���j
		LagChk10ms();						// �֐�Call�^ Lag timer timeout check
		LCM_10msInt();
		toS_10msInt();
		++LifeTimer10ms;					// 10ms�ώZ��ϰ count up
		Tim10msCount = 0;
	}
	if( 0 != (SYSMNTcomdr_c2c_timer & 0x7fff) ){
		--SYSMNTcomdr_c2c_timer;
	}
	if( 0 != (FROM2_timer & 0x7fff) ){
		--FROM2_timer;
	}
	toS_2msInt();
	if( 0 != (FBcom_2msT1 & 0x7fff) ){
		--FBcom_2msT1;
	}
	if(Tim10msCount == 0){
		if(KSG_gkasagoStarted){
			tfTimerUpdateIsr();
		}
	}
	if( 0 != (Capcom_2msT1 & 0x7fff) ){
		--Capcom_2msT1;
	}
	ist = _di2();
	wkb = TPU1.TSR.BYTE;
	TPU1.TSR.BIT.TGFA = 0;					// TGFA <- 0
	dummy_Read = TPU1.TSR.BYTE;				// Dummy Read
	TPU1.TIER.BIT.TGIEA = 1;				// TGIA������ <- EI
	_ei2( ist );
}
/****************************************************************************/
/*																			*/
/*	TPU2																	*/
/*		VectorNo		TGI2A = 132											*/
/*		ComName			1msTimer������										*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_TPU2_TGI2A )

void	Excep_TPU2_TGI2A( void )
{
	volatile unsigned char	wkb;
	volatile unsigned long	ist;			// ���݂̊�����t���

	/********************************/
	/*	Debug_Routine				*/
	/********************************/
	//	�󂫂̔ėp�o��Port�𗘗p����FlipFlop�M�����o�͂���
	//	1ms��Pulse�o�͂�g�`�ɂĊm�F����

	TPU2.TIER.BIT.TGIEA = 0;				// TGIA������ <- DI
    _ei();
	
	CLK_REC_msec ++;					// �~���b�J�E���gup
	if( CLK_REC_msec > (1000 - 1) ){
		CLK_REC_msec = 0;
		if (CLK_REC.seco < 59) {
			CLK_REC.seco++;
		}else{
			CLK_REC_msec = 999;
		}
	}
	if (SysTimeAdj.state >= _T_BUSY) {
		// msec�����^�C�}
		if (SysTimeAdj.lag_count > 0) {
			SysTimeAdj.lag_count--;
			if (SysTimeAdj.lag_count == 0) {
				queset(OPETCBNO, SNTPMSG_AUTO_SET, 0, NULL);
			}
		}
	}

	/********************************/
	/*	1ms_Handler����Apri����		*/
	/********************************/
	KSG_RauIntervalTimer();
	++RAU_x_1mLifeTime;						/* timer counter. up to every 1ms */
	LagChk_1ms();
	ist = _di2();
	if( 0 != (Btcom_1msT1 & 0x7fff) ){
		--Btcom_1msT1;
	}
	wkb = TPU2.TSR.BYTE;
	TPU2.TSR.BIT.TGFA = 0;					// TGFA <- 0
	dummy_Read = TPU2.TSR.BYTE;				// Dummy Read
	TPU2.TIER.BIT.TGIEA = 1;				// TGIA������ <- EI
	_ei2( ist );
}

/****************************************************************************/
/*																			*/
/*	TPU3																	*/
/*		VectorNo		TGI3A = 132						 					*/
/*		ComName			20msTimer������										*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_TPU3_TGI03 )
void	Excep_TPU3_TGI3A( void )
{
	volatile unsigned char	wkb;
	volatile unsigned long	ist;			// ���݂̊�����t���
	unsigned long	i;

	TPU3.TIER.BIT.TGIEA = 0;				// TGIA������ <- DI
    _ei();
	/*** timer control ***/
	LedOnOffCtrl();							// LED�_�Ő���
	LagChk20ms();							// �֐�Call�^ Lag timer timeout check
	LagChk();								// Ұّ��M�^ Lag timer timeout check
	/*** coinmech control ***/
	if( CN_tsk_timer > 0 ){
		CN_tsk_timer--;
		if( CN_tsk_timer == 0 ){
			CNMTSK_START = 1;				// TASK START
		}
	}
	if( CN_escr_timer > 0 ){
		CN_escr_timer--;
		if( CN_escr_timer == 0 ){
			queset( OPETCBNO, (unsigned short)(COIN_ES_TIM), 0, NULL );	// ������ϰ��ѱ��
		}
	}
	if( RXWAIT > 0 ){
		RXWAIT--;
		if( RXWAIT == 0 ){
			CNMTSK_START = 1;				// TASK START
		}
	}
// MH321800(S) G.So IC�N���W�b�g�Ή�
	if( TXRXWAIT > 0 ){
		TXRXWAIT--;
	}
// MH321800(E) G.So IC�N���W�b�g�Ή�
	// �ެ��فEڼ�āE��������r������(GT��UT����������Ȃ�)
	for( i=0;i<2; i++ ){						// �����/�����N������̌o�ߎ���up
		if( JR_Print_Wait_tim[i] != 0xff ){		// ��ϰ�����čς݂��ǂ����𔻒�
			JR_Print_Wait_tim[i]++;				// ���čς݂Ȃ���Z
			if( JR_Print_Wait_tim[i] > 50 )		// 1s�ȏ�͕s�v�Ȃ̂�
				JR_Print_Wait_tim[i] = 0xff;	// ���������~
		}
	}

	if(( OP_MODUL != -1 )&&( OP_MODUL != 0 )){				// Stop or Timeout �ȊO?(Y)
		OP_MODUL--;
	}

	if(( NT_pcars_timer != -1 )&&( NT_pcars_timer != 0 )){				// Stop or Timeout �ȊO?(Y)
		NT_pcars_timer--;
	}

	if( 0 != (SYSMNTcomdr_sndcmp_timer & 0x7fff) ){
		--SYSMNTcomdr_sndcmp_timer;
	}
	if (0 != (SysMnt_RcvTimer & 0x7fff) ){
		--SysMnt_RcvTimer;
	}

	if( 0 != (FBcom_20msT1 & 0x7fff) ){
		--FBcom_20msT1;
	}

	if( 0 != (FBcom_20msT2 & 0x7fff) ){
		--FBcom_20msT2;
	}

	if( 0 != (FLP_TIM_CTRL & 0x7fff) ){
		--FLP_TIM_CTRL;
	}


	RAU_20msInt();
	if( 0 != (Capcom_20msT1 & 0x7fff) ){
		--Capcom_20msT1;
	}
	if( 0 != (Capcom_20msT2 & 0x7fff) ){
		--Capcom_20msT2;
	}
	if( 0 != (Btcom_20msT1 & 0x7fff) ){
		--Btcom_20msT1;
	}
	/*** 500ms interval process ***/
	++Tim500msCount;						// 500ms�ώZ����UP
	if( 25 <= Tim500msCount ){				// 500ms�o�� (Y)
		++TIM500_START;						// 500ms interval�o���׸�UP
		Tim500msCount = 0;
	}
	/*** 1s interval process ***/
	++Tim1sCount;													// 1s�ώZ�J�E���^UP
	if(50 <= Tim1sCount){
		LCM_1secInt();
		Tim1sCount = 0;												// 1s�ώZ�J�E���^�N���A
	}

	system_ticks++;
	Out1shotSig_Interval();					// �o�͐M��1shot����
	ist = _di2();
	wkb = TPU3.TSR.BYTE;
	TPU3.TSR.BIT.TGFA = 0;					// TGFA <- 0
	dummy_Read = TPU3.TSR.BYTE;				// Dummy Read
	TPU3.TIER.BIT.TGIEA = 1;				// TGIA������ <- EI
   	_ei2( ist );
}

/****************************************************************************/
/*																			*/
/*	CAN1																	*/
/*		VectorNo		52						 							*/
/*		ComName			�@����ʐM(FIFO MailBox):��M						*/
/*		ComRate			1Mbps												*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_CAN1_RXF1 )
void	Excep_CAN1_RXF1( void )
{
	can_std_frame_t	rx_dataframe;
	unsigned long api_status = R_CAN_OK;
	unsigned int i;

	for(i=0; i<4; i++){// ��MFIFO�o�b�t�@��4�����Ȃ��̂ōő�4��̃��[�v�ƂȂ�
		api_status = R_CAN_RxRead(1, CANBOX_RX, &rx_dataframe);	// ��M���b�Z�[�W�̎��o��
		if (api_status == R_CAN_OK) {
			can_rcv_queset(&rx_dataframe);
			g_ucCAN1RxFlag = 1;						// ��M�����t���O���Z�b�g
		}else{
			break;//��M���郁�b�Z�[�W�������Ȃ������I������
		}
	}
}
/****************************************************************************/
/*																			*/
/*	CAN1																	*/
/*		VectorNo		55						 							*/
/*		ComName			�@����ʐM(MailBox):���M							*/
/*		ComRate			1Mbps												*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_CAN1_TXM1 )
void	Excep_CAN1_TXM1( void )
{
	unsigned int	api_status;

	api_status = R_CAN_TxCheck(1, CANBOX_TX);	// ���b�Z�[�W���M�����`�F�b�N
	if (api_status == R_CAN_OK) {
		if (0 != transfer[CAN_DISPLAY].sndque->count) {
			api_status = R_CAN_TxSet(1, CANBOX_TX, &transfer[CAN_DISPLAY].sndque->dataframe[transfer[CAN_DISPLAY].sndque->readidx], DATA_FRAME);	// ���M���[���{�b�N�X�̐ݒ�
			--transfer[CAN_DISPLAY].sndque->count;
			++transfer[CAN_DISPLAY].sndque->readidx;
			if (transfer[CAN_DISPLAY].sndque->readidx >= transfer[CAN_DISPLAY].sndque->countmax) {
				transfer[CAN_DISPLAY].sndque->readidx = 0;
			}
		} else {
			if (transfer[CAN_DISPLAY].state == SDO_BLOCK_DOWNLOAD_IN_PROGRESS) {
				transfer[CAN_DISPLAY].state = SDO_END_DOWNLOAD_REQUEST;
				transfer[CAN_DISPLAY].timer = LifeTimGet();		// �����҂��^�C�}�Z�b�g
			}
			g_ucCAN1SendFlag = 0;	// ���̑��M����t
		}
	} else {
		nop();
	}
}
/****************************************************************************/
/*																			*/
/*	CANi ERSi																*/
/*		VectorNo		106						 							*/
/*		ComName			�@����ʐM(MailBox):Error							*/
/*		ComRate			1Mbps												*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_CANi_ERSi )
void	Excep_CANi_ERSi( void )
{
	volatile struct st_can __evenaccess * can_block_p;

	can_block_p = (struct st_can __evenaccess *) 0x91200;	// CAN1

	if (can_block_p->EIFR.BYTE) {
		g_ucCAN1ErrInfo |= can_block_p->EIFR.BYTE;
	}

	can_block_p->EIFR.BYTE = 0x00;		// �G���[���荞�ݗv���N���A
}
/********************************************************************************/
/*	RIIC1																		*/
/*		VectorNo		186														*/
/*		ComName			RS232C�ʐMDevice����(IC1,2,3): TMOF, AL, SP, Nack, ST	*/
/*		ComRate			400kbps													*/
/*		PCLK 			50MHz(20nsec/1clock)									*/
/*		------------------------------------------------------					*/
/*		�ڑ�IC			�Ǘ�Device		�g�pport	Baudrate					*/
/*		------------------------------------------------------					*/
/*		ExIC1			FOMA(I2cDev2_1)	PortA		19.200bps					*/
/*						AI-�W			PortB		19.200bps					*/
/*		ExIC2			ReceiptPrinter	PortA		19,200bps					*/
/*						JarnalPrinter	PortB		19,200bps					*/
/*		RTC				Epson_RTC		---			---							*/
/********************************************************************************/
//#pragma interrupt ( Excep_RIIC1_EEI1 )
void	Excep_RIIC1_EEI1( void )
{
	DBG_i2c_EEI1_cnt++;
	IIC_EEI_Int();		// TMOF, AL, SP, Nack, ST
}
/****************************************************************************/
/*	RIIC1																	*/
/*		VectorNo		187													*/
/*		ComName			RS232C�ʐMDevice����(IC1,2,3): ��M					*/
/*		ComRate			400kbps												*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*		------------------------------------------------------				*/
/*		�ڑ�IC			�Ǘ�Device		�g�pport	Baudrate				*/
/*		------------------------------------------------------				*/
/*		ExIC1			FOMA(I2cDev2_1)	PortA		19.200bps				*/
/*						AI-�W			PortB		19.200bps				*/
/*		ExIC2			ReceiptPrinter	PortA		19,200bps				*/
/*						JarnalPrinter	PortB		19,200bps				*/
/*		RTC				Epson_RTC		---			---						*/
/****************************************************************************/
//#pragma interrupt ( Excep_RIIC1_RXI1 )
void	Excep_RIIC1_RXI1( void )
{
	DBG_i2c_RXI1_cnt++;
	IIC_RXI_Int();
}
/****************************************************************************/
/*	RIIC1																	*/
/*		VectorNo		188													*/
/*		ComName			RS232C�ʐMDevice����(IC1,2,3): ���M					*/
/*		ComRate			400kbps												*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*		------------------------------------------------------				*/
/*		�ڑ�IC			�Ǘ�Device		�g�pport	Baudrate				*/
/*		------------------------------------------------------				*/
/*		ExIC1			FOMA(I2cDev2_1)	PortA		19.200bps				*/
/*						AI-�W			PortB		19.200bps				*/
/*		ExIC2			ReceiptPrinter	PortA		19,200bps				*/
/*						JarnalPrinter	PortB		19,200bps				*/
/*		RTC				Epson_RTC		---			---						*/
/****************************************************************************/
//#pragma interrupt ( Excep_RIIC1_TXI1 )
void	Excep_RIIC1_TXI1( void )
{
	DBG_i2c_TXI1_cnt++;
	IIC_TXI_Int();
}
/****************************************************************************/
/*	RIIC1																	*/
/*		VectorNo		189													*/
/*		ComName			RS232C�ʐMDevice����(IC1,2,3): Transmitt End		*/
/*		ComRate			400kbps												*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*		------------------------------------------------------				*/
/*		�ڑ�IC			�Ǘ�Device		�g�pport	Baudrate				*/
/*		------------------------------------------------------				*/
/*		ExIC1			FOMA(I2cDev2_1)	PortA		19.200bps				*/
/*						AI-�W			PortB		19.200bps				*/
/*		ExIC2			ReceiptPrinter	PortA		19,200bps				*/
/*						JarnalPrinter	PortB		19,200bps				*/
/*		RTC				Epson_RTC		---			---						*/
/****************************************************************************/
//#pragma interrupt ( Excep_RIIC1_TEI1 )
void	Excep_RIIC1_TEI1( void )
{
	DBG_i2c_TEI1_cnt++;
	IIC_TEI_Int();			// Transmitt End
}
/****************************************************************************/
/*																			*/
/*	SCI2																	*/
/*		VectorNo		220													*/
/*		ComName			JVMA(���ү�,����ذ��):��M							*/
/*		ComRate			4,800bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI2_RXI2 )
void	Excep_SCI2_RXI2( void )
{
	volatile unsigned char c;
	/************************************************************/
	/*	RX630��SSR�ɂ�RDRF���Ȃ��̂�							*/
	/*	ORER(Overrun) FER(Framing) PER(Parity)��Error���Ȃ����	*/
	/*	Read����ReadData���i�[����(RDRF <-0���Ȃ�)				*/
	/************************************************************/
	if((SCI2.SSR.BIT.ORER==1)||(SCI2.SSR.BIT.FER==1)||(SCI2.SSR.BIT.PER==1)) {
		c = 0xff;
	}
	else{
		c = SCI2.RDR;								// Received Data
	}
	dummy_Read = SCI2.SSR.BYTE;					// Dummy Read

	if( RXRSLT.BIT.B7 == 0 ){					// Already RECEIVE?
		if( RXCONT >= R_BUFSIZ ){				// Over flow?
			RXRSLT.BIT.B2 = 1;
		}else{
			*(RXBUFF+RXCONT) = c;
			RXCONT++;
			if( RXCONT == 1 ){					// First charactor?
				switch( c ){
					case 0x11:					// ACK1
						if( TXSDCM != 1 ){		// �ꊇ�v��?
							RXRSLT.BIT.B7 = 1;	// Reicve complete
							CNMTSK_START = 1;	// TASK START
						}
						break;
					case 0x44:					// ACK4
					case 0x55:					// ACK5
					case 0xEE:					// NAK
						RXRSLT.BIT.B7 = 1;		// Reicve complete
						CNMTSK_START = 1;		// TASK START
						break;
					case 0x22:					// ACK2
					case 0x33:					// ACK3
						break;
					default:					// ACK1�`5 �y�� NAK�ȊO�͔p������
						RXCONT = 0;
						return;
						//break;
				}
			}else if( RXCONT == 2 ){			// Second charactor?
				RXDTCT = (char)(c + 1);			// ��M���ׂ��޲Đ����
			}else{
				if( RXDTCT > 0 ){
					RXDTCT--;
					if( RXDTCT == 0 ){
						RXRSLT.BIT.B7 = 1;		// Reicve complete
						CNMTSK_START = 1;		// TASK START
					}
				}
			}
		}
		RXWAIT = RCV_MON_TIM;
	}
}
/****************************************************************************/
/*																			*/
/*	SCI2																	*/
/*		VectorNo		221													*/
/*		ComName			JVMA(���ү�,����ذ��):���M							*/
/*		ComRate			4,800bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI2_TXI2 )
void	Excep_SCI2_TXI2( void )
{
	/************************************************************/
	/*	RX630��SSR�ɂ�TDRE���Ȃ��̂�							*/
	/*	ORER(Overrun) FER(Framing) PER(Parity)��Error���Ȃ����	*/
	/*	Read����ReadData���i�[����(TDRE <-0���Ȃ�)				*/
	/************************************************************/
	if((SCI2.SSR.BIT.ORER==1)||(SCI2.SSR.BIT.FER==1)||(SCI2.SSR.BIT.PER==1)) {
		return;
	}
	if( TXWAIT != -1 ){									// Now TX mode ?
		if( TXRPTR >= TXCONT ){						// TX complete?
			TXWAIT = -1;							// TX complete set
			SCI2.SCR.BIT.TIE = 0;					// ���M������ <- DI
			SCI2.SCR.BIT.TEIE = 1;					// TEND���荞�݋��� 
													//���M������TEND���荞�݂Ŕ���
			dummy_Read = SCI2.SSR.BYTE;				// Dummy Read
		}else{
			if(( TXRPTR == 0 )&&( TXSDCM >= 0 )){
				SYN_ENB();							// SYN <- EI
			}
			wait2us( 40L );							// 80us wait
			SCI2.TDR = TXBUFF[TXRPTR];				// Write 1st charactor
			dummy_Read = SCI2.SSR.BYTE;				// Dummy Read
			TXRPTR++;								// TX charactor count+1
		}
	}else{
		SCI2.SCR.BIT.TIE = 0;						// ���M������ <- DI
		dummy_Read = SCI2.SSR.BYTE;					// Dummy Read
	}
}
/****************************************************************************/
/*																			*/
/*	SCI2																	*/
/*		VectorNo		222													*/
/*		ComName			JVMA(���ү�,����ذ��):Error							*/
/*		ComRate			4,800bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI2_TEI2 )
void	Excep_SCI2_TEI2( void )
{

	SCI2.SSR.BIT.ORER = 0 ;			// ERROR clear
	SCI2.SSR.BIT.FER = 0 ;
	SCI2.SSR.BIT.PER = 0 ;
	dummy_Read = SCI2.SSR.BYTE;		// Dummy Read
	SCI2.SCR.BIT.TE = 0;
	SCI2.SCR.BIT.TEIE = 0;
}

/****************************************************************************/
/*																			*/
/*	SCI3																	*/
/*		VectorNo		223													*/
/*		ComName			���CReader����:��M									*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI3_RXI3 )
void	Excep_SCI3_RXI3( void )
{
	/************************************************************/
	/*	RX630��SSR�ɂ�RDRF���Ȃ��̂�							*/
	/*	ORER(Overrun) FER(Framing) PER(Parity)��Error���Ȃ����	*/
	/*	Read����ReadData���i�[����(RDRF <-0���Ȃ�)				*/
	/************************************************************/
	FB_Int_RXI();
}
/****************************************************************************/
/*																			*/
/*	SCI3																	*/
/*		VectorNo		224													*/
/*		ComName			���CReader����:���M									*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI3_TXI3 )
void	Excep_SCI3_TXI3( void )
{
	FB_Int_TXI();
}
/****************************************************************************/
/*																			*/
/*	SCI3																	*/
/*		VectorNo		225													*/
/*		ComName			���CReader����:Error								*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI3_TEI3 )
void	Excep_SCI3_TEI3( void )
{
	FB_Int_TEI();
}
/****************************************************************************/
/*																			*/
/*	SCI4																	*/
/*		VectorNo		226													*/
/*		ComName			Flap����:��M										*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI4_RXI4 )
void	Excep_SCI4_RXI4( void )
{
	toS_Int_RXI2();
}
/****************************************************************************/
/*																			*/
/*	SCI4																	*/
/*		VectorNo		227													*/
/*		ComName			Flap����:���M										*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI4_TXI4 )
void	Excep_SCI4_TXI4( void )
{
	toS_Int_TXI2();
}
/****************************************************************************/
/*																			*/
/*	SCI4																	*/
/*		VectorNo		228													*/
/*		ComName			Flap����:Error										*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI4_TEI4 )
void	Excep_SCI4_TEI4( void )
{
	toS_Int_TEI2();
}
/****************************************************************************/
/*																			*/
/*	SCI6																	*/
/*		VectorNo		232													*/
/*		ComName			Bluetooth:��M										*/
/*		ComRate			115,200bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI6_RXI6 )
void	Excep_SCI6_RXI6( void )
{
	Bluetooth_Int_RXI();
}
/****************************************************************************/
/*																			*/
/*	SCI6																	*/
/*		VectorNo		233													*/
/*		ComName			Bluetooth:���M										*/
/*		ComRate			115,200bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI6_TXI6 )
void	Excep_SCI6_TXI6( void )
{
	Bluetooth_Int_TXI();
}
/****************************************************************************/
/*																			*/
/*	SCI6																	*/
/*		VectorNo		234													*/
/*		ComName			Bluetooth:TEI										*/
/*		ComRate			115,200bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI6_TEI6 )
void	Excep_SCI6_TEI6( void )
{
	Bluetooth_Int_TEI();
}
/****************************************************************************/
/*																			*/
/*	SCI7																	*/
/*		VectorNo		235													*/
/*		ComName			serverFOMA_Rau:��M									*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI7_RXI7 )
void	Excep_SCI7_RXI7( void )
{
	volatile unsigned char c;
	volatile unsigned char ErrCnt =0;

	KSG_Rau_Int_RXI();
}
/****************************************************************************/
/*																			*/
/*	SCI7																	*/
/*		VectorNo		236													*/
/*		ComName			serverFOMA_Rau:���M									*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI7_TXI7 )
void	Excep_SCI7_TXI7( void )
{
	KSG_Rau_Int_TXI();
}
/****************************************************************************/
/*																			*/
/*	SCI7																	*/
/*		VectorNo		237													*/
/*		ComName			serverFOMA_Rau:Error								*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI7_TEI7 )
void	Excep_SCI7_TEI7( void )
{
	KSG_Rau_Int_TEI();
}
/****************************************************************************/
/*																			*/
/*	SCI9																	*/
/*		VectorNo		241													*/
/*		ComName			NT-NET�ʐM:��M										*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI9_RXI9 )
void	Excep_SCI9_RXI9( void )
{
	volatile unsigned char c;
	volatile unsigned char ErrCnt =0;

	/************************************************************/
	/*	RX630��SSR�ɂ�RDRF���Ȃ��̂�							*/
	/*	ORER(Overrun) FER(Framing) PER(Parity)��Error���Ȃ����	*/
	/*	Read����ReadData���i�[����(RDRF <-0���Ȃ�)				*/
	/************************************************************/
	if((SCI9.SSR.BIT.ORER==1)||(SCI9.SSR.BIT.FER==1)||(SCI9.SSR.BIT.PER==1)) {
		ErrCnt++;
		return;
	}
	c = SCI9.RDR;								// Received Data
	dummy_Read = SCI9.SSR.BYTE;					// Dummy Read
	/********************************/
	/*	SCI9_Handler����Apri����	*/
	/********************************/
}
/****************************************************************************/
/*																			*/
/*	SCI9																	*/
/*		VectorNo		242													*/
/*		ComName			NT-NET�ʐM:���M										*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI9_TXI9 )
void	Excep_SCI9_TXI9( void )
{
	/********************************/
	/*	SCI9_Handler����Apri����	*/
	/********************************/
}
/****************************************************************************/
/*																			*/
/*	SCI9																	*/
/*		VectorNo		243													*/
/*		ComName			NT-NET�ʐM:Error									*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI9_TEI9 )
void	Excep_SCI9_TEI9( void )
{
	volatile unsigned char	c;

	if( SCI9.SSR.BIT.PER ){			// Parity Err?
		//	SCI9_Handler����Error����
	}
	if( SCI9.SSR.BIT.FER ){			// Flaming Err?
		//	SCI9_Handler����Error����
	}
	if( SCI9.SSR.BIT.ORER ){		// Overrun Err?
		//	SCI9_Handler����Error����
	}
	c = SCI9.RDR;					// Received Data
	SCI9.SSR.BIT.ORER = 0 ;			// ERROR clear
	SCI9.SSR.BIT.FER = 0 ;
	SCI9.SSR.BIT.PER = 0 ;
	dummy_Read = SCI9.SSR.BYTE;		// Dummy Read
}

/****************************************************************************/
/*																			*/
/*	SCI10																	*/
/*		VectorNo		244													*/
/*		ComName			ParkiPro:��M										*/
/*		ComRate			9,600bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI10_RXI10 )
void	Excep_SCI10_RXI10( void )
{
	unsigned char		uc_rxdat;
	/************************************************************/
	/*	RX630��SSR�ɂ�RDRF���Ȃ��̂�							*/
	/*	ORER(Overrun) FER(Framing) PER(Parity)��Error���Ȃ����	*/
	/*	Read����ReadData���i�[����(RDRF <-0���Ȃ�)				*/
	/************************************************************/
	if((SCI10.SSR.BIT.ORER==1)||(SCI10.SSR.BIT.FER==1)||(SCI10.SSR.BIT.PER==1)) {
		uc_rxdat = 0xff;
	} else {
		uc_rxdat = SCI10.RDR;				// ��M�f�[�^
	}

	if( PIPRAM.ui_count_h < R_BUF_MAX_H ){		// ��M�o�b�t�@�󂫂���
		PIPRAM.uc_r_buf_h[PIPRAM.ui_write_h] = uc_rxdat;
		PIPRAM.ui_write_h++;
		if( PIPRAM.ui_write_h >= R_BUF_MAX_H ){
			PIPRAM.ui_write_h = 0;
		}
		PIPRAM.ui_count_h++;
	}
	dummy_Read = SCI10.SSR.BYTE;				// Dummy Read
}
/****************************************************************************/
/*																			*/
/*	SCI10																	*/
/*		VectorNo		245													*/
/*		ComName			ParkiPro:���M										*/
/*		ComRate			9,600bpsbps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI10_TXI10 )
void	Excep_SCI10_TXI10( void )
{
	if((SCI10.SSR.BIT.ORER==0) && (SCI10.SSR.BIT.FER==0) && (SCI10.SSR.BIT.PER==0)){	/* send enable (Y)	*/
		if(( PIPRAM.ui_txpointer_h < PIPRAM.ui_txdlength_i_h )){			/* �܂����M�f�[�^�����芎���̃o�C�g�𑗂�܂� */
			SCI10.TDR = PIPRAM.huc_txdata_i_h[PIPRAM.ui_txpointer_h];		// �P�o�C�g���M
			PIPRAM.ui_txpointer_h++;
			dummy_Read = SCI10.SSR.BYTE;	// Dummy Read
			SCI10.SCR.BIT.TIE = 1;			// ���M�f�[�^�G���v�e�B(TXI)���荞�݋���
		} else {
			SCI10.SCR.BIT.TIE  = 0;			// ���M�f�[�^�G���v�e�B(TXI)���荞�݋֎~
			SCI10.SCR.BIT.TEIE = 1;			// ���M�I��(TEI)���荞�݋���
		}
	}
}
/****************************************************************************/
/*																			*/
/*	SCI10																	*/
/*		VectorNo		246													*/
/*		ComName			ParkiPro:Error										*/
/*		ComRate			9,600bpsbps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI10_TEI10 )
void	Excep_SCI10_TEI10( void )
{
	if( PIPRAM.ui_txpointer_h >= PIPRAM.ui_txdlength_i_h ) {	// �ʐM�f�[�^
		PIPRAM.ui_txdlength_i_h = 0;		// ���M�f�[�^�� �N���A
		PIPRAM.uc_send_end_h = 1;			// ���M�����t���O�Z�b�g
	}
	SCI10.SCR.BIT.TE = 0;		// �V���A�����M����֎~
	SCI10.SCR.BIT.TEIE = 0;		// ���M�I��(TEI)���荞�݋֎~
}

/****************************************************************************/
/*																			*/
/*	SCI11																	*/
/*		VectorNo		247													*/
/*		ComName			MultiICcardR/W:��M									*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI11_RXI11 )
void	Excep_SCI11_RXI11( void )
{
}
/****************************************************************************/
/*																			*/
/*	SCI11																	*/
/*		VectorNo		248													*/
/*		ComName			MultiICcardR/W:���M									*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI11_TXI11 )
void	Excep_SCI11_TXI11( void )
{
}
/****************************************************************************/
/*																			*/
/*	SCI11																	*/
/*		VectorNo		249													*/
/*		ComName			MultiICcardR/W:Error								*/
/*		ComRate			38,400bps											*/
/*		PCLK 			50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
//#pragma interrupt ( Excep_SCI11_TEI11 )
void	Excep_SCI11_TEI11( void )
{
}
//[]----------------------------------------------------------------------[]
///	@brief			�O���[�v12���荞�ݏ���(SCI,SPI�G���[���荞�ݏ���)
//[]----------------------------------------------------------------------[]
///	@return			ret		: void
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/01<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//#pragma interrupt (Excep_ICU_GL0)
void Excep_ICU_GL0( void )
{ 
// NOTE:
// �O���[�v12�ɂ͈ȉ��̊��荞�ݗv�������ނ���Ă���B
// �ȉ��̃G���[�������g���b�v�������ꍇ�́A�{�֐��ɃG���[���̏������L�q����K�v������B
//  SCI0	ERI0�iSCI0��M�G���[�j---�{��ł͖��g�p
//  SCI1	ERI1�iSCI1��M�G���[�j---�{��ł͖��g�p
//  SCI2	ERI2�iSCI2��M�G���[�j
//  SCI3	ERI3�iSCI3��M�G���[�j
//  SCI4	ERI4�iSCI4��M�G���[�j
//  SCI5	ERI5�iSCI5��M�G���[�j---�{��ł͖��g�p
//  SCI6	ERI6�iSCI6��M�G���[�j
//  SCI7	ERI7�iSCI7��M�G���[�j
//  SCI8	ERI8�iSCI8��M�G���[�j---�{��ł͖��g�p
//  SCI9	ERI9�iSCI9��M�G���[�j
//  SCI10	ERI10�iSCI10��M�G���[�j
//  SCI11	ERI11�iSCI11��M�G���[�j
//  SCI12	ERI12�iSCI12��M�G���[�j---�{��ł͖��g�p
//  RSPI0	SPEI0�i�G���[���荞�݁j
//  RSPI1	SPEI1�i�G���[���荞�݁j
//  RSPI2	SPEI2�i�G���[���荞�݁j
// ���荞�ݗv�����͏����ɂ���
// �O���[�v12�̊��荞�݂̓��x�����o�ł�
// �uRX630�O���[�v ���[�U�[�Y�}�j���A���@�n�[�h�E�F�A�ҁv�� �}15.16 ���Q�l�ɂ����B

	// SCI2��M�G���[�̊m�F
	if ( ICU.GRP[GRP_SCI2_ERI2].BIT.IS2 != 0 ) {		// ERI2�iSCI2��M�G���[�j���������Ă��邩�H
		if ( ICU.IER[IER_ICU_GROUPL0].BIT.IEN2 != 0 ) {	// �O���[�v12���荞�݂����ɂȂ��Ă��邩�H
			CNM_Int_ERI();
		}
	}

	// SCI3��M�G���[�̊m�F
	if ( ICU.GRP[GRP_SCI3_ERI3].BIT.IS3 != 0 ) {		// ERI3�iSCI3��M�G���[�j���������Ă��邩�H
		if ( ICU.IER[IER_ICU_GROUPL0].BIT.IEN2 != 0 ) {	// �O���[�v12���荞�݂����ɂȂ��Ă��邩�H
			FB_Int_ERI();
		}
	}

	// SCI4��M�G���[�̊m�F
	if ( ICU.GRP[GRP_SCI4_ERI4].BIT.IS4 != 0 ) {		// ERI4�iSCI4��M�G���[�j���������Ă��邩�H
		if ( ICU.IER[IER_ICU_GROUPL0].BIT.IEN2 != 0 ) {	// �O���[�v12���荞�݂����ɂȂ��Ă��邩�H
			toS_Int_ERI2();
		}
	}

	// SCI6��M�G���[�̊m�F
	if ( ICU.GRP[GRP_SCI6_ERI6].BIT.IS6 != 0 ) {		// ERI6�iSCI6��M�G���[�j���������Ă��邩�H
		if ( ICU.IER[IER_ICU_GROUPL0].BIT.IEN2 != 0 ) {	// �O���[�v12���荞�݂����ɂȂ��Ă��邩�H
			Bluetooth_Int_ERI();
		}
	}

	// SCI7��M�G���[�̊m�F
	if ( ICU.GRP[GRP_SCI7_ERI7].BIT.IS7 != 0 ) {		// ERI7�iSCI7��M�G���[�j���������Ă��邩�H
		if ( ICU.IER[IER_ICU_GROUPL0].BIT.IEN2 != 0 ) {	// �O���[�v12���荞�݂����ɂȂ��Ă��邩�H
			KSG_Rau_Int_ERI();
		}
	}

	// SCI9��M�G���[�̊m�F
	if ( ICU.GRP[GRP_SCI9_ERI9].BIT.IS9 != 0 ) {		// ERI9�iSCI9��M�G���[�j���������Ă��邩�H
		if ( ICU.IER[IER_ICU_GROUPL0].BIT.IEN2 != 0 ) {	// �O���[�v12���荞�݂����ɂȂ��Ă��邩�H
			if( SCI9.SSR.BIT.PER ){			// Parity Err?
				//	SCI9_Handler����Error����
			}
			if( SCI9.SSR.BIT.FER ){			// Flaming Err?
				//	SCI9_Handler����Error����
			}
			if( SCI9.SSR.BIT.ORER ){		// Overrun Err?
				//	SCI9_Handler����Error����
			}
			dummy_Read = SCI9.RDR;			// Received Data
			SCI9.SSR.BIT.ORER = 0 ;			// ERROR clear
			SCI9.SSR.BIT.FER = 0 ;
			SCI9.SSR.BIT.PER = 0 ;
			dummy_Read = SCI9.SSR.BYTE;		// Dummy Read
		}
	}

	// SCI10��M�G���[�̊m�F
	if ( ICU.GRP[GRP_SCI10_ERI10].BIT.IS10 != 0 ) {		// ERI10�iSCI10��M�G���[�j���������Ă��邩�H
		if ( ICU.IER[IER_ICU_GROUPL0].BIT.IEN2 != 0 ) {	// �O���[�v12���荞�݂����ɂȂ��Ă��邩�H
			if( SCI10.SSR.BIT.ORER ){
				pip_f_txerr_h_ovrun = 1;		// �I�[�o�[�����G���[�Z�b�g
			}
			if( SCI10.SSR.BIT.FER ){
				pip_f_txerr_h_fr = 1;			// �t���[�~���O�G���[�Z�b�g
			}
			if( SCI10.SSR.BIT.PER ){
				pip_f_txerr_h_p = 1;			// �p���e�B�G���[�Z�b�g
			}
			SCI10.SSR.BYTE &= 0xC4;				// �G���[�t���O�N���A
		}
	}

	// SCI11��M�G���[�̊m�F

	// ���荞�݃X�e�[�^�X���N���A����
	ICU.IR[VECT_ICU_GROUPL0].BIT.IR = 0;
}
/****************************************************************************/
/*	Write(NormalSequence) = IicMilestone.Write								*/
/*								wt_1		IIC_TXI_IntI2cWrite				*/
/*								wt_2		IIC_TXI_IntI2cWrite				*/
/*								wt_3		IIC_TXI_IntI2cWrite				*/
/*								wt_4		IIC_TXI_IntI2cWrite				*/
/*								wt_5		IIC_TEI_IntI2cWrite				*/
/*								wt_6		IIC_EEI_IntSP					*/
/*	Read(NormalSequence)  = IicMilestone.Read								*/
/*								rd_1		IIC_TXI_IntI2cRead				*/
/*								rd_2		IIC_TXI_IntI2cRead				*/
/*								rd_3		IIC_TXI_IntI2cRead				*/
/*								rd_4		IIC_TEI_IntI2cRead				*/
/*								rd_5		IIC_EEI_IntST					*/
/*								rd_6		IIC_RXI_IntI2cRead				*/
/*								rd_7		IIC_RXI_IntI2cRead(3)			*/
/*								rd_8		IIC_RXI_IntI2cRead				*/
/*								rd_9		IIC_EEI_IntSP					*/
/*	IicMilestone.Error			1			IIC_EEI_IntTimeOut				*/
/*								2			IIC_EEI_IntAL					*/
/*								3, 4		IIC_EEI_IntSP					*/
/*								0x10, 0x11	IIC_EEI_IntSP					*/
/*								0x20, 0x21	IIC_RXI_Int						*/
/*								0x30		IIC_TXI_Int						*/
/*								0x40		IIC_TEI_Int						*/
/*								0x50, 0x51	IIC_GenClkSP					*/
/*	IicMilestone.I2cINT[IicMilestone.I2cINT_num++]							*/
/****************************************************************************/
/*																			*/
/*	TMOF, AL, SP, Nack, ST Detected	Process									*/
/*		call�� = Excep_RIIC1_EEI1											*/
/*		�d�����ē������������ꍇ�͈ȉ��̏��ňꊇ�ŏ����������s��			*/
/****************************************************************************/
#define I2C_INT_NUM_EEI_IntTimeOut		0x01
#define I2C_INT_NUM_EEI_IntAL			0x02
#define I2C_INT_NUM_EEI_IntNack			0x03
#define I2C_INT_NUM_EEI_IntSP1			0x04
#define I2C_INT_NUM_EEI_IntSP2			0x05
#define I2C_INT_NUM_EEI_IntSP3			0x06
#define I2C_INT_NUM_EEI_IntSP4			0x07
#define I2C_INT_NUM_EEI_IntSP5			0x08
#define I2C_INT_NUM_EEI_IntSP6			0x09
#define I2C_INT_NUM_EEI_IntST			0x0A
#define I2C_INT_NUM_RXI_IntI2cRead1		0x0B
#define I2C_INT_NUM_RXI_IntI2cRead2		0x0C
#define I2C_INT_NUM_RXI_IntI2cRead3		0x0D
#define I2C_INT_NUM_RXI_IntI2cRead4		0x0E
#define I2C_INT_NUM_RXI_IntI2cRead5		0x0F
#define I2C_INT_NUM_RXI_IntI2cRead6		0x10
#define I2C_INT_NUM_TXI_IntI2cWrite1	0x11
#define I2C_INT_NUM_TXI_IntI2cWrite2	0x12
#define I2C_INT_NUM_TXI_IntI2cWrite3	0x13
#define I2C_INT_NUM_TXI_IntI2cWrite4	0x14
#define I2C_INT_NUM_TXI_IntI2cRead1		0x15
#define I2C_INT_NUM_TXI_IntI2cRead2		0x16
#define I2C_INT_NUM_TXI_IntI2cRead3		0x17
#define I2C_INT_NUM_TEI_IntI2cWrite		0x18
#define I2C_INT_NUM_TEI_IntI2cRead		0x19
// Sc16RecvInSending_IRQ5_Level_L start
void I2cSendDataBuff_logging(unsigned char data)
{
//	I2cSendLsiAddr
	switch(I2cSendSlaveAddr) {
	case SLV_ADDR_I2CDEV2_1:		//	SLV_ADDR_I2CDEV2_2
	case SLV_ADDR_RP:			//	SLV_ADDR_JP
		I2cSendDataBuff[I2cSendDataCnt & 0x00ff] = data;
		I2cSendDataCnt++;
		break;
	default:
		break;
	}
}
// Sc16RecvInSending_IRQ5_Level_L end
void IIC_EEI_Int(void)
{
	if((RIIC1.ICSR2.BIT.TMOF!=0) && (RIIC1.ICIER.BIT.TMOIE!=0)) {
		IIC_EEI_IntTimeOut();		// Timeout
	}
	if((RIIC1.ICSR2.BIT.AL!=0) && (RIIC1.ICIER.BIT.ALIE!=0)) {
		IIC_EEI_IntAL();			// Arbitration Lost
	}
	if((RIIC1.ICSR2.BIT.STOP!=0) && (RIIC1.ICIER.BIT.SPIE!=0)) {
		IIC_EEI_IntSP();			// stop condition detection
	}
	if((RIIC1.ICSR2.BIT.NACKF != 0) && (RIIC1.ICIER.BIT.NAKIE!=0)) {
		IIC_EEI_IntNack();			// NACK reception
	}
	if((RIIC1.ICSR2.BIT.START != 0) && (RIIC1.ICIER.BIT.STIE!=0)) {
		IIC_EEI_IntST();			// start condition detection
	}
}
/****************************************************************************/
/*																			*/
/*	Time out function														*/
/*																			*/
/****************************************************************************/
void IIC_EEI_IntTimeOut(void)
{
	IicMilestone.Error = 1;		// = DebugInfo
	IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntTimeOut;	// = DebugInfo
	IIC_GenClkSP();
}
/****************************************************************************/
/*																			*/
/*	Arbitration Lost function												*/
/*	SingleMaster�Ȃ̂ő������Ȃ�											*/
/****************************************************************************/
void IIC_EEI_IntAL(void)
{
	IicMilestone.Error = 2;		// = DebugInfo
	IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntAL;			// = DebugInfo
	IIC_GenClkSP();
}
/****************************************************************************/
/*																			*/
/*	NACK reception function													*/
/*																			*/
/****************************************************************************/
void IIC_EEI_IntNack(void)
{
	/********************************/
	/*	Change the internal mode	*/
	/********************************/
	iic_status = RIIC_STATUS_NACK;
	IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntNack;		// = DebugInfo
	/****************************************/
	/*	NACK reception interrupt <- DI		*/
	/****************************************/
	RIIC1.ICIER.BIT.NAKIE = 0;
	/********************************/
	/*	Generate Stop Condition		*/
	/********************************/
	RIIC1.ICSR2.BIT.STOP = 0;
	RIIC1.ICCR2.BIT.SP = 1;			// StopBit -> Out
}
/****************************************************************************/
/*																			*/
/*	Stop condition detection function										*/
/*																			*/
/****************************************************************************/
void IIC_EEI_IntSP(void)
{
	/****************************/
	/*	Clear each status flag	*/
	/****************************/
	RIIC1.ICSR2.BIT.NACKF = 0;
	RIIC1.ICSR2.BIT.STOP = 0;
	/************************************************************/
	/*	Enable/Disable each interrupt							*/
	/*		b0:1 Time Out interrupt <- DI 						*/
	/*		b1:1 Arbitration Lost interrupt <- EI 				*/
	/*		b2:0 Start Condition Detection Interrupt <- DI		*/
	/*		b3:1 Stop condition detection interrupt <- EI 		*/
	/*		b4:1 NACK reception interrupt <- EI 				*/
	/*		b5:1 Receive Data Full Interrupt <- EI	 			*/
	/*		b6:0 Transmit End Interrupt <- DI	 				*/
	/*		b7:1 Transmit Data Empty Interrupt <- EI			*/
	/************************************************************/
	RIIC1.ICIER.BYTE = 0xBB;
	switch(iic_mode) {
		case IIC_MODE_I2C_WRITE:
			if(iic_ActiveBuff.RWCnt != 0) {		// the communication is not complete
				iic_status = RIIC_STATUS_FAILED;
				IicMilestone.Error = 3;			// = DebugInfo
				IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntSP1;	// = DebugInfo
			} else if(iic_status != RIIC_STATUS_NACK) {
				iic_status = RIIC_STATUS_IDLE;				// wt_6
				IicMilestone.Write = 6;						// = DebugInfo
				IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntSP2;	// = DebugInfo
// Sc16RecvInSending_IRQ5_Level_L start
//I2C_Event_Info.BIT.Print1ByteSendReq = 1;
//wait2us(200L);			// 19200(54us x 8 = 432us)
//_ei();
//Sc16SendInI2C_RP();
//Sc16SendInI2C_JP();
// Sc16RecvInSending_IRQ5_Level_L end
			}
			break;
		case IIC_MODE_I2C_READ:
			if(iic_ActiveBuff.RWCnt != 0) {		// the communication is not complete
				iic_status = RIIC_STATUS_FAILED;
				IicMilestone.Error = 4;			// = DebugInfo
				IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntSP3;	// = DebugInfo
			} else {
				iic_status = RIIC_STATUS_IDLE;				// rd_9
				IicMilestone.Read = 9;						// = DebugInfo
				IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntSP4;	// = DebugInfo
//				RIIC1.ICFER.BIT.TMOE = 0;	// time out detection <- DI
				IicMilestone.I2cSP_Sending = 0;
//				IicMilestone.debugSP1_IntBuff[IicMilestone.debugSP1_Cnt & 0xff] = TPU5.TCNT;
//				IicMilestone.debugSP1_Cnt++;
//				if (IicMilestone.debugSP1_ToutOccer == 1) {
//					IicMilestone.debugSP1_ToutOccer = 9;
//				}
// Sc16RecvInSending_IRQ5_Level_L start
//I2C_Event_Info.BIT.Print1ByteSendReq = 1;
//wait2us(200L);			// 19200(54us x 8 = 432us)
//_ei();
//Sc16SendInI2C_RP();
//Sc16SendInI2C_JP();
// Sc16RecvInSending_IRQ5_Level_L end
			}
			break;
		case IIC_MODE_I2C_IDLE:
			IicMilestone.Error = 0x11;			// = DebugInfo
			IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntSP5;	// = DebugInfo
			break;
		default:
			I2C_BusError_Info.BIT.I2C_R_W_MODE = 1;			// Bit 0 = Read/Write���[�h�G���[		������������
			IicMilestone.Error = 0x10;			// = DebugInfo
			IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntSP6;		// = DebugInfo
			IIC_Error(RIIC_ERR_MODE_SP_INT);// Internal mode error
			break;
	}
	/********************************/
	/*	Initialize ram for RIIC		*/
	/********************************/
	iic_mode = IIC_MODE_I2C_IDLE;
	iic_trm_cnt = 0;
	iic_rcv_cnt = 0;
}
/****************************************************************************/
/*																			*/
/*	Start condition detection function										*/
/*																			*/
/****************************************************************************/
void IIC_EEI_IntST(void)			// rd_5
{
	/************************************************/
	/*	Start Condition Detection Interrupt <- DI	*/
	/************************************************/
	RIIC1.ICSR2.BIT.START = 0;
	RIIC1.ICIER.BIT.STIE = 0;
	/********************************************/
	/*	Transfer slave device address(Read)		*/
	/********************************************/
	RIIC1.ICDRT = iic_ActiveBuff.SlvAdr | 0x01;
	IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_EEI_IntST;		// = DebugInfo
	IicMilestone.Read = 5;		// = DebugInfo
}
/****************************************************************************/
/*																			*/
/*	 Receive Data Full interrupt function									*/
/*		call�� = Excep_RIIC1_RXI1											*/
/****************************************************************************/
void IIC_RXI_Int(void)
{
	switch(iic_mode) {
		case IIC_MODE_I2C_READ:
			IIC_RXI_IntI2cRead();
			break;
		case IIC_MODE_I2C_WRITE:
			I2C_BusError_Info.BIT.I2C_R_W_MODE = 1;			// Bit 0 = Read/Write���[�h�G���[		������������
			IicMilestone.Error = 0x20;		// = DebugInfo
			IIC_Error(RIIC_ERR_MODE_RX_INT_I2C_WRITE);
			break;
		default:
			I2C_BusError_Info.BIT.I2C_R_W_MODE = 1;			// Bit 0 = Read/Write���[�h�G���[		������������
			IicMilestone.Error = 0x21;		// = DebugInfo
			IIC_Error(RIIC_ERR_MODE_RX_INT);
			break;
	}
}
/****************************************************************************/
/*																			*/
/*	RXI interrupt function for I2cRead process								*/
/*																			*/
/****************************************************************************/
void IIC_RXI_IntI2cRead(void)
{
	volatile unsigned char tmp;

	iic_rcv_cnt++;					//	receive data counter++
	IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_RXI_IntI2cRead1;	// = DebugInfo
	/************************/
	/*	1��ڂ�Read���̂�	*/
	/************************/
	if(iic_rcv_cnt == 1) {				// rd_6(CPU���瑗�M����SlaveAddress�Ȃ̂�dummy read�̂ݍs��)
		IicMilestone.Read = 6;			// = DebugInfo
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_RXI_IntI2cRead2;	// = DebugInfo
		/************************************/
		/*	�����Read����Data���ŏI�̎�	*/
		/************************************/
		if(iic_ActiveBuff.RWCnt == 1) {		// rd_7
			IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_RXI_IntI2cRead3;	// = DebugInfo
			IicMilestone.Read = 7;		// = DebugInfo
			/************************************************************************/
			/*	When WAIT= 1, RIIC outputs SCL=Low after it receives 9th clock.		*/
			/*	Then RIIC generates stop condition after it reads receive data.		*/
			/************************************************************************/
			RIIC1.ICMR3.BIT.WAIT = 1;
			/****************************************************************************************************************/
			/*	If RIIC sends ACK when it receives last data, I2cBusDevice may prevent RIIC generating 						*/
			/*	stop condition. Because I2cBusDevice desn't know how many RIIC try to read data.							*/
			/*	When RIIC sends ACK, I2cBusDevice realizes the next rising SCL for stop condition as next first bit SCL.	*/
			/****************************************************************************************************************/
			RIIC1.ICMR3.BIT.ACKBT = 1;	//	Set Nack for final data
		}
		tmp = RIIC1.ICDRR;				//	dummy read
	/************************************/
	/*	�����Read����Data���ŏI�̎�	*/
	/************************************/
	}else if(iic_ActiveBuff.RWCnt == 2) {		// rd_7
		IicMilestone.Read = 7;			// = DebugInfo
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_RXI_IntI2cRead4;	// = DebugInfo
		RIIC1.ICMR3.BIT.WAIT = 1;
		RIIC1.ICMR3.BIT.ACKBT = 1;		//	Set Nack for final data
		*iic_ActiveBuff.pRWData++ = RIIC1.ICDRR;
		iic_ActiveBuff.RWCnt--;
	/************************************/
	/*	�����Read����Data���ŏI�̎�	*/
	/************************************/
	}else if(iic_ActiveBuff.RWCnt == 1) {		// rd_8
		/****************************************/
		/*	Prepare to generate Stop Condition	*/
		/****************************************/
		IicMilestone.Read = 8;			// = DebugInfo
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_RXI_IntI2cRead5;	// = DebugInfo
//		RIIC1.ICFER.BIT.TMOE = 0;	// time out detection <- DI
		IicMilestone.I2cSP_Sending = 1;
//		IicMilestone.debugSP1_SetBuff[IicMilestone.debugSP1_Cnt & 0xff] = TPU5.TCNT;
//		IicMilestone.debugSP1_ToutOccer = 0;
		RIIC1.ICSR2.BIT.STOP = 0;
		RIIC1.ICCR2.BIT.SP = 1;			// StopBit -> Out
		/************************************/
		/*	After read final data, 			*/
		/*	RIIC generates stop condtion	*/
		/************************************/
		*iic_ActiveBuff.pRWData++ = RIIC1.ICDRR;	//	Read final data
		iic_ActiveBuff.RWCnt--;
		RIIC1.ICMR3.BIT.WAIT = 0;
	/****************************************/
	/*	n���(3Frame�ȏ��Read�����鎞)		*/
	/****************************************/
	} else {							// rd_7
		IicMilestone.Read = 7;			// = DebugInfo
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_RXI_IntI2cRead6;	// = DebugInfo
		*iic_ActiveBuff.pRWData++ = RIIC1.ICDRR;
		iic_ActiveBuff.RWCnt--;
	}
}
/****************************************************************************/
/*																			*/
/*	Transmit Data Empty Interrupt function									*/
/*		call�� = Excep_RIIC1_TXI1											*/
/****************************************************************************/
void IIC_TXI_Int(void)
{
	switch(iic_mode) {
		case IIC_MODE_I2C_WRITE:
			IIC_TXI_IntI2cWrite();
			break;
		case IIC_MODE_I2C_READ:
			IIC_TXI_IntI2cRead();
			break;
		default:
//			I2C_BusError_Info.BIT.I2C_R_W_MODE = 1;			// Bit 0 = Read/Write���[�h�G���[		������������
//			IicMilestone.Error = 0x30;		// = DebugInfo
//			IIC_Error(RIIC_ERR_MODE_TX_INT);
			IicMilestone.Error = 0x30;		// = DebugInfo
			break;
	}
}
/****************************************************************************/
/*																			*/
/*	TXI interrupt function for I2c Write process							*/
/*																			*/
/****************************************************************************/
void IIC_TXI_IntI2cWrite(void)
{
	iic_trm_cnt++;
	if(iic_trm_cnt == 1) {
		/********************************************/
		/*	When master transfer, b0 must be '0'	*/
		/********************************************/
		RIIC1.ICDRT = iic_ActiveBuff.SlvAdr & 0xFE;			// wt_1
		IicMilestone.Write = 1;				// = DebugInfo
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TXI_IntI2cWrite1;	// = DebugInfo
// Sc16RecvInSending_IRQ5_Level_L start
//		I2cSendSlaveAddr = iic_ActiveBuff.SlvAdr;
// Sc16RecvInSending_IRQ5_Level_L end
		return;
	};
	/************************************************/
	/*	Transfer I2cBusDevice address for writting	*/
	/************************************************/
	if(iic_ActiveBuff.PreCnt != 0) {
		RIIC1.ICDRT = *iic_ActiveBuff.pPreData++;				// wt_2
		IicMilestone.Write = 2;				// = DebugInfo
		iic_ActiveBuff.PreCnt--;
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TXI_IntI2cWrite2;	// = DebugInfo
		return;
	}
	/********************************/
	/*	Transfer data for writting	*/
	/********************************/
	if(iic_ActiveBuff.RWCnt != 0) {
		RIIC1.ICDRT = *iic_ActiveBuff.pRWData++;				// wt_3
		IicMilestone.Write = 3;				// = DebugInfo
		iic_ActiveBuff.RWCnt--;
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TXI_IntI2cWrite3;	// = DebugInfo
// Sc16RecvInSending_IRQ5_Level_L start
//		I2cSendDataBuff_logging();
// Sc16RecvInSending_IRQ5_Level_L end
		return;
	/************************************/
	/*	Transmit End Interrupt <- EI	*/
	/************************************/
	} else {
		RIIC1.ICIER.BIT.TEIE = 1;						// wt_4
		IicMilestone.Write = 4;				// = DebugInfo
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TXI_IntI2cWrite4;	// = DebugInfo
	}
}
/****************************************************************************/
/*																			*/
/*	TXI interrupt function for I2cRead process								*/
/*																			*/
/****************************************************************************/
void IIC_TXI_IntI2cRead(void)
{
	iic_trm_cnt++;
	if(iic_trm_cnt == 1) {
		/********************************************/
		/*	When master transfer, b0 must be '0'	*/
		/********************************************/
		RIIC1.ICDRT = iic_ActiveBuff.SlvAdr & 0xFE;			// rd_1
		IicMilestone.Read = 1;				// = DebugInfo
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TXI_IntI2cRead1;	// = DebugInfo
		return;
	};
	/************************************************/
	/*	Transfer I2cBusDevice address for writting	*/
	/************************************************/
	if(iic_ActiveBuff.PreCnt != 0) {
		RIIC1.ICDRT = *iic_ActiveBuff.pPreData++;				// rd_2
		IicMilestone.Read = 2;				// = DebugInfo
		iic_ActiveBuff.PreCnt--;
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TXI_IntI2cRead2;	// = DebugInfo
		return;
	/************************************/
	/*	Transmit End Interrupt <- EI	*/
	/************************************/
	} else {
		RIIC1.ICIER.BIT.TEIE = 1;						// rd_3
		IicMilestone.Read = 3;				// = DebugInfo
		IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TXI_IntI2cRead3;	// = DebugInfo
	}
}
/****************************************************************************/
/*																			*/
/*	Transmit End Interrupt function											*/
/*		call�� = Excep_RIIC1_TEI1											*/
/****************************************************************************/
void IIC_TEI_Int(void)
{
	switch(iic_mode) {
		case IIC_MODE_I2C_WRITE:
			IIC_TEI_IntI2cWrite();
			break;
		case IIC_MODE_I2C_READ:
			IIC_TEI_IntI2cRead();
			break;
		default:
			I2C_BusError_Info.BIT.I2C_R_W_MODE = 1;			// Bit 0 = Read/Write���[�h�G���[		������������
			IicMilestone.Error = 0x40;		// = DebugInfo
			IIC_Error(RIIC_ERR_MODE_TE_INT);
			break;
	}
}
/****************************************************************************/
/*																			*/
/*	TEI interrupt function for I2c Write process							*/
/*																			*/
/****************************************************************************/
void IIC_TEI_IntI2cWrite(void)		// wt_5
{
	/************************************/
	/*	Transmit End Interrupt <- DI	*/
	/************************************/
	RIIC1.ICIER.BIT.TEIE = 0;
	/****************************/
	/*	Generate stop condition	*/
	/****************************/
	RIIC1.ICSR2.BIT.STOP = 0;
	RIIC1.ICCR2.BIT.SP = 1;			// StopBit -> Out
	IicMilestone.Write = 5;			// = DebugInfo
	IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TEI_IntI2cWrite;	// = DebugInfo
}
/****************************************************************************/
/*																			*/
/*	TEI interrupt function for I2cRead process								*/
/*																			*/
/****************************************************************************/
void IIC_TEI_IntI2cRead(void)		// rd_4
{
	RIIC1.ICSR2.BIT.START = 0;	// Clear Start Condition Detection flag
	RIIC1.ICIER.BIT.STIE = 1;	// Start Condition Detection Interrupt <- EI
	RIIC1.ICIER.BIT.TEIE = 0;	// Transmit End Interrupt <- DI
	/********************************/
	/*	Generate restart condition	*/
	/********************************/
	RIIC1.ICCR2.BIT.RS = 1;
	IicMilestone.Read = 4;		// = DebugInfo
	IicMilestone.I2cINT[IicMilestone.I2cINT_num++] = I2C_INT_NUM_TEI_IntI2cRead;	// = DebugInfo
}
/****************************************************************************/
/*																			*/
/*		reset RIIC, Then do return processing								*/
/*		call�� =IIC_EEI_IntTimeOut											*/
/*				IIC_EEI_IntAL												*/
/****************************************************************************/
void IIC_GenClkSP(void)
{
	volatile unsigned short cnt;
	/********************************************************/
	/*	ICE=1 and IICRST=1 clear some status flags.			*/
	/*	This reset desn't clear some settings, baud rate, 	*/
	/*	slave address and so on.							*/
	/********************************************************/
	RIIC1.ICCR1.BIT.IICRST = 1;	// Reset RIIC
	/********************************/
	/*	Time out setting			*/
	/********************************/
	RIIC1.ICFER.BIT.TMOE = 0;	// time out detection <- DI
	RIIC1.ICMR2.BIT.TMOL = 1;	// SCL <- DI = Low time out
	RIIC1.ICMR2.BIT.TMOH = 1;	// SCL <- EI = High time out
	RIIC1.ICMR2.BIT.TMOS = 0;	// Long mode (16bits counter), PCLK=48MHz,
								// IIC phi=48MHz, Long mode is about 1.364 ms
	RIIC1.ICCR1.BIT.IICRST = 0;	// Clear reset */
	/****************************************************/
	/*	If RIIC output SDA=Low, SDA output is released	*/
	/*	by IICRST=1. But IIC bus is pulluped therefore 	*/
	/*	the SDA rising is not immediate.				*/
	/****************************************************/
	for(cnt=0; cnt<100; cnt++);
	RIIC1.ICFER.BIT.TMOE = 1;			// time out detection <- EI
	/****************************************************************/
	/*	When SCL=Low though IICRST=1, other device output SCL=Low	*/
	/****************************************************************/
	if(RIIC1.ICCR1.BIT.SCLI == 0) {		// Check SCL level
		I2C_BusError_Info.BIT.I2C_CLOCK_LEVEL = 1;		// Bit 1 = �N���b�N���x���G���[			������������ 
		IicMilestone.Error = 0x50;		// = DebugInfo
		IIC_Error(RIIC_ERR_SCL_LOCK);
	}
	/********************************************************************/
	/*	After reset RIIC, this flow generates Stop condition.			*/
	/*	But MST bit have to be set when RIIC generates Stop condition.	*/
	/*	Here, disable protect, then set TRS and MST bits.				*/
	/*	If set only MST, RIIC generates some clocks because RDRF=0.		*/
	/*	When MST=1, TRS=0 and RDRF=0, RIIC starts Master Receive.		*/
	/********************************************************************/
	RIIC1.ICMR1.BIT.MTWP = 1;
	RIIC1.ICCR2.BIT.TRS = 1;
	RIIC1.ICCR2.BIT.MST = 1;
	/********************************************************************/
	/*	If other devices output SDA=Low, RIIC generates some clocks.	*/
	/*	Usually, devices stop SDA=Low when they recieve some clocks.	*/
	/********************************************************************/
	if(RIIC1.ICCR1.BIT.SDAI == 0) {
		for(cnt=0; cnt<10; cnt++) {					// Generate 10 clocks until SDA=High
			if(RIIC1.ICCR1.BIT.SDAI == 0) {
				RIIC1.ICCR1.BIT.CLO = 1;			// Generate a clock
				while(RIIC1.ICCR1.BIT.CLO != 0) {	// Wait for complete a generating clock
					if(RIIC1.ICSR2.BIT.TMOF !=0) {	// When other device output SCL=Low,
						RIIC1.ICSR2.BIT.TMOF =0;
						break;
					}
				}
			} else {
				break;	// When SDA=High, stop generating clocks
			}
			/************************************************************************/
			/*	Call error function if SDA = Low after RIIC generates some clocks	*/
			/************************************************************************/
			if((cnt == 9)&&(RIIC1.ICCR1.BIT.SDAI == 0)) {
				I2C_BusError_Info.BIT.I2C_SEND_DATA_LEVEL = 1;	// Bit 2 = ���M�f�[�^���x���G���[		������������
				IicMilestone.Error = 0x51;		// = DebugInfo
				IIC_Error(RIIC_ERR_SDA_LOCK);
			}
		}
	}
	/********************************************************/
	/*	When Bus free, RIIC does't generate Stop condition	*/
	/********************************************************/
	if(RIIC1.ICCR2.BIT.BBSY == 0) {
		RIIC1.ICCR1.BIT.IICRST = 1;		// Reset RIIC
		RIIC1.ICCR1.BIT.IICRST = 0;		// Clear reset
	} else {
		/****************************/
		/*	Generate stop condition	*/
		/****************************/
		RIIC1.ICSR2.BIT.STOP = 0;
		RIIC1.ICCR2.BIT.SP = 1;
	}
	/************************************/
	/*	MST/TRS Write Protect <- EI		*/
	/************************************/
	RIIC1.ICMR1.BIT.MTWP = 0;
	/********************************/
	/*	Time out setting			*/
	/********************************/
	RIIC1.ICFER.BIT.TMOE = 0;	// time out detection <- DI
	RIIC1.ICMR2.BIT.TMOL = 0;	// SCL <- DI = Low time out
	RIIC1.ICMR2.BIT.TMOH = 1;	// SCL <- EI = High time out
	RIIC1.ICMR2.BIT.TMOS = 0;	// Long mode (16bits counter), PCLK=48MHz,
								// IIC phi=48MHz. Long mode is about 1.365 ms
	RIIC1.ICFER.BIT.TMOE = 1;	// time out detection <- EI
}
/****************************************************************************/
/*																			*/
/*	Usually this function is not called										*/
/*																			*/
/****************************************************************************/
void IIC_Error(enum RiicErrCode_e error_code)
{
	unsigned long cnt;

	switch(error_code) {
	case RIIC_ERR_MODE_SP_INT:				break;
	case RIIC_ERR_MODE_RX_INT_I2C_WRITE:	break;
	case RIIC_ERR_MODE_RX_INT:				break;
	case RIIC_ERR_MODE_TX_INT:				break;
	case RIIC_ERR_MODE_TE_INT:				break;
	case RIIC_ERR_SCL_LOCK:					break;
	case RIIC_ERR_SDA_LOCK:					break;
	case RIIC_ERR_GEN_CLK_BBSY:				break;
	default:								break;
	}

	MSTP_RIIC1 = 0;					// �������ǉ�����K�v������B
	RIIC1.ICCR1.BIT.ICE 	= 0;	// RIIC disable for RIIC initial
	RIIC1.ICCR1.BIT.IICRST 	= 1;	// RIIC all reset
	RIIC1.ICCR1.BIT.IICRST 	= 0;	// Clear reset
	RIIC1.ICMR1.BIT.CKS = 2;		// IIC phi = PCLK clock
	RIIC1.ICBRL.BYTE = 0xF0;
	RIIC1.ICBRH.BYTE = 0xE7;
	RIIC1.ICMR2.BIT.TMOL = 0;		// Disable SCL=Low time out
	RIIC1.ICMR2.BIT.TMOH = 1;		// Enable SCL=High time out
	RIIC1.ICMR2.BIT.TMOS = 0;		// Long mode (16bits counter)
									// PCLK=48MHz, IIC phi=48MHz,
									// Long mode is about 1.365 ms
	RIIC1.ICFER.BIT.TMOE = 1;		// Enable time out detection
	RIIC1.ICSER.BYTE = 0x00;
	RIIC1.ICMR3.BIT.ACKWP = 1;
	RIIC1.ICIER.BYTE = 0xBB;
	iic_mode 	= IIC_MODE_I2C_IDLE;
	iic_status 	= RIIC_STATUS_IDLE;
	iic_trm_cnt = 0;
	iic_rcv_cnt = 0;
	RIIC1.ICCR1.BIT.ICE = 1;
	for (cnt=0; cnt<0x100000; cnt++);
}

/****************************************************************/
/*	RP, JP�̏ꍇ��1Byte��Application�Ɏ󂯓n���Ƃ̎�			*/
/*		---> ��MData����������Task�֑��M����(queueset)			*/
/*	���M��Task = Device����̔񓯊���M�Ȃ̂�TaskNoTo�ɂ͂Ȃ�	*/
/****************************************************************/
void I2c_RecvNotifyFromRp_IRQ5(void)
{
	PRN_DATA_BUFF	*p_data_buff;	// ������ް��ޯ̧�߲���
	uchar			rxd = I2cAM.DevBufRp[I2cAM.CtrRp.DevRecvP-1];
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	ushort			pinfo_size;
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

	if (I2cAM.CtrRp.DevRecvP > SIZE_DEV_RP_MAX) {
		if (I2cErrStatus.RecvRP == 0) {
			I2cErrStatus.RecvRP = 1;
			I2cErrEvent.RecvRP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_RECV_RP = 1;	// BIT 9 = ���V�[�g�v�����^��M�o�b�t�@�t��(�wI2Cڼ�Ď�M�ޯ̧�فx)		����/������
		}
	} else {
		if (I2cErrStatus.RecvRP != 0) {
			I2cErrStatus.RecvRP = 0;
			I2cErrEvent.RecvRP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_RECV_RP = 0;	// BIT 9 = ���V�[�g�v�����^��M�o�b�t�@�t��(�wI2Cڼ�Ď�M�ޯ̧�فx)		����/������
		}
	}
	p_data_buff = (PRN_DATA_BUFF *)&rct_prn_buff;							// ������ް��ޯ̧�߲������
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	pinfo_size = 0;
	if (isPrnInfoReq_R()) {
		// �v�����^���v����
		if (isPrnInfoInitRecv_R()) {
			// �����M
			if (isPrnInfoRes_R(I2cAM.DevBufRp[0])) {
				// �v�����^��񉞓�
				pinfo_size = I2cAM.CtrRp.DevRecvP;
			}
		}
		else {
			// 2��ڈȍ~��M
			pinfo_size = I2cAM.CtrRp.DevRecvP;
		}
	}

	if( isPrnInfoReq_R() && pinfo_size > 0 ){
		// �v�����^��񉞓���M
		if (PrnCmd_InfoRes_Proc_R(&I2cAM.DevBufRp[0], pinfo_size) != 0) {
			I2C_Event_Info.BIT.R_PRINTER_RCV_INFO = 1;
		}
		I2cAM.CtrRp.DevRecvP = 0;
	}
	else {
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	I2cAM.CtrRp.DevRecvP = 0;
	if( p_data_buff->PrnStWork != rxd ){								// ���ނ��Ă���ð���Ǝ�M�ް��̔�r
		//	�ð�����ω������ꍇ
		I2C_Event_Info.BIT.R_PRINTER_RCV_CMP = 1;
		p_data_buff->PrnStateMnt = rxd;									// ��������ݽ�p�ɾ���
		if(( p_data_buff->PrnState[0] == 0x01) && 
		  (( p_data_buff->PrnState[0] & 0x01 ) != ( rxd &0x01 ))){		// ��Ʊ���޽ð���Ǝ�MƱ���޽ð���̔�r
			//	Ʊ���޽ð�����ω������ꍇ
			p_data_buff->PrnState[2]	=	p_data_buff->PrnState[1];	// �O�X��ð���@���@�O��ð��
			p_data_buff->PrnState[1]	=	p_data_buff->PrnState[0];	// ���ð���@���@���ð��
			p_data_buff->PrnState[0]	|=	(uchar)( rxd & 0xfe );		// ���ð�����
			p_data_buff->PrnStWork		=	rxd;						// ��M�ð������
			I2C_Event_Info.BIT.R_PRINTER_NEAR_END = 1;
		}
		else{
			//	Ʊ���޽ð���ȊO���ω������ꍇ
			p_data_buff->PrnState[2]	=	p_data_buff->PrnState[1];	// �O�X��ð���@���@�O��ð��
			p_data_buff->PrnState[1]	=	p_data_buff->PrnState[0];	// ���ð���@���@���ð��
			p_data_buff->PrnState[0]	=	rxd;						// ���ð�����
			p_data_buff->PrnStWork		=	rxd;						// ��M�ð������
			I2C_Event_Info.BIT.R_PRINTER_NEAR_END = 0;
		}
	}
	p_data_buff->PrnStateRcvCnt++;
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
}
/****************************************************************/
/*	RP, JP�̏ꍇ��1Byte��Application�Ɏ󂯓n���Ƃ̎�			*/
/*		---> ��MData����������Task�֑��M����(queueset)			*/
/*	���M��Task = Device����̔񓯊���M�Ȃ̂�TaskNoTo�ɂ͂Ȃ�	*/
/****************************************************************/
void I2c_RecvNotifyFromJp_IRQ5(void)
{
	PRN_DATA_BUFF	*p_data_buff;	// ������ް��ޯ̧�߲���
	uchar			rxd = I2cAM.DevBufJp[I2cAM.CtrJp.DevRecvP-1];
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�d�q�W���[�i���ڑ��Ő��Z�@���N�����Ȃ��j
	ushort			pinfo_size;
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�d�q�W���[�i���ڑ��Ő��Z�@���N�����Ȃ��j
	
	if (I2cAM.CtrJp.DevRecvP > SIZE_DEV_JP_MAX) {
		if (I2cErrStatus.RecvJP == 0) {
			I2cErrStatus.RecvJP = 1;
			I2cErrEvent.RecvJP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_RECV_JP = 1;	// BIT11 = �W���[�i���v�����^��M�o�b�t�@�t��(�wI2C�ެ��َ�M�ޯ̧�فx)	����/������
		}
	} else {
		if (I2cErrStatus.RecvJP != 0) {
			I2cErrStatus.RecvJP = 0;
			I2cErrEvent.RecvJP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_RECV_JP = 0;	// BIT11 = �W���[�i���v�����^��M�o�b�t�@�t��(�wI2C�ެ��َ�M�ޯ̧�فx)	����/������
		}
	}
	if( PrnJnlCheck() == ON ){
		p_data_buff = (PRN_DATA_BUFF *)&jnl_prn_buff;						// ������ް��ޯ̧�߲������
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�d�q�W���[�i���ڑ��Ő��Z�@���N�����Ȃ��j
//		if (isPrnInfoReq()) {
//			// �v�����^��񉞓���M
//			if (PrnCmd_InfoRes_Proc(I2cAM.DevBufJp, I2cAM.CtrJp.DevRecvP) != 0) {
//				I2C_Event_Info.BIT.J_PRINTER_RCV_INFO = 1;
//			}
//			I2cAM.CtrJp.DevRecvP = 0;
//		}
		pinfo_size = 0;
		if (isPrnInfoReq()) {
			// �v�����^���v����
			if (isPrnInfoInitRecv()) {
				// �����M
				if (isPrnInfoRes(I2cAM.DevBufJp[0])) {
					// �v�����^��񉞓�
					pinfo_size = I2cAM.CtrJp.DevRecvP;
				}
			}
			else {
				// 2��ڈȍ~��M
				pinfo_size = I2cAM.CtrJp.DevRecvP;
			}
		}

		if (isPrnInfoReq() && pinfo_size > 0) {
			// �v�����^��񉞓���M
			if (PrnCmd_InfoRes_Proc(&I2cAM.DevBufJp[0], pinfo_size) != 0) {
				I2C_Event_Info.BIT.J_PRINTER_RCV_INFO = 1;
			}
			I2cAM.CtrJp.DevRecvP = 0;
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�d�q�W���[�i���ڑ��Ő��Z�@���N�����Ȃ��j
		else {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		I2cAM.CtrJp.DevRecvP = 0;
		if( p_data_buff->PrnStWork != rxd ){								// ���ނ��Ă���ð���Ǝ�M�ް��̔�r
			//	�ð�����ω������ꍇ
			I2C_Event_Info.BIT.J_PRINTER_RCV_CMP = 1;
			p_data_buff->PrnStateMnt = rxd;									// ��������ݽ�p�ɾ���
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			if (isEJA_USE()) {
				// �d�q�W���[�i���̃v�����^�X�e�[�^�X��M
				p_data_buff->PrnState[2]	=	p_data_buff->PrnState[1];	// �O�X��ð���@���@�O��ð��
				p_data_buff->PrnState[1]	=	p_data_buff->PrnState[0];	// ���ð���@���@���ð��
				p_data_buff->PrnState[0]	=	rxd;						// ���ð�����
				p_data_buff->PrnStWork		=	rxd;						// ��M�ð������
				I2C_Event_Info.BIT.EJA_RCV_STS = 1;
			}
			else {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
			if(( p_data_buff->PrnState[0] == 0x01 ) &&
			  (( p_data_buff->PrnState[0] & 0x01 ) != ( rxd &0x01 ))){		// ��Ʊ���޽ð���Ǝ�MƱ���޽ð���̔�r
				//	Ʊ���޽ð�����ω������ꍇ
				p_data_buff->PrnState[2]	=	p_data_buff->PrnState[1];	// �O�X��ð���@���@�O��ð��
				p_data_buff->PrnState[1]	=	p_data_buff->PrnState[0];	// ���ð���@���@���ð��
				p_data_buff->PrnState[0]	|=	(uchar)( rxd & 0xfe );		// ���ð�����
				p_data_buff->PrnStWork		=	rxd;						// ��M�ð������
				I2C_Event_Info.BIT.J_PRINTER_NEAR_END = 1;
			}
			else{
				//	Ʊ���޽ð���ȊO���ω������ꍇ
				p_data_buff->PrnState[2]	=	p_data_buff->PrnState[1];	// �O�X��ð���@���@�O��ð��
				p_data_buff->PrnState[1]	=	p_data_buff->PrnState[0];	// ���ð���@���@���ð��
				p_data_buff->PrnState[0]	=	rxd;						// ���ð�����
				p_data_buff->PrnStWork		=	rxd;						// ��M�ð������
				I2C_Event_Info.BIT.J_PRINTER_NEAR_END = 0;
			}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		}
		p_data_buff->PrnStateRcvCnt++;
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	}
}
/****************************************************************/
/*	I2cDev2_1�̏ꍇ�͈ȉ��̏������s��							*/
/*	1.	��MData���CRLF��Check�����o������2, 3�̏������s��		*/
/*	2.	��MBuffer�̐擪-CRLF�܂ł�Data�� 						*/
/*		---> I2cDev2_1��MData����������Task�֑��M����(queueset)*/
/*	3.	I2cAM.CtrI2cDev2_1.DevRecvP <- 0						*/
/*	���M��Task = Device����̔񓯊���M�Ȃ̂�TaskNoTo�ɂ͂Ȃ�	*/
/****************************************************************/
void I2c_RecvNotifyFromI2cDev2_1_IRQ8(void)
{
	//	�d�v --->	�{�֐���DevRecvP <- 0���Ȃ���UserBuffeOverFlow����̂�
	//				�{�֐��������ɂł���܂ł͈ȉ��̏�����ǉ�����
	I2cAM.CtrI2cDev2_1.DevRecvP = 0;
}
/****************************************************************/
/*																*
/*																*
/****************************************************************/
void I2c_RecvNotifyFromI2cDev2_2_IRQ8(void)
{
	//	�d�v --->	�{�֐���DevRecvP <- 0���Ȃ���UserBuffeOverFlow����̂�
	//				�{�֐��������ɂł���܂ł͈ȉ��̏�����ǉ�����
	I2cAM.CtrI2cDev2_2.DevRecvP = 0;
}
/********************************************************************************************/
/****************************************************************/
/*	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)		*/
/*	�I��Message���M��:	ReceptPrinter����Task					*/
/*	����Task�Ƃ�CommandI/F��`�͕ʓr�쐬						*/
/*	���M��Task = TaskNoTo�Ɋi�[����Ă���						*/
/****************************************************************/
void I2c_SendEndNotifyToRp_IRQ5(void)
{
	//	APL����Request����RequestCode�͈ȉ��Ɋi�[����Ă��܂��̂�
	//	����I������RequestCode�͈ȉ����Q�Ƃ���
	//	�I������Request��Write�v���Ȃ̂�Read�v���Ȃ̂����f���ĉ������B
	//	RequestCode�̒�`��I2c_driver.c��I2C_RequestCode�ōs���Ă���܂��B
	//	I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex-1) & 0x03].RequestCode
	I2C_Event_Info.BIT.R_PRINTER_SND_CMP = 1;
}
/****************************************************************/
/*	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)		*/
/*	�I��Message���M��:	JernalPrinter����Task					*/
/*	����Task�Ƃ�CommandI/F��`�͕ʓr�쐬						*/
/*	���M��Task = TaskNoTo�Ɋi�[����Ă���						*/
/****************************************************************/
void I2c_SendEndNotifyToJp_IRQ5(void)
{
	//	APL����Request����RequestCode�͈ȉ��Ɋi�[����Ă��܂��̂�
	//	����I������RequestCode�͈ȉ����Q�Ƃ���
	//	�I������Request��Write�v���Ȃ̂�Read�v���Ȃ̂����f���ĉ������B
	//	RequestCode�̒�`��I2c_driver.c��I2C_RequestCode�ōs���Ă���܂��B
	//	I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex-1) & 0x03].RequestCode
	I2C_Event_Info.BIT.J_PRINTER_SND_CMP = 1;
}
/****************************************************************/
/*	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)		*/
/*	�I��Message���M��:	I2cDev2_1����Task						*/
/*	����Task�Ƃ�CommandI/F��`�͕ʓr�쐬						*/
/*	���M��Task = TaskNoTo�Ɋi�[����Ă���						*/
/****************************************************************/
void I2c_SendEndNotifyToI2cDev2_1_IRQ8(void)
{
	//	APL����Request����RequestCode�͈ȉ��Ɋi�[����Ă��܂��̂�
	//	����I������RequestCode�͈ȉ����Q�Ƃ���
	//	�I������Request��Write�v���Ȃ̂�Read�v���Ȃ̂����f���ĉ������B
	//	RequestCode�̒�`��I2c_driver.c��I2C_RequestCode�ōs���Ă���܂��B
	unsigned	short	task = CAPPITCBNO;
	queset(task, I2CDEV2_1_EVT_I2CSNDCMP_RCV, 0, NULL);
}
/****************************************************************/
/*	Write/Read�̏I���ʒm�𓖊Y��Task�ɍs��(queueset����)		*/
/*	�I��Message���M��:	I2cDev2_2����Task						*/
/*	����Task�Ƃ�CommandI/F��`�͕ʓr�쐬						*/
/*	���M��Task = TaskNoTo�Ɋi�[����Ă���						*/
/****************************************************************/
void I2c_SendEndNotifyToI2cDev2_2_IRQ8(void)
{
	//	APL����Request����RequestCode�͈ȉ��Ɋi�[����Ă��܂��̂�
	//	����I������RequestCode�͈ȉ����Q�Ƃ���
	//	�I������Request��Write�v���Ȃ̂�Read�v���Ȃ̂����f���ĉ������B
	//	RequestCode�̒�`��I2c_driver.c��I2C_RequestCode�ōs���Ă���܂��B
}

//[]----------------------------------------------------------------------[]
///	@brief			SPI0��M���荞�ݏ���
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Excep_RSPI0_SPRI0( void )
{ 
	if(Sodiac_FROM_Get == 1){
		Rspi_from_SPRI();
	} else if(Sodiac_FROM_Get == 2){
		Sodiac_From_Rcv();
	} else {
		Sodiac_From_Rcv();
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			CMTU0_CMT0 1shot�^�C�}���荞�ݏ���(JVMA SYN�M���p ����800us)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2013/07/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Excep_CMTU0_CMT0( void )
{ 
	CP_CN_SYN = 1;				/* SYM Disnable	*/
	CMT.CMSTR0.BIT.STR0 = 0;	// CMT0 �����~
}

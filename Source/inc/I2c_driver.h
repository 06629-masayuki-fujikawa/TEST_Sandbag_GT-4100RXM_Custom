#ifndef _I2C_DRIVER_H_
#define _I2C_DRIVER_H_
/****************************************************************************/
/*																			*/
/*	definitions for I2C Aplication, Arbitration, Driver						*/
/*																			*/
/****************************************************************************/
/****************************************************************************/
/*	enum definitions														*/
/****************************************************************************/
/************************************************/
/*	To Application Return Value					*/
/*		I2C_Request()�̖߂�l					*/
/*		I2C_Arbitration_Manager���ł̂ݐݒ�		*/
/************************************************/
enum I2cResultFnc_e{
	RIIC_RSLT_OK = 0,			// ����I��
	RIIC_RSLT_BUS_BUSY,			// I2C_Bus�オ���炩�̗��R�ɂ��Busy��Q���(���M�s��)
	RIIC_RSLT_MODE_ERROR,		// ����I2C_Bus��ł�Wirte�܂���Read, ACK_Polling�����s��(���M�s��)
	RIIC_RSLT_PRM_ERROR,		// Application/ArbitrationM�����Parameter���s���ł�
	RIIC_RSLT_OVER_DATA_ERROR,	// ApplicationM�����write/read��Data����Max�ȏ�
	RIIC_RSLT_BUFF_OVER_ERROR,	// Write/Read���s���\��Buffer����t�ł�
	RIIC_RSLT_PREV_TOUT,		// �O��X�e�[�^�X�ω��҂��^�C���A�E�g
	RIIC_RSLT_MAX
};
/************************************************/
/*	RIIC1.ICCR2.BIT.BBSY�̏��					*/
/*		I2C_Arbitration_Manager���ł̂ݎg�p		*/
/************************************************/
enum RiicBusStatus_e{
	RIIC_BUS_STATUS_FREE = 0,
	RIIC_BUS_STATUS_BBSY,
	RIIC_BUS_STATUS_MAX
};
/****************************************/
/*	iic_status�ϐ��̒l					*/
/****************************************/
enum RiicStatus_e{
	RIIC_STATUS_IDLE = 0,
	RIIC_STATUS_ON_COMMUNICATION,
	RIIC_STATUS_NACK,
	RIIC_STATUS_FAILED,
	RIIC_STATUS_MAX
};
/****************************************/
/*	iic_mode�ϐ��̒l					*/
/****************************************/
enum RiicInternalMode_e{
	IIC_MODE_I2C_IDLE = 0,
	IIC_MODE_I2C_READ,
	IIC_MODE_I2C_WRITE,
	IIC_MODE_I2C_MAX
};
/****************************************/
/*	I2C_Driver���ł̂ݐݒ�/�g�p			*/
/****************************************/
enum RiicErrCode_e{
	RIIC_ERR_NONE = 0,
	RIIC_ERR_MODE_SP_INT,
	RIIC_ERR_MODE_RX_INT_I2C_WRITE,
	RIIC_ERR_MODE_RX_INT,
	RIIC_ERR_MODE_TX_INT,
	RIIC_ERR_MODE_TE_INT,
	RIIC_ERR_SCL_LOCK,
	RIIC_ERR_SDA_LOCK,
	RIIC_ERR_GEN_CLK_BBSY,
	RIIC_ERR_MAX
};
/****************************************/
/*	TaskNo From Application				*/
/****************************************/
//	--->	Task�\������(1/17) --->	��������ɓo�^����
enum I2C_TaskNo{
	I2C_TASK_NONE = 0,	// 
	I2C_TASK_IDLE,		// 
	I2C_TASK_OPE,		// 
	I2C_TASK_CNM,		// Coin-mech and Note-reader
	I2C_TASK_PRN,		// ����������: 				
	I2C_TASK_LKCOM,		// ۯ����u�Ď�����: 		
	I2C_TASK_FLA,		// �O��FROM2�Ǘ����W���[��: 
	I2C_TASK_NTNET,		// NT-NET�ʐM���W���[��: 	
	I2C_TASK_HRCV,		// hsnd_task = HSNDTSKNO, 	
	I2C_TASK_NTNETDOPA,	// NT-NET-DOPA�ʐM���W���[��
	I2C_TASK_REMOTEDL,	// ���u�_�E�����[�h: 		
	I2C_TASK_I2CDEV2_1,	// I2cDev2_1
	I2C_TASK_MAX
};
/****************************************/
/*	1.DeviceNo From Application			*/
/*	2.ArbitrationManager��ActiveDevice	*/
/****************************************/
enum I2C_DeviceNo{
	I2C_DN_NON = 0,						// 
	I2C_DN_CHECK_EVENT = I2C_DN_NON,	// Application���ŉ��炩��Event�̗L����Check���������ɔ��s
	I2C_DN_I2CDEV2_1,					// 
	I2C_DN_I2CDEV2_2,					// 
	I2C_DN_RP,							// 
	I2C_DN_JP,							// 
	I2C_DN_RTC,							// 
	I2C_DN_MAX
};
/****************************************/
/*	I2C_CONTROLS -> Staus��`			*/
/****************************************/
#define DEVICE_STS_IDLE		0	//	
#define DEVICE_STS_SENDING	1	//	Writing & Reading from SC16
#define DEVICE_STS_RECVING	2	//	Receiving from SC16(maybe not used)
/****************************************/
/*	I2C_INT_MODE��`					*/
/*		I2cNextDeviceProc()��Parameter	*/
/****************************************/
#define I2C_INT_MODE_EI		0	// I2cInterrupt�͊����݉\��Ԃł���
#define I2C_INT_MODE_DI		1	// I2cInterrupt�͊����݉\��ԂłȂ�
/************************************************************/
/*	RequestCode From Application							*/
/*		~_RCV = read~ ()�֐� = InternalRegister�����Read	*/
/*		~_SND = write~()�֐� = InternalRegister�ւ�Write	*/
/*	* 
/************************************************************/
enum I2C_RequestCode{
//	RTC							// �擪InternalAddres	Byte��
	I2C_RC_NON = 0,				// 
	I2C_RC_RTC_STATUS_RCV,		// �g��_0x1E			1
	I2C_RC_RTC_TIME_RCV,		// �g��_0x10			8
	I2C_RC_RTC_TIME_SND,		// �g��_0x10			8
	I2C_RC_RTC_FLAG_SND,		// �g��_0x1E			1
	I2C_RC_RTC_EX_FLAG_CTL_SND,	// �g��_0x1D			3
	I2C_RC_RTC_EX_SND,			// �g��_0x1D			1
	I2C_RC_RTC_CTL_SND,			// �g��_0x1F			1
	I2C_RC_RTC_WEEK_RCV,		// �g��_0x14			1
	I2C_RC_RTC_DAY_RCV,			// �g��_0x15			1
	I2C_RC_RTC_MONTH_RCV,		// �g��_0x16			1
	I2C_RC_RTC_YEAR_RCV,		// �g��_0x17			1
	I2C_RC_RTC_EX_FLAG_CTL_RCV,	// �g��_0x1D			3
	I2C_RC_RTC_EX_RCV,			// �g��_0x1D			1
	I2C_RC_RTC_CTL_RCV,			// �g��_0x1F			1
	I2C_RC_RTC_FLAG_RCV,		// �g��_0x1E			1
	I2C_RC_I2CDEV2_1_RTS_ON_SND,	// 
	I2C_RC_I2CDEV2_1_RTS_OFF_SND,	// 
	I2C_RC_I2CDEV2_1_DTR_ON_SND,	// 
	I2C_RC_I2CDEV2_1_DTR_OFF_SND,	// 
	I2C_RC_I2CDEV2_1_SIGNAL_IN_RCV,	// 
	I2C_RC_I2CDEV2_1_SND,			// THR�ւ�Write�v��
	I2C_RC_I2CDEV2_1_RCV,			// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
	I2C_RC_I2CDEV2_2_SND,			// THR�ւ�Write�v��
	I2C_RC_I2CDEV2_2_RCV,			// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
//	RP
	I2C_RC_RP_SND,				// THR�ւ�Write�v��
	I2C_RC_RP_RCV,				// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
//	JP
	I2C_RC_JP_SND,				// THR�ւ�Write�v��
	I2C_RC_JP_RCV,				// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
//	.							<------		cappi,rp,jp,rtc��I/F�����RequestCode�̕ύX�A�ǉ����s��
//	.
//	.
	I2C_RC_MAX
};
/********************************************/
/*	I2CDEV2_1 DTR, RTS SIGNAL Definitions	*/
/********************************************/
#define I2CDEV2_1_SIGNAL_OFF	0		// 
#define I2CDEV2_1_SIGNAL_ON		1		// 
/****************************************************************************/
/*	Evecute Mode definitions														*/
/****************************************************************************/
#define EXE_MODE_QUEUE		0
#define EXE_MODE_FORCE		1
/************************************************************/
/*		RTCMojule(RX-8803LC)				Slave = 0x64	*/
/************************************************************/
#define SLV_ADDR_RTC			0x64
#define SLV_ADDR_RTC_W			0x64
#define SLV_ADDR_RTC_R			0x65
/****************************************************************/
/*	ExIC1	FOMA(I2cDev2_1)	PortA	19.200bps, 	Slave = 0x92	*/
/*			AI-�W		PortB	19.200bps, 	Slave = 0x92		*/
/****************************************************************/
#define SLV_ADDR_I2CDEV2_1		0x92
#define SLV_ADDR_I2CDEV2_2		0x92
/************************************************************/
/*	ExIC2	R-Printer	PortA	19,200bps, 	Slave = 0x90	*/
/*			J-Printer	PortB	19,200bps, 	Slave = 0x90	*/
/************************************************************/
#define SLV_ADDR_RP			0x90
#define SLV_ADDR_JP			0x90
/************************************************************/
/*	Length of InternalRegsterAddress						*/
/************************************************************/
#define ADDR_LEN_I2CDEV2_1		(unsigned char)1
#define ADDR_LEN_I2CDEV2_2		(unsigned char)1
#define ADDR_LEN_RP			(unsigned char)1
#define ADDR_LEN_JP			(unsigned char)1
#define ADDR_LEN_RTC		(unsigned char)1
/************************************************************/
/*	iic_buff_prm[]��ArrayNo									*/
/************************************************************/
#define BUFF_NUM_WRITE		0
#define BUFF_NUM_READ		1
/************************************************************/
/*	SC16IS752																*/
/****************************************************************************/
/************************************************/
/*	SC16IS752 InternalRegister Definitions		*/
/************************************************/
#define SC16_THR_W	 		0x00	// transmit FIFO, write only
#define SC16_RHR_R			0x00	// receive FIFO, read oly
#define SC16_IER_RW			0x01	// interrup enable reg., R/W
#define SC16_FCR_W			0x02	// FIFO control, write only
#define SC16_IIR_R			0x02	// interrupt status, read only
#define SC16_LCR_RW			0x03	// line control, R/W
#define SC16_MCR_RW			0x04	// modem control, R/W
#define SC16_LSR_R			0x05	// Line status, R
#define SC16_MSR_R			0x06	// Modem status, R
#define SC16_SPR_RW			0x07	// scratch pad, R/W
#define SC16_TCR_RW			0x06	// Transmission_Control_Register(accessible only when SC16_EFR_EH_RW[4] = 1 and SC16_MCR_RW[2] = 1) R/W
#define SC16_TLR_RW			0x07	// Trigger_Level_Register(accessible only when SC16_EFR_EH_RW[4] = 1 and SC16_MCR_RW[2] = 1) R/W
#define SC16_TXLVL_R		0x08	// TX FIFO, R
#define SC16_RXLVL_R		0x09	// RX FIFO, R
#define SC16_IODIR_RW		0x0A	// IO Direction Control R/W
#define SC16_IOSTATE_RW		0x0B	// IO State R/W
#define SC16_IOINTENA_RW	0x0C	// IO Interrupt Mask R/W
#define SC16_IOCONTROL_RW	0x0E	// IO Control R/W
#define SC16_EFCR_RW		0x0F	//  Enhancede Function Reg R/W
/* The Special Register set is accessible only when SC16_LCR_RW[7] = logic 1 and SC16_LCR_RW is not 0xBF.  */
#define SC16_DLL_SP_RW		0x00	// baud rate divisor, LSB, R/W
#define SC16_DLH_SP_RW		0x01	// baud rate divisor, MSB, R/W
/* Enhanced_Features_Registers(SC16_EFR_EH_RW) are only accessible when SC16_LCR_RW = 0xBF. */
#define SC16_EFR_EH_RW		0x02	// enhanced register, R/W
#define SC16_XON1_EH_RW		0x04	// flow control R/W
#define SC16_XON2_EH_RW		0x05	// flow control R/W
#define SC16_XOFF1_EH_RW	0x06	// flow control R/W
#define SC16_XOFF2_EH_RW	0x07	// flow control R/W
/********************************************************/
/*	SC16IS752 InternalRegisterBit Definitions			*/
/*		*2 = when EFR[4] = EI, can be modified			*/
/*		*3 = can access, when EFR[4] = 1 & MCR[2] = 1	*/
/********************************************************/
#define BIT_CTS_INT_EI_IER	 		(0x80)	// 0/1 = DI CTS INT/EI											*2
#define BIT_RTS_INT_EI_IER	 		(0x40)	// 0/1 = DI RTS INT/EI											*2
#define BIT_XOFF_INT_EI_IER			(0x20)	// 0/1 = DI XOFF INT/EI											*2
#define BIT_SLEEP_INT_EI_IER		(0x10)	// 0/1 = DI SLEEP INT/EI										*2
#define BIT_MODEM_INT_EI_IER		(0x08)	// 0/1 = DI MODEM INT/EI
#define BIT_RCV_LINE_INT_EI_IER		(0x04)	// 0/1 = DI RCV_LINE INT/EI
#define BIT_THR_EMPTY_INT_EI_IER	(0x02)	// 0/1 = DI THR INT/EI
#define BIT_RHR_RCV_INT_EI_IER		(0x01)	// 0/1 = DI RCV INT/EI
#define BIT_RXFF_TRG_LVL_FCR		(0xc0)	// RX_FIFO trigger level(00/01/10/11 = 8/16/56/60character)
#define BIT_TXFF_TRG_LVL_FCR		(0x30)	// TX_FIFO trigger level(00/01/10/11 = 8/16/56/60character)		*2
#define BIT_TXFF_RESET_FCR			(0x04)	// 1 = clear TX_FIFO & TX_FIFO_LEVEL
#define BIT_RXFF_RESET_FCR			(0x02)	// 1 = clear RX_FIFO & RX_FIFO_LEVEL
#define BIT_FIFO_TRG_EI_FCR			(0x01)	// 0/1 = TX_FIFO & RX_FIFO DI/EI
#define BIT_MIRR_FIFO_TRG_EI_IIR	(0xc0)	// Mirror the contents of FCR[0]
#define BIT_INT_ENCODED_IIR			(0x3e)	// 5-bit encoded interrupt										*2(bit4,5)
//	Prioritylevel 	IIR[5] 	IIR[4] 	IIR[3] 	IIR[2] 	IIR[1] 	IIR[0] 	Source of the interrupt 
//	-----------------------------------------------------------------------------------
//	1 				0 		0 		0 		1 		1 		0 		Receive Line Status error 
//	2 				0 		0 		1 		1 		0 		0 		Receiver time-out interrupt 
//	2 				0 		0 		0 		1 		0 		0 		RHR interrupt 
//	3 				0 		0 		0 		0 		1 		0 		THR interrupt 
//	4 				0 		0 		0 		0 		0 		0 		modem interrupt[1] 
//	5 				1 		1 		0 		0 		0 		0 		input pin change of state[1] 
//	6 				0 		1 		0 		0 		0 		0 		received Xoff signal/special character 
//	7 				1 		0 		0 		0 		0 		0 		CTS, RTS change of state from active (LOW) to inactive (HIGH) 
#define BIT_INT_STATUS_IIR			(0x01)	// 0/1 = INT pending/not
#define BIT_DIV_LATCH_EI_LCR		(0x80)	// G/S/E(0/1/BF) Register set access Mode
#define BIT_BREAK_CTR_LCR			(0x40)	// 1/0 = force TX break output/not
#define BIT_FORCE_PARITY_LCR		(0x20)	// 1/0 = force parity bit/not
#define BIT_EVEN_PARITY_LCR			(0x10)	// 0/1 = odd/even(Pon=1)
#define BIT_PARITY_EI_LCR			(0x08)	// 1/0 = Parity EI/DI(Pon=1)
#define BIT_NUM_STOP_BIT_LCR		(0x04)	// stop bit length select(Pon=1)
#define BIT_WORD_LENGTH_LCR			(0x03)	// data length 5,6,7,8 select(Pon=01)
#define BIT_CLOCK_DIV_MCR			(0x80)	// 0/1 = Clock divisor 1clock/4					*2
#define BIT_IRDA_MODE_EI_MCR		(0x40)	// 0/1 = normal UART mode/IrDA mode				*2
#define BIT_XON_ANY_EI_MCR			(0x20)	// 0/1 = DI Xon Any function/EI					*2
#define BIT_LOOPBACK_EI_MCR			(0x10)	// 0/1 = normal operating/local Loopback
#define BIT_TCR_TLR_EI_MCR			(0x04)	// 0/1 = access DI TCR & TLR/EI					*2
#define BIT_RTS_FORCE_SET_MCR		(0x02)	// 0/1 = force RTS output(H)/(L)
#define BIT_DTR_FORCE_SET_MCR		(0x01)	// 0/1 = force DTR output(H)/(L)
#define BIT_FIFO_DATA_ERR_LSR		(0x80)	// 0/1 = no error/at least one error in the RX FIFO
#define BIT_THR_TSR_EMPTY_LSR		(0x40)	// 0/1 = THR & TSR not empty/empty(Pon=1)
#define BIT_THR_EMPTY_LSR			(0x20)	// 0/1 = THR not empty/empty(Pon=1)
#define BIT_BREAK_INT_LSR			(0x10)	// 0/1 = no/break condition occurred
#define BIT_FRAMING_ERR_LSR			(0x08)	// 0/1 = no/framing error occurred in RX FIFO
#define BIT_PARITY_ERR_LSR			(0x04)	// 0/1 = no/parity error in RX FIFO
#define BIT_OVERRUN_ERR_LSR			(0x02)	// 0/1 = no/overrun error occurred
#define BIT_DATA_RECEIVED_LSR		(0x01)	// 0/1 = no/at least one character in RX FIFO
#define BIT_PIN_STATE_CD_MSR		(0x80)	// CD_pin_status(active HIGH)(Pon=input)
#define BIT_PIN_STATE_RI_MSR		(0x40)	// RI_pin_status(active HIGH)(Pon=input)
#define BIT_PIN_STATE_DSR_MSR		(0x20)	// DSR_pin_status(active HIGH)(Pon=input)
#define BIT_PIN_STATE_CTS_MSR		(0x10)	// CTS_pin_status(active HIGH)(Pon=input)
#define BIT_PIN_CHANGE_CD_MSR		(0x08)	// 1 = CD_pin_changed(Cleared on a read)
#define BIT_PIN_CHANGE_RI_MSR		(0x04)	// 1 = RI_pin_changed(Cleared on a read)
#define BIT_PIN_CHANGE_DSR_MSR		(0x02)	// 1 = DSR_pin_changed(Cleared on a read)
#define BIT_PIN_CHANGE_CTS_MSR		(0x01)	// 1 = CTS_pin_changed(Cleared on a read)
#define BIT_RXLVL_RESUME_TCR		(0xf0)	// Receiver_FIFO_Level_register(RXLVL) to resume(TCR[3:0] > TCR[7:4])
#define BIT_RXLVL_HALT_TCR			(0x0f)	// Receiver_FIFO_Level_register(RXLVL) to halt(*3)
											// transmission TCR trigger levels are available
											// from 0 bytes to 60 characters with a granularity of four
#define BIT_RXFF_TRG_LVL_TLR		(0xf0)	// RX FIFO trigger levels (4 to 60), number of characters available(*3)
#define BIT_TXFF_TRG_LVL_TLR		(0x0f)	// TX FIFO trigger levels (4 to 60), number of spaces available
											// If TLR[3:0] or TLR[7:4] are logical 0,
											// the selectable trigger levels via the FIFO_Control_Register(FCR)
											// are used for the transmit and receive FIFO trigger levels.
#define BIT_NUM_TX_FIFO_TXLVL		(0x7f)	// number of spaces available  in TX FIFO, from 0 to 64(Pon=0x40)
#define BIT_NUM_RX_FIFO_RXLVL		(0x7f)	// number of characters stored in RX FIFO, from 0 to 64
#define BIT_GPIO0_IN_OUT_IODIR		(0x01)	// GPIO0_DSRB_pin_InOutSelect	(0/1 = in/out)
#define BIT_GPIO1_IN_OUT_IODIR		(0x02)	// GPIO1_DTRB_pin_InOutSelect	(0/1 = in/out)
#define BIT_GPIO2_IN_OUT_IODIR		(0x04)	// GPIO2_CDB_pin_InOutSelect	(0/1 = in/out)
#define BIT_GPIO3_IN_OUT_IODIR		(0x08)	// GPIO3_RIB_pin_InOutSelect	(0/1 = in/out)
#define BIT_GPIO4_IN_OUT_IODIR		(0x10)	// GPIO4_DSRA_pin_InOutSelect	(0/1 = in/out)
#define BIT_GPIO5_IN_OUT_IODIR		(0x20)	// GPIO5_DTRA_pin_InOutSelect	(0/1 = in/out)
#define BIT_GPIO6_IN_OUT_IODIR		(0x40)	// GPIO6_CDA_pin_InOutSelect	(0/1 = in/out)
#define BIT_GPIO7_IN_OUT_IODIR		(0x80)	// GPIO7_RIA_pin_InOutSelect	(0/1 = in/out)
#define BIT_GPIO0_DSRB_DATA_IOSTATE	(0x01)	// GPIO0_DSRB_pin_Status		(0/1 = L/H)
#define BIT_GPIO1_DTRB_DATA_IOSTATE	(0x02)	// GPIO1_DTRB_pin_Status		(0/1 = L/H)
#define BIT_GPIO2_CDB_DATA_IOSTATE	(0x04)	// GPIO2_CDB_pin_Status			(0/1 = L/H)
#define BIT_GPIO3_RIB_DATA_IOSTATE	(0x08)	// GPIO3_RIB_pin_Status			(0/1 = L/H)
#define BIT_GPIO4_DSRA_DATA_IOSTATE	(0x10)	// GPIO4_DSRA_pin_Status		(0/1 = L/H)
#define BIT_GPIO5_DTRA_DATA_IOSTATE	(0x20)	// GPIO5_DTRA_pin_Status		(0/1 = L/H)
#define BIT_GPIO6_CDA_DATA_IOSTATE	(0x40)	// GPIO6_CDA_pin_Status			(0/1 = L/H)
#define BIT_GPIO7_RIA_DATA_IOSTATE	(0x80)	// GPIO7_RIA_pin_Status			(0/1 = L/H)
#define BIT_GPIO0_INT_EI_IOINTENA	(0x01)	// GPIO0_DSRB_pin_ChangeIntEi	(0/1 = DI/EI)
#define BIT_GPIO1_INT_EI_IOINTENA	(0x02)	// GPIO1_DTRB_pin_ChangeIntEi	(0/1 = DI/EI)
#define BIT_GPIO2_INT_EI_IOINTENA	(0x04)	// GPIO2_CDB_pin_ChangeIntEi	(0/1 = DI/EI)
#define BIT_GPIO3_INT_EI_IOINTENA	(0x08)	// GPIO3_RIB_pin_ChangeIntEi	(0/1 = DI/EI)
#define BIT_GPIO4_INT_EI_IOINTENA	(0x10)	// GPIO4_DSRA_pin_ChangeIntEi	(0/1 = DI/EI)
#define BIT_GPIO5_INT_EI_IOINTENA	(0x20)	// GPIO5_DTRA_pin_ChangeIntEi	(0/1 = DI/EI)
#define BIT_GPIO6_INT_EI_IOINTENA	(0x40)	// GPIO6_CDA_pin_ChangeIntEi	(0/1 = DI/EI)
#define BIT_GPIO7_INT_EI_IOINTENA	(0x80)	// GPIO7_RIA_pin_ChangeIntEi	(0/1 = DI/EI)
#define BIT_SOFT_RESET_IOCONTROL	(0x08)	// 1/0 = Software Reset(Once device is reset, automatically set to 0)
#define BIT_GPIO0_3_SEL_IOCONTROL	(0x04)	// 0/1 = GPIO/RIB, CDB, DTRB, DSRB 
#define BIT_GPIO4_7_SEL_IOCONTROL	(0x02)	// 0/1 = GPIO/RIA, CDA, DTRA, DSRA 
#define BIT_IOLATCH_EI_IOCONTROL	(0x01)	// 1/0 = input values are latched/not
#define BIT_IRDA_MODE_EFCR			(0x80)	// IrDAMode	(0 = IrDA SIR, 3.16 pulse ratio, data rate up to 115.2 kbit/s)
//														(1 = IrDA SIR, 1.4 pulse ratio, data rate up to 1.152 Mbit/s[1])
#define BIT_RTS_INVERT_EFCR			(0x20)	// Invert RTS signal in RS-485 mode: 0/1 =  RTS = 0(transmissing)/RTS = 1(transmissing)
#define BIT_RTS_CTR_EI_EFCR			(0x10)	// 1/0 = Enable transmitter controls RTS pin/not
#define BIT_TX_SEND_DI_EFCR			(0x04)	// 0/1 = Disable transmitter is EI/DI
#define BIT_RX_RECV_DI_EFCR			(0x02)	// 0/1 = Disable receiver is EI/DI
#define BIT_SEL_232_485_EFCR		(0x01)	// 0/1 = normal RS-232 mode/enables RS-485 mode 
#define BIT_AUTO_CTS_CTR_EI_EFR		(0x80)	// 1/0 = CTS flow control is EI/DI = AutoCTS
											// Transmission will stop when a HIGH signal is detected on the CTS pin
#define BIT_AUTO_RTS_CTR_EI_EFR		(0x40)	// 1/0 = RTS flow control is EI/DI = AutoRTS
											// The RTS pin goes HIGH when the receiver FIFO halt trigger level TCR[3:0] is reached,
											// and goes LOW when the receiver FIFO resume transmission trigger level TCR[7:4] is reached. 
#define BIT_SPCL_CHAR_DETECT_EFR	(0x20)	// 1/0 = special character detect EI/DI
											// Received data is compared with Xoff2 data.
											// If a match occurs, the received data is transferred to FIFO and IIR[4] is set
											// to a logical 1 to indicate a special character has been detected.
#define BIT_ENHANCED_FUNC_EI_EFR	(0x10)	// 1 = EI modify to IER[7-4], FCR[5-4], and MCR[7-5]
#define BIT_SOFT_FLOW_CTR_EFR		(0x0f)	// Combinations of software flow control can be selected by programming these bits
/********************************/
#define RTC_REG_NO_EXT_10MS		0x10
#define RTC_REG_NO_EXT_SEC		0x11
#define RTC_REG_NO_EXT_MIN		0x12
#define RTC_REG_NO_EXT_HOUR		0x13
#define RTC_REG_NO_EXT_WEEK		0x14
#define RTC_REG_NO_EXT_DAY		0x15
#define RTC_REG_NO_EXT_MONTH	0x16
#define RTC_REG_NO_EXT_YEAR		0x17
#define RTC_REG_NO_EXT_MIN_A	0x18
#define RTC_REG_NO_EXT_HOUR_A	0x19
#define RTC_REG_NO_EXT_W_D_A	0x1A
#define RTC_REG_NO_EXT_TC0		0x1B
#define RTC_REG_NO_EXT_TC1		0x1C
#define RTC_REG_NO_EXT_EX		0x1D	//	TEST	WADA	USEL	TE		FSEL1	FSEL0	TSEL1	TSEL0
#define RTC_REG_NO_EXT_FLAG		0x1E	//	---		---		UF		TF		AF		EVF		VLF		VDET
#define RTC_REG_NO_EXT_CTR		0x1F	//	CSEL1	CSEL0	UIE		TIE		AIE		EIE		---		RESET

//Extension Register
//	Bit7 	TEST						0�Œ�
//	Bit6	WADA						0
//	Bit5	USEL(���v�X�V����)			1(���X�V)
//	Bit4	TE							0
//	Bit3,2	FSEL1,0						00
//	Bit1,0	TSEL1,0						00
//Flag Register
//	Bit7,6 	---							00�Œ�
//	Bit5	UF							0
//	Bit4	TF							0
//	Bit3	AF							0
//	Bit2	EVF							0
//	Bit1	VLF(���U��H�d���ቺ)		0(Clear)
//	Bit0	VDET(���x�⏞��H�d���ቺ)	0(Clear)
//Control Register
//	Bit7,6 	CSEL1,0(���x�⏞����Ԋu)	01(2.0S)
//	Bit5	UIE(���v�X�V��������Flag)	1(����)
//	Bit4	TIE(Timer��������Flag)		0(�֎~)
//	Bit3	AIE(Alarm��������Flag)		0(�֎~)
//	Bit2	EIE							0�Œ�
//	Bit1	---							0�Œ�
//	Bit0	RESET(1/100����CounterClear)0(Clear���Ȃ�)
/********************************/
/*	BaudRate Value				*/
/********************************/
#define	I2C_DEV_BAUD_RATE_L_1200	0x03		// 1200bps(N=768 L=0x00 H=0x03)
#define	I2C_DEV_BAUD_RATE_L_2400	0x80		// 2400bps(N=384 L=0x80 H=0x01)
#define	I2C_DEV_BAUD_RATE_L_4800	0xC0		// 4800bps(N=192 L=0xC0 H=0x00)
#define	I2C_DEV_BAUD_RATE_L_9600	0x60		// 9600bps(N=96 L=0x60 H=0x00)
#define	I2C_DEV_BAUD_RATE_L_19200	0x30		// 19200bps(N=48 L=0x30 H=0x00)
#define	I2C_DEV_BAUD_RATE_L_38400	0x18		// 38400bps(N=24 L=0x18 H=0x00)
#define	I2C_DEV_BAUD_RATE_L_57600	0x10		// 57600bps(N=16 L=0x10 H=0x00)
#define	I2C_DEV_BAUD_RATE_H_1200	0x03		// 1200bps(N=768 L=0x00 H=0x03)
#define	I2C_DEV_BAUD_RATE_H_2400	0x01		// 2400bps(N=384 L=0x80 H=0x01)
#define	I2C_DEV_BAUD_RATE_H_4800	0x00		// 4800bps(N=192 L=0xC0 H=0x00)
#define	I2C_DEV_BAUD_RATE_H_9600	0x00		// 9600bps(N=96 L=0x60 H=0x00)
#define	I2C_DEV_BAUD_RATE_H_19200	0x00		// 19200bps(N=48 L=0x30 H=0x00)
#define	I2C_DEV_BAUD_RATE_H_38400	0x00		// 38400bps(N=24 L=0x18 H=0x00)
#define	I2C_DEV_BAUD_RATE_H_57600	0x00		// 57600bps(N=16 L=0x10 H=0x00)

/********************************/
/*	Size of trm/rcv				*/
/********************************/
#define SIZE_TRM_I2CDEV2_1_MAX	512
#define SIZE_TRM_I2CDEV2_2_MAX	128
#define SIZE_RCV_I2CDEV2_1_MAX	512
#define SIZE_RCV_I2CDEV2_2_MAX	128
#define SIZE_TRM_RP_MAX		128
#define SIZE_TRM_JP_MAX		128
#define SIZE_RCV_RP_MAX		64
#define SIZE_RCV_JP_MAX		64
#define SIZE_TRM_RTC_MAX	16
#define SIZE_RCV_RTC_MAX	16
/********************************/
#define SIZE_APL_I2CDEV2_1_MAX	512
#define SIZE_APL_I2CDEV2_2_MAX	128
#define SIZE_APL_RP_MAX		128
#define SIZE_APL_JP_MAX		128
#define SIZE_APL_RTC_MAX	16

#define SIZE_DEV_I2CDEV2_1_MAX	512
#define SIZE_DEV_I2CDEV2_2_MAX	128
#define SIZE_DEV_RP_MAX		16
#define SIZE_DEV_JP_MAX		16
/********************************/
/*	SC16_ImageDataArrayNo		*/
/*	CH_Num Definition			*/
/********************************/
#define SC16_I2CDEV2_1		0		// IC1-PortA = CH0
#define SC16_I2CDEV2_2		1		// IC1-PortB = CH1
#define SC16_RP			2		// IC2-PortA = CH0
#define SC16_JP			3		// IC2-PortB = CH1
#define SC16_IC_CH_NUM	4		// Number
/********************************/
/*	ChannelNo for InternalRegs	*/
/********************************/
#define CH0				0		// PortA(InternalReg_bit1,2)
#define CH1				2		// PortB(InternalReg_bit1,2)
/********************************/
/*	BusFreeWait Definitions		*/
/********************************/
#define BUSFREE_IGNO	0x00000000	// 
#define BUSFREE_WAIT	0x00000001	// 
/****************************************************************************/
/*	Structure definitions													*/
/****************************************************************************/
typedef struct
{
	unsigned long 	Write;				// I2c_driver����WriteSequence(wt_1-) = DebugInfo
	unsigned long 	Read;				// I2c_driver����ReadSequence(rd_1-) = DebugInfo
	unsigned long 	Error;				// I2c_driver����ErrorSequence = DebugInfo
	unsigned long 	Wcnt;				// I2C_Write()��call������ = DebugInfo
	unsigned long 	Rcnt;				// I2C_Read()��call������ = DebugInfo
	unsigned long 	RTCcnt;				// RTC����I2c_Request()��call������ = DebugInfo
	unsigned long 	ForceReqStatus;		// 0x00 = �������M�v���Ȃ�
										// 0xff = �������M�v����
										// 0xfe = �������M����&�������f�͎�t�Ă��Ȃ�/
										// 1,2,3,4,5,6,7,8 = �������M����&�������f��
										// 0xfd = 1,2,3,4,5,6,7,8���I2cNextDeviceProc()���s�������
	unsigned short 	I2cINT_num;			// I2cInt�֘A�̔�������Event�ԍ� = DebugInfo
	unsigned char 	I2cINT[32];			// I2cInt�֘A�̔�������Event��Sequencial�Ɋi�[����Ă��� = DebugInfo
	unsigned long 	I5cnt;				// IRQ5�̔����� = DebugInfo
	unsigned long 	I5_RP_THRcnt;		// IRQ5���ł�RP�����ݗv��(LSR)��THR�̎��̐� = DebugInfo
	unsigned long 	I5_RP_RHRcnt;		// IRQ5���ł�RP�����ݗv��(LSR)��RHR�̎��̐� = DebugInfo
	unsigned long 	I5_JP_THRcnt;		// IRQ5���ł�JP�����ݗv��(LSR)��THR�̎��̐� = DebugInfo
	unsigned long 	I5_JP_RHRcnt;		// IRQ5���ł�JP�����ݗv��(LSR)��RHR�̎��̐� = DebugInfo
	unsigned char 	I5_RP_RHRbuf[16];	// IRQ5���ł�RP�����ݗv��(LSR)��RHR�̎��̎�MData�̓��e = DebugInfo
	unsigned char 	I5_JP_RHRbuf[16];	// IRQ5���ł�JP�����ݗv��(LSR)��RHR�̎��̎�MData�̓��e = DebugInfo
	unsigned long 	I8cnt;				// IRQ8�̔����� = DebugInfo
	unsigned long 	I8_I2CDEV2_1_THRcnt;	// IRQ8���ł�I2CDEV2_1�����ݗv��(LSR)��THR�̎��̐� = DebugInfo
	unsigned long 	I8_I2CDEV2_1_RHRcnt;	// IRQ8���ł�I2CDEV2_1�����ݗv��(LSR)��RHR�̎��̐� = DebugInfo
	unsigned long 	RP_THR_HistCnt;		// I2C_Write()���ł�RP��THR��Write���s������ = DebugInfo
	unsigned char 	RP_THR_History[256];// I2C_Write()���ł�RP��THR��Write���s����Data�̓��e = DebugInfo
	unsigned long 	JP_THR_HistCnt;		// I2C_Write()���ł�JP��THR��Write���s������ = DebugInfo
	unsigned char 	JP_THR_History[256];// I2C_Write()���ł�JP��THR��Write���s����Data�̓��e = DebugInfo
	unsigned short 	I2cSP_Sending;		// 
//	unsigned short 	debugSP1_Cnt;
//	unsigned short 	debugSP1_SetBuff[256];
//	unsigned short 	debugSP1_ToutBuff[256];
//	unsigned short 	debugSP1_IntBuff[256];
//	unsigned short 	debugSP1_ToutOccer;
} IIC_MILESTONE;
/****************************************************************************/
/*	Structure definitions													*/
/****************************************************************************/
typedef struct
{
	unsigned char	SlvAdr;		// SLV_ADDR_RTC, SLV_ADDR_I2CDEV2_1, SLV_ADDR_I2CDEV2_2, SLV_ADDR_RP, SLV_ADDR_JP
	unsigned char	PreCnt;		// ADDR_LEN_RTC, ADDR_LEN_I2CDEV2_1, ADDR_LEN_I2CDEV2_2, ADDR_LEN_RP, ADDR_LEN_JP
	unsigned char	*pPreData;	// rcv_adr_RTC, rcv_adr_I2CDEV2_1, rcv_adr_I2CDEV2_2, rcv_adr_RP, rcv_adr_JP
								// trm_adr_RTC, trm_adr_I2CDEV2_1, trm_adr_I2CDEV2_2, trm_adr_RP, trm_adr_JP
	unsigned long	RWCnt;		// Number of Data
 	unsigned char	*pRWData;	// rcv_buff_RTC, rcv_buff_I2CDEV2_1, rcv_buff_I2CDEV2_2, rcv_buff_RP, rcv_buff_JP
								// trm_buff_RTC, trm_buff_I2CDEV2_1, trm_buff_I2CDEV2_2, trm_buff_RP, trm_buff_JP
} I2C_MESSAGE;
/****************************************/
/*	I2C_Driver_Info From Application	*/
/****************************************/
typedef struct
{
	unsigned long	RWCnt;			// Write/Read����Byte��
	unsigned char	*pRWData;		// Write/Read����Buffer�ւ�Pointer
} I2C_REQ_DATA_INFO;
/****************************************/
/*	I2C_Driver_Info From Application	*/
/****************************************/
typedef struct
{
	unsigned char 		TaskNoTo;			// I2C_TaskNo = I2C_Request�����������������Ɋ����ʒm����TaskNo(*9)
	unsigned char 		DeviceNo;			// I2C_DeviceNo = DeviceNo From Application
	unsigned char 		RequestCode;		// I2C_RequestCode = RequestCode From Application
	I2C_REQ_DATA_INFO	I2cReqDataInfo;		// I2C_Request Data_Info From Application
} I2C_REQUEST;
//	*9:	I2C_TASK_NONE���w�肳�ꂽ���͏����������Ɋ����ʒm�͍s���܂���B
/****************************************/
/*	I2C_Active_ControlInfo 				*/
/****************************************/
typedef struct
{
	unsigned char 		Status;				// None, Sending(ApplicationRequest), Receiving(DeviceRequest)
//	unsigned char 		SreqInRecv;			// Receiving(DeviceRequest)����Sending(ApplicationRequest)����
//	unsigned char 		RreqInSend;			// Sending(ApplicationRequest)����Receiving(DeviceRequest)����
	unsigned short 		AplWriteP;			// +1��, AplWriteP >= SIZE_APL_xxx_MAX*4 ---> AplWriteP<-0
	unsigned short 		AplReadP;			// +1��, AplReadP  >= SIZE_APL_xxx_MAX*4 ---> AplReadP<-0
	unsigned char 		AplRegistedNum;		// queueing = +1, 1Data�I�� = -1(max=4), max�̎�Apl����Request��������j������
	unsigned char 		AplRegistBoxIndex;	// Current��Box�ɓo�^���鎞��[AplRegistBoxIndex]
											// APL���瑗�M�\��o�^��������++����(-�͂��Ȃ�)
	unsigned char 		AplActiveBoxIndex;	// Current��Box��Access���鎞��[AplActiveBoxIndex]
											// �V���M�������s������++����(-�͂��Ȃ�)
	I2C_REQUEST 		AplReqBox[4];		// [] = AplRegistBoxIndex
	unsigned long 		AplActiveRWCnt;		// compare with AplReqBox[0-3].I2cReqDataInfo.RWCnt(1Data��AplActiveBoxIndex�ɑ΂���RWcnt)
											// Incriment = ���M�������ɍs��
	unsigned short 		DevRecvP;			// DevBufxxx��ReceivePointer(128byte��RingBufferPointer)
											// �{Layer�ł�Command��͍͂s��Ȃ��̂�1byte��M���ɓ��Y��Task��queueset����
	unsigned char 		SendData[2];		// writeRegSC16/readRegSC16
} I2C_CONTROLS;
/****************************************/
/*	I2C_Arbitration_ManagerInfo			*/
/****************************************/
typedef struct
{
	unsigned char 		ActiveDevice;						// I2C_DeviceNo
	unsigned char 		I2c_ProcStatus;						// I2C_ProcessStatus 0229
	unsigned char 		ForceRequest;						// �������MRequest(0xff -> 0xfe, 1,2,3,4,5,6,7,8)
	unsigned char 		NextDeviceProcNo;					// �������M��̑��M�ĊJ/�V���MCode(0xfe, 1,2,3,4,5,6,7,8)
	unsigned char 		AplBufI2cDev2_1[SIZE_APL_I2CDEV2_1_MAX * 4];	// W/R_RequestFromAPL
	unsigned char 		AplBufI2cDev2_2[SIZE_APL_I2CDEV2_2_MAX * 4];	// W/R_RequestFromAPL
	unsigned char 		AplBufRp[SIZE_APL_RP_MAX * 4];		// W/R_RequestFromAPL
	unsigned char 		AplBufJp[SIZE_APL_JP_MAX * 4];		// W/R_RequestFromAPL
	unsigned char 		AplBufRtc[SIZE_APL_RTC_MAX * 4];	// W/R_RequestFromAPL
	I2C_CONTROLS 		CtrI2cDev2_1;						// 
	I2C_CONTROLS 		CtrI2cDev2_2;						// 
	I2C_CONTROLS 		CtrRp;								// 
	I2C_CONTROLS 		CtrJp;								// 
	I2C_CONTROLS 		CtrRtc;								// 
	unsigned char 		DevBufI2cDev2_1[SIZE_DEV_I2CDEV2_1_MAX];		// RHR_RequestFromDEV
	unsigned char 		DevBufI2cDev2_2[SIZE_DEV_I2CDEV2_2_MAX];		// W/R_RequestFromDEV
	unsigned char 		DevBufRp[SIZE_DEV_RP_MAX];			// W/R_RequestFromDEV
	unsigned char 		DevBufJp[SIZE_DEV_JP_MAX];			// W/R_RequestFromDEV
} I2C_ARBITRATION_MANAGER;
/********************************************************/
typedef struct
{
	unsigned char 		AplDtrRtsData;		// 
	unsigned char 		AplSignalIn;		// CD:7, RI:6, DSR:5, CTS:4, -, -, -, -
} I2C_I2CDEV2_1_SIG_REQ;
/********************************************************/
/*	*2 = when sc16_EFR_EH_RW[BIT_ENHANCED_FUNC_EI_EFR:4]*/
/*			 = EI, can be modified						*/
/*		CTS_INT_EI_IER(0x80)	// *2					*/
/*		RTS_INT_EI_IER(0x40)	// *2					*/
/*		XOFF_INT_EI_IER(0x20)	// *2					*/
/*		SLEEP_INT_EI_IER(0x10)	// *2					*/
/*		TXFF_TRG_LVL_FCR(0x30)	// *2					*/
/*		INT_ENCODED_IIR(0x3e)	// *2(bit4,5)			*/
/*		CLOCK_DIV_MCR(0x80)		// *2					*/
/*		IRDA_MODE_EI_MCR(0x40)	// *2					*/
/*		XON_ANY_EI_MCR(0x20)	// *2					*/
/*		TCR_TLR_EI_MCR(0x04)	// *2					*/
/********************************************************/
typedef struct
{
	unsigned char	sc16_THR_W;	 		// transmit FIFO, write only
	unsigned char	sc16_RHR_R;			// receive FIFO, read oly
	unsigned char	sc16_IER_RW;		// interrup enable reg., R/W
	unsigned char	sc16_FCR_W;			// FIFO control, write only
	unsigned char	sc16_IIR_R;			// interrupt status, read only
	unsigned char	sc16_LCR_RW;		// line control, R/W
	unsigned char	sc16_MCR_RW;		// modem control, R/W
	unsigned char	sc16_LSR_R;			// Line status, R
	unsigned char	sc16_MSR_R;			// Modem status, R
	unsigned char	sc16_SPR_RW;		// User Temporary Register R/W
/* TCR, TLR = 	sc16_EFR_EH_RW[BIT_ENHANCED_FUNC_EI_EFR:4] = 1 and		*/
/*				sc16_MCR_RW[BIT_TCR_TLR_EI_MCR:2] = 1, accessable		*/
	unsigned char	sc16_TCR_RW;		// Transmission_Control_Register(accessible only when sc16_EFR_EH_RW[4] = 1 and sc16_MCR_RW[2] = 1) R/W
	unsigned char	sc16_TLR_RW;		// Trigger_Level_Register(accessible only when sc16_EFR_EH_RW[4] = 1 and sc16_MCR_RW[2] = 1) R/W
	unsigned char	sc16_TXLVL_R;		// TX FIFO, R
	unsigned char	sc16_RXLVL_R;		// RX FIFO, R
	unsigned char	sc16_IODIR_RW;		// IO Direction Control R/W
	unsigned char	sc16_IOSTATE_RW;	// IO State R/W
	unsigned char	sc16_IOINTENA_RW;	// IO Interrupt Mask R/W
	unsigned char	sc16_IOCONTROL_RW;	// IO Control R/W
	unsigned char	sc16_EFCR_RW;		//  Enhancede Function Reg R/W
/* accessible only when sc16_LCR_RW[BIT_DIV_LATCH_EI_LCR:7] = 1 and sc16_LCR_RW is not 0xBF.  */
	unsigned char	sc16_DLL_SP_RW;		// baud rate divisor, LSB, R/W
	unsigned char	sc16_DLH_SP_RW;		// baud rate divisor, MSB, R/W
/* only accessible when sc16_LCR_RW = 0xBF. */
	unsigned char	sc16_EFR_EH_RW;		// enhanced register, R/W
	unsigned char	sc16_XON1_EH_RW;	// flow control R/W
	unsigned char	sc16_XON2_EH_RW;	// flow control R/W
	unsigned char	sc16_XOFF1_EH_RW;	// flow control R/W
	unsigned char	sc16_XOFF2_EH_RW;	// flow control R/W
} I2C_SC16_REGS;
/************************************************/
#endif	// _I2C_DRIVER_H_

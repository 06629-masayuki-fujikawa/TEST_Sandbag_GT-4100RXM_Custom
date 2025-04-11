//	Pending
/****************************************************************************************/
/*																						*/
/*			I2C_Request																	*/
/*		 		I2C_RequestForce														*/
/*		 		I2C_RequestQueue														*/
/*		 		I2C_RequestQueueI2cDev2_1												*/
/*		 		I2C_RequestQueueI2cDev2_2												*/
/*		 		I2C_RequestQueueRp														*/
/*		 		I2C_RequestQueueJp														*/
/*		 		I2C_RequestQueueRtc														*/
/*			writeRegSC16																*/
/*			readRegSC16																	*/
/*			writeRegRTC																	*/
/*			readRegRTC																	*/
/*			writeControl_I2C															*/
/*			readControl_I2C																*/
/*			AppliI2cAckPolling															*/
/*			I2C_Write																	*/
/*			I2C_Read																	*/
/*			I2C_GetStatus																*/
/*			di_I2cArbitration															*/
/*			ei_I2cArbitration															*/
/*																						*/
/****************************************************************************************/
#include	<stdbool.h>
#include 	"iodefine.h"
#include	"extern_Debugdata.h"
#include 	"extern_I2Cdata.h"	// I2c_driver.h is included in extern_I2Cdata.h already
#include	"system.h"
/****************************************************************************/
/*																			*/
/*	Portal Function of I2C_Arbitration_Manager Layer						*/
/*																			*/
/****************************************************************************/
/****************************/
/*	ProtoType Definitions	*/
/****************************/
enum I2cResultFnc_e 	I2cI2cDev2_1DtrControl(int OnOff);
enum I2cResultFnc_e 	I2cI2cDev2_1RtsControl(int OnOff);
enum I2cResultFnc_e 	I2cI2cDev2_1SignalInput(void);
enum I2cResultFnc_e 	I2C_Request(I2C_REQUEST *pRequest, unsigned char ExeMode);
enum I2cResultFnc_e 	I2C_RequestForce(I2C_REQUEST *pRequest);
enum I2cResultFnc_e 	I2C_RequestQueue(I2C_REQUEST *pRequest);
enum I2cResultFnc_e 	I2C_RequestQueueI2cDev2_1(I2C_REQUEST *pRequest);
enum I2cResultFnc_e 	I2C_RequestQueueI2cDev2_2(I2C_REQUEST *pRequest);
enum I2cResultFnc_e 	I2C_RequestQueueRp(I2C_REQUEST *pRequest);
enum I2cResultFnc_e 	I2C_RequestQueueJp(I2C_REQUEST *pRequest);
enum I2cResultFnc_e 	I2C_RequestQueueRtc(I2C_REQUEST *pRequest);
enum I2cResultFnc_e 	writeRegSC16(unsigned char,	unsigned char, unsigned long, unsigned char *,  unsigned long);
enum I2cResultFnc_e 	readRegSC16 (unsigned char, unsigned char, unsigned long, unsigned char *, unsigned long);
enum I2cResultFnc_e 	writeRegRTC (unsigned char, unsigned long, unsigned char *, unsigned long);
enum I2cResultFnc_e 	readRegRTC  (unsigned char, unsigned long, unsigned char *, unsigned long);
enum I2cResultFnc_e 	writeControl_I2C(I2C_MESSAGE *, unsigned long);
enum I2cResultFnc_e 	readControl_I2C (I2C_MESSAGE *, unsigned long);
enum I2cResultFnc_e 	I2C_Write(I2C_MESSAGE);
enum I2cResultFnc_e 	I2C_Read(I2C_MESSAGE);
void I2C_GetStatus(	enum RiicStatus_e *, enum RiicBusStatus_e *);
bool AppliI2cAckPolling(unsigned char, unsigned char, unsigned long);
void I2c_SendResume_IRQ5_8(int DeviceNo, unsigned long I2cIntMode);
void I2c_SendNewstart_Start_IRQ5_8(int DeviceNo, unsigned long I2cIntMode);
unsigned char	I2cNextDeviceReNewStartCheck(int DeviceNo);
unsigned char	I2cNextDeviceCheckI2cDev2_1(void);
unsigned char	I2cNextDeviceCheckI2cDev2_2(void);
unsigned char	I2cNextDeviceCheckRp(void);
unsigned char	I2cNextDeviceCheckJp(void);
void I2cNextDeviceProc(unsigned char NextDeviceProcNo, unsigned long I2cIntMode);
void di_I2cArbitration(void);
void ei_I2cArbitration(void);
void I2cSendErrDataInit(unsigned char type);
#define	I2C_TOTAL_WAIT_CNT	30000	// 3�b
#define	I2C_ONE_WAIT		5		// 10us

/************************************************************************/
/*		Call From I2cDev2_1Modem_Control_Layer							*/
/************************************************************************/
enum I2cResultFnc_e 	I2cI2cDev2_1DtrControl(int OnOff)
{
	I2C_REQUEST 		request;
	enum I2cResultFnc_e	ret;

	request.TaskNoTo				= I2C_TASK_NONE;
	request.DeviceNo 	 			= I2C_DN_I2CDEV2_1;
	request.I2cReqDataInfo.RWCnt	= 1;
	request.I2cReqDataInfo.pRWData	= &I2cDev2_1Signal.AplDtrRtsData;	// WriteDataAddress
	if (OnOff == I2CDEV2_1_SIGNAL_OFF) {
		I2cDev2_1Signal.AplDtrRtsData &= 0xfe;	// 0 = OutputSignal=H(inactive)
		request.RequestCode	= I2C_RC_I2CDEV2_1_DTR_OFF_SND;
	// I2CDEV2_1_SIGNAL_ON
	} else {
		I2cDev2_1Signal.AplDtrRtsData |= 0x01;	// 1 = OutputSignal=L(active)
		request.RequestCode	= I2C_RC_I2CDEV2_1_DTR_ON_SND;
	}
	/************************************************************************************/
	/*	�{���M��IC1_I2CDEV2_1��SC16_THR_W�ւ�Write�łȂ��̂�IRQ5�͔������Ȃ��Ǝv���܂�	*/
	/*	���炩��Device��Queue���삵�Ă��Ȃ����͖{call��FORCE�ł�QUEUE�ł�				*/
	/*	�{call�ő��M�������s���K�v������܂��B											*/
	/*	�A��.....																		*/
	/************************************************************************************/
	ret = I2C_Request(&request, EXE_MODE_FORCE);	// EXE_MODE_QUEUE
	return(ret);
}
/************************************************************************/
/*		Call From I2cDev2_1Modem_Control_Layer							*/
/************************************************************************/
enum I2cResultFnc_e 	I2cI2cDev2_1RtsControl(int OnOff)
{
	I2C_REQUEST 		request;
	enum I2cResultFnc_e	ret;

	request.TaskNoTo				= I2C_TASK_NONE;
	request.DeviceNo 	 			= I2C_DN_I2CDEV2_1;
	request.I2cReqDataInfo.RWCnt	= 1;
	request.I2cReqDataInfo.pRWData	= &I2cDev2_1Signal.AplDtrRtsData;	// WriteDataAddress
	if (OnOff == I2CDEV2_1_SIGNAL_OFF) {
		I2cDev2_1Signal.AplDtrRtsData &= 0xfd;	// 0 = OutputSignal=H(inactive)
		request.RequestCode	= I2C_RC_I2CDEV2_1_RTS_OFF_SND;
	// I2CDEV2_1_SIGNAL_ON
	} else {
		I2cDev2_1Signal.AplDtrRtsData |= 0x02;	// 1 = OutputSignal=L(active)
		request.RequestCode	= I2C_RC_I2CDEV2_1_RTS_ON_SND;
	}
	/************************************************************************************/
	/*	�{���M��IC1_I2CDEV2_1��SC16_THR_W�ւ�Write�łȂ��̂�IRQ5�͔������Ȃ��Ǝv���܂�	*/
	/*	���炩��Device��Queue���삵�Ă��Ȃ����͖{call��FORCE�ł�QUEUE�ł�				*/
	/*	�{call�ő��M�������s���K�v������܂��B											*/
	/*	�A��.....																		*/
	/************************************************************************************/
	ret = I2C_Request(&request, EXE_MODE_FORCE);	// EXE_MODE_QUEUE
	return(ret);
}
/************************************************************************/
/*		Call From I2cDev2_1Modem_Control_Layer							*/
/************************************************************************/
enum I2cResultFnc_e 	I2cI2cDev2_1SignalInput(void)
{
	I2C_REQUEST 		request;
	enum I2cResultFnc_e	ret;

	request.TaskNoTo				= I2C_TASK_NONE;
	request.DeviceNo 	 			= I2C_DN_I2CDEV2_1;
	request.I2cReqDataInfo.RWCnt	= 1;
	request.I2cReqDataInfo.pRWData	= &I2cDev2_1Signal.AplSignalIn;		// CD:7, RI:6, DSR:5, CTS:4, ��, ��, ��, ��
	request.RequestCode	= I2C_RC_I2CDEV2_1_SIGNAL_IN_RCV;
	/************************************************************************************/
	/*	�{���M��IC1_I2CDEV2_1��SC16_THR_W�ւ�Write�łȂ��̂�IRQ5�͔������Ȃ��Ǝv���܂�	*/
	/*	���炩��Device��Queue���삵�Ă��Ȃ����͖{call��FORCE�ł�QUEUE�ł�				*/
	/*	�{call�ő��M�������s���K�v������܂��B											*/
	/*	�A��.....																		*/
	/************************************************************************************/
	ret = I2C_Request( &request, EXE_MODE_FORCE );	// EXE_MODE_QUEUE
	// ��ʂ̔���_���ɍ��킹��ׂ�bit���𔽓]������
	I2cDev2_1Signal.AplSignalIn = ~I2cDev2_1Signal.AplSignalIn;
	return(ret);
}
/********************************************************************************************************/
/*		Call From Application Layer																		*/
/*		call�� = IRQ4, ......																			*/
/*	enum I2cResultFnc_e{																				*/
/*	RIIC_RSLT_OK = 0,			// ����I��																*/
/*	RIIC_RSLT_BUS_BUSY,			// I2C_Bus�オ���炩�̗��R�ɂ��Busy��Q���(���M�s��)					*/
/*	RIIC_RSLT_MODE_ERROR,		// ����I2C_Bus��ł�Wirte�܂���Read, ACK_Polling�����s��				*/
/*	RIIC_RSLT_PRM_ERROR,		// Application/ArbitrationM�����Parameter���s���ł�					*/
/*	RIIC_RSLT_OVER_DATA_ERROR,	// ApplicationM�����write/read��Data����Max�ȏ�						*/
/*	RIIC_RSLT_BUFF_OVER_ERROR,	// Write/Read���s���\��Buffer����t�ł�									*/
/*	RIIC_RSLT_PREV_TOUT,		// �O��X�e�[�^�X�ω��҂��^�C���A�E�g									*/
/*	RIIC_RSLT_MAX																						*/
/*	};																									*/
/*		--->	RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR�ȊO = Error�o�^���s��	*/
/*		Error��� = I2C_Event_Info��Bit�o�^����(�V�^�t���b�vNT-NET ErrorList_Rev110.xls)				*/
/*					IRQ5 -> I2C_Event_Analize()�֎󂯓n��												*/
/*					I2C_Event_Analize()����																*/
/*					err_chk(Error���, ErrorCode, ����/�폜/�o�^, 0, 0)��call							*/
/********************************************************************************************************/
enum I2cResultFnc_e 	I2C_Request(I2C_REQUEST *pRequest, unsigned char ExeMode)
{
	enum I2cResultFnc_e 	ret;
	/****************************/
	/*	InputParameter��Check	*/
	/****************************/
	if (pRequest->DeviceNo == I2C_DN_NON) 		{	ret = RIIC_RSLT_PRM_ERROR;	return(ret);	}
	if (pRequest->RequestCode == I2C_RC_NON) 	{	ret = RIIC_RSLT_PRM_ERROR;	return(ret);	}
	if (pRequest->I2cReqDataInfo.RWCnt == 0) 	{	ret = RIIC_RSLT_PRM_ERROR;	return(ret);	}
	if (pRequest->I2cReqDataInfo.pRWData == 0) 	{	ret = RIIC_RSLT_PRM_ERROR;	return(ret);	}
	/********************************/
	/*	ExeMode = EXE_MODE_FORCE	*/
	/*	1.	RTC�͑S��FORCE			*/
	/*	2.	I2cDev2_1Signal�͑S��FORCE	*/
	/********************************/
	if (ExeMode == EXE_MODE_FORCE) {
		ret = I2C_RequestForce(pRequest);
	/********************************/
	/*	ExeMode = EXE_MODE_QUEUE	*/
	/********************************/
	} else {
	/********************************************************************/
	/*	RTC�̏ꍇ��Priority������, �܂�Write/Read�̏I�������݂�������	*/
	/*	�I�����肵��queset()���s�������ł��Ȃ��̂ő����ɍs���B			*/
	/********************************************************************/
		if (pRequest->DeviceNo == I2C_DN_RTC) {
			ret = I2C_RequestForce(pRequest);
		} else {
			ret = I2C_RequestQueue(pRequest);
		}
	}
	return(ret);
}
/************************************************************************/
/*																		*/
/*	especialy used for Initialize										*/
/*	ExeMode = EXE_MODE_FORCE											*/
/*		1.	RTC�͑S��FORCE												*/
/*		2.	I2cDev2_1Signal�͑S��FORCE										*/
/*																		*/
/************************************************************************/
enum I2cResultFnc_e 	I2C_RequestForce(I2C_REQUEST *pRequest)
{
	enum I2cResultFnc_e 	ret;
	unsigned long			loopcnt;
	unsigned long			RWcnt;
	unsigned char			*pRWData;

	ret = RIIC_RSLT_OK;
	/********************************************************************************/
	/*	1.	ActiveDevice == I2C_DN_NON�ł����I2cBusFree��Ԃł���					*/
	/*		���M�\���Device��1���Ȃ��̂Ŗ������ő��M�������s����					*/
	/*		(---> FORCE�ő��M����Data�͑S��IRQ5, 8�̊����݂͂�����Ȃ�)				*/
	/*																				*/
	/*	2.	�A��ActiveDevice != I2C_DN_NON�ł���΂��Âꂩ��Device��				*/
	/*		QueueData�̑��M���s���Ă���̂�IRQ5����IRQ8��							*/
	/*		���ݑ��M����QueueData_1Byte���I����������ForceRequest==0xff�Ȃ�			*/
	/*		�������M�v���L��ƔF����ForceRequest<-1,2,3,4,5,6,7,8����				*/
	/*		(I2cNextDeviceProc�֐���call���鎞��Parameter: 0xfe�̎��͍s��Ȃ�)		*/
	/*		�������f��t�Ƃ�Device�̑��M�ĊJ����/�V���M�������s�킸��				*/
	/*		�{�ӏ��ŋ������M�J�n���s��(see ---> �������f��)							*/
	/*		�A��IRQ5����IRQ8�ő��M����1Byte��Data���ŏI�ňȌ�						*/
	/*		���M�\�񂪉����Ȃ����ForceRequest<-0xfe�Ƃ���							*/
	/*		(�������M���� & �������f�͎�t�Ă��Ȃ�)									*/
	/*																				*/
	/*		�������f��:	�������M���Restart�����ɂ��Ă�							*/
	/*					�������M�������I�����ċ������f��t���Ă����ꍇ�ɂ�			*/
	/*					Device�̑��M�ĊJ����/�V���M�������s��Ȃ���΂����Ȃ�		*/
	/*					�̂�I2cNextDeviceProc()�֐���call����						*/
	/*																				*/
	/*	3.	10ms����I2C_RC_I2CDEV2_1_SIGNAL_IN_RCV��								*/
	/*		1Min����IRQ4�����Request�ɂ���										*/
	/*																				*/
	/*																				*/
	/********************************************************************************/
	I2cAM.ForceRequest = 0;
	I2cAM.NextDeviceProcNo = 0;


	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		I2cAM.ForceRequest = 0xff;
 		IicMilestone.ForceReqStatus = 0xff;	// 0xff = �������M�v����
		/********************************************************/
		/*	IRQ5����IRQ8�Ō��ݑ��M����QueueData_1Byte���I������	*/
		/*	ForceRequest��F�������̂ŋ������M�J�n��			*/
		/********************************************************/
///////////////////////////////////////////////////////
		loopcnt = LifeTimGet();
		do{
			WACDOG;
			if( 50 <= LifePastTimGet( loopcnt ) ){
				I2cSendErrDataInit(I2cAM.ActiveDevice);
				I2cAM.ForceRequest = 0;
 				IicMilestone.ForceReqStatus = 0;
				break;
			}
		}while(I2cAM.ForceRequest == 0xff);
		/********************************************************************/
		/*	NextDeviceProcNo = 1,2=I2cDev2_1, 3,4=I2cDev2_2, 5,6=RP, 7,8=JP	*/
		/*	0xfe = �������M���� & �������f�͎�t�Ă��Ȃ�					*/
		/********************************************************************/
//////////////////////////////////////////////////////////////
//		�������M�Ŕ�������Timeout�͉ғ�����Device�̒ʐM�s�ǂƂ��ēo�^���ꂽ�̂�
//		���̑��M�ΏۂƂ͂�����Request���ꂽ�������M�𑦎��ɍs�����Ƃ���
		if (I2cAM.ForceRequest != 0) {
			I2cAM.NextDeviceProcNo = I2cAM.ForceRequest;
			IicMilestone.ForceReqStatus = I2cAM.ForceRequest;
			I2cAM.ForceRequest = 0;
		}
	}
	/************************************************************************/
	/*	RP, JP�̏ꍇ��I2c_Reuest()��Main����Call����邪					*/
	/*	RTC�̏ꍇ��IRQ4����Call����闝�R��IRQ5,8��Interrupt���ꂽ����		*/
	/*	Resource�ی�ׂ̈�IRQ4, IRQ5, IRQ8�̂�DI��I2cInt�͐������Ă���		*/
	/************************************************************************/
	di_I2cArbitration();
	/****************************/
	/*	Start of Device Access	*/
	/****************************/
	RWcnt 	= pRequest->I2cReqDataInfo.RWCnt;
	pRWData	= pRequest->I2cReqDataInfo.pRWData;
	//	
	//	------->	RequestCode��Device���ɈقȂ�̂�switch�Őؑւ���Logic���쐬����
	//	
	switch(pRequest->DeviceNo) {
		case I2C_DN_I2CDEV2_1:
			switch(pRequest->RequestCode) {
////////////////////////////////////////////////////////
//	SCI�ւ�Data����M��Force�͂Ȃ���DeBug�p�ɗL���Ƃ���
			case I2C_RC_I2CDEV2_1_SND:
				for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
					ret = writeRegSC16(SC16_I2CDEV2_1, SC16_THR_W, 1, pRWData, BUSFREE_WAIT);
					if (ret != RIIC_RSLT_OK)	{	break;	}
					++pRWData;
				}
				break;
		case I2C_RC_I2CDEV2_1_RCV:
				for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
					ret = readRegSC16(SC16_I2CDEV2_1, SC16_RHR_R, 1, pRWData, BUSFREE_WAIT);
					if (ret != RIIC_RSLT_OK)	{	break;	}
					++pRWData;
				}
				break;
////////////////////////////////////////////////////////
			case I2C_RC_I2CDEV2_1_RTS_ON_SND:
			case I2C_RC_I2CDEV2_1_RTS_OFF_SND:
			case I2C_RC_I2CDEV2_1_DTR_ON_SND:
			case I2C_RC_I2CDEV2_1_DTR_OFF_SND:
				ret = writeRegSC16(	SC16_I2CDEV2_1, SC16_MCR_RW, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_I2CDEV2_1_SIGNAL_IN_RCV:
				ret = readRegSC16(SC16_I2CDEV2_1, SC16_MSR_R, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			default:
				ret = RIIC_RSLT_PRM_ERROR;
				break;
			}
			break;
		case I2C_DN_I2CDEV2_2:
////////////////////////////////////////////////////////
//	SCI�ւ�Data����M��Force�͂Ȃ���DeBug�p�ɗL���Ƃ���
			switch(pRequest->RequestCode) {
			case I2C_RC_I2CDEV2_2_SND:
				for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
					ret = writeRegSC16(SC16_I2CDEV2_2, SC16_THR_W, 1, pRWData, BUSFREE_WAIT);
					if (ret != RIIC_RSLT_OK)	{	break;	}
					++pRWData;
				}
				break;
			case I2C_RC_I2CDEV2_2_RCV:
				for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
					ret = readRegSC16(SC16_I2CDEV2_2, SC16_RHR_R, 1, pRWData, BUSFREE_WAIT);
					if (ret != RIIC_RSLT_OK)	{	break;	}
					++pRWData;
				}
				break;
			default:
				ret = RIIC_RSLT_PRM_ERROR;
				break;
			}
////////////////////////////////////////////////////////
			break;
		case I2C_DN_RP:
////////////////////////////////////////////////////////
//	SCI�ւ�Data����M��Force�͂Ȃ���DeBug�p�ɗL���Ƃ���
			switch(pRequest->RequestCode) {
			case I2C_RC_RP_SND:
				for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
					ret = writeRegSC16(SC16_RP, SC16_THR_W, 1, pRWData, BUSFREE_WAIT);
					if (ret != RIIC_RSLT_OK)	{	break;	}
					++pRWData;
				}
				break;
			case I2C_RC_RP_RCV:
				for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
					ret = readRegSC16(SC16_RP, SC16_RHR_R, 1, pRWData, BUSFREE_WAIT);
					if (ret != RIIC_RSLT_OK)	{	break;	}
					++pRWData;
				}
				break;
			default:
				ret = RIIC_RSLT_PRM_ERROR;
				break;
			}
////////////////////////////////////////////////////////
			break;
		case I2C_DN_JP:
////////////////////////////////////////////////////////
//	SCI�ւ�Data����M��Force�͂Ȃ���DeBug�p�ɗL���Ƃ���
			switch(pRequest->RequestCode) {
			case I2C_RC_JP_SND:
				for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
					ret = writeRegSC16(SC16_JP, SC16_THR_W, 1, pRWData, BUSFREE_WAIT);
					if (ret != RIIC_RSLT_OK)	{	break;	}
					++pRWData;
				}
				break;
			case I2C_RC_JP_RCV:
				for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
					ret = readRegSC16(SC16_JP, SC16_RHR_R, 1, pRWData, BUSFREE_WAIT);
					if (ret != RIIC_RSLT_OK)	{	break;	}
					++pRWData;
				}
				break;
			default:
				ret = RIIC_RSLT_PRM_ERROR;
				break;
			}
////////////////////////////////////////////////////////
			break;
		case I2C_DN_RTC:
			IicMilestone.RTCcnt++;		// = DebugInfo
			/********************************************************/
			/*	RTC�ւ̑��M����/��M������return�����犮���Ƃ���	*/
			/********************************************************/
			switch(pRequest->RequestCode) {
			case I2C_RC_RTC_WEEK_RCV:			// �g��_0x14			1
				ret = readRegRTC(RTC_REG_NO_EXT_WEEK, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_DAY_RCV:			// �g��_0x15			1
				ret = readRegRTC(RTC_REG_NO_EXT_DAY, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_MONTH_RCV:			// �g��_0x16			1
				ret = readRegRTC(RTC_REG_NO_EXT_MONTH, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_YEAR_RCV:			// �g��_0x17			1
				ret = readRegRTC(RTC_REG_NO_EXT_YEAR, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_STATUS_RCV:			// �g��_0x1E(RWcnt=1)
				ret = readRegRTC(RTC_REG_NO_EXT_FLAG, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_TIME_RCV:			// �g��_0x10(RWcnt=8)
				ret = readRegRTC(RTC_REG_NO_EXT_10MS, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_EX_RCV:
				ret = readRegRTC(RTC_REG_NO_EXT_EX, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_FLAG_RCV:
				ret = readRegRTC(RTC_REG_NO_EXT_FLAG, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_CTL_RCV:
				ret = readRegRTC(RTC_REG_NO_EXT_CTR, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_EX_FLAG_CTL_RCV:
				ret = readRegRTC(RTC_REG_NO_EXT_EX, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_TIME_SND:			// �g��_0x10(RWcnt=7)
				ret = writeRegRTC(RTC_REG_NO_EXT_SEC, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_FLAG_SND:			// �g��_0x1E(RWcnt=1)
				ret = writeRegRTC(RTC_REG_NO_EXT_FLAG, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_EX_FLAG_CTL_SND:	// �g��_0x1D(RWcnt=3)
				ret = writeRegRTC(RTC_REG_NO_EXT_EX, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_EX_SND:				// �g��_0x1D(RWcnt=1)
				ret = writeRegRTC(RTC_REG_NO_EXT_EX, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_CTL_SND:			// �g��_0x1F(RWcnt=1)
				ret = writeRegRTC(RTC_REG_NO_EXT_CTR, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			default:
				ret = RIIC_RSLT_PRM_ERROR;
				break;
			}
			break;
		default:
			ret = RIIC_RSLT_PRM_ERROR;
			break;
	}
	/********************************************************************/
	/*	NextDeviceProcNo = 1,2=I2cDev2_1, 3,4=I2cDev2_2, 5,6=RP, 7,8=JP	*/
	/*	0xfe = �������M���� & �������f�͎�t�Ă��Ȃ�					*/
	/********************************************************************/
	if ((I2cAM.NextDeviceProcNo >= 1) && (I2cAM.NextDeviceProcNo <= 8)) {
		I2cNextDeviceProc(I2cAM.NextDeviceProcNo, I2C_INT_MODE_EI);
		IicMilestone.ForceReqStatus = 0xfd;	// 0xfd = 1,2,3,4,5,6,7,8���I2cNextDeviceProc()���s�������
		I2cAM.NextDeviceProcNo = 0;
	}
	ei_I2cArbitration();
	return(ret);
}
/************************************************************************/
/*																		*/
/*		1.	RequestData����͂��ē��YBuffer�Ɋi�[����					*/
/*		2.	ActiveDevice��Check����										*/
/*		ExeMode = EXE_MODE_QUEUE										*/
/*																		*/
/************************************************************************/
enum I2cResultFnc_e 	I2C_RequestQueue(I2C_REQUEST *pRequest)
{
	enum I2cResultFnc_e 	ret;
	/************************************************************************/
	/*	RP, JP�̏ꍇ��I2c_Reuest()��Main����Call����邪					*/
	/*	RTC�̏ꍇ��IRQ4����Call����闝�R��IRQ5,8��Interrupt���ꂽ����		*/
	/*	Resource�ی�ׂ̈�IRQ4, IRQ5, IRQ8�̂�DI��I2cInt�͐������Ă���		*/
	/************************************************************************/
	di_I2cArbitration();
	ret = RIIC_RSLT_OK;
	/****************************/
	/*	Device Type Check		*/
	/****************************/
	switch(pRequest->DeviceNo) {
		case I2C_DN_I2CDEV2_1:
			ret = I2C_RequestQueueI2cDev2_1(pRequest);
			break;
		case I2C_DN_I2CDEV2_2:
			ret = I2C_RequestQueueI2cDev2_2(pRequest);
			break;
		case I2C_DN_RP:
			ret = I2C_RequestQueueRp(pRequest);		//	<--- 0210 Debug for JP
			break;
		case I2C_DN_JP:
			ret = I2C_RequestQueueJp(pRequest);		//	<--- 0210 Debug for RP
			break;
		case I2C_DN_RTC:
			ret = I2C_RequestQueueRtc(pRequest);
			break;
		default:
			ret = RIIC_RSLT_PRM_ERROR;
			break;
	}
	ei_I2cArbitration();
	return(ret);
}
/********************************************************************************************/
/*																							*/
/*		Baudrate: 	400Kbps(2.5us/1bit)														*/
/*		write(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+D:8+A:1+P:1) =: 30bit(75us)					*/
/*		read(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+SlvAdr:7+R:1+A:1+D:8+A:1+P:1) =: 40bit(100us)	*/
/*																							*/
/********************************************************************************************/
enum I2cResultFnc_e 	I2C_RequestQueueI2cDev2_1(I2C_REQUEST *pRequest)
{
	enum I2cResultFnc_e 	ret;
	unsigned long			loopcnt;
	unsigned long			RWcnt;
	unsigned char			*pRWData;
	unsigned char			FifoReverse;

	ret = RIIC_RSLT_OK;
	RWcnt 	= pRequest->I2cReqDataInfo.RWCnt;
	pRWData	= pRequest->I2cReqDataInfo.pRWData;
	/********************************/
	/*	RequestCode Check			*/
	/********************************/
	if ((pRequest->RequestCode != I2C_RC_I2CDEV2_1_SND) 		&&
		(pRequest->RequestCode != I2C_RC_I2CDEV2_1_RCV)) {
		ret = RIIC_RSLT_PRM_ERROR;
	}
	/********************************/
	/*	Status Check				*/
	/*	---> ���M�\����s��Ȃ�		*/
	/********************************/

	/********************************/
	/*	Buffer Over Check			*/
	/********************************/
	if (I2cAM.CtrI2cDev2_1.AplRegistedNum >= 4) {
		ret = RIIC_RSLT_BUFF_OVER_ERROR;	// Write/Read���s���\��Buffer����t�ł�
		// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR�ȊO = Error�o�^���s��
		return(ret);
	}
	/********************************/
	/*	Start of Buffer Queueing	*/
	/********************************/
	FifoReverse = 0;
	for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
		I2cAM.AplBufI2cDev2_1[I2cAM.CtrI2cDev2_1.AplWriteP++] = *pRWData++;
		if (I2cAM.CtrI2cDev2_1.AplWriteP >= SIZE_APL_I2CDEV2_1_MAX * 4) {
			I2cAM.CtrI2cDev2_1.AplWriteP = 0;
			FifoReverse = 1;
		}
	}
	I2cAM.CtrI2cDev2_1.AplRegistedNum++;
	I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplRegistBoxIndex) & 0x03].I2cReqDataInfo.RWCnt = pRequest->I2cReqDataInfo.RWCnt;
	I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplRegistBoxIndex) & 0x03].I2cReqDataInfo.pRWData = pRequest->I2cReqDataInfo.pRWData;
	I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplRegistBoxIndex) & 0x03].TaskNoTo = pRequest->TaskNoTo;
	I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplRegistBoxIndex) & 0x03].DeviceNo = pRequest->DeviceNo;
	I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplRegistBoxIndex) & 0x03].RequestCode = pRequest->RequestCode;
	I2cAM.CtrI2cDev2_1.AplRegistBoxIndex++;
	/************************************************************************/
	/*	����Queue�\�񂵂����MData�͑���Device���͎�����Sending/Receiving	*/
	/*	�łȂ���΍��񂱂���Drive����										*/
	/*	�ł���Ύ��ɔ�������IRQ5�̏����ɂđ��M�J�nCheck���s��Drive����		*/
	/************************************************************************/
	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		return(ret);		// ����ɑ��M�\�񂳂ꂽ������Device���͎�����Send/Recv���Ă���
	}
	//	��L��I2C_DN_NON�Ȃ�SDevice��Status��IDLE�̂͂��Ȃ̂�Check���Ȃ�
	/****************************************************************/
	/*	���YDevice��AplWriteP : AplReadP��Check����					*/
	/*	���YDevice��Buffer���Queueing����Ă���Message�𑗐M����	*/
	/*		BusFreeWait = BUSFREE_IGNO								*/
	/****************************************************************/
	if (FifoReverse == 0) {
		if (I2cAM.CtrI2cDev2_1.AplWriteP <= I2cAM.CtrI2cDev2_1.AplReadP) {
			return(ret);
		}
	} else {
		if (I2cAM.CtrI2cDev2_1.AplWriteP > I2cAM.CtrI2cDev2_1.AplReadP) {
			return(ret);
		}
	}
	/****************************************/
	/*	SendNewStart ---> Target Device		*/
	/****************************************/
	I2c_SendNewstart_Start_IRQ5_8(SC16_I2CDEV2_1, I2C_INT_MODE_EI);
	return(ret);
}
/********************************************************************************************/
/*																							*/
/*		Baudrate: 	400Kbps(2.5us/1bit)														*/
/*		write(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+D:8+A:1+P:1) =: 30bit(75us)					*/
/*		read(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+SlvAdr:7+R:1+A:1+D:8+A:1+P:1) =: 40bit(100us)	*/
/*																							*/
/********************************************************************************************/
enum I2cResultFnc_e 	I2C_RequestQueueI2cDev2_2(I2C_REQUEST *pRequest)
{
	enum I2cResultFnc_e 	ret;
	unsigned long			loopcnt;
	unsigned long			RWcnt;
	unsigned char			*pRWData;
	unsigned char			FifoReverse;

	ret = RIIC_RSLT_OK;
	RWcnt 	= pRequest->I2cReqDataInfo.RWCnt;
	pRWData	= pRequest->I2cReqDataInfo.pRWData;
	/********************************/
	/*	RequestCode Check			*/
	/********************************/
	if ((pRequest->RequestCode != I2C_RC_I2CDEV2_2_SND) &&
		(pRequest->RequestCode != I2C_RC_I2CDEV2_2_RCV)) {
		ret = RIIC_RSLT_PRM_ERROR;
	}
	/********************************/
	/*	Status Check				*/
	/*	---> ���M�\����s��Ȃ�		*/
	/********************************/

	/********************************/
	/*	Buffer Over Check			*/
	/********************************/
	if (I2cAM.CtrI2cDev2_2.AplRegistedNum >= 4) {
		ret = RIIC_RSLT_BUFF_OVER_ERROR;	// Write/Read���s���\��Buffer����t�ł�
		// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR�ȊO = Error�o�^���s��
		return(ret);
	}
	/********************************/
	/*	Start of Buffer Queueing	*/
	/********************************/
	FifoReverse = 0;
	for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
		I2cAM.AplBufI2cDev2_2[I2cAM.CtrI2cDev2_2.AplWriteP++] = *pRWData++;
		if (I2cAM.CtrI2cDev2_2.AplWriteP >= SIZE_APL_I2CDEV2_2_MAX * 4) {
			I2cAM.CtrI2cDev2_2.AplWriteP = 0;
			FifoReverse = 1;
		}
	}
	I2cAM.CtrI2cDev2_2.AplRegistedNum++;
	I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplRegistBoxIndex) & 0x03].I2cReqDataInfo.RWCnt = pRequest->I2cReqDataInfo.RWCnt;
	I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplRegistBoxIndex) & 0x03].I2cReqDataInfo.pRWData = pRequest->I2cReqDataInfo.pRWData;
	I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplRegistBoxIndex) & 0x03].TaskNoTo = pRequest->TaskNoTo;
	I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplRegistBoxIndex) & 0x03].DeviceNo = pRequest->DeviceNo;
	I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplRegistBoxIndex) & 0x03].RequestCode = pRequest->RequestCode;
	I2cAM.CtrI2cDev2_2.AplRegistBoxIndex++;
	/************************************************************************/
	/*	����Queue�\�񂵂����MData�͑���Device���͎�����Sending/Receiving	*/
	/*	�łȂ���΍��񂱂���Drive����										*/
	/*	�ł���Ύ��ɔ�������IRQ5�̏����ɂđ��M�J�nCheck���s��Drive����		*/
	/************************************************************************/
	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		return(ret);		// ����ɑ��M�\�񂳂ꂽ������Device���͎�����Send/Recv���Ă���
	}
	/****************************************************************/
	/*	���YDevice��AplWriteP : AplReadP��Check����					*/
	/*	���YDevice��Buffer���Queueing����Ă���Message�𑗐M����	*/
	/*		BusFreeWait = BUSFREE_IGNO								*/
	/****************************************************************/
	if (FifoReverse == 0) {
		if (I2cAM.CtrI2cDev2_2.AplWriteP <= I2cAM.CtrI2cDev2_2.AplReadP) {
			return(ret);
		}
	} else {
		if (I2cAM.CtrI2cDev2_2.AplWriteP > I2cAM.CtrI2cDev2_2.AplReadP) {
			return(ret);
		}
	}
	/****************************************/
	/*	SendNewStart ---> Target Device		*/
	/****************************************/
	I2c_SendNewstart_Start_IRQ5_8(SC16_I2CDEV2_2, I2C_INT_MODE_EI);
	return(ret);
}
/************************************************************************************************************************/
/*		Baudrate: 	400Kbps(2.5us/1bit)																					*/
/*		write(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+D:8+A:1+P:1) =: 30bit(75us)												*/
/*		read(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+SlvAdr:7+R:1+A:1+D:8+A:1+P:1) =: 40bit(100us)								*/
/*																														*/
/*	---------------------------------------------------------------------------------------------------------------		*/
/*	Byte��		Data���e				pri_def.h�Ƃ̑Ή�																*/
/*	---------------------------------------------------------------------------------------------------------------		*/
/*	2		0x1b, 0x40				PCMD_INIT			[������̏�����]					��ESC @��						*/
/*	3		0x1d, 0x76, 0x00		PCMD_STATUS_AUTO	[������ð���̎������M]			��GS v NULL��					*/
/*	2		0x1b, 0x76				PCMD_STATUS_SEND	[������ð���̑��M]				��ESC v��						*/
/*	3		0x1b, 0x21, 0x00		PCMD_FONT_K_N		[������Ӱ�ވꊇ�w��(�ʏ�)]	��FS ! n(=0x00)��				*/
/*	3		0x1b, 0x33, 0xff		PCMD_KAIGYO_xx		[���s�ʎw��]					��ESC 3 n��						*/
/*	3		0x1b, 0x20, 0x00		PCMD_R_SPACE		[�����̉E��߰���(0mm)]			��ESC SPC n(=0x04)��			*/
/*	3		0x1c, 0x43, 0x01		PCMD_SHIFTJIS		[�������ޑ̌n�I��(���JIS)]		��FS C n(=0x01)��				*/
/*	3 		0x1c, 0x57, 0x00		PCMD_KANJI4_CLR		[�����̂S�{�p����(����)]		��FS W n(=0x00)��				*/
/*	3 		0x1c, 0x21, 0x00		PCMD_FONT_K_N		[������Ӱ�ވꊇ�w��(�ʏ�)]	��FS ! n(=0x00)��				*/
/*	4 		0x1c, 0x53, 0x00, 0x00	PCMD_SPACE_KANJI	[������߰��ʂ̎w��(��0mm,�E0mm)]��FS S n1(=0x00) n2(=0x04)��	*/
/*	--------																											*/
/*	2 		0x1b, 0x40				PCMD_INIT			[������̏�����]					��ESC @��						*/
/*	3 		0x1d, 0x76, 0x00		PCMD_STATUS_AUTO	[������ð���̎������M]			��GS v NULL��					*/
/*	2 		0x1b, 0x76				PCMD_STATUS_SEND	[������ð���̑��M]				��ESC v��						*/
/*	3 		0x1b, 0x20, 0x00		PCMD_R_SPACE		[�����̉E��߰���(0mm)]			��ESC SPC n(=0x04)��			*/
/*	3 		0x1c, 0x43, 0x01		PCMD_SHIFTJIS		[�������ޑ̌n�I��(���JIS)]		��FS C n(=0x01)��				*/
/*	3 		0x1c, 0x57, 0x00		PCMD_KANJI4_CLR		[�����̂S�{�p����(����)]		��FS W n(=0x00)��				*/
/*	3 		0x1c, 0x21, 0x00		PCMD_FONT_K_N		[������Ӱ�ވꊇ�w��(�ʏ�)]	��FS ! n(=0x00)��				*/
/*	4 		0x1c, 0x53, 0x00, 0x00	PCMD_SPACE_KANJI	[������߰��ʂ̎w��(��0mm,�E0mm)]��FS S n1(=0x00) n2(=0x04)��	*/
/************************************************************************************************************************/
enum I2cResultFnc_e 	I2C_RequestQueueRp(I2C_REQUEST *pRequest)
{
	enum I2cResultFnc_e 	ret;
	unsigned long			loopcnt;
	unsigned long			RWcnt;
	unsigned char			*pRWData;
	unsigned char			FifoReverse;

	ret = RIIC_RSLT_OK;
	RWcnt 	= pRequest->I2cReqDataInfo.RWCnt;
	pRWData	= pRequest->I2cReqDataInfo.pRWData;
	/********************************/
	/*	RequestCode Check			*/
	/********************************/
	if ((pRequest->RequestCode != I2C_RC_RP_SND) &&
		(pRequest->RequestCode != I2C_RC_RP_RCV)) {
		ret = RIIC_RSLT_PRM_ERROR;
	}
	/********************************/
	/*	Buffer Over Check			*/
	/********************************/
	if (I2cAM.CtrRp.AplRegistedNum >= 4) {
		if (I2cErrStatus.SendRP == 0) {
			I2cErrStatus.SendRP = 1;
			I2cErrEvent.SendRP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_SEND_RP = 1;	// BIT 8 = ���V�[�g�v�����^���M�o�b�t�@�t��(�wI2Cڼ�đ��M�ޯ̧�فx)		����/������
		}
		ret = RIIC_RSLT_BUFF_OVER_ERROR;	// Write/Read���s���\��Buffer����t�ł�
		return(ret);
	} else {
		if (I2cErrStatus.SendRP != 0) {
			I2cErrStatus.SendRP = 0;
			I2cErrEvent.SendRP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_SEND_RP = 0;	// BIT 8 = ���V�[�g�v�����^���M�o�b�t�@�t��(�wI2Cڼ�đ��M�ޯ̧�فx)		����/������
		}
	}
	/********************************/
	/*	Start of Buffer Queueing	*/
	/********************************/
	FifoReverse = 0;
	for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
		I2cAM.AplBufRp[I2cAM.CtrRp.AplWriteP++] = *pRWData++;
		if (I2cAM.CtrRp.AplWriteP >= SIZE_APL_RP_MAX * 4) {
			I2cAM.CtrRp.AplWriteP = 0;
			FifoReverse = 1;
		}
	}
	I2cAM.CtrRp.AplRegistedNum++;
	I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplRegistBoxIndex) & 0x03].I2cReqDataInfo.RWCnt = pRequest->I2cReqDataInfo.RWCnt;
	I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplRegistBoxIndex) & 0x03].I2cReqDataInfo.pRWData = pRequest->I2cReqDataInfo.pRWData;
	I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplRegistBoxIndex) & 0x03].TaskNoTo = pRequest->TaskNoTo;
	I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplRegistBoxIndex) & 0x03].DeviceNo = pRequest->DeviceNo;
	I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplRegistBoxIndex) & 0x03].RequestCode = pRequest->RequestCode;
	I2cAM.CtrRp.AplRegistBoxIndex++;
	/************************************************************************/
	/*	����Queue�\�񂵂����MData�͑���Device���͎�����Sending/Receiving	*/
	/*	�łȂ���΍��񂱂���Drive����										*/
	/*	�ł���Ύ��ɔ�������IRQ5�̏����ɂđ��M�J�nCheck���s��Drive����		*/
	/************************************************************************/
	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		return(ret);		// ����ɑ��M�\�񂳂ꂽ������Device���͎�����Send/Recv���Ă���
	}
	//	��L��I2C_DN_NON�Ȃ�SDevice��Status��IDLE�̂͂��Ȃ̂�Check���Ȃ�
	/****************************************************************/
	/*	���YDevice��AplWriteP : AplReadP��Check����					*/
	/*	���YDevice��Buffer���Queueing����Ă���Message�𑗐M����	*/
	/*		BusFreeWait = BUSFREE_IGNO								*/
	/****************************************************************/
	if (FifoReverse == 0) {
		if (I2cAM.CtrRp.AplWriteP <= I2cAM.CtrRp.AplReadP) {
			return(ret);
		}
	} else {
		if (I2cAM.CtrRp.AplWriteP > I2cAM.CtrRp.AplReadP) {
			return(ret);
		}
	}
	/****************************************/
	/*	SendNewStart ---> Target Device		*/
	/****************************************/
	I2c_SendNewstart_Start_IRQ5_8(SC16_RP, I2C_INT_MODE_EI);
	return(ret);
}
/********************************************************************************************/
/*																							*/
/*		Baudrate: 	400Kbps(2.5us/1bit)														*/
/*		write(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+D:8+A:1+P:1) =: 30bit(75us)					*/
/*		read(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+SlvAdr:7+R:1+A:1+D:8+A:1+P:1) =: 40bit(100us)	*/
/*																							*/
/********************************************************************************************/
enum I2cResultFnc_e 	I2C_RequestQueueJp(I2C_REQUEST *pRequest)
{
	enum I2cResultFnc_e 	ret;
	unsigned long			loopcnt;
	unsigned long			RWcnt;
	unsigned char			*pRWData;
	unsigned char			FifoReverse;

	ret = RIIC_RSLT_OK;
	RWcnt 	= pRequest->I2cReqDataInfo.RWCnt;
	pRWData	= pRequest->I2cReqDataInfo.pRWData;
	/********************************/
	/*	RequestCode Check			*/
	/********************************/
	if ((pRequest->RequestCode != I2C_RC_JP_SND) &&
		(pRequest->RequestCode != I2C_RC_JP_RCV)) {
		ret = RIIC_RSLT_PRM_ERROR;
	}
	/********************************/
	/*	Buffer Over Check			*/
	/********************************/
	if (I2cAM.CtrJp.AplRegistedNum >= 4) {
		if (I2cErrStatus.SendJP == 0) {
			I2cErrStatus.SendJP = 1;
			I2cErrEvent.SendJP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_SEND_JP = 1;	// BIT10 = �W���[�i���v�����^���M�o�b�t�@�t��(�wI2C�ެ��ّ��M�ޯ̧�فx)	����/������
		}
		ret = RIIC_RSLT_BUFF_OVER_ERROR;	// Write/Read���s���\��Buffer����t�ł�
		return(ret);
	} else {
		if (I2cErrStatus.SendJP != 0) {
			I2cErrStatus.SendJP = 0;
			I2cErrEvent.SendJP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_SEND_JP = 0;	// BIT10 = �W���[�i���v�����^���M�o�b�t�@�t��(�wI2C�ެ��ّ��M�ޯ̧�فx)	����/������
		}
	}
	/********************************/
	/*	Start of Buffer Queueing	*/
	/********************************/
	FifoReverse = 0;
	for(loopcnt=0; loopcnt < RWcnt; loopcnt++){
		I2cAM.AplBufJp[I2cAM.CtrJp.AplWriteP++] = *pRWData++;
		if (I2cAM.CtrJp.AplWriteP >= SIZE_APL_JP_MAX * 4) {
			I2cAM.CtrJp.AplWriteP = 0;
			FifoReverse = 1;
		}
	}
	I2cAM.CtrJp.AplRegistedNum++;
	I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplRegistBoxIndex) & 0x03].I2cReqDataInfo.RWCnt = pRequest->I2cReqDataInfo.RWCnt;
	I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplRegistBoxIndex) & 0x03].I2cReqDataInfo.pRWData = pRequest->I2cReqDataInfo.pRWData;
	I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplRegistBoxIndex) & 0x03].TaskNoTo = pRequest->TaskNoTo;
	I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplRegistBoxIndex) & 0x03].DeviceNo = pRequest->DeviceNo;
	I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplRegistBoxIndex) & 0x03].RequestCode = pRequest->RequestCode;
	I2cAM.CtrJp.AplRegistBoxIndex++;
	/************************************************************************/
	/*	����Queue�\�񂵂����MData�͑���Device���͎�����Sending/Receiving	*/
	/*	�łȂ���΍��񂱂���Drive����										*/
	/*	�ł���Ύ��ɔ�������IRQ5�̏����ɂđ��M�J�nCheck���s��Drive����		*/
	/************************************************************************/
	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		return(ret);		// ����ɑ��M�\�񂳂ꂽ������Device���͎�����Send/Recv���Ă���
	}
	//	��L��I2C_DN_NON�Ȃ�SDevice��Status��IDLE�̂͂��Ȃ̂�Check���Ȃ�
	/****************************************************************/
	/*	���YDevice��AplWriteP : AplReadP��Check����					*/
	/*	���YDevice��Buffer���Queueing����Ă���Message�𑗐M����	*/
	/*		BusFreeWait = BUSFREE_IGNO								*/
	/****************************************************************/
	if (FifoReverse == 0) {
		if (I2cAM.CtrJp.AplWriteP <= I2cAM.CtrJp.AplReadP) {
			return(ret);
		}
	} else {
		if (I2cAM.CtrJp.AplWriteP > I2cAM.CtrJp.AplReadP) {
			return(ret);
		}
	}
	/****************************************/
	/*	SendNewStart ---> Target Device		*/
	/****************************************/
	I2c_SendNewstart_Start_IRQ5_8(SC16_JP, I2C_INT_MODE_EI);
	return(ret);
}
/********************************************************************************************/
/*																							*/
/*		Baudrate: 	400Kbps(2.5us/1bit)														*/
/*		write(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+D:8+A:1+P:1) =: 30bit(75us)					*/
/*		read(S:1+SlvAdr:7+W:1+A:1+Adr:8+A:1+SlvAdr:7+R:1+A:1+D:8+A:1+P:1) =: 40bit(100us)	*/
/*																							*/
/********************************************************************************************/
enum I2cResultFnc_e 	I2C_RequestQueueRtc(I2C_REQUEST *pRequest)
{
	enum I2cResultFnc_e 			ret;
//	--->	Stab�Ŏc���Ă���
	ret = RIIC_RSLT_OK;			// ����I��
	return(ret);
}
/****************************************************************************/
/*																			*/
/*																			*/
/****************************************************************************/
enum I2cResultFnc_e 	writeRegSC16( 	unsigned char 	DevNo,			// SC16_I2CDEV2_1, SC16_I2CDEV2_2, SC16_RP, SC16_JP
										unsigned char	RegAdr,			// SC16��InternalRegsterAddress
										unsigned long 	WriteNum,		// Write��
										unsigned char 	*pWriteData,	// Pointe of WriteData
										unsigned long 	BusFree)		// 0/1 = non/ wait until BusFree
{
	volatile unsigned long 	cnt;
	I2C_MESSAGE 			WriteInfo;
	enum I2cResultFnc_e 			ret;
	enum RiicStatus_e 		wait_status;
	enum RiicBusStatus_e 	wait_bus_status;
	unsigned short	status_wait_cnt = 0;

	/****************************************************************/
	/*	�O�񑗐M�f�[�^������(I2cBusFree)�܂ő��M�f�[�^�̍쐬�҂�	*/
	/****************************************************************/
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( status_wait_cnt >= 1000 ){								// 1�b�ԕω����Ȃ��ꍇ�͔�����
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;				// Bit 4 = �O��Status�ω��҂�Timeout	������������
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);

	status_wait_cnt = 0;
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( status_wait_cnt >= 1000 ){								// 1�b�ԕω����Ȃ��ꍇ�͔�����
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;				// Bit 4 = �O��Status�ω��҂�Timeout	������������
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_bus_status != RIIC_BUS_STATUS_FREE);
	ret = RIIC_RSLT_OK;			// ����I��
	switch(DevNo) {
		case SC16_I2CDEV2_1:
			if(WriteNum > SIZE_TRM_I2CDEV2_1_MAX) {
				return 	RIIC_RSLT_OVER_DATA_ERROR;
			}
			trm_adr_I2CDEV2_1[0] = (RegAdr << 3) | CH0;
			WriteInfo.SlvAdr	= SLV_ADDR_I2CDEV2_1;
			WriteInfo.PreCnt	= ADDR_LEN_I2CDEV2_1;
			WriteInfo.pPreData	= trm_adr_I2CDEV2_1;
			WriteInfo.pRWData	= pWriteData;
			break;
		case SC16_I2CDEV2_2:
			if(WriteNum > SIZE_TRM_I2CDEV2_2_MAX) {
				return 	RIIC_RSLT_OVER_DATA_ERROR;
			}
			trm_adr_I2CDEV2_2[0] = (RegAdr << 3) | CH1;
			WriteInfo.SlvAdr	= SLV_ADDR_I2CDEV2_2;
			WriteInfo.PreCnt	= ADDR_LEN_I2CDEV2_2;
			WriteInfo.pPreData	= trm_adr_I2CDEV2_2;
			WriteInfo.pRWData	= pWriteData;
			break;
		case SC16_RP:
			if(WriteNum > SIZE_TRM_RP_MAX) {
				return 	RIIC_RSLT_OVER_DATA_ERROR;
			}
			trm_adr_RP[0] = (RegAdr << 3) | CH0;
			WriteInfo.SlvAdr	= SLV_ADDR_RP;
			WriteInfo.PreCnt	= ADDR_LEN_RP;
			WriteInfo.pPreData	= trm_adr_RP;
			WriteInfo.pRWData	= pWriteData;
			break;
		case SC16_JP:
			if(WriteNum > SIZE_TRM_JP_MAX) {
				return 	RIIC_RSLT_OVER_DATA_ERROR;
			}
			trm_adr_JP[0] = (RegAdr << 3) | CH1;
			WriteInfo.SlvAdr	= SLV_ADDR_JP;
			WriteInfo.PreCnt	= ADDR_LEN_JP;
			WriteInfo.pPreData	= trm_adr_JP;
			WriteInfo.pRWData	= pWriteData;
			break;
		default:
			return 	RIIC_RSLT_PRM_ERROR;
			break;
	}
	WriteInfo.RWCnt		= WriteNum;
	/********************************/
	/*	Write Data to Device		*/
	/********************************/
	ret = writeControl_I2C(&WriteInfo, BusFree);
	return(ret);
}
/****************************************************************************/
/*																			*/
/*																			*/
/****************************************************************************/
enum I2cResultFnc_e 	readRegSC16(unsigned char 	DevNo,		// SC16_I2CDEV2_1, SC16_I2CDEV2_2, SC16_RP, SC16_JP
									unsigned char	RegAdr,		// SC16��InternalRegsterAddress
									unsigned long 	ReadNum,	// Read��
									unsigned char 	*pReadData,	// Pointe of ReadData
									unsigned long 	BusFree)	// 0/1 = non/ wait until BusFree
{
	volatile unsigned long 	cnt;
	I2C_MESSAGE 			ReadInfo;
	enum I2cResultFnc_e 	ret;

	switch(DevNo) {
		case SC16_I2CDEV2_1:
			if(ReadNum > SIZE_RCV_I2CDEV2_1_MAX) {
				return 	RIIC_RSLT_OVER_DATA_ERROR;
			}
		rcv_adr_I2CDEV2_1[0] = (RegAdr << 3) | CH0;
			ReadInfo.SlvAdr		= SLV_ADDR_I2CDEV2_1;
			ReadInfo.PreCnt		= ADDR_LEN_I2CDEV2_1;
			ReadInfo.pPreData	= rcv_adr_I2CDEV2_1;
			ReadInfo.pRWData	= pReadData;
			break;
		case SC16_I2CDEV2_2:
			if(ReadNum > SIZE_RCV_I2CDEV2_2_MAX) {
				return 	RIIC_RSLT_OVER_DATA_ERROR;
			}
			rcv_adr_I2CDEV2_2[0] = (RegAdr << 3) | CH1;
			ReadInfo.SlvAdr		= SLV_ADDR_I2CDEV2_2;
			ReadInfo.PreCnt		= ADDR_LEN_I2CDEV2_2;
			ReadInfo.pPreData	= rcv_adr_I2CDEV2_2;
			ReadInfo.pRWData	= pReadData;
			break;
		case SC16_RP:
			if(ReadNum > SIZE_RCV_RP_MAX) {
				return 	RIIC_RSLT_OVER_DATA_ERROR;
			}
			rcv_adr_RP[0] = (RegAdr << 3) | CH0;
			ReadInfo.SlvAdr		= SLV_ADDR_RP;
			ReadInfo.PreCnt		= ADDR_LEN_RP;
			ReadInfo.pPreData	= rcv_adr_RP;
			ReadInfo.pRWData	= pReadData;
			break;
		case SC16_JP:
			if(ReadNum > SIZE_RCV_JP_MAX) {
				return 	RIIC_RSLT_OVER_DATA_ERROR;
			}
			rcv_adr_JP[0] = (RegAdr << 3) | CH1;
			ReadInfo.SlvAdr		= SLV_ADDR_JP;
			ReadInfo.PreCnt		= ADDR_LEN_JP;
			ReadInfo.pPreData	= rcv_adr_JP;
			ReadInfo.pRWData	= pReadData;
			break;
		default:
			return 	RIIC_RSLT_PRM_ERROR;
			break;
	}
	ReadInfo.RWCnt		= ReadNum;
	/********************************/
	/*	Read Data from Device		*/
	/********************************/
	ret = readControl_I2C(&ReadInfo, BusFree);
	return(ret);
}
/****************************************************************************/
/*																			*/
/*																			*/
/****************************************************************************/
enum I2cResultFnc_e 	writeRegRTC(unsigned char	RegAdr,			// SC16��InternalRegsterAddress
									unsigned long 	WriteNum,		// Write��
									unsigned char 	*pWriteData,	// Pointe of WriteData
									unsigned long 	BusFree)		// 0/1 = non/ wait until BusFree
{
	volatile unsigned long 	cnt;
	I2C_MESSAGE 			WriteInfo;
	enum I2cResultFnc_e 	ret;

	if(WriteNum > SIZE_TRM_RTC_MAX) {
		return 	RIIC_RSLT_OVER_DATA_ERROR;
	}
	trm_adr_RTC[0] = RegAdr;
	/********************************/
	/*	Write Data to Device		*/
	/********************************/
	WriteInfo.SlvAdr	= SLV_ADDR_RTC;
	WriteInfo.PreCnt	= ADDR_LEN_RTC;
	WriteInfo.pPreData	= trm_adr_RTC;
	WriteInfo.RWCnt		= WriteNum;
	WriteInfo.pRWData	= pWriteData;
	/********************************/
	/*	Write Data to Device		*/
	/********************************/
	ret = writeControl_I2C(&WriteInfo, BusFree);
	return(ret);
}
/****************************************************************************/
/*																			*/
/*																			*/
/****************************************************************************/
enum I2cResultFnc_e 	readRegRTC(	unsigned char	RegAdr,		// SC16��InternalRegsterAddress
									unsigned long 	ReadNum,	// Read��
									unsigned char 	*pReadData,	// Pointe of ReadData
									unsigned long 	BusFree)	// 0/1 = non/ wait until BusFree
{
	volatile unsigned long 	cnt;
	I2C_MESSAGE 			ReadInfo;
	enum I2cResultFnc_e 		ret;

	if(ReadNum > SIZE_RCV_RTC_MAX) {
		return 	RIIC_RSLT_OVER_DATA_ERROR;
	}
	rcv_adr_RTC[0] = RegAdr;
	/********************************/
	/*	Read Data from Device		*/
	/********************************/
	ReadInfo.SlvAdr	 	= SLV_ADDR_RTC;
	ReadInfo.PreCnt		= ADDR_LEN_RTC;
	ReadInfo.pPreData	= rcv_adr_RTC;
	ReadInfo.RWCnt		= ReadNum;
	ReadInfo.pRWData	= pReadData;
	/********************************/
	/*	Read Data from Device		*/
	/********************************/
	ret = readControl_I2C(&ReadInfo, BusFree);
	return(ret);
}
/************************************************************************************************************/
/*																											*/
/*	I2C Write Process Control Routine																		*/
/*	Input																									*/
/*		unsigned char	SlvAdr;		// SLV_ADDR_RTC, SLV_ADDR_I2CDEV2_1, SLV_ADDR_I2CDEV2_2, SLV_ADDR_RP, SLV_ADDR_JP	*/
/*		unsigned short	PreCnt;		// ADDR_LEN_RTC, ADDR_LEN_I2CDEV2_1, ADDR_LEN_I2CDEV2_2, ADDR_LEN_RP, ADDR_LEN_JP	*/
/*		unsigned char	*pPreData;	// rcv_adr_RTC, rcv_adr_I2CDEV2_1, rcv_adr_I2CDEV2_2, rcv_adr_RP, rcv_adr_JP		*/
/*									// trm_adr_RTC, trm_adr_I2CDEV2_1, trm_adr_I2CDEV2_2, trm_adr_RP, trm_adr_JP		*/
/*		unsigned long	RWCnt;		// Number of Data														*/
/*		unsigned char	*pRWData;	// rcv_buff_RTC, rcv_buff_I2CDEV2_1, rcv_buff_I2CDEV2_2, rcv_buff_RP, rcv_buff_JP	*/
/*									// trm_buff_RTC, trm_buff_I2CDEV2_1, trm_buff_I2CDEV2_2, trm_buff_RP, trm_buff_JP	*/
/*																											*/
/************************************************************************************************************/
enum I2cResultFnc_e 	writeControl_I2C(I2C_MESSAGE	*pWriteInfo, unsigned long BusFree)
{
	enum RiicStatus_e 		wait_status;
	enum RiicBusStatus_e 	wait_bus_status;
	unsigned short	status_wait_cnt = 0;

	/********************************/
	/*	Set the parameters 			*/
	/********************************/
	iic_buff_prm[BUFF_NUM_WRITE].SlvAdr 	= pWriteInfo->SlvAdr;	// 
	iic_buff_prm[BUFF_NUM_WRITE].PreCnt 	= pWriteInfo->PreCnt;	// 
	iic_buff_prm[BUFF_NUM_WRITE].pPreData 	= pWriteInfo->pPreData;	// 
	iic_buff_prm[BUFF_NUM_WRITE].RWCnt 		= pWriteInfo->RWCnt;	// 
	iic_buff_prm[BUFF_NUM_WRITE].pRWData 	= pWriteInfo->pRWData;	// 
	/************************************************************************/
	/*	�O�񑗐M����Device������(I2cBusFree)���Ȃ���΍���̑��M�͍s���Ȃ�	*/
	/************************************************************************/
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( status_wait_cnt >= 1000 ){								// 1�b�ԕω����Ȃ��ꍇ�͔�����
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);
	
	status_wait_cnt = 0;
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( status_wait_cnt >= 1000 ){								// 1�b�ԕω����Ȃ��ꍇ�͔�����
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_bus_status != RIIC_BUS_STATUS_FREE);
	/****************************************/
	/*	Start I2cBus-Device Write 			*/
	/****************************************/
	I2C_Write(iic_buff_prm[BUFF_NUM_WRITE]);
	if (BusFree == BUSFREE_WAIT) {
		/********************************************************************/
		/*	����̑��M������(I2cBusFree)����܂�APL�Ɍ��ʂ͒ʒm�ł��Ȃ�		*/
		/********************************************************************/
		status_wait_cnt = 0;
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if( status_wait_cnt >= 1000 ){							//	1�b�ԕω����Ȃ��ꍇ�͔�����
				I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
				return	RIIC_RSLT_PREV_TOUT;
			}else{
				wait2us(I2C_ONE_WAIT);								// 100us wait
				status_wait_cnt++;
			}
		}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);

		status_wait_cnt = 0;
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if( status_wait_cnt >= 1000 ){							// 1�b�ԕω����Ȃ��ꍇ�͔�����
				I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
				return	RIIC_RSLT_PREV_TOUT;
			}else{
				wait2us(I2C_ONE_WAIT);								// 100us wait
				status_wait_cnt++;
			}
		}while(wait_bus_status != RIIC_BUS_STATUS_FREE);
	}
	return (RIIC_RSLT_OK);
}
/************************************************************************************************************/
/*																											*/
/*	I2C Read Process Control Routine																		*/
/*	Input																									*/
/*		unsigned char	SlvAdr;		// SLV_ADDR_RTC, SLV_ADDR_I2CDEV2_1, SLV_ADDR_I2CDEV2_2, SLV_ADDR_RP, SLV_ADDR_JP	*/
/*		unsigned short	PreCnt;		// ADDR_LEN_RTC, ADDR_LEN_I2CDEV2_1, ADDR_LEN_I2CDEV2_2, ADDR_LEN_RP, ADDR_LEN_JP	*/
/*		unsigned char	*pPreData;	// rcv_adr_RTC, rcv_adr_I2CDEV2_1, rcv_adr_I2CDEV2_2, rcv_adr_RP, rcv_adr_JP		*/
/*									// trm_adr_RTC, trm_adr_I2CDEV2_1, trm_adr_I2CDEV2_2, trm_adr_RP, trm_adr_JP		*/
/*		unsigned long	RWCnt;		// Number of Data														*/
/*		unsigned char	*pRWData;	// rcv_buff_RTC, rcv_buff_I2CDEV2_1, rcv_buff_I2CDEV2_2, rcv_buff_RP, rcv_buff_JP	*/
/*									// trm_buff_RTC, trm_buff_I2CDEV2_1, trm_buff_I2CDEV2_2, trm_buff_RP, trm_buff_JP	*/
/*																											*/
/************************************************************************************************************/
enum I2cResultFnc_e 	readControl_I2C(I2C_MESSAGE 	*pReadInfo, unsigned long BusFree)
{

	enum RiicStatus_e 		wait_status;
	enum RiicBusStatus_e 	wait_bus_status;
	unsigned short	status_wait_cnt = 0;

	/********************************/
	/*	Set the parameters 			*/
	/********************************/
	iic_buff_prm[BUFF_NUM_READ].SlvAdr 		= pReadInfo->SlvAdr;	// 
	iic_buff_prm[BUFF_NUM_READ].PreCnt 		= pReadInfo->PreCnt;	// 
	iic_buff_prm[BUFF_NUM_READ].pPreData 	= pReadInfo->pPreData;	// 
	iic_buff_prm[BUFF_NUM_READ].RWCnt 		= pReadInfo->RWCnt;		// 
	iic_buff_prm[BUFF_NUM_READ].pRWData 	= pReadInfo->pRWData;	// 
	/************************************************************************/
	/*	�O�񑗐M����Device������(I2cBusFree)���Ȃ���΍���̑��M�͍s���Ȃ�	*/
	/************************************************************************/
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( status_wait_cnt >= I2C_TOTAL_WAIT_CNT ){				//	3�b�ԕω����Ȃ��ꍇ�͔�����
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);

	status_wait_cnt = 0;
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( status_wait_cnt >= I2C_TOTAL_WAIT_CNT ){				//	3�b�ԕω����Ȃ��ꍇ�͔�����
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_bus_status != RIIC_BUS_STATUS_FREE);
	/****************************************/
	/*	Start I2cBus-Device Read 			*/
	/****************************************/
	I2C_Read(iic_buff_prm[BUFF_NUM_READ]);
	if (BusFree == BUSFREE_WAIT) {
		/********************************************************************/
		/*	����̑��M������(I2cBusFree)����܂�APL�Ɍ��ʂ͒ʒm�ł��Ȃ�		*/
		/********************************************************************/
		status_wait_cnt = 0;
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if( status_wait_cnt >= I2C_TOTAL_WAIT_CNT ){			//	3�b�ԕω����Ȃ��ꍇ�͔�����
				I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
				return	RIIC_RSLT_PREV_TOUT;
			}else{
				wait2us(I2C_ONE_WAIT);								// 100us wait
				status_wait_cnt++;
			}
		}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);

		status_wait_cnt = 0;
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if( status_wait_cnt >= I2C_TOTAL_WAIT_CNT ){			//	3�b�ԕω����Ȃ��ꍇ�͔�����
				I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
				return	RIIC_RSLT_PREV_TOUT;
			}else{
				wait2us(I2C_ONE_WAIT);								// 100us wait
				status_wait_cnt++;
			}
		}while(wait_bus_status != RIIC_BUS_STATUS_FREE);
	}
	return (RIIC_RSLT_OK);
}
/************************************************************************************************************/
/*	Start I2cBusDevice write process or Ack polling.														*/
/*																											*/
/*	Input																									*/
/*		unsigned char	SlvAdr;		// SLV_ADDR_RTC, SLV_ADDR_I2CDEV2_1, SLV_ADDR_I2CDEV2_2, SLV_ADDR_RP, SLV_ADDR_JP	*/
/*		unsigned short	PreCnt;		// ADDR_LEN_RTC, ADDR_LEN_I2CDEV2_1, ADDR_LEN_I2CDEV2_2, ADDR_LEN_RP, ADDR_LEN_JP	*/
/*		unsigned char	*pPreData;	// rcv_adr_RTC, rcv_adr_I2CDEV2_1, rcv_adr_I2CDEV2_2, rcv_adr_RP, rcv_adr_JP		*/
/*									// trm_adr_RTC, trm_adr_I2CDEV2_1, trm_adr_I2CDEV2_2, trm_adr_RP, trm_adr_JP		*/
/*		unsigned long	RWCnt;		// Number of Data														*/
/*		unsigned char	*pRWData;	// rcv_buff_RTC, rcv_buff_I2CDEV2_1, rcv_buff_I2CDEV2_2, rcv_buff_RP, rcv_buff_JP	*/
/*									// trm_buff_RTC, trm_buff_I2CDEV2_1, trm_buff_I2CDEV2_2, trm_buff_RP, trm_buff_JP	*/
/*																											*/
/************************************************************************************************************/
enum I2cResultFnc_e 	I2C_Write(I2C_MESSAGE 	data1)
{
	int loopcnt;
	unsigned char *pData;

	/************************************/
	/*	Check the IIC mode				*/
	/*	If IIC bus is BusBusy, return	*/
	/************************************/
	if(iic_mode != IIC_MODE_I2C_IDLE) {
		// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR�ȊO = Error�o�^���s�� ---> �s��Ȃ�
		return RIIC_RSLT_MODE_ERROR;
	}
	if(RIIC1.ICCR2.BIT.BBSY != 0) {
		I2C_BusError_Info.BIT.I2C_BUS_BUSY = 1;			// Bit 3 = �o�XBUSY						������������
		return RIIC_RSLT_BUS_BUSY;
	}
	/********************************/
	/*	Set the I2c Parameter Data	*/
	/********************************/
	iic_ActiveBuff.SlvAdr 	= data1.SlvAdr;
	iic_ActiveBuff.PreCnt 	= data1.PreCnt;
	iic_ActiveBuff.pPreData = data1.pPreData;
	iic_ActiveBuff.RWCnt 	= data1.RWCnt;
	iic_ActiveBuff.pRWData 	= data1.pRWData;
	/**********************************************/
	iic_mode 		= IIC_MODE_I2C_WRITE;
	iic_status 		= RIIC_STATUS_ON_COMMUNICATION;
	iic_trm_cnt 	= 0;
	IicMilestone.Write = 0;	// = DebugInfo
	IicMilestone.Error = 0;	// = DebugInfo
	for (loopcnt=0; loopcnt<32; loopcnt++) {
		IicMilestone.I2cINT[loopcnt] = 0;
	}
	IicMilestone.I2cINT_num = 0;	// = DebugInfo
	IicMilestone.Wcnt++;			// = DebugInfo
	/*************************/
	if ((iic_ActiveBuff.SlvAdr == SLV_ADDR_RP) || (iic_ActiveBuff.SlvAdr == SLV_ADDR_JP)) {	// SLV_ADDR_RTC, SLV_ADDR_I2CDEV2_1
		if (*iic_ActiveBuff.pPreData ==  ((SC16_THR_W << 3) | CH0)) {
			pData = iic_ActiveBuff.pRWData;
			for (loopcnt=0; loopcnt<iic_ActiveBuff.RWCnt; loopcnt++) {
				IicMilestone.RP_THR_History[IicMilestone.RP_THR_HistCnt++] = *pData++;	// = DebugInfo
				if (IicMilestone.RP_THR_HistCnt >= 256) {	// = DebugInfo
					IicMilestone.RP_THR_HistCnt = 0;		// = DebugInfo
				}
			}
		} else if (*iic_ActiveBuff.pPreData ==  ((SC16_THR_W << 3) | CH1)) {
			pData = iic_ActiveBuff.pRWData;
			for (loopcnt=0; loopcnt<iic_ActiveBuff.RWCnt; loopcnt++) {
				IicMilestone.JP_THR_History[IicMilestone.JP_THR_HistCnt++] = *pData++;	// = DebugInfo
				if (IicMilestone.JP_THR_HistCnt >= 256) {	// = DebugInfo
					IicMilestone.JP_THR_HistCnt = 0;		// = DebugInfo
				}
			}
		}
	}
	/*************************/
	/*	start condition <- 1 */	//	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	/*************************/
//	RIIC1.ICFER.BIT.TMOE = 1;	// time out detection <- EI
	IicMilestone.I2cSP_Sending = 0;
	RIIC1.ICCR2.BIT.ST = 1;
	/****************************/
	/*	Write Process Started	*/
	/****************************/
	return RIIC_RSLT_OK;
}
/************************************************************************************************************/
/*																											*/
/*	Start Randam read process																				*/
/*	Input																									*/
/*		unsigned char	SlvAdr;		// SLV_ADDR_RTC, SLV_ADDR_I2CDEV2_1, SLV_ADDR_I2CDEV2_2, SLV_ADDR_RP, SLV_ADDR_JP	*/
/*		unsigned short	PreCnt;		// ADDR_LEN_RTC, ADDR_LEN_I2CDEV2_1, ADDR_LEN_I2CDEV2_2, ADDR_LEN_RP, ADDR_LEN_JP	*/
/*		unsigned char	*pPreData;	// rcv_adr_RTC, rcv_adr_I2CDEV2_1, rcv_adr_I2CDEV2_2, rcv_adr_RP, rcv_adr_JP		*/
/*									// trm_adr_RTC, trm_adr_I2CDEV2_1, trm_adr_I2CDEV2_2, trm_adr_RP, trm_adr_JP		*/
/*		unsigned long	RWCnt;		// Number of Data														*/
/*		unsigned char	*pRWData;	// rcv_buff_RTC, rcv_buff_I2CDEV2_1, rcv_buff_I2CDEV2_2, rcv_buff_RP, rcv_buff_JP	*/
/*									// trm_buff_RTC, trm_buff_I2CDEV2_1, trm_buff_I2CDEV2_2, trm_buff_RP, trm_buff_JP	*/
/*																											*/
/************************************************************************************************************/
enum I2cResultFnc_e 	I2C_Read(I2C_MESSAGE data1)
{
	volatile unsigned char 	tmp;
	volatile unsigned long 	cnt;
	int loopcnt;

	/************************************/
	/*	Check the IIC mode				*/
	/*	Check parameter					*/
	/*	If IIC bus is BusBusy, return	*/
	/************************************/
	if(iic_mode != IIC_MODE_I2C_IDLE) {
		// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR�ȊO = Error�o�^���s�� ---> �s��Ȃ�
		return RIIC_RSLT_MODE_ERROR;
	}
	if((data1.RWCnt == 0) || ((data1.PreCnt == 0))) {
		return RIIC_RSLT_PRM_ERROR;
	}
	if(RIIC1.ICCR2.BIT.BBSY != 0) {
		I2C_BusError_Info.BIT.I2C_BUS_BUSY = 1;			// Bit 3 = �o�XBUSY						������������
		return RIIC_RSLT_BUS_BUSY;
	}
	/********************************/
	/*	Set the I2c Parameter Data	*/
	/********************************/
	iic_ActiveBuff.SlvAdr 	= data1.SlvAdr;
	iic_ActiveBuff.PreCnt 	= data1.PreCnt;
	iic_ActiveBuff.pPreData = data1.pPreData;
	iic_ActiveBuff.RWCnt 	= data1.RWCnt;
	iic_ActiveBuff.pRWData 	= data1.pRWData;
	/**********************************************/
	iic_mode 		= IIC_MODE_I2C_READ;
	iic_status 		= RIIC_STATUS_ON_COMMUNICATION;
	iic_trm_cnt 	= 0;
	iic_rcv_cnt 	= 0;
	IicMilestone.Read = 0;	// = DebugInfo
	IicMilestone.Error = 0;	// = DebugInfo
	for (loopcnt=0; loopcnt<32; loopcnt++) {
		IicMilestone.I2cINT[loopcnt] = 0;	// = DebugInfo
	}
	IicMilestone.I2cINT_num = 0;			// = DebugInfo
	IicMilestone.Rcnt++;					// = DebugInfo
	/*************************/
	/*	start condition <- 1 */	//	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	/*************************/
//	RIIC1.ICFER.BIT.TMOE = 1;	// time out detection <- EI
	IicMilestone.I2cSP_Sending = 0;
	RIIC1.ICCR2.BIT.ST = 1;
	/****************************/
	/*	Write Process Started	*/
	/****************************/
	return RIIC_RSLT_OK;
}
/****************************************************************************/
/*																			*/
/*	Get IIC status & BusBusy												*/
/*		*data1 <--- iic_status�̒l											*/
/*		*data2 <--- RIIC1.ICCR2.BIT.BBSY�̒l								*/
/****************************************************************************/
/*	wait_status																*/
/*		�ʐM��(RIIC_STATUS_ON_COMMUNICATION)�ł����I2cBusFree�łȂ��̂�	*/
/*		wait����(��������RIIC_STATUS_IDLE��wt_6, rd_9��set�����)			*/
/*		���s����RIIC_STATUS_NACK, RIIC_STATUS_FAILED��set�����				*/
/*		* set�ӏ���I2c������Handler���ł���									*/
/*	wait_bus_status															*/
/*		RIIC1.ICCR2.BIT.BBSY�̏�Ԃł���									*/
/*			0:	RIIC_BUS_STATUS_FREE										*/
/*			1:	RIIC_BUS_STATUS_BBSY										*/
/*																			*/
/*	---->	���Â��I2c������Handler��EI�łȂ��ƌ��o�ł��Ȃ��̂�			*/
/*			���o���Ԓ���I2c��EI�ł��鎖										*/
/*																			*/
/****************************************************************************/
/*																			*/
/****************************************************************************/
void I2C_GetStatus(	enum RiicStatus_e 		*data1,	// iic_status(RIIC_STATUS_)
					enum RiicBusStatus_e 	*data2)	// RIIC_BUS_STATUS_FREE/RIIC_BUS_STATUS_BBSY
{
	*data1 = iic_status;	//	RIIC_STATUS_

	if(RIIC1.ICCR2.BIT.BBSY == 0) {
		*data2 = RIIC_BUS_STATUS_FREE;
	} else {
		*data2 = RIIC_BUS_STATUS_BBSY;
	}
}
/****************************************************************************/
/*																			*/
/*	Application I2cBus-Device Polling call									*/
/*		�ȉ���Loop���̂�Application�ōs��									*/
/****************************************************************************/
bool AppliI2cAckPolling(unsigned char 	in_addr1,	// Slave device address
						unsigned char 	in_num,		// Number of Ack polling
						unsigned long 	in_len)		// Length between each polling
{
	I2C_MESSAGE 			tmp_buff;
	enum RiicStatus_e 		wait_status;
	enum RiicBusStatus_e 	wait_bus_status;
	unsigned char 			tmp_num;
	volatile unsigned long 	cnt1, cnt2;

	/********************************/
	/*	Set the parameters 			*/
	/********************************/
	tmp_buff.SlvAdr = in_addr1;
	tmp_buff.PreCnt = 0;
	tmp_buff.RWCnt 	= 0;
	tmp_num 		= in_num;
	/****************************************************/
	/* Ack polling until target I2cBusDevice reply ACK 	*/
	/****************************************************/
	do{
		/****************************************/
		/*	Start Ack polling (Master transfer)	*/
		/****************************************/
		while(RIIC_RSLT_OK != I2C_Write(tmp_buff))
		{	};
		/****************************************/
		/*	Wait for communication complete		*/
		/****************************************/
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);
		/****************************************************************************************/
		// When I2cBusDevice reply NACK, wait for some time ---> Then start Ack polling again	*/
		/****************************************************************************************/
		if(wait_status == RIIC_STATUS_NACK) {
			for(cnt1=in_len; cnt1!=0; cnt1--) {
				for(cnt2=0; cnt2<100; cnt2++);
			}
		} else if(wait_status == RIIC_STATUS_IDLE) {	// ACK
			return true;
		}
		tmp_num--;
	}while((tmp_num != 0) || (in_num == 0));
	return false;
}
/****************************************************************/
/*	It's called from below Modules								*/
/*		1.	IRQ5												*/
/*		2.	IRQ8												*/
/*	* RequestCode��Check���鎞��[AplActiveBoxIndex-1]			*/
/*	  = [0-]�Ȃ̂�CurrentData(AplActiveBoxIndex-1)�ōs��		*/
/****************************************************************/
void I2c_SendResume_IRQ5_8(int DeviceNo, unsigned long I2cIntMode)
{

	if (I2cIntMode == I2C_INT_MODE_DI) {
		_ei();
	}
	switch(DeviceNo) {
	case SC16_RP:
		switch(I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex-1) & 0x03].RequestCode) {
		case I2C_RC_RP_SND:
			I2cAM.CtrRp.SendData[0] = I2cAM.AplBufRp[I2cAM.CtrRp.AplReadP++];
			if (I2cAM.CtrRp.AplReadP >= SIZE_APL_RP_MAX * 4) {
				I2cAM.CtrRp.AplReadP = 0;
			}
			I2cAM.CtrRp.Status = DEVICE_STS_SENDING;
			I2cAM.CtrRp.SendData[1] = 0x03;
			writeRegSC16(SC16_RP, SC16_THR_W, 1, &I2cAM.CtrRp.SendData[0], BUSFREE_IGNO);
			writeRegSC16(SC16_RP, SC16_IER_RW, 1, &I2cAM.CtrRp.SendData[1], BUSFREE_IGNO);	// Sc16RecvInSending_IRQ5_Level_L
			break;
		case I2C_RC_RP_RCV:	// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
			I2cAM.CtrRp.SendData[0] = I2cAM.AplBufRp[I2cAM.CtrRp.AplReadP++];
			if (I2cAM.CtrRp.AplReadP >= SIZE_APL_RP_MAX * 4) {
				I2cAM.CtrRp.AplReadP = 0;
			}
			readRegSC16(SC16_RP, SC16_RHR_R, 1, &I2cAM.CtrRp.SendData[0], BUSFREE_IGNO);
			break;
		default:
			break;
		}
		break;
	case SC16_JP:
		switch(I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex-1) & 0x03].RequestCode) {
		case I2C_RC_JP_SND:
			I2cAM.CtrJp.SendData[0] = I2cAM.AplBufJp[I2cAM.CtrJp.AplReadP++];
			if (I2cAM.CtrJp.AplReadP >= SIZE_APL_JP_MAX * 4) {
				I2cAM.CtrJp.AplReadP = 0;
			}
			I2cAM.CtrJp.Status = DEVICE_STS_SENDING;
			I2cAM.CtrJp.SendData[1] = 0x03;
			writeRegSC16(SC16_JP, SC16_THR_W, 1, &I2cAM.CtrJp.SendData[0], BUSFREE_IGNO);
			writeRegSC16(SC16_JP, SC16_IER_RW, 1, &I2cAM.CtrJp.SendData[1], BUSFREE_IGNO);	// Sc16RecvInSending_IRQ5_Level_L
			break;
		case I2C_RC_JP_RCV:	// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
			I2cAM.CtrJp.SendData[0] = I2cAM.AplBufJp[I2cAM.CtrJp.AplReadP++];
			if (I2cAM.CtrJp.AplReadP >= SIZE_APL_JP_MAX * 4) {
				I2cAM.CtrJp.AplReadP = 0;
			}
			readRegSC16(SC16_JP, SC16_RHR_R, 1, &I2cAM.CtrJp.SendData[0], BUSFREE_IGNO);
			break;
		default:
			break;
		}
		break;
	case SC16_I2CDEV2_1:
		switch(I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplActiveBoxIndex-1) & 0x03].RequestCode) {
		case I2C_RC_I2CDEV2_1_SND:
			I2cAM.CtrI2cDev2_1.SendData[0] = I2cAM.AplBufI2cDev2_1[I2cAM.CtrI2cDev2_1.AplReadP++];
			if (I2cAM.CtrI2cDev2_1.AplReadP >= SIZE_APL_I2CDEV2_1_MAX * 4) {
				I2cAM.CtrI2cDev2_1.AplReadP = 0;
			}
			I2cAM.CtrI2cDev2_1.Status = DEVICE_STS_SENDING;
			I2cAM.CtrI2cDev2_1.SendData[1] = 0x03;
			writeRegSC16(SC16_I2CDEV2_1, SC16_THR_W, 1, &I2cAM.CtrI2cDev2_1.SendData[0], BUSFREE_IGNO);
			writeRegSC16(SC16_I2CDEV2_1, SC16_IER_RW, 1, &I2cAM.CtrI2cDev2_1.SendData[1], BUSFREE_IGNO);	// Sc16RecvInSending_IRQ5_Level_L
			break;
		case I2C_RC_I2CDEV2_1_RCV:	// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
			I2cAM.CtrI2cDev2_1.SendData[0] = I2cAM.AplBufI2cDev2_1[I2cAM.CtrI2cDev2_1.AplReadP++];
			if (I2cAM.CtrI2cDev2_1.AplReadP >= SIZE_APL_I2CDEV2_1_MAX * 4) {
				I2cAM.CtrI2cDev2_1.AplReadP = 0;
			}
			readRegSC16(SC16_I2CDEV2_1, SC16_RHR_R, 1, &I2cAM.CtrI2cDev2_1.SendData[0], BUSFREE_IGNO);
			break;
		default:
			break;
		}
		break;
	case SC16_I2CDEV2_2:
		break;
	default:
		break;
	}
	if (I2cIntMode == I2C_INT_MODE_DI) {
		_di();
	}
}
/****************************************************************/
/*	It's called from below Modules								*/
/*		1.	I2C_RequestQueueXxxx								*/
/*		2.	IRQ5												*/
/*		3.	IRQ8												*/
/*	* RequestCode��Check���鎞��[AplActiveBoxIndex]				*/
/*	  = [0-]�Ȃ̂�NextData(AplActiveBoxIndex)�ōs��				*/
/****************************************************************/
void I2c_SendNewstart_Start_IRQ5_8(int DeviceNo, unsigned long I2cIntMode)
{

	if (I2cIntMode == I2C_INT_MODE_DI) {
		_ei();
	}
	switch(DeviceNo) {
	case SC16_RP:
		switch(I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex) & 0x03].RequestCode) {
		case I2C_RC_RP_SND:
			I2cAM.ActiveDevice = I2C_DN_RP;
			I2cAM.CtrRp.Status = DEVICE_STS_SENDING;
			I2cAM.CtrRp.AplActiveBoxIndex++;
			I2cAM.CtrRp.AplActiveRWCnt = 0;
			I2cAM.CtrRp.SendData[0] = I2cAM.AplBufRp[I2cAM.CtrRp.AplReadP++];
			if (I2cAM.CtrRp.AplReadP >= SIZE_APL_RP_MAX * 4) {
				I2cAM.CtrRp.AplReadP = 0;
			}
			I2cAM.CtrRp.SendData[1] = 0x03;
			writeRegSC16(SC16_RP, SC16_THR_W, 1, &I2cAM.CtrRp.SendData[0], BUSFREE_IGNO);
			writeRegSC16(SC16_RP, SC16_IER_RW, 1, &I2cAM.CtrRp.SendData[1], BUSFREE_IGNO);	// Sc16RecvInSending_IRQ5_Level_L
			break;
		case I2C_RC_RP_RCV:	// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
			I2cAM.ActiveDevice = I2C_DN_RP;
			I2cAM.CtrRp.Status = DEVICE_STS_SENDING;
			I2cAM.CtrRp.AplActiveBoxIndex++;
			I2cAM.CtrRp.AplActiveRWCnt = 0;
			I2cAM.CtrRp.SendData[0] = I2cAM.AplBufRp[I2cAM.CtrRp.AplReadP++];
			if (I2cAM.CtrRp.AplReadP >= SIZE_APL_RP_MAX * 4) {
				I2cAM.CtrRp.AplReadP = 0;
			}
			readRegSC16(SC16_RP, SC16_RHR_R, 1, &I2cAM.CtrRp.SendData[0], BUSFREE_IGNO);
			break;
		default:
			break;
		}
		break;
	case SC16_JP:
		switch(I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex) & 0x03].RequestCode) {
		case I2C_RC_JP_SND:
			I2cAM.ActiveDevice = I2C_DN_JP;
			I2cAM.CtrJp.Status = DEVICE_STS_SENDING;
			I2cAM.CtrJp.AplActiveBoxIndex++;
			I2cAM.CtrJp.AplActiveRWCnt = 0;
			I2cAM.CtrJp.SendData[0] = I2cAM.AplBufJp[I2cAM.CtrJp.AplReadP++];
			if (I2cAM.CtrJp.AplReadP >= SIZE_APL_JP_MAX * 4) {
				I2cAM.CtrJp.AplReadP = 0;
			}
			I2cAM.CtrJp.SendData[1] = 0x03;
			writeRegSC16(SC16_JP, SC16_THR_W, 1, &I2cAM.CtrJp.SendData[0], BUSFREE_IGNO);
			writeRegSC16(SC16_JP, SC16_IER_RW, 1, &I2cAM.CtrJp.SendData[1], BUSFREE_IGNO);	// Sc16RecvInSending_IRQ5_Level_L
			break;
		case I2C_RC_JP_RCV:	// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
			I2cAM.ActiveDevice = I2C_DN_JP;
			I2cAM.CtrJp.Status = DEVICE_STS_SENDING;
			I2cAM.CtrJp.AplActiveBoxIndex++;
			I2cAM.CtrJp.AplActiveRWCnt = 0;
			I2cAM.CtrJp.SendData[0] = I2cAM.AplBufJp[I2cAM.CtrJp.AplReadP++];
			if (I2cAM.CtrJp.AplReadP >= SIZE_APL_JP_MAX * 4) {
				I2cAM.CtrJp.AplReadP = 0;
			}
			readRegSC16(SC16_JP, SC16_RHR_R, 1, &I2cAM.CtrJp.SendData[0], BUSFREE_IGNO);
			break;
		default:
			break;
		}
		break;
	case SC16_I2CDEV2_1:
		switch(I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplActiveBoxIndex) & 0x03].RequestCode) {
		case I2C_RC_I2CDEV2_1_SND:
			I2cAM.ActiveDevice = I2C_DN_I2CDEV2_1;
			I2cAM.CtrI2cDev2_1.Status = DEVICE_STS_SENDING;
			I2cAM.CtrI2cDev2_1.AplActiveBoxIndex++;
			I2cAM.CtrI2cDev2_1.AplActiveRWCnt = 0;
			I2cAM.CtrI2cDev2_1.SendData[0] = I2cAM.AplBufI2cDev2_1[I2cAM.CtrI2cDev2_1.AplReadP++];
			if (I2cAM.CtrI2cDev2_1.AplReadP >= SIZE_APL_I2CDEV2_1_MAX * 4) {
				I2cAM.CtrI2cDev2_1.AplReadP = 0;
			}
			I2cAM.CtrI2cDev2_1.SendData[1] = 0x03;
			writeRegSC16(SC16_I2CDEV2_1, SC16_THR_W, 1, &I2cAM.CtrI2cDev2_1.SendData[0], BUSFREE_IGNO);
			writeRegSC16(SC16_I2CDEV2_1, SC16_IER_RW, 1, &I2cAM.CtrI2cDev2_1.SendData[1], BUSFREE_IGNO);	// Sc16RecvInSending_IRQ5_Level_L
			break;
		case I2C_RC_I2CDEV2_1_RCV:	// RHR�����Read�v�� ---> ���YIRQ��BIT_DATA_RECEIVED_LSR=1�ɂ�Read����̂ŕs�v
			I2cAM.ActiveDevice = I2C_DN_I2CDEV2_1;
			I2cAM.CtrI2cDev2_1.Status = DEVICE_STS_SENDING;
			I2cAM.CtrI2cDev2_1.AplActiveBoxIndex++;
			I2cAM.CtrI2cDev2_1.AplActiveRWCnt = 0;
			I2cAM.CtrI2cDev2_1.SendData[0] = I2cAM.AplBufI2cDev2_1[I2cAM.CtrI2cDev2_1.AplReadP++];
			if (I2cAM.CtrI2cDev2_1.AplReadP >= SIZE_APL_I2CDEV2_1_MAX * 4) {
				I2cAM.CtrI2cDev2_1.AplReadP = 0;
			}
			readRegSC16(SC16_I2CDEV2_1, SC16_RHR_R, 1, &I2cAM.CtrI2cDev2_1.SendData[0], BUSFREE_IGNO);
			break;
		default:
			break;
		}
		break;
	case SC16_I2CDEV2_2:
		break;
	default:
		break;
	}
	if (I2cIntMode == I2C_INT_MODE_DI) {
		_di();
	}
}
/****************************************************/
/*	SearchSequence:		I2CDEV2_1 -> I2CDEV2_2 -> RP -> JP	*/
/*		in:				out:						*/
/*						���M�ĊJ	�V���M			*/
/*		SC16_I2CDEV2_1		inx2+1:1	inx2+2:2		*/
/*		SC16_I2CDEV2_2		inx2+1:3	inx2+2:4		*/
/*		SC16_RP			inx2+1:5	inx2+2:6		*/
/*		SC16_JP			inx2+1:7	inx2+2:8		*/
/*	0xfe:	�������M���� & �������f�͎�t�Ă��Ȃ�	*/
/****************************************************/
unsigned char	I2cNextDeviceReNewStartCheck(int DeviceNo)
{
	unsigned char	ret;
	switch(DeviceNo) {
		case SC16_I2CDEV2_1:
			ret = I2cNextDeviceCheckI2cDev2_1();
			break;
		case SC16_I2CDEV2_2:
			ret = I2cNextDeviceCheckI2cDev2_2();
			break;
		case SC16_RP:
			ret = I2cNextDeviceCheckRp();
			break;
		case SC16_JP:
			ret = I2cNextDeviceCheckJp();
			break;
		default:
			ret = RIIC_RSLT_PRM_ERROR;
			break;
	}
	return(ret);
}
/****************************************************/
/*													*/
/****************************************************/
unsigned char	I2cNextDeviceCheckI2cDev2_1(void)
{
	unsigned char	ret = 0xfe;
	if (I2cAM.CtrI2cDev2_2.AplActiveRWCnt != I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 3;
	} else if (I2cAM.CtrI2cDev2_2.AplWriteP != I2cAM.CtrI2cDev2_2.AplReadP) {
		ret = 4;
	} else if (I2cAM.CtrRp.AplActiveRWCnt != I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 5;
	} else if (I2cAM.CtrRp.AplWriteP != I2cAM.CtrRp.AplReadP) {
		ret = 6;
	} else if (I2cAM.CtrJp.AplActiveRWCnt != I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 7;
	} else if (I2cAM.CtrJp.AplWriteP != I2cAM.CtrJp.AplReadP) {
		ret = 8;
	} else 	if (I2cAM.CtrI2cDev2_1.AplActiveRWCnt != I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 1;
	} else if (I2cAM.CtrI2cDev2_1.AplWriteP != I2cAM.CtrI2cDev2_1.AplReadP) {
		ret = 2;
	}
	return(ret);
}
/****************************************************/
/*													*/
/****************************************************/
unsigned char	I2cNextDeviceCheckI2cDev2_2(void)
{
	unsigned char	ret = 0xfe;
	if (I2cAM.CtrRp.AplActiveRWCnt != I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 5;
	} else if (I2cAM.CtrRp.AplWriteP != I2cAM.CtrRp.AplReadP) {
		ret = 6;
	} else if (I2cAM.CtrJp.AplActiveRWCnt != I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 7;
	} else if (I2cAM.CtrJp.AplWriteP != I2cAM.CtrJp.AplReadP) {
		ret = 8;
	} else 	if (I2cAM.CtrI2cDev2_1.AplActiveRWCnt != I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 1;
	} else if (I2cAM.CtrI2cDev2_1.AplWriteP != I2cAM.CtrI2cDev2_1.AplReadP) {
		ret = 2;
	} else if (I2cAM.CtrI2cDev2_2.AplActiveRWCnt != I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 3;
	} else if (I2cAM.CtrI2cDev2_2.AplWriteP != I2cAM.CtrI2cDev2_2.AplReadP) {
		ret = 4;
	}
	return(ret);
}
/****************************************************/
/*													*/
/****************************************************/
unsigned char	I2cNextDeviceCheckRp(void)
{
	unsigned char	ret = 0xfe;
	if (I2cAM.CtrJp.AplActiveRWCnt != I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 7;
	} else if (I2cAM.CtrJp.AplWriteP != I2cAM.CtrJp.AplReadP) {
		ret = 8;
	} else 	if (I2cAM.CtrI2cDev2_1.AplActiveRWCnt != I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 1;
	} else if (I2cAM.CtrI2cDev2_1.AplWriteP != I2cAM.CtrI2cDev2_1.AplReadP) {
		ret = 2;
	} else if (I2cAM.CtrI2cDev2_2.AplActiveRWCnt != I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 3;
	} else if (I2cAM.CtrI2cDev2_2.AplWriteP != I2cAM.CtrI2cDev2_2.AplReadP) {
		ret = 4;
	} else if (I2cAM.CtrRp.AplActiveRWCnt != I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 5;
	} else if (I2cAM.CtrRp.AplWriteP != I2cAM.CtrRp.AplReadP) {
		ret = 6;
	}
	return(ret);
}
/****************************************************/
/*													*/
/****************************************************/
unsigned char	I2cNextDeviceCheckJp(void)
{
	unsigned char	ret = 0xfe;
	if (I2cAM.CtrI2cDev2_1.AplActiveRWCnt != I2cAM.CtrI2cDev2_1.AplReqBox[(I2cAM.CtrI2cDev2_1.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 1;
	} else if (I2cAM.CtrI2cDev2_1.AplWriteP != I2cAM.CtrI2cDev2_1.AplReadP) {
		ret = 2;
	} else if (I2cAM.CtrI2cDev2_2.AplActiveRWCnt != I2cAM.CtrI2cDev2_2.AplReqBox[(I2cAM.CtrI2cDev2_2.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 3;
	} else if (I2cAM.CtrI2cDev2_2.AplWriteP != I2cAM.CtrI2cDev2_2.AplReadP) {
		ret = 4;
	} else if (I2cAM.CtrRp.AplActiveRWCnt != I2cAM.CtrRp.AplReqBox[(I2cAM.CtrRp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 5;
	} else if (I2cAM.CtrRp.AplWriteP != I2cAM.CtrRp.AplReadP) {
		ret = 6;
	} else if (I2cAM.CtrJp.AplActiveRWCnt != I2cAM.CtrJp.AplReqBox[(I2cAM.CtrJp.AplActiveBoxIndex-1) & 0x03].I2cReqDataInfo.RWCnt) {
		ret = 7;
	} else if (I2cAM.CtrJp.AplWriteP != I2cAM.CtrJp.AplReadP) {
		ret = 8;
	}
	return(ret);
}
/****************************************************************/
/*	I2cIntMode													*/
/*		I2C_INT_MODE_EI	// I2cInterrupt�͊����݉\��Ԃł���	*/
/*		I2C_INT_MODE_DI	// I2cInterrupt�͊����݉\��ԂłȂ�	*/
/****************************************************************/
void I2cNextDeviceProc(unsigned char NextDeviceProcNo, unsigned long I2cIntMode)
{
	switch(NextDeviceProcNo) {
		case 1:
			I2c_SendResume_IRQ5_8(SC16_I2CDEV2_1, I2cIntMode);
			break;
		case 2:
			I2c_SendNewstart_Start_IRQ5_8(SC16_I2CDEV2_1, I2cIntMode);
			break;
		case 3:
			I2c_SendResume_IRQ5_8(SC16_I2CDEV2_2, I2cIntMode);
			break;
		case 4:
			I2c_SendNewstart_Start_IRQ5_8(SC16_I2CDEV2_2, I2cIntMode);
			break;
		case 5:
			I2c_SendResume_IRQ5_8(SC16_RP, I2cIntMode);
			break;
		case 6:
			I2c_SendNewstart_Start_IRQ5_8(SC16_RP, I2cIntMode);
			break;
		case 7:
			I2c_SendResume_IRQ5_8(SC16_JP, I2cIntMode);
			break;
		case 8:
			I2c_SendNewstart_Start_IRQ5_8(SC16_JP, I2cIntMode);
			break;
		default:
			break;
	}
}

/****************************************************************/
/*		���M�ُ펞�̃f�[�^��j������Ƌ��ɁA�e��t���O���N���A	*/
/*		����													*/
/****************************************************************/
void I2cSendErrDataInit(unsigned char type)
{
	I2C_CONTROLS	*p;
	char wtemp = I2cAM.ActiveDevice;
	p = 0;
	
	if(type){
		wtemp = type;
	}
	switch(wtemp) {
		case I2C_DN_RP:
			p = &I2cAM.CtrRp;
			PrnStop_R();
			I2cAM.ActiveDevice = I2C_DN_NON;
			break;
		case I2C_DN_JP:
			p = &I2cAM.CtrJp;
			PrnStop_J();
			I2cAM.ActiveDevice = I2C_DN_NON;
			break;
		case I2C_DN_I2CDEV2_1:
			p = &I2cAM.CtrI2cDev2_1;
			I2cAM.ActiveDevice = I2C_DN_NON;
			break;
		case I2C_DN_I2CDEV2_2:
			p = &I2cAM.CtrI2cDev2_2;
			I2cAM.ActiveDevice = I2C_DN_NON;
			break;
		case I2C_DN_RTC:
			p = &I2cAM.CtrRtc;
			break;
		default:
			I2cAM.ActiveDevice = I2C_DN_NON;
			return;
	}
	p->AplWriteP = 0;
	p->AplReadP = 0;
	p->AplRegistedNum = 0;
	p->AplRegistBoxIndex = 0;
	p->AplActiveBoxIndex = 0;
	memset( p->AplReqBox, 0, sizeof(p->AplReqBox));
	p->AplActiveRWCnt = 0;
	p->DevRecvP = 0;
}
/****************************************************************************/
/*																			*/
/*																			*/
/****************************************************************************/
void di_I2cArbitration(void)
{
	unsigned long	ist;		// ���݂̊�����t���

	ist = _di2();
	ICU.IER[8].BYTE &= 0xcf;	// ICU_IRQ4 = IER08.IEN4, ICU_IRQ5 = IER08.IEN5 <- DI
	ICU.IER[9].BYTE &= 0xfe;	// ICU_IRQ8 = IER09.IEN0 <- DI
	_ei2(ist);
}
/****************************************************************************/
void ei_I2cArbitration(void)
{
	unsigned long	ist;		// ���݂̊�����t���

	ist = _di2();
	ICU.IER[8].BYTE |= 0x30;	// ICU_IRQ4 = IER08.IEN4, ICU_IRQ5 = IER08.IEN5 <- EI
	ICU.IER[9].BYTE |= 0x01;	// ICU_IRQ8 = IER09.IEN0 <- EI
	_ei2(ist);
}

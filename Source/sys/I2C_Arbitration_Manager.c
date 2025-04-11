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
#define	I2C_TOTAL_WAIT_CNT	30000	// 3秒
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
	/*	本送信はIC1_I2CDEV2_1のSC16_THR_WへのWriteでないのでIRQ5は発生しないと思われます	*/
	/*	何らかのDeviceがQueue動作していない時は本callがFORCEでもQUEUEでも				*/
	/*	本callで送信処理を行う必要があります。											*/
	/*	但し.....																		*/
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
	/*	本送信はIC1_I2CDEV2_1のSC16_THR_WへのWriteでないのでIRQ5は発生しないと思われます	*/
	/*	何らかのDeviceがQueue動作していない時は本callがFORCEでもQUEUEでも				*/
	/*	本callで送信処理を行う必要があります。											*/
	/*	但し.....																		*/
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
	request.I2cReqDataInfo.pRWData	= &I2cDev2_1Signal.AplSignalIn;		// CD:7, RI:6, DSR:5, CTS:4, △, △, △, △
	request.RequestCode	= I2C_RC_I2CDEV2_1_SIGNAL_IN_RCV;
	/************************************************************************************/
	/*	本送信はIC1_I2CDEV2_1のSC16_THR_WへのWriteでないのでIRQ5は発生しないと思われます	*/
	/*	何らかのDeviceがQueue動作していない時は本callがFORCEでもQUEUEでも				*/
	/*	本callで送信処理を行う必要があります。											*/
	/*	但し.....																		*/
	/************************************************************************************/
	ret = I2C_Request( &request, EXE_MODE_FORCE );	// EXE_MODE_QUEUE
	// 上位の判定論理に合わせる為にbit情報を反転させる
	I2cDev2_1Signal.AplSignalIn = ~I2cDev2_1Signal.AplSignalIn;
	return(ret);
}
/********************************************************************************************************/
/*		Call From Application Layer																		*/
/*		call元 = IRQ4, ......																			*/
/*	enum I2cResultFnc_e{																				*/
/*	RIIC_RSLT_OK = 0,			// 正常終了																*/
/*	RIIC_RSLT_BUS_BUSY,			// I2C_Bus上が何らかの理由によりBusy障害状態(送信不可)					*/
/*	RIIC_RSLT_MODE_ERROR,		// 現在I2C_Bus上ではWirteまたはRead, ACK_Pollingを実行中				*/
/*	RIIC_RSLT_PRM_ERROR,		// Application/ArbitrationMからのParameterが不正です					*/
/*	RIIC_RSLT_OVER_DATA_ERROR,	// ApplicationMからのwrite/readのData数がMax以上						*/
/*	RIIC_RSLT_BUFF_OVER_ERROR,	// Write/Readを行う予約Bufferが一杯です									*/
/*	RIIC_RSLT_PREV_TOUT,		// 前回ステータス変化待ちタイムアウト									*/
/*	RIIC_RSLT_MAX																						*/
/*	};																									*/
/*		--->	RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR以外 = Error登録を行う	*/
/*		Error情報 = I2C_Event_InfoにBit登録して(新型フラップNT-NET ErrorList_Rev110.xls)				*/
/*					IRQ5 -> I2C_Event_Analize()へ受け渡す												*/
/*					I2C_Event_Analize()内で																*/
/*					err_chk(Error種別, ErrorCode, 発生/削除/登録, 0, 0)をcall							*/
/********************************************************************************************************/
enum I2cResultFnc_e 	I2C_Request(I2C_REQUEST *pRequest, unsigned char ExeMode)
{
	enum I2cResultFnc_e 	ret;
	/****************************/
	/*	InputParameterのCheck	*/
	/****************************/
	if (pRequest->DeviceNo == I2C_DN_NON) 		{	ret = RIIC_RSLT_PRM_ERROR;	return(ret);	}
	if (pRequest->RequestCode == I2C_RC_NON) 	{	ret = RIIC_RSLT_PRM_ERROR;	return(ret);	}
	if (pRequest->I2cReqDataInfo.RWCnt == 0) 	{	ret = RIIC_RSLT_PRM_ERROR;	return(ret);	}
	if (pRequest->I2cReqDataInfo.pRWData == 0) 	{	ret = RIIC_RSLT_PRM_ERROR;	return(ret);	}
	/********************************/
	/*	ExeMode = EXE_MODE_FORCE	*/
	/*	1.	RTCは全てFORCE			*/
	/*	2.	I2cDev2_1Signalは全てFORCE	*/
	/********************************/
	if (ExeMode == EXE_MODE_FORCE) {
		ret = I2C_RequestForce(pRequest);
	/********************************/
	/*	ExeMode = EXE_MODE_QUEUE	*/
	/********************************/
	} else {
	/********************************************************************/
	/*	RTCの場合はPriorityが高く, またWrite/Readの終了割込みが無い為	*/
	/*	終了判定してqueset()を行う事ができないので即時に行う。			*/
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
/*		1.	RTCは全てFORCE												*/
/*		2.	I2cDev2_1Signalは全てFORCE										*/
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
	/*	1.	ActiveDevice == I2C_DN_NONであればI2cBusFree状態でかつ					*/
	/*		送信予約のDeviceが1つもないので無条件で送信処理を行える					*/
	/*		(---> FORCEで送信するDataは全てIRQ5, 8の割込みはかからない)				*/
	/*																				*/
	/*	2.	但しActiveDevice != I2C_DN_NONであればいづれかのDeviceが				*/
	/*		QueueDataの送信を行っているのでIRQ5又はIRQ8で							*/
	/*		現在送信中のQueueData_1Byteが終了した時にForceRequest==0xffなら			*/
	/*		強制送信要求有りと認識しForceRequest<-1,2,3,4,5,6,7,8して				*/
	/*		(I2cNextDeviceProc関数をcallする時のParameter: 0xfeの時は行わない)		*/
	/*		強制中断受付としDeviceの送信再開処理/新送信処理を行わずに				*/
	/*		本箇所で強制送信開始を行う(see ---> 強制中断時)							*/
	/*		但しIRQ5又はIRQ8で送信した1ByteのDataが最終で以後						*/
	/*		送信予約が何もなければForceRequest<-0xfeとする							*/
	/*		(強制送信許可 & 強制中断は受付ていない)									*/
	/*																				*/
	/*		強制中断時:	強制送信後のRestart処理については							*/
	/*					強制送信完了が終了して強制中断受付していた場合には			*/
	/*					Deviceの送信再開処理/新送信処理を行わなければいけない		*/
	/*					のでI2cNextDeviceProc()関数をcallする						*/
	/*																				*/
	/*	3.	10ms毎のI2C_RC_I2CDEV2_1_SIGNAL_IN_RCVと								*/
	/*		1Min毎のIRQ4からのRequestについて										*/
	/*																				*/
	/*																				*/
	/********************************************************************************/
	I2cAM.ForceRequest = 0;
	I2cAM.NextDeviceProcNo = 0;


	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		I2cAM.ForceRequest = 0xff;
 		IicMilestone.ForceReqStatus = 0xff;	// 0xff = 強制送信要求中
		/********************************************************/
		/*	IRQ5又はIRQ8で現在送信中のQueueData_1Byteが終了して	*/
		/*	ForceRequestを認識したので強制送信開始可			*/
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
		/*	0xfe = 強制送信許可 & 強制中断は受付ていない					*/
		/********************************************************************/
//////////////////////////////////////////////////////////////
//		強制送信で発生したTimeoutは稼働中のDeviceの通信不良として登録されたので
//		次の送信対象とはせずにRequestされた強制送信を即時に行う事とする
		if (I2cAM.ForceRequest != 0) {
			I2cAM.NextDeviceProcNo = I2cAM.ForceRequest;
			IicMilestone.ForceReqStatus = I2cAM.ForceRequest;
			I2cAM.ForceRequest = 0;
		}
	}
	/************************************************************************/
	/*	RP, JPの場合はI2c_Reuest()がMainからCallされるが					*/
	/*	RTCの場合はIRQ4からCallされる理由とIRQ5,8がInterruptされた時に		*/
	/*	Resource保護の為にIRQ4, IRQ5, IRQ8のみDIしI2cIntは生かしておく		*/
	/************************************************************************/
	di_I2cArbitration();
	/****************************/
	/*	Start of Device Access	*/
	/****************************/
	RWcnt 	= pRequest->I2cReqDataInfo.RWCnt;
	pRWData	= pRequest->I2cReqDataInfo.pRWData;
	//	
	//	------->	RequestCodeがDevice毎に異なるのでswitchで切替えてLogicを作成する
	//	
	switch(pRequest->DeviceNo) {
		case I2C_DN_I2CDEV2_1:
			switch(pRequest->RequestCode) {
////////////////////////////////////////////////////////
//	SCIへのData送受信はForceはないがDeBug用に有効とする
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
//	SCIへのData送受信はForceはないがDeBug用に有効とする
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
//	SCIへのData送受信はForceはないがDeBug用に有効とする
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
//	SCIへのData送受信はForceはないがDeBug用に有効とする
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
			/*	RTCへの送信完了/受信完了はreturnしたら完了とする	*/
			/********************************************************/
			switch(pRequest->RequestCode) {
			case I2C_RC_RTC_WEEK_RCV:			// 拡張_0x14			1
				ret = readRegRTC(RTC_REG_NO_EXT_WEEK, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_DAY_RCV:			// 拡張_0x15			1
				ret = readRegRTC(RTC_REG_NO_EXT_DAY, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_MONTH_RCV:			// 拡張_0x16			1
				ret = readRegRTC(RTC_REG_NO_EXT_MONTH, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_YEAR_RCV:			// 拡張_0x17			1
				ret = readRegRTC(RTC_REG_NO_EXT_YEAR, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_STATUS_RCV:			// 拡張_0x1E(RWcnt=1)
				ret = readRegRTC(RTC_REG_NO_EXT_FLAG, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_TIME_RCV:			// 拡張_0x10(RWcnt=8)
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
			case I2C_RC_RTC_TIME_SND:			// 拡張_0x10(RWcnt=7)
				ret = writeRegRTC(RTC_REG_NO_EXT_SEC, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_FLAG_SND:			// 拡張_0x1E(RWcnt=1)
				ret = writeRegRTC(RTC_REG_NO_EXT_FLAG, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_EX_FLAG_CTL_SND:	// 拡張_0x1D(RWcnt=3)
				ret = writeRegRTC(RTC_REG_NO_EXT_EX, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_EX_SND:				// 拡張_0x1D(RWcnt=1)
				ret = writeRegRTC(RTC_REG_NO_EXT_EX, RWcnt, pRWData, BUSFREE_WAIT);
				break;
			case I2C_RC_RTC_CTL_SND:			// 拡張_0x1F(RWcnt=1)
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
	/*	0xfe = 強制送信許可 & 強制中断は受付ていない					*/
	/********************************************************************/
	if ((I2cAM.NextDeviceProcNo >= 1) && (I2cAM.NextDeviceProcNo <= 8)) {
		I2cNextDeviceProc(I2cAM.NextDeviceProcNo, I2C_INT_MODE_EI);
		IicMilestone.ForceReqStatus = 0xfd;	// 0xfd = 1,2,3,4,5,6,7,8後のI2cNextDeviceProc()発行した状態
		I2cAM.NextDeviceProcNo = 0;
	}
	ei_I2cArbitration();
	return(ret);
}
/************************************************************************/
/*																		*/
/*		1.	RequestDataを解析して当該Bufferに格納する					*/
/*		2.	ActiveDeviceをCheckして										*/
/*		ExeMode = EXE_MODE_QUEUE										*/
/*																		*/
/************************************************************************/
enum I2cResultFnc_e 	I2C_RequestQueue(I2C_REQUEST *pRequest)
{
	enum I2cResultFnc_e 	ret;
	/************************************************************************/
	/*	RP, JPの場合はI2c_Reuest()がMainからCallされるが					*/
	/*	RTCの場合はIRQ4からCallされる理由とIRQ5,8がInterruptされた時に		*/
	/*	Resource保護の為にIRQ4, IRQ5, IRQ8のみDIしI2cIntは生かしておく		*/
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
	/*	---> 送信予約も行わない		*/
	/********************************/

	/********************************/
	/*	Buffer Over Check			*/
	/********************************/
	if (I2cAM.CtrI2cDev2_1.AplRegistedNum >= 4) {
		ret = RIIC_RSLT_BUFF_OVER_ERROR;	// Write/Readを行う予約Bufferが一杯です
		// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR以外 = Error登録を行う
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
	/*	今回Queue予約した送信Dataは他のDevice又は自分がSending/Receiving	*/
	/*	でなければ今回ここでDriveする										*/
	/*	であれば次に発生したIRQ5の処理にて送信開始Checkを行いDriveする		*/
	/************************************************************************/
	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		return(ret);		// 正常に送信予約されたが他のDevice又は自分がSend/Recvしている
	}
	//	上記がI2C_DN_NONなら全DeviceのStatusはIDLEのはずなのでCheckしない
	/****************************************************************/
	/*	当該DeviceのAplWriteP : AplReadPをCheckして					*/
	/*	当該DeviceのBufferよりQueueingされているMessageを送信する	*/
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
	/*	---> 送信予約も行わない		*/
	/********************************/

	/********************************/
	/*	Buffer Over Check			*/
	/********************************/
	if (I2cAM.CtrI2cDev2_2.AplRegistedNum >= 4) {
		ret = RIIC_RSLT_BUFF_OVER_ERROR;	// Write/Readを行う予約Bufferが一杯です
		// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR以外 = Error登録を行う
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
	/*	今回Queue予約した送信Dataは他のDevice又は自分がSending/Receiving	*/
	/*	でなければ今回ここでDriveする										*/
	/*	であれば次に発生したIRQ5の処理にて送信開始Checkを行いDriveする		*/
	/************************************************************************/
	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		return(ret);		// 正常に送信予約されたが他のDevice又は自分がSend/Recvしている
	}
	/****************************************************************/
	/*	当該DeviceのAplWriteP : AplReadPをCheckして					*/
	/*	当該DeviceのBufferよりQueueingされているMessageを送信する	*/
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
/*	Byte数		Data内容				pri_def.hとの対応																*/
/*	---------------------------------------------------------------------------------------------------------------		*/
/*	2		0x1b, 0x40				PCMD_INIT			[ﾌﾟﾘﾝﾀの初期化]					＜ESC @＞						*/
/*	3		0x1d, 0x76, 0x00		PCMD_STATUS_AUTO	[ﾌﾟﾘﾝﾀｽﾃｰﾀｽの自動送信]			＜GS v NULL＞					*/
/*	2		0x1b, 0x76				PCMD_STATUS_SEND	[ﾌﾟﾘﾝﾀｽﾃｰﾀｽの送信]				＜ESC v＞						*/
/*	3		0x1b, 0x21, 0x00		PCMD_FONT_K_N		[漢字印字ﾓｰﾄﾞ一括指定(通常)]	＜FS ! n(=0x00)＞				*/
/*	3		0x1b, 0x33, 0xff		PCMD_KAIGYO_xx		[改行量指定]					＜ESC 3 n＞						*/
/*	3		0x1b, 0x20, 0x00		PCMD_R_SPACE		[文字の右ｽﾍﾟｰｽ量(0mm)]			＜ESC SPC n(=0x04)＞			*/
/*	3		0x1c, 0x43, 0x01		PCMD_SHIFTJIS		[漢字ｺｰﾄﾞ体系選択(ｼﾌﾄJIS)]		＜FS C n(=0x01)＞				*/
/*	3 		0x1c, 0x57, 0x00		PCMD_KANJI4_CLR		[漢字の４倍角文字(解除)]		＜FS W n(=0x00)＞				*/
/*	3 		0x1c, 0x21, 0x00		PCMD_FONT_K_N		[漢字印字ﾓｰﾄﾞ一括指定(通常)]	＜FS ! n(=0x00)＞				*/
/*	4 		0x1c, 0x53, 0x00, 0x00	PCMD_SPACE_KANJI	[漢字ｽﾍﾟｰｽ量の指定(左0mm,右0mm)]＜FS S n1(=0x00) n2(=0x04)＞	*/
/*	--------																											*/
/*	2 		0x1b, 0x40				PCMD_INIT			[ﾌﾟﾘﾝﾀの初期化]					＜ESC @＞						*/
/*	3 		0x1d, 0x76, 0x00		PCMD_STATUS_AUTO	[ﾌﾟﾘﾝﾀｽﾃｰﾀｽの自動送信]			＜GS v NULL＞					*/
/*	2 		0x1b, 0x76				PCMD_STATUS_SEND	[ﾌﾟﾘﾝﾀｽﾃｰﾀｽの送信]				＜ESC v＞						*/
/*	3 		0x1b, 0x20, 0x00		PCMD_R_SPACE		[文字の右ｽﾍﾟｰｽ量(0mm)]			＜ESC SPC n(=0x04)＞			*/
/*	3 		0x1c, 0x43, 0x01		PCMD_SHIFTJIS		[漢字ｺｰﾄﾞ体系選択(ｼﾌﾄJIS)]		＜FS C n(=0x01)＞				*/
/*	3 		0x1c, 0x57, 0x00		PCMD_KANJI4_CLR		[漢字の４倍角文字(解除)]		＜FS W n(=0x00)＞				*/
/*	3 		0x1c, 0x21, 0x00		PCMD_FONT_K_N		[漢字印字ﾓｰﾄﾞ一括指定(通常)]	＜FS ! n(=0x00)＞				*/
/*	4 		0x1c, 0x53, 0x00, 0x00	PCMD_SPACE_KANJI	[漢字ｽﾍﾟｰｽ量の指定(左0mm,右0mm)]＜FS S n1(=0x00) n2(=0x04)＞	*/
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
			I2C_BusError_Info.BIT.I2C_BFULL_SEND_RP = 1;	// BIT 8 = レシートプリンタ送信バッファフル(『I2Cﾚｼｰﾄ送信ﾊﾞｯﾌｧﾌﾙ』)		発生/解除別
		}
		ret = RIIC_RSLT_BUFF_OVER_ERROR;	// Write/Readを行う予約Bufferが一杯です
		return(ret);
	} else {
		if (I2cErrStatus.SendRP != 0) {
			I2cErrStatus.SendRP = 0;
			I2cErrEvent.SendRP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_SEND_RP = 0;	// BIT 8 = レシートプリンタ送信バッファフル(『I2Cﾚｼｰﾄ送信ﾊﾞｯﾌｧﾌﾙ』)		発生/解除別
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
	/*	今回Queue予約した送信Dataは他のDevice又は自分がSending/Receiving	*/
	/*	でなければ今回ここでDriveする										*/
	/*	であれば次に発生したIRQ5の処理にて送信開始Checkを行いDriveする		*/
	/************************************************************************/
	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		return(ret);		// 正常に送信予約されたが他のDevice又は自分がSend/Recvしている
	}
	//	上記がI2C_DN_NONなら全DeviceのStatusはIDLEのはずなのでCheckしない
	/****************************************************************/
	/*	当該DeviceのAplWriteP : AplReadPをCheckして					*/
	/*	当該DeviceのBufferよりQueueingされているMessageを送信する	*/
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
			I2C_BusError_Info.BIT.I2C_BFULL_SEND_JP = 1;	// BIT10 = ジャーナルプリンタ送信バッファフル(『I2Cｼﾞｬｰﾅﾙ送信ﾊﾞｯﾌｧﾌﾙ』)	発生/解除別
		}
		ret = RIIC_RSLT_BUFF_OVER_ERROR;	// Write/Readを行う予約Bufferが一杯です
		return(ret);
	} else {
		if (I2cErrStatus.SendJP != 0) {
			I2cErrStatus.SendJP = 0;
			I2cErrEvent.SendJP = 1;
			I2C_BusError_Info.BIT.I2C_BFULL_SEND_JP = 0;	// BIT10 = ジャーナルプリンタ送信バッファフル(『I2Cｼﾞｬｰﾅﾙ送信ﾊﾞｯﾌｧﾌﾙ』)	発生/解除別
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
	/*	今回Queue予約した送信Dataは他のDevice又は自分がSending/Receiving	*/
	/*	でなければ今回ここでDriveする										*/
	/*	であれば次に発生したIRQ5の処理にて送信開始Checkを行いDriveする		*/
	/************************************************************************/
	if (I2cAM.ActiveDevice != I2C_DN_NON) {
		return(ret);		// 正常に送信予約されたが他のDevice又は自分がSend/Recvしている
	}
	//	上記がI2C_DN_NONなら全DeviceのStatusはIDLEのはずなのでCheckしない
	/****************************************************************/
	/*	当該DeviceのAplWriteP : AplReadPをCheckして					*/
	/*	当該DeviceのBufferよりQueueingされているMessageを送信する	*/
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
//	--->	Stabで残しておく
	ret = RIIC_RSLT_OK;			// 正常終了
	return(ret);
}
/****************************************************************************/
/*																			*/
/*																			*/
/****************************************************************************/
enum I2cResultFnc_e 	writeRegSC16( 	unsigned char 	DevNo,			// SC16_I2CDEV2_1, SC16_I2CDEV2_2, SC16_RP, SC16_JP
										unsigned char	RegAdr,			// SC16内InternalRegsterAddress
										unsigned long 	WriteNum,		// Write数
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
	/*	前回送信データが完了(I2cBusFree)まで送信データの作成待ち	*/
	/****************************************************************/
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( status_wait_cnt >= 1000 ){								// 1秒間変化がない場合は抜ける
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;				// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);

	status_wait_cnt = 0;
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( status_wait_cnt >= 1000 ){								// 1秒間変化がない場合は抜ける
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;				// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_bus_status != RIIC_BUS_STATUS_FREE);
	ret = RIIC_RSLT_OK;			// 正常終了
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
									unsigned char	RegAdr,		// SC16内InternalRegsterAddress
									unsigned long 	ReadNum,	// Read数
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
enum I2cResultFnc_e 	writeRegRTC(unsigned char	RegAdr,			// SC16内InternalRegsterAddress
									unsigned long 	WriteNum,		// Write数
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
enum I2cResultFnc_e 	readRegRTC(	unsigned char	RegAdr,		// SC16内InternalRegsterAddress
									unsigned long 	ReadNum,	// Read数
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
	/*	前回送信中のDeviceが完了(I2cBusFree)しなければ今回の送信は行えない	*/
	/************************************************************************/
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( status_wait_cnt >= 1000 ){								// 1秒間変化がない場合は抜ける
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);
	
	status_wait_cnt = 0;
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( status_wait_cnt >= 1000 ){								// 1秒間変化がない場合は抜ける
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
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
		/*	今回の送信が完了(I2cBusFree)するまでAPLに結果は通知できない		*/
		/********************************************************************/
		status_wait_cnt = 0;
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if( status_wait_cnt >= 1000 ){							//	1秒間変化がない場合は抜ける
				I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
				return	RIIC_RSLT_PREV_TOUT;
			}else{
				wait2us(I2C_ONE_WAIT);								// 100us wait
				status_wait_cnt++;
			}
		}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);

		status_wait_cnt = 0;
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if( status_wait_cnt >= 1000 ){							// 1秒間変化がない場合は抜ける
				I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
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
	/*	前回送信中のDeviceが完了(I2cBusFree)しなければ今回の送信は行えない	*/
	/************************************************************************/
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( status_wait_cnt >= I2C_TOTAL_WAIT_CNT ){				//	3秒間変化がない場合は抜ける
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
			return	RIIC_RSLT_PREV_TOUT;
		}else{
			wait2us(I2C_ONE_WAIT);									// 100us wait
			status_wait_cnt++;
		}
	}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);

	status_wait_cnt = 0;
	do{		I2C_GetStatus(&wait_status, &wait_bus_status);
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( status_wait_cnt >= I2C_TOTAL_WAIT_CNT ){				//	3秒間変化がない場合は抜ける
			I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
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
		/*	今回の送信が完了(I2cBusFree)するまでAPLに結果は通知できない		*/
		/********************************************************************/
		status_wait_cnt = 0;
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if( status_wait_cnt >= I2C_TOTAL_WAIT_CNT ){			//	3秒間変化がない場合は抜ける
				I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
				return	RIIC_RSLT_PREV_TOUT;
			}else{
				wait2us(I2C_ONE_WAIT);								// 100us wait
				status_wait_cnt++;
			}
		}while(wait_status == RIIC_STATUS_ON_COMMUNICATION);

		status_wait_cnt = 0;
		do{		I2C_GetStatus(&wait_status, &wait_bus_status);
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if( status_wait_cnt >= I2C_TOTAL_WAIT_CNT ){			//	3秒間変化がない場合は抜ける
				I2C_BusError_Info.BIT.I2C_PREV_TOUT = 1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
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
		// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR以外 = Error登録を行う ---> 行わない
		return RIIC_RSLT_MODE_ERROR;
	}
	if(RIIC1.ICCR2.BIT.BBSY != 0) {
		I2C_BusError_Info.BIT.I2C_BUS_BUSY = 1;			// Bit 3 = バスBUSY						発生解除同時
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
		// RIIC_RSLT_OK & RIIC_RSLT_PRM_ERROR & RIIC_RSLT_OVER_DATA_ERROR以外 = Error登録を行う ---> 行わない
		return RIIC_RSLT_MODE_ERROR;
	}
	if((data1.RWCnt == 0) || ((data1.PreCnt == 0))) {
		return RIIC_RSLT_PRM_ERROR;
	}
	if(RIIC1.ICCR2.BIT.BBSY != 0) {
		I2C_BusError_Info.BIT.I2C_BUS_BUSY = 1;			// Bit 3 = バスBUSY						発生解除同時
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
/*		*data1 <--- iic_statusの値											*/
/*		*data2 <--- RIIC1.ICCR2.BIT.BBSYの値								*/
/****************************************************************************/
/*	wait_status																*/
/*		通信中(RIIC_STATUS_ON_COMMUNICATION)であればI2cBusFreeでないので	*/
/*		waitする(成功時はRIIC_STATUS_IDLEがwt_6, rd_9でsetされる)			*/
/*		失敗時はRIIC_STATUS_NACK, RIIC_STATUS_FAILEDがsetされる				*/
/*		* set箇所はI2c割込みHandler内である									*/
/*	wait_bus_status															*/
/*		RIIC1.ICCR2.BIT.BBSYの状態である									*/
/*			0:	RIIC_BUS_STATUS_FREE										*/
/*			1:	RIIC_BUS_STATUS_BBSY										*/
/*																			*/
/*	---->	いづれもI2c割込みHandlerがEIでないと検出できないので			*/
/*			検出期間中はI2cはEIである事										*/
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
/*		以下のLoop自体もApplicationで行う									*/
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
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
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
/*	* RequestCodeをCheckする時は[AplActiveBoxIndex-1]			*/
/*	  = [0-]なのでCurrentData(AplActiveBoxIndex-1)で行う		*/
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
		case I2C_RC_RP_RCV:	// RHRからのRead要求 ---> 当該IRQでBIT_DATA_RECEIVED_LSR=1にてReadするので不要
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
		case I2C_RC_JP_RCV:	// RHRからのRead要求 ---> 当該IRQでBIT_DATA_RECEIVED_LSR=1にてReadするので不要
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
		case I2C_RC_I2CDEV2_1_RCV:	// RHRからのRead要求 ---> 当該IRQでBIT_DATA_RECEIVED_LSR=1にてReadするので不要
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
/*	* RequestCodeをCheckする時は[AplActiveBoxIndex]				*/
/*	  = [0-]なのでNextData(AplActiveBoxIndex)で行う				*/
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
		case I2C_RC_RP_RCV:	// RHRからのRead要求 ---> 当該IRQでBIT_DATA_RECEIVED_LSR=1にてReadするので不要
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
		case I2C_RC_JP_RCV:	// RHRからのRead要求 ---> 当該IRQでBIT_DATA_RECEIVED_LSR=1にてReadするので不要
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
		case I2C_RC_I2CDEV2_1_RCV:	// RHRからのRead要求 ---> 当該IRQでBIT_DATA_RECEIVED_LSR=1にてReadするので不要
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
/*						送信再開	新送信			*/
/*		SC16_I2CDEV2_1		inx2+1:1	inx2+2:2		*/
/*		SC16_I2CDEV2_2		inx2+1:3	inx2+2:4		*/
/*		SC16_RP			inx2+1:5	inx2+2:6		*/
/*		SC16_JP			inx2+1:7	inx2+2:8		*/
/*	0xfe:	強制送信許可 & 強制中断は受付ていない	*/
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
/*		I2C_INT_MODE_EI	// I2cInterruptは割込み可能状態である	*/
/*		I2C_INT_MODE_DI	// I2cInterruptは割込み可能状態でない	*/
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
/*		送信異常時のデータを破棄すると共に、各種フラグをクリア	*/
/*		する													*/
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
	unsigned long	ist;		// 現在の割込受付状態

	ist = _di2();
	ICU.IER[8].BYTE &= 0xcf;	// ICU_IRQ4 = IER08.IEN4, ICU_IRQ5 = IER08.IEN5 <- DI
	ICU.IER[9].BYTE &= 0xfe;	// ICU_IRQ8 = IER09.IEN0 <- DI
	_ei2(ist);
}
/****************************************************************************/
void ei_I2cArbitration(void)
{
	unsigned long	ist;		// 現在の割込受付状態

	ist = _di2();
	ICU.IER[8].BYTE |= 0x30;	// ICU_IRQ4 = IER08.IEN4, ICU_IRQ5 = IER08.IEN5 <- EI
	ICU.IER[9].BYTE |= 0x01;	// ICU_IRQ8 = IER09.IEN0 <- EI
	_ei2(ist);
}

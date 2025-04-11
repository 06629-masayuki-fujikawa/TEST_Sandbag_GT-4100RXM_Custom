/************************************************************************************************/
/*																								*/
/*	ﾓｼﾞｭｰﾙ名称	:ksgRauModemControl.c	:-----------------------------------------------------: */
/*	ﾓｼﾞｭｰﾙｼﾝﾎﾞﾙ	:						:ﾓﾃﾞﾑ制御											  : */
/*	ｺﾝﾊﾟｲﾗ		:						:													  : */
/*	ﾊﾞｰｼﾞｮﾝ		:						:													  : */
/*	ﾀｰｹﾞｯﾄCPU	:						:-----------------------------------------------------: */
/*	対応機種	:						: 													  : */
/*										:-----------------------------------------------------: */
/*																								*/
/************************************************************************************************/

#include "system.h"														/*							*/
																		/*							*/
#include 	<machine.h>													/*							*/
#include 	<string.h>													/*							*/
#include 	"iodefine.h"												/* I/O定義					*/
#include	"message.h"
#include 	"ksgmac.h"													/* MAC 搭載機能ﾍｯﾀﾞ			*/
#include 	"ksgRauTable.h"												/* ﾃｰﾌﾞﾙﾃﾞｰﾀ定義			*/
#include 	"ksgRauModem.h"												/* ﾓﾃﾞﾑ関連関数郡			*/
#include 	"ksgRauModemData.h"											/* ﾓﾃﾞﾑ関連ﾃﾞｰﾀ定義			*/
#include	"ksg_def.h"
#include	"raudef.h"
// GG109800(S) A.Iiizumi 2020/01/20 LTE アマノ環境（PPP認証IDパスワード有）対応（設定で切換え可能とする36-0052④）
#include	"prm_tbl.h"	
// GG109800(E) A.Iiizumi 2020/01/20 LTE アマノ環境（PPP認証IDパスワード有）対応（設定で切換え可能とする36-0052④）
																		/*							*/
BITS	mdm_result_f;													/* ﾘｻﾞﾙﾄｺｰﾄﾞ･ﾌﾗｸﾞ			*/
#define f_OK		mdm_result_f.BIT.B0									/* OK						*/
#define f_CONN		mdm_result_f.BIT.B1									/* CONNECT					*/
#define f_NOCON		mdm_result_f.BIT.B2									/* NO CARRIER				*/
#define f_BUSY		mdm_result_f.BIT.B3									/* BUSY						*/
#define f_DELAYED	mdm_result_f.BIT.B4									/* DELAYED					*/
#define f_ERR		mdm_result_f.BIT.B5									/* ERROR					*/
#define f_RING	 	mdm_result_f.BIT.B6									/* RING						*/
#define f_ECHOBACK 	mdm_result_f.BIT.B7									/* ATE0						*/
																		/*							*/
BITS	mdm_init_f;
#define	f_TYPE_OK	mdm_init_f.BIT.B0									// 機器接続OK

extern	unsigned short	KSG_Tm_Reset_t;									/* 機器ﾘｾｯﾄ出力ﾀｲﾏ			*/
extern	unsigned short	PC_STEP;										/* 							*/
static	unsigned char	KSG_NoReciveCnt;								/* 無応答ｶｳﾝﾀ				*/

BITS	KSG_uc_mdm_res_Foma_f;
BITS	KSG_uc_mdm_ErrSndReq_f;

#define	f_resFomaRestriction 	KSG_uc_mdm_res_Foma_f.BIT.B0			/* RESTRICTION				*/
#define f_resFomaAntenaLow	 	KSG_uc_mdm_res_Foma_f.BIT.B1			/* *DANTE:0					*/
#define f_ErrSndReqERRCD		KSG_uc_mdm_ErrSndReq_f.BIT.B0			/* MODEM ERROR CD HIGH		*/
#define	f_ErrSndReqMDMOFF		KSG_uc_mdm_ErrSndReq_f.BIT.B1			/* MODEM POWER OFF Request.	*/

unsigned long  ConnTimeOutTimer;

//extern signed long	DownLoadOffDelay;									/* ﾀﾞｳﾝﾛｰﾄﾞ時の切断までのﾃﾞｨﾚｲ 100m x n */
//extern signed long	KSG_DownLoadSafeTime;								/* ｽﾀﾝﾊﾞｲﾀｲﾑｵｰﾊﾞｰﾁｪｯｸﾀｲﾏｰ */
																		/*							*/
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
extern uchar	KSG_RauBkupAPNName[32];// APNの設定値の比較用バックアップ FOMAモジュールに設定済みのデータか比較するためのもの
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御

enum FOMA_AT_FHASE {
	FOMA_AT_PHASE_PURA3 = 1,	// +++
	FOMA_AT_PHASE_ATH0,			// ATH0
	FOMA_AT_PHASE_GMM,			// AT+GMM
	FOMA_AT_PHASE_ATZ,			// ATZ
	FOMA_AT_PHASE_ATS0,			// ATS0=1
	FOMA_AT_PHASE_ATE0,			// ATE0
	FOMA_AT_PHASE_CNUM,			// AT+CNUM
	FOMA_AT_PHASE_DRPW,			// AT*DRPW
	FOMA_AT_PHASE_DANTE,		// AT*DANTE
	FOMA_AT_PHASE_CGDCONT,		// AT+CGDCONT=
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
//	FOMA_AT_PHASE_MAX			// FOMA AT初期化ｺﾏﾝﾄﾞ数
	FOMA_AT_PHASE_KPPPAUTH,		// AT*KPPPAUTH UM04対応 PPP認証方式設定
	FOMA_AT_PHASE_KALWAYSON,	// AT*KALWAYSON UM04対応 常時接続設定
	FOMA_AT_PHASE_MAX			// FOMA AT初期化ｺﾏﾝﾄﾞ数
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
};

char * const mdmFoMa_cmd[] = {
	"",					//dumy//
	"+++",				//1//
	"ATH0\r\n",			//2//
	"AT+GMM\r\n",		//3//
	"ATZ\r\n",			//4//
	"ATS0=0\r\n",		//5//
	"ATE0\r\n",			//6//
	"AT+CNUM\r\n",		//7//
	"AT*DRPW\r\n",		//8//
	"AT*DANTE\r\n",		//9//
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
//	""					//10// AT+CGDCONT
	"",					//10// AT+CGDCONT
	"AT*KPPPAUTH=1,0\r\n",//11// UM04対応 PPP認証方式設定
	"AT*KALWAYSON=1,3,0\r\n"//12// UM04対応 常時接続設定
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
};
// GG109800(S) A.Iiizumi 2020/01/20 LTE アマノ環境（PPP認証IDパスワード有）対応（設定で切換え可能とする36-0052④）
char * const mdmFoMa_cmd2[] = {
	"",					//dumy//
	"+++",				//1//
	"ATH0\r\n",			//2//
	"AT+GMM\r\n",		//3//
	"ATZ\r\n",			//4//
	"ATS0=0\r\n",		//5//
	"ATE0\r\n",			//6//
	"AT+CNUM\r\n",		//7//
	"AT*DRPW\r\n",		//8//
	"AT*DANTE\r\n",		//9//
	"",					//10// AT+CGDCONT
	"AT*KPPPAUTH=1,2\r\n",//11// UM04対応 PPP認証方式設定  認証方式CHAP
	"AT*KALWAYSON=1,3,2,\"amano\",\"amano\"\r\n"//12// UM04対応 常時接続設定 認証方式CHAP ID amano PASS amano
};
// GG109800(E) A.Iiizumi 2020/01/20 LTE アマノ環境（PPP認証IDパスワード有）対応（設定で切換え可能とする36-0052④）
const char KSG_RauMdmFmaATCGD_1[]={"AT+CGDCONT=1,\"PPP\",\""};	// Access Point Name Setting

#define	ANTENA_REQCNT_MAX		60
enum Antena_Level {
	LEVEL_0 = 0,	// アンテナレベル0
	LEVEL_1,		// アンテナレベル1
	LEVEL_2,		// アンテナレベル2
	LEVEL_MAX		// アンテナレベル3
};
static enum Antena_Level antena_level = LEVEL_MAX;
static unsigned short antena_req_cnt = 0;		// FOMAアンテナレベル要求をした回数
static unsigned short antena_req_interval = 0;	// FOMAアンテナレベル要求間隔
int		KSG_gAntLevel;							// アンテナレベル
int		KSG_gReceptionLevel;					// 受信電力指標
unsigned char		KSG_AntennaLevelCheck = 0;		// アンテナレベルチェック状態 0:未計測 1:計測中
unsigned char		KSG_AntAfterPppStarted = 0;		// アンテナレベルチェックによるE7778抑止用
unsigned short		reception_level = 0;

#define		KSG_PPP_CLOSE_WAIT_TIME		30 * 100	// アンテナレベルチェック時のPPP切断待ち時間(10ms単位)

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑｺﾝﾄﾛｰﾙ																				   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmControl();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmControl(void)
{
	unsigned char	uc_ATinitphaseMax;

	uc_ATinitphaseMax = 7;							//DOPA

	if ( KSG_uc_FomaFlag == 1 ){
		// ATｺﾏﾝﾄﾞ数(+1)
		uc_ATinitphaseMax = FOMA_AT_PHASE_MAX + 1;	//FOMA

		if ( KSG_mdm_status == 1 ){
			// ER HIGH後の DR HIGH監視.//
			if( (KSG_line_stat_h & KSG_DR_SIG) == 0 ){	// モデムがＤＲ OFF
				if ( KSG_uc_FomaWaitDRFlag == 0 ){
					KSG_uc_FomaWaitDRFlag = 1;		// 監視中 //
					KSG_f_ERDRtime_rq = KSG_CLR;
					KSG_Tm_ERDR_TIME.BIT.bit0 = (ushort)KSG_Tm_FomaMdmDRwait; // 5秒
					if (KSG_uc_AdapterType == 1) {
					// UM03-KOはもう少し起動時間が掛かる
						KSG_Tm_ERDR_TIME.BIT.bit0 = (unsigned short)-UM03KO_ER_DR_Int;
					}
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
					else if (KSG_uc_AdapterType == 2){
						// UM04-KO用の起動時間設定
						KSG_Tm_ERDR_TIME.BIT.bit0 = (unsigned short)-UM04KO_ER_DR_Int;
					}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
					KSG_f_ERDRtime_ov = KSG_CLR;
					KSG_f_ERDRtime_rq = KSG_SET;
				}
				else if ( KSG_f_ERDRtime_ov == KSG_SET ){
					KSG_f_ERDRtime_ov = KSG_CLR;
					RauSysErrSet( RAU_ERR_FMA_MODEMERR_DR, 1 );
					if ( KSG_uc_MdmInitRetry < 3 ){
						KSG_uc_MdmInitRetry++;
						KSG_uc_FomaWaitDRFlag = 0;
						KSG_RauModem_OFF_2( );
					}
				}
			}else{
				KSG_f_ERDRtime_ov = KSG_CLR;
				KSG_f_ERDRtime_rq = KSG_CLR;
				KSG_uc_FomaWaitDRFlag = 0;
				RauSysErrSet( RAU_ERR_FMA_MODEMERR_DR, 0 );
				RauSysErrSet( RAU_ERR_FMA_MODEMERR_CD, 0 );
				KSG_uc_MdmInitRetry = 0;
			}
		}
		else if (KSG_mdm_status > 1 ){
			KSG_f_ERDRtime_ov = KSG_CLR;
			KSG_f_ERDRtime_rq = KSG_CLR;
			KSG_uc_FomaWaitDRFlag = 0;
			RauSysErrSet( RAU_ERR_FMA_MODEMERR_DR, 0 );
			RauSysErrSet( RAU_ERR_FMA_MODEMERR_CD, 0 );
			KSG_uc_MdmInitRetry = 0;
		}

		if( f_ErrSndReqMDMOFF == 1 ){
			f_ErrSndReqMDMOFF = 0;
 			RauSysErrSet( RAU_ERR_FMA_MODEMPOWOFF, 0 );
		}
		if( f_ErrSndReqERRCD == 1 ){
			f_ErrSndReqERRCD = 0;
			RauSysErrSet( RAU_ERR_FMA_MODEMERR_CD, 1 );
		}

	}
	switch( KSG_mdm_status ) {											/* ﾓﾃﾞﾑｽﾃｰﾀｽによる分岐		*/
	/* --- DTE Ready ---------------------------------------------------------------------------*/
	case 1:																/*							*/
		SCI7_DTR = 0;													/* DTR ON					*/
		if( KSG_line_stat_h & KSG_DR_SIG ) {							/* DSR ON?					*/
			KSG_mdm_status = 2;											/* ⇒DTE-DCE確立			*/
			KSG_NoReciveCnt = 0;										/* 無応答ｶｳﾝﾀｸﾘｱ			*/
			KSG_f_init_repeat = 0;	f_OK = 1;							/* 初期化回数ﾘｾｯﾄ			*/
			KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;							/* ﾀｲﾏ停止					*/
			KSG_Tm_AtCom.BIT.bit0 = (ushort)KSG_Tm_AtCom_3s;			/* ⇒3秒ﾀｲﾏｾｯﾄ				*/
			KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;	KSG_Tm_AtCom.BIT.f_rq = KSG_SET;	/* ﾀｲﾏｽﾀｰﾄ					*/
			SCI7_RTS = 0; 												/* RTS ON					*/
			mdm_init_f.BYTE = 0;
		}																/*							*/
		break;															/*							*/

	/* --- DTE-DCE確立 -------------------------------------------------------------------------*/
	case 2:																/*							*/
		KSG_f_DialConn_ov = KSG_CLR;	KSG_f_DialConn_rq = KSG_CLR;	/* 回線接続待ちﾀｲﾏ停止		*/
		if( ( KSG_line_stat_h & KSG_DR_SIG ) == 0) {					/* DSR OFF?					*/
			KSG_mdm_status = 1;											/* ⇒DTE Ready				*/
			KSG_mdm_init_sts = 1;										/* ﾓﾃﾞﾑ初期化要求 			*/
			SCI7_RTS = 1; 												/* RTS OFF					*/
		}																/*							*/
		else if( KSG_mdm_init_sts < uc_ATinitphaseMax ) {				/* DSR ON? <ﾓﾃﾞﾑ初期化待ち>	*/
			SCI7_RTS = 0; 												/* RTS ON					*/
			if( KSG_f_Gurd_ov == 1 ) {									/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏ			*/
				if ( KSG_uc_FomaFlag == 1 ) {
				//==================================================================================
				//	FOMA初期化ｺﾏﾝﾄﾞ発行処理
				//
				//  OK 応答を3秒間待つ、3秒以内に OK 受信時は 1秒ｳｪｲﾄし、次のｺﾏﾝﾄﾞを送信する
				//  3秒経過しても OK 応答が無い場合はﾘﾄﾗｲし、3回ﾘﾄﾗｲ失敗で Modem Resetからやりなおす.
				//==================================================================================
					if ( KSG_uc_AtResultWait == 0 ){
						//** ｺﾏﾝﾄﾞ送信 **//
						if (KSG_mdm_init_sts < FOMA_AT_PHASE_MAX) {
							// ﾀｲﾏｵｰﾊﾞｰﾁｪｯｸ(前のｺﾏﾝﾄﾞからのｳｪｲﾄ)
							if( KSG_Tm_AtCom.BIT.f_ov == 1 ) {
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
								if( KSG_uc_AdapterType == 2){// UM04-KO
									if (KSG_mdm_init_sts == FOMA_AT_PHASE_CGDCONT) {
										if(0 == memcmp(KSG_RauBkupAPNName,KSG_RauConf.APNName,sizeof(KSG_RauBkupAPNName))) {
											// APNがモジュールに設定済みの場合、モデム初期化終了とする
											KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
											KSG_Tm_AtCom.BIT.bit0 = (ushort)-20;			// 2s timer set
											KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
											KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
											KSG_f_mdm_init_err = 0;
											mdm_result_f.BYTE = 0;
											KSG_uc_mdm_res_Foma_f.BYTE = 0;
											KSG_f_init_repeat = 0;
											KSG_mdm_init_sts = FOMA_AT_PHASE_MAX + 1;	// モデム初期化完了 //
											break;
										}
										
									}
								}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
								KSG_uc_AtResultWait = 1;					// 応答待ちｾｯﾄ //
								//	FOMA初期化ｺﾏﾝﾄﾞ
								mdm_result_f.BYTE = 0;
								KSG_uc_mdm_res_Foma_f.BYTE = 0;
																		// AT ｺﾏﾝﾄﾞ送信要求 //
								KSG_RauMdmInitCmdSetFOMA( KSG_mdm_init_sts );

								//	ﾀｲﾏ値設定
								KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
								if ( KSG_mdm_init_sts == FOMA_AT_PHASE_PURA3 ){		// +++ 必ず OK が来るわけではない,ｴｽｹｰﾌﾟ成功でOK,ｴｽｹｰﾌﾟ中は無応答 //
									f_OK = 1;
								}
								if ( KSG_mdm_init_sts == FOMA_AT_PHASE_ATH0 ){
									KSG_Tm_AtCom.BIT.bit0 = (ushort)-300;	// 30s ATH0 は CD落ちるのを待つ場合あるため長く.. //
								}else{
									KSG_Tm_AtCom.BIT.bit0 = (ushort)-30;	// 3s 応答待ち時間 //
								}
								KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
								KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
							}
						}
						else {
							KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
							KSG_Tm_AtCom.BIT.bit0 = (ushort)-20;			// 2s timer set
							KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
							KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
							KSG_f_mdm_init_err = 0;
							mdm_result_f.BYTE = 0;
							KSG_uc_mdm_res_Foma_f.BYTE = 0;
							KSG_f_init_repeat = 0;
							KSG_mdm_init_sts = FOMA_AT_PHASE_MAX + 1;	// モデム初期化完了 //
						}
					}else{
						//** 応答待ち **//
						if ( f_OK == 1 ){							// OK 受信 //

// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
							if( KSG_uc_AdapterType == 2){// UM04-KO
								if (KSG_mdm_init_sts == (FOMA_AT_PHASE_MAX - 1)) {// コマンド最後まで終了
									// UM04-KOの場合、接続先設定「AT+CGDCONT」と常時接続設定「AT*KALWAYSON」を実行後はモデムをリブートする必要がある
									memcpy ( &KSG_RauBkupAPNName, &KSG_RauConf.APNName, sizeof(KSG_RauBkupAPNName) );// APNの設定が成功したのでバックアップ
									KSG_RauModem_OFF();
								}
							}else{
								// UM03-KO以前は「AT+CGDCONT=」が最後のコマンド
								if (KSG_mdm_init_sts == (FOMA_AT_PHASE_CGDCONT)) {// コマンド最後まで終了
									KSG_mdm_init_sts = FOMA_AT_PHASE_MAX - 1;
								}
							}

// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
							if ( f_resFomaAntenaLow	== 1 ){
							/* アンテナレベルエラーは発生解除同時 */
								switch(antena_level){
								case LEVEL_0:
									RauSysErrSet( RAU_ERR_FMA_ANTENALOW, 0 );
									break;
								case LEVEL_1:
									RauSysErrSet( RAU_ERR_FMA_ANTENALOW1, 0 );
									break;
								case LEVEL_2:
									RauSysErrSet( RAU_ERR_FMA_ANTENALOW2, 0 );
									break;
								default:
									break;
								}
							}

							if (KSG_mdm_init_sts == FOMA_AT_PHASE_GMM) {
								if (f_TYPE_OK) {
									RauSysErrSet( RAU_ERR_FMA_ADAPTER_NG, 0 );		// FOMAアダプタ設定不良 解除
								}
								else {
									RauSysErrSet( RAU_ERR_FMA_ADAPTER_NG, 1 );		// FOMAアダプタ設定不良 発生
								}
							}

							KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
							KSG_Tm_AtCom.BIT.bit0 = (ushort)-12;		// 1s 次の AT 送信までのウェイト 1S保障,誤差100m +α.
							KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
							KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
							KSG_uc_AtResultWait = 0;
							KSG_uc_AtRetryCnt = 0;
							KSG_mdm_init_sts ++;						// 次の AT コマンドへ .

						}else if ( f_NOCON == 1 && KSG_mdm_init_sts == FOMA_AT_PHASE_ATH0 ){ // ATH0 --> NO CARRIER //
							KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
							KSG_Tm_AtCom.BIT.bit0 = (ushort)-12;		// 1s 次の AT 送信までのウェイト 1S保障,誤差100m +α.
							KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
							KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
							KSG_uc_AtResultWait = 0;
							KSG_uc_AtRetryCnt = 0;
							KSG_mdm_init_sts ++;						// 次の AT コマンドへ .
							RauSysErrSet( RAU_ERR_FMA_NOCARRIER, 1 );	// NO CARRIER発生

						}else if ( KSG_Tm_AtCom.BIT.f_ov == KSG_SET ){// 応答待ち Time Out //

							KSG_uc_AtResultWait = 0;

							if ( (++KSG_uc_AtRetryCnt) >= 3 ){
																	// 無応答が 3回連続. //
								KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
								KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
								if ( f_ERR == 1 && KSG_mdm_init_sts == FOMA_AT_PHASE_CNUM ){ // AT+CNUM --> ERROR //
									RauSysErrSet( RAU_ERR_FMA_SIMERROR, 1 );	// SIMカード外れ検知エラー発生
								}
								KSG_f_mdm_init_err = 1;
								f_ERR = 0;
								KSG_uc_AtRetryCnt = 0;
								KSG_RauModem_OFF();
							}else{
								KSG_Tm_AtCom.BIT.f_ov = KSG_SET;		// Retry.
							}
						}
					}
					break;		// BREAK ! //
				}
			}															/*							*/
		}																/*							*/
		//=======================================================================================
		// AT 初期化済.  発信待機
		//=======================================================================================
		else if( 	( KSG_f_mdm_init_err == 0 ) &&						/*							*/
					( KSG_Tm_AtCom.BIT.f_ov == KSG_SET ) ) {			/* ﾓﾃﾞﾑ初期化済み?			*/
			SCI7_RTS = 0; 												/* RTS ON					*/
			KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;							/* ﾀｲﾏ停止					*/
			if( ( f_NOCON == 1 ) && ( KSG_mdm_mode == 0 ) ) {			/* "NO CARRIER"				*/
				f_NOCON = 0;											/*							*/
				RauSysErrSet( RAU_ERR_FMA_NOCARRIER, 1 );				/* NO CARRIER発生			*/
			}															/*							*/
			if( KSG_f_Gurd_ov == 1 ) {									/* ｶﾞｰﾄﾞﾀｲﾏ停止中?			*/
				if( KSG_line_stat_h & KSG_CD_SIG ) {					/* ⇒CD ON?					*/
					f_CONN = 0;											/*							*/
					KSG_mdm_status = 4;									/* 通信中(着信)				*/
// MH322914 (s) kasiyama 2016/07/13 再発呼待ちタイマが正しく動いていないバグに対応する[共通バグNo.1148](MH341106)
					KSG_mdm_cut_req = 0;								/* modem切断リクエストフラグOFF */
// MH322914 (e) kasiyama 2016/07/13 再発呼待ちタイマが正しく動いていないバグに対応する[共通バグNo.1148](MH341106)
					KSG_dials.BIT.in = 1;								/* 着信						*/
				}														/*							*/
				else {													/* ⇒ﾎｽﾄ接続要求ﾌﾗｸﾞ?		*/
					KSG_RauMdmConnect();								/* ⇒ﾎｽﾄ接続要求			*/
					KSG_dials.BIT.ing = 1;								/* ﾀﾞｲｱﾙ中					*/
					KSG_f_host_conn_req = 0;							/*							*/
					ConnTimeOutTimer = KSG_Now_t;
					if( ( KSG_RauConf.Dpa_cnct_rty_cn > 0 )&&			/* リダイアル回数			*/
						( KSG_RauConf.Dpa_cnct_rty_cn < 999 ) ) {		/*							*/
						KSG_Cnt_Retry = KSG_RauConf.Dpa_cnct_rty_cn;	/* 接続ﾘﾄﾗｲｶｳﾝﾀ ｾｯﾄ			*/
					}													/*							*/
					else {												/*							*/
						KSG_Cnt_Retry = 999;							/* 接続ﾘﾄﾗｲｶｳﾝﾀ 未使用		*/
					}													/*							*/

					KSG_Tm_ReDialTime.BIT.f_rq = KSG_CLR;				/* ﾀｲﾏ停止					*/
					KSG_Tm_ReDialTime.BIT.bit0 = (ushort)-180;			// 3分 //
					KSG_Tm_ReDialTime.BIT.f_ov = KSG_CLR;				/* ｵｰﾊﾞｰﾌﾛｰ解除				*/
					KSG_Tm_ReDialTime.BIT.f_rq = KSG_SET;				/* ﾀｲﾏ開始					*/

					KSG_mdm_status = 3;									/* 接続中(発信)				*/
					KSG_uc_ConnAftWait = 0;
					RauSysErrSet( RAU_ERR_FMA_SIMERROR, 0 );			/* SIMカード外れ検知エラー解除 */
				}														/*							*/
			}															/*							*/
		}																/*							*/
		break;															/*							*/

	/* --- 接続中 ------------------------------------------------------------------------------*/
	case 3:																/*							*/
		if( KSG_line_stat_h & KSG_CD_SIG ) {							/* CD ON?					*/
			if ( KSG_uc_FomaFlag == 1 ){
				if ( KSG_uc_ConnAftWait == 0 )
				{
					// RESTRICTION 解除
					RauSysErrSet( RAU_ERR_FMA_RESTRICTION, 0 );

					KSG_uc_ConnAftWait = 1;
					KSG_Tm_AtCom.BIT.bit0 = (ushort)-6;					// 500msec+N
					KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
					KSG_Tm_AtCom.BIT.f_rq = KSG_SET;

					break;
				}else{
					if ( KSG_Tm_AtCom.BIT.f_ov != KSG_SET ){
						break;
					}
					KSG_uc_ConnAftWait = 0;								// CONNECT後ｳｴｲﾄ管理ﾌﾗｸﾞ //
				}
			}
			if( f_CONN == 1 ) {
				RauSysErrSet( RAU_ERR_FMA_NOCARRIER, 0 );				/* NO CARRIER解除			*/
			}
			f_CONN = 0;													/*							*/
			KSG_NoReciveCnt = 0;										/* 無応答ｶｳﾝﾀｸﾘｱ			*/
			KSG_mdm_status = 4;											/* 通信中(発信)				*/
			KSG_f_host_conn_req = 0;									/* ﾎｽﾄ接続要求ﾌﾗｸﾞ			*/
			if( KSG_dials.BIT.ing ) {
				KSG_dials.BYTE = 1;										/* 発信						*/
			}
			else {
				KSG_dials.BYTE = 2;										/* 着信						*/
			}
			KSG_f_DialConn_rq = KSG_CLR;	KSG_f_DialConn_ov = KSG_CLR;/* 回線接続待ちﾀｲﾏ停止		*/
// MH322914 (s) kasiyama 2016/07/13 再発呼待ちタイマが正しく動いていないバグに対応する[共通バグNo.1148](MH341106)
			KSG_mdm_cut_req = 0;										/* modem切断リクエストフラグOFF */
// MH322914 (e) kasiyama 2016/07/13 再発呼待ちタイマが正しく動いていないバグに対応する[共通バグNo.1148](MH341106)
			// PPP接続不良解除
			RauSysErrSet( RAU_ERR_FMA_PPP_TIMEOUT, 0 );
			break;														/*							*/
		}																/*							*/
		else if( ( f_BUSY == 1 ) ||										/* "BUSY"					*/
		       ( ( f_NOCON == 1 ) && ( KSG_mdm_mode == 0 ) ) ) {		/* "NO CARRIER"				*/
			if( f_BUSY ) {												/*							*/
				f_BUSY = 0;												/*							*/
			}															/*							*/
			if( f_NOCON ) {												/*							*/
				f_NOCON = 0;											/*							*/
				RauSysErrSet( RAU_ERR_FMA_NOCARRIER, 1 );				/* NO CARRIER発生			*/
			}															/*							*/
			KSG_NoReciveCnt = 0;										/* 無応答ｶｳﾝﾀｸﾘｱ			*/
			KSG_f_DialConn_rq = KSG_CLR;	KSG_f_DialConn_ov = KSG_CLR;/* 回線接続待ちﾀｲﾏ停止		*/
			KSG_dials.BYTE = 0;											/* ﾀﾞｲｱﾙｽﾃｰﾀｽﾘｾｯﾄ			*/
			if( KSG_Cnt_Retry > 0 ) {									/* ﾘﾄﾗｲ回数ﾁｪｯｸ?			*/
				KSG_f_host_conn_req = 1;								/* ⇒ﾎｽﾄ接続要求ﾌﾗｸﾞ		*/
				if( KSG_RauConf.Dpa_cnct_rty_tm > 0 ) {					/* 							*/
					KSG_Tm_ReDial.BIT.bit0 = -KSG_RauConf.Dpa_cnct_rty_tm;	/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ値ｾｯﾄ		*/
					KSG_f_ReDial_rq = KSG_SET;		KSG_f_ReDial_ov = KSG_CLR;		/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ開始		*/
				}														/*							*/
				else {													/*							*/
					KSG_Tm_ReDial.BIT.bit0 = 0;							/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ値ｾｯﾄ		*/
					KSG_f_ReDial_rq = KSG_CLR;		KSG_f_ReDial_ov = KSG_SET;		/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ開始		*/
				}														/*							*/
				if( ( KSG_RauConf.Dpa_cnct_rty_cn > 0 )&&				/* リダイアル回数			*/
						( KSG_RauConf.Dpa_cnct_rty_cn < 999 ) ) {		/*							*/
					KSG_Cnt_Retry--;									/* 接続ﾘﾄﾗｲｶｳﾝﾀ 減算		*/
				}
			}															/*							*/
			else {														/*							*/
				KSG_f_mdm_conn_err = 1;									/* ﾎｽﾄ接続ｴﾗｰ				*/
				KSG_mdm_init_sts = 0;									/* ﾓﾃﾞﾑ初期化動作停止		*/
				if ( KSG_uc_FomaFlag == 1 ){
					// リダイアル回数オーバー
					RauSysErrSet( RAU_ERR_FMA_REDIALOV, 0 );
					if ( f_resFomaRestriction == 1 ){
						f_resFomaRestriction  = 0;
						// RESTRICTION 発生
						RauSysErrSet( RAU_ERR_FMA_RESTRICTION, 1 );
					}
				}
				KSG_RauModem_OFF();										/*							*/
			}															/*							*/
		}																/*							*/
		else if( f_DELAYED == 1 ) {										/*							*/
			KSG_NoReciveCnt = 0;										/* 無応答ｶｳﾝﾀｸﾘｱ			*/
			KSG_f_DialConn_rq = KSG_CLR;	KSG_f_DialConn_ov = KSG_CLR;/* 回線接続待ちﾀｲﾏ停止		*/
			f_DELAYED = 0;												/*							*/
			KSG_dials.BIT.ing = 0;										/* ﾀﾞｲｱﾙしてない			*/
			KSG_f_host_conn_req = 1;									/* ﾎｽﾄ接続要求ﾌﾗｸﾞ			*/
			KSG_Tm_ReDial.BIT.bit0 = (ushort)-180;						/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ値ｾｯﾄ		*/
			KSG_f_ReDial_rq = KSG_SET;		KSG_f_ReDial_ov = KSG_CLR;	/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ開始		*/
		}																/*							*/
		else if(( KSG_f_ReDial_ov == 1 ) && ( KSG_f_host_conn_req == 1 )) {	/* ﾘﾀﾞｲｱﾙﾀｲﾑｵｰﾊﾞｰ			*/
			KSG_RauMdmConnect();										/* ﾎｽﾄ接続要求				*/
			KSG_dials.BIT.ing = 1;										/* ﾀﾞｲｱﾙ中					*/
			KSG_f_host_conn_req = 0;									/*							*/
		}																/*							*/
		if( ( KSG_line_stat_h & KSG_DR_SIG ) == 0 ) {					/* DSR OFF?					*/
			KSG_mdm_status = 1;											/* ⇒DTE Ready				*/
			SCI7_RTS = 1; 												/* RTS OFF					*/
			KSG_dials.BIT.ing = 0;										/* ﾀﾞｲｱﾙしてない			*/
			KSG_f_host_conn_req = 0;									/* ﾎｽﾄ接続要求ﾌﾗｸﾞ			*/
			KSG_f_ReDial_rq = KSG_CLR;		KSG_f_ReDial_ov = KSG_SET;	/* ﾘﾀﾞｲｱﾙﾀｲﾏ停止			*/
			KSG_f_DialConn_rq = KSG_CLR;	KSG_f_DialConn_ov = KSG_CLR;/* 回線接続待ちﾀｲﾏ停止		*/
		}																/*							*/
		if( KSG_f_DialConn_ov == 1 ) {									/* 回線接続待ちﾀｲﾏｵｰﾊﾞｰ		*/
			KSG_NoReciveCnt++;											/* 無応答ｶｳﾝﾄｱｯﾌﾟ			*/
			if( KSG_NoReciveCnt > KSG_RauConf.Dpa_cnct_rty_cn ){		/* 無応答ｶｳﾝﾄｵｰﾊﾞｰ			*/
				KSG_f_mdm_conn_err = 1;									/* ﾎｽﾄ接続ｴﾗｰ				*/
				KSG_mdm_init_sts = 0;									/* ﾓﾃﾞﾑ初期化動作停止		*/
				KSG_RauModem_OFF();										/*							*/
			}															/*							*/
			else {														/*							*/
				KSG_f_mdm_conn_err = 1;									/* ⇒ﾎｽﾄ接続ｴﾗｰ				*/
				KSG_mdm_status = 2;										/* DTE-DCE確立				*/
				KSG_dials.BIT.ing = 0;									/* ﾀﾞｲｱﾙしてない			*/
				KSG_f_host_conn_req = 0;								/* ﾎｽﾄ接続要求ﾌﾗｸﾞ			*/
			}															/*							*/
			KSG_f_ReDial_rq = KSG_CLR;		KSG_f_ReDial_ov = KSG_SET;	/* ﾘﾀﾞｲｱﾙﾀｲﾏ停止			*/
		}																/*							*/
		if ( KSG_GetInterval( ConnTimeOutTimer ) > (70L * 1000L) ){
			// PPP接続不良発生
			RauSysErrSet( RAU_ERR_FMA_PPP_TIMEOUT, 1 );
		}
		if ( KSG_f_ReDialTime_ov == 1 ){
			KSG_Tm_ReDialTime.tm = 0;
		}
		break;															/*							*/

	/* --- 通信中 ------------------------------------------------------------------------------*/
	case 4:																/*							*/
		KSG_Tm_ReDialTime.tm = 0;
		KSG_Cnt_Retry =0;												/* 接続ﾘﾄﾗｲｶｳﾝﾀ ﾘｾｯﾄ		*/
		KSG_NoReciveCnt = 0;											/* 無応答ｶｳﾝﾀｸﾘｱ			*/
		KSG_f_ReDial_rq = KSG_CLR;	KSG_f_ReDial_ov = KSG_SET;			/* ﾘﾀﾞｲｱﾙﾀｲﾏ停止			*/
		KSG_f_mdm_conn_err = 0;											/* ﾎｽﾄ接続ｴﾗｰ解除			*/
		KSG_RauModemOnline = 1;
		if( ( KSG_line_stat_h & KSG_CD_SIG ) == 0 ) {					/* 回線切断					*/
			KSG_mdm_status = 2;											/* DTE-DCE確立				*/
			KSG_RauModemOnline = 0;										/* ﾎｽﾄ回線状態ﾌﾗｸﾞ 切断		*/
			KSG_dials.BIT.out = KSG_dials.BIT.in = 0;					/* ﾀﾞｲｱﾙｽﾃｰﾀｽ ｸﾘｱ			*/
			KSG_f_init_repeat = 0;										/* 初期化回数 ﾘｾｯﾄ			*/
			f_OK = 1;													/*							*/
			KSG_Tm_AtCom.BIT.f_ov = KSG_SET;							/*							*/
			KSG_mdm_mode = 0;											/* ｺﾏﾝﾄﾞﾓｰﾄﾞ				*/
			KSG_Tm_Gurd.BIT.bit0 = (ushort)KSG_Tm_Gurd_COUNT;			/* ﾀﾞｲｱﾙ抑制ﾀｲﾏ値ｾｯﾄ		*/
			KSG_f_Gurd_rq = KSG_SET;	KSG_f_Gurd_ov = KSG_CLR;		/* ﾀﾞｲｱﾙ抑制ﾀｲﾏ開始			*/
			KSG_ClosePPPSession(gInterfaceHandle_PPP_rau);
		}																/*							*/
		if( ( KSG_line_stat_h & KSG_DR_SIG ) == 0 ) {					/* DSR OFF					*/
			KSG_RauModemOnline = 0;										/* ﾎｽﾄ回線状態ﾌﾗｸﾞ 切断		*/
			KSG_dials.BIT.out = KSG_dials.BIT.in = 0;					/* ﾀﾞｲｱﾙｽﾃｰﾀｽ ｸﾘｱ			*/
			KSG_mdm_status = 1;											/* DTE Ready				*/
			SCI7_RTS = 1; 												/* RTS OFF					*/
			KSG_Tm_Gurd.BIT.bit0 = (ushort)KSG_Tm_Gurd_COUNT;			/* ﾀﾞｲｱﾙ抑制ﾀｲﾏ値ｾｯﾄ		*/
			KSG_f_Gurd_rq = KSG_SET;	KSG_f_Gurd_ov = KSG_CLR;		/* ﾀﾞｲｱﾙ抑制ﾀｲﾏ開始			*/
			KSG_ClosePPPSession(gInterfaceHandle_PPP_rau);
		}																/*							*/
		break;															/*							*/

	case 10:															/* ｱﾝﾃﾅﾚﾍﾞﾙ、受信電力指標取得 */
		KSG_RauMdmAntLevMode();
		break;
	case 11:
		break;
	/* --- その他 ------------------------------------------------------------------------------*/
	default:															/*							*/
		if( KSG_f_Gurd_ov == 1 ) {										/*							*/
			if( KSG_mdm_init_sts > 0 ) KSG_RauModem_ON();				/* ﾓﾃﾞﾑ初期化動作中			*/
		}																/*							*/
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}																	/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	アンテナレベル、受信電力指数取得														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmAntLevMode();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void KSG_RauMdmAntLevMode( void )
{
	// 
	switch( KSG_mdm_ant_sts ){
	case 1:	//+++
	case 2: //ATH
	case 3: //AT*DRPW
	case 4: //AT*DANTE
		if( KSG_uc_AtResultWait == 0 && KSG_Tm_AtCom.BIT.f_ov == 1 ) {
			KSG_uc_AtResultWait = 1;
			//	初期化ｺﾏﾝﾄﾞ
			mdm_result_f.BYTE = 0;
			KSG_RauMdmAntLevelCmdSetFOMA( KSG_mdm_ant_sts );
			//	ﾀｲﾏ値設定
			KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
			if(KSG_mdm_ant_sts == 2) {
				KSG_Tm_AtCom.BIT.bit0 = (ushort)-400;		// 40s timer set.For Result Wait.
			}
			else {
				KSG_Tm_AtCom.BIT.bit0 = (ushort)-30;		// 3s timer set.For Result Wait.
			}
			KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
			KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
		}
		if ( KSG_uc_AtResultWait == 1 && (f_OK == 1 || f_NOCON == 1)){
			// 応答待ちで OK 受信
			KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
			KSG_Tm_AtCom.BIT.bit0 = (ushort)-1;
			KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
			KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
			KSG_uc_AtResultWait = 0;
			KSG_uc_AtRetryCnt = 0;
			KSG_mdm_ant_sts ++;
		}
		if ( KSG_uc_AtResultWait == 1 && KSG_Tm_AtCom.BIT.f_ov == KSG_SET ){ // 応答待ち Time Out //
			KSG_uc_AtResultWait = 0;
			if(KSG_mdm_ant_sts != 1){
				KSG_mdm_ant_sts = 1;
			}
			else{
				KSG_mdm_ant_sts = 3;
			}
			KSG_Tm_AtCom.BIT.f_ov = KSG_SET;		// Retry from 1 //
		}
		break;
	case 5:
		KSG_gAntLevel = antena_level;
		KSG_gReceptionLevel = reception_level;
		KSG_mdm_ant_sts = 3;
		queset( OPETCBNO, KSG_ANT_EVT, 0, NULL );
		KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
		KSG_Tm_AtCom.BIT.bit0 = (ushort)antena_req_interval;
		KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
		KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
		break;
	default:
		break;
	}
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾘｻﾞﾙﾄｺｰﾄﾞﾁｪｯｸ																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmResultCheck();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmResultCheck(void)
{																		/*							*/

if (KSG_rx_save_len > 1) {												/* 受信ﾃﾞｰﾀ有り				*/
		KSG_mdm_r_buf[ KSG_rx_save_len ] = 0;							/*							*/
		if		(	( KSG_mdm_r_buf[0] == 'O' ) &&						/* "OK"?					*/
					( KSG_mdm_r_buf[1] == 'K' ) ){						/*							*/
			f_OK = 1;													/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'C' ) &&						/* "CONN"?					*/
					( KSG_mdm_r_buf[1] == 'O' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'N' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'N' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_CONN = 1;													/*							*/
			KSG_mdm_mode = 1;											/* 接続完了ﾓｰﾄﾞ				*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'B' ) &&						/* "BUSY"?					*/
					( KSG_mdm_r_buf[1] == 'U' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'S' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'Y' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_BUSY = 1;													/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'D' ) &&						/* "DELA"?					*/
					( KSG_mdm_r_buf[1] == 'E' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'L' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'A' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_DELAYED = 1;												/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'N' ) &&						/* "NO"?					*/
					( KSG_mdm_r_buf[1] == 'O' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_NOCON = 1;												/*							*/
			KSG_mdm_mode = 0;											/* ｺﾏﾝﾄﾞﾓｰﾄﾞ				*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'E' ) &&						/* "ERR"?					*/
					( KSG_mdm_r_buf[1] == 'R' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'R' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_ERR = 1;													/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == '+' ) &&						/* "AT+CNUM"?				*/
					( KSG_mdm_r_buf[1] == 'C' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'N' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'U' ) &&						/*							*/
					( KSG_mdm_r_buf[4] == 'M' ) &&						/*							*/
					( KSG_mdm_r_buf[5] == ':' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_ERR = 0;													/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'A' ) &&						/* "ATE0"?					*/
					( KSG_mdm_r_buf[1] == 'T' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'E' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == '0' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_ECHOBACK = 1;												/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'R' ) &&						/* "RING"?					*/
					( KSG_mdm_r_buf[1] == 'I' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'N' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'G' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_RING  = 1;												/*							*/
		}																/*							*/
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_mdm_r_buf[0] == 'R' ) &&		// RESTRICTION //
				 ( KSG_mdm_r_buf[1] == 'E' ) &&
				 ( KSG_mdm_r_buf[2] == 'S' ) &&
				 ( KSG_mdm_r_buf[3] == 'T' ) &&
				 ( KSG_mdm_r_buf[4] == 'R' ) &&
				 ( KSG_mdm_r_buf[5] == 'I' ) &&
				 ( KSG_mdm_r_buf[6] == 'C' ) &&
				 ( KSG_mdm_r_buf[7] == 'T' ) &&
				 ( KSG_mdm_r_buf[8] == 'I' ) &&
				 ( KSG_mdm_r_buf[9] == 'O' ) &&
				 ( KSG_mdm_r_buf[10]== 'N' ) ){

			mdm_result_f.BYTE = 0x00;
			f_NOCON = 1;
			f_resFomaRestriction = 1;
			KSG_mdm_mode = 0;							  // コマンドモード
		}
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_rx_save_len >= (4+2) ) &&
				 ( KSG_mdm_r_buf[0] == 'F' ) &&
				 ( KSG_mdm_r_buf[1] == 'O' ) &&
				 ( KSG_mdm_r_buf[2] == 'M' ) &&
				 ( KSG_mdm_r_buf[3] == 'A' )){
			if (KSG_rx_save_len >= (4+5+2) &&
				 ( KSG_mdm_r_buf[4] == ' ' ) &&
				 ( KSG_mdm_r_buf[5] == 'U' ) &&
				 ( KSG_mdm_r_buf[6] == 'M' ) &&
				 ( KSG_mdm_r_buf[7] == '0' )){
				if (KSG_uc_AdapterType == 0 &&
					KSG_mdm_r_buf[8] == '2') {
					f_TYPE_OK = 1;
				}
				else if (KSG_uc_AdapterType == 1 &&
					KSG_mdm_r_buf[8] == '3') {
					f_TYPE_OK = 1;
				}
			}
		}
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_uc_AdapterType == 2 ) && // UM04-KO
				 ( KSG_mdm_r_buf[0] == 'U' ) &&
				 ( KSG_mdm_r_buf[1] == 'M' ) &&
				 ( KSG_mdm_r_buf[2] == '0' ) &&
				 ( KSG_mdm_r_buf[3] == '4' ) &&
				 ( KSG_mdm_r_buf[4] == '-' ) &&
				 ( KSG_mdm_r_buf[5] == 'K' ) &&
				 ( KSG_mdm_r_buf[6] == 'O' ) ){
					f_TYPE_OK = 1;
		}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_rx_save_len >= (7+2) ) &&
				 ( KSG_mdm_r_buf[0] == '*' ) &&
				 ( KSG_mdm_r_buf[1] == 'D' ) &&
				 ( KSG_mdm_r_buf[2] == 'A' ) &&
				 ( KSG_mdm_r_buf[3] == 'N' ) &&
				 ( KSG_mdm_r_buf[4] == 'T' ) &&
				 ( KSG_mdm_r_buf[5] == 'E' ) &&
				 ( KSG_mdm_r_buf[6] == ':' )){

			// UM03-KOの場合、コロンの後にスペースが来るので読み捨てる
		 	if (KSG_mdm_r_buf[7] == ' ') {
		 		KSG_mdm_r_buf[7] = KSG_mdm_r_buf[8];
		 		KSG_rx_save_len--;					// 長さを調整する
		 	}
			if ( KSG_rx_save_len == (7+2) ){		// "*DANTE:CrLf" = 0 扱いとする //
				KSG_mdm_r_buf[7] = '0';
			}
			KSG_RauAntenaLevelSet();
		}
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_rx_save_len >= (6+2) ) &&
				 ( KSG_mdm_r_buf[0] == '*' ) &&
				 ( KSG_mdm_r_buf[1] == 'D' ) &&
				 ( KSG_mdm_r_buf[2] == 'R' ) &&
				 ( KSG_mdm_r_buf[3] == 'P' ) &&
				 ( KSG_mdm_r_buf[4] == 'W' ) &&
				 ( KSG_mdm_r_buf[5] == ':' )){

			// UM03-KOの場合、コロンの後にスペースが来るので読み捨てる
		 	if ( KSG_mdm_r_buf[6] == ' ' ) {
	 			KSG_mdm_r_buf[6] = KSG_mdm_r_buf[7];
	 			KSG_mdm_r_buf[7] = KSG_mdm_r_buf[8];
		 		KSG_rx_save_len--;					// 長さを調整する
		 	}
			if ( KSG_rx_save_len == (6+2) ){		// "*DRPW:CrLf" = 0 扱いとする //
				KSG_mdm_r_buf[6] = '0';
				KSG_mdm_r_buf[7] = '0';
				// アンテナレベルエラー(E7735,E7736,E7779)に付加する電力指数値が取得できない場合はFFをセット
				if( KSG_mdm_status != 10 ) {
					// メンテナンスからのアンテナレベル取得中でなければFFをセット
					reception_level = 0xFF;
					return;
				}
			}
			if ( KSG_rx_save_len == (7+2) ){
				KSG_mdm_r_buf[7] = KSG_mdm_r_buf[6];
				KSG_mdm_r_buf[6] = '0';
			}
			KSG_RauReceptionLevelSet();
		}
	}																	/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	FOMAアンテナレベル取得																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauAntenaLevelSet();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void KSG_RauAntenaLevelSet(void)
{

	f_resFomaAntenaLow = 1;

	if ( KSG_mdm_r_buf[7] == '0' ){	// アンテナレベル０
		antena_level = LEVEL_0;
		antena_req_cnt = 0;
	}
	else if ( KSG_mdm_r_buf[7] == '1' ){	// アンテナレベル１
		antena_level = LEVEL_1;
		antena_req_cnt = 0;
	}
	else if ( KSG_mdm_r_buf[7] == '2' ){	// アンテナレベル２
		antena_level = LEVEL_2;
		antena_req_cnt = 0;
	}
	else if ( KSG_mdm_r_buf[7] == '3' ){	// アンテナレベル３
		antena_level = LEVEL_MAX;
		f_resFomaAntenaLow = 0;
		antena_req_cnt = 0;
	}
	else{
		// 電源投入直後にDANTEコマンドを実行するとリザルトの数字(0～3)がセットされないことがある
		/* 	UM02-Fモデムは、FOMA圏内に入ってから20～30秒程度経過しないとDANTEコマンドに対するアンテナレベル値をセットしなかった。
			そこで、余裕を見て倍の約60秒間、アンテナレベルがセットされるのを待つようにした。*/
		if(++antena_req_cnt >= ANTENA_REQCNT_MAX ){
			antena_level = LEVEL_0;		// 60回DANTEコマンドを実行してもアンテナレベルを取得できない場合は圏外と見なす
			antena_req_cnt = 0;
		}else{
			f_resFomaAntenaLow = 0;
		}
	}
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	FOMA受信電力指標取得																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauReceptionLevelSet();												   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void KSG_RauReceptionLevelSet(void)
{
	unsigned short reception1;
	unsigned short reception2;

	if( (KSG_mdm_r_buf[6] >= '0') &&
		(KSG_mdm_r_buf[6] <= '9') &&
		(KSG_mdm_r_buf[7] >= '0') &&
		(KSG_mdm_r_buf[7] <= '9') ){
		reception1 = KSG_mdm_r_buf[6] - 0x30;
		reception2 = KSG_mdm_r_buf[7] - 0x30;

		reception_level = (reception1*10) + reception2;
		antena_req_cnt = 0;
	}
	else{
		// 電源投入直後にDRPWコマンドを実行するとリザルトの数字がセットされない対策
		if(++antena_req_cnt >= ANTENA_REQCNT_MAX ){
			reception_level = 0;		// 60回DRPWコマンドを実行しても受信電力指標が取得できない場合はレベル0と見なす
			antena_req_cnt = 0;
		}
	}
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑ接続要求処理																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModem_ON();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauModem_ON(void)
{																		/*							*/
	KSG_code_sts = 0x00;												/*							*/
	KSG_ptr_rx_save = &KSG_mdm_r_buf[0];								/* 受信ﾃﾞｰﾀ格納				*/
	KSG_rx_save_len = 0;												/* 受信ﾃﾞｰﾀ格納数ｸﾘｱ		*/
	SCI7_DTR = 0;														/* ER(DTR) ON				*/
	KSG_mdm_status = 1;													/* DTE Ready				*/
//	if ( KSG_RauConf.ConnectFlag == 1 ) {
//		KSG_RauConf.ConnectFlag = 0;									// '接続なし' は 'する'に変える
//		KSG_mdm_status = 5;												// Standby
//		KSG_DownLoadStandbyFlag = 5;									// ｽﾀﾝﾊﾞｲ
//		//ｽﾀﾝﾊﾞｲが n 分継続したら通常に戻す//
//		KSG_DownLoadSafeTime = (long)((long)KSG_RauConf.ExitStandbySafeTime * 10L * 60L);
//	}
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑ切断要求処理																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModem_OFF();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauModem_OFF(void)
{																		/*							*/
	if ( KSG_uc_FomaFlag == 1 ){
		RAUhost_TcpConnReq_Clear ( );
		KSG_RauModem_OFF_2 ( );
		return;
	}
	SCI7_RTS = 1;														/* RS(RTS) OFF				*/
	SCI7_DTR = 1;														/* ER(DTR) OFF				*/
	KSG_mdm_init_sts = 0;												/* ﾓﾃﾞﾑ初期化動作停止		*/
	KSG_modem_req = 0;													/* modem接続ﾘｸｴｽﾄﾌﾗｸﾞ		*/
	KSG_mdm_status = 0;													/* DTE not Ready			*/
	KSG_RauModemOnline = 0;												/* ﾎｽﾄ回線状態ﾌﾗｸﾞ 切断		*/
	KSG_dials.BYTE = 0;													/* ﾀﾞｲｱﾙｽﾃｰﾀｽ ﾘｾｯﾄ			*/
	KSG_RauMdmSndQueInit();												/*							*/
	KSG_Tm_Gurd.BIT.bit0 = (ushort)KSG_Tm_Gurd_COUNT;					/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏ設定		*/
	KSG_f_Gurd_rq = KSG_SET;	KSG_f_Gurd_ov = KSG_CLR;				/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏ開始		*/
	KSG_Tm_Reset_t = 50;												/*							*/

}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑ切断要求処理-2																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModem_OFF_2();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauModem_OFF_2(void)
{																		/*							*/
	SCI7_RTS = 1;														/* RS(RTS) OFF				*/
	SCI7_DTR = 1;														/* ER(DTR) OFF				*/
	KSG_mdm_init_sts = 0;												/* ﾓﾃﾞﾑ初期化動作停止		*/
	KSG_modem_req = 0;													/* modem接続ﾘｸｴｽﾄﾌﾗｸﾞ(未使用)*/
	KSG_mdm_status = 0;													/* DTE not Ready			*/
	KSG_uc_AtResultWait = 0;											/* AT Restult Wait Flag Clear */
	KSG_RauModemOnline = 0;												/* ﾎｽﾄ回線状態ﾌﾗｸﾞ 切断		*/
	KSG_dials.BYTE = 0;													/* ﾀﾞｲｱﾙｽﾃｰﾀｽ ﾘｾｯﾄ			*/
	KSG_RauMdmSndQueInit();												/*							*/

	KSG_uc_FomaPWRphase = 1;											// (1)CD OFF 監視中 //
	KSG_Tm_Reset_t = KSG_FomaModemPwrPhase1;							// 60秒後に CD OFF 確認 //
	f_ErrSndReqERRCD = 0;
	f_ErrSndReqMDMOFF = 0;
	KSG_Tm_Gurd.tm = 0;													// T_ResetOutFOMA() でﾀｲﾏｰｾｯﾄ //
	KSG_uc_FomaWaitDRFlag = 0;		// DR監視していない

	return;
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑ初期化要求処理																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmInitReq();															   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmInitReq(void)
{																		/*							*/
	KSG_mdm_init_sts = 1; 												/* ﾓﾃﾞﾑ初期化要求			*/
	KSG_modem_req = 2;													/* modem接続ﾘｸｴｽﾄﾌﾗｸﾞ		*/
}																		/*							*/


//---------------------------------------------------------------------------------------------
// TelNum の 数値のみセットする.
//---------------------------------------------------------------------------------------------
short KSG_RauMdmInitCmdSetFOMA_SetTelNum ( unsigned char *setBuf, unsigned char *TelNum, int len )
{
	short i,s;
	for ( i=s=0; i < len; i++ ){
		if ( TelNum[i] >= '0' && TelNum[i] <= '9' ){
			setBuf[s++] = TelNum[i];
		}
	}
	return s;
}
//---------------------------------------------------------------------------------------------
// 8Bit x 4 で格納されれいる IP Addressを文字列にして　. を挿入して格納する.
//---------------------------------------------------------------------------------------------
void KSG_RauMdmInitCmdSetFOMA_GetIpAdrStr ( unsigned char *setBuf, unsigned char *IPdat )
{
	//012.456.890.234
	intoas ( &setBuf[0], IPdat[0], 3 );
	setBuf[3] = '.';
	intoas ( &setBuf[4], IPdat[1], 3 );
	setBuf[7] = '.';
	intoas ( &setBuf[8], IPdat[2], 3 );
	setBuf[11] = '.';
	intoas ( &setBuf[12], IPdat[3], 3 );
}
//---------------------------------------------------------------------------------------------
// 12桁の IP 文字列に . を挿入してセットする.
//---------------------------------------------------------------------------------------------
void KSG_RauMdmInitCmdSetFOMA_SetIpAdr ( unsigned char *setBuf, unsigned char *IPbuf )
{
	setBuf[0] = IPbuf[0];
	setBuf[1] = IPbuf[1];
	setBuf[2] = IPbuf[2];
	setBuf[3] = '.';
	setBuf[4] = IPbuf[3];
	setBuf[5] = IPbuf[4];
	setBuf[6] = IPbuf[5];
	setBuf[7] = '.';
	setBuf[8] = IPbuf[6];
	setBuf[9] = IPbuf[7];
	setBuf[10] = IPbuf[8];
	setBuf[11] = '.';
	setBuf[12] = IPbuf[9];
	setBuf[13] = IPbuf[10];
	setBuf[14] = IPbuf[11];
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑ初期化ｺﾏﾝﾄﾞ送信処理																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmInitCmdSetFOMA();												   |*/
/*| PARAMETER1  :   signed short InitPhase	:	初期化ﾌｪｰｽﾞ									   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmInitCmdSetFOMA( signed short InitPhase )
{																		/*							*/

	unsigned char		of;

	switch( InitPhase ) {												/*							*/
		default:
// GG109800(S) A.Iiizumi 2020/01/20 LTE アマノ環境（PPP認証IDパスワード有）対応（設定で切換え可能とする36-0052④）
//			KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );
//			// memcpyでいいのか？
//			memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			if (prm_get(COM_PRM, S_CEN, 52, 1, 3) == 1) {// 裏設定 36-0052④ PPP認証IDパスワード有
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd2[InitPhase] );
				memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd2[InitPhase], KSG_mdm_cmd_len );
			} else {// 本環境動作
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );
				memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			}
// GG109800(E) A.Iiizumi 2020/01/20 LTE アマノ環境（PPP認証IDパスワード有）対応（設定で切換え可能とする36-0052④）
			break;
		case FOMA_AT_PHASE_CNUM:		// AT+CNUM
			if ( KSG_RauConf.Dpa_IP_h.SEG[0] == 0 &&
				 KSG_RauConf.Dpa_IP_h.SEG[1] == 0 &&
				 KSG_RauConf.Dpa_IP_h.SEG[2] == 0 &&
				 KSG_RauConf.Dpa_IP_h.SEG[3] == 0 &&
				 KSG_RauConf.Dpa_port_h == 0 ){
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );	// AT&P
				memcpy( &KSG_mdm_cmd_buf[0], mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
				KSG_mdm_init_sts = FOMA_AT_PHASE_MAX - 1;			// 終わらせる //
				break;
			}
			KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );	// AT&P
			memcpy( &KSG_mdm_cmd_buf[0], mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			break;
		case FOMA_AT_PHASE_CGDCONT:
				//AT+CGDCONT=1,"PPP","//
				memcpy( &KSG_mdm_cmd_buf[0], KSG_RauMdmFmaATCGD_1, sizeof( KSG_RauMdmFmaATCGD_1 ) );
				of = KSG_RauConf.APNLen;
				memcpy ( &KSG_mdm_cmd_buf[20], &KSG_RauConf.APNName[0], of ); of += 20;
				memcpy ( &KSG_mdm_cmd_buf[of], "\"\r\n", 3 ); of += 3;
				KSG_mdm_cmd_len = of;
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
//				KSG_mdm_init_sts = FOMA_AT_PHASE_MAX - 1;			// 終わらせる //
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
			break;
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
		case FOMA_AT_PHASE_KALWAYSON:// UM04-KO固有処理
// GG109800(S) A.Iiizumi 2020/01/20 LTE アマノ環境（PPP認証IDパスワード有）対応（設定で切換え可能とする36-0052④）
//			KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );
//			memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			if (prm_get(COM_PRM, S_CEN, 52, 1, 3) == 1) {// 裏設定 36-0052④ PPP認証IDパスワード有
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd2[InitPhase] );
				memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd2[InitPhase], KSG_mdm_cmd_len );
			} else {// 本環境動作
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );
				memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			}
// GG109800(E) A.Iiizumi 2020/01/20 LTE アマノ環境（PPP認証IDパスワード有）対応（設定で切換え可能とする36-0052④）
			KSG_mdm_init_sts = FOMA_AT_PHASE_MAX - 1;//(FOMA_AT_PHASE_KALWAYSONが最後のコマンド)
			break;
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
	}
	KSG_f_mdm_kind_err = 0;												/*							*/
	KSG_RauMdmTxStart();												/*							*/
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ｱﾝﾃﾅﾚﾍﾞﾙ、受信電力指数取得ｺﾏﾝﾄﾞ送信処理													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmAntLevelCmdSetFOMA();											   |*/
/*| PARAMETER1  :   signed short phase	:	ﾌｪｰｽﾞ											   |*/
/*[]------------------------------------------------------------------------------------------[]*/
const char *mdmFoMa_AntLevCmd[5] = {
	"",
	"+++",
	"ATH0\r\n",		// 回線切断
	"AT*DRPW\r\n",	// 受信電力指数取得
	"AT*DANTE\r\n"	// アンテナレベル取得
};

void	KSG_RauMdmAntLevelCmdSetFOMA( signed short phase )
{
	KSG_mdm_cmd_len = strlen( mdmFoMa_AntLevCmd[phase] );
	memcpy( &KSG_mdm_cmd_buf, mdmFoMa_AntLevCmd[phase], KSG_mdm_cmd_len );
	KSG_f_mdm_kind_err = 0;
	KSG_RauMdmTxStart();

}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信開始処理																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmTxStart();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmTxStart( void )
{																		/*							*/
	KSG_mdm_cmd_buf[KSG_mdm_cmd_len] = 0;								/*							*/
	KSG_mdm_cmd_ptr = 0;												/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信ﾃﾞｰﾀ 先頭	*/
	SCI7.TDR = KSG_mdm_cmd_buf[KSG_mdm_cmd_ptr];						/* 1 byte送信				*/
	KSG_mdm_cmd_ptr++;													/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信ﾃﾞｰﾀ 加算	*/
	SCI7.SCR.BIT.TIE = 1;												/* 送信ﾃﾞｰﾀ空き割込み許可	*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾎｽﾄ接続要求処理																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmConnect();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmConnect( void )
{
	if ( KSG_uc_FomaFlag == 1 ){
		strcpy((char *)&KSG_mdm_cmd_buf[0], "ATD*99***1#\r\n");
		KSG_mdm_cmd_len = strlen( (char*)KSG_mdm_cmd_buf );
		KSG_mdm_cmd_ptr = 0x00;
		f_resFomaRestriction = 0;

		KSG_RauMdmTxStart();			// モデムコマンド送信開始

		// 回線接続待ちタイマ設定
		if ( KSG_RauConf.Dpa_dial_wait_tm ){
			KSG_Tm_DialConn.BIT.bit0 = -( KSG_RauConf.Dpa_dial_wait_tm * 10 );/* ダイアル応答待ちﾀｲﾏ設定		*/
		}else{
			KSG_Tm_DialConn.BIT.bit0 = (ushort)-1250;					// 125 S // 2008/11/19.
		}
		KSG_f_DialConn_rq = KSG_SET;
		KSG_f_DialConn_ov = KSG_CLR;
		return;
	}
//	strcpy((char *)&KSG_mdm_cmd_buf[0], "ATD");							/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ転送			*/
//	memcpy( &KSG_mdm_cmd_buf[3], &KSG_RauActTBL.TelNumber[0], 12 );		/* HOST電話番号				*/
//	KSG_mdm_cmd_buf[15] = KSG_CR;	KSG_mdm_cmd_buf[16] = KSG_LF;		/* CR/LFの付加				*/
//	KSG_mdm_cmd_len = 17;												/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信ﾃﾞｰﾀ長ｾｯﾄ	*/
//	KSG_mdm_cmd_ptr = 0;												/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信ﾃﾞｰﾀ先頭	*/
//	KSG_RauMdmTxStart();												/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信開始		*/
//	if( KSG_RauActTBL.DialWait > 0 ) {									/*							*/
//		KSG_Tm_DialConn.BIT.bit0 = -( KSG_RauActTBL.DialWait * 10 );	/* 回線接続待ちﾀｲﾏ設定		*/
//		KSG_f_DialConn_rq = KSG_SET;	KSG_f_DialConn_ov = KSG_CLR;	/*							*/
//	}																	/*							*/
//	else {																/*							*/
//		KSG_Tm_DialConn.tm = 0;											/*							*/
//	}																	/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	RS232C制御信号 ﾁｬﾀ取りｴﾘｱ初期化処理														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSigInfoInit();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmSigInfoInit( void )
{																		/*							*/
	KSG_DSR = 1;														/* DSR off					*/
	KSG_CD  = 1;														/* CD  off					*/
	KSG_CTS = 1;
	KSG_CI = 1;
	KSG_Chatt_DSR = 0xFF;
	KSG_Chatt_CTS = 0xFF;
	KSG_Chatt_CD = 0xFF;
	KSG_Chatt_CI = 0xFF;
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	機器ﾘｾｯﾄ出力処理																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauTResetOut();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauTResetOut( void )
{																		/*							*/
	if ( KSG_uc_FomaFlag == 1 ){
		KSG_RauTResetOutFOMA();
		return;
	}
	if( KSG_Tm_Reset_t != 0x00 ) {										/*							*/
		KSG_Tm_Reset_t--;												/*							*/
		if( KSG_Tm_Reset_t == 0x00 ) {
			SCI7_RST = 0;												/* 機器ﾘｾｯﾄ出力(OFF)		*/
		}
		else {
			SCI7_RST = 1;												/* 機器ﾘｾｯﾄ出力(ON)			*/
		}
	}																	/*							*/
}																		/*							*/
																		/*							*/
/*[]----------------------------------------------------------------------[]*/
/*|             KSG_RauTResetOutFOMA()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|         機器リセット出力処理 & DR監視処理                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2012-09-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	KSG_RauTResetOutFOMA( void )
{
	switch(KSG_uc_AdapterType) {
	default:
		KSG_RauTResetOutFOMA_NTNET();
		break;
	case	1:
		KSG_RauTResetOut_UM03();
		break;
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
	case	2:
		KSG_RauTResetOut_UM04();
		break;
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*|             KSG_RauTResetOutFOMA_NTNET()                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|         機器リセット出力処理 & DR監視処理                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2012-09-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
//※ 下記関数の時間パラメータは、UM02-KOの電源ON/OFFシーケンスと合わないがこのままとする
void KSG_RauTResetOutFOMA_NTNET( void )							/* 機器ﾘｾｯﾄ出力処理			*/
{
	if (KSG_Tm_Reset_t != 0) {
		KSG_Tm_Reset_t--;

		// LAN / ｼﾘｱﾙ の通信切替に伴い.OFF --> ON の間隔を速くする.必ず待たないで信号ﾁｪｯｸし,次に進める場合は進む.
		switch(KSG_uc_FomaPWRphase){
		case	1:						// CD 監視 //
			if( KSG_Tm_Reset_t <= 0 || KSG_CD != 0 ){ //タイムアウトか、CD OFF まで //

										// MODEMの電源OFF要求発生 //
				f_ErrSndReqMDMOFF = 1;	// 2008/09/09

				if ( KSG_CD == 0 ){			// CD LOWにならない //
					f_ErrSndReqERRCD = 1;// 2008/09/09
				}
				KSG_uc_FomaPWRphase = 2;
				SCI7_RST = 1;			// 機器リセット出力(ON) [MODEMから見た意味は POWER OFF]
				KSG_Tm_Reset_t = KSG_FomaModemPwrPhase2; //(20S)
			}
			break;
		// ﾌｪｰｽﾞを1つ追加. 1=CD Off待ち 2=DSR Off待ち 3=ONまでのｲﾝﾀｰﾊﾞﾙ 3S
		case	2:
			if( KSG_Tm_Reset_t <= 0 || KSG_DSR != 0 ){	// タイムアウトか DSR OFFまで.
				KSG_uc_FomaPWRphase = 3;
				KSG_Tm_Reset_t = KSG_FomaModemPwrPhase3; //(5S) ONまでのｲﾝﾀｰﾊﾞﾙ .
			}
			break;
		case	3:
			if( KSG_Tm_Reset_t <= 0 ){
				// Time UP //
				SCI7_RST = 0;			// 機器リセット出力停止(OFF) [MODEMから見た意味は POWER ON]
					// ダイヤルガードタイマ設定 //
					KSG_Tm_Gurd.BIT.bit0 = (unsigned short)-20; //(2S)[2 + DSR_ONからATまでの間隔 3 = 5S]
					KSG_f_Gurd_rq = KSG_SET;
					KSG_f_Gurd_ov = KSG_CLR;
					KSG_uc_FomaPWRphase = 0;
			}
			break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|				KSG_RauTResetOut_UM03()										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|			機器リセット出力処理 & DR監視処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author		:	S.Fujii													|*/
/*|	Date		:	2015-05-22												|*/
/*|	Update		:															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void KSG_RauTResetOut_UM03( void )
{
// PWRパルス処理
	if (KSG_Tm_ResetPulse != 0) {
		if (--KSG_Tm_ResetPulse == 0) {
			SCI7_RST = 0;		// PWR=OPEN
		}
	}

// リセットシーケンス実施
	if (KSG_Tm_Reset_t != 0) {
		KSG_Tm_Reset_t--;

		switch(KSG_uc_FomaPWRphase){
		default:
			break;
		case	1:
		// CD OFF監視　：UM03-KO仕様には無いが、従来の論理を踏襲しておく
			if (KSG_Tm_Reset_t <= 0 || KSG_CD != 0) {			// タイムアウトか、CD OFFまで

				f_ErrSndReqMDMOFF = 1;			// MODEMの電源OFF要求発生 //
				if (KSG_CD == 0){
					f_ErrSndReqERRCD = 1;		// CD LOWにならない
				}

				KSG_uc_FomaPWRphase = 2;
				KSG_Tm_Reset_t = UM03KO_PowerOffTime;			// (2S)
				SCI7_RST = 1;									// PWR=SG
				KSG_Tm_ResetPulse = UM03KO_PWR_PulseTime;		// (1S)
			}
			break;
		case	2:
		// CS & DR OFF監視
			if (KSG_Tm_Reset_t <= 0 || KSG_DSR != 0) {			// タイムアウトか DSR OFFまで
				KSG_uc_FomaPWRphase = 3;
				KSG_Tm_Reset_t = UM03KO_PowerOnDelay;			// (63S) ONまでのｲﾝﾀｰﾊﾞﾙ .
			}
			break;
		case	3:
		// PWR ON待ち
		// UM03-KOの電源制御はPWRパルス入力毎にトグル変化するため、通信ケーブルや電源ケーブルを
		// 抜き差しすることで、ソフトの意図している電源状態と、実際の機器電源状態に差が出ると、
		// 回復できない。このため、電源ON用パルス出力前にDRが既にONならば処理をスキップする。
		// （この場合リセットできていないので、次に問題を見つけたときに再度リセットする。）
		// DRはERがONしていないとONにならないのでチェック前に一度ERをONする。
		// （CD、DRなどの制御信号サンプリングのため２秒ほど時間を空ける。）
			if (KSG_Tm_Reset_t == 20) {							// タイムアウト２秒前
				SCI7_DTR = 0;									// ER(DTR) ON（チェック用）
			}
			else if (KSG_Tm_Reset_t <= 0) {						// タイムアウトまで
				if (KSG_DSR != 0) {
				// DSR OFFなら（正常）電源ONパルス出力
					SCI7_RST = 1;								// PWR=SG
					KSG_Tm_ResetPulse = UM03KO_PWR_PulseTime;	// (1S)
				}
				SCI7_DTR = 1;									// ER(DTR) OFF

				// ダイヤルガードタイマ設定:UM03-KOはDR ONならばコマンド受付可なので最小値で良い
				KSG_Tm_Gurd.BIT.bit0 = (unsigned short)-UM03KO_PWR_PulseTime; // (1S)
				KSG_f_Gurd_rq = KSG_SET;
				KSG_f_Gurd_ov = KSG_CLR;
				KSG_uc_FomaPWRphase = 0;
			}
			break;
		}
	}
}
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
/*[]-----------------------------------------------------------------------[]*/
/*|				KSG_RauTResetOut_UM04()										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|			機器リセット出力処理 & DR監視処理 AD-04S(UM04-KO)用				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author		:	A.Iiizumi												|*/
/*|	Date		:	2019-01-23												|*/
/*|	Update		:															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void KSG_RauTResetOut_UM04( void )
{
	if (KSG_Tm_Reset_t != 0) {
		KSG_Tm_Reset_t--;

		// LAN / ｼﾘｱﾙ の通信切替に伴い.OFF --> ON の間隔を速くする.必ず待たないで信号ﾁｪｯｸし,次に進める場合は進む.
		switch(KSG_uc_FomaPWRphase){
		case	1:						// CD 監視 //
			if( KSG_Tm_Reset_t <= 0 || KSG_CD != 0 ){ //タイムアウトか、CD OFF まで //

										
				f_ErrSndReqMDMOFF = 1;	// MODEMの電源OFF要求発生 //

				if ( KSG_CD == 0 ){			// CD LOWにならない //
					f_ErrSndReqERRCD = 1;
				}
				KSG_uc_FomaPWRphase = 2;
				SCI7_RST = 1;			// 機器リセット出力(ON) [MODEMから見た意味は POWER OFF]
				KSG_Tm_Reset_t = UM04KO_PowerOffTime; // UM04-KO CS&DR OFF待ち時間：10秒
			}
			break;
		case	2:
			if( KSG_Tm_Reset_t <= 0 || KSG_DSR != 0 ){	// タイムアウトか DSR OFFまで.
				KSG_uc_FomaPWRphase = 3;
				KSG_Tm_Reset_t = UM04KO_PowerOnDelay; // UM04-KO CS OFF→PWR ON待ち時間：60秒
			}
			break;
		case	3:
			if( KSG_Tm_Reset_t <= 0 ){
				// Time UP //
				SCI7_RST = 0;			// 機器リセット出力停止(OFF) [MODEMから見た意味は POWER ON]
				// ダイヤルガードタイマ設定 //
				// PWR ON からDTR ONさせるまでのwait時間：規定はないため2Sとしておく(KSG_RauModem_ON()起動待ち)
				KSG_Tm_Gurd.BIT.bit0 =  (unsigned short)-UM04KO_ER_ON;

				KSG_f_Gurd_rq = KSG_SET;
				KSG_f_Gurd_ov = KSG_CLR;
				KSG_uc_FomaPWRphase = 0;
			}
			break;
		}
	}
}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
/*[]----------------------------------------------------------------------[]*/
/*|             KSG_RauGetAntLevel()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|         アンテナレベル取得処理                                         |*/
/*|           mode     : 0=開始                                            |*/
/*|                    : 1=終了                                            |*/
/*|           interval : 送信間隔                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2012-09-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
short  KSG_RauGetAntLevel( unsigned short mode, unsigned short interval )
{
	unsigned long	StartTime;

	KSG_gAntLevel = -1;
	KSG_gReceptionLevel = -1;

	if(KSG_mdm_status < 3){									// モデム起動中
		return -1;
	}

	if(mode == 0){											// 送信間隔設定
		// HOSTと送受信中ならアンテナレベル取得開始不可
		if( RAUhost_GetSndSeqFlag() != RAU_SND_SEQ_FLG_NORMAL ||
			Credit_GetSeqFlag() != RAU_RCV_SEQ_FLG_NORMAL ||
			RAUhost_GetRcvSeqFlag() != RAU_RCV_SEQ_FLG_NORMAL ){
			return -1;
		}		
		RAU_SetAntennaLevelState(1);
		// TCPが切断されるのを待つ
		StartTime = LifeTimGet();
		do {
			taskchg(IDLETSKNO);
		} while(LifePastTimGet(StartTime) < RAU_TCP_CLOSE_TIME);	// 5秒
		antena_req_interval = interval * -1;
	}

	if(mode == 0){
		KSG_mdm_status = 10;								// アンテナレベル取得モード
		KSG_uc_AtResultWait = 0;
		KSG_mdm_ant_sts = 1;
		KSG_RauModemOnline = 0;
	}
	else{
		KSG_AntennaLevelCheck = 0;
		RAU_SetAntennaLevelState(0);
		KSG_AntAfterPppStarted = 1;							// アンテナレベルチェックによるE7778抑止用
		KSG_RauModem_OFF();
		KSG_ClosePPPSession(gInterfaceHandle_PPP_rau);
	}
	return 0;
}

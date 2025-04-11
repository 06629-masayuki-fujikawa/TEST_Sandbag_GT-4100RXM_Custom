/************************************************************************************************/
/*																								*/
/*	ﾓｼﾞｭｰﾙ名称	:ksgRauModemMain.c	:---------------------------------------------------------: */
/*	ﾓｼﾞｭｰﾙｼﾝﾎﾞﾙ	:					:ﾓﾃﾞﾑ制御ﾒｲﾝ											  : */
/*	ｺﾝﾊﾟｲﾗ		:					:														  : */
/*	ﾊﾞｰｼﾞｮﾝ		:					:														  : */
/*	ﾀｰｹﾞｯﾄCPU	:					:---------------------------------------------------------: */
/*	対応機種	:					: 														  : */
/*																								*/
/************************************************************************************************/
																		/*							*/
#include	<machine.h>													/*							*/
#include	<string.h>													/*							*/
#include	"system.h"													/*							*/
#include	"ksgmac.h"													/*							*/
#include	"ksg_def.h"
#include	"ksgRauTable.h"												/* ﾃｰﾌﾞﾙﾃﾞｰﾀ定義			*/
#include	"ksgRauModem.h"												/* ﾓﾃﾞﾑ関連関数郡			*/
#include	"ksgRauModemData.h"											/* ﾓﾃﾞﾑ関連ﾃﾞｰﾀ定義			*/

extern BITS	KSG_uc_mdm_res_Foma_f;
extern BITS	KSG_uc_mdm_ErrSndReq_f;
																		/*							*/
/*[]------------------------------------------------------------------------------------------[]*/
/*|	各ﾃﾞﾊﾞｲｽの起動																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : KSG_RauDeviceStart()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauDeviceStart( void )	
{																		/*							*/
	/* MobileArkの起動			*/
	KSG_mdm_Condit = 1;													/* 							*/
	KSG_mdm_TCPcnct_req = 1;											/*							*/
	KSG_mdm_ActFunc.BIT.B0 = 1;		KSG_mdm_ActFunc.BIT.B1 = 1;			/* 接続ｽﾀﾝﾊﾞｲ				*/
	KSG_mdm_Start.BIT.B0 = 1;											/* MobileArk開始			*/
}	

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑﾃﾞｰﾀ初期化																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmEarlyDataInit()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void KSG_RauMdmEarlyDataInit( void )
{																		/*							*/
	KSG_mdm_ActFunc.BYTE = 0;											/*							*/
	KSG_mdm_Start.BYTE = 0;												/* ﾓﾃﾞﾑ停止					*/
	KSG_mdm_f_TmStart = 0;												/* 時間監視機能停止			*/
	KSG_RauModemOnline = 0;												/* ﾎｽﾄ回線状態ﾌﾗｸﾞ 切断		*/
	KSG_dials.BYTE = 0;													/* ﾀﾞｲｱﾙｽﾃｰﾀｽ ﾘｾｯﾄ			*/
	KSG_mdm_status = 0;													/* ﾓﾃﾞﾑ接続ｽﾃｰﾀｽ			*/
	KSG_mdm_init_sts = 0;												/* ﾓﾃﾞﾑ初期化動作停止		*/
	KSG_modem_req = 0;													/* ﾓﾃﾞﾑ接続ﾘｸｴｽﾄﾌﾗｸﾞ		*/
	KSG_mdm_TCP_CNCT = 0;												/* TCPｺﾈｸｼｮﾝ状態ﾌﾗｸﾞ		*/
	KSG_line_stat_h = (KSG_CD_SIG | KSG_DR_SIG | KSG_CS_SIG);			/* ﾎｽﾄ ﾗｲﾝｽﾃｰﾀｽ				*/
	KSG_uc_MdmInitRetry = 0;											// 電源ON失敗時リトライカウンタ //
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑの初期化																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModemInit()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauModemInit( void )
{																		/*							*/

	KSG_dials.BYTE = 0;													/* ﾀﾞｲｱﾙｽﾃｰﾀｽ ﾘｾｯﾄ			*/
	KSG_txdlen_mdm = 0;													/* 送信ﾃﾞｰﾀ長				*/
	KSG_rxdlen_mdm = 0;													/* 受信ﾃﾞｰﾀ長				*/
	KSG_RauMdmRcvQueInit();												/* 受信ｷｭｰの初期化			*/
	KSG_RauMdmSndQueInit();												/* 送信ｷｭｰの初期化			*/
	KSG_RauMdmTimerInit();												/* 時間監視機能初期化		*/
 	KSG_mdm_Condit = 0;													/* 							*/
	KSG_dial_go = 0;													/* ﾀﾞｲｱﾙ切断				*/
	KSG_net_online = 0;													/* ｵﾝﾗｲﾝﾌﾗｸﾞ切断			*/
	KSG_rx_save_len = 0;												/* 受信完了ﾃﾞｰﾀ長			*/
	KSG_ptr_rx_save = &KSG_mdm_r_buf[0];								/* 受信ﾃﾞｰﾀ格納				*/
	KSG_f_ReDial_ov = KSG_SET;											/* ﾘﾀﾞｲｱﾙﾀｲﾏ停止			*/
	KSG_Tm_Gurd.tm = 0;
	KSG_Tm_Gurd.BIT.bit0 = (ushort)-10;	// 1S //
	KSG_f_Gurd_rq = KSG_SET;
	KSG_mdm_e_flag.BYTE = 0;											/* ﾓﾃﾞﾑｴﾗｰﾌﾗｸﾞｸﾘｱ			*/
	KSG_mdm_cut_req = 0;												/* ﾓﾃﾞﾑ切断ﾘｸｴｽﾄﾌﾗｸﾞOFF		*/
	KSG_disconect_flg = 0;												/* TCPｺﾈｸｼｮﾝ切断ﾌﾗｸﾞ		*/
	KSG_tcp_inout = 2;													/* TCP着信起動状態			*/
	KSG_RauMdmSigInfoInit();											/* 信号 ﾁｬﾀ取りｴﾘｱ初期化	*/
	KSG_Tm_Reset_t = 0;													/* 機器ﾘｾｯﾄﾀｲﾏ停止			*/
	KSG_uc_AtRetryCnt = 0;						// 3 times retury counter //
	KSG_uc_AtResultWait = 0;					// 0:-- 1:Waiting for Result Code //
	KSG_uc_ConnAftWait = 0;						// CONNECT ( 500m ) -> PPP //
	KSG_uc_FomaWaitDRFlag = 0;					// 100msec. ER ON 中 //
	KSG_uc_FomaPWRphase = 0;					// 電源管理フェーズ 0=IDLE, 1=CD off 監視, 2=PWR信号 HIGH //
	KSG_uc_mdm_res_Foma_f.BYTE = 0;
	KSG_uc_mdm_ErrSndReq_f.BYTE = 0;
	KSG_Tm_ReDialTime.tm = 0;

}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾓﾃﾞﾑﾒｲﾝ処理																				   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModemMain()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
unsigned char	KSG_RauModemMain( void )
{																		/*							*/
	switch( KSG_mdm_Condit ) {
	// --- 初期化 ----------------------------------------------------------------------------- */
	case 1:																/*							*/
		if( KSG_mdm_Start.BYTE & KSG_mdm_ActFunc.BYTE ) {				/*							*/
			KSG_mdm_Condit = 3;											/*							*/
			KSG_mdm_f_TmStart = 1;										/* 時間監視機能開始			*/
			KSG_RauMdmInitReq();										/* ﾓﾃﾞﾑ初期化要求処理		*/
		}																/*							*/
		break;															/*							*/

	// ---------------------------------------------------------------------------------------- */
	case 3:																/*							*/
		if( ( KSG_mdm_status < 2 ) && ( KSG_f_data_rty_ov == 1 ) ) {	/* DTE確立前				*/
			KSG_RauMdmInitReq();										/* ﾓﾃﾞﾑ初期化要求処理		*/
		}																/*							*/
		KSG_RauMdmControl();											/* ﾓﾃﾞﾑｺﾝﾄﾛｰﾙ				*/
		break;															/*							*/

	// ---------------------------------------------------------------------------------------- */
	default:															/*							*/
		KSG_Tm_No_Action.tm = 0;										/* 無通信ﾀｲﾏ起動停止		*/
		KSG_Tm_TCP_DISCNCT.tm = 0;										/* TCPｺﾈｸｼｮﾝ切断待ちﾀｲﾏ停止	*/
		break;															/*							*/
	}																	/*							*/
// MH322914 (s) kasiyama 2016/07/13 再発呼待ちタイマが正しく動いていないバグに対応する[共通バグNo.1148](MH341106)
//	if(( KSG_mdm_cut_req >= 2 ) && ( KSG_mdm_status < 4 )) KSG_mdm_cut_req = 0;	/* modem切断ﾘｸｴｽﾄﾌﾗｸﾞOFF	*/
// MH322914 (e) kasiyama 2016/07/13 再発呼待ちタイマが正しく動いていないバグに対応する[共通バグNo.1148](MH341106)

	// モデムステータスを返却する
	return KSG_mdm_status;
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	受信ｷｭｰの初期化																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmRcvQueInit()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmRcvQueInit( void )
{																		/*							*/
	KSG_mdm_Rcv_Ctrl.Count = 0;											/* 格納数のｸﾘｱ				*/
	KSG_mdm_Rcv_Ctrl.ReadIdx = 0;										/* 取出ﾎﾟｲﾝﾄｸﾘｱ				*/
	KSG_mdm_Rcv_Ctrl.WriteIdx = 0;										/* 格納ﾎﾟｲﾝﾄｸﾘｱ				*/
	KSG_mdm_Rcv_Ctrl.dummy = 0;											/*							*/
}																		/*							*/

// 未使用
/*[]------------------------------------------------------------------------------------------[]*/
/*|	受信ｷｭｰの設定																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmRcvQueSet()											  			   |*/
/*| PARAMETER1  : unsigned char *pData  :	受信ﾃﾞｰﾀ										   |*/
/*| PARAMETER2  : unsigned short len  	:	文字数											   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmRcvQueSet( unsigned char *pData, unsigned int len )
{																		/*							*/
	if( len > KSG_MDM_DATA_MAX )	len = KSG_MDM_DATA_MAX;				/* 文字長の制限				*/
	KSG_mdm_Rcv_Ctrl.modem_data[ KSG_mdm_Rcv_Ctrl.WriteIdx ].Len = len;	/* 文字長の格納				*/
	memcpy( (char *)KSG_mdm_Rcv_Ctrl.modem_data							/* 文字列の格納				*/
				[ KSG_mdm_Rcv_Ctrl.WriteIdx ].Dat, pData, (size_t)len );/*							*/
	++KSG_mdm_Rcv_Ctrl.Count;											/* 格納数の加算				*/
	++KSG_mdm_Rcv_Ctrl.WriteIdx;										/* 格納ﾎﾟｲﾝﾄの加算			*/
	KSG_mdm_Rcv_Ctrl.WriteIdx &= 3;										/* 格納ﾎﾟｲﾝﾄ範囲制限		*/
}																		/*							*/

// 未使用
/*[]------------------------------------------------------------------------------------------[]*/
/*|	受信ｷｭｰの読出し																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmRcvQueRead()													   |*/
/*| PARAMETER1  : unsigned char *pData  :	受信ﾃﾞｰﾀ										   |*/
/*| PARAMETER2  : unsigned short len  	:	文字数											   |*/
/*[]------------------------------------------------------------------------------------------[]*/
unsigned char	KSG_RauMdmRcvQueRead( unsigned char *pData, unsigned short *len )
{																		/*							*/
	unsigned short	w_len;												/*							*/
																		/*							*/
	if( 0 == KSG_mdm_Rcv_Ctrl.Count )	return	0x00;					/* 受信が無ければ出る		*/
	w_len = KSG_mdm_Rcv_Ctrl.modem_data[ KSG_mdm_Rcv_Ctrl.ReadIdx ].Len;/* 文字長の抽出				*/
	if( w_len > KSG_MDM_DATA_MAX )	w_len = KSG_MDM_DATA_MAX;			/* 文字長の制限				*/
	*len = w_len;														/* 文字長を返す				*/
	memcpy( pData, (char *)KSG_mdm_Rcv_Ctrl.modem_data					/* 受信文字列を取出			*/
				[ KSG_mdm_Rcv_Ctrl.ReadIdx ].Dat, (size_t)w_len );		/*							*/
	--KSG_mdm_Rcv_Ctrl.Count;											/* 格納数減算				*/
	++KSG_mdm_Rcv_Ctrl.ReadIdx;											/* 取出ﾎﾟｲﾝﾄ加算			*/
	KSG_mdm_Rcv_Ctrl.ReadIdx &= 3;										/* 取出ﾎﾟｲﾝﾄ範囲制限		*/
	return 0x01;														/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	送信ｷｭｰの初期化																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSndQueInit()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmSndQueInit( void )
{																		/*							*/
	KSG_mdm_Snd_Ctrl.Count = 0;											/* 格納数ｸﾘｱ				*/
	KSG_mdm_Snd_Ctrl.ReadIdx = 0;										/* 取出ﾎﾟｲﾝﾄｸﾘｱ				*/
	KSG_mdm_Snd_Ctrl.WriteIdx = 0;										/* 格納ﾎﾟｲﾝﾄｸﾘｱ				*/
	KSG_mdm_Snd_Ctrl.dummy = 0;											/* 							*/
}																		/*							*/

// 未使用
/*[]------------------------------------------------------------------------------------------[]*/
/*|	送信ｷｭｰの設定																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSndQueSet()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmSndQueSet( unsigned char *pData, unsigned int len , unsigned char ch )
{																		/*							*/
	if( len > KSG_MDM_DATA_MAX ) len = KSG_MDM_DATA_MAX;				/* 文字長範囲制限			*/
	KSG_mdm_Snd_Ctrl.modem_data											/* 文字長の格納				*/
			[ KSG_mdm_Snd_Ctrl.WriteIdx ].Len = len;					/*							*/
	memcpy( (char *)KSG_mdm_Snd_Ctrl.modem_data							/* 文字の格納				*/
			[ KSG_mdm_Snd_Ctrl.WriteIdx ].Dat, pData, (size_t)len );	/*							*/
	KSG_mdm_Snd_Ctrl.modem_data[ KSG_mdm_Snd_Ctrl.WriteIdx ].ch = ch;	/* 指定ｿｹｯﾄ(UDPのみ)		*/
	++KSG_mdm_Snd_Ctrl.Count;											/* 格納数加算				*/
	++KSG_mdm_Snd_Ctrl.WriteIdx;										/* 格納ﾎﾟｲﾝﾄ加算			*/
	KSG_mdm_Snd_Ctrl.WriteIdx &= 3;										/* 格納ﾎﾟｲﾝﾄ制限			*/
}																		/*							*/

// 未使用
/*[]------------------------------------------------------------------------------------------[]*/
/*|	送信ｷｭｰの読出し																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSndQueRead()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
unsigned char	KSG_RauMdmSndQueRead( unsigned char *pData, unsigned int *len, unsigned char *ch, unsigned char kind )
{																		/*							*/
	unsigned int	w_len;												/*							*/
																		/*							*/
	if( 0 == KSG_mdm_Snd_Ctrl.Count )	return	0;						/* 送信ｷｭｰが空なら出る		*/
	if( kind )						return	1;							/* 有無確認だけなら出る		*/
	w_len = KSG_mdm_Snd_Ctrl.modem_data[ KSG_mdm_Snd_Ctrl.ReadIdx ].Len;/* 格納文字長を取得			*/
	if( w_len > KSG_MDM_DATA_MAX )		w_len = KSG_MDM_DATA_MAX;		/* 格納文字長制限			*/
	*len = w_len;														/* 格納文字長をｾｯﾄ			*/
	*ch = KSG_mdm_Snd_Ctrl.modem_data[ KSG_mdm_Snd_Ctrl.ReadIdx ].ch;	/* 指定ｿｹｯﾄをｾｯﾄ			*/
	memcpy( pData, (char *)KSG_mdm_Snd_Ctrl.modem_data					/* 格納文字列をｾｯﾄ			*/
			[ KSG_mdm_Snd_Ctrl.ReadIdx ].Dat, (size_t)w_len );			/*							*/
	--KSG_mdm_Snd_Ctrl.Count;											/* 格納数減算				*/
	++KSG_mdm_Snd_Ctrl.ReadIdx;											/* 取出ﾎﾟｲﾝﾀ加算			*/
	KSG_mdm_Snd_Ctrl.ReadIdx &= 3;										/* 取出ﾎﾟｲﾝﾀ範囲制限		*/
	return	1;															/* 							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾁｪｯｸｻﾑ																					   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSumCheck()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmSumCheck( unsigned char *pData, unsigned short len, unsigned char *SUM )
{																		/*							*/
	unsigned short	i, s;												/*							*/
																		/*							*/
	for( i = 0, s = 0; i < len; i++ )									/* 文字列全体を積算			*/
		s = s + (unsigned short)pData[i];								/*							*/
	s = s & 0x00ff;														/* 最下位ﾊﾞｲﾄを有効			*/
	i = ( s & 0x00f0 )>>4;												/* 上位4ﾋﾞｯﾄ抽出			*/
	if( i >= 0x000a )	SUM[0] = (unsigned char)( i + 0x0037 );			/* A～FのASCII変換			*/
	else				SUM[0] = (unsigned char)( i + 0x0030 );			/* 0～9のASCII変換			*/
	i = s & 0x000f;														/* 下位4ﾋﾞｯﾄ				*/
	if( i >= 0x000a )	SUM[1] = (unsigned char)( i + 0x0037 );			/* A～FのASCII変換			*/
	else				SUM[1] = (unsigned char)( i + 0x0030 );			/* 0～9のASCII変換			*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	PPP切断時の後処理																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmPostPppClose()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmPostPppClose( void )
{
	// モデム切断
	// アンテナレベルチェックならモデムOFFしない
	if(KSG_AntennaLevelCheck == 0) {
		KSG_RauModem_OFF();
	}
}
/*[]------------------------------------------------------------------------------------------[]*/
/*|	PPP切断時の後処理																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauClosePPPSession()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauClosePPPSession( void )
{
	if(gInterfaceHandle_PPP_rau && KSG_gPpp_RauStarted) {	// ハンドル有効で接続中?
		KSG_ClosePPPSession(gInterfaceHandle_PPP_rau);
	}
}
/*[]------------------------------------------------------------------------------------------[]*/
/*|	モデム切断要求状態取得																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauGetMdmCutState()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
uchar	KSG_RauGetMdmCutState( void )
{
	return KSG_mdm_cut_req;
}
/*[]------------------------------------------------------------------------------------------[]*/
/*|	モデム切断要求状態設定																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauSetMdmCutState()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauSetMdmCutState( unsigned char modemCutRequestState )
{
	KSG_mdm_cut_req = modemCutRequestState;
}

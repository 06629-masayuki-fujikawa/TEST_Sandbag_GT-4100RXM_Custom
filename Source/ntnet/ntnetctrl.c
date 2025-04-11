/*[]----------------------------------------------------------------------[]*/
/*| NT-NET関連制御                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
#include	<string.h>
#include	<stddef.h>
#include	"system.h"
#include	"iodefine.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"LKmain.h"
#include	"flp_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"prm_tbl.h"
#include	"irq1.h"
#include	"cnm_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"mnt_def.h"
#include	"ntnetauto.h"
#include	"Suica_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"ifm_ctrl.h"
#include	"remote_dl.h"
#include	"mdl_def.h"
#include	"aes_sub.h"
#include	"raudef.h"
#include	"oiban.h"
#include	"raudef.h"
#include	"cre_ctrl.h"
#include	"remote_dl.h"
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6554 センタ用端末情報データ 変更対応
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6554 センタ用端末情報データ 変更対応

RECEIVE_BACKUP	RecvBackUp;								/* NE-NET要求ﾃﾞｰﾀ受信時ﾊﾞｯｸｱｯﾌﾟ	*/

union {
	t_TKI_CYUSI	TeikiCyusi;
} z_Work;

static uchar s_Is234StateValid = 0;
static uchar s_IsRevData80_R = 0;
static uchar basicSeqNo = 0;								// シーケンスNo(1～99)ただし起動時の初期値は「0」

// MH322914(S) K.Onodera 2016/08/30 AI-V対応：振替精算
ushort	PiP_FurikaeSts;		// 振替精算ステータスフラグ
// MH322914(E) K.Onodera 2016/08/30 AI-V対応：振替精算
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
static ulong	VehicleCountDateTime = 0;
static ushort	VehicleCountSeqNo = 0;

SETTLEMENT_INFO	t_Settlement;
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応

static const ushort c_coin[COIN_SYU_CNT] = {10, 50, 100, 500};
static	char	ntdata65_UnknownVer[] = "--------";

// T合計/T小計 送信関数
static	void	NTNET_Snd_SyukeiKihon(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiRyokinMai(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiBunrui(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiWaribiki(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiTeiki(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//static	void	NTNET_Snd_SyukeiShashitsuMai(LOKTOTAL *syukei, uchar ID, uchar Save, ushort Type);
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
static	void	NTNET_Snd_SyukeiKinsen(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiSyuryo(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);

#define	_NTNET_Snd_Data30(syu)		NTNET_Snd_SyukeiKihon(syu, 30, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data31(syu)		NTNET_Snd_SyukeiRyokinMai(syu, 31, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data32(syu)		NTNET_Snd_SyukeiBunrui(syu, 32, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data33(syu)		NTNET_Snd_SyukeiWaribiki(syu, 33, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data34(syu)		NTNET_Snd_SyukeiTeiki(syu, 34, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data35(syu)		NTNET_Snd_SyukeiShashitsuMai(syu, 35, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data36(syu)		NTNET_Snd_SyukeiKinsen(syu, 36, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data41(syu)		NTNET_Snd_SyukeiSyuryo(syu, 41, NTNET_DO_BUFFERING, 1)

#define	_NTNET_Snd_Data158(syu)		NTNET_Snd_SyukeiKihon(syu, 158, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data159(syu)		NTNET_Snd_SyukeiRyokinMai(syu, 159, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data160(syu)		NTNET_Snd_SyukeiBunrui(syu, 160, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data161(syu)		NTNET_Snd_SyukeiWaribiki(syu, 161, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data162(syu)		NTNET_Snd_SyukeiTeiki(syu, 162, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data163(syu)		NTNET_Snd_SyukeiShashitsuMai(syu, 163, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data164(syu)		NTNET_Snd_SyukeiKinsen(syu, 164, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data169(syu)		NTNET_Snd_SyukeiSyuryo(syu, 169, NTNET_NOT_BUFFERING, 11)

// 複数合計/複数小計 受信関数
static	void	NTNET_Rev_SyukeiKihon(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiRyokinMai(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiBunrui(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiWaribiki(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiTeiki(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiKinsen(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiSyuryo(void);

#define	_NTNET_RevData66(syu)		NTNET_Rev_SyukeiKihon(syu)
#define	_NTNET_RevData67(syu)		NTNET_Rev_SyukeiRyokinMai(syu)
#define	_NTNET_RevData68(syu)		NTNET_Rev_SyukeiBunrui(syu)
#define	_NTNET_RevData69(syu)		NTNET_Rev_SyukeiWaribiki(syu)
#define	_NTNET_RevData70(syu)		NTNET_Rev_SyukeiTeiki(syu)
#define	_NTNET_RevData72(syu)		NTNET_Rev_SyukeiKinsen(syu)
#define	_NTNET_RevData77(syu)		NTNET_Rev_SyukeiSyuryo()

#define	_NTNET_RevData194(syu)		NTNET_Rev_SyukeiKihon(syu)
#define	_NTNET_RevData195(syu)		NTNET_Rev_SyukeiRyokinMai(syu)
#define	_NTNET_RevData196(syu)		NTNET_Rev_SyukeiBunrui(syu)
#define	_NTNET_RevData197(syu)		NTNET_Rev_SyukeiWaribiki(syu)
#define	_NTNET_RevData198(syu)		NTNET_Rev_SyukeiTeiki(syu)
#define	_NTNET_RevData200(syu)		NTNET_Rev_SyukeiKinsen(syu)
#define	_NTNET_RevData205(syu)		NTNET_Rev_SyukeiSyuryo()

static void		ntnet_DateTimeCnv(date_time_rec2 *dst, const date_time_rec *src);
static void		ntnet_TeikiCyusiCnvCrmToNt(TEIKI_CHUSI *ntnet, const struct TKI_CYUSI *crm, BOOL all);
static BOOL		ntnet_TeikiCyusiCnvNtToCrm(struct TKI_CYUSI *crm, const TEIKI_CHUSI *ntnet);
static short	ntnet_SearchTeikiCyusiData(ulong ParkingNo, ulong PassID);
static ushort	ntnet_CoinIdx(ushort kind);
static BOOL	chk_hpmonday(t_splday_yobi date);
uchar	cyushi_chk( ushort no );

void	ntnet_Snd_CoinData( uchar kind, ushort type );
void	ntnet_Snd_NoteData( uchar kind, ushort type );
void	ntnet_set_req(ulong req, uchar *dat, int max);
int		car_park_condition(int no, uchar new_cond);
ulong	Convert_PassInfo( ulong ParkingNo, ulong PassID );
void	NTNET_Snd_Data22_FusDel( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind );
static	int		get_param_addr(ushort kind, ushort addr, ushort *seg, ushort *inaddr);
int		check_acceptable(void);
void NTNET_AfterRcv_CallBack();

// 設定一時保存バッファ（※システム起動時に０クリア）
struct _setdata_ {
	ushort	addr;
	ushort	count;
	long	data[1];
};
NTNET_TMP_BUFFER tmp_buffer;
uchar	SetCarInfoSelect( short index );
#define	SP_RANGE_DEF( a )	((a*1000L)+(a*100L)+(a*10L))
static	uchar	InCarTimeChk( ushort num, struct clk_rec *clk_para );
static void	NTNET_Edit_BasicData( uchar, uchar, uchar, DATA_BASIC *);
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//static	void	NTNET_Snd_Data152_SK(void);
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH810105(S) MH364301 QRコード決済対応 GT-4100
//// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
//static void NTNET_Edit_Data56_ParkingTkt_Pass( Receipt_data *p_RcptDat, DATA_KIND_56_r17 *p_NtDat );
//// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
static void NTNET_Edit_Data56_ParkingTkt_Pass( Receipt_data *p_RcptDat, DATA_KIND_56_rXX *p_NtDat );
// MH810105(E) MH364301 QRコード決済対応 GT-4100

// MH321800(S) hosoda ICクレジット対応
#define	CARD_EC_KOUTSUU_USED_TEMP		99				// 既存と決済リーダでの交通系カードとを区別するための一時的な値
static const	ushort	ec_discount_kind_tbl[EC_BRAND_TOTAL_MAX] = {
// ブランド番号と割引集計の割引種別との対応
// ※ブランド番号の順に定義してください
	NTNET_EDY_0, NTNET_NANACO_0, NTNET_WAON_0, NTNET_SAPICA_0, NTNET_SUICA_1,
// MH810105(S) MH364301 QRコード決済対応
//	NTNET_ID_0, NTNET_QUICPAY_0, 0, 0, 0,
// MH810105(S) MH364301 PiTaPa対応
//	NTNET_ID_0, NTNET_QUICPAY_0, NTNET_QR, 0, 0,
	NTNET_ID_0, NTNET_QUICPAY_0, NTNET_QR, NTNET_PITAPA_0, 0,
// MH810105(E) MH364301 PiTaPa対応
// MH810105(E) MH364301 QRコード決済対応
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
};
// MH321800(E) hosoda ICクレジット対応
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
static void	NTNET_RevDataPiP(void);
static ushort PIP_CheckCarInfo(uchar area,ulong no, ushort* pos);
static void NTNET_RevData16_01_ReceiptAgain(void);
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//static void NTNET_RevData16_03_Furikae( void );
//static ushort PiP_CheckFurikae( void );
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
static void NTNET_RevData16_03_Furikae_Info( void );
static void NTNET_RevData16_03_Furikae_Check( void );
static void NTNET_RevData16_03_Furikae_Go( void );
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//static void NTNET_RevData16_05_ReceiveTkt(void);
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
static void NTNET_RevData16_08_RemoteCalc( void );
static void NTNET_RevData16_08_RemoteCalc_Time( void );
static void NTNET_RevData182_RemoteCalcFee(void);
static void NTNET_RevData16_10_ErrorAlarm( void );
static ushort PIP_GetOccurErr( PIP_ErrAlarm *tbl );
static ushort PIP_GetOccurAlarm( PIP_ErrAlarm *tbl );
static void PiP_GetFurikaeInfo( Receipt_data* receipt );
static uchar PiP_GetNewestPayReceiptData( Receipt_data *rcp, ushort area, ushort no );

extern	short	LKopeGetLockNum( uchar, ushort, ushort * );

extern	const	uchar	ErrAct_index[];
extern	uchar	get_crm_state(void);
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

/*[]----------------------------------------------------------------------[]*/
/*| ﾃﾞｰﾀ受信処理                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_GetRevData                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_GetRevData( ushort msg, uchar *data )
{
	t_NtNet_ClrHandle	h;
	eNTNET_BUF_KIND		BufKind = NTNET_BUF_INIT;
	const t_NtBufState	*ntbufst;
	ushort				usdata;

	switch( msg ){
	case IBK_NTNET_DAT_REC:											// NTNETﾃﾞｰﾀ受信
		if( NTBUF_GetRcvNtData( &RecvNtnetDt, NTNET_BUF_PRIOR ) > 0 ){
			// 優先ﾃﾞｰﾀ受信
			BufKind = NTNET_BUF_PRIOR;
		}
		else if( NTBUF_GetRcvNtData( &RecvNtnetDt, NTNET_BUF_NORMAL ) > 0 ){
			// ﾊﾞｯﾌｧﾘﾝｸﾞでない通常ﾃﾞｰﾀ受信
			BufKind = NTNET_BUF_NORMAL;
		}
		else if( NTBUF_GetRcvNtData( &RecvNtnetDt, NTNET_BUF_BROADCAST ) > 0 ){
			// 同報データ
			BufKind = NTNET_BUF_BROADCAST;
		}

		if( BufKind == NTNET_BUF_PRIOR || BufKind == NTNET_BUF_NORMAL ||	// 優先or通常
			BufKind == NTNET_BUF_BROADCAST ){								// or同報ﾃﾞｰﾀ受信?
			NTNET_CtrlRecvData();
			//phase = 1;											// 受信ﾃﾞｰﾀｸﾘｱ
			NTBUF_ClrRcvNtData_Prepare( BufKind, &h );
			//phase = 2;
			NTBUF_ClrRcvNtData_Exec( &h );
			//phase = 3;
		}
		break;
	case IBK_NTNET_FREE_REC:										// NTNET FREEﾊﾟｹｯﾄﾃﾞｰﾀ受信
		break;
	case IBK_NTNET_ERR_REC:											// NTNET ｴﾗｰﾃﾞｰﾀ受信
		err_chk( (char)data[0], (char)data[1], (char)data[2], 0, 0 );	// NTNET IBKｴﾗｰ
		break;
	case IBK_NTNET_BUFSTATE_CHG:									// NTNET ﾊﾞｯﾌｧ状態変化
		memcpy( &usdata, data, 2 );
		ntbufst = NTBUF_GetBufState();
		switch( usdata ){
		case 20:													// 入庫ﾃﾞｰﾀ
			if( !(ntbufst->car_in & 0x02) ){						// ﾊﾞｯﾌｧFULL解除?
				err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_BUFFULL, NTERR_RELEASE, 0, 0 );
			}
			queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
			break;
		case 22:													// 精算ﾃﾞｰﾀ
			if( !(ntbufst->sale & 0x02) ){							// ﾊﾞｯﾌｧFULL解除?
				err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_BUFFULL, NTERR_RELEASE, 0, 0 );
			}
			if( !(ntbufst->sale & 0x01) ){							// ﾊﾞｯﾌｧNEAR FULL解除?
				err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_BUFNFULL, NTERR_RELEASE, 0, 0 );
			}
			queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
			break;
		}
		
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾃﾞｰﾀ受信処理                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_CtrlRecvData                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTNET_CtrlRecvData( void )
{
	// システムIDチェック
	switch (RecvNtnetDt.DataBasic.SystemID) {
	case REMOTE_DL_SYSTEM:
		NTNET_CtrlRecvData_DL();
		return;
// MH322914(S) K.Onodera 2016/08/08 AI-V対応
	case PIP_SYSTEM:
		NTNET_RevDataPiP();
		return;
// MH322914(E) K.Onodera 2016/08/08 AI-V対応
	default:
		break;
	}

	switch( RecvNtnetDt.DataBasic.DataKind ){
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	case 1:													// 車室問合せﾃﾞｰﾀ
//		NTNET_RevData01();
//		break;
//	case 2:													// 車室問合せ結果ﾃﾞｰﾀ
//		NTNET_RevData02();
//		break;
//	case 3:													// 車室問合せ結果NGﾃﾞｰﾀ
//		NTNET_RevData03();
//		break;
//	case 4:													// ﾛｯｸ制御ﾃﾞｰﾀ
//		NTNET_RevData04();
//		break;
//	case 22:												// 精算ﾃﾞｰﾀ(CPS)
//	case 23:												// 精算ﾃﾞｰﾀ(EPS)
//		NTNET_RevData22();
//		break;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	case 100:												// 制御ﾃﾞｰﾀ
		NTNET_RevData100();
		break;
	case 103:												// 管理ﾃﾞｰﾀ要求
		NTNET_RevData103();
		break;
	case 109:												// ﾃﾞｰﾀ要求2
		NTNET_RevData109();
		break;
	case 154:
		NTNET_RevData154();									// センタ用データ要求
		break;
	case 156:
		NTNET_RevData156();									// 同期時刻設定要求
		break;
	case 119:												// 時計ﾃﾞｰﾀ
		NTNET_AfterRcv_CallBack();
		NTNET_RevData119();
		break;
	case 143:												// 定期券問合せ結果ﾃﾞｰﾀ
		NTNET_RevData143();
		break;
	case 80:												// 共通設定ﾃﾞｰﾀ
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_RevData80_R();							// 遠隔IBK制御データ
		} else {
			NTNET_RevData80();								// 設定データ送信要求
		}
		break;
	case 91:
		NTNET_RevData91();									// 定期券精算中止テーブルデータ
		break;
	case 93:												// 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ
		NTNET_RevData93();
		break;
	case 94:												// 定期券更新ﾃｰﾌﾞﾙ
		NTNET_RevData94();
		break;
	case 95:												// 定期券出庫時刻ﾃｰﾌﾞﾙ
		NTNET_RevData95();
		break;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	case 97:												// 車室パラメータ受信
//		NTNET_RevData97();
//		break;
//	case 98:
//		NTNET_RevData98();									// ﾛｯｸ装置ﾊﾟﾗﾒｰﾀ
//		break;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	case 110:
		NTNET_RevData110();									// ﾃﾞｰﾀ要求2結果NG
		break;
	case 116:	// no break
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_RevData116_R();							// 通信チェック要求データ
			break;
		}
	case 117:
		NTNET_RevData116();									// 定期券データ更新・定期券データチェック
		break;
	/* 複数合計集計データ */
	case 66:
		_NTNET_RevData66(&sky.fsyuk);						// 集計基本データ
		break;
	case 67:
		_NTNET_RevData67(&sky.fsyuk);						// 料金種別毎集計データ
		break;
		case 68:
		_NTNET_RevData68(&sky.fsyuk);						// 分類集計データ
		break;
	case 69:
		_NTNET_RevData69(&sky.fsyuk);						// 割引集計データ
		break;
	case 70:
		_NTNET_RevData70(&sky.fsyuk);						// 定期集計データ
		break;
	case 71:
	//	_NTNET_RevData71									// 車室毎集計データ
		break;
	case 72:
		_NTNET_RevData72(&sky.fsyuk);						// 金銭集計データ
		break;
	case 73:
	//	_NTNET_RevData73									// タイムレジ集計データ
		break;
	case 74:
	//	_NTNET_RevData74									// 個別集計データ
		break;
	case 77:
		_NTNET_RevData77(&sky.fsyuk);						// 集計終了通知データ
		break;
	/* 複数小計集計データ */
	case 194:
		_NTNET_RevData194(&sky.fsyuk);						// 集計基本データ
		break;
	case 195:
		_NTNET_RevData195(&sky.fsyuk);						// 料金種別毎集計データ
		break;
	case 196:
		_NTNET_RevData196(&sky.fsyuk);						// 分類集計データ
		break;
	case 197:
		_NTNET_RevData197(&sky.fsyuk);						// 割引集計データ
		break;
	case 198:
		_NTNET_RevData198(&sky.fsyuk);						// 定期集計データ
		break;
	case 199:
	//	_NTNET_RevData199									// 車室毎集計データ
		break;
	case 200:
		_NTNET_RevData200(&sky.fsyuk);						// 金銭集計データ
		break;
	case 201:
	//	_NTNET_RevData201									// タイムレジ集計データ
		break;
	case 202:
	//	_NTNET_RevData202									// 個別集計データ
		break;
	case 205:
		_NTNET_RevData205(&sky.fsyuk);						// 集計終了通知データ
		break;
	case 83:
	//	NTNET_RevData83();									// 設定データ送信要求
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_RevData83_R();							// テーブル件数データ
		}
		else {
			NTNET_RevData83();									// 設定データ送信要求
		}
		break;
	case 78:
		NTNET_RevData78();									// 設定データ要求
		break;
	case 114:
		NTNET_RevData114();									// 設定データ送信要求
		break;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	case 234:
//		NTNET_RevData234();									// 現在台数管理データ
//		break;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	case 92:
		NTNET_RevData92();									// 遠隔定期券ステータステーブル
		break;
	case 240:
		NTNET_RevData240();									// 遠隔制御データ
		break;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	case 243:
//		NTNET_RevData243();									// 遠隔データ要求
//		break;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	case 90:
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_RevData90_R();									// ニアフル通知データ
		}
		break;
	case 101:
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_Rev_Data101_R();								// 通信チェック結果データ
		}
		break;
	case 142:												// 定期券問合せデータ
		NTNET_RevData142();								// 定期券問合せデータ
		break;
	case 150:
		NTNET_RevData150();									// 無効係員IDテーブルデータ
		break;
	case 254:
		NTNET_RevData254();									// 係員ID状態データ
		break;
	case 188:												// 遠隔ダウンロード要求
		NTNET_RevData188(&RecvNtnetDt);
		break;
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 車室問合せﾃﾞｰﾀ(ﾃﾞｰﾀ種別01)受信処理                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData01                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData01( void )
{
	ulong	ulwork;
	ushort	i;
	ushort	uswork;

	uswork = 1;														// 未接続
	if( RecvNtnetDt.RData01.LockNo != 0 ){							// 区画情報あり？
		for( i = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if( !SetCarInfoSelect((short)i) ){
				continue;
			}
			ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// 区画情報Get
			if( RecvNtnetDt.RData01.LockNo == ulwork ){				// 要求された区画情報と一致?
				uswork = 0;											// 正常
				if( m_mode_chk_psl( (ushort)(i+1) ) == ON ){		// 手動ﾓｰﾄﾞﾁｪｯｸ
					uswork = 2;										// 手動ﾓｰﾄﾞ中
				}
				break;
			}
		}
	}
	// 未接続の場合
	if (uswork == 1) {
		ulwork = RecvNtnetDt.RData01.LockNo;
		i = 0;
	}

	NTNET_Snd_Data02( ulwork, (ushort)(i+1), uswork );				// NT-NET車室問合せ結果ﾃﾞｰﾀ作成
}


/*[]----------------------------------------------------------------------[]*/
/*| 車室問合せ結果ﾃﾞｰﾀ(ﾃﾞｰﾀ種別02)受信処理                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData02                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData02( void )
{
	uchar	i = 0;
	uchar	j = 0;
	if( RecvNtnetDt.RData02.LockNo == OPECTL.multi_lk ){			// 問合せした車室№と同様?
		LOCKMULTI.LockNo = RecvNtnetDt.RData02.LockNo;
		if( RecvNtnetDt.RData02.Answer == 2 ){
			LOCKMULTI.Answer = 4;
		}
		else{
			LOCKMULTI.Answer = RecvNtnetDt.RData02.Answer;
		}
		/****************************/
		/*	LOCKMULTI.Answer		*/
		/*		0:正常				*/
		/*		1:未接続（該当なし）*/
		/*		4:手動ﾓｰﾄﾞ中		*/
		/****************************/
		memcpy( &LOCKMULTI.lock_mlt, RecvNtnetDt.RData02.LkInfo, sizeof( RecvNtnetDt.RData02.LkInfo ) );
/*追加ﾃﾞｰﾀを格納*/
		LOCKMULTI.ppc_chusi_ryo_mlt = RecvNtnetDt.RData02.PpcData;
		for(i=0 ; i<15 ; i++)
			LOCKMULTI.sev_tik_mlt[i] = (uchar)RecvNtnetDt.RData02.SvsTiket[i];
		for(i=0,j=0 ; i<5 ; i++,j+=2){
			LOCKMULTI.kake_data_mlt[i].mise_no = (ushort)RecvNtnetDt.RData02.KakeTiket[j];
			LOCKMULTI.kake_data_mlt[i].maisuu = (uchar)RecvNtnetDt.RData02.KakeTiket[j+1];
		}
		queset( OPETCBNO, IBK_NTNET_LOCKMULTI, 0, NULL );			// NTNET ﾏﾙﾁ精算用ﾃﾞｰﾀ(車室問合せ結果ﾃﾞｰﾀ)受信通知
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 車室問合せ結果NGﾃﾞｰﾀ(ﾃﾞｰﾀ種別03)受信処理                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData03                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData03( void )
{
	if( RecvNtnetDt.RData03.LockNo == OPECTL.multi_lk ){			// 問合せした車室№と同様?
		LOCKMULTI.LockNo = RecvNtnetDt.RData03.LockNo;

		if(  RecvNtnetDt.RData03.Answer == 0 ){
			LOCKMULTI.Answer = 2;									// 未接続（該当なし）
		}
		else{
			LOCKMULTI.Answer = 3;									// 通信障害中
		}
		queset( OPETCBNO, IBK_NTNET_LOCKMULTI, 0, NULL );			// NTNET ﾏﾙﾁ精算用ﾃﾞｰﾀ(車室問合せ結果ﾃﾞｰﾀ)受信通知
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ﾛｯｸ制御ﾃﾞｰﾀ(ﾃﾞｰﾀ種別04)受信処理                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData04                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData04( void )
{
	ulong	ulwork;
	ushort	i;
	uchar		CarCnd;
	flp_com*	fl_wrk;
	void	(*_ntnet_snd_data05)(ulong, uchar, uchar);		/* 関数ポインタ */
	ushort	recv_MAF = 0;
	ushort	move;
	
	_ntnet_snd_data05 = NTNET_Snd_Data05;		// 通常のNT-NETへの送信ポインタをセット
	move = RecvNtnetDt.RData04.MoveMode;
	i = 0;

	if( RecvBackUp.ReceiveFlg == 0 ){								// 要求受信済みではない?

		RecvBackUp.ReceiveFlg = 1;									// 要求受信済み
		memcpy( &RecvBackUp.ParkingNo, &RecvNtnetDt.DataBasic.ParkingNo, 22 );	// 受信ﾃﾞｰﾀﾊﾞｯｸｱｯﾌﾟ

		switch( move ){	// 処理区分？

			case	5:	// 全ﾛｯｸ装置閉
				if( recv_MAF )	i = 1;
				queset( FLPTCBNO, LOCK_A_CLS_SND_NTNET, sizeof(i), &i );	// ﾛｯｸ装置管理処理（fcmain）へﾒｯｾｰｼﾞ送信
				wopelg( OPLOG_SET_A_LOCK_CLOSE, 0, 0 );				// 操作履歴登録：全ﾛｯｸ装置閉(NT-NET)
				_ntnet_snd_data05( 0, 0, 34 );						// 返信（区画情報＝0、車両検知状態＝接続無し、装置状態＝全ﾛｯｸ装置閉）
				break;

			case	6:	// 全ﾛｯｸ装置開
				if( recv_MAF )	i = 1;
				queset( FLPTCBNO, LOCK_A_OPN_SND_NTNET, sizeof(i), &i );	// ﾛｯｸ装置管理処理（fcmain）へﾒｯｾｰｼﾞ送信
				wopelg( OPLOG_SET_A_LOCK_OPEN, 0, 0 );				// 操作履歴登録：全ﾛｯｸ装置開(NT-NET)
				_ntnet_snd_data05( 0, 0, 35 );						// 返信（区画情報＝0、車両検知状態＝接続無し、装置状態＝全ﾛｯｸ装置開）
				memset( &Kakari_Numu[0], 0, LOCK_MAX );				// 強制出庫用係員№ｴﾘｱｸﾘｱ
				break;

			case	32:	// 全ﾌﾗｯﾌﾟ上昇
				if( recv_MAF )	i = 1;
				queset( FLPTCBNO, FLAP_A_UP_SND_NTNET, sizeof(i), &i );	// ﾛｯｸ装置管理処理（fcmain）へﾒｯｾｰｼﾞ送信
				queset( FLPTCBNO, INT_FLAP_A_UP_SND_NTNET, sizeof(i), &i );	// ﾛｯｸ装置管理処理（fcmain）へﾒｯｾｰｼﾞ送信
				wopelg( OPLOG_SET_A_FLAP_UP, 0, 0 );				// 操作履歴登録：全ﾌﾗｯﾌﾟ上昇(NT-NET)
				_ntnet_snd_data05( 0, 0, 32 );						// 返信（区画情報＝0、車両検知状態＝接続無し、装置状態＝全ﾌﾗｯﾌﾟ上昇）
				break;

			case	33:	// 全ﾌﾗｯﾌﾟ下降
				if( recv_MAF )	i = 1;
				queset( FLPTCBNO, FLAP_A_DOWN_SND_NTNET, sizeof(i), &i );	// ﾛｯｸ装置管理処理（fcmain）へﾒｯｾｰｼﾞ送信
				queset( FLPTCBNO, INT_FLAP_A_DOWN_SND_NTNET, sizeof(i), &i );	// ﾛｯｸ装置管理処理（fcmain）へﾒｯｾｰｼﾞ送信
				wopelg( OPLOG_SET_A_FLAP_DOWN, 0, 0 );				// 操作履歴登録：全ﾌﾗｯﾌﾟ下降(NT-NET)
				_ntnet_snd_data05( 0, 0, 33 );						// 返信（区画情報＝0、車両検知状態＝接続無し、装置状態＝全ﾌﾗｯﾌﾟ下降）
				memset( &Kakari_Numu[0], 0, LOCK_MAX );				// 強制出庫用係員№ｴﾘｱｸﾘｱ
				break;

			default:	// その他
				if( RecvNtnetDt.RData04.LockNo != 0 ){				// 区画情報あり？
				for( i = 0; i < LOCK_MAX; i++ ){
					WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
					if( !SetCarInfoSelect((short)i) ){
						continue;
					}
					ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// 区画情報Get
					if( RecvNtnetDt.RData04.LockNo == ulwork ){								// 要求された区画情報と一致?
						i++;
						if( RecvNtnetDt.RData04.MoveMode == 1 ){							// ﾛｯｸ装置閉指示
							if( recv_MAF ){
								recv_MAF = FLAP_UP_SND_MAF;
							}else{
								recv_MAF = FLAP_UP_SND_NTNET;
							}
							queset( FLPTCBNO, recv_MAF, sizeof(i), &i );
							if( i < TOTAL_CAR_LOCK_MAX ){
								wopelg(OPLOG_SET_FLAP_UP, 0, ulwork);
							}
							else{
								wopelg(OPLOG_SET_LOCK_CLOSE, 0, ulwork);
							}
							return;
						}
						else if( RecvNtnetDt.RData04.MoveMode == 2 ){						// ﾛｯｸ装置開指示
							if( recv_MAF ){
								recv_MAF = FLAP_DOWN_SND_MAF;
							}else{
								recv_MAF = FLAP_DOWN_SND_NTNET;
							}
							queset( FLPTCBNO, recv_MAF, sizeof(i), &i );
							if( i < TOTAL_CAR_LOCK_MAX ){
								wopelg(OPLOG_SET_FLAP_DOWN, 0, ulwork);
							}
							else{
								wopelg(OPLOG_SET_LOCK_OPEN, 0, ulwork);
							}
							Kakari_Numu[i-1] = 0;								// 強制出庫用係員№ｴﾘｱｸﾘｱ
							return;
						}
						else if (RecvNtnetDt.RData04.MoveMode == 31) {		// 暗証番号消去指示
							fl_wrk = &FLAPDT.flp_data[i-1];
							if(fl_wrk->passwd != 0){						// 暗証番号が登録されている場合
								fl_wrk->passwd = 0;							// 暗証番号(Atype)消去
							}
							wopelg(OPLOG_ANSHOU_B_CLR, 0, ulwork);
			
							// 車両検知状態・ﾛｯｸ装置状態取得（NTNET_Snd_Data05_Sub流用）
							CarCnd = 0;
							if( fl_wrk->nstat.bits.b08 == 0 ){				// 接続有り?
								if( fl_wrk->nstat.bits.b00 ){				// 車両有り?
									CarCnd = 1;								// 車両有り
								}else{
									CarCnd = 2;								// 車両無し
								}
							}else{
								CarCnd = 0;									// 接続無し
							}
							// 返信(ロック装置状態31：暗証番号解除)
							_ntnet_snd_data05( RecvNtnetDt.RData04.LockNo, CarCnd, 31 );
							return;
						}
						break;
					}
				}
			}
			// 要求された区画情報(車室)が設定されていない、もしくは区画情報・処理区分異常の場合
			_ntnet_snd_data05( RecvNtnetDt.RData04.LockNo, 0, 6 );		// 接続無しで強制返信
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算ﾃﾞｰﾀ(ﾃﾞｰﾀ種別22)受信処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData22                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData22( void )
{
	ulong	ulwork;
	ushort	i;

	if(	(RecvNtnetDt.RData22.PayMethod == 5)||						// 定期券更新
		(RecvNtnetDt.RData22.LockNo == 0 )){						// 区画情報
		return;
	}
	if( (RecvNtnetDt.RData22.DataBasic.MachineNo == RcvDt22.MachineNo )&&	// 端末機械№
		(RecvNtnetDt.RData22.PayCount == RcvDt22.PayCount)&&				// 精算追番
		(RecvNtnetDt.RData22.LockNo	== RcvDt22.LockNo)&&					// 区画情報
		(RecvNtnetDt.RData22.PayClass == RcvDt22.PayClass)&&				// 処理区分
		(memcmp( &RecvNtnetDt.RData22.OutTime, &RcvDt22.OutTime, 
					sizeof(date_time_rec2) ) == 0 ) )						// 出庫年月日(精算年月日)
	{
		return;
	}
	RcvDt22.MachineNo = RecvNtnetDt.RData22.DataBasic.MachineNo;
	RcvDt22.PayCount = RecvNtnetDt.RData22.PayCount;
	RcvDt22.LockNo = RecvNtnetDt.RData22.LockNo;
	RcvDt22.PayClass = RecvNtnetDt.RData22.PayClass;
	memcpy( &RcvDt22.OutTime,&RecvNtnetDt.RData22.OutTime,sizeof(date_time_rec2) );

	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( !SetCarInfoSelect((short)i) ){
			continue;
		}
		ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// 区画情報Get
		if( RecvNtnetDt.RData22.LockNo == ulwork ){					// 要求された区画情報と一致?
			i++;
			if( RecvNtnetDt.RData22.PayClass == 2 ||
				RecvNtnetDt.RData22.PayClass == 3 ){
				// 精算中止、再精算中止
				mulchu_syu( i );									// ﾏﾙﾁ精算中止集計(精算ﾃﾞｰﾀ受信集計)
			}else if( RecvNtnetDt.RData22.PayClass == 0 ||			// 精算完了or再精算完了
					  RecvNtnetDt.RData22.PayClass == 1 ||
					  RecvNtnetDt.RData22.PayClass == 4 ||			// クレジット精算・再精算
					  RecvNtnetDt.RData22.PayClass == 5 ||			// クレジット再精算
					  RecvNtnetDt.RData22.PayClass == 14 ||			// 法人カード精算
					  RecvNtnetDt.RData22.PayClass == 15 ){			// 法人カード再精算
				// 精算完了
				queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof(i), &i );	// 精算完了ｷｭｰｾｯﾄ(ﾌﾗｯﾊﾟｰ下降)
				mulkan_syu( i );									// ﾏﾙﾁ精算完了集計(精算ﾃﾞｰﾀ受信集計)
				SvsTime_Syu[i-1] = (uchar)RecvNtnetDt.RData22.Syubet;	// 車室毎の精算時の料金種別set
				memcpy( &FLAPDT.flp_data[i-1].s_year, &RecvNtnetDt.RData22.OutTime.Year, 6 ); // 出庫(精算)時刻をセットする
				IoLog_write(IOLOG_EVNT_AJAST_FIN, (ushort)LockInfo[i - 1].posi, 0, 0);
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 満車制御情報変更			                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : car_park_condition                                      |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int		car_park_condition(int no, uchar new_cond)
{
	uchar	old_cond;

//	・NTNETで送られてくる満空制御１～３はcar_full[1～3]に保存する
//	・car_full[n]が変化したら操作モニタに登録する
//	 ※本体設定PPrmSSには影響しない

	if (new_cond) {
	// 変更あり
		old_cond = FLAGPT.car_full[no];
		switch( new_cond ){
		case	1:								// 強制満車
			FLAGPT.car_full[no] = 1;
			break;
		case	2:								// 強制空車
			FLAGPT.car_full[no] = 2;
			break;
		case	3:								// ほぼ満車
		case	4:								// 自動
			FLAGPT.car_full[no] = 0;
			break;
		default:								// 未定義なら書き換えなし
			break;
		}
		if (FLAGPT.car_full[no] != old_cond)
		// car_full[no]が変化した
			return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 制御ﾃﾞｰﾀ(ﾃﾞｰﾀ種別100)受信処理                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData100                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData100( void )
{
	ulong work;
	ulong req;
	RECEIVE_REC		tmp;
	req = 0;

	// 営休業切換
	memcpy(&tmp,&FLAGPT.receive_rec,sizeof(tmp));
	work = (ulong)(tmp.flg_ock);
	if( RecvNtnetDt.RData100.ControlData[0] == 1 ){						// 休業?
		tmp.flg_ock = 2;												// 強制休業切換
		tmp.flg_ocd = CLK_REC.ndat;
		tmp.flg_oct = CLK_REC.nmin;
	}
	else if( RecvNtnetDt.RData100.ControlData[0] == 2 ){				// 営業?
		tmp.flg_ock = 1;												// 強制営業切換
		tmp.flg_ocd = CLK_REC.ndat;
		tmp.flg_oct = CLK_REC.nmin;
	}
	else if( RecvNtnetDt.RData100.ControlData[0] == 3 ){				// 自動?
		tmp.flg_ock = 0;												// 営休業自動
		tmp.flg_ocd = 0;
		tmp.flg_oct = 0;
	}
	if( RecvNtnetDt.RData100.ControlData[0] ){							// 営休業切替指示あり?
		nmisave(&FLAGPT.receive_rec, &tmp, sizeof(RECEIVE_REC) );
		queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
	}
	if (work != (ulong)(tmp.flg_ock)) {
		wopelg(OPLOG_SET_EIKYUGYO, 0, 0);
	}
	
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	work = 0;
//	// 満車制御１
//	work |= (ulong)car_park_condition(1, RecvNtnetDt.RData100.ControlData[1]);
//	// 満車制御２
//	work |= (ulong)car_park_condition(2, RecvNtnetDt.RData100.ControlData[2]);
//	// 満車制御３
//	work |= (ulong)car_park_condition(3, RecvNtnetDt.RData100.ControlData[10]);
//	if (work) {
//	// どれかが変化したら
//		wopelg(OPLOG_SET_MANSYACTRL, 0, 0);
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

	// ｱﾝﾁﾊﾟｽ切換
	if( RecvNtnetDt.RData100.ControlData[3] == 1 ){						// 自動?
		PPrmSS[S_P01][3] = 0L;											// ｱﾝﾁﾊﾟｽﾁｪｯｸ自動
		wopelg( OPLOG_SET_NYUSYUTUCHK_ON, 0, 0 );		// 操作履歴登録
	}
	else if( RecvNtnetDt.RData100.ControlData[3] == 2 ){				// 解除?
		PPrmSS[S_P01][3] = 1L;											// ｱﾝﾁﾊﾟｽﾁｪｯｸ強制解除
		wopelg( OPLOG_SET_NYUSYUTUCHK_OFF, 0, 0 );		// 操作履歴登録
	}

	if( RecvNtnetDt.RData100.ControlData[7] == 1 ){						// リセット指示(管理データ要求)有り？
		req = 0L;
		if (prm_get(0, S_NTN, 43, 1, 1) == 1) {
			// 時計データ送信チェック
			req |= NTNET_MANDATA_CLOCK;
		}
		if (prm_get(0, S_NTN, 43, 1, 2) == 1) {
			// 制御データ送信チェック
			req |= NTNET_MANDATA_CTRL;
		}
		if (prm_get(0, S_NTN, 43, 1, 4) == 1) {
			// 定期券ステータスデータ送信チェック
			req |= NTNET_MANDATA_PASSSTS;
		}
		if (prm_get(0, S_NTN, 43, 1, 5) == 1) {
			// 定期券中止データ送信チェック
			req |= NTNET_MANDATA_PASSSTOP;
		}
		if (prm_get(0, S_NTN, 43, 1, 6) == 1) {
			// 定期券出庫時刻データ送信チェック
			req |= NTNET_MANDATA_PASSEXIT;
		}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		if (prm_get(0, S_NTN, 44, 1, 1) == 1) {
//			// 車室パラメータデータ送信チェック
//			req |= NTNET_MANDATA_LOCKINFO;
//		}
//		if (prm_get(0, S_NTN, 44, 1, 2) == 1) {
//			// ロック装置データ送信チェック
//			req |= NTNET_MANDATA_LOCKMARKER;
//		}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		if (prm_get(0, S_NTN, 44, 1, 4) == 1) {
			// 定期券更新データ送信チェック
			req |= NTNET_MANDATA_PASSCHANGE;
		}
		if (prm_get(0, S_NTN, 44, 1, 6) == 1) {
			// 特別日設定データ送信チェック
			req |= NTNET_MANDATA_SPECIALDAY;
		}
	}

	NTNET_Snd_Data101( RecvNtnetDt.RData100.DataBasic.MachineNo );		// NT-NET制御応答ﾃﾞｰﾀ作成
	if( req ){															// 管理ﾃﾞｰﾀ要求あり?
		NTNET_Snd_Data104(req);											// NT-NET管理ﾃﾞｰﾀ要求作成
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 管理ﾃﾞｰﾀ要求(ﾃﾞｰﾀ種別103)受信処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData103                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData103( void )
{
	ushort	i;

	char req = 0;
	ushort code;
	
	if (_is_ntnet_remote()) {
		for (i = 0; i < 70; i++) {
			if (RecvNtnetDt.RData103.ControlData[i]) {
				req++;
				code = i;
			}
		}
		switch(code) {
		case	0:	// 時計ﾃﾞｰﾀ要求
		case	18:	// 特別日設定データ要求
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		case	25:	// 駐車台数管理データ要求
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			break;
		default:
			req = 0;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
		if (req != 1) {
			// 管理データ要求NG送信
			code = 0;	// 未接続として返送
			NTNET_Snd_Data105(RecvNtnetDt.RData103.DataBasic.MachineNo, code);
			return;
		}
	}

	if( RecvNtnetDt.RData103.ControlData[0] ){						// 時計ﾃﾞｰﾀ要求あり
		NTNET_Snd_Data229_Exec();										// NT-NET時計ﾃﾞｰﾀ作成
	}
	if (_is_ntnet_remote() == 0) {
		if( RecvNtnetDt.RData103.ControlData[2] ){						// 共通設定ﾃﾞｰﾀ要求要求あり
			for (i = S_SYS; i < C_PRM_SESCNT_END; i++) {
				NTNET_Snd_Data208(RecvNtnetDt.RData103.DataBasic.MachineNo, 0, i);	// NT-NET共通設定ﾃﾞｰﾀ作成
			}
			NTNET_Snd_Data208(RecvNtnetDt.RData103.DataBasic.MachineNo, 1, i);		// NT-NET共通設定ﾃﾞｰﾀ作成
		}
		if( RecvNtnetDt.RData103.ControlData[13] ){						// 定期券精算中止ﾃｰﾌﾞﾙﾃﾞｰﾀ要求あり
			NTNET_Snd_Data219(2, NULL);									// NT-NET定期券精算中止ﾃｰﾌﾞﾙﾃﾞｰﾀ作成
		}
		if( RecvNtnetDt.RData103.ControlData[15] ){						// 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙﾃﾞｰﾀ要求あり
			NTNET_Snd_Data221();										// NT-NET定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙﾃﾞｰﾀ作成
		}
		if( RecvNtnetDt.RData103.ControlData[16] ){						// 定期券更新ﾃｰﾌﾞﾙﾃﾞｰﾀ要求あり
			NTNET_Snd_Data222();										// NT-NET定期券更新ﾃｰﾌﾞﾙﾃﾞｰﾀ作成
		}
		if( RecvNtnetDt.RData103.ControlData[17] ){						// 定期出庫時刻ﾃｰﾌﾞﾙﾃﾞｰﾀ要求あり
			NTNET_Snd_Data223();										// NT-NET定期券出庫時刻ﾃｰﾌﾞﾙﾃﾞｰﾀ作成
		}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		if( RecvNtnetDt.RData103.ControlData[19] ){						// 車室ﾊﾟﾗﾒｰﾀ要求あり
//			NTNET_Snd_Data225(RecvNtnetDt.RData103.DataBasic.MachineNo);	// NT-NET車室ﾊﾟﾗﾒｰﾀ作成
//		}
//		if( RecvNtnetDt.RData103.ControlData[20] ){						// ﾛｯｸ装置ﾊﾟﾗﾒｰﾀ要求あり
//			NTNET_Snd_Data226(RecvNtnetDt.RData103.DataBasic.MachineNo);	// NT-NETﾛｯｸ装置ﾊﾟﾗﾒｰﾀ作成
//		}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	}

	if (RecvNtnetDt.RData103.ControlData[18]) {		// 特別日設定ﾃﾞｰﾀ
		NTNET_Snd_Data211_Exec();
	}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	if (RecvNtnetDt.RData103.ControlData[25]) {		// 駐車台数管理ﾃﾞｰﾀ
//		NTNET_Snd_Data235_Exec(1);
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
}


/*[]----------------------------------------------------------------------[]*/
/*| ﾃﾞｰﾀ要求2(ﾃﾞｰﾀ種別109)受信処理                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData109                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData109( void )
{
	char req = 0;
	ushort code, i;

	if (_is_ntnet_remote()) {
		for (i = 0; i < 20; i++) {
			if (RecvNtnetDt.RData109.ControlData[i]) {
				req++;
				code = i;
			}
		}
		switch(code) {
		case	5:	// コイン金庫小計データ
		case	6:	// 紙幣金庫小計データ
		case	7:	// 動作カウントデータ
		case	8:	// 端末情報ﾃﾞｰﾀ要求
		case	9:	// 端末状態ﾃﾞｰﾀ要求
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		case	11:	// 簡易車室情報ﾃｰﾌﾞﾙ要求
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			break;
		default:
			req = 0;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
		if (req != 1) {
			// 管理データ要求NG送信
			code = 0;	// 未接続として返送
			NTNET_Snd_Data110(RecvNtnetDt.RData109.DataBasic.MachineNo, code);
			return;
		}
	}

	if( RecvNtnetDt.RData109.ControlData[0] ){							// T小計データ要求あり
		NTNET_Snd_TSYOUKEI();	// T小計データ作成
	}
	if( RecvNtnetDt.RData109.ControlData[8] ){							// 端末情報ﾃﾞｰﾀ要求あり
		NTNET_Snd_Data230( RecvNtnetDt.RData109.DataBasic.MachineNo );	// NT-NET端末情報ﾃﾞｰﾀ作成
	}
	if( RecvNtnetDt.RData109.ControlData[9] ){							// 端末状態ﾃﾞｰﾀ要求あり
		NTNET_Snd_Data231( RecvNtnetDt.RData109.DataBasic.MachineNo );	// NT-NET端末状態ﾃﾞｰﾀ作成
	}
	if( RecvNtnetDt.RData109.ControlData[10] ){							// 金銭管理データ要求あり
		NTNET_Snd_Data126( RecvNtnetDt.RData109.DataBasic.MachineNo, 40 );	// NT-NET金銭管理データ作成
	}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	if( RecvNtnetDt.RData109.ControlData[11] ){							// 簡易車室情報ﾃｰﾌﾞﾙ要求あり
//		NTNET_Snd_Data12( RecvNtnetDt.RData109.DataBasic.MachineNo );	// NT-NET簡易車室情報ﾃｰﾌﾞﾙ作成
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	if (RecvNtnetDt.RData109.ControlData[5]) {		// コイン金庫小計データ送信
		NTNET_Snd_Data130();
	}
	if (RecvNtnetDt.RData109.ControlData[6]) {		// 紙幣金庫小計データ送信
		NTNET_Snd_Data132();
	}
	if (RecvNtnetDt.RData109.ControlData[7]) {		// 動作カウントデータ送信
		NTNET_Snd_Data228(RecvNtnetDt.RData109.DataBasic.MachineNo);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| センタ用データ要求(ﾃﾞｰﾀ種別154)受信処理                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData154                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData154( void )
{
	char req = 0;
	ushort code;

	if (_is_ntnet_remote()) {
		if( RecvNtnetDt.RData154.SeisanJyoutai_Req ){	// 精算状態データ要求
			req++;
		}
		if( RecvNtnetDt.RData154.RealTime_Req ){		// リアルタイム情報要求
			req++;
		}
		if( RecvNtnetDt.RData154.TermInfo_Req ) {		// 端末情報データ要求
			req++;
		}
		if( RecvNtnetDt.RData154.SynchroTime_Req ) {	// 同期時刻データ要求
			req++;
		}
		if (req != 1) {
			// センタ用データ要求NG送信
			code = 9;								// 9:パラメータNG
			NTNET_Snd_Data155(RecvNtnetDt.RData154.DataBasic.MachineNo, code);
			return;
		}
		if( RecvNtnetDt.RData154.SeisanJyoutai_Req ){	// 精算状態データ要求
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
//			NTNET_Snd_Data152();						// 精算状態データ要求
// MH810105(S) MH364301 QRコード決済対応
//			NTNET_Snd_Data152_r12();					// 精算状態データ要求
			NTNET_Snd_Data152_rXX();					// 精算状態データ要求
// MH810105(E) MH364301 QRコード決済対応
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
		}
		if( RecvNtnetDt.RData154.RealTime_Req ){		// リアルタイム情報要求
			NTNET_Snd_Data153();						// リアルタイム情報要求
		}
		if( RecvNtnetDt.RData154.TermInfo_Req ) {		// 端末情報データ要求
			// センター用データ要求結果を送信
			code = 100;									// 100:正常受付（端末情報要求に対してのみ）
			NTNET_Snd_Data155(RecvNtnetDt.RData109.DataBasic.MachineNo, code);
			NTNET_Snd_Data65(RecvNtnetDt.RData109.DataBasic.MachineNo);
		}
		if( RecvNtnetDt.RData154.SynchroTime_Req ) {	// 同期時刻データ要求
			code = 0;								// 0:正常
			NTNET_Snd_Data157(RecvNtnetDt.RData154.DataBasic.MachineNo, code ,NTNET_SYNCHRO_GET );
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 同期時刻設定要求(ﾃﾞｰﾀ種別156)受信処理                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData156                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData156( void )
{
	char err;
	ushort code;
	ulong 	date1;

	err = 0;
	if (_is_ntnet_remote()) {
		if( 0 == RecvNtnetDt.RData156.Time_synchro_Req ){	// 同期時刻設定要求なしの場合
			err = 1;
		}
		if( 23 < RecvNtnetDt.RData156.synchro_hour ){		// 同期時刻 23より大きい場合はNG
			err = 1;
		}
		if( 59 < RecvNtnetDt.RData156.synchro_minu ){		// 同期時刻 59より大きい場合はNG
			err = 1;
		}
		if ( 1 == err ) {
			// センタ用データ要求NG送信
			code = 9;	// 9:パラメータNG
		} else {
			// 同期時間を共通パラメータへセット
			date1 = CPrmSS[S_NTN][122];
			date1 /= 10000;												// 上2桁は変更しない
			date1 *= 100;
			date1 += (ulong)RecvNtnetDt.RData156.synchro_hour;
			date1 *= 100;
			date1 += (ulong)RecvNtnetDt.RData156.synchro_minu;
			CPrmSS[S_NTN][122] = date1;									// 同期時刻をセット

			DataSumUpdate(OPE_DTNUM_COMPARA);							// 共通設定ﾃﾞｰﾀｻﾑを更新
			FLT_WriteParam1(FLT_NOT_EXCLUSIVE);							// FlashROM update
			SetSetDiff(SETDIFFLOG_SYU_REMSET);							// デフォルトセットログ登録処理
			wopelg(OPLOG_SET_SNTP_SYNCHROTIME, 0, 0);					// 操作履歴登録
			code = 0;	// 0:正常
		}
		NTNET_Snd_Data157(RecvNtnetDt.RData156.DataBasic.MachineNo, code, NTNET_SYNCHRO_SET);
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 時計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別119)受信処理                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData119                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData119( void )
{
	struct	clk_rec wk_CLK_REC;				// 受信電文内容(バイナリ)
	ushort	wk_CLK_REC_msec;				//							ミリ秒
	ushort	Setting_Supplement_msec;		// 設定の遅延時間値
	ushort	NormDay;						// 年月日ノーマライズ値
	ulong	NormTime;						// 時分秒ミリ秒ノーマライズ値
	ushort	WaitTime_ForTimeSet;			// 時計セットまでのWAIT時間
	ulong	wkul;
	ushort	wkus;

	if( prm_get(COM_PRM,S_NTN,29,1,1) != 0 ){
	// 34-0029が０以外の時は受信処理しない。
		return;
	}
	// NT-NET 時計データ新電文に対応（補正値つき）
	// 時計更新要求は常に受け付けることする

	wk_CLK_REC.year = RecvNtnetDt.RData119.Year;
	wk_CLK_REC.mont = RecvNtnetDt.RData119.Mon;
	wk_CLK_REC.date = RecvNtnetDt.RData119.Day;
	wk_CLK_REC.hour = RecvNtnetDt.RData119.Hour;
	wk_CLK_REC.minu = RecvNtnetDt.RData119.Min;
	wk_CLK_REC.seco = (uchar)RecvNtnetDt.RData119.Sec;
	wk_CLK_REC_msec = RecvNtnetDt.RData119.MSec;

	// 受信電文の有効性チェック
	if( !c_ClkCheck(&wk_CLK_REC, &wk_CLK_REC_msec) ){		// 時計値NG
		return;
	}

	// 補正値Limitチェック
	if(_is_ntnet_remote() == 0){							// NT-NET
		wkul = prm_get(COM_PRM,S_NTN,34,2,5);				// 補正値有効時間設定Get(秒)get(34-0034①②)
	}else{													// 遠隔NT-NET
		wkul = prm_get(COM_PRM,S_CEN,76,2,5);				// 補正値有効時間設定Get(秒)get(36-0076①②)
	}
	wkul *= 1000L;											// 単位をmsに変換
	if( wkul < RecvNtnetDt.RData119.HOSEI_MSec ){			// 有効時間外
		return;
	}

	// 時計設定値をノーマライズして補正時刻加算
	c_Normalize_ms( &wk_CLK_REC, &wk_CLK_REC_msec, &NormDay, &NormTime );	// 設定時刻ノーマライズ値(ms)get

	if(_is_ntnet_remote() == 0){											// NT-NET
		Setting_Supplement_msec = (ushort)prm_get(COM_PRM,S_NTN,34,4,1);	// 遅延時間設定値(ms)get(34-0034③④⑤⑥)
	}else{																	// 遠隔NT-NET
		Setting_Supplement_msec = (ushort)prm_get(COM_PRM,S_CEN,76,4,1);	// 遅延時間設定値(ms)get(36-0076③④⑤⑥)
	}

	wkul = c_2msPastTimeGet( RecvNtnetDt.RData119.HOSEI_wk );				// Arcnet受信～現在までの経過時間(ms)get

	wkul = NormTime + RecvNtnetDt.RData119.HOSEI_MSec + Setting_Supplement_msec + wkul;	
																			// 補正値を加算した時刻ノーマライズ値 算出

	// ガード処理（ないと考えるがガード）
	if( NormTime > wkul ){									// 補正したら時計値がマイナスになる場合は補正しない
		;
	}else{
		NormTime = wkul;
	}

	// 時計の更新(set)は、本電文を元に、次の00ミリ秒のタイミングで行なう
	// ここでは、次の00秒までの時間(ミリ秒)と、その時の時刻値を求める
	wkus = (ushort)(NormTime % 1000);						// ミリ秒時間get
	WaitTime_ForTimeSet = 1000 - wkus;						// 次の00秒までの時間get
	NormTime += (ulong)WaitTime_ForTimeSet;					// 時間を次の00秒にする

	// 補正値加算した時計設定値をアンノーマライズ
	c_UnNormalize_ms( &NormDay, &NormTime, &wk_CLK_REC, &wk_CLK_REC_msec );

	xPause2ms( (ulong)(WaitTime_ForTimeSet/2) );			// 00ミリ秒となるまでwait

	// 時計更新
	Ope_clk_set( &wk_CLK_REC, OPLOG_SET_TIME2_COM );
}

/*[]----------------------------------------------------------------------[]*/
/*| 時計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別119)sub処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| NT-NET lowerモジュールが時計データ受信と判断した直後にCallされる関数   |*/
/*|	0カットデータ伸張した後にCallされる。								   |*/
/*|	LifeTime値をセットする												   |*/
/*|	旧電文でも（電文長が短くても）長くして新電文（データはオール0）とする  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : *dat = NT-NET電文 ID3からのデータへのポインタ		   |*/
/*|				   siz = datのデータサイズ(ID3から)						   |*/
/*| RETURN VALUE : 追加したデータ長                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2009/05/12                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void NTNET_AfterRcv_CallBack()
{
	ulong	LifeTime;

	LifeTime = LifeTim2msGet();
	RecvNtnetDt.RData119.HOSEI_wk = LifeTime;				// +37(予約)に受信時点のLifeTime値をset
}

/*[]----------------------------------------------------------------------[]*/
/*| 駐車場番号＋定期券ID変換処理 		                          		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Convert_PassInfo                                        |*/
/*| PARAMETER    : ParkingNo:駐車場番号                                    |*/
/*|		 : PassID   :定期券ID					   						   |*/
/*| RETURN VALUE : 変換した値                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.namioka                                               |*/
/*| Date         : 2007-12-04                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ulong	Convert_PassInfo( ulong ParkingNo, ulong PassID ){

	ulong ul_Prm;

	ul_Prm = ParkingNo;
	ul_Prm *= 100000L;				// 定期券駐車場set
	ul_Prm += PassID;				// 定期券IDset(1～12000)

	return( ul_Prm );
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券問合せ結果ﾃﾞｰﾀ(ﾃﾞｰﾀ種別143)受信処理                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData143                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-11-04                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData143( void )
{
	ushort status;
	ulong use_parking;
	ushort valid;
	uchar	parking_no_check=0;

	status = 0xFFFF;
	use_parking = 0xFFFFFFFF;
	valid = 0xFFFF;

	if (RecvNtnetDt.RData116.UsingParkingNo != 0) {
		use_parking = (ulong)RecvNtnetDt.RData143.UseParkingNo;
	}else{
		return;
	}

	parking_no_check = NTNET_GetParkingKind( RecvNtnetDt.RData143.ParkingNo, PKOFS_SEARCH_LOCAL );

	if (RecvNtnetDt.RData143.PassState == NTNET_PASSUPDATE_INVALID) {
		if( parking_no_check != 0xFF ){
			if(RecvNtnetDt.RData143.ParkingNo < 1000 ){	//APS
				wopelg(OPLOG_SET_TEIKI_YUKOMUKO, 0, Convert_PassInfo(RecvNtnetDt.RData143.ParkingNo,RecvNtnetDt.RData143.PassID));		// 操作履歴登録
			}else{											//GT
				wopelg(OPLOG_GT_SET_TEIKI_YUKOMUKO, RecvNtnetDt.RData143.ParkingNo, RecvNtnetDt.RData143.PassID);		// 操作履歴登録
			}
		}
		valid = 1;
	} else {
		if( parking_no_check != 0xFF ){
			if(RecvNtnetDt.RData143.ParkingNo < 1000 ){				//APS
				wopelg(OPLOG_SET_STATUS_CHENGE, 0, Convert_PassInfo(RecvNtnetDt.RData143.ParkingNo,RecvNtnetDt.RData143.PassID));		// 操作履歴登録
			}else{														//GT
				wopelg(OPLOG_GT_SET_STATUS_CHENGE, RecvNtnetDt.RData143.ParkingNo, RecvNtnetDt.RData143.PassID);		// 操作履歴登録
			}
		}
		valid = 0;
		status = RecvNtnetDt.RData143.PassState;
	}
	WritePassTbl( (ulong)RecvNtnetDt.RData143.ParkingNo,
				  (ushort)RecvNtnetDt.RData143.PassID,
				  (ushort)(status),
				  (ulong)use_parking,
				  (ushort)valid );

	if( RecvNtnetDt.RData143.ParkingNo == OPECTL.ChkPassPkno &&		// 問合せした駐車場№と同様?
		RecvNtnetDt.RData143.PassID == OPECTL.ChkPassID ){			// 問合せした定期券IDと同様?

		queset( OPETCBNO, IBK_NTNET_CHKPASS, 22, &RecvNtnetDt.RData143.Reserve );	// NTNET 定期問合せ結果ﾃﾞｰﾀ受信通知
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 無効係員IDテーブルデータ(ﾃﾞｰﾀ種別150)受信処理                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData150                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-06-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_RevData150( void )
{
}

/* 係員ID状態データ電文用ステータス */
#define	NTNET_KAKARI_YUKOU		0x00	/* 係員ID有効 */
#define	NTNET_KAKARI_MUKOU		0x03	/* 係員ID無効 */

/*[]----------------------------------------------------------------------[]*/
/*| 係員ID状態データ(ID254)受信処理                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData254		                                   |*/
/*| PARAMETER    : data  : 係員ID状態データ電文                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-06-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_RevData254( void )
{
	return ;
}

/* 係員保守データ送信用ステータス */
#define	NTNET_KAKARI_NYUKO		0x00	/* 入庫							*/
#define	NTNET_KAKARI_MNTSTART	0x01	/* 係員ｶｰﾄﾞによりﾒﾝﾃﾅﾝｽ開始		*/
#define	NTNET_KAKARI_MNTEND		0x02	/* ﾒﾝﾃﾅﾝｽ終了					*/
#define	NTNET_KAKARI_SYUKKO		0x03	/* 出庫							*/
#define	NTNET_KAKARI_KAISYUU	0x04	/* 係員ｶｰﾄﾞ無効のためｶｰﾄﾞ回収	*/

/*[]----------------------------------------------------------------------[]*/
/*| 係員保守ﾃﾞｰﾀ(ID108)作成送信処理                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNETDOPA_Snd_Data108                                   |*/
/*| PARAMETER    : Status  : 係員保守データステータス                      |*/
/*|             0:入庫 (NTDOPA_KAKARI_NYUKO)                               |*/
/*|             1:係員カードによりメンテナンス開始 (NTDOPA_KAKARI_MNTSTAR) |*/
/*|             2:メンテナンス終了 (NTDOPA_KAKARI_MNTEND)                  |*/
/*|             3:出庫 (NTDOPA_KAKARI_SYUKKO)                              |*/
/*|             4:係員カード無効の為カード回収 (NTDOPA_KAKARI_KAISYUU)     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-06-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_Snd_Data108(ushort KakariNo, uchar Status)
{
	/* ステータス値チェック */
	if(Status != NTNET_KAKARI_NYUKO	&&
	   Status != NTNET_KAKARI_MNTSTART	&&
	   Status != NTNET_KAKARI_MNTEND	&&
	   Status != NTNET_KAKARI_SYUKKO	&&
	   Status != NTNET_KAKARI_KAISYUU	){
		return ;	
	}

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_108 ) );

	BasicDataMake( 108, 0x01 );									// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData108.ParkingNo	= (ulong)CPrmSS[S_SYS][1];	// 駐車場№
	SendNtnetDt.SData108.KakariNo	= KakariNo;					// 係員No.
	SendNtnetDt.SData108.Status		= Status;					// ステータス
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_108 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_108 ), NTNET_BUF_NORMAL );	// ﾃﾞｰﾀ送信登録
	}
	return ;
}

/*[]----------------------------------------------------------------------[]*/
/*| 基本ﾃﾞｰﾀ作成処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BasicDataMake                                           |*/
/*| PARAMETER    : knd  : ﾃﾞｰﾀ種別                                         |*/
/*|                keep : ﾃﾞｰﾀ保持ﾌﾗｸﾞ                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	BasicDataMake( uchar knd, uchar keep )
{
	// ｼｽﾃﾑID
// MH322914(S) K.Onodera 2016/08/10 AI-V対応
//	SendNtnetDt.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];
	switch( knd ){
		case PIP_RES_KIND_REMOTE_CALC_ENTRY:
		case PIP_RES_KIND_REMOTE_CALC_FEE:
		case PIP_RES_KIND_OCCUR_ERRALM:
		case PIP_RES_KIND_RECEIPT_AGAIN:
		case PIP_RES_KIND_FURIKAE:
		case PIP_RES_KIND_RECEIVE_TKT:
			// システムID16の応答？
			if( RecvNtnetDt.DataBasic.SystemID == PIP_SYSTEM ){
				SendNtnetDt.DataBasic.SystemID = PIP_SYSTEM;
				break;
			}
			// no break;
		default:
			SendNtnetDt.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];
																	// 0:ﾌﾗｯﾌﾟｼｽﾃﾑ
																	// 1:遠隔ﾌﾗｯﾌﾟｼｽﾃﾑ
																	// 2:ﾌﾗｯﾌﾟ･駐輪ｼｽﾃﾑ
																	// 3:遠隔ﾌﾗｯﾌﾟ･駐輪ｼｽﾃﾑ
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
																	// 4:ｹﾞｰﾄ式ｼｽﾃﾑ
																	// 5:遠隔ｹﾞｰﾄ式ｼｽﾃﾑ
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		break;
	}
// MH322914(E) K.Onodera 2016/08/10 AI-V対応

	SendNtnetDt.DataBasic.DataKind = knd;							// ﾃﾞｰﾀ種別
	SendNtnetDt.DataBasic.DataKeep = keep;							// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	SendNtnetDt.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];		// 駐車場№
	SendNtnetDt.DataBasic.ModelCode = NTNET_MODEL_CODE;				// 機種ｺｰﾄﾞ

	SendNtnetDt.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];		// 機械№
	if(( knd == 100 )||( knd == 109 ))
		SendNtnetDt.DataBasic.MachineNo = (ulong)CPrmSS[S_NTN][3];	// 送信元ターミナル№
	if( ((knd >= 30) && (knd <= 36)) || (knd == 41)||	//T合計データの場合は集計した時間をセットする
	    ((knd >=158) && (knd <=164)) || (knd ==169)){	//T小計データの場合は集計した時間をセットする
		SendNtnetDt.DataBasic.Year = (uchar)( NTNetTTotalTime.Year % 100 );	// 年
		SendNtnetDt.DataBasic.Mon =  NTNetTTotalTime.Mon;					// 月
		SendNtnetDt.DataBasic.Day =  NTNetTTotalTime.Day;					// 日
		SendNtnetDt.DataBasic.Hour = NTNetTTotalTime.Hour;					// 時
		SendNtnetDt.DataBasic.Min =  NTNetTTotalTime.Min;					// 分
		SendNtnetDt.DataBasic.Sec =  (uchar)NTNetTTotalTime.Sec;			// 秒
	}else if(knd == 152){
		switch(PayInfo_Class){										// 精算情報データ用処理区分を参照する
		case 8:														// 精算前は要求された時刻をセット
			SendNtnetDt.DataBasic.Year = (uchar)( CLK_REC.year % 100 );		// 年
			SendNtnetDt.DataBasic.Mon = (uchar)CLK_REC.mont;				// 月
			SendNtnetDt.DataBasic.Day = (uchar)CLK_REC.date;				// 日
			SendNtnetDt.DataBasic.Hour = (uchar)CLK_REC.hour;				// 時
			SendNtnetDt.DataBasic.Min = (uchar)CLK_REC.minu;				// 分
			SendNtnetDt.DataBasic.Sec = (uchar)CLK_REC.seco;				// 秒
			break;
		case 0:														// 精算後は前回精算時刻をセット
		case 2:														// 精算中止は前回精算中止時刻をセット
// MH322914 (s) kasiyama 2016/07/11 精算情報応答データの処理年月日に精算時刻が入らない(クレジット)[共通バグNo.1179]
		// 予備元で処理区分は0になる
		case 4:
// MH322914 (e) kasiyama 2016/07/11 精算情報応答データの処理年月日に精算時刻が入らない(クレジット)[共通バグNo.1179]
			SendNtnetDt.DataBasic.Year = (uchar)( NTNetTime_152.Year % 100 );	// 年
			SendNtnetDt.DataBasic.Mon =  NTNetTime_152.Mon;						// 月
			SendNtnetDt.DataBasic.Day =  NTNetTime_152.Day;						// 日
			SendNtnetDt.DataBasic.Hour = NTNetTime_152.Hour;					// 時
			SendNtnetDt.DataBasic.Min =  NTNetTime_152.Min;						// 分
			SendNtnetDt.DataBasic.Sec =  (uchar)NTNetTime_152.Sec;				// 秒
			break;
		case 9:															// 精算中は精算開始時刻をセット
			SendNtnetDt.DataBasic.Year = (uchar)( NTNetTime_152_wk.Year % 100 );	// 年
			SendNtnetDt.DataBasic.Mon =  NTNetTime_152_wk.Mon;						// 月
			SendNtnetDt.DataBasic.Day =  NTNetTime_152_wk.Day;						// 日
			SendNtnetDt.DataBasic.Hour = NTNetTime_152_wk.Hour;						// 時
			SendNtnetDt.DataBasic.Min =  NTNetTime_152_wk.Min;						// 分
			SendNtnetDt.DataBasic.Sec =  (uchar)NTNetTime_152_wk.Sec;				// 秒
			break;
		}
	}else{
		SendNtnetDt.DataBasic.Year = (uchar)( CLK_REC.year % 100 );		// 年
		SendNtnetDt.DataBasic.Mon = (uchar)CLK_REC.mont;				// 月
		SendNtnetDt.DataBasic.Day = (uchar)CLK_REC.date;				// 日
		SendNtnetDt.DataBasic.Hour = (uchar)CLK_REC.hour;				// 時
		SendNtnetDt.DataBasic.Min = (uchar)CLK_REC.minu;				// 分
		SendNtnetDt.DataBasic.Sec = (uchar)CLK_REC.seco;				// 秒
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 車室問合せﾃﾞｰﾀ(ﾃﾞｰﾀ種別01)作成処理                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data01                                        |*/
/*| PARAMETER    : op_lokno : 接客用駐車位置番号(上2桁A～Z,下4桁1～9999)   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data01( ulong op_lokno )
{
	if (_is_ntnet_remote()) {
		return;
	}

	memset( &LOCKMULTI, 0, sizeof( lock_multi ) );
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_01 ) );

	BasicDataMake( 1, 1 );											// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData01.LockNo = op_lokno;							// 区画情報

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_01 ));

	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_01 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 車室問合せ結果ﾃﾞｰﾀ(ﾃﾞｰﾀ種別02)作成処理                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data02                                        |*/
/*| PARAMETER    : op_lokno : 接客用駐車位置番号(上2桁A～Z,下4桁1～9999)   |*/
/*|                pr_lokno : 内部処理用駐車位置番号(1～324)               |*/
/*|                ans      : 結果 0=正常, 1=未接続(該当無し)              |*/
/*|                                2=手動ﾓｰﾄﾞ中                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data02( ulong op_lokno, ushort pr_lokno, ushort ans )
{
	ushort	lkno;
	uchar	cnt;
	uchar	i,j;
	ulong	cal;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_02 ) );

	BasicDataMake( 2, 1 );											// 基本ﾃﾞｰﾀ作成
	// 駐車場№～処理秒までは車室問合せﾃﾞｰﾀにて受信したﾃﾞｰﾀをそのままｾｯﾄする
	memcpy( &SendNtnetDt.DataBasic.ParkingNo, &RecvNtnetDt.DataBasic.ParkingNo, 22 );

	SendNtnetDt.SData02.LockNo = op_lokno;							// 区画情報
	SendNtnetDt.SData02.Answer = ans;								// 結果
	if( ans == 0 || ans == 2 ){		// 結果＝「正常」または「手動ﾓｰﾄﾞ中」？
		lkno = pr_lokno - 1;

		FLAPDT.flp_data[lkno].ryo_syu = (ushort)LockInfo[lkno].ryo_syu;

		memcpy( SendNtnetDt.SData02.LkInfo,
				&FLAPDT.flp_data[lkno].mode,
				sizeof( SendNtnetDt.SData02.LkInfo ) );				// 車室情報
		SendNtnetDt.SData02.LkInfo[2] &= 0x03;
		SendNtnetDt.SData02.LkInfo[4] &= 0x03;
		memset( &SendNtnetDt.SData02.LkInfo[26],0,sizeof(char)*8);	//中止料金、領収額に精算完了ﾃﾞｰﾀがｾｯﾄされる為ｸﾘｱ
		cal = OrgCal( (ushort)lkno );
		memcpy( &SendNtnetDt.SData02.LkInfo[26], &cal, 4);
		memset( &SendNtnetDt.SData02.LkInfo[46], 0, 2 );	// 入庫判定ﾀｲﾏ/ﾌﾗｯﾌﾟ上昇ﾀｲﾏは不要なのでセットしない
		
		cnt = cyushi_chk( lkno );			//詳細中止検索
		if( cnt != 99 ){
			SendNtnetDt.SData02.PpcData = FLAPDT_SUB[cnt].ppc_chusi_ryo;
			for(i=0 ; i<15 ; i++)
				SendNtnetDt.SData02.SvsTiket[i] = FLAPDT_SUB[cnt].sev_tik[i];
			for(i=0,j=0 ; i<5 ; i++,j+=2){
				SendNtnetDt.SData02.KakeTiket[j] = FLAPDT_SUB[cnt].kake_data[i].mise_no;
				SendNtnetDt.SData02.KakeTiket[j+1] = FLAPDT_SUB[cnt].kake_data[i].maisuu;
			}
		}
	}

	if( ans == 0 ){
		IoLog_write(IOLOG_EVNT_AJAST_STA, (ushort)op_lokno, 0, 0);
	}
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_02 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_02 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ﾛｯｸ制御結果ﾃﾞｰﾀ(ﾃﾞｰﾀ種別05)作成処理                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data05                                        |*/
/*| PARAMETER    : op_lokno : 接客用駐車位置番号(上2桁A～Z,下4桁1～9999)   |*/
/*|                loksns   : 車両検知状態                                 |*/
/*|                lokst    : ﾛｯｸ装置状態                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data05( ulong op_lokno, uchar loksns, uchar lokst )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_05 ) );

	BasicDataMake( 5, 1 );											// 基本ﾃﾞｰﾀ作成

	// 駐車場№～処理秒までは車室問合せﾃﾞｰﾀにて受信したﾃﾞｰﾀをそのままｾｯﾄする
	memcpy( &SendNtnetDt.DataBasic.ParkingNo, &RecvBackUp.ParkingNo, 22 );

	SendNtnetDt.SData05.LockNo = op_lokno;							// 区画情報
	SendNtnetDt.SData05.LockSense = loksns;							// 車両検知状態
	SendNtnetDt.SData05.LockState = lokst;							// ﾛｯｸ装置状態

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_05 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_05 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}

	RecvBackUp.ReceiveFlg = 0;										// 要求未受信
}

/*[]----------------------------------------------------------------------[]*/
/*| 簡易車室情報ﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別12)作成処理                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data12                                        |*/
/*| PARAMETER    : MachineNo : 送信先端末機械№                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data12( ulong MachineNo )
{
	short	i;
	short	j = 0;
	ushort	len = 0;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_12 ) );

	BasicDataMake( 12, 1 );											// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData12.SMachineNo = MachineNo;						// 送信先端末機械№
	len = sizeof(DATA_BASIC) + 4;	// 基本データ＋送信先端末機械№
	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行

		if (_is_ntnet_normal() || FlpSetChk((ushort)i)) {
			// 有効な車室情報のみセット
			if( !SetCarInfoSelect(i) ){
				continue;
			}
			if( j >= OLD_LOCK_MAX ){
				break;
			}
			SendNtnetDt.SData12.LockState[j].LockNo =
				(ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// 区画情報

			SendNtnetDt.SData12.LockState[j].NowState =
									FLAPDT.flp_data[i].nstat.word & 0x01FF;		// 現在ｽﾃｰﾀｽ

			SendNtnetDt.SData12.LockState[j].Year =
									FLAPDT.flp_data[i].year;			// 入庫年

			SendNtnetDt.SData12.LockState[j].Mont =
									FLAPDT.flp_data[i].mont;			// 入庫月

			SendNtnetDt.SData12.LockState[j].Date =
									FLAPDT.flp_data[i].date;			// 入庫日

			SendNtnetDt.SData12.LockState[j].Hour =
									FLAPDT.flp_data[i].hour;			// 入庫時

			SendNtnetDt.SData12.LockState[j].Minu =
									FLAPDT.flp_data[i].minu;			// 入庫分

			SendNtnetDt.SData12.LockState[j].Syubet =
									(ushort)LockInfo[i].ryo_syu;		// 料金種別

			SendNtnetDt.SData12.LockState[j].TyuRyo = 0L;				// 駐車料金(未使用)

			j++;
			len += sizeof(LOCK_STATE);
		}
	}
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, len, NTNET_BUF_NORMAL );	// ﾃﾞｰﾀ送信登録
	}
}
void	NTNET_Snd_ParkCarNumDataMk(void *p, uchar knd)
{
	int i;
	PARKCAR_DATA11	*p1;
	PARKCAR_DATA21	*p2;

	// リアルタイムな満空車判別を取得する為に、現在台数ﾁｪｯｸを行う
	//（5秒ごとに行なわれる信号出力ﾁｪｯｸsig_chk()内でコールされているが、リアルタイムな情報にする為）
	fulchk();																// 現在台数ﾁｪｯｸ

	if( knd == 0 ) {
		p1 = (PARKCAR_DATA11 *)p;

		memset( p1, 0, sizeof( PARKCAR_DATA11 ) );

		// 定期車両カウント 0固定

		if (_is_ntnet_remote()) {											// 遠隔NT-NET設定のときのみデータセット
			if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {					// 01-0039⑤駐車台数を本体で管理する
				switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
				case 0:														// 01-0039⑥用途別駐車台数切替なし
					p1->FullNo1 = (ushort)PPrmSS[S_P02][3];					// 満車台数
					p1->CarCnt1 = (ushort)PPrmSS[S_P02][2];					// 現在台数
					switch (PPrmSS[S_P02][1]) {								// 強制満空車ﾓｰﾄﾞ
					case	1:												// 強制満車
						p1->Full[0] = 11;									// 「強制満車」状態セット
						break;
					case	2:												// 強制空車
						p1->Full[0] = 10;									// 「強制空車」状態セット
						break;
					default:												// 自動
						p1->Full[0] = CARFULL_stat[0];						// 「空車or満車」状態セット
						break;
					}
					break;
				case 3:														// 01-0039⑥用途別駐車台数２系統
				case 4:														// 01-0039⑥用途別駐車台数３系統
					p1->FullNo1 = (ushort)PPrmSS[S_P02][7];					// 満車台数
					p1->CarCnt1 = (ushort)PPrmSS[S_P02][6];					// 現在台数１
					switch (PPrmSS[S_P02][5]) {								// 強制満空車モード1
					case	1:												// 強制満車
						p1->Full[0] = 11;									// 「強制満車」状態セット
						break;
					case	2:												// 強制空車
						p1->Full[0] = 10;									// 「強制空車」状態セット
						break;
					default:												// 自動
						p1->Full[0] = CARFULL_stat[0];						// 「空車or満車」状態セット
						break;
					}

					p1->FullNo2 = (ushort)PPrmSS[S_P02][11];				// 満車台数２
					p1->CarCnt2 = (ushort)PPrmSS[S_P02][10];				// 現在台数２
					switch (PPrmSS[S_P02][9]) {								// 強制満空車モード2
					case	1:												// 強制満車
						p1->Full[1] = 11;									// 「強制満車」状態セット
						break;
					case	2:												// 強制空車
						p1->Full[1] = 10;									// 「強制空車」状態セット
						break;
					default:												// 自動
						p1->Full[1] = CARFULL_stat[1];						// 「空車or満車」状態セット
						break;
					}

					if (i == 4) {
						p1->FullNo3 = (ushort)PPrmSS[S_P02][15];			// 満車台数３
						p1->CarCnt3 = (ushort)PPrmSS[S_P02][14];			// 現在台数３
						switch (PPrmSS[S_P02][13]) {						// 強制満空車モード3
						case	1:											// 強制満車
							p1->Full[2] = 11;								// 「強制満車」状態セット
							break;
						case	2:											// 強制空車
							p1->Full[2] = 10;								// 「強制空車」状態セット
							break;
						default:											// 自動
							p1->Full[2] = CARFULL_stat[2];					// 「空車or満車」状態セット
							break;
						}
					}
					break;
				default:
					break;
				}
			}
		}
	} else {
		p2 = (PARKCAR_DATA21 *)p;

		memset( p2, 0, sizeof( PARKCAR_DATA21 ) );

		// ｸﾞﾙｰﾌﾟ番号

		// 用途別駐車台数設定

		if (_is_ntnet_remote()) {											// 遠隔NT-NET設定のときのみデータセット
			if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {					// 01-0039⑤駐車台数を本体で管理する
				switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
				case 0:														// 01-0039⑥用途別駐車台数切替なし
					p2->FullNo1 = (ushort)PPrmSS[S_P02][3];					// 満車台数
					if (PPrmSS[S_P02][2] > PPrmSS[S_P02][3]) {
						p2->EmptyNo1 = 0;									// 空車台数
					} else {
						p2->EmptyNo1 = (ushort)(PPrmSS[S_P02][3] - PPrmSS[S_P02][2]);	// 空車台数(満車台数 - 現在駐車台数)
					}
					switch (PPrmSS[S_P02][1]) {								// 強制満空車ﾓｰﾄﾞ
					case	1:												// 強制満車
						p2->FullSts1 = 11;									// 「強制満車」状態セット
						break;
					case	2:												// 強制空車
						p2->FullSts1 = 10;									// 「強制空車」状態セット
						break;
					default:												// 自動
						p2->FullSts1 = CARFULL_stat[0];						// 「空車or満車」状態セット
						break;
					}
					break;
				case 3:														// 01-0039⑥用途別駐車台数２系統
				case 4:														// 01-0039⑥用途別駐車台数３系統
					p2->FullNo1 = (ushort)PPrmSS[S_P02][7];					// 満車台数１
					if (PPrmSS[S_P02][6] > PPrmSS[S_P02][7]) {
						p2->EmptyNo1 = 0;									// 空車台数１
					} else {
						p2->EmptyNo1 = (ushort)(PPrmSS[S_P02][7] - PPrmSS[S_P02][6]);	// 空車台数１(満車台数 - 現在駐車台数)
					}
					switch (PPrmSS[S_P02][5]) {								// 強制満空車モード1
					case	1:												// 強制満車
						p2->FullSts1 = 11;									// 「強制満車」状態セット
						break;
					case	2:												// 強制空車
						p2->FullSts1 = 10;									// 「強制空車」状態セット
						break;
					default:												// 自動
						p2->FullSts1 = CARFULL_stat[0];						// 「空車or満車」状態セット
						break;
					}

					p2->FullNo2 = (ushort)PPrmSS[S_P02][11];				// 満車台数２
					if (PPrmSS[S_P02][10] > PPrmSS[S_P02][11]) {
						p2->EmptyNo2 = 0;									// 空車台数２
					} else {
						p2->EmptyNo2 = (ushort)(PPrmSS[S_P02][11] - PPrmSS[S_P02][10]);	// 空車台数２(満車台数 - 現在駐車台数)
					}
					switch (PPrmSS[S_P02][9]) {								// 強制満空車モード2
					case	1:												// 強制満車
						p2->FullSts2 = 11;									// 「強制満車」状態セット
						break;
					case	2:												// 強制空車
						p2->FullSts2 = 10;									// 「強制空車」状態セット
						break;
					default:												// 自動
						p2->FullSts2 = CARFULL_stat[1];						// 「空車or満車」状態セット
						break;
					}

					if (i == 4) {
						p2->FullNo3 = (ushort)PPrmSS[S_P02][15];			// 満車台数３
						if (PPrmSS[S_P02][14] > PPrmSS[S_P02][15]) {
							p2->EmptyNo3 = 0;								// 空車台数３
						} else {
							p2->EmptyNo3 = (ushort)(PPrmSS[S_P02][15] - PPrmSS[S_P02][14]);	// 空車台数３(満車台数 - 現在駐車台数)
						}
						switch (PPrmSS[S_P02][13]) {						// 強制満空車モード3
						case	1:											// 強制満車
							p2->FullSts3 = 11;								// 「強制満車」状態セット
							break;
						case	2:											// 強制空車
							p2->FullSts3 = 10;								// 「強制空車」状態セット
							break;
						default:											// 自動
							p2->FullSts3 = CARFULL_stat[2];					// 「空車or満車」状態セット
							break;
						}
					}
					break;

				default:
					break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 入庫ﾃﾞｰﾀ(ﾃﾞｰﾀ種別20)作成処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data20                                        |*/
/*| PARAMETER    : pr_lokno : 内部処理用駐車位置番号(1～324)               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data20( ushort pr_lokno )
{
	ushort	lkno;
	const t_NtBufState	*ntbufst;
	uchar	ans;
	ushort	len;

	lkno = pr_lokno - 1;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_20 ) );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		BasicDataMake( 54, 0 );										// 基本ﾃﾞｰﾀ作成
	} else {
		BasicDataMake( 20, 0 );										// 基本ﾃﾞｰﾀ作成
	}

	SendNtnetDt.SData20.InCount = NTNetDataCont[0];					// 入庫追い番
	NTNetDataCont[0]++;												// 入庫追い番+1

	SendNtnetDt.SData20.Syubet = LockInfo[lkno].ryo_syu;			// 処理区分

	if( FLAPDT.flp_data[lkno].lag_to_in.BIT.SYUUS == 1 ){	// 修正精算の時
		SendNtnetDt.SData20.InMode = 6;								// 入庫ﾓｰﾄﾞ：修正精算入庫
	}else{
		SendNtnetDt.SData20.InMode = (FLAPDT.flp_data[lkno].lag_to_in.BIT.FUKUG)? 5 : 0;	// 入庫ﾓｰﾄﾞ
	}

	SendNtnetDt.SData20.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
								+ LockInfo[lkno].posi );			// 区画情報
	SendNtnetDt.SData20.CardType = 0;								// 駐車券ﾀｲﾌﾟ(未使用)
	SendNtnetDt.SData20.CMachineNo = 0;								// 駐車券機械№
	SendNtnetDt.SData20.CardNo = 0L;								// 駐車券番号(発券追い番)
	SendNtnetDt.SData20.InTime.Year = FLAPDT.flp_data[lkno].year;	// 入庫年
	SendNtnetDt.SData20.InTime.Mon = FLAPDT.flp_data[lkno].mont;	// 入庫月
	SendNtnetDt.SData20.InTime.Day = FLAPDT.flp_data[lkno].date;	// 入庫日
	SendNtnetDt.SData20.InTime.Hour = FLAPDT.flp_data[lkno].hour;	// 入庫時
	SendNtnetDt.SData20.InTime.Min = FLAPDT.flp_data[lkno].minu;	// 入庫分
	SendNtnetDt.SData20.InTime.Sec = 0;								// 入庫秒
	SendNtnetDt.SData20.PassCheck = 1;								// ｱﾝﾁﾊﾟｽﾁｪｯｸしない
	if( prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[lkno].ryo_syu-1)*6)),1,1 ) ){	// 種別毎ｶｳﾝﾄする設定?
		SendNtnetDt.SData20.CountSet = 0;							// 在車ｶｳﾝﾄ(する)
	}else{
		SendNtnetDt.SData20.CountSet = 1;							// 在車ｶｳﾝﾄ(しない)
	}
	// 定期券ﾃﾞｰﾀは0固定

	len = sizeof( DATA_KIND_20 );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		memmove(&SendNtnetDt.SData54.InCount, &SendNtnetDt.SData20.InCount, sizeof(DATA_KIND_20)-sizeof(DATA_BASIC));

		SendNtnetDt.SData54.FmtRev = 0;								// フォーマットRev.№
		NTNET_Snd_ParkCarNumDataMk(&SendNtnetDt.SData54.ParkData, 0);

		len = sizeof( DATA_KIND_54 );
	}
	if(_is_ntnet_remote()) {
		ans = RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		ans = NTBUF_SetSendNtData( &SendNtnetDt, len, NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
	switch( ans ){
	case NTNET_BUFSET_NORMAL:										// 正常終了
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_SENDBUF, NTERR_RELEASE, 0, 0 );
		break;
	case NTNET_BUFSET_STATE_CHG:									// ﾊﾞｯﾌｧ状態変化(書込みは完了)
		ntbufst = NTBUF_GetBufState();
		if( ntbufst->car_in & 0x02 ){								// ﾊﾞｯﾌｧFULL発生?
			err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_BUFFULL, NTERR_EMERGE, 0, 0 );
		}
		queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
		break;
	case NTNET_BUFSET_DEL_OLD:										// 最古ﾃﾞｰﾀを消去
	case NTNET_BUFSET_DEL_NEW:										// 最新ﾃﾞｰﾀを消去
	case NTNET_BUFSET_CANT_DEL:										// ﾊﾞｯﾌｧFULLだが設定が"休業"のため消去不可
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_SENDBUF, NTERR_EMERGE, 0, 0 );
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 入庫ﾃﾞｰﾀ(ﾃﾞｰﾀ種別20)作成処理 振替精算時の仮入庫用送信関数              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data20_frs                                    |*/
/*| PARAMETER    : pr_lokno : 内部処理用駐車位置番号(1～324)               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
void	NTNET_Snd_Data20_frs( ushort pr_lokno, void *data )
{
	ushort	lkno;
	const t_NtBufState	*ntbufst;
	uchar	ans;
	flp_com *flp_data = (flp_com*)data;
	ushort	len;

	lkno = pr_lokno - 1;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_20 ) );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		BasicDataMake( 54, 0 );										// 基本ﾃﾞｰﾀ作成
	} else {
		BasicDataMake( 20, 0 );										// 基本ﾃﾞｰﾀ作成
	}

	SendNtnetDt.SData20.InCount = NTNetDataCont[0];					// 入庫追い番
	NTNetDataCont[0]++;												// 入庫追い番+1

	SendNtnetDt.SData20.Syubet = LockInfo[lkno].ryo_syu;			// 処理区分
	SendNtnetDt.SData20.InMode = (flp_data->lag_to_in.BIT.FUKUG)? 5 : 0;		// 入庫ﾓｰﾄﾞ
	SendNtnetDt.SData20.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
								+ LockInfo[lkno].posi );			// 区画情報
	SendNtnetDt.SData20.CardType = 0;								// 駐車券ﾀｲﾌﾟ(未使用)
	SendNtnetDt.SData20.CMachineNo = 0;								// 駐車券機械№
	SendNtnetDt.SData20.CardNo = 0L;								// 駐車券番号(発券追い番)
	SendNtnetDt.SData20.InTime.Year = flp_data->year;	// 入庫年
	SendNtnetDt.SData20.InTime.Mon = flp_data->mont;	// 入庫月
	SendNtnetDt.SData20.InTime.Day = flp_data->date;	// 入庫日
	SendNtnetDt.SData20.InTime.Hour = flp_data->hour;	// 入庫時
	SendNtnetDt.SData20.InTime.Min = flp_data->minu;	// 入庫分
	SendNtnetDt.SData20.InTime.Sec = 0;								// 入庫秒
	SendNtnetDt.SData20.PassCheck = 1;						// ｱﾝﾁﾊﾟｽﾁｪｯｸしない

	if( prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[lkno].ryo_syu-1)*6)),1,1 ) ){	// 種別毎ｶｳﾝﾄする設定?
		SendNtnetDt.SData20.CountSet = 0;							// 在車ｶｳﾝﾄ(する)
	}else{
		SendNtnetDt.SData20.CountSet = 1;							// 在車ｶｳﾝﾄ(しない)
	}
	// 定期券ﾃﾞｰﾀは0固定

	len = sizeof( DATA_KIND_20 );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		memmove(&SendNtnetDt.SData54.InCount, &SendNtnetDt.SData20.InCount, sizeof(DATA_KIND_20)-sizeof(DATA_BASIC));

		SendNtnetDt.SData54.FmtRev = 0;								// フォーマットRev.№
		NTNET_Snd_ParkCarNumDataMk(&SendNtnetDt.SData54.ParkData, 0);

		len = sizeof( DATA_KIND_54 );
	}
	if(_is_ntnet_remote()) {
		ans = RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		ans = NTBUF_SetSendNtData( &SendNtnetDt, len, NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
	switch( ans ){
	case NTNET_BUFSET_NORMAL:										// 正常終了
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_SENDBUF, NTERR_RELEASE, 0, 0 );
		break;
	case NTNET_BUFSET_STATE_CHG:									// ﾊﾞｯﾌｧ状態変化(書込みは完了)
		ntbufst = NTBUF_GetBufState();
		if( ntbufst->car_in & 0x02 ){								// ﾊﾞｯﾌｧFULL発生?
			err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_BUFFULL, NTERR_EMERGE, 0, 0 );
		}
		queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
		break;
	case NTNET_BUFSET_DEL_OLD:										// 最古ﾃﾞｰﾀを消去
	case NTNET_BUFSET_DEL_NEW:										// 最新ﾃﾞｰﾀを消去
	case NTNET_BUFSET_CANT_DEL:										// ﾊﾞｯﾌｧFULLだが設定が"休業"のため消去不可
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_SENDBUF, NTERR_EMERGE, 0, 0 );
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| クレジット精算判別                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_decision_credit                                   |*/
/*| RETURN VALUE : 1 as credit                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.9.20                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int	ntnet_decision_credit(credit_use *p)
{
	if (p->pay_ryo) {
		switch(p->cre_type) {
		default:
			break;
		}
		return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算中の不正出庫対策												   |*/
/*| 精算完了した場合、その車室が不正出庫登録されていれば消去する。		   |*/
/*| ※駐車料金精算完了時のみCallすること。								   |*/
/*| 　（定期更新時、精算中止時にCallすてはならない）					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data22_FusDel                                 |*/
/*| PARAMETER    : pr_lokno  : 内部処理用駐車位置番号(1～324)              |*/
/*|              : paymethod : 精算方式                                    |*/
/*|              : payclass  : 処理区分                                    |*/
/*|              : outkind   : 0=通常精算, 1=強制出庫                      |*/
/*|              :             2=精算しない出庫, 3=不正出庫                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2007-12-12                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data22_FusDel( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind )
{
	ushort	i,j;
	ushort	FuseiDataCount;										// 不正・強制ﾃﾞｰﾀ内 登録件数
	uchar	f_FuseiDataFound = 0;								// 1=found
	char	save_cycl_fg;										// 精算ｻｲｸﾙﾌﾗｸﾞ保存ｴﾘｱ


	if( pr_lokno == 0xffff ){									// 相互精算のとき
		return;													// 他の精算機の車室なのでデータなし
	}

	if( ( paymethod != 5 ) &&									// 定期更新精算
		( payclass != 2 && payclass != 3 ) &&					// 精算中止
		( fusei.kensuu != 0) )									// 不正出庫情報あり
	{
		FuseiDataCount = fusei.kensuu;							// 不正・強制ﾃﾞｰﾀ内 登録件数get
		if( FuseiDataCount > LOCK_MAX ){						// ﾃﾞｰﾀ破損時のｶﾞｰﾄﾞ
			FuseiDataCount = LOCK_MAX;
		}

		for( j=0; j<FuseiDataCount; j++ ){
			if( fusei.fus_d[j].t_iti == pr_lokno ){				// 消去対象データあり
				f_FuseiDataFound = 1;							// 未処理の不正出庫記録に今精算完了した車室がある
				break;
			}
		}

		if( !f_FuseiDataFound )									// 消去対象データなし
			return;

		// 不正データエリアから該当車室のデータを消去する
		// TempFusに消去後データを作成

		memset( &TempFus, 0, sizeof( struct FUSEI_SD ) );		// 不正ﾃﾞｰﾀﾜｰｸｴﾘｱｸﾘｱ

		for( i=j=0; j<FuseiDataCount; j++ ){					// 不正・強制ﾃﾞｰﾀ内件数分回る
			if( fusei.fus_d[j].t_iti == pr_lokno ){				// 消去対象データ
				;	// 何もしない
			}
			else{												// 消去しないデータ
				memcpy( &TempFus.fus_d[i], &fusei.fus_d[j], sizeof(struct FUSEI_D) );	// 1件分copy
				++i;											// 次格納位置set（兼 登録件数）
			}
		}
		TempFus.kensuu = i;										// 登録件数set
																// TempFusに新ﾃﾞｰﾀset完了
		// fuseiｴﾘｱの更新
		
		memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );	// 不正ﾃﾞｰﾀをﾜｰｸｴﾘｱ（ﾊﾞｯﾃﾘｰﾊﾞｯｸｱｯﾌﾟ領域）へ

		save_cycl_fg = ac_flg.cycl_fg;							// 現精算ｻｲｸﾙﾌﾗｸﾞ値保存
		ac_flg.cycl_fg = 122;									// 122:不正ﾃﾞｰﾀをﾜｰｸｴﾘｱへ転送完了（☆①）

		memcpy( &fusei, &TempFus, sizeof( struct FUSEI_SD ) );	// 新ﾃﾞｰﾀに更新

		ac_flg.cycl_fg = save_cycl_fg;							// 精算ｻｲｸﾙﾌﾗｸﾞ値復帰（☆②）
																// 不正ﾃﾞｰﾀｴﾘｱ更新完了のため
		// 【停電動作】☆①～☆②間に停電があった場合、wkfus → fuseiに戻して、ac_flg.cycl_fg = 15 からやり直す。
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算ﾃﾞｰﾀ(ﾃﾞｰﾀ種別22,23)作成処理(フラップ上昇、ロック閉ﾀｲﾏ内出庫用)     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data22_LO                                     |*/
/*| PARAMETER    : pr_lokno  : 内部処理用駐車位置番号(1～324)              |*/
/*|              : paymethod : 精算方式                                    |*/
/*|              : payclass  : 処理区分                                    |*/
/*|              : outkind   : 0=通常精算, 1=強制出庫                      |*/
/*|              :             2=精算しない出庫, 3=不正出庫                |*/
/*|              :             97=フラップ上昇、ロック閉ﾀｲﾏ内出庫          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2007-09-03                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data22_LO( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind )
{
	ushort	lkno;
	const t_NtBufState	*ntbufst;
	uchar	work, ans;
	ushort	len;

	ntbufst = NTBUF_GetBufState();
	if( ntbufst->sale & 0x01 ){										// ﾊﾞｯﾌｧNEAR FULL発生中?
		return;														// ﾆｱﾌﾙ発生中は不正強制の精算ﾃﾞｰﾀは作らない
	}

	lkno = pr_lokno - 1;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_22 ) );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			work = 57;												// 出口ﾃﾞｰﾀ
		}else{
			work = 56;												// 事前ﾃﾞｰﾀ
		}
	} else {
	if( CPrmSS[S_NTN][27] == 0 ){
		work = 23;													// 出口ﾃﾞｰﾀ
	}else{
		work = 22;													// 事前ﾃﾞｰﾀ
	}
	}
	BasicDataMake( work, 0 );										// 基本ﾃﾞｰﾀ作成

	CountGet( PAYMENT_COUNT, (ST_OIBAN *)&SendNtnetDt.SData22.PayCount );			// 精算追い番
	SendNtnetDt.SData22.PayMethod = paymethod;						// 精算方法
	SendNtnetDt.SData22.PayClass = payclass;						// 処理区分
	SendNtnetDt.SData22.PayMode = 0;								// 精算ﾓｰﾄﾞ
	SendNtnetDt.SData22.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
								+ LockInfo[lkno].posi );			// 区画情報
	SendNtnetDt.SData22.CardType = 0;								// 駐車券ﾀｲﾌﾟ
	SendNtnetDt.SData22.CMachineNo = 0;								// 駐車券機械№
	SendNtnetDt.SData22.CardNo = 0L;								// 駐車券番号(発券追い番)
	SendNtnetDt.SData22.OutTime.Year = car_ot_f.year;				// 出庫年
	SendNtnetDt.SData22.OutTime.Mon = car_ot_f.mon;					// 出庫月
	SendNtnetDt.SData22.OutTime.Day = car_ot_f.day;					// 出庫日
	SendNtnetDt.SData22.OutTime.Hour = car_ot_f.hour;				// 出庫時
	SendNtnetDt.SData22.OutTime.Min = car_ot_f.min;					// 出庫分
	SendNtnetDt.SData22.OutTime.Sec = 0;							// 出庫秒

	SendNtnetDt.SData22.KakariNo = 0;								// 係員№
	SendNtnetDt.SData22.OutKind = outkind;							// 精算出庫
	SendNtnetDt.SData22.InTime.Year = car_in_f.year;				// 入庫年
	SendNtnetDt.SData22.InTime.Mon = car_in_f.mon;					// 入庫月
	SendNtnetDt.SData22.InTime.Day = car_in_f.day;					// 入庫日
	SendNtnetDt.SData22.InTime.Hour = car_in_f.hour;				// 入庫時
	SendNtnetDt.SData22.InTime.Min = car_in_f.min;					// 入庫分
	SendNtnetDt.SData22.InTime.Sec = 0;								// 入庫秒
	// 精算年月日時分秒は0とする
	// 前回精算年月日時分秒は0とする
	SendNtnetDt.SData22.TaxPrice = 0;								// 課税対象額
	SendNtnetDt.SData22.TotalPrice = 0;								// 合計金額(HOST未使用のため)
	SendNtnetDt.SData22.Tax = 0;									// 消費税額
	SendNtnetDt.SData22.Syubet = (ushort)LockInfo[lkno].ryo_syu;		// 料金種別
	SendNtnetDt.SData22.Price = 0;									// 駐車料金
	SendNtnetDt.SData22.PassCheck = 1;								// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	if(( CPrmSS[S_NTN][27] == 0 )&&									// 出口ﾃﾞｰﾀ?
	   ( prm_get( COM_PRM,S_SHA,(short)(2+((ryo_buf.tik_syu-1)*6)),1,1 ) )){	// 種別毎ｶｳﾝﾄする設定?
		SendNtnetDt.SData22.CountSet = 2;							// 在車ｶｳﾝﾄ-1する
	}else{
		SendNtnetDt.SData22.CountSet = 1;							// 在車ｶｳﾝﾄしない
	}
// MH321800(S) Y.Tanizaki ICクレジット対応
//	memset( SendNtnetDt.SData22.Reserve1, 0x20, 2 );				// 最大料金適用回数（将来対応）
// MH321800(E) Y.Tanizaki ICクレジット対応

	len = sizeof( DATA_KIND_22 );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		memmove(&SendNtnetDt.SData56.PayCount, &SendNtnetDt.SData22.PayCount, sizeof(DATA_KIND_22)-sizeof(DATA_BASIC));

		SendNtnetDt.SData56.FmtRev = 0;								// フォーマットRev.№
		NTNET_Snd_ParkCarNumDataMk(&SendNtnetDt.SData56.ParkData, 0);

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 		len = sizeof( DATA_KIND_56 ) - sizeof( DISCOUNT_DATA );
		len = sizeof( DATA_KIND_56 ) - sizeof( t_SeisanDiscountOld );
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	}
	if(_is_ntnet_remote()) {
		ans = RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		ans = NTBUF_SetSendNtData( &SendNtnetDt, len, NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
	switch( ans ){
	case NTNET_BUFSET_NORMAL:										// 正常終了
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_SENDBUF, NTERR_RELEASE, 0, 0 );
		break;
	case NTNET_BUFSET_STATE_CHG:									// ﾊﾞｯﾌｧ状態変化(書込みは完了)
		ntbufst = NTBUF_GetBufState();
		if( ntbufst->sale & 0x02 ){									// ﾊﾞｯﾌｧFULL発生?
			err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_BUFFULL, NTERR_EMERGE, 0, 0 );
		}
		if( ntbufst->sale & 0x01 ){									// ﾊﾞｯﾌｧNEAR FULL発生?
			err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_BUFNFULL, NTERR_EMERGE, 0, 0 );
		}
		queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
		break;
	case NTNET_BUFSET_DEL_OLD:										// 最古ﾃﾞｰﾀを消去
	case NTNET_BUFSET_DEL_NEW:										// 最新ﾃﾞｰﾀを消去
	case NTNET_BUFSET_CANT_DEL:										// ﾊﾞｯﾌｧFULLだが設定が"休業"のため消去不可
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_SENDBUF, NTERR_EMERGE, 0, 0 );
		break;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 駐車台数データ/駐車台数応答データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_DataParkCarNum
 *| PARAMETER    : uchar 58: / 59:
 *|              : mod : 送信モード
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2012-03-06
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTNET_Snd_DataParkCarNumWeb(uchar kind)
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_58 ) );

	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( kind, 1 );
	// フォーマットRev.№
	SendNtnetDt.SData58.FmtRev = 0;

	// 駐車台数データ作成
	NTNET_Snd_ParkCarNumDataMk((void*)&SendNtnetDt.SData58.ParkData, 1);

	// ﾃﾞｰﾀ送信登録
	if(memcmp(&SData58_bk.ParkData, &SendNtnetDt.SData58.ParkData, sizeof(PARKCAR_DATA2)) != 0) {
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_58 ));
		}
		else {
			NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_58), NTNET_BUF_NORMAL);
		}
		memcpy(&SData58_bk, &SendNtnetDt.SData58, sizeof(DATA_KIND_58));
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 駐車台数データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data58
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2012-03-06
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTNET_Snd_Data58()
{
	NTNET_Snd_DataParkCarNumWeb((uchar)58);
}


/*[]----------------------------------------------------------------------[]*/
/*| 制御応答ﾃﾞｰﾀ(ﾃﾞｰﾀ種別101)作成処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data101                                       |*/
/*| PARAMETER    : MachineNo : 送信先端末機械№                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data101( ulong MachineNo )
{
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//uchar	wk_ful;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_100 ) );

	BasicDataMake( 101, 1 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData101.SMachineNo = MachineNo;					// 送信先端末機械№
	if( RecvNtnetDt.RData100.ControlData[0] ){						// 営休業切換?
		if( FLAGPT.receive_rec.flg_ock == 0 ){
			SendNtnetDt.SData101.ControlData[0] = 3;				// 自動
		}
		else if( FLAGPT.receive_rec.flg_ock == 1 ){
			SendNtnetDt.SData101.ControlData[0] = 2;				// 営業
		}
		else if( FLAGPT.receive_rec.flg_ock == 2 ){
			SendNtnetDt.SData101.ControlData[0] = 1;				// 休業
		}
	}

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	if (RecvNtnetDt.RData100.ControlData[1]) {
//		wk_ful = FLAGPT.car_full[1]; 
//
//		switch (wk_ful) {
//		case 0:
//			SendNtnetDt.SData101.ControlData[1] = 4;				// 自動
//			break;
//		case 1:
//			SendNtnetDt.SData101.ControlData[1] = 1;				// 強制満車
//			break;
//		case 2:
//			SendNtnetDt.SData101.ControlData[1] = 2;				// 強制空車
//			break;
//		case 3:
//			SendNtnetDt.SData101.ControlData[1] = 0;				// ほぼ満車
//			break;
//		default:
//			break;
//		}
//	}
//	if (RecvNtnetDt.RData100.ControlData[2]) {
//		switch (FLAGPT.car_full[2]) {
//		case 0:
//			SendNtnetDt.SData101.ControlData[2] = 4;				// 自動
//			break;                                  
//		case 1:                                     
//			SendNtnetDt.SData101.ControlData[2] = 1;				// 強制満車
//			break;                                  
//		case 2:                                     
//			SendNtnetDt.SData101.ControlData[2] = 2;				// 強制空車
//			break;                                  
//		case 3:                                     
//			SendNtnetDt.SData101.ControlData[2] = 0;				// ほぼ満車
//			break;
//		default:
//			break;
//		}
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	if( RecvNtnetDt.RData100.ControlData[3] ){						// ｱﾝﾁﾊﾟｽ切換?
		if( PPrmSS[S_P01][3] == 0 ){
			SendNtnetDt.SData101.ControlData[3] = 1;				// 自動
		}
		else if( PPrmSS[S_P01][3] == 1 ){
			SendNtnetDt.SData101.ControlData[3] = 2;				// 解除
		}
	}
	// ﾘｾｯﾄ要求を受け付ける
	SendNtnetDt.SData101.ControlData[7] = RecvNtnetDt.RData100.ControlData[7];	// ﾘｾｯﾄ指示
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	if (RecvNtnetDt.RData100.ControlData[10]) {
//		switch (FLAGPT.car_full[3]) {
//		case 0:
//			SendNtnetDt.SData101.ControlData[10] = 4;				// 自動
//			break;                                  
//		case 1:                                     
//			SendNtnetDt.SData101.ControlData[10] = 1;				// 強制満車
//			break;                                  
//		case 2:                                     
//			SendNtnetDt.SData101.ControlData[10] = 2;				// 強制空車
//			break;                                  
//		case 3:                                     
//			SendNtnetDt.SData101.ControlData[10] = 0;				// ほぼ満車
//			break;
//		default:
//			break;
//		}
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_100 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 管理ﾃﾞｰﾀ要求(ﾃﾞｰﾀ種別104)作成処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data104                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data104( ulong req )
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_103 ) );

	BasicDataMake( 104, 1 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData104.SMachineNo = 0L;							// 送信先端末機械№
	if (req & NTNET_MANDATA_CLOCK) {
		// 時計データ送信チェック
		SendNtnetDt.SData104.ControlData[0] = 1;
	}
	if (req & NTNET_MANDATA_CTRL) {
		// 制御データ送信チェック
		SendNtnetDt.SData104.ControlData[1] = 1;
	}
	if (req & NTNET_MANDATA_COMMON) {
		// 共通設定データ送信チェック
		SendNtnetDt.SData104.ControlData[2] = 1;
	}
	if (req & NTNET_MANDATA_PASSSTS) {
		// 定期券ステータスデータ送信チェック
		SendNtnetDt.SData104.ControlData[15] = 1;
	}
	if (req & NTNET_MANDATA_PASSCHANGE) {
		// 定期券更新データ送信チェック
		SendNtnetDt.SData104.ControlData[16] = 1;
	}
	if (req & NTNET_MANDATA_PASSEXIT) {
		// 定期券出庫時刻データ送信チェック
		SendNtnetDt.SData104.ControlData[17] = 1;
	}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	if (req & NTNET_MANDATA_LOCKINFO) {
//		// 車室パラメータデータ送信チェック
//		SendNtnetDt.SData104.ControlData[19] = 1;
//	}
//	if (req & NTNET_MANDATA_LOCKMARKER) {
//		// ロック装置データ送信チェック
//		SendNtnetDt.SData104.ControlData[20] = 1;
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	if (req & NTNET_MANDATA_INVTKT) {
		// 無効駐車券データ送信チェック
		SendNtnetDt.SData104.ControlData[11] = 1;
	}
	if (req & NTNET_MANDATA_PASSSTOP) {
		// 定期券中止データ送信チェック
		SendNtnetDt.SData104.ControlData[13] = 1;
	}
	if (req & NTNET_MANDATA_SPECIALDAY) {
		// 特別日設定データ送信チェック
		SendNtnetDt.SData104.ControlData[18] = 1;
	}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	if (req & NTNET_MANDATA_PARKNUMCTL) {
//		// 駐車台数管理データ送信チェック
//		SendNtnetDt.SData104.ControlData[25] = 1;
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_103 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_103 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ｴﾗｰﾃﾞｰﾀ(ﾃﾞｰﾀ種別120)作成処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data120                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data120( void )
{
}


/*[]----------------------------------------------------------------------[]*/
/*| ｴﾗｰﾃﾞｰﾀ(ﾃﾞｰﾀ種別120)作成処理(ｴﾗｰｸﾘｱ要求用)                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data120_CL                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data120_CL( void )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_120 ) );

	BasicDataMake( 120, 0 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData120.Errsyu = 255;								// ｴﾗｰ種別
	SendNtnetDt.SData120.Errcod = 255;								// ｴﾗｰｺｰﾄﾞ
	SendNtnetDt.SData120.Errdtc = 2;								// ｴﾗｰ発生/解除
	SendNtnetDt.SData120.Errlev = 5;								// ｴﾗｰﾚﾍﾞﾙ
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_120 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_120 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ｱﾗｰﾑﾃﾞｰﾀ(ﾃﾞｰﾀ種別121)作成処理                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data121                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data121( void )
{
}


/*[]----------------------------------------------------------------------[]*/
/*| ｱﾗｰﾑﾃﾞｰﾀ(ﾃﾞｰﾀ種別121)作成処理(ｱﾗｰﾑｸﾘｱ要求用)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data121_CL                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data121_CL( void )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_121 ) );

	BasicDataMake( 121, 0 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData121.Armsyu = 255;		// ｴﾗｰ種別
	SendNtnetDt.SData121.Armcod = 255;		// ｴﾗｰｺｰﾄﾞ
	SendNtnetDt.SData121.Armdtc = 2;		// ｴﾗｰ発生/解除
	SendNtnetDt.SData121.Armlev = 5;		// ｴﾗｰﾚﾍﾞﾙ
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_121 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_121 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 金銭管理ﾃﾞｰﾀ(ﾃﾞｰﾀ種別126)作成処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data126                                       |*/
/*| PARAMETER    : payclass : 処理区分                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data126( ulong MachineNo, ushort payclass )
{
	char	i;
	ushort	wk_mai;
	uchar	f_Send;
	uchar	pos = 0;


	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_126 ) );

	BasicDataMake( 126, 0 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData126.SMachineNo = MachineNo;					// 送信先端末機械№

	if( ( ( 0 == payclass ) || ( 1 == payclass ) ) && (OpeNtnetAddedInfo.PayMethod !=5 )){	// 精算完了（精算or再精算）and 定期更新ではない
		if (ntnet_decision_credit(&PayData.credit)) {
			payclass += 4;											// 処理区分をｸﾚｼﾞｯﾄ精算（ｸﾚｼﾞｯﾄ再精算）にする
		}
	}

	if( payclass <= 5 ){											// 精算動作
		SendNtnetDt.SData126.PayCount = CountSel( &PayData.Oiban);	// 精算 or 精算中止追番set
// MH810105(S) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
//		SendNtnetDt.SData126.PayTime.Year = PayData.TOutTime.Year;	// 精算年
//		SendNtnetDt.SData126.PayTime.Mon = PayData.TOutTime.Mon;	// 精算月
//		SendNtnetDt.SData126.PayTime.Day = PayData.TOutTime.Day;	// 精算日
//		SendNtnetDt.SData126.PayTime.Hour = PayData.TOutTime.Hour;	// 精算時
//		SendNtnetDt.SData126.PayTime.Min = PayData.TOutTime.Min;	// 精算分
//		SendNtnetDt.SData126.PayTime.Sec = 0;						// 精算秒

// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//		// 種別	(0=事前精算／1＝未精算出庫精算)
//		if(PayData.shubetsu == 0){
		// 種別	(0=事前精算／1＝未精算出庫精算／2=遠隔精算(精算中変更))
		if(PayData.shubetsu == 0 || PayData.shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
			SendNtnetDt.SData126.PayTime.Year = PayData.TOutTime.Year;	// 精算年
			SendNtnetDt.SData126.PayTime.Mon = PayData.TOutTime.Mon;	// 精算月
			SendNtnetDt.SData126.PayTime.Day = PayData.TOutTime.Day;	// 精算日
			SendNtnetDt.SData126.PayTime.Hour = PayData.TOutTime.Hour;	// 精算時
			SendNtnetDt.SData126.PayTime.Min = PayData.TOutTime.Min;	// 精算分
			SendNtnetDt.SData126.PayTime.Sec = 0;						// 精算秒
		}else{
			SendNtnetDt.SData126.PayTime.Year = PayData.TUnpaidPayTime.Year;	// 精算年
			SendNtnetDt.SData126.PayTime.Mon = PayData.TUnpaidPayTime.Mon;		// 精算月
			SendNtnetDt.SData126.PayTime.Day = PayData.TUnpaidPayTime.Day;		// 精算日
			SendNtnetDt.SData126.PayTime.Hour = PayData.TUnpaidPayTime.Hour;	// 精算時
			SendNtnetDt.SData126.PayTime.Min = PayData.TUnpaidPayTime.Min;		// 精算分
			SendNtnetDt.SData126.PayTime.Sec = 0;								// 精算秒

		}
// MH810105(E) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
	}
	SendNtnetDt.SData126.PayClass = payclass;						// 処理区分

	SendNtnetDt.SData126.KakariNo = OPECTL.Kakari_Num;				// 係員№

	SendNtnetDt.SData126.CoinSf[0].Money = 10;						// ｺｲﾝ金庫金種(10円)
	SendNtnetDt.SData126.CoinSf[1].Money = 50;						// ｺｲﾝ金庫金種(50円)
	SendNtnetDt.SData126.CoinSf[2].Money = 100;						// ｺｲﾝ金庫金種(100円)
	SendNtnetDt.SData126.CoinSf[3].Money = 500;						// ｺｲﾝ金庫金種(500円)
	if( payclass != 10 ){											// ｺｲﾝ金庫合計でない
		SendNtnetDt.SData126.CoinSf[0].Mai = SFV_DAT.safe_dt[0];	// ｺｲﾝ金庫枚数(10円)
		SendNtnetDt.SData126.CoinSf[1].Mai = SFV_DAT.safe_dt[1];	// ｺｲﾝ金庫枚数(50円)
		SendNtnetDt.SData126.CoinSf[2].Mai = SFV_DAT.safe_dt[2];	// ｺｲﾝ金庫枚数(100円)
		SendNtnetDt.SData126.CoinSf[3].Mai = SFV_DAT.safe_dt[3];	// ｺｲﾝ金庫枚数(500円)
	}

	for( i=0; i<4; i++ ){
		SendNtnetDt.SData126.CoinSfTotal +=							// ｺｲﾝ金庫総額
// MH322914 (s) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
//			SendNtnetDt.SData126.CoinSf[i].Mai * SendNtnetDt.SData126.CoinSf[i].Money;
			(ulong)SendNtnetDt.SData126.CoinSf[i].Mai * (ulong)SendNtnetDt.SData126.CoinSf[i].Money;
// MH322914 (e) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
	}

	SendNtnetDt.SData126.NoteSf[0].Money = 1000;					// 紙幣金庫金種(1000円)
	if( payclass != 11 ){											// 紙幣金庫合計でない
		SendNtnetDt.SData126.NoteSf[0].Mai = SFV_DAT.nt_safe_dt;	// 紙幣金庫枚数(1000円)
	}
	SendNtnetDt.SData126.NoteSfTotal =								// 紙幣金庫総額
// MH322914 (s) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
//		SendNtnetDt.SData126.NoteSf[0].Mai * SendNtnetDt.SData126.NoteSf[0].Money;
		(ulong)SendNtnetDt.SData126.NoteSf[0].Mai * (ulong)SendNtnetDt.SData126.NoteSf[0].Money;
// MH322914 (e) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)

	// 金銭管理なし時は金庫情報のみ送信
	if( CPrmSS[S_KAN][1] ){											// 金銭管理あり設定
		switch( (uchar)payclass ){

		case 20:
		case 30:
		case 32:
			SendNtnetDt.SData126.Coin[0].Mai = turi_kan.turi_dat[0].sin_mai;	// ｺｲﾝ循環枚数(10円)
			SendNtnetDt.SData126.Coin[1].Mai = turi_kan.turi_dat[1].sin_mai;	// ｺｲﾝ循環枚数(50円)
			SendNtnetDt.SData126.Coin[2].Mai = turi_kan.turi_dat[2].sin_mai;	// ｺｲﾝ循環枚数(100円)
			SendNtnetDt.SData126.Coin[3].Mai = turi_kan.turi_dat[3].sin_mai;	// ｺｲﾝ循環枚数(500円)
			// NTNET_Snd_Data126()の方はFT4000では未使用の為（将来対応）修正しておく。
			turikan_subtube_set();
			for( i=0;i<2;i++ ){
				switch((( turi_kan.sub_tube >> (i*8)) & 0x000F )){
					case 0x01:
						pos = 0;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 10;								// 金種金額(10円)
						break;
					case 0x02:
						pos = 1;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 50;								// 金種金額(50円)
						break;
					case 0x04:
						pos = 2;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 100;								// 金種金額(100円)
						break;
					case 0:
					default:																		// 接続なし
						continue;
				}
				SendNtnetDt.SData126.CoinYotiku[i].Mai = turi_kan.turi_dat[pos].ysin_mai;			// ｺｲﾝ予蓄枚数(10円/50円/100円)
			}
			break;

		default:
			// 10円
			SendNtnetDt.SData126.Coin[0].Mai = turi_kan.turi_dat[0].zen_mai		// ｺｲﾝ循環枚数(10円)
									+ (ushort)turi_kan.turi_dat[0].sei_nyu;
			wk_mai = (ushort)(turi_kan.turi_dat[0].sei_syu + turi_kan.turi_dat[0].kyosei);
			if( SendNtnetDt.SData126.Coin[0].Mai < wk_mai ){
				SendNtnetDt.SData126.Coin[0].Mai = 0;
			}else{
				SendNtnetDt.SData126.Coin[0].Mai -= wk_mai;
			}
			// 50円
			SendNtnetDt.SData126.Coin[1].Mai = turi_kan.turi_dat[1].zen_mai		// ｺｲﾝ循環枚数(50円)
									+ (ushort)turi_kan.turi_dat[1].sei_nyu;
			wk_mai = (ushort)(turi_kan.turi_dat[1].sei_syu + turi_kan.turi_dat[1].kyosei);
			if( SendNtnetDt.SData126.Coin[1].Mai < wk_mai ){
				SendNtnetDt.SData126.Coin[1].Mai = 0;
			}else{
				SendNtnetDt.SData126.Coin[1].Mai -= wk_mai;
			}
			// 100円
			SendNtnetDt.SData126.Coin[2].Mai = turi_kan.turi_dat[2].zen_mai		// ｺｲﾝ循環枚数(100円)
									+ (ushort)turi_kan.turi_dat[2].sei_nyu;
			wk_mai = (ushort)(turi_kan.turi_dat[2].sei_syu + turi_kan.turi_dat[2].kyosei);
			if( SendNtnetDt.SData126.Coin[2].Mai < wk_mai ){
				SendNtnetDt.SData126.Coin[2].Mai = 0;
			}else{
				SendNtnetDt.SData126.Coin[2].Mai -= wk_mai;
			}
			// 500円
			SendNtnetDt.SData126.Coin[3].Mai = turi_kan.turi_dat[3].zen_mai		// ｺｲﾝ循環枚数(500円)
									+ (ushort)turi_kan.turi_dat[3].sei_nyu;
			wk_mai = (ushort)(turi_kan.turi_dat[3].sei_syu + turi_kan.turi_dat[3].kyosei);
			if( SendNtnetDt.SData126.Coin[3].Mai < wk_mai ){
				SendNtnetDt.SData126.Coin[3].Mai = 0;
			}else{
				SendNtnetDt.SData126.Coin[3].Mai -= wk_mai;
			}

			// NTNET_Snd_Data126()の方はFT4000では未使用の為（将来対応）修正しておく。
			turikan_subtube_set();
			for( i=0;i<2;i++ ){
				switch((( turi_kan.sub_tube >> (i*8)) & 0x000F )){
					case 0x01:
						pos = 0;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 10;								// 金種金額(10円)
						break;
					case 0x02:
						pos = 1;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 50;								// 金種金額(50円)
						break;
					case 0x04:
						pos = 2;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 100;								// 金種金額(100円)
						break;
					case 0:
					default:																		// 接続なし
						continue;
				}
				SendNtnetDt.SData126.CoinYotiku[i].Mai = turi_kan.turi_dat[pos].yzen_mai;			// ｺｲﾝ予蓄枚数(10円/50円/100円)
				wk_mai = (ushort)(turi_kan.turi_dat[pos].ysei_syu + turi_kan.turi_dat[pos].ykyosei);
				if( SendNtnetDt.SData126.CoinYotiku[i].Mai < wk_mai ){
					SendNtnetDt.SData126.CoinYotiku[i].Mai = 0;
				}else{
					SendNtnetDt.SData126.CoinYotiku[i].Mai -= wk_mai;
				}
			}
			break;
		}
		SendNtnetDt.SData126.Coin[0].Money = 10;						// ｺｲﾝ循環金種(10円)
		SendNtnetDt.SData126.Coin[1].Money = 50;						// ｺｲﾝ循環金種(50円)
		SendNtnetDt.SData126.Coin[2].Money = 100;						// ｺｲﾝ循環金種(100円)
		SendNtnetDt.SData126.Coin[3].Money = 500;						// ｺｲﾝ循環金種(500円)
	}
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
	// ログ作成時に判定を行っているので、ここでは不要であるが残しておく
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
	/* 精算動作時は金銭情報に変化が無い場合は送信しない */
	f_Send = 1;
	if( payclass <= 5 ){											// 精算動作（完了or中止。）
		if( 0 == memcmp( &SendNtnetDt.SData126.CoinSf[0], 			// お金情報が前回送信内容と同じ（電文中 ｺｲﾝ金庫以下）
						 &Ntnet_Prev_SData126.CoinSf[0],
						 (sizeof(DATA_KIND_126) - sizeof(DATA_BASIC) - 20) ) )
		{
			f_Send = 0;												// 金銭管理データを送信しない
		}
		//年月日が0の時は送信しない(最小単位と比較としておく)
		if( (SendNtnetDt.SData126.PayTime.Year < 1980) ||
			(SendNtnetDt.SData126.PayTime.Mon < 1) ||
			(SendNtnetDt.SData126.PayTime.Day < 1) ){
			f_Send = 0;												// 金銭管理データを送信しない
		}
	}

	if( f_Send ){													// 金銭管理データを送信する
		memcpy( &Ntnet_Prev_SData126, &SendNtnetDt.SData126, sizeof(DATA_KIND_126) );
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_126 ));
		}
		else {
			NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_126 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
		}
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 定期券問合せﾃﾞｰﾀ(ﾃﾞｰﾀ種別142)作成処理                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data142                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-11-04                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data142( ulong pkno, ulong passid )
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_142 ) );

	BasicDataMake( 142, 1 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData142.ParkingNo = pkno;							// 駐車場№
	SendNtnetDt.SData142.PassID = passid;							// 定期券ID

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_142 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_142 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 時計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別229)作成処理                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data229                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data229( void )
{
	if (_is_ntnet_remote()) {
		return;
	}

	NTNET_Snd_Data229_Exec();
}

void	NTNET_Snd_Data229_Exec( void )
{
	struct  clk_rec wk_CLK_REC;
	ushort	wk_CLK_REC_msec;
	ulong	NowLifeTime;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_119 ) );

	BasicDataMake( 229, 1 );										// 基本ﾃﾞｰﾀ作成

	c_Now_CLK_REC_ms_Read( &wk_CLK_REC, &wk_CLK_REC_msec );			// 現在時刻get
	NowLifeTime = LifeTim2msGet();									// 現在LifeTime get

	SendNtnetDt.SData229.Year = wk_CLK_REC.year;					// Year
	SendNtnetDt.SData229.Mon  = wk_CLK_REC.mont;					// Month
	SendNtnetDt.SData229.Day  = wk_CLK_REC.date;					// Day
	SendNtnetDt.SData229.Hour = wk_CLK_REC.hour;					// Hour
	SendNtnetDt.SData229.Min  = wk_CLK_REC.minu;					// Minute
	SendNtnetDt.SData229.Sec  = wk_CLK_REC.seco;					// Second
	SendNtnetDt.SData229.MSec = wk_CLK_REC_msec;					// Millisecond
	//SendNtnetDt.SData229.HOSEI_MSec								// 送信時にセットする
	if( NowLifeTime == 0 ){											// レングスを固定する
		NowLifeTime = 1;											// （0カットされないように）
	}
	SendNtnetDt.SData229.HOSEI_wk = NowLifeTime;					// 補正値 作業域

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_119 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_119 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 端末情報ﾃﾞｰﾀ(ﾃﾞｰﾀ種別230)作成処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data230                                       |*/
/*| PARAMETER    : MachineNo : 送信先端末機械№                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data230( ulong MachineNo )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_230 ) );

	BasicDataMake( 230, 1 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData230.SMachineNo = MachineNo;					// 送信先端末機械№
	memset( SendNtnetDt.SData230.ProgramVer, 0x20, 12 );			// CRMｿﾌﾄﾊﾞｰｼﾞｮﾝ格納ｴﾘｱｸﾘｱ
	memcpy( &SendNtnetDt.SData230.ProgramVer[4], VERSNO.ver_part, 8 );	// CRMｿﾌﾄﾊﾞｰｼﾞｮﾝ格納(右詰め)

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_230 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_230 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 端末状態ﾃﾞｰﾀ(ﾃﾞｰﾀ種別231)作成処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data231                                       |*/
/*| PARAMETER    : MachineNo : 送信先端末機械№                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data231( ulong MachineNo )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_231 ) );

	BasicDataMake( 231, 1 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData231.SMachineNo = MachineNo;					// 送信先端末機械№
	if( OPECTL.Mnt_mod ){											// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ
		SendNtnetDt.SData231.TerminalSt = 9;						// 端末状態
	}else{
		switch( OPECTL.Ope_mod ){
		case 0:														// 待機
		case 100:													// 休業処理
		case 110:													// ﾄﾞｱ閉時ｱﾗｰﾑ表示処理
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		case 230:													// 精算済み案内処理
		case 255:													// チケットレス用起動待機
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		default:
			SendNtnetDt.SData231.TerminalSt = 0;					// 端末状態
			break;
		case 1:														// 駐車位置番号入力処理
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		case 11:													// 修正精算駐車位置番号入力処理
//		case 70:													// ﾊﾟｽﾜｰﾄﾞ入力処理(精算時)
//		case 80:													// ﾊﾟｽﾜｰﾄﾞ登録処理(入庫時)
//		case 90:													// 受付券発行処理
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
			SendNtnetDt.SData231.TerminalSt = 1;					// 端末状態
			break;
		case 2:														// 料金表示,入金処理
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		case 12:													// 修正精算料金表示
//		case 13:													// 修正精算入金処理
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
			SendNtnetDt.SData231.TerminalSt = 2;					// 端末状態
			break;
		case 3:														// 精算完了処理
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		case 14:													// 修正精算完了処理
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
			SendNtnetDt.SData231.TerminalSt = 3;					// 端末状態
			break;
		}
	}
	if( opncls() == 2 ){											// 休業?
		SendNtnetDt.SData231.OpenClose = 1;							// 休業
	}
	SendNtnetDt.SData231.CardNGType = 0;							// NGｶｰﾄﾞ読取内容

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_231 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_231 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| leading zero suppress			                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : regist monitor with informations                        |*/
/*| PARAMETER    : code = monitor code                                     |*/
/*|              : error = error code                                      |*/
/*|              : param = optional data                                   |*/
/*|              : num = number of param digits                            |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.10.10                                              |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	regist_mon(ushort code, ushort error, ushort param, ushort num)
{
	uchar	info[10];
	memset(info, 0, sizeof(info));
	if (error == 0) {
		info[0] =
		info[1] =
		info[2] = ' ';
	}
	else {
		intoas(info, error, 3);
	}
	if (num) {
		intoas(&info[3], param, num);
	}
	wmonlg(code, info, 0);
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定アドレス変換				                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : get_param_addr                                          |*/
/*| PARAMETER    : kind = 0 as all, 1 as r/w                               |*/
/*|              : addr = specified address                                |*/
/*|              : *seg = internal segment address                         |*/
/*|              : *inaddr = internal address                              |*/
/*| RETURN VALUE : 1 as OK                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.9.29                                               |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
	struct _paraaddr_ {
		ushort	topaddr;
		ushort	endaddr;
		ushort	seg;
		ushort	inaddr;
	} addr_tbl[] = {
		{412, 413, 6, 3},	// A車種曜日毎の料金種別
		{414, 415, 6, 9},	// B車種曜日毎の料金種別
		{416, 417, 6,15},	// C車種曜日毎の料金種別
		{418, 419, 6,21},	// D車種曜日毎の料金種別
		{420, 421, 6,27},	// E車種曜日毎の料金種別
		{422, 423, 6,33},	// F車種曜日毎の料金種別
		{424, 425, 6,39},	// G車種曜日毎の料金種別
		{426, 427, 6,45},	// H車種曜日毎の料金種別
		{428, 429, 6,51},	// I車種曜日毎の料金種別
		{430, 431, 6,57},	// J車種曜日毎の料金種別
		{432, 433, 6,63},	// K車種曜日毎の料金種別
		{434, 435, 6,69},	// L車種曜日毎の料金種別
		{436, 438, 6, 4},
		{472, 472, 39, 105},	// 法人カード使用可/不可設定
		{490, 495, 4, 1},	// 特別期間１～３
		{551, 551, 28, 2},

		{558, 559, 28, 13},
		{602, 618, 31, 3},			{660, 899, 31, 61},
		{902, 918, 31, 303},		{960, 1199, 31, 361},
		{1202, 1218, 31, 603},		{1260, 1499, 31, 661},
		{1502,1518, 58, 3},			{1560, 1799, 58, 61},		// 第４料金体系料金設定
		{1802,1818, 58, 303},		{1860, 2099, 58, 361},		// 第５料金体系料金設定
		{2102,2118, 58, 603},		{2160, 2399, 58, 661},		// 第６料金体系料金設定
		{2402,2418, 59, 3},			{2460, 2699, 59, 61},		// 第７料金体系料金設定
		{2702,2718, 59, 303},		{2760, 2999, 59, 361},		// 第８料金体系料金設定
		{3002,3018, 59, 603},		{3060, 3299, 59, 661},		// 第９料金体系料金設定
		{5001,5005, 56, 1},			// 種別毎最大料金設定
		{5020, 5031, 56, 20},		// ｎ時間最大１設定(料金計算拡張)
		{5032, 5043, 56, 32},		// ｎ時間最大２設定
		{5101, 5101, 28, 41},		// ｎ時間最大設定(料金計算拡張無し)
		{5201, 5201, 39, 14},		// ０：００時分割設定
		{5301, 5340, 29,  1},		// サービスタイム設定（グレースタイム・ラグタイム）
		{5351, 5352,  1, 40},		// 時刻指定の営休業切換の営業開始・終了時刻
		{5361, 5363,  2, 29},		// バックライト設定（調整方法・点灯時間）
		{0, 0, 0, 0}
	};

static	int		get_param_addr(ushort kind, ushort addr, ushort *seg, ushort *inaddr)
{
	static	const

	struct _paraaddr_	*p;
	p = addr_tbl;
	while(p->topaddr) {
		if (addr >= p->topaddr && addr <= p->endaddr) {
			*seg = p->seg;
			*inaddr = (addr - p->topaddr) + p->inaddr;
			return 1;
		}
		p++;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定アドレスチェック			                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : check_remote_addr                                       |*/
/*| PARAMETER    : kind = 0 as all, 1 as r/w                               |*/
/*|              : addr = specified address                                |*/
/*|              : count = data count                                      |*/
/*| RETURN VALUE : 1 as OK                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.9.29                                               |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int		check_remote_addr(ushort kind, ushort addr, ushort count)
{
	ushort	seg, inaddr;
	if (count == 0)
		return 0;
	if (count >= 19)		//最大設定データ数：18
		return 0;
	while(count) {
		if (!get_param_addr(kind, addr, &seg, &inaddr))
			return 0;
		addr++;
		count--;
	}
	return 1;
}

/*[]----------------------------------------------------------------------[]*/
/*| 10進ﾊﾞｲﾅﾘｰの指定桁を０にする                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : param6_mask_zero         							   |*/
/*| PARAMETER    : indat(in) 元データ                                      |*/
/*| PARAMETER    : maskptn(in) マスクパターン(16進.ex> 0x00000F=1の位を0   |*/
/*| RETURN VALUE : MASK後ﾃﾞｰﾀ											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : COSMO_OKAMOTO                                           |*/
/*| Date         : 2008.6.30                                               |*/
/*| UpDate       :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
long param6_mask_zero ( long indat, long maskptn )
{
	unsigned char	cnt;
	unsigned char	lkt[6];
	long			ldat,lmsk;

	ldat = indat;
	lmsk = 0x0f;
	for( cnt = 0; cnt < 6; cnt ++){
		if( maskptn & lmsk ){
			lkt[cnt] = 0;
		}else{
			lkt[cnt] = (unsigned char)(ldat % 10L);
		}
		ldat /= 10L;
		lmsk <<= 4;
	}
	return (long)( 	(long)lkt[5]*100000L + 
					(long)lkt[4]*10000L + 
					(long)lkt[3]*1000L + 
					(long)lkt[2]*100L + 
					(long)lkt[1]*10L + 
					(long)lkt[0] );
}
//10進の１桁の数値を取り出すマクロ EX.> MC_UPPARA_P10( 123456 ) ==> 5 //
#define		MC_UDPARA_P1000(val) ((val % 10000L) / 1000L)
#define		MC_UDPARA_P100(val) ((val % 1000L) / 100L)
#define		MC_UDPARA_P10(val) ((val % 100L) / 10L)
#define		MC_UDPARA_P1(val) (val % 10L)
#define		MAX_TYPE_SET_S	5002		// 最大料金タイプ設定アドレス(開始)
#define		MAX_TYPE_SET_E	5005		// 最大料金タイプ設定アドレス(終了)
#define		FT4500_MX_BAND		8		// FT4500時間帯最大設定値
#define		FT4500_MX_N_HOUR	7		// FT4500ｎ時間ｍ回最大設定値
#define		GETA_10_4	10000L			// パラメータ算出用
#define		GETA_10_2	100L			// パラメータ算出用
/*[]----------------------------------------------------------------------[]*/
/*| 料金設定ログ(遠隔料金設定変更用)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ryokin_Settei_Log                                       |*/
/*| PARAMETER    : addr   :		アドレス(FT4500互換)                       |*/
/*|              : data	  :		設定値                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2010.05.20                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	Ryokin_Settei_Log(ushort addr, long data)
{
	memcpy( &remote_Change_data.rcv_Time, &CLK_REC, sizeof( date_time_rec ) );		// 現在時刻
	remote_Change_data.addr = addr;													// 設定アドレス
	remote_Change_data.rcv_data = data;												// 設定データ

	Log_Write(eLOG_REMOTE_SET, &remote_Change_data, TRUE);							// 遠隔料金設定ログ
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定データ更新					                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : update_param                                            |*/
/*| PARAMETER    : none                                                    |*/
/*| RETURN VALUE : 1 as OK                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.9.29                                               |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int		update_param(void)
{
	int		blks;
	struct _setdata_	*p;
	ushort	addr, i;
	ushort	seg, inaddr;
	long	lpwk;
	long	lpwk2;

	blks = tmp_buffer.bufcnt;
	p = (struct _setdata_*)tmp_buffer.buffer;
	while(blks) {
		addr = p->addr;
		for (i = 0; i < p->count; i++) {
			get_param_addr(1, addr, &seg, &inaddr);
			Ryokin_Settei_Log(addr, p->data[i]);									// 遠隔設定ログ

			switch( addr ){
			case 551:
				if ( p->data[i] == 7L ){
					CPrmSS[seg][inaddr] = 10L;
				}
				else if ( p->data[i] == 8L ){
					CPrmSS[seg][inaddr] = 5L;
				}else{
					CPrmSS[seg][inaddr] = p->data[i];
				}
				break;
			case 558:
					lpwk = ((prm_get( COM_PRM, S_CAL, 38, 2, 5 )  * 10000L) + 
							( p->data[i] ) +
							( prm_get( COM_PRM, S_CAL, 38, 2, 1 ) ));
					CPrmSS[S_CAL][38] = lpwk;
				break;

			case 559:
					lpwk = ((prm_get( COM_PRM, S_CAL, 38, 2, 5 )  * 10000L) + 
							( prm_get( COM_PRM, S_CAL, 38, 2, 3 ) * 100L) +
							( p->data[i] % 100L ));
					CPrmSS[S_CAL][38] = lpwk;
				break;
			case 412:
			case 414:	// B種
			case 416:	// C種
			case 418:	// D種
			case 420:	// E種
			case 422:	// F種
			case 424:	// G種
			case 426:	// H種
			case 428:	// I種
			case 430:	// J種
			case 432:	// K種
			case 434:	// L種
				// [＿月火水木＿]
				CPrmSS[S_SHA][(3+((addr%412)*3))] = param6_mask_zero ( CPrmSS[S_SHA][(3+((addr%412)*3))], 0x0ffff0 ) + ((p->data[i] % 10000L) * 10L);
				break;
			case 413:
			case 415:	// B種
			case 417:	// C種
			case 419:	// D種
			case 421:	// E種
			case 423:	// F種
			case 425:	// G種
			case 427:	// H種
			case 429:	// I種
			case 431:	// J種
			case 433:	// K種
			case 435:	// L種
				lpwk = param6_mask_zero ( CPrmSS[S_SHA][(3+((addr%413)*3))], 0xf0000f );
				lpwk += MC_UDPARA_P1000(p->data[i]);	 		// =>[－－－－－金]
				lpwk += MC_UDPARA_P10  (p->data[i]) * 100000L;	// =>[日－－－－－]
				CPrmSS[S_SHA][(3+((addr%413)*3))] = lpwk;

				lpwk = param6_mask_zero ( CPrmSS[S_SHA][(4+((addr%413)*3))], 0xff0000 );
				lpwk += MC_UDPARA_P100(p->data[i]) * 100000L;	// =>[土－－－－－]
				lpwk += MC_UDPARA_P1  (p->data[i]) * 10000L;	// =>[－特－－－－]
				CPrmSS[S_SHA][(4+((addr%413)*3))] = lpwk;
				break;
			case 436:
				//特別期間 A =>[..O...]
				CPrmSS[S_SHA][4] = param6_mask_zero ( CPrmSS[S_SHA][4],  0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1000 (p->data[i])));
				//特別期間 B =>[..O...]
				CPrmSS[S_SHA][10] = param6_mask_zero( CPrmSS[S_SHA][10], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P100 (p->data[i])));
				//特別期間 C =>[..O...]
				CPrmSS[S_SHA][16] = param6_mask_zero( CPrmSS[S_SHA][16], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P10 (p->data[i])));
				//特別期間 D =>[..O...]
				CPrmSS[S_SHA][22] = param6_mask_zero( CPrmSS[S_SHA][22], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1 (p->data[i])));
				break;
			case 437:
				//特別期間 E =>[..O...]
				CPrmSS[S_SHA][28] = param6_mask_zero( CPrmSS[S_SHA][28], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1000 (p->data[i])));
				//特別期間 F =>[..O...]
				CPrmSS[S_SHA][34] = param6_mask_zero( CPrmSS[S_SHA][34], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P100 (p->data[i])));
				//特別期間 G =>[..O...]
				CPrmSS[S_SHA][40] = param6_mask_zero( CPrmSS[S_SHA][40], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P10 (p->data[i])));
				//特別期間 H =>[..O...]                                               
				CPrmSS[S_SHA][46] = param6_mask_zero( CPrmSS[S_SHA][46], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1 (p->data[i])));
				break;
			case 438:
				//特別期間 I =>[..O...]
				CPrmSS[S_SHA][52] = param6_mask_zero( CPrmSS[S_SHA][52], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1000 (p->data[i])));
				//特別期間 J =>[..O...]
				CPrmSS[S_SHA][58] = param6_mask_zero( CPrmSS[S_SHA][58], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P100 (p->data[i])));
				//特別期間 K =>[..O...]
				CPrmSS[S_SHA][64] = param6_mask_zero( CPrmSS[S_SHA][64], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P10 (p->data[i])));
				//特別期間 L =>[..O...]
				CPrmSS[S_SHA][70] = param6_mask_zero( CPrmSS[S_SHA][70], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1 (p->data[i])));
				break;
			case 472:
				CPrmSS[seg][inaddr] = ((p->data[i]/1000L)%10L);						// 1000の桁をそのままセットする
				break;				
			case 5361:
				lpwk = ((prm_get(COM_PRM, (short)seg, (short)inaddr, 1, 2) * 10L) + (p->data[i]%10L));	// テンキー設定＋1の桁のみセットする
				CPrmSS[seg][inaddr] = lpwk;											
				break;				
			default:
				if( addr >= MAX_TYPE_SET_S && addr <= MAX_TYPE_SET_E){				// 種別毎の最大料金タイプの設定 
				/* 最大料金タイプはFT4500用の設定値で送られてくるので変換する(7 → 10 8 → 5) */
				/* ひとつのアドレスに３つの設定が入っているため２桁ずつ抽出する */
					lpwk = p->data[i]/GETA_10_4;					// ①②を抽出
					if(lpwk == FT4500_MX_N_HOUR){					// ｎ時間ｍ回最大
						lpwk2 = SP_MX_N_MH_NEW*GETA_10_4;			// FT4800では１０
					}
					else if(lpwk == FT4500_MX_BAND){				// 時間帯最大
						lpwk2 = SP_MX_BAND*GETA_10_4;				// FT4800では５
					}
					else {
						lpwk2 = lpwk*GETA_10_4;
					}
					lpwk = (p->data[i]%GETA_10_4)/GETA_10_2;		// ③④を抽出
					if(lpwk == FT4500_MX_N_HOUR){
						lpwk2 += SP_MX_N_MH_NEW*GETA_10_2;
					}
					else if(lpwk == FT4500_MX_BAND){
						lpwk2 += SP_MX_BAND*GETA_10_2;
					}
					else {
						lpwk2 += lpwk*GETA_10_2;
					}
					lpwk = p->data[i]%GETA_10_2;						// ⑤⑥を抽出
					if(lpwk == FT4500_MX_N_HOUR){
						lpwk2 += SP_MX_N_MH_NEW;
					}
					else if(lpwk == FT4500_MX_BAND){
						lpwk2 += SP_MX_BAND;
					}
					else {
						lpwk2 += lpwk;
					}
					CPrmSS[seg][inaddr] = lpwk2;
					break;
				}
				CPrmSS[seg][inaddr] = p->data[i];
				break;
			}
			addr++;
		}
		p = (struct _setdata_*)&p->data[p->count];
		blks--;
	}
	return 1;
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算機状態チェック				                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : check_acceptable                                        |*/
/*| PARAMETER    : none                                                    |*/
/*| RETURN VALUE : 1 as OK                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.11.06                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int		check_acceptable(void)
{
	if (OPECTL.Mnt_mod == 0 &&
		(OPECTL.Ope_mod == 0				// 待機
		 || OPECTL.Ope_mod == 1				// 駐車券待ち
		 || OPECTL.Ope_mod == 100))			// 休業?
		return 1;
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定準備クリア					                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_ClrSetup                                          |*/
/*| PARAMETER    : none                                                    |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.11.06                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_ClrSetup(void)
{
	tmp_buffer.prepare = 0;					// cancel prepare command
}

/*[]----------------------------------------------------------------------[]*/
/*| 共通設定ﾃﾞｰﾀ(ﾃﾞｰﾀ種別80)受信処理                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData80                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData80( void )
{
	RP_DATA_KIND_80	*msg = &RecvNtnetDt.RP_RData80;
	ushort	moncode;
	
	uchar	okng;
	int		size;

	size = (2+2) + msg->DataCount * (int)(sizeof(long));
	okng = 1;
	if (msg->SMachineNo != (ulong)CPrmSS[S_PAY][2]) {
		okng = _RPKERR_INVALID_MACHINENO;
	}
// prepare command is received?
	else if (tmp_buffer.prepare == 0) {
		okng = _RPKERR_NO_PREPARE_COMMAND;
	}
// specified address is valid?
	else if (msg->Segment != 0 ||
			check_remote_addr(1, msg->TopAddr, msg->DataCount) == 0) {
		okng = _RPKERR_INVALID_PARA_ADDR;
	}
// data is bufferable?
	else if ((tmp_buffer.bufofs + size) >= sizeof(tmp_buffer.buffer)) {
		okng = _RPKERR_NO_PREPARE_COMMAND;		// overflow
	}
	else if (! check_acceptable()) {
		okng = _RPKERR_COMMAND_REJECT;
	}
	else {
// all OK! now save the data

		memcpy(&tmp_buffer.buffer[tmp_buffer.bufofs], &msg->TopAddr, (size_t)size);
		tmp_buffer.bufofs += size;
		tmp_buffer.bufcnt++;
	}
// answer
	if (msg->Status == 1) {
		NTNET_Ans_Data99(msg->DataBasic.SystemID, msg->DataBasic.MachineNo,
								NTNET_COMPLETE_CPRM, okng);
	}
// regist monitor
	moncode = OPMON_RSETUP_RCVOK;
	if (okng == 1) {
		okng = 0;
	}
	else {
		moncode++;
		NTNET_ClrSetup();					// cancel prepare command if error
	}
	regist_mon(moncode, (ushort)okng, msg->TopAddr, 6);
}

/*[]----------------------------------------------------------------------[]*/
/*| 特別日設定データ(ﾃﾞｰﾀ種別83)受信処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData83                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2006-09-27                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData83( void )
{
	ushort 	i;
	ulong 	date1, date2, date3, shift;

	f_NTNET_RCV_SPLDATE = 1;					// 特別日設定データ受信による共通ﾊﾟﾗﾒｰﾀ更新中

	// 特別日設定受信ﾃﾞｰﾀを共通ﾊﾟﾗﾒｰﾀTBLに反映する。
	for( i=0; i<3; i++ ){												// 特別期間１～３
		date1 = (ulong)RecvNtnetDt.RData83.kikan[i].sta_Mont;
		date1 *= 100;
		date1 += (ulong)RecvNtnetDt.RData83.kikan[i].sta_Date;
		CPrmSS[S_TOK][i*2+1] = date1;									// 開始月日

		date1 = (ulong)RecvNtnetDt.RData83.kikan[i].end_Mont;
		date1 *= 100;
		date1 += (ulong)RecvNtnetDt.RData83.kikan[i].end_Date;
		CPrmSS[S_TOK][i*2+2] = date1;									// 終了月日
	}

	for( i=0; i<31; i++ ){												// 特別日＆シフト１～３１
		date1  = (ulong)RecvNtnetDt.RData83.date[i].mont;
		date1 *= 100;
		date1 += (ulong)RecvNtnetDt.RData83.date[i].day;
		shift = (ulong)RecvNtnetDt.RData83.Shift[i];

		CPrmSS[S_TOK][NTNET_SPLDAY_START+i] = shift*100000 + date1;		// シフト＋月日
	}

	if (CPrmSS[S_TOK][41]) {
		for( i=0; i<12; i++ ){											// 特別曜日１～１２
			date1 = (ulong)RecvNtnetDt.RData83.yobi[i].mont;			// 月
			date2 = (ulong)RecvNtnetDt.RData83.yobi[i].week;			// 週
			date3 = (ulong)RecvNtnetDt.RData83.yobi[i].yobi;			// 曜日

			CPrmSS[S_TOK][NTNET_SPYOBI_START+i] = date1*100 + date2*10 +date3;
		}
	}
	else {
		for( i=0; i<6; i++ ){											// ハッピーマンデー１～１２
			date1 = 0;
			date2 = 0;
			if(chk_hpmonday(RecvNtnetDt.RData83.yobi[i*2])) {			// ハッピーマンデーにマッチするか（前）
				date1 = (ulong)RecvNtnetDt.RData83.yobi[i*2].mont;		// 月
				date1 *= 10;
				date1 += (ulong)RecvNtnetDt.RData83.yobi[i*2].week;		// 週
			}
			if(chk_hpmonday(RecvNtnetDt.RData83.yobi[i*2+1])) {			// ハッピーマンデーにマッチするか（後）
				date2 = (ulong)RecvNtnetDt.RData83.yobi[i*2+1].mont;	// 月
				date2 *= 10;
				date2 += RecvNtnetDt.RData83.yobi[i*2+1].week;			// 週
			}

			CPrmSS[S_TOK][NTNET_HMON_START+i] = date1*1000 + date2;		// ハッピーマンデー（前＋後）
		}
	}
	
	CPrmSS[S_TOK][NTNET_5_6] = ((RecvNtnetDt.RData83.spl56 & 0x01 ) ^ 0x01);		// 5／6を特別日とする
																		// spl56が0の場合は1とし、spl56が1の場合は0とする
	for( i=0 ; i<6; i++ ){												// 特別年月日１～６
		date1 = (ulong)RecvNtnetDt.RData83.year[i].year;				// 年
		if( date1 >= 2000 ){
			date1 -= 2000;
		}
		date2 = (ulong)RecvNtnetDt.RData83.year[i].mont;				// 月
		date3 = (ulong)RecvNtnetDt.RData83.year[i].day;					// 日

		CPrmSS[S_TOK][NTNET_SPYEAR_START+i] = date1*10000 + date2*100 + date3;
	}

	DataSumUpdate(OPE_DTNUM_COMPARA);									// 共通設定ﾃﾞｰﾀｻﾑを更新
	FLT_WriteParam1(FLT_NOT_EXCLUSIVE);									// FlashROM update
	SetSetDiff(SETDIFFLOG_SYU_REMSET);									// デフォルトセットログ登録処理

	f_NTNET_RCV_MC10_EXEC = 1;											// mc10()更新が必要な設定が通信で更新された情報set
	wopelg(OPLOG_SET_TOKUBETU_UPDATE, 0, 0);							// 操作履歴登録

	NTNET_Snd_Data99(RecvNtnetDt.RData83.DataBasic.MachineNo, NTNET_COMPLETE_SPDAY);	// NT-NET設定受信完了データ作成
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券精算中止データテーブル(ﾃﾞｰﾀ種別91)受信処理                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData91                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData91( void )
{
	t_TKI_CYUSI *work = &z_Work.TeikiCyusi;	// 定期券中止データ編集用ワーク
	TEIKI_CHUSI *ntdata;
	short idx,i;
	
	f_NTNET_RCV_TEIKITHUSI = 1;

	switch (RecvNtnetDt.RData91.ProcMode) {
	case 0:		// 1件更新
		ntdata = &RecvNtnetDt.RData91.TeikiChusi[0];
		// 処理対象とするデータを検索
		idx = ntnet_SearchTeikiCyusiData(ntdata->ParkingNo, ntdata->PassID);
		// 停電保証用ワークエリアにデータ作成
		memcpy(work, &tki_cyusi, sizeof(t_TKI_CYUSI));
		if (idx >= 0) {
			// 該当データあり → そのデータを上書き
			if (ntnet_TeikiCyusiCnvNtToCrm(&work->dt[_TKI_Idx2Ofs(idx)], &RecvNtnetDt.RData91.TeikiChusi[0])) {
				// ワークエリアから実領域へ書き込み
				nmisave(&tki_cyusi, work, sizeof(t_TKI_CYUSI));
				wopelg(OPLOG_SET_TEIKI_TYUSI, 0, 0);
			}
		}
		else {
			// 該当データなし → 最後尾に追加
			if (ntnet_TeikiCyusiCnvNtToCrm(&work->dt[work->wtp], &RecvNtnetDt.RData91.TeikiChusi[0])) {
				if (work->count < TKI_CYUSI_MAX) {
					work->count++;
				}
				if (++work->wtp >= TKI_CYUSI_MAX) {
					work->wtp = 0;
				}
				// ワークエリアから実領域へ書き込み
				nmisave(&tki_cyusi, work, sizeof(t_TKI_CYUSI));
				wopelg(OPLOG_SET_TEIKI_TYUSI, 0, 0);
			}
		}
		break;
	case 1:		// 1件削除
		ntdata = &RecvNtnetDt.RData91.TeikiChusi[0];
		idx = ntnet_SearchTeikiCyusiData(ntdata->ParkingNo, ntdata->PassID);
		if (idx >= 0) {
			TKI_Delete(idx);
			wopelg(OPLOG_SET_TEIKI_TYUSI, 0, 0);
		}
		break;
	case 2:		// 全データ更新
		// 停電保証用ワークエリアにデータ作成
		memset(work, 0, sizeof(t_TKI_CYUSI));
		i = 0;
		while (work->count < TKI_CYUSI_MAX) {
			if (RecvNtnetDt.RData91.TeikiChusi[i].ParkingNo == 0) {
				// 以降、有効データなし
				break;
			}
			if (ntnet_TeikiCyusiCnvNtToCrm(&work->dt[work->count], &RecvNtnetDt.RData91.TeikiChusi[i]) == FALSE) {
				i++;
				continue;
			}
			i++;
			work->count++;
		}
		if (work->count < TKI_CYUSI_MAX) {
			work->wtp = work->count;
		} else {
			work->wtp = 0;
		}
		// ワークエリアから実領域へ書き込み
		nmisave(&tki_cyusi, work, sizeof(t_TKI_CYUSI));
		wopelg(OPLOG_SET_TEIKI_TYUSI, 0, 0);
		break;
	default:
		break;
	}
	NTNET_Snd_Data99(RecvNtnetDt.RData91.DataBasic.MachineNo, NTNET_COMPLETE_TEIKI_TYUSHI);		/* 設定受信完了通知ﾃﾞｰﾀ送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別93)受信処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData93                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData93( void )
{
	/* 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ更新 */
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	memcpy(pas_tbl, RecvNtnetDt.RData93.PassTable, sizeof(pas_tbl));
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
	wopelg(OPLOG_SET_STATUS_TBL_UPDATE, 0, 0);		// 操作履歴登録
	
	NTNET_Snd_Data99(RecvNtnetDt.RData93.DataBasic.MachineNo, NTNET_COMPLETE_TEIKI_STS);		/* 設定受信完了通知ﾃﾞｰﾀ送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券更新ﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別94)受信処理                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData94                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData94( void )
{
	/* 定期券更新ﾃｰﾌﾞﾙ更新 */
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	memcpy(pas_renewal, RecvNtnetDt.RData94.PassRenewal, sizeof(pas_renewal));
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
	NTNET_Snd_Data99(RecvNtnetDt.RData94.DataBasic.MachineNo, NTNET_COMPLETE_TEIKI_UPDATE);		/* 設定受信完了通知ﾃﾞｰﾀ送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券出庫時刻ﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別95)受信処理                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData95                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData95( void )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	short	i;
//
//	f_NTNET_RCV_TEIKIEXTIM = 1;
//
//	/* 定期券出庫時刻ﾃｰﾌﾞﾙ更新 */
//	for (i = 0; i < PASS_EXTIMTBL_MAX; i++) {
//		if (RecvNtnetDt.RData95.PassExTable[i].PassId == 0)
//			break;
//		pas_extimtbl.PassExTbl[i] = RecvNtnetDt.RData95.PassExTable[i];
//	}
//	pas_extimtbl.Count = i;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
	
	NTNET_Snd_Data99(RecvNtnetDt.RData95.DataBasic.MachineNo, NTNET_COMPLETE_TEIKI_CAROUT);		/* 設定受信完了通知ﾃﾞｰﾀ送信 */
}

/*[]----------------------------------------------------------------------[]*
 *| 車室パラメータ受信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData97
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2005-11-30
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData97( void )
{

	if (RecvNtnetDt.RData97.ModelCode != NTNET_MODEL_CODE) {
		return;
	}
	
	if (OPESETUP_SetupDataChk(97, 0)) {
		NtNet_FukudenParam.DataKind	= 97;
		NtNet_FukudenParam.Src		= &RecvNtnetDt.RData97;
		NTNET_UpdateParam(&NtNet_FukudenParam);
		wopelg(OPLOG_SET_SYASITUPARAM, 0, 0);
	}
	
	NTNET_Snd_Data99(RecvNtnetDt.RData97.DataBasic.MachineNo, NTNET_COMPLETE_SHASHITSU_PRM);	// NT-NET設定受信完了データ作成
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾛｯｸ装置ﾊﾟﾗﾒｰﾀ設定(ﾃﾞｰﾀ種別98)受信処理                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData98                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART machida                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData98( void )
{

	if (RecvNtnetDt.RData98.ModelCode != NTNET_MODEL_CODE) {
		return;
	}
	if (OPESETUP_SetupDataChk(98, 0)) {
		NtNet_FukudenParam.DataKind	= 98;
		NtNet_FukudenParam.Src		= &RecvNtnetDt.RData98;
		NTNET_UpdateParam(&NtNet_FukudenParam);
		wopelg(OPLOG_SET_LOCKPARAM, 0, 0);
	}
	
	NTNET_Snd_Data99(RecvNtnetDt.RData98.DataBasic.MachineNo, NTNET_COMPLETE_LOCK_PRM);	// NT-NET設定受信完了データ作成
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾃﾞｰﾀ要求2結果NG(ﾃﾞｰﾀ種別110)受信処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData110                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData110( void )
{
	if( (RecvNtnetDt.RData110.ControlData[3] == 1) || 
		(RecvNtnetDt.RData110.ControlData[4] == 1) ||
		(RecvNtnetDt.RData110.ControlData[4] == 2) ){
		NTNetTotalEndError = RecvNtnetDt.RData110.Result;
		if (NTNetTotalEndFlag == 0) {
			NTNetTotalEndFlag = -1;		// 複数集計データ受信失敗
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期データ更新(ﾃﾞｰﾀ種別116)受信処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData116                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData116( void )
{
	ushort status;
//	ushort use_parking;
	ulong use_parking;
	ushort valid;
	date_time_rec exit_time;
	uchar parking_no_check=0;

	status = 0xFFFF;
	use_parking = 0xFFFFFFFF;
	valid = 0xFFFF;

	parking_no_check = NTNET_GetParkingKind( RecvNtnetDt.RData116.ParkingNo, PKOFS_SEARCH_LOCAL );

	if (RecvNtnetDt.RData116.Status <= NTNET_PASSUPDATE_INVALID) {
		// １項目ずつの設定
		if ((RecvNtnetDt.RData116.ProcMode & NTNET_PASSUPDATE_STATUS) == NTNET_PASSUPDATE_STATUS) {
			// 定期ステータステーブル更新
			if (RecvNtnetDt.RData116.ParkingNo != 0 && RecvNtnetDt.RData116.PassID != 0) {
				if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_INVALID) {
					if( parking_no_check != 0xFF ){
						if(RecvNtnetDt.RData116.ParkingNo < 1000 ){			//APS
							wopelg(OPLOG_SET_TEIKI_YUKOMUKO, 0, Convert_PassInfo(RecvNtnetDt.RData116.ParkingNo,RecvNtnetDt.RData116.PassID));		// 操作履歴登録
						}else{												//GT
							wopelg(OPLOG_GT_SET_TEIKI_YUKOMUKO, RecvNtnetDt.RData116.ParkingNo,RecvNtnetDt.RData116.PassID);		// 操作履歴登録
						}
					}
					valid = 1;
				} else {
					if( parking_no_check != 0xFF ){
						if(RecvNtnetDt.RData116.ParkingNo < 1000 ){			//APS
							wopelg(OPLOG_SET_STATUS_CHENGE, 0, Convert_PassInfo(RecvNtnetDt.RData116.ParkingNo,RecvNtnetDt.RData116.PassID));		// 操作履歴登録
						}else{												//GT
							wopelg(OPLOG_GT_SET_STATUS_CHENGE, RecvNtnetDt.RData116.ParkingNo,RecvNtnetDt.RData116.PassID);		// 操作履歴登録
						}
					}
					valid = 0;
					status = RecvNtnetDt.RData116.Status;
				}
				if (RecvNtnetDt.RData116.UsingParkingNo != 0) {
					use_parking = (ulong)RecvNtnetDt.RData116.UsingParkingNo;
				}

				WritePassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, (ushort)RecvNtnetDt.RData116.PassID, status, use_parking, valid );
			}
		}

		if ((RecvNtnetDt.RData116.ProcMode & NTNET_PASSUPDATE_UPDATESTATUS) == NTNET_PASSUPDATE_UPDATESTATUS) {
			// 定期更新ステータステーブル更新
			if (RecvNtnetDt.RData116.UpdateStatus == 0 || RecvNtnetDt.RData116.UpdateStatus == 1) {
				if (RecvNtnetDt.RData116.ParkingNo != 0 && RecvNtnetDt.RData116.PassID != 0) {
				}
			} else if (RecvNtnetDt.RData116.UpdateStatus == 2 || RecvNtnetDt.RData116.UpdateStatus == 3) {
				if (RecvNtnetDt.RData116.ParkingNo != 0) {
				}
			}
		}

		if ((RecvNtnetDt.RData116.ProcMode & NTNET_PASSUPDATE_OUTTIME) == NTNET_PASSUPDATE_OUTTIME) {
			// 出庫時刻テーブル更新
			if ((ushort)RecvNtnetDt.RData116.ParkingNo  == 0 && (ushort)RecvNtnetDt.RData116.PassID == 0){
				if (RecvNtnetDt.RData116.OutYear == 0
				&& RecvNtnetDt.RData116.OutMonth == 0
				&& RecvNtnetDt.RData116.OutDay == 0
				&& RecvNtnetDt.RData116.OutHour == 0
				&& RecvNtnetDt.RData116.OutMin == 0) {
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//						// 駐車場番号　定期番号　出庫時刻　オール　０　のため出庫時刻テーブル全削除。
//						memset(&pas_extimtbl, 0, sizeof(PASS_EXTIMTBL));
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
				}
			} else if ((ushort)RecvNtnetDt.RData116.ParkingNo != 0 && (ushort)RecvNtnetDt.RData116.PassID != 0) {
				if (RecvNtnetDt.RData116.OutYear == 0
				&& RecvNtnetDt.RData116.OutMonth == 0
				&& RecvNtnetDt.RData116.OutDay == 0
				&& RecvNtnetDt.RData116.OutHour == 0
				&& RecvNtnetDt.RData116.OutMin == 0) {
					// 出庫時刻　オール　０　のため出庫時刻テーブル削除。
					PassExitTimeTblDelete( (ulong)RecvNtnetDt.RData116.ParkingNo, (ushort)RecvNtnetDt.RData116.PassID );
				} else {
					exit_time.Year	= RecvNtnetDt.RData116.OutYear;
					exit_time.Mon	= RecvNtnetDt.RData116.OutMonth;
					exit_time.Day	= RecvNtnetDt.RData116.OutDay;
					exit_time.Hour	= RecvNtnetDt.RData116.OutHour;
					exit_time.Min	= RecvNtnetDt.RData116.OutMin;
					PassExitTimeTblWrite( (ulong)RecvNtnetDt.RData116.ParkingNo, (ushort)RecvNtnetDt.RData116.PassID, &exit_time );
				}
			}
		}

	} else {
		// １定期テーブル設定
		if (RecvNtnetDt.RData116.ParkingNo != 0) {

			if (RecvNtnetDt.RData116.UsingParkingNo != 0) {
				use_parking = (ulong)RecvNtnetDt.RData116.UsingParkingNo;
			}
			if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_ALL_INITIAL) {
				// 全項目初期化 4
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 0, use_parking, 0, 0 );
				wopelg(OPLOG_SET_TEIKI_ALLYUKO, 0, 0);		// 操作履歴登録

			} else if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_ALL_INITIAL2) {
				// 定期テーブル内の有効項目のみ初期化 11
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 0, use_parking, 0, 1 );

			} else if (NTNET_PASSUPDATE_ALL_IN == RecvNtnetDt.RData116.Status) {
				// 全項目入庫中 5
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 2, use_parking, 0, 0 );

			} else if (NTNET_PASSUPDATE_ALL_OUT == RecvNtnetDt.RData116.Status) {
				// 全項目出庫中 6
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 1, use_parking, 0, 0 );

			} else if (NTNET_PASSUPDATE_ALL_OUT_FREE == RecvNtnetDt.RData116.Status) {
				// 全項目入庫可 7
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 2, use_parking, 0, 2 );

			} else if (NTNET_PASSUPDATE_ALL_IN_FREE == RecvNtnetDt.RData116.Status) {
				// 全項目出庫可 8
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 1, use_parking, 0, 3 );

			} else if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_ALL_INVALID) {
				// １定期テーブル内の有効・無効ビットのみ無効に 9
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 0xFFFF, use_parking, 1, 0 );
				wopelg(OPLOG_SET_TEIKI_ALLMUKO, 0, 0);		// 操作履歴登録

			} else if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_ALL_VALID) {
				// １定期テーブル内の有効・無効ビットのみ有効に 10
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 0xFFFF, use_parking, 0, 0 );
				wopelg(OPLOG_SET_TEIKI_ALLYUKO, 0, 0);		// 操作履歴登録
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定データ送信応答(ID84)作成処理                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data84                                        |*/
/*| PARAMETER    : MachineNo : 送信先端末機械№                            |*/
/*|                ng  : 1 = 送信NG, 0 = 送信OK                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data84(ulong MachineNo, uchar ng)
{
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定受信完了通知データ(ID99)作成処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data99                                        |*/
/*| PARAMETER    : MachineNo : 送信先端末機械№                            |*/
/*|                CompleteKind  : 設定完了したデータの種類                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data99(ulong MachineNo, int CompleteKind)
{
	if (CompleteKind < NTNET_COMPLETE_MAX) {
		memset(&SendNtnetDt, 0, sizeof(DATA_KIND_99));
		BasicDataMake(99, 1);										// 基本ﾃﾞｰﾀ作成
		SendNtnetDt.SData99.SMachineNo = MachineNo;					// 送信先端末機械№
		SendNtnetDt.SData99.CompleteInfo[CompleteKind] = 1;			// 設定完了情報
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_99 ));
		}
		else {
			NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_99), NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 制御応答ﾃﾞｰﾀ(ﾃﾞｰﾀ種別101)作成処理 （リセット応答専用）                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data101_2                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data101_2( void )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_100 ) );

	BasicDataMake( 101, 1 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData101.SMachineNo = 0;							// 送信先端末機械№

	SendNtnetDt.SData101.ControlData[7] = 1;						// 自動

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_100 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}

	NTNetID100ResetFlag = 0;										// リセットフラグ解除
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定受信完了通知データ(ID99)作成処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Ans_Data99                                        |*/
/*| PARAMETER    : SysID : 送信先システムID                                |*/
/*|              : MachineNo : 送信先端末機械№                            |*/
/*|                CompleteKind  : 設定完了したデータの種類                |*/
/*|                CompleteInfo  : 応答内容                                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.10.1                                               |*/
/*| NOTE         : リパーク設定応答用                                      |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Ans_Data99(uchar SysID, ulong MachineNo, int CompleteKind, uchar CompleteInfo)
{
	DATA_KIND_99	*msg = &SendNtnetDt.SData99;

	if (CompleteKind < NTNET_COMPLETE_MAX) {
		memset(msg, 0, sizeof(*msg));
		BasicDataMake(99, 1);								// 基本ﾃﾞｰﾀ作成
		msg->SMachineNo = MachineNo;						// 送信先端末機械№
		msg->CompleteInfo[CompleteKind] = CompleteInfo;		// 設定完了情報
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(*msg));
		}
		else {
			NTBUF_SetSendNtData(msg, sizeof(*msg), NTNET_BUF_PRIOR);	// ﾃﾞｰﾀ送信登録(優先データ）
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾃﾞｰﾀ要求2(ﾃﾞｰﾀ種別109)作成処理                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data109                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data109( ulong req, char value )
{
	int i;

	if (_is_ntnet_remote()) {
		return;
	}

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_109 ) );
	
	BasicDataMake( 109, 1 );										// 基本ﾃﾞｰﾀ作成
	
	SendNtnetDt.SData109.SMachineNo = 0L;							// 送信先端末機械№

	NTNetTotalEndError = 0;

	if (req & (NTNET_DATAREQ2_MSYOUKEI | NTNET_DATAREQ2_MGOUKEI | NTNET_DATAREQ2_NMSYOUKEI)) {
		memset( &sky.fsyuk, 0, sizeof( SYUKEI ) );
	}
	
	// 要求内容をセット(複数小計,複数合計,現在複数小計は多要求と併用できない仕様のため個別に設定する)
	if ((req & NTNET_DATAREQ2_MSYOUKEI) == NTNET_DATAREQ2_MSYOUKEI) {
		SendNtnetDt.SData109.ControlData[3] = 1;				// 複数小計
		NTNetTotalEndFlag = 0;	// 複数集計データ受信待機中
	} else if ((req & NTNET_DATAREQ2_MGOUKEI) == NTNET_DATAREQ2_MGOUKEI) {
		SendNtnetDt.SData109.ControlData[4] = value;			// 複数合計
		NTNetTotalEndFlag = 0;	// 複数集計データ受信待機中
	} else if ((req & NTNET_DATAREQ2_NMSYOUKEI) == NTNET_DATAREQ2_NMSYOUKEI) {
		SendNtnetDt.SData109.ControlData[12] = value;			// 現在複数小計
		NTNetTotalEndFlag = 0;	// 複数集計データ受信待機中
	} else {
		for (i = 0; i < _countof(SendNtnetDt.SData109.ControlData); i++) {
			if (req & (0x00000001L << i)) {
				SendNtnetDt.SData109.ControlData[i] = 1;				// 要求ON
			}
		}
		
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_109 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_109 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券データ更新(ﾃﾞｰﾀ種別116)作成処理                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data116                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data116( uchar ProcMode, ulong PassId, ulong ParkingId, uchar UseParkingKind, uchar Status, uchar UpdateStatus, date_time_rec *ExitTime )
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_116 ) );

	BasicDataMake( 116, 1 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData116.ProcMode = ProcMode;
	SendNtnetDt.SData116.PassID = PassId;
	SendNtnetDt.SData116.ParkingNo = ParkingId;

	if ((ProcMode & NTNET_PASSUPDATE_STATUS) == NTNET_PASSUPDATE_STATUS) {
		SendNtnetDt.SData116.Status = Status;
		if (Status <= NTNET_PASSUPDATE_INVALID) {
			SendNtnetDt.SData116.UsingParkingNo = (ulong)CPrmSS[S_SYS][1];		// 精算機の基本駐車場№ set
		}
	}

	if ((ProcMode & NTNET_PASSUPDATE_OUTTIME) == NTNET_PASSUPDATE_OUTTIME) {
		if (ExitTime == NULL) {
			SendNtnetDt.SData116.OutYear	= 0;
			SendNtnetDt.SData116.OutMonth	= 0;
			SendNtnetDt.SData116.OutDay		= 0;
			SendNtnetDt.SData116.OutHour	= 0;
			SendNtnetDt.SData116.OutMin		= 0;
		} else {
			SendNtnetDt.SData116.OutYear	= ExitTime->Year;
			SendNtnetDt.SData116.OutMonth	= ExitTime->Mon;
			SendNtnetDt.SData116.OutDay		= ExitTime->Day;
			SendNtnetDt.SData116.OutHour	= ExitTime->Hour;
			SendNtnetDt.SData116.OutMin		= ExitTime->Min;
		}
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_116 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_116 ), NTNET_BUF_NORMAL );	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| モニタデータ(ﾃﾞｰﾀ種別122)作成処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data122                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data122( uchar kind, uchar code, uchar level, uchar *info, uchar *message )
{
	uchar	wks;
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_122 ) );

	BasicDataMake( 122, 0 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData122.Monsyu = kind;		// モニタ種別
	SendNtnetDt.SData122.Moncod = code;		// モニタコード
	SendNtnetDt.SData122.Monlev = level;	// モニタレベル
	if (info != NULL) {
		memcpy( &SendNtnetDt.SData122.Mondat1, info, sizeof(SendNtnetDt.SData122.Mondat1));
	}
	if (message != NULL) {
		memcpy( &SendNtnetDt.SData122.Mondat2, message, sizeof(SendNtnetDt.SData122.Mondat2));
	}
	
	wks = (uchar)prm_get(COM_PRM, S_NTN, 37, 1, 1);
	
	if ( wks != 9 && SendNtnetDt.SData122.Monlev >= wks ) {			// 親機送信ﾚﾍﾞﾙ
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_122 ));
		}
		else {
			NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_122 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
		}
	}

	/* 共通パラメータ39-0022の1の位にて送信可/不可チェック */
}

/*[]----------------------------------------------------------------------[]*/
/*| 操作モニタデータ(ﾃﾞｰﾀ種別123)作成処理                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data123                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data123( uchar kind, uchar code, uchar level, uchar *before, uchar *after, uchar *message )
{
	uchar	wks;
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_123 ) );

	BasicDataMake( 123, 0 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData123.OpeMonsyu = kind;	// 操作モニタ種別
	SendNtnetDt.SData123.OpeMoncod = code;	// 操作モニタコード
	SendNtnetDt.SData123.OpeMonlev = level;	// 操作モニタレベル
	memcpy( &SendNtnetDt.SData123.OpeMondat1, before, sizeof(SendNtnetDt.SData123.OpeMondat1));
	memcpy( &SendNtnetDt.SData123.OpeMondat2, after, sizeof(SendNtnetDt.SData123.OpeMondat2));
	if (message != NULL) {
		memcpy( &SendNtnetDt.SData123.OpeMondat3, message, sizeof(SendNtnetDt.SData123.OpeMondat3));
	}
	
	if( SendNtnetDt.SData123.OpeMonsyu == 80 ){
		if( SendNtnetDt.SData123.OpeMoncod == 86 ){								// 遠隔ダウンロード完了の操作モニタ
			if( *((ulong*)SendNtnetDt.SData123.OpeMondat1) > PROG_DL_RESET ){	// リセット発生関連のモニタ
				*((ulong*)SendNtnetDt.SData123.OpeMondat1) = PROG_DL_RESET;		// リセット（9）として電文は送信する
			}
		}
	}
	
	wks = (uchar)prm_get(COM_PRM, S_NTN, 37, 1, 2);
	
	if ( wks != 9 && SendNtnetDt.SData123.OpeMonlev >= wks ) {		// 親機送信ﾚﾍﾞﾙ
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_123 ));
		}
		else {
			NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_123 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
		}
	}

	/* 共通パラメータ39-0022の10の位にて送信可/不可チェック */
}

/*[]----------------------------------------------------------------------[]*/
/*| 共通設定ﾃﾞｰﾀ(ﾃﾞｰﾀ種別208)作成処理 - 1ｾｸｼｮﾝ送信                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data208                                       |*/
/*| PARAMETER    : mac   : 送信先端末機械№                                |*/
/*|              : sts   : ﾃﾞｰﾀｽﾃｰﾀｽ(0:中間 1:最終)                        |*/
/*|              : seg   : 設定ｾｸﾞﾒﾝﾄ(ｾｸｼｮﾝ)                               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data208(ulong mac, ushort sts, ushort seg)
{
	_NTNET_Snd_Data208(mac, sts, seg, 1, CPrmCnt[seg]);
}

/*[]----------------------------------------------------------------------[]*/
/*| 共通設定ﾃﾞｰﾀ(ﾃﾞｰﾀ種別208)作成処理 - ｱﾄﾞﾚｽ指定                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : _NTNET_Snd_Data208                                      |*/
/*| PARAMETER    : mac   : 送信先端末機械№                                |*/
/*|              : sts   : ﾃﾞｰﾀｽﾃｰﾀｽ(0:中間 1:最終)                        |*/
/*|              : seg   : 設定ｾｸﾞﾒﾝﾄ(ｾｸｼｮﾝ)                               |*/
/*|              : addr  : 開始ｱﾄﾞﾚｽ                                       |*/
/*|              : count : 設定ﾃﾞｰﾀ数                                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	_NTNET_Snd_Data208(ulong mac, ushort sts, ushort seg, ushort addr, ushort count)
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_80));

	BasicDataMake(208, 1);														// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData208.SMachineNo = mac;										// 送信先端末機械№
	SendNtnetDt.SData208.uMode = 0;												// 更新ﾓｰﾄﾞ
	SendNtnetDt.SData208.ModelCode = NTNET_MODEL_CODE;							// 端末機種コード
	SendNtnetDt.SData208.Status = sts;											// ﾃﾞｰﾀｽﾃｰﾀｽ
	SendNtnetDt.SData208.Segment = seg;											// 設定ｾｸﾞﾒﾝﾄ
	SendNtnetDt.SData208.TopAddr = addr;										// 開始ｱﾄﾞﾚｽ
	SendNtnetDt.SData208.DataCount = count;										// 設定ﾃﾞｰﾀ数
	memcpy(SendNtnetDt.SData208.Data, &CPrmSS[seg][addr], count*sizeof(long));	// 設定ﾃﾞｰﾀ

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_80 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_80), NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 共通設定ﾃﾞｰﾀ(ﾃﾞｰﾀ種別208)作成処理 - ｱﾄﾞﾚｽ指定                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Ans_Data208                                       |*/
/*| PARAMETER    : SysID : 送信先システムID                                |*/
/*|              : MachineNo : 送信先端末機械№                            |*/
/*|              : addr  : 開始ｱﾄﾞﾚｽ                                       |*/
/*|              : count : 設定ﾃﾞｰﾀ数                                      |*/
/*|              : okng  : 処理結果                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.10.1                                               |*/
/*| NOTE         : リパーク設定応答用                                      |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Ans_Data208(uchar SysID, ulong MachineNo, ushort addr, ushort count, uchar okng)
{
	RP_DATA_KIND_80	*msg = &SendNtnetDt.RP_SData208;
	ushort	seg, inaddr;
	ushort	i;
	long	tmp;
	long	lpwk,lpwk2;

	memset(msg, 0, sizeof(*msg));

	BasicDataMake( 208, 1 );										// 基本ﾃﾞｰﾀ作成
	msg->SMachineNo	= MachineNo;
	msg->ProcMode = okng;
	msg->Status = 1;
	msg->Segment = 0;
	msg->TopAddr = addr;
	msg->DataCount = count;		// <= RP_PARAMDATA_MAX　であること
	if (okng == 0) {
		for (i = 0; i < count; i++) {
			switch(addr) {
			case	412:	// A種
			case	414:	// B種
			case	416:	// C種
			case	418:	// D種
			case	420:	// E種
			case	422:	// F種
			case	424:	// G種
			case	426:	// H種
			case	428:	// I種
			case	430:	// J種
			case	432:	// K種
			case	434:	// L種
				msg->Data[i] = prm_get(COM_PRM, S_SHA, (short)(3+((addr%412)*3)), 4, 2);	// 2345:月－木
				break;
			case	413:	// A種
			case	415:	// B種
			case	417:	// C種
			case	419:	// D種
			case	421:	// E種
			case	423:	// F種
			case	425:	// G種
			case	427:	// H種
			case	429:	// I種
			case	431:	// J種
			case	433:	// K種
			case	435:	// L種
				tmp = prm_get(COM_PRM, S_SHA, (short)(3+((addr%413)*3)), 1, 1);		// 6:金
				tmp *= 10;
				tmp += prm_get(COM_PRM, S_SHA, (short)(4+((addr%413)*3)), 1, 6);		// 1:土
				tmp *= 10;
				tmp += prm_get(COM_PRM, S_SHA, (short)(3+((addr%413)*3)), 1, 6);		// 1:日
				tmp *= 10;
				tmp += prm_get(COM_PRM, S_SHA, (short)(4+((addr%413)*3)), 1, 5);		// 2:特
				msg->Data[i] = tmp;
				break;
			case	436:
			case	437:
			case	438:
				tmp = 0;
				inaddr = 4 + (addr-436)*24;
				for (seg = 0; seg < 4; seg++) {
					tmp *= 10;
					tmp += prm_get(COM_PRM, S_SHA, (short)inaddr, 1, 4);
					inaddr += 6;
				}
				msg->Data[i] = tmp;
				break;
// 4 -> 7 : ｎ時間ｍ回最大
// 5 -> 8 : 時間帯最大
			case	551:
				get_param_addr(0, addr, &seg, &inaddr);
				msg->Data[i] = CPrmSS[seg][inaddr];
				if( msg->Data[i] == 4 || msg->Data[i] == 10 ){
					msg->Data[i] = 7;
				}else if( msg->Data[i] == 5 ){
					msg->Data[i] = 8;
				}
				break;
// n時間最大 24 -> 2400 = 24:00
			case	558:
				get_param_addr(0, addr, &seg, &inaddr);
					msg->Data[i] = prm_get(COM_PRM, S_CAL, 38, 2, 3);
				msg->Data[i] *=100;
				break;
			case	559:
					msg->Data[i] = prm_get(COM_PRM, S_CAL, 38, 2, 1);
				break;
			case	472:
				break;
			case	5361:
				msg->Data[i] = prm_get(COM_PRM, S_PAY, 29, 1, 1);
				break;
			default:
				get_param_addr(0, addr, &seg, &inaddr);
				if( addr >= MAX_TYPE_SET_S && addr <= MAX_TYPE_SET_E){
				/* 種別毎の最大料金タイプの設定 */
				/* 最大料金タイプは設定値をFT4500用に変換(10→7  5→8)して送信する */
					lpwk = CPrmSS[seg][inaddr]/GETA_10_4;					// ①②を抽出
					if(lpwk == SP_MX_N_MH_NEW){								// ｎ時間ｍ回最大(１０)
						lpwk2 = FT4500_MX_N_HOUR*GETA_10_4;					// FT4500では７
					}
					else if(lpwk == SP_MX_BAND){							// 時間帯最大(５)
						lpwk2 = FT4500_MX_BAND*GETA_10_4;					// FT4500では８
					}
					else {
						lpwk2 = lpwk*GETA_10_4;
					}
					lpwk = (CPrmSS[seg][inaddr]%GETA_10_4)/GETA_10_2;		// ③④を抽出
					if(lpwk == SP_MX_N_MH_NEW){								// ｎ時間ｍ回最大(１０)
						lpwk2 += FT4500_MX_N_HOUR*GETA_10_2;				// FT4500では７
					}
					else if(lpwk == SP_MX_BAND){							// 時間帯最大(５)
						lpwk2 += FT4500_MX_BAND*GETA_10_2;					// FT4500では８
					}
					else {
						lpwk2 += lpwk*GETA_10_2;
					}
					lpwk = CPrmSS[seg][inaddr]%GETA_10_2;					// ⑤⑥を抽出
					if(lpwk == SP_MX_N_MH_NEW){
						lpwk2 += FT4500_MX_N_HOUR;
					}
					else if(lpwk == SP_MX_BAND){
						lpwk2 += FT4500_MX_BAND;
					}
					else {
						lpwk2 += lpwk;
					}
					msg->Data[i] = lpwk2;
					break;
				}
				msg->Data[i] = CPrmSS[seg][inaddr];
				break;
			}
			addr++;
		}
	}
	NTBUF_SetSendNtData(msg, (ushort)RP_DATA80_LEN, NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券精算中止データテーブル(ﾃﾞｰﾀ種別219)送信処理                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData219                                        |*/
/*| PARAMETER    : Mode ⇒ 0=1件更新, 1=1件削除, 2=全データ更新            |*/
/*|                Data ⇒ 送信データ(Modeが0or1のときのみ使用)            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data219(uchar Mode, struct TKI_CYUSI *Data)
{
	short i;
	
	if (_is_ntnet_remote()) {
		return;
	}
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_219));
	BasicDataMake(219, 1);														// 基本ﾃﾞｰﾀ作成
	SendNtnetDt.SData219.ProcMode = Mode;
	
	switch (Mode) {
	case 1:		// 1件削除
		ntnet_TeikiCyusiCnvCrmToNt(&SendNtnetDt.SData219.TeikiChusi[0], Data, TRUE);
		break;
	case 0:		// 1件更新
		ntnet_TeikiCyusiCnvCrmToNt(&SendNtnetDt.SData219.TeikiChusi[0], Data, TRUE);
		break;
	case 2:		// 全データ更新
		for (i = 0; i < tki_cyusi.count; i++) {
			ntnet_TeikiCyusiCnvCrmToNt(&SendNtnetDt.SData219.TeikiChusi[i], TKI_Get(NULL, i), TRUE);
		}
		break;
	default:
		return;
	}
	
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_219 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_219), NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別221)作成処理                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data221                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data221(void)
{
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_93));
	BasicDataMake(221, 1);														// 基本ﾃﾞｰﾀ作成
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	memcpy(SendNtnetDt.SData221.PassTable, pas_tbl, sizeof(pas_tbl));			// 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_93 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_93), NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券更新ﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別222)作成処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data222                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data222(void)
{
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_94));
	BasicDataMake(222, 1);														// 基本ﾃﾞｰﾀ作成
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	memcpy(SendNtnetDt.SData222.PassRenewal, pas_renewal, sizeof(pas_renewal));	// 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_94 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_94), NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券出庫時刻ﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別223)作成処理                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data223                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data223(void)
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	short	i;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)

	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_95));
	BasicDataMake(223, 1);																				// 基本ﾃﾞｰﾀ作成
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	/* 定期券出庫時刻ﾃｰﾌﾞﾙ */
//	for (i = 0; i < pas_extimtbl.Count; i++) {
//		SendNtnetDt.SData223.PassExTable[i] = pas_extimtbl.PassExTbl[i];
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_95 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_95), NTNET_BUF_NORMAL);							// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 車室パラメータ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data225
 *| PARAMETER    : MachineNo = 送信先端末機械No.
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2005-11-30
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data225( ulong MachineNo )
{

	short i;
	short j=0;
	if (_is_ntnet_remote()) {
		return;
	}

	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_225));
	BasicDataMake(225, 1);										// 基本ﾃﾞｰﾀ作成
	SendNtnetDt.SData225.ModelCode = NTNET_MODEL_CODE;			// 端末機種コード
	SendNtnetDt.SData225.SMachineNo	= MachineNo;				// 送信先端末機械№

	for (i = 0; i < LOCK_MAX; i++) {

		WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( !SetCarInfoSelect(i) ){
			continue;
		}
		if( j >= OLD_LOCK_MAX ){
			break;
		}
		SendNtnetDt.SData225.LockInfo[j].lok_syu	=	LockInfo[i].lok_syu;
		SendNtnetDt.SData225.LockInfo[j].ryo_syu	=	LockInfo[i].ryo_syu;
		SendNtnetDt.SData225.LockInfo[j].area		=	LockInfo[i].area;
		SendNtnetDt.SData225.LockInfo[j].posi		=	LockInfo[i].posi;
		SendNtnetDt.SData225.LockInfo[j].if_oya		=	LockInfo[i].if_oya;
		if( i < BIKE_START_INDEX ) {	// 駐車情報の場合、変換が必要
			SendNtnetDt.SData225.LockInfo[j].lok_no		=	LockInfo[i].lok_no + PRM_LOKNO_MIN ;	// 表示用に+100した値をわたす
		} else {													// 駐輪は従来通り
			SendNtnetDt.SData225.LockInfo[j].lok_no		=	LockInfo[i].lok_no;
		}
		j++;
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_225 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_225), NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
	}

}

/*[]----------------------------------------------------------------------[]*/
/*| ﾛｯｸ装置ﾊﾟﾗﾒｰﾀ(ﾃﾞｰﾀ種別226)作成処理                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data226                                       |*/
/*| PARAMETER    : MachineNo = 送信先端末機械No.                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data226( ulong MachineNo )
{
	int i;
	if (_is_ntnet_remote()) {
		return;
	}
	
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_226));
	BasicDataMake(226, 1);										// 基本ﾃﾞｰﾀ作成
	SendNtnetDt.SData226.ModelCode = NTNET_MODEL_CODE;			// 端末機種コード

	SendNtnetDt.SData226.SMachineNo	= MachineNo;				// 送信先端末機械№
	
	for (i = 0; i < 6; i++) {
		SendNtnetDt.SData226.LockMaker[i].in_tm		= LockMaker[i].in_tm;
		SendNtnetDt.SData226.LockMaker[i].ot_tm		= LockMaker[i].ot_tm;
		SendNtnetDt.SData226.LockMaker[i].r_cnt		= LockMaker[i].r_cnt;
		SendNtnetDt.SData226.LockMaker[i].r_tim		= LockMaker[i].r_tim;
		SendNtnetDt.SData226.LockMaker[i].open_tm	= LockMaker[i].open_tm;
		SendNtnetDt.SData226.LockMaker[i].clse_tm	= LockMaker[i].clse_tm;
	}
	
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_226 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_226), NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 設定データ要求受信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData78
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-28
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData78( void )
{
	RP_DATA_KIND_78	*msg = &RecvNtnetDt.RP_RData78;
	ushort	moncode;
	uchar	okng;

	NTNET_ClrSetup();

	okng = 0;
// match machine no.?
	if (msg->SMachineNo != (ulong)CPrmSS[S_PAY][2]) {
		okng = _RPKERR_INVALID_MACHINENO;
	}
// specified address is valid?
	else if (msg->Segment != 0 ||
			check_remote_addr(0, msg->TopAddr, msg->DataCount) == 0) {
		okng = _RPKERR_INVALID_PARA_ADDR;
	}
	else if (! check_acceptable()) {
		okng = _RPKERR_COMMAND_REJECT;
	}
// answer
	NTNET_Ans_Data208(msg->DataBasic.SystemID, msg->DataBasic.MachineNo,
							msg->TopAddr, msg->DataCount, okng);
// regist monitor
	moncode = OPMON_RSETUP_REFOK;
	if (okng != 0) {
		moncode++;
	}
	regist_mon(moncode, (ushort)okng, msg->TopAddr, 6);
}

/*[]----------------------------------------------------------------------[]*
 *| 設定データ送信要求処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData114
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-28
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData114( void )
{
	RP_DATA_KIND_114	*msg = &RecvNtnetDt.RData114;
	ushort	moncode;
	uchar	okng;

// check each request
	okng = 0;
	if (msg->reqSend == 1) {
		moncode = OPMON_RSETUP_PREOK;
	}
	else if (msg->reqSend == 2) {
		moncode = OPMON_RSETUP_CANOK;
	}
	else if (msg->reqExec == 1) {
		moncode = OPMON_RSETUP_EXEOK;
	}
	else {
		moncode = OPMON_RSETUP_NOREQ;
	}

// check parameter & condition
	if (moncode == OPMON_RSETUP_NOREQ) {
		okng = _RPKERR_COMMAND_REJECT;
	}
	else {
	// match machine no.?
		if (msg->SMachineNo != (ulong)CPrmSS[S_PAY][2]) {
			okng = _RPKERR_INVALID_MACHINENO;
			moncode++;		// NG code
		}
		else {
			switch(moncode) {
			case	OPMON_RSETUP_EXEOK:
				if (tmp_buffer.prepare == 0) {
					okng = _RPKERR_NO_PREPARE_COMMAND;
					moncode++;		// NG code
					break;
				}
			/*	not-break */
			case	OPMON_RSETUP_PREOK:
				if (! check_acceptable()) {
					okng = _RPKERR_COMMAND_REJECT;
					moncode++;		// NG code
					break;
				}
				break;
			default:
				break;
			}
		}
	}
// answer for request
	NTNET_Ans_Data115(msg->DataBasic.SystemID, msg->DataBasic.MachineNo,
								okng);

// exec that request & regist monitor
	if (okng == 0) {
		wmonlg(moncode, 0, 0);
		switch(moncode) {
		case	OPMON_RSETUP_PREOK:
			tmp_buffer.prepare = 1;
			tmp_buffer.bufcnt = 0;
			tmp_buffer.bufofs = 0;
			break;
		case	OPMON_RSETUP_CANOK:
			NTNET_ClrSetup();
			break;
		case	OPMON_RSETUP_EXEOK:
			update_param();
			NTNET_ClrSetup();
			DataSumUpdate(OPE_DTNUM_COMPARA);		/* update parameter sum on ram */
													// 個別ﾊﾟﾗﾒｰﾀはｺﾋﾟｰしない（AppServ_PParam_Copy = OFF）
			FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		/* FlashROM update */
			f_NTNET_RCV_MC10_EXEC = 1;				/* mc10()更新が必要な設定が通信で更新された情報set */
			wmonlg(OPMON_RSETUP_CHGOK, 0, 0);		// 変更終了OK
			SetSetDiff(SETDIFFLOG_SYU_REMSET);		// デフォルトセットログ登録処理
			break;
		}
	}
	else {
		NTNET_ClrSetup();					// cancel prepare command if error
		regist_mon(moncode, (ushort)okng, 0, 0);
	}
}

const ushort SwDspNum[5] = { 1, 0, 0, 2, 3 };	// 切替画面数
												// 0: 画面切替なし
												// 1: 未対応
												// 2: 未対応
												// 3: ２画面切替
												// 4: ３画面切替
/*[]----------------------------------------------------------------------[]*
 *| 現在台数管理データ受信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData234
 *| PARAMETER    : 
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData234(void)
{
	ushort i;
	PARK_NUM_CTRL_Sub*	pt;
	uchar addr;
	uchar stat = (uchar)prm_get(COM_PRM,S_SYS,39,1,2);

	if ((stat == 0) ||													// 駐車状況表示機能なし
		(prm_get(COM_PRM,S_SYS,39,1,1) != RecvNtnetDt.RData234.State))	// 用途別駐車台数設定≠共通設定
	{
		s_Is234StateValid = 0;
		return;
	}
	s_Is234StateValid = 1;

	if (SwDspNum[RecvNtnetDt.RData234.State]) {
		if (RecvNtnetDt.RData234.State == 0) {
			addr = 1;
		} else {
			addr = 5;
		}
		pt = (PARK_NUM_CTRL_Sub*)&PPrmSS[S_P02][addr];

		for (i = 0; i < SwDspNum[RecvNtnetDt.RData234.State]; i++) {
			switch (RecvNtnetDt.RData234.Data[i].CurNum) {			// 現在台数
			case 10000:				// 現在台数を＋１
				if (stat == 2) {
					pt->car_cnt++;
				}
				break;
			case 10001:				// 変更なし
				break;
			case 10002:				// 現在台数をー１
				if (pt->car_cnt && (stat == 2)) {
					pt->car_cnt--;
				}
				break;
			default:
				if (RecvNtnetDt.RData234.Data[i].CurNum <= 9999)
				{
					pt->car_cnt = RecvNtnetDt.RData234.Data[i].CurNum;
				}
				break;
			}

			switch (RecvNtnetDt.RData234.Data[i].NoFullNum) {			// 満車解除台数
			case 10001:				// 変更なし
				break;
			default:
				pt->kai_cnt = RecvNtnetDt.RData234.Data[i].NoFullNum;	// 満車解除台数
				break;
			}

			switch (RecvNtnetDt.RData234.Data[i].FullNum) {				// 満車台数
			case 10001:				// 変更なし
				break;
			default:
				pt->ful_cnt = RecvNtnetDt.RData234.Data[i].FullNum;		// 満車台数
				break;
			}
			pt++;
		}

		NTNET_Snd_Data99(RecvNtnetDt.RData234.DataBasic.MachineNo, NTNET_COMPLETE_PRKCTRL);	// NT-NET設定受信完了データ作成
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 現在台数管理データ受信判定処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_is234StateValid
 *[]----------------------------------------------------------------------[]*
 *| Date         : 2006-09-27
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	NTNET_is234StateValid(void)
{
	if (s_Is234StateValid) {
		// 用途別駐車台数設定＝共通設定
		return 1;
	} else {
		// 用途別駐車台数設定≠共通設定
		return 0;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 遠隔定期券ステータステーブル受信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData92
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData92(void)
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	ushort i;
//
//	/* 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ更新 */
//	switch (RecvNtnetDt.RData92.ProcMode) {
//	case 0:		// 全更新
//		memcpy(pas_tbl, RecvNtnetDt.RData92.PassTable, sizeof(pas_tbl));
//		break;
//	case 1:		// 有効無効ビット(b07)のみ更新
//		for (i = 0; i < PAS_MAX; i++) {
//			pas_tbl[i].BIT.INV = RecvNtnetDt.RData92.PassTable[i].BIT.INV;
//		}
//		break;
//	case 2:		// 入出庫ステータス(b0～b3)のみ更新
//		for (i = 0; i < PAS_MAX; i++) {
//			pas_tbl[i].BIT.STS = RecvNtnetDt.RData92.PassTable[i].BIT.STS;
//		}
//		break;
//	default:
//		break;
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
	wopelg(OPLOG_SET_STATUS_TBL_UPDATE, 0, 0);		// 操作履歴登録
}

/*[]----------------------------------------------------------------------[]*
 *| 遠隔制御データ受信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData240
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData240(void)
{
	ulong work;

	// グループNo  対応なし
	// ｱﾝﾁﾊﾟｽ切換
	if( RecvNtnetDt.RData240.ControlData[0] == 1 ){						// 自動?
		PPrmSS[S_P01][3] = 0L;											// ｱﾝﾁﾊﾟｽﾁｪｯｸ自動
		wopelg( OPLOG_SET_NYUSYUTUCHK_ON, 0, 0 );		// 操作履歴登録
	}
	else if( RecvNtnetDt.RData240.ControlData[0] == 2 ){				// 解除?
		PPrmSS[S_P01][3] = 1L;											// ｱﾝﾁﾊﾟｽﾁｪｯｸ強制解除
		wopelg( OPLOG_SET_NYUSYUTUCHK_OFF, 0, 0 );		// 操作履歴登録
	}

	// 満空車指定ﾓｰﾄﾞ 対応なし
	// 満空車制御番号 対応なし
	work = 0;
	// 満車制御１
	work |= (ulong)car_park_condition(1, RecvNtnetDt.RData240.ControlData[3]);
	// 満車制御２
	work |= (ulong)car_park_condition(2, RecvNtnetDt.RData240.ControlData[4]);
	// 満車制御３
	work |= (ulong)car_park_condition(3, RecvNtnetDt.RData240.ControlData[5]);
	if (work) {
	// どれかが変化したら
		wopelg(OPLOG_SET_MANSYACTRL, 0, 0);
	}
}

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
///*[]----------------------------------------------------------------------[]*
// *| 遠隔データ要求
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData243
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| Author       : 
// *| Date         : 2007-02-02
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//void	NTNET_RevData243(void)
//{
//	char i;
//	char req = 0;
//	ushort code;
//	
//	if (_is_ntnet_remote()) {
//		req = 0;
//		for (i = 0; i < 20; i++) {
//			if (RecvNtnetDt.RData243.ControlData[i]) {
//				req++;
//				code = i;
//			}
//		}
//		switch(code) {
//		case	0:	// 全車室情報テーブル要求
//		case	1:	// 駐車台数データ要求
//			break;
//		default:
//			req = 0;
//// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
//			break;
//// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
//		}
//		if (req != 1) {
//			// 管理データ要求NG送信
//			code = 0;	// 未接続として返送
//			NTNET_Snd_Data244(code);
//			return;
//		}
//	}
//
//	// 全車室情報テーブル要求
//	if (RecvNtnetDt.RData243.ControlData[0] != 0) {
//		memset(&RyoCalSim, 0, sizeof(RyoCalSim));
//		RyoCalSim.GroupNo = RecvNtnetDt.RData243.GroupNo;
//		queset(OPETCBNO, OPE_REQ_CALC_FEE, 0, NULL);
//	}
//	// 駐車台数データ要求
//	if (RecvNtnetDt.RData243.ControlData[1]) {
//		NTNET_Snd_Data238();
//	}
//}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

/*[]----------------------------------------------------------------------[]*
 *| 設定データ送信応答処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_SndData115
 *| PARAMETER    : 送信先端末機械№
 *|                処理区分(0: OK/ 1: NG)
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-28
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data115( ulong MachineNo, uchar ProcMode )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_115 ) );

	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( 115, 1 );
	
	SendNtnetDt.SData115.SMachineNo = MachineNo;
	SendNtnetDt.SData115.ProcMode   = ProcMode;
									
	// ﾃﾞｰﾀ送信登録
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_115 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_115 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定データ送信応答(ID115)作成処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data115                                       |*/
/*| PARAMETER    : SysID : 送信先システムID                                |*/
/*|              : MachineNo : 送信先端末機械№                            |*/
/*|                ng  : 0 = 送信OK/その他 = エラーコード                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.10.1                                               |*/
/*| NOTE         : リパーク設定応答用                                      |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Ans_Data115(uchar SysID, ulong MachineNo, uchar ng)
{
	RP_DATA_KIND_115	*msg = &SendNtnetDt.SData115;

	memset(msg, 0, sizeof(*msg));

	BasicDataMake( 115, 1 );										// 基本ﾃﾞｰﾀ作成
	msg->SMachineNo	= MachineNo;
	msg->ProcMode = ng;

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(*msg));
	}
	else {
		NTBUF_SetSendNtData(msg, sizeof(*msg), NTNET_BUF_PRIOR);	// ﾃﾞｰﾀ送信登録(優先データ）
	}
}

/*[]----------------------------------------------------------------------[]*
 *| コイン金庫小計データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data130
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data130( void )
{
	ntnet_Snd_CoinData(130, 41);
}

/*[]----------------------------------------------------------------------[]*
 *| コイン金庫合計データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data131
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-10-18
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data131( void )
{
	ntnet_Snd_CoinData(131, 51);
}

/*[]----------------------------------------------------------------------[]*
 *| コイン金庫集計データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : ntnet_Snd_CoinData
 *| PARAMETER    : kind : データ種別
 *|                type : 集計タイプ（小計：41/合計：51）
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-10-18
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ntnet_Snd_CoinData( uchar kind, ushort type )
{
	ushort i;
	uchar  keep;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_130 ) );

	// 基本ﾃﾞｰﾀ作成
	keep = 0;				// 保持する
	if( type == 41 ){		// 小計のとき
		keep = 1;			// 保持しない
	}
	BasicDataMake( kind, keep );

	if( type == 51 )		// 合計のとき
	{
		SendNtnetDt.SData130.KakariNo = OPECTL.Kakari_Num;		// 係員№
		SendNtnetDt.SData130.SeqNo = CountSel( &coin_syu.Oiban );			// 追番
	}

	// タイプ設定
	SendNtnetDt.SData130.Type = type;		// 小計(41)/合計(51)
	
	SendNtnetDt.SData130.Sf.Coin[0].Money = 10;						// ｺｲﾝ金庫金種(10円)
	SendNtnetDt.SData130.Sf.Coin[1].Money = 50;						// ｺｲﾝ金庫金種(50円)
	SendNtnetDt.SData130.Sf.Coin[2].Money = 100;					// ｺｲﾝ金庫金種(100円)
	SendNtnetDt.SData130.Sf.Coin[3].Money = 500;					// ｺｲﾝ金庫金種(500円)

	SendNtnetDt.SData130.Sf.Coin[0].Mai = SFV_DAT.safe_dt[0];		// ｺｲﾝ金庫枚数(10円)
	SendNtnetDt.SData130.Sf.Coin[1].Mai = SFV_DAT.safe_dt[1];		// ｺｲﾝ金庫枚数(50円)
	SendNtnetDt.SData130.Sf.Coin[2].Mai = SFV_DAT.safe_dt[2];		// ｺｲﾝ金庫枚数(100円)
	SendNtnetDt.SData130.Sf.Coin[3].Mai = SFV_DAT.safe_dt[3];		// ｺｲﾝ金庫枚数(500円)

	for( i=0; i<4; i++ ){
		SendNtnetDt.SData130.SfTotal +=								// ｺｲﾝ金庫総額
// MH322914 (s) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
//			SendNtnetDt.SData130.Sf.Coin[i].Mai * SendNtnetDt.SData130.Sf.Coin[i].Money;
			(ulong)SendNtnetDt.SData130.Sf.Coin[i].Mai * (ulong)SendNtnetDt.SData130.Sf.Coin[i].Money;
// MH322914 (e) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
	}
									
	// ﾃﾞｰﾀ送信登録
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_130 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_130 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 紙幣金庫集計データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data132
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data132( void )
{
	ntnet_Snd_NoteData(132, 42);
}

/*[]----------------------------------------------------------------------[]*
 *| 紙幣金庫集計データ送信処理(合計)
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data133
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-10-18
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data133( void )
{
	ntnet_Snd_NoteData(133, 52);
}

/*[]----------------------------------------------------------------------[]*
 *| 紙幣金庫集計データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : ntnet_Snd_NoteData
 *| PARAMETER    : kind : データ種別
 *|                type : 集計タイプ（小計：42/合計：52）
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-10-18
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ntnet_Snd_NoteData( uchar kind, ushort type )
{
	uchar  keep;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_132 ) );

	// 基本ﾃﾞｰﾀ作成
	keep = 0;				// 保持する
	if( type == 42 ){		// 小計のとき
		keep = 1;			// 保持しない
	}
	BasicDataMake( kind, keep );
	if( type == 52 )		// 合計のとき
	{
		SendNtnetDt.SData132.KakariNo = OPECTL.Kakari_Num;			// 係員№
		SendNtnetDt.SData132.SeqNo = CountSel ( &note_syu.Oiban );	// 追番
	}

	// タイプ設定
	SendNtnetDt.SData132.Type = type;		// 紙幣金庫小計(42)/合計(52)

	SendNtnetDt.SData132.Sf.Note[0].Money = 1000;					// 紙幣金庫金種(1000円)
	SendNtnetDt.SData132.Sf.Note[0].Mai = SFV_DAT.nt_safe_dt;		// 紙幣金庫枚数(1000円)
	SendNtnetDt.SData132.SfTotal =									// 紙幣金庫総額
// MH322914 (s) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
//		SendNtnetDt.SData132.Sf.Note[0].Mai * SendNtnetDt.SData132.Sf.Note[0].Money;
		(ulong)SendNtnetDt.SData132.Sf.Note[0].Mai * (ulong)SendNtnetDt.SData132.Sf.Note[0].Money;
// MH322914 (e) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
									
	// ﾃﾞｰﾀ送信登録
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_132 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_132 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 特別日設定データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data211
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : GT4700から流用
 *| Date         : 2006-09-28
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data211( void )
{
	if (_is_ntnet_remote()) {
		return;
	}
	NTNET_Snd_Data211_Exec();
}

void	NTNET_Snd_Data211_Exec( void )
{
	ushort	i, pos;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_211 ) );

	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( 211, 1 );										
	
	// 特別日設定ﾃﾞｰﾀ作成
	for( i=0; i<3; i++ ){														// 特別期間１～３
		SendNtnetDt.SData211.kikan[i].sta_Mont = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)(i*2+1), 2, 3 );			// 開始月
		SendNtnetDt.SData211.kikan[i].sta_Date = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)(i*2+1), 2, 1 );			// 開始日
		SendNtnetDt.SData211.kikan[i].end_Mont = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)(i*2+2), 2, 3 );			// 終了月
		SendNtnetDt.SData211.kikan[i].end_Date = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)(i*2+2), 2, 1 );			// 終了日
	}
    
	for( i=0, pos=NTNET_SPLDAY_START; pos<= NTNET_SPLDAY_END; i++, pos++ ){		// 特別日＆シフト１～３１
		SendNtnetDt.SData211.date[i].mont =
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 3 );				// 月
		SendNtnetDt.SData211.date[i].day = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 1 );				// 日
		SendNtnetDt.SData211.Shift[i] =
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 6 );				// シフト
	} 
	
	if (CPrmSS[S_TOK][41]) {
		for( i=0, pos=NTNET_SPYOBI_START; pos<= NTNET_SPYOBI_END; i++, pos++ ){	// 特別曜日１～１２
			SendNtnetDt.SData211.yobi[i].mont = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 3 );			// 月
			SendNtnetDt.SData211.yobi[i].week = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 2 );			// 週
			SendNtnetDt.SData211.yobi[i].yobi = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 1 );			// 曜日
		}
	}
	else {
		for( i=0, pos=NTNET_HMON_START; pos<= NTNET_HMON_END; i+=2, pos++ ){	// ハッピーマンデー１～１２
			SendNtnetDt.SData211.yobi[i].mont = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 5 );			// 月
			SendNtnetDt.SData211.yobi[i].week = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 4 );			// 週
			if( 0 != SendNtnetDt.SData211.yobi[i].mont )						// データあり
				SendNtnetDt.SData211.yobi[i].yobi = 1;							// 曜日（月曜日固定）

			SendNtnetDt.SData211.yobi[i+1].mont = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 2 );			// 月
			SendNtnetDt.SData211.yobi[i+1].week = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 1 );			// 週
			if( 0 != SendNtnetDt.SData211.yobi[i+1].mont )						// データあり
				SendNtnetDt.SData211.yobi[i+1].yobi = 1;						// 曜日（月曜日固定）
		}
	}

	SendNtnetDt.SData211.spl56 = (ushort)((CPrmSS[S_TOK][NTNET_5_6] & 0x01) ^ 0x01 );	// 5／6を特別日とする
	
	for( i=0, pos=NTNET_SPYEAR_START; pos<= NTNET_SPYEAR_END; i++, pos++ ){		// 特別年月日１～６
		SendNtnetDt.SData211.year[i].year = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 5 );				// 年
		if( SendNtnetDt.SData211.year[i].year != 0 ){
			SendNtnetDt.SData211.year[i].year += 2000;
		}
		SendNtnetDt.SData211.year[i].mont = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 3 );				// 月
		SendNtnetDt.SData211.year[i].day = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 1 );				// 日
	}

	// ﾃﾞｰﾀ送信登録
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_211 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_211 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 動作カウントデータ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data228
 *| PARAMETER    : MachineNo : 送信先端末機械№
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data228( ulong MachineNo )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_228 ) );

	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( 228, 1 );
	
	SendNtnetDt.SData228.MachineNo = MachineNo;
	memcpy(SendNtnetDt.SData228.Count, Mov_cnt_dat, (sizeof(ulong) * MOV_CNT_MAX));

	// ﾃﾞｰﾀ送信登録
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_228 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_228 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 駐車台数管理データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data235
 *| PARAMETER    : 
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data235(char bReq)
{
	if (_is_ntnet_remote()) {
		return;
	}
	NTNET_Snd_Data235_Exec(bReq);
}

void	NTNET_Snd_Data235_Exec( char bReq )
{
	ushort i = 0;
	PARK_NUM_CTRL_Sub*	pt;
	uchar addr;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_235 ) );

	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( 235, 1 );

	if ((prm_get(COM_PRM,S_SYS,39,1,2) == 0) ||
		((prm_get(COM_PRM,S_SYS,39,1,2) == 1) && (bReq == 1))) {
		// グループ以下、すべて0で送信
		SendNtnetDt.SData235.GroupNo = 0;
		SendNtnetDt.SData235.State   = 0;
		SendNtnetDt.SData235.Data[i].CurNum    = 0;
		SendNtnetDt.SData235.Data[i].NoFullNum = 0;
		SendNtnetDt.SData235.Data[i].FullNum   = 0;
	} else {
		SendNtnetDt.SData235.GroupNo = 0;
		SendNtnetDt.SData235.State   = prm_get(COM_PRM,S_SYS,39,1,1);		// 用途別駐車台数設定
		
		if (SwDspNum[SendNtnetDt.SData235.State]) {
			if (SendNtnetDt.SData235.State == 0) {
				addr = 1;
			} else {
				addr = 5;
			}
			pt = (PARK_NUM_CTRL_Sub*)&PPrmSS[S_P02][addr];
		
			for (i = 0; i < SwDspNum[SendNtnetDt.SData235.State]; i++) {
				SendNtnetDt.SData235.Data[i].CurNum    = pt->car_cnt;		// 現在台数
				SendNtnetDt.SData235.Data[i].NoFullNum = pt->kai_cnt;		// 満車解除台数
				SendNtnetDt.SData235.Data[i].FullNum   = pt->ful_cnt;		// 満車台数
				pt++;
			}
		}
	}

	// ﾃﾞｰﾀ送信登録
	switch( bReq ){
		case	0:
		case	1:	// 親機のみに送信
			if(_is_ntnet_remote()) {
				RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_235 ));
			}
			else {
				NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_235 ), NTNET_BUF_NORMAL );  // FT4800N
			}
			break;
		case	2:	// 親機及びMAFに送信
			if(_is_ntnet_remote()) {
				RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_235 ));
			}
			else {
			NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_235 ), NTNET_BUF_NORMAL );
			}
			// no break;
			break;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 駐車台数データ/駐車台数応答データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_DataParkCarNum
 *| PARAMETER    : uchar 236: / 238:
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-01-31
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_DataParkCarNum(uchar kind)
{
	int i, j;
	ulong curnum = 0;
	uchar uc_prm;
	ulong curnum_1 = 0;
	ulong curnum_2 = 0;
	ulong curnum_3 = 0;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_236 ) );

	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( kind, 1 );
	// ｸﾞﾙｰﾌﾟ番号
	SendNtnetDt.SData236.GroupNo = 0;
	for (i = 0; i < LOCK_MAX; i++) {
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		// 料金種別毎に現在在車ｶｳﾝﾄ
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {

				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );	// 種別毎ｶｳﾝﾄする設定get

				if( uc_prm == 1 ){			// 現在台数１をカウント
					curnum++;
					curnum_1++;
					SendNtnetDt.SData236.RyCurNum[j]++;
				}
				else if( uc_prm == 2 ){		// 現在台数２をカウント
					curnum++;
					curnum_2++;
					SendNtnetDt.SData236.RyCurNum[j]++;
				}
				else if( uc_prm == 3 ){		// 現在台数３をカウント
					curnum++;
					curnum_3++;
					SendNtnetDt.SData236.RyCurNum[j]++;
				}
			}
		}
	}
	// 現在駐車台数
	SendNtnetDt.SData236.CurNum = curnum;

	// 用途別駐車台数設定
	SendNtnetDt.SData236.State = prm_get(COM_PRM,S_SYS,39,1,1);
	if (_is_ntnet_remote()) {											// 遠隔NT-NET設定のときのみデータセット
		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {					// 01-0039⑤駐車台数を本体で管理する
			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
			case 0:														// 01-0039⑥用途別駐車台数切替なし
				SendNtnetDt.SData236.FullNo1 = PPrmSS[S_P02][3];		// 満車台数
				if (curnum > PPrmSS[S_P02][3]) {
					SendNtnetDt.SData236.EmptyNo1 = 0;					// 空車台数
				} else {
					SendNtnetDt.SData236.EmptyNo1 = PPrmSS[S_P02][3] - curnum;					// 空車台数(満車台数 - 現在駐車台数)
				}
				if( SendNtnetDt.SData236.EmptyNo1 == 0 ){		// 空車が「０」の場合
					SendNtnetDt.SData236.Full[0] = 1;			// 「満車」状態セット
				}
				switch (PPrmSS[S_P02][1]) {								// 強制満空車ﾓｰﾄﾞ
				case	1:												// 強制満車
					SendNtnetDt.SData236.Full[0] = 1;					// 「強制満車」状態セット
					break;
				case	2:												// 強制空車
					SendNtnetDt.SData236.Full[0] = 0;					// 「強制空車」状態セット
					break;
				default:												// 自動
					break;
				}
				break;

			case 3:														// 01-0039⑥用途別駐車台数２系統
			case 4:														// 01-0039⑥用途別駐車台数３系統
				SendNtnetDt.SData236.FullNo1 = PPrmSS[S_P02][7];		// 満車台数１
				if (curnum_1 > PPrmSS[S_P02][7]) {
					SendNtnetDt.SData236.EmptyNo1 = 0;					// 空車台数１
				} else {
					SendNtnetDt.SData236.EmptyNo1 = PPrmSS[S_P02][7] - curnum_1;				// 空車台数１(満車台数 - 現在駐車台数)
				}
				if( SendNtnetDt.SData236.EmptyNo1 == 0 ){		// 空車が「０」の場合
					SendNtnetDt.SData236.Full[0] = 1;			// 「満車」状態セット
				}
				switch (PPrmSS[S_P02][5]) {								// 強制満空車モード1
				case	1:												// 強制満車
					SendNtnetDt.SData236.Full[0] = 1;					// 「強制満車」状態セット
					break;
				case	2:												// 強制空車
					SendNtnetDt.SData236.Full[0] = 0;					// 「強制空車」状態セット
					break;
				default:												// 自動
					break;
				}

				SendNtnetDt.SData236.FullNo2 = PPrmSS[S_P02][11];		// 満車台数２
				if (curnum_2 > PPrmSS[S_P02][11]) {
					SendNtnetDt.SData236.EmptyNo2 = 0;					// 空車台数２
				} else {
					SendNtnetDt.SData236.EmptyNo2 = PPrmSS[S_P02][11] - curnum_2;				// 空車台数２(満車台数 - 現在駐車台数)
				}
				if( SendNtnetDt.SData236.EmptyNo2 == 0 ){		// 空車が「０」の場合
					SendNtnetDt.SData236.Full[1] = 1;			// 「満車」状態セット
				}
				switch (PPrmSS[S_P02][9]) {								// 強制満空車モード1
				case	1:												// 強制満車
					SendNtnetDt.SData236.Full[1] = 1;					// 「強制満車」状態セット
					break;
				case	2:												// 強制空車
					SendNtnetDt.SData236.Full[1] = 0;					// 「強制空車」状態セット
					break;
				default:												// 自動
					break;
				}

				if (i == 4) {
					SendNtnetDt.SData236.FullNo3 = PPrmSS[S_P02][15];	// 満車台数３
					if (curnum_3 > PPrmSS[S_P02][15]) {
						SendNtnetDt.SData236.EmptyNo3 = 0;				// 空車台数３
					} else {
						SendNtnetDt.SData236.EmptyNo3 = PPrmSS[S_P02][15] - curnum_3;			// 空車台数３(満車台数 - 現在駐車台数)
					}
					if( SendNtnetDt.SData236.EmptyNo3 == 0 ){		// 空車が「０」の場合
						SendNtnetDt.SData236.Full[2] = 1;			// 「満車」状態セット
					}
					switch (PPrmSS[S_P02][13]) {							// 強制満空車モード1
					case	1:												// 強制満車
						SendNtnetDt.SData236.Full[2] = 1;					// 「強制満車」状態セット
						break;
					case	2:												// 強制空車
						SendNtnetDt.SData236.Full[2] = 0;					// 「強制空車」状態セット
						break;
					default:												// 自動
						break;
					}
				}
				break;

			default:
				break;
			}
		}
		// ※現状"2"(ほぼ満車)は未使用
	}
	if(_is_ntnet_remote()) {
		if (kind != 236) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_236 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_236), NTNET_BUF_NORMAL);
	}
}
/*[]----------------------------------------------------------------------[]*
 *| 駐車台数データ送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data236
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-01-31
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data236(void)
{
	if( prm_get(COM_PRM,S_NTN,121,1,1)!=0 ) {
		memset(&SData58_bk, 0, sizeof(DATA_KIND_58));
		NTNET_Snd_DataParkCarNumWeb((uchar)58);
		return;
	}
	NTNET_Snd_DataParkCarNum((uchar)236);
}

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
///*[]----------------------------------------------------------------------[]*
// *| 駐車台数応答データ 送信処理
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_Snd_Data238
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| Author       : 
// *| Date         : 2006-02-02
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//void	NTNET_Snd_Data238(void)
//{
//	NTNET_Snd_DataParkCarNum((uchar)238);
//}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

/*[]----------------------------------------------------------------------[]*
 *| 管理データ要求NG 送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data105
 *| PARAMETER    : MachineNo:送信先端末機械№ / code:エラーコード
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data105(ulong MachineNo, ushort code)
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_105 ) );

	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( 105, 1 );

	SendNtnetDt.SData105.TermNo = MachineNo;
	SendNtnetDt.SData105.Result = code;

	// ﾃﾞｰﾀ送信登録
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_105 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_105), NTNET_BUF_NORMAL);
	}
}

/*[]----------------------------------------------------------------------[]*
 *| データ要求２結果NG 送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data110
 *| PARAMETER    : MachineNo:送信先端末機械№ / code:エラーコード
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data110(ulong MachineNo, ushort code)
{
	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( 110, 1 );

	SendNtnetDt.SData110.SMachineNo = MachineNo;
	memcpy(SendNtnetDt.SData110.ControlData, RecvNtnetDt.RData109.ControlData, 
			sizeof(RecvNtnetDt.RData109.ControlData));
	SendNtnetDt.SData110.Result = code;

	// 障害端末のターミナルNo01～32(all 0)
	memset(SendNtnetDt.SData110.ErrTerminal, 0, sizeof(SendNtnetDt.SData110.ErrTerminal));

	// ﾃﾞｰﾀ送信登録
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_110 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_110), NTNET_BUF_NORMAL);
	}
}
/*[]----------------------------------------------------------------------[]*
 *| センタ用データ要求結果送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data155
 *| PARAMETER    : MachineNo:送信先端末機械№ / code:エラーコード
 *| RETURN VALUE : void
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data155(ulong MachineNo, ushort code)
{
	// 基本ﾃﾞｰﾀ作成
	BasicDataMake( 155, 1 );

	SendNtnetDt.SData155.FmtRev = 10;							// フォーマットRev.No.(10固定)
	SendNtnetDt.SData155.SMachineNo = MachineNo;
	SendNtnetDt.SData155.SeisanJyoutai_Req = RecvNtnetDt.RData154.SeisanJyoutai_Req;
	SendNtnetDt.SData155.RealTime_Req = RecvNtnetDt.RData154.RealTime_Req;
	SendNtnetDt.SData155.Reserve1 = RecvNtnetDt.RData154.Reserve1;
	SendNtnetDt.SData155.Reserve2 = RecvNtnetDt.RData154.Reserve2;
	SendNtnetDt.SData155.Reserve3 = RecvNtnetDt.RData154.Reserve3;
	SendNtnetDt.SData155.Reserve4 = RecvNtnetDt.RData154.Reserve4;
	SendNtnetDt.SData155.Reserve5 = RecvNtnetDt.RData154.Reserve5;
	SendNtnetDt.SData155.Reserve6 = RecvNtnetDt.RData154.Reserve6;
	SendNtnetDt.SData155.SynchroTime_Req = RecvNtnetDt.RData154.SynchroTime_Req;
	SendNtnetDt.SData155.TermInfo_Req = RecvNtnetDt.RData154.TermInfo_Req;
	SendNtnetDt.SData155.Result = code;

	// 障害端末のターミナルNo01～32(all 0)
	memset(SendNtnetDt.SData155.ErrTerminal, 0, sizeof(SendNtnetDt.SData155.ErrTerminal));

	// ﾃﾞｰﾀ送信登録
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_155));
	} else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_155), NTNET_BUF_NORMAL);
	}
}
/*[]----------------------------------------------------------------------[]*
 *| 同期時刻設定要求 結果(ﾃﾞｰﾀ種別157)送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data157
 *| PARAMETER    : MachineNo:送信先端末機械№ / code:エラーコード
 *| RETURN VALUE : void
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data157(ulong MachineNo, ushort code , uchar setget )
{
	// 基本ﾃﾞｰﾀ作成
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_157 ) );
	BasicDataMake( 157, 1 );
	SendNtnetDt.SData157.FmtRev = 10;							// フォーマットRev.No.(10固定)
	SendNtnetDt.SData157.SMachineNo = MachineNo;
	SendNtnetDt.SData157.Reserve = 0;
	SendNtnetDt.SData157.Result = code;
	
	if( NTNET_SYNCHRO_GET == setget ){	// 同期時刻データの参照( NTNET_RevData154 )
		SendNtnetDt.SData157.Time_synchro_Req = 0;				// 0 = 変更なし
		SendNtnetDt.SData157.synchro_hour = (uchar)prm_get(COM_PRM, S_NTN, 122, 2, 3);
		SendNtnetDt.SData157.synchro_minu = (uchar)prm_get(COM_PRM, S_NTN, 122, 2, 1);
	} else {							// 同期時刻データの設定( NTNET_RevData156 )
		SendNtnetDt.SData157.Time_synchro_Req = ( ( code == 9 ) ? 0:1 );	// code = 9(失敗)の場合、0：変更なし、code = 0(成功)の場合、1：変更あり
		SendNtnetDt.SData157.synchro_hour = RecvNtnetDt.RData156.synchro_hour;
		SendNtnetDt.SData157.synchro_minu = RecvNtnetDt.RData156.synchro_minu;
	}

	// ﾃﾞｰﾀ送信登録
	RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_157));
}
/************************************/
/*									*/
/************************************/
uchar	NTNET_GetMostErrCode(const uchar ErrMdl)
{
	uchar	ErrCode = 0;
	ushort	i;
	ushort	GetLevel;
	uchar	ErrLevel= 0;

	for (i=0; i<ERR_NUM_MAX; i++) {
		GetLevel = GetErrorOccuerdLevel(ErrMdl, i);	// see -> IsErrorOccuerd()
		//	ErrLevelのﾁｪｯｸ
		if ((uchar)GetLevel > ErrLevel) {
			ErrLevel = (uchar)GetLevel;
			ErrCode = i;
		}
	}
	return(ErrCode);
}
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
///*[]----------------------------------------------------------------------[]*
// *| 精算情報データ送信処理
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//void	NTNET_Snd_Data152(void)
//{
//	ushort	i;
//	ushort	j;
//	ushort	num;
//	uchar	ErrCode;		// ERRMDL_COIM, ERRMDL_NOTE, ERRMDL_READER種別のｺｰﾄﾞ
//	EMONEY	t_EMoney;
//	CREINFO t_Credit;
//
//	ST_OIBAN w_oiban;
//	
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_152 ) );
//
//	/************************/
//	/*	基本ﾃﾞｰﾀ作成		*/
//	/************************/
//	BasicDataMake( 152, 1 );					// ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定
//
//	SendNtnetDt.SData152.FmtRev = 10;			// ﾌｫｰﾏｯﾄRev.№
//	/************************************/
//	/*	機器状態 = t_KikiStatus			*/		// NTNET_Edit_Data56_r10
//	/************************************/
//	if( opncls() == 1 ){	// 営業中
//		SendNtnetDt.SData152.Kiki.KyugyoStatus 	= 0;	// 機器状態_営休業	0:営業
//	} else {				// 休業中
//		SendNtnetDt.SData152.Kiki.KyugyoStatus 	= 1;	// 機器状態_営休業	1:休業
//	}
//	SendNtnetDt.SData152.Kiki.Lc1Status 		= 0;	// 機器状態_LC1状態	0:OFF/1:ON
//	SendNtnetDt.SData152.Kiki.Lc2Status 		= 0;	// 機器状態_LC2状態	0:OFF/1:ON
//	SendNtnetDt.SData152.Kiki.Lc3Status 		= 0;	// 機器状態_LC3状態	0:OFF/1:ON
//	SendNtnetDt.SData152.Kiki.GateStatus 		= 0;	// 機器状態_ゲート状態	0:閉/1:開
//	/********************************************************************/
//	/*	AlmAct[ALM_MOD_MAX][ALM_NUM_MAX]の[2][0]のﾃｰﾌﾞﾙ(02:券関連)より	*/
//	/*	00-99を対象に最後に発生したｱﾗｰﾑ									*/
//	/*	ALMMDL_SUB2		alm_chk2()	t_EAM_Act	AlmAct					*/
//	/********************************************************************/
//	SendNtnetDt.SData152.Kiki.NgCardRead 	= NgCard;	// NGカード読取内容	0＝なし, 1～255＝NT-NETのアラーム種別02のコード内容と同じ
//	/****************************************************************************************/
//	/*	発生中エラー = t_Erroring															*/
//	/*	ErrAct[ERR_MOD_MAX][ERR_NUM_MAX]													*/
//	/*	IsErrorOccuerd()関数にて当該のｴﾗｰ種別より00-99を対象に最上位の若番のｴﾗｰｺｰﾄﾞを設定	*/
//	/****************************************************************************************/
//	// ERRMDL_COIM		3.Coin
//	ErrCode = NTNET_GetMostErrCode(ERRMDL_COIM);
//	SendNtnetDt.SData152.Err.ErrCoinmech	= ErrCode;	// コインメック	エラー種別03のコード内容と同じ(レベルの高いものからセットする)
//	// ERRMDL_NOTE		4.Note
//	ErrCode = NTNET_GetMostErrCode(ERRMDL_NOTE);
//	SendNtnetDt.SData152.Err.ErrNoteReader	= ErrCode;	// 紙幣リーダー	エラー種別04のコード内容と同じ(レベルの高いものからセットする)
//	// ERRMDL_READER	1.Reader
//	ErrCode = NTNET_GetMostErrCode(ERRMDL_READER);
//	SendNtnetDt.SData152.Err.ErrGikiReader	= ErrCode;	// 磁気リーダー	エラー種別01のコード内容と同じ(レベルの高いものからセットする)
//	// 以下、FT-4000 では未対応
//	SendNtnetDt.SData152.Err.ErrNoteHarai	= 0;		// 紙幣払出機	エラー種別05のコード内容と同じ(レベルの高いものからセットする)
//	SendNtnetDt.SData152.Err.ErrCoinJyunkan	= 0;		// コイン循環	エラー種別06のコード内容と同じ(レベルの高いものからセットする)
//	SendNtnetDt.SData152.Err.ErrNoteJyunkan	= 0;		// 紙幣循環		エラー種別14のコード内容と同じ(レベルの高いものからセットする)
//	/************************************/
//	/*									*/
//	/************************************/
//	if( PayInfo_Class == 4 ){
//		// クレジット精算
//		SendNtnetDt.SData152.PayClass	= (uchar)0;					// 処理区分
//	}else{
//		SendNtnetDt.SData152.PayClass	= (uchar)PayInfo_Class;		// 処理区分
//	}
//	if(SendNtnetDt.SData152.PayClass == 8) {						// 精算前か
//		SendNtnetDt.SData152.MoneyInOut.MoneyKind_In = 0x1f;		// 金種有無	搭載金種の有無
//		SendNtnetDt.SData152.MoneyInOut.MoneyKind_Out = 0x1f;		// 金種有無	搭載金種の有無
//		SendNtnetDt.SData152.AntiPassCheck = 1;						// アンチパスチェック	1＝チェックOFF
//		goto LSendPayInfo;
//	}
//	else if(SendNtnetDt.SData152.PayClass == 100 ||					// 強制出庫
//			SendNtnetDt.SData152.PayClass == 101 ||					// 不正出庫
//			SendNtnetDt.SData152.PayClass == 102) {					// フラップ上昇前出庫
//		NTNET_Snd_Data152_SK();
//		goto LSendPayInfo;
//	}
//
//	w_oiban.w = ntNet_152_SaveData.Oiban.w;
//	w_oiban.i = ntNet_152_SaveData.Oiban.i;
//	SendNtnetDt.SData152.PayCount 	= CountSel( &w_oiban);	// 精算追番(0～99999)	syusei[LOCK_MAX]
//	SendNtnetDt.SData152.PayMethod 	= ntNet_152_SaveData.PayMethod;	// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
//	SendNtnetDt.SData152.PayMode 	= ntNet_152_SaveData.PayMode;	// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算/20＝Mifareプリペイド精算)
//	SendNtnetDt.SData152.CMachineNo = 0;							// 駐車券機械№(入庫機械№)	0～255
//
//	if(SendNtnetDt.SData152.PayClass == 9 ){								// 精算中
//		num = ryo_buf.pkiti - 1;
//		SendNtnetDt.SData152.FlapArea 	= (ushort)LockInfo[num].area;		// フラップシステム	区画		0～99
//		SendNtnetDt.SData152.FlapParkNo = (ushort)LockInfo[num].posi;		// 					車室番号	0～9999
//	}
//	else if(SendNtnetDt.SData152.PayMethod != 5 ){							// 定期券更新以外
//		SendNtnetDt.SData152.FlapArea 	= (ushort)(ntNet_152_SaveData.WPlace/10000);	// フラップシステム	区画		0～99
//		SendNtnetDt.SData152.FlapParkNo = (ushort)(ntNet_152_SaveData.WPlace%10000);	// 					車室番号	0～9999
//		SendNtnetDt.SData152.KakariNo 	= (ushort)ntNet_152_SaveData.KakariNo;			// 係員№	0～9999
//		SendNtnetDt.SData152.OutKind 	= ntNet_152_SaveData.OutKind;					// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
//																			//  	   10＝ゲート開放/20＝Mifareプリペイド精算出庫
//																			// 		   97＝ロック開・フラップ上昇前未精算出庫/98＝ラグタイム内出庫
//																			// 		   99＝サービスタイム内出庫
//	}
//	if(SendNtnetDt.SData152.PayClass == 9 ){								// 精算中
//		SendNtnetDt.SData152.CountSet 	= 1;								// 在車カウント 1＝しない
//	}
//	else {
//		SendNtnetDt.SData152.CountSet 	= ntNet_152_SaveData.CountSet;		// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
//	}
//	/********************************************/
//	/*	入庫/前回精算_YMDHMS = t_InPrevYMDHMS	*/
//	/********************************************/
//	if( SendNtnetDt.SData152.PayMethod != 5 ){					// 定期券更新以外
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Year		= (uchar)(ntNet_152_SaveData.carInTime.year%100);	// 入庫	年		00～99 2000～2099
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Mon		= ntNet_152_SaveData.carInTime.mon;					// 		月		01～12
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Day		= ntNet_152_SaveData.carInTime.day;					// 		日		01～31
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Hour		= ntNet_152_SaveData.carInTime.hour;				// 		時		00～23
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Min		= ntNet_152_SaveData.carInTime.min;					// 		分		00～59
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Sec		= 0;												// 		秒		00～59(磁気カードは0固定)
//		// 前回精算
//	}
//	/************************************/
//	/*									*/
//	/************************************/
//	SendNtnetDt.SData152.ReceiptIssue 	= ntNet_152_SaveData.ReceiptIssue;	// 領収証発行有無	0＝領収証なし/1＝領収証あり
//	if(SendNtnetDt.SData152.PayMethod != 5 ){					// 定期券更新以外
//		SendNtnetDt.SData152.Syubet 	= ntNet_152_SaveData.Syubet;	// 料金種別			1～
//		SendNtnetDt.SData152.Price 		= ntNet_152_SaveData.Price;		// 駐車料金			0～
//	}
//	SendNtnetDt.SData152.CashPrice 		= ntNet_152_SaveData.CashPrice;	// 現金売上			0～
//	SendNtnetDt.SData152.InPrice 		= ntNet_152_SaveData.InPrice;	// 投入金額			0～
//	SendNtnetDt.SData152.ChgPrice 		= (ushort)ntNet_152_SaveData.ChgPrice;	// 釣銭金額			0～9999
//
//	if(SendNtnetDt.SData152.PayClass == 9 ){									// 精算中
//		SendNtnetDt.SData152.Syubet 	= (char)(ryo_buf.syubet + 1);			// 料金種別			1～
//		if( ryo_buf.ryo_flg < 2 ) {												// 駐車券精算処理
//			SendNtnetDt.SData152.Price = ryo_buf.tyu_ryo;						// 駐車料金
//		}
//		else {
//			SendNtnetDt.SData152.Price = ryo_buf.tei_ryo;						// 定期料金
//		}
//		SendNtnetDt.SData152.InPrice 	= ryo_buf.nyukin;						// 投入金額			0～
//		SendNtnetDt.SData152.ChgPrice 	= (ushort)ryo_buf.turisen;				// 釣銭金額			0～9999
//	}
//	/****************************************************/
//	/*	金銭情報(投入金枚数/払出金枚数) = t_MoneyInOut	*/
//	/****************************************************/
//	SendNtnetDt.SData152.MoneyInOut.MoneyKind_In = 0x1f;						// 金種有無	搭載金種の有無
//																				//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
//																				//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
//	if(SendNtnetDt.SData152.PayClass == 0 || 									// 処理区分 精算済
//		SendNtnetDt.SData152.PayClass == 2 || 									// 処理区分 精算中止
//		SendNtnetDt.SData152.PayClass == 3) { 									// 処理区分 再精算中止
//		SendNtnetDt.SData152.MoneyInOut.In_10_cnt    = (uchar)ntNet_152_SaveData.in_coin[0];	// 投入金枚数(10円)		0～255
//		SendNtnetDt.SData152.MoneyInOut.In_50_cnt    = (uchar)ntNet_152_SaveData.in_coin[1];	// 投入金枚数(50円)		0～255
//		SendNtnetDt.SData152.MoneyInOut.In_100_cnt   = (uchar)ntNet_152_SaveData.in_coin[2];	// 投入金枚数(100円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_500_cnt   = (uchar)ntNet_152_SaveData.in_coin[3];	// 投入金枚数(500円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_1000_cnt  = (uchar)ntNet_152_SaveData.in_coin[4]	// 投入金枚数(1000円)	0～255
//														+ ntNet_152_SaveData.f_escrow;		// エスクロ紙幣返却時は+1する
//		SendNtnetDt.SData152.MoneyInOut.In_2000_cnt  = 0;							// 投入金枚数(2000円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_5000_cnt  = 0;							// 投入金枚数(5000円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_10000_cnt = 0;							// 投入金枚数(10000円)	0～255
//	}
//	else if(SendNtnetDt.SData152.PayClass == 9) {								// 処理区分 精算中
//		SendNtnetDt.SData152.MoneyInOut.In_10_cnt    = (uchar)ryo_buf.in_coin[0];	// 投入金枚数(10円)		0～255
//		SendNtnetDt.SData152.MoneyInOut.In_50_cnt    = (uchar)ryo_buf.in_coin[1];	// 投入金枚数(50円)		0～255
//		SendNtnetDt.SData152.MoneyInOut.In_100_cnt   = (uchar)ryo_buf.in_coin[2];	// 投入金枚数(100円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_500_cnt   = (uchar)ryo_buf.in_coin[3];	// 投入金枚数(500円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_1000_cnt  = (uchar)ryo_buf.in_coin[4];	// 投入金枚数(1000円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_2000_cnt  = 0;							// 投入金枚数(2000円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_5000_cnt  = 0;							// 投入金枚数(5000円)	0～255
//		SendNtnetDt.SData152.MoneyInOut.In_10000_cnt = 0;							// 投入金枚数(10000円)	0～255
//	}
//	SendNtnetDt.SData152.MoneyInOut.MoneyKind_Out = 0x1f;						// 金種有無	搭載金種の有無
//																				//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
//																				//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
//	if(SendNtnetDt.SData152.PayClass == 0 || 									// 処理区分 精算済
//		SendNtnetDt.SData152.PayClass == 2 || 									// 処理区分 精算中止
//		SendNtnetDt.SData152.PayClass == 3) { 									// 処理区分 再精算中止
//		SendNtnetDt.SData152.MoneyInOut.Out_10_cnt    = (uchar)ntNet_152_SaveData.out_coin[0];	// 払出金枚数(10円)		0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_50_cnt    = (uchar)ntNet_152_SaveData.out_coin[1];	// 払出金枚数(50円)		0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_100_cnt   = (uchar)ntNet_152_SaveData.out_coin[2];	// 払出金枚数(100円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_500_cnt   = (uchar)ntNet_152_SaveData.out_coin[3];	// 払出金枚数(500円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_1000_cnt  = ntNet_152_SaveData.f_escrow;			// 払出金枚数(1000円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_2000cnt   = 0;							// 払出金枚数(2000円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_5000_cnt  = 0;							// 払出金枚数(5000円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_10000_cnt = 0;							// 払出金枚数(10000円)	0～255(払い出し不足分は含まない)
//	}
//	else if(SendNtnetDt.SData152.PayClass == 9) {								// 処理区分 精算中
//		SendNtnetDt.SData152.MoneyInOut.Out_10_cnt    = (uchar)ryo_buf.out_coin[0]	// 払出金枚数(10円)		0～255(払い出し不足分は含まない)
//															+ (uchar)ryo_buf.out_coin[4];
//		SendNtnetDt.SData152.MoneyInOut.Out_50_cnt    = (uchar)ryo_buf.out_coin[1]	// 払出金枚数(50円)		0～255(払い出し不足分は含まない)
//															+ (uchar)ryo_buf.out_coin[5];
//		SendNtnetDt.SData152.MoneyInOut.Out_100_cnt   = (uchar)ryo_buf.out_coin[2]	// 払出金枚数(100円)	0～255(払い出し不足分は含まない)
//															+ (uchar)ryo_buf.out_coin[6];
//		SendNtnetDt.SData152.MoneyInOut.Out_500_cnt   = (uchar)ryo_buf.out_coin[3];	// 払出金枚数(500円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_1000_cnt  = 0;							// 払出金枚数(1000円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_2000cnt   = 0;							// 払出金枚数(2000円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_5000_cnt  = 0;							// 払出金枚数(5000円)	0～255(払い出し不足分は含まない)
//		SendNtnetDt.SData152.MoneyInOut.Out_10000_cnt = 0;							// 払出金枚数(10000円)	0～255(払い出し不足分は含まない)
//	}
//	/************************************/
//	/*									*/
//	/************************************/
//	SendNtnetDt.SData152.HaraiModoshiFusoku = (ushort)ntNet_152_SaveData.HaraiModoshiFusoku;			// 払戻不足額	0～9999
//	SendNtnetDt.SData152.Reserve1 = 0;											// 予備(サイズ調整用)	0
//	if( SendNtnetDt.SData152.PayClass != 8 &&									// 精算前以外
//		SendNtnetDt.SData152.PayClass != 9) {									// 精算中以外
//		SendNtnetDt.SData152.AntiPassCheck = ntNet_152_SaveData.AntiPassCheck;	// アンチパスチェック	0＝チェックON/1＝チェックOFF/2＝強制OFF
//	}
//	else {																		// 精算前or精算中
//		SendNtnetDt.SData152.AntiPassCheck = 1;									// アンチパスチェック	1＝チェックOFF
//	}
//																				//						※定期を使用しない場合は「１＝チェックOFF」固定
//	if( (SendNtnetDt.SData152.PayClass != 9 && ntNet_152_SaveData.ParkNoInPass) || 		// 精算中以外 定期券精算処理
//		(SendNtnetDt.SData152.PayClass == 9 && ryo_buf.ryo_flg >= 2) ) {		// 精算中 定期券精算処理
//		if(SendNtnetDt.SData152.PayClass != 9) {
//			SendNtnetDt.SData152.ParkNoInPass = ntNet_152_SaveData.ParkNoInPass;		// 定期券　駐車場№	0～999999
//		}
//		else {
//			SendNtnetDt.SData152.ParkNoInPass = CPrmSS[S_SYS][ntNet_152_SaveData.pkno_syu+1];	// 定期券　駐車場№	0～999999
//		}
//		/****************************************/
//		/*	精算媒体情報1, 2 = t_MediaInfo		*/
//		/****************************************/
//		SendNtnetDt.SData152.Media[0].MediaKind = (ushort)(ntNet_152_SaveData.pkno_syu + 2);	// 精算媒体情報１　種別(メイン媒体)	0～99
//		// カード番号[30]
//		intoasl(SendNtnetDt.SData152.Media[0].MediaCardNo, ntNet_152_SaveData.teiki_id, 5);				// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		// カード情報[16]
//		intoasl(SendNtnetDt.SData152.Media[0].MediaCardInfo, ntNet_152_SaveData.teiki_syu, 2);	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		// 精算媒体情報2（サブ媒体）は オール０とする
//	}
//	/************************************/
//	/*									*/
//	/************************************/
//	if( ntNet_152_SaveData.e_pay_kind != 0 ){									// 電子決済種別 Suica:1, Edy:2
//		SendNtnetDt.SData152.CardKind = (ushort)2;								// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	}else if( PayInfo_Class == 4 ){												// クレジット精算
//		SendNtnetDt.SData152.CardKind = (ushort)1;								// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	}
//
//	switch( SendNtnetDt.SData152.CardKind ){									// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	case 1:
//		SendNtnetDt.SData152.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// 暗号化方式
//		SendNtnetDt.SData152.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// 暗号鍵番号
//		memset( &t_Credit, 0, sizeof( t_Credit ) );
//		t_Credit.amount = (ulong)ntNet_152_SaveData.c_pay_ryo;					// 決済額
//		memcpyFlushLeft( &t_Credit.card_no[0], &ntNet_152_SaveData.c_Card_No[0], sizeof(t_Credit.card_no), sizeof(ntNet_152_SaveData.c_Card_No) );	// 会員番号
//		memcpyFlushLeft( &t_Credit.cct_num[0], &ntNet_152_SaveData.c_cct_num[0],
//							sizeof(t_Credit.cct_num), CREDIT_TERM_ID_NO_SIZE );	// 端末識別番号
//		memcpyFlushLeft( &t_Credit.kid_code[0], &ntNet_152_SaveData.c_kid_code[0], sizeof(t_Credit.kid_code), sizeof(ntNet_152_SaveData.c_kid_code) );	// KID コード
//		t_Credit.app_no = ntNet_152_SaveData.c_app_no;							// 承認番号
//		t_Credit.center_oiban = 0;												// ｾﾝﾀｰ処理追番
//		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );				// AES 暗号化
//		memcpy( (uchar *)&SendNtnetDt.SData152.settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// 決済情報
//		break;
//	case 2:
//		SendNtnetDt.SData152.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// 暗号化方式
//		SendNtnetDt.SData152.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// 暗号鍵番号
//		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//		t_EMoney.amount = ntNet_152_SaveData.e_pay_ryo;							// 電子決済精算情報　決済額
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//		memcpy( &t_EMoney.card_id[0], &ntNet_152_SaveData.e_Card_ID[0],
//				sizeof(ntNet_152_SaveData.e_Card_ID) );						// 電子決済精算情報　ｶｰﾄﾞID (Ascii 16桁)
//		t_EMoney.card_zangaku = ntNet_152_SaveData.e_pay_after;					// 電子決済精算情報　決済後残高
//		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );				// AES 暗号化
//		memcpy( (uchar *)&SendNtnetDt.SData152.settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// 決済情報
//		break;
//	}
//	/************************************/
//	/*	割引 = t_SeisanDiscount			*/
//	/************************************/
//	for( i = j = 0; i < WTIK_USEMAX; i++ ){								// 精算中止以外の割引情報コピー
//		if(( ntNet_152_SaveData.DiscountData[i].DiscSyu != 0 ) &&					// 割引種別あり
//		   (( ntNet_152_SaveData.DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// 精算中止割引情報でない
//		    ( NTNET_CFRE < ntNet_152_SaveData.DiscountData[i].DiscSyu ))){
//		    if( ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA以外(番号)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(支払額・残額)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO以外(番号)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(支払額・残額)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY以外	(番号)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(支払額・残額)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA以外(番号)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(支払額・残額)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&	// IC-Card以外(番号)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICCARD_2 ) 	//			(支払額・残額)
//		    {
//				SendNtnetDt.SData152.SDiscount[j].ParkingNo	= ntNet_152_SaveData.DiscountData[i].ParkingNo;		// 駐車場No.
//				SendNtnetDt.SData152.SDiscount[j].Kind		= ntNet_152_SaveData.DiscountData[i].DiscSyu;		// 割引種別
//				SendNtnetDt.SData152.SDiscount[j].Group		= ntNet_152_SaveData.DiscountData[i].DiscNo;		// 割引区分
//				SendNtnetDt.SData152.SDiscount[j].Callback	= ntNet_152_SaveData.DiscountData[i].DiscCount;		// 回収枚数
//				SendNtnetDt.SData152.SDiscount[j].Amount	= ntNet_152_SaveData.DiscountData[i].Discount;		// 割引額
//				SendNtnetDt.SData152.SDiscount[j].Info1		= ntNet_152_SaveData.DiscountData[i].DiscInfo1;		// 割引情報1
//				if( SendNtnetDt.SData152.SDiscount[j].Kind != NTNET_PRI_W ){
//					SendNtnetDt.SData152.SDiscount[j].Info2 = ntNet_152_SaveData.DiscountData[i].uDiscData.common.DiscInfo2;	// 割引情報2
//				}
//				j++;
//		    }
//		}
//	}
//	/********************/
//	/*	ﾃﾞｰﾀ送信登録	*/
//	/********************/
//LSendPayInfo:
//	if(_is_ntnet_remote()) {
//		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_152));
//	} else {
//		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_152), NTNET_BUF_NORMAL);
//	}
//}
//
///*[]----------------------------------------------------------------------[]*
// *| 精算情報データ不正出庫・強制出庫・フラップ上昇前送信処理
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//static	void	NTNET_Snd_Data152_SK(void)
//{
//	SendNtnetDt.SData152.PayClass 	= 0;							// 処理区分(0＝精算/1＝再精算/2＝精算中止/3＝再精算中止/(4＝クレジット精算※))
//	SendNtnetDt.SData152.PayCount 	= CountSel(&PayInfoData_SK.Oiban);	// 強制出庫or不正出庫追番orフラップ上昇前（精算追番）
//	SendNtnetDt.SData152.PayMethod 	= 0;							// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
//	SendNtnetDt.SData152.PayMode 	= 0;							// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算/20＝Mifareプリペイド精算)
//	SendNtnetDt.SData152.CMachineNo = 0;							// 駐車券機械№(入庫機械№)	0～255
//
//	SendNtnetDt.SData152.FlapArea 	= (ushort)(PayInfoData_SK.WPlace/10000);	// フラップシステム	区画		0～99
//	SendNtnetDt.SData152.FlapParkNo = (ushort)(PayInfoData_SK.WPlace%10000);	// 					車室番号	0～9999
//	SendNtnetDt.SData152.KakariNo 	= PayInfoData_SK.KakariNo;		// 係員№	0～9999
//	SendNtnetDt.SData152.OutKind 	= PayInfoData_SK.OutKind;		// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
//	SendNtnetDt.SData152.CountSet	= PayInfoData_SK.CountSet;		// 在車ｶｳﾝﾄ-1する
//
//	/********************************************/
//	/*	入庫/前回精算_YMDHMS = t_InPrevYMDHMS	*/
//	/********************************************/
//	if( SendNtnetDt.SData152.PayMethod != 5 ){					// 定期券更新以外
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Year	= 
//										(uchar)(PayInfoData_SK.TInTime.Year % 100);	// 入庫	年		00～99 2000～2099
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Mon	= PayInfoData_SK.TInTime.Mon;	// 		月		01～12
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Day	= PayInfoData_SK.TInTime.Day;	// 		日		01～31
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Hour	= PayInfoData_SK.TInTime.Hour;	// 		時		00～23
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Min	= PayInfoData_SK.TInTime.Min;	// 		分		00～59
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Sec	= 0;							// 		秒		00～59(磁気カードは0固定)
//	}
//
//	SendNtnetDt.SData152.Syubet 	= PayInfoData_SK.syu;					// 料金種別			1～
//	SendNtnetDt.SData152.Price 		= PayInfoData_SK.WPrice;				// 駐車料金			0～
//
//	SendNtnetDt.SData152.MoneyInOut.MoneyKind_In = 0x1f;					// 金種有無	搭載金種の有無
//																			//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
//																			//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
//	SendNtnetDt.SData152.MoneyInOut.MoneyKind_Out = 0x1f;					// 金種有無	搭載金種の有無
//																			//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
//																			//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
//	SendNtnetDt.SData152.AntiPassCheck = PayInfoData_SK.PassCheck;			// アンチパスチェック	0＝チェックON/1＝チェックOFF/2＝強制OFF
//																			//						※定期を使用しない場合は「１＝チェックOFF」固定
//
//}
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応

/*[]----------------------------------------------------------------------[]*
 *| リアルタイム情報送信処理
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data153(void)
{
	uchar	Kan0001_6;
	PARKCAR_DATA11	NowParkcarData;
	ushort	coinNowPoint;
	ushort	coinFullPoint;
	ushort	coinPerPoint;
	uchar	i;
	ushort	shu, mai;
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	uchar	f_update = 0;
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応

	memset( &NowParkcarData, 0, sizeof(PARKCAR_DATA11) );
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_153 ) );

	/************************/
	/*	基本ﾃﾞｰﾀ作成		*/
	/************************/
	BasicDataMake( 153, 1 );					// ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定

// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
//	SendNtnetDt.SData153.FmtRev = 10;			// ﾌｫｰﾏｯﾄRev.№	10固定 ---> Phase1の他のﾃﾞｰﾀの値は0固定でした
	SendNtnetDt.SData153.FmtRev = 11;			// ﾌｫｰﾏｯﾄRev.№	11固定
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	/************************************/
	/*	駐車台数 = PARKCAR_DATA11		*/
	/************************************/
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	NTNET_Snd_ParkCarNumDataMk(&SendNtnetDt.SData153.ParkData ,0);
	// 満車台数1～駐車3満空状態は0固定とする
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	/************************************/
	/*	精算機金銭情報 = t_MacMoneyInfo	*/
	/************************************/
	SendNtnetDt.SData153.MoneyInfo[0].ModelCode = NTNET_MODEL_CODE;				// 機種コード(001～999) 0=情報なし
	SendNtnetDt.SData153.MoneyInfo[0].MachineNo = prm_get(COM_PRM, S_PAY, 2, 2, 1);		// 機械№
	SendNtnetDt.SData153.MoneyInfo[0].Result 	= 0;							// 要求結果(0＝正常、1＝応答なし、2＝タイムアウト)
	SendNtnetDt.SData153.MoneyInfo[0].Uriage 	= sky.tsyuk.Uri_Tryo;			// 総売上額				0～999999
	SendNtnetDt.SData153.MoneyInfo[0].Cash 		= sky.tsyuk.Genuri_Tryo;		// 総現金売上額			0～999999
	
// MH322917(S) A.Iiizumi 2018/12/04 リアルタイム情報 フォーマットRev11対応
	if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する
		f_update = date_uriage_update_chk();
		if((f_update == DATE_URIAGE_CLR_TODAY)||(f_update == DATE_URIAGE_CLR_BEFORE)){
			// 日付は変わったがまだ当日集計されていない状態（前日の情報のまま）集計加算時に更新されるため判定が必要
			SendNtnetDt.SData153.MoneyInfo[0].Date_Uriage_use = 1;									// 日付切替時刻基準 使用有無 1＝使用
			// 日付切替時刻基準 総売上額      当日のデータはないため0をセット
			SendNtnetDt.SData153.MoneyInfo[0].Date_Uriage = 0;
			// 日付切替時刻基準 総現金売上額  当日のデータはないため0をセット
			SendNtnetDt.SData153.MoneyInfo[0].Date_Cash = 0;
			SendNtnetDt.SData153.MoneyInfo[0].Date_hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// 日付切替時刻基準 切替時 0～23
			SendNtnetDt.SData153.MoneyInfo[0].Date_min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// 日付切替時刻基準 切替分 0～12
		}else{
			SendNtnetDt.SData153.MoneyInfo[0].Date_Uriage_use = 1;									// 日付切替時刻基準 使用有無 1＝使用
			SendNtnetDt.SData153.MoneyInfo[0].Date_Uriage = Date_Syoukei.Uri_Tryo;					// 日付切替時刻基準 総売上額      0～999999
			SendNtnetDt.SData153.MoneyInfo[0].Date_Cash = Date_Syoukei.Genuri_Tryo;					// 日付切替時刻基準 総現金売上額  0～999999
			SendNtnetDt.SData153.MoneyInfo[0].Date_hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// 日付切替時刻基準 切替時 0～23
			SendNtnetDt.SData153.MoneyInfo[0].Date_min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// 日付切替時刻基準 切替分 0～12
		}
	}
// MH322917(E) A.Iiizumi 2018/12/04 リアルタイム情報 フォーマットRev11対応
	// 金庫入金総額加算
	SendNtnetDt.SData153.MoneyInfo[0].CoinTotal = (ulong)SFV_DAT.safe_dt[0] * 10L;
	SendNtnetDt.SData153.MoneyInfo[0].CoinTotal += (ulong)SFV_DAT.safe_dt[1] * 50L;
	SendNtnetDt.SData153.MoneyInfo[0].CoinTotal += (ulong)SFV_DAT.safe_dt[2] * 100L;
	SendNtnetDt.SData153.MoneyInfo[0].CoinTotal += (ulong)SFV_DAT.safe_dt[3] * 500L;
	
	SendNtnetDt.SData153.MoneyInfo[0].NoteTotal = (ulong)SFV_DAT.nt_safe_dt * 1000L;	// 紙幣金庫総入金額		0～999999

	// コイン金庫収納率(%)のセット
	coinNowPoint = (SFV_DAT.safe_dt[0] * prm_get(COM_PRM, S_MON, 20, 2, 1)) +
					(SFV_DAT.safe_dt[1] * prm_get(COM_PRM, S_MON, 21, 2, 1)) +
					(SFV_DAT.safe_dt[2] * prm_get(COM_PRM, S_MON, 22, 2, 1)) +
					(SFV_DAT.safe_dt[3] * prm_get(COM_PRM, S_MON, 23, 2, 1));
	coinFullPoint = prm_get(COM_PRM, S_MON, 27, 5, 1);
	if(coinNowPoint < coinFullPoint) {
		coinPerPoint = (coinNowPoint * 100) / coinFullPoint;
		SendNtnetDt.SData153.MoneyInfo[0].CoinReceive = coinPerPoint;
	}
	else {
		SendNtnetDt.SData153.MoneyInfo[0].CoinReceive = 100;
	}
	// 紙幣金庫収納枚数のセット
	SendNtnetDt.SData153.MoneyInfo[0].NoteReceive = SFV_DAT.nt_safe_dt;

	Kan0001_6 = (ushort)prm_get(COM_PRM, S_KAN, 1, 1, 1);		//	20_0001⑥
	if ((Kan0001_6 == 1) || (Kan0001_6 == 2)) {
		SendNtnetDt.SData153.MoneyInfo[0].KinsenKanriFlag = 0;	// 金銭管理あり
	} else {
		SendNtnetDt.SData153.MoneyInfo[0].KinsenKanriFlag = 1;	// 金銭管理なし
	}
	//	CNM_REC_REC   CN_RDAT	--->	釣銭切れ状態は1000円はなし
	if( CN_RDAT.r_dat09[0] == 0 ){								// 釣銭切れ状態 10円なし
		SendNtnetDt.SData153.MoneyInfo[0].TuriStatus |= 0x01;
	}
	if( CN_RDAT.r_dat09[1] == 0 ){								// 釣銭切れ状態 50円なし
		SendNtnetDt.SData153.MoneyInfo[0].TuriStatus |= 0x02;
	}
	if( CN_RDAT.r_dat09[2] == 0 ){								// 釣銭切れ状態 100円なし
		SendNtnetDt.SData153.MoneyInfo[0].TuriStatus |= 0x04;
	}
	if( CN_RDAT.r_dat09[3] == 0 ){								// 釣銭切れ状態 500円なし
		SendNtnetDt.SData153.MoneyInfo[0].TuriStatus |= 0x08;
	}

	SendNtnetDt.SData153.MoneyInfo[0].TuriMai_10 	= turi_kan.turi_dat[0].gen_mai;	// 10円保有枚数
	SendNtnetDt.SData153.MoneyInfo[0].TuriMai_50 	= turi_kan.turi_dat[1].gen_mai;	// 50円保有枚数
	SendNtnetDt.SData153.MoneyInfo[0].TuriMai_100	= turi_kan.turi_dat[2].gen_mai;	// 100円保有枚数
	SendNtnetDt.SData153.MoneyInfo[0].TuriMai_500 	= turi_kan.turi_dat[3].gen_mai;	// 500円保有枚数
	// 予蓄1, 2
	turikan_subtube_set();
	for( i=0; i<2; i++ ){
		switch( ((turi_kan.sub_tube >> (i*8)) & 0x000F) ){
		case 0x01:
			shu = 10;
			mai = turi_kan.turi_dat[0].ygen_mai;
			break;
		case 0x02:
			shu = 50;
			mai = turi_kan.turi_dat[1].ygen_mai;
			break;
		case 0x04:
			shu = 100;
			mai = turi_kan.turi_dat[2].ygen_mai;
			break;
		case 0:
		default:
			shu = 0;
			mai = 0;
			break;
		}
		if( i == 0 ){
			SendNtnetDt.SData153.MoneyInfo[0].Yochiku1_shu = shu;	// 金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
			SendNtnetDt.SData153.MoneyInfo[0].Yochiku1_mai = mai;	// 保有枚数	0～9999
		} else {
			SendNtnetDt.SData153.MoneyInfo[0].Yochiku2_shu = shu;	// 金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
			SendNtnetDt.SData153.MoneyInfo[0].Yochiku2_mai = mai;	// 保有枚数	0～9999
		}
	}
	// 予蓄3, 4, 5
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku3_shu = 0;		// 金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku3_mai = 0;		// 保有枚数	0～9999
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku4_shu = 0;		// 金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku4_mai = 0;		// 保有枚数	0～9999
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku5_shu = 0;		// 金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku5_mai = 0;		// 保有枚数	0～9999
	//	精算機２情報 - 精算機３２情報は0で送信する
	/********************/
	/*	ﾃﾞｰﾀ送信登録	*/
	/********************/
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_153));
	} else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_153), NTNET_BUF_NORMAL);
	}
}

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
///*[]----------------------------------------------------------------------[]*
// *| 遠隔データ要求応答NG 送信処理
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_Snd_Data244
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| Author       : 
// *| Date         : 2006-02-02
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//void	NTNET_Snd_Data244(ushort code)
//{
//	// 基本ﾃﾞｰﾀ作成
//	BasicDataMake( 244, 1 );
//
//	SendNtnetDt.SData244.GroupNo = RecvNtnetDt.RData243.GroupNo;
//	memcpy(SendNtnetDt.SData244.ControlData, RecvNtnetDt.RData243.ControlData, 
//			sizeof(RecvNtnetDt.RData243.ControlData));
//	SendNtnetDt.SData244.Result = code;
//
//	// 障害端末のターミナルNo01～32(all 0)
//	memset(SendNtnetDt.SData244.MachineNo, 0, sizeof(SendNtnetDt.SData244.MachineNo));
//
//	// ﾃﾞｰﾀ送信登録
//	if(_is_ntnet_remote()) {
//		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_244 ));
//	}
//	else {
//		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_244), NTNET_BUF_NORMAL);
//	}
//}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

/*[]----------------------------------------------------------------------[]*
 *| 全車室情報データ 送信処理
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data245
 *| PARAMETER    : GroupNo : グループNo
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data245(ushort GroupNo)
{
	short	i;
	RYO_INFO	*ryo_info;
	ushort	len = 0;

	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_245));

	BasicDataMake(245, 1);											// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData245.GroupNo = GroupNo;							// グループNo
	len = sizeof(DATA_BASIC) + 2;	// 基本データ＋グループNo
	// 有効な車室分ループする
	i = 0;
	ryo_info = &RyoCalSim.RyoInfo[0];
	while (ryo_info[i].pr_lokno) {
		WACDOG;																	// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行

		SendNtnetDt.SData245.LockState[i].LockNo = ryo_info[i].op_lokno;		// 区画情報
		SendNtnetDt.SData245.LockState[i].NowState = ryo_info[i].nstat;			// 現在ｽﾃｰﾀｽ
		SendNtnetDt.SData245.LockState[i].Year = ryo_info[i].indate.year;		// 入庫年
		SendNtnetDt.SData245.LockState[i].Mont = ryo_info[i].indate.mont;		// 入庫月
		SendNtnetDt.SData245.LockState[i].Date = ryo_info[i].indate.date;		// 入庫日
		SendNtnetDt.SData245.LockState[i].Hour = ryo_info[i].indate.hour;		// 入庫時
		SendNtnetDt.SData245.LockState[i].Minu = ryo_info[i].indate.minu;		// 入庫分
		SendNtnetDt.SData245.LockState[i].Syubet = ryo_info[i].kind;			// 料金種別
		SendNtnetDt.SData245.LockState[i].TyuRyo = ryo_info[i].fee;				// 駐車料金

		i++;
		len += sizeof(LOCK_STATE);
		if (i >= OLD_LOCK_MAX) {
			break;
		}
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, len, NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ハッピーマンデーチェック                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : chk_hpmonday                                            |*/
/*| PARAMETER    : date                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2006-09-28                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	chk_hpmonday(t_splday_yobi date)
{
	if ((date.mont < 1) || (date.mont > 12)) return FALSE;
	if ((date.week < 1) || (date.week > 5))  return FALSE;
	if (date.yobi != 1)                      return FALSE;
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*| T合計集計データ作成処理                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_TGOUKEI                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_TGOUKEI(void)
{
}

/*[]----------------------------------------------------------------------[]*/
/*| T小計集計データ作成処理                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_TSYOUKEI                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_TSYOUKEI(void)
{
	memset(&NTNetTTotalTime,0,sizeof(NTNetTTotalTime));
	//集計時刻を保存。ひとつの集計群で時刻が異ならないように修正。BasicDataMake()のみで使用される。
	NTNetTTotalTime.Year = CLK_REC.year;	// 年
	NTNetTTotalTime.Mon  = CLK_REC.mont;	// 月
	NTNetTTotalTime.Day  = CLK_REC.date;	// 日
	NTNetTTotalTime.Hour = CLK_REC.hour;	// 時
	NTNetTTotalTime.Min  = CLK_REC.minu;	// 分
	NTNetTTotalTime.Sec  = CLK_REC.seco;	// 秒

	_NTNET_Snd_Data158(&sky.tsyuk);
	_NTNET_Snd_Data159(&sky.tsyuk);
	_NTNET_Snd_Data160(&sky.tsyuk);
	_NTNET_Snd_Data161(&sky.tsyuk);
	_NTNET_Snd_Data162(&sky.tsyuk);
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	_NTNET_Snd_Data163(&loktl.tloktl);
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	_NTNET_Snd_Data164(&sky.tsyuk);
	_NTNET_Snd_Data169(&sky.tsyuk);
}

/*[]----------------------------------------------------------------------[]*/
/*| 集計基本データ作成処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiKihon                                   |*/
/*| PARAMETER    : syukei    : 集計データ(元データ)                        |*/
/*|                ID        : データID                                    |*/
/*|                Save      : データ保持フラグ                            |*/
/*|                Type      : 集計タイプ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiKihon(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_30 *data = &SendNtnetDt.SData30;
	
	memset(data, 0, sizeof(DATA_KIND_30));
	
	BasicDataMake(ID, Save);										// 基本ﾃﾞｰﾀ作成
	data->Type					= Type;								// 集計タイプ
	data->KakariNo				= syukei->Kakari_no;				// 係員No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// 集計追番
	ntnet_DateTimeCnv(&data->NowTime, &syukei->NowTime);			// 今回集計
	ntnet_DateTimeCnv(&data->LastTime, &syukei->OldTime);			// 前回集計
	data->SettleNum				= syukei->Seisan_Tcnt;				// 総精算回数
	data->Kakeuri				= syukei->Kakeuri_Tryo;				// 総掛売額
	data->Cash					= syukei->Genuri_Tryo;				// 総現金売上額
	data->Uriage				= syukei->Uri_Tryo;					// 総売上額
	data->Tax					= syukei->Tax_Tryo;					// 総消費税額
	data->Charge				= syukei->Turi_modosi_ryo;			// 釣銭払戻額
	data->CoinTotalNum			= syukei->Ckinko_goukei_cnt;		// コイン金庫合計回数
	data->NoteTotalNum			= syukei->Skinko_goukei_cnt;		// 紙幣金庫合計回数
	data->CyclicCoinTotalNum	= syukei->Junkan_goukei_cnt;		// 循環コイン合計回数
	data->NoteOutTotalNum		= syukei->Siheih_goukei_cnt;		// 紙幣払出機合計回数
	data->SettleNumServiceTime	= syukei->In_svst_seisan;			// サービスタイム内精算回数
	data->Shortage.Num			= syukei->Harai_husoku_cnt;			// 払出不足回数
	data->Shortage.Amount		= syukei->Harai_husoku_ryo;			// 払出不足金額
	data->Cancel.Num			= syukei->Seisan_chusi_cnt;			// 精算中止回数
	data->Cancel.Amount			= syukei->Seisan_chusi_ryo;			// 精算中止金額
	data->AntiPassOffSettle		= syukei->Apass_off_seisan;			// アンチパスOFF精算回数
	data->ReceiptIssue			= syukei->Ryosyuu_pri_cnt;			// 領収証発行枚数
	data->WarrantIssue			= syukei->Azukari_pri_cnt;			// 預り証発行枚数
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	data->AllSystem.CarOutIllegal.Num	= syukei->Husei_out_Tcnt;	// 全装置  不正出庫回数
//	data->AllSystem.CarOutIllegal.Amount= syukei->Husei_out_Tryo;	//                 金額
//	data->AllSystem.CarOutForce.Num		= syukei->Kyousei_out_Tcnt;	//         強制出庫回数
//	data->AllSystem.CarOutForce.Amount	= syukei->Kyousei_out_Tryo;	//                 金額
//	data->AllSystem.AcceptTicket		= syukei->Uketuke_pri_Tcnt;	//         受付券発行回数
//	data->AllSystem.ModifySettle.Num	= syukei->Syuusei_seisan_Tcnt;	//     修正精算回数
//	data->AllSystem.ModifySettle.Amount	= syukei->Syuusei_seisan_Tryo;	//             金額
//	data->CarInTotal			= syukei->In_car_Tcnt;				// 総入庫台数
//	data->CarOutTotal			= syukei->Out_car_Tcnt;				// 総出庫台数
//	data->CarIn1				= syukei->In_car_cnt[0];			// 入庫1入庫台数
//	data->CarOut1				= syukei->Out_car_cnt[0];			// 出庫1出庫台数
//	data->CarIn2				= syukei->In_car_cnt[1];			// 入庫2入庫台数
//	data->CarOut2				= syukei->Out_car_cnt[1];			// 出庫2出庫台数
//	data->CarIn3				= syukei->In_car_cnt[2];			// 入庫3入庫台数
//	data->CarOut3				= syukei->Out_car_cnt[2];			// 出庫3出庫台数
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	data->MiyoCount	= syukei->Syuusei_seisan_Mcnt;					// 未入金回数
	data->MiroMoney	= syukei->Syuusei_seisan_Mryo;					// 未入金額
	data->LagExtensionCnt		= syukei->Lag_extension_cnt;		// ラグタイム延長回数

	if(_is_ntnet_remote()) {
		if (ID != 30) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_30 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_30), NTNET_BUF_NORMAL);
	}
	/* 基本集計データ(ID30)をMAFに送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金種別毎集計データ作成処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiRyokinMai                               |*/
/*| PARAMETER    : syukei    : 集計データ(元データ)                        |*/
/*|                ID        : データID                                    |*/
/*|                Save      : データ保持フラグ                            |*/
/*|                Type      : 集計タイプ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiRyokinMai(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_31 *data = &SendNtnetDt.SData31;
	int i;
	int j;
	
	memset(data, 0, sizeof(DATA_KIND_31));
	
	BasicDataMake(ID, Save);										// 基本ﾃﾞｰﾀ作成
	data->Type					= Type;								// 集計タイプ
	data->KakariNo				= syukei->Kakari_no;				// 係員No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// 集計追番
	for (i = 0, j = 0; i < RYOUKIN_SYU_CNT; i++) {							// 種別01～50
		if (syukei->Rsei_cnt[i] == 0 && syukei->Rsei_ryo[i] == 0
		 && syukei->Rtwari_cnt[i] == 0 && syukei->Rtwari_ryo[i] == 0) {
		 	// データがすべて０の場合は電文に格納せずスキップする。
			continue;
		} else {
			data->Kind[j].Kind	= i + 1;								//            料金種別
			data->Kind[j].Settle.Num		= syukei->Rsei_cnt[i];		//            精算回数
			data->Kind[j].Settle.Amount		= syukei->Rsei_ryo[i];		//            売上額
			data->Kind[j].Discount.Num		= syukei->Rtwari_cnt[i];	//            割引回数
			data->Kind[j].Discount.Amount	= syukei->Rtwari_ryo[i];	//            割引額
			j++;
		}
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 31) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_31 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_31), NTNET_BUF_NORMAL);
	}
	/* 料金種別毎集計データ(ID31)をMAFに送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 分類集計データ作成処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiBunrui                                  |*/
/*| PARAMETER    : syukei    : 集計データ(元データ)                        |*/
/*|                ID        : データID                                    |*/
/*|                Save      : データ保持フラグ                            |*/
/*|                Type      : 集計タイプ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiBunrui(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	int i;
	DATA_KIND_32 *data = &SendNtnetDt.SData32;
	char	flg[3] = {0,0,0};
	char	j,cnt = 0;
	char	pram_set[] = {0,0,50,100};

	memset(data, 0, sizeof(DATA_KIND_32));
	
	BasicDataMake(ID, Save);										// 基本ﾃﾞｰﾀ作成
	data->Type					= Type;								// 集計タイプ
	data->KakariNo				= syukei->Kakari_no;				// 係員No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// 集計追番

/*分類集計１*/
	for(j = 1 ; j <= 3 ; j++){
		for(i = 1 ; i <= 6 ; i++){
			if(j == (uchar)prm_get(COM_PRM, S_BUN, 52, 1, (char)i)){
				flg[cnt] = j;
				cnt++;
				break;
			}
			if(j == (uchar)prm_get(COM_PRM, S_BUN, 53, 1, (char)i)){
				flg[cnt] = j;
				cnt++;
				break;
			}
		}
	}
	if(flg[0]){
		data->Kind					= CPrmSS[S_BUN][1]+pram_set[flg[0]];	// 駐車分類集計の種類
		for (i = 0; i < BUNRUI_CNT; i++) {									// 分類01～48
			data->Group[i].Num		= syukei->Bunrui1_cnt[flg[0]-1][i];		//            台数1
			data->Group[i].Amount	= syukei->Bunrui1_ryo[flg[0]-1][i];		//            台数2／金額
		}
		data->GroupTotal.Num		= syukei->Bunrui1_cnt1[flg[0]-1];		// 分類以上   台数1
		data->GroupTotal.Amount		= syukei->Bunrui1_ryo1[flg[0]-1];		//            台数2／金額
		data->Unknown.Num			= syukei->Bunrui1_cnt2[flg[0]-1];		// 分類不明   台数1
		data->Unknown.Amount		= syukei->Bunrui1_ryo2[flg[0]-1];		//            台数2／金額
		flg[0] = 0;
	}
/*分類集計２*/
	if(flg[1]){
		data->Kind2					= CPrmSS[S_BUN][1]+pram_set[flg[1]];	// 駐車分類集計の種類
		for (i = 0; i < BUNRUI_CNT; i++) {									// 分類01～48
			data->Group2[i].Num		= syukei->Bunrui1_cnt[flg[1]-1][i];		//            台数1
			data->Group2[i].Amount	= syukei->Bunrui1_ryo[flg[1]-1][i];		//            台数2／金額
		}
		data->GroupTotal2.Num		= syukei->Bunrui1_cnt1[flg[1]-1];		// 分類以上   台数1
		data->GroupTotal2.Amount		= syukei->Bunrui1_ryo1[flg[1]-1];	//            台数2／金額
		data->Unknown2.Num			= syukei->Bunrui1_cnt2[flg[1]-1];		// 分類不明   台数1
		data->Unknown2.Amount		= syukei->Bunrui1_ryo2[flg[1]-1];		//            台数2／金額
		flg[1] = 0;
	}
/*分類集計３*/
	if(flg[2]){
		data->Kind3					= CPrmSS[S_BUN][1]+pram_set[flg[2]];	// 駐車分類集計の種類
		for (i = 0; i < BUNRUI_CNT; i++) {									// 分類01～48
			data->Group3[i].Num		= syukei->Bunrui1_cnt[flg[2]-1][i];		//            台数1
			data->Group3[i].Amount	= syukei->Bunrui1_ryo[flg[2]-1][i];		//            台数2／金額
		}
		data->GroupTotal3.Num		= syukei->Bunrui1_cnt1[flg[2]-1];		// 分類以上   台数1
		data->GroupTotal3.Amount		= syukei->Bunrui1_ryo1[flg[2]-1];	//            台数2／金額
		data->Unknown3.Num			= syukei->Bunrui1_cnt2[flg[2]-1];		// 分類不明   台数1
		data->Unknown3.Amount		= syukei->Bunrui1_ryo2[flg[2]-1];		//            台数2／金額
		flg[2] = 0;
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 32) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_32 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_32), NTNET_BUF_NORMAL);
	}
	/* 分類集計データ(ID32)をMAFに送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 割引集計データ作成処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiWaribiki                                |*/
/*| PARAMETER    : syukei    : 集計データ(元データ)                        |*/
/*|                ID        : データID                                    |*/
/*|                Save      : データ保持フラグ                            |*/
/*|                Type      : 集計タイプ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiWaribiki(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_33 *data = &SendNtnetDt.SData33;
	int i, parking, group;

	wk_media_Type = 0;
	memset(data, 0, sizeof(DATA_KIND_33));
	
	BasicDataMake(ID, Save);										// 基本ﾃﾞｰﾀ作成
	data->Type					= Type;								// 集計タイプ
	data->KakariNo				= syukei->Kakari_no;				// 係員No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// 集計追番
	i = 0;
	
// クレジット
	if (syukei->Ccrd_sei_cnt != 0 || syukei->Ccrd_sei_ryo != 0) {

	    data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];				// 基本駐車場番号ｾｯﾄ

		data->Discount[i].Kind		= 30;
		data->Discount[i].Num		= syukei->Ccrd_sei_cnt;
		data->Discount[i].Amount	= syukei->Ccrd_sei_ryo;
		i++;
	}

// サービス券
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		for (group = 0; group < SERVICE_SYU_CNT; group++) {
			if (syukei->Stik_use_cnt[parking][group] == 0
			 && syukei->Stik_use_ryo[parking][group] == 0) {
				// データが０の場合は電文に格納せずスキップする。
				continue;
			} else {
				data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
				data->Discount[i].Kind		= 1;
				data->Discount[i].Group		= group + 1;
				data->Discount[i].Num		= syukei->Stik_use_cnt[parking][group];
				data->Discount[i].Amount	= syukei->Stik_use_ryo[parking][group];
				i++;
			}
		}
	}

// プリペイド
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Pcrd_use_cnt[parking] == 0
		 && syukei->Pcrd_use_ryo[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 11;
			data->Discount[i].Num		= syukei->Pcrd_use_cnt[parking];
			data->Discount[i].Amount	= syukei->Pcrd_use_ryo[parking];
			i++;
		}
	}

// 回数券
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Ktik_use_cnt[parking] == 0
		 && syukei->Ktik_use_ryo[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 5;
			data->Discount[i].Num		= syukei->Ktik_use_cnt[parking];
			data->Discount[i].Amount	= syukei->Ktik_use_ryo[parking];
			i++;
		}
	}

// 店割引 店No.毎
	for (parking = 0; parking < PKNO_WARI_CNT; parking++) {
		for (group = 0; group < MISE_NO_CNT; group++) {
			if (syukei->Mno_use_cnt4[parking][group] == 0
			 && syukei->Mno_use_ryo4[parking][group] == 0) {
				// データが０の場合は電文に格納せずスキップする。
				continue;
			} else {
				data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
				data->Discount[i].Kind		= 2;
				if( CPrmSS[S_TAT][1] == 1L ){
					data->Discount[i].Kind = 3;
					data->Discount[i].Info = ( (group + 1) > 100 ?
												CPrmSS[S_TAT][32+((group + 1)-101)]:CPrmSS[S_STO][3+3*group]);
				}
				data->Discount[i].Group		= group + 1;
				data->Discount[i].Num		= syukei->Mno_use_cnt4[parking][group];
				data->Discount[i].Amount	= syukei->Mno_use_ryo4[parking][group];
				i++;
			}
		}
	}

// 店割引 駐車場No.毎
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt1[parking] == 0
		 && syukei->Mno_use_ryo1[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 60;
			data->Discount[i].Num		= syukei->Mno_use_cnt1[parking];
			data->Discount[i].Amount	= syukei->Mno_use_ryo1[parking];
			i++;
		}
	}

// 店割引 1～100合計
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt2[parking] == 0
		 && syukei->Mno_use_ryo2[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 61;
			data->Discount[i].Num		= syukei->Mno_use_cnt2[parking];
			data->Discount[i].Amount	= syukei->Mno_use_ryo2[parking];
			i++;
		}
	}

// 店割引 101～999合計
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt3[parking] == 0
		 && syukei->Mno_use_ryo3[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 62;
			data->Discount[i].Num		= syukei->Mno_use_cnt3[parking];
			data->Discount[i].Amount	= syukei->Mno_use_ryo3[parking];
			i++;
		}
	}

// 店割引 全割引合計
	if (syukei->Mno_use_Tcnt != 0 || syukei->Mno_use_Tryo != 0) {

	    data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];				// 基本駐車場番号ｾｯﾄ
		data->Discount[i].Kind		= 63;
		data->Discount[i].Num		= syukei->Mno_use_Tcnt;
		data->Discount[i].Amount	= syukei->Mno_use_Tryo;
		i++;
	}

// 種別割引
	for (group = 0; group < RYOUKIN_SYU_CNT; group++) {
		if( syukei->Rtwari_cnt[group] != 0 ){
		data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];
		data->Discount[i].Kind		= 50;
		data->Discount[i].Group		= group + 1;
		data->Discount[i].Num		= syukei->Rtwari_cnt[group];
	  //data->Discount[i].Callback  = 0;
		data->Discount[i].Amount	= syukei->Rtwari_ryo[group];
	  //data->Discount[i].Info		= 0;
	  //data->Discount[i].Rsv		= 0;
		i++;
		}
	}

// Suica
	if( syukei->Electron_sei_cnt ){
		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
		data->Discount[i].Kind = NTNET_SUICA_1;							// 割引種別：31（Suica決済）固定
		data->Discount[i].Group = 0;									// 割引区分：未使用(0)
		data->Discount[i].Num = syukei->Electron_sei_cnt;				// 割引回数：
		data->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
		data->Discount[i].Amount = syukei->Electron_sei_ryo;			// 割引額  ：
		data->Discount[i].Info = 0;										// 割引情報：未使用(0)
		data->Discount[i].Rsv = 0;										// 予備　　：未使用(0)
		i++;
	}

	if( syukei->Electron_psm_cnt ){
// PASMO
		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
		data->Discount[i].Kind = NTNET_PASMO_0;							// 割引種別：33（PASMO決済）固定
		data->Discount[i].Group = 0;									// 割引区分：未使用(0)
		data->Discount[i].Num = syukei->Electron_psm_cnt;				// 割引回数：
		data->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
		data->Discount[i].Amount = syukei->Electron_psm_ryo;			// 割引額  ：
		data->Discount[i].Info = 0;										// 割引情報：未使用(0)
		data->Discount[i].Rsv = 0;										// 予備　　：未使用(0)
		i++;
	}

// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//	if( syukei->Electron_edy_cnt ){
//// Edy
//		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
//		data->Discount[i].Kind = NTNET_EDY_0;							// 割引種別：32（Edy決済）固定
//		data->Discount[i].Group = 0;									// 割引区分：(0)取引成功分
//		data->Discount[i].Num = syukei->Electron_edy_cnt;				// 割引回数：
//		data->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
//		data->Discount[i].Amount = syukei->Electron_edy_ryo;			// 割引額  ：
//		data->Discount[i].Info = 0;										// 割引情報：未使用(0)
//		data->Discount[i].Rsv = 0;										// 予備　　：未使用(0)
//
//		i++;
//	}
//
//	if( syukei->Electron_Arm_cnt ){
//		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
//		data->Discount[i].Kind = NTNET_EDY_0;							// 割引種別：32（Edy決済）固定
//		data->Discount[i].Group = 1;									// 割引区分：(1)ｱﾗｰﾑ取引分
//		data->Discount[i].Num = syukei->Electron_Arm_cnt;				// 割引回数：
//		data->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
//		data->Discount[i].Amount = syukei->Electron_Arm_ryo;			// 割引額  ：
//		data->Discount[i].Info = 0;										// 割引情報：未使用(0)
//		data->Discount[i].Rsv = 0;										// 予備　　：未使用(0)
//		i++;
//	}
//
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
	if( syukei->Electron_ico_cnt ){
// ICOCA
		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
		data->Discount[i].Kind = NTNET_ICOCA_0;							// 割引種別：35（ICOCA決済）固定
		data->Discount[i].Group = 0;									// 割引区分：(0)取引成功分
		data->Discount[i].Num = syukei->Electron_ico_cnt;				// 割引回数：
		data->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
		data->Discount[i].Amount = syukei->Electron_ico_ryo;			// 割引額  ：
		data->Discount[i].Info = 0;										// 割引情報：未使用(0)
		data->Discount[i].Rsv = 0;										// 予備　　：未使用(0)

		i++;
	}
	
	if( syukei->Electron_icd_cnt ){

// IC-Card
		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
		data->Discount[i].Kind = NTNET_ICCARD_0;						// 割引種別：36（IC-Card決済）固定
		data->Discount[i].Group = 0;									// 割引区分：(0)取引成功分
		data->Discount[i].Num = syukei->Electron_icd_cnt;				// 割引回数：
		data->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
		data->Discount[i].Amount = syukei->Electron_icd_ryo;			// 割引額  ：
		data->Discount[i].Info = 0;										// 割引情報：未使用(0)
		data->Discount[i].Rsv = 0;										// 予備　　：未使用(0)

		i++;
	}

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	if (syukei->Gengaku_seisan_cnt) {
//		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
//		data->Discount[i].Kind = NTNET_GENGAKU;							// 割引種別：減額精算
//		data->Discount[i].Group = 0;									// 割引区分：未使用(0)
//		data->Discount[i].Num = syukei->Gengaku_seisan_cnt;				// 割引回数：
//		data->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
//		data->Discount[i].Amount = syukei->Gengaku_seisan_ryo;			// 割引額  ：
//		data->Discount[i].Info = 0;										// 割引情報：未使用(0)
//		data->Discount[i].Rsv = 0;										// 予備　　：未使用(0)
//		i++;
//	}
//
//	if (syukei->Furikae_seisan_cnt) {
//		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
//		data->Discount[i].Kind = NTNET_FURIKAE;							// 割引種別：振替精算
//		data->Discount[i].Group = 0;									// 割引区分：未使用(0)
//		data->Discount[i].Num = syukei->Furikae_seisan_cnt;				// 割引回数：
//		data->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
//		data->Discount[i].Amount = syukei->Furikae_seisan_ryo;			// 割引額  ：
//		data->Discount[i].Info = 0;										// 割引情報：未使用(0)
//		data->Discount[i].Rsv = 0;										// 予備　　：未使用(0)
//		i++;
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	if(_is_ntnet_remote()) {
		if (ID != 33) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_33 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_33), NTNET_BUF_NORMAL);
	}
	/* 割引集計データ(ID33)をMAFに送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期集計データ作成処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiTeiki                                   |*/
/*| PARAMETER    : syukei    : 集計データ(元データ)                        |*/
/*|                ID        : データID                                    |*/
/*|                Save      : データ保持フラグ                            |*/
/*|                Type      : 集計タイプ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiTeiki(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_34 *data = &SendNtnetDt.SData34;
	int i, parking, kind;
	
	memset(data, 0, sizeof(DATA_KIND_34));
	
	BasicDataMake(ID, Save);										// 基本ﾃﾞｰﾀ作成
	data->Type					= Type;								// 集計タイプ
	data->KakariNo				= syukei->Kakari_no;				// 係員No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// 集計追番
	i = 0;
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		for (kind = 0; kind < TEIKI_SYU_CNT; kind++) {
			if ( ( syukei->Teiki_use_cnt[parking][kind] == 0 ) &&		// 定期使用回数が０の場合
				 ( syukei->Teiki_kou_cnt[parking][kind] == 0 ) ) {		// 定期更新回数が０の場合
				// データが０の場合は電文に格納せずスキップする。
				continue;
			} else {
				data->Pass[i].ParkingNo		= CPrmSS[S_SYS][1+parking];				// 駐車場No.
				data->Pass[i].Kind			= kind + 1;								// 種別
				data->Pass[i].Num			= syukei->Teiki_use_cnt[parking][kind];	// 回数
				data->Pass[i].Update.Num	= syukei->Teiki_kou_cnt[parking][kind];	// 更新回数
				data->Pass[i].Update.Amount	= syukei->Teiki_kou_ryo[parking][kind];	// 更新売上金額
				i++;
			}
		}
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 34) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_34 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_34), NTNET_BUF_NORMAL);
	}
	/* 定期集計データ(ID34)をMAFに送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 車室毎集計データ作成処理                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiShashitsuMai                            |*/
/*| PARAMETER    : syukei    : 集計データ(元データ)                        |*/
/*|                ID        : データID                                    |*/
/*|                Save      : データ保持フラグ                            |*/
/*|                Type      : 集計タイプ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiShashitsuMai(LOKTOTAL *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_35 *data = &SendNtnetDt.SData35;
	t_SyuSub_Lock	*dst;
	LOKTOTAL_DAT	*src;
	int i;
	int	cnt;
	ulong	posi;
	
	memset(data, 0, sizeof(DATA_KIND_35));
	
	BasicDataMake(ID, Save);										// 基本ﾃﾞｰﾀ作成
	data->Type					= Type;								// 集計タイプ
	data->KakariNo				= syukei->Kakari_no;				// 係員No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// 集計追番
	for (i = 0 , cnt = 0; i < LOCK_MAX; i++) {
		WACDOG;														// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if(LockInfo[i].lok_no != 0){								//接続有？
			if( !SetCarInfoSelect((short)i) ){
				continue;
			}
			if( cnt >= OLD_LOCK_MAX ){
				break;
			}
			posi = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	/* 駐車位置 		*/
			dst = &data->Lock[cnt];
			src = &syukei->loktldat[i];
			dst->LockNo					= posi;						// 区画情報
			dst->CashAmount				= src->Genuri_ryo;				// 現金売上
			dst->Settle					= src->Seisan_cnt;				// 精算回数
			dst->CarOutIllegal.Num		= src->Husei_out_cnt;			// 不正出庫回数
			dst->CarOutIllegal.Amount	= src->Husei_out_ryo;			//         金額
			dst->CarOutForce.Num		= src->Kyousei_out_cnt;			// 強制出庫回数
			dst->CarOutForce.Amount		= src->Kyousei_out_ryo;			//         金額
			dst->AcceptTicket			= src->Uketuke_pri_cnt;			// 受付券発行回数
			dst->ModifySettle.Num		= src->Syuusei_seisan_cnt;		// 修正精算回数
			dst->ModifySettle.Amount	= src->Syuusei_seisan_ryo;		//         金額
			cnt++;
		}
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 35) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_35 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_35), NTNET_BUF_NORMAL);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 金銭集計データ作成処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiKinsen                                  |*/
/*| PARAMETER    : syukei    : 集計データ(元データ)                        |*/
/*|                ID        : データID                                    |*/
/*|                Save      : データ保持フラグ                            |*/
/*|                Type      : 集計タイプ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiKinsen(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{

	DATA_KIND_36 *data = &SendNtnetDt.SData36;
	ulong	w;
	int		i;
	
	memset(data, 0, sizeof(DATA_KIND_36));
	
	BasicDataMake(ID, Save);										// 基本ﾃﾞｰﾀ作成
	data->Type					= Type;								// 集計タイプ
	data->KakariNo				= syukei->Kakari_no;				// 係員No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// 集計追番
	data->Total					= syukei->Kinko_Tryo;				// 金庫総入金額
	data->NoteTotal				= syukei->Note_Tryo;				// 紙幣金庫総入金額
	data->CoinTotal				= syukei->Coin_Tryo;				// コイン金庫総入金額
	for (i = 0; i < COIN_SYU_CNT; i++) {							// コイン1～4
		data->Coin[i].Kind		= c_coin[i];						//        金種
		data->Coin[i].Num		= syukei->Coin_cnt[i];				//        枚数
	}
	data->Note[0].Kind			= 1000;								// 紙幣1  金種
	data->Note[0].Num2			= syukei->Note_cnt[0];				//        紙幣枚数
	w = 0;
	for (i = 0; i < COIN_SYU_CNT; i++) {
		w += ((syukei->tou[i] + syukei->hoj[i]) * c_coin[i]);
	}
	data->CycleAccept			= w;								// 循環部総入金額
	w = 0;
	for (i = 0; i < COIN_SYU_CNT; i++) {
		w += ((syukei->sei[i] + syukei->kyo[i]) * c_coin[i]);
	}
	data->CyclePay				= w;								// 循環部総出金額
	data->NoteAcceptTotal		= syukei->tou[4] * 1000;			// 紙幣総入金額
	for (i = 0; i < _countof(data->Cycle); i++) {
		data->Cycle[i].CoinKind			= c_coin[i];				// 循環1～4 コイン金種
		data->Cycle[i].Accept			= syukei->tou[i];			//          入金枚数
		data->Cycle[i].Pay				= syukei->sei[i];			//          出金枚数
		data->Cycle[i].ChargeSupply		= syukei->hoj[i];			//          釣銭補充枚数
		data->Cycle[i].SlotInventory	= syukei->kyo[i];			//          インベントリ枚数(取出口)
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 36) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_36 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_36), NTNET_BUF_NORMAL);
	}
	/* 金銭集計データ(ID36)をMAFに送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 集計終了通知データ作成処理                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiSyuryo                                  |*/
/*| PARAMETER    : syukei    : 集計データ(元データ)                        |*/
/*|                ID        : データID                                    |*/
/*|                Save      : データ保持フラグ                            |*/
/*|                Type      : 集計タイプ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiSyuryo(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_41 *data = &SendNtnetDt.SData41;
	
	memset(data, 0, sizeof(DATA_KIND_41));
	
	BasicDataMake(ID, Save);										// 基本ﾃﾞｰﾀ作成
	data->Type					= Type;								// 集計タイプ
	data->KakariNo				= syukei->Kakari_no;				// 係員No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// 集計追番
	
	if(_is_ntnet_remote()) {
		if (ID != 41) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_41 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_41), NTNET_BUF_NORMAL);
	}
	/* 集計終了通知データ(ID41)をMAFに送信 */
}


/*[]----------------------------------------------------------------------[]*/
/*| 集計基本データ受信処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiKihon                                   |*/
/*| PARAMETER    : syukei : 集計データ					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ogura                                                   |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiKihon(SYUKEI *syukei)
{
	/* 複数集計データ受信待機中以外 */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no			= RecvNtnetDt.RData66.KakariNo;							// 係員No.
	syukei->Oiban.w				= 														// 集計追番
	syukei->Oiban.i				= RecvNtnetDt.RData66.SeqNo;							// 集計追番
	syukei->NowTime.Year		= RecvNtnetDt.RData66.NowTime.Year;						// 今回集計
	syukei->NowTime.Mon			= RecvNtnetDt.RData66.NowTime.Mon;						// 
	syukei->NowTime.Day			= RecvNtnetDt.RData66.NowTime.Day;						// 
	syukei->NowTime.Hour		= RecvNtnetDt.RData66.NowTime.Hour;						// 
	syukei->NowTime.Min			= RecvNtnetDt.RData66.NowTime.Min;						// 
	syukei->OldTime.Year		= RecvNtnetDt.RData66.LastTime.Year;					// 前回集計;
	syukei->OldTime.Mon			= RecvNtnetDt.RData66.LastTime.Mon;						// 
	syukei->OldTime.Day			= RecvNtnetDt.RData66.LastTime.Day;						// 
	syukei->OldTime.Hour		= RecvNtnetDt.RData66.LastTime.Hour;					// 
	syukei->OldTime.Min			= RecvNtnetDt.RData66.LastTime.Min;						// 
	syukei->Seisan_Tcnt			= RecvNtnetDt.RData66.SettleNum;						// 総精算回数m;
	syukei->Kakeuri_Tryo		= RecvNtnetDt.RData66.Kakeuri;							// 総掛売額
	syukei->Genuri_Tryo			= RecvNtnetDt.RData66.Cash;   							// 総現金売上額
	syukei->Uri_Tryo			= RecvNtnetDt.RData66.Uriage; 							// 総売上額
	syukei->Tax_Tryo			= RecvNtnetDt.RData66.Tax;    							// 総消費税額
	syukei->Turi_modosi_ryo		= RecvNtnetDt.RData66.Charge; 							// 釣銭払戻額
	syukei->Ckinko_goukei_cnt	= RecvNtnetDt.RData66.CoinTotalNum;						// コイン金庫合計回数
	syukei->Skinko_goukei_cnt	= RecvNtnetDt.RData66.NoteTotalNum;						// 紙幣金庫合計回数
	syukei->Junkan_goukei_cnt	= RecvNtnetDt.RData66.CyclicCoinTotalNum;				// 循環コイン合計回数
	syukei->Siheih_goukei_cnt	= RecvNtnetDt.RData66.NoteOutTotalNum;					// 紙幣払出機合計回数
	syukei->In_svst_seisan		= RecvNtnetDt.RData66.SettleNumServiceTime;				// サービスタイム内精算回数
	syukei->Harai_husoku_cnt	= RecvNtnetDt.RData66.Shortage.Num;						// 払出不足回数
	syukei->Harai_husoku_ryo	= RecvNtnetDt.RData66.Shortage.Amount;					// 払出不足金額
	syukei->Seisan_chusi_cnt	= RecvNtnetDt.RData66.Cancel.Num;						// 精算中止回数
	syukei->Seisan_chusi_ryo	= RecvNtnetDt.RData66.Cancel.Amount;					// 精算中止金額
	syukei->Apass_off_seisan	= RecvNtnetDt.RData66.AntiPassOffSettle;				// アンチパスOFF精算回数
	syukei->Ryosyuu_pri_cnt		= RecvNtnetDt.RData66.ReceiptIssue;						// 領収証発行枚数
	syukei->Azukari_pri_cnt		= RecvNtnetDt.RData66.WarrantIssue;						// 預り証発行枚数
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	syukei->Husei_out_Tcnt		= RecvNtnetDt.RData66.AllSystem.CarOutIllegal.Num;		// 全装置 不正出庫 回数
//	syukei->Husei_out_Tryo		= RecvNtnetDt.RData66.AllSystem.CarOutIllegal.Amount;	//                 金額
//	syukei->Kyousei_out_Tcnt	= RecvNtnetDt.RData66.AllSystem.CarOutForce.Num;		//        強制出庫 回数
//	syukei->Kyousei_out_Tryo	= RecvNtnetDt.RData66.AllSystem.CarOutForce.Amount;		//                 金額
//	syukei->Uketuke_pri_Tcnt	= RecvNtnetDt.RData66.AllSystem.AcceptTicket;			//        受付券発行回数
//	syukei->Syuusei_seisan_Tcnt	= RecvNtnetDt.RData66.AllSystem.ModifySettle.Num;		//        修正精算 回数
//	syukei->Syuusei_seisan_Tryo	= RecvNtnetDt.RData66.AllSystem.ModifySettle.Amount;	//                 金額
//	syukei->In_car_Tcnt			= RecvNtnetDt.RData66.CarInTotal;						// 総入庫台数
//	syukei->Out_car_Tcnt		= RecvNtnetDt.RData66.CarOutTotal;						// 総出庫台数
//	syukei->In_car_cnt[0]		= RecvNtnetDt.RData66.CarIn1;							// 入庫1入庫台数
//	syukei->Out_car_cnt[0]		= RecvNtnetDt.RData66.CarOut1;							// 出庫1出庫台数1;
//	syukei->In_car_cnt[1]		= RecvNtnetDt.RData66.CarIn2;							// 入庫2入庫台数
//	syukei->Out_car_cnt[1]		= RecvNtnetDt.RData66.CarOut2;							// 出庫2出庫台数2;
//	syukei->In_car_cnt[2]		= RecvNtnetDt.RData66.CarIn3;							// 入庫3入庫台数
//	syukei->Out_car_cnt[2]		= RecvNtnetDt.RData66.CarOut3;							// 出庫3出庫台数3;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	syukei->Lag_extension_cnt	= RecvNtnetDt.RData66.LagExtensionCnt;					// ラグタイム延長回数
	
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金種別毎集計データ受信処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiRyokinMai                               |*/
/*| PARAMETER    : syukei : 集計データ					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiRyokinMai(SYUKEI *syukei)
{
	ushort	i, kind;
	
	/* 複数集計データ受信待機中以外 */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no	= RecvNtnetDt.RData67.KakariNo;								// 係員No.
	syukei->Oiban.w		= 															// 集計追番
	syukei->Oiban.i		= RecvNtnetDt.RData67.SeqNo;								// 集計追番
	
	for ( i = 0; i < _countof( RecvNtnetDt.RData67.Kind ); i++) {					// 種別 01～50
		kind = RecvNtnetDt.RData67.Kind[i].Kind;
		if ( kind >= 1 && kind <= RYOUKIN_SYU_CNT ){								// 料金種別数 1～12
			kind--;
			syukei->Rsei_cnt[kind]		= RecvNtnetDt.RData67.Kind[i].Settle.Num;		// 精算回数
			syukei->Rsei_ryo[kind]		= RecvNtnetDt.RData67.Kind[i].Settle.Amount;	// 売上額
			syukei->Rtwari_cnt[kind]	= RecvNtnetDt.RData67.Kind[i].Discount.Num;		// 割引回数
			syukei->Rtwari_ryo[kind]	= RecvNtnetDt.RData67.Kind[i].Discount.Amount;	// 割引額
		}
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| 分類集計データ受信処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiBunrui                                  |*/
/*| PARAMETER    : syukei : 集計データ					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiBunrui(SYUKEI *syukei)
{
	uchar i;
	uchar	bn_syu;
	uchar	tyu_syu;
	/* 複数集計データ受信待機中以外 */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no	= RecvNtnetDt.RData68.KakariNo;						// 係員No.
	syukei->Oiban.w		= 													// 集計追番
	syukei->Oiban.i		= RecvNtnetDt.RData68.SeqNo;						// 集計追番

	/*分類集計１*/
	bn_syu = 0xff;
	tyu_syu= 0xff;
	if(( RecvNtnetDt.RData68.Kind >= 1 ) && ( RecvNtnetDt.RData68.Kind <= 7 )){
		bn_syu = 0;
		tyu_syu = (uchar)RecvNtnetDt.RData68.Kind;//集計種類
	}else if(( RecvNtnetDt.RData68.Kind >= 51 ) && ( RecvNtnetDt.RData68.Kind <= 57 )){
		bn_syu = 1;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind - 50);//集計種類
	}else if(( RecvNtnetDt.RData68.Kind >= 101 ) && ( RecvNtnetDt.RData68.Kind <= 107 )){
		bn_syu = 2;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind - 100);//集計種類
	}
	if(( tyu_syu == CPrmSS[S_BUN][1] )&&( bn_syu != 0xff )){
		for (i = 0; i < BUNRUI_CNT; i++) {										// 分類01～48
			syukei->Bunrui1_cnt[bn_syu][i] = RecvNtnetDt.RData68.Group[i].Num;		//           台数1
			syukei->Bunrui1_ryo[bn_syu][i] = RecvNtnetDt.RData68.Group[i].Amount;	//           台数2／金額
		}
		syukei->Bunrui1_cnt1[bn_syu] = RecvNtnetDt.RData68.GroupTotal.Num;			// 分類以上　台数
		syukei->Bunrui1_ryo1[bn_syu] = RecvNtnetDt.RData68.GroupTotal.Amount;		//           台数2／金額
		syukei->Bunrui1_cnt2[bn_syu] = RecvNtnetDt.RData68.Unknown.Num;				// 分類不明　台数
		syukei->Bunrui1_ryo2[bn_syu] = RecvNtnetDt.RData68.Unknown.Amount;			//           台数2／金額
	}
	/*分類集計２*/
	bn_syu = 0xff;
	tyu_syu= 0xff;
	if(( RecvNtnetDt.RData68.Kind2 >= 1 ) && ( RecvNtnetDt.RData68.Kind2 <= 7 )){
		bn_syu = 0;
		tyu_syu = (uchar)RecvNtnetDt.RData68.Kind2;//集計種類
	}else if(( RecvNtnetDt.RData68.Kind2 >= 51 ) && ( RecvNtnetDt.RData68.Kind2 <= 57 )){
		bn_syu = 1;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind2 - 50);//集計種類
	}else if(( RecvNtnetDt.RData68.Kind2 >= 101 ) && ( RecvNtnetDt.RData68.Kind2 <= 107 )){
		bn_syu = 2;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind2 - 100);//集計種類
	}
	if(( tyu_syu == CPrmSS[S_BUN][1] )&&( bn_syu != 0xff )){
		for (i = 0; i < BUNRUI_CNT; i++) {										// 分類01～48
			syukei->Bunrui1_cnt[bn_syu][i] = RecvNtnetDt.RData68.Group2[i].Num;		//           台数1
			syukei->Bunrui1_ryo[bn_syu][i] = RecvNtnetDt.RData68.Group2[i].Amount;	//           台数2／金額
		}
		syukei->Bunrui1_cnt1[bn_syu] = RecvNtnetDt.RData68.GroupTotal2.Num;			// 分類以上　台数
		syukei->Bunrui1_ryo1[bn_syu] = RecvNtnetDt.RData68.GroupTotal2.Amount;		//           台数2／金額
		syukei->Bunrui1_cnt2[bn_syu] = RecvNtnetDt.RData68.Unknown2.Num;				// 分類不明　台数
		syukei->Bunrui1_ryo2[bn_syu] = RecvNtnetDt.RData68.Unknown2.Amount;			//           台数2／金額
	}
	/*分類集計３*/
	bn_syu = 0xff;
	tyu_syu= 0xff;
	if(( RecvNtnetDt.RData68.Kind3 >= 1 ) && ( RecvNtnetDt.RData68.Kind3 <= 7 )){
		bn_syu = 0;
		tyu_syu = (uchar)RecvNtnetDt.RData68.Kind3;//集計種類
	}else if(( RecvNtnetDt.RData68.Kind3 >= 51 ) && ( RecvNtnetDt.RData68.Kind3 <= 57 )){
		bn_syu = 1;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind3 - 50);//集計種類
	}else if(( RecvNtnetDt.RData68.Kind3 >= 101 ) && ( RecvNtnetDt.RData68.Kind3 <= 107 )){
		bn_syu = 2;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind3 - 100);//集計種類
	}
	if(( tyu_syu == CPrmSS[S_BUN][1] )&&( bn_syu != 0xff )){
		for (i = 0; i < BUNRUI_CNT; i++) {										// 分類01～48
			syukei->Bunrui1_cnt[bn_syu][i] = RecvNtnetDt.RData68.Group3[i].Num;		//           台数1
			syukei->Bunrui1_ryo[bn_syu][i] = RecvNtnetDt.RData68.Group3[i].Amount;	//           台数2／金額
		}
		syukei->Bunrui1_cnt1[bn_syu] = RecvNtnetDt.RData68.GroupTotal3.Num;			// 分類以上　台数
		syukei->Bunrui1_ryo1[bn_syu] = RecvNtnetDt.RData68.GroupTotal3.Amount;		//           台数2／金額
		syukei->Bunrui1_cnt2[bn_syu] = RecvNtnetDt.RData68.Unknown3.Num;				// 分類不明　台数
		syukei->Bunrui1_ryo2[bn_syu] = RecvNtnetDt.RData68.Unknown3.Amount;			//           台数2／金額
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 割引集計データ受信処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiWaribiki                                |*/
/*| PARAMETER    : syukei : 集計データ					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiWaribiki(SYUKEI *syukei)
{
	uchar	parking;
	ushort	i, group, kind;
	
	/* 複数集計データ受信待機中以外 */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no	= RecvNtnetDt.RData69.KakariNo;									// 係員No.
	syukei->Oiban.w		= 																// 集計追番
	syukei->Oiban.i		= RecvNtnetDt.RData69.SeqNo;									// 集計追番
	
	for ( i = 0; i < _countof( RecvNtnetDt.RData69.Discount ); i++ ){					//割引01～500
		kind = RecvNtnetDt.RData69.Discount[i].Kind;									//割引種別獲得
		group = RecvNtnetDt.RData69.Discount[i].Group;									//割引区分獲得
		parking = NTNET_GetParkingKind( RecvNtnetDt.RData69.Discount[i].ParkingNo, PKOFS_SEARCH_LOCAL );	//駐車場番号獲得

		// クレジット
		if ( kind == 30 ){
			syukei->Ccrd_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Ccrd_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// 店割引 全割引合計
		} else if ( kind == 63 ){
			syukei->Mno_use_Tcnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Mno_use_Tryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// 種別割引
		// Suica
		} else if ( kind == 31 ){
// MH321800(S) Y.Tanizaki ICクレジット対応
//			syukei->Electron_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
//			syukei->Electron_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			if(isEC_USE()) {
				syukei->koutsuu_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
				syukei->koutsuu_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			} else {
				syukei->Electron_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
				syukei->Electron_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			}
// MH321800(E) Y.Tanizaki ICクレジット対応
		// PASMO
		} else if ( kind == 33 ){
			syukei->Electron_psm_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Electron_psm_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// ICOCA
		} else if ( kind == NTNET_ICOCA_0 ){
			syukei->Electron_ico_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Electron_ico_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// IC-Card
		} else if ( kind == NTNET_ICCARD_0 ){
			syukei->Electron_icd_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Electron_icd_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// Edy
		} else if ( kind == 32 ){
			if( 0 == group ){		// 成功取引
				syukei->Electron_edy_cnt = RecvNtnetDt.RData69.Discount[i].Num;
				syukei->Electron_edy_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			}else{					// 失敗取引（アラーム取引）
				syukei->Electron_Arm_cnt = RecvNtnetDt.RData69.Discount[i].Num;
				syukei->Electron_Arm_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			}
// MH321800(S) Y.Tanizaki ICクレジット対応
		} else if(kind == NTNET_NANACO_0) {
			syukei->nanaco_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->nanaco_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		} else if(kind == NTNET_WAON_0) {
			syukei->waon_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->waon_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		} else if(kind == NTNET_SAPICA_0) {
			syukei->sapica_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->sapica_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		} else if(kind == NTNET_ID_0) {
			syukei->id_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->id_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		} else if(kind == NTNET_QUICPAY_0) {
			syukei->quicpay_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->quicpay_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
// MH321800(E) Y.Tanizaki ICクレジット対応
		} else {
			if ( parking != 0xFF ){														//一致する駐車場番号がある場合
				switch ( kind ){
				// サービス券
				case	01:
					if ( group >= 1 && group <= SERVICE_SYU_CNT ) {						// サービス券割引区分1～3
						group--;
						syukei->Stik_use_cnt[parking][group] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Stik_use_ryo[parking][group] = RecvNtnetDt.RData69.Discount[i].Amount;
					}
					break;
				// プリペイド
				case	11:
						syukei->Pcrd_use_cnt[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Pcrd_use_ryo[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				// 回数券
				case	05:
						syukei->Ktik_use_cnt[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Ktik_use_ryo[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				// 店割引 店No.毎
				case	02:
						if (((group >= 1) && (group <= MISE_NO_CNT)) && ((parking == 0) || (parking == 1))) {	// 店割引区分1～100　かつ　基本・拡張
							group--;
							syukei->Mno_use_cnt4[parking][group] = RecvNtnetDt.RData69.Discount[i].Num;
							syukei->Mno_use_ryo4[parking][group] = RecvNtnetDt.RData69.Discount[i].Amount;
						}
					break;
				// 店割引 駐車場No.毎
				case	60:
						syukei->Mno_use_cnt1[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Mno_use_ryo1[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				// 店割引 1～100合計
				case	61:
						syukei->Mno_use_cnt2[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Mno_use_ryo2[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				// 店割引 101～255合計
				case	62:
						syukei->Mno_use_cnt3[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Mno_use_ryo3[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				default:
					break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期集計データ受信処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiTeiki                                   |*/
/*| PARAMETER    : syukei : 集計データ					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiTeiki(SYUKEI *syukei)
{
	uchar	parking;
	ushort	i;

	/* 複数集計データ受信待機中以外 */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no	= RecvNtnetDt.RData70.KakariNo;									// 係員No.
	syukei->Oiban.w		= 																// 集計追番
	syukei->Oiban.i		= RecvNtnetDt.RData70.SeqNo;									// 集計追番
	for ( i = 0; i < _countof( RecvNtnetDt.RData70.Pass ); i++) {						// 定期券001～100
		parking = NTNET_GetParkingKind( RecvNtnetDt.RData70.Pass[i].ParkingNo, PKOFS_SEARCH_LOCAL );
		if ( parking != 0xFF ){
			// 定期券種別数1～15
			if ( RecvNtnetDt.RData70.Pass[i].Kind >= 1 && RecvNtnetDt.RData70.Pass[i].Kind <= TEIKI_SYU_CNT ){ 
				syukei->Teiki_use_cnt[parking][RecvNtnetDt.RData70.Pass[i].Kind-1] = RecvNtnetDt.RData70.Pass[i].Num;
			}
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 金銭集計データ受信処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiKinsen                                  |*/
/*| PARAMETER    : syukei : 集計データ					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiKinsen(SYUKEI *syukei)
{
	static const ushort c_coin[] = { 10, 50, 100, 500 };
	ushort	i, j;

	/* 複数集計データ受信待機中以外 */
	if (NTNetTotalEndFlag)
		return;

	
	syukei->Kakari_no	= RecvNtnetDt.RData72.KakariNo;									// 係員No.
	syukei->Oiban.w		= 																// 集計追番
	syukei->Oiban.i		= RecvNtnetDt.RData72.SeqNo;									// 集計追番
	syukei->Kinko_Tryo	= RecvNtnetDt.RData72.Total;									// 金庫総入金額
	syukei->Note_Tryo	= RecvNtnetDt.RData72.NoteTotal;								// 紙幣金庫総入金額
	syukei->Coin_Tryo	= RecvNtnetDt.RData72.CoinTotal;								// コイン金庫総入金額

	for ( i = 0; i < COIN_SYU_CNT; i++ ) {												// コイン1～4
		j = ntnet_CoinIdx(RecvNtnetDt.RData72.Coin[i].Kind);							// コイン金種
		if( j != 0xFFFF ){
			syukei->Coin_ryo[j] = RecvNtnetDt.RData72.Coin[i].Kind * RecvNtnetDt.RData72.Coin[i].Num;	// コイン金庫入金額　各金種毎
			syukei->Coin_cnt[j] = RecvNtnetDt.RData72.Coin[i].Num;						// コイン金庫入金数　各金種毎
		}
	}
	
	for ( i = 0; i < SIHEI_SYU_CNT; i++ ) {												// 紙幣1～4
		if ( RecvNtnetDt.RData72.Note[i].Kind == 1000 ){								// 紙幣金種
			syukei->Note_ryo[0] = RecvNtnetDt.RData72.Note[i].Kind * RecvNtnetDt.RData72.Note[i].Num2;	// 紙幣金庫入金額　各金種毎
			syukei->Note_cnt[0] = RecvNtnetDt.RData72.Note[i].Num2;						// 紙幣金庫入金数　各金種毎
		}
	}
	
	syukei->tou[4] = RecvNtnetDt.RData72.NoteAcceptTotal / 1000;						// 紙幣総入金数
	
	for ( i = 0; i < COIN_SYU_CNT; i++ ) {												// 循環1～4
		j = ntnet_CoinIdx(RecvNtnetDt.RData72.Cycle[i].CoinKind);						// 循環1～4コイン金種
		if( j != 0xFFFF ){
			syukei->tou[j] = RecvNtnetDt.RData72.Cycle[i].Accept;						// 精算投入枚数　各金種毎
			syukei->hoj[j] = RecvNtnetDt.RData72.Cycle[i].ChargeSupply;					// 釣銭補充枚数　各金種毎
			syukei->sei[j] = RecvNtnetDt.RData72.Cycle[i].Pay;							// 精算払出枚数　各金種毎
			syukei->kyo[j] = RecvNtnetDt.RData72.Cycle[i].SlotInventory;				// 強制払出枚数　各金種毎
		}
	}
	syukei->tounyu = 1000 * syukei->tou[4];
	syukei->hojyu  = 
	syukei->seisan = 
	syukei->kyosei = 0;
	for (i = 0; i < _countof(c_coin); i++) {
		syukei->tounyu += ( c_coin[i] * syukei->tou[i] );								// 精算投入金額　総額
		syukei->hojyu  += ( c_coin[i] * syukei->hoj[i] );								// 釣銭補充金額　総額
		syukei->seisan += ( c_coin[i] * syukei->sei[i] );								// 精算払出金額　総額
		syukei->kyosei += ( c_coin[i] * syukei->kyo[i] );								// 強制払出金額　総額
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 集計終了通知データ受信処理                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiSyuryo                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ogura                                                   |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiSyuryo( void )
{
	/* 複数集計データ受信完了 */
	if (NTNetTotalEndFlag == 0) {
		NTNetTotalEndFlag = 1;		// 複数集計データ受信完了
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| Freeﾃﾞｰﾀ作成処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_DataFree                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_DataFree( void )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_Free ) );
	memcpy( SendNtnetDt.SDataFree.FreeData, "FREE", 4 );
	SendNtnetDt.SDataFree.FreeData[5] = 5;	/* データクリア */
	if( NTBUF_SetSendFreeData( &SendNtnetDt ) == 0 ){
		// バッファオーバーライト発生
		
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 制御ﾃﾞｰﾀ(ﾃﾞｰﾀ種別100)作成処理  （ｱﾝﾁﾊﾟｽ設定）                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data100                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.Sekiguchi                                             |*/
/*| Date         : 2006-08-23                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_Snd_Data100( void )
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_100 ) );			//データクリア
	BasicDataMake( 100 , 1 );									//基本データ作成
	SendNtnetDt.SData101.SMachineNo = 0;						// 送信先端末機械№（親機宛）
//ｱﾝﾁﾊﾟｽ設定
	if( PPrmSS[S_P01][3] == 0 ){
		SendNtnetDt.SData101.ControlData[3] = 1;				// 自動
	}
	else if( PPrmSS[S_P01][3] == 1 ){
		SendNtnetDt.SData101.ControlData[3] = 2;				// 解除
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_100 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定ﾃﾞｰﾀ要求(ﾃﾞｰﾀ種別90)作成処理 - ｱﾄﾞﾚｽ指定                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data90                                        |*/
/*| PARAMETER    : mac   : 送信先端末機械№                                |*/
/*|              : sts   : ﾃﾞｰﾀｽﾃｰﾀｽ(0:中間 1:最終)                        |*/
/*|              : seg   : 設定ｾｸﾞﾒﾝﾄ(ｾｸｼｮﾝ)                               |*/
/*|              : addr  : 開始ｱﾄﾞﾚｽ                                       |*/
/*|              : count : 設定ﾃﾞｰﾀ数                                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2006-11-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static const ushort NtDataPrm90[1][4]= {

		{  0,   S_DIS,   8,  18 },			// 券期限
};

void	NTNET_Snd_Data90(uchar type)
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_78));

	BasicDataMake(90, 1);														// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData90.SMachineNo = 0L;										// 送信先ﾀｰﾐﾅﾙ№＝親機
	SendNtnetDt.SData90.ModelCode = (ulong)NTNET_MODEL_CODE;					// 端末機種コード
	SendNtnetDt.SData90.PrmDiv = NtDataPrm90[type][0];							// ﾊﾟﾗﾒｰﾀ区分
	SendNtnetDt.SData90.Segment = NtDataPrm90[type][1];							// 要求ｾｸﾞﾒﾝﾄ
	SendNtnetDt.SData90.TopAddr = NtDataPrm90[type][2];							// 開始ｱﾄﾞﾚｽ
	SendNtnetDt.SData90.DataCount = NtDataPrm90[type][3];						// 設定ﾃﾞｰﾀ数

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_78 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_78), NTNET_BUF_NORMAL);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 受信NTNETデータで車室パラメータorロック装置パラメータ更新(停電保証対応) |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_UpdateParam                                       |*/
/*| PARAMETER    : fukuden - 更新元データ情報                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_UpdateParam(NTNETCTRL_FUKUDEN_PARAM *fukuden)
{
	union {
		DATA_KIND_97 *p97;
		DATA_KIND_98 *p98;
	}u;
	int i;
	int j=0;
	
	switch (fukuden->DataKind) {
	case 97:	// 車室パラメータ
		u.p97 = (DATA_KIND_97*)fukuden->Src;
		for (i = 0; i < LOCK_MAX; i++) {
			WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if( !SetCarInfoSelect((short)i) ){
				continue;
			}
			if( j >= OLD_LOCK_MAX ){
				break;
			}
			LockInfo[i].lok_syu	=	(uchar)u.p97->LockInfo[j].lok_syu;
			LockInfo[i].ryo_syu	=	(uchar)u.p97->LockInfo[j].ryo_syu;
			LockInfo[i].area	=	(uchar)u.p97->LockInfo[j].area;
			LockInfo[i].posi	=	(ulong)u.p97->LockInfo[j].posi;
			LockInfo[i].if_oya	=	(uchar)u.p97->LockInfo[j].if_oya;
			if( i < BIKE_START_INDEX ) {	// 駐車情報の場合、変換が必要
				if( u.p97->LockInfo[j].lok_no < PRM_LOKNO_MIN || u.p97->LockInfo[j].lok_no > PRM_LOKNO_MAX ){	// 範囲チェック( 100～315 有効 )
					LockInfo[i].lok_no	=	0;														// 範囲外の場合、0(接続無状態)を保存
				} else {
					LockInfo[i].lok_no	=	(uchar)( u.p97->LockInfo[j].lok_no - PRM_LOKNO_MIN );	// 範囲内の場合、100を引いて保存する(保存領域が1byteのため)
				}
			} else {						// 駐輪は従来通り
				LockInfo[i].lok_no	=	(uchar)u.p97->LockInfo[j].lok_no;
			}
			j++;
		}
		DataSumUpdate(OPE_DTNUM_LOCKINFO);				// RAM上のSUM更新
		FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);			// FLASH書込み
		break;
	case 98:	// ロック装置パラメータ
		u.p98 = (DATA_KIND_98*)fukuden->Src;
		for (i = 0; i < _countof(LockMaker); i++) {
			LockMaker[i].in_tm		= (uchar)u.p98->LockMaker[i].in_tm;
			LockMaker[i].ot_tm		= (uchar)u.p98->LockMaker[i].ot_tm;
			LockMaker[i].r_cnt		= (uchar)u.p98->LockMaker[i].r_cnt;
			LockMaker[i].r_tim		= u.p98->LockMaker[i].r_tim;
			LockMaker[i].open_tm	= (uchar)u.p98->LockMaker[i].open_tm;
			LockMaker[i].clse_tm	= (uchar)u.p98->LockMaker[i].clse_tm;
		}
		break;
	default:	// 復電処理なし
		break;
	}
	
	fukuden->DataKind = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 駐車場番号取得                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_GetParkinngNo                                     |*/
/*| PARAMETER    : uchar ParkingKind                                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong NTNET_GetParkingNo(uchar ParkingKind)
{
	ulong parking_no;

	parking_no = CPrmSS[S_SYS][65 + ParkingKind];
	return parking_no;

}

/*[]----------------------------------------------------------------------[]*/
/*| 定期更新ステータス取得                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_ReadUpDateStatus                                  |*/
/*| PARAMETER    : ulong PassId                                            |*/
/*| RETURN VALUE : uchar                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar ntnet_ReadUpDateStatus(ulong PassId)
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	PAS_RENEWAL work;
//	uchar status;
//	work = pas_renewal[PassId / 4];
//
//	switch (PassId % 4) {
//	case 1:
//		status = work.BIT.Bt01;
//		break;
//	case 2:
//		status = work.BIT.Bt23;
//		break;
//	case 3:
//		status = work.BIT.Bt45;
//		break;
//	case 0:
//	default:
//		status = work.BIT.Bt67;
//		break;
//	
//	}
//
//	return status;
	return 0;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 集計終了通知データ                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_isTotalEndReceived                                |*/
/*| PARAMETER    : ushort result                                           |*/
/*| RETURN VALUE : int 0:未受信                                            |*/
/*| RETURN VALUE : int 1:受信完了                                          |*/
/*| RETURN VALUE : int-1:NG受信                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	NTNET_isTotalEndReceived( ushort *result )
{
	*result = NTNetTotalEndError;
	return NTNetTotalEndFlag;
}

/*[]----------------------------------------------------------------------[]*/
/*| 日時データ変換                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_DateTimeCnv                                       |*/
/*| PARAMETER    : dst = 変換先, src = 変換元                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ntnet_DateTimeCnv(date_time_rec2 *dst, const date_time_rec *src)
{
	dst->Year	= src->Year;
	dst->Mon	= src->Mon;
	dst->Day	= src->Day;
	dst->Hour	= src->Hour;
	dst->Min	= src->Min;
	dst->Sec	= 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券精算中止データ検索                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_SearchTeikiCyusiData                              |*/
/*| PARAMETER    : ParkingNo = 検索キー1(駐車場No.)                        |*/
/*|                PassID    = 検索キー2(定期券ID)                         |*/
/*| RETURN VALUE : 該当データのテーブル先頭からのインデックス              |*/
/*|                データが見つからないときは、-1                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	ntnet_SearchTeikiCyusiData(ulong ParkingNo, ulong PassID)
{
	short idx, ofs;
	
	idx = 0;
	for (idx = 0; idx < tki_cyusi.count; idx++) {
		ofs = _TKI_Idx2Ofs(idx);
		if (tki_cyusi.dt[ofs].pk == ParkingNo && tki_cyusi.dt[ofs].no == PassID) {
			return idx;
		}
	}
	
	return -1;
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券精算中止データ変換(CRMフォーマット⇒NTNETフォーマット)           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_TeikiCyusiCnvCrmToNt                              |*/
/*| PARAMETER    : ntnet = NTNETフォーマットデータ           <OUT>         |*/
/*|                crm   = CRMフォーマットデータ             <IN>          |*/
/*|                all   = TRUE→全ての項目をセット                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ntnet_TeikiCyusiCnvCrmToNt(TEIKI_CHUSI *ntnet, const struct TKI_CYUSI *crm, BOOL all)
{
	
	ntnet->ParkingNo		=	crm->pk;
	ntnet->PassID			=	crm->no;
	if (all) {
		ntnet->PassKind			=	crm->tksy;
		ntnet->Year				=	crm->year;
		ntnet->Month			=	crm->mon;
		ntnet->Day				=	crm->day;
		ntnet->Hour				=	crm->hour;
		ntnet->Min				=	crm->min;
		ntnet->Sec				=	0;
		ntnet->ChargeType		=	crm->syubetu;
		ntnet->UseCount			=	crm->use_count;
		ntnet->CouponRyo		=	crm->kry;
		ntnet->DiscountMoney	=	crm->wryo;
		ntnet->DiscountTime		=	crm->wminute;
		ntnet->DiscountRate		=	crm->wpercent;
		ntnet->ShopNo			=	crm->mno;
		ntnet->ServiceTicketA	=	crm->sa[0];
		ntnet->ServiceTicketB	=	crm->sa[1];
		ntnet->ServiceTicketC	=	crm->sa[2];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券精算中止データ変換(NTNETフォーマット⇒CRMフォーマット)           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_TeikiCyusiCnvNtToCrm                              |*/
/*| PARAMETER    : crm   = CRMフォーマットデータ           <OUT>           |*/
/*|                ntnet = NTNETフォーマットデータ         <IN>            |*/
/*| RETURN VALUE : BOOL                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	ntnet_TeikiCyusiCnvNtToCrm(struct TKI_CYUSI *crm, const TEIKI_CHUSI *ntnet)
{
	crm->pk			=	ntnet->ParkingNo;
	crm->no			=	(short)ntnet->PassID;
	crm->tksy		=	(uchar)ntnet->PassKind;
	crm->year		=	ntnet->Year;
	crm->mon		=	(uchar)ntnet->Month;
	crm->day		=	(uchar)ntnet->Day;
	crm->hour		=	(uchar)ntnet->Hour;
	crm->min		=	(uchar)ntnet->Min;
	crm->syubetu	=	(uchar)ntnet->ChargeType;
	crm->use_count	=	(uchar)ntnet->UseCount;
	crm->kry		=	ntnet->CouponRyo;
	crm->wryo		=	ntnet->DiscountMoney;
	crm->wminute	=	ntnet->DiscountTime;
	crm->wpercent	=	(uchar)ntnet->DiscountRate;
	crm->mno		=	(char)ntnet->ShopNo;
	crm->sa[0]		=	(char)ntnet->ServiceTicketA;
	crm->sa[1]		=	(char)ntnet->ServiceTicketB;
	crm->sa[2]		=	(char)ntnet->ServiceTicketC;
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期区分取得（マスターの区分）                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_GetParkingKind                                    |*/
/*| PARAMETER    : ulong PassId                                            |*/
/*|                int search = PKOFS_SEARCH_LOCAL or PKOFS_SEARCH_MASTER  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar NTNET_GetParkingKind(ulong ParkingId, int search)
{
	int i, base;
	uchar parking_kind;

	parking_kind = 0xFF;

	if (ParkingId == 0) {
		return parking_kind;
	}

	if (search == PKOFS_SEARCH_MASTER) {
		base = 65;
	}
	else {	// search == PKOFS_SEARCH_LOCAL
		base = 1;
	}
	for (i = 0; i < 4; i++) {
		if (ParkingId == CPrmSS[S_SYS][base + i]) {
			parking_kind = (uchar)i;
			break;
		}
	}

	return parking_kind;
}

/*[]----------------------------------------------------------------------[]*/
/*| コイン金種からCRM内データインデックスを取得                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_CoinIdx                                           |*/
/*| PARAMETER    : kind = コイン金種                                       |*/
/*| RETURN VALUE : CRM内データインデックス(0～3)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	ntnet_CoinIdx(ushort kind)
{
	switch (kind){
	case	10:
		return 0;
	case	50:
		return 1;
	case	100:
		return 2;
	case	500:
		return 3;
	default:
		return 0xFFFF;
	}
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 詳細中止ｴﾘｱ検索																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: cyushi_chk( no )																   |*/
/*| PARAMETER	: no	; 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: H.Sekiguchi																	   |*/
/*| Date		: 2006-10-15																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar cyushi_chk( ushort no )
{
	struct CAR_TIM wk_time1,wk_time2;
	uchar	i;

	for(i = 0 ; i < 10 ; i++){
		if(FLAPDT_SUB[i].WPlace == LockInfo[no].posi){
			/*入庫時間*/
			wk_time1.year = FLAPDT.flp_data[no].year ;
			wk_time1.mon = FLAPDT.flp_data[no].mont ;
			wk_time1.day = FLAPDT.flp_data[no].date ;
			wk_time1.hour = FLAPDT.flp_data[no].hour ;
			wk_time1.min = FLAPDT.flp_data[no].minu ;
			/*詳細中止入庫ﾃﾞｰﾀ*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;
			
			if(0 == ec64(&wk_time1,&wk_time2)){
					break;
			}
		}
	}
	if(i >= 10){
		return 99;									//再精算&&詳細中止ｴﾘｱ無
	}else{
		return i;									//再精算&&詳細中止ｴﾘｱ有
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 基本ﾃﾞｰﾀ2作成処理                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BasicDataMake_R                                         |*/
/*| PARAMETER    : knd  : ﾃﾞｰﾀ種別                                         |*/
/*|                keep : ﾃﾞｰﾀ保持ﾌﾗｸﾞ                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	BasicDataMake_R( uchar knd, uchar keep )
{
	SendNtnetDt.DataBasicR.SystemID = REMOTE_SYSTEM;	// ｼｽﾃﾑID
	SendNtnetDt.DataBasicR.DataKind = knd;				// ﾃﾞｰﾀ種別
	SendNtnetDt.DataBasicR.DataKeep = keep;				// ﾃﾞｰﾀ保持ﾌﾗｸﾞ+ 自動/手動ﾌﾗｸﾞ
}

/*[]----------------------------------------------------------------------[]*/
/*| 遠隔IBK制御データ(ﾃﾞｰﾀ種別60)送信処理                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data60_R                                      |*/
/*| PARAMETER    : req = 要求データフラグ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data60_R(ulong req)
{
	memset(&SendNtnetDt.SData60_R, 0, sizeof(DATA_KIND_60_R));
	BasicDataMake_R(60, 1);	// 最上位ビット：自動固定
	ntnet_set_req(req, SendNtnetDt.SData60_R.Data, REMOTE_IBKCTRL_MAX);

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_60_R ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_60_R), NTNET_BUF_PRIOR);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 送信・テーブルクリア要求データ(ﾃﾞｰﾀ種別61/62)送信処理                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_DataTblCtrl                                   |*/
/*| PARAMETER    : id  = 61: 送信要求/ 62: クリア要求                      |*/
/*|                req = 要求データフラグ                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_DataTblCtrl(uchar id, ulong req, uchar bAuto)
{
	uchar keep;

	if (bAuto) {
		keep = 0x01;	// 自動
	} else {
		keep = 0x81;	// 手動
	}
	memset(&SendNtnetDt.SData61_R, 0, sizeof(DATA_KIND_61_R));
	BasicDataMake_R(id, keep);	// 最上位ビット：自動/手動
	ntnet_set_req(req, SendNtnetDt.SData61_R.Data, NTNET_BUFCTRL_REQ_MAX);

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_61_R ));
	}
	else {
		if( keep == 0x81 )		// 手動送信
			NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_61_R), NTNET_BUF_PRIOR);		// 優先ﾊﾞｯﾌｧで送信登録
		else					// 自動送信
			NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_61_R), NTNET_BUF_NORMAL);	// 通常ﾊﾞｯﾌｧで送信登録
	}
}

// 要求データを設定
void	ntnet_set_req(ulong req, uchar *dat, int max)
{
	ushort i;
	ulong  flg = 0x00000001;

	for (i = 0; i < max; i++) {
		if (req & (flg << i)) dat[i] = 1;
	}
}

static DATA_KIND_100R_sub SndSata100R_BU;
/*[]----------------------------------------------------------------------[]*/
/*| 通信チェック要求データ(ﾃﾞｰﾀ種別100)送信処理                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data100_R                                     |*/
/*| PARAMETER    : mode = 0: ﾁｪｯｸ/ 1: 中止                                 |*/
/*|                chk_num = 通信ﾁｪｯｸNo.                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data100_R(uchar mode, uchar chk_num)
{
	memset(&SendNtnetDt.SData100_R, 0, sizeof(DATA_KIND_100_R));
	memset(&SndSata100R_BU, 0, sizeof(DATA_KIND_100R_sub));

	BasicDataMake_R(100, 0x01);

	SendNtnetDt.SData100_R.SeqNo = 0;									// ｼｰｹﾝｼｬﾙ№(0:ﾁｪｯｸしない)

	SendNtnetDt.SData100_R.sub.ParkingNo  = (ulong)CPrmSS[S_SYS][1];	// 駐車場№
	SendNtnetDt.SData100_R.sub.ModelCode  = NTNET_MODEL_CODE;			// 機種ｺｰﾄﾞ
	SendNtnetDt.SData100_R.sub.SMachineNo = 0;	// 送信元№ 0:main

	SendNtnetDt.SData100_R.sub.Year = (uchar)( CLK_REC.year % 100 );	// 年
	SendNtnetDt.SData100_R.sub.Mon  = (uchar)CLK_REC.mont;				// 月
	SendNtnetDt.SData100_R.sub.Day  = (uchar)CLK_REC.date;				// 日
	SendNtnetDt.SData100_R.sub.Hour = (uchar)CLK_REC.hour;				// 時
	SendNtnetDt.SData100_R.sub.Min  = (uchar)CLK_REC.minu;				// 分
	SendNtnetDt.SData100_R.sub.Sec  = (uchar)CLK_REC.seco;				// 秒

	SendNtnetDt.SData100_R.sub.ChkNo = chk_num;							// 通信ﾁｪｯｸNo.

	SendNtnetDt.SData100_R.Data[0] = mode;								// 要求種別
	SendNtnetDt.SData100_R.Data[1] = 10;								// ﾀｲﾑｱｳﾄ

	memcpy(&SndSata100R_BU, &SendNtnetDt.SData100_R.sub, sizeof(DATA_KIND_100_R));	// ﾃﾞｰﾀﾊﾞｯｸｱｯﾌﾟ

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100_R ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_100_R), NTNET_BUF_PRIOR);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 通信チェック結果データ(ﾃﾞｰﾀ種別101)受信処理                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_Data101_R                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Rev_Data101_R(void)
{
	int ret;
	
	// 送信データと比較
	ret = memcmp(&SndSata100R_BU, &RecvNtnetDt.RData101_R.sub, sizeof(DATA_KIND_100R_sub));
	if (ret == 0) {
		IBK_ComChkResult = RecvNtnetDt.RData101_R.Data[0];	// 結果コードセット
	}
	else {
		IBK_ComChkResult = 99;								// 受信データ不一致
	}

	queset( OPETCBNO, IBK_COMCHK_END, 0, NULL );			// NTNET 通信テスト終了通知
}

#define RelaySize	38
static union {
	uchar  cRelayBuf[RelaySize];
	ushort sRelayBuf[RelaySize/2];
} rbuf;

/*[]----------------------------------------------------------------------[]*/
/*| テーブル件数データ(ﾃﾞｰﾀ種別83)受信処理                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData83_R                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData83_R(void)
{

	//kind:80 受信からの流れなら、HOSTに合算値で通知(IBK_BufCountはIBKの持ってる値)
	if (s_IsRevData80_R) {
		NTNET_Snd_Data63_R();
		s_IsRevData80_R = 0;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 遠隔IBK制御データ
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData80_R
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData80_R(void)
{
	if (RecvNtnetDt.RData60_R.Data[0] == 1) {
		// 自動フラグON  NTNET_RevData83_R時に参照する
		s_IsRevData80_R = 1;
		NTNET_Snd_Data60_R(1);
	}
}

/*[]----------------------------------------------------------------------[]*
 *| テーブル件数データ送信
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data63_R
 *| PARAMETER    : 
 *| RETURN VALUE : 
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data63_R(void)
{
	t_NtBufCount buf;
	ushort *pt = rbuf.sRelayBuf;

	// CRM保持の件数取得
	NTBUF_GetBufCount(&buf);

	memset(&SendNtnetDt.SData63_R, 0, sizeof(DATA_KIND_83_R));
	BasicDataMake_R( 63, 1 );										// 基本ﾃﾞｰﾀ作成

	// IBK + CRM（一旦中継ワークにコピー）
	memset(&rbuf.sRelayBuf, 0, sizeof(rbuf.sRelayBuf));
	// 入庫

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 6)) {
//		*pt = (ushort)buf.sndbuf_incar;
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	// 出庫
	*pt++;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	*pt = (ushort)buf.sndbuf_outcar;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	// 精算
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 5)) {
		*pt = (ushort)buf.sndbuf_sale;
	}
	// 集計
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 4)) {
		if(0 != prm_get(COM_PRM, S_NTN, 26, 1, 3)) {	// 集計で34-0026④=0(送信しない）なら０件とする
			*pt = (ushort)buf.sndbuf_ttotal;
		}
	}
	// エラー
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 3)) {
		*pt = (ushort)buf.sndbuf_error;
	}
	// アラーム
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 2)) {
		*pt = (ushort)buf.sndbuf_alarm;
	}
	// モニター
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 1)) {
		*pt = (ushort)buf.sndbuf_monitor;
	}
	// 操作モニター
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 6)) {
		*pt = (ushort)buf.sndbuf_ope_monitor;
	}
	// コイン金庫集計
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 5)) {
		*pt = (ushort)buf.sndbuf_coin;
	}
	// 紙幣金庫集計
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 4)) {
		*pt = (ushort)buf.sndbuf_note;
	}
	// 駐車台数
	*pt++;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 3)) {
//		*pt = (ushort)buf.sndbuf_car_count;
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	// 区画台数・満車データ
	*pt++;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	*pt = (ushort)buf.sndbuf_area_count;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	// 金銭管理
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 1)) {
		*pt = (ushort)buf.sndbuf_money;
	}

	memcpy(SendNtnetDt.SData63_R.Data, rbuf.cRelayBuf, RelaySize);

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_83_R ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_83_R ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}


/*[]----------------------------------------------------------------------[]*
 *| ニアフル通知データ
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData90_R
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData90_R(void)
{
	int i;
	ulong	occur = 0;	// 変化のあったもの
	ulong	mask = 0;	// ニアフル発生のもの
	ulong	bit;

	// 送信要求/データクリア要求の並びに合わせる -->
	// 精算データ(ID22とID23)のマージ
	if (RecvNtnetDt.RData90_R.Data[2] == 0) {
		// ID22変化なし
		RecvNtnetDt.RData90_R.Data[2] = RecvNtnetDt.RData90_R.Data[3];	// ID23
	}
	else if (RecvNtnetDt.RData90_R.Data[2] == 2) {
		// ID22ニアフル解除
		if (RecvNtnetDt.RData90_R.Data[3] == 1)
			RecvNtnetDt.RData90_R.Data[2] = 1;		// 発生を優先
	}
	// ひとつ前につめる
	memcpy(&RecvNtnetDt.RData90_R.Data[3], &RecvNtnetDt.RData90_R.Data[4], NTNET_BUFCTRL_REQ_MAX-3);
	RecvNtnetDt.RData90_R.Data[NTNET_BUFCTRL_REQ_MAX-1] = 0;

	// 変換後データからビットデータ取得
	bit = 1L;
	for (i = 0; i < NTNET_BUFCTRL_REQ_MAX; i++) {
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		if(( i == 0 ) || ( i == 1 ) || ( i == 10 ) || ( i == 11 )){
			// 入庫/出庫/駐車台数/区画台数・満車データはチェックせず
			// bitをずらしてスキップする
			bit <<= 1;
			continue;
		}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		// 変化あり
		if (RecvNtnetDt.RData90_R.Data[i] != 0) {
			mask |= bit;
		}
		// ニアフル発生
		if (RecvNtnetDt.RData90_R.Data[i] == 1) {
			if(i == 12) {
				bit <<= 1;
			}
			occur |= bit;
		}
		bit <<= 1;
	}

	// 通知
	if (occur) {
		ntautoPostNearFull(occur);	// 自動送信へ発生通知
	}
	if (mask) {
		NTBUF_SetIBKNearFull(mask, occur);	// Buffer層へ通知
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 通信チェック要求データ
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData116_R
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData116_R(void)
{
	IBK_ComChkResult = RecvNtnetDt.RData101_R.Data[0];	// エラーコードセット

	if (RecvNtnetDt.RData101_R.Data[0] != 0) {	// チェック開始以外なら読み捨て
		return;
	}

	NTNET_Snd_Data117_R(0, (char*)&RecvNtnetDt.RData101_R);	// 結果コード"OK"で返答
}

/*[]----------------------------------------------------------------------[]*
 *| 通信チェック結果データ送信
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data117_R
 *| PARAMETER    : 
 *| RETURN VALUE : 
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data117_R(uchar code, char *data)
{
	DATA_KIND_100_R *blk;

	blk = (DATA_KIND_100_R*)data;
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_100_R ) );

	BasicDataMake_R( 117, 1 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData100_R.SeqNo = blk->SeqNo;
	memcpy((char*)&SendNtnetDt.SData100_R.sub, (char*)&blk->sub, sizeof(DATA_KIND_100R_sub));

	SendNtnetDt.SData100_R.Data[0] = code;

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100_R ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_100_R ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 定期券問合せデータ受信
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData142
 *| PARAMETER    : 
 *| RETURN VALUE : 
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-03-20
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData142(void)
{
	if (!_is_ntnet_remote()) {
		return;		// HOST通信時のみ対応
	}

	if (RecvNtnetDt.RData142.ParkingNo == 0 ||
		RecvNtnetDt.RData142.PassID == 0) {
		return;		// 駐車場No、定期券IDが0の場合は無視
	}

	NTNET_Snd_Data143(RecvNtnetDt.RData142.ParkingNo, RecvNtnetDt.RData142.PassID);
}

/*[]----------------------------------------------------------------------[]*
 *| 定期券問合せ結果データ送信
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data143
 *| PARAMETER    : 
 *| RETURN VALUE : 
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-03-20
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data143(ulong ParkingNo, ulong PassID)
{
	PAS_TBL		*pass_tbl;
	date_time_rec	time;
	ushort		status;

	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_143));

	// 定期券ｽﾃｰﾀｽﾃﾞｰﾀ取得
	pass_tbl = GetPassData((ulong)ParkingNo, (ushort)PassID);
	if (pass_tbl == NULL) {
		return;
	}
	//定期券更新ｽﾃｰﾀｽ取得
	if (ReadPassRenewalTbl((ulong)ParkingNo, (ushort)PassID, &status) < 0) {
		return;
	}
	// 定期券出庫時刻ﾃｰﾌﾞﾙ読出し
	memset(&time, 0, sizeof(date_time_rec));
	PassExitTimeTblRead((ulong)ParkingNo, (ushort)PassID, &time);

	// ﾃﾞｰﾀｾｯﾄ
	BasicDataMake(143, 1);
	SendNtnetDt.SData143.ParkingNo = ParkingNo;
	SendNtnetDt.SData143.PassID = PassID;
	if(pass_tbl->BIT.INV == 0){									// 有効
		SendNtnetDt.SData143.PassState = pass_tbl->BIT.STS;		// 0:初期　1:出庫中  2:入庫中
	}else{														// 無効
		SendNtnetDt.SData143.PassState = 3;						// 3:無効
	}
	SendNtnetDt.SData143.RenewalState = (uchar)status;
	memcpy(&SendNtnetDt.SData143.OutTime, &time, sizeof(date_time_rec));
	SendNtnetDt.SData143.UseParkingNo = (ulong)prm_get(COM_PRM, S_SYS, 1, 6, 1);

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_143 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_143), NTNET_BUF_PRIOR);	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| パスワードデータ(ﾃﾞｰﾀ種別84)受信処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_Data84                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Suzuki                                                |*/
/*| Date         : 2006-06-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_RevData84( void )
{
	uchar	w_uc;
	ulong 	w_ul;

	// ﾊﾟｽﾜｰﾄﾞﾃﾞｰﾀ受信ﾃﾞｰﾀを共通ﾊﾟﾗﾒｰﾀTBLに反映する。
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord[0] & 0x0f);
	w_ul = (ulong)(w_uc * 1000); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord[1] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc * 100); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord[2] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc * 10); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord[3] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc); 

	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord2[0] & 0x0f);
	w_ul = (ulong)(w_uc * 1000); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord2[1] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc * 100); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord2[2] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc * 10); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord2[3] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc); 

	DataSumUpdate(OPE_DTNUM_COMPARA);					/* update parameter sum on ram */
	FLT_WriteParam1(FLT_NOT_EXCLUSIVE);					// FlashROM update
	SetSetDiff(SETDIFFLOG_SYU_REMSET);					// デフォルトセットログ登録処理

	// 操作ログを登録する
	wopelg(OPLOG_SET_PASSWORD, 0, 0);
}


typedef	struct {
	uchar			id;			// 種別
	uchar			code;		// ｺｰﾄﾞ
} t_MltE_CPS;

const t_MltE_CPS	MltE_CPS_SndTbl[4][50] = {	// 複数台精算機時の親機送信要否チェック用テーブル
	{	// type=00（エラー）
		{0x03, 0x01}, {0x03, 0x05}, {0x03, 0x06}, {0x03, 0x07}, {0x03, 0x0c},
		{0x03, 0x0d}, {0x03, 0x0e}, {0x03, 0x0f}, {0x03, 0x12}, {0x03, 0x13},
		{0x03, 0x14}, {0x03, 0x05}, {0x04, 0x01}, {0x04, 0x05}, {0x04, 0x06},
		{0x04, 0x07}, {0x04, 0x08}, {0x02, 0x01}, {0x02, 0x04}, {0x00, 0x11},
		{0x01, 0x01}, {0x01, 0x0a}, {0x01, 0x0b}, {0x01, 0x0d}, {0x01, 0x0e},
		{0x01, 0x0f}, {0x01, 0x10}, {0x01, 0x11}, {0x01, 0x12}, {0x01, 0x13},
		{0x01, 0x14}, {0x01, 0x18}, {0x01, 0x19}, {0x01, 0x20}, {0x00, 0x1f},
		{0x00, 0x12}, {0x09, 0x0a}, {0x09, 0x0b}, {0x00, 0x19}, {0x3f, 0x01},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}
	},
	{	// type=01（アラーム）
		{0x01, 0x1d}, {0x01, 0x02}, {0x00, 0x01}, {0x00, 0x05}, {0x00, 0x06},
		{0x01, 0x06}, {0x01, 0x20}, {0x01, 0x07}, {0x01, 0x1f}, {0x01, 0x0f},
		{0x01, 0x10}, {0x01, 0x11}, {0x01, 0x12}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}
	},
	{	// type=02（モニタ）
		{0x01, 0x0d}, {0x01, 0x0e}, {0x01, 0x0b}, {0x01, 0x0c}, {0x01, 0x07},
		{0x01, 0x08}, {   0,   88}, {   0,   89}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}
	},
	{	// type=03（操作モニタ）
		{0x01, 0x01}, {0x01, 0x02}, {0x01, 0x0b}, {0x01, 0x0c}, {0x12, 0x01},
		{0x12, 0x02}, {0x12, 0x03}, {0x12, 0x04}, {0x12, 0x05}, {0x12, 0x06},
		{0x12, 0x07}, {0x11, 0x01}, {0x11, 0x02}, {0x11, 0x0c}, {0x01, 0x0f},
		{0x50, 0x5a}, {0x50, 0x5b}, {0x01, 0x14}, {0x01, 0x15}, {0x01, 0x16},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}
	},
};

/*[]----------------------------------------------------------------------[]
 *|	name	: Remote exclusive use Charge calculation
 *[]----------------------------------------------------------------------[]
 *| summary	: 遠隔用料金計算処理
 *| return	: 駐車料金
 *[]----------------------------------------------------------------------[]*/
ulong	OrgCal( ushort Index )
{

	char	tim[6];
	ulong	ret=0;
	
	if ( LockInfo[Index].lok_syu != 0 ) {				// 接続がある車室の場合
		if( FLAPDT.flp_data[Index].nstat.bits.b00 ){	// 在車している場合
			if( FLAPDT.flp_data[Index].timer < 0){
				if( InCarTimeChk( (ushort)(Index+1), &CLK_REC ) == 0 ){	// 入出庫時刻OK?(Y)
					ret = 1;
				}
			}
		}
	}

	if( !ret )
		return ret;
	
	memcpy( &CAL_WORK.car_in, &car_in, sizeof( struct CAR_TIM ));  	/*　入車時刻　*/
	memcpy( &CAL_WORK.car_ot, &car_ot, sizeof( struct CAR_TIM ));  	/*　出車時刻　*/
	memcpy( &CAL_WORK.org_in, &org_in, sizeof( struct CAR_TIM )); 	/*　入車時刻　*/
	memcpy( &CAL_WORK.org_ot, &org_ot, sizeof( struct CAR_TIM )); 	/*　出車時刻　*/
	memcpy( &CAL_WORK.carin_mt, &carin_mt, sizeof( struct CAR_TIM )); /*　入車時刻(元)　*/
	memcpy( &CAL_WORK.carot_mt, &carot_mt, sizeof( struct CAR_TIM )); /*　出車時刻(元)　*/
	memcpy( &CAL_WORK.nmax_in, &nmax_in, sizeof( struct CAR_TIM ));  /*　ｎ時間最大基点時刻　*/
	
	CAL_WORK.wk_strt = wk_strt;								  		/*指定時刻最大check時刻に加算する時間　　　　 */
	CAL_WORK.iti_flg = iti_flg;		  								/*　一定料金帯フラグ　　　　　　　　　　　　　*/
	CAL_WORK.amartim = amartim;		  								/*▲一定料金帯余り時間　　　　　　　　　　　　*/

	memcpy( tim,&FLAPDT.flp_data[Index].year,6 ); 					/* 入車時刻ｾｯﾄ 			*/
	car_in.year  = *(short*)tim ;									/* 入車年				*/
	car_in.mon   = tim[2]	;										/*	   月				*/
	car_in.day   = tim[3]	;										/*　　 日 　　　　　	*/
	car_in.hour  = tim[4]	;										/*　　 時 　　　　　	*/
	car_in.min   = tim[5]	;										/*　　 分 　　　　　	*/
	car_in.week  = (char)youbiget( (short)car_in.year ,				/*　　 曜日 　　　　	*/
						 (short)car_in.mon  ,						/*　　	　　　　　　	*/
						 (short)car_in.day  )	;					/*　　	　　　　　　	*/

	memcpy( tim, &CLK_REC, 6 );

	car_ot.year = *(short*)tim   ;									/* 出庫年 　　　　　	*/
	car_ot.mon  = tim[2]       ;									/*　　 月 　　　　　	*/
	car_ot.day  = tim[3]       ;									/*     日 　　　　　	*/
	car_ot.week  = (char)youbiget( (short)car_in.year ,				//     曜日
								(short)car_in.mon  ,
								(short)car_in.day  );				/*　　	　　　　　　	*/
	car_ot.hour = tim[4]       ;									/* 	   時 　　　　　	*/
	car_ot.min  = tim[5]       ;									/*　　 分 　　　　　	*/

	memcpy( &nmax_in,	&org_in, 7 );								// 実際の入庫時刻をＮ時間基点時刻バッファへ格納
	CAL_WORK.ryoukin = ryoukin;
	CAL_WORK.total = total;
	CAL_WORK.syubt = (int)req_rkn.syubt;
	CAL_WORK.base_ryo = base_ryo;
	CAL_WORK.discount = discount;
	CAL_WORK.ntnet_nmax_bak = ntnet_nmax_flg;						/*　最大料金ﾌﾗｸﾞﾊﾞｯｸｱｯﾌﾟ　*/
	memcpy( &CAL_WORK.stim, &se_svt.stim, sizeof( struct	SVT	));
	CAL_WORK.mod = 1;												/* ﾊﾞｯｸｱｯﾌﾟ中をｾｯﾄ		*/
	ryoukin = 0;
	if( LockInfo[Index].ryo_syu <= 0 || LockInfo[Index].ryo_syu > 12){
		req_rkn.syubt = 1;
	}else{
		req_rkn.syubt = LockInfo[Index].ryo_syu;
	}
	if( CPrmSS[S_STM][1] == 0 ){									/* 全共通ｻｰﾋﾞｽﾀｲﾑ,ｸﾞﾚｰｽﾀｲﾑ		*/
		se_svt.stim = (short)CPrmSS[S_STM][2];						/* ｻｰﾋﾞｽﾀｲﾑget					*/
		se_svt.gtim = (short)CPrmSS[S_STM][3];						/* ｸﾞﾚｰｽﾀｲﾑget					*/
		se_svt.rtim = (short)CPrmSS[S_STM][4];						/* ﾗｸﾞﾀｲﾑget					*/
	}else{															/* 種別毎ｻｰﾋﾞｽﾀｲﾑ				*/
		se_svt.stim = (short)CPrmSS[S_STM][5+(3*(req_rkn.syubt-1))];/* 種別毎ｻｰﾋﾞｽﾀｲﾑget			*/
		se_svt.gtim = (short)CPrmSS[S_STM][6+(3*(req_rkn.syubt-1))];/* 種別毎ｸﾞﾚｰｽﾀｲﾑget			*/
		se_svt.rtim = (short)CPrmSS[S_STM][7+(3*(req_rkn.syubt-1))];/* 種別毎ﾗｸﾞﾀｲﾑget				*/
	}																/*								*/

	if( FLAPDT.flp_data[Index].lag_to_in.BIT.LAGIN == ON ){
		se_svt.stim = 0;											/* Yes→ｻｰﾋﾞｽﾀｲﾑを無効とする	*/
	}
	else
	{
// MH322914 (s) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
//		if(	( FLAPDT.flp_data[Index].mode == FLAP_CTRL_MODE3 ) ||
//			( FLAPDT.flp_data[Index].mode == FLAP_CTRL_MODE4 ) )
//		{
//			long	svtm,uptm;
//
//			svtm = (long)se_svt.stim;
//			svtm *= 60;
//
//			if( (Index+1) >= LOCK_START_NO)
//			{
//				uptm = Carkind_Param(ROCK_CLOSE_TIMER, req_rkn.syubt, 5,1);
//			}
//			else
//			{
//				uptm = Carkind_Param(FLAP_UP_TIMER, req_rkn.syubt, 5,1);
//			}
//			uptm = (long)(((uptm/100)*60) + (uptm%100));
//
//			if( svtm == uptm )											
//			{
//				se_svt.stim = 0;											// ｻｰﾋﾞｽﾀｲﾑ無効
//			}
//		}
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//		if ( prm_get( COM_PRM, S_TYP, 68, 1, 5 ) == 0 ) {
//			// ロック閉タイマーとサービスタイムが同じ時間の時にサービスタイムをロック閉までとする
//			if(	( FLAPDT.flp_data[Index].mode == FLAP_CTRL_MODE3 ) ||
//				( FLAPDT.flp_data[Index].mode == FLAP_CTRL_MODE4 ) )
//			{
//				long	svtm,uptm;
//
//				svtm = (long)se_svt.stim;
//				svtm *= 60;
//
//				if( (Index+1) >= LOCK_START_NO)
//				{
//					uptm = Carkind_Param(ROCK_CLOSE_TIMER, req_rkn.syubt, 5,1);
//				}
//				else
//				{
//					uptm = Carkind_Param(FLAP_UP_TIMER, req_rkn.syubt, 5,1);
//				}
//				uptm = (long)(((uptm/100)*60) + (uptm%100));
//
//				if( svtm == uptm )											
//				{
//					se_svt.stim = 0;											// ｻｰﾋﾞｽﾀｲﾑ無効
//				}
//			}
//		}
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
// MH322914 (e) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
	}
	WACDOG;// RX630はWDGストップが出来ない仕様のためWDGリセットにする
	et40_remote();													/* 料金算出				*/
	WACDOG;// RX630はWDGストップが出来ない仕様のためWDGリセットにする
	ret = ryoukin;
	Remote_Cal_Data_Restor();
	return( ret );

}

/*[]----------------------------------------------------------------------[]*/
/*| 入出庫時刻判定                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : InCarTimeChk( num, clk )				                   |*/
/*| PARAMETER    : num      : 車室№                                       |*/
/*|              : clk      : 現在時刻                                     |*/
/*| RETURN VALUE : ret      : 入出庫時刻判定 0:OK 1:NG 					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2010-10-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
uchar	InCarTimeChk( ushort num, struct clk_rec *clk_para )
{
	flp_com	*frp;
	ushort	in_tim, out_tim;

	frp = &FLAPDT.flp_data[num-1];

	if( ( !frp->year || !frp->mont || !frp->date ) ||
	    chkdate( (short)frp->year, (short)frp->mont, (short)frp->date )){	// 入庫時刻規定外?
		return( 1 );
	}

	in_tim = dnrmlzm((short)frp->year,								// (入庫日)normlize
					(short)frp->mont,
					(short)frp->date );
	out_tim = dnrmlzm((short)clk_para->year,						// (出庫日)現在年月日をnormlize
					(short)clk_para->mont,
					(short)clk_para->date );
					
	if( in_tim > out_tim ){											// 入庫日の方が大きい場合
		return( 1 );												// エラーとする
	}																
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| バックアップデータリストア処理                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Remote_Cal_Data_Restor( void )		                   |*/
/*| PARAMETER    : void					                                   |*/
/*| RETURN VALUE : void								 					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2009-06-02                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void Remote_Cal_Data_Restor( void ){

	if( CAL_WORK.mod ){			/* ﾊﾞｯｸｱｯﾌﾟ中?		*/
		ryoukin = CAL_WORK.ryoukin;
		total = CAL_WORK.total;
		req_rkn.syubt = (uchar)CAL_WORK.syubt;
		base_ryo = CAL_WORK.base_ryo;
		discount = CAL_WORK.discount;
		memcpy( &car_in, &CAL_WORK.car_in, sizeof( struct CAR_TIM )); /*　入車時刻　*/
		memcpy( &car_ot, &CAL_WORK.car_ot, sizeof( struct CAR_TIM )); /*　出車時刻　*/
		memcpy( &org_in, &CAL_WORK.org_in, sizeof( struct CAR_TIM )); /*　入車時刻　*/
		memcpy( &org_ot, &CAL_WORK.org_ot, sizeof( struct CAR_TIM )); /*　出車時刻　*/
		ntnet_nmax_flg = CAL_WORK.ntnet_nmax_bak;					/*　最大料金ﾌﾗｸﾞﾘｽﾄｱ	*/
		memcpy( &se_svt.stim, &CAL_WORK.stim, sizeof( struct	SVT	));
		memcpy( &carin_mt, &CAL_WORK.carin_mt, sizeof( struct CAR_TIM )); /*　入車時刻(元)　*/
		memcpy( &carot_mt, &CAL_WORK.carot_mt, sizeof( struct CAR_TIM )); /*　出車時刻(元)　*/
		memcpy( &nmax_in, &CAL_WORK.nmax_in, sizeof( struct CAR_TIM ));  /*　ｎ時間最大基点時刻　*/
		wk_strt = CAL_WORK.wk_strt;								  		/*指定時刻最大check時刻に加算する時間　　　　 */
		iti_flg = CAL_WORK.iti_flg;		  								/*　一定料金帯フラグ　　　　　　　　　　　　　*/
		amartim = CAL_WORK.amartim;		  								/*▲一定料金帯余り時間　　　　　　　　　　　　*/
		CAL_WORK.mod = 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 車室情報格納範囲チェック処理			                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SetCarInfoSelect			                               |*/
/*| PARAMETER    : index	車室情報を格納するためのIndex				   |*/
/*| RETURN VALUE : ret	0:無効範囲	1:有効範囲						       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka	                                               |*/
/*| Date         : 2009-09-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	SetCarInfoSelect( short index )
{
	uchar ret=1;
	uchar param=1;
		
	param = GetCarInfoParam();
	
	switch( param ){
		case	0x01:			// CRA接続のみ
			if( index < BIKE_START_INDEX ){
				ret = 0;		// 無効
			}
			break;
		case	0x02:			// IBC接続のみ
			if( index >= INT_CAR_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x03:			// IBC+CRA接続併用
			if( index >= INT_CAR_START_INDEX && index < BIKE_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x04:			// CRR接続のみ
			if( index < INT_CAR_START_INDEX || index >= BIKE_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x05:			// CRA+CRR接続併用
			if( index < INT_CAR_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x06:			// IBC+CRR接続併用
			if( index >= BIKE_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x07:			// IBC+CRA+CRR接続併用
			if( index >= OLD_LOCK_MAX ){
				ret = 0;
			}
			break;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金計算用エリア更新                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mc10exec( void )		         	                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN		 : void					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : namioka                                                 |*/
/*| Date         : 2010-05-12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	mc10exec( void )
{
	if( f_NTNET_RCV_MC10_EXEC ){				// mc10()更新が必要な設定が通信で更新された
		if( check_acceptable() ){				// mc10()実行可能条件OK？
			mc10();
			f_NTNET_RCV_MC10_EXEC = 0;
		}
	}
}

const char	Series_Name[12] = {"FT-4000FX   "};
/*[]----------------------------------------------------------------------[]*/
/*| 精算機機器情報データ(ﾃﾞｰﾀ種別190)作成処理                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data190                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2010-10-05                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	NTNET_Snd_Data190( void )
{

}
/*[]----------------------------------------------------------------------[]*/
/*| 遠隔ダウンロード要求(ﾃﾞｰﾀ種別188)受信処理                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData188                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2010-10-12                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	NTNET_RevData188( void *data )
{

}
/*[]----------------------------------------------------------------------[]*/
/*| 遠隔ダウンロード応答データ(ﾃﾞｰﾀ種別188)送信処理		                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data189	                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2010-04-28                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	NTNET_Snd_Data189( uchar err_code, uchar data )
{

}

/*[]----------------------------------------------------------------------[]*/
/*| 精算ﾃﾞｰﾀ(ﾃﾞｰﾀ種別22,23)編集処理                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data22                                        |*/
/*| PARAMETER    : p_RcptDat(IN) 精算ログデータ(Receipt_data型)へのポインタ|*/
/*|              : p_NtDat  (IN) 精算データ(DATA_KIND_22型)へのポインタ    |*/
/*| RETURN VALUE : ret        精算データサイズ(システムID～)               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                             |*/
/*| Date         : 2012-02-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	NTNET_Edit_Data22( Receipt_data *p_RcptDat, DATA_KIND_22 *p_NtDat )
{
	uchar	i;
	uchar	j;
	ushort	ret;
// MH321800(S) 割引種別8000のセット方法を修正
	ushort		wk_kind1, wk_kind2;
// MH321800(E) 割引種別8000のセット方法を修正

	memset( p_NtDat, 0, sizeof( DATA_KIND_22 ) );

	p_NtDat->DataBasic.SystemID = p_RcptDat->DataBasic.SystemID;		// ｼｽﾃﾑID
	p_NtDat->DataBasic.DataKind = p_RcptDat->DataBasic.DataKind;		// ﾃﾞｰﾀ種別
	p_NtDat->DataBasic.DataKeep = p_RcptDat->DataBasic.DataKeep;		// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	p_NtDat->DataBasic.ParkingNo = p_RcptDat->DataBasic.ParkingNo;		// 駐車場№
	p_NtDat->DataBasic.ModelCode = p_RcptDat->DataBasic.ModelCode;		// 機種ｺｰﾄﾞ
	p_NtDat->DataBasic.MachineNo = p_RcptDat->DataBasic.MachineNo;		// 機械№
	memcpy( &p_NtDat->DataBasic.Year, &p_RcptDat->DataBasic.Year, 6 );	// 処理年月日時分秒

	p_NtDat->PayCount = CountSel( &p_RcptDat->Oiban);					// 精算or精算中止追い番
	p_NtDat->PayMethod = p_RcptDat->PayMethod;							// 精算方法
	p_NtDat->PayClass = p_RcptDat->PayClass;							// 処理区分
	p_NtDat->PayMode = p_RcptDat->PayMode;								// 精算ﾓｰﾄﾞ(自動精算)

// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->LockNo = p_RcptDat->WPlace;							// 区画情報
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し 2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	p_NtDat->CardType = 0;												// 駐車券ﾀｲﾌﾟ
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(LCD_IF対応)
//	p_NtDat->CMachineNo = 0;											// 駐車券機械№
	p_NtDat->CMachineNo	= p_RcptDat->CMachineNo;						// 駐車券機械№
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(LCD_IF対応)
	p_NtDat->CardNo = 0L;												// 駐車券番号(発券追い番)
	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->OutTime.Year = p_RcptDat->TOutTime.Year;				// 出庫年
		p_NtDat->OutTime.Mon = p_RcptDat->TOutTime.Mon;					// 出庫月
		p_NtDat->OutTime.Day = p_RcptDat->TOutTime.Day;					// 出庫日
		p_NtDat->OutTime.Hour = p_RcptDat->TOutTime.Hour;				// 出庫時
		p_NtDat->OutTime.Min = p_RcptDat->TOutTime.Min;					// 出庫分
		p_NtDat->OutTime.Sec = 0;										// 出庫秒
		p_NtDat->KakariNo = p_RcptDat->KakariNo;						// 係員№
		p_NtDat->OutKind = p_RcptDat->OutKind;							// 精算出庫
	}
	p_NtDat->ReceiptIssue = p_RcptDat->ReceiptIssue;					// 領収証発行有無
	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->InTime.Year = p_RcptDat->TInTime.Year;					// 入庫年
		p_NtDat->InTime.Mon = p_RcptDat->TInTime.Mon;					// 入庫月
		p_NtDat->InTime.Day = p_RcptDat->TInTime.Day;					// 入庫日
		p_NtDat->InTime.Hour = p_RcptDat->TInTime.Hour;					// 入庫時
		p_NtDat->InTime.Min = p_RcptDat->TInTime.Min;					// 入庫分
		p_NtDat->InTime.Sec = 0;										// 入庫秒
	}
	// 精算年月日時分秒は0とする
	// 前回精算年月日時分秒は0とする
	p_NtDat->TaxPrice = 0;												// 課税対象額
	p_NtDat->TotalPrice = 0;											// 合計金額(HOST未使用のため)
	p_NtDat->Tax = p_RcptDat->Wtax;										// 消費税額
	if( p_NtDat->PayMethod != 5 ){										// 更新精算以外
		p_NtDat->Syubet = p_RcptDat->syu;								// 料金種別
		p_NtDat->Price = p_RcptDat->WPrice;								// 駐車料金
	}
	p_NtDat->CashPrice =
						p_RcptDat->WInPrice - p_RcptDat->WChgPrice;		// 現金売上
	p_NtDat->InPrice = p_RcptDat->WInPrice;								// 投入金額
	p_NtDat->ChgPrice = p_RcptDat->WChgPrice;							// 釣銭金額
	p_NtDat->Fusoku = p_RcptDat->WFusoku;								// 釣銭払出不足金額
	p_NtDat->FusokuFlg = 0;												// 釣銭払出不足発生ﾌﾗｸﾞ
	p_NtDat->PayObsFlg = 0;												// 精算中障害発生ﾌﾗｸﾞ
	p_NtDat->ChgOverFlg = 0;											// 払戻上限額ｵｰﾊﾞｰ発生ﾌﾗｸﾞ

	p_NtDat->PassCheck = p_RcptDat->PassCheck;							// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	p_NtDat->CountSet = p_RcptDat->CountSet;							// 在車ｶｳﾝﾄしない
	if( p_RcptDat->teiki.ParkingNo ){									// 定期券精算処理or定期券更新時
		p_NtDat->PassData.ParkingNo = p_RcptDat->teiki.ParkingNo;		// 定期券駐車場№
		p_NtDat->PassData.PassID = p_RcptDat->teiki.id;					// 定期券ID
		p_NtDat->PassData.Syubet = p_RcptDat->teiki.syu;				// 定期券種別
		p_NtDat->PassData.State = p_RcptDat->teiki.status;				// 定期券ｽﾃｰﾀｽ
		p_NtDat->PassData.MoveMode = 0;									// 定期券返却
		p_NtDat->PassData.ReadMode = 0;									// 定期券ﾘｰﾄﾞﾗｲﾄ
		p_NtDat->PassData.SYear =
								(uchar)(p_RcptDat->teiki.s_year%100);	// 定期券開始年
		p_NtDat->PassData.SMon = p_RcptDat->teiki.s_mon;				// 定期券開始月
		p_NtDat->PassData.SDate = p_RcptDat->teiki.s_day;				// 定期券開始日
		p_NtDat->PassData.EYear =
								(uchar)(p_RcptDat->teiki.e_year%100);	// 定期券終了年
		p_NtDat->PassData.EMon = p_RcptDat->teiki.e_mon;				// 定期券終了月
		p_NtDat->PassData.EDate = p_RcptDat->teiki.e_day;				// 定期券終了日
	}
	if( p_NtDat->PayMethod == 5 ){
		p_NtDat->PassRenewalPric = p_RcptDat->WPrice;					// 定期券更新料金

		p_NtDat->PassRenewalCondition = 1;								// 定期券更新条件
		if( rangechk( 1, 3, p_RcptDat->teiki.status ) ){				// ｽﾃｰﾀｽ1～3範囲内
			p_NtDat->PassRenewalCondition = 11;							// 定期券更新条件
		}
		p_NtDat->PassRenewalPeriod = p_RcptDat->teiki.update_mon;		// 定期券更新期間(更新月数)
	}

// MH810103 GG119202(S) 決済精算中止時は精算データにみなし決済情報をセットしない
//	if (ntnet_decision_credit(&p_RcptDat->credit)) {
	if (ntnet_decision_credit(&p_RcptDat->credit) &&
		p_NtDat->PayClass != 2 && p_NtDat->PayClass != 3 ){					// 精算中止、再精算中止ではない
// MH810103 GG119202(E) 決済精算中止時は精算データにみなし決済情報をセットしない
// MH321800(S) NT-NET精算データ仕様変更
//	memcpy( p_NtDat->CreditCardNo,
//			p_RcptDat->credit.card_no,
//			sizeof( p_NtDat->CreditCardNo ) );							// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
	if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
	    p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
		// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.を0x20h埋めする。
		// [*]印字みなし決済と同様の会員Noをセットする。
		memset( &p_NtDat->CreditCardNo[0], 0x20, sizeof(p_NtDat->CreditCardNo) );
	}
	else {
// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
	memcpyFlushLeft( &p_NtDat->CreditCardNo[0],
					 (uchar *)&p_RcptDat->credit.card_no[0],
					 sizeof( p_NtDat->CreditCardNo ),
					 sizeof( p_RcptDat->credit.card_no ) );				// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
	}
// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
// MH321800(S) NT-NET精算データ仕様変更
// MH321800(S) D.Inaba ICクレジット対応
	// 会員Noの上7桁目～下5桁目以外で'*'が格納されていたら'0'に置換
	change_CharInArray( &p_NtDat->CreditCardNo[0], sizeof(p_NtDat->CreditCardNo), 7, 5, '*', '0' );
// MH321800(E) D.Inaba ICクレジット対応	
	p_NtDat->Credit_ryo = p_RcptDat->credit.pay_ryo;					// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
	p_NtDat->CreditSlipNo = p_RcptDat->credit.slip_no;					// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	p_NtDat->CreditAppNo = p_RcptDat->credit.app_no;					// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号
// MH321800(S) NT-NET精算データ仕様変更
//	memcpy( p_NtDat->CreditName,
//			p_RcptDat->credit.card_name,
//			sizeof( p_NtDat->CreditName ) );							// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
	memcpyFlushLeft( &p_NtDat->CreditName[0],
					 (uchar *)&p_RcptDat->credit.card_name[0],
					 sizeof( p_NtDat->CreditName ),
					 sizeof( p_RcptDat->credit.card_name ) );			// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
// MH321800(E) NT-NET精算データ仕様変更
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	p_NtDat->CreditDate[0] = p_RcptDat->credit.CreditDate[0];			// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(年)
//	p_NtDat->CreditDate[1] = p_RcptDat->credit.CreditDate[1];			// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(月)
//	p_NtDat->CreditProcessNo = p_RcptDat->credit.CenterProcOiBan;		// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀ処理追い番
	// 明示的に0をセット
	p_NtDat->CreditDate[0] = 0;											// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(年)
	p_NtDat->CreditDate[1] = 0;											// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(月)
	p_NtDat->CreditProcessNo = 0;										// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀ処理追い番
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
	}

	for( i = j = 0; i < WTIK_USEMAX; i++ ){								// 精算中止以外の割引情報コピー
		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&				// 割引種別あり
		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||	// 精算中止割引情報でない
		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 			memcpy( &p_NtDat->DiscountData[j], &p_RcptDat->DiscountData[i], sizeof( DISCOUNT_DATA ) );	// 割引情報
// 			if( p_NtDat->DiscountData[j].DiscSyu == NTNET_PRI_W ){
// 				memset( &p_NtDat->DiscountData[j].uDiscData, 0, 8 );	// 不要データクリア
// 			}
// 			j++;
			if ( p_RcptDat->DiscountData[i].Discount || p_RcptDat->DiscountData[i].DiscCount ) {
				p_NtDat->DiscountData[j].ParkingNo = p_RcptDat->DiscountData[i].ParkingNo;
				p_NtDat->DiscountData[j].DiscSyu   = p_RcptDat->DiscountData[i].DiscSyu;
				p_NtDat->DiscountData[j].DiscNo    = p_RcptDat->DiscountData[i].DiscNo;
				p_NtDat->DiscountData[j].DiscCount = p_RcptDat->DiscountData[i].DiscCount;
				p_NtDat->DiscountData[j].Discount  = p_RcptDat->DiscountData[i].Discount;
				p_NtDat->DiscountData[j].DiscInfo1 = p_RcptDat->DiscountData[i].DiscInfo1;
				p_NtDat->DiscountData[j].uDiscData.common.DiscInfo2 =
					p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;
				p_NtDat->DiscountData[j].uDiscData.common.DiscFlg = 0;
				switch ( p_RcptDat->DiscountData[i].DiscSyu ) {
				case NTNET_SVS_M:			// サービス券割引(金額)
				case NTNET_SVS_T:			// サービス券割引(時間)
				case NTNET_KAK_M:			// 店割引(金額)
				case NTNET_KAK_T:			// 店割引(時間)
				case NTNET_TKAK_M:			// 多店舗割引(金額)
				case NTNET_TKAK_T:			// 多店舗割引(時間)
				case NTNET_SYUBET:			// 種別割引(金額)
				case NTNET_SYUBET_TIME:		// 種別割引(時間)
				case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
				case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
				case NTNET_GENGAKU:			// 減額精算
				case NTNET_FURIKAE:			// 振替精算
				case NTNET_SYUSEI_1:		// 修正精算
				case NTNET_SYUSEI_2:		// 修正精算
				case NTNET_SYUSEI_3:		// 修正精算
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 1;
					break;
				case NTNET_PRI_W:			// 割引プリペイドカード
					memset(&p_NtDat->DiscountData[j].uDiscData, 0, 8);	// 不要データクリア
					break;
				default:
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 0;
					break;
				}
				j++;
			}

			if ( p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscount ||
				 p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscCount ) {
				p_NtDat->DiscountData[j].ParkingNo = p_RcptDat->DiscountData[i].ParkingNo;
				p_NtDat->DiscountData[j].DiscSyu   = p_RcptDat->DiscountData[i].DiscSyu;
				p_NtDat->DiscountData[j].DiscNo    = p_RcptDat->DiscountData[i].DiscNo;
				p_NtDat->DiscountData[j].DiscCount = p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscCount;
				p_NtDat->DiscountData[j].Discount  = p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscount;
				p_NtDat->DiscountData[j].DiscInfo1 = p_RcptDat->DiscountData[i].DiscInfo1;
				p_NtDat->DiscountData[j].uDiscData.common.DiscInfo2 =
					p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;
				p_NtDat->DiscountData[j].uDiscData.common.DiscFlg = 1;
				switch ( p_RcptDat->DiscountData[i].DiscSyu ) {
				case NTNET_SVS_M:			// サービス券割引(金額)
				case NTNET_SVS_T:			// サービス券割引(時間)
				case NTNET_KAK_M:			// 店割引(金額)
				case NTNET_KAK_T:			// 店割引(時間)
				case NTNET_TKAK_M:			// 多店舗割引(金額)
				case NTNET_TKAK_T:			// 多店舗割引(時間)
				case NTNET_SYUBET:			// 種別割引(金額)
				case NTNET_SYUBET_TIME:		// 種別割引(時間)
				case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
				case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
				case NTNET_GENGAKU:			// 減額精算
				case NTNET_FURIKAE:			// 振替精算
				case NTNET_SYUSEI_1:		// 修正精算
				case NTNET_SYUSEI_2:		// 修正精算
				case NTNET_SYUSEI_3:		// 修正精算
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 1;
					break;
				case NTNET_PRI_W:			// 割引プリペイドカード
					memset(&p_NtDat->DiscountData[j].uDiscData, 0, 8);	// 不要データクリア
					break;
				default:
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 0;
					break;
				}
				j++;
			}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
		}
	}
// MH321800(S) 割引種別8000のセット方法を修正
	// 決済リーダ接続あり
	if (isEC_USE()) {
// MH810103 GG119202(S) 決済精算中止時は精算データにみなし決済情報をセットしない
		// 精算中止、再精算中止時は割引情報にセットしない
		if (p_NtDat->PayClass != 2 && p_NtDat->PayClass != 3) {
// MH810103 GG119202(E) 決済精算中止時は精算データにみなし決済情報をセットしない
		if (p_RcptDat->credit.pay_ryo != 0) {
			// クレジット決済
			/* 空きｴﾘｱ検索 */
			for (i = 0; (i < NTNET_DIC_MAX) &&
					(0 != p_NtDat->DiscountData[i].ParkingNo); i++) {
				;
			}		/* データがある間				*/
			if (i < NTNET_DIC_MAX) {
				p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];		// 駐車場No.
				p_NtDat->DiscountData[i].DiscSyu = NTNET_INQUIRY_NUM;		// 割引種別
				// クレジット決済時は問い合わせ番号を受信しないので0x20埋めとする
				memset(&p_NtDat->DiscountData[i].DiscNo, 0x20, 16);
				p_NtDat->DiscountData[i].uDiscData.common.MoveMode = 1;
				p_NtDat->DiscountData[i].uDiscData.common.DiscFlg = 0;
// MH810103 (s) 電子マネー対応
				i++;
				j++;
// MH810103 (e) 電子マネー対応
			}
		}
		else if (p_RcptDat->Electron_data.Ec.pay_ryo != 0) {
			// 電子マネー決済
			/* 空きｴﾘｱ検索 */
			for (i = 0; (i < NTNET_DIC_MAX) &&
					(0 != p_NtDat->DiscountData[i].ParkingNo); i++) {
				;
			}		/* データがある間				*/
			if (i < (NTNET_DIC_MAX-2)) {
				switch (p_RcptDat->Electron_data.Ec.e_pay_kind) {			// 決済種別から振り分け
				case	EC_EDY_USED:
					wk_kind1 = NTNET_EDY_1;									// 割引種別：Edyｶｰﾄﾞ番号
					wk_kind2 = NTNET_EDY_2;									// 割引種別：Edy支払額、残額
					break;
				case	EC_NANACO_USED:
					wk_kind1 = NTNET_NANACO_1;								// 割引種別：nanacoｶｰﾄﾞ番号
					wk_kind2 = NTNET_NANACO_2;								// 割引種別：nanaco支払額、残額
					break;
				case	EC_WAON_USED:
					wk_kind1 = NTNET_WAON_1;								// 割引種別：WAONｶｰﾄﾞ番号
					wk_kind2 = NTNET_WAON_2;								// 割引種別：WAON支払額、残額
					break;
				case	EC_SAPICA_USED:
					wk_kind1 = NTNET_SAPICA_1;								// 割引種別：SAPICAｶｰﾄﾞ番号
					wk_kind2 = NTNET_SAPICA_2;								// 割引種別：SAPICA支払額、残額
					break;
				case	EC_KOUTSUU_USED:
					// 交通系ICカードはSuicaのIDを使用する
					wk_kind1 = NTNET_SUICA_1;								// 割引種別：Suicaｶｰﾄﾞ番号
					wk_kind2 = NTNET_SUICA_2;								// 割引種別：Suica支払額、残額
					break;
				case	EC_ID_USED:
					wk_kind1 = NTNET_ID_1;									// 割引種別：iDｶｰﾄﾞ番号
					wk_kind2 = NTNET_ID_2;									// 割引種別：iD支払額、残額
					break;
				case	EC_QUIC_PAY_USED:
					wk_kind1 = NTNET_QUICPAY_1;								// 割引種別：QUICPayｶｰﾄﾞ番号
					wk_kind2 = NTNET_QUICPAY_2;								// 割引種別：QUICPay支払額、残額
					break;
				default:
					wk_kind1 = 0;
					wk_kind2 = 0;
					break;
				}

				if (wk_kind1 != 0 && wk_kind2 != 0) {
					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// 駐車場No.
					p_NtDat->DiscountData[i].DiscSyu = wk_kind1;			// 割引種別
					// カード番号
// MH810103 GG119202(S) 電子マネー対応
//					memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Ec.Card_ID, 16);
// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
///					memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Ec.Card_ID, sizeof(p_RcptDat->Electron_data.Ec.Card_ID));
					if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
	    				p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
						// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.をカード桁数分0x30埋めし左詰めでZZする。
						// [*]印字みなし決済と同様の会員Noをセットする。
						memset( &p_NtDat->DiscountData[i].DiscNo, 0x20, sizeof(p_RcptDat->Electron_data.Ec.Card_ID) );
						// それぞれのカード桁数分0x30埋め
						memset( &p_NtDat->DiscountData[i].DiscNo, 0x30, (size_t)(EcBrandEmoney_Digit[p_RcptDat->Electron_data.Ec.e_pay_kind - EC_USED]) );
						// 左詰めでZZ
						memset( &p_NtDat->DiscountData[i].DiscNo, 'Z', 2 );
					}
					else {
						memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Ec.Card_ID, sizeof(p_RcptDat->Electron_data.Ec.Card_ID));
					}
// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
// MH810103 GG119202(E) 電子マネー対応
// MH810103 GG119202(S) 電子マネーカード番号変換処理
					switch (p_RcptDat->Electron_data.Ec.e_pay_kind) {
					case EC_KOUTSUU_USED:
						// フェーズ1フォーマットでは電子マネーのカード番号は'*'->'0'変換する
						change_CharInArray((uchar*)&p_NtDat->DiscountData[i].DiscNo, 20, 3, 5, '*', '0');
						break;
					case EC_EDY_USED:
					case EC_NANACO_USED:
					case EC_WAON_USED:
					case EC_ID_USED:
					case EC_QUIC_PAY_USED:
// MH810103 MHUT40XX(S) Edy・WAON対応
						// フェーズ1フォーマットでは電子マネーのカード番号は'*'->'0'変換する
						change_CharInArray((uchar*)&p_NtDat->DiscountData[i].DiscNo, 20, 1, 5, '*', '0');
// MH810103 MHUT40XX(E) Edy・WAON対応
						break;
					default:
						break;
					}
// MH810103 GG119202(E) 電子マネーカード番号変換処理
					i++;
// MH810103 (s) 電子マネー対応
					j++;
// MH810103 (e) 電子マネー対応

					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// 駐車場No.
					p_NtDat->DiscountData[i].DiscSyu = wk_kind2;			// 割引種別
					p_NtDat->DiscountData[i].DiscNo = 0;					// 割引区分
					p_NtDat->DiscountData[i].DiscCount = 1;					// 使用枚数
					p_NtDat->DiscountData[i].Discount = p_RcptDat->Electron_data.Ec.pay_ryo;	// 支払金額
					p_NtDat->DiscountData[i].DiscInfo1 = p_RcptDat->Electron_data.Ec.pay_after;	// 残額
					switch (p_RcptDat->Electron_data.Ec.e_pay_kind) {		// 決済種別から振り分け
					case	EC_EDY_USED:
// MH810103 MHUT40XX(S) Edy・WAON対応
//						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = p_RcptDat->Electron_data.Ec.Brand.Edy.DealNo;			// Edy取引通番
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = astoinl(p_RcptDat->Electron_data.Ec.Brand.Edy.CardDealNo, 5);	// カード取引通番
// MH810103 MHUT40XX(E) Edy・WAON対応
						break;
					case	EC_NANACO_USED:
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = p_RcptDat->Electron_data.Ec.Brand.Nanaco.DealNo;		// nanaco取引通番
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = astoinl(p_RcptDat->Electron_data.Ec.Brand.Nanaco.DealNo, 6);	// 端末取引通番
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
						break;
					case	EC_WAON_USED:
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = p_RcptDat->Electron_data.Ec.Brand.Waon.point;			// WAON今回付与ポイント
						break;
					case	EC_SAPICA_USED:
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = p_RcptDat->Electron_data.Ec.Brand.Sapica.Details_ID;	// SAPICA一件明細ID
						break;
// MH810104 GG119202(S) nanaco・iD・QUICPay対応
					case	EC_ID_USED:
						// 残額が通知されることはないが、念のため割引情報1をクリアする
						p_NtDat->DiscountData[i].DiscInfo1 = 0;
						// セットする項目なし
						break;
					case	EC_QUIC_PAY_USED:
						// 伝票番号は0をセット
						p_NtDat->DiscountData[i].DiscInfo1 = 0;
						// 取引通番は0をセット
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = 0;
						break;
// MH810104 GG119202(E) nanaco・iD・QUICPay対応
					default:
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = 0;
						break;
					}
					p_NtDat->DiscountData[i].uDiscData.common.MoveMode = 0;
					p_NtDat->DiscountData[i].uDiscData.common.DiscFlg = 0;
					i++;
// MH810103 (s) 電子マネー対応
					j++;
// MH810103 (e) 電子マネー対応

					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// 駐車場No.
					p_NtDat->DiscountData[i].DiscSyu = NTNET_INQUIRY_NUM;	// 割引種別
// MH810103 GG119202(S) 電子マネー対応
//					memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Ec.inquiry_num, 16);
					memcpyFlushLeft( (uchar *)&p_NtDat->DiscountData[i].DiscNo, (uchar *)&p_RcptDat->Electron_data.Ec.inquiry_num[0],
										16, sizeof( p_RcptDat->Electron_data.Ec.inquiry_num ) );	// 問い合わせ番号
// MH810103 GG119202(E) 電子マネー対応
					p_NtDat->DiscountData[i].uDiscData.common.MoveMode = 1;
					p_NtDat->DiscountData[i].uDiscData.common.DiscFlg = 0;
// MH810103 (s) 電子マネー対応
					i++;
					j++;
// MH810103 (e) 電子マネー対応
				}
			}
		}
// MH810103 GG119202(S) 決済精算中止時は精算データにみなし決済情報をセットしない
		}
// MH810103 GG119202(E) 決済精算中止時は精算データにみなし決済情報をセットしない
	}
	else if (isSX10_USE()) {
		if (p_RcptDat->Electron_data.Suica.pay_ryo != 0) {
			/* 空きｴﾘｱ検索 */
			for (i = 0; (i < NTNET_DIC_MAX) &&
					(0 != p_NtDat->DiscountData[i].ParkingNo); i++) {
				;
			}		/* データがある間				*/
			if (i < (NTNET_DIC_MAX-1)) {
				switch(Ope_Disp_Media_Getsub(1)) {
				case	OPE_DISP_MEDIA_TYPE_SUICA:
					wk_kind1 = NTNET_SUICA_1;								// 割引種別：Suicaｶｰﾄﾞ番号
					wk_kind2 = NTNET_SUICA_2;								// 割引種別：Suica支払額、残額
					break;
				case	OPE_DISP_MEDIA_TYPE_PASMO:
					wk_kind1 = NTNET_PASMO_1;								// 割引種別：PASMOｶｰﾄﾞ番号
					wk_kind2 = NTNET_PASMO_2;								// 割引種別：PASMO支払額、残額
					break;
				case	OPE_DISP_MEDIA_TYPE_ICOCA:
					wk_kind1 = NTNET_ICOCA_1;								// 割引種別：ICOCAｶｰﾄﾞ番号
					wk_kind2 = NTNET_ICOCA_2;								// 割引種別：ICOCA支払額、残額
					break;
				case	OPE_DISP_MEDIA_TYPE_ICCARD:
					wk_kind1 = NTNET_ICCARD_1;								// 割引種別：IC-Cardｶｰﾄﾞ番号
					wk_kind2 = NTNET_ICCARD_2;								// 割引種別：IC-Card支払額、残額
					break;
				default:
					wk_kind1 = 0;
					wk_kind2 = 0;
					break;
				}

				if (wk_kind1 != 0 && wk_kind2 != 0) {
					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// 駐車場No.
					p_NtDat->DiscountData[i].DiscSyu = wk_kind1;			// 割引種別
					// カード番号
					memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Suica.Card_ID, 16);
					i++;
// MH810103 (s) 電子マネー対応
					j++;
// MH810103 (e) 電子マネー対応

					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// 駐車場No.
					p_NtDat->DiscountData[i].DiscSyu = wk_kind2;			// 割引種別
					p_NtDat->DiscountData[i].DiscNo = 0;					// 割引区分
					p_NtDat->DiscountData[i].DiscCount = 1;					// 使用枚数
					p_NtDat->DiscountData[i].Discount = p_RcptDat->Electron_data.Suica.pay_ryo;		// 支払金額
					p_NtDat->DiscountData[i].DiscInfo1 = p_RcptDat->Electron_data.Suica.pay_after;	// 残額
					p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = 0;
					p_NtDat->DiscountData[i].uDiscData.common.MoveMode = 0;
					p_NtDat->DiscountData[i].uDiscData.common.DiscFlg = 0;
// MH810103 (s) 電子マネー対応
					i++;
					j++;
// MH810103 (e) 電子マネー対応
				}
			}
		}
	}
// MH321800(E) 割引種別8000のセット方法を修正

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	ret = sizeof( DATA_KIND_22 ) - sizeof( DISCOUNT_DATA )*NTNET_DIC_MAX;// 割引データまでのサイズ算出
	ret = sizeof(DATA_KIND_22) - sizeof(t_SeisanDiscountOld) * NTNET_DIC_MAX;	// 割引データまでのサイズ算出
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	ret += (ushort)sizeof( DISCOUNT_DATA )*j;							// セットした割引データサイズを加算

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算ﾃﾞｰﾀ(ﾃﾞｰﾀ種別22,23)編集処理(不正強制用)                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Edit_Data22_SK                                    |*/
/*| PARAMETER    : p_RcptDat(IN) 精算ログデータ(Receipt_data型)へのポインタ|*/
/*|              : p_NtDat  (IN) 精算データ(DATA_KIND_22型)へのポインタ    |*/
/*| RETURN VALUE : ret        精算データサイズ(システムID～)               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                             |*/
/*| Date         : 2012-02-16                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	NTNET_Edit_Data22_SK( Receipt_data *p_RcptDat, DATA_KIND_22 *p_NtDat )
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_22 ) );

	p_NtDat->DataBasic.SystemID = p_RcptDat->DataBasic.SystemID;		// ｼｽﾃﾑID
	p_NtDat->DataBasic.DataKind = p_RcptDat->DataBasic.DataKind;		// ﾃﾞｰﾀ種別
	p_NtDat->DataBasic.DataKeep = p_RcptDat->DataBasic.DataKeep;		// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	p_NtDat->DataBasic.ParkingNo = p_RcptDat->DataBasic.ParkingNo;		// 駐車場№
	p_NtDat->DataBasic.ModelCode = p_RcptDat->DataBasic.ModelCode;		// 機種ｺｰﾄﾞ
	p_NtDat->DataBasic.MachineNo = p_RcptDat->DataBasic.MachineNo;		// 機械№
	memcpy( &p_NtDat->DataBasic.Year, &p_RcptDat->DataBasic.Year, 6 );	// 処理年月日時分秒

	p_NtDat->PayCount = CountSel( &p_RcptDat->Oiban );					// 精算or精算中止追い番
	p_NtDat->PayMethod = p_RcptDat->PayMethod;							// 精算方法
	p_NtDat->PayClass = p_RcptDat->PayClass;							// 処理区分
	p_NtDat->PayMode = 0;												// 精算ﾓｰﾄﾞ(自動精算)
	p_NtDat->LockNo = p_RcptDat->WPlace;								// 区画情報
	p_NtDat->CardType = 0;												// 駐車券ﾀｲﾌﾟ
	p_NtDat->CMachineNo = 0;											// 駐車券機械№
	p_NtDat->CardNo = 0L;												// 駐車券番号(発券追い番)
	p_NtDat->OutTime.Year = p_RcptDat->TOutTime.Year;					// 出庫年
	p_NtDat->OutTime.Mon = p_RcptDat->TOutTime.Mon;						// 出庫月
	p_NtDat->OutTime.Day = p_RcptDat->TOutTime.Day;						// 出庫日
	p_NtDat->OutTime.Hour = p_RcptDat->TOutTime.Hour;					// 出庫時
	p_NtDat->OutTime.Min = p_RcptDat->TOutTime.Min;						// 出庫分
	p_NtDat->OutTime.Sec = 0;											// 出庫秒
	p_NtDat->KakariNo = p_RcptDat->KakariNo;							// 係員№
	p_NtDat->OutKind = p_RcptDat->OutKind;								// 精算出庫
	p_NtDat->InTime.Year = p_RcptDat->TInTime.Year;						// 入庫年
	p_NtDat->InTime.Mon = p_RcptDat->TInTime.Mon;						// 入庫月
	p_NtDat->InTime.Day = p_RcptDat->TInTime.Day;						// 入庫日
	p_NtDat->InTime.Hour = p_RcptDat->TInTime.Hour;						// 入庫時
	p_NtDat->InTime.Min = p_RcptDat->TInTime.Min;						// 入庫分
	p_NtDat->InTime.Sec = 0;											// 入庫秒
	// 精算年月日時分秒は0とする
	// 前回精算年月日時分秒は0とする
	p_NtDat->TaxPrice = 0;												// 課税対象額
	p_NtDat->TotalPrice = 0;											// 合計金額(HOST未使用のため)
	p_NtDat->Tax = p_RcptDat->Wtax;										// 消費税額
	p_NtDat->Syubet = p_RcptDat->syu;									// 料金種別
	p_NtDat->Price = p_RcptDat->WPrice;									// 駐車料金
	p_NtDat->PassCheck = 1;												// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	p_NtDat->CountSet = p_RcptDat->CountSet;							// 在車ｶｳﾝﾄしない

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	ret = sizeof( DATA_KIND_22 ) - sizeof( DISCOUNT_DATA )*NTNET_DIC_MAX;// 割引データまでのサイズ算出
	ret = sizeof(DATA_KIND_22) - sizeof(t_SeisanDiscountOld) * NTNET_DIC_MAX;	// 割引データまでのサイズ算出
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]

	return ret;
}
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス(RT精算データ対応)
////[]----------------------------------------------------------------------[]
/////	@brief			入庫ログデータ作成(LOGに保存する形式)
////[]----------------------------------------------------------------------[]
/////	@param[in]		pr_lokno : 内部処理用駐車位置番号
/////	@return			woid	: 
/////	@author			A.iiizumi
/////	@attention		
/////	@note			
////[]----------------------------------------------------------------------[]
/////	@date			Create	: 2012/02/15<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//void	Make_Log_Enter( unsigned short pr_lokno)
//{
//	ushort	lkno;
//	int i, j;
//	ushort curnum = 0;
//	uchar uc_prm;
//	ushort curnum_1 = 0;
//	ushort curnum_2 = 0;
//	ushort curnum_3 = 0;
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//	date_time_rec	wk_CLK_REC;
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//
//	lkno = pr_lokno - 1;
//
//	memset( &Enter_data, 0, sizeof( Enter_data ) );
//
//	Enter_data.CMN_DT.DT_54.ID = 20;
//	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
//		Enter_data.CMN_DT.DT_54.ID = 54;
//	}
//
//	Enter_data.InCount = NTNetDataCont[0];											// 入庫追い番
//	NTNetDataCont[0]++;																// 入庫追い番+1
//
//	Enter_data.Syubet = LockInfo[lkno].ryo_syu;										// 処理区分
//
//	if( FLAPDT.flp_data[lkno].lag_to_in.BIT.SYUUS == 1 ){							// 修正精算の時
//		Enter_data.InMode = 6;														// 入庫ﾓｰﾄﾞ：修正精算入庫
//	}else{
//		if(FLAPDT.flp_data[lkno].lag_to_in.BIT.FUKUG != 0){							// 入庫ﾓｰﾄﾞ
//			Enter_data.InMode = 5;
//		} else {
//			Enter_data.InMode = 0;
//		}
//	}
//
//	Enter_data.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
//								+ LockInfo[lkno].posi );							// 区画情報
//	Enter_data.CardType = 0;														// 駐車券ﾀｲﾌﾟ(未使用)
//	Enter_data.CMachineNo = 0;														// 駐車券機械№
//	Enter_data.CardNo = 0L;															// 駐車券番号(発券追い番)
//
//	Enter_data.InTime.Year = FLAPDT.flp_data[lkno].year;							// 入庫年
//	Enter_data.InTime.Mon = FLAPDT.flp_data[lkno].mont;								// 入庫月
//	Enter_data.InTime.Day = FLAPDT.flp_data[lkno].date;								// 入庫日
//	Enter_data.InTime.Hour = FLAPDT.flp_data[lkno].hour;							// 入庫時
//	Enter_data.InTime.Min = FLAPDT.flp_data[lkno].minu;								// 入庫分
//	Enter_data.InTime.Sec = 0;														// 入庫秒
//	// 処理年月日をここで入れる
//	Enter_data.CMN_DT.DT_54.ProcDate.Year	= (uchar)( CLK_REC.year % 100 );		// 年
//	Enter_data.CMN_DT.DT_54.ProcDate.Mon	= (uchar)CLK_REC.mont;					// 月
//	Enter_data.CMN_DT.DT_54.ProcDate.Day	= (uchar)CLK_REC.date;					// 日
//	Enter_data.CMN_DT.DT_54.ProcDate.Hour	= (uchar)CLK_REC.hour;					// 時
//	Enter_data.CMN_DT.DT_54.ProcDate.Min	= (uchar)CLK_REC.minu;					// 分
//	Enter_data.CMN_DT.DT_54.ProcDate.Sec	= (uchar)CLK_REC.seco;					// 秒
//	Enter_data.PassCheck = 1;														// ｱﾝﾁﾊﾟｽﾁｪｯｸしない
//	if( prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[lkno].ryo_syu-1)*6)),1,1 ) ){	// 種別毎ｶｳﾝﾄする設定?
//		Enter_data.CountSet = 0;													// 在車ｶｳﾝﾄ(する)
//	}else{
//		Enter_data.CountSet = 1;													// 在車ｶｳﾝﾄ(しない)
//	}
//	// 定期券ﾃﾞｰﾀは0固定
//	Enter_data.CMN_DT.DT_54.PascarCnt = 0;		// 定期車両カウント
//	if (Enter_data.CMN_DT.DT_54.ID != 54) {		// 20/54
//		return;
//	}
//	for (i = 0; i < LOCK_MAX; i++) {
//		// 料金種別毎に現在在車ｶｳﾝﾄ
//		if (FLAPDT.flp_data[i].nstat.bits.b00) {
//			j = LockInfo[i].ryo_syu - 1;
//			if (j >= 0 && j < 12) {
//				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// 種別毎ｶｳﾝﾄする設定get
//				if( uc_prm == 1 ){			// 現在台数１をカウント
//					curnum++;
//					curnum_1++;
//				}
//				else if( uc_prm == 2 ){		// 現在台数２をカウント
//					curnum++;
//					curnum_2++;
//				}
//				else if( uc_prm == 3 ){		// 現在台数３をカウント
//					curnum++;
//					curnum_3++;
//				}
//			}
//		}
//	}
//	// 定期車両カウント 0固定
//	if (_is_ntnet_remote()) {												// 遠隔NT-NET設定のときのみデータセット
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039⑤駐車台数を本体で管理する
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039⑥用途別駐車台数切替なし
//				Enter_data.CMN_DT.DT_54.FullNo1 = (ushort)PPrmSS[S_P02][3];	// 満車台数
//				Enter_data.CMN_DT.DT_54.CarCnt1 = curnum;					// 現在台数
//				switch (PPrmSS[S_P02][1]) {									// 強制満空車ﾓｰﾄﾞ
//				case	1:													// 強制満車
//					Enter_data.CMN_DT.DT_54.Full[0] = 11;					// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					Enter_data.CMN_DT.DT_54.Full[0] = 10;					// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum >= PPrmSS[S_P02][3]) {
//						Enter_data.CMN_DT.DT_54.Full[0] = 1;				// 「満車」状態セット
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039⑥用途別駐車台数２系統
//			case 4:															// 01-0039⑥用途別駐車台数３系統
//				Enter_data.CMN_DT.DT_54.FullNo1 = (ushort)PPrmSS[S_P02][7];	// 満車台数
//				Enter_data.CMN_DT.DT_54.CarCnt1 = curnum_1;					// 現在台数１
//				switch (PPrmSS[S_P02][5]) {									// 強制満空車モード1
//				case	1:													// 強制満車
//					Enter_data.CMN_DT.DT_54.Full[0] = 11;					// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					Enter_data.CMN_DT.DT_54.Full[0] = 10;					// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						Enter_data.CMN_DT.DT_54.Full[0] = 1;				// 「満車」状態セット
//					}
//					break;
//				}
//				Enter_data.CMN_DT.DT_54.FullNo2 = (ushort)PPrmSS[S_P02][11];// 満車台数２
//				Enter_data.CMN_DT.DT_54.CarCnt2 = curnum_2;					// 現在台数２
//				switch (PPrmSS[S_P02][9]) {									// 強制満空車モード2
//				case	1:													// 強制満車
//					Enter_data.CMN_DT.DT_54.Full[1] = 11;					// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					Enter_data.CMN_DT.DT_54.Full[1] = 10;					// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						Enter_data.CMN_DT.DT_54.Full[1] = 1;				// 「満車」状態セット
//					}
//					break;
//				}
//				if (i == 4) {
//					Enter_data.CMN_DT.DT_54.FullNo3 = (ushort)PPrmSS[S_P02][15];// 満車台数３
//					Enter_data.CMN_DT.DT_54.CarCnt3 = curnum_3;					// 現在台数３
//					switch (PPrmSS[S_P02][13]) {							// 強制満空車モード3
//					case	1:												// 強制満車
//						Enter_data.CMN_DT.DT_54.Full[2] = 11;				// 「強制満車」状態セット
//						break;
//					case	2:												// 強制空車
//						Enter_data.CMN_DT.DT_54.Full[2] = 10;				// 「強制空車」状態セット
//						break;
//					default:												// 自動
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							Enter_data.CMN_DT.DT_54.Full[2] = 1;			// 「満車」状態セット
//						}
//						break;
//					}
//				}
//				break;
//			default:
//				break;
//			}
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//			// 台数管理追番 ------------------------
//			memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
//			// 同一時分？
//			if( GetVehicleCountDate() == Nrm_YMDHM( &wk_CLK_REC ) ){
//				// 同一時分の場合は追い番を＋１
//				AddVehicleCountSeqNo();
//			}else{
//				// 異なる時分の場合は追い番を０とする
//				SetVehicleCountDate( Nrm_YMDHM( &wk_CLK_REC ) );
//				ClrVehicleCountSeqNo();
//			}
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntYear	= (wk_CLK_REC.Year % 100);	// 年
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntMon	= wk_CLK_REC.Mon;			// 月
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntDay	= wk_CLK_REC.Day;			// 日
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntHour	= wk_CLK_REC.Hour;			// 時
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntMin	= wk_CLK_REC.Min;			// 分
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntSeq	= GetVehicleCountSeqNo();	// 追番
//			Enter_data.CMN_DT.DT_54.CarCntInfo.Reserve1		= 0;						// 予備
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//		}
//	}
//	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//		Enter_data.CMN_DT.DT_54.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_ENTER);	// センター追番（入庫）
//		RAU_UpdateCenterSeqNo(RAU_SEQNO_ENTER);										// センター追番更新（入庫）
//	}
//}
////[]----------------------------------------------------------------------[]
/////	@brief			入庫ログデータ作成(LOGに保存する形式)振替精算時の仮入庫用
////[]----------------------------------------------------------------------[]
/////	@param[in]		pr_lokno : 内部処理用駐車位置番号
/////	@param[in]		*data : flp_com型のポインタ
/////	@return			woid	: 
/////	@author			A.iiizumi
/////	@attention		
/////	@note			
////[]----------------------------------------------------------------------[]
/////	@date			Create	: 2012/02/15<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//void	Make_Log_Enter_frs( unsigned short pr_lokno, void *data )
//{
//	ushort	lkno;
//	flp_com *flp_data = (flp_com*)data;
//	int i, j;
//	ushort curnum = 0;
//	uchar uc_prm;
//	ushort curnum_1 = 0;
//	ushort curnum_2 = 0;
//	ushort curnum_3 = 0;
//
//	lkno = pr_lokno - 1;
//
//	memset( &Enter_data, 0, sizeof( Enter_data ) );
//
//	Enter_data.CMN_DT.DT_54.ID = 20;
//	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
//		Enter_data.CMN_DT.DT_54.ID = 54;
//	}
//
//	Enter_data.InCount = NTNetDataCont[0];											// 入庫追い番
//	NTNetDataCont[0]++;																// 入庫追い番+1
//
//	Enter_data.Syubet = LockInfo[lkno].ryo_syu;										// 処理区分
//
//	if(flp_data->lag_to_in.BIT.FUKUG != 0){											// 入庫ﾓｰﾄﾞ
//		Enter_data.InMode = 5;
//	} else {
//		Enter_data.InMode = 0;
//	}
//
//	Enter_data.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
//								+ LockInfo[lkno].posi );							// 区画情報
//	Enter_data.CardType = 0;														// 駐車券ﾀｲﾌﾟ(未使用)
//	Enter_data.CMachineNo = 0;														// 駐車券機械№
//	Enter_data.CardNo = 0L;															// 駐車券番号(発券追い番)
//	Enter_data.InTime.Year = flp_data->year;										// 入庫年
//	Enter_data.InTime.Mon = flp_data->mont;											// 入庫月
//	Enter_data.InTime.Day = flp_data->date;											// 入庫日
//	Enter_data.InTime.Hour = flp_data->hour;										// 入庫時
//	Enter_data.InTime.Min = flp_data->minu;											// 入庫分
//	Enter_data.InTime.Sec = 0;														// 入庫秒
//	// 処理年月日をここで入れる
//	Enter_data.CMN_DT.DT_54.ProcDate.Year	= (uchar)( CLK_REC.year % 100 );		// 年
//	Enter_data.CMN_DT.DT_54.ProcDate.Mon	= (uchar)CLK_REC.mont;					// 月
//	Enter_data.CMN_DT.DT_54.ProcDate.Day	= (uchar)CLK_REC.date;					// 日
//	Enter_data.CMN_DT.DT_54.ProcDate.Hour	= (uchar)CLK_REC.hour;					// 時
//	Enter_data.CMN_DT.DT_54.ProcDate.Min	= (uchar)CLK_REC.minu;					// 分
//	Enter_data.CMN_DT.DT_54.ProcDate.Sec	= (uchar)CLK_REC.seco;					// 秒
//	Enter_data.PassCheck = 1;														// ｱﾝﾁﾊﾟｽﾁｪｯｸしない
//	if( prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[lkno].ryo_syu-1)*6)),1,1 ) ){	// 種別毎ｶｳﾝﾄする設定?
//		Enter_data.CountSet = 0;													// 在車ｶｳﾝﾄ(する)
//	}else{
//		Enter_data.CountSet = 1;													// 在車ｶｳﾝﾄ(しない)
//	}
//	// 定期券ﾃﾞｰﾀは0固定
//	Enter_data.CMN_DT.DT_54.PascarCnt = 0;		// 定期車両カウント
//	if (Enter_data.CMN_DT.DT_54.ID != 54) {		// 20/54
//		return;
//	}
//	for (i = 0; i < LOCK_MAX; i++) {
//		// 料金種別毎に現在在車ｶｳﾝﾄ
//		if (FLAPDT.flp_data[i].nstat.bits.b00) {
//			j = LockInfo[i].ryo_syu - 1;
//			if (j >= 0 && j < 12) {
//				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// 種別毎ｶｳﾝﾄする設定get
//				if( uc_prm == 1 ){			// 現在台数１をカウント
//					curnum++;
//					curnum_1++;
//				}
//				else if( uc_prm == 2 ){		// 現在台数２をカウント
//					curnum++;
//					curnum_2++;
//				}
//				else if( uc_prm == 3 ){		// 現在台数３をカウント
//					curnum++;
//					curnum_3++;
//				}
//			}
//		}
//	}
//	// 定期車両カウント 0固定
//	if (_is_ntnet_remote()) {												// 遠隔NT-NET設定のときのみデータセット
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039⑤駐車台数を本体で管理する
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039⑥用途別駐車台数切替なし
//				Enter_data.CMN_DT.DT_54.FullNo1 = (ushort)PPrmSS[S_P02][3];	// 満車台数
//				Enter_data.CMN_DT.DT_54.CarCnt1 = curnum;					// 現在台数
//				switch (PPrmSS[S_P02][1]) {									// 強制満空車ﾓｰﾄﾞ
//				case	1:													// 強制満車
//					Enter_data.CMN_DT.DT_54.Full[0] = 11;					// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					Enter_data.CMN_DT.DT_54.Full[0] = 10;					// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum >= PPrmSS[S_P02][3]) {
//						Enter_data.CMN_DT.DT_54.Full[0] = 1;				// 「満車」状態セット
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039⑥用途別駐車台数２系統
//			case 4:															// 01-0039⑥用途別駐車台数３系統
//				Enter_data.CMN_DT.DT_54.FullNo1 = (ushort)PPrmSS[S_P02][7];	// 満車台数
//				Enter_data.CMN_DT.DT_54.CarCnt1 = curnum_1;					// 現在台数１
//				switch (PPrmSS[S_P02][5]) {									// 強制満空車モード1
//				case	1:													// 強制満車
//					Enter_data.CMN_DT.DT_54.Full[0] = 11;					// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					Enter_data.CMN_DT.DT_54.Full[0] = 10;					// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						Enter_data.CMN_DT.DT_54.Full[0] = 1;				// 「満車」状態セット
//					}
//					break;
//				}
//				Enter_data.CMN_DT.DT_54.FullNo2 = (ushort)PPrmSS[S_P02][11];// 満車台数２
//				Enter_data.CMN_DT.DT_54.CarCnt2 = curnum_2;					// 現在台数２
//				switch (PPrmSS[S_P02][9]) {									// 強制満空車モード2
//				case	1:													// 強制満車
//					Enter_data.CMN_DT.DT_54.Full[1] = 11;					// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					Enter_data.CMN_DT.DT_54.Full[1] = 10;					// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						Enter_data.CMN_DT.DT_54.Full[1] = 1;				// 「満車」状態セット
//					}
//					break;
//				}
//				if (i == 4) {
//					Enter_data.CMN_DT.DT_54.FullNo3 = (ushort)PPrmSS[S_P02][15];// 満車台数３
//					Enter_data.CMN_DT.DT_54.CarCnt3 = curnum_3;					// 現在台数３
//					switch (PPrmSS[S_P02][13]) {							// 強制満空車モード3
//					case	1:												// 強制満車
//						Enter_data.CMN_DT.DT_54.Full[2] = 11;				// 「強制満車」状態セット
//						break;
//					case	2:												// 強制空車
//						Enter_data.CMN_DT.DT_54.Full[2] = 10;				// 「強制空車」状態セット
//						break;
//					default:												// 自動
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							Enter_data.CMN_DT.DT_54.Full[2] = 1;			// 「満車」状態セット
//						}
//						break;
//					}
//				}
//				break;
//			default:
//				break;
//			}
//		}
//	}
//	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//		Enter_data.CMN_DT.DT_54.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_ENTER);	// センター追番（入庫）
//		RAU_UpdateCenterSeqNo(RAU_SEQNO_ENTER);										// センター追番更新（入庫）
//	}
//}
////[]----------------------------------------------------------------------[]
/////	@brief			入庫データ(データ種別20)編集処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		p_RcptDat : ログから取り出した入庫データのポインタ
/////	@param[out]		p_NtDat   : 入庫データ(DATA_KIND_20型)へのポインタ  
/////	@return			ret       : 入庫データのデータサイズ(システムID～) 
/////	@author			A.iiizumi
/////	@attention		
/////	@note			
////[]----------------------------------------------------------------------[]
/////	@date			Create	: 2012/02/15<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//unsigned short	NTNET_Edit_Data20( enter_log *p_RcptDat, DATA_KIND_20 *p_NtDat )
//{
//	unsigned short ret;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_20 ) );
//	NTNET_Edit_BasicData( 20, 0, p_RcptDat->CMN_DT.DT_54.SeqNo, &p_NtDat->DataBasic);	// 基本データ作成
//
//	memcpy(&p_NtDat->InCount, p_RcptDat, sizeof( enter_log ) - sizeof(p_RcptDat->CMN_DT));
//
//	// 処理年月日をセット
//	p_NtDat->DataBasic.Year	= p_RcptDat->CMN_DT.DT_54.ProcDate.Year;				// 処理年
//	p_NtDat->DataBasic.Mon	= p_RcptDat->CMN_DT.DT_54.ProcDate.Mon;					// 処理月
//	p_NtDat->DataBasic.Day	= p_RcptDat->CMN_DT.DT_54.ProcDate.Day;					// 処理日
//	p_NtDat->DataBasic.Hour	= p_RcptDat->CMN_DT.DT_54.ProcDate.Hour;				// 処理時
//	p_NtDat->DataBasic.Min	= p_RcptDat->CMN_DT.DT_54.ProcDate.Min;					// 処理分
//	p_NtDat->DataBasic.Sec	= p_RcptDat->CMN_DT.DT_54.ProcDate.Sec;					// 処理秒
//
//	ret = sizeof( DATA_KIND_20 ); 
//	return ret;
//}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し 2019/11/15 車番チケットレス(RT精算データ対応)
//[]----------------------------------------------------------------------[]
///	@brief			エラーデータ(データ種別120)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出したエラーデータのポインタ
///	@param[out]		p_NtDat   : エラーデータ(DATA_KIND_120型)へのポインタ  
///	@return			ret       : エラーデータのデータサイズ(システムID～) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data120( Err_log *p_RcptDat, DATA_KIND_120 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_120 ) );
	NTNET_Edit_BasicData( 120, 0, p_RcptDat->ErrSeqNo, &p_NtDat->DataBasic);		// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;														// 処理秒

	p_NtDat->Errsyu = p_RcptDat->Errsyu;											// ｴﾗｰ種別
	p_NtDat->Errcod = p_RcptDat->Errcod;											// ｴﾗｰｺｰﾄﾞ

	switch( p_RcptDat->Errdtc ){
	case 1:																			// 発生
		p_NtDat->Errdtc = 1;														// ｴﾗｰ発生
		break;
	case 2:																			// 発生・解除
		p_NtDat->Errdtc = 3;														// ｴﾗｰ発生・解除
		break;
	default:																		// 解除
		p_NtDat->Errdtc = 2;														// ｴﾗｰ解除
		break;
	}

	p_NtDat->Errlev = p_RcptDat->Errlev;											// ｴﾗｰﾚﾍﾞﾙ

	if( p_RcptDat->Errinf == 2 ){													// ｴﾗｰ情報有無(bin)
		memcpy( &p_NtDat->Errdat1[6],
				&p_RcptDat->ErrBinDat,
				4 );																// ｴﾗｰ情報
	}else if( p_RcptDat->Errinf ==1 ){												//ｴﾗｰ情報が「有り」でアスキーなら
		strncpy( (char*)&p_NtDat->Errdat2, (char*)&p_RcptDat->Errdat,sizeof( p_NtDat->Errdat2 )  );
	}

	ret = sizeof( DATA_KIND_120 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			アラームデータ(データ種別121)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出したアラームデータのポインタ
///	@param[out]		p_NtDat   : アラームデータ(DATA_KIND_121型)へのポインタ  
///	@return			ret       : アラームデータのデータサイズ(システムID～) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data121( Arm_log *p_RcptDat, DATA_KIND_121 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_121 ) );
	NTNET_Edit_BasicData( 121, 0, p_RcptDat->ArmSeqNo, &p_NtDat->DataBasic);		// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;														// 処理秒

	p_NtDat->Armsyu = p_RcptDat->Armsyu;											// ｱﾗｰﾑ種別
	p_NtDat->Armcod = p_RcptDat->Armcod;											// ｱﾗｰﾑｺｰﾄﾞ
	switch( p_RcptDat->Armdtc ){
	case 1:																			// 発生
		p_NtDat->Armdtc = 1;														// ｱﾗｰﾑ発生
		break;
	case 2:																			// 発生・解除
		p_NtDat->Armdtc = 3;														// ｱﾗｰﾑ発生・解除
		break;
	default:																		// 解除
		p_NtDat->Armdtc = 2;														// ｱﾗｰﾑ解除
		break;
	}
	p_NtDat->Armlev = p_RcptDat->Armlev;											// ｱﾗｰﾑﾚﾍﾞﾙ


	// ｱﾗｰﾑ情報付き電文作成 
	if( 2 == p_RcptDat->Arminf ){													// ﾊﾞｲﾅﾘﾃﾞｰﾀあり
		memcpy(	&p_NtDat->Armdat1[6], 
				&p_RcptDat->ArmBinDat , 4 );
	}else if( 1 == p_RcptDat->Arminf ){												// asciiﾃﾞｰﾀあり
		if(p_RcptDat->Armsyu == 2){
			if( ( (p_RcptDat->Armcod >= 40) && (p_RcptDat->Armcod <= 49) )
			 || (p_RcptDat->Armcod == 54)){
				memcpy(	&p_NtDat->Armdat1[4], 
					&p_RcptDat->Armdat[0] , 6 );
			}
		}
	}

	ret = sizeof( DATA_KIND_121 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			エラーデータ(データ種別63)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出したエラーデータのポインタ
///	@param[out]		p_NtDat   : エラーデータ(DATA_KIND_63型)へのポインタ  
///	@return			ret       : エラーデータのデータサイズ(システムID～) 
///	@author			t.hashimoto
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2013/02/27<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data63( Err_log *p_RcptDat, DATA_KIND_63 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_63 ) );
	NTNET_Edit_BasicData( 63, 0, p_RcptDat->ErrSeqNo, &p_NtDat->DataBasic);			// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;														// 処理秒

	p_NtDat->FmtRev = 10;															// ﾌｫｰﾏｯﾄRev.№
	p_NtDat->Errsyu = p_RcptDat->Errsyu;											// ｴﾗｰ種別
	p_NtDat->Errcod = p_RcptDat->Errcod;											// ｴﾗｰｺｰﾄﾞ

	switch( p_RcptDat->Errdtc ){
	case 1:																			// 発生
		p_NtDat->Errdtc = 1;														// ｴﾗｰ発生
		break;
	case 2:																			// 発生・解除
		p_NtDat->Errdtc = 3;														// ｴﾗｰ発生・解除
		break;
	default:																		// 解除
		p_NtDat->Errdtc = 2;														// ｴﾗｰ解除
		break;
	}

	p_NtDat->Errlev = p_RcptDat->Errlev;											// ｴﾗｰﾚﾍﾞﾙ
	if( p_RcptDat->ErrDoor & ERR_LOG_DOOR_STS_F ){									// ドア閉,1=開
		p_NtDat->ErrDoor = 1;
	}
	if( p_RcptDat->Errinf == 2 ){													// ｴﾗｰ情報有無(bin)
		memcpy( &p_NtDat->Errdat[6],
				&p_RcptDat->ErrBinDat,
				4 );																// ｴﾗｰ情報
	}

	ret = sizeof( DATA_KIND_63 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			アラームデータ(データ種別64)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出したアラームデータのポインタ
///	@param[out]		p_NtDat   : アラームデータ(DATA_KIND_64型)へのポインタ  
///	@return			ret       : アラームデータのデータサイズ(システムID～) 
///	@author			t.hashimoto
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data64( Arm_log *p_RcptDat, DATA_KIND_64 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_64 ) );
	NTNET_Edit_BasicData( 64, 0, p_RcptDat->ArmSeqNo, &p_NtDat->DataBasic);			// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;														// 処理秒

	p_NtDat->FmtRev = 10;															// ﾌｫｰﾏｯﾄRev.№
	p_NtDat->Armsyu = p_RcptDat->Armsyu;											// ｱﾗｰﾑ種別
	p_NtDat->Armcod = p_RcptDat->Armcod;											// ｱﾗｰﾑｺｰﾄﾞ
	switch( p_RcptDat->Armdtc ){
	case 1:																			// 発生
		p_NtDat->Armdtc = 1;														// ｱﾗｰﾑ発生
		break;
	case 2:																			// 発生・解除
		p_NtDat->Armdtc = 3;														// ｱﾗｰﾑ発生・解除
		break;
	default:																		// 解除
		p_NtDat->Armdtc = 2;														// ｱﾗｰﾑ解除
		break;
	}
	p_NtDat->Armlev = p_RcptDat->Armlev;											// ｱﾗｰﾑﾚﾍﾞﾙ
	if( p_RcptDat->ArmDoor & ERR_LOG_DOOR_STS_F ){									// ドア閉,1=開
		p_NtDat->ArmDoor = 1;
	}

	// ｱﾗｰﾑ情報付き電文作成 
	if( 2 == p_RcptDat->Arminf ){													// ﾊﾞｲﾅﾘﾃﾞｰﾀあり
		memcpy(	&p_NtDat->Armdat[6], 
				&p_RcptDat->ArmBinDat , 4 );
	}

	ret = sizeof( DATA_KIND_64 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			モニタデータ(データ種別122)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出したモニタデータのポインタ
///	@param[out]		p_NtDat   : モニタデータ(DATA_KIND_122型)へのポインタ  
///	@return			ret       : モニタデータのデータサイズ(システムID～) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data122( Mon_log *p_RcptDat, DATA_KIND_122 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_122 ) );
	NTNET_Edit_BasicData( 122, 0, p_RcptDat->MonSeqNo, &p_NtDat->DataBasic);		// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;														// 処理秒

	p_NtDat->Monsyu = p_RcptDat->MonKind;											// モニタ種別
	p_NtDat->Moncod = p_RcptDat->MonCode;											// モニタコード
	p_NtDat->Monlev = p_RcptDat->MonLevel;											// モニタレベル
	// モニタ情報
	memcpy( &p_NtDat->Mondat1, &p_RcptDat->MonInfo[0], sizeof(p_NtDat->Mondat1));

	ret = sizeof( DATA_KIND_122 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			操作モニタデータ(データ種別123)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出した操作モニタデータのポインタ
///	@param[out]		p_NtDat   : 操作モニタデータ(DATA_KIND_123型)へのポインタ  
///	@return			ret       : 操作モニタデータのデータサイズ(システムID～) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data123( Ope_log *p_RcptDat, DATA_KIND_123 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_123 ) );
	NTNET_Edit_BasicData( 123, 0, p_RcptDat->OpeSeqNo, &p_NtDat->DataBasic);		// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;														// 処理秒

	p_NtDat->OpeMonsyu = p_RcptDat->OpeKind;										// 操作モニタ種別
	p_NtDat->OpeMoncod = p_RcptDat->OpeCode;										// 操作モニタコード
	p_NtDat->OpeMonlev = p_RcptDat->OpeLevel;										// 操作モニタレベル

	memcpy( &p_NtDat->OpeMondat1, &p_RcptDat->OpeBefore, sizeof(p_NtDat->OpeMondat1));
	memcpy( &p_NtDat->OpeMondat2, &p_RcptDat->OpeAfter, sizeof(p_NtDat->OpeMondat2));

	ret = sizeof( DATA_KIND_123 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			コイン金庫集計データ(データ種別131)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出したコイン金庫集計データのポインタ
///	@param[out]		p_NtDat   : コイン金庫データ(DATA_KIND_130型)へのポインタ  
///	@return			ret       : コイン金庫データのデータサイズ(システムID～) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data131( COIN_SYU *p_RcptDat, DATA_KIND_130 *p_NtDat )
{
	unsigned short ret,i;

	memset( p_NtDat, 0, sizeof( DATA_KIND_130 ) );
	NTNET_Edit_BasicData( 131, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);			// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->NowTime.Mon;							// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->NowTime.Day;							// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->NowTime.Min;							// 処理分
	p_NtDat->DataBasic.Sec = 0;														// 処理秒

	p_NtDat->KakariNo = p_RcptDat->Kakari_no;										// 係員№
	p_NtDat->SeqNo = CountSel( &p_RcptDat->Oiban );									// 追番

	// タイプ設定
	p_NtDat->Type = 51;																// 合計(51)
	
	p_NtDat->Sf.Coin[0].Money = 10;													// ｺｲﾝ金庫金種(10円)
	p_NtDat->Sf.Coin[1].Money = 50;													// ｺｲﾝ金庫金種(50円)
	p_NtDat->Sf.Coin[2].Money = 100;												// ｺｲﾝ金庫金種(100円)
	p_NtDat->Sf.Coin[3].Money = 500;												// ｺｲﾝ金庫金種(500円)

	p_NtDat->Sf.Coin[0].Mai = p_RcptDat->cnt[0];									// ｺｲﾝ金庫枚数(10円)
	p_NtDat->Sf.Coin[1].Mai = p_RcptDat->cnt[1];									// ｺｲﾝ金庫枚数(50円)
	p_NtDat->Sf.Coin[2].Mai = p_RcptDat->cnt[2];									// ｺｲﾝ金庫枚数(100円)
	p_NtDat->Sf.Coin[3].Mai = p_RcptDat->cnt[3];									// ｺｲﾝ金庫枚数(500円)

	for( i=0; i<4; i++ ){
// MH322914 (s) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
//		p_NtDat->SfTotal += p_NtDat->Sf.Coin[i].Mai * p_NtDat->Sf.Coin[i].Money;	// ｺｲﾝ金庫総額
		p_NtDat->SfTotal += (ulong)p_NtDat->Sf.Coin[i].Mai * (ulong)p_NtDat->Sf.Coin[i].Money;	// ｺｲﾝ金庫総額
// MH322914 (e) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
	}

	ret = sizeof( DATA_KIND_130 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			紙幣金庫集計データ(データ種別133)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出した紙幣金庫合計データのポインタ
///	@param[out]		p_NtDat   : 紙幣金庫データ(DATA_KIND_132型)へのポインタ  
///	@return			ret       : 紙幣金庫データのデータサイズ(システムID～) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data133( NOTE_SYU *p_RcptDat, DATA_KIND_132 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_132 ) );
	NTNET_Edit_BasicData( 133, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);			// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->NowTime.Mon;							// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->NowTime.Day;							// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->NowTime.Min;							// 処理分
	p_NtDat->DataBasic.Sec = 0;														// 処理秒
	p_NtDat->KakariNo = p_RcptDat->Kakari_no;										// 係員№
	p_NtDat->SeqNo = CountSel( &p_RcptDat->Oiban );									// 追番

	// タイプ設定
	p_NtDat->Type = 52;																// 合計(52)

	p_NtDat->Sf.Note[0].Money = 1000;												// 紙幣金庫金種(1000円)
	p_NtDat->Sf.Note[0].Mai = p_RcptDat->cnt[0];									// 紙幣金庫枚数(1000円)
// MH322914 (s) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
//	p_NtDat->SfTotal = p_NtDat->Sf.Note[0].Mai * p_NtDat->Sf.Note[0].Money;			// 紙幣金庫総額
	p_NtDat->SfTotal = (ulong)p_NtDat->Sf.Note[0].Mai * (ulong)p_NtDat->Sf.Note[0].Money;			// 紙幣金庫総額
// MH322914 (e) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)

	ret = sizeof( DATA_KIND_132 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			駐車台数データログ作成(LOGに保存する形式)
//[]----------------------------------------------------------------------[]
///	@param[in]		void    : 
///	@return			woid    : 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Make_Log_ParkCarNum( void )
{

	int i, j;
	char full;
	ulong curnum = 0;
	uchar uc_prm;
	ushort curnum_1 = 0;
	ushort curnum_2 = 0;
	ushort curnum_3 = 0;

	memset( &ParkCar_data, 0, sizeof( ParkCar_data ) );

	ParkCar_data.CMN_DT.DT_58.ID = 236;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		ParkCar_data.CMN_DT.DT_58.ID = 58;
	}

	ParkCar_data.Time.Year = (uchar)( CLK_REC.year % 100 );		// 年
	ParkCar_data.Time.Mon = (uchar)CLK_REC.mont;				// 月
	ParkCar_data.Time.Day = (uchar)CLK_REC.date;				// 日
	ParkCar_data.Time.Hour = (uchar)CLK_REC.hour;				// 時
	ParkCar_data.Time.Min = (uchar)CLK_REC.minu;				// 分
	ParkCar_data.Time.Sec = (uchar)CLK_REC.seco;				// 秒

	for (i = 0; i < LOCK_MAX; i++) {
		// 料金種別毎に現在在車ｶｳﾝﾄ
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {
				ParkCar_data.RyCurNum[j]++;
				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// 種別毎ｶｳﾝﾄする設定get
				if( uc_prm == 1 ){			// 現在台数１をカウント
					curnum++;
					curnum_1++;
				}
				else if( uc_prm == 2 ){		// 現在台数２をカウント
					curnum++;
					curnum_2++;
				}
				else if( uc_prm == 3 ){		// 現在台数３をカウント
					curnum++;
					curnum_3++;
				}
			}
		}
	}

	// 現在駐車台数
	ParkCar_data.CurNum = curnum;

	// 用途別駐車台数設定
	ParkCar_data.State = prm_get(COM_PRM,S_SYS,39,1,1);

	// 駐車1～3満空状態
	for (i = 0; i < 3; i++) {
		// 状態取得
		full = getFullFactor((uchar)i);
		if(ParkCar_data.CMN_DT.DT_58.ID == 236) {
			if (full & 0x10) {
				// 満車
				ParkCar_data.Full[i] = 1;
			} else {
				// 空車
				ParkCar_data.Full[i] = 0;
			}
		}
		else {										// 新フォーマット(ID:58)
			if (full == FORCE_FULL) {				// 強制満車
				ParkCar_data.Full[i] = 11;
			}
			else if(full == FORCE_VACANCY) {		// 強制空車
				ParkCar_data.Full[i] = 10;
			}
			else if (full & 0x10) {					// 満車
				ParkCar_data.Full[i] = 1;
			}
			else {									// 空車
				ParkCar_data.Full[i] = 0;
			}
		}
		// ※現状"2"(ほぼ満車)は未使用
	}
	ParkCar_data.CMN_DT.DT_58.FmtRev = 0;	// フォーマットRev.№
	ParkCar_data.GroupNo = 0;				// ｸﾞﾙｰﾌﾟ番号

	if (ParkCar_data.CMN_DT.DT_58.ID != 58) {		// 236/58
		return;
	}
	// 用途別駐車台数設定
	if (_is_ntnet_remote()) {											// 遠隔NT-NET設定のときのみデータセット
		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {					// 01-0039⑤駐車台数を本体で管理する
			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
			case 0:														// 01-0039⑥用途別駐車台数切替なし
				ParkCar_data.CMN_DT.DT_58.FullNo1 = (ushort)PPrmSS[S_P02][3];	// 満車台数
				if (curnum > PPrmSS[S_P02][3]) {
					ParkCar_data.CMN_DT.DT_58.EmptyNo1 = 0;						// 空車台数
				} else {
					ParkCar_data.CMN_DT.DT_58.EmptyNo1 = (ushort)PPrmSS[S_P02][3] - curnum;	// 空車台数(満車台数 - 現在駐車台数)
				}
//////////////////////////////////////////////////////////////////////////
//				switch (PPrmSS[S_P02][1]) {								// 強制満空車ﾓｰﾄﾞ
//				case	1:												// 強制満車
//					ParkCar_data.Full[0] = 11;							// 「強制満車」状態セット
//					break;
//				case	2:												// 強制空車
//					ParkCar_data.Full[0] = 10;							// 「強制空車」状態セット
//					break;
//				default:												// 自動
//					if( ParkCar_data.CMN_DT.DT_58.EmptyNo1 == 0 ){		// 空車が「０」の場合
//						ParkCar_data.Full[0] = 1;						// 「満車」状態セット
//					}
//					break;
//				}
//////////////////////////////////////////////////////////////////////////
				break;
			case 3:														// 01-0039⑥用途別駐車台数２系統
			case 4:														// 01-0039⑥用途別駐車台数３系統
				ParkCar_data.CMN_DT.DT_58.FullNo1 = (ushort)PPrmSS[S_P02][7];	// 満車台数１
				if (curnum_1 > PPrmSS[S_P02][7]) {
					ParkCar_data.CMN_DT.DT_58.EmptyNo1 = 0;						// 空車台数１
				} else {
					ParkCar_data.CMN_DT.DT_58.EmptyNo1 = (ushort)(PPrmSS[S_P02][7] - curnum_1);	// 空車台数１(満車台数 - 現在駐車台数)
				}
//////////////////////////////////////////////////////////////////////////
//				switch (PPrmSS[S_P02][5]) {								// 強制満空車モード1
//				case	1:												// 強制満車
//					ParkCar_data.Full[0] = 11;							// 「強制満車」状態セット
//					break;
//				case	2:												// 強制空車
//					ParkCar_data.Full[0] = 10;							// 「強制空車」状態セット
//					break;
//				default:												// 自動
//					if( ParkCar_data.CMN_DT.DT_58.EmptyNo1 == 0 ){		// 空車が「０」の場合
//						ParkCar_data.Full[0] = 1;						// 「満車」状態セット
//					}
//					break;
//				}
//////////////////////////////////////////////////////////////////////////
				ParkCar_data.CMN_DT.DT_58.FullNo2 = (ushort)PPrmSS[S_P02][11];	// 満車台数２
				if (curnum_2 > PPrmSS[S_P02][11]) {
					ParkCar_data.CMN_DT.DT_58.EmptyNo2 = 0;						// 空車台数２
				} else {
					ParkCar_data.CMN_DT.DT_58.EmptyNo2 = (ushort)(PPrmSS[S_P02][11] - curnum_2);	// 空車台数２(満車台数 - 現在駐車台数)
				}
//////////////////////////////////////////////////////////////////////////
//				switch (PPrmSS[S_P02][9]) {								// 強制満空車モード2
//				case	1:												// 強制満車
//					ParkCar_data.Full[1] = 11;							// 「強制満車」状態セット
//					break;
//				case	2:												// 強制空車
//					ParkCar_data.Full[1] = 10;							// 「強制空車」状態セット
//					break;
//				default:												// 自動
//					if( ParkCar_data.CMN_DT.DT_58.EmptyNo2 == 0 ){		// 空車が「０」の場合
//						ParkCar_data.Full[1] = 1;						// 「満車」状態セット
//					}
//					break;
//				}
//////////////////////////////////////////////////////////////////////////
				if (i == 4) {
					ParkCar_data.CMN_DT.DT_58.FullNo3 = (ushort)PPrmSS[S_P02][15];	// 満車台数３
					if (curnum_3 > PPrmSS[S_P02][15]) {
						ParkCar_data.CMN_DT.DT_58.EmptyNo3 = 0;						// 空車台数３
					} else {
						ParkCar_data.CMN_DT.DT_58.EmptyNo3 = (ushort)(PPrmSS[S_P02][15] - curnum_3);// 空車台数３(満車台数 - 現在駐車台数)
					}
//////////////////////////////////////////////////////////////////////////
//					switch (PPrmSS[S_P02][13]) {						// 強制満空車モード3
//					case	1:											// 強制満車
//						ParkCar_data.Full[2] = 11;						// 「強制満車」状態セット
//						break;
//					case	2:											// 強制空車
//						ParkCar_data.Full[2] = 10;						// 「強制空車」状態セット
//						break;
//					default:											// 自動
//						if( ParkCar_data.CMN_DT.DT_58.EmptyNo3 == 0 ){	// 空車が「０」の場合
//							ParkCar_data.Full[2] = 1;					// 「満車」状態セット
//						}
//						break;
//					}
//////////////////////////////////////////////////////////////////////////
				}
				break;
			default:
				break;
			}
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			駐車台数データ(データ種別236)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出した駐車台数データのポインタ
///	@param[out]		p_NtDat   : 駐車台数データ(DATA_KIND_236型)へのポインタ  
///	@return			ret       : 駐車台数データのデータサイズ(システムID～) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data236( ParkCar_log *p_RcptDat, DATA_KIND_236 *p_NtDat )
{
	unsigned short ret,i;

	memset( p_NtDat, 0, sizeof( DATA_KIND_236 ) );
	NTNET_Edit_BasicData( 236, 0, p_RcptDat->CMN_DT.DT_58.SeqNo, &p_NtDat->DataBasic);	// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Time.Year % 100);					// 処理年(ログ作成時に下2桁にしている)
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Time.Mon;							// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Time.Day;							// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Time.Hour;							// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Time.Min;							// 処理分
	p_NtDat->DataBasic.Sec = (uchar)p_RcptDat->Time.Sec;							// 処理秒

	p_NtDat->CurNum = p_RcptDat->CurNum;											// 現在駐車台数
	p_NtDat->State = p_RcptDat->State;												// 用途別駐車台数設定

	for(i = 0; i<20; i++){															// 料金種別1～20現在駐車台数
		p_NtDat->RyCurNum[i] =  p_RcptDat->RyCurNum[i];
	}

	for(i = 0; i<3; i++){															// 駐車1～3満空状態
		p_NtDat->Full[i] = p_RcptDat->Full[i];
	}

	ret = sizeof( DATA_KIND_236 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			NT-NET基本データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		knd : データ種別
///	@param[in]		knd : データ保持フラグ
///	@param[in]		seqNo : シーケンシャルNo.
///	@param[in]		basic : DATA_BASIC型のポインタ
///	@return			void  : 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void	NTNET_Edit_BasicData( uchar knd, uchar keep, uchar SeqNo, DATA_BASIC *basic)
{
	// ｼｽﾃﾑID
	basic->SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];
																					// 0:ﾌﾗｯﾌﾟｼｽﾃﾑ
																					// 1:遠隔ﾌﾗｯﾌﾟｼｽﾃﾑ
																					// 2:ﾌﾗｯﾌﾟ･駐輪ｼｽﾃﾑ
																					// 3:遠隔ﾌﾗｯﾌﾟ･駐輪ｼｽﾃﾑ
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
																					// 4:ｹﾞｰﾄ式ｼｽﾃﾑ
																					// 5:遠隔ｹﾞｰﾄ式ｼｽﾃﾑ
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	basic->DataKind = knd;															// ﾃﾞｰﾀ種別
	basic->DataKeep = keep;															// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		basic->SeqNo = SeqNo;														// シーケンスNo(1～99:電源投入直後は0)
	}
	basic->ParkingNo = (ulong)CPrmSS[S_SYS][1];										// 駐車場№
	basic->ModelCode = NTNET_MODEL_CODE;											// 機種ｺｰﾄﾞ

	basic->MachineNo = (ulong)CPrmSS[S_PAY][2];										// 機械№

}

//[]----------------------------------------------------------------------[]
///	@brief			金銭管理ログデータ作成(LOGに保存する形式)
//[]----------------------------------------------------------------------[]
///	@param[in]		payclass : 処理区分
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
/////	@return			woid	: 
///	@return			TRUE	: 金銭管理データ送信要
///					FALSE	: 金銭管理データ送信不要
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//void	Make_Log_MnyMng( ushort payclass )
BOOL	Make_Log_MnyMng( ushort payclass )
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
{
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
	BOOL ret = TRUE;
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)

	if( ( ( 0 == payclass ) || ( 1 == payclass ) ) && (OpeNtnetAddedInfo.PayMethod !=5 )){	// 精算完了（精算or再精算）and 定期更新ではない
		if (ntnet_decision_credit(&PayData.credit)) {
			payclass += 4;													// 処理区分をｸﾚｼﾞｯﾄ精算（ｸﾚｼﾞｯﾄ再精算）にする
		}
	}

	if( payclass <= 5 ){													// 精算動作
		// 精算追番・精算日時set(精算動作時以外は０)
		turi_kan.PayCount = CountSel( &PayData.Oiban );						// 精算 or 精算中止追番set
// MH810105(S) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
//		turi_kan.PayDate.Year = PayData.TOutTime.Year;
//		turi_kan.PayDate.Mon = PayData.TOutTime.Mon;
//		turi_kan.PayDate.Day = PayData.TOutTime.Day;
//		turi_kan.PayDate.Hour = PayData.TOutTime.Hour;
//		turi_kan.PayDate.Min = PayData.TOutTime.Min;
//		// 処理時刻＝出庫時刻とする
//		turi_kan.ProcDate.Year = (uchar)( PayData.TOutTime.Year % 100 );
//		turi_kan.ProcDate.Mon = PayData.TOutTime.Mon;
//		turi_kan.ProcDate.Day = PayData.TOutTime.Day;
//		turi_kan.ProcDate.Hour = PayData.TOutTime.Hour;
//		turi_kan.ProcDate.Min = PayData.TOutTime.Min;
//		turi_kan.ProcDate.Sec = 0;
// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//		// 種別	(0=事前精算／1＝未精算出庫精算)
//		if(PayData.shubetsu == 0){
		// 種別	(0=事前精算／1＝未精算出庫精算／2=遠隔精算(精算中変更))
		if(PayData.shubetsu == 0 || PayData.shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
			turi_kan.PayDate.Year = PayData.TOutTime.Year;
			turi_kan.PayDate.Mon = PayData.TOutTime.Mon;
			turi_kan.PayDate.Day = PayData.TOutTime.Day;
			turi_kan.PayDate.Hour = PayData.TOutTime.Hour;
			turi_kan.PayDate.Min = PayData.TOutTime.Min;
			// 処理時刻＝出庫時刻とする
			turi_kan.ProcDate.Year = (uchar)( PayData.TOutTime.Year % 100 );
			turi_kan.ProcDate.Mon = PayData.TOutTime.Mon;
			turi_kan.ProcDate.Day = PayData.TOutTime.Day;
			turi_kan.ProcDate.Hour = PayData.TOutTime.Hour;
			turi_kan.ProcDate.Min = PayData.TOutTime.Min;
			turi_kan.ProcDate.Sec = 0;

		}else{
			turi_kan.PayDate.Year = PayData.TUnpaidPayTime.Year;
			turi_kan.PayDate.Mon = PayData.TUnpaidPayTime.Mon;
			turi_kan.PayDate.Day = PayData.TUnpaidPayTime.Day;
			turi_kan.PayDate.Hour = PayData.TUnpaidPayTime.Hour;
			turi_kan.PayDate.Min = PayData.TUnpaidPayTime.Min;
			// 処理時刻＝精算日時（未精算出庫用）とする
			turi_kan.ProcDate.Year	= (uchar)(PayData.TUnpaidPayTime.Year % 100);	// 精算年
			turi_kan.ProcDate.Mon	= PayData.TUnpaidPayTime.Mon;				// 精算月
			turi_kan.ProcDate.Day	= PayData.TUnpaidPayTime.Day;				// 精算日
			turi_kan.ProcDate.Hour	= PayData.TUnpaidPayTime.Hour;			// 精算時
			turi_kan.ProcDate.Min	= PayData.TUnpaidPayTime.Min;				// 精算分

		}
// MH810105(E) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
	}else if( payclass == 10 ){												// コイン金庫集計
		// 処理時刻＝コイン金庫集計の今回集計時刻とする
		turi_kan.ProcDate.Year = (uchar)( coin_syu.NowTime.Year % 100 );
		turi_kan.ProcDate.Mon = coin_syu.NowTime.Mon;
		turi_kan.ProcDate.Day = coin_syu.NowTime.Day;
		turi_kan.ProcDate.Hour = coin_syu.NowTime.Hour;
		turi_kan.ProcDate.Min = coin_syu.NowTime.Min;
		turi_kan.ProcDate.Sec = 0;
	}else if( payclass == 11 ){												// 紙幣金庫集計
		// 処理時刻＝紙幣金庫集計の今回集計時刻とする
		turi_kan.ProcDate.Year = (uchar)( note_syu.NowTime.Year % 100 );
		turi_kan.ProcDate.Mon = note_syu.NowTime.Mon;
		turi_kan.ProcDate.Day = note_syu.NowTime.Day;
		turi_kan.ProcDate.Hour = note_syu.NowTime.Hour;
		turi_kan.ProcDate.Min = note_syu.NowTime.Min;
		turi_kan.ProcDate.Sec = 0;
	}else if(( payclass == 20 ) ||											// 係員カードによるコイン投入口からのコイン補充
			 ( payclass == 30 ) ||											// 設定機からの釣銭補充
			 ( payclass == 32 )){											// 金銭管理
		// 処理時刻＝釣銭管理の今回集計時刻とする
		turi_kan.ProcDate.Year = (uchar)( turi_kan.NowTime.Year % 100 );
		turi_kan.ProcDate.Mon = turi_kan.NowTime.Mon;
		turi_kan.ProcDate.Day = turi_kan.NowTime.Day;
		turi_kan.ProcDate.Hour = turi_kan.NowTime.Hour;
		turi_kan.ProcDate.Min = turi_kan.NowTime.Min;
		turi_kan.ProcDate.Sec = 0;
	}else{
		// インベントリ等、処理時刻を取得できない場合は処理時刻＝現在時刻とする
		turi_kan.ProcDate.Year = (uchar)( CLK_REC.year % 100 );
		turi_kan.ProcDate.Mon = (uchar)CLK_REC.mont;
		turi_kan.ProcDate.Day = (uchar)CLK_REC.date;
		turi_kan.ProcDate.Hour = (uchar)CLK_REC.hour;
		turi_kan.ProcDate.Min = (uchar)CLK_REC.minu;
		turi_kan.ProcDate.Sec = (uchar)CLK_REC.seco;
	}

	turi_kan.PayClass = (uchar)payclass;									// 処理区分set

	turi_kan.Kakari_no = OPECTL.Kakari_Num;									// 係員№
	turikan_subtube_set();

	if( payclass != 10 ){													// ｺｲﾝ金庫合計でない
		turi_kan.safe_dt[0] = SFV_DAT.safe_dt[0];							// ｺｲﾝ金庫枚数(10円)set
		turi_kan.safe_dt[1] = SFV_DAT.safe_dt[1];							// ｺｲﾝ金庫枚数(50円)set
		turi_kan.safe_dt[2] = SFV_DAT.safe_dt[2];							// ｺｲﾝ金庫枚数(100円)set
		turi_kan.safe_dt[3] = SFV_DAT.safe_dt[3];							// ｺｲﾝ金庫枚数(500円)set
	}

	if( payclass != 11 ){													// 紙幣金庫合計でない
		turi_kan.nt_safe_dt = SFV_DAT.nt_safe_dt;							// 紙幣金庫枚数(1000円)
	}

// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
	if( payclass <= 5 ) {
		ret = FALSE;

		if( memcmp( turi_kan_bk.safe_dt, turi_kan.safe_dt, sizeof( turi_kan.safe_dt )) != 0 ) {	// コイン金庫枚数
			ret = TRUE;
		}
		if( turi_kan_bk.nt_safe_dt != turi_kan.nt_safe_dt ) {	// 紙幣金庫枚数
			ret = TRUE;
		}
		if( CPrmSS[S_KAN][1] ){													// 金銭管理あり設定
			if( memcmp( turi_kan_bk.turi_dat, turi_kan.turi_dat, sizeof( turi_kan.turi_dat )) != 0 ) {	// 金銭データ（4金種分）
				ret = TRUE;
			}
		}
		
		//年月日が0の時はログ登録しない(最小単位と比較としておく)
		if( (turi_kan.PayDate.Year < 1980) ||
			(turi_kan.PayDate.Mon < 1) ||
			(turi_kan.PayDate.Day < 1) ){
			ret = FALSE;
		}
	}
	
	if(ret == TRUE) {
		turi_kan_bk = turi_kan;
	}

	return ret;
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
}

//[]----------------------------------------------------------------------[]
///	@brief			金銭管理データ(データ種別126)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出した金銭管理情報のポインタ
///	@param[out]		p_NtDat   : 金銭管理データ(DATA_KIND_126型)へのポインタ
///	@return			ret       : 金銭管理データのデータサイズ(システムID～)
///	@author			m.nagashima
///	@attention		
///	@note			前回編集内容と同じで金銭管理データ送信不要の場合、ret=0を返す
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/21<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data126( TURI_KAN *p_RcptDat, DATA_KIND_126 *p_NtDat )
{
	char	i;
	ushort	wk_mai;
	uchar	f_Send;
	ushort	ret = 0;
	uchar	pos = 0;

	// 起動直後の現金、割引なしの精算中止・再精算中止時は金銭管理データを送信しない

	memset( p_NtDat, 0, sizeof( DATA_KIND_126 ) );

	NTNET_Edit_BasicData( 126, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// 基本ﾃﾞｰﾀ作成
	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->ProcDate.Year % 100 );		// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->ProcDate.Mon;				// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->ProcDate.Day;				// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->ProcDate.Hour;				// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->ProcDate.Min;				// 処理分
	p_NtDat->DataBasic.Sec = 0;												// 処理秒

	/* 精算追番と精算日時set(精算動作時以外は０) */
	if( p_RcptDat->PayClass <= 5 ){											// 精算動作
		p_NtDat->PayCount = p_RcptDat->PayCount;							// 精算 or 精算中止追番set
		p_NtDat->PayTime.Year = p_RcptDat->PayDate.Year;					// 精算年
		p_NtDat->PayTime.Mon = p_RcptDat->PayDate.Mon;						// 精算月
		p_NtDat->PayTime.Day = p_RcptDat->PayDate.Day;						// 精算日
		p_NtDat->PayTime.Hour = p_RcptDat->PayDate.Hour;					// 精算時
		p_NtDat->PayTime.Min = p_RcptDat->PayDate.Min;						// 精算分
		p_NtDat->PayTime.Sec = 0;											// 精算秒
	}
	p_NtDat->PayClass = p_RcptDat->PayClass;								// 処理区分

	p_NtDat->KakariNo = p_RcptDat->Kakari_no;								// 係員№

	p_NtDat->CoinSf[0].Money = 10;											// ｺｲﾝ金庫金種(10円)
	p_NtDat->CoinSf[1].Money = 50;											// ｺｲﾝ金庫金種(50円)
	p_NtDat->CoinSf[2].Money = 100;											// ｺｲﾝ金庫金種(100円)
	p_NtDat->CoinSf[3].Money = 500;											// ｺｲﾝ金庫金種(500円)
	if( p_RcptDat->PayClass != 10 ){										// ｺｲﾝ金庫合計でない
		p_NtDat->CoinSf[0].Mai = p_RcptDat->safe_dt[0];						// ｺｲﾝ金庫枚数(10円)
		p_NtDat->CoinSf[1].Mai = p_RcptDat->safe_dt[1];						// ｺｲﾝ金庫枚数(50円)
		p_NtDat->CoinSf[2].Mai = p_RcptDat->safe_dt[2];						// ｺｲﾝ金庫枚数(100円)
		p_NtDat->CoinSf[3].Mai = p_RcptDat->safe_dt[3];						// ｺｲﾝ金庫枚数(500円)
	}

	for( i=0; i<4; i++ ){
		p_NtDat->CoinSfTotal +=												// ｺｲﾝ金庫総額
// MH322914 (s) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
//			p_NtDat->CoinSf[i].Mai * p_NtDat->CoinSf[i].Money;
			(ulong)p_NtDat->CoinSf[i].Mai * (ulong)p_NtDat->CoinSf[i].Money;
// MH322914 (e) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
	}

	p_NtDat->NoteSf[0].Money = 1000;										// 紙幣金庫金種(1000円)
	if( p_RcptDat->PayClass != 11 ){										// 紙幣金庫合計でない
		p_NtDat->NoteSf[0].Mai = p_RcptDat->nt_safe_dt;						// 紙幣金庫枚数(1000円)
	}
	p_NtDat->NoteSfTotal =													// 紙幣金庫総額
// MH322914 (s) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)
//		p_NtDat->NoteSf[0].Mai * p_NtDat->NoteSf[0].Money;
		(ulong)p_NtDat->NoteSf[0].Mai * (ulong)p_NtDat->NoteSf[0].Money;
// MH322914 (e) kasiyama 2016/07/07 Long型に変換してないため、オーバーフローした値で送信される(共通改善No.1188)(MH341106)

	// 金銭管理なし時は金庫情報のみ送信
	if( CPrmSS[S_KAN][1] ){													// 金銭管理あり設定
		switch( (uchar)p_RcptDat->PayClass ){

		case 20:
		case 30:
		case 32:
			p_NtDat->Coin[0].Mai = p_RcptDat->turi_dat[0].sin_mai;			// ｺｲﾝ循環枚数(10円)
			p_NtDat->Coin[1].Mai = p_RcptDat->turi_dat[1].sin_mai;			// ｺｲﾝ循環枚数(50円)
			p_NtDat->Coin[2].Mai = p_RcptDat->turi_dat[2].sin_mai;			// ｺｲﾝ循環枚数(100円)
			p_NtDat->Coin[3].Mai = p_RcptDat->turi_dat[3].sin_mai;			// ｺｲﾝ循環枚数(500円)
			for( i=0;i<2;i++ ){
				switch((( p_RcptDat->sub_tube >> (i*8)) & 0x000F )){
					case 0x01:
						pos = 0;
						p_NtDat->CoinYotiku[i].Money = 10;								// 金種金額(10円)
						break;
					case 0x02:
						pos = 1;
						p_NtDat->CoinYotiku[i].Money = 50;								// 金種金額(50円)
						break;
					case 0x04:
						pos = 2;
						p_NtDat->CoinYotiku[i].Money = 100;								// 金種金額(100円)
						break;
					case 0:
					default:															// 接続なし
						continue;
				}
				p_NtDat->CoinYotiku[i].Mai = p_RcptDat->turi_dat[pos].ysin_mai;			// ｺｲﾝ予蓄枚数(10円/50円/100円)
			}
			break;

		default:
			// 10円
			p_NtDat->Coin[0].Mai = p_RcptDat->turi_dat[0].zen_mai			// ｺｲﾝ循環枚数(10円)
									+ (ushort)p_RcptDat->turi_dat[0].sei_nyu;
			wk_mai = (ushort)(p_RcptDat->turi_dat[0].sei_syu + p_RcptDat->turi_dat[0].kyosei);
			if( p_NtDat->Coin[0].Mai < wk_mai ){
				p_NtDat->Coin[0].Mai = 0;
			}else{
				p_NtDat->Coin[0].Mai -= wk_mai;
			}
			// 50円
			p_NtDat->Coin[1].Mai = p_RcptDat->turi_dat[1].zen_mai			// ｺｲﾝ循環枚数(50円)
									+ (ushort)p_RcptDat->turi_dat[1].sei_nyu;
			wk_mai = (ushort)(p_RcptDat->turi_dat[1].sei_syu + p_RcptDat->turi_dat[1].kyosei);
			if( p_NtDat->Coin[1].Mai < wk_mai ){
				p_NtDat->Coin[1].Mai = 0;
			}else{
				p_NtDat->Coin[1].Mai -= wk_mai;
			}
			// 100円
			p_NtDat->Coin[2].Mai = p_RcptDat->turi_dat[2].zen_mai			// ｺｲﾝ循環枚数(100円)
									+ (ushort)p_RcptDat->turi_dat[2].sei_nyu;
			wk_mai = (ushort)(p_RcptDat->turi_dat[2].sei_syu + p_RcptDat->turi_dat[2].kyosei);
			if( p_NtDat->Coin[2].Mai < wk_mai ){
				p_NtDat->Coin[2].Mai = 0;
			}else{
				p_NtDat->Coin[2].Mai -= wk_mai;
			}
			// 500円
			p_NtDat->Coin[3].Mai = p_RcptDat->turi_dat[3].zen_mai			// ｺｲﾝ循環枚数(500円)
									+ (ushort)p_RcptDat->turi_dat[3].sei_nyu;
			wk_mai = (ushort)(p_RcptDat->turi_dat[3].sei_syu + p_RcptDat->turi_dat[3].kyosei);
			if( p_NtDat->Coin[3].Mai < wk_mai ){
				p_NtDat->Coin[3].Mai = 0;
			}else{
				p_NtDat->Coin[3].Mai -= wk_mai;
			}

			for( i=0;i<2;i++ ){
				switch((( p_RcptDat->sub_tube >> (i*8)) & 0x000F )){
					case 0x01:
						pos = 0;
						p_NtDat->CoinYotiku[i].Money = 10;								// 金種金額(10円)
						break;
					case 0x02:
						pos = 1;
						p_NtDat->CoinYotiku[i].Money = 50;								// 金種金額(50円)
						break;
					case 0x04:
						pos = 2;
						p_NtDat->CoinYotiku[i].Money = 100;								// 金種金額(100円)
						break;
					case 0:
					default:															// 接続なし
						continue;
				}
				p_NtDat->CoinYotiku[i].Mai = p_RcptDat->turi_dat[pos].yzen_mai;	// ｺｲﾝ予蓄枚数(10円)
				wk_mai = (ushort)(p_RcptDat->turi_dat[pos].ysei_syu + p_RcptDat->turi_dat[pos].ykyosei);
				if( p_NtDat->CoinYotiku[i].Mai < wk_mai ){
					p_NtDat->CoinYotiku[i].Mai = 0;
				}else{
					p_NtDat->CoinYotiku[i].Mai -= wk_mai;
				}
			}
			break;
		}
		p_NtDat->Coin[0].Money = 10;										// ｺｲﾝ循環金種(10円)
		p_NtDat->Coin[1].Money = 50;										// ｺｲﾝ循環金種(50円)
		p_NtDat->Coin[2].Money = 100;										// ｺｲﾝ循環金種(100円)
		p_NtDat->Coin[3].Money = 500;										// ｺｲﾝ循環金種(500円)
	}
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
	// ログ作成時に判定を行っているので、ここでは不要であるが残しておく
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
	/* 精算動作時は金銭情報に変化が無い場合は送信しない */
	f_Send = 1;
	if( p_RcptDat->PayClass <= 5 ){											// 精算動作（完了or中止。）
		if( 0 == memcmp( &p_NtDat->CoinSf[0], 								// お金情報が前回送信内容と同じ（電文中 ｺｲﾝ金庫以下）
						 &Ntnet_Prev_SData126.CoinSf[0],
						 (sizeof(DATA_KIND_126) - sizeof(DATA_BASIC) - 20) ) ){
			f_Send = 0;														// 金銭管理データを送信しない
		}
	//※ 電源ON直後の釣銭補充にて金銭管理データが送信されない不具合修正 
		//年月日が0の時は送信しない(最小単位と比較としておく)
		if( (p_NtDat->PayTime.Year < 1980) ||
			(p_NtDat->PayTime.Mon < 1) ||
			(p_NtDat->PayTime.Day < 1) ){
			f_Send = 0;														// 金銭管理データを送信しない
		}
	}

	if( f_Send ){															// 金銭管理データを送信する
		memcpy( &Ntnet_Prev_SData126, p_NtDat, sizeof(DATA_KIND_126) );
																			// 前回送信 金銭管理データ保存
		ret = sizeof( DATA_KIND_126 );
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			釣銭管理集計データ(データ種別135)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出した釣銭管理情報のポインタ
///	@param[out]		p_NtDat   : 釣銭管理データ(DATA_KIND_135型)へのポインタ
///	@return			ret       : 釣銭管理データのデータサイズ(システムID～)
///	@author			
///	@attention		
///	@note			前回編集内容と同じで金銭管理データ送信不要の場合、ret=0を返す
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/12/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data135( TURI_KAN *p_TuriDat, DATA_KIND_135 *p_NtDat )
{
	char	i;
	ushort	ret = 0;
	uchar	pos = 0;

	memset( p_NtDat, 0, sizeof( DATA_KIND_135 ) );

	NTNET_Edit_BasicData( 135, 0, p_TuriDat->SeqNo, &p_NtDat->DataBasic);	// 基本ﾃﾞｰﾀ作成
	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_TuriDat->ProcDate.Year % 100 );		// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_TuriDat->ProcDate.Mon;				// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_TuriDat->ProcDate.Day;				// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_TuriDat->ProcDate.Hour;				// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_TuriDat->ProcDate.Min;				// 処理分
	p_NtDat->DataBasic.Sec = 0;												// 処理秒

	p_NtDat->FmtRev = 10;													// フォーマットRev.No

	p_NtDat->CenterSeqNo = p_TuriDat->CenterSeqNo;							// センター追番（釣銭管理）
	
	p_NtDat->Oiban = CountSel( &p_TuriDat->Oiban );							// 金銭管理合計追番

	p_NtDat->PayClass = p_TuriDat->PayClass;								// 処理区分


	p_NtDat->KakariNo = p_TuriDat->Kakari_no;								// 係員№

	// コイン金種1～4
	for( i=0; i<4; i++ ){
		p_NtDat->turi_dat[i].Kind = c_coin[i];								// 金種金額(10/50/100/500)
		p_NtDat->turi_dat[i].gen_mai = p_TuriDat->turi_dat[i].gen_mai;		// 現在（変更前）保有枚数
		p_NtDat->turi_dat[i].zen_mai = p_TuriDat->turi_dat[i].zen_mai;		// 前回保有枚数
		p_NtDat->turi_dat[i].sei_nyu = p_TuriDat->turi_dat[i].sei_nyu;		// 精算時入金枚数
		if(p_TuriDat->turi_dat[i].sei_syu <= 0xFFFF){
			p_NtDat->turi_dat[i].sei_syu = p_TuriDat->turi_dat[i].sei_syu;	// 精算時出金枚数
		}
		else{
			p_NtDat->turi_dat[i].sei_syu = 0xFFFF;							// 精算時出金枚数
		}
		p_NtDat->turi_dat[i].jyun_syu = 0;									// 循環出金枚数
		if(p_TuriDat->turi_dat[i].hojyu <= 0xFFFF){
			p_NtDat->turi_dat[i].hojyu = p_TuriDat->turi_dat[i].hojyu;		// 釣銭補充枚数
		}
		else{
			p_NtDat->turi_dat[i].hojyu = 0xFFFF;							// 釣銭補充枚数
		}
		if(p_TuriDat->turi_dat[i].hojyu_safe <= 0xFFFF){
			p_NtDat->turi_dat[i].hojyu_safe = p_TuriDat->turi_dat[i].hojyu_safe;// 釣銭補充時金庫搬送枚数
		}
		else{
			p_NtDat->turi_dat[i].hojyu_safe = 0xFFFF;						// 釣銭補充時金庫搬送枚数
		}
		if(p_TuriDat->turi_dat[i].kyosei <= 0xFFFF){
			p_NtDat->turi_dat[i].turi_kyosei = p_TuriDat->turi_dat[i].kyosei;	// 強制払出枚数(釣銭口)
		}
		else{
			p_NtDat->turi_dat[i].turi_kyosei = 0xFFFF;						// 強制払出枚数(釣銭口)
		}
		p_NtDat->turi_dat[i].kin_kyosei = 0;								// 強制払出枚数(金庫)
		p_NtDat->turi_dat[i].sin_mai = p_TuriDat->turi_dat[i].sin_mai;		// 新規設定枚数
	}

		
	for( i=0;i<2;i++ ){
		switch((( p_TuriDat->sub_tube >> (i*8)) & 0x000F )){
			case 0x01:
				pos = 0;
				p_NtDat->yturi_dat[i].Kind = 10;								// 金種金額(10円)
				break;
			case 0x02:
				pos = 1;
				p_NtDat->yturi_dat[i].Kind = 50;								// 金種金額(50円)
				break;
			case 0x04:
				pos = 2;
				p_NtDat->yturi_dat[i].Kind = 100;								// 金種金額(100円)
				break;
			case 0:
			default:															// 接続なし
				continue;
		}
		p_NtDat->yturi_dat[i].gen_mai = p_TuriDat->turi_dat[pos].ygen_mai;		// 現在（変更前）保有枚数
		p_NtDat->yturi_dat[i].zen_mai = p_TuriDat->turi_dat[pos].yzen_mai;		// 前回保有枚数
		p_NtDat->yturi_dat[i].sei_nyu = 0;										// 精算時入金枚数
		if(p_TuriDat->turi_dat[pos].ysei_syu <= 0xFFFF){
			p_NtDat->yturi_dat[i].sei_syu = p_TuriDat->turi_dat[pos].ysei_syu;	// 精算時出金枚数
		}
		else{
			p_NtDat->yturi_dat[i].sei_syu = 0xFFFF;								// 精算時出金枚数
		}
		p_NtDat->yturi_dat[i].jyun_syu = 0;										// 循環出金枚数
		p_NtDat->yturi_dat[i].hojyu = 0;										// 釣銭補充枚数
		p_NtDat->yturi_dat[i].hojyu_safe = 0;									// 釣銭補充時金庫搬送枚数
		if(p_TuriDat->turi_dat[pos].ykyosei <= 0xFFFF){
			p_NtDat->yturi_dat[i].turi_kyosei = p_TuriDat->turi_dat[pos].ykyosei;		// 強制払出枚数(釣銭口)
		}
		else{
			p_NtDat->yturi_dat[i].turi_kyosei = 0xFFFF;							// 強制払出枚数(釣銭口)
		}
		p_NtDat->yturi_dat[i].kin_kyosei = 0;									// 強制払出枚数(金庫)
		p_NtDat->yturi_dat[i].sin_mai = p_TuriDat->turi_dat[pos].ysin_mai;		// 新規設定枚数
	}
	ret = sizeof( DATA_KIND_135 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			集計ログデータ作成(LOGに保存する形式)
//[]----------------------------------------------------------------------[]
///	@param[in]		void 
///	@return			void
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Make_Log_TGOUKEI( void )
{
	// ログワークエリアクリア
	memset( &TSYU_LOG_WK, 0, sizeof( SYUKEI ) );
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	memset( &LCKT_LOG_WK, 0, sizeof( LCKTTL_LOG ) );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	// Ｔ集計エリアコピー
	memcpy( &TSYU_LOG_WK, &skyprn, sizeof( SYUKEI ) );
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	// Ｔ集計エリアに30車室分のデータコピー
//	memcpy( &TSYU_LOG_WK.loktldat[0], &loktl.tloktl.loktldat[0], sizeof( LOKTOTAL_DAT )*SYU_LOCK_MAX );
//	// 車室駐車位置別集計(駐車位置毎)の基本データをＴ集計エリアからセット
//	LCKT_LOG_WK.Kikai_no = TSYU_LOG_WK.Kikai_no;
//	LCKT_LOG_WK.Kakari_no = TSYU_LOG_WK.Kakari_no;
//	LCKT_LOG_WK.Oiban = TSYU_LOG_WK.Oiban;
//	memcpy( &LCKT_LOG_WK.NowTime, &TSYU_LOG_WK.NowTime, sizeof( date_time_rec ) );
//	memcpy( &LCKT_LOG_WK.OldTime, &TSYU_LOG_WK.OldTime, sizeof( date_time_rec ) );
//	// 31車室以降の駐車位置別集計(駐車位置毎)コピー
//	memcpy( &LCKT_LOG_WK.loktldat[0], &loktl.tloktl.loktldat[SYU_LOCK_MAX], sizeof( LOKTOTAL_DAT )*(LOCK_MAX-SYU_LOCK_MAX) );
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し 2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		TSYU_LOG_WK.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_TOTAL);
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			集計基本データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 集計基本データ(DATA_KIND_30型)へのポインタ  
///	@return			ret		: 集計基本データのデータサイズ(システムID～) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiKihon( SYUKEI *syukei, ushort Type, DATA_KIND_30 *p_NtDat )
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_30 ) );
	if( Type >= 1 && Type <= 6) {												// Ｔ合計
		NTNET_Edit_BasicData( 30, 0, syukei->SeqNo[0], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
	}else{
		NTNET_Edit_BasicData( 30, 1, syukei->SeqNo[0], &p_NtDat->DataBasic);	// Ｔ小計：データ保持フラグ = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒

	p_NtDat->Type								= Type;							// 集計タイプ
	p_NtDat->KakariNo							= syukei->Kakari_no;			// 係員No.
	p_NtDat->SeqNo								= CountSel( &syukei->Oiban );	// 集計追番
	ntnet_DateTimeCnv(&p_NtDat->NowTime, &syukei->NowTime);						// 今回集計
	ntnet_DateTimeCnv(&p_NtDat->LastTime, &syukei->OldTime);					// 前回集計
	p_NtDat->SettleNum							= syukei->Seisan_Tcnt;			// 総精算回数
	p_NtDat->Kakeuri							= syukei->Kakeuri_Tryo;			// 総掛売額
	p_NtDat->Cash								= syukei->Genuri_Tryo;			// 総現金売上額
	p_NtDat->Uriage								= syukei->Uri_Tryo;				// 総売上額
	p_NtDat->Tax								= syukei->Tax_Tryo;				// 総消費税額
	p_NtDat->Charge								= syukei->Turi_modosi_ryo;		// 釣銭払戻額
	p_NtDat->CoinTotalNum						= syukei->Ckinko_goukei_cnt;	// コイン金庫合計回数
	p_NtDat->NoteTotalNum						= syukei->Skinko_goukei_cnt;	// 紙幣金庫合計回数
	p_NtDat->CyclicCoinTotalNum					= syukei->Junkan_goukei_cnt;	// 循環コイン合計回数
	p_NtDat->NoteOutTotalNum					= syukei->Siheih_goukei_cnt;	// 紙幣払出機合計回数
	p_NtDat->SettleNumServiceTime				= syukei->In_svst_seisan;		// サービスタイム内精算回数
	p_NtDat->Shortage.Num						= syukei->Harai_husoku_cnt;		// 払出不足回数
	p_NtDat->Shortage.Amount					= syukei->Harai_husoku_ryo;		// 払出不足金額
	p_NtDat->Cancel.Num							= syukei->Seisan_chusi_cnt;		// 精算中止回数
	p_NtDat->Cancel.Amount						= syukei->Seisan_chusi_ryo;		// 精算中止金額
	p_NtDat->AntiPassOffSettle					= syukei->Apass_off_seisan;		// アンチパスOFF精算回数
	p_NtDat->ReceiptIssue						= syukei->Ryosyuu_pri_cnt;		// 領収証発行枚数
	p_NtDat->WarrantIssue						= syukei->Azukari_pri_cnt;		// 預り証発行枚数
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	p_NtDat->AllSystem.CarOutIllegal.Num		= syukei->Husei_out_Tcnt;		// 全装置  不正出庫回数
//	p_NtDat->AllSystem.CarOutIllegal.Amount		= syukei->Husei_out_Tryo;		//                 金額
//	p_NtDat->AllSystem.CarOutForce.Num			= syukei->Kyousei_out_Tcnt;		//         強制出庫回数
//	p_NtDat->AllSystem.CarOutForce.Amount		= syukei->Kyousei_out_Tryo;		//                 金額
//	p_NtDat->AllSystem.AcceptTicket				= syukei->Uketuke_pri_Tcnt;		//         受付券発行回数
//	p_NtDat->AllSystem.ModifySettle.Num			= syukei->Syuusei_seisan_Tcnt;	// 修正精算回数
//	p_NtDat->AllSystem.ModifySettle.Amount		= syukei->Syuusei_seisan_Tryo;	//         金額
//	p_NtDat->CarInTotal							= syukei->In_car_Tcnt;			// 総入庫台数
//	p_NtDat->CarOutTotal						= syukei->Out_car_Tcnt;			// 総出庫台数
//	p_NtDat->CarIn1								= syukei->In_car_cnt[0];		// 入庫1入庫台数
//	p_NtDat->CarOut1							= syukei->Out_car_cnt[0];		// 出庫1出庫台数
//	p_NtDat->CarIn2								= syukei->In_car_cnt[1];		// 入庫2入庫台数
//	p_NtDat->CarOut2							= syukei->Out_car_cnt[1];		// 出庫2出庫台数
//	p_NtDat->CarIn3								= syukei->In_car_cnt[2];		// 入庫3入庫台数
//	p_NtDat->CarOut3							= syukei->Out_car_cnt[2];		// 出庫3出庫台数
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し 2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// 強制完了ｷｰでの未入金は基本ﾃﾞｰﾀに項目を新設して送信する
	p_NtDat->MiyoCount							= syukei->Syuusei_seisan_Mcnt;	// 未入金回数
	p_NtDat->MiroMoney							= syukei->Syuusei_seisan_Mryo;	// 未入金額
	p_NtDat->LagExtensionCnt					= syukei->Lag_extension_cnt;	// ラグタイム延長回数
//	予備ｴﾘｱの余りはRev2[11]

	ret = sizeof( DATA_KIND_30 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金種別毎集計データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 料金種別毎集計データ(DATA_KIND_31型)へのポインタ  
///	@return			ret		: 料金種別毎集計データのデータサイズ(システムID～) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiRyokinMai( SYUKEI *syukei, ushort Type, DATA_KIND_31 *p_NtDat )
{
	int		i;
	int		j;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_31 ) );
	if( Type >= 1 && Type <= 6) {												// Ｔ合計
		NTNET_Edit_BasicData( 31, 0, syukei->SeqNo[1], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
	}else{
		NTNET_Edit_BasicData( 31, 1, syukei->SeqNo[1], &p_NtDat->DataBasic);	// Ｔ小計：データ保持フラグ = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒

	p_NtDat->Type					= Type;										// 集計タイプ
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// 集計追番
	for (i = 0, j = 0; i < RYOUKIN_SYU_CNT; i++) {								// 種別01～50
		if (syukei->Rsei_cnt[i] == 0 && syukei->Rsei_ryo[i] == 0
		 && syukei->Rtwari_cnt[i] == 0 && syukei->Rtwari_ryo[i] == 0) {
		 	// データがすべて０の場合は電文に格納せずスキップする。
			continue;
		} else {
			p_NtDat->Kind[j].Kind	= i + 1;									//            料金種別
			p_NtDat->Kind[j].Settle.Num		= syukei->Rsei_cnt[i];				//            精算回数
			p_NtDat->Kind[j].Settle.Amount		= syukei->Rsei_ryo[i];			//            売上額
			p_NtDat->Kind[j].Discount.Num		= syukei->Rtwari_cnt[i];		//            割引回数
			p_NtDat->Kind[j].Discount.Amount	= syukei->Rtwari_ryo[i];		//            割引額
			j++;
		}
	}

	ret = sizeof( DATA_KIND_31 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			分類集計データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 分類集計データ(DATA_KIND_32型)へのポインタ  
///	@return			ret		: 分類集計データのデータサイズ(システムID～) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiBunrui( SYUKEI *syukei, ushort Type, DATA_KIND_32 *p_NtDat )
{
	int		i;
	char	j = 0;
	char	cnt = 0;
	char	flg[3] = {0,0,0};
	char	pram_set[] = {0,0,50,100};
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_32 ) );
	if( Type >= 1 && Type <= 6) {												// Ｔ合計
		NTNET_Edit_BasicData( 32, 0, syukei->SeqNo[2], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
	}else{
		NTNET_Edit_BasicData( 32, 1, syukei->SeqNo[2], &p_NtDat->DataBasic);	// Ｔ小計：データ保持フラグ = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒

	p_NtDat->Type					= Type;										// 集計タイプ
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// 集計追番

/*分類集計１*/
	for(j = 1 ; j <= 3 ; j++){
		for(i = 1 ; i <= 6 ; i++){
			if(j == (uchar)prm_get(COM_PRM, S_BUN, 52, 1, (char)i)){
				flg[cnt] = j;
				cnt++;
				break;
			}
			if(j == (uchar)prm_get(COM_PRM, S_BUN, 53, 1, (char)i)){
				flg[cnt] = j;
				cnt++;
				break;
			}
		}
	}
	if(flg[0]){
		p_NtDat->Kind					= CPrmSS[S_BUN][1]+pram_set[flg[0]];	// 駐車分類集計の種類
		for (i = 0; i < BUNRUI_CNT; i++) {										// 分類01～48
			p_NtDat->Group[i].Num		= syukei->Bunrui1_cnt[flg[0]-1][i];		//            台数1
			p_NtDat->Group[i].Amount	= syukei->Bunrui1_ryo[flg[0]-1][i];		//            台数2／金額
		}
		p_NtDat->GroupTotal.Num		= syukei->Bunrui1_cnt1[flg[0]-1];			// 分類以上   台数1
		p_NtDat->GroupTotal.Amount		= syukei->Bunrui1_ryo1[flg[0]-1];		//            台数2／金額
		p_NtDat->Unknown.Num			= syukei->Bunrui1_cnt2[flg[0]-1];		// 分類不明   台数1
		p_NtDat->Unknown.Amount		= syukei->Bunrui1_ryo2[flg[0]-1];			//            台数2／金額
		flg[0] = 0;
	}
/*分類集計２*/
	if(flg[1]){
		p_NtDat->Kind2					= CPrmSS[S_BUN][1]+pram_set[flg[1]];	// 駐車分類集計の種類
		for (i = 0; i < BUNRUI_CNT; i++) {										// 分類01～48
			p_NtDat->Group2[i].Num		= syukei->Bunrui1_cnt[flg[1]-1][i];		//            台数1
			p_NtDat->Group2[i].Amount	= syukei->Bunrui1_ryo[flg[1]-1][i];		//            台数2／金額
		}
		p_NtDat->GroupTotal2.Num		= syukei->Bunrui1_cnt1[flg[1]-1];		// 分類以上   台数1
		p_NtDat->GroupTotal2.Amount		= syukei->Bunrui1_ryo1[flg[1]-1];		//            台数2／金額
		p_NtDat->Unknown2.Num			= syukei->Bunrui1_cnt2[flg[1]-1];		// 分類不明   台数1
		p_NtDat->Unknown2.Amount		= syukei->Bunrui1_ryo2[flg[1]-1];		//            台数2／金額
		flg[1] = 0;
	}
/*分類集計３*/
	if(flg[2]){
		p_NtDat->Kind3					= CPrmSS[S_BUN][1]+pram_set[flg[2]];	// 駐車分類集計の種類
		for (i = 0; i < BUNRUI_CNT; i++) {										// 分類01～48
			p_NtDat->Group3[i].Num		= syukei->Bunrui1_cnt[flg[2]-1][i];		//            台数1
			p_NtDat->Group3[i].Amount	= syukei->Bunrui1_ryo[flg[2]-1][i];		//            台数2／金額
		}
		p_NtDat->GroupTotal3.Num		= syukei->Bunrui1_cnt1[flg[2]-1];		// 分類以上   台数1
		p_NtDat->GroupTotal3.Amount		= syukei->Bunrui1_ryo1[flg[2]-1];		//            台数2／金額
		p_NtDat->Unknown3.Num			= syukei->Bunrui1_cnt2[flg[2]-1];		// 分類不明   台数1
		p_NtDat->Unknown3.Amount		= syukei->Bunrui1_ryo2[flg[2]-1];		//            台数2／金額
		flg[2] = 0;
	}

	ret = sizeof( DATA_KIND_32 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			割引集計データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 割引集計データ(DATA_KIND_33型)へのポインタ  
///	@return			ret		: 割引集計データのデータサイズ(システムID～) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiWaribiki( SYUKEI *syukei, ushort Type, DATA_KIND_33 *p_NtDat )
{
	int		i;
	int		parking;
	int		group;
	ushort	ret;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//// MH341107(S) K.Onodera 2016/11/08 AI-V対応
//	ushort	mod;
//// MH341107(E) K.Onodera 2016/11/08 AI-V対応
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
// MH321800(S) hosoda ICクレジット対応
	ulong	sei_cnt, sei_ryo;		// 集計用
	int	j;
// MH321800(E) hosoda ICクレジット対応

	wk_media_Type = 0;
	
	memset( p_NtDat, 0, sizeof( DATA_KIND_33 ) );
	if( Type >= 1 && Type <= 6) {												// Ｔ合計
		NTNET_Edit_BasicData( 33, 0, syukei->SeqNo[3], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
	}else{
		NTNET_Edit_BasicData( 33, 1, syukei->SeqNo[3], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒

	p_NtDat->Type					= Type;										// 集計タイプ
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// 集計追番
	i = 0;
	

// クレジット
	if (syukei->Ccrd_sei_cnt != 0 || syukei->Ccrd_sei_ryo != 0) {

		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 基本駐車場番号ｾｯﾄ

		p_NtDat->Discount[i].Kind		= 30;
		p_NtDat->Discount[i].Num	= syukei->Ccrd_sei_cnt;
		p_NtDat->Discount[i].Amount	= syukei->Ccrd_sei_ryo;
		i++;
	}

// サービス券
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		for (group = 0; group < SERVICE_SYU_CNT; group++) {
			if (syukei->Stik_use_cnt[parking][group] == 0
			 && syukei->Stik_use_ryo[parking][group] == 0) {
				// データが０の場合は電文に格納せずスキップする。
				continue;
			} else {
				p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
				p_NtDat->Discount[i].Kind		= 1;
				p_NtDat->Discount[i].Group		= group + 1;
				p_NtDat->Discount[i].Num		= syukei->Stik_use_cnt[parking][group];
				p_NtDat->Discount[i].Amount	= syukei->Stik_use_ryo[parking][group];
				i++;
			}
		}
	}

// プリペイド
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Pcrd_use_cnt[parking] == 0
		 && syukei->Pcrd_use_ryo[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 11;
			p_NtDat->Discount[i].Num		= syukei->Pcrd_use_cnt[parking];
			p_NtDat->Discount[i].Amount	= syukei->Pcrd_use_ryo[parking];
			i++;
		}
	}

// 回数券
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Ktik_use_cnt[parking] == 0
		 && syukei->Ktik_use_ryo[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 5;
			p_NtDat->Discount[i].Num		= syukei->Ktik_use_cnt[parking];
			p_NtDat->Discount[i].Amount	= syukei->Ktik_use_ryo[parking];
			i++;
		}
	}

// 店割引 店No.毎
	for (parking = 0; parking < PKNO_WARI_CNT; parking++) {
		for (group = 0; group < MISE_NO_CNT; group++) {
			if (syukei->Mno_use_cnt4[parking][group] == 0
			 && syukei->Mno_use_ryo4[parking][group] == 0) {
				// データが０の場合は電文に格納せずスキップする。
				continue;
			} else {
				p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
				p_NtDat->Discount[i].Kind		= 2;
				p_NtDat->Discount[i].Group		= group + 1;
				p_NtDat->Discount[i].Num		= syukei->Mno_use_cnt4[parking][group];
				p_NtDat->Discount[i].Amount	= syukei->Mno_use_ryo4[parking][group];
				i++;
			}
		}
	}

// 店割引 駐車場No.毎
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt1[parking] == 0
		 && syukei->Mno_use_ryo1[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 60;
			p_NtDat->Discount[i].Num		= syukei->Mno_use_cnt1[parking];
			p_NtDat->Discount[i].Amount	= syukei->Mno_use_ryo1[parking];
			i++;
		}
	}

// 店割引 1～100合計
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt2[parking] == 0
		 && syukei->Mno_use_ryo2[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 61;
			p_NtDat->Discount[i].Num		= syukei->Mno_use_cnt2[parking];
			p_NtDat->Discount[i].Amount	= syukei->Mno_use_ryo2[parking];
			i++;
		}
	}

// 店割引 101～255合計
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt3[parking] == 0
		 && syukei->Mno_use_ryo3[parking] == 0) {
			// データが０の場合は電文に格納せずスキップする。
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 62;
			p_NtDat->Discount[i].Num		= syukei->Mno_use_cnt3[parking];
			p_NtDat->Discount[i].Amount	= syukei->Mno_use_ryo3[parking];
			i++;
		}
	}

// 店割引 全割引合計
	if (syukei->Mno_use_Tcnt != 0 || syukei->Mno_use_Tryo != 0) {

		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 基本駐車場番号ｾｯﾄ
		p_NtDat->Discount[i].Kind		= 63;
		p_NtDat->Discount[i].Num		= syukei->Mno_use_Tcnt;
		p_NtDat->Discount[i].Amount	= syukei->Mno_use_Tryo;
		i++;
	}

// 種別割引
	for (group = 0; group < RYOUKIN_SYU_CNT; group++) {
		if( syukei->Rtwari_cnt[group] != 0 ){
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];
			p_NtDat->Discount[i].Kind		= 50;
			p_NtDat->Discount[i].Group		= group + 1;
			p_NtDat->Discount[i].Num		= syukei->Rtwari_cnt[group];
			p_NtDat->Discount[i].Amount	= syukei->Rtwari_ryo[group];
			i++;
		}
	}

// 種別割引
	if( syukei->Electron_sei_cnt ){
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
		p_NtDat->Discount[i].Kind = NTNET_SUICA_1;								// 割引種別：31（Suica決済）固定
		p_NtDat->Discount[i].Group = 0;											// 割引区分：未使用(0)
		p_NtDat->Discount[i].Num = syukei->Electron_sei_cnt;					// 割引回数：
		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_sei_ryo;					// 割引額  ：
		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
		p_NtDat->Discount[i].Rsv = 0;											// 予備　　：未使用(0)
		i++;
	}

	if( syukei->Electron_psm_cnt ){
// PASMO
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
		p_NtDat->Discount[i].Kind = NTNET_PASMO_0;								// 割引種別：33（PASMO決済）固定
		p_NtDat->Discount[i].Group = 0;											// 割引区分：未使用(0)
		p_NtDat->Discount[i].Num = syukei->Electron_psm_cnt;					// 割引回数：
		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_psm_ryo;					// 割引額  ：
		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
		p_NtDat->Discount[i].Rsv = 0;											// 予備　　：未使用(0)
		i++;
	}
// MH321800(S) hosoda ICクレジット対応, 従来のEdy処理を削除
//-	if( syukei->Electron_edy_cnt ){
//-// Edy
//-		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
//-		p_NtDat->Discount[i].Kind = NTNET_EDY_0;								// 割引種別：32（Edy決済）固定
//-		p_NtDat->Discount[i].Group = 0;											// 割引区分：(0)取引成功分
//-		p_NtDat->Discount[i].Num = syukei->Electron_edy_cnt;					// 割引回数：
//-		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
//-		p_NtDat->Discount[i].Amount = syukei->Electron_edy_ryo;					// 割引額  ：
//-		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
//-		p_NtDat->Discount[i].Rsv = 0;											// 予備　　：未使用(0)
//-		i++;
//-	}
//-
//-	if( syukei->Electron_Arm_cnt ){
//-		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
//-		p_NtDat->Discount[i].Kind = NTNET_EDY_0;								// 割引種別：32（Edy決済）固定
//-		p_NtDat->Discount[i].Group = 1;											// 割引区分：(1)ｱﾗｰﾑ取引分
//-		p_NtDat->Discount[i].Num = syukei->Electron_Arm_cnt;					// 割引回数：
//-		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
//-		p_NtDat->Discount[i].Amount = syukei->Electron_Arm_ryo;					// 割引額  ：
//-		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
//-		p_NtDat->Discount[i].Rsv = 0;											// 予備　　：未使用(0)
//-		i++;
//-	}
// MH321800(E) hosoda ICクレジット対応, 従来のEdy処理を削除

	if( syukei->Electron_ico_cnt ){
// ICOCA
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
		p_NtDat->Discount[i].Kind = NTNET_ICOCA_0;								// 割引種別：35（ICOCA決済）固定
		p_NtDat->Discount[i].Group = 0;											// 割引区分：(0)取引成功分
		p_NtDat->Discount[i].Num = syukei->Electron_ico_cnt;					// 割引回数：
		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_ico_ryo;					// 割引額  ：
		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
		p_NtDat->Discount[i].Rsv = 0;											// 予備　　：未使用(0)
		i++;
	}
	
	if( syukei->Electron_icd_cnt ){

// IC-Card
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
		p_NtDat->Discount[i].Kind = NTNET_ICCARD_0;								// 割引種別：36（IC-Card決済）固定
		p_NtDat->Discount[i].Group = 0;											// 割引区分：(0)取引成功分
		p_NtDat->Discount[i].Num = syukei->Electron_icd_cnt;					// 割引回数：
		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_icd_ryo;					// 割引額  ：
		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
		p_NtDat->Discount[i].Rsv = 0;											// 予備　　：未使用(0)
		i++;
	}

// MH321800(S) hosoda ICクレジット対応
	// ※集計データにアラーム取引分はセットしない
	for (j = 0; j < TBL_CNT(ec_discount_kind_tbl); j++) {
		switch (ec_discount_kind_tbl[j]) {
		case	NTNET_EDY_0:
			sei_cnt = syukei->Electron_edy_cnt;
			sei_ryo = syukei->Electron_edy_ryo;
			break;
		case	NTNET_NANACO_0:
			sei_cnt = syukei->nanaco_sei_cnt;
			sei_ryo = syukei->nanaco_sei_ryo;
			break;
		case	NTNET_WAON_0:
			sei_cnt = syukei->waon_sei_cnt;
			sei_ryo = syukei->waon_sei_ryo;
			break;
		case	NTNET_SAPICA_0:
			sei_cnt = syukei->sapica_sei_cnt;
			sei_ryo = syukei->sapica_sei_ryo;
			break;
		case	NTNET_SUICA_1:
			sei_cnt = syukei->koutsuu_sei_cnt;
			sei_ryo = syukei->koutsuu_sei_ryo;
			break;
		case	NTNET_ID_0:
			sei_cnt = syukei->id_sei_cnt;
			sei_ryo = syukei->id_sei_ryo;
			break;
		case	NTNET_QUICPAY_0:
			sei_cnt = syukei->quicpay_sei_cnt;
			sei_ryo = syukei->quicpay_sei_ryo;
			break;
		default	:
			sei_cnt = 0L;
			sei_ryo = 0L;
			break;
		}
		if (sei_cnt != 0L) {
		// 決済集計
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
			p_NtDat->Discount[i].Kind = ec_discount_kind_tbl[j];				// 割引種別：Edy/nanaco/WAON/SAPICAなど
			p_NtDat->Discount[i].Group = 0;										// 割引区分：(0)取引成功分
			p_NtDat->Discount[i].Num = sei_cnt;									// 割引回数：
			p_NtDat->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
			p_NtDat->Discount[i].Amount = sei_ryo;								// 割引額  ：
			p_NtDat->Discount[i].Info = 0;										// 割引情報：未使用(0)
			p_NtDat->Discount[i].Rsv = 0;										// 予備　　：未使用(0)

			i++;
		}
	}
// MH321800(E) hosoda ICクレジット対応

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	if (syukei->Gengaku_seisan_cnt) {
//		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
//		p_NtDat->Discount[i].Kind = NTNET_GENGAKU;								// 割引種別：減額精算
//		p_NtDat->Discount[i].Group = 0;											// 割引区分：未使用(0)
//		p_NtDat->Discount[i].Num = syukei->Gengaku_seisan_cnt;					// 割引回数：
//		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
//		p_NtDat->Discount[i].Amount = syukei->Gengaku_seisan_ryo;				// 割引額  ：
//		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
//		p_NtDat->Discount[i].Rsv = 0;											// 予備　　：未使用(0)
//		i++;
//	}
//
//	if (syukei->Furikae_seisan_cnt) {
//		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
//		p_NtDat->Discount[i].Kind = NTNET_FURIKAE;								// 割引種別：振替精算
//		p_NtDat->Discount[i].Group = 0;											// 割引区分：未使用(0)
//		p_NtDat->Discount[i].Num = syukei->Furikae_seisan_cnt;					// 割引回数：
//		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
//		p_NtDat->Discount[i].Amount = syukei->Furikae_seisan_ryo;				// 割引額  ：
//		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
//		p_NtDat->Discount[i].Rsv = 0;											// 予備　　：未使用(0)
//		i++;
//	}
//// MH341107(S) K.Onodera 2016/11/08 AI-V対応
//	for( mod=1; mod<MOD_TYPE_MAX; mod++ ){
//		if( syukei->Furikae_CardKabarai[mod] ){
//			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
//			p_NtDat->Discount[i].Kind = NTNET_KABARAI;							// 割引種別：振替精算(ParkingWeb版)
//			p_NtDat->Discount[i].Group = mod;									// 割引区分：媒体種別
//			p_NtDat->Discount[i].Num = syukei->Furikae_Card_cnt[mod];			// 割引回数：未使用(0)
//			p_NtDat->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
//			p_NtDat->Discount[i].Amount = syukei->Furikae_CardKabarai[mod];		// 割引額  ：過払い金額
//			p_NtDat->Discount[i].Info = 0;										// 割引情報：未使用(0)
//			i++;
//		}
//	}
//// MH341107(E) K.Onodera 2016/11/08 AI-V対応
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	ret = sizeof( DATA_KIND_33 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			定期集計データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 定期集計データ(DATA_KIND_34型)へのポインタ  
///	@return			ret		: 定期集計データのデータサイズ(システムID～) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiTeiki( SYUKEI *syukei, ushort Type, DATA_KIND_34 *p_NtDat )
{
	int		i;
	int		parking;
	int		kind;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_34 ) );
	if( Type >= 1 && Type <= 6) {												// Ｔ合計
		NTNET_Edit_BasicData( 34, 0, syukei->SeqNo[4], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
	}else{
		NTNET_Edit_BasicData( 34, 1, syukei->SeqNo[4], &p_NtDat->DataBasic);	// Ｔ小計：データ保持フラグ = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒

	p_NtDat->Type					= Type;										// 集計タイプ
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// 集計追番
	i = 0;
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		for (kind = 0; kind < TEIKI_SYU_CNT; kind++) {
			if ( ( syukei->Teiki_use_cnt[parking][kind] == 0 ) &&				// 定期使用回数が０の場合
				 ( syukei->Teiki_kou_cnt[parking][kind] == 0 ) ) {				// 定期更新回数が０の場合
				// データが０の場合は電文に格納せずスキップする。
				continue;
			} else {
				p_NtDat->Pass[i].ParkingNo		= CPrmSS[S_SYS][1+parking];		// 駐車場No.
				p_NtDat->Pass[i].Kind			= kind + 1;						// 種別
				p_NtDat->Pass[i].Num			= syukei->Teiki_use_cnt[parking][kind];	// 回数
				p_NtDat->Pass[i].Update.Num		= syukei->Teiki_kou_cnt[parking][kind];	// 更新回数
				p_NtDat->Pass[i].Update.Amount	= syukei->Teiki_kou_ryo[parking][kind];	// 更新売上金額
				i++;
			}
		}
	}

	ret = sizeof( DATA_KIND_34 ); 
	return ret;
}

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// //[]----------------------------------------------------------------------[]
// ///	@brief			車室毎集計データ編集処理
// //[]----------------------------------------------------------------------[]
// ///	@param[in]		syukei	: ログから取り出した集計データのポインタ
// ///	@param[in]		Type	: 集計タイプ
// ///	@param[out]		p_NtDat	: 車室毎集計データ(DATA_KIND_35型)へのポインタ  
// ///	@return			ret		: 車室毎集計データのデータサイズ(システムID～) 
// ///	@author			m.nagashima
// ///	@attention		
// ///	@note			
// //[]----------------------------------------------------------------------[]
// ///	@date			Create	: 2012/02/24<br>
// ///					Update	: 
// //[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// unsigned short	NTNET_Edit_SyukeiShashitsuMai( SYUKEI *syukei, ushort Type, DATA_KIND_35 *p_NtDat )
// {
// 	t_SyuSub_Lock	*dst;
// 	LOKTOTAL_DAT	*src;
// 	int		i;
// 	int		cnt;
// 	ulong	posi;
// 	ushort	ret;
// 
// 	// 集計データの追番と一致する車室毎集計データを検索する
// 	ret = Ope_Log_ShashitsuLogGet( syukei, &lckttl_wk );
// 	memset( p_NtDat, 0, sizeof( DATA_KIND_35 ) );
// 	if( Type >= 1 && Type <= 6) {
// 		NTNET_Edit_BasicData( 35, 0, syukei->SeqNo[5], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
// 	}else{
// 		NTNET_Edit_BasicData( 35, 1, syukei->SeqNo[5], &p_NtDat->DataBasic);	// Ｔ小計：データ保持フラグ = 1
// 	}
// 
// 	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
// 	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
// 	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
// 	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
// 	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
// 	p_NtDat->DataBasic.Sec = 0;													// 処理秒
// 
// 	p_NtDat->Type					= Type;										// 集計タイプ
// 	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
// 	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// 集計追番
// 	for (i = 0 , cnt = 0; i < LOCK_MAX; i++) {
// 		WACDOG;																	// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
// 		if(LockInfo[i].lok_no != 0){											// 接続有？
// 			if( !SetCarInfoSelect((short)i) ){
// 				continue;
// 			}
// 			if( cnt >= OLD_LOCK_MAX ){
// 				break;
// 			}
// 			posi = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// 駐車位置/
// 			dst = &p_NtDat->Lock[cnt];
// 			if( Type >= 1 && Type <= 6) {										// Ｔ合計
// 				// ログから取出したデータの車室毎エリアのポインタ算出
// 				if( i < SYU_LOCK_MAX ){				// 1～30車室
// 					src = &syukei->loktldat[i];
// 				}else{								// 31車室～
// 					if( ret ){						// 車室毎集計データ検索OK
// 						src = &lckttl_wk.loktldat[i - SYU_LOCK_MAX];
// 					}else{
// 						break;
// 					}
// 				}
// 			}else{
// 				// Ｔ合計用車室毎エリアのポインタ算出
// 				src = &loktl.tloktl.loktldat[i];
// 			}
// 			dst->LockNo					= posi;									// 区画情報
// 			dst->CashAmount				= src->Genuri_ryo;						// 現金売上
// 			dst->Settle					= src->Seisan_cnt;						// 精算回数
// 			dst->CarOutIllegal.Num		= src->Husei_out_cnt;					// 不正出庫回数
// 			dst->CarOutIllegal.Amount	= src->Husei_out_ryo;					//         金額
// 			dst->CarOutForce.Num		= src->Kyousei_out_cnt;					// 強制出庫回数
// 			dst->CarOutForce.Amount		= src->Kyousei_out_ryo;					//         金額
// 			dst->AcceptTicket			= src->Uketuke_pri_cnt;					// 受付券発行回数
// 			dst->ModifySettle.Num		= src->Syuusei_seisan_cnt;				// 修正精算回数
// 			dst->ModifySettle.Amount	= src->Syuusei_seisan_ryo;				//         金額
// 			cnt++;
// 		}
// 	}
// 
// 	ret = sizeof( DATA_KIND_35 ); 
// 	return ret;
// }
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

//[]----------------------------------------------------------------------[]
///	@brief			金銭集計データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 金銭集計データ(DATA_KIND_36型)へのポインタ  
///	@return			ret		: 金銭集計データのデータサイズ(システムID～) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiKinsen( SYUKEI *syukei, ushort Type, DATA_KIND_36 *p_NtDat )
{
	static const ushort c_coin[COIN_SYU_CNT] = {10, 50, 100, 500};
	ulong	w;
	int		i;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_36 ) );
	if( Type >= 1 && Type <= 6) {												// Ｔ合計
		NTNET_Edit_BasicData( 36, 0, syukei->SeqNo[6], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
	}else{
		NTNET_Edit_BasicData( 36, 1, syukei->SeqNo[6], &p_NtDat->DataBasic);	// Ｔ小計：データ保持フラグ = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒

	p_NtDat->Type					= Type;										// 集計タイプ
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// 集計追番
	p_NtDat->Total					= syukei->Kinko_Tryo;						// 金庫総入金額
	p_NtDat->NoteTotal				= syukei->Note_Tryo;						// 紙幣金庫総入金額
	p_NtDat->CoinTotal				= syukei->Coin_Tryo;						// コイン金庫総入金額
	for (i = 0; i < COIN_SYU_CNT; i++) {										// コイン1～4
		p_NtDat->Coin[i].Kind		= c_coin[i];								//        金種
		p_NtDat->Coin[i].Num		= syukei->Coin_cnt[i];						//        枚数
	}
	p_NtDat->Note[0].Kind			= 1000;										// 紙幣1  金種
	p_NtDat->Note[0].Num2			= syukei->Note_cnt[0];						//        紙幣枚数
	w = 0;
	for (i = 0; i < COIN_SYU_CNT; i++) {
		w += ((syukei->tou[i] + syukei->hoj[i]) * c_coin[i]);
	}
	p_NtDat->CycleAccept			= w;										// 循環部総入金額
	w = 0;
	for (i = 0; i < COIN_SYU_CNT; i++) {
		w += ((syukei->sei[i] + syukei->kyo[i]) * c_coin[i]);
	}
	p_NtDat->CyclePay				= w;										// 循環部総出金額
	p_NtDat->NoteAcceptTotal		= syukei->tou[4] * 1000;					// 紙幣総入金額
	for (i = 0; i < _countof(p_NtDat->Cycle); i++) {
		p_NtDat->Cycle[i].CoinKind			= c_coin[i];						// 循環1～4 コイン金種
		p_NtDat->Cycle[i].Accept			= syukei->tou[i];					//          入金枚数
		p_NtDat->Cycle[i].Pay				= syukei->sei[i];					//          出金枚数
		p_NtDat->Cycle[i].ChargeSupply		= syukei->hoj[i];					//          釣銭補充枚数
		p_NtDat->Cycle[i].SlotInventory		= syukei->kyo[i];					//          インベントリ枚数(取出口)
	}

	ret = sizeof( DATA_KIND_36 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			集計終了通知データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 集計終了通知データ(DATA_KIND_41型)へのポインタ  
///	@return			ret		: 集計終了通知データのデータサイズ(システムID～) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiSyuryo( SYUKEI *syukei, ushort Type, DATA_KIND_41 *p_NtDat )
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_41 ) );
	if( Type >= 1 && Type <= 6) {												// Ｔ合計
		NTNET_Edit_BasicData( 41, 0, syukei->SeqNo[7], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
	}else{
		NTNET_Edit_BasicData( 41, 1, syukei->SeqNo[7], &p_NtDat->DataBasic);	// Ｔ小計：データ保持フラグ = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒

	p_NtDat->Type					= Type;										// 集計タイプ
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// 集計追番

	ret = sizeof( DATA_KIND_41 ); 
	return ret;
}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し 2019/11/15 車番チケットレス(RT精算データ対応)
////--------------------------------------------------------------
////	入庫フォーマット判定(20/54)
//unsigned char	NTNET_Edit_isData20_54(enter_log *p_RcptDat)		// 入庫logﾃﾞｰﾀ(IN)
//{
//	unsigned char ret = 0;
//
//	if (p_RcptDat->CMN_DT.DT_54.ID == 20) {
//		ret = 20;
//	} else if (p_RcptDat->CMN_DT.DT_54.ID == 54) {
//		ret = 54;
//	}
//	return ret;
//}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し 2019/11/15 車番チケットレス(RT精算データ対応)
//--------------------------------------------------------------
//	精算フォーマット判定(22/56)
unsigned char	NTNET_Edit_isData22_56(Receipt_data *p_RcptDat)		// 精算logﾃﾞｰﾀ(IN)
{
	unsigned char ret = 0;

	if (p_RcptDat->ID == 22) {
		ret = 22;
	} else if (p_RcptDat->ID == 56) {
		ret = 56;
	}
	return ret;
}
//--------------------------------------------------------------
//	駐車台数フォーマット判定(236/58)
unsigned char	NTNET_Edit_isData236_58(ParkCar_log *p_RcptDat)		// 駐車台数用logﾃﾞｰﾀ(IN)
{
	unsigned char ret = 0;

	if (p_RcptDat->CMN_DT.DT_58.ID == 236) {
		ret = 236;
	} else if (p_RcptDat->CMN_DT.DT_58.ID == 58) {
		ret = 58;
	}
	return ret;
}
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス(RT精算データ対応)
////--------------------------------------------------------------
////	新入庫NT-NETフォーマット設定(20ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA1を付加)	// 参照 = NTNET_Edit_Data20
//unsigned short	NTNET_Edit_Data54(	enter_log 		*p_RcptDat,	// 入庫logﾃﾞｰﾀ(IN)
//									DATA_KIND_54 	*p_NtDat )	// 入庫NT-NET(OUT)
//{
//	unsigned short ret;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_54 ) );
//	NTNET_Edit_BasicData( 54, 0, p_RcptDat->CMN_DT.DT_54.SeqNo, &p_NtDat->DataBasic);	// 基本データ作成
//	memcpy(&p_NtDat->InCount, p_RcptDat, sizeof( enter_log ) - sizeof(p_RcptDat->CMN_DT));
//
//	// 処理年月日をセット
//	p_NtDat->DataBasic.Year	= p_RcptDat->CMN_DT.DT_54.ProcDate.Year;	// 処理年
//	p_NtDat->DataBasic.Mon	= p_RcptDat->CMN_DT.DT_54.ProcDate.Mon;		// 処理月
//	p_NtDat->DataBasic.Day	= p_RcptDat->CMN_DT.DT_54.ProcDate.Day;		// 処理日
//	p_NtDat->DataBasic.Hour	= p_RcptDat->CMN_DT.DT_54.ProcDate.Hour;	// 処理時
//	p_NtDat->DataBasic.Min	= p_RcptDat->CMN_DT.DT_54.ProcDate.Min;		// 処理分
//	p_NtDat->DataBasic.Sec	= p_RcptDat->CMN_DT.DT_54.ProcDate.Sec;		// 処理秒
//
//	// 20 -> 54に追加されたﾃﾞｰﾀの設定(FmtRev, PARKCAR_DATA1)
//	p_NtDat->FmtRev = 0;					// フォーマットRev.№
//	p_NtDat->ParkData.FullNo1 	= p_RcptDat->CMN_DT.DT_54.FullNo1;		// 満車台数１
//	p_NtDat->ParkData.CarCnt1 	= p_RcptDat->CMN_DT.DT_54.CarCnt1;		// 現在台数１
//	p_NtDat->ParkData.FullNo2 	= p_RcptDat->CMN_DT.DT_54.FullNo2;		// 満車台数２
//	p_NtDat->ParkData.CarCnt2 	= p_RcptDat->CMN_DT.DT_54.CarCnt2;		// 現在台数２
//	p_NtDat->ParkData.FullNo3 	= p_RcptDat->CMN_DT.DT_54.FullNo3;		// 満車台数３
//	p_NtDat->ParkData.CarCnt3 	= p_RcptDat->CMN_DT.DT_54.CarCnt3;		// 現在台数３
//	p_NtDat->ParkData.PascarCnt = p_RcptDat->CMN_DT.DT_54.PascarCnt;		// 定期車両カウント
//	p_NtDat->ParkData.Full[0] 	= p_RcptDat->CMN_DT.DT_54.Full[0];		// 駐車1満空状態
//	p_NtDat->ParkData.Full[1] 	= p_RcptDat->CMN_DT.DT_54.Full[1];		// 駐車2満空状態
//	p_NtDat->ParkData.Full[2] 	= p_RcptDat->CMN_DT.DT_54.Full[2];		// 駐車3満空状態
//
//	ret = sizeof( DATA_KIND_54 );
//	return ret;
//}
////	新入庫NT-NETフォーマット設定(20ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA11を付加)	// 参照 = NTNET_Edit_Data20
//unsigned short	NTNET_Edit_Data54_r10(	enter_log 			*p_RcptDat,		// 入庫logﾃﾞｰﾀ(IN)
//										DATA_KIND_54_r10 	*p_NtDat )		// 入庫NT-NET(OUT)
//{
//	unsigned short ret;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_54_r10 ) );
//	NTNET_Edit_BasicData( 54, 0,  p_RcptDat->CMN_DT.DT_54.SeqNo, &p_NtDat->DataBasic );	// 基本データ作成
//
//	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなく入庫時刻をセットするので再セット
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->InTime.Year % 100);		// 入庫年
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->InTime.Mon;					// 入庫月
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->InTime.Day;					// 入庫日
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->InTime.Hour;				// 入庫時
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->InTime.Min;					// 入庫分
//	p_NtDat->DataBasic.Sec	= (uchar)p_RcptDat->InTime.Sec;					// 入庫秒
//
//	p_NtDat->FmtRev = 10;													// フォーマットRev.№
//
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->CMN_DT.DT_54.FullNo1;			// 満車台数１
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CMN_DT.DT_54.CarCnt1;			// 現在台数１
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->CMN_DT.DT_54.FullNo2;			// 満車台数２
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CMN_DT.DT_54.CarCnt2;			// 現在台数２
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->CMN_DT.DT_54.FullNo3;			// 満車台数３
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CMN_DT.DT_54.CarCnt3;			// 現在台数３
//	p_NtDat->ParkData.CarFullFlag	= 0;									// 台数・満空状態フラグ
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->CMN_DT.DT_54.PascarCnt;	// 定期車両カウント
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->CMN_DT.DT_54.Full[0];			// 駐車1満空状態
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->CMN_DT.DT_54.Full[1];			// 駐車2満空状態
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->CMN_DT.DT_54.Full[2];			// 駐車3満空状態
//
//	p_NtDat->CenterSeqNo	= p_RcptDat->CMN_DT.DT_54.CenterSeqNo;			// センター追番（入庫）
//
//	p_NtDat->InCount	= p_RcptDat->InCount;								// 入庫追い番
//	p_NtDat->Syubet		= (uchar)p_RcptDat->Syubet;							// 処理区分
//	p_NtDat->InMode		= (uchar)p_RcptDat->InMode;							// 入庫ﾓｰﾄﾞ
//	p_NtDat->CMachineNo	= (uchar)p_RcptDat->CMachineNo;						// 駐車券機械№
//	p_NtDat->Reserve	= 0;												// 予備（サイズ調整用）
//	p_NtDat->FlapArea	= (ushort)(p_RcptDat->LockNo/10000);				// フラップシステム	区画		0～99
//	p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->LockNo%10000);				// 					車室番号	0～9999
//	p_NtDat->PassCheck	= (uchar)p_RcptDat->PassCheck;						// ｱﾝﾁﾊﾟｽﾁｪｯｸ
//	p_NtDat->CountSet	= (uchar)p_RcptDat->CountSet;						// 在車ｶｳﾝﾄ
//
//	// 入庫媒体情報は 0 をセットする
//	memset( &p_NtDat->Media[0], 0, sizeof( p_NtDat->Media ) );				// 入庫媒体情報1～4
//
//	ret = sizeof( DATA_KIND_54_r10 );
//	return ret;
//}
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス(RT精算データ対応)
//--------------------------------------------------------------
//	新精算NT-NETフォーマット設定(22ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA1を付加)	// 参照 = NTNET_Edit_Data22
unsigned short	NTNET_Edit_Data56(	Receipt_data 	*p_RcptDat,	// 精算logﾃﾞｰﾀ(IN)
									DATA_KIND_56 	*p_NtDat )	// 精算NT-NET(OUT)
{
	uchar	i;
	uchar	j;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_56 ) );

	p_NtDat->DataBasic.SystemID = p_RcptDat->DataBasic.SystemID;		// ｼｽﾃﾑID
	p_NtDat->DataBasic.DataKind = p_RcptDat->DataBasic.DataKind;		// ﾃﾞｰﾀ種別
	p_NtDat->DataBasic.DataKeep = p_RcptDat->DataBasic.DataKeep;		// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	p_NtDat->DataBasic.ParkingNo = p_RcptDat->DataBasic.ParkingNo;		// 駐車場№
	p_NtDat->DataBasic.ModelCode = p_RcptDat->DataBasic.ModelCode;		// 機種ｺｰﾄﾞ
	p_NtDat->DataBasic.MachineNo = p_RcptDat->DataBasic.MachineNo;		// 機械№
	memcpy( &p_NtDat->DataBasic.Year, &p_RcptDat->DataBasic.Year, 6 );	// 処理年月日時分秒

	p_NtDat->PayCount = CountSel( &p_RcptDat->Oiban);					// 精算or精算中止追い番
	p_NtDat->PayMethod = p_RcptDat->PayMethod;							// 精算方法
	p_NtDat->PayClass = p_RcptDat->PayClass;							// 処理区分
	p_NtDat->PayMode = p_RcptDat->PayMode;								// 精算ﾓｰﾄﾞ(自動精算)

// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->LockNo = p_RcptDat->WPlace;							// 区画情報
//	}
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	p_NtDat->CardType = 0;												// 駐車券ﾀｲﾌﾟ
// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
//	p_NtDat->CMachineNo = 0;											// 駐車券機械№
	p_NtDat->CMachineNo	= p_RcptDat->CMachineNo;						// 駐車券機械№
// MH810100(E) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
	p_NtDat->CardNo = 0L;												// 駐車券番号(発券追い番)
	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->OutTime.Year = p_RcptDat->TOutTime.Year;				// 出庫年
		p_NtDat->OutTime.Mon = p_RcptDat->TOutTime.Mon;					// 出庫月
		p_NtDat->OutTime.Day = p_RcptDat->TOutTime.Day;					// 出庫日
		p_NtDat->OutTime.Hour = p_RcptDat->TOutTime.Hour;				// 出庫時
		p_NtDat->OutTime.Min = p_RcptDat->TOutTime.Min;					// 出庫分
		p_NtDat->OutTime.Sec = 0;										// 出庫秒
		p_NtDat->KakariNo = p_RcptDat->KakariNo;						// 係員№
		p_NtDat->OutKind = p_RcptDat->OutKind;							// 精算出庫
	}
	p_NtDat->ReceiptIssue = p_RcptDat->ReceiptIssue;					// 領収証発行有無
	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->InTime.Year = p_RcptDat->TInTime.Year;					// 入庫年
		p_NtDat->InTime.Mon = p_RcptDat->TInTime.Mon;					// 入庫月
		p_NtDat->InTime.Day = p_RcptDat->TInTime.Day;					// 入庫日
		p_NtDat->InTime.Hour = p_RcptDat->TInTime.Hour;					// 入庫時
		p_NtDat->InTime.Min = p_RcptDat->TInTime.Min;					// 入庫分
		p_NtDat->InTime.Sec = 0;										// 入庫秒
	}
	// 精算年月日時分秒は0とする
	// 前回精算年月日時分秒は0とする
	p_NtDat->TaxPrice = 0;												// 課税対象額
	p_NtDat->TotalPrice = 0;											// 合計金額(HOST未使用のため)
	p_NtDat->Tax = p_RcptDat->Wtax;										// 消費税額
	if( p_NtDat->PayMethod != 5 ){										// 更新精算以外
		p_NtDat->Syubet = p_RcptDat->syu;								// 料金種別
		p_NtDat->Price = p_RcptDat->WPrice;								// 駐車料金
	}
	p_NtDat->CashPrice =
						p_RcptDat->WInPrice - p_RcptDat->WChgPrice;		// 現金売上
	p_NtDat->InPrice = p_RcptDat->WInPrice;								// 投入金額
	p_NtDat->ChgPrice = p_RcptDat->WChgPrice;							// 釣銭金額
	p_NtDat->Fusoku = p_RcptDat->WFusoku;								// 釣銭払出不足金額
	p_NtDat->FusokuFlg = 0;												// 釣銭払出不足発生ﾌﾗｸﾞ
	p_NtDat->PayObsFlg = 0;												// 精算中障害発生ﾌﾗｸﾞ
	p_NtDat->ChgOverFlg = 0;											// 払戻上限額ｵｰﾊﾞｰ発生ﾌﾗｸﾞ

	p_NtDat->PassCheck = p_RcptDat->PassCheck;							// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	p_NtDat->CountSet = p_RcptDat->CountSet;							// 在車ｶｳﾝﾄしない
	if( p_RcptDat->teiki.ParkingNo ){									// 定期券精算処理or定期券更新時
		p_NtDat->PassData.ParkingNo = p_RcptDat->teiki.ParkingNo;		// 定期券駐車場№
		p_NtDat->PassData.PassID = p_RcptDat->teiki.id;					// 定期券ID
		p_NtDat->PassData.Syubet = p_RcptDat->teiki.syu;				// 定期券種別
		p_NtDat->PassData.State = p_RcptDat->teiki.status;				// 定期券ｽﾃｰﾀｽ
		p_NtDat->PassData.MoveMode = 0;									// 定期券返却
		p_NtDat->PassData.ReadMode = 0;									// 定期券ﾘｰﾄﾞﾗｲﾄ
		p_NtDat->PassData.SYear =
								(uchar)(p_RcptDat->teiki.s_year%100);	// 定期券開始年
		p_NtDat->PassData.SMon = p_RcptDat->teiki.s_mon;				// 定期券開始月
		p_NtDat->PassData.SDate = p_RcptDat->teiki.s_day;				// 定期券開始日
		p_NtDat->PassData.EYear =
								(uchar)(p_RcptDat->teiki.e_year%100);	// 定期券終了年
		p_NtDat->PassData.EMon = p_RcptDat->teiki.e_mon;				// 定期券終了月
		p_NtDat->PassData.EDate = p_RcptDat->teiki.e_day;				// 定期券終了日
	}
	if( p_NtDat->PayMethod == 5 ){
		p_NtDat->PassRenewalPric = p_RcptDat->WPrice;					// 定期券更新料金

		p_NtDat->PassRenewalCondition = 1;								// 定期券更新条件
		if( rangechk( 1, 3, p_RcptDat->teiki.status ) ){				// ｽﾃｰﾀｽ1～3範囲内
			p_NtDat->PassRenewalCondition = 11;							// 定期券更新条件
		}
		p_NtDat->PassRenewalPeriod = p_RcptDat->teiki.update_mon;		// 定期券更新期間(更新月数)
	}

	if (ntnet_decision_credit(&p_RcptDat->credit)) {

	memcpy( p_NtDat->CreditCardNo,
			p_RcptDat->credit.card_no,
			sizeof( p_NtDat->CreditCardNo ) );							// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
	p_NtDat->Credit_ryo = p_RcptDat->credit.pay_ryo;					// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
	p_NtDat->CreditSlipNo = p_RcptDat->credit.slip_no;					// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	p_NtDat->CreditAppNo = p_RcptDat->credit.app_no;					// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号
	memcpy( p_NtDat->CreditName,
			p_RcptDat->credit.card_name,
			sizeof( p_NtDat->CreditName ) );							// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	p_NtDat->CreditDate[0] = p_RcptDat->credit.CreditDate[0];			// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(年)
//	p_NtDat->CreditDate[1] = p_RcptDat->credit.CreditDate[1];			// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(月)
//	p_NtDat->CreditProcessNo = p_RcptDat->credit.CenterProcOiBan;		// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀ処理追い番
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
	}
//	p_NtDat->PayCalMax = p_RcptDat->PayCalMax;							// 最大料金越えあり

	for( i = j = 0; i < WTIK_USEMAX; i++ ){								// 精算中止以外の割引情報コピー
		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&				// 割引種別あり
		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||	// 精算中止割引情報でない
		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 			memcpy( &p_NtDat->DiscountData[j], &p_RcptDat->DiscountData[i], sizeof( DISCOUNT_DATA ) );	// 割引情報
// 			if( p_NtDat->DiscountData[j].DiscSyu == NTNET_PRI_W ){
// 				memset( &p_NtDat->DiscountData[j].uDiscData, 0, 8 );	// 不要データクリア
// 			}
// 			j++;
			if ( p_RcptDat->DiscountData[i].Discount || p_RcptDat->DiscountData[i].DiscCount ) {
				p_NtDat->DiscountData[j].ParkingNo = p_RcptDat->DiscountData[i].ParkingNo;
				p_NtDat->DiscountData[j].DiscSyu   = p_RcptDat->DiscountData[i].DiscSyu;
				p_NtDat->DiscountData[j].DiscNo    = p_RcptDat->DiscountData[i].DiscNo;
				p_NtDat->DiscountData[j].DiscCount = p_RcptDat->DiscountData[i].DiscCount;
				p_NtDat->DiscountData[j].Discount  = p_RcptDat->DiscountData[i].Discount;
				p_NtDat->DiscountData[j].DiscInfo1 = p_RcptDat->DiscountData[i].DiscInfo1;
				p_NtDat->DiscountData[j].uDiscData.common.DiscInfo2 =
					p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;
				p_NtDat->DiscountData[j].uDiscData.common.DiscFlg = 0;
				switch ( p_RcptDat->DiscountData[i].DiscSyu ) {
				case NTNET_SVS_M:			// サービス券割引(金額)
				case NTNET_SVS_T:			// サービス券割引(時間)
				case NTNET_KAK_M:			// 店割引(金額)
				case NTNET_KAK_T:			// 店割引(時間)
				case NTNET_TKAK_M:			// 多店舗割引(金額)
				case NTNET_TKAK_T:			// 多店舗割引(時間)
				case NTNET_SYUBET:			// 種別割引(金額)
				case NTNET_SYUBET_TIME:		// 種別割引(時間)
				case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
				case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
				case NTNET_GENGAKU:			// 減額精算
				case NTNET_FURIKAE:			// 振替精算
				case NTNET_SYUSEI_1:		// 修正精算
				case NTNET_SYUSEI_2:		// 修正精算
				case NTNET_SYUSEI_3:		// 修正精算
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 1;
					break;
				case NTNET_PRI_W:			// 割引プリペイドカード
					memset(&p_NtDat->DiscountData[j].uDiscData, 0, 8);	// 不要データクリア
					break;
				default:
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 0;
					break;
				}
				j++;
			}

			if ( p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscount ||
				 p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscCount ) {
				p_NtDat->DiscountData[j].ParkingNo = p_RcptDat->DiscountData[i].ParkingNo;
				p_NtDat->DiscountData[j].DiscSyu   = p_RcptDat->DiscountData[i].DiscSyu;
				p_NtDat->DiscountData[j].DiscNo    = p_RcptDat->DiscountData[i].DiscNo;
				p_NtDat->DiscountData[j].DiscCount = p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscCount;
				p_NtDat->DiscountData[j].Discount  = p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscount;
				p_NtDat->DiscountData[j].DiscInfo1 = p_RcptDat->DiscountData[i].DiscInfo1;
				p_NtDat->DiscountData[j].uDiscData.common.DiscInfo2 =
					p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;
				p_NtDat->DiscountData[j].uDiscData.common.DiscFlg = 1;
				switch ( p_RcptDat->DiscountData[i].DiscSyu ) {
				case NTNET_SVS_M:			// サービス券割引(金額)
				case NTNET_SVS_T:			// サービス券割引(時間)
				case NTNET_KAK_M:			// 店割引(金額)
				case NTNET_KAK_T:			// 店割引(時間)
				case NTNET_TKAK_M:			// 多店舗割引(金額)
				case NTNET_TKAK_T:			// 多店舗割引(時間)
				case NTNET_SYUBET:			// 種別割引(金額)
				case NTNET_SYUBET_TIME:		// 種別割引(時間)
				case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
				case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
				case NTNET_GENGAKU:			// 減額精算
				case NTNET_FURIKAE:			// 振替精算
				case NTNET_SYUSEI_1:		// 修正精算
				case NTNET_SYUSEI_2:		// 修正精算
				case NTNET_SYUSEI_3:		// 修正精算
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 1;
					break;
				case NTNET_PRI_W:			// 割引プリペイドカード
					memset(&p_NtDat->DiscountData[j].uDiscData, 0, 8);	// 不要データクリア
					break;
				default:
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 0;
					break;
				}
				j++;
			}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
		}
	}
	// 22 -> 56に追加されたﾃﾞｰﾀの設定(FmtRev, PARKCAR_DATA1)
	p_NtDat->FmtRev = 0;					// フォーマットRev.№
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	p_NtDat->ParkData.FullNo1 	= p_RcptDat->FullNo1;					// 満車台数１
//	p_NtDat->ParkData.CarCnt1 	= p_RcptDat->CarCnt1;					// 現在台数１
//	p_NtDat->ParkData.FullNo2 	= p_RcptDat->FullNo2;					// 満車台数２
//	p_NtDat->ParkData.CarCnt2 	= p_RcptDat->CarCnt2;					// 現在台数２
//	p_NtDat->ParkData.FullNo3 	= p_RcptDat->FullNo3;					// 満車台数３
//	p_NtDat->ParkData.CarCnt3 	= p_RcptDat->CarCnt3;					// 現在台数３
//	p_NtDat->ParkData.PascarCnt = p_RcptDat->PascarCnt;					// 定期車両カウント
//	p_NtDat->ParkData.Full[0] 	= p_RcptDat->Full[0];					// 駐車1満空状態
//	p_NtDat->ParkData.Full[1] 	= p_RcptDat->Full[1];					// 駐車2満空状態
//	p_NtDat->ParkData.Full[2] 	= p_RcptDat->Full[2];					// 駐車3満空状態
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	ret = sizeof( DATA_KIND_56 ) - sizeof( DISCOUNT_DATA );				// 割引情報は24個まで
	ret = sizeof( DATA_KIND_56 ) - sizeof( t_SeisanDiscountOld );		// 割引情報は24個まで
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	return ret;
}
//--------------------------------------------------------------
//	新精算NT-NETフォーマット設定(22ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA1を付加)	// 参照 = NTNET_Edit_Data22_SK
unsigned short	NTNET_Edit_Data56_SK(	Receipt_data 	*p_RcptDat,	// 精算logﾃﾞｰﾀ(IN)
										DATA_KIND_56 	*p_NtDat )	// 精算NT-NET(OUT)
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_56 ) );

	p_NtDat->DataBasic.SystemID = p_RcptDat->DataBasic.SystemID;		// ｼｽﾃﾑID
	p_NtDat->DataBasic.DataKind = p_RcptDat->DataBasic.DataKind;		// ﾃﾞｰﾀ種別
	p_NtDat->DataBasic.DataKeep = p_RcptDat->DataBasic.DataKeep;		// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	p_NtDat->DataBasic.ParkingNo = p_RcptDat->DataBasic.ParkingNo;		// 駐車場№
	p_NtDat->DataBasic.ModelCode = p_RcptDat->DataBasic.ModelCode;		// 機種ｺｰﾄﾞ
	p_NtDat->DataBasic.MachineNo = p_RcptDat->DataBasic.MachineNo;		// 機械№
	memcpy( &p_NtDat->DataBasic.Year, &p_RcptDat->DataBasic.Year, 6 );	// 処理年月日時分秒
	p_NtDat->PayCount = CountSel( &p_RcptDat->Oiban );					// 精算or精算中止追い番
	p_NtDat->PayMethod = p_RcptDat->PayMethod;							// 精算方法
	p_NtDat->PayClass = p_RcptDat->PayClass;							// 処理区分
	p_NtDat->PayMode = 0;												// 精算ﾓｰﾄﾞ(自動精算)
	p_NtDat->LockNo = p_RcptDat->WPlace;								// 区画情報
	p_NtDat->CardType = 0;												// 駐車券ﾀｲﾌﾟ
	p_NtDat->CMachineNo = 0;											// 駐車券機械№
	p_NtDat->CardNo = 0L;												// 駐車券番号(発券追い番)
	p_NtDat->OutTime.Year = p_RcptDat->TOutTime.Year;					// 出庫年
	p_NtDat->OutTime.Mon = p_RcptDat->TOutTime.Mon;						// 出庫月
	p_NtDat->OutTime.Day = p_RcptDat->TOutTime.Day;						// 出庫日
	p_NtDat->OutTime.Hour = p_RcptDat->TOutTime.Hour;					// 出庫時
	p_NtDat->OutTime.Min = p_RcptDat->TOutTime.Min;						// 出庫分
	p_NtDat->OutTime.Sec = 0;											// 出庫秒
	p_NtDat->KakariNo = p_RcptDat->KakariNo;							// 係員№
	p_NtDat->OutKind = p_RcptDat->OutKind;								// 精算出庫
	p_NtDat->InTime.Year = p_RcptDat->TInTime.Year;						// 入庫年
	p_NtDat->InTime.Mon = p_RcptDat->TInTime.Mon;						// 入庫月
	p_NtDat->InTime.Day = p_RcptDat->TInTime.Day;						// 入庫日
	p_NtDat->InTime.Hour = p_RcptDat->TInTime.Hour;						// 入庫時
	p_NtDat->InTime.Min = p_RcptDat->TInTime.Min;						// 入庫分
	p_NtDat->InTime.Sec = 0;											// 入庫秒
	// 精算年月日時分秒は0とする
	// 前回精算年月日時分秒は0とする
	p_NtDat->TaxPrice = 0;												// 課税対象額
	p_NtDat->TotalPrice = 0;											// 合計金額(HOST未使用のため)
	p_NtDat->Tax = p_RcptDat->Wtax;										// 消費税額
	p_NtDat->Syubet = p_RcptDat->syu;									// 料金種別
	p_NtDat->Price = p_RcptDat->WPrice;									// 駐車料金
	p_NtDat->PassCheck = 1;												// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	p_NtDat->CountSet = p_RcptDat->CountSet;							// 在車ｶｳﾝﾄしない
//	p_NtDat->PayCalMax = p_RcptDat->PayCalMax;							// 最大料金越えあり

	// 22 -> 56に追加されたﾃﾞｰﾀの設定(FmtRev, PARKCAR_DATA1)
	p_NtDat->FmtRev = 0;					// フォーマットRev.№
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	p_NtDat->ParkData.FullNo1 	= p_RcptDat->FullNo1;					// 満車台数１
//	p_NtDat->ParkData.CarCnt1 	= p_RcptDat->CarCnt1;					// 現在台数１
//	p_NtDat->ParkData.FullNo2 	= p_RcptDat->FullNo2;					// 満車台数２
//	p_NtDat->ParkData.CarCnt2 	= p_RcptDat->CarCnt2;					// 現在台数２
//	p_NtDat->ParkData.FullNo3 	= p_RcptDat->FullNo3;					// 満車台数３
//	p_NtDat->ParkData.CarCnt3 	= p_RcptDat->CarCnt3;					// 現在台数３
//	p_NtDat->ParkData.PascarCnt = p_RcptDat->PascarCnt;					// 定期車両カウント
//	p_NtDat->ParkData.Full[0] 	= p_RcptDat->Full[0];					// 駐車1満空状態
//	p_NtDat->ParkData.Full[1] 	= p_RcptDat->Full[1];					// 駐車2満空状態
//	p_NtDat->ParkData.Full[2] 	= p_RcptDat->Full[2];					// 駐車3満空状態
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	ret = sizeof( DATA_KIND_56 ) - sizeof( DISCOUNT_DATA );				// 割引情報は24個まで
	ret = sizeof( DATA_KIND_56 ) - sizeof( t_SeisanDiscountOld );		// 割引情報は24個まで
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	return ret;
}

// MH321800(S) Y.Tanizaki ICクレジット対応
////	新精算NT-NETフォーマット設定(22ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA1を付加)	// 参照 = NTNET_Edit_Data22
//unsigned short	NTNET_Edit_Data56_r10(	Receipt_data 		*p_RcptDat,		// 精算logﾃﾞｰﾀ(IN)
//										DATA_KIND_56_r10 	*p_NtDat )	// 精算NT-NET(OUT)
//{
//	uchar	i;
//	uchar	j;
//	ushort	ret;
//	EMONEY	t_EMoney;
//	CREINFO	t_Credit;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_56_r10 ) );
//
//	NTNET_Edit_BasicData( 57, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// 基本ﾃﾞｰﾀ作成
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TOutTime.Year % 100);		// 精算年
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TOutTime.Mon;				// 精算月
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TOutTime.Day;				// 精算日
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TOutTime.Hour;				// 精算時
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TOutTime.Min;				// 精算分
//	p_NtDat->DataBasic.Sec	= 0;
//	p_NtDat->FmtRev = 10;													// フォーマットRev.№
//	p_NtDat->CenterSeqNo = p_RcptDat->CenterSeqNo;							// センター追番（精算）
//
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->FullNo1;						// 満車台数１
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CarCnt1;						// 現在台数１
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->FullNo2;						// 満車台数２
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CarCnt2;						// 現在台数２
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->FullNo3;						// 満車台数３
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CarCnt3;						// 現在台数３
//	p_NtDat->ParkData.CarFullFlag	= (uchar)0;								// 台数・満空状態フラグ
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->PascarCnt;				// 定期車両カウント
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->Full[0];						// 駐車1満空状態
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->Full[1];						// 駐車2満空状態
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->Full[2];						// 駐車3満空状態
//
//	p_NtDat->PayCount	= CountSel( &p_RcptDat->Oiban);						// 精算or精算中止追い番
//	p_NtDat->PayMethod	= (uchar)p_RcptDat->PayMethod;						// 精算方法
//	if( p_RcptDat->PayClass == 4 ){
//		// クレジット精算
//		p_NtDat->PayClass	= (uchar)0;										// 処理区分
//	}else{
//		p_NtDat->PayClass	= (uchar)p_RcptDat->PayClass;					// 処理区分
//	}
//	p_NtDat->PayMode	= (uchar)p_RcptDat->PayMode;						// 精算ﾓｰﾄﾞ(自動精算)
//	p_NtDat->CMachineNo	= 0;												// 駐車券機械№
//
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->FlapArea	= (ushort)(p_RcptDat->WPlace/10000);			// フラップシステム	区画		0～99
//		p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->WPlace%10000);			// 					車室番号	0～9999
//		p_NtDat->KakariNo	= p_RcptDat->KakariNo;							// 係員№
//		p_NtDat->OutKind	= p_RcptDat->OutKind;							// 精算出庫
//	}
//
//	p_NtDat->CountSet	= p_RcptDat->CountSet;								// 在車カウントしない
//
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->InPrev_ymdhms.In_Year	= (uchar)(p_RcptDat->TInTime.Year%100);	// 入庫年
//		p_NtDat->InPrev_ymdhms.In_Mon	= p_RcptDat->TInTime.Mon;				// 入庫月
//		p_NtDat->InPrev_ymdhms.In_Day	= p_RcptDat->TInTime.Day;				// 入庫日
//		p_NtDat->InPrev_ymdhms.In_Hour	= p_RcptDat->TInTime.Hour;				// 入庫時
//		p_NtDat->InPrev_ymdhms.In_Min	= p_RcptDat->TInTime.Min;				// 入庫分
//		p_NtDat->InPrev_ymdhms.In_Sec	= 0;									// 入庫秒
//
//		// 前回精算年月日時分秒は0とする
//		p_NtDat->InPrev_ymdhms.Prev_Year	= 0;							// 前回精算年
//		p_NtDat->InPrev_ymdhms.Prev_Mon		= 0;							// 前回精算月
//		p_NtDat->InPrev_ymdhms.Prev_Day		= 0;							// 前回精算日
//		p_NtDat->InPrev_ymdhms.Prev_Hour	= 0;							// 前回精算時
//		p_NtDat->InPrev_ymdhms.Prev_Min		= 0;							// 前回精算分
//		p_NtDat->InPrev_ymdhms.Prev_Sec		= 0;							// 前回精算秒
//	}
//
//	p_NtDat->ReceiptIssue	= p_RcptDat->ReceiptIssue;						// 領収証発行有無
//
//	if( p_NtDat->PayMethod != 5 ){											// 更新精算以外
//		p_NtDat->Syubet	= p_RcptDat->syu;									// 料金種別
//		p_NtDat->Price	= p_RcptDat->WPrice;								// 駐車料金
//	}
//
//	p_NtDat->CashPrice	= (ulong)p_RcptDat->WTotalPrice;					// 現金売上
//	p_NtDat->InPrice	= p_RcptDat->WInPrice;								// 投入金額
//	p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// 釣銭金額
//
//	p_NtDat->MoneyInOut.MoneyKind_In	= 0x1f;								// 金種有無	搭載金種の有無　投入金（10円,50円,100円,500円,1000円）
//	p_NtDat->MoneyInOut.In_10_cnt		= (uchar)p_RcptDat->in_coin[0];		// 投入金枚数(10円)		0～255
//	p_NtDat->MoneyInOut.In_50_cnt		= (uchar)p_RcptDat->in_coin[1];		// 投入金枚数(50円)		0～255
//	p_NtDat->MoneyInOut.In_100_cnt		= (uchar)p_RcptDat->in_coin[2];		// 投入金枚数(100円)	0～255
//	p_NtDat->MoneyInOut.In_500_cnt		= (uchar)p_RcptDat->in_coin[3];		// 投入金枚数(500円)	0～255
//	p_NtDat->MoneyInOut.In_1000_cnt		= (uchar)p_RcptDat->in_coin[4];		// 投入金枚数(1000円)	0～255
//	if(p_RcptDat->f_escrow) {												// エスクロ紙幣戻しあり
//		++p_NtDat->MoneyInOut.In_1000_cnt;									// エスクロ紙幣戻し分を投入枚数に加算
//	}
//
//	p_NtDat->MoneyInOut.MoneyKind_Out	= 0x1f;								// 金種有無	搭載金種の有無　払出金（10円,50円,100円,500円,1000円）
//	p_NtDat->MoneyInOut.Out_10_cnt		= (uchar)p_RcptDat->out_coin[0];	// 払出金枚数(10円)		0～255
//	p_NtDat->MoneyInOut.Out_50_cnt		= (uchar)p_RcptDat->out_coin[1];	// 払出金枚数(50円)		0～255
//	p_NtDat->MoneyInOut.Out_100_cnt		= (uchar)p_RcptDat->out_coin[2];	// 払出金枚数(100円)	0～255
//	p_NtDat->MoneyInOut.Out_500_cnt		= (uchar)p_RcptDat->out_coin[3];	// 払出金枚数(500円)	0～255
//	p_NtDat->MoneyInOut.Out_1000_cnt	= (uchar)p_RcptDat->f_escrow;		// 払出金枚数(1000円)	0～255
//
//	p_NtDat->HaraiModoshiFusoku	= (ushort)p_RcptDat->WFusoku;				// 払戻不足額	0～9999
//
//	p_NtDat->PassCheck = p_RcptDat->PassCheck;								// アンチパスチェック
//
//	if( p_RcptDat->teiki.ParkingNo ) {
//		p_NtDat->ParkingNo	= p_RcptDat->teiki.ParkingNo;								// 定期券駐車場№
//		p_NtDat->Media[0].MediaKind	= (ushort)(p_RcptDat->teiki.pkno_syu + 2);			// 精算媒体情報１　種別(メイン媒体)	0～99
//		intoasl(p_NtDat->Media[0].MediaCardNo, p_RcptDat->teiki.id, 5);					// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		intoasl(p_NtDat->Media[0].MediaCardInfo, p_RcptDat->teiki.syu, 2);				// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		// 精算媒体情報2（サブ媒体）は オール０とする
//	}
//
//	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 ){	// 電子決済種別 Suica:1, Edy:2
//		p_NtDat->CardKind = (ushort)2;											// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	}else if( p_RcptDat->PayClass == 4 ){					// クレジット精算
//		p_NtDat->CardKind = (ushort)1;											// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	}
//
//	switch( p_NtDat->CardKind ){							// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	case 1:
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
//		memset( &t_Credit, 0, sizeof( t_Credit ) );
//		t_Credit.amount = (ulong)p_RcptDat->credit.pay_ryo;									// 決済額
//		memcpyFlushLeft( &t_Credit.card_no[0], (uchar *)&p_RcptDat->credit.card_no[0], sizeof(t_Credit.card_no), sizeof(p_RcptDat->credit.card_no) );	// 会員番号
//		memcpyFlushLeft( &t_Credit.cct_num[0], (uchar *)&p_RcptDat->credit.CCT_Num[0],
//							sizeof(t_Credit.cct_num), CREDIT_TERM_ID_NO_SIZE );				// 端末識別番号
//		memcpyFlushLeft( &t_Credit.kid_code[0], &p_RcptDat->credit.kid_code[0], sizeof(t_Credit.kid_code), sizeof(p_RcptDat->credit.kid_code) );		// KID コード
//		t_Credit.app_no = p_RcptDat->credit.app_no;											// 承認番号
//		t_Credit.center_oiban = 0;															// ｾﾝﾀｰ処理追番(センターは0固定)
//		
//		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );							// AES 暗号化
//		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// 決済情報
//		break;
//	case 2:
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
//		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//		t_EMoney.amount = p_RcptDat->Electron_data.Suica.pay_ryo;							// 電子決済精算情報　決済額
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//		memcpy( &t_EMoney.card_id[0], &p_RcptDat->Electron_data.Suica.Card_ID[0],
//				sizeof(p_RcptDat->Electron_data.Suica.Card_ID) );							// 電子決済精算情報　ｶｰﾄﾞID (Ascii 16桁)
//		t_EMoney.card_zangaku = p_RcptDat->Electron_data.Suica.pay_after;					// 電子決済精算情報　決済後残高
//		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );							// AES 暗号化
//		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// 決済情報
//		break;
//	}
//
//	p_NtDat->MoneyIn		= (ulong)0;										// タイムレジ情報：入金処理金額
//	p_NtDat->MoneyOut		= (ulong)0;										// タイムレジ情報：出金処理金額
//	p_NtDat->MoneyBack		= (ulong)0;										// タイムレジ情報：払戻金額
//	p_NtDat->MoneyFusoku	= (ulong)0;										// タイムレジ情報：受取り不足金額
//
//	for( i = j = 0; i < WTIK_USEMAX; i++ ){									// 精算中止以外の割引情報コピー
//		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&					// 割引種別あり
//		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// 精算中止割引情報でない
//		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
//		    if( p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA以外(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(支払額・残額)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO以外(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(支払額・残額)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY以外	(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(支払額・残額)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA以外(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(支払額・残額)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card以外(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_2 ) 		//			(支払額・残額)
//		    {
//				p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DiscountData[i].ParkingNo;						// 駐車場No.
//				p_NtDat->SDiscount[j].Kind		= p_RcptDat->DiscountData[i].DiscSyu;						// 割引種別
//				p_NtDat->SDiscount[j].Group		= p_RcptDat->DiscountData[i].DiscNo;						// 割引区分
//				p_NtDat->SDiscount[j].Callback	= p_RcptDat->DiscountData[i].DiscCount;						// 回収枚数
//				p_NtDat->SDiscount[j].Amount	= p_RcptDat->DiscountData[i].Discount;						// 割引額
//				p_NtDat->SDiscount[j].Info1		= p_RcptDat->DiscountData[i].DiscInfo1;						// 割引情報1
//				if( p_NtDat->SDiscount[j].Kind != NTNET_PRI_W ){
//					p_NtDat->SDiscount[j].Info2	= p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;	// 割引情報2
//				}
//				j++;
//		    }
//		}
//	}
//
//	ret = sizeof( DATA_KIND_56_r10 );
//
//	return ret;
//}
// MH321800(E) Y.Tanizaki ICクレジット対応

//--------------------------------------------------------------
//	WEB用駐車台数用NT-NETフォーマット設定(236ﾌｫｰﾏｯﾄとは別に58ﾌｫｰﾏｯﾄを作成)	// 参照 = NTNET_Edit_Data236
unsigned short	NTNET_Edit_Data58(	ParkCar_log		*p_RcptDat,	// 駐車台数用logﾃﾞｰﾀ(IN)
									DATA_KIND_58 	*p_NtDat )	// 駐車台数用NT-NET(OUT)
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_58 ) );
	NTNET_Edit_BasicData( 58, 1, p_RcptDat->CMN_DT.DT_58.SeqNo, &p_NtDat->DataBasic);	// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Time.Year % 100);					// 処理年(ログ作成時に下2桁にしている)
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Time.Mon;							// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Time.Day;							// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Time.Hour;							// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Time.Min;							// 処理分
	p_NtDat->DataBasic.Sec = (uchar)p_RcptDat->Time.Sec;							// 処理秒

	// 236 -> 58(別ﾌｫｰﾏｯﾄ)にﾃﾞｰﾀの設定(FmtRev, PARKCAR_DATA2)
	p_NtDat->FmtRev = 0;					// フォーマットRev.№
	p_NtDat->ParkData.GroupNo	= p_RcptDat->GroupNo;	// ｸﾞﾙｰﾌﾟ番号
	p_NtDat->ParkData.State		= p_RcptDat->State;	// 用途別駐車台数設定
	p_NtDat->ParkData.Full[0]	= p_RcptDat->Full[0];	// 駐車1満空状態
	p_NtDat->ParkData.Full[1]	= p_RcptDat->Full[1];	// 駐車2満空状態
	p_NtDat->ParkData.Full[2]	= p_RcptDat->Full[2];	// 駐車3満空状態
	p_NtDat->ParkData.EmptyNo1	= p_RcptDat->CMN_DT.DT_58.EmptyNo1;	// 空車台数１
	p_NtDat->ParkData.FullNo1	= p_RcptDat->CMN_DT.DT_58.FullNo1;	// 満車台数１
	p_NtDat->ParkData.EmptyNo2	= p_RcptDat->CMN_DT.DT_58.EmptyNo2;	// 空車台数２
	p_NtDat->ParkData.FullNo2	= p_RcptDat->CMN_DT.DT_58.FullNo2;	// 満車台数２
	p_NtDat->ParkData.EmptyNo3	= p_RcptDat->CMN_DT.DT_58.EmptyNo3;	// 空車台数３
	p_NtDat->ParkData.FullNo3	= p_RcptDat->CMN_DT.DT_58.FullNo3;	// 満車台数３

	ret = sizeof( DATA_KIND_58 ); 
	return ret;
}
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
////[]----------------------------------------------------------------------[]
/////	@brief			集計基本データ編集処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		syukei	: ログから取り出した集計データのポインタ
/////	@param[in]		Type	: 集計タイプ
/////	@param[out]		p_NtDat	: 集計終了通知データ(DATA_KIND_42型)へのポインタ  
/////	@return			ret		: 集計終了通知データのデータサイズ(システムID～) 
////[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//unsigned short	NTNET_Edit_SyukeiKihon_r10( SYUKEI *syukei, ushort Type, DATA_KIND_42 *p_NtDat )
//{
//	static const ushort c_coin[COIN_SYU_CNT] = {10, 50, 100, 500};
//	ushort	i;
//	ushort	ret;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_42 ) );
//	NTNET_Edit_BasicData( 42, 0, syukei->SeqNo[0], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0
//
//	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
//	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
//	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
//	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
//	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
//	p_NtDat->DataBasic.Sec = 0;													// 処理秒
//
//	p_NtDat->FmtRev					= 10;										// フォーマットRev.No.
//	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// 集計タイプ 1:T, 2:GT
//	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
//	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// 集計追番
//	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
//	case	0:		// 個別追番
//		if (Type == 11) {														// GT集計
//			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// 開始追番
//			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// 終了追番
//		}
//		break;
//	case	1:		// 通し追い番
//		if (Type == 1 || Type == 11) {
//			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// 開始追番
//			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// 終了追番
//		}
//		break;
//	default:
//		break;
//	}
//	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// センター追番（集計）
//	p_NtDat->LastTime.Year = (uchar)(syukei->OldTime.Year % 100 );				// 処理年
//	p_NtDat->LastTime.Mon = (uchar)syukei->OldTime.Mon;							// 処理月
//	p_NtDat->LastTime.Day = (uchar)syukei->OldTime.Day;							// 処理日
//	p_NtDat->LastTime.Hour = (uchar)syukei->OldTime.Hour;						// 処理時
//	p_NtDat->LastTime.Min = (uchar)syukei->OldTime.Min;							// 処理分
//	p_NtDat->LastTime.Sec = 0;													// 処理秒
//	p_NtDat->SettleNum							= syukei->Seisan_Tcnt;			// 総精算回数
//	p_NtDat->Kakeuri							= syukei->Kakeuri_Tryo;			// 総掛売額
//	p_NtDat->Cash								= syukei->Genuri_Tryo;			// 総現金売上額
//	p_NtDat->Uriage								= syukei->Uri_Tryo;				// 総売上額
//	p_NtDat->Tax								= syukei->Tax_Tryo;				// 総消費税額
//	p_NtDat->Charge								= syukei->Turi_modosi_ryo;		// 釣銭払戻額
//	p_NtDat->CoinTotalNum						= (ushort)syukei->Ckinko_goukei_cnt;	// コイン金庫合計回数
//	p_NtDat->NoteTotalNum						= (ushort)syukei->Skinko_goukei_cnt;	// 紙幣金庫合計回数
//	p_NtDat->CyclicCoinTotalNum					= (ushort)syukei->Junkan_goukei_cnt;	// 循環コイン合計回数
//	p_NtDat->NoteOutTotalNum					= (ushort)syukei->Siheih_goukei_cnt;	// 紙幣払出機合計回数
//	p_NtDat->SettleNumServiceTime				= syukei->In_svst_seisan;		// サービスタイム内精算回数
//	p_NtDat->Shortage.Num						= syukei->Harai_husoku_cnt;		// 払出不足回数
//	p_NtDat->Shortage.Amount					= syukei->Harai_husoku_ryo;		// 払出不足金額
//	p_NtDat->Cancel.Num							= syukei->Seisan_chusi_cnt;		// 精算中止回数
//	p_NtDat->Cancel.Amount						= syukei->Seisan_chusi_ryo;		// 精算中止金額
//	p_NtDat->AntiPassOffSettle					= syukei->Apass_off_seisan;		// アンチパスOFF精算回数
//	p_NtDat->ReceiptIssue						= syukei->Ryosyuu_pri_cnt;		// 領収証発行枚数
//	p_NtDat->WarrantIssue						= syukei->Azukari_pri_cnt;		// 預り証発行枚数
//// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
////	p_NtDat->AllSystem.CarOutIllegal.Num		= syukei->Husei_out_Tcnt;		// 全装置  不正出庫回数
////	p_NtDat->AllSystem.CarOutIllegal.Amount		= syukei->Husei_out_Tryo;		//                 金額
////	p_NtDat->AllSystem.CarOutForce.Num			= syukei->Kyousei_out_Tcnt;		//         強制出庫回数
////	p_NtDat->AllSystem.CarOutForce.Amount		= syukei->Kyousei_out_Tryo;		//                 金額
////	p_NtDat->AllSystem.AcceptTicket				= syukei->Uketuke_pri_Tcnt;		//         受付券発行回数
////	p_NtDat->AllSystem.ModifySettle.Num			= syukei->Syuusei_seisan_Tcnt;	// 修正精算回数
////	p_NtDat->AllSystem.ModifySettle.Amount		= syukei->Syuusei_seisan_Tryo;	//         金額
////	p_NtDat->CarInTotal							= syukei->In_car_Tcnt;			// 総入庫台数
////	p_NtDat->CarOutTotal						= syukei->Out_car_Tcnt;			// 総出庫台数
////	p_NtDat->CarIn1								= syukei->In_car_cnt[0];		// 入庫1入庫台数
////	p_NtDat->CarOut1							= syukei->Out_car_cnt[0];		// 出庫1出庫台数
////	p_NtDat->CarIn2								= syukei->In_car_cnt[1];		// 入庫2入庫台数
////	p_NtDat->CarOut2							= syukei->Out_car_cnt[1];		// 出庫2出庫台数
////	p_NtDat->CarIn3								= syukei->In_car_cnt[2];		// 入庫3入庫台数
////	p_NtDat->CarOut3							= syukei->Out_car_cnt[2];		// 出庫3出庫台数
//// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// 強制完了ｷｰでの未入金は基本ﾃﾞｰﾀに項目を新設して送信する
//	p_NtDat->MiyoCount							= syukei->Syuusei_seisan_Mcnt;	// 未入金回数
//	p_NtDat->MiroMoney							= syukei->Syuusei_seisan_Mryo;	// 未入金額
//	p_NtDat->LagExtensionCnt					= syukei->Lag_extension_cnt;	// ラグタイム延長回数
//	// 金銭関連情報
//	p_NtDat->Total					= syukei->Kinko_Tryo;						// 金庫総入金額
//	p_NtDat->NoteTotal				= syukei->Note_Tryo;						// 紙幣金庫総入金額
//	p_NtDat->CoinTotal				= syukei->Coin_Tryo;						// コイン金庫総入金額
//	for (i = 0; i < COIN_SYU_CNT; i++) {										// コイン1～4
//		p_NtDat->Coin[i].Kind		= c_coin[i];								//        金種
//		p_NtDat->Coin[i].Num		= (ushort)syukei->Coin_cnt[i];				//        枚数
//	}
//	p_NtDat->Note[0].Kind			= 1000;										// 紙幣1  金種
//	p_NtDat->Note[0].Num2			= (ushort)syukei->Note_cnt[0];				//        紙幣枚数
//
//	ret = sizeof( DATA_KIND_42 ); 
//	return ret;
//}
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//[]----------------------------------------------------------------------[]
///	@brief			料金種別毎集計データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 料金種別毎集計データ(DATA_KIND_43型)へのポインタ  
///	@return			ret		: 料金種別毎集計データのデータサイズ(システムID～) 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//unsigned short	NTNET_Edit_SyukeiRyokinMai_r10( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat )
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//unsigned short	NTNET_Edit_SyukeiRyokinMai_r13( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiRyokinMai_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
{
	ushort	i;
	ushort	j;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_43 ) );
	NTNET_Edit_BasicData( 43, 0, syukei->SeqNo[1], &p_NtDat->DataBasic);		// Ｔ合計：データ保持フラグ = 0
	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//	p_NtDat->FmtRev					= 10;										// フォーマットRev.No.
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	p_NtDat->FmtRev					= 13;										// フォーマットRev.No.
	p_NtDat->FmtRev					= 15;										// フォーマットRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// 集計タイプ 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// 集計追番
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// 個別追番
		if (Type == 11) {														// GT集計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// 終了追番
		}
		break;
	case	1:		// 通し追い番
		if (Type == 1 || Type == 11) {
		// T合計／小計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// 終了追番
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// センター追番（集計）
	for (i = 0, j = 0; i < RYOUKIN_SYU_CNT; i++) {								// 種別01～50
// GG124100(S) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
// 		if( prm_get( COM_PRM, S_SHA, (short)(i*6+1), 2 , 5 ) == 1 ) {			// 料金種別の設定あり
		if ( (CLOUD_CALC_MODE &&												// クラウド料金計算モード
			  (syukei->Rsei_cnt[i] ||											//  精算回数データあり
			   syukei->Rsei_ryo[i] ||											//  売上額データあり
			   syukei->Rtwari_cnt[i] ||											//  割引回数データあり
			   syukei->Rtwari_ryo[i])) ||										//  割引額データあり
			 (!CLOUD_CALC_MODE &&												// 通常料金計算モード
			  (prm_get(COM_PRM, S_SHA, (short)(i * 6 + 1), 2 , 5) == 1)) ) {	//  設定あり
// GG124100(E) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
			// 設定されている料金種別は、使用されていない場合でも集計項目をセットする
			p_NtDat->Kind[j].Kind	= i + 1;									// 料金種別
			p_NtDat->Kind[j].Settle.Num			= syukei->Rsei_cnt[i];			// 精算回数
			p_NtDat->Kind[j].Settle.Amount		= syukei->Rsei_ryo[i];			// 売上額
			p_NtDat->Kind[j].Discount.Num		= syukei->Rtwari_cnt[i];		// 割引回数
			p_NtDat->Kind[j].Discount.Amount	= syukei->Rtwari_ryo[i];		// 割引額
			j++;
		}
	}

	ret = sizeof( DATA_KIND_43 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			割引集計データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 割引集計データ(DATA_KIND_45型)へのポインタ  
///	@return			ret		: 割引集計データのデータサイズ(システムID～) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//unsigned short	NTNET_Edit_SyukeiWaribiki_r10( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat )
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//unsigned short	NTNET_Edit_SyukeiWaribiki_r13( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiWaribiki_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
{
	int		i;
	int		parking;
	int		group;
// GG124100(S) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
// 	uchar	set;
// GG124100(E) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
	ushort	pos;
	ushort	ret;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//// 仕様変更(S) K.Onodera 2016/11/02 割引集計データフォーマット対応
//	ushort	mod;
//// 仕様変更(E) K.Onodera 2016/11/02 割引集計データフォーマット対応
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
// MH321800(S) hosoda ICクレジット対応
	ulong	sei_cnt, sei_ryo;		// 集計用
	int	j;
// MH321800(E) hosoda ICクレジット対応
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
	ushort	brand_no;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引集計データ編集処理)
	BOOL	wk_flag = FALSE;
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引集計データ編集処理)

	wk_media_Type = 0;
	
	memset( p_NtDat, 0, sizeof( DATA_KIND_45 ) );
	NTNET_Edit_BasicData( 45, 0, syukei->SeqNo[2], &p_NtDat->DataBasic);		// Ｔ合計：データ保持フラグ = 0

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//	p_NtDat->FmtRev					= 10;										// フォーマットRev.No.
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	p_NtDat->FmtRev					= 13;										// フォーマットRev.No.
	p_NtDat->FmtRev					= 15;										// フォーマットRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// 集計タイプ 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// 集計追番
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// 個別追番
		if (Type == 11) {														// GT集計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// 終了追番
		}
		break;
	case	1:		// 通し追い番
		if (Type == 1 || Type == 11) {
		// T合計／小計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// 終了追番
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// センター追番（集計）
	i = 0;

// クレジット
// MH321801(S) D.Inaba ICクレジット対応
//// MH321800(S) hosoda ICクレジット対応
////	if( CREDIT_ENABLED() ){
//	if( isEcBrandNoEnabledForSetting(EC_CHECK_CREDIT) ){
//// MH321800(E) hosoda ICクレジット対応
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//	if( isEcBrandNoEnabledForSetting(BRANDNO_CREDIT) ){
	if( isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT) ||
		syukei->Ccrd_sei_cnt != 0 || syukei->Ccrd_sei_ryo != 0){
		// ブランドテーブルを検索して見つからなくても集計されていれば
		// 割引集計データにセットする
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
// MH321801(E) D.Inaba ICクレジット対応

		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 基本駐車場番号ｾｯﾄ

		p_NtDat->Discount[i].Kind		= 30;
		p_NtDat->Discount[i].Num	= syukei->Ccrd_sei_cnt;
		p_NtDat->Discount[i].Amount	= syukei->Ccrd_sei_ryo;
		i++;
	}

// サービス券
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if(CPrmSS[S_SYS][1+parking]) {											// 駐車場設定あり
			if(parking == 0) {													// 基本駐車場
				pos = 6;
			}
			else if(parking == 1) {												// 拡張1駐車場
				pos = 1;
			}
			else if(parking == 2) {												// 拡張3駐車場
				pos = 2;
			}
			else if(parking == 3) {												// 拡張3駐車場
				pos = 3;
			}
			if(prm_get(COM_PRM, S_SYS, 71, 1, pos)) {							// 対象の駐車場で使用可
				for (group = 0; group < SERVICE_SYU_CNT; group++) {
// GG124100(S) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
// 					set = (uchar)prm_get(COM_PRM, S_SER, 1 + 3 * group, 1, 1);
// 					if (set >= 1 && set <= 4) {										// 種別使用設定あり
					if ( (CLOUD_CALC_MODE &&												// クラウド料金計算モード
						  (syukei->Stik_use_cnt[parking][group] ||							//  枚数データあり
						   syukei->Stik_use_ryo[parking][group])) ||						//  金額データあり
						 (!CLOUD_CALC_MODE &&												// 通常料金計算モード
						  (prm_get(COM_PRM, S_SER, (short)(1 + 3 * group), 1, 1) >= 1) &&	//  設定あり
						  (prm_get(COM_PRM, S_SER, (short)(1 + 3 * group), 1, 1) <= 4)) ) {		
// GG124100(E) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
						// 設定があれば使用の有無に拘らず設定する
						p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
						p_NtDat->Discount[i].Kind		= 1;
						p_NtDat->Discount[i].Group		= group + 1;
						p_NtDat->Discount[i].Num		= syukei->Stik_use_cnt[parking][group];
						p_NtDat->Discount[i].Amount		= syukei->Stik_use_ryo[parking][group];
						i++;
					}
				}
			}
		}
	}

// プリペイド
	if(1 == prm_get(COM_PRM, S_PRP, 1, 1, 1)) {									// プリペイド設定あり
		for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
			if (CPrmSS[S_SYS][1+parking]) {
				if(parking == 0) {												// 基本駐車場
					pos = 6;
				}
				else if(parking == 1) {											// 拡張1駐車場
					pos = 1;
				}
				else if(parking == 2) {											// 拡張3駐車場
					pos = 2;
				}
				else if(parking == 3) {											// 拡張3駐車場
					pos = 3;
				}
				if(prm_get(COM_PRM, S_SYS, 72, 1, pos)) {						// 対象の駐車場で使用可
					// 設定があれば使用の有無に拘らず設定する
					p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
					p_NtDat->Discount[i].Kind		= 11;
					p_NtDat->Discount[i].Num		= syukei->Pcrd_use_cnt[parking];
					p_NtDat->Discount[i].Amount		= syukei->Pcrd_use_ryo[parking];
					i++;
				}
			}
		}
	}

// 回数券
	if(2 == prm_get(COM_PRM, S_PRP, 1, 1, 1)) {									// 回数券設定あり
		for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
			if (CPrmSS[S_SYS][1+parking]) {
				if(parking == 0) {												// 基本駐車場
					pos = 6;
				}
				else if(parking == 1) {											// 拡張1駐車場
					pos = 1;
				}
				else if(parking == 2) {											// 拡張3駐車場
					pos = 2;
				}
				else if(parking == 3) {											// 拡張3駐車場
					pos = 3;
				}
				if(prm_get(COM_PRM, S_SYS, 72, 1, pos)) {						// 対象の駐車場で使用可
					// 設定があれば使用の有無に拘らず設定する
					p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
					p_NtDat->Discount[i].Kind		= 5;
					p_NtDat->Discount[i].Num		= syukei->Ktik_use_cnt[parking];
					p_NtDat->Discount[i].Amount		= syukei->Ktik_use_ryo[parking];
					i++;
				}
			}
		}
	}

	// 店割引は全割引のみ設定する
// GG124100(S) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
	if ( CLOUD_CALC_MODE &&				// クラウド料金計算モード
		 (syukei->Mno_use_Tcnt ||		//  回数データあり
		  syukei->Mno_use_Tryo) ) {		//  金額データあり
		wk_flag = TRUE;
	} else if ( !CLOUD_CALC_MODE ) {	// 通常料金計算モード
// GG124100(E) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
	for(group = 0; group < MISE_NO_CNT; ++group) {								// 店割引設定があるかチェック
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//		if((CPrmSS[S_STO][1 + 3 * group]) || (CPrmSS[S_STO][3 + 3 * group])){	// 店割引設定(役割or種別切換)あり
		if((prm_get( COM_PRM, S_STO, (short)(1 + 3 * group), 1, 1 )) ||
		   (prm_get( COM_PRM, S_STO, (short)(3 + 3 * group), 2, 1 ))){			// 店割引設定(役割or種別切換)あり
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
			break;
		}
	}
// MH322914(S) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
//	if(group != MISE_NO_CNT) {													// 店割引あり
	// ParkingWeb経由のParkiPro対応あり？
	if( _is_ParkingWeb_pip() || (group != MISE_NO_CNT) ){
// MH322914(E) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引集計データ編集処理)
		wk_flag = TRUE;
	}
	if ( prm_get(COM_PRM, S_TAT, 1, 1, 1) == 1 ) {								// 多店舗割引あり
		for ( group = 0; group < T_MISE_SYU_CNT; ++group ) {
			if ( prm_get(COM_PRM, S_TAT, 2 + (group * 2), 1, 1) != 0 ) {
				wk_flag = TRUE;
				break;
			}
		}
		for ( group = 0; group < (T_MISE_NO_CNT - MISE_NO_CNT); ++group ) {
			if ( prm_get(COM_PRM, S_TAT, 32 + group, 2, 1) != 0 ) {
				wk_flag = TRUE;
				break;
			}
		}
	}
// GG124100(S) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
	}
// GG124100(E) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
	if ( wk_flag ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引集計データ編集処理)
		p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];						// 基本駐車場番号ｾｯﾄ
		p_NtDat->Discount[i].Kind		= 63;
		p_NtDat->Discount[i].Num		= syukei->Mno_use_Tcnt;
		p_NtDat->Discount[i].Amount		= syukei->Mno_use_Tryo;
		i++;
	}

// GG124100(S) R.Endo 2022/10/07 車番チケットレス3.0 #6641 NT-NET割引集計データに使用した買い物割引の内容がセットされない [共通改善項目 No1535]
// 買物割引
	for ( group = 0; group < (KAIMONO_WARISYU_MAX + 1); group++ ) {
		if ( (CLOUD_CALC_MODE &&												// クラウド料金計算モード
			  (syukei->Kaimono_use_cnt[group] ||								//  回数データあり
			   syukei->Kaimono_use_ryo[group])) ||								//  金額データあり
			 (!CLOUD_CALC_MODE) ) {												// 通常料金計算モード
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];
			p_NtDat->Discount[i].Kind		= 6;
			p_NtDat->Discount[i].Group		= 0;
			p_NtDat->Discount[i].Num		= syukei->Kaimono_use_cnt[group];
			p_NtDat->Discount[i].Amount		= syukei->Kaimono_use_ryo[group];
			p_NtDat->Discount[i].Info		= group;
			i++;
		}
	}
// GG124100(E) R.Endo 2022/10/07 車番チケットレス3.0 #6641 NT-NET割引集計データに使用した買い物割引の内容がセットされない [共通改善項目 No1535]

// 種別割引
	for (group = 0; group < RYOUKIN_SYU_CNT; group++) {
// GG124100(S) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
// 		if (1 == prm_get(COM_PRM, S_SHA, 1 + 6 * group, 2, 5) && 				// 種別設定あり
// 			0 != prm_get(COM_PRM, S_SHA, 1 + 6 * group, 4, 1)) { 				// 車種割引あり
			if ( (CLOUD_CALC_MODE &&												// クラウド料金計算モード
				  (syukei->Rtwari_cnt[group] ||										//  回数データあり
				   syukei->Rtwari_ryo[group])) ||									//  金額データあり
				 (!CLOUD_CALC_MODE &&												// 通常料金計算モード
				  (prm_get(COM_PRM, S_SHA, (short)(1 + 6 * group), 2, 5) == 1) &&	//  設定あり
				  (prm_get(COM_PRM, S_SHA, (short)(1 + 6 * group), 4, 1) != 0)) ) {
// GG124100(E) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];
			p_NtDat->Discount[i].Kind		= 50;
			p_NtDat->Discount[i].Group		= group + 1;
			p_NtDat->Discount[i].Num		= syukei->Rtwari_cnt[group];
			p_NtDat->Discount[i].Amount		= syukei->Rtwari_ryo[group];
			i++;
		}
	}

	if(prm_get(COM_PRM, S_PAY,24, 1, 3) == 1){
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
		p_NtDat->Discount[i].Kind = NTNET_SUICA_1;								// 割引種別：31（Suica決済）固定
		p_NtDat->Discount[i].Group = 0;											// 割引区分：未使用(0)
		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
// Suica
		if( syukei->Electron_sei_cnt ){
			p_NtDat->Discount[i].Num = syukei->Electron_sei_cnt;				// 割引回数：
			p_NtDat->Discount[i].Amount = syukei->Electron_sei_ryo;				// 割引額  ：
		}

		if( syukei->Electron_psm_cnt ){
// PASMO
			p_NtDat->Discount[i].Num += syukei->Electron_psm_cnt;				// 割引回数：
			p_NtDat->Discount[i].Amount += syukei->Electron_psm_ryo;			// 割引額  ：
		}

		if( syukei->Electron_ico_cnt ){
// ICOCA
			p_NtDat->Discount[i].Num += syukei->Electron_ico_cnt;				// 割引回数：
			p_NtDat->Discount[i].Amount += syukei->Electron_ico_ryo;			// 割引額  ：
		}

		i++;
	}
	
// MH321800(S) hosoda ICクレジット対応, 従来のEdy処理を削除
//-	if( syukei->Electron_edy_cnt ){
//-// Edy
//-		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
//-		p_NtDat->Discount[i].Kind = NTNET_EDY_0;								// 割引種別：32（Edy決済）固定
//-		p_NtDat->Discount[i].Group = 0;											// 割引区分：(0)取引成功分
//-		p_NtDat->Discount[i].Num = syukei->Electron_edy_cnt;					// 割引回数：
//-		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
//-		p_NtDat->Discount[i].Amount = syukei->Electron_edy_ryo;					// 割引額  ：
//-		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
//-		i++;
//-	}
//-
//-	if( syukei->Electron_Arm_cnt ){
//-		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
//-		p_NtDat->Discount[i].Kind = NTNET_EDY_0;								// 割引種別：32（Edy決済）固定
//-		p_NtDat->Discount[i].Group = 1;											// 割引区分：(1)ｱﾗｰﾑ取引分
//-		p_NtDat->Discount[i].Num = syukei->Electron_Arm_cnt;					// 割引回数：
//-		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
//-		p_NtDat->Discount[i].Amount = syukei->Electron_Arm_ryo;					// 割引額  ：
//-		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
//-		i++;
//-	}
// MH321800(E) hosoda ICクレジット対応, 従来のEdy処理を削除

	if( syukei->Electron_icd_cnt ){

// IC-Card
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
		p_NtDat->Discount[i].Kind = NTNET_ICCARD_0;								// 割引種別：36（IC-Card決済）固定
		p_NtDat->Discount[i].Group = 0;											// 割引区分：(0)取引成功分
		p_NtDat->Discount[i].Num = syukei->Electron_icd_cnt;					// 割引回数：
		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_icd_ryo;					// 割引額  ：
		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
		i++;
	}

// MH321800(S) hosoda ICクレジット対応
	// ※集計データにアラーム取引分はセットしない
	for (j = 0; j < TBL_CNT(ec_discount_kind_tbl); j++) {
		switch (ec_discount_kind_tbl[j]) {
		case	NTNET_EDY_0:
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
			brand_no = BRANDNO_EDY;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
			sei_cnt = syukei->Electron_edy_cnt;
			sei_ryo = syukei->Electron_edy_ryo;
			break;
		case	NTNET_NANACO_0:
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
			brand_no = BRANDNO_NANACO;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
			sei_cnt = syukei->nanaco_sei_cnt;
			sei_ryo = syukei->nanaco_sei_ryo;
			break;
		case	NTNET_WAON_0:
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
			brand_no = BRANDNO_WAON;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
			sei_cnt = syukei->waon_sei_cnt;
			sei_ryo = syukei->waon_sei_ryo;
			break;
		case	NTNET_SAPICA_0:
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
			brand_no = BRANDNO_SAPICA;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
			sei_cnt = syukei->sapica_sei_cnt;
			sei_ryo = syukei->sapica_sei_ryo;
			break;
		case	NTNET_SUICA_1:
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
			brand_no = BRANDNO_KOUTSUU;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
			sei_cnt = syukei->koutsuu_sei_cnt;
			sei_ryo = syukei->koutsuu_sei_ryo;
			break;
		case	NTNET_ID_0:
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
			brand_no = BRANDNO_ID;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
			sei_cnt = syukei->id_sei_cnt;
			sei_ryo = syukei->id_sei_ryo;
			break;
		case	NTNET_QUICPAY_0:
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
			brand_no = BRANDNO_QUIC_PAY;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
			sei_cnt = syukei->quicpay_sei_cnt;
			sei_ryo = syukei->quicpay_sei_ryo;
			break;
// MH810105(S) MH364301 PiTaPa対応
		case	NTNET_PITAPA_0:
			brand_no = BRANDNO_PITAPA;
			sei_cnt = syukei->pitapa_sei_cnt;
			sei_ryo = syukei->pitapa_sei_ryo;
			break;
// MH810105(E) MH364301 PiTaPa対応
// MH810105(S) MH364301 QRコード決済対応
		case	NTNET_QR:
			brand_no = BRANDNO_QR;
			sei_cnt = syukei->qr_sei_cnt;
			sei_ryo = syukei->qr_sei_ryo;
			break;
// MH810105(E) MH364301 QRコード決済対応
		default	:
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
			brand_no = 0;
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
			sei_cnt = 0L;
			sei_ryo = 0L;
			break;
		}
// MH810103 GG119202(S) 使用されていない場合でも割引集計データにセットする
//		if (sei_cnt != 0L) {
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//		if( isEcBrandNoEnabledForSetting(brand_no) ){
		if( isEcBrandNoEnabledForRecvTbl(brand_no) ||
			sei_cnt != 0 || sei_ryo != 0) {
		// ブランドテーブルを検索して見つからなくても集計されていれば
		// 割引集計データにセットする
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
// MH810103 GG119202(E) 使用されていない場合でも割引集計データにセットする
		// 決済集計
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
			p_NtDat->Discount[i].Kind = ec_discount_kind_tbl[j];				// 割引種別：Edy/nanaco/WAON/SAPICAなど
			p_NtDat->Discount[i].Group = 0;										// 割引区分：(0)取引成功分
			p_NtDat->Discount[i].Num = sei_cnt;									// 割引回数：
			p_NtDat->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
			p_NtDat->Discount[i].Amount = sei_ryo;								// 割引額  ：
			p_NtDat->Discount[i].Info = 0;										// 割引情報：未使用(0)

			i++;
		}
	}
// MH321800(E) hosoda ICクレジット対応
// MH810105(S) MH364301 QRコード決済対応
	if (isEMoneyReader()) {
		if( syukei->Ec_minashi_cnt ){
			// みなし決済
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
			p_NtDat->Discount[i].Kind = NTNET_DEEMED_PAY;						// 割引種別：1021（みなし決済）固定
			p_NtDat->Discount[i].Group = 0;										// 割引区分：(0)取引成功分
			p_NtDat->Discount[i].Num = syukei->Ec_minashi_cnt;					// 割引回数：
			p_NtDat->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
			p_NtDat->Discount[i].Amount = syukei->Ec_minashi_ryo;				// 割引額  ：
			p_NtDat->Discount[i].Info = 0;										// 割引情報：未使用(0)

			i++;
		}
		if( syukei->miryo_pay_ok_cnt ){
			// 未了支払済み
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
			p_NtDat->Discount[i].Kind = NTNET_MIRYO_PAY;						// 割引種別：1022（未了支払済み）固定
			p_NtDat->Discount[i].Group = 0;										// 割引区分：(0)取引成功分
			p_NtDat->Discount[i].Num = syukei->miryo_pay_ok_cnt;				// 割引回数：
			p_NtDat->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
			p_NtDat->Discount[i].Amount = syukei->miryo_pay_ok_ryo;				// 割引額  ：
			p_NtDat->Discount[i].Info = 0;										// 割引情報：未使用(0)

			i++;
		}
		if( syukei->miryo_unknown_cnt ){
			// 未了支払不明
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
			p_NtDat->Discount[i].Kind = NTNET_MIRYO_UNKNOWN;					// 割引種別：1023（未了支払不明）固定
			p_NtDat->Discount[i].Group = 0;										// 割引区分：(0)取引成功分
			p_NtDat->Discount[i].Num = syukei->miryo_unknown_cnt;				// 割引回数：
			p_NtDat->Discount[i].Callback = 0;									// 回収枚数：未使用(0)
			p_NtDat->Discount[i].Amount = syukei->miryo_unknown_ryo;			// 割引額  ：
			p_NtDat->Discount[i].Info = 0;										// 割引情報：未使用(0)

			i++;
		}
	}
// MH810105(E) MH364301 QRコード決済対応

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//// MH322914(S) K.Onodera 2016/10/12 AI-V対応
////	if(prm_get(COM_PRM, S_PAY,25, 1, 3) == 1){
//	if( _is_Normal_pip() ){
//// MH322914(E) K.Onodera 2016/10/12 AI-V対応
//		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
//		p_NtDat->Discount[i].Kind = NTNET_GENGAKU;								// 割引種別：減額精算
//		p_NtDat->Discount[i].Group = 0;											// 割引区分：未使用(0)
//		p_NtDat->Discount[i].Num = syukei->Gengaku_seisan_cnt;					// 割引回数：
//		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
//		p_NtDat->Discount[i].Amount = syukei->Gengaku_seisan_ryo;				// 割引額  ：
//		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
//		i++;
//	}
//
//// MH322914(S) K.Onodera 2016/10/12 AI-V対応
////	if(prm_get(COM_PRM, S_PAY,25, 1, 3) == 1){
//	if( _is_Normal_pip() ){
//// MH322914(E) K.Onodera 2016/10/12 AI-V対応
//		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場No：自駐車番号
//		p_NtDat->Discount[i].Kind = NTNET_FURIKAE;								// 割引種別：振替精算
//		p_NtDat->Discount[i].Group = 0;											// 割引区分：未使用(0)
//		p_NtDat->Discount[i].Num = syukei->Furikae_seisan_cnt;					// 割引回数：
//		p_NtDat->Discount[i].Callback = 0;										// 回収枚数：未使用(0)
//		p_NtDat->Discount[i].Amount = syukei->Furikae_seisan_ryo;				// 割引額  ：
//		p_NtDat->Discount[i].Info = 0;											// 割引情報：未使用(0)
//		i++;
//	}
//// MH322914(S) K.Onodera 2016/10/11 AI-V対応：振替精算
//	if( _is_ParkingWeb_pip() ){
//		for( mod=0; mod<MOD_TYPE_MAX; mod++ ){
//			switch( mod ){
//				case MOD_TYPE_CREDIT:
//					// クレジット使用設定あり？
//					if( !prm_get( COM_PRM, S_PAY, 24, 1, 2 ) ){
//						continue;
//					}
//					break;
//				case MOD_TYPE_EMONEY:
//					if( !prm_get( COM_PRM, S_PAY, 24, 1, 3 )  ){
//						continue;
//					}
//					break;
//				default:
//					continue;
//					break;
//			}
//			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// 駐車場No：自駐車番号
//			p_NtDat->Discount[i].Kind = NTNET_KABARAI;							// 割引種別：振替精算(ParkingWeb版)
//			p_NtDat->Discount[i].Group = mod;									// 割引区分：媒体種別
//// 不具合修正(S) K.Onodera 2016/11/25 #1588 割引集計データに振替過払い金の情報がセットされない
////			if( syukei->Furikae_CardKabarai[i] ){
//			if( syukei->Furikae_CardKabarai[mod] ){
//// 不具合修正(E) K.Onodera 2016/11/25 #1588 割引集計データに振替過払い金の情報がセットされない
//				p_NtDat->Discount[i].Num = syukei->Furikae_Card_cnt[mod];		// 割引回数：未使用(0)
//				p_NtDat->Discount[i].Callback = 0;								// 回収枚数：未使用(0)
//// 不具合修正(S) K.Onodera 2016/11/25 #1588 割引集計データに振替過払い金の情報がセットされない
////				p_NtDat->Discount[i].Amount = syukei->Furikae_CardKabarai[i];	// 割引額  ：過払い金額
//				p_NtDat->Discount[i].Amount = syukei->Furikae_CardKabarai[mod];	// 割引額  ：過払い金額
//// 不具合修正(E) K.Onodera 2016/11/25 #1588 割引集計データに振替過払い金の情報がセットされない
//				p_NtDat->Discount[i].Info = 0;									// 割引情報：未使用(0)
//			}
//			i++;
//		}
//	}
//// MH322914(E) K.Onodera 2016/10/11 AI-V対応：振替精算
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	ret = sizeof( DATA_KIND_45 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			定期集計データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 定期集計データ(DATA_KIND_46型)へのポインタ  
///	@return			ret		: 定期集計データのデータサイズ(システムID～) 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//unsigned short	NTNET_Edit_SyukeiTeiki_r10( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat )
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//unsigned short	NTNET_Edit_SyukeiTeiki_r13( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiTeiki_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
{
	int		i;
	int		parking;
	int		kind;
	ushort	pos;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_46 ) );
	NTNET_Edit_BasicData( 46, 0, syukei->SeqNo[3], &p_NtDat->DataBasic);		// Ｔ合計：データ保持フラグ = 0

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//	p_NtDat->FmtRev					= 10;										// フォーマットRev.No.
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	p_NtDat->FmtRev					= 13;										// フォーマットRev.No.
	p_NtDat->FmtRev					= 15;										// フォーマットRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// 集計タイプ 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// 集計追番
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// 個別追番
		if (Type == 11) {														// GT集計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// 終了追番
		}
		break;
	case	1:		// 通し追い番
		if (Type == 1 || Type == 11) {
		// T合計／小計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// 終了追番
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// センター追番（集計）
	i = 0;
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if(CPrmSS[S_SYS][1+parking]) {											// 駐車場設定あり
			if(parking == 0) {													// 基本駐車場
				pos = 6;
			}
			else if(parking == 1) {												// 拡張1駐車場
				pos = 1;
			}
			else if(parking == 2) {												// 拡張3駐車場
				pos = 2;
			}
			else if(parking == 3) {												// 拡張3駐車場
				pos = 3;
			}
			if(prm_get(COM_PRM, S_SYS, 70, 1, pos)) {							// 対象の駐車場で使用可
				for (kind = 0; kind < TEIKI_SYU_CNT; kind++) {
// GG124100(S) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
// 					if(0 != prm_get(COM_PRM, S_PAS, 1 + (10 * kind), 2, 1)) {		// 定期使用可？
					if ( (CLOUD_CALC_MODE &&											// クラウド料金計算モード
						  syukei->Teiki_use_cnt[parking][kind]) ||						//  回数データあり
						 (!CLOUD_CALC_MODE &&											// 通常料金計算モード
						  prm_get(COM_PRM, S_PAS, (short)(1 + (10 * kind)), 2, 1)) ) {	//  設定あり
// GG124100(E) R.Endo 2022/10/05 車番チケットレス3.0 #6636 NT-NET割引集計データに使用した割引の内容が反映されない
						p_NtDat->Pass[i].ParkingNo		= CPrmSS[S_SYS][1+parking];	// 駐車場No.
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//						p_NtDat->Pass[i].Kind			= kind + 1;					// 種別
						p_NtDat->Pass[i].Kind			= (uchar)(kind + 1);		// 種別
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
						p_NtDat->Pass[i].Num			= syukei->Teiki_use_cnt[parking][kind];	// 回数
						p_NtDat->Pass[i].Callback		= 0;						// 回収枚数（未使用）
						i++;
					}
				}
			}
		}
	}

	ret = sizeof( DATA_KIND_46 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			集計終了通知データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 集計終了通知データ(DATA_KIND_53型)へのポインタ  
///	@return			ret		: 集計終了通知データのデータサイズ(システムID～) 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//unsigned short	NTNET_Edit_SyukeiSyuryo_r10( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat )
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//unsigned short	NTNET_Edit_SyukeiSyuryo_r13( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiSyuryo_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_53 ) );
	NTNET_Edit_BasicData( 53, 0, syukei->SeqNo[4], &p_NtDat->DataBasic);		// 基本ﾃﾞｰﾀ作成

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//	p_NtDat->FmtRev					= 10;										// フォーマットRev.No.
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	p_NtDat->FmtRev					= 13;										// フォーマットRev.No.
	p_NtDat->FmtRev					= 15;										// フォーマットRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// 集計タイプ 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// 集計追番
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// 個別追番
		if (Type == 11) {														// GT集計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// 終了追番
		}
		break;
	case	1:		// 通し追い番
		if (Type == 1 || Type == 11) {
		// T合計／小計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// 終了追番
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// センター追番（集計）

	ret = sizeof( DATA_KIND_53 ); 
	return ret;
}

//	WEB用駐車台数用NT-NETフォーマット設定(236ﾌｫｰﾏｯﾄとは別に58_r10ﾌｫｰﾏｯﾄを作成)	// 参照 = NTNET_Edit_Data236
unsigned short	NTNET_Edit_Data58_r10(	ParkCar_log			*p_RcptDat,		// 駐車台数用logﾃﾞｰﾀ(IN)
										DATA_KIND_58_r10 	*p_NtDat_r10 )	// 駐車台数用NT-NET(OUT)
{
	unsigned short ret;

	memset( p_NtDat_r10, 0, sizeof( DATA_KIND_58_r10 ) );
	NTNET_Edit_BasicData( 58, 0, p_RcptDat->CMN_DT.DT_58.SeqNo, &p_NtDat_r10->DataBasic);	// 基本ﾃﾞｰﾀ作成

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat_r10->DataBasic.Year = (uchar)(p_RcptDat->Time.Year % 100);	// 処理年(ログ作成時に下2桁にしている)
	p_NtDat_r10->DataBasic.Mon = (uchar)p_RcptDat->Time.Mon;			// 処理月
	p_NtDat_r10->DataBasic.Day = (uchar)p_RcptDat->Time.Day;			// 処理日
	p_NtDat_r10->DataBasic.Hour = (uchar)p_RcptDat->Time.Hour;			// 処理時
	p_NtDat_r10->DataBasic.Min = (uchar)p_RcptDat->Time.Min;			// 処理分
	p_NtDat_r10->DataBasic.Sec = (uchar)p_RcptDat->Time.Sec;			// 処理秒

	// 236 -> 58_r10(別ﾌｫｰﾏｯﾄ)にﾃﾞｰﾀの設定(FmtRev, PARKCAR_DATA2)
	p_NtDat_r10->FmtRev = 10;											// フォーマットRev.№
	p_NtDat_r10->ParkData.FullSts1	= p_RcptDat->Full[0];				// 駐車1満空状態
	p_NtDat_r10->ParkData.EmptyNo1	= p_RcptDat->CMN_DT.DT_58.EmptyNo1;	// 空車台数１
	p_NtDat_r10->ParkData.FullNo1	= p_RcptDat->CMN_DT.DT_58.FullNo1;	// 満車台数１
	p_NtDat_r10->ParkData.FullSts2	= p_RcptDat->Full[1];				// 駐車2満空状態
	p_NtDat_r10->ParkData.EmptyNo2	= p_RcptDat->CMN_DT.DT_58.EmptyNo2;	// 空車台数２
	p_NtDat_r10->ParkData.FullNo2	= p_RcptDat->CMN_DT.DT_58.FullNo2;	// 満車台数２
	p_NtDat_r10->ParkData.FullSts3	= p_RcptDat->Full[2];				// 駐車3満空状態
	p_NtDat_r10->ParkData.EmptyNo3	= p_RcptDat->CMN_DT.DT_58.EmptyNo3;	// 空車台数３
	p_NtDat_r10->ParkData.FullNo3	= p_RcptDat->CMN_DT.DT_58.FullNo3;	// 満車台数３

	ret = sizeof( DATA_KIND_58_r10 ); 
	return ret;
}
/*[]----------------------------------------------------------------------[]*/
/*| センター用端末情報ﾃﾞｰﾀ(ﾃﾞｰﾀ種別65)作成処理                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data65                                        |*/
/*| PARAMETER    : MachineNo : 送信先端末機械№                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTNET_Snd_Data65( ulong MachineNo )
{
	ushort	len;
	char*	pStr;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_65 ) );

	BasicDataMake( 65, 0 );										// 基本ﾃﾞｰﾀ作成

	SendNtnetDt.SData65.DataBasic.SeqNo = GetNtDataSeqNo();		// シーケンシャルNo.
// GG120600(S) // Phase9 バージョン変更
//// GG120600(S) // MH810100(S) K.Onodera 2019/12/16 車番チケットレス(ParkingWebフェーズ9対応)
////	SendNtnetDt.SData65.FmtRev = 11;							// フォーマットRev.No.(11固定)
//	SendNtnetDt.SData65.FmtRev = 12;							// フォーマットRev.No.(12固定)
//// GG120600(E) // MH810100(S) K.Onodera 2019/12/16 車番チケットレス(ParkingWebフェーズ9対応)
	SendNtnetDt.SData65.FmtRev = 13;							// フォーマットRev.No.(13固定)
// GG120600(E) // Phase9 バージョン変更
	SendNtnetDt.SData65.SMachineNo = MachineNo;					// 送信先端末機械№

	SendNtnetDt.SData65.Result				= 0;				// 要求結果
	SendNtnetDt.SData65.AcceptReq.bits.b00	= ReqAcceptTbl[VER_UP_REQ];			// バージョンアップ要求受付
	SendNtnetDt.SData65.AcceptReq.bits.b01	= ReqAcceptTbl[PARAM_CHG_REQ];		// 設定変更要求受付
	SendNtnetDt.SData65.AcceptReq.bits.b02	= ReqAcceptTbl[PARAM_UPLOAD_REQ];	// 設定要求受付
	SendNtnetDt.SData65.AcceptReq.bits.b03	= ReqAcceptTbl[RESET_REQ];			// リセット要求受付
	SendNtnetDt.SData65.AcceptReq.bits.b04	= ReqAcceptTbl[PROG_ONLY_CHG_REQ];	// プログラム切換要求受付
	SendNtnetDt.SData65.AcceptReq.bits.b05	= ReqAcceptTbl[FTP_CHG_REQ];		// FTP設定変更要求受付
	SendNtnetDt.SData65.AcceptReq.bits.b06	= ReqAcceptTbl[REMOTE_FEE_REQ];		// 遠隔料金設定受付
// MH810100(S) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
	SendNtnetDt.SData65.AcceptReq.bits.b07	= ReqAcceptTbl[PARAM_DIFF_CHG_REQ];	// 設定変更要求受付
// MH810100(E) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)

	// RXMソフトバージョン（右詰め）
	memcpy( SendNtnetDt.SData65.ProgramVer, "RXM ", 4 );
	if(strlen((char*)VERSNO.ver_part)) {
		memcpy( &SendNtnetDt.SData65.ProgramVer[4], VERSNO.ver_part, 8 );
	}
	else {
		memcpy( &SendNtnetDt.SData65.ProgramVer[4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
	}
	
// GG120600(S) // Phase9 Versionを分ける
//// GG120600(S) // MH810100(S) K.Onodera 2019/12/16 車番チケットレス(ParkingWebフェーズ9対応)
//	// ベースプログラムバージョン(＝プログラムバージョン)
//	memcpy( SendNtnetDt.SData65.BaseProgramVer, &SendNtnetDt.SData65.ProgramVer, 12 );
//	
//	// 料金計算dllバージョン(＝プログラムバージョン)
//	memcpy( SendNtnetDt.SData65.RyoCalDllVer, &SendNtnetDt.SData65.ProgramVer, 12 );
//// GG120600(E) // MH810100(E) K.Onodera 2019/12/16 車番チケットレス(ParkingWebフェーズ9対応)

	// ベースプログラムバージョン(＝プログラムバージョン)
	memcpy( SendNtnetDt.SData65.BaseProgramVer, "RXM ", 4 );
	if(strlen((char*)VERSNO_BASE.ver_part)) {
		memcpy( &SendNtnetDt.SData65.BaseProgramVer[4], VERSNO_BASE.ver_part, 8 );
	}
	else {
		memcpy( &SendNtnetDt.SData65.BaseProgramVer[4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
	}

// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6554 センタ用端末情報データ 変更対応
	if ( !CLOUD_CALC_MODE ) {	// 通常料金計算モード
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6554 センタ用端末情報データ 変更対応
	
	// 料金計算dllバージョン(＝プログラムバージョン)
	memcpy( SendNtnetDt.SData65.RyoCalDllVer, "RXM ", 4 );
	if(strlen((char*)VERSNO_RYOCAL.ver_part)) {
		memcpy( &SendNtnetDt.SData65.RyoCalDllVer[4], VERSNO_RYOCAL.ver_part, 8 );
	}
	else {
		memcpy( &SendNtnetDt.SData65.RyoCalDllVer[4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
	}

// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6554 センタ用端末情報データ 変更対応
	}
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6554 センタ用端末情報データ 変更対応

// GG120600(E) // Phase9 Versionを分ける
	
	// RXFソフトバージョン（右詰め）
	memcpy( &SendNtnetDt.SData65.ModuleVer[0][0], "RXF ", 4 );
	pStr = strchr((char*)RXF_VERSION, ' ');
	if(pStr) {
		len = (ushort)((uchar*)pStr - (uchar*)RXF_VERSION);
	}
	else {
		len = strlen((char*)RXF_VERSION);
	}
	if(len) {
		if(len > 9) {
			len = 9;
		}
		memcpy( &SendNtnetDt.SData65.ModuleVer[0][12 - len], RXF_VERSION, len);
	}
	else {
		memcpy( &SendNtnetDt.SData65.ModuleVer[0][4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
	}
// MH810100(S) K.Onodera 2019/12/16 車番チケットレス(ParkingWebフェーズ9対応)
//	// 音声データバージョン（右詰め）
//	memcpy( &SendNtnetDt.SData65.ModuleVer[1][0], "RXM ", 4 );
//	pStr = strchr((char*)SOUND_VERSION, ' ');
//	if(pStr) {
//		len = (ushort)((uchar*)pStr - (uchar*)SOUND_VERSION);
//	}
//	else {
//		len = strlen((char*)SOUND_VERSION);
//	}
//	if(len) {
//		if(len > 9) {
//			len = 9;
//		}
//		memcpy( &SendNtnetDt.SData65.ModuleVer[1][12 - len], SOUND_VERSION, len);
//	}
//	else {
//		memcpy( &SendNtnetDt.SData65.ModuleVer[1][4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
//	}
//	
//	// 磁気リーダーバージョン（右詰め）
//	if( prm_get( COM_PRM,S_PAY,21,1,3 ) ){						// 磁気ﾘｰﾀﾞｰ使用
//		memcpy( &SendNtnetDt.SData65.ModuleVer[2][0], "FBL ", 4 );
//		pStr = strchr((char*)MRD_VERSION, ' ');
//		if(pStr) {
//			len = (ushort)((uchar*)pStr - (uchar*)MRD_VERSION);
//		}
//		else {
//			len = strlen((char*)MRD_VERSION);
//		}
//		if(len) {
//			if(len > 9) {
//				len = 9;
//			}
//			memcpy( &SendNtnetDt.SData65.ModuleVer[2][12 - len], MRD_VERSION, len);
//		}
//		else {
//			memcpy( &SendNtnetDt.SData65.ModuleVer[2][4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
//		}
//	}
// MH810100(E) K.Onodera 2019/12/16 車番チケットレス(ParkingWebフェーズ9対応)
// GG120600(S) // Phase9 センター用端末情報データに受信電文リビジョンを追加
	// 受信電文リビジョン
	SendNtnetDt.SData65.DataRevs[0].DataProto	= 1;				// 1=遠隔NT-NET
	SendNtnetDt.SData65.DataRevs[0].DataSysID	= REMOTE_DL_SYSTEM;	// システムID
	SendNtnetDt.SData65.DataRevs[0].DataID		= REMOTE_MNT_REQ;	// 遠隔メンテナンス要求
	SendNtnetDt.SData65.DataRevs[0].DataRev		= 11;
// GG120600(E) // Phase9 センター用端末情報データに受信電文リビジョンを追加

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_65 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_65 ), NTNET_BUF_PRIOR );	// ﾃﾞｰﾀ送信登録
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 精算情報データ用保存格納処理
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152Save(void *saveData, uchar saveDataCategory)
{

	NTNET_152_U_SAVEINF *uData152;
	
	// 遠隔NT-NETの接続設定が有効でかつ料金テスト実行中でなければ精算情報データを更新
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		uData152 = (NTNET_152_U_SAVEINF *)saveData;
		switch(saveDataCategory){
			case NTNET_152_OIBAN:
				memcpy(&ntNet_152_SaveData.Oiban, (uchar *)&uData152->Oiban, sizeof(ntNet_152_SaveData.Oiban));
				break;
			case NTNET_152_PAYMETHOD:
				ntNet_152_SaveData.PayMethod = uData152->PayMethod;
				break;
			case NTNET_152_PAYMODE:
				ntNet_152_SaveData.PayMode = uData152->PayMode;
				break;
// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
			case NTNET_152_CMACHINENO:
				ntNet_152_SaveData.CMachineNo = uData152->CMachineNo;
				break;
// MH810100(E) K.Onodera  2020/02/05 車番チケットレス(LCD_IF対応)
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//			case NTNET_152_WPLACE:
//				ntNet_152_SaveData.WPlace = uData152->WPlace;
//				break;
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			case NTNET_152_KAKARINO:
				ntNet_152_SaveData.KakariNo = uData152->KakariNo;
				break;
			case NTNET_152_OUTKIND:
				ntNet_152_SaveData.OutKind = uData152->OutKind;
				break;
			case NTNET_152_COUNTSET:
				ntNet_152_SaveData.CountSet = uData152->CountSet;
				break;
			case NTNET_152_CARINTIME:
				memcpy(&ntNet_152_SaveData.carInTime, (uchar *)&uData152->carInTime, sizeof(ntNet_152_SaveData.carInTime));
				break;
			case NTNET_152_RECEIPTISSUE:
				ntNet_152_SaveData.ReceiptIssue = uData152->ReceiptIssue;
				break;
			case NTNET_152_SYUBET:
				ntNet_152_SaveData.Syubet = uData152->Syubet;
				break;
			case NTNET_152_PRICE:
				ntNet_152_SaveData.Price = uData152->Price;
				break;
			case NTNET_152_CASHPRICE:
				ntNet_152_SaveData.CashPrice = uData152->CashPrice;
				break;
			case NTNET_152_INPRICE:
				ntNet_152_SaveData.InPrice = uData152->InPrice;
				break;
			case NTNET_152_CHGPRICE:
				ntNet_152_SaveData.ChgPrice = uData152->ChgPrice;
				break;
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// 不具合修正(S) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
//			case NTNET_152_KABARAI:
//				ntNet_152_SaveData.FrkReturn = uData152->FrkReturn;
//				break;
//// 不具合修正(E) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			case NTNET_152_INCOIN:
				memcpy(&ntNet_152_SaveData.in_coin[0], (uchar *)uData152->in_coin, sizeof(ntNet_152_SaveData.in_coin));
				break;
			case NTNET_152_OUTCOIN:
				memcpy(&ntNet_152_SaveData.out_coin[0], (uchar *)uData152->out_coin, sizeof(ntNet_152_SaveData.out_coin));
				break;
			case NTNET_152_FESCROW:
				ntNet_152_SaveData.f_escrow = uData152->f_escrow;
				break;
			case NTNET_152_HARAIMODOSHIFUSOKU:
				ntNet_152_SaveData.HaraiModoshiFusoku = uData152->HaraiModoshiFusoku;
				break;
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
			case NTNET_152_CARD_FUSOKU_TYPE:
				ntNet_152_SaveData.CardFusokuType = uData152->CardFusokuType;
				break;
			case NTNET_152_CARD_FUSOKU:
				ntNet_152_SaveData.CardFusokuTotal = uData152->CardFusokuTotal;
				break;
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
			case NTNET_152_ANTIPASSCHECK:
				ntNet_152_SaveData.AntiPassCheck = uData152->AntiPassCheck;
				break;
			case NTNET_152_PARKNOINPASS:
				ntNet_152_SaveData.ParkNoInPass = uData152->ParkNoInPass;
				break;
			case NTNET_152_PKNOSYU:
				ntNet_152_SaveData.pkno_syu = uData152->pkno_syu;
				break;
			case NTNET_152_TEIKIID:
				ntNet_152_SaveData.teiki_id = uData152->teiki_id;
				break;
			case NTNET_152_TEIKISYU:
				ntNet_152_SaveData.teiki_syu = uData152->teiki_syu;
				break;
// MH810100(S) K.Onodera 2019/12/16 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
			case NTNET_152_MEDIAKIND1:
				ntNet_152_SaveData.MediaKind1 = uData152->MediaKind1;
				break;
			case NTNET_152_MEDIACARDNO1:
				memcpy( ntNet_152_SaveData.MediaCardNo1, (uchar *)uData152->MediaCardNo1, sizeof(ntNet_152_SaveData.MediaCardNo1));
				break;
			case NTNET_152_MEDIACARDINFO1:
				memcpy( ntNet_152_SaveData.MediaCardInfo1, (uchar *)uData152->MediaCardInfo1, sizeof(ntNet_152_SaveData.MediaCardInfo1));
				break;
			case NTNET_152_MEDIAKIND2:
				ntNet_152_SaveData.MediaKind2 = uData152->MediaKind2;
				break;
			case NTNET_152_MEDIACARDNO2:
				memcpy( ntNet_152_SaveData.MediaCardNo2, (uchar *)uData152->MediaCardNo2, sizeof(ntNet_152_SaveData.MediaCardNo2));
				break;
			case NTNET_152_MEDIACARDINFO2:
				memcpy( ntNet_152_SaveData.MediaCardInfo2, (uchar *)uData152->MediaCardInfo2, sizeof(ntNet_152_SaveData.MediaCardInfo2));
				break;
// MH810100(E) K.Onodera 2019/12/16 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
			case NTNET_152_ECARDKIND:
				ntNet_152_SaveData.e_pay_kind = uData152->e_pay_kind;
				break;
			case NTNET_152_EPAYRYO:
				ntNet_152_SaveData.e_pay_ryo = uData152->e_pay_ryo;
				break;
			case NTNET_152_ECARDID:
// MH321800(S) hosoda ICクレジット対応
//-				memcpy(&ntNet_152_SaveData.e_Card_ID[0], (uchar *)uData152->e_Card_ID, sizeof(ntNet_152_SaveData.e_Card_ID));
				if (EcUseKindCheck(ntNet_152_SaveData.e_pay_kind)) {	// 必ずNTNET_152_ECARDKINDが先に呼ばれているのでカード種別の判定に使う
				// 決済リーダでのカード電文長
					memcpy(&ntNet_152_SaveData.e_Card_ID[0], (uchar *)uData152->e_Card_ID, sizeof(ntNet_152_SaveData.e_Card_ID));
				}
				else {
				// SX10でのカード電文長
					memset(&ntNet_152_SaveData.e_Card_ID[0], 0x20, sizeof(ntNet_152_SaveData.e_Card_ID));
					memcpy(&ntNet_152_SaveData.e_Card_ID[0], (uchar *)uData152->e_Card_ID, sizeof(PayData.Electron_data.Suica.Card_ID));
				}
// MH321800(E) hosoda ICクレジット対応
				break;
			case NTNET_152_EPAYAFTER:
				ntNet_152_SaveData.e_pay_after = uData152->e_pay_after;
				break;
// MH321800(S) D.Inaba ICクレジット対応
			case NTNET_152_ECINQUIRYNUM:
// MH810103 GG119202(S) 電子マネー対応
//				memcpy(&ntNet_152_SaveData.e_inquiry_num[0], (uchar *)uData152->e_inquiry_num, sizeof(ntNet_152_SaveData.e_inquiry_num));
				// 15桁の問い合わせ番号を左詰めで16桁にセット
				memcpyFlushLeft( (uchar *)&ntNet_152_SaveData.e_inquiry_num[0], (uchar *)&uData152->e_inquiry_num[0],
								sizeof(ntNet_152_SaveData.e_inquiry_num), sizeof(PayData.Electron_data.Ec.inquiry_num) );	// 問い合わせ番号
// MH810103 GG119202(E) 電子マネー対応
				break;
// MH321800(E) D.Inaba ICクレジット対応
			case NTNET_152_CPAYRYO:
				ntNet_152_SaveData.c_pay_ryo = uData152->c_pay_ryo;
				break;
			case NTNET_152_CCARDNO:
				memcpy(&ntNet_152_SaveData.c_Card_No[0], (uchar *)uData152->c_Card_No, sizeof(ntNet_152_SaveData.c_Card_No));
				break;
			case NTNET_152_CCCTNUM:
// MH321800(S) D.Inaba ICクレジット対応
//				memcpy(&ntNet_152_SaveData.c_cct_num[0], (uchar *)uData152->c_cct_num, sizeof(ntNet_152_SaveData.c_cct_num));
				memset(&ntNet_152_SaveData.c_cct_num[0], 0x20, sizeof(ntNet_152_SaveData.c_cct_num));
				memcpy(&ntNet_152_SaveData.c_cct_num[0], (uchar *)uData152->c_cct_num, sizeof(ryo_buf.credit.CCT_Num));
// MH321800(E) D.Inaba ICクレジット対応
				break;
			case NTNET_152_CKID:
				memcpy(&ntNet_152_SaveData.c_kid_code[0], (uchar *)uData152->c_kid_code, sizeof(ntNet_152_SaveData.c_kid_code));
				break;
			case NTNET_152_CAPPNO:
				ntNet_152_SaveData.c_app_no = uData152->c_app_no;
				break;
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
			case NTNET_152_CTRADENO:
				memcpy(&ntNet_152_SaveData.c_trade_no[0], (uchar *)uData152->c_trade_no, sizeof(ntNet_152_SaveData.c_trade_no));
				break;
			case NTNET_152_SLIPNO:
				ntNet_152_SaveData.c_slipNo = uData152->c_slipNo;
				break;
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
// MH810105(S) MH364301 QRコード決済対応
			case NTNET_152_TRANS_STS:
				ntNet_152_SaveData.e_Transactiontatus = uData152->e_Transactiontatus;
				break;
			case NTNET_152_QR_MCH_TRADE_NO:
				memcpy(ntNet_152_SaveData.qr_MchTradeNo, uData152->qr_MchTradeNo, sizeof(ntNet_152_SaveData.qr_MchTradeNo));
				break;
			case NTNET_152_QR_PAY_TERM_ID:
				memcpy(ntNet_152_SaveData.qr_PayTermID, uData152->qr_PayTermID, sizeof(ntNet_152_SaveData.qr_PayTermID));
				break;
			case NTNET_152_QR_PAYKIND:
				ntNet_152_SaveData.qr_PayKind = uData152->qr_PayKind;
				break;
// MH810105(E) MH364301 QRコード決済対応
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 精算情報データ用 割引詳細データ保存処理
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_DiscDataSave(void *saveData, uchar saveDataCategory, uchar saveIndex)
{

	NTNET_152_U_SAVEINF *uData152;
	
	// 遠隔NT-NETの接続設定が有効でかつ料金テスト実行中でなければ精算情報データを更新
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		uData152 = (NTNET_152_U_SAVEINF *)saveData;
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// // MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
// 		if( uData152->DiscountData.uDiscData.common.DiscFlg ){	// 割引済み
// 			return;
// 		}
// // MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
		switch(saveDataCategory){
			case NTNET_152_DPARKINGNO:
				ntNet_152_SaveData.DiscountData[saveIndex].ParkingNo = uData152->DiscountData.ParkingNo;
				break;
			case NTNET_152_DSYU:
				ntNet_152_SaveData.DiscountData[saveIndex].DiscSyu = uData152->DiscountData.DiscSyu;
				break;
			case NTNET_152_DNO:
				ntNet_152_SaveData.DiscountData[saveIndex].DiscNo = uData152->DiscountData.DiscNo;
				break;
			case NTNET_152_DCOUNT:
				ntNet_152_SaveData.DiscountData[saveIndex].DiscCount = uData152->DiscountData.DiscCount;
				break;
			case NTNET_152_DISCOUNT:
				ntNet_152_SaveData.DiscountData[saveIndex].Discount = uData152->DiscountData.Discount;
				break;
			case NTNET_152_DINFO1:
				ntNet_152_SaveData.DiscountData[saveIndex].DiscInfo1 = uData152->DiscountData.DiscInfo1;
				break;
			case NTNET_152_DINFO2:
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.DiscInfo2 = uData152->DiscountData.uDiscData.common.DiscInfo2;
				break;
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			case NTNET_152_PREVDISCOUNT:		// 割引 今回使用した前回精算までの割引金額
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.PrevDiscount =
					uData152->DiscountData.uDiscData.common.PrevDiscount;
				break;
			case NTNET_152_PREVUSAGEDISCOUNT:	// 割引 前回精算までの使用済み割引金額
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.PrevUsageDiscount =
					uData152->DiscountData.uDiscData.common.PrevUsageDiscount;
				break;
			case NTNET_152_PREVUSAGEDCOUNT:		// 割引 前回精算までの使用済み枚数
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.PrevUsageDiscCount =
					uData152->DiscountData.uDiscData.common.PrevUsageDiscCount;
				break;
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// GG124100(S) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
			case NTNET_152_FEEKINDSWITCHSETTING:	// 割引 種別切換先車種
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.FeeKindSwitchSetting =
					uData152->DiscountData.uDiscData.common.FeeKindSwitchSetting;
				break;
			case NTNET_152_DROLE:					// 割引 割引役割
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.DiscRole =
					uData152->DiscountData.uDiscData.common.DiscRole;
				break;
// GG124100(E) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 精算情報データ用割引詳細エリアクリア
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_DiscDataClear(void)
{

	// 遠隔NT-NETの接続設定が有効でかつ料金テスト実行中でなければ精算情報データを更新
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		memset((uchar *)&ntNet_152_SaveData.DiscountData[0], 0, sizeof(ntNet_152_SaveData.DiscountData));
	}
}

// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
/*[]----------------------------------------------------------------------[]*
 *| 精算情報データ用 明細データ保存処理
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_DetailDataSave( void *saveData, uchar saveDataCategory, uchar saveIndex )
{

	NTNET_152_U_SAVEINF *uData152;
	
	// 遠隔NT-NETの接続設定が有効でかつ料金テスト実行中でなければ精算情報データを更新
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		uData152 = (NTNET_152_U_SAVEINF *)saveData;
		switch(saveDataCategory){
			case NTNET_152_DPARKINGNO:
				ntNet_152_SaveData.DetailData[saveIndex].ParkingNo = uData152->DetailData.ParkingNo;
				break;
			case NTNET_152_DSYU:
				ntNet_152_SaveData.DetailData[saveIndex].DiscSyu = uData152->DetailData.DiscSyu;
				break;
			case NTNET_152_DNO:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.DiscNo = uData152->DetailData.uDetail.Common.DiscNo;
				break;
			case NTNET_152_DCOUNT:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.DiscCount = uData152->DetailData.uDetail.Common.DiscCount;
				break;
			case NTNET_152_DISCOUNT:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.Discount = uData152->DetailData.uDetail.Common.Discount;
				break;
			case NTNET_152_DINFO1:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.DiscInfo1 = uData152->DetailData.uDetail.Common.DiscInfo1;
				break;
			case NTNET_152_DINFO2:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.DiscInfo2 = uData152->DetailData.uDetail.Common.DiscInfo2;
				break;
			default:
				break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*
 *| 精算情報データ用明細エリアクリア
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_DetailDataClear(void)
{

	// 遠隔NT-NETの接続設定が有効でかつ料金テスト実行中でなければ精算情報データを更新
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		memset((uchar *)&ntNet_152_SaveData.DetailData[0], 0, sizeof(ntNet_152_SaveData.DetailData));
	}
}
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応

/*[]----------------------------------------------------------------------[]*
 *| 精算情報データ用保存領域全クリア
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_SaveDataClear(void)
{

	
	// 遠隔NT-NETの接続設定が有効でかつ料金テスト実行中でなければ精算情報データを更新
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		memset(&ntNet_152_SaveData.Oiban, 0, sizeof(ntNet_152_SaveData.Oiban));
		ntNet_152_SaveData.PayMethod = 0;
		ntNet_152_SaveData.PayMode = 0;
// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
		ntNet_152_SaveData.CMachineNo = 0;
// MH810100(E) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		ntNet_152_SaveData.WPlace = 0;
// MH810100(E) K.Onodera  2020/02/05  車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		ntNet_152_SaveData.KakariNo = 0;
		ntNet_152_SaveData.OutKind = 0;
		ntNet_152_SaveData.CountSet = 0;
		ntNet_152_SaveData.ReceiptIssue = 0;
		ntNet_152_SaveData.Syubet = 0;
		ntNet_152_SaveData.Price = 0;
		ntNet_152_SaveData.CashPrice = 0;
		ntNet_152_SaveData.InPrice = 0;
		ntNet_152_SaveData.ChgPrice = 0;
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
		ntNet_152_SaveData.FrkReturn = 0;
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
		memset(&ntNet_152_SaveData.in_coin[0], 0, sizeof(ntNet_152_SaveData.in_coin));
		memset(&ntNet_152_SaveData.out_coin[0], 0, sizeof(ntNet_152_SaveData.out_coin));
		ntNet_152_SaveData.f_escrow = 0;
		ntNet_152_SaveData.HaraiModoshiFusoku = 0;
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
		ntNet_152_SaveData.CardFusokuType = 0;
		ntNet_152_SaveData.CardFusokuTotal = 0;
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
		ntNet_152_SaveData.AntiPassCheck = 0;
		ntNet_152_SaveData.ParkNoInPass = 0;
		ntNet_152_SaveData.pkno_syu = 0;
		ntNet_152_SaveData.teiki_id = 0;
		ntNet_152_SaveData.teiki_syu = 0;
// MH810100(S) K.Onodera  2020/02/17  車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
		ntNet_152_SaveData.MediaKind1 = 0;
		memset(&ntNet_152_SaveData.MediaCardNo1[0], 0, sizeof(ntNet_152_SaveData.MediaCardNo1));
		memset(&ntNet_152_SaveData.MediaCardInfo1[0], 0, sizeof(ntNet_152_SaveData.MediaCardInfo1));
		ntNet_152_SaveData.MediaKind2 = 0;
		memset(&ntNet_152_SaveData.MediaCardNo2[0], 0, sizeof(ntNet_152_SaveData.MediaCardNo2));
		memset(&ntNet_152_SaveData.MediaCardInfo2[0], 0, sizeof(ntNet_152_SaveData.MediaCardInfo2));
// MH810100(E) K.Onodera  2020/02/17  車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
		ntNet_152_SaveData.e_pay_kind = 0;
		ntNet_152_SaveData.e_pay_ryo = 0;
		memset(&ntNet_152_SaveData.e_Card_ID[0], 0, sizeof(ntNet_152_SaveData.e_Card_ID));
		ntNet_152_SaveData.e_pay_after = 0;
		ntNet_152_SaveData.c_pay_ryo = 0;
		memset(&ntNet_152_SaveData.c_Card_No[0], 0, sizeof(ntNet_152_SaveData.c_Card_No));
		memset(&ntNet_152_SaveData.c_cct_num[0], 0, sizeof(ntNet_152_SaveData.c_cct_num));
		memset(&ntNet_152_SaveData.c_kid_code[0], 0, sizeof(ntNet_152_SaveData.c_kid_code));
		ntNet_152_SaveData.c_app_no = 0;
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
		memset(&ntNet_152_SaveData.c_trade_no[0], 0, sizeof(ntNet_152_SaveData.c_trade_no));
		ntNet_152_SaveData.c_slipNo = 0;
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	}
}

/*[]----------------------------------------------------------------------[]*
 *| 精算情報データ用保存領域更新
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_SaveDataUpdate(void)
{

	uchar i;
// MH810100(S) K.Onodera  2020/02/17  車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)	
	uchar	card_info[16];
// MH810100(E) K.Onodera  2020/02/17  車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)	
	
	// 遠隔NT-NETの接続設定が有効でかつ料金テスト実行中でなければ精算情報データを更新
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		memcpy(&ntNet_152_SaveData.Oiban, &PayData.Oiban, sizeof(ntNet_152_SaveData.Oiban));
		ntNet_152_SaveData.PayMethod = PayData.PayMethod;
		ntNet_152_SaveData.PayMode = PayData.PayMode;
// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
		ntNet_152_SaveData.CMachineNo = PayData.CMachineNo;
// MH810100(E) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		ntNet_152_SaveData.WPlace = PayData.WPlace;
// MH810100(E) K.Onodera  2020/02/05  車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		ntNet_152_SaveData.KakariNo = PayData.KakariNo;
		ntNet_152_SaveData.OutKind = PayData.OutKind;
		ntNet_152_SaveData.CountSet = PayData.CountSet;
		ntNet_152_SaveData.ReceiptIssue = PayData.ReceiptIssue;
		ntNet_152_SaveData.Syubet = PayData.syu;
		ntNet_152_SaveData.Price = PayData.WPrice;
		ntNet_152_SaveData.CashPrice = PayData.WTotalPrice;
		ntNet_152_SaveData.InPrice = PayData.WInPrice;
		ntNet_152_SaveData.ChgPrice = PayData.WChgPrice;
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// 不具合修正(S) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
//		if( PayData.FRK_RetMod == 0 ){
//			ntNet_152_SaveData.FrkReturn = PayData.FRK_Return;
//		}else{
//			ntNet_152_SaveData.FrkReturn = 0;
//		}
//// 不具合修正(E) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
		ntNet_152_SaveData.FrkReturn = 0;
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		memcpy(&ntNet_152_SaveData.in_coin[0], &PayData.in_coin[0], sizeof(ntNet_152_SaveData.in_coin));
		memcpy(&ntNet_152_SaveData.out_coin[0], &PayData.out_coin[0], sizeof(ntNet_152_SaveData.out_coin));
		ntNet_152_SaveData.f_escrow = PayData.f_escrow;
		ntNet_152_SaveData.HaraiModoshiFusoku = PayData.WFusoku;
		ntNet_152_SaveData.AntiPassCheck = PayData.PassCheck;
		ntNet_152_SaveData.ParkNoInPass = PayData.teiki.ParkingNo;
		ntNet_152_SaveData.pkno_syu = PayData.teiki.pkno_syu;
		ntNet_152_SaveData.teiki_id = PayData.teiki.id;
		ntNet_152_SaveData.teiki_syu = PayData.teiki.syu;
// MH810100(S) K.Onodera  2020/02/17  車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
		ntNet_152_SaveData.MediaKind1 = PayData.MediaKind1;
		memcpy( ntNet_152_SaveData.MediaCardNo1, PayData.MediaCardNo1, sizeof( ntNet_152_SaveData.MediaCardNo1 ) );
		if( PayData.MediaKind1 == CARD_TYPE_PASS ){
			memset( card_info, 0, sizeof(card_info) );
			intoas( card_info, PayData.teiki.syu, 2 );
			memcpy( ntNet_152_SaveData.MediaCardInfo1, card_info, sizeof( ntNet_152_SaveData.MediaCardInfo1 ) );
		}
		ntNet_152_SaveData.MediaKind2 = PayData.MediaKind2;
		memcpy( ntNet_152_SaveData.MediaCardNo2, PayData.MediaCardNo2, sizeof( ntNet_152_SaveData.MediaCardNo2 ) );
// MH810100(E) K.Onodera  2020/02/17  車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
// MH321800(S) hosoda ICクレジット対応
		if (isEC_USE()) {
		// 決済リーダ
			ntNet_152_SaveData.e_pay_kind = PayData.Electron_data.Ec.e_pay_kind;
			ntNet_152_SaveData.e_pay_ryo = PayData.Electron_data.Ec.pay_ryo;
			memcpy(&ntNet_152_SaveData.e_Card_ID[0], &PayData.Electron_data.Ec.Card_ID[0], sizeof(ntNet_152_SaveData.e_Card_ID));
			ntNet_152_SaveData.e_pay_after = PayData.Electron_data.Ec.pay_after;
		}
		else {
// MH321800(E) hosoda ICクレジット対応
		ntNet_152_SaveData.e_pay_kind = PayData.Electron_data.Suica.e_pay_kind;
		ntNet_152_SaveData.e_pay_ryo = PayData.Electron_data.Suica.pay_ryo;
// MH321800(S) hosoda ICクレジット対応
//-		memcpy(&ntNet_152_SaveData.e_Card_ID[0], &PayData.Electron_data.Suica.Card_ID[0], sizeof(ntNet_152_SaveData.e_Card_ID));
		// e_Card_ID[20], Suica.Card_ID[16]のため、空白で埋めてから左詰めコピー
		memset(&ntNet_152_SaveData.e_Card_ID[0], 0x20, sizeof(ntNet_152_SaveData.e_Card_ID));
		memcpy(&ntNet_152_SaveData.e_Card_ID[0], &PayData.Electron_data.Suica.Card_ID[0], sizeof(PayData.Electron_data.Suica.Card_ID));
// MH321800(E) hosoda ICクレジット対応
		ntNet_152_SaveData.e_pay_after = PayData.Electron_data.Suica.pay_after;
// MH321800(S) hosoda ICクレジット対応
		}
// MH321800(E) hosoda ICクレジット対応
		ntNet_152_SaveData.c_pay_ryo = PayData.credit.pay_ryo;
		memcpy(&ntNet_152_SaveData.c_Card_No[0], &PayData.credit.card_no[0], sizeof(ntNet_152_SaveData.c_Card_No));
// GG120600(S) // Phase9 静的解析指摘修正
//		memcpy(&ntNet_152_SaveData.c_cct_num[0], &PayData.credit.CCT_Num[0], sizeof(ntNet_152_SaveData.c_cct_num));
		memset(&ntNet_152_SaveData.c_cct_num[0], 0x20, sizeof(ntNet_152_SaveData.c_cct_num));
		memcpy(&ntNet_152_SaveData.c_cct_num[0], &PayData.credit.CCT_Num[0], sizeof(ntNet_152_SaveData.c_cct_num) < sizeof(PayData.credit.CCT_Num) ? sizeof(ntNet_152_SaveData.c_cct_num) : sizeof(PayData.credit.CCT_Num));
// GG120600(E) // Phase9 静的解析指摘修正
		memcpy(&ntNet_152_SaveData.c_kid_code[0], &PayData.credit.kid_code[0], sizeof(ntNet_152_SaveData.c_kid_code));
// MH810105(S) MH364301 QRコード決済対応
//		ntNet_152_SaveData.c_app_no = PayData.credit.app_no;
// MH810105(E) MH364301 QRコード決済対応
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
		memcpy(&ntNet_152_SaveData.c_trade_no[0], &PayData.credit.ShopAccountNo, sizeof(ntNet_152_SaveData.c_trade_no));
		ntNet_152_SaveData.c_slipNo = PayData.credit.slip_no;
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
// MH810105(S) MH364301 QRコード決済対応
		// 取引ステータス
		if (PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
			ntNet_152_SaveData.e_Transactiontatus = 3;	// 未了支払不明
		}
		else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			ntNet_152_SaveData.e_Transactiontatus = 2;	// 未了支払済み
		}
		else if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			ntNet_152_SaveData.e_Transactiontatus = 1;	// みなし決済
		}
		else {
			ntNet_152_SaveData.e_Transactiontatus = 0;	// 支払い
		}

		if (PayData.credit.pay_ryo != 0) {
			// 承認番号
			ntNet_152_SaveData.c_app_no = PayData.credit.app_no;
		}
		else {
			// 問い合わせ番号／取引番号（QR）
			memcpy(ntNet_152_SaveData.e_inquiry_num,
					PayData.Electron_data.Ec.inquiry_num,
					sizeof(PayData.Electron_data.Ec.inquiry_num));
			switch (PayData.Electron_data.Ec.e_pay_kind) {
			case EC_ID_USED:
				// 承認番号
				ntNet_152_SaveData.c_app_no =
						astoinl(PayData.Electron_data.Ec.Brand.Id.Approval_No, 7);
				break;
			case EC_QUIC_PAY_USED:
				// 承認番号
				ntNet_152_SaveData.c_app_no =
						astoinl(PayData.Electron_data.Ec.Brand.Quickpay.Approval_No, 7);
				break;
			case EC_PITAPA_USED:
				// 承認番号
				// オフライン承認決済時はオール'*'のため、'0'へ変換する
				if (PayData.Electron_data.Ec.Brand.Pitapa.Approval_No[0] == '*') {
					ntNet_152_SaveData.c_app_no = 0;
				}
				else {
					ntNet_152_SaveData.c_app_no =
							astoinl(PayData.Electron_data.Ec.Brand.Pitapa.Approval_No, 8);
				}
				break;
			case EC_QR_USED:
				// Mch取引番号
				memcpy(ntNet_152_SaveData.qr_MchTradeNo,
						PayData.Electron_data.Ec.Brand.Qr.MchTradeNo,
						sizeof(ntNet_152_SaveData.qr_MchTradeNo));
				// 支払端末ID
				memcpy(ntNet_152_SaveData.qr_PayTermID,
						PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo,
						sizeof(ntNet_152_SaveData.qr_PayTermID));
				// 決済ブランド
				ntNet_152_SaveData.qr_PayKind =
						PayData.Electron_data.Ec.Brand.Qr.PayKind;
				break;
			default:
				break;
			}
		}
// MH810105(E) MH364301 QRコード決済対応
		for(i=0; i<WTIK_USEMAX; i++){
			ntNet_152_SaveData.DiscountData[i].ParkingNo = PayData.DiscountData[i].ParkingNo;
			ntNet_152_SaveData.DiscountData[i].DiscSyu = PayData.DiscountData[i].DiscSyu;
			ntNet_152_SaveData.DiscountData[i].DiscNo = PayData.DiscountData[i].DiscNo;
			ntNet_152_SaveData.DiscountData[i].DiscCount = PayData.DiscountData[i].DiscCount;
			ntNet_152_SaveData.DiscountData[i].Discount = PayData.DiscountData[i].Discount;
			ntNet_152_SaveData.DiscountData[i].DiscInfo1 = PayData.DiscountData[i].DiscInfo1;
			ntNet_152_SaveData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.DiscountData[i].uDiscData.common.DiscInfo2;
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			ntNet_152_SaveData.DiscountData[i].uDiscData.common.PrevDiscount =
				PayData.DiscountData[i].uDiscData.common.PrevDiscount;
			ntNet_152_SaveData.DiscountData[i].uDiscData.common.PrevUsageDiscount =
				PayData.DiscountData[i].uDiscData.common.PrevUsageDiscount;
			ntNet_152_SaveData.DiscountData[i].uDiscData.common.PrevUsageDiscCount =
				PayData.DiscountData[i].uDiscData.common.PrevUsageDiscCount;
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
		}
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
		for(i=0; i<DETAIL_SYU_MAX; i++){
			ntNet_152_SaveData.DetailData[i].ParkingNo = PayData.DetailData[i].ParkingNo;
			ntNet_152_SaveData.DetailData[i].DiscSyu = PayData.DetailData[i].DiscSyu;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscNo = PayData.DetailData[i].uDetail.Common.DiscNo;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscCount = PayData.DetailData[i].uDetail.Common.DiscCount;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.Discount = PayData.DetailData[i].uDetail.Common.Discount;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscInfo1 = PayData.DetailData[i].uDetail.Common.DiscInfo1;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscInfo2 = PayData.DetailData[i].uDetail.Common.DiscInfo2;
		}
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	}
}

// MH810103 GG119202(S) 不要機能削除(センタークレジット)
///*[]----------------------------------------------------------------------[]*
// *| ｸﾚｼﾞｯﾄ:送信電文(開局ｺﾏﾝﾄﾞ:ｵﾝﾗｲﾝﾃｽﾄ)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_01( uchar ReSend )
//{
//	uchar	ret = 0;
//	struct	clk_rec	clk_tmp;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_01 ) );
//
//	/****************************************************/
//	/*	基本ﾃﾞｰﾀ作成									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定
//
//	/****************************************************/
//	/*	ｸﾚｼﾞｯﾄ共通ﾃﾞｰﾀ作成								*/
//	/****************************************************/
//	SendNtnetDt.SData136_01.Common.FmtRev		= 10;					// ﾌｫｰﾏｯﾄRev.№
//	SendNtnetDt.SData136_01.Common.DestInfo		= 99;					// 送信先情報
//	SendNtnetDt.SData136_01.Common.DataIdCode1	= 1;					// 電文識別コード①
//	SendNtnetDt.SData136_01.Common.DataIdCode2	= 0;					// 電文識別コード②
//
//	/****************************************************/
//	/*	処理追い番編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_01.SeqNo	= creSeqNo_Get();					// 現在の処理追い番取得
//	cre_ctl.Save.SeqNo				= creSeqNo_Get();					// 再送用に退避
//	creSeqNo_Count();													// 次のデータ用に処理追い番＋１
//
//	/****************************************************/
//	/*	処理年月日編集									*/
//	/****************************************************/
//	clk_tmp = CLK_REC;
//	SendNtnetDt.SData136_01.Proc.Year	= clk_tmp.year;					// 処理年
//	SendNtnetDt.SData136_01.Proc.Mon	= clk_tmp.mont;					// 処理月
//	SendNtnetDt.SData136_01.Proc.Day	= clk_tmp.date;					// 処理日
//	SendNtnetDt.SData136_01.Proc.Hour	= clk_tmp.hour;					// 処理時
//	SendNtnetDt.SData136_01.Proc.Min	= clk_tmp.minu;					// 処理分
//	SendNtnetDt.SData136_01.Proc.Sec	= (ushort)clk_tmp.seco;			// 処理秒
//
//	/****************************************************/
//	/*	退避エリア（応答データチェック用）				*/
//	/****************************************************/
//	cre_ctl.Save.Date.Year	= clk_tmp.year;								// 処理年
//	cre_ctl.Save.Date.Mon	= clk_tmp.mont;								// 処理月
//	cre_ctl.Save.Date.Day	= clk_tmp.date;								// 処理日
//	cre_ctl.Save.Date.Hour	= clk_tmp.hour;								// 処理時
//	cre_ctl.Save.Date.Min	= clk_tmp.minu;								// 処理分
//	cre_ctl.Save.Date.Sec	= (ushort)clk_tmp.seco;						// 処理秒
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ送信登録									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_01) );
//		ret = 1;
//	}
//
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]*
// *| ｸﾚｼﾞｯﾄ:送信電文(ｸﾚｼﾞｯﾄ与信問合せﾃﾞｰﾀ)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_03( void )
//{
//	uchar	ret = 0;
//	CARDDAT	t_Card;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_03 ) );
//
//	/****************************************************/
//	/*	基本ﾃﾞｰﾀ作成									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定
//
//	/****************************************************/
//	/*	ｸﾚｼﾞｯﾄ共通ﾃﾞｰﾀ作成								*/
//	/****************************************************/
//	SendNtnetDt.SData136_03.Common.FmtRev		= 10;					// ﾌｫｰﾏｯﾄRev.№
//	SendNtnetDt.SData136_03.Common.DestInfo		= 99;					// 送信先情報
//	SendNtnetDt.SData136_03.Common.DataIdCode1	= 3;					// 電文識別コード①
//	SendNtnetDt.SData136_03.Common.DataIdCode2	= 0;					// 電文識別コード②
//
//	/****************************************************/
//	/*	処理追い番編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_03.SeqNo		= creSeqNo_Get();				// 現在の処理追い番取得
//	cre_ctl.Save.SeqNo					= creSeqNo_Get();				// ｸﾚｼﾞｯﾄ売上依頼ﾃﾞｰﾀ用に退避（03は再送しない）
//	creSeqNo_Count();													// 次のデータ用に処理追い番＋１
//
//	/****************************************************/
//	/*	精算年月日編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_03.Pay.Year	= cre_ctl.PayStartTime.Year;	// 精算開始年
//	SendNtnetDt.SData136_03.Pay.Mon		= cre_ctl.PayStartTime.Mon;		// 精算開始月
//	SendNtnetDt.SData136_03.Pay.Day		= cre_ctl.PayStartTime.Day; 	// 精算開始日
//	SendNtnetDt.SData136_03.Pay.Hour	= cre_ctl.PayStartTime.Hour;	// 精算開始時
//	SendNtnetDt.SData136_03.Pay.Min		= cre_ctl.PayStartTime.Min;		// 精算開始分
//	SendNtnetDt.SData136_03.Pay.Sec		= cre_ctl.PayStartTime.Sec; 	// 精算開始秒
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ詳細編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_03.SlipNo		= creSlipNo_Count();			// 端末処理通番（伝票№）
//	SendNtnetDt.SData136_03.Amount		= (ulong)creSeisanInfo.amount;	// 売上金額
//	memset( &SendNtnetDt.SData136_03.AppNo[0],
//		0x30, sizeof(SendNtnetDt.SData136_03.AppNo) );					// 承認番号
//	SendNtnetDt.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;		// 暗号化方式
//	SendNtnetDt.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;		// 暗号鍵番号
//	memset( &t_Card, 0, sizeof( t_Card ) );
//	memset( &t_Card.JIS2Data[0], 0x20, sizeof( t_Card.JIS2Data ) );		// JIS2ｶｰﾄﾞﾃﾞｰﾀ
//	memcpy( &t_Card.JIS2Data[0], &creSeisanInfo.jis_2[0], sizeof( t_Card.JIS2Data ) );
//	memset( &t_Card.JIS1Data[0], 0x20, sizeof( t_Card.JIS1Data ) );		// JIS1ｶｰﾄﾞﾃﾞｰﾀ
//	memcpy( &t_Card.JIS1Data[0], &creSeisanInfo.jis_1[0], sizeof( t_Card.JIS1Data ) );
//	t_Card.PayMethod	= 0;											// 支払方法（未対応）
//	t_Card.DivCount		= 0;											// 分割回数（未対応）
//	AesCBCEncrypt( (uchar *)&t_Card, sizeof( t_Card ) );				// AES 暗号化
//	memcpy( (uchar *)&SendNtnetDt.SData136_03.Crypt,
//		(uchar *)&t_Card, sizeof( SendNtnetDt.SData136_03.Crypt ) );	// ｸﾚｼﾞｯﾄ情報
//
//	/****************************************************/
//	/*	退避エリア（応答データチェック用）				*/
//	/****************************************************/
//	cre_ctl.Save.Date.Year	= cre_ctl.PayStartTime.Year;				// 精算開始年
//	cre_ctl.Save.Date.Mon	= cre_ctl.PayStartTime.Mon;					// 精算開始月
//	cre_ctl.Save.Date.Day	= cre_ctl.PayStartTime.Day; 				// 精算開始日
//	cre_ctl.Save.Date.Hour	= cre_ctl.PayStartTime.Hour;				// 精算開始時
//	cre_ctl.Save.Date.Min	= cre_ctl.PayStartTime.Min;					// 精算開始分
//	cre_ctl.Save.Date.Sec	= cre_ctl.PayStartTime.Sec; 				// 精算開始秒
//	cre_ctl.Save.SlipNo		= creSlipNo_Get();;							// 端末処理通番（伝票№）
//	cre_ctl.Save.Amount		= (ulong)creSeisanInfo.amount;				// 売上金額
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ送信登録									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_03) );
//		ret = 1;
//	}
//
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]*
// *| ｸﾚｼﾞｯﾄ:送信電文(ｸﾚｼﾞｯﾄ売上依頼ﾃﾞｰﾀ)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_05( uchar ReSend )
//{
//	uchar	ret = 0;
//	t_MediaInfoCre	t_Card;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_05 ) );
//
//	/****************************************************/
//	/*	基本ﾃﾞｰﾀ作成									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定
//
//	/****************************************************/
//	/*	ｸﾚｼﾞｯﾄ共通ﾃﾞｰﾀ作成								*/
//	/****************************************************/
//	SendNtnetDt.SData136_05.Common.FmtRev		= 11;					// ﾌｫｰﾏｯﾄRev.№
//	SendNtnetDt.SData136_05.Common.DestInfo		= 99;					// 送信先情報
//	SendNtnetDt.SData136_05.Common.DataIdCode1	= 5;					// 電文識別コード①
//	SendNtnetDt.SData136_05.Common.DataIdCode2	= 0;					// 電文識別コード②
//
//	/****************************************************/
//	/*	処理追い番編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_05.SeqNo	= creSeqNo_Get();					// 現在の処理追い番取得
//	cre_ctl.Save.SeqNo				= creSeqNo_Get();					// 再送用に退避
//	creSeqNo_Count();													// 次のデータ用に処理追い番＋１
//
//	/****************************************************/
//	/*	精算年月日編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_05.Pay.Year	= cre_uriage.back.time.Year;	// 精算年
//	SendNtnetDt.SData136_05.Pay.Mon		= cre_uriage.back.time.Mon;		// 精算月
//	SendNtnetDt.SData136_05.Pay.Day		= cre_uriage.back.time.Day;		// 精算日
//	SendNtnetDt.SData136_05.Pay.Hour	= cre_uriage.back.time.Hour;	// 精算時
//	SendNtnetDt.SData136_05.Pay.Min		= cre_uriage.back.time.Min;		// 精算分
//	SendNtnetDt.SData136_05.Pay.Sec		= 0;							// 精算秒
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ詳細編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_05.SlipNo		= cre_uriage.back.slip_no;		// 端末処理通番（伝票№）
//	SendNtnetDt.SData136_05.Amount		= cre_uriage.back.ryo;			// 売上金額
//	memcpy( &SendNtnetDt.SData136_05.AppNo[0],							// 承認番号
//		&cre_uriage.back.AppNoChar[0], sizeof(SendNtnetDt.SData136_05.AppNo) );
//	memcpy( &SendNtnetDt.SData136_05.ShopAccountNo[0],					// 加盟店取引番号
//		&cre_uriage.back.shop_account_no[0], sizeof(SendNtnetDt.SData136_05.ShopAccountNo) );
//	SendNtnetDt.SData136_05.PayMethod	= 0;							// 支払方法（未対応）
//	SendNtnetDt.SData136_05.DivCount	= 0;							// 分割回数（未対応）
//	SendNtnetDt.SData136_05.CMachineNo  = cre_uriage.back.CMachineNo;	// 駐車券機械№(入庫機械№)	0～255
//	SendNtnetDt.SData136_05.PayMethod2  = cre_uriage.back.PayMethod2;	// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
//	SendNtnetDt.SData136_05.PayClass    = cre_uriage.back.PayClass;		// 処理区分(0＝精算/1＝再精算/2＝精算中止
//	SendNtnetDt.SData136_05.PayMode     = cre_uriage.back.PayMode;		// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
//	SendNtnetDt.SData136_05.FlapArea    = cre_uriage.back.FlapArea;		// フラップシステム	区画		0～99
//	SendNtnetDt.SData136_05.FlapParkNo  = cre_uriage.back.FlapParkNo;	// 					車室番号	0～9999
//	SendNtnetDt.SData136_05.Price       = cre_uriage.back.Price;		// 駐車料金		0～
//	SendNtnetDt.SData136_05.OptionPrice = cre_uriage.back.OptionPrice;	// その他料金	0～
//	SendNtnetDt.SData136_05.Discount    = cre_uriage.back.Discount;		// 割引額(合算)
//	SendNtnetDt.SData136_05.CashPrice   = cre_uriage.back.CashPrice;	// 現金売上		0～
//	SendNtnetDt.SData136_05.PayCount    = cre_uriage.back.PayCount;		// 精算追番(0～99999)
//	// 精算媒体情報1
//	SendNtnetDt.SData136_05.MediaKind = cre_uriage.back.MediaKind;					// 種別(媒体)	0～99
//	if(cre_uriage.back.MediaKind != 0){// 暗号化は種別が有効な時のみ行なう
//		SendNtnetDt.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;				// 暗号化方式
//		SendNtnetDt.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;				// 暗号鍵番号
//		memset( &t_Card, 0, sizeof( t_Card ) );
//		memcpy( &t_Card.MediaCardNo[0], &cre_uriage.back.Media.MediaCardNo[0], sizeof( t_Card.MediaCardNo ) );// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		memcpy( &t_Card.MediaCardInfo[0], &cre_uriage.back.Media.MediaCardInfo[0], sizeof( t_Card.MediaCardInfo ) );// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		AesCBCEncrypt( (uchar *)&t_Card, sizeof( t_Card ) );						// AES 暗号化
//		memcpy( (uchar *)&SendNtnetDt.SData136_05.Media,
//			(uchar *)&t_Card, sizeof( SendNtnetDt.SData136_05.Media ) );
//	}
//
//	/****************************************************/
//	/*	退避エリア（応答データチェック用）				*/
//	/****************************************************/
//	cre_ctl.Save.Date.Year	= cre_uriage.back.time.Year;				// 精算年
//	cre_ctl.Save.Date.Mon	= cre_uriage.back.time.Mon;					// 精算月
//	cre_ctl.Save.Date.Day	= cre_uriage.back.time.Day;					// 精算日
//	cre_ctl.Save.Date.Hour	= cre_uriage.back.time.Hour;				// 精算時
//	cre_ctl.Save.Date.Min	= cre_uriage.back.time.Min;					// 精算分
//	cre_ctl.Save.Date.Sec	= 0;										// 精算秒
//	cre_ctl.Save.SlipNo		= cre_uriage.back.slip_no;					// 端末処理通番（伝票№）
//	cre_ctl.Save.Amount		= cre_uriage.back.ryo;						// 売上金額
//	memcpy( &cre_ctl.Save.AppNo[0], &cre_uriage.back.AppNoChar[0],
//								sizeof(cre_ctl.Save.AppNo) );			// 承認番号
//	memcpy( &cre_ctl.Save.ShopAccountNo[0], &cre_uriage.back.shop_account_no[0],
//								sizeof(cre_ctl.Save.ShopAccountNo) );	// 加盟店取引番号
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ送信登録									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_05) );
//		ret = 1;
//	}
//
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]*
// *| ｸﾚｼﾞｯﾄ:送信電文(ﾃｽﾄｺﾏﾝﾄﾞ)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_07( uchar ReSend )
//{
//	uchar	ret = 0;
//	uchar	test_kind = 0;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_07 ) );
//
//	/****************************************************/
//	/*	基本ﾃﾞｰﾀ作成									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定
//
//	/****************************************************/
//	/*	ｸﾚｼﾞｯﾄ共通ﾃﾞｰﾀ作成								*/
//	/****************************************************/
//	SendNtnetDt.SData136_07.Common.FmtRev		= 10;					// ﾌｫｰﾏｯﾄRev.№
//	SendNtnetDt.SData136_07.Common.DestInfo		= 99;					// 送信先情報
//	SendNtnetDt.SData136_07.Common.DataIdCode1	= 7;					// 電文識別コード①
//	SendNtnetDt.SData136_07.Common.DataIdCode2	= 0;					// 電文識別コード②
//
//	/****************************************************/
//	/*	処理追い番編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_07.SeqNo	= creSeqNo_Get();					// 現在の処理追い番取得
//	cre_ctl.Save.SeqNo				= creSeqNo_Get();					// 再送用に退避
//	creSeqNo_Count();													// 次のデータ用に処理追い番＋１
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ詳細編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_07.TestKind	= test_kind;					// ﾃｽﾄ種別 0=生存確認
//
//	/****************************************************/
//	/*	退避エリア（応答データチェック用）				*/
//	/****************************************************/
//	cre_ctl.Save.TestKind	= test_kind;								// ﾃｽﾄ種別
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ送信登録									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_07) );
//		ret = 1;
//	}
//
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]*
// *| ｸﾚｼﾞｯﾄ:送信電文(ｸﾚｼﾞｯﾄ返品問合せﾃﾞｰﾀ)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_09( uchar ReSend )
//{
//	uchar	ret = 0;
//	struct	clk_rec	clk_tmp;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_09 ) );
//
//	/****************************************************/
//	/*	基本ﾃﾞｰﾀ作成									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定
//
//	/****************************************************/
//	/*	ｸﾚｼﾞｯﾄ共通ﾃﾞｰﾀ作成								*/
//	/****************************************************/
//	SendNtnetDt.SData136_09.Common.FmtRev		= 10;					// ﾌｫｰﾏｯﾄRev.№
//	SendNtnetDt.SData136_09.Common.DestInfo		= 99;					// 送信先情報
//	SendNtnetDt.SData136_09.Common.DataIdCode1	= 9;					// 電文識別コード①
//	SendNtnetDt.SData136_09.Common.DataIdCode2	= 0;					// 電文識別コード②
//
//	/****************************************************/
//	/*	処理追い番編集									*/
//	/****************************************************/
//	SendNtnetDt.SData136_09.SeqNo	= creSeqNo_Get();					// 現在の処理追い番取得
//	cre_ctl.Save.SeqNo				= creSeqNo_Get();					// 再送用に退避
//	creSeqNo_Count();													// 次のデータ用に処理追い番＋１
//
//	/****************************************************/
//	/*	処理年月日編集									*/
//	/****************************************************/
//	clk_tmp = CLK_REC;
//	SendNtnetDt.SData136_09.Proc.Year	= clk_tmp.year;					// 処理年
//	SendNtnetDt.SData136_09.Proc.Mon	= clk_tmp.mont;					// 処理月
//	SendNtnetDt.SData136_09.Proc.Day	= clk_tmp.date;					// 処理日
//	SendNtnetDt.SData136_09.Proc.Hour	= clk_tmp.hour;					// 処理時
//	SendNtnetDt.SData136_09.Proc.Min	= clk_tmp.minu;					// 処理分
//	SendNtnetDt.SData136_09.Proc.Sec	= (ushort)clk_tmp.seco;			// 処理秒
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ詳細編集									*/
//	/****************************************************/
//// TODO: SlipNo をカウントする／しないは要確認
////	SendNtnetDt.SData136_09.SlipNo		= creSlipNo_Get();				// 端末処理通番（伝票№）
//	SendNtnetDt.SData136_09.SlipNo		= creSlipNo_Count();			// 端末処理通番（伝票№）
//	memset( &SendNtnetDt.SData136_09.AppNo[0],
//		0x30, sizeof(SendNtnetDt.SData136_09.AppNo) );					// 返品対象 承認番号（未対応）
//	memset( &SendNtnetDt.SData136_09.ShopAccountNo[0],
//		0x30, sizeof(SendNtnetDt.SData136_09.ShopAccountNo) );			// 返品対象 加盟店取引番号（未対応）
//
//	/****************************************************/
//	/*	退避エリア（応答データチェック用）				*/
//	/****************************************************/
//	cre_ctl.Save.Date.Year	= clk_tmp.year;								// 処理年
//	cre_ctl.Save.Date.Mon	= clk_tmp.mont;								// 処理月
//	cre_ctl.Save.Date.Day	= clk_tmp.date;								// 処理日
//	cre_ctl.Save.Date.Hour	= clk_tmp.hour;								// 処理時
//	cre_ctl.Save.Date.Min	= clk_tmp.minu;								// 処理分
//	cre_ctl.Save.Date.Sec	= (ushort)clk_tmp.seco;						// 処理秒
//	cre_ctl.Save.SlipNo		= creSlipNo_Get();;							// 端末処理通番（伝票№）
//	memset( &cre_ctl.Save.AppNo[0], 0x30, 6 );							// 承認番号（未対応）
//	memset( &cre_ctl.Save.ShopAccountNo[0], 0x30, 20 );					// 加盟店取引番号（未対応）
//
//	/****************************************************/
//	/*	ﾃﾞｰﾀ送信登録									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_09) );
//		ret = 1;
//	}
//
//	return ret;
//}
// MH810103 GG119202(E) 不要機能削除(センタークレジット)

//[]----------------------------------------------------------------------[]
///	@brief		NT-NETデータに設定するシーケンシャルNo.を取得する
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		シーケンシャルNo.
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
uchar	GetNtDataSeqNo(void)
{
	uchar	seqNo = basicSeqNo;
	
	++basicSeqNo;
	if(basicSeqNo > 99) {
		basicSeqNo = 1;
	}
	
	return seqNo;
}	

void NTNET_CtrlRecvData_DL(void)
{
	switch (RecvNtnetDt.DataBasic.DataKind) {
// GG120600(S) // Phase9 センター用端末情報データに受信電文リビジョンを追加
//	case 111:												// 遠隔メンテナンス要求
	case REMOTE_MNT_REQ:									// 遠隔メンテナンス要求
// GG120600(E) // Phase9 センター用端末情報データに受信電文リビジョンを追加
		NTNET_RevData111_DL();
		break;
	default:
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 遠隔メンテナンス要求(ﾃﾞｰﾀ種別1)受信処理                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData111_DL                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai                                                 |*/
/*| Date         : 2014-12-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
void NTNET_RevData111_DL(void)
{
	DATA_KIND_111_DL *p = &RecvNtnetDt.RData111_DL;
	ushort len1 = sizeof(*p) - offsetof(DATA_KIND_111_DL, ProgDlReq);	// 命令番号1～予備の長さ
	ushort len2 = sizeof(*p) - offsetof(DATA_KIND_111_DL, FtpInfo);		// FTP サーバIP～予備の長さ

	// 送信先ターミナル№チェック
// GG120600(S) // Phase9 #6219 【連動評価指摘】センターに登録されているGT-4100のターミナルNo.と端末の機械No.が異なる場合に設定アップロードに失敗する
//	if ((p->SMachineNo == CPrmSS[S_PAY][2])) {
	if ((p->SMachineNo == CPrmSS[S_NTN][3])) {
// GG120600(E) // Phase9 #6219 【連動評価指摘】センターに登録されているGT-4100のターミナルNo.と端末の機械No.が異なる場合に設定アップロードに失敗する
		// 暗号化されているか？
		if (p->DataBasic.CMN_DT.DT_BASIC.encryptMode) {
			AesCBCDecrypt((uchar *)&p->FtpInfo, sizeof(p->FtpInfo));
		}
		remotedl_ftp_info_set(&p->FtpInfo);
		queset( OPETCBNO, REMOTE_DL_REQ, len1 - len2, &p->ProgDlReq );
	}
//	else {
//		// パラメータNG
//		p->ProgDlReq.ReqResult = REQ_NOT_ACCEPT;
//		NTNET_Snd_Data118_DL(&p->ProgDlReq);
//	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 遠隔メンテナンス要求応答(ﾃﾞｰﾀ種別2)送信処理                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data118_DL                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai                                                 |*/
/*| Date         : 2014-12-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
void NTNET_Snd_Data118_DL(t_ProgDlReq *pDlReq)
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_118_DL ) );

	BasicDataMake( 118, 1 );								// 基本ﾃﾞｰﾀ作成
	SendNtnetDt.DataBasic.SystemID = REMOTE_DL_SYSTEM;		// システムID
	SendNtnetDt.SData118_DL.FmtRev = 10;					// フォーマットRev.№
// GG120600(S) // Phase9 #6219 【連動評価指摘】センターに登録されているGT-4100のターミナルNo.と端末の機械No.が異なる場合に設定アップロードに失敗する
//	SendNtnetDt.SData118_DL.SMachineNo = CPrmSS[S_PAY][2];	// 送信元ターミナル№
	SendNtnetDt.SData118_DL.SMachineNo = CPrmSS[S_NTN][3];	// 送信元ターミナル№
// GG120600(E) // Phase9 #6219 【連動評価指摘】センターに登録されているGT-4100のターミナルNo.と端末の機械No.が異なる場合に設定アップロードに失敗する
	memcpy(&SendNtnetDt.SData118_DL.ProgDlReq, pDlReq, sizeof(*pDlReq));

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_118_DL ));
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 遠隔監視データ(ﾃﾞｰﾀ種別61)編集処理                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Edit_Data125_DL                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai                                                 |*/
/*| Date         : 2014-12-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
unsigned short NTNET_Edit_Data125_DL(Rmon_log *p_RcptDat, DATA_KIND_125_DL *p_NtDat)
{
	int len;

	memset( p_NtDat, 0, sizeof( DATA_KIND_125_DL ) );
	NTNET_Edit_BasicData( 125, 0, p_RcptDat->RmonSeqNo, &p_NtDat->DataBasic);		// 基本ﾃﾞｰﾀ作成
	p_NtDat->DataBasic.SystemID = REMOTE_DL_SYSTEM;									// システムID

	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなくログの時刻をセットするので再セット
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// 処理年
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// 処理分
	p_NtDat->DataBasic.Sec = (uchar)p_RcptDat->Date_Time.Sec;						// 処理秒

// GG120600(S) // Phase9 遠隔監視データ変更
//	p_NtDat->FmtRev = 10;															// ﾌｫｰﾏｯﾄRev.№
	p_NtDat->FmtRev = 11;															// ﾌｫｰﾏｯﾄRev.№
// GG120600(E) // Phase9 遠隔監視データ変更
	p_NtDat->SMachineNo = REMOTE_PC_TERMINAL_NO;									// 送信元ターミナル№
	p_NtDat->InstNo1 = p_RcptDat->RmonInstNo.ulinstNo[0];							// 命令番号1
	p_NtDat->InstNo2 = p_RcptDat->RmonInstNo.ulinstNo[1];							// 命令番号2
	p_NtDat->FuncNo = p_RcptDat->RmonFuncNo;										// 端末機能番号
	p_NtDat->ProcNo = p_RcptDat->RmonProcNo;										// 処理番号
	p_NtDat->ProcInfoNo = p_RcptDat->RmonProcInfoNo;								// 処理情報番号
	p_NtDat->StatusNo = p_RcptDat->RmonStatusNo;									// 状況番号
	p_NtDat->StatusDetailNo = p_RcptDat->RmonStatusDetailNo;						// 状況詳細番号
	p_NtDat->MonitorInfo = p_RcptDat->RmonInfo;										// 端末監視情報
// GG120600(S) // Phase9 遠隔監視データ変更
	p_NtDat->RmonFrom = p_RcptDat->RmonFrom;										// 命令要求元0=センター/1=端末
	p_NtDat->RmonUploadReq = p_RcptDat->RmonUploadReq;								// 設定アップロード要求
// GG120600(E) // Phase9 遠隔監視データ変更

	if (p_RcptDat->RmonStartTime1.Year) {
		p_NtDat->Start_Year1 = p_RcptDat->RmonStartTime1.Year % 100;				// 予定開始時刻1
		memcpy(&p_NtDat->Start_Mon1, &p_RcptDat->RmonStartTime1.Mon, 4);
	}
	if (p_RcptDat->RmonStartTime2.Year) {
		p_NtDat->Start_Year2 = p_RcptDat->RmonStartTime2.Year % 100;				// 予定開始時刻2
		memcpy(&p_NtDat->Start_Mon2, &p_RcptDat->RmonStartTime2.Mon, 4);
	}
	len = strlen((char *)p_RcptDat->RmonProgNo);
	if (len > 0) {
		memcpy(p_NtDat->Prog_No, p_RcptDat->RmonProgNo, len-sizeof(p_NtDat->Prog_Ver));		// プログラム部番
		memcpy(p_NtDat->Prog_Ver, &p_RcptDat->RmonProgNo[len-sizeof(p_NtDat->Prog_Ver)], sizeof(p_NtDat->Prog_Ver));	// プログラムバージョン
	}

	return sizeof( DATA_KIND_125_DL );
}
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
///*[]----------------------------------------------------------------------[]*
// *| Park I Pro電文受信処理
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevDataPiP
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static void	NTNET_RevDataPiP(void)
{
	// ParkingWeb経由ParkiPro設定なし？
	if( !_is_ParkingWeb_pip() ){
		return;
	}

	// データ種別
	switch( RecvNtnetDt.DataBasic.DataKind )
	{
		// 領収証再発行要求
		case PIP_REQ_KIND_RECEIPT_AGAIN:
			NTNET_RevData16_01_ReceiptAgain();
			break;

// MH810100(S) K.Onodera 2019/10/25 車番チケットレス（Park i PRO対応）
//		// 振替精算要求
//		case PIP_REQ_KIND_FURIKAE:
//			NTNET_RevData16_03_Furikae();
//			break;
//
//		// 受付券発行要求
//		case PIP_REQ_KIND_RECEIVE_TKT:
//			NTNET_RevData16_05_ReceiveTkt();
//			break;
// MH810100(E) K.Onodera 2019/10/25 車番チケットレス（Park i PRO対応）

		// 遠隔精算要求
		case PIP_REQ_KIND_REMOTE_CALC:
			NTNET_RevData16_08_RemoteCalc();
			break;

		// 発生中エラーアラーム情報要求
		case PIP_REQ_KIND_OCCUR_ERRALM:
			NTNET_RevData16_10_ErrorAlarm();
			break;

		default:
			break;
	}
}

static ushort PIP_CheckCarInfo(uchar area,ulong no, ushort* pos)
{
	ushort	index;

	if ((area != 0) ||						// ※FT-4800では0固定
		((no <= 0)  || (no > 9900)))
		return PIP_RES_RESULT_NG_PARAM;

	if (LKopeGetLockNum(area, no, pos) == 0)
		return PIP_RES_RESULT_NG_NO_CAR;

	index = *pos - 1;
	if (LockInfo[index].lok_syu == 0 || LockInfo[index].ryo_syu == 0)
		return PIP_RES_RESULT_NG_NO_CAR;

	return PIP_RES_RESULT_OK;
}

// レビュー指摘対応(S) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
static	Receipt_data	wkReceipt;
// レビュー指摘対応(E) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可

///*[]----------------------------------------------------------------------[]*
// *| 受付券発行応答
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData16_05_ReceiveTkt
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
//static void NTNET_RevData16_05_ReceiveTkt(void)
void NTNET_RevData16_05_ReceiveTkt(void)
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
{
	ushort rslt = PIP_RES_RESULT_NG_DENY;
	ushort pos = 0;
	ushort index = 0;
	ushort type = 0;
// レビュー指摘対応(S) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
	uchar  permit = 0;
// レビュー指摘対応(E) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可

	// 受付券機能なし？
	if ( prm_get( COM_PRM,S_TYP,62,1,1 ) == 0 ) {
		rslt = PIP_RES_RESULT_NG_DENY;					// 受付拒否
	}
	// 自動集計中？
	else if( auto_syu_prn == 2 ){
		rslt = PIP_RES_RESULT_NG_DENY;					// 受付拒否
	}
	else {
		rslt = PIP_CheckCarInfo(RecvNtnetDt.RData16_05.Area,RecvNtnetDt.RData16_05.ulNo, &pos);	// 車室情報ﾁｪｯｸ
		if (rslt == PIP_RES_RESULT_OK) {
			index = pos - 1;
// レビュー指摘対応(S) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
			memset( &wkReceipt, 0, sizeof( Receipt_data) );
			if( PiP_GetNewestPayReceiptData( &wkReceipt, RecvNtnetDt.RData16_05.Area, RecvNtnetDt.RData16_05.ulNo ) ){
				// サービスタイム内出庫？且つ、精算後(在車)
				if( wkReceipt.OutKind == 99 && (FLAPDT.flp_data[index].mode == FLAP_CTRL_MODE5 || FLAPDT.flp_data[index].mode == FLAP_CTRL_MODE6) ){
					permit = 1;
				}
			}
// レビュー指摘対応(E) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
// レビュー指摘対応(S) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
//			if (OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod <= 1) &&			// 待機状態?
//				(FLAPDT.flp_data[index].mode >= FLAP_CTRL_MODE2 && FLAPDT.flp_data[index].mode <= FLAP_CTRL_MODE4)) {	// 駐車中?
			if( OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod <= 1 ){
				if( (FLAPDT.flp_data[index].mode >= FLAP_CTRL_MODE2 && FLAPDT.flp_data[index].mode <= FLAP_CTRL_MODE4) || permit ){
// レビュー指摘対応(E) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
					// 受付券発行処理
// レビュー指摘対応(S) K.Onodera 2016/10/04 受付券は上限枚数に関係なく発行し、再発行とする
//					type = (FLAPDT.flp_data[index].uketuke == 0)? 0x12 : 0x11;
					type = (FLAPDT.flp_data[index].uketuke == 0)? 0x22 : 0x21;
// レビュー指摘対応(E) K.Onodera 2016/10/04 受付券は上限枚数に関係なく発行し、再発行とする
					FLAPDT.flp_data[index].uketuke = 0;					// 発行済みフラグOFF
					if( uke_isu((ulong)(LockInfo[index].area*10000L + LockInfo[index].posi), (ushort)(pos), type) ){
						// 発行NG
						rslt = PIP_RES_RESULT_NG_DENY;
					}else{
						wopelg( OPLOG_PARKI_UKETUKEHAKKO, 0, 0 );			// 操作履歴登録
					}
// レビュー指摘対応(S) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
				}else{
					rslt = PIP_RES_RESULT_NG_DENY;
				}
// レビュー指摘対応(E) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
			}else {
				rslt = PIP_RES_RESULT_NG_DENY;
			}
		}
	}
	
	// 応答データ送信
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_06 ) );

	BasicDataMake( PIP_RES_KIND_RECEIVE_TKT, 1 );									// 基本ﾃﾞｰﾀ作成 ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定

	SendNtnetDt.SData16_06.Common.FmtRev 		= 10;								// フォーマットRev.No.(10固定)
	SendNtnetDt.SData16_06.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// 送信先の端末ターミナル№をセットする（99＝遠隔PC）
	SendNtnetDt.SData16_06.Common.CenterSeqNo	= RecvNtnetDt.RData16_05.Common.CenterSeqNo;	// センター追番
	SendNtnetDt.SData16_06.Result				= rslt;								// 結果
	SendNtnetDt.SData16_06.Area 				= RecvNtnetDt.RData16_05.Area;		// 区画
	SendNtnetDt.SData16_06.ulNo 				= RecvNtnetDt.RData16_05.ulNo;		// 車室番号
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_06 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_06 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
}
// レビュー指摘対応(S) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
//static	Receipt_data	wkReceipt;
// レビュー指摘対応(E) K.Onodera 2016/10/05 サービスタイム内精算後は在車であれば受付券発行許可
// MH810105(S) MH364301 QRコード決済対応
static	CREINFO_r14	t_Credit;
static	EMONEY_r14	t_EMoney;
static	QRCODE_rXX	t_QR;
// MH810105(E) MH364301 QRコード決済対応
///*[]----------------------------------------------------------------------[]*
// *| 領収証再発行応答
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData16_01_ReceiptAgain
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static void NTNET_RevData16_01_ReceiptAgain( void )
{
	ushort			rslt = PIP_RES_RESULT_NG_DENY;
// MH810100(S) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
//	ushort			i, j, d;
// MH810100(E) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
	T_FrmReceipt	rec_data;			// 領収証印字要求＞ﾒｯｾｰｼﾞﾃﾞｰﾀ
// MH810100(S) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
//	CREINFO_r14		t_Credit;
//	EMONEY_r14		t_EMoney;
// MH810100(E) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
// 不具合修正(S) K.Onodera 2016/09/27 #1504 領収証再発行応答データで「精算追番」以降の内容が２バイトずれている
	ushort			size = 0;
// 不具合修正(E) K.Onodera 2016/09/27 #1504 領収証再発行応答データで「精算追番」以降の内容が２バイトずれている
// MH810100(S) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
	ushort	cnt_ttl;
	ushort	id;
	ushort	date[3];
	ushort	seisan_date, target_date;
// MH810100(E) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
// MH810100(S) 2020/08/18 車番チケットレス(#4599 ParkiProの領収証再発行でクレジット精算の領収証が再発行ができない)
	uchar			PayClassTmp;										// 処理区分
// MH810100(E) 2020/08/18 車番チケットレス(#4599 ParkiProの領収証再発行でクレジット精算の領収証が再発行ができない)

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_02 ) );
	memset( &wkReceipt, 0, sizeof( Receipt_data) );

// MH810100(S) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
//	// 共通
//	wkReceipt.WFlag			= 0;
//	wkReceipt.chuusi		= 0;
//	wkReceipt.Kikai_no		= (uchar)CPrmSS[S_PAY][2];															// 精算機No.
//	wkReceipt.Oiban.i		= RecvNtnetDt.RData16_01.PayCount;													// 精算追番
//	wkReceipt.WPrice		= RecvNtnetDt.RData16_01.TotalFee;													// 駐車料金or定期更新料金
//	wkReceipt.syu			= RecvNtnetDt.RData16_01.Syubet;													// 料金種別
//	wkReceipt.KakariNo		= RecvNtnetDt.RData16_01.KakariNo;													// 係員№
//	wkReceipt.PayClass		= RecvNtnetDt.RData16_01.PayClass;													// 処理区分
//	wkReceipt.ReceiptIssue	= RecvNtnetDt.RData16_01.ReceiptIssue;												// 領収書発行有無
//	wkReceipt.WInPrice		= RecvNtnetDt.RData16_01.InMoney;													// 入金額
//	wkReceipt.WChgPrice		= RecvNtnetDt.RData16_01.Change;													// 釣銭額
//	wkReceipt.WFusoku		= RecvNtnetDt.RData16_01.Fusoku;													// 払戻し不足
//	// 定期券駐車場№セット？
//	if( RecvNtnetDt.RData16_01.PassInfo.ParkNo ){
//		// teiki.pkno_syu？
//		wkReceipt.teiki.ParkingNo = RecvNtnetDt.RData16_01.PassInfo.ParkNo;
//		if( RecvNtnetDt.RData16_01.MainMedia.Kind >= 2 ){
//			wkReceipt.teiki.pkno_syu = (RecvNtnetDt.RData16_01.MainMedia.Kind - 2);
//		}else{
//			wkReceipt.teiki.pkno_syu = 0;
//		}
//		wkReceipt.teiki.id = astoinl( RecvNtnetDt.RData16_01.MainMedia.CardNo, 5 );
//		wkReceipt.teiki.syu = astoinl( RecvNtnetDt.RData16_01.MainMedia.CardInfo, 2 );
//	}
//	switch( RecvNtnetDt.RData16_01.CardPaymentKind ){			// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//		case 1:
//			memset( &t_Credit, 0, sizeof( t_Credit ) );
//			memcpy( (uchar *)&t_Credit, RecvNtnetDt.RData16_01.CardPaymentInfo, sizeof(t_Credit) );
//			// 暗号化方式=AES？
//			if( RecvNtnetDt.RData16_01.DataBasic.CMN_DT.DT_BASIC.encryptMode == 1 ){
//				AesCBCDecrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );
//			}
//			wkReceipt.credit.pay_ryo = t_Credit.amount;		// 決済額
//			wkReceipt.credit.app_no = t_Credit.app_no;		// 承認番号
//// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
////// 不具合修正(S) K.Onodera 2016/12/12 #1666 クレジットカードによる精算の領収証再発行を実施するとクレジットカード承認№が000000で印字される
////			intoasl( (uchar*)&wkReceipt.credit.AppNoChar[0], wkReceipt.credit.app_no, 6 );		// 承認番号 文字列保持 左詰め
////// 不具合修正(E) K.Onodera 2016/12/12 #1666 クレジットカードによる精算の領収証再発行を実施するとクレジットカード承認№が000000で印字される
//// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//			wkReceipt.credit.slip_no = t_Credit.slip_no;	// 伝票番号
//// MH321800(S) D.Inaba ICクレジット対応
////			memcpyFlushLeft( (uchar *)&wkReceipt.credit.card_no[0], &t_Credit.card_no[0], sizeof(wkReceipt.credit.card_no), sizeof(t_Credit.card_no) );
////			memcpyFlushLeft( (uchar *)&wkReceipt.credit.CCT_Num[0], &t_Credit.cct_num[0], sizeof(wkReceipt.credit.CCT_Num), sizeof(t_Credit.cct_num) );
////			memcpyFlushLeft( (uchar *)&wkReceipt.credit.kid_code[0], &t_Credit.kid_code[0], sizeof(wkReceipt.credit.kid_code), sizeof(t_Credit.kid_code) );
//			// 会員Noの上7桁目～下5桁目以外で'0'が格納されていたら'*'に置換
//			change_CharInArray( (uchar *)&t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '0', '*' );
//// MH321800(S) NT-NET精算データ仕様変更
////			//ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№ みなし決済時はNULL(0x0)で埋める
////			memcpyFlushLeft2( (uchar *)&wkReceipt.credit.card_no[0], &t_Credit.card_no[0], sizeof(wkReceipt.credit.card_no), sizeof(t_Credit.card_no) );
////			//ｸﾚｼﾞｯﾄｶｰﾄﾞ端末識別番号 みなし決済時はNULL(0x0)で埋める
////			memcpyFlushLeft2( (uchar *)&wkReceipt.credit.CCT_Num[0], &t_Credit.cct_num[0], sizeof(wkReceipt.credit.CCT_Num), sizeof(t_Credit.cct_num) );
////			//KIDｺｰﾄﾞ みなし決済時はNULL(0x0)で埋める
////			memcpyFlushLeft2( (uchar *)&wkReceipt.credit.kid_code[0], &t_Credit.kid_code[0], sizeof(wkReceipt.credit.kid_code), sizeof(t_Credit.kid_code) );
//			//ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№ みなし決済時はスペース埋めする
//			memcpyFlushLeft( (uchar *)&wkReceipt.credit.card_no[0], &t_Credit.card_no[0], sizeof(wkReceipt.credit.card_no), sizeof(t_Credit.card_no) );
//			//ｸﾚｼﾞｯﾄｶｰﾄﾞ端末識別番号 みなし決済時はスペース埋めする
//			memcpyFlushLeft( (uchar *)&wkReceipt.credit.CCT_Num[0], &t_Credit.cct_num[0], sizeof(wkReceipt.credit.CCT_Num), sizeof(t_Credit.cct_num) );
//			//KIDｺｰﾄﾞ みなし決済時はスペース埋めする
//			memcpyFlushLeft( (uchar *)&wkReceipt.credit.kid_code[0], &t_Credit.kid_code[0], sizeof(wkReceipt.credit.kid_code), sizeof(t_Credit.kid_code) );
//// MH321800(E) NT-NET精算データ仕様変更
//// MH321800(E) D.Inaba ICクレジット対応
//// 仕様変更(S) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
//			memcpyFlushLeft( (uchar *)&wkReceipt.credit.ShopAccountNo[0], &t_Credit.ShopAccountNo[0], sizeof(wkReceipt.credit.ShopAccountNo), sizeof(t_Credit.ShopAccountNo) );
//			memcpy( (uchar *)&wkReceipt.credit.card_name[0], RecvNtnetDt.RData16_01.card_name, sizeof(wkReceipt.credit.card_name) );
//// 仕様変更(E) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
//// MH321800(S) T.Nagai ICクレジット対応
//			wkReceipt.Electron_data.Ec.e_pay_kind = EC_CREDIT_USED;
//// MH321800(S) NT-NET精算データ仕様変更
////			if (wkReceipt.credit.app_no == 0 &&
////				wkReceipt.credit.slip_no == 0 &&
////				wkReceipt.credit.card_no[0] == 0 &&
////				wkReceipt.credit.CCT_Num[0] == 0 &&
////				wkReceipt.credit.kid_code[0] == 0 ) {
////				// 承認番号、伝票番号、会員番号、端末識別番号、KIDコードが 0 である時
//			if (wkReceipt.credit.app_no == 0 &&
//				wkReceipt.credit.slip_no == 0 &&
//				wkReceipt.credit.card_no[0] == 0x20 &&
//				wkReceipt.credit.CCT_Num[0] == 0x20 &&
//				wkReceipt.credit.kid_code[0] == 0x20 ) {
//				// 承認番号、伝票番号が0、会員番号、端末識別番号、KIDコードの先頭が0x20のとき
//// MH321800(E) NT-NET精算データ仕様変更
//				// みなし決済のフラグを立てる
//				wkReceipt.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;
//			}
//// MH321800(E) T.Nagai ICクレジット対応
//			break;
//		case 2:
//			memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//			memcpy( (uchar *)&t_EMoney, RecvNtnetDt.RData16_01.CardPaymentInfo, sizeof(t_EMoney) );
//			// 暗号化方式=AES？
//			if( RecvNtnetDt.RData16_01.DataBasic.CMN_DT.DT_BASIC.encryptMode == 1 ){
//				AesCBCDecrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );
//			}
//// MH321800(S) Y.Tanizaki ICクレジット対応
////			wkReceipt.Electron_data.Suica.pay_ryo = t_EMoney.amount;			// 電子決済精算情報 決済額
////			wkReceipt.Electron_data.Suica.pay_after = t_EMoney.card_zangaku;	// 電子決済精算情報 決済後残高
////			wkReceipt.Electron_data.Suica.pay_befor = (t_EMoney.card_zangaku + t_EMoney.amount);
////			memcpy( &wkReceipt.Electron_data.Suica.Card_ID[0], &t_EMoney.card_id[0], sizeof(wkReceipt.Electron_data.Suica.Card_ID) );	// 電子決済精算情報ｶｰﾄﾞID (Ascii 16桁)
////// 不具合修正(S) K.Onodera 2016/12/13 #1668 電子マネー精算の領収書再発行を実施した際に領収書に電子マネーの情報が印字されない
////			wkReceipt.Electron_data.Suica.e_pay_kind = SUICA_USED;				// 交通系はまとめてSuicaをセットしておく
////// 不具合修正(E) K.Onodera 2016/12/13 #1668 電子マネー精算の領収書再発行を実施した際に領収書に電子マネーの情報が印字されない
////			break;
//			if(isEC_USE()) {
//				wkReceipt.Electron_data.Ec.pay_ryo = t_EMoney.amount;			// 電子決済精算情報 決済額
//				wkReceipt.Electron_data.Ec.pay_after = t_EMoney.card_zangaku;	// 電子決済精算情報 決済後残高
//				wkReceipt.Electron_data.Ec.pay_befor = (t_EMoney.card_zangaku + t_EMoney.amount);
//				memcpy( &wkReceipt.Electron_data.Ec.Card_ID[0], &t_EMoney.card_id[0], sizeof(wkReceipt.Electron_data.Ec.Card_ID) );
//				wkReceipt.Electron_data.Ec.e_pay_kind = EC_KOUTSUU_USED;
//				if (wkReceipt.Electron_data.Ec.pay_after == 0 &&
//					wkReceipt.Electron_data.Ec.Card_ID[0] == 0) {
//					// 決済後残高、カードIDが 0 である時
//					// みなし決済のフラグを立てる
//					wkReceipt.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;
//				}
//			} else {
//				wkReceipt.Electron_data.Suica.pay_ryo = t_EMoney.amount;			// 電子決済精算情報 決済額
//				wkReceipt.Electron_data.Suica.pay_after = t_EMoney.card_zangaku;	// 電子決済精算情報 決済後残高
//				wkReceipt.Electron_data.Suica.pay_befor = (t_EMoney.card_zangaku + t_EMoney.amount);
//				memcpy( &wkReceipt.Electron_data.Suica.Card_ID[0], &t_EMoney.card_id[0], sizeof(wkReceipt.Electron_data.Suica.Card_ID) );	// 電子決済精算情報ｶｰﾄﾞID (Ascii 16桁)
//				wkReceipt.Electron_data.Suica.e_pay_kind = SUICA_USED;				// 交通系はまとめてSuicaをセットしておく
//			}
//			break;
//		case 3:				// Edy
//		case 5:				// WAON
//		case 6:				// nanaco
//		case 7:				// SAPICA
//		case 12:			// iD
//		case 13:			// QUICPay
//			memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//			memcpy( (uchar *)&t_EMoney, RecvNtnetDt.RData16_01.CardPaymentInfo, sizeof(t_EMoney) );
//			// 暗号化方式=AES？
//			if( RecvNtnetDt.RData16_01.DataBasic.CMN_DT.DT_BASIC.encryptMode == 1 ){
//				AesCBCDecrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );
//			}
//			wkReceipt.Electron_data.Ec.pay_ryo = t_EMoney.amount;			// 電子決済精算情報 決済額
//			wkReceipt.Electron_data.Ec.pay_after = t_EMoney.card_zangaku;	// 電子決済精算情報 決済後残高
//			wkReceipt.Electron_data.Ec.pay_befor = (t_EMoney.card_zangaku + t_EMoney.amount);
//			memcpy( &wkReceipt.Electron_data.Ec.Card_ID[0], &t_EMoney.card_id[0], sizeof(wkReceipt.Electron_data.Ec.Card_ID) );	// 電子決済精算情報ｶｰﾄﾞID
//			switch(RecvNtnetDt.RData16_01.CardPaymentKind) {
//				case 3:				// Edy
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_EDY_USED;
//					break;
//				case 5:				// WAON
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_WAON_USED;
//					break;
//				case 6:				// nanaco
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_NANACO_USED;
//					break;
//				case 7:				// SAPICA
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_SAPICA_USED;
//					break;
//				case 12:			// iD
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_ID_USED;
//					break;
//				case 13:		// QUICPay
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_QUIC_PAY_USED;
//					break;
//			}
//			if (wkReceipt.Electron_data.Ec.pay_after == 0 &&
//				wkReceipt.Electron_data.Ec.Card_ID[0] == 0) {
//				// みなし決済のフラグを立てる
//				wkReceipt.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;
//			}
//			break;
//// MH321800(E) Y.Tanizaki ICクレジット対応
//	}
//	for( i = j = d = 0; i < (WTIK_USEMAX + DETAIL_SYU_MAX); i++ ){
//		if(( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != 0 ) &&					// 割引種別あり
//		   (( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet < NTNET_CSVS_M ) ||		// 精算中止割引情報でない
//		    ( NTNET_CFRE < RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet ))){
//// 仕様変更(S) K.Onodera 2016/11/07 領収証再発行要求フォーマット変更対応
////		    if( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_SUICA_1 &&		// SUICA以外(番号)
//			// 振替精算
//			if( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_FURIKAE_2 ||
//				RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_FURIKAE_DETAIL ||
//				RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_FURIKAE_DETAIL ||
//				RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_FUTURE || 
//				RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_AFTER_PAY ){
//				wkReceipt.DetailData[d].ParkingNo				 = RecvNtnetDt.RData16_01.stDiscount[i].ParkNo;
//				wkReceipt.DetailData[d].DiscSyu					 = RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet;
//				wkReceipt.DetailData[d].uDetail.Common.DiscNo	 = RecvNtnetDt.RData16_01.stDiscount[i].DiscType;
//				wkReceipt.DetailData[d].uDetail.Common.DiscCount = RecvNtnetDt.RData16_01.stDiscount[i].DiscCount;
//				wkReceipt.DetailData[d].uDetail.Common.Discount	 = RecvNtnetDt.RData16_01.stDiscount[i].DiscPrice;
//				wkReceipt.DetailData[d].uDetail.Common.DiscInfo1 = RecvNtnetDt.RData16_01.stDiscount[i].DiscInfo1;
//				wkReceipt.DetailData[d].uDetail.Common.DiscInfo2 = RecvNtnetDt.RData16_01.stDiscount[i].DiscInfo2;
//				d++;
//			}
//			else if( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_SUICA_1 &&		// SUICA以外(番号)
//// 仕様変更(E) K.Onodera 2016/11/07 領収証再発行要求フォーマット変更対応
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_SUICA_2 &&		//			(支払額・残額)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_PASMO_1 &&		// PASMO以外(番号)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_PASMO_2 &&		//			(支払額・残額)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_EDY_1 &&		// EDY以外	(番号)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_EDY_2 &&		//			(支払額・残額)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_ICOCA_1 &&		// ICOCA以外(番号)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_ICOCA_2 &&		//			(支払額・残額)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_ICCARD_1 &&	// IC-Card以外(番号)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_ICCARD_2 ) 	//			(支払額・残額)
//		    {
//				wkReceipt.DiscountData[j].ParkingNo	= RecvNtnetDt.RData16_01.stDiscount[i].ParkNo;			// 駐車場No.
//				wkReceipt.DiscountData[j].DiscSyu	= RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet;		// 割引種別
//				wkReceipt.DiscountData[j].DiscNo	= RecvNtnetDt.RData16_01.stDiscount[i].DiscType;		// 割引区分
//				wkReceipt.DiscountData[j].DiscCount	= RecvNtnetDt.RData16_01.stDiscount[i].DiscCount;		// 回収枚数
//				wkReceipt.DiscountData[j].Discount	= RecvNtnetDt.RData16_01.stDiscount[i].DiscPrice;		// 割引額
//				wkReceipt.DiscountData[j].DiscInfo1	= RecvNtnetDt.RData16_01.stDiscount[i].DiscInfo1;		// 割引情報1
//				if( wkReceipt.DiscountData[j].DiscSyu != NTNET_PRI_W ){
//					wkReceipt.DiscountData[j].uDiscData.common.DiscInfo2 = RecvNtnetDt.RData16_01.stDiscount[i].DiscInfo2;	// 割引情報2
//				}
//// 不具合修正(S) K.Onodera 2016/11/30 #1604 領収証を再発行で支払い前残額が「0円」で印字されてしまう
//		    	else{
//		    		// 使用前残額 = 使用金額 + 残額
//		    		wkReceipt.DiscountData[j].uDiscData.common.DiscInfo2 = (wkReceipt.DiscountData[j].Discount + wkReceipt.DiscountData[j].DiscInfo1);
//		    	}
//// 不具合修正(E) K.Onodera 2016/11/30 #1604 領収証を再発行で支払い前残額が「0円」で印字されてしまう
//				j++;
//		    }
//		}
//	}
//	// 精算？
//	if( !RecvNtnetDt.RData16_01.PayInfoKind ){
//// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
////		wkReceipt.WPlace	= (ulong)((RecvNtnetDt.RData16_01.Area * 10000L) + RecvNtnetDt.RData16_01.No);
//// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		wkReceipt.PayMode	= RecvNtnetDt.RData16_01.PayMode;													// 精算モード
//// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
//		wkReceipt.CMachineNo	= RecvNtnetDt.RData16_01.CMachineNo;											// 駐車券機械№(入庫機械№)
//// MH810100(E) K.Onodera  2020/02/05 車番チケットレス(LCD_IF対応)
//		for( i=0; i<6; i++ ){
//			if( RecvNtnetDt.RData16_01.InTime[i] ){
//				break;
//			}
//		}
//		// オール0でないこと
//		if( i != 6 ){
//			wkReceipt.TInTime.Year	= RecvNtnetDt.RData16_01.InTime[0] + 2000;	// 入庫年
//			wkReceipt.TInTime.Mon	= RecvNtnetDt.RData16_01.InTime[1];			// 入庫月
//			wkReceipt.TInTime.Day	= RecvNtnetDt.RData16_01.InTime[2];			// 入庫日
//			wkReceipt.TInTime.Hour	= RecvNtnetDt.RData16_01.InTime[3];			// 入庫時
//			wkReceipt.TInTime.Min	= RecvNtnetDt.RData16_01.InTime[4];			// 入庫分
//		}
//// MH810100(S) K.Onodera  2020/03/04 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		for( i=0; i<6; i++ ){
//			if( RecvNtnetDt.RData16_01.PreOutTime[i] ){
//				break;
//			}
//		}
//		// オール0でないこと
//		if( i != 6 ){
//			wkReceipt.BeforeTPayTime.Year	= RecvNtnetDt.RData16_01.PreOutTime[0] + 2000;	// 前回精算年
//			wkReceipt.BeforeTPayTime.Mon	= RecvNtnetDt.RData16_01.PreOutTime[1];			// 前回精算月
//			wkReceipt.BeforeTPayTime.Day	= RecvNtnetDt.RData16_01.PreOutTime[2];			// 前回精算日
//			wkReceipt.BeforeTPayTime.Hour	= RecvNtnetDt.RData16_01.PreOutTime[3];			// 前回精算時
//			wkReceipt.BeforeTPayTime.Min	= RecvNtnetDt.RData16_01.PreOutTime[4];			// 前回精算分
//		}
//// MH810100(E) K.Onodera  2020/03/04 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		for( i=0; i<6; i++ ){
//			if( RecvNtnetDt.RData16_01.OutTime[i] ){
//				break;
//			}
//		}
//		// オール0でないこと
//		if( i != 6 ){
//// MH810100(S) K.Onodera  2020/03/04 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:コメント修正)
////			wkReceipt.TOutTime.Year	= RecvNtnetDt.RData16_01.OutTime[0] + 2000;	// 前回精算年
////			wkReceipt.TOutTime.Mon	= RecvNtnetDt.RData16_01.OutTime[1];		// 前回精算月
////			wkReceipt.TOutTime.Day	= RecvNtnetDt.RData16_01.OutTime[2];		// 前回精算日
////			wkReceipt.TOutTime.Hour	= RecvNtnetDt.RData16_01.OutTime[3];		// 前回精算時
////			wkReceipt.TOutTime.Min	= RecvNtnetDt.RData16_01.OutTime[4];		// 前回精算分
//			wkReceipt.TOutTime.Year	= RecvNtnetDt.RData16_01.OutTime[0] + 2000;	// 精算年
//			wkReceipt.TOutTime.Mon	= RecvNtnetDt.RData16_01.OutTime[1];		// 精算月
//			wkReceipt.TOutTime.Day	= RecvNtnetDt.RData16_01.OutTime[2];		// 精算日
//			wkReceipt.TOutTime.Hour	= RecvNtnetDt.RData16_01.OutTime[3];		// 精算時
//			wkReceipt.TOutTime.Min	= RecvNtnetDt.RData16_01.OutTime[4];		// 精算分
//// MH810100(E) K.Onodera  2020/03/04 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:コメント修正)
//		}
//		wkReceipt.OutKind	= RecvNtnetDt.RData16_01.OutKind;				// 精算出庫
//		wkReceipt.CountSet	= RecvNtnetDt.RData16_01.CountSet;				// 在車カウント
//		wkReceipt.PayMethod	= RecvNtnetDt.RData16_01.PayMethod;				// 精算方法
//		wkReceipt.PassCheck	= RecvNtnetDt.RData16_01.Antipas;				// アンチパスチェック
//// 仕様変更(S) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
//		// 振替先精算？
//		if( RecvNtnetDt.RData16_01.PayMethod == 12 ){
//			wkReceipt.FRK_RetMod = RecvNtnetDt.RData16_01.CardFusokuType;
//			// 現金以外の不足額あり？
//			if( wkReceipt.FRK_RetMod ){
//				wkReceipt.FRK_Return = RecvNtnetDt.RData16_01.CardFusoku;
//			}else{
//				wkReceipt.FRK_Return = RecvNtnetDt.RData16_01.Change;
//				wkReceipt.WChgPrice  = 0;
//			}
//		}
//// 仕様変更(E) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
//	}
//	// 定期更新？
//	else{
//// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
////		wkReceipt.WPlace = 9999;
//// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		if( RecvNtnetDt.RData16_01.PassInfo.ParkNo ){
//			wkReceipt.teiki.id		= RecvNtnetDt.RData16_01.PassInfo.Id;					// 定期券ID
//			wkReceipt.teiki.syu		= RecvNtnetDt.RData16_01.PassInfo.Kind;					// 定期券種別
//			wkReceipt.teiki.status	= RecvNtnetDt.RData16_01.PassInfo.Status;				// 定期券ｽﾃｰﾀｽ
//			wkReceipt.teiki.s_year	= RecvNtnetDt.RData16_01.PassInfo.StartDate[0]+2000;	// 定期券開始年
//			wkReceipt.teiki.s_mon	= RecvNtnetDt.RData16_01.PassInfo.StartDate[1];			// 定期券開始月
//			wkReceipt.teiki.s_day	= RecvNtnetDt.RData16_01.PassInfo.StartDate[2];			// 定期券開始日
//			wkReceipt.teiki.e_year	= RecvNtnetDt.RData16_01.PassInfo.EndDate[0]+2000;		// 定期券終了年
//			wkReceipt.teiki.e_mon	= RecvNtnetDt.RData16_01.PassInfo.EndDate[1];			// 定期券終了月
//			wkReceipt.teiki.e_day	= RecvNtnetDt.RData16_01.PassInfo.EndDate[2];			// 定期券終了日
//			wkReceipt.WPrice		= RecvNtnetDt.RData16_01.PassInfo.UpdateFee;			// 定期券更新料金
//			wkReceipt.teiki.update_mon = RecvNtnetDt.RData16_01.PassInfo.UpdateTerm;		// 定期券更新期間(更新月数)
//		}
//	}
	// 精算？
	if( !RecvNtnetDt.RData16_01.PayInfoKind ){
		// 個別精算情報ログから検索する
		date[0] = RecvNtnetDt.RData16_01.OutTime[0] + 2000;
		date[1] = RecvNtnetDt.RData16_01.OutTime[1];
		date[2] = RecvNtnetDt.RData16_01.OutTime[2];
		target_date = dnrmlzm( date[0], date[1], date[2] );	// 対象の日付(ノーマライズ)
		cnt_ttl = Ope2_Log_CountGet_inDate( eLOG_PAYMENT, &date[0], &id);				// 個別精算情報ログから対象日のログ件数を取得
		for( ; cnt_ttl > 0 ; ) {
			if( 0 == Ope_Log_1DataGet(eLOG_PAYMENT, id, &wkReceipt )) {
				// ログ取得できなかったら終了
				cnt_ttl = 0;
				break;
			}
// MH810100(S) 2020/08/20 車番チケットレス(#4748 【未清算出庫の領収証を領収証再発行要求データ（ID:16）を実施したとき、領収書再発行応答データが「結果＝12」となり領収証の発行が実行されない)
//			seisan_date = dnrmlzm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day );
// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//			// 種別	(0=事前精算／1＝未精算出庫精算)
//			if(wkReceipt.shubetsu == 0){
			// 種別	(0=事前精算／1＝未精算出庫精算／2=遠隔精算(精算中変更))
			if(wkReceipt.shubetsu == 0 || wkReceipt.shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
				seisan_date = dnrmlzm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day );

			}else{
				seisan_date = dnrmlzm( wkReceipt.TUnpaidPayTime.Year, wkReceipt.TUnpaidPayTime.Mon, wkReceipt.TUnpaidPayTime.Day );

			}
// MH810100(e) 2020/08/20 車番チケットレス(#4748 【未清算出庫の領収証を領収証再発行要求データ（ID:16）を実施したとき、領収書再発行応答データが「結果＝12」となり領収証の発行が実行されない)
			if( target_date != seisan_date ) {
				// 取得したログの日付が違っていたら終了
				cnt_ttl = 0;
				break;
			}
// MH810100(S) 2020/08/18 車番チケットレス(#4599 ParkiProの領収証再発行でクレジット精算の領収証が再発行ができない)
//			if(wkReceipt.TOutTime.Hour == RecvNtnetDt.RData16_01.OutTime[3]
//				&& wkReceipt.TOutTime.Min == RecvNtnetDt.RData16_01.OutTime[4] ) {
//				if( wkReceipt.PayClass == RecvNtnetDt.RData16_01.PayClass
			PayClassTmp = wkReceipt.PayClass;										// 処理区分
			if( PayClassTmp == 4 || PayClassTmp == 5){
				// クレジット精算・クレジット再精算の場合は、0・1に変換する（NT-NETでは4，5は来ない）
				PayClassTmp = PayClassTmp - 4;
			}
// MH810100(S) 2020/08/20 車番チケットレス(#4748 【未清算出庫の領収証を領収証再発行要求データ（ID:16）を実施したとき、領収書再発行応答データが「結果＝12」となり領収証の発行が実行されない)
//			if(wkReceipt.TOutTime.Hour == RecvNtnetDt.RData16_01.OutTime[3]
//				&& wkReceipt.TOutTime.Min == RecvNtnetDt.RData16_01.OutTime[4] ) {
//				if( PayClassTmp == RecvNtnetDt.RData16_01.PayClass
//// MH810100(E) 2020/08/18 車番チケットレス(#4599 ParkiProの領収証再発行でクレジット精算の領収証が再発行ができない)
//					&& CountSel( &wkReceipt.Oiban ) == RecvNtnetDt.RData16_01.PayCount ) {
//					// 精算日時、処理区分、精算追番が一致していたら見つかったことにする
//					break;
//				}
//			}
// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//			// 種別	(0=事前精算／1＝未精算出庫精算)
//			if(wkReceipt.shubetsu == 0){
			// 種別	(0=事前精算／1＝未精算出庫精算／2=遠隔精算(精算中変更))
			if(wkReceipt.shubetsu == 0 || wkReceipt.shubetsu == 0){
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
				if(wkReceipt.TOutTime.Hour == RecvNtnetDt.RData16_01.OutTime[3]
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//					&& wkReceipt.TOutTime.Min == RecvNtnetDt.RData16_01.OutTime[4] ) {
					&& wkReceipt.TOutTime.Min == RecvNtnetDt.RData16_01.OutTime[4]
					&& wkReceipt.TOutTime_Sec == RecvNtnetDt.RData16_01.OutTime[5] ) {
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
					if( PayClassTmp == RecvNtnetDt.RData16_01.PayClass
						&& CountSel( &wkReceipt.Oiban ) == RecvNtnetDt.RData16_01.PayCount ) {
						// 精算日時、処理区分、精算追番が一致していたら見つかったことにする
						break;
					}
				}
			}else{
				if(wkReceipt.TUnpaidPayTime.Hour == RecvNtnetDt.RData16_01.OutTime[3]
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//					&& wkReceipt.TUnpaidPayTime.Min == RecvNtnetDt.RData16_01.OutTime[4] ) {
					&& wkReceipt.TUnpaidPayTime.Min == RecvNtnetDt.RData16_01.OutTime[4]
					&& wkReceipt.TUnpaidPayTime_Sec == RecvNtnetDt.RData16_01.OutTime[5] ) {
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
					if( PayClassTmp == RecvNtnetDt.RData16_01.PayClass
						&& CountSel( &wkReceipt.Oiban ) == RecvNtnetDt.RData16_01.PayCount ) {
						// 精算日時、処理区分、精算追番が一致していたら見つかったことにする
						break;
					}
				}

			}
// MH810100(E) 2020/08/20 車番チケットレス(#4748 【未清算出庫の領収証を領収証再発行要求データ（ID:16）を実施したとき、領収書再発行応答データが「結果＝12」となり領収証の発行が実行されない)
			id++;
			cnt_ttl--;
		}
	}
	// 定期更新？
	else{
		// 定期更新は未対応
		cnt_ttl = 0;
	}
// MH810100(E) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）

// MH810105(S) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
//// MH810105(S) MH364301 インボイス対応
//	wkReceipt.WTaxRate = Disp_Tax_Value( &wkReceipt.TOutTime );								// 適用税率
//// MH810105(E) MH364301 インボイス対応
// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//	// 種別	(0=事前精算／1＝未精算出庫精算)
//	if(wkReceipt.shubetsu == 0){
	// 種別	(0=事前精算／1＝未精算出庫精算／2=遠隔精算(精算中変更))
	if(wkReceipt.shubetsu == 0 || wkReceipt.shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
		wkReceipt.WTaxRate = Disp_Tax_Value( &wkReceipt.TOutTime );								// 適用税率
	}else{
		wkReceipt.WTaxRate = Disp_Tax_Value( &wkReceipt.TUnpaidPayTime );						// 適用税率
	}
// MH810105(E) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
// GG129001(S) データ保管サービス対応（課税対象額をセットする）
	wkReceipt.WBillAmount = Billingcalculation(&wkReceipt);
	if (wkReceipt.chuusi == 0) {
		wkReceipt.WTaxPrice = TaxAmountcalculation(&wkReceipt, 0);
	}
	else {
		wkReceipt.WTaxPrice = TaxAmountcalculation(&wkReceipt, 1);
	}
// GG129001(E) データ保管サービス対応（課税対象額をセットする）

	// 状態OK？(待機or車室番号入力中且つ、自動集計中でないこと)
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//// MH810100(S) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
////	if( OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod <= 1) && auto_syu_prn != 2 ){
//  if( OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod <= 1) && auto_syu_prn != 2 && cnt_ttl > 0){
//// MH810100(E) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
	if( OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod <= 1) && auto_syu_prn != 2 && cnt_ttl > 0 &&
		Ope_isPrinterReady() != 0 ){
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
	// 領収証再発行
// GG129002(S) ゲート式車番チケットレスシステム対応（新プリンタ印字処理不具合修正）
		memset(&rec_data, 0, sizeof(rec_data));
// GG129002(E) ゲート式車番チケットレスシステム対応（新プリンタ印字処理不具合修正）
		rec_data.prn_kind = R_PRI;						// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
		rec_data.prn_data = &wkReceipt;					// 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ
		rec_data.kakari_no = 99;						// 係員No.99（固定）
		rec_data.reprint = ON;							// 再発行ﾌﾗｸﾞｾｯﾄ（再発行）
		memcpy( &rec_data.PriTime, &CLK_REC, sizeof(date_time_rec) );
														// 再発行日時（現在日時）ｾｯﾄ
// MH810105(S) MH364301 インボイス対応
//		queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
//		wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );		// 操作履歴登録
//		LedReq( CN_TRAYLED, LED_ON );					// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON
//		LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// 電子決済中止領収証発行時の取り出し口LED消灯ﾀｲﾏｰ
		if (IS_INVOICE) {
			rec_data.prn_kind = J_PRI;					// ジャーナル印字が先
			memcpy(&Cancel_pri_work, &wkReceipt, sizeof(Cancel_pri_work));
			queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
			OPECTL.f_ReIsuType = 1;
		}
		else {
			queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
			wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );	// 操作履歴登録
			LedReq( CN_TRAYLED, LED_ON );				// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON
			LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// 電子決済中止領収証発行時の取り出し口LED消灯ﾀｲﾏｰ
		}
// MH810105(E) MH364301 インボイス対応
		rslt = PIP_RES_RESULT_OK;
	}
// MH810100(S) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
	else if ( cnt_ttl == 0 ){
		// 個別精算情報ログから見つからなかった場合はパラメータ不良とする
		rslt = PIP_RES_RESULT_NG_PARAM;					// パラメータ不良
	}
// MH810100(E) S.Fujii 2020/07/30 #4542 【連動評価指摘事項】AI-Vからの領収証再発行を変更する（検証課指摘 No17）
	else {
		rslt = PIP_RES_RESULT_NG_DENY;					// 受付不可状態
	}

	// 1度上書き
// 不具合修正(S) K.Onodera 2016/09/27 #1504 領収証再発行応答データで「精算追番」以降の内容が２バイトずれている
//		memcpy( &SendNtnetDt.SData16_02, &RecvNtnetDt.RData16_01, sizeof(DATA_KIND_16_02) );
	size =  (ushort)_offsetof(DATA_KIND_16_02, PayCount);
	size = ( sizeof(DATA_KIND_16_02) - size );
	memcpy( &SendNtnetDt.SData16_02.PayCount, &RecvNtnetDt.RData16_01.PayCount, size );
// 不具合修正(E) K.Onodera 2016/09/27 #1504 領収証再発行応答データで「精算追番」以降の内容が２バイトずれている

	// 応答データ送信
	BasicDataMake( PIP_RES_KIND_RECEIPT_AGAIN, 1 );									// 基本ﾃﾞｰﾀ作成 ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定
// MH310100(S) S.Fujii 2020/09/07 車番チケットレス(#4596 NT-NET精算情報データのシーケンシャルNoが変化しない)
	SendNtnetDt.SData16_02.DataBasic.SeqNo = GetNtDataSeqNo();
// MH310100(E) S.Fujii 2020/09/07 車番チケットレス(#4596 NT-NET精算情報データのシーケンシャルNoが変化しない)
	
// MH810105(S) MH364301 領収証再発行要求データRev11受信
//	SendNtnetDt.SData16_02.Common.FmtRev 		= 10;											// フォーマットRev.No.(10固定)
	SendNtnetDt.SData16_02.Common.FmtRev 		= RecvNtnetDt.RData16_01.Common.FmtRev;			// フォーマットRev.No.(受信したRevを返す)
// MH810105(E) MH364301 領収証再発行要求データRev11受信
	SendNtnetDt.SData16_02.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// 送信先の端末ターミナル№をセットする（99＝遠隔PC）
	SendNtnetDt.SData16_02.Common.CenterSeqNo	= RecvNtnetDt.RData16_01.Common.CenterSeqNo;	// センター追番
	SendNtnetDt.SData16_02.Result				= rslt;								// 結果
	SendNtnetDt.SData16_02.Type					= RecvNtnetDt.RData16_01.PayInfoKind;// 精算情報種別
	// カード決済区分が0でない場合は暗号化する
	if( RecvNtnetDt.RData16_01.CardPaymentKind ){
		SendNtnetDt.SData16_02.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// 暗号化方式
		SendNtnetDt.SData16_02.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// 暗号鍵番号
	}
	if( _is_ntnet_remote() ){
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_02 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_02 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		発生中エラーアラーム要求受信
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/09/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_10_ErrorAlarm( void )
{
	ushort	rslt = PIP_RES_RESULT_OK;
	ushort	cnt = 0;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_11 ) );
	BasicDataMake( PIP_RES_KIND_OCCUR_ERRALM, 1 );									// 基本ﾃﾞｰﾀ作成 ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定

	// エラー要求？
	if( RecvNtnetDt.RData16_10.ReqFlg == 0 ){
		cnt = PIP_GetOccurErr( &SendNtnetDt.SData16_11.stErrAlm[0] );
	}
	// アラーム要求？
	else if( RecvNtnetDt.RData16_10.ReqFlg == 1 ){
		cnt = PIP_GetOccurAlarm( &SendNtnetDt.SData16_11.stErrAlm[0] );
	}
	// その他
	else{
		rslt = PIP_RES_RESULT_NG_PARAM;
	}

	// 共通部分セット＆送信
	SendNtnetDt.SData16_11.Common.FmtRev 		= 10;											// フォーマットRev.No.(10固定)
	SendNtnetDt.SData16_11.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;						// 送信先の端末ターミナル№をセットする（99＝遠隔PC）
	SendNtnetDt.SData16_11.Common.CenterSeqNo	= RecvNtnetDt.RData16_10.Common.CenterSeqNo;	// センター追番
	SendNtnetDt.SData16_11.Result				= rslt;											// 結果
	SendNtnetDt.SData16_11.Kind					= RecvNtnetDt.RData16_10.ReqFlg;				// 応答エラーアラームフラグ（0=エラー／1=アラーム）
	SendNtnetDt.SData16_11.Count				= cnt;											// 発生中エラーアラーム総数
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_11 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_11 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
}

///*[]----------------------------------------------------------------------[]*
// *| 発生中エラー情報応答
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData182_OccurErr
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static ushort PIP_GetOccurErr( PIP_ErrAlarm *tbl )
{
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	extern 	ushort	LKcom_RoomNoToType( ulong roomNo );
//	static const uchar	ucBit[] = { 0x01, 0x02, 0x04, 0x08 };
//	ushort		Index = 0, i = 0, j = 0, errcod, sno, smax;
//	ulong		errinfo;
//	uchar		ucErrWk;
	ushort		Index = 0, i = 0, j = 0;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	// 発生中のエラーを詰める（フラップとロックは除く）
	for( i = 0; i < ERR_MOD_MAX; i++ ){
		WACDOG;
		for( j=1; j<ERR_NUM_MAX; j++ ){
// 不具合修正(S) K.Onodera 2016/10/13 #1505 アラーム01-61(5000円札釣り切れ)発生を含むデータが応答されてしまう
//			if( ERR_CHK[i][j] ){
			if( ERR_CHK[i][j] && isDefToErrAlmTbl(0, i, j) ){
// 不具合修正(E) K.Onodera 2016/10/13 #1505 アラーム01-61(5000円札釣り切れ)発生を含むデータが応答されてしまう
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//				if( ( i == mod_flapcrb ) &&
//					( j == ERR_FLAPLOCK_LOCKCLOSEFAIL || j == ERR_FLAPLOCK_LOCKOPENFAIL ||	// 開/閉 or 上昇/下降エラー
//		 			  j == ERR_FLAPLOCK_DOWNRETRYOVER || j == ERR_FLAPLOCK_DOWNRETRY ) ) {	// 保護処理リトライオーバー/保護処理開始
//					// エラー種別：１６（通信タイプ：フラップ／ロック装置）
//					continue;
//				}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
				// セット可能な最大数を超えたらエラー数のみカウントする
				if( Index < PIP_OCCUR_ERROR_ALARM_MAX ){
					tbl[Index].ucKind	= ErrAct_index[i];				// 種別
					tbl[Index].ucCoce	= j;							// コード
					tbl[Index].ucLevel	= (uchar)getErrLevel(i,j);		// レベル
					if( OPE_SIG_DOOR_Is_OPEN == 0 ){					// ドア状態
						tbl[Index].ucDoor = 0;
					}else{
						tbl[Index].ucDoor = 1;
					}
					memcpy( &tbl[Index].ucInfo[6], &ERR_INFO[i][j], 4 );	// エラー情報
				}
				Index++;
			}
		}
	}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	// エラー種別：１６（通信タイプ：フラップ／ロック装置）上昇／下降ロックの検索
//	switch( GetCarInfoParam() ){	// 設定03-0100参照　車、バイク
//		case 0x01: // ロックのみ
//			sno = BIKE_START_INDEX;						// 100～
//			smax = BIKE_START_INDEX + BIKE_LOCK_MAX;	// 100+50
//			break;
//		case 0x04: // 車のみ
//			sno = INT_CAR_START_INDEX;						// 50～
//			smax = INT_CAR_START_INDEX + INT_CAR_LOCK_MAX;	// 50+20
//			break;
//		case 0x05: // 両方
//		default:
//			sno = INT_CAR_START_INDEX;					// 50～
//			smax = BIKE_START_INDEX + BIKE_LOCK_MAX;	// 100+50
//			break;
//	}
//
//	for( ; sno < smax; sno++ ){
//
//		ucErrWk = flp_err_search( sno, &errinfo );
//		// エラー車室あり？
//		if( ucErrWk ){
//			// エラーの車室はロック装置？
//			if( 0 == LKcom_RoomNoToType(errinfo) ){
//				// 18-0019⑥=1　上昇/下降及び開/閉ロック印字しない？
//				if( prm_get( COM_PRM, S_PRN, 19, 1, 1 ) == 0 ){
//					continue;
//				}
//			}
//			// エラーの車種はフラップ？
//			else{
//				// 18-0019④=1　上昇/下降及び開/閉ロック印字しない？
//				if( prm_get( COM_PRM, S_PRN, 19, 1, 3 ) == 0 ){
//					continue;
//				}
//			}
//			for( i=0; i<4; i++ ){
//				if( ucErrWk & ucBit[i] ){
//					switch( ucBit[i] ){
//						case 0x01:
//							errcod = ERR_FLAPLOCK_LOCKCLOSEFAIL;			// E1638:上昇ロック発生中
//							break;
//						case 0x02:
//							errcod = ERR_FLAPLOCK_LOCKOPENFAIL;				// E1639:下昇ロック発生中
//							break;
//						case 0x04:
//							errcod = ERR_FLAPLOCK_DOWNRETRYOVER;			// E1640:リトライオーバーエラー
//							break;
//						case 0x08:
//							errcod = ERR_FLAPLOCK_DOWNRETRY;				// E1641:リトライ動作開始エラー
//							break;
//						default:
//							continue;
//							break;
//					}
//					if( isDefToErrAlmTbl(0, (uchar)mod_flapcrb, (uchar)errcod) ){
//						if( Index < PIP_OCCUR_ERROR_ALARM_MAX ){
//							tbl[Index].ucKind	= ErrAct_index[mod_flapcrb];				// 種別
//							tbl[Index].ucCoce	= errcod;									// コード
//							tbl[Index].ucLevel	= (uchar)getErrLevel(mod_flapcrb,errcod);	// レベル
//							if( OPE_SIG_DOOR_Is_OPEN == 0 ){								// ドア状態
//								tbl[Index].ucDoor = 0;
//							}else{
//								tbl[Index].ucDoor = 1;
//							}
//							memcpy( &tbl[Index].ucInfo[6], &ERR_LOCK_INFO[sno][i], 4 );	// エラー情報
//						}
//						Index++;
//					}
//				}
//			}
//		}
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	return Index;
}

///*[]----------------------------------------------------------------------[]*
// *| 発生中アラーム情報応答
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData182_OccurAlarm
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static ushort PIP_GetOccurAlarm( PIP_ErrAlarm *tbl )
{
	ushort		Index = 0;
	ushort		i = 0, j = 0;
	ushort	count = 0, Total = 0;

	// 発生中ｱﾗｰﾑの数を数える
	for( i = 0; i < ALM_MOD_MAX; i++ ){
		WACDOG;
		for( j = 1; j < ALM_NUM_MAX; j++ ){
// 不具合修正(S) K.Onodera 2016/10/13 #1505 アラーム01-61(5000円札釣り切れ)発生を含むデータが応答されてしまう
//			if( ALM_CHK[i][j] ){
			if( ALM_CHK[i][j] && isDefToErrAlmTbl(1, i, j) ){
// 不具合修正(E) K.Onodera 2016/10/13 #1505 アラーム01-61(5000円札釣り切れ)発生を含むデータが応答されてしまう
				count++;
			}
		}
	}
	Total = count;		// 発生数取得
	if( count > PIP_OCCUR_ERROR_ALARM_MAX ){
		count = PIP_OCCUR_ERROR_ALARM_MAX;
	}
	if( count != 0 ){
		for( i = 0; i < ALM_MOD_MAX; i++ ){
			WACDOG;
			for( j = 1; j < ALM_NUM_MAX; j++ ){
// 不具合修正(S) K.Onodera 2016/10/13 #1505 アラーム01-61(5000円札釣り切れ)発生を含むデータが応答されてしまう
//				if( ALM_CHK[i][j] ){
				if( ALM_CHK[i][j] && isDefToErrAlmTbl(1, i, j) ){
// 不具合修正(E) K.Onodera 2016/10/13 #1505 アラーム01-61(5000円札釣り切れ)発生を含むデータが応答されてしまう
					tbl[Index].ucKind = i;							// 種別
					tbl[Index].ucCoce = j;							// コード
					tbl[Index].ucLevel = (uchar)getAlmLevel(i,j);	// レベル
					if( OPE_SIG_DOOR_Is_OPEN == 0 ){				// ドア状態
						tbl[Index].ucDoor = 0;
					}else{
						tbl[Index].ucDoor = 1;
					}
					memcpy( &tbl[Index].ucInfo[6], &ALM_INFO[i][j], 4 );	// エラー情報
					count--;
					Index++;
				}
			}
		}
	}

	return Total;
}

//[]----------------------------------------------------------------------[]
///	@brief		振替精算要求受信
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/18<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
//static void NTNET_RevData16_03_Furikae( void )
void NTNET_RevData16_03_Furikae( void )
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
{
	switch( RecvNtnetDt.RData16_03.ReqKind ){
		// 振替対象情報
		case PIP_FURIKAE_TYPE_INFO:
			NTNET_RevData16_03_Furikae_Info();
			break;
		// 振替精算実行可能チェック
		case PIP_FURIKAE_TYPE_CHK:
			NTNET_RevData16_03_Furikae_Check();
			break;
		// 振替精算
		case PIP_FURIKAE_TYPE_GO:
			NTNET_RevData16_03_Furikae_Go();
			break;
		default:
			// NG応答(パラメータ不良)
			NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_PARAM );
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		振替精算初期化処理
//[]----------------------------------------------------------------------[]
void PiP_FurikaeInit( void )
{
	PiP_FurikaeSts = PIP_FRK_STS_IDLE;
}
//[]----------------------------------------------------------------------[]
///	@brief		振替精算開始処理
//[]----------------------------------------------------------------------[]
void PiP_FurikaeStart( void )
{
	PiP_FurikaeSts = PIP_FRK_STS_ACP;
}
//[]----------------------------------------------------------------------[]
///	@brief		振替精算開始処理
//[]----------------------------------------------------------------------[]
ushort PiP_GetFurikaeSts( void )
{
	return PiP_FurikaeSts;
}
//[]----------------------------------------------------------------------[]
///	@brief		振替精算終了処理
//[]----------------------------------------------------------------------[]
void PiP_FurikaeEnd( void )
{
	PiP_FurikaeSts = PIP_FRK_STS_IDLE;
}

//[]----------------------------------------------------------------------[]
///	@brief		振替精算終了処理
//[]----------------------------------------------------------------------[]
Receipt_data* GetFurikaeSrcReciptData( void )
{
	return &wkReceipt;
}
//[]----------------------------------------------------------------------[]
///	@brief		振替チェック処理
//[]----------------------------------------------------------------------[]
static ushort PiP_CheckFurikae( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ushort			index, pos1, pos2;
	uchar			sts = 0;

	// 各種チェック
	do{
		// １．状態チェック(振替中でないこと)
		if( PiP_FurikaeSts == PIP_FRK_STS_IDLE ){
			memset( &g_PipCtrl.stFurikaeInfo, 0, sizeof(g_PipCtrl.stFurikaeInfo) );
		}else{
			rslt = PIP_RES_RESULT_NG_DENY;
			break;
		}

		// ２．状態チェック(待機画面であること)
		sts = (get_crm_state() - 0x30);			// 精算処理状態
		// 車室番号入力中？
		if( OPECTL.Ope_mod == 1 && OPECTL.Mnt_mod == 0 ){
			sts = 0;
		}
		// 振替対象情報要求の場合、応答用に保持
		if( RecvNtnetDt.RData16_03.ReqKind == PIP_FURIKAE_TYPE_INFO ){
			g_PipCtrl.stFurikaeInfo.PayState = (ushort)sts;
		}
		// 待機でない？
		if( sts ){
			rslt = PIP_RES_RESULT_NG_DENY;
			break;
		}

		// ３.自動集計中チェック
		if( auto_syu_prn == 2 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// 受付不可状態
			break;
		}

		// ４．クレジットHOST接続中？
		if( OPECTL.InquiryFlg ){
			rslt = PIP_RES_RESULT_NG_BUSY;
			break;
		}

		// ５．振替元車室情報チェック
		rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_03.SrcArea,RecvNtnetDt.RData16_03.SrcNo, &pos1 );	// 車室情報取得
		if( rslt == PIP_RES_RESULT_OK ){
			index = pos1 - 1;
			// 実行可能チェックでなければ応答用に保持
			if( RecvNtnetDt.RData16_03.ReqKind != PIP_FURIKAE_TYPE_CHK ){
				g_PipCtrl.stFurikaeInfo.SrcStatus = FLAPDT.flp_data[index].nstat.word;
			}
		}else{
			break;
		}

		// ６．フラップ状態
		if( FLAPDT.flp_data[index].mode < FLAP_CTRL_MODE5 || FLAPDT.flp_data[index].mode > FLAP_CTRL_MODE6 ){
			rslt = PIP_RES_RESULT_NG_FURIKAE_SRC;	// 振替元条件エラー
			break;
		}

		// ７．振替先車室情報チェック
		rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_03.DestArea,RecvNtnetDt.RData16_03.DestNo, &pos2 );	// 車室情報取得
		if( rslt == PIP_RES_RESULT_OK ){
			index = pos2 - 1;
			// 実行可能チェックでなければ応答用に保持
			if( RecvNtnetDt.RData16_03.ReqKind != PIP_FURIKAE_TYPE_CHK ){
				g_PipCtrl.stFurikaeInfo.DestStatus = FLAPDT.flp_data[index].nstat.word;
			}
		}else{
			break;
		}

		// ８．フラップ状態
		if( FLAPDT.flp_data[index].mode < FLAP_CTRL_MODE2 || FLAPDT.flp_data[index].mode > FLAP_CTRL_MODE4 ){
			rslt = PIP_RES_RESULT_NG_FURIKAE_DST;	// 振替先条件エラー
			break;
		}

// レビュー指摘対応(S) K.Onodera 2016/10/04 不要な判定を削除
//		// ９．振替元と振替先が一致？
//		if( pos1 == pos2 ){
//			rslt = PIP_RES_RESULT_NG_PARAM;
//			break;
//		}
// レビュー指摘対応(E) K.Onodera 2016/10/04 不要な判定を削除

	}while(0);

	return rslt;
}

//[]----------------------------------------------------------------------[]
///	@brief		車室№を元に精算データ検索 ※最新から
//[]----------------------------------------------------------------------[]
static uchar PiP_GetNewestPayReceiptData( Receipt_data *rcp, ushort area, ushort no )
{
	uchar			chk = 0;	// 一致車室なし
	ushort			index = 0, index_ttl = 0;

	// 精算データ検索
	index_ttl = Ope_SaleLog_TotalCountGet( PAY_LOG_CMP );
	index = index_ttl;
	while( index != 0 ){
		index--;
		Ope_SaleLog_1DataGet( index, PAY_LOG_CMP, index_ttl, rcp );
		// 車室一致？
		if( rcp->WPlace == ( area * 10000L ) + no ){
			chk = 1;
			break;
		}
	}

	return chk;
}

//[]----------------------------------------------------------------------[]
///	@brief		振替対象情報要求
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/18<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_03_Furikae_Info( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ushort			i;
	ulong			CompTime1 = 0, CompTime2 = 0;

	// 振替前チェック処理
	rslt = PiP_CheckFurikae();

	// チェックOK？
	if( rslt == PIP_RES_RESULT_OK ){

		// 精算データ取得
		memset( &wkReceipt, 0, sizeof( Receipt_data) );
		if( 0 == PiP_GetNewestPayReceiptData( &wkReceipt, RecvNtnetDt.RData16_03.SrcArea, RecvNtnetDt.RData16_03.SrcNo ) ){
			rslt = PIP_RES_RESULT_NG_NO_CAR;
			// NG応答
			NTNET_Snd_Data16_04( rslt );
			return;
		}
		// 振替元精算時刻ノーマライズ
		CompTime1 = enc_nmlz_mdhm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day,
																wkReceipt.TOutTime.Hour, wkReceipt.TOutTime.Min );
		// 前回集計時刻ノーマライズ
		CompTime2 = enc_nmlz_mdhm( sky.tsyuk.OldTime.Year, sky.tsyuk.OldTime.Mon, sky.tsyuk.OldTime.Day,
																sky.tsyuk.OldTime.Hour, sky.tsyuk.OldTime.Min );
		// 集計を跨いでいる？
		if( CompTime1 <= CompTime2 ){
			// NG応答
			rslt = PIP_RES_RESULT_NG_DENY;
			NTNET_Snd_Data16_04( rslt );
			return;
		}

		// 応答用データセット =============
		// 機器情報
		g_PipCtrl.stFurikaeInfo.OpenClose	= (uchar)(opncls()-1);				// 営休業状況
		g_PipCtrl.stFurikaeInfo.ErrOccur	= Err_onf;							// エラー発生状況
		g_PipCtrl.stFurikaeInfo.AlmOccur	= Alm_onf;							// アラーム発生状況
		if( OPECTL.Mnt_mod ){
			g_PipCtrl.stFurikaeInfo.MntMode	= 1; 								// 精算機動作モード状況
		}
		// 振替元
		PiP_GetFurikaeInfo( &wkReceipt );										// 振替元情報を取得
		g_PipCtrl.stFurikaeInfo.PassUse		= vl_frs.antipassoff_req;			// 振替元定期利用有無
		g_PipCtrl.stFurikaeInfo.SrcArea		= RecvNtnetDt.RData16_03.SrcArea;	// 区画
		g_PipCtrl.stFurikaeInfo.SrcNo		= RecvNtnetDt.RData16_03.SrcNo;		// 車室
		g_PipCtrl.stFurikaeInfo.SrcFeeKind	= wkReceipt.syu;					// 料金種別
		g_PipCtrl.stFurikaeInfo.SrcFee		= wkReceipt.WPrice;					// 駐車料金
		for( i=0; i<WTIK_USEMAX; i++ ){
// 不具合修正(S) K.Onodera 2016/11/24 #1585 振替元割引金額に無関係の値がセットされる
//			g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DiscountData[i].Discount;	// 割引額加算
			if( ( wkReceipt.DiscountData[i].DiscSyu != 0 ) &&					// 割引種別あり
			    (( wkReceipt.DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// 精算中止割引情報でない
			    ( NTNET_CFRE < wkReceipt.DiscountData[i].DiscSyu )) ){
			    if( wkReceipt.DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA以外(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO以外(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_EDY_1 &&			// EDY以外	(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_EDY_2 &&			//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA以外(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card以外(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICCARD_2 &&		//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu < NTNET_SECTION_WARI_MAX) // 割引情報として使用するのは種別001～999まで
			    {
			    	g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DiscountData[i].Discount;	// 割引額加算
			    }
			}
// 不具合修正(E) K.Onodera 2016/11/24 #1585 振替元割引金額に無関係の値がセットされる
		}
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
		for( i=0; i<DETAIL_SYU_MAX; i++ ){
			if( wkReceipt.DiscountData[i].DiscSyu == NTNET_FURIKAE_2 ){
				g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DetailData[i].uDetail.Furikae.Total;
			}
		}
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
		memcpy( &g_PipCtrl.stFurikaeInfo.SrcPayTime, &wkReceipt.TOutTime, sizeof(date_time_rec) );	// 精算時刻 ※分まで
		memcpy( &g_PipCtrl.stFurikaeInfo.SrcInTime,  &wkReceipt.TInTime, sizeof(date_time_rec) );		// 入庫時刻 ※分まで
		// 振替先
		g_PipCtrl.stFurikaeInfo.DestArea	= RecvNtnetDt.RData16_03.DestArea;	// 区画
		g_PipCtrl.stFurikaeInfo.DestNo		= RecvNtnetDt.RData16_03.DestNo;	// 車室
		// 上記以外の振替先情報は料金計算が必要なため、OPE側でセット

		// 料金計算要求
		queset( OPETCBNO, OPE_REQ_FURIKAE_TARGET_INFO, 0, NULL );

	}
	else{
		// NG応答
		NTNET_Snd_Data16_04( rslt );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		振替元情報取得
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
static void PiP_GetFurikaeInfo( Receipt_data* receipt )
{
	ushort	i;

	memset(&vl_frs, 0, sizeof(vl_frs));

	PIP_CheckCarInfo( RecvNtnetDt.RData16_03.SrcArea,RecvNtnetDt.RData16_03.SrcNo, &vl_frs.lockno );

	// 定期使用あり？
	if( RecvNtnetDt.RData16_03.SrcPassUse ){
		vl_frs.antipassoff_req = (receipt->teiki.id)? 1 : 0;	// 振替元で定期使用
	}

	vl_frs.price = receipt->WPrice /* + receipt.Wtax */;		// 内税ならばWtax==0：外税対応しない

	// 以下の項目は０で初期化されているので、無条件に積算可
	vl_frs.in_price = vl_frs.genkin_furikaegaku = receipt->WInPrice - receipt->WChgPrice;	// 現金
	for (i = 0; i < WTIK_USEMAX; i++) {
		if ((receipt->DiscountData[i].DiscSyu == NTNET_FRE ) ||	// 回数券
		    (receipt->DiscountData[i].DiscSyu == NTNET_PRI_W )) {// プリペイドカード
			vl_frs.in_price += receipt->DiscountData[i].Discount;	// 割引額加算
		}
	}
	vl_frs.in_price += receipt->ppc_chusi;						// プリペイド／回数券中止データ
	vl_frs.in_price += receipt->credit.pay_ryo;					// クレジット
	vl_frs.in_price += receipt->Electron_data.Suica.pay_ryo;		// 電子マネー
// 仕様変更(S) K.Onodera 2016/10/28 領収証フォーマット
	// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
// MH321800(S) T.Nagai ICクレジット対応
//	if( receipt->Electron_data.Suica.e_pay_kind != 0 ){	// 電子決済種別 Suica:1, Edy:2
	if( EcUseKindCheck(receipt->Electron_data.Ec.e_pay_kind) ){
		switch(receipt->Electron_data.Ec.e_pay_kind) {
		case EC_EDY_USED:
			vl_frs.card_type = (ushort)3;
			break;
		case EC_NANACO_USED:
			vl_frs.card_type = (ushort)6;
			break;
		case EC_WAON_USED:
			vl_frs.card_type = (ushort)5;
			break;
		case EC_SAPICA_USED:
			vl_frs.card_type = (ushort)7;
			break;
		case EC_KOUTSUU_USED:
			vl_frs.card_type = (ushort)2;
			break;
		case EC_ID_USED:
			vl_frs.card_type = (ushort)8;
			break;
		case EC_QUIC_PAY_USED:
			vl_frs.card_type = (ushort)9;
			break;
		default:
			break;		// ありえない
		}
		vl_frs.card_furikaegaku = receipt->Electron_data.Ec.pay_ryo;
	}else if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind )){
// MH321800(E) T.Nagai ICクレジット対応
		vl_frs.card_type = (ushort)2;
		vl_frs.card_furikaegaku = receipt->Electron_data.Suica.pay_ryo;
	}else if( receipt->PayClass == 4 ){					// クレジット精算
		vl_frs.card_type = (ushort)1;
		vl_frs.card_furikaegaku = receipt->credit.pay_ryo;
	}
// 仕様変更(E) K.Onodera 2016/10/28 領収証フォーマット
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
	for( i=0; i<DETAIL_SYU_MAX; i++ ){
		if( receipt->DetailData[i].DiscSyu == NTNET_FURIKAE_2 ){
			vl_frs.in_price += (receipt->DetailData[i].uDetail.Furikae.Total - receipt->FRK_Return );
		}
		if( receipt->DetailData[i].DiscSyu == NTNET_FURIKAE_DETAIL ){
			// 現金？
			if( !receipt->DetailData[i].uDetail.FurikaeDetail.Mod ){
				vl_frs.genkin_furikaegaku += (receipt->DetailData[i].uDetail.FurikaeDetail.FrkMoney - receipt->FRK_Return);
			}
			// クレジットor電子マネー？
			else{
				vl_frs.card_type		   = receipt->DetailData[i].uDetail.FurikaeDetail.Mod;
				vl_frs.card_furikaegaku   += (receipt->DetailData[i].uDetail.FurikaeDetail.FrkCard - receipt->FRK_Return);
			}
		}
	}
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
	vl_frs.syubetu = receipt->syu;									// 料金種別
	vl_frs.seisan_oiban = receipt->Oiban;							// 精算追い番
	vl_frs.seisan_time = receipt->TOutTime;						// 精算時刻
}

//[]----------------------------------------------------------------------[]
///	@brief		振替精算実行可能チェック要求
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/18<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_03_Furikae_Check( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ulong			CompTime1 = 0, CompTime2 = 0;

	// 実行可能チェック
	rslt = PiP_CheckFurikae();

	// チェックOK？
	if( rslt == PIP_RES_RESULT_OK ){

		// 精算データ取得
		memset( &wkReceipt, 0, sizeof( Receipt_data) );
		if( 0 == PiP_GetNewestPayReceiptData( &wkReceipt, RecvNtnetDt.RData16_03.SrcArea, RecvNtnetDt.RData16_03.SrcNo ) ){
			rslt = PIP_RES_RESULT_NG_NO_CAR;
			// NG応答
			NTNET_Snd_Data16_04( rslt );
			return;
		}
		// 振替元精算時刻ノーマライズ
		CompTime1 = enc_nmlz_mdhm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day,
																wkReceipt.TOutTime.Hour, wkReceipt.TOutTime.Min );
		// 前回集計時刻ノーマライズ
		CompTime2 = enc_nmlz_mdhm( sky.tsyuk.OldTime.Year, sky.tsyuk.OldTime.Mon, sky.tsyuk.OldTime.Day,
																sky.tsyuk.OldTime.Hour, sky.tsyuk.OldTime.Min );
		// 集計を跨いでいる？
		if( CompTime1 <= CompTime2 ){
			// NG応答
			rslt = PIP_RES_RESULT_NG_DENY;
			NTNET_Snd_Data16_04( rslt );
			return;
		}

		// 振替元車室
		g_PipCtrl.stFurikaeInfo.SrcArea		= RecvNtnetDt.RData16_03.SrcArea;	// 区画
		g_PipCtrl.stFurikaeInfo.SrcNo		= RecvNtnetDt.RData16_03.SrcNo;		// 車室
		// 振替先車室
		g_PipCtrl.stFurikaeInfo.DestArea	= RecvNtnetDt.RData16_03.DestArea;	// 区画
		g_PipCtrl.stFurikaeInfo.DestNo		= RecvNtnetDt.RData16_03.DestNo;	// 車室
	}

	// 結果応答(OK/NG)
	NTNET_Snd_Data16_04( rslt );
}

//[]----------------------------------------------------------------------[]
///	@brief		振替精算要求
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/18<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_03_Furikae_Go( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ushort			i;
	ulong			CompTime1 = 0, CompTime2 = 0;

	// 実行可能チェック
	rslt = PiP_CheckFurikae();

	// チェックOK？
	if( rslt == PIP_RES_RESULT_OK ){

		// 精算データ取得
		memset( &wkReceipt, 0, sizeof( Receipt_data) );
		if( 0 == PiP_GetNewestPayReceiptData( &wkReceipt, RecvNtnetDt.RData16_03.SrcArea, RecvNtnetDt.RData16_03.SrcNo ) ){
			rslt = PIP_RES_RESULT_NG_NO_CAR;
			// NG応答
			NTNET_Snd_Data16_04( rslt );
			return;
		}
		// 振替元精算時刻ノーマライズ
		CompTime1 = enc_nmlz_mdhm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day,
																wkReceipt.TOutTime.Hour, wkReceipt.TOutTime.Min );
		// 前回集計時刻ノーマライズ
		CompTime2 = enc_nmlz_mdhm( sky.tsyuk.OldTime.Year, sky.tsyuk.OldTime.Mon, sky.tsyuk.OldTime.Day,
																sky.tsyuk.OldTime.Hour, sky.tsyuk.OldTime.Min );
		// 集計を跨いでいる？
		if( CompTime1 <= CompTime2 ){
			// NG応答
			rslt = PIP_RES_RESULT_NG_DENY;
			NTNET_Snd_Data16_04( rslt );
			return;
		}

		g_PipCtrl.stFurikaeInfo.ReqKind = UNI_KIND_FURIKAE;
		// 振替元車室
		PiP_GetFurikaeInfo( &wkReceipt );										// 振替元情報を取得
		g_PipCtrl.stFurikaeInfo.PassUse		= vl_frs.antipassoff_req;			// 振替元定期利用有無
		g_PipCtrl.stFurikaeInfo.SrcArea		= RecvNtnetDt.RData16_03.SrcArea;	// 区画
		g_PipCtrl.stFurikaeInfo.SrcNo		= RecvNtnetDt.RData16_03.SrcNo;		// 車室
		g_PipCtrl.stFurikaeInfo.SrcFeeKind	= wkReceipt.syu;					// 料金種別
		g_PipCtrl.stFurikaeInfo.SrcFee		= wkReceipt.WPrice;					// 駐車料金
		for( i=0; i<WTIK_USEMAX; i++ ){
// 不具合修正(S) K.Onodera 2016/11/24 #1585 振替元割引金額に無関係の値がセットされる
//			g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DiscountData[i].Discount;	// 割引額加算
			if( ( wkReceipt.DiscountData[i].DiscSyu != 0 ) &&					// 割引種別あり
			    (( wkReceipt.DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// 精算中止割引情報でない
			    ( NTNET_CFRE < wkReceipt.DiscountData[i].DiscSyu )) ){
			    if( wkReceipt.DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA以外(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO以外(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_EDY_1 &&			// EDY以外	(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_EDY_2 &&			//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA以外(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(支払額・残額)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card以外(番号)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICCARD_2 &&		// 			(支払額・残額)
			   		wkReceipt.DiscountData[i].DiscSyu < NTNET_SECTION_WARI_MAX ) // 割引情報として使用するのは種別001～999まで
			    {
			    	g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DiscountData[i].Discount;	// 割引額加算
			    }
			}
// 不具合修正(E) K.Onodera 2016/11/24 #1585 振替元割引金額に無関係の値がセットされる
		}
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
		for( i=0; i<DETAIL_SYU_MAX; i++ ){
			if( wkReceipt.DiscountData[i].DiscSyu == NTNET_FURIKAE_2 ){
				// 振替額を加算(駐車料金より高くなる場合があるが、マイナスとして印字しているため)
				g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DetailData[i].uDetail.Furikae.Total;
			}
		}
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
		memcpy( &g_PipCtrl.stFurikaeInfo.SrcPayTime, &wkReceipt.TOutTime, sizeof(date_time_rec) );	// 精算時刻 ※分まで
		memcpy( &g_PipCtrl.stFurikaeInfo.SrcInTime,  &wkReceipt.TInTime, sizeof(date_time_rec) );		// 入庫時刻 ※分まで
		// 振替先車室
		g_PipCtrl.stFurikaeInfo.DestArea	= RecvNtnetDt.RData16_03.DestArea;	// 区画
		g_PipCtrl.stFurikaeInfo.DestNo		= RecvNtnetDt.RData16_03.DestNo;	// 車室

		// OPEに振替実行要求
		queset( OPETCBNO, OPE_REQ_FURIKAE_GO, 0, NULL );
	}
	// チェックNG？
	else{
		// NG応答
		NTNET_Snd_Data16_04( rslt );
	}
}

///*[]----------------------------------------------------------------------[]*
// *| 振替精算応答
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
void NTNET_Snd_Data16_04( ushort rslt )
{
// MH322914(S) K.Onodera 2016/12/22 [静的解析-570]配列インデックスオーバー
	ushort size = 0;
// MH322914(E) K.Onodera 2016/12/22 [静的解析-570]配列インデックスオーバー

	// 応答データ送信
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_04 ) );

	BasicDataMake( PIP_RES_KIND_FURIKAE, 1 );									// 基本ﾃﾞｰﾀ作成 ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定

	SendNtnetDt.SData16_04.Common.FmtRev 		= 10;								// フォーマットRev.No.(10固定)
	SendNtnetDt.SData16_04.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// 送信先の端末ターミナル№をセットする（99＝遠隔PC）
	SendNtnetDt.SData16_04.Common.CenterSeqNo	= RecvNtnetDt.RData16_03.Common.CenterSeqNo;	// センター追番
	SendNtnetDt.SData16_04.RcvKind				= RecvNtnetDt.RData16_03.ReqKind;	// 応答種別
	SendNtnetDt.SData16_04.Result				= rslt;								// 結果

	// 正常応答
	if( rslt == PIP_RES_RESULT_OK ){
// MH322914(S) K.Onodera 2016/12/22 [静的解析-575]インデックスオーバー
//		memcpy( &SendNtnetDt.SData16_04.OpenClose, &g_PipCtrl.stFurikaeInfo.OpenClose, sizeof(g_PipCtrl.stFurikaeInfo) );
		size =  (ushort)_offsetof(DATA_KIND_16_04, OpenClose);
		size = ( sizeof(DATA_KIND_16_04) - size );
		memcpy( &SendNtnetDt.SData16_04.OpenClose, &g_PipCtrl.stFurikaeInfo.OpenClose, size );
// MH322914(E) K.Onodera 2016/12/22 [静的解析-575]配列インデックスオーバー
		SendNtnetDt.SData16_04.stDestInfo.SrcFeeDiscount = vl_frs.furikaegaku;	// 振替可能額をセット
	}
	else{
		PiP_FurikaeSts = PIP_FRK_STS_IDLE;
	}
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_04 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_04 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
}


//[]----------------------------------------------------------------------[]
///	@brief		遠隔精算要求受信
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_08_RemoteCalc( void )
{
	switch( RecvNtnetDt.RData16_07.ReqKind ){
		case PIP_REMOTE_TYPE_CALC:		// 料金計算
		case PIP_REMOTE_TYPE_CALC_TIME:	// 入庫時刻指定遠隔精算
			NTNET_RevData16_08_RemoteCalc_Time();
			break;
		case PIP_REMOTE_TYPE_CALC_FEE:	// 精算金額指定遠隔精算
			NTNET_RevData182_RemoteCalcFee();
			break;
		default:
			// NG応答
			NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_PARAM );	// パラメータ不良
			break;
	}
}


//[]----------------------------------------------------------------------[]
///	@brief		遠隔精算料金計算/入庫時間指定精算要求
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_08_RemoteCalc_Time( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ushort			ndat = 0;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	ushort 			pos = 0;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	// 条理チェック
	do{
		// 1.年月日チェック
		if( chkdate( (short)RecvNtnetDt.RData16_07.InTime.Year, (short)RecvNtnetDt.RData16_07.InTime.Mon, (short)RecvNtnetDt.RData16_07.InTime.Day ) ){
			// NG
			rslt = PIP_RES_RESULT_NG_PARAM;		// パラメータ不良
			break;
		}
		// 2.時分秒チェック
		if( RecvNtnetDt.RData16_07.InTime.Hour > 23 ||
			RecvNtnetDt.RData16_07.InTime.Min  > 59 ||
			RecvNtnetDt.RData16_07.InTime.Sec  > 59 ){
			// NG
			rslt = PIP_RES_RESULT_NG_PARAM;		// パラメータ不良
			break;
		}
		// 3.日付チェック
		ndat = dnrmlzm( (short)RecvNtnetDt.RData16_07.InTime.Year, (short)RecvNtnetDt.RData16_07.InTime.Mon, (short)RecvNtnetDt.RData16_07.InTime.Day );
		// 未来？
		if( CLK_REC.ndat < ndat ){
			// NG
			rslt = PIP_RES_RESULT_NG_PARAM;		// パラメータ不良
			break;
		}
		// 同日？
		else if( CLK_REC.ndat == ndat ){
			// 未来？
			if( CLK_REC.nmin < tnrmlz ( (short)0, (short)0, (short)RecvNtnetDt.RData16_07.InTime.Hour, (short)RecvNtnetDt.RData16_07.InTime.Min ) ){
				// NG
				rslt = PIP_RES_RESULT_NG_PARAM;		// パラメータ不良
				break;
			}
		}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		// 4.車室状態チェック
//		rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_07.Area, RecvNtnetDt.RData16_07.ulNo, &pos );
//		if( rslt != PIP_RES_RESULT_OK ){
//			break;
//		}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		// 5.状態チェック
		if( OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod > 1 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// 受付不可状態
			break;
		}
		// 6.自動集計中チェック
		if( auto_syu_prn == 2 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// 受付不可状態
			break;
		}

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
		if ( !CLOUD_CALC_MODE ) {	// 通常料金計算モード
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		// 料金種別チェック
		if( prm_get( COM_PRM,S_SHA,(short)(1+6*(RecvNtnetDt.RData16_07.Syubet-1)),2,5 ) == 0L ){	// 車種設定なし（使用する設定でない）
			// NG
			rslt = PIP_RES_RESULT_NG_PARAM;		// パラメータ不良
			break;
		}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
		}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

		// 7.割引区分、種別チェック
		if( RecvNtnetDt.RData16_07.stDiscount.ParkNo ){
			if( 0 == RecvNtnetDt.RData16_07.stDiscount.DiscSyubet ||
				(2 == RecvNtnetDt.RData16_07.stDiscount.DiscSyubet && 0 == RecvNtnetDt.RData16_07.stDiscount.DiscType) ){
				// NG
				rslt = PIP_RES_RESULT_NG_PARAM;		// パラメータ不良
				break;
			}
		}
	}while(0);

	// 遠隔精算時刻指定
	if( rslt == PIP_RES_RESULT_OK ){
		g_PipCtrl.stRemoteTime.ReqKind = UNI_KIND_REMOTE_TIME;
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
		g_PipCtrl.stRemoteTime.ulPno = RecvNtnetDt.RData16_07.DataBasic.ParkingNo;				// 駐車場№
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		g_PipCtrl.stRemoteTime.Area = RecvNtnetDt.RData16_07.Area;
//		g_PipCtrl.stRemoteTime.ulNo = RecvNtnetDt.RData16_07.ulNo;
		g_PipCtrl.stRemoteTime.RyoSyu = RecvNtnetDt.RData16_07.Syubet;							// 料金種別
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		memcpy( &g_PipCtrl.stRemoteTime.InTime, &RecvNtnetDt.RData16_07.InTime, sizeof(g_PipCtrl.stRemoteTime.InTime) );
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		g_PipCtrl.stRemoteTime.FutureFee = RecvNtnetDt.RData16_07.FutureFee;
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		// 割引情報
		g_PipCtrl.stRemoteTime.DiscountKind = RecvNtnetDt.RData16_07.stDiscount.DiscSyubet;		// 割引種別
		g_PipCtrl.stRemoteTime.DiscountType = RecvNtnetDt.RData16_07.stDiscount.DiscType;		// 割引区分
		g_PipCtrl.stRemoteTime.DiscountCnt = RecvNtnetDt.RData16_07.stDiscount.DiscCount;		// 割引使用枚数
		g_PipCtrl.stRemoteTime.Discount = RecvNtnetDt.RData16_07.stDiscount.DiscPrice;			// 割引金額
		g_PipCtrl.stRemoteTime.DiscountInfo1 = RecvNtnetDt.RData16_07.stDiscount.DiscInfo1;		// 割引情報１
		g_PipCtrl.stRemoteTime.DiscountInfo2 = RecvNtnetDt.RData16_07.stDiscount.DiscInfo2;		// 割引情報２
		// OPEへ通知
		if( RecvNtnetDt.RData16_07.ReqKind == PIP_REMOTE_TYPE_CALC ){
			queset( OPETCBNO, OPE_REQ_REMOTE_CALC_TIME_PRE, 0, NULL );
		}else{
			queset( OPETCBNO, OPE_REQ_REMOTE_CALC_TIME, 0, NULL );
		}
	}else{
		// NG応答データ送信
		NTNET_Snd_Data16_08( rslt );
	}
}

///*[]----------------------------------------------------------------------[]*
// *| 遠隔精算入庫時刻指定応答
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
void NTNET_Snd_Data16_08( ushort rslt )
{
	ulong	intime, outtime;
	ushort	indate, outdate;


	// 応答データ送信
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_08 ) );

	BasicDataMake( PIP_RES_KIND_REMOTE_CALC_ENTRY, 1 );								// 基本ﾃﾞｰﾀ作成 ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定

	SendNtnetDt.SData16_08.Common.FmtRev 		= 10;								// フォーマットRev.No.(10固定)
	SendNtnetDt.SData16_08.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// 送信先の端末ターミナル№をセットする（99＝遠隔PC）
	SendNtnetDt.SData16_08.Common.CenterSeqNo	= RecvNtnetDt.RData16_07.Common.CenterSeqNo;	// センター追番
	SendNtnetDt.SData16_08.RcvKind		= RecvNtnetDt.RData16_07.ReqKind;			// 応答種別
	SendNtnetDt.SData16_08.Result				= rslt;								// 結果

	// 正常時にセット -----------------
	if( rslt == PIP_RES_RESULT_OK ){
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		SendNtnetDt.SData16_08.Area					= RecvNtnetDt.RData16_07.Area;		// 区画
//		SendNtnetDt.SData16_08.ulNo					= RecvNtnetDt.RData16_07.ulNo;		// 車室
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		memcpy( &SendNtnetDt.SData16_08.InTime, &RecvNtnetDt.RData16_07.InTime, sizeof(SendNtnetDt.SData16_08.InTime) );	// 入庫時刻
		memcpy( &SendNtnetDt.SData16_08.stDiscount, &RecvNtnetDt.RData16_07.stDiscount, sizeof(SendNtnetDt.SData16_08.stDiscount) );	// 割引情報
		SendNtnetDt.SData16_08.stDiscount.DiscPrice = g_PipCtrl.stRemoteTime.Discount;
		SendNtnetDt.SData16_08.Syubet			= g_PipCtrl.stRemoteTime.RyoSyu;
		SendNtnetDt.SData16_08.Price			= g_PipCtrl.stRemoteTime.Price;
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		SendNtnetDt.SData16_08.FutureFee		= RecvNtnetDt.RData16_07.FutureFee;
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		memcpy( &SendNtnetDt.SData16_08.PayTime, &g_PipCtrl.stRemoteTime.OutTime, sizeof(SendNtnetDt.SData16_08.PayTime) );	// 料金計算時刻

		// 入庫日計算
		indate	= dnrmlzm( (short)SendNtnetDt.SData16_08.InTime.Year, (short)SendNtnetDt.SData16_08.InTime.Mon, (short)SendNtnetDt.SData16_08.InTime.Day );
		// 出庫日計算
		outdate	= dnrmlzm( (short)SendNtnetDt.SData16_08.PayTime.Year, (short)SendNtnetDt.SData16_08.PayTime.Mon, (short)SendNtnetDt.SData16_08.PayTime.Day );

		// 入庫日時（秒換算）計算
		intime	=	(indate*24*60*60) +								// 年月日
					(SendNtnetDt.SData16_08.InTime.Hour*60*60) +	// 時
					(SendNtnetDt.SData16_08.InTime.Min*60) +		// 分
					(SendNtnetDt.SData16_08.InTime.Sec);			// 秒
		// 出庫日時（秒換算）計算
		outtime	=	(outdate*24*60*60) +
					(SendNtnetDt.SData16_08.PayTime.Hour*60*60) +
					(SendNtnetDt.SData16_08.PayTime.Min*60) +
					(SendNtnetDt.SData16_08.PayTime.Sec);
		// 出庫日時≧入庫日時
		if( outtime >= intime ){
			SendNtnetDt.SData16_08.ParkTotalSec	= outtime - intime;												// 駐車時間取得（秒換算）
			SendNtnetDt.SData16_08.ParkTimeDay	= SendNtnetDt.SData16_08.ParkTotalSec/(60*60*24);				// 駐車時間（日）計算
			SendNtnetDt.SData16_08.ParkTimeHour	= SendNtnetDt.SData16_08.ParkTotalSec%(60*60*24)/(60*60);		// 駐車時間（時）計算
			SendNtnetDt.SData16_08.ParkTimeMin	= SendNtnetDt.SData16_08.ParkTotalSec%(60*60)/60;				// 駐車時間（分）計算
			SendNtnetDt.SData16_08.ParkTimeSec	= SendNtnetDt.SData16_08.ParkTotalSec%60;						// 駐車時間（秒）計算
		}
	}
	
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_08 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_08 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
}



///*[]----------------------------------------------------------------------[]*
// *| 遠隔精算金額指定精算要求
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData182_RemoteCalcFee
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static void NTNET_RevData182_RemoteCalcFee(void)
{
	ushort			rslt = PIP_RES_RESULT_OK;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	ushort 			pos = 0;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	// 条理チェック
	do{
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		// 1.車室状態チェック
//// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
////		rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_07.Area, RecvNtnetDt.RData16_07.ulNo, &pos );
//		// 後日精算？
//		if( RecvNtnetDt.RData16_07.Type ){
//			//if( RecvNtnetDt.RData16_07.ulNo != 9900 ){
//			//	rslt = PIP_RES_RESULT_NG_PARAM;
//			//}
//		}
//		// 出庫精算？
//		else{
//			rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_07.Area, RecvNtnetDt.RData16_07.ulNo, &pos );
//		}
//// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//		if( rslt != PIP_RES_RESULT_OK ){
//			break;
//		}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		// 2.状態チェック
		if( OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod > 1 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// 受付不可状態
			break;
		}
		// 3.自動集計中チェック
		if( auto_syu_prn == 2 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// 受付不可状態
			break;
		}
		// 4.割引区分、種別チェック
		if( RecvNtnetDt.RData16_07.stDiscount.ParkNo ){
			if( 0 == RecvNtnetDt.RData16_07.stDiscount.DiscSyubet ||
				(2 == RecvNtnetDt.RData16_07.stDiscount.DiscSyubet && 0 == RecvNtnetDt.RData16_07.stDiscount.DiscType) ){
				// NG
				rslt = PIP_RES_RESULT_NG_PARAM;		// パラメータ不良
				break;
			}
		}
	}while(0);

	// 遠隔精算金額指定
	if( rslt == PIP_RES_RESULT_OK ){
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		g_PipCtrl.stRemoteFee.ReqKind = UNI_KIND_REMOTE_FEE;
		g_PipCtrl.stRemoteFee.Type = RecvNtnetDt.RData16_07.Type;
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
// 仕様変更(S) K.Onodera 2016/11/07 後日精算対応
		g_PipCtrl.stRemoteFee.RyoSyu = RecvNtnetDt.RData16_07.Syubet;
// 仕様変更(E) K.Onodera 2016/11/07 後日精算対応
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		g_PipCtrl.stRemoteFee.Area = RecvNtnetDt.RData16_07.Area;
//		g_PipCtrl.stRemoteFee.ulNo = RecvNtnetDt.RData16_07.ulNo;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		g_PipCtrl.stRemoteFee.Price = RecvNtnetDt.RData16_07.Price;
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		g_PipCtrl.stRemoteFee.FutureFee = RecvNtnetDt.RData16_07.FutureFee;
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		// 割引情報
		g_PipCtrl.stRemoteFee.DiscountKind = RecvNtnetDt.RData16_07.stDiscount.DiscSyubet;	// 割引種別
		g_PipCtrl.stRemoteFee.DiscountType = RecvNtnetDt.RData16_07.stDiscount.DiscType;	// 割引区分
		g_PipCtrl.stRemoteFee.DiscountCnt = RecvNtnetDt.RData16_07.stDiscount.DiscCount;	// 割引使用枚数
		g_PipCtrl.stRemoteFee.Discount = RecvNtnetDt.RData16_07.stDiscount.DiscPrice;		// 割引金額
		g_PipCtrl.stRemoteFee.DiscountInfo1 = RecvNtnetDt.RData16_07.stDiscount.DiscInfo1;	// 割引情報１
		g_PipCtrl.stRemoteFee.DiscountInfo2 = RecvNtnetDt.RData16_07.stDiscount.DiscInfo2;	// 割引情報２
		// OPEへ通知
		queset( OPETCBNO, OPE_REQ_REMOTE_CALC_FEE, 0, NULL );
	}else{
		// NG応答データ送信
		NTNET_Snd_Data16_09(rslt);
	}
}

///*[]----------------------------------------------------------------------[]*
// *| 遠隔精算金額指定応答
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
void NTNET_Snd_Data16_09( ushort rslt )
{
	// 応答データ送信
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_09 ) );

	BasicDataMake( PIP_RES_KIND_REMOTE_CALC_FEE, 1 );								// 基本ﾃﾞｰﾀ作成 ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定

	SendNtnetDt.SData16_09.Common.FmtRev 		= 10;								// フォーマットRev.No.(10固定)
	SendNtnetDt.SData16_09.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// 送信先の端末ターミナル№をセットする（99＝遠隔PC）
	SendNtnetDt.SData16_09.Common.CenterSeqNo	= RecvNtnetDt.RData16_07.Common.CenterSeqNo;	// センター追番
	SendNtnetDt.SData16_09.Result				= rslt;								// 結果
	// 正常時にセット -----------------
	if( rslt == PIP_RES_RESULT_OK ){
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
		SendNtnetDt.SData16_09.Type					= RecvNtnetDt.RData16_07.Type;		// 遠隔精算種別
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//		SendNtnetDt.SData16_09.Area					= RecvNtnetDt.RData16_07.Area;		// 区画
//		SendNtnetDt.SData16_09.ulNo					= RecvNtnetDt.RData16_07.ulNo;		// 車室
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		SendNtnetDt.SData16_09.Syubet				= g_PipCtrl.stRemoteFee.RyoSyu;		// 料金種別
		SendNtnetDt.SData16_09.Price				= RecvNtnetDt.RData16_07.Price;		// 駐車料金
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		SendNtnetDt.SData16_09.FutureFee			= RecvNtnetDt.RData16_07.FutureFee;
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
		memcpy( &SendNtnetDt.SData16_09.stDiscount, &RecvNtnetDt.RData16_07.stDiscount, sizeof(SendNtnetDt.SData16_09.stDiscount) );	// 割引情報
		SendNtnetDt.SData16_09.stDiscount.DiscPrice = g_PipCtrl.stRemoteFee.Discount;
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_09 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_09 ), NTNET_BUF_BUFFERING );	// ﾃﾞｰﾀ送信登録
	}
}
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

// MH321800(S) Y.Tanizaki ICクレジット対応
//// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
////[]----------------------------------------------------------------------[]
/////	@brief		精算データ(Rev.14：フェーズ7)
////[]----------------------------------------------------------------------[]
/////	@param[in]	p_RcptDat	：精算ログデータ
/////	@param[out]	p_NtDat		：精算NT-NETデータ
/////	@return		none
////[]----------------------------------------------------------------------[]
/////	@date		Create	: 2016/11/01<br>
/////				Update	: 
////[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
//unsigned short	NTNET_Edit_Data56_r14( Receipt_data *p_RcptDat, DATA_KIND_56_r14 *p_NtDat )
//{
//	uchar		i;
//	uchar		j;
//	ushort		ret;
//	EMONEY_r14	t_EMoney;
//	CREINFO_r14	t_Credit;
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
////	date_time_rec	wk_CLK_REC;
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_56_r14 ) );
//
//	NTNET_Edit_BasicData( 57, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// 基本ﾃﾞｰﾀ作成
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TOutTime.Year % 100);		// 精算年
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TOutTime.Mon;				// 精算月
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TOutTime.Day;				// 精算日
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TOutTime.Hour;				// 精算時
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TOutTime.Min;				// 精算分
//	p_NtDat->DataBasic.Sec	= 0;
//	p_NtDat->FmtRev = 14;													// フォーマットRev.№
//	p_NtDat->CenterSeqNo = p_RcptDat->CenterSeqNo;							// センター追番（精算）
//
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->FullNo1;						// 満車台数１
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CarCnt1;						// 現在台数１
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->FullNo2;						// 満車台数２
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CarCnt2;						// 現在台数２
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->FullNo3;						// 満車台数３
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CarCnt3;						// 現在台数３
//	p_NtDat->ParkData.CarFullFlag	= (uchar)0;								// 台数・満空状態フラグ
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->PascarCnt;				// 定期車両カウント
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->Full[0];						// 駐車1満空状態
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->Full[1];						// 駐車2満空状態
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->Full[2];						// 駐車3満空状態
//
//	// 台数管理追番 ------------------------
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
////	memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
////	// 同一時分？
////	if( VehicleCountDateTime == Nrm_YMDHM( &wk_CLK_REC ) ){
////		// 同一時分の場合は追い番を＋１
////		VehicleCountSeqNo++;
////	}else{
////		// 異なる時分の場合は追い番を０とする
////		VehicleCountDateTime = Nrm_YMDHM( &wk_CLK_REC );
////		VehicleCountSeqNo = 0;
////	}
////	p_NtDat->ParkData.CarCntInfo.CarCntYear		= (wk_CLK_REC.Year % 100);	// 年
////	p_NtDat->ParkData.CarCntInfo.CarCntMon		= wk_CLK_REC.Mon;			// 月
////	p_NtDat->ParkData.CarCntInfo.CarCntDay		= wk_CLK_REC.Day;			// 日
////	p_NtDat->ParkData.CarCntInfo.CarCntHour		= wk_CLK_REC.Hour;			// 時
////	p_NtDat->ParkData.CarCntInfo.CarCntMin		= wk_CLK_REC.Min;			// 分
////	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= VehicleCountSeqNo;		// 追番
////	p_NtDat->ParkData.CarCntInfo.Reserve1		= 0;						// 予備
//	p_NtDat->ParkData.CarCntInfo.CarCntYear		= p_RcptDat->CarCntInfo.CarCntYear;	// 年
//	p_NtDat->ParkData.CarCntInfo.CarCntMon		= p_RcptDat->CarCntInfo.CarCntMon;	// 月
//	p_NtDat->ParkData.CarCntInfo.CarCntDay		= p_RcptDat->CarCntInfo.CarCntDay;	// 日
//	p_NtDat->ParkData.CarCntInfo.CarCntHour		= p_RcptDat->CarCntInfo.CarCntHour;	// 時
//	p_NtDat->ParkData.CarCntInfo.CarCntMin		= p_RcptDat->CarCntInfo.CarCntMin;	// 分
//	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= p_RcptDat->CarCntInfo.CarCntSeq;	// 追番
//	p_NtDat->ParkData.CarCntInfo.Reserve1		= p_RcptDat->CarCntInfo.Reserve1;	// 予備
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//	// -------------------------------------
//
//	p_NtDat->PayCount	= CountSel( &p_RcptDat->Oiban);						// 精算or精算中止追い番
//	p_NtDat->PayMethod	= (uchar)p_RcptDat->PayMethod;						// 精算方法
//	if( p_RcptDat->PayClass == 4 ){
//		// クレジット精算
//		p_NtDat->PayClass	= (uchar)0;										// 処理区分
//	}else{
//		p_NtDat->PayClass	= (uchar)p_RcptDat->PayClass;					// 処理区分
//	}
//	p_NtDat->PayMode	= (uchar)p_RcptDat->PayMode;						// 精算ﾓｰﾄﾞ(自動精算)
//	p_NtDat->CMachineNo	= 0;												// 駐車券機械№
//
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->FlapArea	= (ushort)(p_RcptDat->WPlace/10000);			// フラップシステム	区画		0～99
//		p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->WPlace%10000);			// 					車室番号	0～9999
//		p_NtDat->KakariNo	= p_RcptDat->KakariNo;							// 係員№
//		p_NtDat->OutKind	= p_RcptDat->OutKind;							// 精算出庫
//	}
//
//	p_NtDat->CountSet	= p_RcptDat->CountSet;								// 在車カウントしない
//
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->InPrev_ymdhms.In_Year	= (uchar)(p_RcptDat->TInTime.Year%100);	// 入庫年
//		p_NtDat->InPrev_ymdhms.In_Mon	= p_RcptDat->TInTime.Mon;				// 入庫月
//		p_NtDat->InPrev_ymdhms.In_Day	= p_RcptDat->TInTime.Day;				// 入庫日
//		p_NtDat->InPrev_ymdhms.In_Hour	= p_RcptDat->TInTime.Hour;				// 入庫時
//		p_NtDat->InPrev_ymdhms.In_Min	= p_RcptDat->TInTime.Min;				// 入庫分
//		p_NtDat->InPrev_ymdhms.In_Sec	= 0;									// 入庫秒
//
//		// 前回精算年月日時分秒は0とする
//		p_NtDat->InPrev_ymdhms.Prev_Year	= 0;							// 前回精算年
//		p_NtDat->InPrev_ymdhms.Prev_Mon		= 0;							// 前回精算月
//		p_NtDat->InPrev_ymdhms.Prev_Day		= 0;							// 前回精算日
//		p_NtDat->InPrev_ymdhms.Prev_Hour	= 0;							// 前回精算時
//		p_NtDat->InPrev_ymdhms.Prev_Min		= 0;							// 前回精算分
//		p_NtDat->InPrev_ymdhms.Prev_Sec		= 0;							// 前回精算秒
//	}
//
//	p_NtDat->ReceiptIssue	= p_RcptDat->ReceiptIssue;						// 領収証発行有無
//
//	if( p_NtDat->PayMethod != 5 ){											// 更新精算以外
//		p_NtDat->Syubet	= p_RcptDat->syu;									// 料金種別
//		p_NtDat->Price	= p_RcptDat->WPrice;								// 駐車料金
//	}
//
//	p_NtDat->CashPrice	= p_RcptDat->WTotalPrice;							// 現金売上
//	p_NtDat->InPrice	= p_RcptDat->WInPrice;								// 投入金額
//// 仕様変更(S) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
////	p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// 釣銭金額
//	if( p_RcptDat->FRK_RetMod == 0 ){
//		p_NtDat->ChgPrice	= (ushort)(p_RcptDat->WChgPrice + p_RcptDat->FRK_Return);	// 釣銭金額 + 振替過払い金
//	}else{
//		p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// 釣銭金額
//	}
//// 仕様変更(E) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
//
//	p_NtDat->MoneyInOut.MoneyKind_In	= 0x1f;								// 金種有無	搭載金種の有無　投入金（10円,50円,100円,500円,1000円）
//	p_NtDat->MoneyInOut.In_10_cnt		= (uchar)p_RcptDat->in_coin[0];		// 投入金枚数(10円)		0～255
//	p_NtDat->MoneyInOut.In_50_cnt		= (uchar)p_RcptDat->in_coin[1];		// 投入金枚数(50円)		0～255
//	p_NtDat->MoneyInOut.In_100_cnt		= (uchar)p_RcptDat->in_coin[2];		// 投入金枚数(100円)	0～255
//	p_NtDat->MoneyInOut.In_500_cnt		= (uchar)p_RcptDat->in_coin[3];		// 投入金枚数(500円)	0～255
//	p_NtDat->MoneyInOut.In_1000_cnt		= (uchar)p_RcptDat->in_coin[4];		// 投入金枚数(1000円)	0～255
//	if(p_RcptDat->f_escrow) {												// エスクロ紙幣戻しあり
//		++p_NtDat->MoneyInOut.In_1000_cnt;									// エスクロ紙幣戻し分を投入枚数に加算
//	}
//
//	p_NtDat->MoneyInOut.MoneyKind_Out	= 0x1f;								// 金種有無	搭載金種の有無　払出金（10円,50円,100円,500円,1000円）
//	p_NtDat->MoneyInOut.Out_10_cnt		= (uchar)p_RcptDat->out_coin[0];	// 払出金枚数(10円)		0～255
//	p_NtDat->MoneyInOut.Out_50_cnt		= (uchar)p_RcptDat->out_coin[1];	// 払出金枚数(50円)		0～255
//	p_NtDat->MoneyInOut.Out_100_cnt		= (uchar)p_RcptDat->out_coin[2];	// 払出金枚数(100円)	0～255
//	p_NtDat->MoneyInOut.Out_500_cnt		= (uchar)p_RcptDat->out_coin[3];	// 払出金枚数(500円)	0～255
//	p_NtDat->MoneyInOut.Out_1000_cnt	= (uchar)p_RcptDat->f_escrow;		// 払出金枚数(1000円)	0～255
//
//	p_NtDat->HaraiModoshiFusoku	= (ushort)p_RcptDat->WFusoku;				// 払戻不足額	0～9999
//	if( p_RcptDat->FRK_Return && p_RcptDat->FRK_RetMod ){
//		p_NtDat->CardFusokuType = p_RcptDat->FRK_RetMod;					// 払出不足媒体種別(現金以外)
//		p_NtDat->CardFusokuTotal = p_RcptDat->FRK_Return;					// 払出不足額(現金以外)
//	}
//	p_NtDat->SaleParkingNo = (ulong)CPrmSS[S_SYS][1];						// 売上先駐車場№(0～999999   ※特注用（標準では基本駐車場№をセット）
//
//	p_NtDat->PassCheck = p_RcptDat->PassCheck;								// アンチパスチェック
//
//	if( p_RcptDat->teiki.ParkingNo ) {
//		p_NtDat->ParkingNo	= p_RcptDat->teiki.ParkingNo;								// 定期券駐車場№
//		p_NtDat->Media[0].MediaKind	= (ushort)(p_RcptDat->teiki.pkno_syu + 2);			// 精算媒体情報１　種別(メイン媒体)	0～99
//		intoasl(p_NtDat->Media[0].MediaCardNo, p_RcptDat->teiki.id, 5);					// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		intoasl(p_NtDat->Media[0].MediaCardInfo, p_RcptDat->teiki.syu, 2);				// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		// 精算媒体情報2（サブ媒体）は オール０とする
//	}
//
//	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 ){	// 電子決済種別 Suica:1, Edy:2
//		p_NtDat->CardKind = (ushort)2;											// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	}else if( p_RcptDat->PayClass == 4 ){					// クレジット精算
//		p_NtDat->CardKind = (ushort)1;											// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	}
//
//	switch( p_NtDat->CardKind ){							// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//	case 1:
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
//		memset( &t_Credit, 0, sizeof( t_Credit ) );
//		t_Credit.amount = (ulong)p_RcptDat->credit.pay_ryo;									// 決済額
//		memcpyFlushLeft( &t_Credit.card_no[0], (uchar *)&p_RcptDat->credit.card_no[0], sizeof(t_Credit.card_no), sizeof(p_RcptDat->credit.card_no) );	// 会員番号
//		memcpyFlushLeft( &t_Credit.cct_num[0], (uchar *)&p_RcptDat->credit.CCT_Num[0],
//							sizeof(t_Credit.cct_num), CREDIT_TERM_ID_NO_SIZE );				// 端末識別番号
//		memcpyFlushLeft( &t_Credit.kid_code[0], &p_RcptDat->credit.kid_code[0], sizeof(t_Credit.kid_code), sizeof(p_RcptDat->credit.kid_code) );		// KID コード
//		t_Credit.app_no = p_RcptDat->credit.app_no;											// 承認番号
//		t_Credit.center_oiban = 0;															// ｾﾝﾀｰ処理追番(センターは0固定)
//		memcpyFlushLeft( &t_Credit.ShopAccountNo[0], (uchar*)&p_RcptDat->credit.ShopAccountNo[0], sizeof(t_Credit.ShopAccountNo), sizeof(p_RcptDat->credit.ShopAccountNo) );	// 加盟店取引番号
//		t_Credit.slip_no = p_RcptDat->credit.slip_no;										// 伝票番号
//		
//		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );							// AES 暗号化
//		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// 決済情報
//		break;
//	case 2:
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
//		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//		t_EMoney.amount = p_RcptDat->Electron_data.Suica.pay_ryo;							// 電子決済精算情報　決済額
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//		memcpy( &t_EMoney.card_id[0], &p_RcptDat->Electron_data.Suica.Card_ID[0],
//				sizeof(p_RcptDat->Electron_data.Suica.Card_ID) );							// 電子決済精算情報　ｶｰﾄﾞID (Ascii 16桁)
//		t_EMoney.card_zangaku = p_RcptDat->Electron_data.Suica.pay_after;					// 電子決済精算情報　決済後残高
//		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );							// AES 暗号化
//		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// 決済情報
//		break;
//	}
//
//	p_NtDat->TotalSale		= (ulong)0;										// 合計金額(物販)
//	p_NtDat->DeleteSeq		= (ulong)0;										// 削除データ追番
//
//	for( i = j = 0; i < WTIK_USEMAX; i++ ){									// 精算中止以外の割引情報コピー
//		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&					// 割引種別あり
//		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// 精算中止割引情報でない
//		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
//		    if( p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA以外(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(支払額・残額)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO以外(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(支払額・残額)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY以外	(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(支払額・残額)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA以外(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(支払額・残額)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card以外(番号)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_2 ) 		//			(支払額・残額)
//		    {
//				p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DiscountData[i].ParkingNo;						// 駐車場No.
//				p_NtDat->SDiscount[j].Kind		= p_RcptDat->DiscountData[i].DiscSyu;						// 割引種別
//				p_NtDat->SDiscount[j].Group		= p_RcptDat->DiscountData[i].DiscNo;						// 割引区分
//				p_NtDat->SDiscount[j].Callback	= p_RcptDat->DiscountData[i].DiscCount;						// 回収枚数
//				p_NtDat->SDiscount[j].Amount	= p_RcptDat->DiscountData[i].Discount;						// 割引額
//				p_NtDat->SDiscount[j].Info1		= p_RcptDat->DiscountData[i].DiscInfo1;						// 割引情報1
//				if( p_NtDat->SDiscount[j].Kind != NTNET_PRI_W ){
//					p_NtDat->SDiscount[j].Info2	= p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;	// 割引情報2
//				}
//				j++;
//		    }
//		}
//	}
//	for( i=0; i<DETAIL_SYU_MAX; i++ ){
//		if( p_RcptDat->DetailData[i].DiscSyu ){
//			p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DetailData[i].ParkingNo;					// 駐車場No.
//			p_NtDat->SDiscount[j].Kind		= p_RcptDat->DetailData[i].DiscSyu;						// 割引種別
//			p_NtDat->SDiscount[j].Group		= p_RcptDat->DetailData[i].uDetail.Common.DiscNo;		// 割引区分
//			p_NtDat->SDiscount[j].Callback	= p_RcptDat->DetailData[i].uDetail.Common.DiscCount;	// 回収枚数
//			p_NtDat->SDiscount[j].Amount	= p_RcptDat->DetailData[i].uDetail.Common.Discount;		// 割引額
//			p_NtDat->SDiscount[j].Info1		= p_RcptDat->DetailData[i].uDetail.Common.DiscInfo1;	// 割引情報1
//			p_NtDat->SDiscount[j].Info2		= p_RcptDat->DetailData[i].uDetail.Common.DiscInfo2;	// 割引情報2
//			j++;
//		}
//	}
//
//// 不具合修正(S) K.Onodera 2016/12/09 連動評価指摘(0カット前のサイズが誤っている)
////	ret = sizeof( DATA_KIND_56_r10 );
//	ret = sizeof( DATA_KIND_56_r14 );
//// 不具合修正(E) K.Onodera 2016/12/09 連動評価指摘(0カット前のサイズが誤っている)
//
//	return ret;
//}
//
//[]----------------------------------------------------------------------[]
///	@brief		精算データ(Rev.17：フェーズ7)
//[]----------------------------------------------------------------------[]
///	@param[in]	p_RcptDat	：精算ログデータ
///	@param[out]	p_NtDat		：精算NT-NETデータ
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/11/01<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
// MH810105(S) MH364301 QRコード決済対応
//unsigned short	NTNET_Edit_Data56_r17( Receipt_data *p_RcptDat, DATA_KIND_56_r17 *p_NtDat )
unsigned short	NTNET_Edit_Data56_rXX( Receipt_data *p_RcptDat, DATA_KIND_56_rXX *p_NtDat )
// MH810105(E) MH364301 QRコード決済対応
{
	uchar		i;
	uchar		j;
	ushort		ret;
// MH810105(S) MH364301 QRコード決済対応
//	EMONEY_r14	t_EMoney;
//	CREINFO_r14 t_Credit;
// MH810105(E) MH364301 QRコード決済対応
// MH810100(S) K.Onodera  2020/01/31 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	uchar		knd;
// MH810100(E) K.Onodera  2020/01/31 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH810100(S) K.Onodera  2020/02/07 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	t_MediaInfoCre2 EncMediaInfo;
// MH810100(E) K.Onodera  2020/02/07 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
// GG124100(S) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
	SHABAN_INFO	EncShabanInfo;
// GG124100(E) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット

// MH810105(S) MH364301 QRコード決済対応
//	memset( p_NtDat, 0, sizeof( DATA_KIND_56_r17 ) );
	memset( p_NtDat, 0, sizeof( DATA_KIND_56_rXX ) );
// MH810105(E) MH364301 QRコード決済対応

// MH810100(S) K.Onodera  2020/01/31 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	NTNET_Edit_BasicData( 57, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// 基本ﾃﾞｰﾀ作成
	
	if( prm_get( COM_PRM, S_NTN, 27, 1, 1 ) == 0 ){	// 精算データIDの取り扱い = 出口精算機IDで送信？
		knd = 57;	// EPS
	}else{
		knd = 56;	// CPS
	}
	NTNET_Edit_BasicData( knd, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// 基本ﾃﾞｰﾀ作成
// MH810100(E) K.Onodera  2020/01/31 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH810100(S) 2020/08/31 #4787 リアルタイム通信　未精算出庫の精算データ（ID：57）の精算年月日時分秒が出庫年月日時分秒と同じになっている
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TOutTime.Year % 100);		// 精算年
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TOutTime.Mon;				// 精算月
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TOutTime.Day;				// 精算日
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TOutTime.Hour;				// 精算時
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TOutTime.Min;				// 精算分

// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//	// 種別	(0=事前精算／1＝未精算出庫精算)
//	if(p_RcptDat->shubetsu == 0){
	// 種別	(0=事前精算／1＝未精算出庫精算／2=遠隔精算(精算中変更))
	if(p_RcptDat->shubetsu == 0 || p_RcptDat->shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
		p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TOutTime.Year % 100);		// 精算年
		p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TOutTime.Mon;				// 精算月
		p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TOutTime.Day;				// 精算日
		p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TOutTime.Hour;				// 精算時
		p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TOutTime.Min;				// 精算分
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
		p_NtDat->DataBasic.Sec	= (uchar)p_RcptDat->TOutTime_Sec;				// 精算秒
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）

	}else{
		p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TUnpaidPayTime.Year % 100);	// 精算年
		p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TUnpaidPayTime.Mon;				// 精算月
		p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TUnpaidPayTime.Day;				// 精算日
		p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TUnpaidPayTime.Hour;			// 精算時
		p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TUnpaidPayTime.Min;				// 精算分
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
		p_NtDat->DataBasic.Sec	= (uchar)p_RcptDat->TUnpaidPayTime_Sec;				// 精算秒
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）

	}
// MH810100(E) 2020/08/31 #4787 リアルタイム通信　未精算出庫の精算データ（ID：57）の精算年月日時分秒が出庫年月日時分秒と同じになっている
// GG129000(S) R.Endo 2023/02/16 車番チケットレス4.0 #6937 NTNETの精算データの処理秒にデータが乗らない
// 	p_NtDat->DataBasic.Sec	= 0;
// GG129000(E) R.Endo 2023/02/16 車番チケットレス4.0 #6937 NTNETの精算データの処理秒にデータが乗らない
// MH810105(S) MH364301 QRコード決済対応
//	p_NtDat->FmtRev = 17;													// フォーマットRev.№
// GG129001(S) データ保管サービス対応（Rev.No.更新）
//	p_NtDat->FmtRev = 18;													// フォーマットRev.№
	p_NtDat->FmtRev = 20;													// フォーマットRev.№
// GG129001(E) データ保管サービス対応（Rev.No.更新）
// MH810105(E) MH364301 QRコード決済対応
	p_NtDat->CenterSeqNo = p_RcptDat->CenterSeqNo;							// センター追番（精算）

// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->FullNo1;						// 満車台数１
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CarCnt1;						// 現在台数１
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->FullNo2;						// 満車台数２
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CarCnt2;						// 現在台数２
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->FullNo3;						// 満車台数３
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CarCnt3;						// 現在台数３
//	p_NtDat->ParkData.CarFullFlag	= (uchar)0;								// 台数・満空状態フラグ
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->PascarCnt;				// 定期車両カウント
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->Full[0];						// 駐車1満空状態
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->Full[1];						// 駐車2満空状態
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->Full[2];						// 駐車3満空状態
//
//	// 台数管理追番 ------------------------
//	p_NtDat->ParkData.CarCntInfo.CarCntYear		= p_RcptDat->CarCntInfo.CarCntYear;	// 年
//	p_NtDat->ParkData.CarCntInfo.CarCntMon		= p_RcptDat->CarCntInfo.CarCntMon;	// 月
//	p_NtDat->ParkData.CarCntInfo.CarCntDay		= p_RcptDat->CarCntInfo.CarCntDay;	// 日
//	p_NtDat->ParkData.CarCntInfo.CarCntHour		= p_RcptDat->CarCntInfo.CarCntHour;	// 時
//	p_NtDat->ParkData.CarCntInfo.CarCntMin		= p_RcptDat->CarCntInfo.CarCntMin;	// 分
//	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= p_RcptDat->CarCntInfo.CarCntSeq;	// 追番
//	p_NtDat->ParkData.CarCntInfo.Reserve1		= p_RcptDat->CarCntInfo.Reserve1;	// 予備
//	// -------------------------------------
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH810100(S) 2020/07/09 車番チケットレス(#4531 仕様変更 台数管理追番を台数管理をしなくても付与する)
	// 台数管理追番 ------------------------
	p_NtDat->ParkData.CarCntInfo.CarCntYear		= p_RcptDat->CarCntInfo.CarCntYear;	// 年
	p_NtDat->ParkData.CarCntInfo.CarCntMon		= p_RcptDat->CarCntInfo.CarCntMon;	// 月
	p_NtDat->ParkData.CarCntInfo.CarCntDay		= p_RcptDat->CarCntInfo.CarCntDay;	// 日
	p_NtDat->ParkData.CarCntInfo.CarCntHour		= p_RcptDat->CarCntInfo.CarCntHour;	// 時
	p_NtDat->ParkData.CarCntInfo.CarCntMin		= p_RcptDat->CarCntInfo.CarCntMin;	// 分
	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= p_RcptDat->CarCntInfo.CarCntSeq;	// 追番
	p_NtDat->ParkData.CarCntInfo.Reserve1		= p_RcptDat->CarCntInfo.Reserve1;	// 予備
// MH810100(E) 2020/07/09 車番チケットレス(#4531 仕様変更 台数管理追番を台数管理をしなくても付与する)

	p_NtDat->PayCount	= CountSel( &p_RcptDat->Oiban);						// 精算or精算中止追い番
	p_NtDat->PayMethod	= (uchar)p_RcptDat->PayMethod;						// 精算方法
	if( p_RcptDat->PayClass == 4 ){
		// クレジット精算
		p_NtDat->PayClass	= (uchar)0;										// 処理区分
// MH810100(S) 2020/08/18 車番チケットレス(#4594 再精算でクレジット精算するとNT-NET事前精算データ(ID:56)の処理区分が5になる)
	}else if( p_RcptDat->PayClass == 5){
		// クレジット再精算
		p_NtDat->PayClass	= (uchar)1;										// 処理区分
// MH810100(E) 2020/08/18 車番チケットレス(#4594 再精算でクレジット精算するとNT-NET事前精算データ(ID:56)の処理区分が5になる)
	}else{
		p_NtDat->PayClass	= (uchar)p_RcptDat->PayClass;					// 処理区分
	}
	p_NtDat->PayMode	= (uchar)p_RcptDat->PayMode;						// 精算ﾓｰﾄﾞ(自動精算)
// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
//	p_NtDat->CMachineNo	= 0;												// 駐車券機械№
	p_NtDat->CMachineNo	= p_RcptDat->CMachineNo;							// 駐車券機械№
// MH810100(E) K.Onodera  2020/02/05 車番チケットレス(LCD_IF対応)

	if( p_NtDat->PayMethod != 5 ){
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		p_NtDat->FlapArea	= (ushort)(p_RcptDat->WPlace/10000);			// フラップシステム	区画		0～99
//		p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->WPlace%10000);			// 					車室番号	0～9999
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		p_NtDat->KakariNo	= p_RcptDat->KakariNo;							// 係員№
		p_NtDat->OutKind	= p_RcptDat->OutKind;							// 精算出庫
	}

	p_NtDat->CountSet	= p_RcptDat->CountSet;								// 在車カウントしない

	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->InPrev_ymdhms.In_Year	= (uchar)(p_RcptDat->TInTime.Year%100);	// 入庫年
		p_NtDat->InPrev_ymdhms.In_Mon	= p_RcptDat->TInTime.Mon;				// 入庫月
		p_NtDat->InPrev_ymdhms.In_Day	= p_RcptDat->TInTime.Day;				// 入庫日
		p_NtDat->InPrev_ymdhms.In_Hour	= p_RcptDat->TInTime.Hour;				// 入庫時
		p_NtDat->InPrev_ymdhms.In_Min	= p_RcptDat->TInTime.Min;				// 入庫分
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//		p_NtDat->InPrev_ymdhms.In_Sec	= 0;									// 入庫秒
		p_NtDat->InPrev_ymdhms.In_Sec	= p_RcptDat->TInTime_Sec;				// 入庫秒
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）

// MH810100(S) K.Onodera  2020/01/29 車番チケットレス(再精算対応)
// MH810100(S) 2020/08/19 車番チケットレス(#4595 再精算でクレジット精算するとNT-NET事前精算データ(ID:56)の前回精算年月日時分秒が0になる)
//		if(( p_RcptDat->PayClass == 1 ) || ( p_RcptDat->PayClass == 3 )){	// 再精算or再精算中止
		if(( p_NtDat->PayClass == 1 ) || ( p_NtDat->PayClass == 3 )){	// 再精算or再精算中止
// MH810100(E) 2020/08/19 車番チケットレス(#4595 再精算でクレジット精算するとNT-NET事前精算データ(ID:56)の前回精算年月日時分秒が0になる)
			p_NtDat->InPrev_ymdhms.Prev_Year = (uchar)(p_RcptDat->BeforeTPayTime.Year%100);	// 前回精算年
			p_NtDat->InPrev_ymdhms.Prev_Mon	 = p_RcptDat->BeforeTPayTime.Mon;				// 前回精算月
			p_NtDat->InPrev_ymdhms.Prev_Day	 = p_RcptDat->BeforeTPayTime.Day;				// 前回精算日
			p_NtDat->InPrev_ymdhms.Prev_Hour = p_RcptDat->BeforeTPayTime.Hour;				// 前回精算時
			p_NtDat->InPrev_ymdhms.Prev_Min	 = p_RcptDat->BeforeTPayTime.Min;				// 前回精算分
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//			p_NtDat->InPrev_ymdhms.Prev_Sec	 = 0;											// 前回精算秒
			p_NtDat->InPrev_ymdhms.Prev_Sec	 = p_RcptDat->BeforeTPayTime_Sec;				// 前回精算秒
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
		}else{
// MH810100(E) K.Onodera  2020/01/29 車番チケットレス(再精算対応)
			// 前回精算年月日時分秒は0とする
			p_NtDat->InPrev_ymdhms.Prev_Year	= 0;							// 前回精算年
			p_NtDat->InPrev_ymdhms.Prev_Mon		= 0;							// 前回精算月
			p_NtDat->InPrev_ymdhms.Prev_Day		= 0;							// 前回精算日
			p_NtDat->InPrev_ymdhms.Prev_Hour	= 0;							// 前回精算時
			p_NtDat->InPrev_ymdhms.Prev_Min		= 0;							// 前回精算分
			p_NtDat->InPrev_ymdhms.Prev_Sec		= 0;							// 前回精算秒
// MH810100(S) K.Onodera  2020/01/29 車番チケットレス(再精算対応)
		}
// MH810100(E) K.Onodera  2020/01/29 車番チケットレス(再精算対応)
	}

	p_NtDat->ReceiptIssue	= p_RcptDat->ReceiptIssue;						// 領収証発行有無

	if( p_NtDat->PayMethod != 5 ){											// 更新精算以外
		p_NtDat->Syubet	= p_RcptDat->syu;									// 料金種別
		p_NtDat->Price	= p_RcptDat->WPrice;								// 駐車料金
	}

	p_NtDat->CashPrice	= p_RcptDat->WTotalPrice;							// 現金売上
	p_NtDat->InPrice	= p_RcptDat->WInPrice;								// 投入金額
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	if( p_RcptDat->FRK_RetMod == 0 ){
//		p_NtDat->ChgPrice	= (ushort)(p_RcptDat->WChgPrice + p_RcptDat->FRK_Return);	// 釣銭金額 + 振替過払い金
//	}else{
//		p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// 釣銭金額
//	}
	p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// 釣銭金額
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	p_NtDat->MoneyInOut.MoneyKind_In	= 0x1f;								// 金種有無	搭載金種の有無　投入金（10円,50円,100円,500円,1000円）
	p_NtDat->MoneyInOut.In_10_cnt		= (uchar)p_RcptDat->in_coin[0];		// 投入金枚数(10円)		0～255
	p_NtDat->MoneyInOut.In_50_cnt		= (uchar)p_RcptDat->in_coin[1];		// 投入金枚数(50円)		0～255
	p_NtDat->MoneyInOut.In_100_cnt		= (uchar)p_RcptDat->in_coin[2];		// 投入金枚数(100円)	0～255
	p_NtDat->MoneyInOut.In_500_cnt		= (uchar)p_RcptDat->in_coin[3];		// 投入金枚数(500円)	0～255
	p_NtDat->MoneyInOut.In_1000_cnt		= (uchar)p_RcptDat->in_coin[4];		// 投入金枚数(1000円)	0～255
	if(p_RcptDat->f_escrow) {												// エスクロ紙幣戻しあり
		++p_NtDat->MoneyInOut.In_1000_cnt;									// エスクロ紙幣戻し分を投入枚数に加算
	}

	p_NtDat->MoneyInOut.MoneyKind_Out	= 0x1f;								// 金種有無	搭載金種の有無　払出金（10円,50円,100円,500円,1000円）
	p_NtDat->MoneyInOut.Out_10_cnt		= (uchar)p_RcptDat->out_coin[0];	// 払出金枚数(10円)		0～255
	p_NtDat->MoneyInOut.Out_50_cnt		= (uchar)p_RcptDat->out_coin[1];	// 払出金枚数(50円)		0～255
	p_NtDat->MoneyInOut.Out_100_cnt		= (uchar)p_RcptDat->out_coin[2];	// 払出金枚数(100円)	0～255
	p_NtDat->MoneyInOut.Out_500_cnt		= (uchar)p_RcptDat->out_coin[3];	// 払出金枚数(500円)	0～255
	p_NtDat->MoneyInOut.Out_1000_cnt	= (uchar)p_RcptDat->f_escrow;		// 払出金枚数(1000円)	0～255

	p_NtDat->HaraiModoshiFusoku	= (ushort)p_RcptDat->WFusoku;				// 払戻不足額	0～9999
	if( p_RcptDat->FRK_Return && p_RcptDat->FRK_RetMod ){
		p_NtDat->CardFusokuType = p_RcptDat->FRK_RetMod;					// 払出不足媒体種別(現金以外)
		p_NtDat->CardFusokuTotal = p_RcptDat->FRK_Return;					// 払出不足額(現金以外)
	}
	p_NtDat->SaleParkingNo = (ulong)CPrmSS[S_SYS][1];						// 売上先駐車場№(0～999999   ※特注用（標準では基本駐車場№をセット）

// GG132000(S) 精算完了／中止時のNT-NET通信の精算データにデータ項目を追加
	p_NtDat->MaxChargeApplyFlg		= ntNet_56_SaveData.MaxFeeApplyFlg;		// 最大料金適用フラグ
	p_NtDat->MaxChargeApplyCnt		= ntNet_56_SaveData.MaxFeeApplyCnt;		// 最大料金適用回数
	p_NtDat->MaxChargeSettingFlg	= ntNet_56_SaveData.MaxFeeSettingFlg;	// 最大料金設定有無
// GG132000(E) 精算完了／中止時のNT-NET通信の精算データにデータ項目を追加

	p_NtDat->PassCheck = p_RcptDat->PassCheck;								// アンチパスチェック

	if( p_RcptDat->teiki.ParkingNo ) {
		p_NtDat->ParkingNo	= p_RcptDat->teiki.ParkingNo;								// 定期券駐車場№
// MH810100(S) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
//		p_NtDat->Media[0].MediaKind	= (ushort)(p_RcptDat->teiki.pkno_syu + 2);			// 精算媒体情報１　種別(メイン媒体)	0～99
//		intoasl(p_NtDat->Media[0].MediaCardNo, p_RcptDat->teiki.id, 5);					// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		intoasl(p_NtDat->Media[0].MediaCardInfo, p_RcptDat->teiki.syu, 2);				// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		// 精算媒体情報2（サブ媒体）は オール０とする
	}
	if( p_RcptDat->MediaKind1 ){	// 精算媒体情報1種別あり？
		/************************************/
		/*	精算媒体情報1 = t_MediaInfo		*/
		/************************************/
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
		p_NtDat->Media[0].MediaKind	= p_RcptDat->MediaKind1;								// 精算媒体情報1種別(メイン媒体) = 駐車券(101)/定期券(102)
		memset( &EncMediaInfo, 0, sizeof( t_MediaInfoCre2 ) );
		memcpy( EncMediaInfo.MediaCardNo, p_RcptDat->MediaCardNo1,							// カード番号(駐車券:駐車券機械№(入庫機械№)(上位3桁) + 駐車券番号(下位6桁)
													sizeof( EncMediaInfo.MediaCardNo ) );	//            定期券:定期券ID(5桁))
		if( p_RcptDat->MediaKind1 == CARD_TYPE_PASS ){	// 精算媒体情報1種別 = 定期券(102)？
			intoas( EncMediaInfo.MediaCardInfo,p_RcptDat->teiki.syu, 2 );					// カード情報(定期券種別(2桁、1～15))
		}
// MH810100(S) m.saito 2020/05/15 車番チケットレス(#4166 精算媒体種別101,102,203は暗号化しない)対応
//// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:380)対応
////		AesCBCEncrypt( EncMediaInfo.MediaCardNo, sizeof( t_MediaInfoCre2 ) );				// AES 暗号化
//		AesCBCEncrypt( (unsigned char*)&EncMediaInfo, sizeof( t_MediaInfoCre2 ) );			// AES 暗号化
//// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:380)対応
// MH810100(E) m.saito 2020/05/14 車番チケットレス(#4166 精算媒体種別101,102,203は暗号化しない)対応
// MH810100(S) m.saito 2020/05/14 車番チケットレス(#4166 精算データの設定内容が不正)の対応
//// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:381,382)対応
////		memcpy( p_NtDat->Media[0].MediaCardNo, EncMediaInfo.MediaCardNo,					// カード番号(30Byte) + カード情報(16Byte) + 予約(2Byte)
//		memcpy( &p_NtDat->Media[0], &EncMediaInfo,											// カード番号(30Byte) + カード情報(16Byte) + 予約(2Byte)
//// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:381,382)対応
		memcpy( p_NtDat->Media[0].MediaCardNo, EncMediaInfo.MediaCardNo,					// カード番号(30Byte) + カード情報(16Byte) + 予約(2Byte)
// MH810100(E) m.saito 2020/05/14 車番チケットレス(#4166 精算データの設定内容が不正)の対応
													 sizeof( t_MediaInfoCre2 ) );
	}
	if( p_RcptDat->MediaKind2 == CARD_TYPE_CAR_NUM ){	// 精算媒体情報種別 = 車番(203)あり？
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;				// 暗号化方式
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;				// 暗号鍵番号
		p_NtDat->Media[1].MediaKind	= CARD_TYPE_CAR_NUM;						// 精算媒体情報種別(サブ媒体) = 車番(203)
		memset( &EncMediaInfo, 0, sizeof( t_MediaInfoCre2 ) );
		memcpy( EncMediaInfo.MediaCardNo, p_RcptDat->MediaCardNo2,				// カード番号(陸運支局名(2Byte) + 分類番号(3Byte)
										sizeof( EncMediaInfo.MediaCardNo ) );	//            + 用途文字(2Byte) + 一連指定番号(4Byte))
// MH810100(S) m.saito 2020/05/15 車番チケットレス(#4166 精算媒体種別101,102,203は暗号化しない)対応
//// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:383)対応
////		AesCBCEncrypt( EncMediaInfo.MediaCardNo, sizeof( t_MediaInfoCre2 ) );	// AES 暗号化
//		AesCBCEncrypt( (unsigned char*)&EncMediaInfo, sizeof( t_MediaInfoCre2 ) );	// AES 暗号化
//// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:383)対応
// MH810100(E) m.saito 2020/05/15 車番チケットレス(#4166 精算媒体種別101,102,203は暗号化しない)対応
// MH810100(S) m.saito 2020/05/14 車番チケットレス(#4166 精算データの設定内容が不正)の対応
//// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:384,385)対応
////		memcpy( p_NtDat->Media[1].MediaCardNo, EncMediaInfo.MediaCardNo,		// カード番号(30Byte) + カード情報(16Byte) + 予約(2Byte)
//		memcpy( &p_NtDat->Media[1], &EncMediaInfo,									// カード番号(30Byte) + カード情報(16Byte) + 予約(2Byte)
//// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:384,385)対応
		memcpy( p_NtDat->Media[1].MediaCardNo, EncMediaInfo.MediaCardNo,		// カード番号(30Byte) + カード情報(16Byte) + 予約(2Byte)
// MH810100(E) m.saito 2020/05/14 車番チケットレス(#4166 精算データの設定内容が不正)の対応
												 sizeof( t_MediaInfoCre2 ) );
// MH810100(E) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	}

// GG129001(S) データ保管サービス対応（適格請求書情報有無をセットする）
	// データ保管サービス対応以降は必ず1をセットする
	p_NtDat->Invoice = 1;
// GG129001(E) データ保管サービス対応（適格請求書情報有無をセットする）
// GG129001(S) データ保管サービス対応（登録番号をセットする）
	memcpy(p_NtDat->RegistNum, p_RcptDat->RegistNum, sizeof(p_RcptDat->RegistNum));
// GG129001(E) データ保管サービス対応（登録番号をセットする）
// GG129001(S) データ保管サービス対応（課税対象額をセットする）
	p_NtDat->TaxPrice = p_RcptDat->WTaxPrice;
// GG129001(E) データ保管サービス対応（課税対象額をセットする）
// GG129001(S) データ保管サービス対応（適用税率をセットする）
	p_NtDat->TaxRate = p_RcptDat->WTaxRate;
// GG129001(E) データ保管サービス対応（適用税率をセットする）

// MH810103 GG119202(S) 決済精算中止時は精算データにみなし決済情報をセットしない
//	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 ){	// 電子決済種別 Suica:1, Edy:2
// MH810105(S) MH364301 QRコード決済対応
//	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 &&	// 電子決済種別 Suica:1, Edy:2
//		p_NtDat->PayClass != 2 && p_NtDat->PayClass != 3 ){	// 精算中止、再精算中止ではない
	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 ){	// 電子決済種別 Suica:1, Edy:2
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(E) 決済精算中止時は精算データにみなし決済情報をセットしない
		switch (p_RcptDat->Electron_data.Suica.e_pay_kind) {
		case EC_EDY_USED:
			p_NtDat->CardKind = (ushort)3;
			break;
		case EC_NANACO_USED:
			p_NtDat->CardKind = (ushort)6;
			break;
		case EC_WAON_USED:
			p_NtDat->CardKind = (ushort)5;
			break;
		case EC_SAPICA_USED:
			p_NtDat->CardKind = (ushort)7;
			break;
		case EC_KOUTSUU_USED:
			p_NtDat->CardKind = (ushort)CARD_EC_KOUTSUU_USED_TEMP;		// 既存と区別するための値を一時的にセット
			break;
		case EC_ID_USED:
			p_NtDat->CardKind = (ushort)12;
			break;
		case EC_QUIC_PAY_USED:
			p_NtDat->CardKind = (ushort)13;
			break;
		case EC_CREDIT_USED:
			p_NtDat->CardKind = (ushort)1;
			break;
		case EC_HOUJIN_USED:
			p_NtDat->CardKind = (ushort)1;
			break;
// MH810105(S) MH364301 PiTaPa対応
		case EC_PITAPA_USED:
			p_NtDat->CardKind = (ushort)4;
			break;
// MH810105(E) MH364301 PiTaPa対応
// MH810105(S) MH364301 QRコード決済対応
		case EC_QR_USED:
			p_NtDat->CardKind = (ushort)16;
			break;
// MH810105(E) MH364301 QRコード決済対応
		default:		// 2=交通系電子マネー(Suica,PASMO,ICOCA等)
			p_NtDat->CardKind = (ushort)2;
			break;
		}
	}

	switch( p_NtDat->CardKind ){							// カード決済区分	0=なし、2=交通系電子マネー(Suica,PASMO,ICOCA等)
	case 1:
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
		memset( &t_Credit, 0, sizeof( t_Credit ) );
// MH810105(S) MH364301 QRコード決済対応
//		t_Credit.amount = (ulong)p_RcptDat->credit.pay_ryo;									// 決済額
//// MH321800(S) NT-NET精算データ仕様変更
////		memcpy( &t_Credit.card_no[0], &p_RcptDat->credit.card_no[0], sizeof( t_Credit.card_no ) );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
//// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//		if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
//			p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
//			// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.を0x20h埋めする。
//			// [*]印字みなし決済と同様の会員Noをセットする。
//			memset( &t_Credit.card_no[0], 0x20, sizeof(t_Credit.card_no) );
//		}
//		else {
//// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//		// みなし決済時はスペース埋めする
//		memcpyFlushLeft( &t_Credit.card_no[0], (uchar *)&p_RcptDat->credit.card_no[0],
//						sizeof(t_Credit.card_no), sizeof(p_RcptDat->credit.card_no) );		// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
//// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//		}
//// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//// MH321800(E) NT-NET精算データ仕様変更
//		// 会員Noの上7桁目～下5桁目以外で'*'が格納されていたら'0'に置換
//		change_CharInArray( &t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '*', '0' );
//// MH321800(S) NT-NET精算データ仕様変更
////		// みなし決済時はNULL(0x0)で埋める
////		memcpyFlushLeft2( &t_Credit.cct_num[0], (uchar *)&p_RcptDat->credit.CCT_Num[0],
////						sizeof(t_Credit.cct_num), sizeof(p_RcptDat->credit.CCT_Num));		// 端末識別番号
////		memcpy( &t_Credit.kid_code[0], &p_RcptDat->credit.kid_code[0], sizeof(t_Credit.kid_code));// KID コード
//		// みなし決済時はスペース埋めする
//		memcpyFlushLeft( &t_Credit.cct_num[0], (uchar *)&p_RcptDat->credit.CCT_Num[0],
//						sizeof(t_Credit.cct_num), sizeof(p_RcptDat->credit.CCT_Num));		// 端末識別番号
//		// みなし決済時はスペース埋めする
//		memcpyFlushLeft( &t_Credit.kid_code[0], (uchar *)&p_RcptDat->credit.kid_code[0],
//						sizeof(t_Credit.kid_code), sizeof(p_RcptDat->credit.kid_code) );	// KIDコード
//// MH321800(E) NT-NET精算データ仕様変更
//		t_Credit.app_no = p_RcptDat->credit.app_no;											// 承認番号
//		t_Credit.center_oiban = 0;															// ｾﾝﾀｰ処理追番(センターは0固定)
//		memcpyFlushLeft( &t_Credit.ShopAccountNo[0], (uchar*)&p_RcptDat->credit.ShopAccountNo[0],
//						sizeof(t_Credit.ShopAccountNo), sizeof(p_RcptDat->credit.ShopAccountNo) );	// 加盟店取引番号
//		t_Credit.slip_no = p_RcptDat->credit.slip_no;										// 伝票番号
		// 決済端末区分
		p_NtDat->PayTerminalClass = 1;

		// 取引ステータス
		if (p_RcptDat->Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
			// 未了支払不明
			p_NtDat->Transactiontatus = 3;
		}
		else if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			// みなし決済
			p_NtDat->Transactiontatus = 1;
		}
		else {
			// 支払
			p_NtDat->Transactiontatus = 0;
		}

		// 処理区分＝２、３で決済精算中止の場合
		if (p_RcptDat->PayClass == 2 || p_RcptDat->PayClass == 3) {
			// 利用金額
			t_Credit.amount = 0;
			// カード番号
			memset(&t_Credit.card_no[0], 0x20, sizeof(t_Credit.card_no));
			// 端末識別番号
			memset(&t_Credit.cct_num[0], 0x20, sizeof(t_Credit.cct_num));
			// KIDコード
			memset(&t_Credit.kid_code[0], 0x20, sizeof(t_Credit.kid_code));
			// 承認番号
			t_Credit.app_no = 0;
			// 伝票番号
			t_Credit.slip_no = 0;
		}
		else {
			// 利用金額
			t_Credit.amount = p_RcptDat->credit.pay_ryo;
			// カード番号
			if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
				p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
				// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.を0x20h埋めする。
				// [*]印字みなし決済と同様の会員Noをセットする。
				memset( &t_Credit.card_no[0], 0x20, sizeof(t_Credit.card_no) );
			}
			else {
				memcpyFlushLeft(&t_Credit.card_no[0],
								(uchar*)&p_RcptDat->credit.card_no[0],
								sizeof(t_Credit.card_no),
								sizeof(p_RcptDat->credit.card_no));
			}
			change_CharInArray( &t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '*', '0' );
			// 端末識別番号
			memcpyFlushLeft(&t_Credit.cct_num[0],
							(uchar*)&p_RcptDat->credit.CCT_Num[0],
							sizeof(t_Credit.cct_num),
							sizeof(p_RcptDat->credit.CCT_Num));
			// KIDコード
			memcpyFlushLeft(&t_Credit.kid_code[0],
							(uchar*)&p_RcptDat->credit.kid_code[0],
							sizeof(t_Credit.kid_code),
							sizeof(p_RcptDat->credit.kid_code));
			// 承認番号
			t_Credit.app_no = p_RcptDat->credit.app_no;
			// 伝票番号
			t_Credit.slip_no = p_RcptDat->credit.slip_no;
		}

		// センタ処理追番
		t_Credit.center_oiban = 0;
		// 加盟店取引番号
		memset(t_Credit.ShopAccountNo, 0x20, sizeof(t_Credit.ShopAccountNo));
// MH810105(E) MH364301 QRコード決済対応
		
		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );							// AES 暗号化
		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// 決済情報
		break;
	case 2:				// 交通系電子マネー(SX-20)
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
		t_EMoney.amount = p_RcptDat->Electron_data.Suica.pay_ryo;							// 電子決済精算情報　決済額
		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
		memcpy( &t_EMoney.card_id[0], &p_RcptDat->Electron_data.Suica.Card_ID[0],
				sizeof(p_RcptDat->Electron_data.Suica.Card_ID) );							// 電子決済精算情報　ｶｰﾄﾞID (Ascii 16桁)
		t_EMoney.card_zangaku = p_RcptDat->Electron_data.Suica.pay_after;					// 電子決済精算情報　決済後残高
		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );							// AES 暗号化
		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// 決済情報
		break;
	case 3:				// Edy
// MH810105(S) MH364301 PiTaPa対応
	case 4:				// PiTaPa
// MH810105(E) MH364301 PiTaPa対応
	case 5:				// WAON
	case 6:				// nanaco
	case 7:				// SAPICA
	case CARD_EC_KOUTSUU_USED_TEMP:		// 交通系 (既存と区別するため一時的にセットした値)
	case 12:			// iD
	case 13:			// QUICPay
		if (p_NtDat->CardKind == CARD_EC_KOUTSUU_USED_TEMP) {
		// 交通系ICカードのカード決済区分に正規の値をセット
			p_NtDat->CardKind = 2;
		}
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
// MH810105(S) MH364301 QRコード決済対応
//		t_EMoney.amount = p_RcptDat->Electron_data.Ec.pay_ryo;								// 電子決済精算情報　決済額
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//		if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
//			p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
//			// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.をカード桁数分0x30埋めし左詰めでZZする。
//			// [*]印字みなし決済と同様の会員Noをセットする。
//			// それぞれのカード桁数分0x30埋め
//			memset( &t_EMoney.card_id[0], 0x30, (size_t)(EcBrandEmoney_Digit[p_RcptDat->Electron_data.Suica.e_pay_kind - EC_USED]) );
//			// 左詰めでZZ
//			memset( &t_EMoney.card_id[0], 'Z', 2 );
//		}
//		else {
//// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//		memcpy( &t_EMoney.card_id[0], &p_RcptDat->Electron_data.Ec.Card_ID[0],
//				sizeof(p_RcptDat->Electron_data.Ec.Card_ID) );								// 電子決済精算情報　ｶｰﾄﾞID (Ascii 20桁)
//// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//		}
//// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//		t_EMoney.card_zangaku = p_RcptDat->Electron_data.Ec.pay_after;						// 電子決済精算情報　決済後残高
//// MH810103 GG119202(S) 精算データの決済情報に問い合わせ番号はセットしない
//		// 決済情報に問合せ番号をセットする仕様はフォーマットRev18で対応
////// GG119200(S) 電子マネー対応
//////		// みなし決済時はNULL(0x0)で埋める
//////		memcpyFlushLeft2( &t_EMoney.inquiry_num[0], &ntNet_152_SaveData.e_inquiry_num[0],
//////				sizeof(ntNet_152_SaveData.e_inquiry_num), sizeof(t_EMoney.inquiry_num) );	// 問合せ番号
////		// みなし決済時はNULL(0x0)で埋める
////		memcpyFlushLeft( &t_EMoney.inquiry_num[0], &p_RcptDat->Electron_data.Ec.inquiry_num[0],
////						sizeof(t_EMoney.inquiry_num), sizeof(p_RcptDat->Electron_data.Ec.inquiry_num) );	// 問合せ番号
////// GG119200(E) 電子マネー対応
//// MH810103 GG119202(E) 精算データの決済情報に問い合わせ番号はセットしない
		// 決済端末区分
		p_NtDat->PayTerminalClass = 1;

		// 取引ステータス
		if (p_RcptDat->Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
			// 未了支払不明
			p_NtDat->Transactiontatus = 3;
		}
		else if (p_RcptDat->EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			// 未了支払済み
			p_NtDat->Transactiontatus = 2;
		}
		else if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			// みなし決済
			p_NtDat->Transactiontatus = 1;
		}
		else {
			// 支払
			p_NtDat->Transactiontatus = 0;
		}

		// 処理区分＝２、３で決済精算中止の場合
		if (p_RcptDat->PayClass == 2 || p_RcptDat->PayClass == 3) {
			// 利用金額
			t_EMoney.amount = 0;
			// カード番号
			memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
			memset(&t_EMoney.card_id[0], 0x30, (size_t)(EcBrandEmoney_Digit[p_RcptDat->Electron_data.Ec.e_pay_kind - EC_USED]));
			memset(&t_EMoney.card_id[0], 'Z', 2);
			// カード残額
			t_EMoney.card_zangaku = 0;
			// 問い合わせ番号
			memset(&t_EMoney.inquiry_num[0], 0x20, sizeof(t_EMoney.inquiry_num));
			// 承認番号
			t_EMoney.approbal_no = 0;
		}
		else {
			// 利用金額
			t_EMoney.amount = p_RcptDat->Electron_data.Ec.pay_ryo;

			// カード番号
			if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
				p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
				// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.をカード桁数分0x30埋めし左詰めでZZする。
				// [*]印字みなし決済と同様の会員Noをセットする。
				memset(	&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id) );
				memset( &t_EMoney.card_id[0], 0x30, (size_t)(EcBrandEmoney_Digit[p_RcptDat->Electron_data.Suica.e_pay_kind - EC_USED]) );
				memset( &t_EMoney.card_id[0], 'Z', 2 );
			}
			else{
				memcpyFlushLeft(&t_EMoney.card_id[0],
								(uchar*)&p_RcptDat->Electron_data.Ec.Card_ID[0],
								sizeof(t_EMoney.card_id),
								sizeof(p_RcptDat->Electron_data.Ec.Card_ID));
			}
			// カード残額
			t_EMoney.card_zangaku = p_RcptDat->Electron_data.Ec.pay_after;
			// 問い合わせ番号
			memcpyFlushLeft(&t_EMoney.inquiry_num[0],
							(uchar*)&p_RcptDat->Electron_data.Ec.inquiry_num[0],
							sizeof(t_EMoney.inquiry_num),
							sizeof(p_RcptDat->Electron_data.Ec.inquiry_num));

			// 承認番号
			switch (p_RcptDat->Electron_data.Ec.e_pay_kind) {
			case EC_ID_USED:
				t_EMoney.approbal_no =
							astoinl(p_RcptDat->Electron_data.Ec.Brand.Id.Approval_No, 7);
				break;
			case EC_QUIC_PAY_USED:
				t_EMoney.approbal_no =
							astoinl(p_RcptDat->Electron_data.Ec.Brand.Quickpay.Approval_No, 7);
				break;
			case EC_PITAPA_USED:
				// オフライン承認決済時はオール'*'のため、'0'へ変換する
				if (p_RcptDat->Electron_data.Ec.Brand.Pitapa.Approval_No[0] == '*') {
					t_EMoney.approbal_no = 0;
				}
				else {
					t_EMoney.approbal_no =
								astoinl(p_RcptDat->Electron_data.Ec.Brand.Pitapa.Approval_No, 8);
				}
				break;
			default:
				t_EMoney.approbal_no = 0;
				break;
			}
		}
// MH810105(E) MH364301 QRコード決済対応
		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );							// AES 暗号化
		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// 決済情報
		break;
// MH810105(S) MH364301 QRコード決済対応
	case 16:
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// 暗号化方式
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// 暗号鍵番号
		memset( &t_QR, 0, sizeof( t_QR ) );
		// 決済端末区分
		p_NtDat->PayTerminalClass = 1;

		// 取引ステータス
		if (p_RcptDat->Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
			// 未了支払不明
			p_NtDat->Transactiontatus = 3;
		}
		else if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			// みなし決済
			p_NtDat->Transactiontatus = 1;
		}
		else {
			// 支払
			p_NtDat->Transactiontatus = 0;
		}

		// 処理区分＝２、３で決済精算中止の場合
		if (p_RcptDat->PayClass == 2 || p_RcptDat->PayClass == 3) {
			// 利用金額
			t_QR.amount = 0;
			// Mch取引番号
			memset(&t_QR.MchTradeNo[0], 0x20, sizeof(t_QR.MchTradeNo));

			if (p_RcptDat->Electron_data.Ec.E_Flag.BIT.deemSettleCancal == 1) {
				// 中止であるが決済結果を受信できなかったので
				// それぞれの桁数分0x20埋めし、支払端末IDは13桁、取引番号は15桁、0x30埋めする
				// 支払端末ID
				memset(&t_QR.PayTerminalNo, 0x20, sizeof(t_QR.PayTerminalNo));
				memset(&t_QR.PayTerminalNo, 0x30, 13);
				// 取引番号
				memset(&t_QR.DealNo, 0x20, sizeof(t_QR.DealNo));
				memset(&t_QR.DealNo, 0x30, 15);
			}
			else {
				// 支払端末ID
				memcpyFlushLeft(&t_QR.PayTerminalNo[0],
								&p_RcptDat->Electron_data.Ec.Brand.Qr.PayTerminalNo[0],
								sizeof(t_QR.PayTerminalNo),
								sizeof(p_RcptDat->Electron_data.Ec.Brand.Qr.PayTerminalNo));
				// 取引番号
				memcpyFlushLeft(&t_QR.DealNo[0],
								&p_RcptDat->Electron_data.Ec.inquiry_num[0],
								sizeof(t_QR.DealNo),
								sizeof(p_RcptDat->Electron_data.Ec.inquiry_num));
			}
		}
		else {
			// 利用金額
			t_QR.amount = p_RcptDat->Electron_data.Ec.pay_ryo;

			if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
				// Mch取引番号
				memset(&t_QR.MchTradeNo[0], 0x20, sizeof(t_QR.MchTradeNo));
				// 支払端末ID
				memcpyFlushLeft(&t_QR.PayTerminalNo[0],
								&p_RcptDat->Electron_data.Ec.Brand.Qr.PayTerminalNo[0],
								sizeof(t_QR.PayTerminalNo),
								sizeof(p_RcptDat->Electron_data.Ec.Brand.Qr.PayTerminalNo));
				// 取引番号
				memcpyFlushLeft(&t_QR.DealNo[0],
								&p_RcptDat->Electron_data.Ec.inquiry_num[0],
								sizeof(t_QR.DealNo),
								sizeof(p_RcptDat->Electron_data.Ec.inquiry_num));
			}
			else{
				// Mch取引番号
				memcpyFlushLeft(&t_QR.MchTradeNo[0],
								&p_RcptDat->Electron_data.Ec.Brand.Qr.MchTradeNo[0],
								sizeof(t_QR.MchTradeNo),
								sizeof(p_RcptDat->Electron_data.Ec.Brand.Qr.MchTradeNo));
				// 支払端末ID
				memset(&t_QR.PayTerminalNo, 0x20, sizeof(t_QR.PayTerminalNo));
// MH810105(S) MH364301 QRコード決済対応 #6381 QRコード決済で通常精算時の精算データで決済情報の支払端末ID、取引番号が 0となってしまう
//				memset(&t_QR.PayTerminalNo, 0x30, 13);
// MH810105(E) MH364301 QRコード決済対応 #6381 QRコード決済で通常精算時の精算データで決済情報の支払端末ID、取引番号が 0となってしまう
				// 取引番号
				memset(&t_QR.DealNo, 0x20, sizeof(t_QR.DealNo));
// MH810105(S) MH364301 QRコード決済対応 #6381 QRコード決済で通常精算時の精算データで決済情報の支払端末ID、取引番号が 0となってしまう
//				memset(&t_QR.DealNo, 0x30, 15);
// MH810105(E) MH364301 QRコード決済対応 #6381 QRコード決済で通常精算時の精算データで決済情報の支払端末ID、取引番号が 0となってしまう
			}
		}
		// 決済ブランド
		t_QR.PayKind = p_RcptDat->Electron_data.Ec.Brand.Qr.PayKind;

		AesCBCEncrypt( (uchar *)&t_QR, sizeof( t_QR ) );									// AES 暗号化
		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_QR, sizeof( t_QR ) );			// 決済情報
		break;
// MH810105(E) MH364301 QRコード決済対応

	default:
		break;
	}

	p_NtDat->TotalSale		= (ulong)0;										// 合計金額(物販)
	p_NtDat->DeleteSeq		= (ulong)0;										// 削除データ追番

// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6590 遠隔精算時、NT-NETの精算データの項目「車番情報有無」が車番情報ありになる [共通改善項目 No1532]
// // GG124100(S) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
// 	p_NtDat->CarNumInfoFlg	= (uchar)1;										// 車番情報有無
// 
// 	// 車番情報
// 	memcpy(&EncShabanInfo, &p_RcptDat->ShabanInfo, sizeof(SHABAN_INFO));
// 	AesCBCEncrypt((unsigned char *)&EncShabanInfo, sizeof(EncShabanInfo));	// AES 暗号化
// 	memcpy(&p_NtDat->ShabanInfo, &EncShabanInfo, sizeof(SHABAN_INFO));
// // GG124100(E) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
	// 車番情報有無
	if ( p_RcptDat->ShabanInfo.LandTransOfficeName[0] ||	// 陸運支局名あり
		 p_RcptDat->ShabanInfo.ClassNum[0] ||				// 分類番号あり
		 p_RcptDat->ShabanInfo.UsageCharacter[0] ||			// 用途文字あり
		 p_RcptDat->ShabanInfo.SeqDesignNumber[0] ) {		// 一連指定番号あり
		p_NtDat->CarNumInfoFlg = (uchar)1;
	}

	// 車番情報
	if ( p_NtDat->CarNumInfoFlg ) {
		memcpy(&EncShabanInfo, &p_RcptDat->ShabanInfo, sizeof(SHABAN_INFO));
		AesCBCEncrypt((unsigned char *)&EncShabanInfo, sizeof(EncShabanInfo));	// AES 暗号化
		memcpy(&p_NtDat->ShabanInfo, &EncShabanInfo, sizeof(SHABAN_INFO));
	}
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6590 遠隔精算時、NT-NETの精算データの項目「車番情報有無」が車番情報ありになる [共通改善項目 No1532]

	for( i = j = 0; i < WTIK_USEMAX; i++ ){									// 精算中止以外の割引情報コピー
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&					// 割引種別あり
// 		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// 精算中止割引情報でない
// 		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
// 		    if( p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA以外(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO以外(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY以外	(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_WAON_1 &&		// WAON以外	(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_WAON_2 &&		//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SAPICA_1 &&		// SAPICA以外	(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SAPICA_2 &&		//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_NANACO_1 &&		// NANACO以外	(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_NANACO_2 &&		//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ID_1 &&			// iD以外	(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ID_2 &&			//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_QUICPAY_1 &&	// QUICPay以外	(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_QUICPAY_2 &&	//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA以外(番号)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card以外(番号)
// // MH810100(S) 2020/06/12 #4199 【連動評価指摘事項】NT-NET精算データでの割引の差分格納
// //		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_2 )		//			(支払額・残額)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_2 &&		//			(支払額・残額)
// 		    	!((( p_RcptDat->DiscountData[i].DiscSyu == NTNET_SHOP_DISC_TIME ) ||	// 割引済み買物割引以外
// 		    	   ( p_RcptDat->DiscountData[i].DiscSyu == NTNET_SHOP_DISC_AMT )) &&
// 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) &&
// // MH810100(E) 2020/06/12 #4199 【連動評価指摘事項】NT-NET精算データでの割引の差分格納
// // MH810100(S) K.Onodera  2020/01/27 車番チケットレス(割引済み対応)
// 		    	!((( p_RcptDat->DiscountData[i].DiscSyu == NTNET_SVS_T ) ||	// 割引済みサービス券以外
// 		    	   ( p_RcptDat->DiscountData[i].DiscSyu == NTNET_SVS_M )) &&
// 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) &&
// 		    	!((( p_RcptDat->DiscountData[i].DiscSyu == NTNET_KAK_T ) ||	// 割引済み店割引以外
// 		    	   ( p_RcptDat->DiscountData[i].DiscSyu == NTNET_KAK_M )) &&
// // MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(NT-NET精算データの変更)
// // 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) )
// 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) &&
// 		    	!((( p_RcptDat->DiscountData[i].DiscSyu == NTNET_TKAK_T ) ||	// 割引済み多店舗割引以外
// 		    	   ( p_RcptDat->DiscountData[i].DiscSyu == NTNET_TKAK_M )) &&
// 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) )
// // MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(NT-NET精算データの変更)
// // MH810100(E) K.Onodera  2020/01/27 車番チケットレス(割引済み対応)
// GG124100(S) R.Endo 2022/08/04 車番チケットレス3.0 #6522 割引額が0円の割引がNT-NET精算データの割引情報にセットされない
// 		// 「割引種別」は割引計算時にチェック済みのため「今回使用した割引金額」の有無のみチェックする
// 		if ( p_RcptDat->DiscountData[i].Discount > 0 ) {
		// 「割引種別」は割引計算時にチェック済みのためここではチェックしない
		if ( p_RcptDat->DiscountData[i].Discount ||		// 今回使用した割引金額
			 p_RcptDat->DiscountData[i].DiscCount ) {	// 今回使用した枚数
// GG124100(E) R.Endo 2022/08/04 車番チケットレス3.0 #6522 割引額が0円の割引がNT-NET精算データの割引情報にセットされない
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
		    {
				p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DiscountData[i].ParkingNo;						// 駐車場No.
				p_NtDat->SDiscount[j].Kind		= p_RcptDat->DiscountData[i].DiscSyu;						// 割引種別
				p_NtDat->SDiscount[j].Group		= p_RcptDat->DiscountData[i].DiscNo;						// 割引区分
				p_NtDat->SDiscount[j].Callback	= p_RcptDat->DiscountData[i].DiscCount;						// 回収枚数
				p_NtDat->SDiscount[j].Amount	= p_RcptDat->DiscountData[i].Discount;						// 割引額
				p_NtDat->SDiscount[j].Info1		= p_RcptDat->DiscountData[i].DiscInfo1;						// 割引情報1
				if( p_NtDat->SDiscount[j].Kind != NTNET_PRI_W ){
					p_NtDat->SDiscount[j].Info2	= p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;	// 割引情報2
				}
				j++;
		    }
		}
	}
	for( i=0; i<DETAIL_SYU_MAX; i++ ){
		if( p_RcptDat->DetailData[i].DiscSyu ){
			p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DetailData[i].ParkingNo;					// 駐車場No.
			p_NtDat->SDiscount[j].Kind		= p_RcptDat->DetailData[i].DiscSyu;						// 割引種別
			p_NtDat->SDiscount[j].Group		= p_RcptDat->DetailData[i].uDetail.Common.DiscNo;		// 割引区分
			p_NtDat->SDiscount[j].Callback	= p_RcptDat->DetailData[i].uDetail.Common.DiscCount;	// 回収枚数
			p_NtDat->SDiscount[j].Amount	= p_RcptDat->DetailData[i].uDetail.Common.Discount;		// 割引額
			p_NtDat->SDiscount[j].Info1		= p_RcptDat->DetailData[i].uDetail.Common.DiscInfo1;	// 割引情報1
			p_NtDat->SDiscount[j].Info2		= p_RcptDat->DetailData[i].uDetail.Common.DiscInfo2;	// 割引情報2
			j++;
		}
	}
// MH810105(S) MH364301 QRコード決済対応
//// MH321800(S) 割引種別8000のセット方法を修正
//	// 決済リーダ接続あり
//	if (isEC_USE()) {
//// MH810103 GG119202(S) 決済精算中止時は精算データにみなし決済情報をセットしない
//		// 精算中止、再精算中止時は割引情報にセットしない
//		if (p_NtDat->PayClass != 2 && p_NtDat->PayClass != 3) {
//// MH810103 GG119202(E) 決済精算中止時は精算データにみなし決済情報をセットしない
//		if (p_RcptDat->credit.pay_ryo != 0) {
//			// クレジット決済
//			/* 空きｴﾘｱ検索 */
//			for (i = 0; (i < NTNET_DIC_MAX) &&
//					(0 != p_NtDat->SDiscount[i].ParkingNo); i++) {
//				;
//			}		/* データがある間				*/
//			if (i < NTNET_DIC_MAX) {
//				p_NtDat->SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];			// 駐車場No.
//				p_NtDat->SDiscount[i].Kind = NTNET_INQUIRY_NUM;				// 割引種別
//				// クレジット決済時は問い合わせ番号を受信しないので0x20埋めとする
//				memset(&p_NtDat->SDiscount[i].Group, 0x20, 16);
//			}
//		}
//		else if (p_RcptDat->Electron_data.Ec.pay_ryo != 0) {
//			// 電子マネー決済
//			// 割引情報にカードNo.と利用額はセットしない
//			/* 空きｴﾘｱ検索 */
//			for (i = 0; (i < NTNET_DIC_MAX) &&
//					(0 != p_NtDat->SDiscount[i].ParkingNo); i++) {
//				;
//			}		/* データがある間				*/
//			if (i < NTNET_DIC_MAX) {
//				p_NtDat->SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];			// 駐車場No.
//				p_NtDat->SDiscount[i].Kind = NTNET_INQUIRY_NUM;				// 割引種別
//// MH810103 GG119202(S) 電子マネー対応
////				memcpy(&p_NtDat->SDiscount[i].Group, p_RcptDat->Electron_data.Ec.inquiry_num, 16);
//				memcpyFlushLeft( (uchar *)&p_NtDat->SDiscount[i].Group, (uchar *)&p_RcptDat->Electron_data.Ec.inquiry_num[0],
//									16, sizeof( p_RcptDat->Electron_data.Ec.inquiry_num ) );	// 問い合わせ番号
//// MH810103 GG119202(E) 電子マネー対応
//			}
//		}
//// MH810103 GG119202(S) 決済精算中止時は精算データにみなし決済情報をセットしない
//		}
//// MH810103 GG119202(E) 決済精算中止時は精算データにみなし決済情報をセットしない
//	}
//// MH321800(E) 割引種別8000のセット方法を修正
	if ((p_RcptDat->PayClass == 2 || p_RcptDat->PayClass == 3) &&
		p_NtDat->Transactiontatus == 3) {
		// 決済精算中止
		/* 空きｴﾘｱ検索 */
		for (i = 0; (i < NTNET_DIC_MAX) &&
				(0 != p_NtDat->SDiscount[i].ParkingNo); i++) {
			;
		}		/* データがある間				*/
		if (i < NTNET_DIC_MAX) {
			p_NtDat->SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];	// 駐車場No.
			p_NtDat->SDiscount[i].Kind = NTNET_MIRYO_UNKNOWN;	// 割引種別
			p_NtDat->SDiscount[i].Group = p_NtDat->CardKind;	// 割引区分
			p_NtDat->SDiscount[i].Callback = 1;					// 使用枚数
			p_NtDat->SDiscount[i].Amount = 0;					// 割引額

			if (p_RcptDat->credit.pay_ryo != 0) {
				p_NtDat->SDiscount[i].Info1 = p_RcptDat->credit.pay_ryo;			// 割引情報1
			}
			else if (p_RcptDat->Electron_data.Ec.pay_ryo != 0) {
				p_NtDat->SDiscount[i].Info1 = p_RcptDat->Electron_data.Ec.pay_ryo;	// 割引情報1
			}

			p_NtDat->SDiscount[i].Info2 = 0;					// 割引情報2
		}
	}
// MH810105(E) MH364301 QRコード決済対応

// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
	NTNET_Edit_Data56_ParkingTkt_Pass(p_RcptDat, p_NtDat);
// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更

// MH810105(S) MH364301 QRコード決済対応
//	ret = sizeof( DATA_KIND_56_r17 );
	ret = sizeof( DATA_KIND_56_rXX );
// MH810105(E) MH364301 QRコード決済対応

	return ret;
}
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
//[]----------------------------------------------------------------------[]
///	@brief		精算データ(駐車券/定期券)
//[]----------------------------------------------------------------------[]
///	@param[in]	p_RcptDat	：精算ログデータ
///	@param[out]	p_NtDat		：精算NT-NETデータ
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2021/03/17<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
// MH810105(S) MH364301 QRコード決済対応
//static void NTNET_Edit_Data56_ParkingTkt_Pass( Receipt_data *p_RcptDat, DATA_KIND_56_r17 *p_NtDat )
static void NTNET_Edit_Data56_ParkingTkt_Pass( Receipt_data *p_RcptDat, DATA_KIND_56_rXX *p_NtDat )
// MH810105(E) MH364301 QRコード決済対応
{
	t_MediaInfoCre2 EncMediaInfo;

	// 駐車券設定
	if ( (ntNet_56_SaveData.NormalFlag != 0) &&	// 定期利用以外
		 (p_RcptDat->PayMethod == 2) ) {		// 定期券精算
		// 精算方法
		p_NtDat->PayMethod = 0;											// 券なし精算

		// 在車カウント
		// 34-0027(精算データIDの取り扱い)や07-00X4(定期X カウント信号／現在台数カウント)等を元に
		// 判定した結果を設定すべきだが、GT-4100では在車管理しないため「1」固定とする。
		// 判定の詳細はopesubのSetAddPayData()を参照。
		p_NtDat->CountSet = 1;											// しない

		// アンチパスチェック
		p_NtDat->PassCheck = 1;											// チェックOFF

		// 定期券駐車場№
		p_NtDat->ParkingNo = 0;

		// 精算媒体１種別(メイン媒体)
		p_NtDat->Media[0].MediaKind = CARD_TYPE_PARKING_TKT;			// 駐車券

		// 入庫/精算 媒体情報
		memset(&EncMediaInfo, 0, sizeof(t_MediaInfoCre2));
		memcpy(EncMediaInfo.MediaCardNo,								// 駐車券機械№(入庫機械№)(上位3桁、0～255)
			ntNet_56_SaveData.MachineTktNo,								// 駐車券番号(下位6桁、0～999999)
			sizeof(ntNet_56_SaveData.MachineTktNo));
		memcpy(p_NtDat->Media[0].MediaCardNo, &EncMediaInfo, sizeof(t_MediaInfoCre2));
	}

	// 定期券設定
	if ( (ntNet_56_SaveData.NormalFlag == 0) &&	// 定期利用
		 (p_RcptDat->PayMethod != 2) &&			// 定期券精算以外
		 (ntNet_56_SaveData.ParkingNo) ) {		// 定期券カード情報あり
		// 精算方法
		p_NtDat->PayMethod = 2;											// 定期券精算

		// 在車カウント(駐車券設定と同様)
		p_NtDat->CountSet = 1;											// しない

		// アンチパスチェック
		// 07-00X2(定期X 入出チェック)等を元に判定した結果を設定すべきだが、
		// GT-4100ではアンチパスチェックしないため「1」固定とする。
		// 判定の詳細はopesubのSetAddPayData()を参照。
		p_NtDat->PassCheck = 1;											// チェックOFF

		// 定期券 駐車場№
		p_NtDat->ParkingNo = ntNet_56_SaveData.ParkingNo;				// 駐車場№

		// 精算媒体１種別(メイン媒体)
		p_NtDat->Media[0].MediaKind = CARD_TYPE_PASS;					// 定期券

		// 入庫/精算 媒体情報
		memset(&EncMediaInfo, 0, sizeof(t_MediaInfoCre2));
		intoasl(EncMediaInfo.MediaCardNo, ntNet_56_SaveData.id, 5);		// 定期券ID(5桁、0～12000)
		intoas(EncMediaInfo.MediaCardInfo, ntNet_56_SaveData.syu, 2);	// 定期券種別(2桁、1～15)
		memcpy(p_NtDat->Media[0].MediaCardNo, &EncMediaInfo, sizeof(t_MediaInfoCre2));
	}
}
// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更

// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
////[]----------------------------------------------------------------------[]
/////	@brief		入庫データ(Rev.13：フェーズ7)
////[]----------------------------------------------------------------------[]
/////	@param[in]	p_RcptDat	：入庫ログデータ
/////	@param[out]	p_NtDat		：入庫NT-NETデータ
/////	@return		none
////[]----------------------------------------------------------------------[]
/////	@date		Create	: 2016/11/02<br>
/////				Update	: 
////[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
//unsigned short	NTNET_Edit_Data54_r13( enter_log *p_RcptDat, DATA_KIND_54_r13 *p_NtDat )
//{
//	unsigned short ret;
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
////	date_time_rec	wk_CLK_REC;
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_54_r13 ) );
//	NTNET_Edit_BasicData( 54, 0,  p_RcptDat->CMN_DT.DT_54.SeqNo, &p_NtDat->DataBasic );	// 基本データ作成
//
//	// 処理年月日時分秒は BasicDataMake()でセットした時刻ではなく入庫時刻をセットするので再セット
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->InTime.Year % 100);		// 入庫年
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->InTime.Mon;					// 入庫月
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->InTime.Day;					// 入庫日
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->InTime.Hour;				// 入庫時
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->InTime.Min;					// 入庫分
//	p_NtDat->DataBasic.Sec	= (uchar)p_RcptDat->InTime.Sec;					// 入庫秒
//
//	p_NtDat->FmtRev = 13;													// フォーマットRev.№
//
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->CMN_DT.DT_54.FullNo1;			// 満車台数１
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CMN_DT.DT_54.CarCnt1;			// 現在台数１
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->CMN_DT.DT_54.FullNo2;			// 満車台数２
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CMN_DT.DT_54.CarCnt2;			// 現在台数２
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->CMN_DT.DT_54.FullNo3;			// 満車台数３
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CMN_DT.DT_54.CarCnt3;			// 現在台数３
//	p_NtDat->ParkData.CarFullFlag	= 0;									// 台数・満空状態フラグ
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->CMN_DT.DT_54.PascarCnt;	// 定期車両カウント
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->CMN_DT.DT_54.Full[0];			// 駐車1満空状態
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->CMN_DT.DT_54.Full[1];			// 駐車2満空状態
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->CMN_DT.DT_54.Full[2];			// 駐車3満空状態
//	// 台数管理追番 ------------------------
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
////	memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
////	// 同一時分？
////	if( VehicleCountDateTime == Nrm_YMDHM( &wk_CLK_REC ) ){
////		// 同一時分の場合は追い番を＋１
////		VehicleCountSeqNo++;
////	}else{
////		// 異なる時分の場合は追い番を０とする
////		VehicleCountDateTime = Nrm_YMDHM( &wk_CLK_REC );
////		VehicleCountSeqNo = 0;
////	}
////	p_NtDat->ParkData.CarCntInfo.CarCntYear		= (wk_CLK_REC.Year % 100);	// 年
////	p_NtDat->ParkData.CarCntInfo.CarCntMon		= wk_CLK_REC.Mon;			// 月
////	p_NtDat->ParkData.CarCntInfo.CarCntDay		= wk_CLK_REC.Day;			// 日
////	p_NtDat->ParkData.CarCntInfo.CarCntHour		= wk_CLK_REC.Hour;			// 時
////	p_NtDat->ParkData.CarCntInfo.CarCntMin		= wk_CLK_REC.Min;			// 分
////	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= VehicleCountSeqNo;		// 追番
////	p_NtDat->ParkData.CarCntInfo.Reserve1		= 0;						// 予備
//	p_NtDat->ParkData.CarCntInfo.CarCntYear		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntYear;	// 年
//	p_NtDat->ParkData.CarCntInfo.CarCntMon		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntMon;		// 月
//	p_NtDat->ParkData.CarCntInfo.CarCntDay		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntDay;		// 日
//	p_NtDat->ParkData.CarCntInfo.CarCntHour		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntHour;	// 時
//	p_NtDat->ParkData.CarCntInfo.CarCntMin		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntMin;		// 分
//	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntSeq;		// 追番
//	p_NtDat->ParkData.CarCntInfo.Reserve1		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.Reserve1;		// 予備
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//	// -------------------------------------
//
//	p_NtDat->CenterSeqNo	= p_RcptDat->CMN_DT.DT_54.CenterSeqNo;			// センター追番（入庫）
//
//	p_NtDat->InCount	= p_RcptDat->InCount;								// 入庫追い番
//	p_NtDat->Syubet		= (uchar)p_RcptDat->Syubet;							// 処理区分
//	p_NtDat->InMode		= (uchar)p_RcptDat->InMode;							// 入庫ﾓｰﾄﾞ
//	p_NtDat->CMachineNo	= (uchar)p_RcptDat->CMachineNo;						// 駐車券機械№
//	p_NtDat->Reserve	= 0;												// 予備（サイズ調整用）
//	p_NtDat->FlapArea	= (ushort)(p_RcptDat->LockNo/10000);				// フラップシステム	区画		0～99
//	p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->LockNo%10000);				// 					車室番号	0～9999
//	p_NtDat->PassCheck	= (uchar)p_RcptDat->PassCheck;						// ｱﾝﾁﾊﾟｽﾁｪｯｸ
//	p_NtDat->CountSet	= (uchar)p_RcptDat->CountSet;						// 在車ｶｳﾝﾄ
//
//	// 入庫媒体情報は 0 をセットする
//	memset( &p_NtDat->Media[0], 0, sizeof( p_NtDat->Media ) );				// 入庫媒体情報1～4
//
//// 不具合修正(S) K.Onodera 2016/12/09 連動評価指摘(0カット前のサイズが誤っている)
////	ret = sizeof( DATA_KIND_54_r10 );
//	ret = sizeof( DATA_KIND_54_r13 );
//// 不具合修正(E) K.Onodera 2016/12/09 連動評価指摘(0カット前のサイズが誤っている)
//	return ret;
//}
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
//[]----------------------------------------------------------------------[]
///	@brief		精算情報データ(Rev.12：フェーズ7)
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/11/04<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
// MH810105(S) MH364301 QRコード決済対応
//void	NTNET_Snd_Data152_r12( void )
void	NTNET_Snd_Data152_rXX( void )
// MH810105(E) MH364301 QRコード決済対応
{
	ushort	i;
	ushort	j;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	ushort	num;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	uchar	ErrCode;		// ERRMDL_COIM, ERRMDL_NOTE, ERRMDL_READER種別のｺｰﾄﾞ
// MH810105(S) MH364301 QRコード決済対応
//	EMONEY_r14	t_EMoney;
//	CREINFO_r14 t_Credit;
// MH810105(E) MH364301 QRコード決済対応
// MH810100(S) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	t_MediaInfoCre2 EncMediaInfo;
// MH810100(E) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)

	ST_OIBAN w_oiban;
	
// MH810105(S) MH364301 QRコード決済対応
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_152_r12 ) );
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_152_rXX ) );
// MH810105(E) MH364301 QRコード決済対応

	/************************/
	/*	基本ﾃﾞｰﾀ作成		*/
	/************************/
	BasicDataMake( 152, 1 );					// ﾃﾞｰﾀ保持ﾌﾗｸﾞ = 1固定
// MH310100(S) S.Nishimoto 2020/08/21 車番チケットレス(#4596 NT-NET精算情報データのシーケンシャルNoが変化しない)
	SendNtnetDt.SData152_r12.DataBasic.SeqNo = GetNtDataSeqNo();
// MH310100(E) S.Nishimoto 2020/08/21 車番チケットレス(#4596 NT-NET精算情報データのシーケンシャルNoが変化しない)

// MH321800(S) Y.Tanizaki 精算情報データRev.13対応
//	SendNtnetDt.SData152_r12.FmtRev = 12;			// ﾌｫｰﾏｯﾄRev.№
// MH810105(S) MH364301 QRコード決済対応
//	SendNtnetDt.SData152_r12.FmtRev = 13;			// ﾌｫｰﾏｯﾄRev.№
	SendNtnetDt.SData152_r12.FmtRev = 14;			// ﾌｫｰﾏｯﾄRev.№
// MH810105(E) MH364301 QRコード決済対応
// MH321800(E) Y.Tanizaki 精算情報データRev.13対応
	/************************************/
	/*	機器状態 = t_KikiStatus			*/		// NTNET_Edit_Data56_r10
	/************************************/
	if( opncls() == 1 ){	// 営業中
		SendNtnetDt.SData152_r12.Kiki.KyugyoStatus 	= 0;	// 機器状態_営休業	0:営業
	} else {				// 休業中
		SendNtnetDt.SData152_r12.Kiki.KyugyoStatus 	= 1;	// 機器状態_営休業	1:休業
	}
	SendNtnetDt.SData152_r12.Kiki.Lc1Status 		= 0;	// 機器状態_LC1状態	0:OFF/1:ON
	SendNtnetDt.SData152_r12.Kiki.Lc2Status 		= 0;	// 機器状態_LC2状態	0:OFF/1:ON
	SendNtnetDt.SData152_r12.Kiki.Lc3Status 		= 0;	// 機器状態_LC3状態	0:OFF/1:ON
	SendNtnetDt.SData152_r12.Kiki.GateStatus 		= 0;	// 機器状態_ゲート状態	0:閉/1:開
	/********************************************************************/
	/*	AlmAct[ALM_MOD_MAX][ALM_NUM_MAX]の[2][0]のﾃｰﾌﾞﾙ(02:券関連)より	*/
	/*	00-99を対象に最後に発生したｱﾗｰﾑ									*/
	/*	ALMMDL_SUB2		alm_chk2()	t_EAM_Act	AlmAct					*/
	/********************************************************************/
	SendNtnetDt.SData152_r12.Kiki.NgCardRead 	= NgCard;	// NGカード読取内容	0＝なし, 1～255＝NT-NETのアラーム種別02のコード内容と同じ
	/****************************************************************************************/
	/*	発生中エラー = t_Erroring															*/
	/*	ErrAct[ERR_MOD_MAX][ERR_NUM_MAX]													*/
	/*	IsErrorOccuerd()関数にて当該のｴﾗｰ種別より00-99を対象に最上位の若番のｴﾗｰｺｰﾄﾞを設定	*/
	/****************************************************************************************/
	// ERRMDL_COIM		3.Coin
	ErrCode = NTNET_GetMostErrCode(ERRMDL_COIM);
	SendNtnetDt.SData152_r12.Err.ErrCoinmech	= ErrCode;	// コインメック	エラー種別03のコード内容と同じ(レベルの高いものからセットする)
	// ERRMDL_NOTE		4.Note
	ErrCode = NTNET_GetMostErrCode(ERRMDL_NOTE);
	SendNtnetDt.SData152_r12.Err.ErrNoteReader	= ErrCode;	// 紙幣リーダー	エラー種別04のコード内容と同じ(レベルの高いものからセットする)
	// ERRMDL_READER	1.Reader
	ErrCode = NTNET_GetMostErrCode(ERRMDL_READER);
	SendNtnetDt.SData152_r12.Err.ErrGikiReader	= ErrCode;	// 磁気リーダー	エラー種別01のコード内容と同じ(レベルの高いものからセットする)
	// 以下、FT-4000 では未対応
	SendNtnetDt.SData152_r12.Err.ErrNoteHarai	= 0;		// 紙幣払出機	エラー種別05のコード内容と同じ(レベルの高いものからセットする)
	SendNtnetDt.SData152_r12.Err.ErrCoinJyunkan	= 0;		// コイン循環	エラー種別06のコード内容と同じ(レベルの高いものからセットする)
	SendNtnetDt.SData152_r12.Err.ErrNoteJyunkan	= 0;		// 紙幣循環		エラー種別14のコード内容と同じ(レベルの高いものからセットする)
	/************************************/
	/*									*/
	/************************************/
	if( PayInfo_Class == 4 ){
		// クレジット精算
		SendNtnetDt.SData152_r12.PayClass	= (uchar)0;					// 処理区分
	}else{
		SendNtnetDt.SData152_r12.PayClass	= (uchar)PayInfo_Class;		// 処理区分
	}
	if(SendNtnetDt.SData152_r12.PayClass == 8) {						// 精算前か
		SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_In = 0x1f;		// 金種有無	搭載金種の有無
		SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_Out = 0x1f;		// 金種有無	搭載金種の有無
		SendNtnetDt.SData152_r12.AntiPassCheck = 1;						// アンチパスチェック	1＝チェックOFF
		goto LSendPayInfo;
	}
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	else if(SendNtnetDt.SData152_r12.PayClass == 100 ||					// 強制出庫
//			SendNtnetDt.SData152_r12.PayClass == 101 ||					// 不正出庫
//			SendNtnetDt.SData152_r12.PayClass == 102) {					// フラップ上昇前出庫
//		NTNET_Snd_Data152_SK();
//		goto LSendPayInfo;
//	}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し

	w_oiban.w = ntNet_152_SaveData.Oiban.w;
	w_oiban.i = ntNet_152_SaveData.Oiban.i;
	SendNtnetDt.SData152_r12.PayCount 	= CountSel( &w_oiban);	// 精算追番(0～99999)	syusei[LOCK_MAX]
	SendNtnetDt.SData152_r12.PayMethod 	= ntNet_152_SaveData.PayMethod;	// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
	SendNtnetDt.SData152_r12.PayMode 	= ntNet_152_SaveData.PayMode;	// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算/20＝Mifareプリペイド精算)
// MH810100(S) K.Onodera  2020/02/05  車番チケットレス(LCD_IF対応)
//	SendNtnetDt.SData152_r12.CMachineNo = 0;							// 駐車券機械№(入庫機械№)	0～255
	SendNtnetDt.SData152_r12.CMachineNo = ntNet_152_SaveData.CMachineNo;// 駐車券機械№(入庫機械№)	0～255
// MH810100(E) K.Onodera  2020/02/05 車番チケットレス(LCD_IF対応)

// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	if(SendNtnetDt.SData152_r12.PayClass == 9 ){								// 精算中
//		num = ryo_buf.pkiti - 1;
//		SendNtnetDt.SData152_r12.FlapArea 	= (ushort)LockInfo[num].area;		// フラップシステム	区画		0～99
//		SendNtnetDt.SData152_r12.FlapParkNo = (ushort)LockInfo[num].posi;		// 					車室番号	0～9999
//	}
//	else if(SendNtnetDt.SData152_r12.PayMethod != 5 ){							// 定期券更新以外
//		SendNtnetDt.SData152_r12.FlapArea 	= (ushort)(ntNet_152_SaveData.WPlace/10000);	// フラップシステム	区画		0～99
//		SendNtnetDt.SData152_r12.FlapParkNo = (ushort)(ntNet_152_SaveData.WPlace%10000);	// 					車室番号	0～9999
	if(SendNtnetDt.SData152_r12.PayMethod != 5 ){								// 定期券更新以外
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
		SendNtnetDt.SData152_r12.KakariNo 	= (ushort)ntNet_152_SaveData.KakariNo;			// 係員№	0～9999
		SendNtnetDt.SData152_r12.OutKind 	= ntNet_152_SaveData.OutKind;					// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
																			//  	   10＝ゲート開放/20＝Mifareプリペイド精算出庫
																			// 		   97＝ロック開・フラップ上昇前未精算出庫/98＝ラグタイム内出庫
																			// 		   99＝サービスタイム内出庫
	}
	if(SendNtnetDt.SData152_r12.PayClass == 9 ){								// 精算中
		SendNtnetDt.SData152_r12.CountSet 	= 1;								// 在車カウント 1＝しない
	}
	else {
		SendNtnetDt.SData152_r12.CountSet 	= ntNet_152_SaveData.CountSet;		// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
	}
	/********************************************/
	/*	入庫/前回精算_YMDHMS = t_InPrevYMDHMS	*/
	/********************************************/
	if( SendNtnetDt.SData152_r12.PayMethod != 5 ){					// 定期券更新以外
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Year		= (uchar)(ntNet_152_SaveData.carInTime.year%100);	// 入庫	年		00～99 2000～2099
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Mon		= ntNet_152_SaveData.carInTime.mon;					// 		月		01～12
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Day		= ntNet_152_SaveData.carInTime.day;					// 		日		01～31
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Hour		= ntNet_152_SaveData.carInTime.hour;				// 		時		00～23
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Min		= ntNet_152_SaveData.carInTime.min;					// 		分		00～59
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Sec		= 0;												// 		秒		00～59(磁気カードは0固定)
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Sec		= ntNet_152_SaveData.carInTime.sec;					// 		秒		00～59
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
		// 前回精算
	}
	/************************************/
	/*									*/
	/************************************/
	SendNtnetDt.SData152_r12.ReceiptIssue 	= ntNet_152_SaveData.ReceiptIssue;	// 領収証発行有無	0＝領収証なし/1＝領収証あり
	if(SendNtnetDt.SData152_r12.PayMethod != 5 ){					// 定期券更新以外
		SendNtnetDt.SData152_r12.Syubet 	= ntNet_152_SaveData.Syubet;	// 料金種別			1～
		SendNtnetDt.SData152_r12.Price 		= ntNet_152_SaveData.Price;		// 駐車料金			0～
	}
	SendNtnetDt.SData152_r12.CashPrice 		= ntNet_152_SaveData.CashPrice;	// 現金売上			0～
	SendNtnetDt.SData152_r12.InPrice 		= ntNet_152_SaveData.InPrice;	// 投入金額			0～
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// 不具合修正(S) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
////	SendNtnetDt.SData152_r12.ChgPrice 		= (ushort)ntNet_152_SaveData.ChgPrice;	// 釣銭金額			0～9999
//	SendNtnetDt.SData152_r12.ChgPrice 		= (ushort)(ntNet_152_SaveData.ChgPrice + ntNet_152_SaveData.FrkReturn);	// 釣銭金額 0～9999
//// 不具合修正(E) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
	SendNtnetDt.SData152_r12.ChgPrice 		= (ushort)ntNet_152_SaveData.ChgPrice;	// 釣銭金額			0～9999
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

	if(SendNtnetDt.SData152_r12.PayClass == 9 ){									// 精算中
		SendNtnetDt.SData152_r12.Syubet 	= (char)(ryo_buf.syubet + 1);			// 料金種別			1～
		if( ryo_buf.ryo_flg < 2 ) {												// 駐車券精算処理
			SendNtnetDt.SData152_r12.Price = ryo_buf.tyu_ryo;						// 駐車料金
		}
		else {
			SendNtnetDt.SData152_r12.Price = ryo_buf.tei_ryo;						// 定期料金
		}
		SendNtnetDt.SData152_r12.InPrice 	= ryo_buf.nyukin;						// 投入金額			0～
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//// 仕様変更(S) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
////		SendNtnetDt.SData152_r12.ChgPrice 	= (ushort)ryo_buf.turisen;				// 釣銭金額			0～9999
//		SendNtnetDt.SData152_r12.ChgPrice 	= (ushort)(ryo_buf.turisen + ryo_buf.kabarai);	// 釣銭金額			0～9999
//// 仕様変更(E) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
		SendNtnetDt.SData152_r12.ChgPrice 	= (ushort)ryo_buf.turisen;				// 釣銭金額			0～9999
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	}
	/****************************************************/
	/*	金銭情報(投入金枚数/払出金枚数) = t_MoneyInOut	*/
	/****************************************************/
	SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_In = 0x1f;						// 金種有無	搭載金種の有無
																				//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
																				//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
	if(SendNtnetDt.SData152_r12.PayClass == 0 || 									// 処理区分 精算済
		SendNtnetDt.SData152_r12.PayClass == 2 || 									// 処理区分 精算中止
		SendNtnetDt.SData152_r12.PayClass == 3) { 									// 処理区分 再精算中止
		SendNtnetDt.SData152_r12.MoneyInOut.In_10_cnt    = (uchar)ntNet_152_SaveData.in_coin[0];	// 投入金枚数(10円)		0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_50_cnt    = (uchar)ntNet_152_SaveData.in_coin[1];	// 投入金枚数(50円)		0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_100_cnt   = (uchar)ntNet_152_SaveData.in_coin[2];	// 投入金枚数(100円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_500_cnt   = (uchar)ntNet_152_SaveData.in_coin[3];	// 投入金枚数(500円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_1000_cnt  = (uchar)ntNet_152_SaveData.in_coin[4]	// 投入金枚数(1000円)	0～255
														+ ntNet_152_SaveData.f_escrow;		// エスクロ紙幣返却時は+1する
		SendNtnetDt.SData152_r12.MoneyInOut.In_2000_cnt  = 0;							// 投入金枚数(2000円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_5000_cnt  = 0;							// 投入金枚数(5000円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_10000_cnt = 0;							// 投入金枚数(10000円)	0～255
	}
	else if(SendNtnetDt.SData152_r12.PayClass == 9) {								// 処理区分 精算中
		SendNtnetDt.SData152_r12.MoneyInOut.In_10_cnt    = (uchar)ryo_buf.in_coin[0];	// 投入金枚数(10円)		0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_50_cnt    = (uchar)ryo_buf.in_coin[1];	// 投入金枚数(50円)		0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_100_cnt   = (uchar)ryo_buf.in_coin[2];	// 投入金枚数(100円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_500_cnt   = (uchar)ryo_buf.in_coin[3];	// 投入金枚数(500円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_1000_cnt  = (uchar)ryo_buf.in_coin[4];	// 投入金枚数(1000円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_2000_cnt  = 0;							// 投入金枚数(2000円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_5000_cnt  = 0;							// 投入金枚数(5000円)	0～255
		SendNtnetDt.SData152_r12.MoneyInOut.In_10000_cnt = 0;							// 投入金枚数(10000円)	0～255
	}
	SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_Out = 0x1f;						// 金種有無	搭載金種の有無
																				//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
																				//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
	if(SendNtnetDt.SData152_r12.PayClass == 0 || 									// 処理区分 精算済
		SendNtnetDt.SData152_r12.PayClass == 2 || 									// 処理区分 精算中止
		SendNtnetDt.SData152_r12.PayClass == 3) { 									// 処理区分 再精算中止
		SendNtnetDt.SData152_r12.MoneyInOut.Out_10_cnt    = (uchar)ntNet_152_SaveData.out_coin[0];	// 払出金枚数(10円)		0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_50_cnt    = (uchar)ntNet_152_SaveData.out_coin[1];	// 払出金枚数(50円)		0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_100_cnt   = (uchar)ntNet_152_SaveData.out_coin[2];	// 払出金枚数(100円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_500_cnt   = (uchar)ntNet_152_SaveData.out_coin[3];	// 払出金枚数(500円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_1000_cnt  = ntNet_152_SaveData.f_escrow;			// 払出金枚数(1000円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_2000cnt   = 0;							// 払出金枚数(2000円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_5000_cnt  = 0;							// 払出金枚数(5000円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_10000_cnt = 0;							// 払出金枚数(10000円)	0～255(払い出し不足分は含まない)
	}
	else if(SendNtnetDt.SData152_r12.PayClass == 9) {								// 処理区分 精算中
		SendNtnetDt.SData152_r12.MoneyInOut.Out_10_cnt    = (uchar)ryo_buf.out_coin[0]	// 払出金枚数(10円)		0～255(払い出し不足分は含まない)
															+ (uchar)ryo_buf.out_coin[4];
		SendNtnetDt.SData152_r12.MoneyInOut.Out_50_cnt    = (uchar)ryo_buf.out_coin[1]	// 払出金枚数(50円)		0～255(払い出し不足分は含まない)
															+ (uchar)ryo_buf.out_coin[5];
		SendNtnetDt.SData152_r12.MoneyInOut.Out_100_cnt   = (uchar)ryo_buf.out_coin[2]	// 払出金枚数(100円)	0～255(払い出し不足分は含まない)
															+ (uchar)ryo_buf.out_coin[6];
		SendNtnetDt.SData152_r12.MoneyInOut.Out_500_cnt   = (uchar)ryo_buf.out_coin[3];	// 払出金枚数(500円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_1000_cnt  = 0;							// 払出金枚数(1000円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_2000cnt   = 0;							// 払出金枚数(2000円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_5000_cnt  = 0;							// 払出金枚数(5000円)	0～255(払い出し不足分は含まない)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_10000_cnt = 0;							// 払出金枚数(10000円)	0～255(払い出し不足分は含まない)
	}
	/************************************/
	/*									*/
	/************************************/
	SendNtnetDt.SData152_r12.HaraiModoshiFusoku = (ushort)ntNet_152_SaveData.HaraiModoshiFusoku;		// 払戻不足額	0～9999
	if( ntNet_152_SaveData.CardFusokuTotal ){
		SendNtnetDt.SData152_r12.CardFusokuType = ntNet_152_SaveData.CardFusokuType;					// 払出不足媒体種別(現金以外)
		SendNtnetDt.SData152_r12.CardFusokuTotal = ntNet_152_SaveData.CardFusokuTotal;					// 払出不足額(現金以外)
	}
	SendNtnetDt.SData152_r12.Reserve1 = 0;											// 予備(サイズ調整用)	0
	if( SendNtnetDt.SData152_r12.PayClass != 8 &&									// 精算前以外
		SendNtnetDt.SData152_r12.PayClass != 9) {									// 精算中以外
		SendNtnetDt.SData152_r12.AntiPassCheck = ntNet_152_SaveData.AntiPassCheck;	// アンチパスチェック	0＝チェックON/1＝チェックOFF/2＝強制OFF
	}
	else {																		// 精算前or精算中
		SendNtnetDt.SData152_r12.AntiPassCheck = 1;									// アンチパスチェック	1＝チェックOFF
	}
																				//						※定期を使用しない場合は「１＝チェックOFF」固定
	if( (SendNtnetDt.SData152_r12.PayClass != 9 && ntNet_152_SaveData.ParkNoInPass) || 		// 精算中以外 定期券精算処理
		(SendNtnetDt.SData152_r12.PayClass == 9 && ryo_buf.ryo_flg >= 2) ) {		// 精算中 定期券精算処理
		if( SendNtnetDt.SData152_r12.PayClass != 9 ){
			SendNtnetDt.SData152_r12.ParkNoInPass = ntNet_152_SaveData.ParkNoInPass;		// 定期券　駐車場№	0～999999
		}
		else {
			SendNtnetDt.SData152_r12.ParkNoInPass = CPrmSS[S_SYS][ntNet_152_SaveData.pkno_syu+1];	// 定期券　駐車場№	0～999999
		}
// MH810100(S) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
//		/****************************************/
//		/*	精算媒体情報1, 2 = t_MediaInfo		*/
//		/****************************************/
//		SendNtnetDt.SData152_r12.Media[0].MediaKind = (ushort)(ntNet_152_SaveData.pkno_syu + 2);	// 精算媒体情報１　種別(メイン媒体)	0～99
//		// カード番号[30]
//		intoasl(SendNtnetDt.SData152_r12.Media[0].MediaCardNo, ntNet_152_SaveData.teiki_id, 5);		// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		// カード情報[16]
//		intoasl(SendNtnetDt.SData152_r12.Media[0].MediaCardInfo, ntNet_152_SaveData.teiki_syu, 2);	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		// 精算媒体情報2（サブ媒体）は オール０とする
	}
	if( ntNet_152_SaveData.MediaKind1 ){														// 精算媒体情報1種別あり？
		/************************************/
		/*	精算媒体情報1 = t_MediaInfo		*/
		/************************************/
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;				// 暗号化方式
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;				// 暗号鍵番号
		SendNtnetDt.SData152_r12.Media[0].MediaKind	= ntNet_152_SaveData.MediaKind1;			// 精算媒体情報1種別(メイン媒体) = 駐車券(101)/定期券(102)
		memset( &EncMediaInfo, 0, sizeof( t_MediaInfoCre2 ) );
		memcpy( EncMediaInfo.MediaCardNo, ntNet_152_SaveData.MediaCardNo1,						// カード番号(駐車券:駐車券機械№(入庫機械№)(上位3桁) + 駐車券番号(下位6桁)
														sizeof( EncMediaInfo.MediaCardNo ) );	//            定期券:定期券ID(5桁))
		memcpy( EncMediaInfo.MediaCardInfo, ntNet_152_SaveData.MediaCardInfo1,					// カード情報(駐車券:－(なし)
														sizeof( EncMediaInfo.MediaCardInfo ) );	//            定期券:定期券種別(2桁))
		AesCBCEncrypt( EncMediaInfo.MediaCardNo, sizeof( t_MediaInfoCre2 ) );					// AES 暗号化
		memcpy( SendNtnetDt.SData152_r12.Media[0].MediaCardNo, EncMediaInfo.MediaCardNo,		// カード番号(30Byte) + カード情報(16Byte) + 予約(2Byte)
														 sizeof( t_MediaInfoCre2 ) );
	}
	if( ntNet_152_SaveData.MediaKind2 == CARD_TYPE_CAR_NUM ){									// 精算媒体情報2種別 = 車番(203)あり？
		/************************************/
		/*	精算媒体情報2 = t_MediaInfo		*/
		/************************************/
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;				// 暗号化方式
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;				// 暗号鍵番号
		SendNtnetDt.SData152_r12.Media[1].MediaKind	= CARD_TYPE_CAR_NUM;						// 精算媒体情報2種別(サブ媒体) = 車番(203)
		memset( &EncMediaInfo, 0, sizeof( t_MediaInfoCre2 ) );
		memcpy( EncMediaInfo.MediaCardNo, ntNet_152_SaveData.MediaCardNo2,						// カード番号(陸運支局名(2Byte) + 分類番号(3Byte)
														sizeof( EncMediaInfo.MediaCardNo ) );	//            + 用途文字(2Byte) + 一連指定番号(4Byte))
		AesCBCEncrypt( EncMediaInfo.MediaCardNo, sizeof( t_MediaInfoCre2 ) );					// AES 暗号化
		memcpy( SendNtnetDt.SData152_r12.Media[1].MediaCardNo, EncMediaInfo.MediaCardNo,		// カード番号(30Byte) + カード情報(16Byte) + 予約(2Byte)
														 sizeof( t_MediaInfoCre2 ) );
// MH810100(E) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	}
	/************************************/
	/*									*/
	/************************************/
	if( ntNet_152_SaveData.e_pay_kind != 0 ){									// 電子決済種別 Suica:1, Edy:2
// MH321800(S) hosoda ICクレジット対応
//-		SendNtnetDt.SData152_r12.CardKind = (ushort)2;								// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
		switch (ntNet_152_SaveData.e_pay_kind) {
		case EC_EDY_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)3;
			break;
		case EC_NANACO_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)6;
			break;
		case EC_WAON_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)5;
			break;
		case EC_SAPICA_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)7;
			break;
		case EC_ID_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)12;
			break;
		case EC_QUIC_PAY_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)13;
			break;
// MH810105(S) MH364301 PiTaPa対応
		case EC_PITAPA_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)4;
			break;
// MH810105(E) MH364301 PiTaPa対応
// MH810105(S) MH364301 QRコード決済対応
		case EC_QR_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)16;
			break;
// MH810105(E) MH364301 QRコード決済対応
		case EC_KOUTSUU_USED:	// 交通系ICカード
		default:		// 2=交通系電子マネー(Suica,PASMO,ICOCA等)
			SendNtnetDt.SData152_r12.CardKind = (ushort)2;
			break;
		}
// MH321800(E) hosoda ICクレジット対応
// MH810105(S) MH364301 QRコード決済対応
//	}else if( PayInfo_Class == 4 ){												// クレジット精算
	}else if( PayInfo_Class == 4 || ntNet_152_SaveData.c_pay_ryo != 0 ){			// クレジット精算
// MH810105(E) MH364301 QRコード決済対応
		SendNtnetDt.SData152_r12.CardKind = (ushort)1;								// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
	}

	switch( SendNtnetDt.SData152_r12.CardKind ){									// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
	case 1:
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// 暗号化方式
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// 暗号鍵番号
		memset( &t_Credit, 0, sizeof( t_Credit ) );
// MH810105(S) MH364301 QRコード決済対応
//		t_Credit.amount = (ulong)ntNet_152_SaveData.c_pay_ryo;					// 決済額
//// MH321800(S) D.Inaba ICクレジット対応
////		memcpyFlushLeft( &t_Credit.card_no[0], &ntNet_152_SaveData.c_Card_No[0], sizeof(t_Credit.card_no), sizeof(ntNet_152_SaveData.c_Card_No) );	// 会員番号
////		memcpyFlushLeft( &t_Credit.cct_num[0], &ntNet_152_SaveData.c_cct_num[0],
////							sizeof(t_Credit.cct_num), CREDIT_TERM_ID_NO_SIZE );	// 端末識別番号
////		memcpyFlushLeft( &t_Credit.kid_code[0], &ntNet_152_SaveData.c_kid_code[0], sizeof(t_Credit.kid_code), sizeof(ntNet_152_SaveData.c_kid_code) );	// KID コード
//// MH321800(S) NT-NET精算データ仕様変更
////		memcpy( &t_Credit.card_no[0], &ntNet_152_SaveData.c_Card_No[0], sizeof(t_Credit.card_no) );		// 会員番号
//		// みなし決済時はスペース埋めする
//		memcpyFlushLeft( &t_Credit.card_no[0], &ntNet_152_SaveData.c_Card_No[0],
//						sizeof(t_Credit.card_no), sizeof(ntNet_152_SaveData.c_Card_No) );	// 会員番号
//// MH321800(E) NT-NET精算データ仕様変更
//		// 会員Noの上7桁目～下5桁目以外で'*'が格納されていたら'0'に置換
//		change_CharInArray( &t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '*', '0' );
//// MH321800(S) NT-NET精算データ仕様変更
////		memcpyFlushLeft2( &t_Credit.cct_num[0], &ntNet_152_SaveData.c_cct_num[0], sizeof(t_Credit.cct_num), sizeof(ntNet_152_SaveData.c_cct_num) );		// 端末識別番号
////		memcpy( &t_Credit.kid_code[0], &ntNet_152_SaveData.c_kid_code[0], sizeof(t_Credit.kid_code) );	// KID コード
//		// みなし決済時はスペース埋めする
//		memcpyFlushLeft( &t_Credit.cct_num[0], &ntNet_152_SaveData.c_cct_num[0],
//						sizeof(t_Credit.cct_num), sizeof(ntNet_152_SaveData.c_cct_num));	// 端末識別番号
//		// みなし決済時はスペース埋めする
//		memcpyFlushLeft( &t_Credit.kid_code[0], &ntNet_152_SaveData.c_kid_code[0],
//						sizeof(t_Credit.kid_code), sizeof(ntNet_152_SaveData.c_kid_code) );	// KIDコード
//// MH321800(E) NT-NET精算データ仕様変更
//// MH321800(E) D.Inaba ICクレジット対応
//		t_Credit.app_no = ntNet_152_SaveData.c_app_no;							// 承認番号
//		t_Credit.center_oiban = 0;												// ｾﾝﾀｰ処理追番
//		memcpyFlushLeft( &t_Credit.ShopAccountNo[0], &ntNet_152_SaveData.c_trade_no[0], sizeof(t_Credit.ShopAccountNo), sizeof(ntNet_152_SaveData.c_trade_no) );	// 加盟店取引番号
//		t_Credit.slip_no = ntNet_152_SaveData.c_slipNo;							// 伝票番号
		// 決済端末区分
		SendNtnetDt.SData152_r12.PayTerminalClass = 1;
		// 取引ステータス
		SendNtnetDt.SData152_r12.Transactiontatus = ntNet_152_SaveData.e_Transactiontatus;

		// 利用金額
		if (ntNet_152_SaveData.e_Transactiontatus != 3) {
			t_Credit.amount = ntNet_152_SaveData.c_pay_ryo;
		}
		else {
			// 未了支払不明は利用金額0円とする
			t_Credit.amount = 0;
		}

		// カード番号
		memcpyFlushLeft(&t_Credit.card_no[0],
						&ntNet_152_SaveData.c_Card_No[0],
						sizeof(t_Credit.card_no),
						sizeof(ntNet_152_SaveData.c_Card_No) );
		// 会員Noの上7桁目～下5桁目以外で'*'が格納されていたら'0'に置換
		change_CharInArray( &t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '*', '0' );

		// 端末識別番号
		memcpyFlushLeft(&t_Credit.cct_num[0],
						&ntNet_152_SaveData.c_cct_num[0],
						sizeof(t_Credit.cct_num),
						sizeof(ntNet_152_SaveData.c_cct_num));
		// KIDコード
		memcpyFlushLeft(&t_Credit.kid_code[0],
						&ntNet_152_SaveData.c_kid_code[0],
						sizeof(t_Credit.kid_code),
						sizeof(ntNet_152_SaveData.c_kid_code));
		// 承認番号
		t_Credit.app_no = ntNet_152_SaveData.c_app_no;
		// 伝票番号
		t_Credit.slip_no = ntNet_152_SaveData.c_slipNo;

		// ｾﾝﾀｰ処理追番
		t_Credit.center_oiban = 0;
		// 加盟店取引番号
		memset(&t_Credit.ShopAccountNo[0], 0x20, sizeof(t_Credit.ShopAccountNo));
// MH810105(E) MH364301 QRコード決済対応
		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );				// AES 暗号化
		memcpy( (uchar *)&SendNtnetDt.SData152_r12.settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// 決済情報
		break;
	case 2:
// MH321800(S) hosoda ICクレジット対応
	case 3:				// Edy
	case 5:				// WAON
	case 6:				// nanaco
	case 7:				// SAPICA
	case 12:			// iD
	case 13:			// QUICPay
// MH321800(E) hosoda ICクレジット対応
// MH810105(S) MH364301 PiTaPa対応
	case 4:				// PiTaPa
// MH810105(E) MH364301 PiTaPa対応
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// 暗号化方式
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// 暗号鍵番号
		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
// MH810105(S) MH364301 QRコード決済対応
//		t_EMoney.amount = ntNet_152_SaveData.e_pay_ryo;							// 電子決済精算情報　決済額
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//		memcpy( &t_EMoney.card_id[0], &ntNet_152_SaveData.e_Card_ID[0],
//				sizeof(ntNet_152_SaveData.e_Card_ID) );						// 電子決済精算情報　ｶｰﾄﾞID (Ascii 16桁)
//		t_EMoney.card_zangaku = ntNet_152_SaveData.e_pay_after;					// 電子決済精算情報　決済後残高
//// MH321800(S) D.Inaba ICクレジット対応
//		if( isEC_USE() ){														// 決済リーダでの精算？
//// MH810103 GG119202(S) 電子マネー対応
////			// みなし決済時はNULL(0x0)で埋める
////			memcpyFlushLeft2( &t_EMoney.inquiry_num[0], &ntNet_152_SaveData.e_inquiry_num[0],
////					sizeof(ntNet_152_SaveData.e_inquiry_num), sizeof(t_EMoney.inquiry_num) );	// 問合せ番号
//// MH810103 GG119201(S) 決済情報に問い合わせ番号をセットしない
//			// 決済情報に問合せ番号をセットする仕様はフォーマットRev14以降で対応
////			// みなし決済時はNULL(0x0)で埋める
////			memcpyFlushLeft( &t_EMoney.inquiry_num[0], &ntNet_152_SaveData.e_inquiry_num[0],
////						sizeof(t_EMoney.inquiry_num), sizeof(ntNet_152_SaveData.e_inquiry_num) );	// 問い合わせ番号
//// MH810103 GG119201(E) 決済情報に問い合わせ番号をセットしない
//// MH810103 GG119202(E) 電子マネー対応
//		}
//// MH321800(E) D.Inaba ICクレジット対応
		// 決済端末区分
		SendNtnetDt.SData152_r12.PayTerminalClass = 1;

		// 取引ステータス
		SendNtnetDt.SData152_r12.Transactiontatus = ntNet_152_SaveData.e_Transactiontatus;

		// 利用金額
		if (ntNet_152_SaveData.e_Transactiontatus != 3) {
			t_EMoney.amount = ntNet_152_SaveData.e_pay_ryo;
		}
		else {
			t_EMoney.amount = 0;
		}

		// カード残額
		t_EMoney.card_zangaku = ntNet_152_SaveData.e_pay_after;
		// カード番号
		memcpyFlushLeft(&t_EMoney.card_id[0],
						&ntNet_152_SaveData.e_Card_ID[0],
						sizeof(t_EMoney.card_id),
						sizeof(ntNet_152_SaveData.e_Card_ID) );
		// 問い合わせ番号
		memcpyFlushLeft(&t_EMoney.inquiry_num[0],
						&ntNet_152_SaveData.e_inquiry_num[0],
						sizeof(t_EMoney.inquiry_num),
						sizeof(ntNet_152_SaveData.e_inquiry_num) );
		// 承認番号
		t_EMoney.approbal_no = ntNet_152_SaveData.c_app_no;
// MH810105(E) MH364301 QRコード決済対応
		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );				// AES 暗号化
		memcpy( (uchar *)&SendNtnetDt.SData152_r12.settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// 決済情報
		break;
// MH810105(S) MH364301 QRコード決済対応
	case 16:			// QRコード決済
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// 暗号化方式
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// 暗号鍵番号
		memset(&t_QR, 0, sizeof(t_QR));

		// 決済端末区分
		SendNtnetDt.SData152_r12.PayTerminalClass = 1;
		// 取引ステータス
		SendNtnetDt.SData152_r12.Transactiontatus = ntNet_152_SaveData.e_Transactiontatus;

		// 利用金額
		if (ntNet_152_SaveData.e_Transactiontatus != 3) {
			t_QR.amount = ntNet_152_SaveData.e_pay_ryo;
		}
		else {
			t_QR.amount = 0;
		}

		// Mch取引番号
		memcpyFlushLeft(&t_QR.MchTradeNo[0],
						&ntNet_152_SaveData.qr_MchTradeNo[0],
						sizeof(t_QR.MchTradeNo),
						sizeof(ntNet_152_SaveData.qr_MchTradeNo));
		// 支払端末ID
		memcpyFlushLeft(&t_QR.PayTerminalNo[0],
						&ntNet_152_SaveData.qr_PayTermID[0],
						sizeof(t_QR.PayTerminalNo),
						sizeof(ntNet_152_SaveData.qr_PayTermID));
		// 取引番号
		memcpyFlushLeft(&t_QR.DealNo[0],
						&ntNet_152_SaveData.e_inquiry_num[0],
						sizeof(t_QR.DealNo),
						sizeof(ntNet_152_SaveData.e_inquiry_num));

		// 決済ブランド
		t_QR.PayKind = ntNet_152_SaveData.qr_PayKind;

		AesCBCEncrypt( (uchar *)&t_QR, sizeof( t_QR ) );				// AES 暗号化
		memcpy( (uchar *)&SendNtnetDt.SData152_r12.settlement[0], (uchar *)&t_QR, sizeof( t_QR ) );	// 決済情報
		break;
// MH810105(E) MH364301 QRコード決済対応
	}
	/************************************/
	/*	割引 = t_SeisanDiscount			*/
	/************************************/
	for( i = j = 0; i < WTIK_USEMAX; i++ ){								// 精算中止以外の割引情報コピー
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 		if(( ntNet_152_SaveData.DiscountData[i].DiscSyu != 0 ) &&					// 割引種別あり
// 		   (( ntNet_152_SaveData.DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// 精算中止割引情報でない
// 		    ( NTNET_CFRE < ntNet_152_SaveData.DiscountData[i].DiscSyu ))){
// 		    if( ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA以外(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(支払額・残額)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO以外(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(支払額・残額)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY以外	(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(支払額・残額)
// // MH321800(S) hosoda ICクレジット対応
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_WAON_1 &&		// WAON以外	(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_WAON_2 &&		//			(支払額・残額)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SAPICA_1 &&		// SAPICA以外	(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SAPICA_2 &&		//			(支払額・残額)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_NANACO_1 &&		// NANACO以外	(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_NANACO_2 &&		//			(支払額・残額)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ID_1 &&			// iD以外	(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ID_2 &&			//			(支払額・残額)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_QUICPAY_1 &&	// QUICPay以外	(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_QUICPAY_2 &&	//			(支払額・残額)
// // MH321800(E) hosoda ICクレジット対応
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA以外(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(支払額・残額)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&	// IC-Card以外(番号)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICCARD_2 ) 	//			(支払額・残額)
// GG124100(S) R.Endo 2022/08/04 車番チケットレス3.0 #6522 割引額が0円の割引がNT-NET精算データの割引情報にセットされない
// 		// 「割引種別」は割引計算時にチェック済みのため「今回使用した割引金額」の有無のみチェックする
// 		if ( ntNet_152_SaveData.DiscountData[i].Discount > 0 ) {
		// 「割引種別」は割引計算時にチェック済みのためここではチェックしない
		if ( ntNet_152_SaveData.DiscountData[i].Discount ||		// 今回使用した割引金額
			 ntNet_152_SaveData.DiscountData[i].DiscCount ) {	// 今回使用した枚数
// GG124100(E) R.Endo 2022/08/04 車番チケットレス3.0 #6522 割引額が0円の割引がNT-NET精算データの割引情報にセットされない
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
		    {
				SendNtnetDt.SData152_r12.SDiscount[j].ParkingNo	= ntNet_152_SaveData.DiscountData[i].ParkingNo;		// 駐車場No.
				SendNtnetDt.SData152_r12.SDiscount[j].Kind		= ntNet_152_SaveData.DiscountData[i].DiscSyu;		// 割引種別
				SendNtnetDt.SData152_r12.SDiscount[j].Group		= ntNet_152_SaveData.DiscountData[i].DiscNo;		// 割引区分
				SendNtnetDt.SData152_r12.SDiscount[j].Callback	= ntNet_152_SaveData.DiscountData[i].DiscCount;		// 回収枚数
				SendNtnetDt.SData152_r12.SDiscount[j].Amount	= ntNet_152_SaveData.DiscountData[i].Discount;		// 割引額
				SendNtnetDt.SData152_r12.SDiscount[j].Info1		= ntNet_152_SaveData.DiscountData[i].DiscInfo1;		// 割引情報1
				if( SendNtnetDt.SData152_r12.SDiscount[j].Kind != NTNET_PRI_W ){
					SendNtnetDt.SData152_r12.SDiscount[j].Info2 = ntNet_152_SaveData.DiscountData[i].uDiscData.common.DiscInfo2;	// 割引情報2
				}
				j++;
		    }
		}
	}
	for( i=0; i<DETAIL_SYU_MAX; i++ ){
		if( ntNet_152_SaveData.DetailData[i].DiscSyu != 0  ){
			SendNtnetDt.SData152_r12.SDiscount[j].ParkingNo	= ntNet_152_SaveData.DetailData[i].ParkingNo;					// 駐車場No.
			SendNtnetDt.SData152_r12.SDiscount[j].Kind		= ntNet_152_SaveData.DetailData[i].DiscSyu;						// 割引種別
			SendNtnetDt.SData152_r12.SDiscount[j].Group		= ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscNo;		// 割引区分
			SendNtnetDt.SData152_r12.SDiscount[j].Callback	= ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscCount;	// 回収枚数
			SendNtnetDt.SData152_r12.SDiscount[j].Amount	= ntNet_152_SaveData.DetailData[i].uDetail.Common.Discount;		// 割引額
			SendNtnetDt.SData152_r12.SDiscount[j].Info1		= ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscInfo1;	// 割引情報1
			SendNtnetDt.SData152_r12.SDiscount[j].Info2		= ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscInfo2;	// 割引情報2
			j++;
		}
	}
// MH810105(S) MH364301 QRコード決済対応
//// MH321800(S) 割引種別8000のセット方法を修正
//	// 決済リーダ接続あり
//	if (isEC_USE()) {
//		if (ntNet_152_SaveData.c_pay_ryo != 0) {
//			// クレジット決済
//			/* 空きｴﾘｱ検索 */
//			for (i = 0; (i < NTNET_DIC_MAX) &&
//					(0 != SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo); i++) {
//				;
//			}		/* データがある間				*/
//			if (i < NTNET_DIC_MAX) {
//				SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];			// 駐車場No.
//				SendNtnetDt.SData152_r12.SDiscount[i].Kind = NTNET_INQUIRY_NUM;				// 割引種別
//				// クレジット決済時は問い合わせ番号を受信しないので0x20埋めとする
//				memset(&SendNtnetDt.SData152_r12.SDiscount[i].Group, 0x20, 16);
//			}
//		}
//		else if (ntNet_152_SaveData.e_pay_ryo != 0) {
//			// 電子マネー決済
//			// 割引情報にカードNo.と利用額はセットしない
//			/* 空きｴﾘｱ検索 */
//			for (i = 0; (i < NTNET_DIC_MAX) &&
//					(0 != SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo); i++) {
//				;
//			}		/* データがある間				*/
//			if (i < NTNET_DIC_MAX) {
//				SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];			// 駐車場No.
//				SendNtnetDt.SData152_r12.SDiscount[i].Kind = NTNET_INQUIRY_NUM;				// 割引種別
//// MH810103 GG119202(S) 電子マネー対応
////				memcpy(&SendNtnetDt.SData152_r12.SDiscount[i].Group, ntNet_152_SaveData.e_inquiry_num, 16);
//				memcpyFlushLeft( (uchar *)&SendNtnetDt.SData152_r12.SDiscount[i].Group, (uchar *)&ntNet_152_SaveData.e_inquiry_num[0],
//									16, sizeof( ntNet_152_SaveData.e_inquiry_num ) );		// 問い合わせ番号
//// MH810103 GG119202(E) 電子マネー対応
//			}
//		}
//	}
//// MH321800(E) 割引種別8000のセット方法を修正
	if ((PayInfo_Class == 2 || PayInfo_Class == 3) &&
		ntNet_152_SaveData.e_Transactiontatus == 3) {
		// 決済精算中止
		/* 空きｴﾘｱ検索 */
		for (i = 0; (i < NTNET_DIC_MAX) &&
				(0 != SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo); i++) {
			;
		}		/* データがある間				*/
		if (i < NTNET_DIC_MAX) {
			SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];	// 駐車場No.
			SendNtnetDt.SData152_r12.SDiscount[i].Kind = NTNET_MIRYO_UNKNOWN;	// 割引種別
			SendNtnetDt.SData152_r12.SDiscount[i].Group =
											SendNtnetDt.SData152_r12.CardKind;	// 割引区分
			SendNtnetDt.SData152_r12.SDiscount[i].Callback = 1;					// 使用枚数
			SendNtnetDt.SData152_r12.SDiscount[i].Amount = 0;					// 割引額

			if (ntNet_152_SaveData.c_pay_ryo != 0) {
				SendNtnetDt.SData152_r12.SDiscount[i].Info1 =
												ntNet_152_SaveData.c_pay_ryo;	// 割引情報1
			}
			else if (ntNet_152_SaveData.e_pay_ryo != 0) {
				SendNtnetDt.SData152_r12.SDiscount[i].Info1 =
												ntNet_152_SaveData.e_pay_ryo;	// 割引情報1
			}

			SendNtnetDt.SData152_r12.SDiscount[i].Info2 = 0;					// 割引情報2
		}
	}
// MH810105(E) MH364301 QRコード決済対応

	/********************/
	/*	ﾃﾞｰﾀ送信登録	*/
	/********************/
LSendPayInfo:
// MH810105(S) MH364301 QRコード決済対応
//	if(_is_ntnet_remote()) {
//		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_152_r12));
//	} else {
//		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_152_r12), NTNET_BUF_NORMAL);
//	}
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_152_rXX));
	} else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_152_rXX), NTNET_BUF_NORMAL);
	}
// MH810105(E) MH364301 QRコード決済対応
}
/*[]----------------------------------------------------------------------[]*
 *| 精算情報データ不正出庫・強制出庫・フラップ上昇前送信処理
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
// MH810100(S) K.Onodera  2019/12/25 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
//static	void	NTNET_Snd_Data152_SK(void)
void	NTNET_Snd_Data152_SK(void)
// MH810100(E) K.Onodera  2019/12/25 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
{
	SendNtnetDt.SData152_r12.PayClass 	= 0;							// 処理区分(0＝精算/1＝再精算/2＝精算中止/3＝再精算中止/(4＝クレジット精算※))
	SendNtnetDt.SData152_r12.PayCount 	= CountSel(&PayInfoData_SK.Oiban);	// 強制出庫or不正出庫追番orフラップ上昇前（精算追番）
	SendNtnetDt.SData152_r12.PayMethod 	= 0;							// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
	SendNtnetDt.SData152_r12.PayMode 	= 0;							// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算/20＝Mifareプリペイド精算)
	SendNtnetDt.SData152_r12.CMachineNo = 0;							// 駐車券機械№(入庫機械№)	0～255

	SendNtnetDt.SData152_r12.FlapArea 	= (ushort)(PayInfoData_SK.WPlace/10000);	// フラップシステム	区画		0～99
	SendNtnetDt.SData152_r12.FlapParkNo = (ushort)(PayInfoData_SK.WPlace%10000);	// 					車室番号	0～9999
	SendNtnetDt.SData152_r12.KakariNo 	= PayInfoData_SK.KakariNo;		// 係員№	0～9999
	SendNtnetDt.SData152_r12.OutKind 	= PayInfoData_SK.OutKind;		// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
	SendNtnetDt.SData152_r12.CountSet	= PayInfoData_SK.CountSet;		// 在車ｶｳﾝﾄ-1する

	/********************************************/
	/*	入庫/前回精算_YMDHMS = t_InPrevYMDHMS	*/
	/********************************************/
	if( SendNtnetDt.SData152_r12.PayMethod != 5 ){					// 定期券更新以外
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Year	= 
										(uchar)(PayInfoData_SK.TInTime.Year % 100);	// 入庫	年		00～99 2000～2099
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Mon	= PayInfoData_SK.TInTime.Mon;	// 		月		01～12
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Day	= PayInfoData_SK.TInTime.Day;	// 		日		01～31
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Hour	= PayInfoData_SK.TInTime.Hour;	// 		時		00～23
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Min	= PayInfoData_SK.TInTime.Min;	// 		分		00～59
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Sec	= 0;							// 		秒		00～59(磁気カードは0固定)
	}

	SendNtnetDt.SData152_r12.Syubet 	= PayInfoData_SK.syu;					// 料金種別			1～
	SendNtnetDt.SData152_r12.Price 		= PayInfoData_SK.WPrice;				// 駐車料金			0～

	SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_In = 0x1f;					// 金種有無	搭載金種の有無
																			//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
																			//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
	SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_Out = 0x1f;					// 金種有無	搭載金種の有無
																			//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
																			//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
	SendNtnetDt.SData152_r12.AntiPassCheck = PayInfoData_SK.PassCheck;			// アンチパスチェック	0＝チェックON/1＝チェックOFF/2＝強制OFF
																			//						※定期を使用しない場合は「１＝チェックOFF」固定

}
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
// 仕様変更(S) K.Onodera 2016/11/04 集計基本データフォーマット対応
//[]----------------------------------------------------------------------[]
///	@brief			集計基本データ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ログから取り出した集計データのポインタ
///	@param[in]		Type	: 集計タイプ
///	@param[out]		p_NtDat	: 集計終了通知データ(DATA_KIND_42_r13型)へのポインタ  
///	@return			ret		: 集計終了通知データのデータサイズ(システムID～) 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//unsigned short	NTNET_Edit_SyukeiKihon_r13( SYUKEI *syukei, ushort Type, DATA_KIND_42_r13 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiKihon_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_42 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
{
	static const ushort c_coin[COIN_SYU_CNT] = {10, 50, 100, 500};
	ushort	i;
	ushort	ret;

// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	memset( p_NtDat, 0, sizeof( DATA_KIND_42_r13 ) );
	memset( p_NtDat, 0, sizeof( DATA_KIND_42 ) );
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
	NTNET_Edit_BasicData( 42, 0, syukei->SeqNo[0], &p_NtDat->DataBasic);	// Ｔ合計：データ保持フラグ = 0

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// 処理年
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// 処理月
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// 処理日
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// 処理時
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// 処理分
	p_NtDat->DataBasic.Sec = 0;													// 処理秒

// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	p_NtDat->FmtRev					= 13;										// フォーマットRev.No.
	p_NtDat->FmtRev					= 15;										// フォーマットRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// 集計タイプ 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// 係員No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// 集計追番
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// 個別追番
		if (Type == 11) {														// GT集計
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// 終了追番
		}
		break;
	case	1:		// 通し追い番
		if (Type == 1 || Type == 11) {
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// 開始追番
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// 終了追番
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// センター追番（集計）
	p_NtDat->LastTime.Year = (uchar)(syukei->OldTime.Year % 100 );				// 処理年
	p_NtDat->LastTime.Mon = (uchar)syukei->OldTime.Mon;							// 処理月
	p_NtDat->LastTime.Day = (uchar)syukei->OldTime.Day;							// 処理日
	p_NtDat->LastTime.Hour = (uchar)syukei->OldTime.Hour;						// 処理時
	p_NtDat->LastTime.Min = (uchar)syukei->OldTime.Min;							// 処理分
	p_NtDat->LastTime.Sec = 0;													// 処理秒
	p_NtDat->SettleNum							= syukei->Seisan_Tcnt;			// 総精算回数
	p_NtDat->Kakeuri							= syukei->Kakeuri_Tryo;			// 総掛売額
	p_NtDat->Cash								= syukei->Genuri_Tryo;			// 総現金売上額
	p_NtDat->Uriage								= syukei->Uri_Tryo;				// 総売上額
	p_NtDat->Tax								= syukei->Tax_Tryo;				// 総消費税額
	p_NtDat->Excluded							= 0;							// 総売上対象外金額
	p_NtDat->Charge								= syukei->Turi_modosi_ryo;		// 釣銭払戻額
	p_NtDat->CoinTotalNum						= (ushort)syukei->Ckinko_goukei_cnt;	// コイン金庫合計回数
	p_NtDat->NoteTotalNum						= (ushort)syukei->Skinko_goukei_cnt;	// 紙幣金庫合計回数
	p_NtDat->CyclicCoinTotalNum					= (ushort)syukei->Junkan_goukei_cnt;	// 循環コイン合計回数
	p_NtDat->NoteOutTotalNum					= (ushort)syukei->Siheih_goukei_cnt;	// 紙幣払出機合計回数
	p_NtDat->SettleNumServiceTime				= syukei->In_svst_seisan;		// サービスタイム内精算回数
	p_NtDat->Shortage.Num						= syukei->Harai_husoku_cnt;		// 払出不足回数
	p_NtDat->Shortage.Amount					= syukei->Harai_husoku_ryo;		// 払出不足金額
	p_NtDat->Cancel.Num							= syukei->Seisan_chusi_cnt;		// 精算中止回数
	p_NtDat->Cancel.Amount						= syukei->Seisan_chusi_ryo;		// 精算中止金額
	p_NtDat->AntiPassOffSettle					= syukei->Apass_off_seisan;		// アンチパスOFF精算回数
	p_NtDat->ReceiptIssue						= syukei->Ryosyuu_pri_cnt;		// 領収証発行枚数
	p_NtDat->WarrantIssue						= syukei->Azukari_pri_cnt;		// 預り証発行枚数
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	p_NtDat->AllSystem.CarOutIllegal.Num		= syukei->Husei_out_Tcnt;		// 全装置  不正出庫回数
//	p_NtDat->AllSystem.CarOutIllegal.Amount		= syukei->Husei_out_Tryo;		//                 金額
//	p_NtDat->AllSystem.CarOutForce.Num			= syukei->Kyousei_out_Tcnt;		//         強制出庫回数
//	p_NtDat->AllSystem.CarOutForce.Amount		= syukei->Kyousei_out_Tryo;		//                 金額
//	p_NtDat->AllSystem.AcceptTicket				= syukei->Uketuke_pri_Tcnt;		//         受付券発行回数
//	p_NtDat->AllSystem.ModifySettle.Num			= syukei->Syuusei_seisan_Tcnt;	// 修正精算回数
//	p_NtDat->AllSystem.ModifySettle.Amount		= syukei->Syuusei_seisan_Tryo;	//         金額
//	p_NtDat->CarInTotal							= syukei->In_car_Tcnt;			// 総入庫台数
//	p_NtDat->CarOutTotal						= syukei->Out_car_Tcnt;			// 総出庫台数
//	p_NtDat->CarIn1								= syukei->In_car_cnt[0];		// 入庫1入庫台数
//	p_NtDat->CarOut1							= syukei->Out_car_cnt[0];		// 出庫1出庫台数
//	p_NtDat->CarIn2								= syukei->In_car_cnt[1];		// 入庫2入庫台数
//	p_NtDat->CarOut2							= syukei->Out_car_cnt[1];		// 出庫2出庫台数
//	p_NtDat->CarIn3								= syukei->In_car_cnt[2];		// 入庫3入庫台数
//	p_NtDat->CarOut3							= syukei->Out_car_cnt[2];		// 出庫3出庫台数
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
// 強制完了ｷｰでの未入金は基本ﾃﾞｰﾀに項目を新設して送信する
	p_NtDat->MiyoCount							= syukei->Syuusei_seisan_Mcnt;	// 未入金回数
	p_NtDat->MiroMoney							= syukei->Syuusei_seisan_Mryo;	// 未入金額
	p_NtDat->LagExtensionCnt					= syukei->Lag_extension_cnt;	// ラグタイム延長回数
	p_NtDat->SaleParkingNo						= (ulong)CPrmSS[S_SYS][1];		// 売上先駐車場№(0～999999   ※特注用（標準では基本駐車場№をセット）
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
//	p_NtDat->FurikaeCnt							= syukei->Furikae_seisan_cnt2;	// 振替回数
//	p_NtDat->FurikaeTotal						= syukei->Furikae_seisan_ryo2;	// 振替額
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し
	p_NtDat->RemoteCnt							= syukei->Remote_seisan_cnt;	// 遠隔精算回数
	p_NtDat->RemoteTotal						= syukei->Remote_seisan_ryo;	// 遠隔精算額
	// 金銭関連情報
	p_NtDat->Total					= syukei->Kinko_Tryo;						// 金庫総入金額
	p_NtDat->NoteTotal				= syukei->Note_Tryo;						// 紙幣金庫総入金額
	p_NtDat->CoinTotal				= syukei->Coin_Tryo;						// コイン金庫総入金額
	for (i = 0; i < COIN_SYU_CNT; i++) {										// コイン1～4
		p_NtDat->Coin[i].Kind		= c_coin[i];								//        金種
		p_NtDat->Coin[i].Num		= (ushort)syukei->Coin_cnt[i];				//        枚数
	}
	p_NtDat->Note[0].Kind			= 1000;										// 紙幣1  金種
	p_NtDat->Note[0].Num2			= (ushort)syukei->Note_cnt[0];				//        紙幣枚数

// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	ret = sizeof( DATA_KIND_42_r13 ); 
	ret = sizeof( DATA_KIND_42 ); 
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
	return ret;
}
// 仕様変更(E) K.Onodera 2016/11/04 集計基本データフォーマット対応
// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
void SetVehicleCountDate( ulong val ){
	VehicleCountDateTime = val;
}
ulong GetVehicleCountDate( void ){
	return VehicleCountDateTime;
}
void AddVehicleCountSeqNo( void ){
	VehicleCountSeqNo++;
}
void ClrVehicleCountSeqNo( void ){
	VehicleCountSeqNo = 0;
}
ushort GetVehicleCountSeqNo( void ){
	return VehicleCountSeqNo;
}
// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//[]----------------------------------------------------------------------[]
///	@brief			長期駐車情報ログデータ作成(LOGに保存する形式)
//[]----------------------------------------------------------------------[]
///	@param[in]		pr_lokno : 内部処理用駐車位置番号 0の場合は全解除(車室なし)
///	@param[in]		time	: 検出時間(設定値) 全解除の場合「0」をセット
///	@param[in]		knd	: 0=発生 1=解除 2=全解除
///	@param[in]		knd2: 0=指定なし(強制解除) 1=精算 2=強制／不正 (kndが1=解除の時のみ有効)
///	@return			woid	: 
///	@author			A.iiizumi
///	@attention		
///	@note			全解除の場合は検出時間1/検出時間2は判定しない(不要なため)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2018/09/03<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	Make_Log_LongParking_Pweb( unsigned long pr_lokno, unsigned short time, uchar knd, uchar knd2)
{
	ushort	lkno;

	memset( &LongParking_data_Pweb, 0, sizeof( LongParking_data_Pweb ) );

	LongParking_data_Pweb.Knd = knd;		// 発生/解除/全解除
	LongParking_data_Pweb.Ck_Time = time;	// 長期駐車検出時間
	LongParking_data_Pweb.LockNo = pr_lokno;// 内部処理用駐車位置番号
	LongParking_data_Pweb.SeqNo = GetNtDataSeqNo();// シーケンシャルNo.の設定

	if(knd == LONGPARK_LOG_RESET){// 解除
		if(knd2 == LONGPARK_LOG_PAY){// 精算による解除
			LongParking_data_Pweb.OutTime.Year = car_ot_f.year;	// 精算時刻	（年）をｾｯﾄ
			LongParking_data_Pweb.OutTime.Mon = car_ot_f.mon;	// 			（月）をｾｯﾄ
			LongParking_data_Pweb.OutTime.Day = car_ot_f.day;	// 			（日）をｾｯﾄ
			LongParking_data_Pweb.OutTime.Hour = car_ot_f.hour;	// 			（時）をｾｯﾄ
			LongParking_data_Pweb.OutTime.Min = car_ot_f.min;	// 			（分）をｾｯﾄ
			// 秒は「0」固定
		}else if(knd2 == LONGPARK_LOG_KYOUSEI_FUSEI){// 強制/不正出庫による解除
			LongParking_data_Pweb.OutTime.Year = CLK_REC.year;	// 処理時刻	（年）をｾｯﾄ
			LongParking_data_Pweb.OutTime.Mon = CLK_REC.mont;	// 			（月）をｾｯﾄ
			LongParking_data_Pweb.OutTime.Day = CLK_REC.date;	// 			（日）をｾｯﾄ
			LongParking_data_Pweb.OutTime.Hour = CLK_REC.hour;	// 			（時）をｾｯﾄ
			LongParking_data_Pweb.OutTime.Min = CLK_REC.minu;	// 			（分）をｾｯﾄ
			// 秒は「0」固定
		}
	}

	if(knd != LONGPARK_LOG_ALL_RESET){// 全解除以外
		// 全解除以外は入庫時刻をセット
		lkno = pr_lokno - 1;
		LongParking_data_Pweb.InTime.Year = FLAPDT.flp_data[lkno].year;		// 入庫年
		LongParking_data_Pweb.InTime.Mon = FLAPDT.flp_data[lkno].mont;		// 入庫月
		LongParking_data_Pweb.InTime.Day = FLAPDT.flp_data[lkno].date;		// 入庫日
		LongParking_data_Pweb.InTime.Hour = FLAPDT.flp_data[lkno].hour;		// 入庫時
		LongParking_data_Pweb.InTime.Min = FLAPDT.flp_data[lkno].minu;		// 入庫分
	}

	// 処理年月日をここで入れる
	LongParking_data_Pweb.ProcDate.Year	= CLK_REC.year;				// 年
	LongParking_data_Pweb.ProcDate.Mon	= CLK_REC.mont;				// 月
	LongParking_data_Pweb.ProcDate.Day	= CLK_REC.date;				// 日
	LongParking_data_Pweb.ProcDate.Hour	= CLK_REC.hour;				// 時
	LongParking_data_Pweb.ProcDate.Min	= CLK_REC.minu;				// 分
	LongParking_data_Pweb.ProcDate.Sec	= (ushort)CLK_REC.seco;		// 秒

}
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(電文対応)
//[]----------------------------------------------------------------------[]
///	@brief			長期駐車情報データ(データ種別61)編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ログから取り出した長期駐車情報データのポインタ
///	@param[out]		p_NtDat   : 入庫データ(DATA_KIND_20型)へのポインタ  
///	@return			ret       : 入庫データのデータサイズ(システムID～) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2018/09/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data61(	LongPark_log_Pweb 	*p_RcptDat,		// 長期駐車情報logﾃﾞｰﾀ(IN)
									DATA_KIND_61_r10 	*p_NtDat )		// 長期駐車情報NT-NET(OUT)
{
	unsigned short ret;
	unsigned long data;
	memset( p_NtDat, 0, sizeof( DATA_KIND_61_r10 ) );
	NTNET_Edit_BasicData( 61, 0,  p_RcptDat->SeqNo, &p_NtDat->DataBasic );	// 基本データ作成

	p_NtDat->dtc = p_RcptDat->Knd;							// 発生/解除/全解除

	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->ProcDate.Year % 100);	// 処理年
	p_NtDat->DataBasic.Mon  = p_RcptDat->ProcDate.Mon;					// 処理月
	p_NtDat->DataBasic.Day  = p_RcptDat->ProcDate.Day;					// 処理日
	p_NtDat->DataBasic.Hour = p_RcptDat->ProcDate.Hour;					// 処理時
	p_NtDat->DataBasic.Min  = p_RcptDat->ProcDate.Min;					// 処理分
	p_NtDat->DataBasic.Sec  = p_RcptDat->ProcDate.Sec;					// 処理秒

	p_NtDat->FmtRev = 10;									// フォーマットRev.№

	p_NtDat->InTime.Year = (uchar)(p_RcptDat->InTime.Year % 100);		// 入庫年
	p_NtDat->InTime.Mon  = p_RcptDat->InTime.Mon;						// 入庫月
	p_NtDat->InTime.Day  = p_RcptDat->InTime.Day;						// 入庫日
	p_NtDat->InTime.Hour = p_RcptDat->InTime.Hour;						// 入庫時
	p_NtDat->InTime.Min  = p_RcptDat->InTime.Min;						// 入庫分
	// 秒は0

	p_NtDat->OutTime.Year = (uchar)(p_RcptDat->OutTime.Year % 100);	// 精算(出庫)時刻（年）をｾｯﾄ
	p_NtDat->OutTime.Mon  = p_RcptDat->OutTime.Mon;						// （月）
	p_NtDat->OutTime.Day  = p_RcptDat->OutTime.Day;						// （日）
	p_NtDat->OutTime.Hour = p_RcptDat->OutTime.Hour;					// （時）
	p_NtDat->OutTime.Min  = p_RcptDat->OutTime.Min;						// （分）
	// 秒は0

	p_NtDat->Ck_Time = (ulong)(p_RcptDat->Ck_Time * 60);		// 長期駐車検出時間(分)

	if(p_RcptDat->Knd != LONGPARK_LOG_ALL_RESET){				// 全解除以外はセットする
		// 入庫媒体の設定
		p_NtDat->Media[0].MediaKind = 51;						// 種別(媒体) ロック・フラップ
		// 機械№
		data = (ulong)CPrmSS[S_PAY][2];
		intoasl(p_NtDat->Media[0].MediaCardNo, data, 3);		// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
		// 区画
		data = (ulong)LockInfo[p_RcptDat->LockNo-1].area;
		intoasl(&p_NtDat->Media[0].MediaCardNo[3], data, 2);	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
		// 車室シーケンシャルNo
		data = 0;// 0固定
		intoasl(&p_NtDat->Media[0].MediaCardNo[5], data, 2);	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
		// 車室No
		data = LockInfo[p_RcptDat->LockNo-1].posi;
		intoasl(&p_NtDat->Media[0].MediaCardNo[7], data, 4);	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	}

	ret = sizeof( DATA_KIND_61_r10 );
	return ret;
}
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(電文対応)

/*[]----------------------------------------------------------------------[]*/
/*| ｼｽﾃﾑﾒﾝﾃﾅﾝｽ操作                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005.04.12                                               |*/
/*| UpDate      : ｽｸﾛｰﾙ方式変更 2005-06-15 T.Hashimoto                     |*/
/*| UpDate      : font_change 2005-09-01 T.Hashimoto                       |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"LKmain.H"
#include	"PRM_TBL.H"
#include	"AppServ.h"
#include	"sysmnt_def.h"
#include	"fla_def.h"
#include	"ntnet.h"
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)

/* Function Define */
// MH810100(S) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)
//unsigned short	BPara_Set( void );
// MH810100(E) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)
unsigned short	CPara_Set( void );
unsigned short	RPara_Set( void );

unsigned short	SetDefault(ushort mode, char * p_f_DataChange);

void	pra_dp_1( long pno, short mod, short direction );
void	car_pra_dp( long pno, short mod, short direction );
void	rck_pra_dp( long pno, short mod, short direction );
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// void	pra_dp_2( long pno, short mod );
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
short	adrno( long address );
short	sesno( long address );
long	nextadd( long address, int mode );
char	isvalidadd( long add );
short	csesno( long address );
short	cadrno( long address );
long	cnextadd( long add, int mode );
char	cisvalidadd( long add );
unsigned long	cprmread( short carno, short caradd );
void	cprmwrit( short carno, short caradd, long data );
short	rsesno( long address );
short	radrno( long address );
long	rnextadd( long add, int mode );
char	risvalidadd( long add );
unsigned long	rprmread( short carno, short caradd );
void	rprmwrit( short carno, short caradd, long data );
long	prm_set(char kin, short ses, short adr, char len, char pos, long dat);

static short	gCurLoc;		// ｶｰｿﾙ位置の段数
static long		gPreAdr;		// 前回のｱﾄﾞﾚｽ値

short DispAdjuster;	// 車室パラメータ表示用調整値
const	ushort	Car_Start_Index[3] = {INT_CAR_START_INDEX, CAR_START_INDEX, BIKE_START_INDEX};

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// long	nomask_ses_next31( long add );
// long	nomask_ses_prev31( long add );
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
long	nomask_line_top( long pno, short curloc );
char	get_disp_line( long add );
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// char	is_nomask( long add );
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
unsigned short	PutGuideMessage( void );
char	IsRebootRequest( long add );

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
long get_nomask_ses_add_common( long current_add, int mode, PRM_MASK* p_mask, short mask_max );
long get_nomask_ses_add( long current_add, int mode );
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)

//未使用です。（S）
/*[]----------------------------------------------------------------------[]*/
/*| ﾊﾟﾗﾒｰﾀ設定ﾒﾆｭｰ画面                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ParSetMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short ParSetMain( void )
{
	unsigned short	usParaEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;


	for( ; ; )
	{

		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[0] );			// [00]	"＜パラメーター設定＞　　　　　"

		if(( rt_ctrl.param.set == ON ) ||			// 料金計算テストからの起動
		   ( rt_ctrl.param.no != 0 )){				// 料金計算テストからの戻り
			usParaEvent = BASC_PARA;
		}else{
			usParaEvent = Menu_Slt( PRMENU, PARA_SET_TBL, (char)PARA_SET_MAX, (char)1 );
		}

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usParaEvent ){

			case BASC_PARA:
				usParaEvent = BPara_Set();			// 基本ﾊﾟﾗﾒｰﾀ
				break;
			case CELL_PARA:
				usParaEvent = CPara_Set();			// 車室ﾊﾟﾗﾒｰﾀ
				break;
			case ROCK_PARA:
				usParaEvent = RPara_Set();			// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀ
				break;

			//Return Status of Prameter Menu
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usParaEvent;
				break;

			default:
				break;
		}
		//Return Status of Prameter Operation
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usParaEvent == MOD_CHG ){
		if( usParaEvent == MOD_CHG || usParaEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usParaEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
		if(( usParaEvent == MOD_EXT ) &&
		   (( rt_ctrl.param.set == ON ) || ( rt_ctrl.param.no != 0 ))){
			return MNT_RYOTS;
		}
	}
}
//未使用です。（S）

/*[]----------------------------------------------------------------------[]*/
/*| 基本ﾊﾟﾗﾒｰﾀ設定                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BPara_Set( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short BPara_Set( void )
{
	unsigned short	usBParaEvent;
	char	wk[2];
	char	org[2];
	T_FrmSetteiData FrmSetteiData;
	char	f_DataChange;
	uchar			priend;
	T_FrmPrnStop	FrmPrnStop;
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//	char	edy_DataChange;
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
	char	param_chk = 0;
	ushort	sntp_prm_after;		// SNTP同期時刻変更モニタログ登録用(設定34-0122の③④⑤⑥を保持)

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	// オート変数を初期化していないため、メニューのどこにも入らない場合は不定値になってしまうので先頭でクリア
	f_DataChange = 0;

	for( ; ; )
	{
		dispclr();
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[1] );			// [01]	"＜基本パラメーター設定＞　　　"
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[0] );			// [0]	"＜パラメーター設定＞　　　"
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)

		if(( rt_ctrl.param.set == ON ) ||			// 料金計算テストからの起動
		   ( rt_ctrl.param.no != 0 )){				// 料金計算テストからの戻り
			usBParaEvent = CMON_PARA;
		}else{
			usBParaEvent = Menu_Slt( BPRMENU, BPARA_SET_TBL, (char)BPARA_SET_MAX, (char)1 );
		}

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usBParaEvent ){

			case CMON_PARA:
				sntp_prm_before = (ushort)prm_get(COM_PRM, S_NTN, 122, 4, 1);		// SNTP同期時刻の値を取得
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//				usBParaEvent = SysParWrite( &f_DataChange, &edy_DataChange );
				usBParaEvent = SysParWrite( &f_DataChange );
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
				if( 1 == f_ParaUpdate.BIT.bpara ){					// changed parameter data
					DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
					(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		// FlashROM update
					wopelg( OPLOG_KYOTUPARAWT, 0, 0 );					// 操作履歴登録
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
					mnt_SetFtpFlag( FTP_REQ_WITH_POWEROFF );	// FTP更新フラグセット（要電断）
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					if( edy_DataChange == 1 )						// Edy関連設定変更？
//						wopelg( OPLOG_EDY_PRM_CHG,0,0 );			// 操作履歴登録(Edy関連設定変更)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					sntp_prm_after = (ushort)prm_get(COM_PRM, S_NTN, 122, 4, 1);
					if (sntp_prm_before != sntp_prm_after) {
					// SNTP同期時刻設定値に変更があった場合、M8036登録
						wopelg( OPLOG_SET_SNTP_SYNCHROTIME, (ulong)sntp_prm_before, (ulong)sntp_prm_after);
					}
					SetSetDiff(SETDIFFLOG_SYU_SYSMNT);				// 設定更新履歴ログ登録

					// この処理は重いうえ,毎回呼ぶ必要が無いので電源ON時と設定変更時のみCallする事とする。 
					mc10();		// 料金計算用設定ｾｯﾄ
//					LCD_renewal = (uchar)CPrmSS[S_PAY][27];			// ｺﾝﾄﾗｽﾄ調整のため再表示要求
// GG129001(S) データ保管サービス対応（登録番号をセットする）
					PrnGetRegistNum();
// GG129001(E) データ保管サービス対応（登録番号をセットする）
					param_chk = 1;
					f_ParaUpdate.BYTE = 0;
				}
				break;

			case INDV_PARA:
				break;

			case PRNT_PARA:
				if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
					BUZPIPI();
					break;
				}
				FrmSetteiData.prn_kind = R_PRI;
				FrmSetteiData.Kikai_no = (ushort)CPrmSS[S_PAY][2];
				queset( PRNTCBNO, PREQ_SETTEIDATA, sizeof(T_FrmSetteiData), &FrmSetteiData );
				Ope_DisableDoorKnobChime();
				/* プリント終了を待ち合わせる */
				Fun_Dsp(FUNMSG[82]);				// Fｷｰ表示　"　　　　　　 中止 　　　　　　"

				for ( priend = 0 ; priend == 0 ; ) {

					usBParaEvent = StoF( GetMessage(), 1 );

					if( (usBParaEvent&0xff00) == (INNJI_ENDMASK|0x0600) ){
						usBParaEvent &= (~INNJI_ENDMASK);
					}
					switch( usBParaEvent ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case LCD_DISCONNECT:
							priend = 1;
							break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case	KEY_MODECHG:		// ﾓｰﾄﾞﾁｪﾝｼﾞ
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
							// ドアノブの状態にかかわらずトグル動作してしまうので、
							// ドアノブ閉かどうかのチェックを実施
							if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
							priend = 1;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
							}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
							break;

						case	PREQ_SETTEIDATA:	// 印字終了
							priend = 1;
							break;

						case	KEY_TEN_F3:			// F3ｷｰ（中止）

							BUZPI();
							FrmPrnStop.prn_kind = R_PRI;
							queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// 印字中止要求
							priend = 1;
							break;
					}
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				if( usBParaEvent == KEY_MODECHG ){
				if( usBParaEvent == KEY_MODECHG || usBParaEvent == LCD_DISCONNECT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					if( param_chk ){
						usBParaEvent = parameter_upload_chk();
						param_chk = 0;
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//					}
//						return MOD_CHG;
//					}
					}else if ( usBParaEvent == KEY_MODECHG ){
						return MOD_CHG;
					}else{
						return MOD_CUT;
					}
				}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
				Fun_Dsp(FUNMSG[25]);				// Fｷｰ表示　"　▲　　▼　　　　 読出  終了 "

				break;

			case DEFA_PARA:
				f_DataChange = 0;
				usBParaEvent = SetDefault(DEFA_PARA, &f_DataChange);
				if( 1 == f_DataChange ){							// changed parameter data
					DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
					(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		// FlashROM update

					// この処理は重いうえ,毎回呼ぶ必要が無いので電源ON時と設定変更時のみCallする事とする。 
					mc10();		// 料金計算用設定ｾｯﾄ
//					LCD_renewal = (uchar)CPrmSS[S_PAY][27];			// ｺﾝﾄﾗｽﾄ調整のため再表示要求
					SetSetDiff(SETDIFFLOG_SYU_DEFLOAD);				// 設定更新履歴ログ登録
					SetDiffLogReset(2);								// 共通パラメータミラーリング
					// 車室パラメータの端末設定を共通パラメータに反映
					param_chk = 1;
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
					mnt_SetFtpFlag( FTP_REQ_WITH_POWEROFF );				// FTP更新フラグセット（要電断）
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
// GG129001(S) データ保管サービス対応（登録番号をセットする）
					PrnGetRegistNum();
// GG129001(E) データ保管サービス対応（登録番号をセットする）
				}
				break;

			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				if( param_chk ){
					usBParaEvent = parameter_upload_chk();
				}
				return usBParaEvent;
				break;
	
// MH810100(S) Y.Yamauchi 20191213 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 20191213 車番チケットレス(メンテナンス)
			case MOD_CHG:
				if( param_chk ){
					usBParaEvent = parameter_upload_chk();
				}
				return usBParaEvent;
				break;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
//		if( usBParaEvent == MOD_CHG ){
//			if( param_chk ){
		if( usBParaEvent == MOD_CHG ||  usBParaEvent == MOD_CUT ){
			if( param_chk &&  usBParaEvent == MOD_CHG ){
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
				usBParaEvent = parameter_upload_chk();
			}
			return usBParaEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
		if(( usBParaEvent == MOD_EXT ) &&
		   (( rt_ctrl.param.set == ON ) || ( rt_ctrl.param.no != 0 ))){
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス(メンテナンス)
//			return MOD_EXT;
		   	return MNT_RYOTS;
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス(メンテナンス)
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  共通ﾊﾟﾗﾒｰﾀ設定                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ret = SysParWrite();                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : short    ret;    0x00fe : F5 Key ( End Key )            |*/
/*|              :                  0x00ff : Maintenance Key               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 行ｽｸﾛｰﾙ方式にする                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/

#define Lin				(gCurLoc+2)
#define	TRIM6DGT(d)		(d % 1000000L)
#define	BPAR_TOP		(long)10001

// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//unsigned short	SysParWrite( char * p_f_DataChange, char * p_edy_DataChange )
unsigned short	SysParWrite( char * p_f_DataChange )
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
{
	unsigned short	msg;
	unsigned char	ddd[6];		//work buffer
	unsigned char	wk[14];		//Display address
	unsigned short	ret;
	long			no;			//Current address pointer
	long			address;	//Input address
	long			data;		//Input data
	char			r_md;		//Cursol position
	char			i;
	unsigned short	f_Reboot;

	*p_f_DataChange = 0;
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//	*p_edy_DataChange = 0;
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
	address = -1;
	data = -1;
	r_md = 0;
	ret = 0;
	f_Reboot = 0;
	if( rt_ctrl.param.no != 0 ){		// 料金計算テストからの戻り
		no = rt_ctrl.param.no;
		rt_ctrl.param.no = 0;
	}else{								// 通常起動
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 		if(OPECTL.Mnt_lev < 4){
// 		no = nomask_all_next( BPAR_TOP - 1 );
// 		}
// 		else{
// 			no = BPAR_TOP;
// 		}
		no = nomask_all_next(BPAR_TOP - 1);
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
	}

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[3] );	// [03]	"＜共通パラメーター設定＞　　　"
	if( rt_ctrl.param.set == ON ){										// 料金計算テストからの起動
		Fun_Dsp( FUNMSG[29] );											// [29]	"  ＋  －／読  →   取消  終了 "
	}else{																// 通常起動または料金計算テストからの戻り
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 		Fun_Dsp( FUNMSG[116] );											// [116]"  ▲  ▼／読  ⊃  テスト 終了 "
		if ( CLOUD_CALC_MODE ) {										// クラウド料金計算モード
			Fun_Dsp(FUNMSG[29]);										// [29]	"  ▲  ▼／読  ⊃         終了 "
		} else {														// 通常料金計算モード
			Fun_Dsp(FUNMSG[116]);										// [116]"  ▲  ▼／読  ⊃  テスト 終了 "
		}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
	}
																		// [30]	"  ▲    ▼    ⊂   書込  終了  "
																		//		   F1    F2    F3    F4    F5 
	gCurLoc = 0;
	gPreAdr = 0;
	pra_dp_1( no, 0, 2 );

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){

// MH810100(S) Y.Yamauchi 20191213 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 20191213 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:
				BUZPI();
				if( rt_ctrl.param.set == ON ){		// 料金計算テストからの起動
					rt_ctrl.param.no = 0;			// パラメータアドレスクリア
				}
				if( f_Reboot ){
					ret = PutGuideMessage();
					return( ret );
				}
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:						//F1 address"-"
				BUZPI();
				if( r_md == 0 ){
					address = -1;
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 				if(OPECTL.Mnt_lev < 4){
// 					no = nomask_prev( no );
// 				}
// 				else{
// 					no = nextadd( no, -1 );
// 				}
					no = nomask_prev(no);
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
					pra_dp_1( no, 0, 1 );
				}else{
					data = -1;
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 				if(OPECTL.Mnt_lev < 4){
// 					no = nomask_prev( no );
// 				}
// 				else{
// 					no = nextadd( no, -1 );			//F1 address"-"
// 				}
					no = nomask_prev(no);
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
					pra_dp_1( no, 1, 1 );
				}
				break;

			case KEY_TEN_F2:
				if( r_md == 0 ){				//F2 address"+"
					if( address == -1 ){
						BUZPI();
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 						if(OPECTL.Mnt_lev < 4){
// 							no = nomask_next( no );
// 						}
// 						else{
// 							no = nextadd( no, 1 );
// 						}
						no = nomask_next(no);
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
						pra_dp_1( no, 0, 0 );

					}else {						//F2 address set"XXXXXX"
						if( isvalidadd( address ) ){
							BUZPI();
							no = address;
							gCurLoc = 0;
							pra_dp_1( no, 0, 2 );
						}else {
							BUZPIPI();
						}
						address = -1;
					}
				}else{							//F2 address"+"
					BUZPI();
					data = -1;
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 					if(OPECTL.Mnt_lev < 4){
// 						no = nomask_next( no );
// 					}
// 					else{
// 						no = nextadd( no, 1 );
// 					}
					no = nomask_next(no);
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
					pra_dp_1( no, 1, 0 );
				}
				break;

			case KEY_TEN_F3:						//F3 Cursol shift"<-->"
				BUZPI();
				data = -1;
				address = -1;
				if( r_md == 0 ){
					r_md = 1;
					Fun_Dsp( FUNMSG[30] );
					pra_dp_1( no, 1, 2 );
				}else {
					r_md = 0;
					if( rt_ctrl.param.set == ON ){	// 料金計算テストからの起動
						Fun_Dsp( FUNMSG[29] );		// [29]	"  ▲  ▼／読  ⊃         終了 "
					}else{							// 通常起動または料金計算テストからの戻り
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 						Fun_Dsp( FUNMSG[116] );		// [112]]"  ＋  －／読  →  テスト 終了 "
						if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
							Fun_Dsp(FUNMSG[29]);	// [29]	"  ▲  ▼／読  ⊃         終了 "
						} else {					// 通常料金計算モード
							Fun_Dsp(FUNMSG[116]);	// [116]"  ▲  ▼／読  ⊃  テスト 終了 "
						}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
					}
					pra_dp_1( no, 0, 2 );
				}
				break;

			case KEY_TEN_F4:						// 書込
				if( r_md != 0 ){
					BUZPI();
					if( data != -1 ){
						for( i = 0; i < 6; i++ ){
							ddd[i] -= '0';
							if( ddd[i] > 9 ){
								ddd[i] -= 0x07;
							}
						}
						CPrmSS[sesno(no)][adrno(no)] = data;
						nmisave( &(CPrmSS[sesno(no)][adrno(no)]), &data, 4 );
						f_ParaUpdate.BIT.bpara = 1;
						*p_f_DataChange = 1;
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//						if( sesno(no) == S_SCA && ( adrno(no) >= 51 && adrno(no) <= 100 ))
//							*p_edy_DataChange = 1;
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
						if( f_Reboot == 0 ){
							if( IsRebootRequest( no ) ){
								f_Reboot = 1;
							}
						}
					}
					data = -1;
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 					if(OPECTL.Mnt_lev < 4){
// 						no = nomask_next( no );
// 					}
// 					else{
// 						no = nextadd( no, 1 );		//F1 address"+"
// 					}
					no = nomask_next(no);
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
					pra_dp_1( no, 1, 0 );
				}else{
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 					if( rt_ctrl.param.set != ON ){	// 料金計算テストからの起動でない
					if ( (rt_ctrl.param.set != ON) &&	// 料金計算テストからの起動でない
						 !CLOUD_CALC_MODE ) {			// 通常料金計算モード
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
						BUZPI();
						rt_ctrl.param.no = no;		// パラメータアドレスを保持
						return( MOD_EXT );
					}
				}
				break;

			case KEY_TEN_CL:						// Clear"C"
				BUZPI();
				if( r_md == 0 ){
					if( address != -1 ){
						address = -1;
					}
					//section no
					cnvdec2( (char *)ddd, sesno(no) );
					as1chg( ddd, wk, 2 );
					//"-"
					wk[4] = 0x81;		/* "－" */
					wk[5] = 0x7c;
					//address
					cnvdec4( (char *)ddd, (long)adrno(no) );
					as1chg( ddd, &wk[6], 4 );
					grachr ( (unsigned short)Lin,  2, 14, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					if( data != -1 ) {
						data = -1;
					}
					intoasl( (unsigned char *)ddd, (unsigned long)TRIM6DGT(CPrmSS[sesno(no)][adrno(no)]), 6 );
					as1chg( ddd, wk, 6 );
					grachr ( (unsigned short)Lin, 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;


			case KEY_TEN:
				BUZPI();
				if( r_md == 0 ){
					address = ( address == -1 ) ? (short)(msg - KEY_TEN0):
						( address % 100000 ) * 10 + (short)(msg - KEY_TEN0);//@
					//section no
					cnvdec2( (char *)ddd, sesno(address) );
					as1chg( ddd, wk, 2 );
					//"-"
					wk[4] = 0x81;		/* "－" */
					wk[5] = 0x7c;
					//address
					cnvdec4( (char *)ddd, (long)adrno(address) );
					as1chg( ddd, &wk[6], 4 );
					grachr ( (unsigned short)Lin,  2, 14, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					data = ( data == -1 ) ? (short)(msg - KEY_TEN0) :
											( data % 100000L ) * 10 + (short)(msg - KEY_TEN0);
					intoasl( (unsigned char *)ddd, (unsigned long)data, 6 );
					as1chg ( ddd, wk, 6 );
					grachr ( (unsigned short)Lin, 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*|  System Parameter Read                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pra_dp_1( pno, mod );                                   |*/
/*| PARAMETER    : short    pno;    Parameter No. ( 1 - 1499 )             |*/
/*|              :          mod;    Cursor Disp Position                   |*/
/*|                                   0 : Parameter No.                    |*/
/*|                                   1 : Parameter Data                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 行ｽｸﾛｰﾙ方式にする                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/

void pra_dp_1( long pno, short mod, short direction )
{
	short			i;
	long			incadd;
	unsigned short	lincnt;
	unsigned char	work[26];
	unsigned char	ddd[6];
	long			add;

	// incadd<=最上段に表示するｱﾄﾞﾚｽを求める。
	incadd = pno;
	if( sesno(gPreAdr) != sesno(pno) ){
		if( direction == 1 )
		{	// -方向
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 			if(OPECTL.Mnt_lev < 4){
// 				add = nomask_line_top( pno, 4 );
// 				if( add ){
// 					incadd = add;
// 				}
// 			}
// 			else{
// 				incadd = pno - 4;
// 			}
			add = nomask_line_top(pno, 4);
			if ( add ) {
				incadd = add;
			}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
		}
	}else{
		if( direction == 0 )
		{	// +方向
			if( gCurLoc >= 4 ){
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 				if(OPECTL.Mnt_lev < 4){
// 					add = nomask_line_top( pno, 4 );
// 					if( add ){
// 						incadd = add;
// 					}
// 				}
// 				else{
// 					incadd = pno - 4;
// 				}
				add = nomask_line_top(pno, 4);
				if ( add ) {
					incadd = add;
				}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
			}else{
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 				if(OPECTL.Mnt_lev < 4){
// 					add = nomask_line_top( pno, (short)(gCurLoc + 1) );
// 					if( add ){
// 						incadd = add;
// 					}
// 				}
// 				else{
// 					incadd = pno - gCurLoc - 1;
// 				}
				add = nomask_line_top(pno, (short)(gCurLoc + 1));
				if ( add ) {
					incadd = add;
				}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
			}
		}else if( direction == 1 )
		{	// -方向
			if( gCurLoc != 0 ){
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 				if(OPECTL.Mnt_lev < 4){
// 					add = nomask_line_top( pno, (short)(gCurLoc - 1) );
// 					if( add ){
// 						incadd = add;
// 					}
// 				}
// 				else{
// 					incadd = pno - gCurLoc + 1;
// 				}
				add = nomask_line_top(pno, (short)(gCurLoc - 1));
				if ( add ) {
					incadd = add;
				}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
			}
		}
		else
		{	// 左右移動
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 			if(OPECTL.Mnt_lev < 4){
// 				add = nomask_line_top( pno, gCurLoc );
// 				if( add ){
// 					incadd = add;
// 				}
// 			}
// 			else{
// 				incadd = pno - gCurLoc;
// 			}
			add = nomask_line_top(pno, gCurLoc);
			if ( add ) {
				incadd = add;
			}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
		}
	}
	lincnt = get_disp_line( incadd );

	for( i = 0; i < 5 ; i++ ){

		if( i >= lincnt ){
			displclr((unsigned short)(i + 2));
			continue;
		}

		//section no
		cnvdec2( (char *)ddd, sesno(incadd) );
		as1chg( ddd, work, 2 );
		//"-"
		work[4] = 0x81;		/* '－' */
		work[5] = 0x7c;
		//address
		cnvdec4( (char *)ddd, (long)adrno(incadd) );
		as1chg( ddd, &work[6], 4 );
		//data
		intoasl( (unsigned char *)ddd, (unsigned long)TRIM6DGT(CPrmSS[sesno(incadd)][adrno(incadd)]), 6 );
		as1chg( ddd, &work[14], 6 );

		if( pno == incadd ){

			gCurLoc = i;  // 現在ｶｰｿﾙ位置をｾｰﾌﾞ
			if( mod == 0 ){
				grachr( (unsigned short)(i+2), 2, 14, 1, COLOR_BLACK, LCD_BLINK_OFF, work );		//sec & add
				grachr( (unsigned short)(i+2), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );	//data
			} else {
				grachr( (unsigned short)(i+2), 2, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+2), 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );
			}
		} else {
				grachr( (unsigned short)(i+2), 2, 14, 0,  COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+2), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );
		}

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 		if(OPECTL.Mnt_lev < 4){
// 			incadd = nomask_ses_next( incadd );
// 		}
// 		else{
// 			incadd = nextadd(incadd, 1);
// 		}
		incadd = nomask_ses_next(incadd);
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
	}
	gPreAdr = pno;	// 前回ｱﾄﾞﾚｽをｾｰﾌﾞ
	return;
}

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// /*[]----------------------------------------------------------------------[]*/
// /*|  System Parameter Read                                                 |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : pra_dp_2( pno, mod );                                   |*/
// /*| PARAMETER    : short    pno;    Parameter No. ( 1 - 1499 )             |*/
// /*|              :          mod;    Cursor Disp Position                   |*/
// /*|                                   0 : Parameter No.                    |*/
// /*|                                   1 : Parameter Data                   |*/
// /*| RETURN VALUE : void                                                    |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
// 
// void pra_dp_2( long pno, short mod )
// {
// 	short			i;
// 	long			incadd;
// 	unsigned char	work[26];
// 	unsigned char	ddd[6];
// 	long			BPAR_BTM;
// 	BPAR_BTM = (C_PRM_SESCNT_MAX-1)*10000L;
// 	BPAR_BTM += CPrmCnt[C_PRM_SESCNT_MAX-1];
// 
// 	if(OPECTL.Mnt_lev < 4){
// 		incadd = nomask_all_prev( pno );
// 		if( incadd == 0 ){
// 			// 該当データなしの場合、最後尾から探し直し
// 			incadd = nomask_all_prev( BPAR_BTM + 1 );
// 		}
// 		incadd = nomask_all_prev( incadd );
// 		if( incadd == 0 ){
// 			// 該当データなしの場合、最後尾から探し直し
// 			incadd = nomask_all_prev( BPAR_BTM + 1 );
// 		}
// 	}
// 	else{
// 		incadd = nextadd( pno, -1 );
// 		incadd = nextadd( incadd, -1 );
// 	}
// 
// 	for( i = 0; i < 5; i++ ){
// 
// 		//section no
// 		cnvdec2( (char *)ddd, sesno(incadd) );
// 		as1chg( ddd, work, 2 );
// 		//"-"
// 		work[4] = 0x81;		/* "－" */
// 		work[5] = 0x7c;
// 		//address
// 		cnvdec4( (char *)ddd, (long)adrno(incadd) );
// 		as1chg( ddd, &work[6], 4 );
// 		//data
// 		intoasl( (unsigned char *)ddd, (unsigned long)TRIM6DGT(CPrmSS[sesno(incadd)][adrno(incadd)]), 6 );
// 		as1chg( ddd, &work[14], 6 );
// 
// 		if( i == 2 ){	//fix to 3rd line
// 
// 			if( mod == 0 ){
// 				grachr( (unsigned short)(i+2), 2, 14, 1, COLOR_BLACK, LCD_BLINK_OFF, work );		//sec & add
// 				grachr( (unsigned short)(i+2), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );	//data
// 			} else {
// 				grachr( (unsigned short)(i+2), 2, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, work );
// 				grachr( (unsigned short)(i+2), 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );
// 			}
// 		} else {
// 				grachr( (unsigned short)(i+2), 2, 14, 0,  COLOR_BLACK, LCD_BLINK_OFF, work );
// 				grachr( (unsigned short)(i+2), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );
// 		}
// 
// 		if(OPECTL.Mnt_lev < 4){
// 			incadd = nomask_ses_prev( incadd );
// 		}
// 		else{
// 			incadd = nextadd(incadd, 1);
// 		}
// 	}
// 	return;
// }
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)

/* ＠アドレスセクションを返す */
short	sesno( long address )
{

	return (short)(address/10000L);
}
/* ＠アドレスを返す */
short	adrno( long address )
{
	return (short)(address%10000L);
}
/* ＠前後のアドレス(6digit)を返す */
long	nextadd( long add, int mode )
{

	long nextadd;
	long			BPAR_BTM;
	BPAR_BTM = (C_PRM_SESCNT_MAX-1)*10000L;
	BPAR_BTM += CPrmCnt[C_PRM_SESCNT_MAX-1];
	nextadd = 0;

	if( 1 == mode ){

		if( add >= BPAR_BTM ){

			nextadd = BPAR_TOP;

		}else if( CPrmCnt[sesno(add)] < adrno(add) + 1 ){
			// ｾｸｼｮﾝの最後であれば次ｾｸｼｮﾝの最初にする
			nextadd = 1 + ((sesno(add) + 1) * 10000L);

		}else{

			nextadd = add + 1;
		}

	}else if( -1 == mode ){

		if( add <= BPAR_TOP ){

			nextadd = BPAR_BTM;

		}else if( 0 >= (adrno(add)-1) ){
			// ｾｸｼｮﾝの最初であれば前ｾｸｼｮﾝの最初にする
			nextadd = CPrmCnt[sesno(add) - 1] + (sesno(add) - 1) * 10000L;

		}else{

			nextadd = add - 1;
		}
	}else{

	}
	return nextadd;
}
/* ＠テンキー入力値は有効アドレス？ */
char	isvalidadd( long add )
{
	char ret = 1;

	if( ( C_PRM_SESCNT_MAX - 1 < sesno(add) )||
		( sesno(add) == 0 ) ||
		( CPrmCnt[sesno(add)] < adrno(add) ) ||
		( adrno(add) == 0 ) )
	{
		ret = 0;
	}

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	if(OPECTL.Mnt_lev < 4){
// 		if( ret ){
// 			// 非マスクアドレスか？
// 			ret = is_nomask( add );
// 		}
// 	}
	// マスクアドレスであれば無効
	if ( is_nomask(add) == 0 ) {
		ret = 0;
	}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 車室ﾊﾟﾗﾒｰﾀｰ設定処理                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CPara_Set()                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0xfffe : F5 Key ( End Key )                       |*/
/*|              :       0xffff : 設定ﾓｰﾄﾞｽｲｯﾁOFF                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-04-14                                              |*/
/*| Update       : 行ｽｸﾛｰﾙ方式にする                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CPara_Set( void )
{
	unsigned short	usCParaEvent;
	char	wk[2];
	char	org[2];
	T_FrmLockPara	FrmLockPara;
	uchar			priend;
	T_FrmPrnStop	FrmPrnStop;
	char	f_DataChange;
	long	bk_prk;

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[2] );							// "＜車室パラメーター設定＞　　　"

		usCParaEvent = Menu_Slt( CPRMENU, CPARA_SET_TBL, (char)CPARA_SET_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usCParaEvent ){
			case CARP_PARA:
				if(( bk_prk = Is_CarMenuMake( CAR_3_MENU )) == 0 ){
					BUZPIPI();
					continue;
				}
				do {
					if( Ext_Menu_Max > 1 ){
						dispclr();
						grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[71]);		/* "＜車室パラメーター＞　　　　　" */
						gCurSrtPara = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
					}else{
						gCurSrtPara = (short)bk_prk;
						bk_prk = 0;
					}
					switch ((ushort)gCurSrtPara) {
					case MOD_EXT:
						break;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case MOD_CHG:
						return (ushort)gCurSrtPara;
						break;
					default:
						// 表示用調整値設定
						DispAdjuster = Car_Start_Index[gCurSrtPara-MNT_INT_CAR];
					

						f_DataChange = 0;
						usCParaEvent = CarParWrite( &f_DataChange );

						if( 1 == f_ParaUpdate.BIT.cpara ) {					// changed parameter data
							wopelg( OPLOG_SHASHITUPARAWT, 0, 0 );			// 操作履歴登録
							DataSumUpdate(OPE_DTNUM_LOCKINFO);				// RAM上のSUM更新
							FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);			// FLASH書込み
							if( prm_get(COM_PRM,S_NTN,39,1,2) == 1 ){
								NTNET_Snd_Data225(0);						// NTNETへデータ送信
							}
							f_ParaUpdate.BYTE = 0;
						}

						if (usCParaEvent == MOD_EXT) break;
						if (usCParaEvent == MOD_CHG) return MOD_CHG;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						if (usCParaEvent == LCD_DISCONNECT ) return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
					}
				} while (bk_prk && (gCurSrtPara != (short)MOD_EXT));
				break;
			case CARP_PRNT:
				if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
					BUZPIPI();
					break;
				}
				Cal_Parameter_Flg = 0;
				FrmLockPara.prn_kind = R_PRI;
				FrmLockPara.Kikai_no = (ushort)CPrmSS[S_PAY][2];
				queset( PRNTCBNO, PREQ_LOCK_PARA, sizeof(T_FrmLockPara), &FrmLockPara );
				Ope_DisableDoorKnobChime();
				/* プリント終了を待ち合わせる */
				Fun_Dsp(FUNMSG[82]);				// Fｷｰ表示　"　　　　　　 中止 　　　　　　"

				for ( priend = 0 ; priend == 0 ; ) {

					usCParaEvent = StoF( GetMessage(), 1 );

					if( (usCParaEvent&0xff00) == (INNJI_ENDMASK|0x0600) ){
						usCParaEvent &= (~INNJI_ENDMASK);
					}
					switch( usCParaEvent ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case LCD_DISCONNECT:	
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case	KEY_MODECHG:		// ﾓｰﾄﾞﾁｪﾝｼﾞ
							priend = 1;
							break;

						case	PREQ_LOCK_PARA:		// 印字終了
							priend = 1;
							break;

						case	KEY_TEN_F3:			// F3ｷｰ（中止）

							BUZPI();
							FrmPrnStop.prn_kind = R_PRI;
							queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// 印字中止要求
							priend = 1;
							break;
					}
				}
				if( usCParaEvent == KEY_MODECHG ){
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				if( usCParaEvent == LCD_DISCONNECT ){
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				Fun_Dsp(FUNMSG[25]);				// Fｷｰ表示　"　▲　　▼　　　　 読出  終了 "

				break;

			case CARP_DEFA:
				f_DataChange = 0;
				usCParaEvent = SetDefault(CARP_DEFA, &f_DataChange);
				if( 1 == f_DataChange ){							// changed parameter data
					DataSumUpdate(OPE_DTNUM_LOCKINFO);				/* RAM上のSUM更新 */
					FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);	/* FLASH書込み */
					// 車室パラメータの端末設定を共通パラメータに反映
					if( prm_get(COM_PRM,S_NTN,39,1,2) == 1 ){
						NTNET_Snd_Data225(0);						// NTNETへデータ送信
					}
				}
				break;
			case MOD_EXT:											// F5 Key
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return( usCParaEvent );
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)	
			case MOD_CHG:											// 設定ﾓｰﾄﾞｽｲｯﾁOFF
				return( usCParaEvent );
				break;
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usCParaEvent == KEY_MODECHG ){
		if( usCParaEvent == KEY_MODECHG ||usCParaEvent == LCD_DISCONNECT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			return( usCParaEvent );
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 車室ﾊﾟﾗﾒｰﾀｰ設定書換え処理                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CarParWrite()                                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0xfffe : F5 Key ( End Key )                       |*/
/*|              :       0xffff : 設定ﾓｰﾄﾞｽｲｯﾁOFF                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define CLin			(gCurLoc+1)
#define	CarAddMax		6
#define	IntCarAddMax	6

#define	CCPAR_TOP		(((CAR_START_INDEX+1)*100L)+1)							// 車室ﾊﾟﾗﾒｰﾀｰ開始ｱﾄﾞﾚｽ(車室1-ｱﾄﾞﾚｽ01)
#define	CCPAR_BTM		(((CAR_START_INDEX+CAR_LOCK_MAX)*100L)+CarAddMax)		// 車室ﾊﾟﾗﾒｰﾀｰ終了ｱﾄﾞﾚｽ(車室50-ｱﾄﾞﾚｽ6)
#define	CICPAR_TOP		(((INT_CAR_START_INDEX+1)*100L)+1)						// 車室ﾊﾟﾗﾒｰﾀｰ開始ｱﾄﾞﾚｽ(車室51-ｱﾄﾞﾚｽ01)
#define	CICPAR_BTM		(((INT_CAR_START_INDEX+INT_CAR_LOCK_MAX)*100L)+IntCarAddMax)// 車室ﾊﾟﾗﾒｰﾀｰ終了ｱﾄﾞﾚｽ(車室100-ｱﾄﾞﾚｽ6)
#define	CBPAR_TOP		(((BIKE_START_INDEX+1)*100L)+1)							// 車室ﾊﾟﾗﾒｰﾀｰ開始ｱﾄﾞﾚｽ(車室101-ｱﾄﾞﾚｽ01)
#define	CBPAR_BTM		(((BIKE_START_INDEX+BIKE_LOCK_MAX)*100L)+CarAddMax)		// 車室ﾊﾟﾗﾒｰﾀｰ終了ｱﾄﾞﾚｽ(車室150-ｱﾄﾞﾚｽ6)

unsigned short	CarParWrite( char * p_f_DataChange )
{
	unsigned short	msg;
	unsigned char	ddd[6];		//work buffer
	unsigned char	wk[12];		//Display address
	unsigned short	ret;
	long			no;			//Current address pointer
	long			address;	//Input address
	long			data;		//Input data
	char			r_md;		//Cursol position

	address = -1;
	data = -1;
	r_md = 0;
	ret = 0;

	dispclr();	
	if (gCurSrtPara == MNT_CAR) {
	// 駐車
		no = CCPAR_TOP;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[8] );		// [06]	"＜車室パラメーター（駐車ＩＦ）＞"
	}
	else if (gCurSrtPara == MNT_INT_CAR) {
	// 駐車（内蔵）
		no = CICPAR_TOP;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[6] );		// [06]	"＜車室パラメーター（駐車）＞　"
	}
	else if (gCurSrtPara == MNT_BIK) {
	// 駐輪
		no = CBPAR_TOP;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[7] );		// [07]	"＜車室パラメーター（駐輪）＞　"
	}
	
	Fun_Dsp( FUNMSG[29] );					// [29]	"  ＋  －／読  →   取消  終了 "
											// [30]	"       書込   ←   取消  終了 "
											//		   F1    F2    F3    F4    F5 
	gCurLoc = 0;
	gPreAdr = 0;
	car_pra_dp( no, 0, 2 );

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:
				BUZPI();
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:						//F1 address"-"
				BUZPI();
				if( r_md == 0 ){
					address = -1;
					no = cnextadd( no, -1 );
					car_pra_dp( no, 0, 1 );
				}else{
					data = -1;
					no = cnextadd( no, -1 );		//F1 address"-"
					car_pra_dp( no, 1, 1 );
				}
				break;

			case KEY_TEN_F2:
				if( r_md == 0 ){				//F2 address"+"
					if( address == -1 ){
						BUZPI();
						no = cnextadd( no, 1 );
						car_pra_dp( no, 0, 0 );

					}else {						//F2 address set"XXXXXX"
						if( cisvalidadd( address ) ){
							BUZPI();
							no = address;
							no += (DispAdjuster * 100);
							gCurLoc = 0;
							car_pra_dp( no, 0, 2 );
						}else {
							BUZPIPI();
						}
						address = -1;
					}
				}
				else{							//F2 address"+"
					BUZPI();
					data = -1;
					no = cnextadd( no, 1 );
					car_pra_dp( no, 1, 0 );
				}
				break;

			case KEY_TEN_F3:						//F3 Cursol shift"<-->"
				BUZPI();
				data = -1;
				address = -1;
				if( r_md == 0 ){
					r_md = 1;
					Fun_Dsp( FUNMSG[30] );
					car_pra_dp( no, 1, 2 );
				}else {
					r_md = 0;
					Fun_Dsp( FUNMSG[29] );
					car_pra_dp( no, 0, 2 );
				}
				break;

			case KEY_TEN_F4:
				if( r_md != 0 ){
					BUZPI();
					if( data != -1 ){
						cprmwrit( csesno(no), cadrno(no), data );
						f_ParaUpdate.BIT.cpara = 1;
						*p_f_DataChange = 1;
					}
					data = -1;
					no = cnextadd( no, 1 );		//F1 address"+"
					car_pra_dp( no, 1, 0 );
				}
				break;

			case KEY_TEN_CL:						// Clear
				BUZPI();
				if( r_md == 0 ){
					if( address != -1 ){
						address = -1;
					}
					//section no
// MH322914 (s) kasiyama 2016/07/20 車室パラメータアドレス表示NG対策(共通改善No.1255)(GM567102)
//					intoas( ddd, (ushort)csesno(no), 3 );
					intoas( ddd, (ushort)(csesno(no) - DispAdjuster), 3 );
// MH322914 (e) kasiyama 2016/07/20 車室パラメータアドレス表示NG対策(共通改善No.1255)(GM567102)
					as1chg( ddd, wk, 3 );
					//"-"
					wk[6] = 0x81;	/* "－" */
					wk[7] = 0x7c;
					//address
					intoas( ddd, (ushort)cadrno(no), 2 );
					as1chg( ddd, &wk[8], 2 );
					grachr ( (unsigned short)CLin,  2, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					if( data != -1 ){
						data = -1;
					}
					intoasl( ddd, cprmread( csesno(no), cadrno(no) ), 6 );
					as1chg( ddd, wk, 6 );
					grachr( (unsigned short)CLin, 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;

			case KEY_TEN:
				BUZPI();
				if( r_md == 0 ){
					address = ( address == -1 ) ? (short)(msg - KEY_TEN0):
						( address % 10000 ) * 10 + (short)(msg - KEY_TEN0);//@
					//section no
					intoas( ddd, (ushort)csesno(address), 3 );
					as1chg( ddd, wk, 3 );
					//"-"
					wk[6] = 0x81;	/* "－" */
					wk[7] = 0x7c;
					//address
					intoas( ddd, (ushort)cadrno(address), 2 );
					as1chg( ddd, &wk[8], 2 );
					grachr ( (unsigned short)CLin,  2, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					data = ( data == -1 ) ? (short)(msg - KEY_TEN0) :
											( data % 100000L ) * 10 + (short)(msg - KEY_TEN0);
					intoasl( ddd, (unsigned long)data, 6 );
					as1chg ( ddd, wk, 6 );
					grachr ( (unsigned short)CLin,  18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;
		}
	}
	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| 車室ﾊﾟﾗﾒｰﾀｰ設定読取処理                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : car_pra_dp()                                            |*/
/*| PARAMETER    : pno : Parameter No. ( 1-01 ～ 540-13 )                  |*/
/*|                mod : Cursor Disp Position  0 = Parameter No.           |*/
/*|                                            1 = Parameter Data          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 行ｽｸﾛｰﾙ方式にする                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	car_pra_dp( long pno, short mod, short direction )
{
	short			i;
	long			incadd;
	unsigned short	lincnt;
	unsigned char	work[24];
	unsigned char	ddd[6];

	// incadd<=最上段に表示するｱﾄﾞﾚｽを求める。
	incadd = pno;
	if( csesno(gPreAdr) != csesno(pno) ){
		if( direction == 1 )
		{	// -方向
			if( incadd >= CICPAR_TOP && incadd <= CICPAR_BTM ) {
				incadd = pno - 5;
			}
			else {
			incadd = pno - 5;
			}
		}
	}else{
		if( direction == 0 )
		{	// +方向
			if( incadd >= CICPAR_TOP && incadd <= CICPAR_BTM ) {
				if( gCurLoc >= 5 ){
					incadd = pno - 5;
				}else{
					incadd = pno - gCurLoc - 1;
				}
			}
			else {
			if( gCurLoc >= 5 ){
				incadd = pno - 5;
			}else{
				incadd = pno - gCurLoc - 1;
			}
			}
		}else if( direction == 1 )
		{	// -方向
			if( gCurLoc != 0 ){
				incadd = pno - gCurLoc + 1;
			}
		}
		else
		{	// 左右移動
			incadd = pno - gCurLoc;
		}
	}

	if( incadd >= CICPAR_TOP && incadd <= CICPAR_BTM ) {
		lincnt = ( IntCarAddMax < cadrno( incadd + 5 ) ) ?
					 ( IntCarAddMax - cadrno(incadd) + 1 ) : 6;
	}
	else {
	lincnt = ( CarAddMax < cadrno( incadd + 5 ) ) ?
				 ( CarAddMax - cadrno(incadd) + 1 ) : 6;
	}

	for( i = 0; i < 6; i++ ){

		if( i >= lincnt ){
			displclr((unsigned short)(i + 1));
			continue;
		}

		//car no
		intoas( ddd, (ushort)(csesno(incadd) - DispAdjuster), 3 );
		as1chg( ddd, work, 3 );
		//"-"
		work[6] = 0x81;	/* "－" */
		work[7] = 0x7c;
		//address
		intoas( ddd, (ushort)cadrno(incadd), 2 );
		as1chg( ddd, &work[8], 2 );
		//data
		intoasl( ddd, cprmread( csesno(incadd), cadrno(incadd) ), 6 );
		as1chg( ddd, &work[12], 6 );

		if( pno == incadd ){

			gCurLoc = i; // 現在ｶｰｿﾙ位置をｾｰﾌﾞ

			if( mod == 0 ){
				grachr( (unsigned short)(i+1), 2, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, work );		// car no
				grachr( (unsigned short)(i+1), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[12] );	// data
			} else {
				grachr( (unsigned short)(i+1), 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+1), 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, &work[12] );
			}
		} else {
			grachr( (unsigned short)(i+1), 2, 12, 0,  COLOR_BLACK, LCD_BLINK_OFF, work );
			grachr( (unsigned short)(i+1), 18, 12, 0,  COLOR_BLACK, LCD_BLINK_OFF, &work[12] );
		}

		incadd = cnextadd(incadd, 1);
	}
	gPreAdr = pno;	// 前回ｱﾄﾞﾚｽをｾｰﾌﾞ
	return;
}

/* 車室№を返す */
short	csesno( long address )
{
	return (short)(address/100L);
}

/* アドレスを返す */
short	cadrno( long address )
{
	return (short)(address%100L);
}

/* ＠前後のアドレス(6digi)を返す */
long	cnextadd( long add, int mode )
{

	long nextadd;
	long add_top, add_btm;
	long add_max;
	
	add_max = 0;
	add_top = 0;
	nextadd = 0;
	add_btm = 0;
	if (gCurSrtPara == MNT_CAR) {
		add_top = CCPAR_TOP;
		add_btm = CCPAR_BTM;
		add_max = CarAddMax;
	}
	else if (gCurSrtPara == MNT_INT_CAR){
		add_top = CICPAR_TOP;
		add_btm = CICPAR_BTM;
		add_max = IntCarAddMax;
	}
	else if (gCurSrtPara == MNT_BIK){
		add_top = CBPAR_TOP;
		add_btm = CBPAR_BTM;
		add_max = CarAddMax;
	}
	
	if( 1 == mode ){

		if ( add >= add_btm ) {
			nextadd = add_top;
		}else if( add_max < cadrno(add) + 1 ){
			nextadd = 1 + ((csesno(add) + 1) * 100L);
		}else{
			nextadd = add + 1;
		}

	}else if( -1 == mode ){

		if ( add <= add_top ) {
			nextadd = add_btm;
		}else if( 0 >= cadrno(add) - 1 ){
			nextadd = add_max + (csesno(add) - 1) * 100L;
		}else{
			nextadd = add - 1;
		}
	}
	return nextadd;
}
/* ＠テンキー入力値は有効アドレス？ */
char	cisvalidadd( long add )
{
	char ret = 1;
	long min, max;
	short c_no, c_ad;
	long add_max;
	
	add_max = 0;
	max = 0;
	min = 0;
	if (gCurSrtPara == MNT_CAR) {
		min = CAR_START_INDEX + 1;
		max = CAR_START_INDEX + CAR_LOCK_MAX;
		add_max = CarAddMax;
	}
	else if (gCurSrtPara == MNT_INT_CAR){
		min = INT_CAR_START_INDEX + 1;
		max = INT_CAR_START_INDEX + INT_CAR_LOCK_MAX;
		add_max = IntCarAddMax;
	}
	else if (gCurSrtPara == MNT_BIK){
		min = BIKE_START_INDEX + 1;
		max = BIKE_START_INDEX + BIKE_LOCK_MAX;
		add_max = CarAddMax;
	}

	c_no = csesno(add) + DispAdjuster;
	c_ad = cadrno(add);

	if( ( min > c_no ) || ( max < c_no ) || ( c_no == 0 ) ||
		( add_max < c_ad ) || ( c_ad == 0 ) )
	{
		ret = 0;
	}
	return( ret );
}

/* 車室設定読取 */
unsigned long	cprmread( short carno, short caradd )
{
	ulong	ret = 0;

	switch( caradd ){
		case 1:
			ret = (ulong)LockInfo[carno-1].lok_syu;
			break;
		case 2:
			ret = (ulong)LockInfo[carno-1].ryo_syu;
			break;
		case 3:
			ret = (ulong)LockInfo[carno-1].area;
			break;
		case 4:
			ret = (ulong)LockInfo[carno-1].posi;
			break;
		case 5:
			ret = (ulong)LockInfo[carno-1].if_oya;
			break;
		case 6:
			if (gCurSrtPara == MNT_BIK) {	// 駐輪の場合はそのまま
				ret = (ulong)LockInfo[carno-1].lok_no;
			} else {						// 駐車の場合、変換が必要
				ret = (ulong)LockInfo[carno-1].lok_no + PRM_LOKNO_MIN;	// 表示用に+100をする
			}
			break;
		default:
			break;
	}

	return ret;
}

/* 車室設定書込み */
void	cprmwrit( short carno, short caradd, long data )
{
	switch( caradd ){
		case 1:
			LockInfo[carno-1].lok_syu = (uchar)data;
			break;
		case 2:
			LockInfo[carno-1].ryo_syu = (uchar)data;
			break;
		case 3:
			LockInfo[carno-1].area = (uchar)data;
			break;
		case 4:
			LockInfo[carno-1].posi = (ulong)data;
			break;
		case 5:
			LockInfo[carno-1].if_oya = (uchar)data;
			break;
		case 6:
			if (gCurSrtPara == MNT_BIK) {	// 駐輪の場合はそのまま
				LockInfo[carno-1].lok_no = (uchar)data;
			} else {						// 駐車の場合、変換が必要
				if( data < PRM_LOKNO_MIN || data > PRM_LOKNO_MAX ){	// 範囲チェック( 100～315 有効 )
					LockInfo[carno-1].lok_no = 0;								// 範囲外の場合、0(接続無状態)を保存
				} else {
					LockInfo[carno-1].lok_no = (uchar)(data - PRM_LOKNO_MIN);	// 範囲内の場合、100を引いて保存する(保存領域が1byteのため)
				}
			}
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾛｯｸ装置ﾊﾟﾗﾒｰﾀｰ設定処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RPara_Set()                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0xfffe : F5 Key ( End Key )                       |*/
/*|              :       0xffff : 設定ﾓｰﾄﾞｽｲｯﾁOFF                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Tanaka                                                  |*/
/*| Date         : 2005-04-27                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	RPara_Set( void )
{
	unsigned short	usRParaEvent;
	char	wk[2];
	char	org[2];
	T_FrmLockSettei	FrmLockSettei;
	char	f_DataChange;

	if (!(GetCarInfoParam() & 0x01)) {	// 駐輪場併設なしの場合
		BUZPIPI();
		return MOD_EXT;
	}

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[5] );							// "＜ﾛｯｸ種別ﾊﾟﾗﾒｰﾀｰ設定＞　　　　 "

		usRParaEvent = Menu_Slt( RPRMENU, RPARA_SET_TBL, (char)RPARA_SET_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usRParaEvent ){
			case RCKP_PARA:
				f_DataChange = 0;
				usRParaEvent = RckParWrite( &f_DataChange );
				if( 1 == f_DataChange ){							// changed parameter data
					wopelg( OPLOG_ROCKPARAWT, 0, 0 );			// 操作履歴登録
				}
				break;
			case RCKP_PRNT:
				if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
					BUZPIPI();
					break;
				}
				FrmLockSettei.prn_kind = R_PRI;
				FrmLockSettei.Kikai_no = (ushort)CPrmSS[S_PAY][2];
				queset( PRNTCBNO, PREQ_LOCK_SETTEI, sizeof(T_FrmLockSettei), &FrmLockSettei );
				Ope_DisableDoorKnobChime();
				/* プリント終了を待ち合わせる */
				Lagtim(OPETCBNO, 6,1000);		/* 20sec timer start */
				for ( ; ; ) {
					usRParaEvent = StoF( GetMessage(), 1 );
					/* プリント終了 */
					if((usRParaEvent&0xff00) == (INNJI_ENDMASK|0x0600)) {
						Lagcan(OPETCBNO, 6);
						break;
					}
					/* タイムアウト(20秒)検出 */
					if (usRParaEvent == TIMEOUT6) {
						BUZPIPI();
						break;
					}
					/* モードチェンジ */
					if (usRParaEvent == KEY_MODECHG) {
						BUZPI();
						Lagcan(OPETCBNO, 6);
						return MOD_CHG;
					}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					if ( usRParaEvent == LCD_DISCONNECT ) {
						Lagcan(OPETCBNO, 6);
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				}
				break;
			case RCKP_DEFA:
				f_DataChange = 0;
				usRParaEvent = SetDefault(RCKP_DEFA, &f_DataChange);
				if( 1 == f_DataChange ){							// changed parameter data
				}
				break;
			case MOD_EXT:											// F5 Key
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return( usRParaEvent );
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)						
			case MOD_CHG:											// 設定ﾓｰﾄﾞｽｲｯﾁOFF
				return( usRParaEvent );
				break;
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//		if( usRParaEvent == MOD_CHG ){
		if( usRParaEvent == MOD_CHG || usRParaEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			return( usRParaEvent );
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾛｯｸ種別ﾊﾟﾗﾒｰﾀｰ設定書換え処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RckParWrite()                                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0xfffe : F5 Key ( End Key )                       |*/
/*|              :       0xffff : 設定ﾓｰﾄﾞｽｲｯﾁOFF                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Tanaka                                                  |*/
/*| Date         : 2005-04-27                                              |*/
/*| Update       : 行ｽｸﾛｰﾙ方式にする                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	ROCK_MAX	6
#define	RLin		(gCurLoc+1)
#define	RckAddMax	6
#define	RPAR_TOP	101L						// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀｰ開始ｱﾄﾞﾚｽ(01-ｱﾄﾞﾚｽ01)
#define	RPAR_BTM	(ROCK_MAX*100)+RckAddMax	// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀｰ終了ｱﾄﾞﾚｽ(06-ｱﾄﾞﾚｽ06)

unsigned short	RckParWrite( char * p_f_DataChange )
{
	unsigned short	msg;
	unsigned char	ddd[6];		//work buffer
	unsigned char	wk[12];		//Display address
	unsigned short	ret;
	long			no;			//Current address pointer
	long			address;	//Input address
	long			data;		//Input data
	char			r_md;		//Cursol position

	address = -1;
	data = -1;
	r_md = 0;
	ret = 0;
	no = RPAR_TOP;

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[5] );		// [05]	"＜ﾛｯｸ種別ﾊﾟﾗﾒｰﾀｰ設定＞　　　　 "
	Fun_Dsp( FUNMSG[29] );												// [29]	"  ＋  －／読  →   取消  終了 "
																		// [30]	"       書込   ←   取消  終了 "
																		//		   F1    F2    F3    F4    F5 
	gCurLoc = 0;
	gPreAdr = 0;
	rck_pra_dp( no, 0, 2 );

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:
				BUZPI();
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:						//F1 address"-"
				BUZPI();
				if( r_md == 0 ){
					BUZPI();
					address = -1;
					no = rnextadd( no, -1 );
					rck_pra_dp( no, 0, 1 );
				}else{							//F1 Write
					data = -1;
					no = rnextadd( no, -1 );		//F1 address"-"
					rck_pra_dp( no, 1, 1 );
				}
				break;

			case KEY_TEN_F2:
				if( r_md == 0 ){				//F2 address"+"
					if( address == -1 ){
						BUZPI();
						no = rnextadd( no, 1 );
						rck_pra_dp( no, 0, 0 );
					}else {						//F2 address set"XXXXXX"
						if( risvalidadd( address ) ){
							BUZPI();
							no = address;
							gCurLoc = 0;
							rck_pra_dp( no, 0, 2 );
						}else {
							BUZPIPI();
						}
						address = -1;
					}
				}else{							//F2 address"+"
					BUZPI();
					data = -1;
					no = rnextadd( no, 1 );
					rck_pra_dp( no, 1, 0 );
				}
				break;

			case KEY_TEN_F3:						//F3 Cursol shift"<-->"
				BUZPI();
				data = -1;
				address = -1;
				if( r_md == 0 ){
					r_md = 1;
					Fun_Dsp( FUNMSG[30] );
					rck_pra_dp( no, 1, 2 );
				}else {
					r_md = 0;
					Fun_Dsp( FUNMSG[29] );
					rck_pra_dp( no, 0, 2 );
				}
				break;

			case KEY_TEN_F4:						//F4
				if( r_md != 0 ){
					BUZPI();
					if( data != -1 ){
						rprmwrit( rsesno(no), radrno(no), data );
						*p_f_DataChange = 1;
					}
					data = -1;
					no = rnextadd( no, 1 );		//F1 address"+"
					rck_pra_dp( no, 1, 0 );
				}
				break;

			case KEY_TEN_CL:						// Clear
				BUZPI();
				if( r_md == 0 ){
					if( address != -1 ){
						address = -1;
					}
					//section no
					intoas( ddd, (ushort)rsesno(no), 2 );
					as1chg( ddd, wk, 2 );
					//"-"
					wk[4] = 0x81;	/* "－" */
					wk[5] = 0x7c;
					//address
					intoas( ddd, (ushort)radrno(no), 2 );
					as1chg( ddd, &wk[6], 2 );
					grachr ( (unsigned short)RLin,  2, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					if( data != -1 ){
						data = -1;
					}
					intoasl( ddd, rprmread( rsesno(no), radrno(no) ), 6 );
					as1chg( ddd, wk, 6 );
					grachr( (unsigned short)RLin, 16, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;

			case KEY_TEN:
				BUZPI();
				if( r_md == 0 ){
					address = ( address == -1 ) ? (short)(msg - KEY_TEN0):
						( address % 1000 ) * 10 + (short)(msg - KEY_TEN0);//@
					//section no
					intoas( ddd, (ushort)rsesno(address), 2 );
					as1chg( ddd, wk, 2 );
					//"-"
					wk[4] = 0x81;	/* "－" */
					wk[5] = 0x7c;
					//address
					intoas( ddd, (ushort)radrno(address), 2 );
					as1chg( ddd, &wk[6], 2 );
					grachr ( (unsigned short)RLin,  2, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					data = ( data == -1 ) ? (short)(msg - KEY_TEN0) :
											( data % 100000L ) * 10 + (short)(msg - KEY_TEN0);
					intoasl( ddd, (unsigned long)data, 6 );
					as1chg ( ddd, wk, 6 );
					grachr ( (unsigned short)RLin,  16, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 車室ﾊﾟﾗﾒｰﾀｰ設定読取処理                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rck_pra_dp()                                            |*/
/*| PARAMETER    : pno : Parameter No. ( 1-01 ～ 540-13 )                  |*/
/*|                mod : Cursor Disp Position  0 = Parameter No.           |*/
/*|                                            1 = Parameter Data          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Tanaka                                                  |*/
/*| Date         : 2005-04-27                                              |*/
/*| Update       : 行ｽｸﾛｰﾙ方式にする                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	rck_pra_dp( long pno, short mod, short direction )
{
	short			i;
	long			incadd;
	unsigned short	lincnt;
	unsigned char	work[22];
	unsigned char	ddd[6];

	// incadd<=最上段に表示するｱﾄﾞﾚｽを求める。
	incadd = pno;
	if( rsesno(gPreAdr) != rsesno(pno) ){
		if( direction == 1 )
		{	// -方向
			incadd = pno - 5;
		}
	}else{
		if( direction == 0 )
		{	// +方向
			if( gCurLoc >= 5 ){
				incadd = pno - 5;
			}else{
				incadd = pno - gCurLoc - 1;
			}
		}else if( direction == 1 )
		{	// -方向
			if( gCurLoc != 0 ){
				incadd = pno - gCurLoc + 1;
			}
		}
		else
		{	// 左右移動
			incadd = pno - gCurLoc;
		}
	}
	lincnt = ( RckAddMax < radrno( incadd + 5 ) ) ?
				 ( RckAddMax - radrno(incadd) + 1 ) : 6;

	for( i = 0; i < 6; i++ ){

		if( i >= lincnt ){
			displclr((unsigned short)(i + 1));
			continue;
		}

		//car no
		intoas( ddd, (ushort)rsesno(incadd), 2 );
		as1chg( ddd, work, 2 );
		//"-"
		work[4] = 0x81;	/* "－" */
		work[5] = 0x7c;
		//address
		intoas( ddd, (ushort)radrno(incadd), 2 );
		as1chg( ddd, &work[6], 2 );
		//data
		intoasl( ddd, rprmread( rsesno(incadd), radrno(incadd) ), 6 );
		as1chg( ddd, &work[10], 6 );

		if( pno == incadd ){

			gCurLoc = i; // 現在ｶｰｿﾙ位置をｾｰﾌﾞ

			if( mod == 0 ){
				grachr( (unsigned short)(i+1), 2, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, work );		// car no
				grachr( (unsigned short)(i+1), 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[10] );	// data
			} else {
				grachr( (unsigned short)(i+1), 2, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+1), 16, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, &work[10] );
			}
		} else {
				grachr( (unsigned short)(i+1), 2, 10, 0,  COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+1), 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[10] );
		}

		incadd = rnextadd(incadd, 1);
	}
	gPreAdr = pno;	// 前回ｱﾄﾞﾚｽをｾｰﾌﾞ
	return;
}

/* 車室№を返す */
short	rsesno( long address )
{
	return (short)(address/100L);
}

/* アドレスを返す */
short	radrno( long address )
{
	return (short)(address%100L);
}

/* ＠前後のアドレス(6digi)を返す */
long	rnextadd( long add, int mode )
{

	long nextadd = 0;

	if( 1 == mode ){

		if( add >= RPAR_BTM ){

			nextadd = RPAR_TOP;

		}else if( RckAddMax < radrno(add) + 1 ){

			nextadd = 1 + ((rsesno(add) + 1) * 100L);

		}else{

			nextadd = add + 1;
		}

	}else if( -1 == mode ){

		if( add <= RPAR_TOP ){

			nextadd = RPAR_BTM;

		}else if( 0 >= radrno(add) - 1 ){

			nextadd = RckAddMax + (rsesno(add) - 1) * 100L;

		}else{

			nextadd = add - 1;
		}

	}
	return nextadd;
}
/* ＠テンキー入力値は有効アドレス？ */
char	risvalidadd( long add )
{
	char ret = 1;

	if( ( ROCK_MAX < rsesno(add) ) ||
		( rsesno(add) == 0 ) ||
		( RckAddMax < radrno(add) ) ||
		( radrno(add) == 0 ) )
	{
		ret = 0;
	}
	return( ret );
}

/* 車室設定読取 */
unsigned long	rprmread( short rckno, short rckadd )
{
	ulong	ret = 0;

	switch( rckadd ){
		case 1:
			ret = (ulong)LockMaker[rckno-1].in_tm;
			break;
		case 2:
			ret = (ulong)LockMaker[rckno-1].ot_tm;
			break;
		case 3:
			ret = (ulong)LockMaker[rckno-1].r_cnt;
			break;
		case 4:
			ret = (ulong)LockMaker[rckno-1].r_tim;
			break;
		case 5:
			ret = (ulong)LockMaker[rckno-1].open_tm;
			break;
		case 6:
			ret = (ulong)LockMaker[rckno-1].clse_tm;
			break;
	}

	return ret;
}

/* 車室設定書込み */
void	rprmwrit( short rckno, short rckadd, long data )
{
	switch( rckadd ){
		case 1:
			LockMaker[rckno-1].in_tm = (uchar)data;
			break;
		case 2:
			LockMaker[rckno-1].ot_tm = (uchar)data;
			break;
		case 3:
			LockMaker[rckno-1].r_cnt = (uchar)data;
			break;
		case 4:
			LockMaker[rckno-1].r_tim = (ushort)data;
			break;
		case 5:
			LockMaker[rckno-1].open_tm = (uchar)data;
			break;
		case 6:
			LockMaker[rckno-1].clse_tm = (uchar)data;
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| パラメーター初期化                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SetDefault( type )                                      |*/
/*| PARAMETER    : ushort mode : DEFA_PARA 基本パラメーター                |*/
/*|              :             : CARP_DEFA 車室パラメーター                |*/
/*|              :             : RCKP_DEFA ロック種別パラメーター          |*/
/*|              : char * p_f_DataChange : defaultｾｯﾄを実行したﾌﾗｸﾞ        |*/
/*|              :                         1=実行した, 0=してない          |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : 設定ﾓｰﾄﾞｽｲｯﾁOFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	SetDefault( ushort mode, char * p_f_DataChange )
{
	ushort	msg;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[14]);		/* "＜デフォルトセット＞　　　　　" */
	if (mode == DEFA_PARA) {
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[10]);	/* "　共通パラメータを　　　　　　" */
	}
	else if (mode == CARP_DEFA) {
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[11]);	/* "　車室パラメーターを　　　　　" */
	}
	else {	/* if (type == RCKP_DEFA) */
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[12]);	/* "　ロック種別パラメーターを　　" */
	}
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[13]);		/* "　　デフォルトにしますか？　　" */
	Fun_Dsp(FUNMSG[19]);													/* "　　　　　　 はい いいえ　　　" */

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F3:	/* "はい" */
			/* デフォルト値をセットする */
			if (mode == DEFA_PARA) {
				prm_clr(0, 1, 0);
			}
			else if (mode == CARP_DEFA) {
				lockinfo_clr(1);
			}
			else {	/* if (type == RCKP_DEFA) */
				lockmaker_clr(1);
			}
			*p_f_DataChange = 1;
		case KEY_TEN_F4:	/* "いいえ" */
			BUZPI();
			return MOD_EXT;
		default:
			break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 次の非マスクデータのアドレスを返す                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_next                                             |*/
/*| PARAMETER    : long add : 現在のアドレス（６桁）                       |*/
/*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
/*|              :          : 0 以外 = 次のアドレス                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_next( long add )
{
	long	ret = 0, tmp = 0;

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	tmp = nomask_ses_next( add );
// 	if( tmp == 0 ){
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
	// 該当データなしの場合、次のセクションを探す
	tmp = nomask_all_next( add );
	if( tmp == 0 ){
		// 該当データなしの場合、先頭から探し直す
		ret = nomask_all_next( BPAR_TOP - 1 );
	}else{
		ret = tmp;
	}
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	}else{
// 		ret = tmp;
// 	}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 前の非マスクデータのアドレスを返す                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_prev                                             |*/
/*| PARAMETER    : long add : 現在のアドレス（６桁）                       |*/
/*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
/*|              :          : 0 以外 = 次のアドレス                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_prev( long add )
{
	long	ret = 0, tmp = 0;
	long	BPAR_BTM;
	BPAR_BTM = (C_PRM_SESCNT_MAX-1)*10000L;
	BPAR_BTM += CPrmCnt[C_PRM_SESCNT_MAX-1];

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	tmp = nomask_ses_prev( add );
// 	if( tmp == 0 ){
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
	// 該当データなしの場合、前のセクションを探す
	tmp = nomask_all_prev( add );
	if( tmp == 0 ){
		// 該当データなしの場合、最後尾から探し直す
		ret = nomask_all_prev( BPAR_BTM + 1 );
	}else{
		ret = tmp;
	}
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	}else{
// 		ret = tmp;
// 	}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
	return ret;
}
/*[]----------------------------------------------------------------------[]*/
/*| 設定全体から次の非マスクデータのアドレスを返す                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_all_next                                         |*/
/*| PARAMETER    : long add : 現在のアドレス（６桁）                       |*/
/*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
/*|              :          : 0 以外 = 次のアドレス                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_all_next( long add )
{
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	long	ret = 0, ret31 = 0;
// 	short	i;
// 
// 	add = nextadd( add, 1 );
// 
// 	// セクション３１の場合は別対応
// 	if( sesno(add) == S_RAT ){
// 		ret31 = nomask_ses_next31(add);
// 		if( ret31 ){
// 			// セクション３１内で非マスクアドレスが見つかった
// 			return ret31;
// 		}
// 		// 見つからなかった場合は次のセクションの先頭から通常対応に戻って継続する
// 		add = 1 + (sesno(add)+1) * 10000L;
// 	}
// 
// 	for( i=0; i<prm_mask_max; i++ ){
// 		if( sesno(param_mask[i].from) < sesno(add) ){
// 			// 検索対象よりセクション番号が小さい場合はスキップする
// 			continue;
// 		}
// 		if( add < param_mask[i].from ){
// 			// 非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 		if( add <= param_mask[i].to ){
// 			// マスク対象なので、to の次のアドレスへ
// 			add = nextadd( param_mask[i].to, 1 );
// 			if( sesno(param_mask[i].from) > sesno(add) ){
// 				// 一周した場合は該当データなしとする
// 				break;
// 			}
// 
// 			// セクション３１の場合は別対応
// 			if( sesno(add) == S_RAT ){
// 				ret31 = nomask_ses_next31(add);
// 				if( ret31 ){
// 					// セクション３１内で非マスクアドレスが見つかった
// 					return ret31;
// 				}
// 				// 見つからなかった場合は次のセクションの先頭から通常対応に戻って継続する
// 				add = 1 + (sesno(add)+1) * 10000L;
// 			}
// 
// 		}
// 		// マスク範囲を超えた
// 		if( i == prm_mask_max-1 ){
// 			// 最後の要素なら、非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
	long	ret = 0;
	long	next_add = add;
	short	i;

	// セクション内の次の非マスクアドレス取得を試みる
	next_add = nomask_ses_next(next_add);

	if ( next_add != 0 ) {	// 取得成功
		ret = next_add;	// 非マスクアドレス
	} else {				// 取得失敗
		for ( i = sesno(add) + 1; i < C_PRM_SESCNT_MAX; i++ ) {
			// セクション内の最初のアドレス-1を設定
			next_add = (i * 10000L);

			// 次セクションの非マスクアドレス取得を試みる
			next_add = nomask_ses_next(next_add);

			// 取得成功したら非マスクアドレスを返す
			if ( next_add != 0 ) {
				ret = next_add;	// 非マスクアドレス
				break;
			}
		}
	}

	return ret;
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
}

/*[]----------------------------------------------------------------------[]*/
/*| 設定全体から前の非マスクデータのアドレスを返す                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_all_prev                                         |*/
/*| PARAMETER    : long add : 現在のアドレス（６桁）                       |*/
/*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
/*|              :          : 0 以外 = 前のアドレス                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_all_prev( long add )
{
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	long	ret = 0, ret31 = 0;
// 	short	i;
// 
// 	add = nextadd( add, -1 );
// 
// 	// セクション３１の場合は別対応
// 	if( sesno(add) == S_RAT ){
// 		ret31 = nomask_ses_prev31(add);
// 		if( ret31 ){
// 			// セクション３１内で非マスクアドレスが見つかった
// 			return ret31;
// 		}
// 		// 見つからなかった場合は前のセクションの最後尾から通常対応に戻って継続する
// 		add = CPrmCnt[(sesno(add)-1)] + (sesno(add)-1) * 10000L;
// 	}
// 
// 	for( i=prm_mask_max-1; i>=0; i-- ){
// 		if( sesno(param_mask[i].from) > sesno(add) ){
// 			// 検索対象よりセクション番号が大きい場合はスキップする
// 			continue;
// 		}
// 		if( add > param_mask[i].to ){
// 			// 非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 		if( add >= param_mask[i].from ){
// 			// マスク対象なので、from の前のアドレスへ
// 			add = nextadd( param_mask[i].from, -1 );
// 			if( sesno(param_mask[i].from) < sesno(add) ){
// 				// 一周した場合は該当データなしとする
// 				break;
// 			}
// 
// 			// セクション３１の場合は別対応
// 			if( sesno(add) == S_RAT ){
// 				ret31 = nomask_ses_prev31(add);
// 				if( ret31 ){
// 					// セクション３１内で非マスクアドレスが見つかった
// 					return ret31;
// 				}
// 				// 見つからなかった場合は前のセクションの最後尾から通常対応に戻って継続する
// 				add = CPrmCnt[(sesno(add)-1)] + (sesno(add)-1) * 10000L;
// 			}
// 
// 		}
// 		// マスク範囲を超えた
// 		if( i == 0 ){
// 			// 先頭の要素なら、非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
	long	ret = 0;
	long	prev_add = add;
	short	i;

	// セクション内の前の非マスクアドレス取得を試みる
	prev_add = nomask_ses_prev(prev_add);

	if ( prev_add != 0 ) {	// 取得成功
		ret = prev_add;	// 非マスクアドレス
	} else {				// 取得失敗
		for ( i = sesno(add) - 1; i > 0; i-- ) {
			// セクション内の最後のアドレス+1を設定
			prev_add = (i * 10000L) + CPrmCnt[i] + 1;

			// 前セクションの非マスクアドレス取得を試みる
			prev_add = nomask_ses_prev(prev_add);

			// 取得成功したら非マスクアドレスを返す
			if ( prev_add != 0 ) {
				ret = prev_add;	// 非マスクアドレス
				break;
			}
		}
	}

	return ret;
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
}

/*[]----------------------------------------------------------------------[]*/
/*| セクション内で次の非マスクアドレスを返す                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_ses_next                                         |*/
/*| PARAMETER    : long add : 現在のアドレス（６桁）                       |*/
/*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
/*|              :          : 0 以外 = 次のアドレス                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_ses_next( long add )
{
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	long	ret = 0, keep = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 
// 	add = nextadd( add, 1 );
// 	if( sesno(add) != sno ){
// 		// セクション番号が一致しない場合は該当データなし
// 		return ret;
// 	}
// 
// 	// セクション３１の場合は別対応
// 	if( sno == S_RAT ){
// 		ret = nomask_ses_next31(add);
// 		return ret;
// 	}
// 
// 	// 該当セクションにマスクが一件もない場合に備えて保存する
// 	keep = add;
// 
// 	for( i=0; i<prm_mask_max; i++ ){
// 		if( sesno(param_mask[i].from) < sno ){
// 			// 検索対象よりセクション番号が小さい場合はスキップする
// 			continue;
// 		}
// 		if( sesno(param_mask[i].from) > sno ){
// 			// 検索対象よりセクション番号が大きい場合（セクションの切替り）
// 			if( keep ){
// 				// 保持アドレスがあれば戻り値として返す。なければ該当データなし
// 				ret = add;
// 			}
// 			break;
// 		}
// 		// 以下、マスク判定
// 		keep = 0;
// 		if( add < param_mask[i].from ){
// 			// 非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 		if( add <= param_mask[i].to ){
// 			// マスク対象なので、to の次のアドレスへ
// 			add = nextadd( param_mask[i].to, 1 );
// 			if( sesno(add) != sno ){
// 				// セクションが変わった為、該当データなし
// 				break;
// 			}
// 		}
// 		// マスク範囲を超えた
// 		// セクションの切替りを考慮して一旦保存
// 		keep = add;
// 		if( i == prm_mask_max-1 ){
// 			// 最後の要素なら、非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
	long	next_add = 0;

	// 次のアドレスを取得
	next_add = nextadd(add, 1);

	// セクションが切り替わった
	if ( sesno(next_add) != sesno(add) ) {
		return 0;	// 該当データなし
	}

	// 次の非マスクアドレスを取得
	next_add = get_nomask_ses_add(next_add, 1);

	return next_add;
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
}

/*[]----------------------------------------------------------------------[]*/
/*| セクション内で前の非マスクアドレスを返す                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_ses_prev                                         |*/
/*| PARAMETER    : long add : 現在のアドレス（６桁）                       |*/
/*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
/*|              :          : 0 以外 = 前のアドレス                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_ses_prev( long add )
{
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	long	ret = 0, keep = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 
// 	add = nextadd( add, -1 );
// 	if( sesno(add) != sno ){
// 		// セクション番号が一致しない場合は該当データなし
// 		return ret;
// 	}
// 
// 	// セクション３１の場合は別対応
// 	if( sno == S_RAT ){
// 		ret = nomask_ses_prev31(add);
// 		return ret;
// 	}
// 
// 	// 該当セクションにマスクが一件もない場合に備えて保存する
// 	keep = add;
// 
// 	for( i=prm_mask_max-1; i>=0; i-- ){
// 		if( sesno(param_mask[i].from) > sno ){
// 			// 検索対象よりセクション番号が大きい場合はスキップする
// 			continue;
// 		}
// 		if( sesno(param_mask[i].from) < sno ){
// 			// 検索対象よりセクション番号が小さい場合（セクションの切替り）
// 			if( keep ){
// 				// 保持データがあれば戻り値として返す。なければ該当データなし
// 				ret = add;
// 			}
// 			break;
// 		}
// 		// 以下、マスク判定
// 		keep = 0;
// 		if( add > param_mask[i].to ){
// 			// 非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 		if( add >= param_mask[i].from ){
// 			// マスク対象なので、from の前のアドレスへ
// 			add = nextadd( param_mask[i].from, -1 );
// 			if( sesno(add) != sno ){
// 				// セクションが変わった為、該当データなし
// 				break;
// 			}
// 		}
// 		// マスク範囲を超えた
// 		// セクションの切替りを考慮して一旦保存
// 		keep = add;
// 		if( i == 0 ){
// 			// 最初の要素なら、非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
	long	prev_add = 0;

	// 前のアドレスを取得
	prev_add = nextadd(add, -1);

	// セクションが切り替わった
	if ( sesno(prev_add) != sesno(add) ) {
		return 0;	// 該当データなし
	}

	// 次の非マスクアドレスを取得
	prev_add = get_nomask_ses_add(prev_add, -1);

	return prev_add;
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
}

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// /*[]----------------------------------------------------------------------[]*/
// /*| セクション３１内で次の非マスクアドレスを返す                           |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : nomask_ses_next31                                       |*/
// /*| PARAMETER    : long add : 現在のアドレス（６桁）                       |*/
// /*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
// /*|              :          : 0 以外 = 次のアドレス                        |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
// long	nomask_ses_next31( long add )
// {
// 	long	ret = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 	short	mask_max = 0;
// 	PRM_MASK	*pmask = 0;
// 
// 	// セクション３１でない場合は該当データなしを返す
// 	if( sno != S_RAT ){
// 		return ret;
// 	}
// 
// 	// セクション３１（料金設定）の対応
// 	if( CPrmSS[S_CAL][1] == 0 ){
// 		// 昼夜帯
// 		mask_max = prm_mask31_0_max;
// 		pmask = (PRM_MASK *)&param_mask31_0[0];
// 
// 	}else if( CPrmSS[S_CAL][1] == 1 ){
// 		// 逓減帯
// 		mask_max = prm_mask31_1_max;
// 		pmask = (PRM_MASK *)&param_mask31_1[0];
// 	}
// 
// 	for( i=0; i<mask_max; i++ ){
// 		if( add < pmask[i].from ){
// 			// 非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 		if( add <= pmask[i].to ){
// 			// マスク対象なので、to の次のアドレスへ
// 			add = nextadd( pmask[i].to, 1 );
// 			if( sesno(add) != sno ){
// 				// セクションが変わった為、該当データなし
// 				break;
// 			}
// 		}
// 		// マスク範囲を超えた
// 		if( i == mask_max-1 ){
// 			// 最後の要素なら、非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
// }
// 
// /*[]----------------------------------------------------------------------[]*/
// /*| セクション３１内で前の非マスクアドレスを返す                           |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : nomask_ses_prev31                                       |*/
// /*| PARAMETER    : long add : 現在のアドレス（６桁）                       |*/
// /*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
// /*|              :          : 0 以外 = 前のアドレス                        |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
// long	nomask_ses_prev31( long add )
// {
// 	long	ret = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 	short	mask_max = 0;
// 	PRM_MASK	*pmask = 0;
// 
// 	// セクション３１でない場合は該当データなしを返す
// 	if( sno != S_RAT ){
// 		return 0;
// 	}
// 
// 	// セクション３１（料金設定）の対応
// 	if( CPrmSS[S_CAL][1] == 0 ){
// 		// 昼夜帯
// 		mask_max = prm_mask31_0_max;
// 		pmask = (PRM_MASK *)&param_mask31_0[0];
// 
// 	}else if( CPrmSS[S_CAL][1] == 1 ){
// 		// 逓減帯
// 		mask_max = prm_mask31_1_max;
// 		pmask = (PRM_MASK *)&param_mask31_1[0];
// 	}
// 
// 	for( i=mask_max-1; i>=0; i-- ){
// 		if( add > pmask[i].to ){
// 			// 非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 		if( add >= pmask[i].from ){
// 			// マスク対象なので、from の前のアドレスへ
// 			add = nextadd( pmask[i].from, -1 );
// 			if( sesno(add) != sno ){
// 				// セクションが変わった為、該当データなし
// 				break;
// 			}
// 		}
// 		// マスク範囲を超えた
// 		if( i == 0 ){
// 			// 最初の要素なら、非マスクアドレスなので戻り値として返す
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
// }
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)

// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
//[]----------------------------------------------------------------------[]
///	@brief			セクション内の非マスクアドレスを取得(マスクデータ毎共通処理)
//[]----------------------------------------------------------------------[]
///	@param[in]		current_add	: 現在のアドレス
///	@param[in]		mode		: モード<br>
///								    1=次の非マスクアドレスを返す<br>
///								    0=現在のアドレスが非マスクアドレスか確認<br>
///								   -1=前の非マスクアドレスを返す<br>
///	@param[in]		p_mask		: マスクデータテーブル
///	@param[in]		mask_max	: マスクデータ件数
///	@return			ret			: 0 = 該当なし、マスクアドレス<br>
///								  0以外 = 非マスクアドレス<br>
///	@note			現在のアドレスからチェックする
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long get_nomask_ses_add_common( long current_add, int mode, PRM_MASK* p_mask, short mask_max )
{
	long	ret = 0;
	long	next_add = current_add;
	short	current_ses = sesno(current_add);
	short	i;

	// マスクデータのチェック
	if ( mode >= 0 ) {			// 次、確認
		for ( i = 0; i < mask_max; i++ ) {
			// 検索対象よりセクション番号が小さい
			if ( sesno(p_mask[i].from) < sesno(next_add) ) {
				continue;	// スキップ
			}

			// 検索対象よりセクション番号が大きい(同一セクション)
			if ( sesno(p_mask[i].from) > sesno(next_add) ) {
				ret = next_add;	// 非マスクアドレス
				break;
			}

			// アドレスが非マスクアドレス
			if ( next_add < p_mask[i].from ) {
				ret = next_add;	// 非マスクアドレス
				break;
			}

			// アドレスがマスクアドレス
			if ( next_add <= p_mask[i].to ) {
				// 現在のアドレスの確認ならここで終了
				if ( mode == 0 ) {
					return 0;	// マスクアドレス
				}

				// マスクアドレスなのでマスク終了アドレスの次を設定
				next_add = nextadd(p_mask[i].to, 1);

				// セクションが切り替わった
				if ( sesno(next_add) != current_ses ) {
					return 0;	// 該当データなし
				}
			}
		}

		// 最後の要素
		if ( i == mask_max ) {
			ret = next_add;	// 非マスクアドレス
		}
	} else {					// 前
		for ( i = mask_max - 1; i >= 0; i-- ) {
			// 検索対象よりセクション番号が大きい
			if ( sesno(p_mask[i].from) > sesno(next_add) ) {
				continue;	// スキップ
			}

			// 検索対象よりセクション番号が小さい
			if ( sesno(p_mask[i].from) < sesno(next_add) ) {
				ret = next_add;	// 非マスクアドレス
				break;
			}

			// アドレスが非マスクアドレス
			if ( next_add > p_mask[i].to ) {
				ret = next_add;	// 非マスクアドレス
				break;
			}

			// アドレスがマスクアドレス
			if ( next_add >= p_mask[i].from ) {
				// マスクアドレスなのでマスク終了アドレスの前を設定
				next_add = nextadd(p_mask[i].from, -1);

				// セクションが切り替わった
				if ( sesno(next_add) != current_ses ) {
					return 0;	// 該当データなし
				}
			}
		}

		// 最初の要素
		if ( i < 0 ) {
			ret = next_add;	// 非マスクアドレス
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			セクション内の非マスクアドレスを取得
//[]----------------------------------------------------------------------[]
///	@param[in]		current_add	: 現在のアドレス
///	@param[in]		mode		: モード<br>
///								    1=次の非マスクアドレスを返す<br>
///								    0=現在のアドレスが非マスクアドレスか確認<br>
///								   -1=前の非マスクアドレスを返す<br>
///	@return			ret			: 0 = 該当なし、マスクアドレス<br>
///								  0以外 = 非マスクアドレス<br>
///	@note			現在のアドレスからチェックする
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long get_nomask_ses_add( long current_add, int mode )
{
	long	next_add = current_add;
	long	temp_add;
	int		check_flg = 0;
	short	current_ses = sesno(current_add);

	if ( OPECTL.Mnt_lev < 4 ) {	// 技術者権限以下
		check_flg = 1;	// チェックする
	}

	while ( check_flg ) {
		// チェック開始時のアドレスを保持
		temp_add = next_add;

		// 技術者権限では設定禁止とするアドレスをマスク
		if ( current_ses == S_RAT ) {	// セクション31(料金設定)
			if ( CPrmSS[S_CAL][1] == 1 ) {	// 逓減帯
				next_add = get_nomask_ses_add_common(next_add, mode,
					(PRM_MASK*)&param_mask31_1[0], prm_mask31_1_max);
			} else {						// 昼夜帯
				next_add = get_nomask_ses_add_common(next_add, mode,
					(PRM_MASK*)&param_mask31_0[0], prm_mask31_0_max);
			}
		} else {						// セクション31(料金設定)以外
			next_add = get_nomask_ses_add_common(next_add, mode, 
				(PRM_MASK*)&param_mask[0], prm_mask_max);
		}

		// クラウド料金計算では設定禁止とするアドレスをマスク
		if ( ( next_add != 0 ) && CLOUD_CALC_MODE ) {	// 非マスクアドレス＆クラウド料金計算モード
			next_add = get_nomask_ses_add_common(next_add, mode, 
				(PRM_MASK*)&param_mask_cc[0], prm_mask_max_cc);
		}

		if ( (next_add == 0) ||			// 該当なし、マスクアドレス
			 (next_add == temp_add) ) {	// 全てのチェックで非マスクアドレス
			check_flg = 0;	// チェック終了
		}
	}

	return next_add;
}

//[]----------------------------------------------------------------------[]
///	@brief			非マスクアドレスか確認
//[]----------------------------------------------------------------------[]
///	@param[in]		current_add	: 現在のアドレス
///	@return			ret			: 0 = 該当なし、マスクアドレス<br>
///								  0以外 = 非マスクアドレス<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long is_nomask( long current_add )
{
	return get_nomask_ses_add(current_add, 0);
}
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)

/*[]----------------------------------------------------------------------[]*/
/*| カーソル位置に応じた画面最上段の非マスクアドレスを返す                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_line_top                                         |*/
/*| PARAMETER    : long pno : 現在のアドレス（６桁）                       |*/
/*|              : short curloc : カーソル位置                             |*/
/*| RETURN VALUE : long ret : 0 = 該当データなし                           |*/
/*|              :          : 0 以外 = 前のアドレス                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_line_top( long pno, short curloc )
{
	long	ret = 0, add = 0;
	short	i;

	add = pno;
	for( i=0; i<curloc; i++ ){
		// カーソル位置から先頭行まで遡って非マスクアドレスを取得する
		add = nomask_ses_prev(add);
		if( add == 0 ){
			break;
		}
		ret = add;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 次回画面表示する行数を返す                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : get_disp_line                                           |*/
/*| PARAMETER    : long add : 先頭行のアドレス（６桁）                     |*/
/*| RETURN VALUE : char line_cnt : 表示行数                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
char	get_disp_line( long add )
{
	char	line_cnt = 1;	// 最低１行は表示する
	short	i;

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	if(OPECTL.Mnt_lev < 4){
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
		for( i=0; i<4; i++ ){
			// セクション内の次の非マスクアドレスを取得する
			add = nomask_ses_next(add);
			if( add == 0 ){
				// 次の該当データなし
				break;
			}
			line_cnt++;
		}
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// 	}
// 	else{
// 		line_cnt = ( CPrmCnt[sesno(add)] < adrno(add + 4) ) ?
// 					 (CPrmCnt[sesno(add)] - adrno(add) + 1) : 5;
// 	}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)

	return line_cnt;
}

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
// /*[]----------------------------------------------------------------------[]*/
// /*| 指定アドレスが非マスクアドレスか判定する                               |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : is_nomask                                               |*/
// /*| PARAMETER    : long add : 設定アドレス（６桁）                         |*/
// /*| RETURN VALUE : char ret : 0 = マスクする                               |*/
// /*|                         : 1 = マスクしない or 該当データなし           |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
// char	is_nomask( long add )
// {
// 	char	ret = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 	short	mask_max = 0;
// 	PRM_MASK	*pmask = 0;
// 
// 	// セクション３１？
// 	if( sno == S_RAT ){
// 		// セクション３１（料金設定）の対応
// 		if( CPrmSS[S_CAL][1] == 0 ){
// 			// 昼夜帯
// 			mask_max = prm_mask31_0_max;
// 			pmask = (PRM_MASK *)&param_mask31_0[0];
// 
// 		}else if( CPrmSS[S_CAL][1] == 1 ){
// 			// 逓減帯
// 			mask_max = prm_mask31_1_max;
// 			pmask = (PRM_MASK *)&param_mask31_1[0];
// 		}
// 	}else{
// 		mask_max = prm_mask_max;
// 		pmask = (PRM_MASK *)&param_mask[0];
// 	}
// 
// 	for( i=0; i<mask_max; i++ ){
// 		if( sesno(pmask[i].from) < sno ){
// 			// 検索対象よりセクション番号が小さい場合はスキップする
// 			continue;
// 		}
// 		if( sesno(pmask[i].from) > sno ){
// 			// 検索対象よりセクション番号が大きくなった場合は非マスクアドレス
// 			ret = 1;
// 			break;
// 		}
// 		if( add < pmask[i].from ){
// 			// 非マスクアドレスなので OK を返す
// 			ret = 1;
// 			break;
// 		}
// 		if( add <= pmask[i].to ){
// 			// マスク対象なので NG を返す
// 			break;
// 		}
// 	}
// // MH810104(S) R.Endo 2021/09/25 車番チケットレス フェーズ2.3 #6012 【個別部隊指摘】60-0201以降（QR/バーコードフォーマットパターン設定）のアドレスのマスクを外しても（非表示→表示）ジャンプ読出できない 
// 	if ( i == mask_max ) {
// 		// 該当データなしなら非マスクアドレスなので OK を返す
// 		ret = 1;
// 	}
// // MH810104(E) R.Endo 2021/09/25 車番チケットレス フェーズ2.3 #6012 【個別部隊指摘】60-0201以降（QR/バーコードフォーマットパターン設定）のアドレスのマスクを外しても（非表示→表示）ジャンプ読出できない 
// 
// 	return ret;
// }
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(設定非表示)
/*[]----------------------------------------------------------------------[]*/
/*| 再起動案内を画面表示する                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PutGuideMessage                                         |*/
/*| PARAMETER    : long add : 設定アドレス（６桁）                         |*/
/*	RETURN VALUE :	MOD_EXT	: F5 Key ( End Key )                           |*/
/*					MOD_CHG	: 設定ﾓｰﾄﾞｽｲｯﾁOFF                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
unsigned short	PutGuideMessage( void )
{
	ushort	msg;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[3]);			/* "＜共通パラメーター設定＞　　　" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[21]);		/* "　 精算機の再起動が必要です 　" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[22]);		/* "　　 電源のOFF/ONを行い、 　　" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[23]);		/* "　精算機を再起動してください　" */
	Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:												/* " 終了 " */
			BUZPI();
			return MOD_EXT;
		default:
			break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 更新アドレスが再起動要求項目か判定する                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : IsRebootRequest                                         |*/
/*| PARAMETER    : long add : 設定アドレス（６桁）                         |*/
/*| RETURN VALUE : char ret : 0 = 再起動不要, 1 = 再起動必要               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
char	IsRebootRequest( long add )
{
	char	ret = 0;

	return ret;
}

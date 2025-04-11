//[]----------------------------------------------------------------------[]
///	@brief			WAON-Suica Operation control
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/08<br>
///					Update
///	@file			ope_suica_ctrl.c
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"irq1.h"
#include	"suica_def.h"
#include	"tbl_rkn.h"
#include	"lcd_def.h"
#include	"mdl_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

static short	RecvDspData( uchar* );
static short	RecvSettData( uchar* );
// MH321800(S) T.Nagai ICクレジット対応
//static short	RecvErrData( uchar* );
// MH321800(E) T.Nagai ICクレジット対応
static short	RecvStatusData( uchar* );
// MH321800(S) T.Nagai ICクレジット対応
//static short	RecvTimeData( uchar* );
//static short	RecvFixData( uchar* );
//static short	RecvPointData( uchar* );
// MH321800(E) T.Nagai ICクレジット対応
// MH810103 GG119202(S) SX-20処理修正
static short	RecvErrData( uchar* );
static short	RecvTimeData( uchar* );
static short	RecvFixData( uchar* );
static short	RecvPointData( uchar* );
// MH810103 GG119202(E) SX-20処理修正

static void 	Log_data_edit( struct  suica_log_rec *data_cuf, uchar kind );
static void 	Log_data_edit2( struct  suica_log_rec *data_cuf, uchar kind );
static void 	Log_data_edit3( struct  suica_log_rec *data_cuf, ushort log_size, ushort loopcount );
void 	Suica_Log_regist( unsigned char* , ushort ,uchar);
static void 	time_snd_ope( struct clk_rec *sndbuf );
static uchar	Suica_Read_RcvQue( void );
// MH321800(S) G.So ICクレジット対応
//static void 	miryo_timeout_after_disp( void );
//static void lcd_wmsg_dsp_elec( char type, const uchar *msg1, const uchar *msg2, uchar timer, uchar ann, ushort color, ushort blink );
//static void		Suica_Status_Chg( void );
// MH321800(E) G.So ICクレジット対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern void lcdbm_notice_dsp( ePOP_DISP_KIND kind, uchar DispStatus );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
#if (4 == AUTO_PAYMENT_PROGRAM)								// 試験用に通信ログ取得方式を変更する
void 	Suica_Log_regist_for_debug( unsigned char* , ushort ,uchar);
#endif
static void		No_Responce_Timeout( void );

typedef short (*CMD_FUNC)(uchar*);

typedef struct{
	unsigned char Command;
	CMD_FUNC Func;
}RCV_CMD_FUNC;

RCV_CMD_FUNC CommandTbl[] = {
//	{ 受信ｺﾏﾝﾄﾞﾋﾞｯﾄ,処理関数},			/* 処理名 */
	{ 0x02,			RecvSettData},		/* 決済結果データ */
	{ 0x08,			RecvStatusData},	/* 状態データ */
	{ 0x01,			RecvDspData},		/* 表示依頼データ */
// MH321800(S) T.Nagai ICクレジット対応
//	{ 0x04,			RecvErrData},		/* 異常データ */
//	{ 0x16,			RecvTimeData},		/* 時刻同期データ */
//	{ 0x32,			RecvFixData},		/* 固定データ */
//	{ 0x64,			RecvPointData},		/* 印字依頼データ(ポイントデータ) */
// MH321800(E) T.Nagai ICクレジット対応
// MH810103 GG119202(S) SX-20処理修正
	{ 0x04,			RecvErrData},		/* 異常データ */
	{ 0x16,			RecvTimeData},		/* 時刻同期データ */
	{ 0x32,			RecvFixData},		/* 固定データ */
	{ 0x64,			RecvPointData},		/* 印字依頼データ(ポイントデータ) */
// MH810103 GG119202(E) SX-20処理修正
};

// MH321800(S) G.So ICクレジット対応
//static uchar	suica_dsp_buff[30];						// 画面表示用表示文字列格納領域
//static uchar	suica_Log_wbuff[S_BUF_MAXSIZE];			// 通信ログ用一時編集領域
//static uchar	wrcvbuf[S_BUF_MAXSIZE];					// データ受信用の一時編集領域
//static uchar	wpsnddata[S_BUF_MAXSIZE];
//
//#define WAR_MONEY_CHECK		(!Suica_Rec.Data.BIT.MONEY_IN && !ryo_buf.nyukin )
//
//#define	OPE_SUICA_MIRYO_TIME	prm_get(COM_PRM, S_SCA, 13, 3, 1)		// 255
//
//static	uchar	err_wk[31];
uchar	suica_dsp_buff[30];						// 画面表示用表示文字列格納領域
uchar	suica_Log_wbuff[S_BUF_MAXSIZE];			// 通信ログ用一時編集領域
uchar	wrcvbuf[S_BUF_MAXSIZE+1];				// データ受信用の一時編集領域
uchar	wpsnddata[S_BUF_MAXSIZE];
uchar	err_wk[40];
uchar	moni_wk[10];
// MH321800(E) G.So ICクレジット対応
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善
//// MH810103 GG119202(S) みなし決済扱い時の動作
//const uchar msg_idx[4] = {
//	49, 50,					// 1ページ目
//	51, 52,					// 2ページ目
//};
//// MH810103 GG119202(E) みなし決済扱い時の動作
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善

//[]----------------------------------------------------------------------[]
///	@brief			Suica Snd Data que Set & read
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/09/29<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Suica_Snd_regist( uchar *kind, uchar *snddata )
{
	uchar	count_status,Read_pt;
	uchar	wpkind;

	wpkind=*kind;										// auto変数に種別を保持 
	memcpy( &wpsnddata,snddata,(size_t)S_BUF_MAXSIZE);	// auto変数にﾊﾞｯﾌｧﾃﾞｰﾀを保持

	if( *kind == 0){
		if( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind ){
			memcpy( snddata, Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].Snd_Buf, (size_t)S_BUF_MAXSIZE );
			*kind = Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind;
			return 1;
		}
	}else{

		count_status = 1;										// ﾊﾞｯﾌｧｽﾃｰﾀｽｾｯﾄ
		for( Read_pt=0;Read_pt<5;Read_pt++ ){					// 未送信ﾃﾞｰﾀの検索
			if( !Suica_Snd_Buf.Suica_Snd_q[Read_pt].snd_kind ){	// 未送信ﾃﾞｰﾀがあれば
				count_status = 0;								// ﾊﾞｯﾌｧｽﾃｰﾀｽを更新
				break;
			}
		}
		if( count_status ){										// ﾊﾞｯﾌｧがﾌﾙの場合
			for( Read_pt=0;Read_pt<BUF_MAX_DELAY_COUNT;Read_pt++ ){		// 未送信ﾃﾞｰﾀの検索
				xPause( BUF_MAX_DELAY_TIME );					// ﾊﾞｯﾌｧMAX時の送信完了待ち
				if( !Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind ){	// 未送信ﾃﾞｰﾀがあれば送信ﾃﾞｰﾀの登録実行
					break;
				}
			}			
		}

		if( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind != 0 ){	// ﾃﾞｰﾀがすでに格納済みの場合
			if( Suica_Snd_Buf.read_wpt > 3 )						// ﾘｰﾄﾞﾎﾟｲﾝﾀがMAX以上？
				Suica_Snd_Buf.read_wpt = 0;							// ｷｭｰの先頭を指定 
			else
				Suica_Snd_Buf.read_wpt++;							// ﾘｰﾄﾞﾎﾟｲﾝﾀをｶｳﾝﾄUP			
		}

		Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind = wpkind;
		memcpy( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].Snd_Buf, wpsnddata,(size_t)S_BUF_MAXSIZE );

		if( Suica_Snd_Buf.write_wpt > 3 )
			Suica_Snd_Buf.write_wpt = 0;
		else
			Suica_Snd_Buf.write_wpt++;
	}

	return 0;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica Snd Data Set
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Suica_Data_Snd( uchar kind, void *snddata )
{
	long tmp = 0;
    ulong paydata = 0;

	if( prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1) 					/* suica使用？ */
		return;

	if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR ){	// 受信した決済額と受信した商品選択データとで差異があった
		return;											// 以降すべての電文送信せずに抜ける
	}

	if( OPECTL.Ope_mod == 13 )										/* ﾌｪｰｽﾞが修正精算だった場合電文送信しない */
		return;														
	memset( suica_work_buf,0,sizeof(suica_work_buf) );
	Suica_Rec.snd_kind = kind;										/* 送信種別のｾｯﾄ */

	switch( kind ){
		case S_CNTL_DATA:											/* 制御ﾃﾞｰﾀ作成 */
			memcpy(&suica_work_buf,snddata,sizeof(suica_work_buf[0]));
			if( suica_work_buf[0] == 0x80 && Suica_Rec.Data.BIT.SEND_CTRL80 ){		// 送信対象が取引終了(0x80)で送信可能状態でない場合
				return;																// 送信しないで抜ける
			}
Suica_Data_Snd_10:
			if( (suica_work_buf[0] & 0x01 ) == 0 ){
				Suica_Rec.Data.BIT.OPE_CTRL = 0;
				if( suica_work_buf[0] == 0 ){
					tmp = (unsigned char)prm_get(COM_PRM, S_SCA, 16, 2, 3);
					LagTim500ms( LAG500_SUICA_NONE_TIMER, (short)((tmp * 2) + 1), snd_ctrl_timeout3 );			// 受付不可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始(5s)
					LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );	/* 受付可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
				}
			}else if( (suica_work_buf[0] & 0x01 ) == 1 ){
				if( OPECTL.op_faz == 8 || OPECTL.InquiryFlg || 						// 電子媒体停止中かｸﾚｼﾞｯﾄHOST通信中か
					OPECTL.ChkPassSyu != 0 || Suica_Rec.Data.BIT.MIRYO_TIMEOUT )			// ｱﾝﾁﾊﾟｽﾁｪｯｸ中かSuica未了タイムアウト
					return;
				Suica_Rec.Data.BIT.OPE_CTRL = 1;
				tmp = (unsigned char)prm_get(COM_PRM, S_SCA, 15, 2, 3);
				LagTim500ms( LAG500_SUICA_NO_RESPONSE_TIMER, (short)(tmp*2+1), snd_ctrl_timeout2 );	// 受付可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始(500ms)
				LagCan500ms( LAG500_SUICA_NONE_TIMER );								// 受付不可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始(5s)
			}
			Ope_Suica_Status = 1;
			break;
		case S_SELECT_DATA:											/* 商品選択ﾃﾞｰﾀ作成 */
			// Suica停止条件の場合は、商品選択データを送信せずにSuica停止処理を実行する
			if( Suica_Rec.Data.BIT.MIRYO_TIMEOUT || 				// 未了タイムアウト発生後は商品選択データを送信しない
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				Edy_Rec.edy_status.BIT.CTRL_MIRYO || 				// Edyの引去り未了中
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				OPECTL.InquiryFlg ||												// ｸﾚｼﾞｯﾄのHOST問い合わせ中
				OPECTL.ChkPassSyu != 0 ||							// 定期問い合わせ中
				OPECTL.op_faz == 8 ){								// 電子媒体停止処理中
				Suica_Rec.snd_kind = S_CNTL_DATA;
				memset( suica_work_buf,0,sizeof(suica_work_buf) );
				goto Suica_Data_Snd_10;
			}
			else if( !Suica_Rec.Data.BIT.ADJUSTOR_START ){			// 商品選択データ送信不可状態
				return;												// 送信せずに抜ける
			}
			else if( Suica_Rec.Data.BIT.PRI_NG ) { 						// ジャーナルプリンタ使用不可
				return;
			}
// MH321800(S) Y.Tanizaki ICクレジット対応(Suica決済異常発生(E6961)後は決済不可とする)
			else if( Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR ) {	// Suica決済異常発生?
				return;													// Suica決済異常発生時は送信せずに抜ける
			}
// MH321800(E) Y.Tanizaki ICクレジット対応(Suica決済異常発生(E6961)後は決済不可とする)
			tmp = 10000;
			suica_work_buf[0] = 0x01;
			memcpy( &paydata,snddata,sizeof(snddata));
			suica_work_buf[6] = binbcd( (unsigned char)(paydata/tmp) );
			paydata %= tmp;
			tmp/=100;
			suica_work_buf[5] = binbcd( (unsigned char)(paydata/tmp) );
			paydata %= tmp;
			suica_work_buf[4] = binbcd( (unsigned char)paydata );
			Product_Select_Data = *((long*)snddata);
			break;
		case S_INDIVIDUAL_DATA:										/* 個別ﾃﾞｰﾀ作成 */
			suica_work_buf[0] = 0x00;
			suica_work_buf[1] = 0x06;
			suica_work_buf[2] = 0x00;
			if( !CPrmSS[S_SCA][10] )
				time_set_snd(&suica_work_buf[3],snddata);
			suica_work_buf[11] = 0xFE;
			suica_work_buf[22] = (uchar)OPE_SUICA_MIRYO_TIME;

			break;
		case S_PAY_DATA:											/* 精算ﾃﾞｰﾀ作成 */
			if( !CPrmSS[S_SCA][10] )
				time_set_snd(suica_work_buf,snddata);
			break;
		default:
			break;
	}

	Suica_Snd_regist( &Suica_Rec.snd_kind, suica_work_buf );
	CNMTSK_START = 1;												/* タスク起動 */	
}

//[]----------------------------------------------------------------------[]
///	@brief			受信ﾃﾞｰﾀをｾｰﾌﾞし、ﾒｯｾｰｼﾞｷｭｰを登録する
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/02/19<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void suica_save( unsigned char *buf, short siz )
{
	unsigned short 	work;
	unsigned char	*pt;
	unsigned char	status_timer;
// MH321800(S) G.So ICクレジット対応
//	unsigned char	moni_wk[10];
// MH321800(E) G.So ICクレジット対応
	
	pt = buf + 1;	/* ﾃﾞｰﾀ部の先頭ｱﾄﾞﾚｽ(種別の次) */
	memset( wrcvbuf,0,sizeof( wrcvbuf ));
	memcpy( wrcvbuf, pt, (size_t)siz );
	Suica_Rec.suica_err_event.BIT.COMFAIL = 0;


	switch( *buf ){														/* 受信ﾃﾞｰﾀ判定 */
		case S_DSP_DATA:	/* 表示依頼データ */
			pay_dsp = settlement_amount( wrcvbuf );						/* 受信ﾃﾞｰﾀ変換 */

			if( Suica_Rec.Data.BIT.INITIALIZE )							/* Suica初期化終了？ */
				Suica_Rec.suica_rcv_event.BIT.DSP_DATA = 1;				/* 表示依頼ﾃﾞｰﾀ受信ﾌﾗｸﾞON */

			break;
		case S_SETTLEMENT_DATA:	/* 決済結果データ */
			if( wrcvbuf[0] != 0 ){
				if( wrcvbuf[0] != 0x01 && wrcvbuf[0] != 0x03 ){			/* 受信ﾃﾞｰﾀ種別が0x01・0x03以外？ */
					err_chk( ERRMDL_SUICA, ERR_SUICA_PAY_NG, 2, 0, 0 ); /* ｴﾗｰﾛｸﾞ登録（登録）*/
					break;
				}	
				if(wrcvbuf[0] == 0x01){											// 正常データ
					memcpy( &moni_wk[0],&wrcvbuf[12],(size_t)2);					// ｶｰﾄﾞ番号取得(上2、下4桁)
					memcpy( &moni_wk[2],&wrcvbuf[24],(size_t)4);
					work = (ushort)settlement_amount(&wrcvbuf[6]);				// 決済前残額
					memcpy(&moni_wk[6],(uchar*)(&work),sizeof(short));
					work = (ushort)settlement_amount(&wrcvbuf[9]);				// 決済後残額
					memcpy(&moni_wk[8],(uchar*)(&work),sizeof(short));
					wmonlg( OPMON_SUICA_SETTLEMENT, moni_wk, 0 );				// モニタ登録
				}

				if( Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1 &&		 /* すでに決済結果受信済みで */
				    Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0 ){   /* まだOpeが処理を行っていない場合 or */
				    Settlement_rcv_faze_err( wrcvbuf, 2 );						 /* エラー登録 */
					break;														 /* 以後の処理をしないので抜ける */
				}else if( Suica_Rec.Data.BIT.PAY_CTRL == 1 ){					 /* すでに１度、電子決済を行っている場合 */
				    Settlement_rcv_faze_err( wrcvbuf, 3 );						 /* エラー登録 */
					break;														 /* 以後の処理をしないので抜ける */
				}

				/* 受信ﾃﾞｰﾀｾｯﾄ */
				memset( &Settlement_Res,0,sizeof( Settlement_Res ));
				memcpy( &Settlement_Res.Result,&wrcvbuf[0],(size_t)3);
				Settlement_Res.settlement_data = settlement_amount(&wrcvbuf[3]);
				Settlement_Res.settlement_data_before = settlement_amount(&wrcvbuf[6]);
				Settlement_Res.settlement_data_after = settlement_amount(&wrcvbuf[9]);
				memcpy( &Settlement_Res.Suica_ID,&wrcvbuf[12],sizeof(Settlement_Res.Suica_ID));

				#if (3 == AUTO_PAYMENT_PROGRAM)										/* 試験用に決済額をごまかす場合（ﾃｽﾄ用）*/
					if( CPrmSS[S_SYS][9] ){
						Settlement_Res.settlement_data = CPrmSS[S_SYS][9];
						if(( Settlement_Res.settlement_data_after - CPrmSS[S_SYS][9]) >= 0){
							Settlement_Res.settlement_data_after = Settlement_Res.settlement_data_before - CPrmSS[S_SYS][9];
						}else{
							Settlement_Res.settlement_data_after = 0;
						}
					}
				#endif
				if( Settlement_Res.Result == 1 && Product_Select_Data != Settlement_Res.settlement_data ){
					Suica_Ctrl( S_CNTL_DATA, 0x80 );								// 制御ﾃﾞｰﾀ（取引終了）を送信する
					Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR = 1;					// 受信した決済額と送信した商品選択データに差異がある
					memset( err_wk, 0, sizeof( err_wk ));
					intoasl( err_wk, (ulong)Product_Select_Data, 5 );				// 決済額をセット
					err_wk[5] = '-';												// 区切り文字セット
					err_wk[6] = '>';												// 区切り文字セット
					err_wk[12] = ':';												// 区切り文字セット
					intoasl( &err_wk[7], (ulong)Settlement_Res.settlement_data, 5 );// 決済額をセット
					memcpy( &err_wk[13], Settlement_Res.Suica_ID, sizeof( Settlement_Res.Suica_ID ) );							// 上位４桁をマスクする
					err_chk2( ERRMDL_SUICA, ERR_SUICA_PAY_ERR, 1, 1, 0, err_wk );/* ｴﾗｰﾛｸﾞ登録（登録）*/
					Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;							// 未了タイムアウトが発生したことにする
					Suica_Rec.Data.BIT.ADJUSTOR_START = 0;							// 商品選択データ送信可否フラグクリア
					dsp_change = 0;													// 未了表示ﾌﾗｸﾞを初期化
					queset( OPETCBNO, SUICA_PAY_DATA_ERR, 0, NULL );				// 決済異常を通知
					return;															// 決済処理はしない（ただ取りとする）
				}

				if( Suica_Rec.Data.BIT.INITIALIZE )						/* Suica初期化終了？ */
					Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;	/* 決済結果ﾃﾞｰﾀ受信ﾌﾗｸﾞON */

				if( Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1 && 
					Settlement_Res.Result == 0x01){						/* 決済結果ﾃﾞｰﾀ受信ﾌﾗｸﾞON? */
					// このﾌﾗｸﾞをｸﾘｱするﾀｲﾐﾝｸﾞは精算ﾛｸﾞ及び精算中止ﾛｸﾞに決済データを登録するか、
					// 次精算時の精算を開始した場合とする。
					Suica_Rec.Data.BIT.LOG_DATA_SET = 1;				/* 決済結果ﾃﾞｰﾀ受信済み */

					// このフラグは精算を開始する前及び決済が行われた場合にクリアし、
					// セットするタイミングは精算開始後、駐車料金が発生した場合とする
					Suica_Rec.Data.BIT.ADJUSTOR_START = 0;						/* 商品選択データ送信可否フラグクリア */
				}

#if (3 == AUTO_PAYMENT_PROGRAM)											// 試験用に利用カード種別をごまかす場合（ﾃｽﾄ用）
				work = (short)prm_get(COM_PRM, S_SYS, 6, 1, 1);			// 01-0006⑥にカード種別をセットする

				/** 01-0006⑥=7時は、ファンクションキーを押しながらSuicaタッチでカード種を切り替える **/
				if( work == 7 ){
					if( SHORT_KEY1 == 1 ){								// F1キー押下中
						work = 1;										// Suicaに読み替え
					}
					else if( SHORT_KEY2 == 1 ){							// F2キー押下中
						work = 2;										// PASMOに読み替え
					}
					else if( SHORT_KEY3 == 1 ){							// F3キー押下中
						work = 4;										// ICOCAに読み替え
					}
					else if( SHORT_KEY4 == 1 ){							// F4キー押下中
						work = 9;										// AMANOに読み替え
					}
				}

				//   カード番号の先頭2文字を書き換える。
				//   1=Suica("JE")、2=PASMO("PB")、4=ICOCA("JW")、9=その他("AM")
				switch( work ){
				case	1:												// Suicaに読み替え
					memcpy( &Settlement_Res.Suica_ID, "JE", 2 );
					break;
				case	2:												// PASMOに読み替え
					memcpy( &Settlement_Res.Suica_ID, "PB", 2 );
					break;
				case	4:												// ICOCAに読み替え
					memcpy( &Settlement_Res.Suica_ID, "JW", 2 );
					break;
				case	9:												// AMANOに読み替え
					memcpy( &Settlement_Res.Suica_ID, "AM", 2 );
					break;
				}
#endif

			}
			#if (3 == AUTO_PAYMENT_PROGRAM)											// 試験用に受信データをごまかす場合（ﾃｽﾄ用）
			else{
						if( SHORT_KEY2 == 1 ){							// F2キー押下中
							STATUS_DATA.status_data = 1;							// 受付不可を受信したかの用に振舞う
							goto DEBUG_SUICA_SAVE;						// 状態ﾃﾞｰﾀ受信処理へ移動
						}
			}
			#endif
			
			break;
		case S_ERR_DATA: /* 異常データ */
			err_suica_chk( wrcvbuf ,&err_data ,ERR_SUICA_RECEIVE );	/* ｴﾗｰﾁｪｯｸ */

			err_data = wrcvbuf[0];
			work = (short)prm_get(COM_PRM, S_SCA, 14, 2, 3);			/* SX-10状態監視ﾀｲﾏ値取得 */
			if( err_data ){											/* ｴﾗｰ発生？ */
				Suica_Rec.suica_err_event.BIT.ERR_RECEIVE = 1;			/* 異常ﾃﾞｰﾀ受信ﾌﾗｸﾞON */

				if( work )												/* SX-10状態監視ﾀｲﾏ値が正常値？ */
					LagTim500ms( LAG500_SUICA_STATUS_TIMER, (short)(120*work), snd_ctrl_timeout );	// SX-10状態監視ﾀｲﾏ開始
			}else{														/* ｴﾗｰ解除 */
				Suica_Rec.suica_err_event.BIT.ERR_RECEIVE = 0;			/* 異常ﾃﾞｰﾀ受信ﾌﾗｸﾞOFF */
				LagCan500ms( LAG500_SUICA_STATUS_TIMER );				/* SX-10状態監視ﾀｲﾏﾘｾｯﾄ */
			}
			break;
		case S_STATUS_DATA: /* 状態データ */
			#if (3 == AUTO_PAYMENT_PROGRAM)									/* 試験用に未了タイマーをごまかす（ﾃｽﾄ用）*/
				if( CPrmSS[S_SYS][7] ){											// 未了発生後の設定無視動作あり？
					if( Suica_Rec.Data.BIT.CTRL_MIRYO ){						// 未了発生中
						CPrmSS[S_SYS][7]--;										// 一回制御データ無視回数をデクリメント
						break;													// 受信データを処理しない
					}
				}
			#endif
			STATUS_DATA.status_data = wrcvbuf[0];									/* 受信ﾃﾞｰﾀｽﾃｰﾀｽｾｯﾄ */

			#if (3 == AUTO_PAYMENT_PROGRAM)											// 試験用に受信データをごまかす場合（ﾃｽﾄ用）
				if( SHORT_KEY1 == 1 ){								// F1キー押下中
					if( STATUS_DATA.status_data == 0x03 ){						// 取引キャンセル＆受付不可
						STATUS_DATA.status_data = 0x02;							// 取引キャンセル受付に変更
					}
				}else if( SHORT_KEY2 == 1 ){						// F2キー押下中
					if( Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA ){ // 決済結果を受信済み
						break;										// 何も処理させないで抜ける
					}
				}
DEBUG_SUICA_SAVE:
			#endif

			Suica_Rec.Data.BIT.CTRL = STATUS_DATA.StatusInfo.ReceptStatus ^ 0x01;	/* 受信ﾃﾞｰﾀの1Byte目を参照し、ﾘｰﾀﾞｰ状態として取得する */
			if( STATUS_DATA.StatusInfo.MiryoStatus && 				/* 未了Bitをチェック */
				!Suica_Rec.Data.BIT.CTRL_MIRYO ){					/* まだ未了が発生していない */
				Suica_Rec.Data.BIT.CTRL_MIRYO = 1;					/* Suica未了ｽﾃｰﾀｽをｾｯﾄ */
				read_sht_cls();										/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ */
				cn_stat( 2, 2 );									/* 入金不可 */
				work = (ushort)OPE_SUICA_MIRYO_TIME;							/* 決済ﾘﾄﾗｲﾀｲﾏｰ値取得 */

			#if (3 == AUTO_PAYMENT_PROGRAM)									/* 試験用に未了タイマーをごまかす（ﾃｽﾄ用）*/
				if( CPrmSS[S_SYS][6] ){
					work = (ushort)CPrmSS[S_SYS][6];
				}
			#endif
				LagTim500ms( LAG500_SUICA_MIRYO_RESET_TIMER, (short)(2*(work+5)), miryo_timeout );	// 決済ﾘﾄﾗｲ状態監視ﾀｲﾏ開始
				OPECTL.InquiryFlg = 1;
				err_chk( ERRMDL_SUICA, ERR_SUICA_MIRYO_START, 2, 0, 0 ); 	/* ｴﾗｰﾛｸﾞ登録（発生＆解除）*/
			}else{
				switch( STATUS_DATA.status_data & 0x03 ){					/* 下2Bitの状態をチェック */
					case 0:		/* 受付可 */
						if( !Ope_Suica_Status )					/* 精算機側からの送信要求なし */
							Suica_Rec.Data.BIT.OPE_CTRL = 1;	/* 最後に精算機側からの送信要求は受付可とする */

						if( Suica_Rec.Data.BIT.OPE_CTRL ){		/* 最後に精算機側からの送信要求は受付可？ */
							LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );		/* 受付可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
							err_chk( ERRMDL_SUICA, ERR_SUICA_OPEN, 0, 0, 0 ); 	/* ｴﾗｰﾛｸﾞ登録（解除）*/
							Suica_Rec.suica_err_event.BIT.OPEN = 0;				/* 開局異常ﾌﾗｸﾞを解除 */
							Status_Retry_Count_OK = 0;							/* ﾘﾄﾗｲｶｳﾝﾄ数を初期化 */
						}
						else{									/* 最後に精算機側からの送信要求は受付不可？ */
							status_timer = (unsigned char)prm_get(COM_PRM, S_SCA, 16, 2, 3);
							LagTim500ms( LAG500_SUICA_NONE_TIMER, (short)((status_timer * 2) + 1), snd_ctrl_timeout3 );			// 受付不可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始(5s)
						}
						break;
					case 0x01:	/* 受付不可 */
						if( !Ope_Suica_Status )					/* 精算機側からの送信要求なし */
							Suica_Rec.Data.BIT.OPE_CTRL = 0;    /* 最後に精算機側からの送信要求は受付不可とする */
																
						if( !Suica_Rec.Data.BIT.INITIALIZE )				/* Suica初期化終了？ */
							break;
						if( !Suica_Rec.Data.BIT.OPE_CTRL ){		/* 最後に精算機側からの送信要求は受付不可？ */
							LagCan500ms( LAG500_SUICA_NONE_TIMER );				/* 受付不可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
							if( Suica_Rec.Data.BIT.CTRL_MIRYO &&			/* 未了状態から受付不可を受信？かつ */ 
								Suica_Rec.Data.BIT.INITIALIZE ){			/* 初期化が終了している場合 */ 
								LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );	/* 未了状態監視ﾀｲﾏﾘｾｯﾄ */
								Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* 未了状態解除 */
							}
							Status_Retry_Count_NG = 0;							/* ﾘﾄﾗｲｶｳﾝﾄ数を初期化 */
						}else{
							status_timer = (unsigned char)prm_get(COM_PRM, S_SCA, 15, 2, 3);
							LagTim500ms( LAG500_SUICA_NO_RESPONSE_TIMER, (short)(status_timer*2+1), snd_ctrl_timeout2 );	// 受付可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始(500ms)
						}
						break;
					case 0x02:	/* 取引キャンセル受付 */
					case 0x03:	/* 受付不可＆取引キャンセル受付 */
						LagCan500ms( LAG500_SUICA_NONE_TIMER );				/* 受付不可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
						if( Suica_Rec.Data.BIT.CTRL_MIRYO ){				/* 未了状態？ */
							Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* 未了状態解除 */
							LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );	/* 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ */
							if( !SUICA_CM_BV_RD_STOP && OPECTL.op_faz != 3 ){ /* 未了後の動作は全精算媒体を停止しない場合 かつ精算中止処理中ではない */
								miryo_timeout_after_proc();
							}
							err_chk( ERRMDL_SUICA, ERR_SUICA_MIRYO, 2, 0, 0 ); 	/* ｴﾗｰﾛｸﾞ登録（登録）*/
							OPECTL.InquiryFlg = 0;		
						}
						Status_Retry_Count_NG = 0;							/* ﾘﾄﾗｲｶｳﾝﾄ数を初期化 */
						break;
					default:
						break;
				}
			}

			if( Suica_Rec.Data.BIT.INITIALIZE )							/* Suica初期化終了？ */
				Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;			/* 制御ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
				
			Ope_Suica_Status = 0;	
			break;
		case S_TIME_DATA: /* 時間同期データ */
			if( !Suica_Rec.Data.BIT.INITIALIZE ){						/* Suica初期化終了？ */
				break;

			}
			if( CPrmSS[S_SCA][10] ){ 									/* SX-10から時計データでの整時する？ */
				time_get_rcv(&time_data,wrcvbuf);						/* 受信ﾃﾞｰﾀの変換処理 */
				timset( &time_data );									/* 時刻ﾃﾞｰﾀ変換処理実施 */
				time_snd_ope( &time_data );								/* Opeに時刻ﾃﾞｰﾀｾｯﾄ要求 */
			}

			break;
	    case S_FIX_DATA: /* 固定データ */
			memcpy( fix_data,wrcvbuf,sizeof(fix_data));					/* 受信ﾃﾞｰﾀを画面表示用に保持 */
			break;

	    default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica Set time
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void time_set_snd( unsigned char *sndbuf, void *snddata )
{
	unsigned char year_data;	

	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->seco) );		// 秒
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->minu) );		// 分
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->hour) );		// 時
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->week) );		// 週
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->date) );		// 日
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->mont) );		// 月

	year_data = binbcd( (unsigned char)(((struct clk_rec*)snddata)->year%100) );	// 年・下位２桁
	memcpy( sndbuf,&year_data,sizeof(year_data));
	year_data = binbcd( (unsigned char)(((struct clk_rec*)snddata)->year/100) );	// 年・上位２桁
	sndbuf++;
	memcpy( sndbuf,&year_data,sizeof(year_data));
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica Rcv time
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void time_get_rcv(  struct clk_rec *sndbuf, void *snddata)
{
	unsigned char *rcv;
	ushort	year_date;
	rcv = snddata;

	memset(sndbuf,0,sizeof(sndbuf));

	sndbuf->seco = bcdbin( *rcv );		// 秒
	sndbuf->minu = bcdbin( *(++rcv) );	// 分
	sndbuf->hour = bcdbin( *(++rcv) );	// 時
	sndbuf->week = bcdbin( *(++rcv) );	// 週
	sndbuf->date = bcdbin( *(++rcv) );	// 日
	sndbuf->mont = bcdbin( *(++rcv) );	// 月
	sndbuf->year = bcdbin( *(++rcv) );	// 年・上位２桁
	year_date 	= bcdbin( *(++rcv) );

	sndbuf->year += year_date*100; 		// 年・下位２桁	
}

//[]----------------------------------------------------------------------[]
///	@brief			time_snd_ope
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static void time_snd_ope( struct clk_rec *sndbuf )
{
	unsigned char snd_time[11];
	union {
		unsigned long	ul;
		unsigned char	uc[4];
	} u_LifeTime;
	memset(snd_time,0,sizeof(snd_time));
	snd_time[0] = binbcd( (unsigned char)( sndbuf->year / 100 ));	/* 年(上２桁) */
	snd_time[1] = binbcd( (unsigned char)( sndbuf->year % 100 ));	/* 年(下２桁) */
	snd_time[2] = binbcd( sndbuf->mont );							/* 月 */
	snd_time[3] = binbcd( sndbuf->date );							/* 日 */
	snd_time[4] = binbcd( sndbuf->hour );							/* 時間 */
	snd_time[5] = binbcd( sndbuf->minu );							/* 分 */
	u_LifeTime.ul = LifeTim2msGet();
	snd_time[7] = u_LifeTime.uc[0];
	snd_time[8] = u_LifeTime.uc[1];
	snd_time[9] = u_LifeTime.uc[2];
	snd_time[10] = u_LifeTime.uc[3];
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Rec.edy_status.BIT.TIME_SYNC_REQ = 1;						// 日時同期データ送信要求
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	queset( OPETCBNO, CLOCK_CHG, 11, snd_time );						/* Send message to opration task */
}
//[]----------------------------------------------------------------------[]
///	@brief			表示依頼データ・決済結果データ変換
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 07/02/29<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
long settlement_amount( unsigned char *buf )
{
	unsigned char count;
	long	res=0;
	long	pay=0;
	long	ofset=1;

	for( ;ofset<100000; ){		/* 金額を数値に変換する */
		count = bcdbin( (unsigned char)(*buf & 0x0f) );
		pay = count;
		res += pay*ofset;
		count = bcdbin( (unsigned char)(*buf >> 4) );
		pay = count;
		res += pay*(ofset*10);
		ofset *= 100;
		buf++;
	}

	return (res);	
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_ctrl_timeout2
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void snd_ctrl_timeout2( void )
{
	uchar	Retry_count = 0;
	LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );			/* 受付可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */

	// 最後に送信したデータが受付不可の場合はタイムアウト処理をやらない
	if( Suica_Rec.Data.BIT.OPE_CTRL == 0 ){
		return;
	}

	Retry_count = (uchar)prm_get(COM_PRM, S_SCA, 15, 2, 1);	/* 設定からﾘﾄﾗｲｶｳﾝﾄ数を取得 */
	if( Status_Retry_Count_OK < Retry_count ){				/* 現在のｶｳﾝﾄ数が設定値以下？ */
		if( !Suica_Rec.Data.BIT.CTRL ){
			Suica_Ctrl( S_CNTL_DATA, 0x80 );								/* 制御ﾃﾞｰﾀ（取引終了）を送信 */
			Suica_Ctrl( S_CNTL_DATA, 0x01 );								/* 制御ﾃﾞｰﾀ（受付可）を送信 */
			Status_Retry_Count_OK++;						/* ﾘﾄﾗｲｶｳﾝﾄをｲﾝｸﾘﾒﾝﾄ */
			return;
		}
	}			

	Status_Retry_Count_OK = 0;								/* ﾘﾄﾗｲｶｳﾝﾄをｸﾘｱ */
	if( Suica_Rec.Data.BIT.CTRL ){							/* Suicaの状態が受付可の場合 */
		Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;		/* 制御ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
		STATUS_DATA.StatusInfo.ReceptStatus = 0;			/* 制御ﾃﾞｰﾀ内容に受付可ｾｯﾄ */
		RecvStatusData( &OPECTL.Ope_mod );					/* 制御データ受信処理実施 */
	}
	else{
		Suica_Rec.suica_err_event.BIT.OPEN = 1;				/* 開局異常ﾌﾗｸﾞをｾｯﾄ */
		err_chk( ERRMDL_SUICA, ERR_SUICA_OPEN, 1, 0, 0 );	/* ｴﾗｰﾛｸﾞ登録（登録）*/
	}	
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_ctrl_timeout3
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/08/10<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void snd_ctrl_timeout3( void )
{
	uchar	Retry_count = 0;

	LagCan500ms( LAG500_SUICA_NONE_TIMER );					/* 受付不可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
	if(	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER == 1 && STATUS_DATA.StatusInfo.Running ){		// Suicaから応答がなく、未了タイムアウトした場合
		// 精算機のタイマーにて未了タイムアウトになった後の受付不可の応答が実行中の場合は永久に受付不可を繰り返す。
		// ただし、受付不可の応答が実行中BITが立たないデータを受信すれば、通常のリトライ処理に移行する。
		STATUS_DATA.StatusInfo.Running = 0;													// 不可の応答で実行中を受信し、再度受付不可を送信する場合は実行中ﾌﾗｸﾞを落とす
		Status_Retry_Count_NG = 0;															// 実行中で返ってきた場合にはリトライカウントをクリアする。
		Suica_Ctrl( S_CNTL_DATA, 0 );														/* 制御ﾃﾞｰﾀ（受付不可）を送信 */
		return;
	}

	// 最後に送信したデータが受付可の場合はタイムアウト処理をやらない
	if( Suica_Rec.Data.BIT.OPE_CTRL == 1 ){
		return;
	}

	if( Suica_Rec.Data.BIT.CTRL ){								/* Suicaが受付可状態？ */
		Retry_count = (uchar)prm_get(COM_PRM, S_SCA, 16, 2, 1);	/* 設定からﾘﾄﾗｲｶｳﾝﾄ数を取得 */
		if( Status_Retry_Count_NG < Retry_count ){				/* 現在のｶｳﾝﾄ数が設定値以下？ */
			Suica_Ctrl( S_CNTL_DATA, 0 );									/* 制御ﾃﾞｰﾀ（受付不可）を送信 */
			Status_Retry_Count_NG++;							/* ﾘﾄﾗｲｶｳﾝﾄをｲﾝｸﾘﾒﾝﾄ */
			return;
		}else{
			Suica_Rec.Data.BIT.CTRL = 0;
			No_Responce_Timeout();
		}			
	}else{
		No_Responce_Timeout();
	}
	Status_Retry_Count_NG = 0;									/* ﾘﾄﾗｲｶｳﾝﾄをｸﾘｱ */
	
}

//[]----------------------------------------------------------------------[]
///	@brief			No_Responce_Timeout(未了タイムアウトが無応答時の動作)
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/06/16<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void No_Responce_Timeout( void )
{
	if(	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER && Suica_Rec.Data.BIT.CTRL_MIRYO ){			/* 未了状態 */
		Suica_Rec.Data.BIT.CTRL_MIRYO = 0;					/* 未了状態解除 */
	}
	Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;			/* 制御ﾃﾞｰﾀ受信ﾌﾗｸﾞON */	
	STATUS_DATA.StatusInfo.ReceptStatus = 1;				/* 制御ﾃﾞｰﾀ内容に受付不可ｾｯﾄ */
	if( Suica_Rec.Data.BIT.ADJUSTOR_NOW ){					/* 決済中の受付不可応答が無い場合 */
		STATUS_DATA.StatusInfo.TradeCansel = 1;				/* 取引ｷｬﾝｾﾙﾌﾗｸﾞもON */
	}
	RecvStatusData( &OPECTL.Ope_mod );						/* 制御データ受信処理実施 */
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica_Log_regist
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/26<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Suica_Log_regist( unsigned char* logdata, ushort log_size, uchar kind )
{
	ushort  i,k,wks = 0;

	memset( &time_data, 0, sizeof( time_data ));
	memset( &suica_Log_wbuff, 0, sizeof( suica_Log_wbuff ));									// ﾛｸﾞﾃﾞｰﾀ編集領域の初期化

	if( !SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start ){		// ﾛｸﾞﾃﾞｰﾀが初期化されている場合
		SUICA_LOG_REC.log_time_old = CLK_REC.ndat;					// 最古の日付ﾃﾞｰﾀをﾊﾞｯｸｱｯﾌﾟ
		Log_data_edit( &SUICA_LOG_REC, 1 );							// 日付ﾃﾞｰﾀの編集
	}
	if( CLK_REC.ndat != suica_work_time.ndat && SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start ){	// 日付が更新されている場合
		Log_data_edit( &SUICA_LOG_REC, 1 );														// 日付ﾃﾞｰﾀの編集
	}
	if( CLK_REC.nmin != suica_work_time.nmin || !SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start ){	// 時間が更新されている場合
		Log_data_edit( &SUICA_LOG_REC, 0 );														// 時間ﾃﾞｰﾀの編集
	}

	Log_data_edit2( &SUICA_LOG_REC,kind );														// ﾃﾞｰﾀﾍｯﾀﾞの編集(送信【SD】・受信【RD】)

	if( (SUICA_LOG_REC.log_wpt+(log_size*2)) > SUICA_LOG_MAXSIZE-1){							// 編集ﾃﾞｰﾀのｻｲｽﾞﾁｪｯｸ
		wks = SUICA_LOG_MAXSIZE-SUICA_LOG_REC.log_wpt;											// ﾊﾞｯﾌｧの最後まで格納できるｻｲｽﾞを算出
		
		for( i=0,k=0; i<log_size; i++){															// 通信ﾃﾞｰﾀの編集
			hxtoas ( &suica_Log_wbuff[k],*(logdata+i) );												// ﾍｷｻﾃﾞｰﾀを文字列に変換
			k+=2;																				
		}
		memcpy( &SUICA_LOG_REC.log_Buf[SUICA_LOG_REC.log_wpt], suica_Log_wbuff, (size_t)wks );			// ﾊﾞｯﾌｧの最後に書き込める分の通信ﾃﾞｰﾀを書込む
		SUICA_LOG_REC.log_wpt=7;																// 書込みﾎﾟｲﾝﾀの更新

		Log_data_edit3( &SUICA_LOG_REC, (ushort)(SUICA_LOG_REC.log_wpt+k-wks), SUICA_LOG_REC.log_wpt );	// 日付更新ﾁｪｯｸ

		if( (k-wks) != 0 ){
			memcpy( &SUICA_LOG_REC.log_Buf[7], &suica_Log_wbuff[wks], (size_t)(k-wks) );				// 残りのﾃﾞｰﾀを先頭から書込む
			SUICA_LOG_REC.log_wpt += (k-wks);													// 書込みﾎﾟｲﾝﾀの更新
		}
		SUICA_LOG_REC.Suica_log_event.BIT.write_flag=1;											// 日付更新ﾁｪｯｸﾌﾗｸﾞ更新

	}else{
		if( SUICA_LOG_REC.Suica_log_event.BIT.write_flag ){										// 日付更新ﾁｪｯｸﾌﾗｸﾞON
			Log_data_edit3( &SUICA_LOG_REC, (ushort)(log_size+SUICA_LOG_REC.log_wpt), SUICA_LOG_REC.log_wpt );	// 日付更新ﾁｪｯｸ
		}
		for( i=0; i<log_size; i++){
			hxtoas ( &((uchar)SUICA_LOG_REC.log_Buf[SUICA_LOG_REC.log_wpt]),*(logdata+i) );		// 通信ﾃﾞｰﾀの変換・書込み
			SUICA_LOG_REC.log_wpt+=2;															// 書込みﾎﾟｲﾝﾀの更新
		}
	}

	if( !SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start )									// 初期化ﾌﾗｸﾞが０の場合
		SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start = 1;									// 初期化ﾌﾗｸﾞの更新

	suica_work_time = CLK_REC;																		
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Log_data_edit
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Log_data_edit( struct  suica_log_rec *data_cuf, uchar kind )
{
	ushort wks;
	memset( &suica_Log_wbuff, 0, sizeof( suica_Log_wbuff ));

	if( (data_cuf->log_wpt+7) > SUICA_LOG_MAXSIZE-1 ){									// 書き込みﾃﾞｰﾀｻｲｽﾞﾁｪｯｸ
		if( kind )																		// 日付ﾃﾞｰﾀ？
			sprintf( (char*)suica_Log_wbuff,"[%02d/%02d]",CLK_REC.mont,CLK_REC.date );				// 日付を設定
		else																			// 時刻ﾃﾞｰﾀ？
			sprintf( (char*)suica_Log_wbuff,"[%02d:%02d]",CLK_REC.hour,CLK_REC.minu );				// 時刻を設定

		wks = SUICA_LOG_MAXSIZE-data_cuf->log_wpt;										// 書込みﾃﾞｰﾀｻｲｽﾞを算出
		memcpy( &data_cuf->log_Buf[data_cuf->log_wpt],suica_Log_wbuff,(size_t)wks);			// ﾃﾞｰﾀの書込み
		data_cuf->log_wpt = 7;															// 書込みﾎﾟｲﾝﾀの更新
		Log_data_edit3( data_cuf, (ushort)(7+data_cuf->log_wpt), data_cuf->log_wpt );	// 日付更新ﾁｪｯｸ
		if( (7-wks) != 0){
			memcpy( &data_cuf->log_Buf[data_cuf->log_wpt],&suica_Log_wbuff[wks],(size_t)(7-wks));	// 残りのﾃﾞｰﾀを先頭から書込む
			data_cuf->log_wpt += (7-wks);														// 書込みﾎﾟｲﾝﾀの更新
		}

		data_cuf->Suica_log_event.BIT.write_flag=1;										// 日付更新ﾁｪｯｸﾌﾗｸﾞ更新

	}else{
		if(	data_cuf->Suica_log_event.BIT.write_flag )									// 日付更新ﾁｪｯｸﾌﾗｸﾞON
			Log_data_edit3( data_cuf, (ushort)(7+data_cuf->log_wpt), data_cuf->log_wpt );// 日付更新ﾁｪｯｸ

		if( kind )																		// 日付ﾃﾞｰﾀ？
			sprintf( (char*)suica_Log_wbuff,"[%02d/%02d]",CLK_REC.mont,CLK_REC.date );				// 日付を設定？
		else																			// 時刻ﾃﾞｰﾀ？
			sprintf( (char*)suica_Log_wbuff,"[%02d:%02d]",CLK_REC.hour,CLK_REC.minu );				// 時刻を設定？

		memcpy( &data_cuf->log_Buf[data_cuf->log_wpt], suica_Log_wbuff,7 );					// 編集ﾃﾞｰﾀの設定
		data_cuf->log_wpt += 7;															// 書込みﾎﾟｲﾝﾀの更新
	}
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			Log_data_edit2
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Log_data_edit2( struct  suica_log_rec *data_cuf, uchar kind )
{
	ushort wks;
	memset( &suica_Log_wbuff, 0, sizeof( suica_Log_wbuff ));

	if( (data_cuf->log_wpt+4) > SUICA_LOG_MAXSIZE-1 ){									// 書き込みﾃﾞｰﾀｻｲｽﾞﾁｪｯｸ
		if( kind )																		// 送信ﾃﾞｰﾀ？
		     sprintf( (char*)suica_Log_wbuff,"[SD]" );												// 【SD】ｾｯﾄ
		else																			// 受信ﾃﾞｰﾀ？
		     sprintf( (char*)suica_Log_wbuff,"[RD]" );												// 【RD】ｾｯﾄ

		wks = SUICA_LOG_MAXSIZE-data_cuf->log_wpt;										// 書込みﾃﾞｰﾀｻｲｽﾞを算出
		memcpy( &data_cuf->log_Buf[data_cuf->log_wpt],suica_Log_wbuff,(size_t)wks);			// ﾃﾞｰﾀの書込み
		data_cuf->log_wpt = 7;															// 書込みﾎﾟｲﾝﾀの更新
		Log_data_edit3( data_cuf, (ushort)(4+data_cuf->log_wpt), data_cuf->log_wpt );// 日付更新ﾁｪｯｸ
		if( (4-wks) != 0){
			memcpy( &data_cuf->log_Buf[data_cuf->log_wpt],&suica_Log_wbuff[wks],(size_t)(4-wks));	// 残りのﾃﾞｰﾀを先頭から書込む
			data_cuf->log_wpt += (4-wks);													// 書込みﾎﾟｲﾝﾀの更新
		}
		data_cuf->Suica_log_event.BIT.write_flag=1;										// 日付更新ﾁｪｯｸﾌﾗｸﾞ更新
	}else{			
		if(	data_cuf->Suica_log_event.BIT.write_flag )									// 日付更新ﾁｪｯｸﾌﾗｸﾞON
			Log_data_edit3( data_cuf, (ushort)(4+data_cuf->log_wpt), data_cuf->log_wpt );// 日付更新ﾁｪｯｸ

		if( kind )																		// 送信ﾃﾞｰﾀ？
		     sprintf( (char*)suica_Log_wbuff,"[SD]" );												// 【SD】ｾｯﾄ
		else																			// 受信ﾃﾞｰﾀ？
		     sprintf( (char*)suica_Log_wbuff,"[RD]" );												// 【RD】ｾｯﾄ
	
		memcpy( &data_cuf->log_Buf[data_cuf->log_wpt], suica_Log_wbuff,4 );					// 編集ﾃﾞｰﾀのｾｯﾄ

		data_cuf->log_wpt += 4;															// 書込みﾎﾟｲﾝﾀの更新
	}
	return;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Log_data_edit3
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Log_data_edit3( struct  suica_log_rec *data_cuf, ushort log_size, ushort loopcount )
{
	ushort i,time_year,time_work,siz;
	uchar	time_data[2];

	if( (log_size-data_cuf->log_wpt) < 15 ){			// 書込みｻｲｽﾞが15Byte以下の場合は、日付ﾃﾞｰﾀが中途半端に上書きされ、最古日付の
		if( (siz = data_cuf->log_wpt+15) > SUICA_LOG_MAXSIZE-1 ){	// 更新がされない事態を回避するために検索Byte数を増やす
			siz = SUICA_LOG_MAXSIZE-1;
		}
	}
	else
		siz = log_size;									// 検索ｻｲｽﾞ数をｾｯﾄ
	
	
	for( i=siz;i>loopcount;i-- ){						
		if( data_cuf->log_Buf[i] == 0x5d ){				// 【]】であるか？
			if( data_cuf->log_Buf[i-3] == 0x2f && data_cuf->log_Buf[i-6] == 0x5b){	// 【/】・【[】であるか？ 
				time_year = CLK_REC.year;											// 年を取得
				time_data[0] = (uchar)astoin( &((uchar)data_cuf->log_Buf[i-5]),2 );	// ﾊﾞｯﾌｧから日付を取得(月)
				time_data[1] = (uchar)astoin( &((uchar)data_cuf->log_Buf[i-2]),2 );	// ﾊﾞｯﾌｧから日付を取得(日)

				if( data_cuf->log_time_old < (time_work = dnrmlzm( (short)time_year, (short)time_data[0], (short)time_data[1] ))){ // 最古ﾃﾞｰﾀかどうかﾁｪｯｸ
					sprintf( (char*)suica_Log_wbuff,"[%02d/%02d]",time_data[0],time_data[1] );	// 日付ﾃﾞｰﾀ作成
					memcpy( data_cuf->log_Buf, suica_Log_wbuff, (size_t)7);				// ﾊﾞｯﾌｧに設定
					data_cuf->log_time_old = time_work;								// 最古日付の更新
					break;
				}
			}
		}		
	}
	return;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Log_Count_search
//[]----------------------------------------------------------------------[]
///	@return			data_count  ログ上の送受信件数
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short Log_Count_search( uchar	search_kind )
{
	ushort i;// ログ容量増加により変更
	short data_count = 0;

	struct	suica_log_rec *wlogbuf;
	
	if( search_kind ){
		wlogbuf = &SUICA_LOG_REC_FOR_ERR;
	}else{
		wlogbuf = &SUICA_LOG_REC;
	}
	
	for( i=0; i<SUICA_LOG_MAXSIZE-4; i++ ){					// 通信ﾛｸﾞの検索
		if( strncmp( &wlogbuf->log_Buf[i], "[RD]", 4 ) == 0 || strncmp( &wlogbuf->log_Buf[i], "[SD]", 4 ) == 0){	// 【RD】・【SD】を検索
			data_count++;									// ｶｳﾝﾄｱｯﾌﾟ
			i+=3;											// 検索ﾎﾟｲﾝﾀの更新
		}
	}
	return (data_count);									// ｶｳﾝﾄ数を戻す	
}

//[]----------------------------------------------------------------------[]
///	@brief			miryo_timeout
//[]----------------------------------------------------------------------[]
///	@return			data_count  ログ上の送受信件数
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/08/10<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void miryo_timeout( void )
{
	LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );		/* 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ */

	OPECTL.InquiryFlg = 0;								// FT4800ではこのﾌﾗｸﾞは未了中という意味合いでも使用するのでここでおとす
	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;				// Suicaから応答がなく、未了タイムアウトした場合
	Suica_Ctrl( S_CNTL_DATA, 0 );						// 制御ﾃﾞｰﾀ（受付不可）を送信
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_Suica_Event
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			:getmassegeにて取得したID 
///					ope_faze	:ｺｰﾙ元のﾌｪｰｽ
///	@return			ret			:精算終了判定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/11/27<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short Ope_Suica_Event( ushort msg, uchar ope_faze )
{
	short	ret = 0;
	uchar	i;

// MH321800(S) G.So ICクレジット対応
	if (isEC_USE()) {
		return Ope_Ec_Event(msg, ope_faze);
	}
// MH321800(E) G.So ICクレジット対応
	for( ;suica_rcv_que.count != 0; ){								// 受信済みのデータを全て処理するまで回す
		if( Suica_Read_RcvQue() == 1 )								// 受信待ち合わせ中の場合は
			continue;												// 後続のデータ受信させる為、解析処理はさせない

		for( i=0; i < TBL_CNT(CommandTbl); i++){					// 受信データテーブル検索実行
			if( CommandTbl[i].Command == (Suica_Rec.suica_rcv_event.BYTE & CommandTbl[i].Command) ){	// 受信したデータが処理可能な電文の場合
				if(( ret = CommandTbl[i].Func(&ope_faze)) != 0 )	// 電文毎の解析処理を実行
					break;										
			}
		}
	}
	
	return ret;	
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_TimeOut_10
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze		:ｺｰﾙ元のﾌｪｰｽﾞ
///	@param[in]		e_pram_set		:電子決済使用設定
///	@return			ret				:精算終了判定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/11/27<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short Ope_TimeOut_10( uchar ope_faze, ushort  e_pram_set )
{
	short	ret=0;

	switch( ope_faze ){
		case	0:
				if( e_pram_set == 1 ){											// 電子ﾏﾈｰ使用可能設定？
					op_mod01_dsp_sub();
					grachr( 6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]] );				// 6行目に画面切替用表示をする
					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );		// 画面切替用ﾀｲﾏｰ起動(Timer10)
				}

				break;
		case	2:
				if( OPECTL.op_faz == 8 )								// ｸﾚｼﾞｯﾄ使用で電子媒体停止中の場合は処理をせずに抜ける
					break;
				if( First_Pay_Dsp() ){ 									// 初回表示（料金表示）
					if( !Ex_portFlag[EXPORT_CHGNEND] ){						   // ～が利用できます表示かつ釣銭不足でない時の場合
						dspCyclicMsgRewrite(4);
						Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
					}
					if( OPECTL.op_faz == 3 || OPECTL.op_faz == 9 ){		// 取消し処理実行中の場合
						if( Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL){	// Suicaが有効かつ最後に送信したのが受付許可の場合
							Suica_Ctrl( S_CNTL_DATA, 0 );												// Suica利用を不可にする
						}
					}
				} else {												// Lagtimer10の使用状態= 1:Suica停止後の受付許可再送信Wait終了
					if( OPECTL.op_faz == 3 ){							// ﾀｲﾏｰ実行中に取り消しﾎﾞﾀﾝ押下された場合はｷｬﾝｾﾙ処理実行
						Suica_Ctrl( S_CNTL_DATA, 0x80 );													// 制御ﾃﾞｰﾀ（取引終了）を送信する
						Op_Cansel_Wait_sub( 0 );											// Suica停止済み処理実行
						break;
					}							
					if( ryo_buf.zankin == 0 ){							// 他媒体による精算完了時は残高不足検出後の受付可を送信しない
						break;
					}

					if( OPECTL.op_faz == 9 )								// 取消し押下後は受付可を送信しない
						break;
					
					if( !Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ){			// Suicaが受け付け不可状態且つ、最後に送信しているのが受付不可要求
						Suica_Ctrl( S_CNTL_DATA, 0x01 );													// Suica利用を可にする
					}
					if(suica_fusiku_flg == 0) {							// Suica残高不足で再タッチ待ち時はｻｲｸﾘｯｸ表示を再開しない
						mode_Lagtim10 = 0;									// Lagtimer10の使用状態 0:利用可能媒体ｻｲｸﾘｯｸ表示再開
						Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
					}
				}

				break;

	}
	return	ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_TimeOut_11
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze		:ｺｰﾙ元のﾌｪｰｽﾞ
///	@param[in]		e_pram_set		:電子決済使用設定
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/11/27<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Ope_TimeOut_11( uchar ope_faze, ushort  e_pram_set )
{
	switch( ope_faze ){
		case	0:
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( Suica_Rec.Data.BIT.CTRL == 1 || Edy_Rec.edy_status.BIT.CTRL == 1 ){	// 残高表示用処理
			if( Suica_Rec.Data.BIT.CTRL == 1 ){	// 残高表示用処理
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				if( edy_dsp.BIT.suica_zangaku_dsp ){						// Suicaの残額表示中
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					Edy_StopAndStart();										// EdyStop＆LED消灯 
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					edy_dsp.BIT.suica_zangaku_dsp = 0;						// Edy残額表示ﾌﾗｸﾞOFF
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				}else if( edy_dsp.BIT.edy_zangaku_dsp ){					// Edyの残額表示中
//					break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				}
				op_mod01_dsp_sub();
				grachr( 6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]] );			// 6行目に画面切替用表示をする
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( DspChangeTime[1] && (!Suica_Rec.suica_err_event.BYTE || // ﾀｲﾏｰ値が正常にｾｯﾄされているかつSX-10関連のｴﾗｰが発生していない場合
//										 ( !EDY_USE_ERR ) ))				// Edy精算利用可能
				if( DspChangeTime[1] && (!Suica_Rec.suica_err_event.BYTE) ) // ﾀｲﾏｰ値が正常にｾｯﾄされているかつSX-10関連のｴﾗｰが発生していない場合
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );	// 画面切替用ﾀｲﾏｰ起動(Timer10)
			}else{															// 取引キャンセル後の制御ﾃﾞｰﾀ送信
				if( !Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL){
					Suica_Ctrl( S_CNTL_DATA, 0x01 );												// Suica利用を可にする
				}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( DspChangeTime[1] && (!Suica_Rec.suica_err_event.BYTE || // ﾀｲﾏｰ値が正常にｾｯﾄされているかつSX-10関連のｴﾗｰが発生していない場合
//										 ( !EDY_USE_ERR ) ))				// Edy精算利用可能
				if( DspChangeTime[1] && (!Suica_Rec.suica_err_event.BYTE) ) // ﾀｲﾏｰ値が正常にｾｯﾄされているかつSX-10関連のｴﾗｰが発生していない場合
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );	// 画面切替用ﾀｲﾏｰ起動(Timer10)
			}	
			break;
		case	2:
			if(!Suica_Rec.Data.BIT.CTRL_MIRYO	&&						// 精算未了状態で無い場合で かつ
				!Suica_Rec.Data.BIT.CTRL 		&&						// 制御ﾃﾞｰﾀが不可　かつ
				OPECTL.op_faz != 3 && ryo_buf.zankin ) {				// 取消しボタン押下時で無い　かつ 残額がある場合
				if( OPECTL.op_faz == 9 )
					break;
				if( OPECTL.op_faz == 8 || OPECTL.op_faz == 10 )			// ｸﾚｼﾞｯﾄでの精算開始(Host応答待ち)でEdy/Suica停止待ち合わせ時か
					break;
				if( ope_faze == 22 ){
				}else{
					if( MifStat == MIF_WROTE_FAIL ){						// Mifare書込み失敗
						break;
					}
				}
				Suica_Ctrl( S_CNTL_DATA, 0x01 );													// Suica利用を可にする
				w_settlement = 0;										// Suica残高不即時の精算額（０時は残高不足でないを意味する）
			}
			break;
	}
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_MiryoEventCheck
//[]----------------------------------------------------------------------[]
///	@param[in]		msg				:getmassegeにて取得したID
///	@return			ret				: 0=処理継続 1=ﾙｰﾌﾟ処理をContinueする
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char Ope_MiryoEventCheck( ushort msg )
{
	char	ret=0;
	uchar	setflag = 0;
	ushort	msgdata = 0;
   uchar	i;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( Suica_Rec.Data.BIT.CTRL_MIRYO || Edy_Rec.edy_status.BIT.CTRL_MIRYO ) // Suica/Edy精算未了状態（タッチ操作未完了）の場合
	if( Suica_Rec.Data.BIT.CTRL_MIRYO ) // Suica精算未了状態（タッチ操作未完了）の場合
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	{
		switch( msg ){
			case COIN_EVT:									// ｲﾍﾞﾝﾄがｺｲﾝﾒｯｸｲﾍﾞﾝﾄの場合/
				if( OPECTL.CN_QSIG == 1 || OPECTL.CN_QSIG == 5 ){
					msgdata = COIN_IN_EVT;
				// ｺｲﾝﾒｯｸから払い出し可を受けた時に、すでに精算完了しているか、精算中止時にはｲﾍﾞﾝﾄ保持
				}else if( OPECTL.CN_QSIG == 7 && (!ryo_buf.zankin || OPECTL.op_faz == 3) ){		// 払い出し可
					msgdata = COIN_EN_EVT;
				}
				if( msgdata != 0 )							// 登録するデータがあれば
					setflag = 1;								// ｲﾍﾞﾝﾄ発生ﾌﾗｸﾞをｾｯﾄ
				break;
			case NOTE_EVT:									// ｲﾍﾞﾝﾄが紙幣ﾘｰﾀﾞｰｲﾍﾞﾝﾄの場合
				if( OPECTL.NT_QSIG == 1 ){
					msgdata = NOTE_IN_EVT;
					setflag = 1;							// ｲﾍﾞﾝﾄ発生ﾌﾗｸﾞをｾｯﾄ
				}
				break;
			case TIMEOUT1:									// ｲﾍﾞﾝﾄがTIMEOUTｲﾍﾞﾝﾄの場合
			case TIMEOUT2:
				if( !ryo_buf.zankin || OPECTL.op_faz == 3 ){	// 残額が無い(精算完了)か、精算中止中の場合
					for( i=0; i < DELAY_MAX; i++ ){				// ｲﾍﾞﾝﾄﾊﾞｯﾌｧを検索
						if( nyukin_delay[i] == COIN_EN_EVT )	// ﾒｯｸ停止ｲﾍﾞﾝﾄがあった場合
							break;								// ループを抜ける
					}
					if( i != DELAY_MAX ){						// ﾒｯｸ停止ｲﾍﾞﾝﾄあり？
						break;									// TIMEOUT系のｲﾍﾞﾝﾄは登録しない
					}
					msgdata = msg;
					setflag = 1;							// ｲﾍﾞﾝﾄ発生ﾌﾗｸﾞをｾｯﾄ
				}
				break;
			case ARC_CR_E_EVT:
				if( WaitForTicektRemove ){
					if(	Suica_Rec.Data.BIT.CTRL_MIRYO ||								// Suica未了中
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						Edy_Rec.edy_status.BIT.CTRL_MIRYO || 							// Edyの引去り未了中
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
						OPECTL.InquiryFlg ||											// ｸﾚｼﾞｯﾄのHOST問い合わせ中
						OPECTL.ChkPassSyu ){											// 定期問い合わせ中
						read_sht_cls();
					}
				} 
				break;
			default:
				break;	
		}
		if( setflag ){
			nyukin_delay[delay_count] = msgdata;			// ｲﾍﾞﾝﾄ保持
			delay_count++;									// ｲﾍﾞﾝﾄ保持件数を更新
		}
		if( msg == ARC_CR_R_EVT ){
			ope_anm( AVM_CARD_ERR5 );			// Card Error Announce(只今このカードはお取り扱いできません)
			if( 1 != rd_tik )								// 駐車券保留なし
				opr_snd( 13 );								// 駐車券保留位置からでも前排出
			else
				opr_snd( 2 );								// 前排出
		}
		switch( msg ){										// 引去り未了中でも受け付けたいﾒｯｾｰｼﾞcase以下に追記
			case SUICA_EVT:									// Suica関連ﾒｯｾｰｼﾞ	
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			case IBK_EDY_RCV:								// Edy関連ﾒｯｾｰｼﾞ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			case CAR_FURIKAE_OUT:							// 振替精算中の振替元が出庫した通知
			case IFMPAY_GENGAKU:							// 減算精算電文受信通知
			case IFMPAY_FURIKAE:							// 振替精算電文受信通知
// MH321800(S) D.Inaba ICクレジット対応
			case EC_EVT_DEEMED_SETTLEMENT:					// みなし決済トリガ発生 
			case AUTO_CANCEL:								// 未了中の自動とりけし発生(決済リーダより未了残高中止受信)
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
			case EC_INQUIRY_WAIT_TIMEOUT:									// 問合せ(処理中)待ちタイムアウト
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH321800(E) D.Inaba ICクレジット対応	
				break;
// MH321800(S) D.Inaba ICクレジット対応
			case COIN_RJ_EVT:								// 精算中止(レバー)による[取消]イベント
			case KEY_TEN_F4:								// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				// 未了確定時は取消イベントを発生させる
// MH810103 GG119202(S) 未了仕様変更対応
//				if(Suica_Rec.Data.BIT.MIRYO_CONFIRM){
//					ret = 0;
				if(isEC_CONF_MIRYO()){						// 未了確定(状態データ)受信済み
					Suica_Ctrl( S_CNTL_DATA, 0 );			// Suica利用を不可にする	
					ret = 1;
// MH810103 GG119202(E) 未了仕様変更対応
				}
				// 未了確定前は取消イベントを発生させない
				else {
					ret = 1;
				}
				break;
// MH321800(E) D.Inaba ICクレジット対応	
			default:
				ret = 1;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
	}

	if( msg == ARC_CR_R_EVT ){												// ｶｰﾄﾞﾘｰﾄﾞｲﾍﾞﾝﾄの場合
		i = 0;
		if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP && SUICA_CM_BV_RD_STOP ){		// 未了タイムアウト後の使用不可設定時
			i = 1;															// 挿入されたカードを返却する
		}
		if( i == 1 ){
			ope_anm( AVM_CARD_ERR1 );			// Card Error Announce(このカードは使えません)
			if( 1 != rd_tik )												// 駐車券保留なし
				opr_snd( 13 );												// 駐車券保留位置からでも前排出
			else
				opr_snd( 2 );												// 前排出
			ret = 1;
		}
	}

	return	ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_ArmClearCheck
//[]----------------------------------------------------------------------[]
///	@param[in]		msg				:getmassegeにて取得したID
///	@return			ret				: 0=処理継続 1=ﾙｰﾌﾟ処理をContinueする
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char Ope_ArmClearCheck( ushort msg )
{
	char	ret=0;
	char	exec=0;
// MH321800(S) G.So ICクレジット対応
	if (isEC_USE() != 0) {
		return	Ope_EcArmClearCheck(msg);
	}
// MH321800(E) G.So ICクレジット対応
	switch( msg ){												// Suica残高不足の反転ﾒｯｾｰｼﾞ表示を消去する
		case SUICA_EVT:											// Suica(Sx-10)からの受信ﾃﾞｰﾀ
			// 決済結果ﾃﾞｰﾀ以外のﾃﾞｰﾀ受信か、決済結果受信で残額不足の場合か、制御ﾃﾞｰﾀ受信の場合は抜ける
			if(Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA != 1 || Settlement_Res.Result == 0x03 || 
			   Suica_Rec.suica_rcv_event.BIT.STATUS_DATA ) {
				break;
			}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case IBK_EDY_RCV:										// EMの受信ﾃﾞｰﾀ
//			// 減算結果通知以外のﾃﾞｰﾀ受信か、未了実施中か、Edyﾘｰﾀﾞｰ状態が不可状態の場合は抜ける
//			if(Edy_Rec.rcv_kind != R_SUBTRACTION || Edy_Rec.edy_status.BIT.CTRL_MIRYO || 
//			   !Edy_Rec.edy_status.BIT.CTRL ) {					
//				break;
//			}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		case COIN_EVT:											// Coin Mech event
		case NOTE_EVT:											// Note Reader event
			if(( COIN_EVT == msg && !( OPECTL.CN_QSIG == 1 || OPECTL.CN_QSIG == 5 )) ||
   			   ( NOTE_EVT == msg && !( OPECTL.NT_QSIG == 1 || OPECTL.NT_QSIG == 5 )))
   			   	break;
		case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
		case KEY_TEN_F4:										// 取消ﾎﾞﾀﾝ ON
		case ELE_EVENT_CANSEL:									// 終了イベント（精算中止）
		case ELE_EVT_STOP:										// 精算完了イベント（電子媒体の停止）
			if( msg == ELE_EVT_STOP && CCT_Cansel_Status.BIT.STOP_REASON != REASON_PAY_END )	// 精算完了時以外は抜ける
				break;
			if( msg == KEY_TEN_CL && OPECTL.CAN_SW == 1 )		// 取消ﾎﾞﾀﾝの再押下の場合
				ret = 1;										// 取消処理をさせない（ｲﾍﾞﾝﾄ破棄）

			if( OPECTL.InquiryFlg )			// 親機問い合わせ中かクレジット問い合わせ中
				break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			// Suica残額不足表示中 か Edyの残額不足表示中 か Edyの未了ﾀｲﾑｱｳﾄ後のﾒｯｾｰｼﾞ表示中 か Suicaの未了ﾀｲﾑｱｳﾄ後のﾒｯｾｰｼﾞ表示中
//			if( dsp_fusoku ||
//				edy_dsp.BIT.edy_dsp_Warning ||
//				edy_dsp.BIT.edy_Miryo_Loss ){
			// Suica残額不足表示中 か Suicaの未了ﾀｲﾑｱｳﾄ後のﾒｯｾｰｼﾞ表示中
			if( dsp_fusoku ){
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				exec = 1;
			}else if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
				if(!SUICA_CM_BV_RD_STOP || msg == KEY_TEN_CL || (OPECTL.op_faz == 3 || OPECTL.op_faz == 9))
					exec = 1;
			}else if( DspSts == LCD_WMSG_ON ){
				exec = 1;
			}
			if( exec ){
			    LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );// 残額不足表示消去用のﾀｲﾏｰｷｬﾝｾﾙ
				op_SuicaFusokuOff();							// 現在表示中のﾒｯｾｰｼﾞを消去
				dsp_change = 0;
			}
	}

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_EleUseDsp
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Ope_EleUseDsp( void )
{
	e_incnt = 0;															// 精算内の「電子マネー」使用回数（含む中止）をクリア
	Suica_Ctrl( S_CNTL_DATA, 0x01 );												// Suica利用を可にする
	Suica_Rec.Data.BIT.PAY_CTRL = 0;										// Suica精算ﾌﾗｸﾞをﾘｾｯﾄ
	DspChangeTime[0] = (ushort)CPrmSS[S_SCA][6];							// 残高表示時間ｾｯﾄ

	DspChangeTime[1] = 2; 													// 画面切り替え時間ｾｯﾄ

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( Dsp_Prm_Setting != 10 )
//		Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );					// 画面切替用ﾀｲﾏｰ起動(Timer10)
//	else{																	// 07-01-22追加
//		Edy_StopAndLedOff();												// ｶｰﾄﾞ検知停止＆LEDOFF指示送信 07-01-22追加
//	}					
	Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );					// 画面切替用ﾀｲﾏｰ起動(Timer10)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
}

//[]----------------------------------------------------------------------[]
///	@brief			利用可能媒体の表示切替
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	op_mod01_dsp_sub( void )
{
	disp_media_flg = 0;													// 画面表示用　精算媒体使用可(0)否(1)																		
	wk_media_Type = Ope_Disp_Media_Getsub(0);							// 画面表示用　精算媒体種別 取得
	switch(wk_media_Type) {												// 「残高照会（可／不可）」用MsgNoをﾜｰｸｴﾘｱに保持
		case OPE_DISP_MEDIA_TYPE_SUICA:
			if( Suica_Rec.suica_err_event.BYTE == 0 ){					// SX-10関連のｴﾗｰが発生していない場合
				DspWorkerea[3] = 97;									// "Ｓｕｉｃａの残額照会ができます"
			} else {													// SX-10関連のｴﾗｰが発生している場合
				DspWorkerea[3] = 96;									// "只今Ｓｕｉｃａは利用できません"
				disp_media_flg = 1;
			}
			break;

		case OPE_DISP_MEDIA_TYPE_PASMO:
			if( Suica_Rec.suica_err_event.BYTE == 0 ){					// SX-10関連のｴﾗｰが発生していない場合
				DspWorkerea[3] = 117;									// "ＰＡＳＭＯの残額照会ができます"
			} else {													// SX-10関連のｴﾗｰが発生している場合
				DspWorkerea[3] = 101;									// "只今ＰＡＳＭＯは利用できません"
				disp_media_flg = 1;
			}
			break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case OPE_DISP_MEDIA_TYPE_EDY:
//			if( Dsp_Prm_Setting == 10 && !Edy_Rec.edy_status.BIT.ZAN_SW ){
//				DspWorkerea[3] = 0;
//				break;
//			}
//
//			if( !EDY_USE_ERR ){											// Edy精算利用可能
//				DspWorkerea[3] = 98;									// "Ｅｄｙの残高照会ができます    "
//			} else {													// Edy関連のｴﾗｰが発生している場合
//				DspWorkerea[3] = 102;									// "只今Ｅｄｙは利用できません    "
//				disp_media_flg = 1;
//			}
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		case OPE_DISP_MEDIA_TYPE_ICCARD:
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( Suica_Rec.suica_err_event.BYTE &&						// SX-10関連のｴﾗｰが発生していない場合
//				EDY_USE_ERR ){											// Edy精算利用不可能
			if( Suica_Rec.suica_err_event.BYTE ){						// SX-10関連のｴﾗｰが発生していない場合
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				DspWorkerea[3] = 103;									// 只今ＩＣカードは利用できません
				disp_media_flg = 1;
			}else{
				DspWorkerea[3] = 118;									// "ＩＣカードの残額照会ができます"
			}
			break;
		case OPE_DISP_MEDIA_TYPE_ICOCA:
			if( Suica_Rec.suica_err_event.BYTE == 0 ){					// SX-10関連のｴﾗｰが発生していない場合
				DspWorkerea[3] = 124;									// "ＩＣＯＣＡの残額照会ができます"
			} else {													// SX-10関連のｴﾗｰが発生している場合
				DspWorkerea[3] = 123;									// "只今ＩＣＯＣＡは利用できません"
				disp_media_flg = 1;
			}
			break;
		case OPE_DISP_MEDIA_TYPE_eMONEY:
			if( Suica_Rec.suica_err_event.BYTE == 0 ){					// SX-10関連のｴﾗｰが発生していない場合
				DspWorkerea[3] = 130;									// "電子マネーの残額照会ができます"
			} else {													// SX-10関連のｴﾗｰが発生している場合
				DspWorkerea[3] = 129;									// "只今電子マネーは利用できません"
				disp_media_flg = 1;
			}
			break;

// MH321800(S) hosoda ICクレジット対応
		case OPE_DISP_MEDIA_TYPE_EC:
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//			if( isEcEnabled(EC_CHECK_EMONEY) ) {						// 電子マネーが有効
//				if( isEcReady(EC_CHECK_EMONEY) ) {						// 電子マネーで決済可能
//					DspWorkerea[3] = 130;								// "電子マネーの残額照会ができます"
//				} else {												// 電子マネーで決済不可
//					DspWorkerea[3] = 129;								// "只今電子マネーは利用できません"
//					disp_media_flg = 1;
//				}
//			}
//			else {
//				DspWorkerea[3] = 0;										// 残高照会督促表示は行わない
//			}
// MH810103 GG119202(S) 開局・有効条件変更
//			if( isEcEmoneyEnabled(1, 0) ){								// 電子マネー決済可能
			{
// MH810103 GG119202(E) 開局・有効条件変更
				if( isEcEmoneyEnabled(1, 1) ){							// 電子マネー決済・残高照会が可能
// MH810103 GG119202(S) 接客画面の電子マネー対応
//					DspWorkerea[3] = 130;								// "電子マネーの残額照会ができます"
					DspWorkerea[3] = 162;								// "電子マネーの残高照会ができます"
// MH810103 GG119202(E) 接客画面の電子マネー対応
				}
				else {
					DspWorkerea[3] = 0;									// 残高照会督促表示は行わない
				}
			}
// MH810103 GG119202(S) 開局・有効条件変更
//			else {
//// GG119202(S) 接客画面の電子マネー対応
////				DspWorkerea[3] = 129;									// "只今電子マネーは利用できません"
//				DspWorkerea[3] = 0;										// 残高照会督促表示は行わない
//// GG119202(E) 接客画面の電子マネー対応
//				disp_media_flg = 1;
//			}
// MH810103 GG119202(E) 開局・有効条件変更
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
			break;
// MH321800(E) hosoda ICクレジット対応

		default:
			DspWorkerea[3] = 0;											// "							 "
			break;
	}	
}

//[]----------------------------------------------------------------------[]
///	@brief			利用可能媒体の表示
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	op_mod01_dsp( void )
{
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Dsp_Prm_Setting = (uchar)prm_get( COM_PRM, S_PAY, 24, 2 ,3 );
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	op_mod01_dsp_sub();
	grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]]);		// 6行目に画面切替用表示をする
// MH321800(S) G.So ICクレジット対応
//	if( !EDY_USE_ERR && Dsp_Prm_Setting == 11 ){
//		if( Edy_Rec.edy_status.BIT.CTRL )
//			Edy_SndData04();											// ｶｰﾄﾞ検知停止指示送信	 						
//		Edy_SndData01();												// ｶｰﾄﾞ検知指示送信	 
//	}
// MH321800(E) G.So ICクレジット対応

	edy_dsp.BYTE = 0;															// Edy画面表示系領域の初期化
	Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;										// Suica画面表示ﾌﾗｸﾞ初期化
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	Elec_Data_Initialize();														// 電子決済使用領域の初期化
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Op_StopModuleWait
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0：電子媒体停止開始 1：電子媒体無効
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 07/10/12<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Op_StopModuleWait( unsigned char stop_kind )
{

// MH321800(S) G.So ICクレジット対応
// 停止待ちの間に他の要因から再度停止が要求されたときのための保護
	if (OPECTL.op_faz == 8) {
	// 既に停止中ならなにもしない
		if (stop_kind == REASON_PAY_END) {
		// 停止理由＝精算完了は優先させる
			CCT_Cansel_Status.BIT.STOP_REASON = stop_kind;				// 停止理由(要因)をセット
		}
		return 0;
	}
// MH321800(E) G.So ICクレジット対応
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	if (isEC_USE()) {
	// 決済リーダーは、リーダーで決済していなければ止めない
		if (OPECTL.Ope_mod == 2 &&										// 精算中で
			(Suica_Rec.Data.BIT.ADJUSTOR_NOW != 0 ||					// 選択商品データ送信済み　か
			 Suica_Rec.Data.BIT.SELECT_SND != 0 ||						// 選択商品データ送信中　か
			 stop_kind == REASON_PAY_END)) {							// 停止理由＝精算完了
			 ;															// リーダー停止処理する
		}
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
		else if (stop_kind == REASON_RTM_REMOTE_PAY) {
			// 遠隔精算開始時は必ず停止する
		}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
		else {
			return 1;
		}
	}
// MH810103 2GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( (Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR) || 				// Suica/Edyが有効の場合
//	    (Edy_Rec.edy_status.BIT.CTRL && !EDY_USE_ERR)){	
	if( (Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR) ){				// Suicaが有効の場合
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		if( (Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR) ){
// MH321800(S) T.Nagai ICクレジット対応
//			Suica_Ctrl( S_CNTL_DATA, 0 );										// Suica利用を不可にする
			if (isEC_USE()) {
// MH810103 GG119202(S) 精算完了時も金額変更ビットをセットする
//				if (Suica_Rec.Data.BIT.ADJUSTOR_START != 0 &&			// 精算開始済み
//					stop_kind != REASON_PAY_END) {						// 停止理由が精算完了以外
//					// 精算完了による受付禁止以外の場合は金額変更ビットをセットする
//					Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );	// 受付禁止送信(金額変更)
//				}
//				else {
//					Suica_Ctrl( S_CNTL_DATA, 0 );						// 受付禁止送信
//				}
				Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// 受付禁止送信(金額変更)
// MH810103 GG119202(E) 精算完了時も金額変更ビットをセットする
			}
			else {
				Suica_Ctrl( S_CNTL_DATA, 0 );							// 受付禁止送信
			}
// MH321800(E) T.Nagai ICクレジット対応
		}
		else{														// 停止要求時にSuicaがすでに停止済みの場合
			CCT_Cansel_Status.BIT.SUICA_END = 1;
			if( SUICA_USE_ERR ){									// Suica系のエラーが発生している場合
				Suica_Rec.Data.BIT.CTRL = 0;						// 受付不可状態にしておく
			}
		}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		if( (Edy_Rec.edy_status.BIT.CTRL && !EDY_USE_ERR) ){		// Edyが受付可状態で、Edy関連のｴﾗｰがない場合
//			Edy_StopAndLedOff();									// ｶｰﾄﾞ検知停止＆UI LED消灯指示送信				
//		}else{														// 停止要求時にEdyがすでに停止済みの場合
//			CCT_Cansel_Status.BIT.EDY_END = 1;						// Edy停止済みﾌﾗｸﾞをｾｯﾄ
//			if( EDY_USE_ERR ){										// Edyのエラーが発生している場合
//				Edy_Rec.edy_status.BIT.CTRL = 0;					// 受付不可状態にしておく
//			}
//		}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) G.So ICクレジット対応
//		Lagcan( OPETCBNO, 10 );										// ﾀｲﾏｰ10ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
		if(isEC_USE()) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
		} else {
			Lagcan( OPETCBNO, 10 );									// ﾀｲﾏｰ10ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
		}
// MH321800(E) G.So ICクレジット対応
		CCT_Cansel_Status.BIT.INITIALIZE = 1;						// 電子媒体停止開始
		CCT_Cansel_Status.BIT.STOP_REASON = stop_kind;				// 停止理由(要因)をセット
		save_op_faz = OPECTL.op_faz;								// 現在のフェーズを保持
		OPECTL.op_faz = 8;											// 電子媒体停止フェーズをセット

		if( stop_kind != REASON_MIF_WRITE_LOSS ){					// Mifare書込み失敗時はMifareを停止させない
			if( MIFARE_CARD_DoesUse ){									// Mifareが有効な場合
				op_MifareStop_with_LED();								// Mifare無効
			}
		}
		
		LagCan500ms(LAG500_MIF_LED_ONOFF);
		Op_StopModuleWait_sub( 0xff );								// 全デバイスの終了をチェックする
		return 0;													// 停止処理開始
	}

	if( stop_kind == REASON_PAY_END && !CCT_Cansel_Status.BIT.INITIALIZE ){		// 精算完了時には基本的に電子媒体は不可になっているのでここで処理する
		CCT_Cansel_Status.BIT.INITIALIZE = 1;						// 電子媒体停止開始
		CCT_Cansel_Status.BIT.STOP_REASON = stop_kind;				// 停止理由(要因)をセット
		save_op_faz = OPECTL.op_faz;								// 現在のフェーズを保持
		OPECTL.op_faz = 8;											// 電子媒体停止フェーズをセット
		if( !Suica_Rec.Data.BIT.CTRL ){								// 停止要求時にSuicaがすでに停止済みの場合
			CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica停止済みﾌﾗｸﾞをｾｯﾄ						
		}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		if( !Edy_Rec.edy_status.BIT.CTRL ){							// 停止要求時にEdyがすでに停止済みの場合
//			CCT_Cansel_Status.BIT.EDY_END = 1;						// Edy停止済みﾌﾗｸﾞをｾｯﾄ
//		}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		Op_StopModuleWait_sub( 0xff );								// 全デバイスの終了をチェックする
		return 0;													// 停止処理開始		
	}
	return 1;						
}

#define STOP_BIT (32*CCT_Cansel_Status.BIT.STOP_REASON)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define Suica_STOP_BYTE 9
//#define EDY_STOP_BYTE	5
//
//#define Suica_CMN_STOP_BYTE 11
//#define EDY_CMN_STOP_BYTE 7
//#define Suica_EDY_STOP_BYTE 13
//#define ALL_STOP_BYTE 15
#define CMN_STOP_BYTE	0x03
#define Suica_STOP_BYTE	0x05
#define ALL_STOP_BYTE	0x07
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//[]----------------------------------------------------------------------[]
///	@brief			Op_StopModuleWait_sub
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret：0：停止待ち合わせ中 1：停止完了
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 07/10/12<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Op_StopModuleWait_sub( uchar kind )
{
	uchar ret = 0;
	
	switch( kind ){
		case 0:														// Suicaイベント
			if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// 精算完了時の電子媒体停止待ち合わせ
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_CMN_STOP_BYTE ){	// すでにEdyとｺｲﾝﾒｯｸが停止済みの場合
//					ret = 1;											// 
//				}else{													// Edyかｺｲﾝﾒｯｸがまだ停止していない場合
//					if( EDY_USE_ERR ){
//						CCT_Cansel_Status.BIT.EDY_END = 1;				// Edy停止済みﾌﾗｸﾞをｾｯﾄ
//						if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_CMN_STOP_BYTE ){	// Edy設定なしかEdy関連ｴﾗｰが発生中でｺｲﾝﾒｯｸも停止済み
//							ret = 1;											// 
//						}
//					}
//				}
				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == CMN_STOP_BYTE ){	// すでにｺｲﾝﾒｯｸが停止済みの場合
					ret = 1;
				}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica停止済みﾌﾗｸﾞをｾｯﾄ
				break;
			}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_STOP_BYTE || EDY_USE_ERR ){	// すでにEdyが停止済みかEdy関連ｴﾗｰが発生中の場合
//				ret = 1;											// 
//				CCT_Cansel_Status.BIT.EDY_END = 1;					// Edy停止済みﾌﾗｸﾞをｾｯﾄ
//			}
			ret = 1;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica停止済みﾌﾗｸﾞをｾｯﾄ
			break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case 1:														// Edyイベント
//			if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// 精算完了時の電子媒体停止待ち合わせ
//				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == EDY_CMN_STOP_BYTE ){	// すでにSuicaとｺｲﾝﾒｯｸが停止済みの場合
//					ret = 1;											// 
//				}else{													// Suicaかｺｲﾝﾒｯｸがまだ停止していない場合
//					if( SUICA_USE_ERR ){
//						CCT_Cansel_Status.BIT.SUICA_END = 1;				// Suica停止済みﾌﾗｸﾞをｾｯﾄ
//						if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_CMN_STOP_BYTE ){	// Suica設定なしかSuica関連ｴﾗｰが発生中でｺｲﾝﾒｯｸも停止済み
//							ret = 1;											// 
//						}
//					}
//				}
//				CCT_Cansel_Status.BIT.EDY_END = 1;					// Edy停止済みﾌﾗｸﾞをｾｯﾄ
//				break;
//			}
//			if( (CCT_Cansel_Status.BYTE-STOP_BIT) == EDY_STOP_BYTE || SUICA_USE_ERR ){		// Suicaが停止済みかSuica関連ｴﾗｰが発生中
//				ret = 1;
//				CCT_Cansel_Status.BIT.SUICA_END = 1;				// Suica停止済みﾌﾗｸﾞをｾｯﾄ
//			}
//			CCT_Cansel_Status.BIT.EDY_END = 1;						// Edy停止済みﾌﾗｸﾞｾｯﾄ
//			break;
//		case 2:														// 電子媒体の再活性要求
//			if( !EDY_USE_ERR && !Edy_Rec.edy_status.BIT.CTRL ){					// Edy関連のエラー未発生かつEdy停止状態の場合
//				Edy_SndData01();								// Edy利用復活（設定あれば）
//			}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		case 2:														// 電子媒体の再活性要求
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( !EDY_USE_ERR && !Edy_Rec.edy_status.BIT.CTRL ){					// Edy関連のエラー未発生かつEdy停止状態の場合
//				Edy_SndData01();								// Edy利用復活（設定あれば）
//			}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			if( !SUICA_USE_ERR && !Suica_Rec.Data.BIT.CTRL ){					// Suica関連のエラー未発生かつSuica停止状態の場合
// MH321800(S) T.Nagai ICクレジット対応
				if (isEC_USE()) {
					EcSendCtrlEnableData();							// 受付許可送信
				}
				else {
// MH321800(E) T.Nagai ICクレジット対応
				Suica_Ctrl( S_CNTL_DATA, 0x01 );									// Suica利用を可にする
// MH321800(S) T.Nagai ICクレジット対応
				}
// MH321800(E) T.Nagai ICクレジット対応
			}
// MH321800(S) Y.Tanizaki ICクレジット対応
//			Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ
			if(isEC_USE()) {
				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ
			} else {
				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ
			}
// MH321800(E) Y.Tanizaki ICクレジット対応
			break;
		case 3:														// ｺｲﾝﾒｯｸの停止
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_EDY_STOP_BYTE ){	// すでにSuicaとEdyが停止済みの場合
//				ret = 1;											// 
//			}else{													// Suicaかｺｲﾝﾒｯｸがまだ停止していない場合
//				if( SUICA_USE_ERR || Suica_Rec.Data.BIT.MIRYO_TIMEOUT ){	// ｴﾗｰが発生しているか、未了発生後(必ず不可になるため)の場合
//					CCT_Cansel_Status.BIT.SUICA_END = 1;			// Suica停止済みﾌﾗｸﾞをｾｯﾄ
//					if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_EDY_STOP_BYTE ){	// Suica設定なしかSuica関連ｴﾗｰが発生中でｺｲﾝﾒｯｸも停止済み
//						ret = 1;											// 
//					}
//				}
//				if( EDY_USE_ERR || Edy_Rec.edy_status.BIT.MIRYO_LOSS ){
//					CCT_Cansel_Status.BIT.EDY_END = 1;				// Edy停止済みﾌﾗｸﾞをｾｯﾄ
//					if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_EDY_STOP_BYTE ){	// Edy設定なしかEdy関連ｴﾗｰが発生中でｺｲﾝﾒｯｸも停止済み
//						ret = 1;											// 
//					}
//				}
//			}
			if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_STOP_BYTE ){	// すでにSuicaが停止済みの場合
				ret = 1;												// 全ﾃﾞﾊﾞｲｽ停止完了
			}else{														// Suicaかｺｲﾝﾒｯｸがまだ停止していない場合
				if( SUICA_USE_ERR || Suica_Rec.Data.BIT.MIRYO_TIMEOUT ){	// ｴﾗｰが発生しているか、未了発生後(必ず不可になるため)の場合
					CCT_Cansel_Status.BIT.SUICA_END = 1;				// Suica停止済みﾌﾗｸﾞをｾｯﾄ
					ret = 1;											// 全ﾃﾞﾊﾞｲｽ停止完了
				}
			}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			CCT_Cansel_Status.BIT.CMN_END = 1;					// Edy停止済みﾌﾗｸﾞをｾｯﾄ
			if( !CCT_Cansel_Status.BIT.SUICA_END ){					
				Lagtim( OPETCBNO, 2, 10*50 );						// ﾀｲﾏｰ2起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
			}
			break;
		case 4:														// Suica決済による精算完了
			CCT_Cansel_Status.BYTE = 0;
			CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica停止済みﾌﾗｸﾞをｾｯﾄ
			goto Op_StopModuleWait_sub_ALL;
			break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case 5:														// Edy決済による精算完了
//			CCT_Cansel_Status.BYTE = 0;
//			// Edy決済時はSuicaの停止を待ち合わせてから引去るのでSuicaは必ず停止済みになっているため、
//			// Edy決済時にSuicaの終了フラグもセットする
//			CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica停止済みﾌﾗｸﾞをｾｯﾄ
//			CCT_Cansel_Status.BIT.EDY_END = 1;						// Edy停止済みﾌﾗｸﾞをｾｯﾄ
//			goto Op_StopModuleWait_sub_ALL;
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		case 0xff:													// 全てのデバイスが完了しているかどうかを判定する
Op_StopModuleWait_sub_ALL:
			if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// 精算完了時はメックの停止まで待ち合わせる
				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == ALL_STOP_BYTE ){	// すでにすべてのデバイスが停止済み
					ret = 1;										// 全ﾃﾞﾊﾞｲｽ停止完了
				}
			}else{													// それ以外の停止待ち合わせ時はSuicaとEdyの待ち合わせを行う
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_EDY_STOP_BYTE ){	// すでにすべてのデバイスが停止済み
				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_STOP_BYTE ){	// すでにすべてのデバイスが停止済み
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					ret = 1;										// 全ﾃﾞﾊﾞｲｽ停止完了
				}				
			}
			break;	
		default:
			break;
	}

	if( ret == 1 ){
		OPECTL.op_faz = save_op_faz;								// 精算ﾌｪｰｽﾞを元に戻す
		queset( OPETCBNO, ELE_EVT_STOP, 0, NULL );					// オペに電子媒体の停止を通知					
	}

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			初回表示画面判定関数
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret ：0:割引(入金済み)画面 1:初回画面
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/04/16<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char	First_Pay_Dsp( void )
{
	char	ret = 0;

	if((0 == ryo_buf.waribik) &&
	   (0 == c_pay) &&
	   (0 == e_pay) &&
	   (mode_Lagtim10 == 0) &&
	   ( !carduse() ) &&
	   ( vl_ggs.ryokin == 0 &&								// vl_ggs ： 減額精算領域 が0の時にｻｲｸﾘｯｸ表示再開
	   ( vl_frs.lockno == 0 || vl_frs.antipassoff_req) ) &&	// vl_frs ： 振替精算領域 が0、もしくは定期利用有りの時にｻｲｸﾘｯｸ表示再開	   														
	   (ryo_buf.nyukin == 0))
	   ret = 1;

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			決済結果データ受信ﾌｪｰｽﾞ異常時のエラー登録処理
//[]----------------------------------------------------------------------[]
///	@param[in]		*dat ： 決済結果データ
///	@param[in]		kind ： 1:決済不可なﾌｪｰｽﾞで決済結果を受信
///				   		 ： 2:Opeが処理する前に再度決済結果を受信
///				   		 ： 3:Opeが一度決済したのにも関わらず、再度
///				   		 	  決済結果を受信した場合
///				   		 ： 4:決済結果データを受信後、精算LOG等に登録
///				   			  する前にﾌｪｰｽﾞが変わった場合
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/05/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	Settlement_rcv_faze_err( uchar *dat, uchar kind )
{
// MH321800(S) G.So ICクレジット対応(静的解析)
//	uchar	wkuc[24];
	uchar	wkuc[ERR_LOG_ASC_DAT_SIZE];
// MH321800(E) G.So ICクレジット対応(静的解析)
	SUICA_SETTLEMENT_RES *work_set;
	long work;
	
	memset(wkuc, 0, sizeof(wkuc));
	if( kind == 1 || kind == 4 ){								// 決済結果データエリアにすでにデータが格納されている場合はこっち
		work_set = (SUICA_SETTLEMENT_RES *)dat;					// データセット
		intoasl( wkuc, (ulong)work_set->settlement_data, 5 );	// 決済額をセット
		wkuc[5] = wkuc[10] = ':';								//
		wkuc[6] = (uchar)(kind + 0x30);							// 登録種別をセット
		intoas( &wkuc[7], (ushort)OPECTL.Ope_mod, 3 );			// 登録時のｵﾍﾟﾌｪｰｽﾞをセット
		memcpy( &wkuc[11], "****", 4 );							// 上位４桁をマスクする
		memcpy( &wkuc[15], &work_set->Suica_ID[12], 4 );		// カード番号の下４桁をセット
	}else{														// 受信データをそのまま使う場合はこっち
		work = settlement_amount(&dat[3]);						// BCD形式の決済額をBin形式に変換
		intoasl( wkuc, (ulong)work, 5 );						// さらにAscii形式に変換して格納
		wkuc[5] = wkuc[10] = ':';								//
		wkuc[6] = (uchar)(kind + 0x30);							// 登録種別をセット
		intoas( &wkuc[7], (ushort)OPECTL.Ope_mod, 3 );			// 登録時のｵﾍﾟﾌｪｰｽﾞをセット
		memcpy( &wkuc[11], "****", 4 );							// 上位４桁をマスクする
		memcpy( &wkuc[15], &dat[24], 4 );                       // カード番号の下４桁をセット
	}
	err_chk2( (char)ERRMDL_SUICA, (char)61, (char)2, (char)1, (char)1, (void*)wkuc );
			// E6962登録（決済額パラメータあり：ASCII） "E6961   08/03/17 14:38   発生/解除"
			//											"        (99999:888:------------)  "
			//													  引去り金額と、その時のOPmod
	#if (4 != AUTO_PAYMENT_PROGRAM)								// 試験用にカード残額をごまかす場合（ﾃｽﾄ用）
	memcpy( &SUICA_LOG_REC_FOR_ERR, &SUICA_LOG_REC, sizeof( struct	suica_log_rec ));	// 異常ﾛｸﾞとして現在の通信ﾛｸﾞを登録する。
	#endif
	
	// 重複登録を防ぐためにすでに他の要因でエラー登録されていた場合は、kind=4の時の
	// エラー登録要因ﾌﾗｸﾞをクリアする
	Suica_Rec.Data.BIT.LOG_DATA_SET = 0;

// MH321800(S) Y.Tanizaki ICクレジット対応(決済異常発生後は決済させない)
	Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR = 1;
// MH321800(E) Y.Tanizaki ICクレジット対応(決済異常発生後は決済させない)
}

//[]----------------------------------------------------------------------[]
///	@brief			受信データ待ち処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret: 0:Continue	1:recv wait
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/06/10<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar	rcv_split_data_check( void )
{
	uchar ret = 0;
	t_Suica_rcv_event w_rcv_data;
	uchar rcv_judge=0;
	
	w_rcv_data.BYTE = Suica_Rec.suica_rcv_event.BYTE;							// 受信データ情報をワークエリアにセット
	if( !w_rcv_data.BYTE )														// 受信ｲﾍﾞﾝﾄが何も無い場合
		return (ret=1);															// 受信解析処理はしない
	
	rcv_judge = 0x0B;
	w_rcv_data.BYTE &= rcv_judge;												// 表示依頼・決済結果・状態データのみ抽出
	if( !Suica_Rec.Data.BIT.DATA_RCV_WAIT ){									// 待ち合わせ中？
		if( Suica_Rec.Data.BIT.ADJUSTOR_NOW ){									// Suica精算中？(商品選択データ送信時にセットされる)
			if(( w_rcv_data.BYTE == rcv_judge && Settlement_Res.Result == 0x01 )||	// 決済時の必要なデータが全てそろってる？か
			   ( w_rcv_data.BYTE == rcv_judge && Settlement_Res.Result == 0x03 )||	// 残額不足カードタッチ時の必要なデータが全て揃っている
			   ( w_rcv_data.BIT.STATUS_DATA &&										// 状態データ受信で
			   ( STATUS_DATA.StatusInfo.TradeCansel &&							// 取引キャンセル受付ビットがセットされている
			    STATUS_DATA.StatusInfo.ReceptStatus ))){						// 受付不可ビットがセットされている
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;							// Suica精算中フラグクリア
				ret = 0;														// 明示的に0をセット
			}else if( w_rcv_data.BYTE == rcv_judge && Settlement_Res.Result == 0x02 ){	// 使用不可カードタッチ時
				ret = 0;														// 明示的に0をセット
			}else if( w_rcv_data.BIT.STATUS_DATA &&								// 状態データ受信で
				    ( STATUS_DATA.StatusInfo.MiryoStatus ||						// 未了発生か
					  STATUS_DATA.StatusInfo.Running )){						// 実行中の場合は、精算状態は継続し、OpeTaskへの通知を行う
					ret = 0;													// 明示的に0をセット
			}else if( w_rcv_data.BIT.STATUS_DATA  && 
					  (!STATUS_DATA.StatusInfo.ReceptStatus &&					// 受付可かつ取引キャンセル受付BITが立っていない場合は、精算状態は継続し、OpeTaskへの通知は行わない
					   !STATUS_DATA.StatusInfo.TradeCansel) ){					
					ret = 0;													// 明示的に0をセット
			}else{
				if( w_rcv_data.BIT.STATUS_DATA ){								// 受信したデータが制御データで待ち合わせの場合
					STATUS_DATA_WAIT.status_data = STATUS_DATA.status_data;		// 制御情報を待ち合わせようにセーブ
				}
				Suica_Rec.rcv_split_event.BYTE = w_rcv_data.BYTE;				// 待ち合わせ用のエリアに状態を保存
				Suica_Rec.Data.BIT.DATA_RCV_WAIT = 1;							// データ受信待ちフラグセット
				ret = 1;														// 受信処理はしない（待ち合わせ中）
			}
		}
	}else{
		Suica_Rec.rcv_split_event.BYTE |= w_rcv_data.BYTE;						// 受信したデータの状態を待ち合わせエリアに反映

		if( w_rcv_data.BIT.STATUS_DATA ){										// 受信したデータが制御データの場合
			STATUS_DATA.status_data |= STATUS_DATA_WAIT.status_data;			// 待ち合わせ用のデータをマージする
		}
		
		if(( Suica_Rec.rcv_split_event.BYTE == rcv_judge && Settlement_Res.Result == 0x01 )|| // 決済時の必要なデータが全てそろってる？か
		   ( Suica_Rec.rcv_split_event.BYTE == rcv_judge && Settlement_Res.Result == 0x03 )|| // 残額不足カードタッチ時の必要なデータが全て揃っている
		  ((( Suica_Rec.rcv_split_event.BIT.STATUS_DATA ) && 					// 状態データ受信で
		  (STATUS_DATA.StatusInfo.TradeCansel  && 								// 取引キャンセル受付ビットがセットされている
		   STATUS_DATA.StatusInfo.ReceptStatus)))){								// 受付不可ビットがセットされている
			Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;								// Suica精算中フラグクリア
			Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;								// 待ち合わせフラグクリア
			Suica_Rec.suica_rcv_event.BYTE = Suica_Rec.rcv_split_event.BYTE;	// 受信判定用のエリアを更新
			STATUS_DATA.status_data = 0;										// 状態データ待ち合わせ用エリアクリア
			ret = 0;															// 受信処理を行う
		}else if( w_rcv_data.BYTE == rcv_judge && Settlement_Res.Result == 0x02 ){
			ret = 0;															// 受信処理を行う
		}else{
			ret = 1;															// 受信処理はしない（待ち合わせ中）
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			受信データ待ち処理用のフラグをクリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/06/11<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	Suica_rcv_split_flag_clear( void )
{
	Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;						// Suica精算中フラグクリア
	Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;						// 待ち合わせフラグクリア	
	Suica_Rec.Data.BIT.SEND_CTRL80 = 0;							// 取引終了送信情報クリア
	STATUS_DATA.status_data = 0;								// 待ち合わせ時の状態データ保持エリアクリア
	Suica_Rec.Data.BIT.ADJUSTOR_START = 0;						// 精算開始フラグセット
}

#if (4 == AUTO_PAYMENT_PROGRAM)								// 試験用に通信ログ取得方式を変更する
static uchar	d_work_buf[256];
//[]----------------------------------------------------------------------[]
///	@brief			Suica_Log_regist
//[]----------------------------------------------------------------------[]
///	@param[in]		logdata	：登録するログデータ
///				    log_size：登録するログサイズ
///				    kind	：0：受信データ 1：送信データ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/06/10<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Suica_Log_regist_for_debug( unsigned char* logdata, ushort log_size, uchar kind )
{
	ushort  i,k,wks = 0;

	memset( &time_data, 0, sizeof( time_data ));
	memset( &d_work_buf, 0, sizeof( d_work_buf ));

	if( !SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start ){		// ﾛｸﾞﾃﾞｰﾀが初期化されている場合
		SUICA_LOG_REC_FOR_ERR.log_time_old = CLK_REC.ndat;					// 最古の日付ﾃﾞｰﾀをﾊﾞｯｸｱｯﾌﾟ
		Log_data_edit( &SUICA_LOG_REC_FOR_ERR, 1 );							// 日付ﾃﾞｰﾀの編集
	}
	if( CLK_REC.ndat != suica_work_time.ndat && SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start ){	// 日付が更新されている場合
		Log_data_edit( &SUICA_LOG_REC_FOR_ERR, 1 );														// 日付ﾃﾞｰﾀの編集
	}
	if( CLK_REC.nmin != suica_work_time.nmin || !SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start ){	// 時間が更新されている場合
		Log_data_edit( &SUICA_LOG_REC_FOR_ERR, 0 );														// 時間ﾃﾞｰﾀの編集
	}

	Log_data_edit2( &SUICA_LOG_REC_FOR_ERR,kind );														// ﾃﾞｰﾀﾍｯﾀﾞの編集(送信【SD】・受信【RD】)

	if( (SUICA_LOG_REC_FOR_ERR.log_wpt+(log_size*2)) > SUICA_LOG_MAXSIZE-1){							// 編集ﾃﾞｰﾀのｻｲｽﾞﾁｪｯｸ
		wks = SUICA_LOG_MAXSIZE-SUICA_LOG_REC_FOR_ERR.log_wpt;											// ﾊﾞｯﾌｧの最後まで格納できるｻｲｽﾞを算出

		for( i=0,k=0; i<log_size; i++){															// 通信ﾃﾞｰﾀの編集
			hxtoas ( &d_work_buf[k],*(logdata+i) );												// ﾍｷｻﾃﾞｰﾀを文字列に変換
			k+=2;																				
		}
		memcpy( &SUICA_LOG_REC_FOR_ERR.log_Buf[SUICA_LOG_REC_FOR_ERR.log_wpt], d_work_buf, (size_t)wks );			// ﾊﾞｯﾌｧの最後に書き込める分の通信ﾃﾞｰﾀを書込む
		SUICA_LOG_REC_FOR_ERR.log_wpt=7;																// 書込みﾎﾟｲﾝﾀの更新

		Log_data_edit3( &SUICA_LOG_REC_FOR_ERR, (ushort)(SUICA_LOG_REC_FOR_ERR.log_wpt+k-wks), SUICA_LOG_REC_FOR_ERR.log_wpt );	// 日付更新ﾁｪｯｸ

		if( (k-wks) != 0 ){
			memcpy( &SUICA_LOG_REC_FOR_ERR.log_Buf[7], &d_work_buf[wks], (size_t)(k-wks) );				// 残りのﾃﾞｰﾀを先頭から書込む
			SUICA_LOG_REC_FOR_ERR.log_wpt += (k-wks);													// 書込みﾎﾟｲﾝﾀの更新
		}
		SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.write_flag=1;											// 日付更新ﾁｪｯｸﾌﾗｸﾞ更新

	}else{
		if( SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.write_flag ){										// 日付更新ﾁｪｯｸﾌﾗｸﾞON
			Log_data_edit3( &SUICA_LOG_REC_FOR_ERR, (ushort)(log_size+SUICA_LOG_REC_FOR_ERR.log_wpt), SUICA_LOG_REC_FOR_ERR.log_wpt );	// 日付更新ﾁｪｯｸ
		}
		for( i=0; i<log_size; i++){
			hxtoas ( &((uchar)SUICA_LOG_REC_FOR_ERR.log_Buf[SUICA_LOG_REC_FOR_ERR.log_wpt]),*(logdata+i) );		// 通信ﾃﾞｰﾀの変換・書込み
			SUICA_LOG_REC_FOR_ERR.log_wpt+=2;															// 書込みﾎﾟｲﾝﾀの更新
		}
	}

	if( !SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start )									// 初期化ﾌﾗｸﾞが０の場合
		SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start = 1;									// 初期化ﾌﾗｸﾞの更新

	suica_work_time = CLK_REC;																		
	
}
#endif

//[]----------------------------------------------------------------------[]
///	@brief			受信キューから電文をReadする
//[]----------------------------------------------------------------------[]
///	@param[in]		recv_buf ：受信キューから取り出したデータ格納領域
///					recv_size：受信データサイズ
///	@return			ret 0：ﾃﾞｰﾀ種別毎の解析処理実施 1:受信ﾃﾞｰﾀ待ち合わせ
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/10<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar	Suica_Read_RcvQue( void )
{
	uchar *pData = (uchar*)&suica_rcv_que.rcvdata[suica_rcv_que.readpt];	// 受信ｷｭｰのﾎﾟｲﾝﾀをﾜｰｸ領域設定
	uchar i,bc;
	uchar ret = 1;
	ushort	cnt;
	ushort	w_counter[3];
	
	memcpy( w_counter, &suica_rcv_que.readpt, sizeof( w_counter ) );		// ﾎﾟｲﾝﾀ処理領域の初期化

	for( i=0; (i<6 && *pData != ack_tbl[i]); i++);							// 受信種別(ACK1～NACK)検索
	
    if( i==1 ){																// ACK2 データ受信 ?
    	pData+=2;															// ﾃﾞｰﾀﾎﾟｲﾝﾀの更新
		suica_save( pData, (short)*(pData-1) ); 							// 受信ﾃﾞｰﾀの解析処理
		if( rcv_split_data_check() == 0 )									// ﾃﾞｰﾀ待ち合わせ判定処理
			ret = 0;														// 種別毎の解析処理実施
    }else if( (i==2 )||(i==0) ){ 											// ACK3 or ACK1 データ受信?
    	bc = *(pData+1);													// 受信ﾃﾞｰﾀ数を保持
    	pData+=2;															// ﾃﾞｰﾀﾎﾟｲﾝﾀの更新
		for( cnt=0 ;cnt < bc; ){											// 全ての受信ﾃﾞｰﾀの解析が完了するまでﾙｰﾌﾟ
			cnt += (*pData + 1);											// ﾃﾞｰﾀ種別毎のﾃﾞｰﾀｻｲｽﾞを加算
			pData++;														// ﾃﾞｰﾀﾎﾟｲﾝﾀの更新
			suica_save( pData,(short)*(pData-1) ); 							// 受信ﾃﾞｰﾀの解析処理
			pData += *(pData-1);											// ﾃﾞｰﾀﾎﾟｲﾝﾀの更新(次のﾃﾞｰﾀまで進める)
		}
		if( rcv_split_data_check() == 0 )									// ﾃﾞｰﾀ待ち合わせ判定処理
			ret = 0;                                                        // 種別毎の解析処理実施
    }
	
	if( ++w_counter[0] >= SUICA_QUE_MAX_COUNT ){							// ﾘｰﾄﾞﾎﾟｲﾝﾀがMAXに到達
		w_counter[0] = 0;													// ﾘｰﾄﾞﾎﾟｲﾝﾀを先頭に移動
	}
	if( w_counter[2] != 0 ){												// 受信ﾃﾞｰﾀｶｳﾝﾄ数が０でない場合
		w_counter[2]--;														// ﾃﾞｸﾘﾒﾝﾄ実施
	}

	nmisave( &suica_rcv_que.readpt, w_counter, sizeof(w_counter) );			// ﾎﾟｲﾝﾀの更新を停電保障で行う
	return ret;
	
}

//[]----------------------------------------------------------------------[]
///	@brief			表示依頼データ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvDspData( uchar*	ope_faze )
{
	short ret = 0;
	ushort	wk_MsgNo=0;														// 画面表示用　精算媒体種別毎のﾒｯｾｰｼﾞNo.
	
	switch( *ope_faze ){
		case	0:
				if( OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod == 100 )
					break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				Edy_StopAndLedOff();										// EdyStop＆LED消灯
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				edy_dsp.BIT.suica_zangaku_dsp = 1;							// 残額照会中ﾌﾗｸﾞｾｯﾄ

				LcdBackLightCtrl( ON );										// back light ON
				if( Is_SUICA_STYLE_OLD ){									// 旧版動作
					wk_media_Type = Ope_Disp_Media_Getsub(0);				// 画面表示用　精算媒体種別 取得
					switch(wk_media_Type) {
						case OPE_DISP_MEDIA_TYPE_SUICA:
							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[93] );			// "Ｓｕｉｃａ残額：          円  "
							break;
						case OPE_DISP_MEDIA_TYPE_PASMO:
							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[104] );		// "ＰＡＳＭＯ残額：          円  "
							break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						case OPE_DISP_MEDIA_TYPE_EDY:
//							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[94] );			// "   Ｅｄｙ残額：          円   "
//							break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
						case OPE_DISP_MEDIA_TYPE_ICCARD:
							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[105] );		// "ＩＣカード残額：          円  "
							break;
						case OPE_DISP_MEDIA_TYPE_ICOCA:
							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[119] );		// "ＩＣＯＣＡ残額：          円  "
							break;
						default:
							break;
					}
					opedpl( 6, 16, pay_dsp, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );					// 金額を表示
				}															// 旧版動作(e)
				else{														// 新版動作
					grachr( 6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[127] );					// "　　　残額：　　　　　円　　　"

					opedpl( 6, 12, pay_dsp, 5, 0, 0, COLOR_FIREBRICK,  LCD_BLINK_OFF );						// 金額を表示
				}

				if( DspChangeTime[0] != 99 )								// Suica残高表示時間が99秒だった場合は他のｲﾍﾞﾝﾄによる画面切替まで表示
					Lagtim( OPETCBNO,11, (ushort)(DspChangeTime[0]*50) );	// ICｶｰﾄﾞ残高表示用ﾀｲﾏｰ起動(Timer9)

				Lagcan( OPETCBNO, 10 );										// 画面切替用ﾀｲﾏｰﾘｾｯﾄ(Timer10)
				if( DspChangeTime[0] > 10 ){								// 残額表示時間＞10Ｓ // 07-01-16追加 //
					Lagtim( OPETCBNO, 1, (ushort)(DspChangeTime[0]*50) );   // ﾊﾞｯｸﾗｲﾄ点灯時間を残高表示時間とする
				}else{
					Lagtim( OPETCBNO, 1, 10*50 );							// ﾊﾞｯｸﾗｲﾄ点灯時間を10とする
				}
				tim1_mov = 1;												// ﾀｲﾏｰ1起動中 // 07-01-16追加 //
				break;
		case	2:
				if( Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA != 1){
					if( !WAR_MONEY_CHECK ){								// 現金を使用済み
						wk_MsgNo = Ope_Disp_Media_GetMsgNo( 0, 3, 0 );
						lcd_wmsg_dsp_elec( 1, OPE_CHR[wk_MsgNo], OPE_CHR[100], 1, 0, COLOR_RED, LCD_BLINK_OFF );
						dsp_fusoku = 1;									// ｱﾗｰﾑ消去用ﾌﾗｸﾞをｾｯﾄ(残高不足時のｱﾗｰﾑﾌﾗｸﾞと共用)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						Restart_Timer();											// Timer1をﾘｾｯﾄしてTimer8を起動	07-01-22追加
						Lagcan( OPETCBNO, 1 );										// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					}	
					else if(Suica_Rec.Data.BIT.PRI_NG){
						if( DspSts == LCD_WMSG_ON )								//エラーメッセージ表示中なら
							LagCan500ms(LAG500_ERROR_DISP_DELAY);				//タイマーキャンセル

						Lcd_WmsgDisp_ON( 1, COLOR_RED,  LCD_BLINK_OFF, OPE_CHR[136] );						// 「只今電子マネーは使用できません」
						
						LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 13, op_SuicaFusokuOff );	// 6sec後に消去
						
					
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						Restart_Timer();											// Timer1をﾘｾｯﾄしてTimer8を起動
						Lagcan( OPETCBNO, 1 );										// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					}
				}
				break;
		case	13:
		case	22:
				if( !WAR_MONEY_CHECK ){								// 現金を使用済み
					wk_MsgNo = Ope_Disp_Media_GetMsgNo( 0, 3, 0 );
					lcd_wmsg_dsp_elec( 1, OPE_CHR[wk_MsgNo], OPE_CHR[100], 1, 0, COLOR_RED, LCD_BLINK_OFF );
					dsp_fusoku = 1;									// ｱﾗｰﾑ消去用ﾌﾗｸﾞをｾｯﾄ(残高不足時のｱﾗｰﾑﾌﾗｸﾞと共用)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					Restart_Timer();												// Timer1をﾘｾｯﾄしてTimer8を起動	07-01-22追加
					Lagcan( OPETCBNO, 1 );										// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				}	
				break;
		default:
				break;
	}
	
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			決済結果データ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
///	@return			ret		：0固定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvSettData( uchar*	ope_faze )
{
	short ret = 0;
	
	if( (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 		// 決済結果ﾃﾞｰﾀを受信し
		(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) )		// 決済結果ﾃﾞｰﾀ未処理の場合
	{
		switch( OPECTL.Ope_mod ){
		case 2:											// 料金表示,入金処理フェーズ
			// 交通系ICカード「決済結果データ」電文を処理できるフェーズ
			// （ここでしか受信しないはず）
			break;
		default:	// 交通系ICカード「決済結果データ」電文を処理しない処理フェーズで受信した場合
			Settlement_rcv_faze_err((uchar*)&Settlement_Res, 1 );		// エラー登録処理
			Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA = 1;	// 決済結果データ処理済にする
			break;
		}
	}

	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				if( Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0 )			// 決済結果ﾃﾞｰﾀ受信済みで未処理の場合
				{
					Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA = 1;
					if( dsp_change ){													// 精算未了ﾒｯｾｰｼﾞが表示したままの場合
						op_SuicaFusokuOff();											// ﾒｯｾｰｼﾞの消去
						dsp_change = 0;
					}
					Suica_Ctrl( S_CNTL_DATA, 0x80 );													// 制御ﾃﾞｰﾀ（取引終了）を送信する
					
					if( Settlement_Res.Result == 0x02 ){								// 決済NG(使用不可ｶｰﾄﾞﾀｯﾁ時)受信時
						Lcd_WmsgDisp_ON2( 1, COLOR_RED,  LCD_BLINK_OFF, suica_dsp_buff );
						Lcd_WmsgDisp_ON( 1, COLOR_RED,  LCD_BLINK_OFF, OPE_CHR[89] );					// 
						ope_anm( AVM_CARD_ERR1 );							// ｱﾅｳﾝｽ「このカードは使えません」
						LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 12, op_SuicaFusokuOff );	// 6sec後に消去
						if( !Suica_Rec.Data.BIT.MIRYO_TIMEOUT )							// 処理未了後のタイムアウト処理中(以降取消のみ有効)でなければ
							Lagtim( OPETCBNO, 11, 1*10 );								// 受付可送信タイマースタート
					}else
					// 決済結果が残高不足応答の時
					if(Settlement_Res.Result == 0x03) {								
						if( OPECTL.op_faz == 8 && STATUS_DATA.StatusInfo.ReceptStatus ){// 電子媒体停止待ち合わせでSuicaの状態が受付不可状態
							Op_StopModuleWait_sub( 0 );									// Suica停止済み処理
							ret = 0;													// 精算中（未完了）
							break;
						}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						if( OPECTL.op_faz == 10 && STATUS_DATA.StatusInfo.ReceptStatus ){	// Edyでの精算開始でSuicaの状態が受付不可状態
//							OPECTL.op_faz = save_op_faz;								// 保存済みのﾌｪｰｽﾞを戻す
//							ret = Edy_WarDsp_and_Pay();									// Edyの減算指示送信
//						}
//						Restart_Timer();											// Timer1をﾘｾｯﾄしてTimer8を起動 07-01-22追加
						Lagcan( OPETCBNO, 1 );										// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

 
						if( !dsp_fusoku ){									// 残額不足表示がされていない
							#if (3 == AUTO_PAYMENT_PROGRAM)							// 試験用にカード残額をごまかす場合（ﾃｽﾄ用）
								if( 0L != (unsigned long)CPrmSS[S_SYS][5] )			// 設定あれば
									op_SuicaFusoku_Msg( (ulong)CPrmSS[S_SYS][5], suica_dsp_buff, 0 );	// 指定値に読み替える
								else
									op_SuicaFusoku_Msg( (ulong)Settlement_Res.settlement_data_after, suica_dsp_buff, 0 );
							#else
								op_SuicaFusoku_Msg( (ulong)Settlement_Res.settlement_data_after, suica_dsp_buff, 0 );
							#endif
							lcd_wmsg_dsp_elec( 1, suica_dsp_buff, OPE_CHR[89], 1, 1, COLOR_RED, LCD_BLINK_OFF );
							dsp_fusoku = 1;							// 残額不足ﾒｯｾｰｼﾞの編集
						}

						if( OPECTL.op_faz == 9 ) {										// 精算中止ﾎﾞﾀﾝ押下でSuica停止待ち合わせ時は
							OPECTL.op_faz = 1;											// ﾌｪｰｽﾞを元に戻しておく
							op_SuicaFusokuOff();										// 表示中のﾒｯｾｰｼﾞを消去
							Op_Cansel_Wait_sub( 0 );									// Suica停止済み処理

						}else{
							mode_Lagtim10 = 1;											// Lagtimer10の使用状態 1:Suica停止後の受付許可再送信Wait終了
							Lagtim( OPETCBNO, 10, 1*10 );								// Suica停止後の受付許可送信待ちﾀｲﾏｰ200mswait(Timer10)
						}
						break;
					}
					// 決済結果が完了応答の時（残高不足後の再タッチ時を含む）
					else if(Settlement_Res.Result == 0x01) {
						Lagcan( OPETCBNO, 8 );									// ﾀｲﾏｰ8ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用) 07-01-22追加
						Lagcan( OPETCBNO, 1 );									// ﾀｲﾏｰ10ﾘｾｯﾄ
						Lagcan( OPETCBNO, 10 );									// ﾀｲﾏｰ10ﾘｾｯﾄ
						nyukin_delay_check( nyukin_delay, delay_count);					// 保留中のｲﾍﾞﾝﾄがあれば再postする
						e_inflg = 1;													// Suica決済である
						e_pay = Settlement_Res.settlement_data;							// Suica決済額を保存
						e_zandaka = Settlement_Res.settlement_data_after;				// Suica残高を保存
						if( Is_SUICA_STYLE_OLD ){										// 旧版動作
							wk_media_Type = Ope_Disp_Media_Getsub(1);					// 画面表示用　精算媒体種別 取得
							if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		
								ElectronSet_PayData( &Settlement_Res, PASMO_USED );		// PASMOで精算
							} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {
								ElectronSet_PayData( &Settlement_Res, SUICA_USED );		// Suicaで精算
							} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICOCA) {
								ElectronSet_PayData( &Settlement_Res, ICOCA_USED );		// ICOCAで精算
							} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICCARD) {
								ElectronSet_PayData( &Settlement_Res, ICCARD_USED );	// IDCARDで精算
							}
						}																// 旧版動作(e)
						else{															// 新版動作
							ElectronSet_PayData( &Settlement_Res, SUICA_USED );		// Suicaで精算
						}
						ret = al_emony( SUICA_EVT , 0 );								// 電子マネー精算処理
						if( ryo_buf.dsp_ryo ){
							Lagtim( OPETCBNO, 11, 1*10 );								// Suica精算ｶｰﾄﾞ認証ｴﾗｰ動作ﾀｲﾏｰ200mswait(Timer11)
							if( OPECTL.op_faz == 3 ){									// 取消し中の場合
								Op_Cansel_Wait_sub( 0 );								// Suica停止済み処理実行
							}
						}
						// 決済完了時はin_mony()にて精算完了時の処理が行われる
					}

					else {
						// 決済結果は01or03以外はない
					}
				}
				break;
		case	3:
				break;
	}
	
	return ret;	
}

// MH321800(S) T.Nagai ICクレジット対応
////[]----------------------------------------------------------------------[]
/////	@brief			異常データ処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
/////	@return			ret		：0固定
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 08/07/15<br>
/////					Update
////[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
//short	RecvErrData( uchar*	ope_faze )
//{
//	short ret = 0;
//	
//	switch( *ope_faze ){
//		case	1:
//				break;
//		case	2:
//				break;
//		case	3:
//				break;
//	}
//	
//	return ret;	
//}
// MH321800(E) T.Nagai ICクレジット対応
// MH810103 GG119202(S) SX-20処理修正
//[]----------------------------------------------------------------------[]
///	@brief			異常データ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
///	@return			ret		：0固定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvErrData( uchar*	ope_faze )
{
	short ret = 0;
	
	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				break;
		case	3:
				break;
	}
	
	return ret;	
}
// MH810103 GG119202(E) SX-20処理修正

//[]----------------------------------------------------------------------[]
///	@brief			状態データ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
///	@return			ret		：0固定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvStatusData( uchar*	ope_faze )
{
	short ret = 0;
	
	Suica_Status_Chg();
	switch( *ope_faze ){
		case	0:
				if( Suica_Rec.Data.BIT.CTRL == 0 && STATUS_DATA.StatusInfo.ReceptStatus ){
					if( OPECTL.op_faz == 8 ){ 										// 電子媒体停止処理中
						Op_StopModuleWait_sub( 0 );									// Suica停止済み処理
					} 
					Lagtim( OPETCBNO,11, 1*10 );									// 受付可送信ﾀｲﾏｰ起動(Timer11)
				}
				break;
		case	2:
				if(!Suica_Rec.Data.BIT.CTRL_MIRYO) {								// 精算未了状態で無い場合のみ処理を実施
					if(Suica_Rec.Data.BIT.CTRL) {									// 制御ﾃﾞｰﾀが可状態の時は
						if( OPECTL.op_faz == 9 || STATUS_DATA.StatusInfo.Running ) {// 精算中止時のSuica停止待ち合わせ時(正常に停止していない場合)か受信ステータスが実行中
							break;
						}
						if( OPECTL.op_faz == 8 || OPECTL.op_faz == 3 ) {			// 電子媒体停止待ち合わせ中？
							// 電子媒体停止待ち合わせ中での受付可受信時は、ﾌｪｰｽﾞ合わせの為に受付不可を送信する。
							Suica_Ctrl( S_CNTL_DATA, 0 );										// Suica利用を不可にする
							break;
						}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						if( OPECTL.op_faz == 10 ) {									// Edyでの精算開始でSuica停止待ち合わせ時か
//							break;
//						}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

						if( OPECTL.op_faz != 3 && WAR_MONEY_CHECK && !e_incnt){		// 取り消しﾎﾞﾀﾝ押下されてた場合・現金投入後・Suica使用後以外の
																					// 場合は、商品選択ﾃﾞｰﾀ送信する。
							if(w_settlement != 0) {									// 残高不足時の引落し時
								Suica_Data_Snd( S_SELECT_DATA, &w_settlement);		// Suica残高（10円未満切捨て）を商品選択ﾃﾞｰﾀとして送信する
							} else {												// 通常時
								Suica_Ctrl( S_SELECT_DATA, 0 );						// 駐車料金を商品選択ﾃﾞｰﾀとして送信する
								w_settlement = 0;									// 商品選択データ編集領域をｸﾘｱする
							}
						}
					}
					else {															// 制御ﾃﾞｰﾀが不可状態の時で
						if( STATUS_DATA.StatusInfo.ReceptStatus &&					// 受付不可データ及び
							STATUS_DATA.StatusInfo.TradeCansel ) {					// 取引ｷｬﾝｾﾙ受付データを受信時
							if( dsp_change ){
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//								if( !e_incnt && !SUICA_CM_BV_RD_STOP ){				// 電子決済未実施の場合かつ未了後精算可能設定の場合
//									Edy_SndData01();								// ｶｰﾄﾞ検知停止指示送信
//								}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
								op_SuicaFusokuOff();								// 精算未了時の反転ﾒｯｾｰｼﾞ表示を消去する
								dsp_change = 0;
								miryo_timeout_after_mif( 0 );
								nyukin_delay_check( nyukin_delay, delay_count);		// 保留中のｲﾍﾞﾝﾄがあれば再postする
								miryo_timeout_after_disp();							// 未了タイムアウト後の表示を行う
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//								Restart_Timer();									// Timer1をﾘｾｯﾄしてTimer8を起動	07-01-22追加
								Lagcan( OPETCBNO, 1 );								// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
								Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
							}
							Suica_Ctrl( S_CNTL_DATA, 0x80 );										// 制御ﾃﾞｰﾀ（取引終了）を送信
							if( !time_out )											// 初期画面(料金画面)にてﾀｲﾑｱｳﾄ発生時
								Lagtim( OPETCBNO, 11, 1*10 );						// Suica精算ｶｰﾄﾞ認証ｴﾗｰ動作ﾀｲﾏｰ200mswait(Timer11)
						}
						
						if( OPECTL.op_faz == 3 || OPECTL.op_faz == 9 || time_out ) {// 精算中止ﾎﾞﾀﾝ押下でSuica停止待ち合わせ時は
							Op_Cansel_Wait_sub( 0 );								// Suica停止済みとして停止判断を行う
							break;
						}
						
						if( OPECTL.op_faz == 8 ) {									// 電子媒体停止待ち合わせ時は
							if( Op_StopModuleWait_sub( 0 ) == 1 ){					// Suica停止済みとして停止判断を行う
								ret = 0;
								break;
							}
						}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						if( OPECTL.op_faz == 10 ) {									// Edyでの精算開始でSuica停止待ち合わせ時は
//							OPECTL.op_faz = save_op_faz;							// 精算ﾌｪｰｽﾞを元に戻す
//							ret = Edy_WarDsp_and_Pay();								// Edyの精算処理を実行する。
//							break;
//						}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

						if( STATUS_DATA.StatusInfo.ReceptStatus && 					// Suicaﾘｰﾀﾞｰが受付不可状態となった場合で
							!Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA ){	// 決済結果データを未処理の場合は、再活性化処理のタイマーをスタート
																					// 受付可電文を送信するかどうかはタイムアウト先で判定する
							if( !time_out )											// 初期画面(料金画面)にてﾀｲﾑｱｳﾄ発生時
								Lagtim( OPETCBNO, 11, 1*10 );						// 受付可送信タイマースタート
						}
					}
				}else{
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						Edy_StopAndLedOff();										// ｶｰﾄﾞ検知停止＆UI LED消灯指示送信
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
						if( MIFARE_CARD_DoesUse  ){									// Mifareが有効な場合
							LagCan500ms( LAG500_MIF_LED_ONOFF );				// Mifare再有効化ﾀｲﾏｰｽﾄｯﾌﾟ
							op_MifareStop_with_LED();								// Mifare無効
						}
						if( First_Pay_Dsp() ) 					         			// 初期画面(料金表示)
							Lagcan( OPETCBNO, 1 );									// ﾀｲﾏｰ1ﾘｾｯﾄ
						else
							Lagcan( OPETCBNO, 8 );									// ﾀｲﾏｰ8ﾘｾｯﾄ
						Lagcan( OPETCBNO, 10 );										// ﾀｲﾏｰ10ﾘｾｯﾄ
						dsp_change = 1;												//  未了ｱﾗｰﾑ表示中ﾌﾗｸﾞｾｯﾄ
						lcd_wmsg_dsp_elec( 1, OPE_CHR[91], OPE_CHR[92], 0, 0, COLOR_RED, LCD_BLINK_OFF );
						if( dsp_fusoku )											//  不足アラーム表示中？
					    	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );		//  不足アラーム消去ﾀｲﾏｰを止める
				}
				break;
		case	3:
		case	14:
		case	23:
				Suica_Ctrl( S_CNTL_DATA, 0x80 );									// 制御ﾃﾞｰﾀ（取引終了）を送信する
				break;
	}

	
	if( STATUS_DATA.StatusInfo.DetectionSwitch ){									// 締め検知及び
		Suica_Data_Snd( S_PAY_DATA,&CLK_REC);										// SX-10締め処理実行
	}
	return ret;	
}

// MH321800(S) T.Nagai ICクレジット対応
////[]----------------------------------------------------------------------[]
/////	@brief			時刻同期データ処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
/////	@return			ret		：0固定
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 08/07/15<br>
/////					Update
////[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
//short	RecvTimeData( uchar*	ope_faze )
//{
//	short ret = 0;
//	
//	switch( *ope_faze ){
//		case	1:
//				break;
//		case	2:
//				break;
//		case	3:
//				break;
//	}
//	
//	return ret;	
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			固定データ受信処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
/////	@return			ret		：0固定
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 08/07/15<br>
/////					Update
////[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
//short	RecvFixData( uchar*	ope_faze ){
//	
//	short ret = 0;
//	
//	switch( *ope_faze ){
//		case	1:
//				break;
//		case	2:
//				break;
//		case	3:
//				break;
//	}
//	
//	return ret;	
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			印字依頼データ（ポイント）データ受信処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
/////	@return			ret		：0固定
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 08/07/15<br>
/////					Update
////[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
//short	RecvPointData( uchar*	ope_faze ){
//	
//	short ret = 0;
//	
//	switch( *ope_faze ){
//		case	1:
//				break;
//		case	2:
//				break;
//		case	3:
//				break;
//	}
//	
//	return ret;
//	
//}
// MH321800(E) T.Nagai ICクレジット対応
// MH810103 GG119202(S) SX-20処理修正
//[]----------------------------------------------------------------------[]
///	@brief			時刻同期データ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
///	@return			ret		：0固定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvTimeData( uchar*	ope_faze )
{
	short ret = 0;
	
	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				break;
		case	3:
				break;
	}
	
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			固定データ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
///	@return			ret		：0固定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvFixData( uchar*	ope_faze ){
	
	short ret = 0;
	
	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				break;
		case	3:
				break;
	}
	
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			印字依頼データ（ポイント）データ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze ：現在のOpeﾌｪｰｽﾞ
///	@return			ret		：0固定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvPointData( uchar*	ope_faze ){
	
	short ret = 0;
	
	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				break;
		case	3:
				break;
	}
	
	return ret;
	
}
// MH810103 GG119202(E) SX-20処理修正

/*[]----------------------------------------------------------------------[]*/
/*| 電子媒体終了処理実行					                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Cansel_Ele_Start	                                       |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*| 備考		 : 取消ｷｰやﾀｲﾏｰ等で精算画面から復帰する場合に電子媒体を停止|*/
/*|				 : させるための処理。									   |*/
/*|				 : 移植元のGT4700には無いがUTは精算ﾌｪｰｽﾞやﾀｲﾏｰ自動終了等が |*/
/*|				 : あるため本関数にて共通化する							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-12-14                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void Cansel_Ele_Start( void )
{

// MH810103 GG119202(S) 制御データを連続で送信してしまう
//	if(( Suica_Rec.Data.BIT.CTRL || Suica_Rec.Data.BIT.OPE_CTRL )&& !SUICA_USE_ERR ) {			// 制御ﾃﾞｰﾀが可状態の時またはエラーが発生している場合は
// MH810104(S) MH321800(S) 直取中にとりけしすると画面が固まる
//	if (((!Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL) ||	// 受付許可送信中
//		 (Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL)) &&	// 受付禁止送信中
	// 既に受付不可状態であれば、停止状態とする
	if ((Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL) &&	// 受付禁止送信中
// MH810104(E) MH321800(E) 直取中にとりけしすると画面が固まる
		!SUICA_USE_ERR) {												// エラー未発生
		// 受付許可／禁止送信中は状態データ受信を待つ
		// １．受付許可送信中の場合はEcRecvStatusData()で
		// 　　状態データ（受付可）受信すると制御データ（受付禁止）を送信する
		// ２．受付禁止送信中の場合はEcRecvStatusData()で
		// 　　状態データ（受付不可）を受信するとOp_Cansel_Wait_sub(0)を
		// 　　実行する
		Lagtim( OPETCBNO, 2, 10*50 );							// ﾀｲﾏｰ2ｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)(10S)
	}
	else if (Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR) {		// 受付可状態、かつ、エラー未発生
// MH810103 GG119202(E) 制御データを連続で送信してしまう
		Suica_Ctrl( S_CNTL_DATA, 0 );										// Suica利用を不可にする
		Lagtim( OPETCBNO, 2, 10*50 );							// ﾀｲﾏｰ2ｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)(10S)
	}else{														// Suicaがすでに停止済みなら
		cansel_status.BIT.SUICA_END = 1;						// 停止状態とする
	}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ){				// Edy設定が有効の場合、Edyの停止待ちの採集タイマーとして以下実施
//		LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 120, Op_ReturnLastTimer );// 残額表示用ﾀｲﾏｰを一時的に使用する(60秒)
//	}
//
//	if( !EDY_USE_ERR )											// Edy関連のエラー未発生
//	{
//		Edy_StopAndLedOff();									// ｶｰﾄﾞ検知停止＆UI LED消灯指示送信
//		LagCan500ms( LAG500_EDY_LED_RESET_TIMER );				// 延長ﾀｲﾏを停止する
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	cansel_status.BIT.INITIALIZE = 1;							// 電子媒体の停止処理をスタート
	Op_Cansel_Wait_sub( 0xff );									// 全デバイスの終了をチェックする
	
	Suica_Rec.Data.BIT.ADJUSTOR_START = 0;						// 精算中止時には商品選択データ送信可否フラグをクリアする
}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define Suica_CMN_CANSEL_BYTE 0x07								// Suica及びｺｲﾝﾒｯｸが停止済み
//#define EDY_CMN_CANSEL_BYTE 0x0b								// Edy及びｺｲﾝﾒｯｸが停止済み
//#define Suica_EDY_CANSEL_BYTE 0x0d								// Suica及びｺｲﾝﾒｯｸが停止済み
//#define ALL_CANSEL_BYTE 0x0f									// 全ての媒体が停止済み
#define Suica_CANSEL_BYTE	0x05								// Suica及びｺｲﾝﾒｯｸが停止済み
#define CMN_CANSEL_BYTE		0x03								// Edy及びｺｲﾝﾒｯｸが停止済み
#define ALL_CANSEL_BYTE		0x07								// 全ての媒体が停止済み
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

//[]----------------------------------------------------------------------[]
///	@brief			Op_Cansel_Wait_sub
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: イベント種別
///	@return			ret		: 精算処理終了判定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/25<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Op_Cansel_Wait_sub( uchar kind )
{
	uchar ret = 0;

	switch( kind ){
		case 0:															// Suicaイベント
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( cansel_status.BYTE == EDY_CMN_CANSEL_BYTE ){			// 終了ｽﾃｰﾀｽ：Edy&ｺｲﾝﾒｯｸ終了済みの場合
//				ret = 1;												// 精算処理を終了する
//			}else{														// Edy及びﾒｯｸがまだ終了していない場合
//				if( EDY_USE_ERR ){										// Edyが使用不可かｴﾗｰが発生している場合は
//					cansel_status.BIT.EDY_END = 1;						// Suica終了済みﾌﾗｸﾞをｾｯﾄ
//					if( cansel_status.BYTE == EDY_CMN_CANSEL_BYTE ){	// Edy&ｺｲﾝﾒｯｸ終了済み
//						ret = 1;										// 精算処理を終了する
//					}
//				}
//			}
			if( cansel_status.BYTE == CMN_CANSEL_BYTE ){				// 終了ｽﾃｰﾀｽ：ｺｲﾝﾒｯｸ終了済みの場合
				ret = 1;												// 精算処理を終了する
			}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			cansel_status.BIT.SUICA_END = 1;							// Suica終了済みﾌﾗｸﾞをｾｯﾄ
			break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case 1:															// Edyイベント
//			if( cansel_status.BYTE == Suica_CMN_CANSEL_BYTE ){			// 終了ｽﾃｰﾀｽ：Suica&ﾒｯｸ終了済みの場合
//				ret = 1;												// 精算処理を終了する
//			}else{														// Suica及びﾒｯｸがまだ終了していない場合
//				if( SUICA_USE_ERR ){									// suicaが使用不可かｴﾗｰが発生している場合は
//					cansel_status.BIT.SUICA_END = 1;					// Suica終了済みﾌﾗｸﾞをｾｯﾄ
//					if( cansel_status.BYTE == Suica_CMN_CANSEL_BYTE ){	// Suica&ｺｲﾝﾒｯｸ終了済み
//						ret = 1;										// 精算処理を終了する
//					}
//				}
//			}
//			cansel_status.BIT.EDY_END = 1;								// Edy終了済みﾌﾗｸﾞをｾｯﾄ
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		case 2:															// ｺｲﾝﾒｯｸｲﾍﾞﾝﾄ
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( cansel_status.BYTE == Suica_EDY_CANSEL_BYTE ){			// 終了ｽﾃｰﾀｽ：Suica＆Edy終了済みの場合
//				ret = 1;												// 精算処理を終了する
//			}else{														// Suica及びEdyがまだ終了していない場合
//				if( SUICA_USE_ERR ){									// Suica関連のｴﾗｰが発生している場合
//					cansel_status.BIT.SUICA_END = 1;					// Suica終了済みﾌﾗｸﾞをｾｯﾄ
//					if( cansel_status.BYTE == Suica_EDY_CANSEL_BYTE ){	// Suica&Edy終了済み？
//						ret = 1;										// 精算処理を終了する
//					}
//				}
//				if( EDY_USE_ERR ){										// Edy関連ｴﾗｰが発生している場合
//					cansel_status.BIT.EDY_END = 1;						// Edy終了済みﾌﾗｸﾞをｾｯﾄ
//					if( cansel_status.BYTE == Suica_EDY_CANSEL_BYTE ){	// Suica&Edy終了済み？
//						ret = 1;										// 精算処理を終了する
//					}
//				}
//			}
			if( cansel_status.BYTE == Suica_CANSEL_BYTE ){				// 終了ｽﾃｰﾀｽ：Suica終了済みの場合
				ret = 1;												// 精算処理を終了する
			}else{														// Suicaがまだ終了していない場合
				if( SUICA_USE_ERR ){									// Suica関連のｴﾗｰが発生している場合
					cansel_status.BIT.SUICA_END = 1;					// Suica終了済みﾌﾗｸﾞをｾｯﾄ
					ret = 1;											// 精算処理を終了する
				}
			}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			cansel_status.BIT.CMN_END = 1;								// ﾒｯｸ終了済みﾌﾗｸﾞをｾｯﾄ
			break;
		case 0xff:														// 全てのデバイスが完了しているかどうかを判定する
			if( cansel_status.BYTE == ALL_CANSEL_BYTE ){				// すでにすべてのデバイスが停止済み
				ret = 1;												// 精算処理を終了する
			}
			break;	
		default:
			break;
	}
	
	if( ret == 1 && ryo_buf.zankin ){									// 
		queset( OPETCBNO, ELE_EVENT_CANSEL, 0, NULL );					// オペに電子媒体の停止を通知					
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica_fukuden_que_check	 
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/29<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	Suica_fukuden_que_check( void )
{
	uchar *pData;
	uchar i,bc;
	ushort	cnt;
	ushort	w_counter[3];

	if( !suica_rcv_que.count )											// 未処理データが無い場合は抜ける
		return;
		
	memcpy( w_counter, &suica_rcv_que.readpt, sizeof( w_counter ) );	// offset関連をワークに退避
	
	for( ; w_counter[2]!=0; --w_counter[2] ){							// 未処理データ分ループ
		pData = (uchar*)&suica_rcv_que.rcvdata[w_counter[0]];			// 内部処理用にワーク領域に展開
		
		for( i=0; (i<6 && *pData != ack_tbl[i]); i++);					// 受信種別を解析
		
	    if( i==1 ){														// ACK2 データ受信 ? 
	    	pData+=2;													// ポインタ位置を補正
// MH321800(S) G.So ICクレジット対応
			if (isEC_USE() != 0) {
			// 決済リーダ
				if (*pData == S_SETTLEMENT_DATA) {						// 決済結果データ
					EcSettlementPhaseError((pData+1), 5);				// エラーデータ登録実行
				}
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
				else if(*pData == S_RECEIVE_DEEMED)						// 決済直前取引データ
				{
					// ac_flg.cycl_fgが57,58である場合はすでにfukuden()で処理済のはず
					// また、ac_flg.ec_recv_deemed_fgがONの場合もすでにfukuden()で処理済のはず
					// ※ac_flg.cycl_fgが57,58ではなく、ac_flg.ec_recv_deemed_fg=0であり、
					// suica_rcv_que.rcvdataに直前取引データがセットされている状態での復電時のみここで処理する。
					// EcRecvDeemedData.WFlag = 0の場合、復電にて直前取引データがまだログ登録、印字処理されていないのでここで処理する
					if( EcRecvDeemedData.WFlag == 0 ){
						if( EcRecvDeemed_DataSet(pData+1) ){
							// 直前取引データをセットしたらログ登録、印字処理をする
							EcRecvDeemed_RegistPri();
						}
					}
				}
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
			} else {
// MH321800(E) G.So ICクレジット対応
			if( *pData == S_SETTLEMENT_DATA && *(pData+1) == 1 ){		// 決済結果データでデータの内容が有効の場合
				Settlement_rcv_faze_err( (pData+1), 5 );				// エラーデータ登録実行
			}
// MH321800(S) G.So ICクレジット対応
			}
// MH321800(E) G.So ICクレジット対応
	    }else if( (i==2 )||(i==0) ){ 									// ACK3 or ACK1 データ受信?
	    	bc = *(pData+1);											// 受信データバイト数を設定
	    	pData+=2;													// ポインタ位置を補正
			for( cnt=0 ;cnt < bc; ){									// 全データを解析するまでループ
				cnt += (*pData + 1);									// 各データ毎のサイズを加算
				pData++;												// ポインタ位置を補正
// MH321800(S) G.So ICクレジット対応
				if (isEC_USE() != 0) {
				// 決済リーダ
					if (*pData == S_SETTLEMENT_DATA) {					// 決済結果データ
						EcSettlementPhaseError((pData+1), 5);			// エラーデータ登録実行
					}
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
					else if(*pData == S_RECEIVE_DEEMED)					// 決済直前取引データ
					{
						// ac_flg.cycl_fgが57,58である場合はすでにfukuden()で処理済のはず
						// また、ac_flg.ec_recv_deemed_fgがONの場合もすでにfukuden()で処理済のはず
						// ※ac_flg.cycl_fgが57,58ではなく、ac_flg.ec_recv_deemed_fg=0であり、
						// suica_rcv_que.rcvdataに直前取引データがセットされている状態での復電時のみここで処理する。
						// EcRecvDeemedData.WFlag = 0の場合、復電にて直前取引データがまだログ登録、印字処理されていないのでここで処理する
						if( EcRecvDeemedData.WFlag == 0 ){
							if( EcRecvDeemed_DataSet(pData+1) ){
								// 直前取引データをセットしたらログ登録、印字処理をする
								EcRecvDeemed_RegistPri();
							}
						}
					}
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
				} else {
// MH321800(E) G.So ICクレジット対応
				if( *pData == S_SETTLEMENT_DATA && *(pData+1) == 1 ){	// 決済結果データでデータの内容が有効の場合
					Settlement_rcv_faze_err( (pData+1), 5 );			// エラーデータ登録実行
				}
// MH321800(S) G.So ICクレジット対応
				}
// MH321800(E) G.So ICクレジット対応
				pData += *(pData-1);									// ポインタ位置を補正
			}
	    }
		
		if( ++w_counter[0] >= SUICA_QUE_MAX_COUNT ){					// リードポインタがキューの最後の場合
			w_counter[0] = 0;											// 先頭へ戻す
		}

		nmisave( &suica_rcv_que.readpt, w_counter, sizeof(w_counter) );	// 本関数処理中に再度停電した場合に備えてoffset関連を更新
	}
	memset( &suica_rcv_que.readpt, 0, sizeof( w_counter ));				// 処理完了でoffsetを初期化
}

//[]----------------------------------------------------------------------[]
///	@brief			未了タイムアウト後の画面表示処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/08/22<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void miryo_timeout_after_disp( void )
{
	uchar timer=0;
	const uchar *msg;
// MH810103 GG119202(S) みなし決済扱い時の動作
//// GG116200(S) G.So ICクレジット対応
//	static const
//	uchar msg_idx[4] = {49, 50,							// 1ページ目
//						51, 52};						// 2ページ目
//// GG116200(E) G.So ICクレジット対応
// MH810103 GG119202(E) みなし決済扱い時の動作

// MH810103 GG118808_GG118908(S) 現金投入と同時にカードタッチ→未了発生でロックする
	if (isEC_USE() != 0 && ryo_buf.zankin == 0) {
		// 残金＝0であれば、表示しない
		return;
	}
// MH810103 GG118808_GG118908(E) 現金投入と同時にカードタッチ→未了発生でロックする

	if( SUICA_MIRYO_AFTER_STOP ){						// 処理未了後はSuicaを使用不可設定とする
		Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;			// 未了タイムアウト発生
		mode_Lagtim10 = 0;								// Lagtimer10の使用状態 0:利用可能媒体ｻｲｸﾘｯｸ表示再開
		if( SUICA_ONLY_STOP ){							//	Suicaのみを使用不可とする設定の場合
			msg = ERR_CHR[18];
			timer = 1;
		}else{											//  Suica・CM・BV・磁気ﾘｰﾀﾞｰを使用不可とする設定の場合
			msg = ERR_CHR[22];
		}
		
// MH321800(S) G.So ICクレジット対応
//		lcd_wmsg_dsp_elec( 1, ERR_CHR[20], msg, timer, 0, COLOR_RED, LCD_BLINK_OFF );
		if (isEC_USE() != 0) {
// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH810103 GG118809_GG118909(S) WAONの未了残高照会で未引き去りを確認した場合はとりけしボタン押下待ちとする
////			if(Ec_Settlement_Res.brand_no == BRANDNO_WAON) {
//			if(Ec_Settlement_Res.brand_no == BRANDNO_WAON && (ECCTL.ec_Disp_No != 96)) {
//// MH810103 GG118809_GG118909(E) WAONの未了残高照会で未引き去りを確認した場合はとりけしボタン押下待ちとする
//			// 決済リーダでWAONの時
//// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
////				Ope2_ErrChrCyclicDispStart(2000, msg_idx);	// ワーニングサイクリック表示(2秒間隔)
//				Ope2_ErrChrCyclicDispStart(2000, msg_idx, 0);	// ワーニングサイクリック表示(2秒間隔)
//// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
//				LagTim500ms(LAG500_EC_AUTO_CANCEL_TIMER, (short)((30 * 2) + 1), ec_auto_cancel_timeout);	// アラーム取引後の精算自動キャンセルタイマ開始(500ms)
//// MH810105 GG119202(S) iD決済時の案内表示対応
//			} else if (Ec_Settlement_Res.brand_no == BRANDNO_ID &&
//					   ECCTL.ec_Disp_No == 90){
			if (Ec_Settlement_Res.brand_no == BRANDNO_ID &&
				ECCTL.ec_Disp_No == 90){
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
			// 決済リーダでiDの時
				lcd_wmsg_dsp_elec( 1, ERR_CHR[61], ERR_CHR[59], timer, 0, COLOR_RED, LCD_BLINK_OFF );
// MH810105 GG119202(E) iD決済時の案内表示対応
			} else {
				lcd_wmsg_dsp_elec( 1, ERR_CHR[20], msg, timer, 0, COLOR_RED, LCD_BLINK_OFF );
			}
		} else {
			lcd_wmsg_dsp_elec( 1, ERR_CHR[20], msg, timer, 0, COLOR_RED, LCD_BLINK_OFF );
		}
// MH321800(E) G.So ICクレジット対応
		
		Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 1;			// ｱﾗｰﾑﾒｯｾｰｼﾞ表示用ﾌﾗｸﾞｾｯﾄ
		// 処理未了タイムアウト後は、商品選択データ送信可否フラグを落とす。
		Suica_Rec.Data.BIT.ADJUSTOR_START = 0;			// 商品選択データ送信可否フラグクリア
	}
// MH321800(S) Y.Tanizaki ICクレジット対応
//	if(!SUICA_CM_BV_RD_STOP && OPECTL.op_faz != 3 )
//		Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰｽﾀ	
	if(!SUICA_CM_BV_RD_STOP && OPECTL.op_faz != 3 ) {
		if(isEC_USE()) {
			Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰｽﾀ	
		} else {
			Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰｽﾀ	
		}
	}
// MH321800(E) Y.Tanizaki ICクレジット対応
}

//[]----------------------------------------------------------------------[]
///	@brief			引去り未了中の判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		：0：未了中でない<br>
///							  1：未了処理中orSuicaﾀｲﾑｱｳﾄ後の使用不可設定
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/08/22<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Miryo_Exec_check( void )
{
	uchar ret=0;

// MH321800(S) D.Inaba ICクレジット対応
//	if( Suica_Rec.Data.BIT.CTRL_MIRYO ||
//		(Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
//		Edy_Rec.edy_status.BIT.CTRL_MIRYO )
	// 未了残高照会中？
	if( Suica_Rec.Data.BIT.CTRL_MIRYO && !Suica_Rec.Data.BIT.MIRYO_CONFIRM ||
		(Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP))
// MH321800(E) D.Inaba ICクレジット対応

		ret = 1;
		
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			入金金額算出処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0:No balance 1:In the balance
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/09/11<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char in_credit_check( void )
{
	char	i;
	long	crdt, wk;

	crdt = 0l;
	for( i = 0; i < 4; i++ ){
		wk = ( (long)ryo_buf.in_coin[i] );
		crdt += ( (long)( wk * coin_vl[i] ) );
	}
	crdt += ( (long)( ryo_buf.in_coin[4] * 1000l ) );
	
	i = 0;
	if( crdt < ryo_buf.dsp_ryo )
		i = 1;

	return( i );	
}

//[]----------------------------------------------------------------------[]
///	@brief			反転表示処理
//[]----------------------------------------------------------------------[]
///	@param[in]		type ：0：通常表示 1：反転表示
///	@param[in]		msg1 ：表示文字列(7行目)	
///	@param[in]		msg2 ：表示文字列(8行目)
///	@param[in]		timer：0：消去ﾀｲﾏｰ使用しない 1：消去ﾀｲﾏｰ使用する(6秒)
///	@param[in]		ann  ：0：音声案内しない 1：音声案内使用する(使用不可)
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/09/19<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void lcd_wmsg_dsp_elec( char type, const uchar *msg1, const uchar *msg2, uchar timer, uchar ann, ushort color, ushort blink )
{
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_削除
// ﾎﾟｯﾌﾟｱｯﾌﾟ表示要求 ⇒ LCDに送信してLCDで表示する(lcdbm_notice_dsp)
//	if( DspSts == LCD_WMSG_ON )								//エラーメッセージ表示中なら
//		LagCan500ms(LAG500_ERROR_DISP_DELAY);				//タイマーキャンセル
//
//	Lcd_WmsgDisp_ON2( (ushort)type, color, blink, msg1 );
//	Lcd_WmsgDisp_ON( (ushort)type, color, blink, msg2 );
//	
//	if( timer )
//		LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 13, op_SuicaFusokuOff );	// 6sec後に消去
//	
//	if( ann ){
//	}	
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_削除
}

//[]----------------------------------------------------------------------[]
///	@brief			未了タイムアウト後の精算復帰処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/01/14<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void miryo_timeout_after_proc( void )
{
	if( ryo_buf.zankin ){						// 未了解除後に残金がある場合は
		if( (( !pas_kep && !RD_Credit_kep ) || 
			(( RED_stat[2] & TICKET_MOUTH ) ==  TICKET_MOUTH ) ))	/* 券が挿入口にある */
			read_sht_opn();					/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｵｰﾌﾟﾝ */
		cn_stat( 3, 2 );					// 紙幣 & COIN再入金可
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			未了タイムアウト後のmifare復帰処理
//[]----------------------------------------------------------------------[]
///	@param[in]		type: 0：Suica / 1：Edy
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/01/14<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void miryo_timeout_after_mif( uchar type )
{
	uchar ret = 0;
	
	if( MIFARE_CARD_DoesUse &&					// Mifareが有効
		OPECTL.op_faz != 8 ){					// 電子媒体停止要求中でない
		if( type ){								// Edyからのコール
			ret = 1;							// Mifareの再活性化OK
		}else{
			if( !SUICA_CM_BV_RD_STOP )			// 未了後、Suica・CM・BV・RDが使用不可設定以外
				ret = 1;						// Mifareの再活性化OK
		}
		if( ret )
			op_MifareStart();					// Mifare再活性化
		
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			エラー表示・印字の文字列変換処理
//[]----------------------------------------------------------------------[]
///	@param[in]		ItemNum: エラー文字列要素番号
///	@return			wkErrNum：変換後の文字列要素番号
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/01/14<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
ushort SuicaErrCharChange( ushort ItemNum )
{
	ushort wkErrNum = 0;
	ushort media_Type = 0;
	
	wkErrNum = ItemNum - 150;							// 0=通信不良、1=異常、2=閉局

	media_Type = Ope_Disp_Media_Getsub(0);				// 画面表示用 精算媒体種別 取得
	if( !Is_SUICA_STYLE_OLD ){							// 新版動作設定
		media_Type = OPE_DISP_MEDIA_TYPE_eMONEY;		// "電子マネーリーダー..." に固定
	}
	switch(media_Type) {
	case OPE_DISP_MEDIA_TYPE_SUICA:
		wkErrNum += 150;
		break;											// "Suicaリーダー..."表示
	case OPE_DISP_MEDIA_TYPE_PASMO:
		wkErrNum += 168;								// "PASMOリーダー..."表示
		break;
	case OPE_DISP_MEDIA_TYPE_ICOCA:
		wkErrNum += 171;								// "ICOCAリーダー..."表示
		break;
	case OPE_DISP_MEDIA_TYPE_ICCARD:
		wkErrNum += 174;								// "ICカードリーダー..."表示
		break;
	case OPE_DISP_MEDIA_TYPE_eMONEY:
		wkErrNum += 177;								// "電子マネーリーダー..."表示
		break;
	default:
		break;
	}
	return wkErrNum;	
}

//[]----------------------------------------------------------------------[]
///	@brief			制御データを纏める
//[]----------------------------------------------------------------------[]
///	@param[in]		type: 0：受付不可 / 1：受付可 / 0x80：取引終了
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/02/17<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void Suica_Ctrl( ushort req, uchar type )
{
	uchar ctrl_work = type;

// MH321800(S) G.So ICクレジット対応
	if (isEC_USE() != 0) {
		Ec_Pri_Data_Snd(req, type);
		return;
	}
// MH321800(E) G.So ICクレジット対応
	
	switch( req ){
		case S_CNTL_DATA:
			Suica_Data_Snd( S_CNTL_DATA,&ctrl_work );			// 制御ﾃﾞｰﾀ可送信
			break;
		case S_SELECT_DATA:
			if(e_incnt <= 0) {
				w_settlement = ryo_buf.dsp_ryo;
				if( w_settlement > PAY_RYO_MAX){
					w_settlement = 	PAY_RYO_MAX;
				}
				Suica_Data_Snd( S_SELECT_DATA, &w_settlement);		// 駐車料金を商品選択ﾃﾞｰﾀとして送信する
			}
			break;
	}	
}

//[]----------------------------------------------------------------------[]
///	@brief			Suicaの状態が変化したことをOpeに通知する
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/02/17<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void Suica_Status_Chg( void )
{
	ushort req = 0;
	
	if( Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL ){			// Suicaが有効かつ最後に送信したのが受付許可の場合
		req = SUICA_ENABLE_EVT;												// オペに受付可の変化を通知
	}else if( !Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ){	// Suicaが無効かつ最後に送信したのが受付不可の場合
		req = SUICA_DISABLE_EVT;											// オペに受付不可の変化を通知
	}
	
	if( req )
		queset( OPETCBNO, req, 0, NULL );									// オペにイベントを通知
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica系のカードが使用されたかどうかの判定を行う
//[]----------------------------------------------------------------------[]
///	@param[in]		kind：使用カード種別
///	@return			ret ：1：使用済み 0：未使用
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/03/18<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar SuicaUseKindCheck( uchar kind )
{
	uchar ret = 0;
	
	if( (kind == SUICA_USED) ||	// Suica使用時
	    (kind == PASMO_USED) ||	// PASMO使用時
	    (kind == ICOCA_USED) ||	// ICOCA使用時
	    (kind == ICCARD_USED)){	// IC-Card使用時
	    ret = 1;
	}
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			電子媒体停止中のステータスを取得する
//[]----------------------------------------------------------------------[]
///	@param[in]		type：0＝精算完了時 1＝精算中止時
///	@return			ret ：0BIT＝Suica停止済み<br>
///					    ：1BIT＝Edy停止済み<br>
///					    ：2BIT＝ｺｲﾝﾒｯｸ停止済み<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/04/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar StopStatusGet( uchar type )
{
	uchar ret = 0;
	
	if( !type ){
		ret = CCT_Cansel_Status.BIT.SUICA_END;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		ret |= (uchar)(CCT_Cansel_Status.BIT.EDY_END << 1);
		ret |= (uchar)(1 << 1);
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		ret |= (uchar)(CCT_Cansel_Status.BIT.CMN_END << 2);
	}else{
		ret = cansel_status.BIT.SUICA_END;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		ret |= (uchar)(cansel_status.BIT.EDY_END << 1);
		ret |= (uchar)(1 << 1);
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		ret |= (uchar)(cansel_status.BIT.CMN_END << 2);
	}
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			日毎集計用にﾊﾞｯｸｱｯﾌﾟ及びﾘｽﾄｱの処理を実施する
//[]----------------------------------------------------------------------[]
///	@param[in]		type：0=バックアップ 1＝リストア<br>
///	@return			ret ：0＝取得OK		 1＝取得失敗（設定なし）<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/04/14<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar DailyAggregateDataBKorRES( uchar type )
{
	uchar ret = 0;
	
// MH321800(S) G.So ICクレジット対応
	if (isEC_USE() != 0) {
		if( !type )
			memcpy( &bk_syu_dat_ec, &Syuukei_sp.ec_inf.now, sizeof( bk_syu_dat_ec ));
		else
			memcpy( &Syuukei_sp.ec_inf.now, &bk_syu_dat_ec, sizeof( bk_syu_dat_ec ));
	} else
// MH321800(E) G.So ICクレジット対応
	if( prm_get(COM_PRM, S_PAY, 24, 1, 3) == 1 ){
		if( !type )
			memcpy( &bk_syu_dat, &Syuukei_sp.sca_inf.now, sizeof( bk_syu_dat ));
		else	
			memcpy( &Syuukei_sp.sca_inf.now, &bk_syu_dat, sizeof( bk_syu_dat ));
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	}else if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ){
//		if( !type)
//			memcpy( &bk_syu_dat, &Syuukei_sp.edy_inf.now, sizeof( bk_syu_dat ));
//		else	
//			memcpy( &Syuukei_sp.edy_inf.now, &bk_syu_dat, sizeof( bk_syu_dat ));
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	}else{
		ret = 1;
	}
		
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			決済額と商品選択データの金額が一致しない場合のアラーム表示
//[]----------------------------------------------------------------------[]
///	@param			None<br>
///	@return			None<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/06/23<br>
///					Update
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void	PayDataErrDisp( void )
{
// MH321800(S) G.So ICクレジット対応
	if (isEC_USE() != 0) {
		Settlement_Res.settlement_data = Ec_Settlement_Res.settlement_data;
	}
// MH321800(E) G.So ICクレジット対応

// MH321800(S) T.Nagai ICクレジット対応
//	if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR &&				// 決済額と商品選択データで差がある精算が行なわれた
//		Settlement_Res.settlement_data ){ 							// 今回の精算で決済結果データを受信
	if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR ){				// 決済額(0円含む)と商品選択データで差がある精算が行なわれた
// MH321800(E) T.Nagai ICクレジット対応

		switch( OPECTL.Ope_mod ){
			case 2:
				Lcd_WmsgDisp_ON2( 1, COLOR_RED,  LCD_BLINK_OFF, ERR_CHR[47] );
				Lcd_WmsgDisp_ON(  1, COLOR_RED,  LCD_BLINK_OFF, ERR_CHR[22] );
				nyukin_delay_check( nyukin_delay, delay_count);		// 保留中のｲﾍﾞﾝﾄがあれば再postする
				Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 1;				// ｱﾗｰﾑﾒｯｾｰｼﾞ表示用ﾌﾗｸﾞｾｯﾄ

				if( OPECTL.op_faz == 8 && CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ ){
					Op_StopModuleWait_sub( 0 );						// Suica停止済み処理
				}else{
					read_sht_cls();									// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ
				}
				cn_stat( 2, 2 );									// 入金不可
// MH321800(S) Y.Tanizaki ICクレジット対応
//				Lagcan( OPETCBNO, 10 );								// 画面切替用ﾀｲﾏｰﾘｾｯﾄ(Timer10)
				if(isEC_USE()) {
					Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );		// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
				} else {
					Lagcan( OPETCBNO, 10 );							// 画面切替用ﾀｲﾏｰﾘｾｯﾄ(Timer10)
				}
// MH321800(E) Y.Tanizaki ICクレジット対応
				
				if( dsp_fusoku ){											//  不足アラーム表示中？
			    	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );		//  不足アラーム消去ﾀｲﾏｰを止める
			    	dsp_fusoku = 0;											//  不足フラグOFF
			    }
				break;
			case 3:
// MH321800(S) T.Nagai ICクレジット対応
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if (Suica_Rec.Data.BIT.CTRL_CARD) {
				if (isEC_CARD_INSERT()) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					// カードありは抜き取り待ちを優先する
					break;
				}
// MH321800(E) T.Nagai ICクレジット対応
				Lcd_WmsgDisp_ON(  1, COLOR_RED,  LCD_BLINK_OFF, ERR_CHR[47] );
				break;
			default:
				break;
			
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			決済額と商品選択データの金額が一致しない場合のアラーム消去
//[]----------------------------------------------------------------------[]
///	@param			None<br>
///	@return			None<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/06/23<br>
///					Update
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void	PayDataErrDispClr( void )
{
	if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR &&				// 決済異常発生中
		(DspSts == LCD_WMSG_ON || DspSts2 == LCD_WMSG_ON )){		// アラーム表示中
		Lcd_WmsgDisp_OFF2();
		Lcd_WmsgDisp_OFF();
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica_Ctrl内からコールされるSuica関連のパラメータ取得処理<br>
///					処理統合対策
//[]----------------------------------------------------------------------[]
///	@param			None<br>
///	@return			0:Suica有効 1：Suica無効<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/07/04<br>
///					Update
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
short	SuicaParamGet( void )
{
	
	short	ret = 0;
	
// MH321800(S) G.So ICクレジット対応
//	ret = ( prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1 );
	ret = (isEMoneyReader() != 0)? 0 : 1;
// MH321800(E) G.So ICクレジット対応
	
	return ret;
}

// MH322914 (s) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
//[]----------------------------------------------------------------------[]
///	@brief			ジャーナル使用可非によってSuicaを使用できるかどうかを判定する<br>
//[]----------------------------------------------------------------------[]
///	@param			None<br>
///	@return			None<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/07/04<br>
///					Update
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void	PrinterCheckForSuica( void )
{
	
	if( Ope_isJPrinterReady() != 1 || IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)){		// ジャーナル使用不可時
		Suica_Rec.Data.BIT.PRI_NG = 1;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		Edy_Rec.edy_status.BIT.PRI_NG = 1;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	}
}
// MH322914 (e) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)


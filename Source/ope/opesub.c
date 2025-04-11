/*[]----------------------------------------------------------------------[]*/
/*| 磁気ﾘｰﾀﾞｰ関連制御                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"lcd_def.h"
#include	"flp_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"common.h"
#include	"ntnet.h"
#include	"cnm_def.h"
#include	"suica_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"ope_ifm.h"
#include	"ntnet_def.h"
#include	"remote_dl.h"
#include	"mnt_def.h"
#include	"oiban.h"
#include	"raudef.h"
#include	"cre_ctrl.h"
// MH810100(S) K.Onodera  2020/01/23 車番チケットレス(QR確定・取消データ対応)
#include	"pkt_def.h"
// MH810100(E) K.Onodera  2020/01/23 車番チケットレス(QR確定・取消データ対応)

extern ulong FLT_FindFirstLog(short id, t_FltLogHandle *handle, char *buf);
extern ulong FLT_FindNextLog_OnlyDate(short id, t_FltLogHandle *handle, char *buf);
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//extern	void	Pcard_shut_close( void );
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern void lcdbm_notice_dsp( ePOP_DISP_KIND kind, uchar DispStatus );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
extern void lcdbm_notice_dsp3( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info, uchar *str, ulong str_size );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
//// MH810100(S) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
//void DiscountSortCheck();
BOOL DiscountSortCheck();
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
extern BOOL IsDupSyubetuwari(void);
// MH810100(E) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
// MH810100(S) 2020/09/04 #4584 【検証課指摘事項】　 同一割引券種数の制限が「１」の場合（No33 再評価NG）(割引上限対応)
// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// BOOL IsWaribikiLimitOver(void);
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) 2020/09/04 #4584 【検証課指摘事項】　 同一割引券種数の制限が「１」の場合（No33 再評価NG）(割引上限対応)

// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
extern uchar r_zero_call;
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）

// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
short GetTypeSwitch(ushort syu, ushort card_no);
short CheckDiscount(ushort syu, ushort card_no, ushort info);
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)

static	short	opncls_TrbClsChk( void );
char	save_op_faz;

uchar	Chu_Syu_Status;				// 中止集計実施ｽﾃｰﾀｽ 0：未実施 1：実施済み
char	PrcKigenStr[32];
// MH321800(S) G.So ICクレジット対応
t_CyclicDisp	Ope2CyclicDisp;
// MH321800(E) G.So ICクレジット対応
#define	_CHKDATE(s)		chkdate2((short)(s.year + 2000), (short)s.mon, (short)s.day)
short chkdate2(short yyyy, short mm, short dd);
static date_time_rec	*get_log_date_time_rec( ushort LogSyu, uchar *record );
Receipt_data	PayInfoData_SK;							// 精算情報データ用強制・不正出庫データ
uchar	PayInfo_Class;									// 精算情報データ用処理区分
// MH810100(S) 2020/09/02 再精算時の複数枚対応
ulong	g_ulZumiWari;								// 使用済み割引額を分割するよう
// MH810100(E) 2020/09/02 再精算時の複数枚対応
// MH810100(S) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
ulong	g_ulZumiWariOrg;							// 使用済み割引額を分割するまえ
// MH810100(E) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）


/*[]----------------------------------------------------------------------[]*/
/*| 磁気ｶｰﾄﾞ処理                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_card( msg, mod )                                     |*/
/*| PARAMETER    : msg : ﾒｯｾｰｼﾞ                                            |*/
/*|                paymod : 0=通常 1=修正                                  |*/
/*| RETURN VALUE : ret : 精算完了ﾓｰﾄﾞ                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_card( ushort msg, ushort paymod )
{
	extern	uchar ryodsp;
	short	ret, rd_snd, anm;
	short	dno;
	short	ErrNum=0;
	char	f_Card = 0;												// 0:不正係員ｶｰﾄﾞ 1:係員ｶｰﾄﾞ受け -1:係員ｶｰﾄﾞNG
	uchar	PrcKiFlg = 0;
	uchar	CardKind;
// MH321800(S) Y.Tanizaki ICクレジット対応
//	uchar	credit_check = CRE_STATUS_OK;
// MH321800(E) Y.Tanizaki ICクレジット対応

	ret = 0;
	rd_snd = 2;														// 取出し口移動
	anm = 0;
	dno = 0;

	if( MifStat == MIF_WRITING )	// Mifareをタッチ(読取完了)していたら磁気ｶｰﾄﾞは読まない
		goto MifareWriting;		

	CardKind = (uchar)((OPECTL.LastCardInfo&0x8000) ? 0xFF:MAGred[MAG_ID_CODE]);	// JISカード判定
	switch( CardKind ){	
		case 0x1a:													// APS定期
			if( SyuseiEndKeyChk() == 2 ){
				dno = 1;										// "    このカードは使えません    "
				anm = AVM_CARD_ERR1;
				break;
			}
// 不具合修正(S) K.Onodera 2016/10/13 #1587 金額指定の遠隔精算時に全日定期を挿入するとフリーズする
			// 遠隔精算(金額指定)
			if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE ){
				dno = 1;										// "    このカードは使えません    "
				anm = AVM_CARD_ERR1;
				break;
			}
// 不具合修正(E) K.Onodera 2016/10/13 #1587 金額指定の遠隔精算時に全日定期を挿入するとフリーズする

			switch( OPECTL.CR_ERR_DSP = (char)al_pasck( (m_gtapspas *)MAGred ) ){
				case 0:												// OK
					if( SKIP_APASS_CHK ){							// 強制ｱﾝﾁﾊﾟｽOFF設定(定期券ﾁｪｯｸ＝しない)？
						OPECTL.Apass_off_seisan = 1;				// 強制ｱﾝﾁﾊﾟｽOFF設定状態で定期使用
					}
					al_pasck_set();							// PayDataに定期情報を書き込み
					if(OPECTL.PassNearEnd == 1){
						ope_anm( AVM_KIGEN_NEAR_END );
					}
					ryo_cal( 1, OPECTL.Pr_LokNo );					// 料金計算
					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// 入金中
					}

					if( paymod ){									// 修正？
						ac_flg.cycl_fg = 70;						// 入金
					}else{
						ac_flg.cycl_fg = 10;						// 入金
					}

					ret = in_mony( msg, paymod );
					al_mkpas( (m_gtapspas *)MAGred, &CLK_REC );
					if( CPrmSS[S_TIK][10] == 0 ){					// 定期券ﾍﾞﾘﾌｧｲしない設定?
						rd_snd = 16;								// 定期券ﾃﾞｰﾀﾗｲﾄ(ﾍﾞﾘﾌｧｲ無し)
					}else{
						rd_snd = 9;									// 定期券ﾃﾞｰﾀﾗｲﾄ(ﾍﾞﾘﾌｧｲ有り)
					}
					read_sht_opn();										/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｵｰﾌﾟﾝ */
					if( ryo_buf.zankin ){							// 精算する料金がある場合
					if( !SyuseiEndKeyChk() ){
						cn_stat( 3, 2 );									/* 入金可 */
					}
					}

					if (ope_imf_GetStatus() == 2) {
						if (is_paid_remote(&PayData) < 0) {
						// 振替精算未実施（定期利用指示）ならここで計上
							ope_ifm_FurikaeCalc( 0 );
							ret = in_mony(IFMPAY_FURIKAE, 0);
							// すでに精算が完了していれば０円振替なのでretは変化しない（上書きok）
						}
					}
// MH322914(S) K.Onodera 2016/09/14 AI-V対応：振替精算
					if( PiP_GetFurikaeSts() ){
// 不具合修正(S) K.Onodera 2016/10/13 #1514 振替先で定期を利用すると通知した上で定期を使用せずに精算した時の釣銭が正しくない
//						if( is_paid_remote(&PayData) < 0 ){
// 不具合修正(E) K.Onodera 2016/10/13 #1514 振替先で定期を利用すると通知した上で定期を使用せずに精算した時の釣銭が正しくない
							ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
							vl_now = V_FRK;						// 振替精算
							ryo_cal( 3, OPECTL.Pr_LokNo );		// サービス券として計算
							ret = in_mony(OPE_REQ_FURIKAE_GO, 0);
// 不具合修正(S) K.Onodera 2016/10/13 #1514 振替先で定期を利用すると通知した上で定期を使用せずに精算した時の釣銭が正しくない
//						}
// 不具合修正(E) K.Onodera 2016/10/13 #1514 振替先で定期を利用すると通知した上で定期を使用せずに精算した時の釣銭が正しくない
					}
// MH322914(E) K.Onodera 2016/09/14 AI-V対応：振替精算
					if ( OPECTL.Pay_mod == 2 ) {					// 修正精算中
						if (is_paid_syusei(&PayData) < 0) {
							ope_ifm_FurikaeCalc( 1 );
							ret = in_mony(IFMPAY_FURIKAE, 0);
							// すでに精算が完了していれば０円振替なのでretは変化しない（上書きok）
						}
					}
// 不具合修正(S) K.Onodera 2016/11/24 #1581 振替精算の精算方法に、2=定期券精算がセットされてしまう
//					if(OpeNtnetAddedInfo.PayMethod != 11){
					if(OpeNtnetAddedInfo.PayMethod != 11 && OpeNtnetAddedInfo.PayMethod != 12 ){
// 不具合修正(E) K.Onodera 2016/11/24 #1581 振替精算の精算方法に、2=定期券精算がセットされてしまう
						OpeNtnetAddedInfo.PayMethod = 2;	// 精算方法=2:定期券精算
					}
					break;

				case 5:												// 無効定期
					dno = 3;										// "     このカードは無効です     "
					anm = AVM_CARD_ERR3;
					break;

				case 3:												// 期限切れ
				case 6:												// 期限前
					dno = 2;										// "   このカードは期限切れです   "
					anm = AVM_KIGEN_END;
					break;

				case 99:											// HOSTへ問合せ
					read_sht_cls();										/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ */
					cn_stat( 2, 2 );									/* 入金不可 */
					rd_snd = 0;
					NTNET_Snd_Data142( OPECTL.ChkPassPkno, OPECTL.ChkPassID );	// 定期券問合せﾃﾞｰﾀ(ﾃﾞｰﾀ種別142)作成
					grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);	// "                              "
					blink_reg(7, 5, 20, 0, COLOR_DARKSLATEBLUE, &OPE_CHR[7][5]);	// "     しばらくお待ち下さい     "
					Lagtim( OPETCBNO, 6, (ushort)(prm_get( COM_PRM,S_NTN,31,2,1 )*50+1) );	// ﾀｲﾏｰ6(XXs)起動(定期券問合せ監視)
					rd_snd = 0;										// ﾘｰﾀﾞｰ制御無し(ｶｰﾄﾞをﾘｰﾀﾞｰ内に保留する)
					LagCan500ms(LAG500_MIF_LED_ONOFF);
					if( MIFARE_CARD_DoesUse ){						// Mifareが有効な場合
						op_MifareStop_with_LED();					// Mifare無効
					}
					ret = 0;
					break;

				case 4:
				case 29:											// 問合せタイムアウトNG時
					if( OPECTL.CR_ERR_DSP == 4 )
						dno = 19;										// "      入出庫不一致です        "
					else
						dno = 1;										// "    このカードは使えません    "
					anm = AVM_CARD_ERR1;
					break;
				default:
					dno = 1;										// "    このカードは使えません    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;

		case 0x2d:													// ｻｰﾋﾞｽ券
			if( SyuseiEndKeyChk() ){
				dno = 1;										// "    このカードは使えません    "
				anm = AVM_CARD_ERR1;
				break;
			}

			switch( OPECTL.CR_ERR_DSP = (char)al_svsck( (m_gtservic *)MAGred ) ){
				case 0:												// OK
					ryo_cal( 3, OPECTL.Pr_LokNo );					// 料金計算

					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// 入金中
					}

					if( paymod ){									// 修正？
						ac_flg.cycl_fg = 70;						// 入金
					}else{
						ac_flg.cycl_fg = 10;						// 入金
					}

					ret = in_mony( msg, paymod );

					if( ryo_buf.nyukin < ryo_buf.dsp_ryo ){			// 入金額 < 駐車料金 ?
						if(( CPrmSS[S_DIS][3] == 1 )&&				// ｻｰﾋﾞｽ券取り込み
						   ( CPrmSS[S_DIS][2] == 0 )){				// 廃券ﾏｰｸ印字しない
							/*** 連続挿入許可する ***/
							cr_service_holding = 1;					// ｻｰﾋﾞｽ券連続処理 保留中
							rd_snd = 3;								// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
						}else{
							/*** 連続挿入許可しない ***/
							rd_snd = 10;							// ｶｰﾄﾞ後方排出
						}
					}else{
						rd_snd = 11;								// ｶｰﾄﾞ前方ﾌﾟﾘﾝﾄ後、後方排出 (駐車券挿入を可能とする為にTF-4300Nｿﾌﾄを移植)
					}
					break;

				case 3:												// 期限切れ
				case 6:												// 期限前
					dno = 2;										// "   このカードは期限切れです   "
					anm = AVM_KIGEN_END;
					break;

				case 14:											// 限度枚数ｵｰﾊﾞｰ
					dno = 8;										// "    限度枚数を超えています    "
					anm = AVM_CARD_ERR1;
					break;

				default:
					dno = 1;										// "    このカードは使えません    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;

		case 0x0e:													// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ
			if( SyuseiEndKeyChk() ){
				dno = 1;											// "    このカードは使えません    "
				anm = AVM_CARD_ERR1;
				break;
			}
			if( 3 == prm_get( COM_PRM,S_PAY,21,1,3) || 
				4 == prm_get( COM_PRM,S_PAY,21,1,3)) {
				// プリンタなしの場合はプリペイドカードは使用不可
				dno = 1;										// "    このカードは使えません    "
				anm = AVM_CARD_ERR1;
				break;
			}

			switch( OPECTL.CR_ERR_DSP = (char)al_preck( (m_gtprepid *)MAGred ) ){
				case 0:												// OK
					Ope_Last_Use_Card = 1;							// プリペ利用
					ryo_cal( 2, OPECTL.Pr_LokNo );					// 料金計算

					OPECTL.LastUsePCardBal = (ulong)CRD_DAT.PRE.ram;// ﾌﾟﾘﾍﾟｲﾄﾞ残額保存

					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// 入金中
					}

					if( paymod ){									// 修正？
						ac_flg.cycl_fg = 70;						// 入金
					}else{
						ac_flg.cycl_fg = 10;						// 入金
					}

					ret = in_mony( msg, paymod );
					al_mkpre( (m_gtprepid *)MAGred, &CRD_DAT.PRE );
// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//					if( (CPrmSS[S_PRP][10] == 1) ||					// ﾍﾞﾘﾌｧｲする設定
//						((CPrmSS[S_PRP][10] == 2) && (CRD_DAT.PRE.ram)) ){	// 残度数が0でなければﾍﾞﾘﾌｧｲする
					if( ( prm_get( COM_PRM,S_PRP,10,1,1) == 1) ||						// ﾍﾞﾘﾌｧｲする設定
						((prm_get( COM_PRM,S_PRP,10,1,1) == 2) && (CRD_DAT.PRE.ram)) ){	// 残度数が0でなければﾍﾞﾘﾌｧｲする
// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
						rd_snd = 12;								// ﾃﾞｰﾀﾗｲﾄ(ﾍﾞﾘﾌｧｲ有り)
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//						if(1 == prm_get( COM_PRM,S_PRP,10,1,2)){
//							Pcard_shut_close();
//						}
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
					}
					else{
						rd_snd = 17;								// ﾃﾞｰﾀﾗｲﾄ(ﾍﾞﾘﾌｧｲ無し)
					}
					break;

				case 7:												// 残額ｾﾞﾛ
					dno = 4;										// "       残額がありません       "
					anm = AVM_CARD_ERR1;
					break;

				case 10:											// 表裏ｴﾗｰ
					dno = 5;										// "      挿入方向が違います      "
					anm = AVM_CARD_ERR1;
					break;

				case 14:											// 限度枚数ｵｰﾊﾞｰ
					dno = 8;										// "    限度枚数を超えています    "
					anm = AVM_CARD_ERR1;
					break;

				case 3:												// 有効期限切れ 
					alm_chk( ALMMDL_SUB2, ALARM_OUTSIDE_PERIOD_PRC, 2 );	/* ｱﾗｰﾑﾛｸﾞ登録 発生・解除	*/
					PrcKiFlg = 1;
					dno = 2;										// "   このカードは期限切れです   "
					anm = AVM_KIGEN_END;
					break;

				default:
					dno = 1;										// "    このカードは使えません    "
					anm = AVM_CARD_ERR1;
					break;
			}
			GetPrcKigenStr ( PrcKigenStr );
			break;

		case 0x2c:													// 回数券
			if( SyuseiEndKeyChk() ){
				dno = 1;										// "    このカードは使えません    "
				anm = AVM_CARD_ERR1;
				break;
			}
			if( 3 == prm_get( COM_PRM,S_PAY,21,1,3) || 
				4 == prm_get( COM_PRM,S_PAY,21,1,3)) {
				// プリンタなしの場合は回数券使用不可
				dno = 1;										// "    このカードは使えません    "
				anm = AVM_CARD_ERR1;
				break;
			}

			switch( OPECTL.CR_ERR_DSP = (char)al_kasck( (m_gtservic *)MAGred ) ){
				case 100:											// OK (期限切れ間近)
					dno = 10;										// "このｶｰﾄﾞはもうすぐ期限切れです"
					OPECTL.PassNearEnd = 1;
					ope_anm( AVM_KIGEN_NEAR_END );
				case 0:												// OK

					Ope_Last_Use_Card = 2;							// 回数券利用
					ryo_cal( 2, OPECTL.Pr_LokNo );					// 料金計算

					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// 入金中
					}

					ac_flg.cycl_fg = 10;							// 入金

					ret = in_mony( msg, paymod );

					al_mkkas( (m_gtkaisuutik *)MAGred, &CRD_DAT.SVS, &CLK_REC );	// 券ﾃﾞｰﾀ作成
// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//					if( (CPrmSS[S_PRP][10] == 1) ||					// ﾍﾞﾘﾌｧｲする設定
//						((CPrmSS[S_PRP][10] == 2) && (KaisuuAfterDosuu)) ){	// 残度数が0でなければﾍﾞﾘﾌｧｲする
					if( ( prm_get( COM_PRM,S_PRP,10,1,1) == 1) ||							// ﾍﾞﾘﾌｧｲする設定
						((prm_get( COM_PRM,S_PRP,10,1,1) == 2) && (KaisuuAfterDosuu)) ){	// 残度数が0でなければﾍﾞﾘﾌｧｲする
// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
						rd_snd = 20;								// ﾃﾞｰﾀﾗｲﾄ(ﾍﾞﾘﾌｧｲ有り)
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//						if(1 == prm_get( COM_PRM,S_PRP,10,1,2)){
//							Pcard_shut_close();
//						}
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
					}
					else{
						rd_snd = 19;								// ﾃﾞｰﾀﾗｲﾄ(ﾍﾞﾘﾌｧｲ無し)
					}
					break;

				case 3:												// 期限切れ
				case 6:												// 期限前
					dno = 2;										// "   このカードは期限切れです   "
					anm = AVM_KIGEN_END;
					break;

				case 7:												// 残額ｾﾞﾛ
					dno = 4;										// "       残額がありません       "
					anm = AVM_CARD_ERR1;
					break;

				case 10:											// 表裏ｴﾗｰ
					dno = 5;										// "      挿入方向が違います      "
					anm = AVM_CARD_ERR1;
					break;

				case 14:											// 限度枚数ｵｰﾊﾞｰ
					dno = 8;										// "    限度枚数を超えています    "
					anm = AVM_CARD_ERR1;
					break;

				default:
					dno = 1;										// "    このカードは使えません    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;
	    default:
// MH321800(S) Y.Tanizaki ICクレジット対応
//			if( SyuseiEndKeyChk() ){
//				dno = 1;										// "    このカードは使えません    "
//				anm = AVM_CARD_ERR1;
//				break;
//			}
//			// JIS1 と認識したか、その他の（アマノカード以外、プリカ以外）場合。修正精算は credit 不可
//			if( RD_Credit_kep != 0 && ryo_buf.zankin != 0L && paymod != 1)
//			{
//// クレジットカードのときのみ、ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
//				Lagcan( OPETCBNO, 10 );									// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
//				if( Suica_Rec.Data.BIT.CTRL_MIRYO || 
//					Edy_Rec.edy_status.BIT.CTRL_MIRYO )					// 決済未了が発生している場合はｸﾚｼﾞｯﾄの使用を不可とする
//				{
//					opr_snd( 2 );										//前方排出
//					return 0;
//				}
//
//				// クレジット精算可能状態かチェック
//				credit_check = creStatusCheck();
//				if( credit_check != 0 ){								// エラーありの場合
//					if( credit_check & CRE_STATUS_DISABLED ){			// クレジット設定なし
//						ErrNum = 2;		// "    このカードは使えません    "
//					}
//					if( credit_check & CRE_STATUS_NOTINIT ){			// 初回接続未完了
//						;	// 与信問合せ可能
//					}
//					if( credit_check & CRE_STATUS_STOP ){				// クレジット停止中
//						ErrNum = 2;		// "    このカードは使えません    "
//					}
//					if( credit_check & CRE_STATUS_UNSEND ){				// 売上依頼未送信
//						;	// 与信問合せ可能
//					}
//					if( credit_check & CRE_STATUS_SENDING ){			// 通信中
//						;	// 与信問合せ可能
//					}
//					if( credit_check & CRE_STATUS_PPP_DISCONNECT ){		// PPP未接続
//						ErrNum = 2;		// "    このカードは使えません    "
//					}
//				}
//				if( ErrNum == 0 ){
//					// 精算限度額　チェック
//					if( creLimitCheck( ryo_buf.zankin ) != 0 )
//					{														// "クレジットカードでの"
//						ErrNum = 4;											// "精算限度額をこえています"
//					}
//					else if( e_incnt > 0 || PayData.Electron_data.Suica.pay_ryo || PayData.Electron_data.Edy.pay_ryo )
//					{														// ①電子マネー使用済時はクレジットの使用を不可とする
//						ErrNum = 2;											// "    このカードは使えません    "
//					}
//					else if( OpeNtnetAddedInfo.PayMethod == 11 &&			// 振替精算要求受信済み
//							 vl_frs.antipassoff_req &&						// 定期使用あり
//							 OPECTL.Ope_mod != 2 &&							// 修正精算でない
//							 !PayData.teiki.syu ){							// 定期が未使用の場合
//						ErrNum = 2;											// "    このカードは使えません    "
//					}
//					else
//					{
//						OPECTL.InquiryFlg = 1;		// 外部照会中フラグON
//						if( MIFARE_CARD_DoesUse ){									// Mifareが有効な場合
//							op_MifareStop_with_LED();								// Mifare無効
//						}
//						LagCan500ms(LAG500_MIF_LED_ONOFF);
//
//						creSeisanInfo.amount = ryo_buf.zankin;				// CREへの精算要求用
//						ryo_buf.credit.pay_ryo = ryo_buf.zankin;			// 計算用
//
//						memset( creSeisanInfo.jis_1, 0x20, sizeof(creSeisanInfo.jis_1) );
//						memset( creSeisanInfo.jis_2, 0x20, sizeof(creSeisanInfo.jis_2) );
//
//						if( MAGred[MAG_ID_CODE] == CREDIT_CARDID_JIS1 )
//						{
//							memcpy( creSeisanInfo.jis_1, &MAGred[MAG_ID_CODE+2], sizeof(creSeisanInfo.jis_1) );
//						}else{
//							memcpy( creSeisanInfo.jis_2, &MAGred[MAG_ID_CODE], sizeof(creSeisanInfo.jis_2) );
//						}
//
//						if( credit_check & CRE_STATUS_SENDING ){			// 通信中の場合
//							;	// 応答受信するまで何もしない
//						}
//						else if( credit_check & CRE_STATUS_NOTINIT ){		// 初回接続未完了の場合は、開局処理を行う
//							cre_ctl.OpenKind = CRE_KIND_AUTO;				// 開局発生要因に自動時を設定
//							// 開局コマンド(01)送信
//							creCtrl( CRE_EVT_SEND_OPEN );
//						}
//						else if( credit_check & CRE_STATUS_UNSEND ){		// 売上依頼未送信の場合は、売上依頼送信処理を行う
//							cre_ctl.SalesKind = CRE_KIND_AUTO;				// 売上依頼発生要因に自動時を設定
//							// 売上依頼データ(05)送信
//							creCtrl( CRE_EVT_SEND_SALES );
//						}else{
//							// 与信問合せデータ(03)送信, これが正常完了後に 売上依頼データ(05)送信を行う。
//							if(	creCtrl( CRE_EVT_SEND_CONFIRM ) != 0 )
//							{
//								creSeisanInfo.amount = 0;
//								ryo_buf.credit.pay_ryo = 0;
//								ErrNum = 2;										// "    このカードは使えません    "
//
//								xPause( 20 );		// 200ms waitしてからNGメール送信（Mifareが活性化できないため）
//								queset( OPETCBNO, CRE_EVT_04_NG1, 0, 0 );	// OpeMainへ通知 (Mifare,Suica,Edyを復活)
//							}
//						}
//					}
//				}
//				if(ErrNum)
//				{
//					opr_snd( 2 );		//前方排出
//					// メッセージ表示 エラー
//					creMessageAnaOnOff( 1, (short)ErrNum );
//					OPECTL.InquiryFlg = 0;		// 外部照会中フラグOFF
//					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ
//
//				}else{
//					read_sht_cls();									// 強制的にｼｬｯﾀｰを閉じる
//					cn_stat( 2, 2 );								// 入金不可
//
//					/* 照会中表示する前に通常表示することで、問い合わせ完了後にエラー表示が復元されないようにする */
//					if( ryodsp ){
//						Lcd_Receipt_disp();
//					}else{
//						dsp_intime( 7, OPECTL.Pr_LokNo );			// 入庫時刻表示
//					}
//					// メッセージ表示　照会中
//					creMessageAnaOnOff( 1, 1 );
//					if( OPECTL.op_faz == 0 ){
//						OPECTL.op_faz = 1;							// 入金中
//					}
//				}
//				goto AL_CARD_END;
//			}
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810103 GG119202(S) 磁気リーダーにクレジットカード挿入したときの案内放送・表示
			if (isEC_USE()) {
				OPECTL.CR_ERR_DSP = 8;								// "   ＜＜ 使用不可券です ＞＞   "
				if (CardKind == 0xff && isEcBrandNoReady(BRANDNO_CREDIT)) {
				// JISカードなら音声を変える
					anm = AVM_CARD_ERR8;							// ｱﾅｳﾝｽ 「決済リーダーで精算してください」
					dno = 56;										// "決済リーダーで精算してください"
				}
				else {
					anm = AVM_CARD_ERR4;							// ｱﾅｳﾝｽ 「このカードはお取扱いできません」
					dno = 41;										// "このカードはお取扱いできません"
				}
				if( rd_tik == 0 ){									// ﾌﾟﾘﾝﾀﾌﾞﾛｯｸに駐車券無し?
					rd_snd = 13;									// 券取出し口(ﾘｰﾀﾞｰ・ﾌﾟﾘﾝﾀ両方戻し)
				}
				break;
			}
// MH810103 GG119202(E) 磁気リーダーにクレジットカード挿入したときの案内放送・表示
MifareWriting:
			OPECTL.CR_ERR_DSP = 8;									// "   ＜＜ 使用不可券です ＞＞   "
			anm = AVM_CARD_ERR1;
			dno = 1;												// "    このカードは使えません    "
			if( rd_tik == 0 ){										// ﾌﾟﾘﾝﾀﾌﾞﾛｯｸに駐車券無し?
				rd_snd = 13;										// 券取出し口(ﾘｰﾀﾞｰ・ﾌﾟﾘﾝﾀ両方戻し)
			}
			break;
	}

	if(MAGred[MAG_ID_CODE] == 0x1a && ( OPECTL.CR_ERR_DSP != 0 && OPECTL.CR_ERR_DSP != 99 )){						// 磁気定期券かつエラー発生
		if( MIFARE_CARD_DoesUse && OPECTL.ChkPassSyu == 0 ){ 								// 磁気定期を使用する設定かつｱﾝﾁﾊﾟｽ通信ﾁｪｯｸ中でない場合
			op_MifareStart();											// Mifareを有効化
		}
	}

	if( rd_snd ){
		opr_snd( rd_snd );
	}

	if( anm ){
		ope_anm(  anm );								// Card Error Announce
	}

	if( dno ){
		if(PrcKiFlg != 0){
			Lcd_WmsgDisp_ON2(  1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[dno] );	// このカードは期限切れです
			Lcd_WmsgDisp_ON ( 1, COLOR_RED, LCD_BLINK_OFF, (unsigned char*)PrcKigenStr );	// 有効期限ＸＸＸＸ年ＸＸ月ＸＸ日
		}else{
			grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[dno] );	// ｴﾗｰ表示は反転文字にて表示
		}
// MH810105(S) MH364301 インボイス対応
		OPECTL.f_CrErrDisp = 1;							// エラー表示中
// MH810105(E) MH364301 インボイス対応
	}
// MH321800(S) Y.Tanizaki ICクレジット対応
//AL_CARD_END:
// MH321800(E) Y.Tanizaki ICクレジット対応
	if( OPECTL.CR_ERR_DSP || ErrNum || f_Card == 1 ){
		Op_StopModuleWait_sub( 2 );						// 電子媒体再活性化要求
	}
	return( ret );
}

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
////[]----------------------------------------------------------------------[]
/////	@brief			Ｔカード受付け
////[]----------------------------------------------------------------------[]
/////	@return			none
////[]----------------------------------------------------------------------[]
/////	@author			MATSUSHITA
/////	@date			Create	:	2020/06/22<br>
/////					Update
////[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
//void	AcceptTCard(void)
//{
//	short   i;
//	long	Err;
//	char    prev_Tcard_Retry_Cnt = Tcard_Retry_Cnt;
//	
//
//	ec_MessageAnaOnOff(0, 0);
//	T_Rate_now.disable = 0;
//	if ((OPECTL.op_faz > 1)  									// 精算機の処理フェーズが精算中（未入金／入金中）でない
//	|| (Suica_Rec.Data.BIT.CTRL_MIRYO != 0 )){					// 電子マネーの処理未了が発生中である。
//		BrandResRsltTbl.no = EC_BRAND_REASON_299;				// その他のエラー
//	/*	ec_BrandResultDisp(BRANDNO_TCARD, BrandResRsltTbl.no);			エラー表示しない */
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);				// ブランド選択結果応答NG
//		// ブランド選択データは送信しない
//		return;
//	}
//	if (T_Card_Use != 0) { 										// 既にTカード受付済である。	
//		BrandResRsltTbl.no = EC_BRAND_REASON_211;				// その他のエラー
//		ec_BrandResultDisp(BRANDNO_TCARD, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);				// ブランド選択結果応答NG
//		Ope_EcPayStart();										// ブランド選択データ
//		return;
//	}
//
//	// Tカード番号の取得
//	memset(T_membar_num, 0x20, sizeof(T_membar_num));
//	for(i = 0; i < sizeof(T_membar_num); i++){
//		if (MAGred_HOUJIN[MAG_ID_CODE + i] == ' ') {							// 終端スペース
//			break;
//		}
//		if(chk_num(MAGred_HOUJIN[MAG_ID_CODE + i])){							//  '0'～'9'の範囲内 ASCII
//			T_membar_num[i] = (uchar)(MAGred_HOUJIN[MAG_ID_CODE + i] & 0x0f);	// 数値に変換
//		}
//		else {																	// フォーマットエラー
//			// 決済リーダでチェックするのでここには来ない筈だが念のため
//			i = -1;	/* カードデータ不正 */
//			break;
//		}
//	}
//	if (i < 0) {			// 会員番号の全桁が数字（１～９）
//		err_chk( ERRMDL_CARD, ERR_TCARD_CHK_NG, 2, 0, 0 );
//		BrandResRsltTbl.no = EC_BRAND_REASON_203;			// 誤りデータが含まれるカードを読み取りした。
//		ec_BrandResultDisp(BRANDNO_TCARD, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);			// ブランド選択結果応答NG
//		Ope_EcPayStart();									// ブランド選択データ
//		return;
//	}
//	
//	Err = chk_Tcard_use();
//	if (Err != 0) {											// Ｔカード使用可否の設定が有効
//		err_chk2( ERRMDL_CARD, ERR_CARD_T, 2, 2, 0, &Err );
//		T_Card_Use = 0;										// Ｔカード使用ﾌﾗｸﾞを下ろす
//		if(Err == 13){										// 非同期データ応答タイムアウト発生
//			Card_Retry_DispAna(0, &Tcard_Retry_Cnt);
//			if (prev_Tcard_Retry_Cnt < 3) {					// 3回までは203を返す。以降は208を返す。
//				// 精算機表示はCard_Retry_DispAna()で行うため、EC_BrandResultDispは呼ばない
//				BrandResRsltTbl.no = EC_BRAND_REASON_203;	// 誤りデータが含まれるカードを読み取りした。
//			} else {
//				// 精算機表示はCard_Retry_DispAna()で行うため、EC_BrandResultDispは呼ばない
//				BrandResRsltTbl.no = EC_BRAND_REASON_208;	// 有効なカードであるが使用できない。
//			}
//		}
//		else{
//			BrandResRsltTbl.no = EC_BRAND_REASON_208;		// 有効なカードであるが使用できない。
//		}
//		ec_BrandResultDisp(BRANDNO_TCARD, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);			// ブランド選択結果応答NG
//		Ope_EcPayStart();									// ブランド選択データ
//		return;
//	}
//	
//	
//	// Tカード受付		
//	Ope_Last_Use_Card = 6;	
//	ope_anm( AVM_TCARD_OK );							// Ｔカード受付アナウンス
//	T_Card_Use = 0xff;
//	in_mony( EC_BRAND_RESULT_TCARD, 0 );
//	Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1);			// ブランド選択結果応答OK
//	Ope_EcPayStart();									// ブランド選択データ
//}

////[]----------------------------------------------------------------------[]
/////	@brief			法人カード問い合わせ
////[]----------------------------------------------------------------------[]
/////	@return			0: OK 法人カード問い合わせ実行
/////					1: NG
////[]----------------------------------------------------------------------[]
/////	@author			MATSUSHITA
/////	@date			Create	:	2020/07/01<br>
/////					Update
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//int	InquireHoujin(void)
//{
//	extern	void	chg_ShiftIN_OUT(char *data, short size);
//	extern	void	NTNETDOPA_Snd_Data148(uchar type);
//
//	m_Tcard *Hcard_dat;
//	short	ErrNum;
//	short   i;
//
//	ec_MessageAnaOnOff(0, 0);
//	if ((OPECTL.op_faz > 1)  								// 精算機の処理フェーズが精算中（未入金／入金中）でない
//	|| (Suica_Rec.Data.BIT.CTRL_MIRYO != 0 )){				// 電子マネーの処理未了が発生中である。
//		BrandResRsltTbl.no = EC_BRAND_REASON_299;				// その他のエラー
//	/*	ec_BrandResultDisp(BRANDNO_HOUJIN, BrandResRsltTbl.no);			エラー表示しない */
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);				// ブランド選択結果応答NG
//		// ブランド選択データは送信しない
//		return 1;
//	}
//	
//	// al_card()時と同等のチェックを実施する
//	// 電子マネー決済は同じリーダーで行うためありえない→省略
//	if( OpeNtnetAddedInfo.PayMethod == 11 &&						// 振替精算要求受信済み
//		 vl_frs.antipassoff_req &&									// 定期使用あり
//		 OPECTL.Ope_mod != 2 &&										// 修正精算でない
//		 !PayData.teiki.syu ){										// 定期が未使用の場合
//		BrandResRsltTbl.no = EC_BRAND_REASON_299;					// その他のエラー
//		ec_BrandResultDisp(BRANDNO_HOUJIN, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);					// ブランド選択結果応答NG
//		Ope_EcPayStart();											// ブランド選択データ
//		return 1;							
//	}
//	
//	Hcard_dat = (m_Tcard*)&MAGred_HOUJIN[MAG_ID_CODE];
//	chg_ShiftIN_OUT((char*)Hcard_dat, (short)sizeof( m_Tcard ));
//	memcpy(&H_card_buf, Hcard_dat, sizeof( m_Tcard ));
//	for(i = 0; i < 16; i++){
//		if ( (H_card_buf.cre_mem_no[i] == ' ') 							// 終端スペース
//		  || (chk_num(H_card_buf.cre_mem_no[i]))) {						// '0'～'9'
//			/* 処理なし */
//		} else {
//			i = -1;			/* エラー */	
//			break;
//		}
//	}
//	if ((i < 0) 													// 会員番号の全桁が数字'0'～'9'or ' 'でない
//	||  (!chk_num_range( (char*)Hcard_dat->credit_limit, 4 ))) {	// 有効期限が数字'0'～'9'でない
//		BrandResRsltTbl.no = EC_BRAND_REASON_203;						// 誤りデータが含まれるカードを読み取りした。
//		ec_BrandResultDisp(BRANDNO_HOUJIN, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);						// ブランド選択結果応答NG
//		Ope_EcPayStart();												// ブランド選択データ
//		return 1;
//	}
//	
//	// 上限額を超えている。または利用可設定でない。		
//	ErrNum = chk_Hojin_use();
//	if (ErrNum != 0) {				
//		if (ErrNum == 4) {											// 決済金額が法人カード精算限度額を超えている。
//			BrandResRsltTbl.no = EC_BRAND_REASON_209;				// その他のエラー
//		} else {
//			BrandResRsltTbl.no = EC_BRAND_REASON_207;				// 現行機のメッセージ、アナウンスに合わせて、理由コードを207で返しています。
//		}
//		ec_BrandResultDisp(BRANDNO_HOUJIN, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);					// ブランド選択結果応答NG
//		Ope_EcPayStart();
//		return 1;													// ブランド選択データ
//	}
//		
//	H_card_buf.Ptr2_Chk = 0;						// 提携先カードチェック有無は固定
//	H_Card_Use = 1;
//	OPECTL.InquiryFlg = 1;							// 外部照会中フラグON
//	
//	ryo_buf.credit.pay_ryo = ryo_buf.zankin;		// 計算用
//	memset(ryo_buf.credit.card_no, 0, sizeof(ryo_buf.credit.card_no));
//	memcpy(ryo_buf.credit.card_no, H_card_buf.cre_mem_no, 16);
//	NTNETDOPA_Snd_Data148( 3 );						// 問い合わせ＆応答待ちタイマー開始
//	
//	// メッセージ表示　照会中
//	ec_MessageAnaOnOff ( 2, 1 );
//	if( OPECTL.op_faz == 0 ){
//		OPECTL.op_faz = 1;							// 入金中
//	}
//	return 0;
//}

//[]----------------------------------------------------------------------[]
///	@brief			入金受付可
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@author			A.Shindoh
///	@date			Create	:	2020/07/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void EnableNyukin(void) 
{
	read_sht_opn();									// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
	cn_stat( 3, 2 );								// 紙幣 & COIN再入金可
	if( MIFARE_CARD_DoesUse ){						// Mifareが有効な場合
		op_MifareStart();							// Mifareを利用復活（再開）
	}
	if( OPECTL.credit_in_mony ){					// 入金ありでｸﾚｼﾞｯﾄとのすれ違い入金の場合
		in_mony ( COIN_EVT, 0 );					// ｺｲﾝｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
		OPECTL.credit_in_mony = 0;					// すれ違いﾌﾗｸﾞｸﾘｱ
	}
// MH810104 GG119202(S) 後決済・決裁なしブランドのカード読み込み失敗時に利用可能媒体ｻｲｸﾘｯｸが再開しない
	Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ起動(サイクリック表示はさせる)
// MH810104 GG119202(E) 後決済・決裁なしブランドのカード読み込み失敗時に利用可能媒体ｻｲｸﾘｯｸが再開しない
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド選択結果未選択処理
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@author			A.Shindoh
///	@date			Create	:	2020/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void BrandResultUnselected(void)
{
	// 選択結果エラーの表示
	ec_BrandResultDisp(BRANDNO_UNKNOWN, BrandResRsltTbl.no);
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド選択結果理由コード取得
//[]----------------------------------------------------------------------[]
/// @param			msg    			GetMessage()の戻り値
///	@return			理由コード
///                 該当コードなしの場合はEC_BRAND_REASON_299を返す。
//[]----------------------------------------------------------------------[]
///	@author			A.Shindoh
///	@date			Create	:	2020/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort GetBrandReasonCode(ushort msg)
{
	ushort code;

	switch(msg) {
//	case HOJIN_NEGA_RETRY:
//		if (Hojin_Retry_Flag) {
//			code = EC_BRAND_REASON_203;		// カードの問い合わせ結果を受信できない。
//		} else {
//			code = EC_BRAND_REASON_208;		// 有効なカードであるが使用できない。
//		}
//		break;
//	case HOJIN_NEGA_NG:
//		code = EC_BRAND_REASON_207;			// ネガカードを読み取りした。
//		break;
	default:
		code = EC_BRAND_REASON_299;			// その他
		break;
	}
	return code;
}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| 磁気ｶｰﾄﾞ処理(定期券更新用)                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_card_renewal( msg, mod )                             |*/
/*| PARAMETER    : msg : ﾒｯｾｰｼﾞ                                            |*/
/*|                paymod : 0=通常 1=修正                                  |*/
/*| RETURN VALUE : ret : 0=NG 1=OK                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	al_card_renewal( ushort msg, ushort paymod )
{
	short	ret, rd_snd, anm;
	short	dno;

	ret = 0;
	rd_snd = 2;														// 取出し口移動
	anm = 0;
	dno = 0;

	switch( MAGred[0] ){
		case 0x1a:													// APS定期

			switch( OPECTL.CR_ERR_DSP = (char)al_pasck_renewal( (m_gtapspas *)MAGred ) ){
				case 0:												// OK
					rd_snd = 0;
					ret = 1;
					break;

				case 5:												// 無効定期
					dno = 3;										// "     このカードは無効です     "
					anm = AVM_CARD_ERR3;
					break;

				case 3:												// 期限切れ
				case 6:												// 期限前
					dno = 2;										// "   このカードは期限切れです   "
					anm = AVM_KIGEN_END;
					break;

				case 50:											// 更新期間外
					dno = 11;										// " 更新可能な期間ではありません "
					anm = AVM_CARD_ERR1;
					break;

				default:
					dno = 1;										// "    このカードは使えません    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;

	    default:
			OPECTL.CR_ERR_DSP = 8;									// "   ＜＜ 使用不可券です ＞＞   "
			anm = AVM_CARD_ERR1;
			dno = 1;												// "    このカードは使えません    "
			if( rd_tik == 0 ){										// ﾌﾟﾘﾝﾀﾌﾞﾛｯｸに駐車券無し?
				rd_snd = 13;										// 券取出し口(ﾘｰﾀﾞｰ・ﾌﾟﾘﾝﾀ両方戻し)
			}
			break;
	}

	if( rd_snd ){
		opr_snd( rd_snd );
	}

	if( anm ){
		ope_anm(  anm );								// Card Error Announce
	}

	if( dno ){
		grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[dno] );						// ｴﾗｰ表示は反転文字にて表示
	}
	return( ret );
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| 補充ｶｰﾄﾞﾁｪｯｸ処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : hojuu_card( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*|                                                                        |*/
/*| RETURN VALUE : ret : OK/ NG                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-08-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	hojuu_card( void )
{
	short		ret = OK;	// 戻り値
	short		anm = 0;	// 案内ﾒｯｾｰｼﾞ番号
	m_kakari	*crd_data;	// 磁気ｶｰﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ
	long		pk_no;		// 駐車場Ｎｏ．

	crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];

	// 係員カードチェック
	if(	( crd_data->kkr_idc != 0x41 )
			||
		( crd_data->kkr_did != 0x53 )
			||
		( (MAGred[MAG_GT_APS_TYPE] == 0 ) &&
			( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x41 || crd_data->kkr_park[2] != 0x4b ) ) 
			||
		( (MAGred[MAG_GT_APS_TYPE] == 1 ) &&
			( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x47 || crd_data->kkr_park[2] != 0x54 ) ) ){

		// 係員カード以外
		ret = NG;
		anm = AVM_CARD_ERR1;
	}
	else{
		// 係員カード
		for( ; ; ){
			if( crd_data->kkr_type != 0x4b ){							// カードタイプ＝補充カード？
				// 補充カードでない場合
				ret = NG;
				anm = AVM_CARD_ERR1;
				break;
			}

			if( MAGred[MAG_GT_APS_TYPE] == 1 ){//GTフォーマット
				pk_no = (	( (crd_data->kkr_rsv2[4] & 0x0f) * 100000L ) +		// 駐車場Ｎｏ．取得
							( (crd_data->kkr_rsv2[5] & 0x0f) * 10000L )  +
							( (crd_data->kkr_pno[0] & 0x0f) * 1000L )  +
							( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
							( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
							( (crd_data->kkr_pno[3] & 0x0f) * 1L )
						);
			}else{//APSフォーマット
				pk_no = (	( (crd_data->kkr_pno[0] & 0x0f) * 1000L ) +		// 駐車場Ｎｏ．取得
							( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
							( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
							( (crd_data->kkr_pno[3] & 0x0f) * 1L )
						);
			}

			if( CPrmSS[S_SYS][1] != pk_no ){							// 駐車場Ｎｏ．＝基本駐車場Ｎｏ．？
				// 駐車場Ｎｏ．が一致しない場合
				ret = NG;
				anm = AVM_CARD_ERR1;
				break;
			}
			//GTフォーマットチェック
			if( ( prm_get( COM_PRM,S_SYS,12,1,6 ) == 0 ) && (MAGred[MAG_GT_APS_TYPE] == 1) ||
				( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1 ) && (MAGred[MAG_GT_APS_TYPE] == 0) ){
				//GTフォーマット未対応の場合
				ret = NG;
				anm = AVM_CARD_ERR1;
				break;
			}
			if( (crd_data->kkr_role < 0x30)||(crd_data->kkr_role > 0x34) ){
				// 役割が範囲外の場合
				ret = NG;
				anm = AVM_CARD_ERR1;
				break;
			}
			break;														// 補充カードチェックＯＫ
		}
	}
	if( DOWNLOADING() ){
		ret = NG;
		anm = AVM_CARD_ERR1;
	}
	if( anm ){
		ope_anm(  anm );								// Card Error Announce
	}
	if( ret == OK ){
		ret = (short)(crd_data->kkr_role - 0x30);
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ICｶｰﾄﾞ処理                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_iccard( msg, mod )                                   |*/
/*| PARAMETER    : msg : ﾒｯｾｰｼﾞ                                            |*/
/*|                paymod : 0=通常 1=修正                                  |*/
/*| RETURN VALUE : ret : 精算完了ﾓｰﾄﾞ                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_iccard( ushort msg, ushort paymod )
{
	short	ret;
	short	anm, dno;

	ret = 0;
	anm = 0;
	dno = 0;

	switch( msg ){
		case IBK_MIF_A2_OK_EVT:										// Mifareﾃﾞｰﾀ読出し完了

			switch( OPECTL.CR_ERR_DSP = (char)MifareDataChk() ){
				case 0:												// OK
					if (paymod == 0) {								// 通常精算の場合
						MifareDataWrt();							// Mifare書込み処理
						break;
					}
					if( SKIP_APASS_CHK ){							// 強制ｱﾝﾁﾊﾟｽOFF設定(定期券ﾁｪｯｸ＝しない)？
						OPECTL.Apass_off_seisan = 1;				// 強制ｱﾝﾁﾊﾟｽOFF設定状態で定期使用
					}
					al_pasck_set();							// PayDataに定期情報を書き込み

					ryo_cal( 1, OPECTL.Pr_LokNo );					// 料金計算
					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// 入金中
					}
					ret = in_mony( msg, paymod );
					MifareDataWrt();								// Mifare書込み処理

					if( ryo_buf.zankin ){							// 精算する料金がある場合
					if( !SyuseiEndKeyChk() ){
						read_sht_opn();										/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｵｰﾌﾟﾝ */
						cn_stat( 3, 2 );									/* 入金可 */
					}
					}
					break;

				case 5:												// 無効定期
					dno = 3;										// "     このカードは無効です     "
					anm = AVM_CARD_ERR3;
					break;

				case 3:												// 期限切れ
				case 6:												// 期限前
					dno = 2;										// "   このカードは期限切れです   "
					anm = AVM_KIGEN_END;
					break;

				case 99:											// HOSTへ問合せ
					read_sht_cls();										/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ */
					cn_stat( 2, 2 );									/* 入金不可 */
					NTNET_Snd_Data142( OPECTL.ChkPassPkno, OPECTL.ChkPassID );	// 定期券問合せﾃﾞｰﾀ(ﾃﾞｰﾀ種別142)作成
					grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);		// "                              "
					blink_reg(7, 5, 20, 0, COLOR_DARKSLATEBLUE, &OPE_CHR[7][5]);		// "     しばらくお待ち下さい     "
					Lagtim( OPETCBNO, 6, (ushort)(prm_get( COM_PRM,S_NTN,31,2,1 )*50+1) );	// ﾀｲﾏｰ6(XXs)起動(定期券問合せ監視)
					ret = 0;
					break;
				case 29:											// 問合せタイムアウトNG時
					dno = 1;										// "    このカードは使えません    "
					anm = AVM_CARD_ERR1;
					break;
				case 4:
					dno = 19;										// "       入出庫不一致です       "
					anm = AVM_CARD_ERR1;
					break;

				default:
					dno = 1;										// "    このカードは使えません    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;
		case IBK_MIF_A4_OK_EVT:										// Mifareﾃﾞｰﾀ書込みOK
			if (paymod == 0) {										// 通常精算の場合
				if( SKIP_APASS_CHK ){								// 強制ｱﾝﾁﾊﾟｽOFF設定(定期券ﾁｪｯｸ＝しない)？
					OPECTL.Apass_off_seisan = 1;					// 強制ｱﾝﾁﾊﾟｽOFF設定状態で定期使用
				}
				al_pasck_set();								// PayDataに定期情報を書き込み
				ryo_cal( 1, OPECTL.Pr_LokNo );						// 料金計算

				if( OPECTL.op_faz == 0 ){
					OPECTL.op_faz = 1;								// 入金中
				}
				ac_flg.cycl_fg = 10;								// 入金

				ret = in_mony( IBK_MIF_A2_OK_EVT, paymod );
				if( ryo_buf.zankin ){							// 精算する料金がある場合
				if( !SyuseiEndKeyChk() ){
					read_sht_opn();										/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｵｰﾌﾟﾝ */
					cn_stat( 3, 2 );									/* 入金可 */
				}
				}
				if (ope_imf_GetStatus() == 2) {
					if (is_paid_remote(&PayData) < 0) {
					// 振替精算未実施（定期利用指示）ならここで計上
						ope_ifm_FurikaeCalc( 0 );
						ret = in_mony(IFMPAY_FURIKAE, 0);
						// すでに精算が完了していれば０円振替なのでretは変化しない（上書きok）
					}
				}
			}
			break;
	}

	if( OPECTL.CR_ERR_DSP != 0 && OPECTL.CR_ERR_DSP != 99 ){											// 磁気定期券かつエラー発生 
		Op_StopModuleWait_sub( 2 );						// 電子媒体再活性化要求
	}

	if( anm ){
		ope_anm( anm );								// Card Error Announce
	}

	if( dno ){
		grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[dno] );						// ｴﾗｰ表示は反転文字にて表示
		if (OPECTL.Pay_mod == 0) {
			OPECTL.MIF_CR_ERR = 1;		// Mifare券ﾃﾞｰﾀｴﾗｰ
		}
// MH810105(S) MH364301 インボイス対応
		OPECTL.f_CrErrDisp = 1;			// エラー表示中
// MH810105(E) MH364301 インボイス対応
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 1精算情報ｾｯﾄ                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PayData_set( wflg, typ )                                |*/
/*| PARAMETER    : wflg : 0 = 通常, 1 = 停電                               |*/
/*|              : typ  : 0 = 精算完了, 1 = 精算中止                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PayData_set( uchar wflg, uchar typ )
{
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	ushort	num;
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
	ushort	i;
	uchar	isSet = 0;
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
// MH810100(S) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
	struct clk_rec *pTime;
// MH810100(E) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
// MH810105(S) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
	date_time_rec	pay_time;
// MH810105(E) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう

	if( OPECTL.f_KanSyuu_Cmp )										// 完了集計実施済み（本関数もCallされたということ）
		return;

	PayData.WFlag = wflg;											// 印字種別１（0:通常 1:復電）
	PayData.chuusi = typ;											// 印字種別２（0:領収証 1:精算中止 2:預り証）
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	if( PayData.teiki.update_mon ){									// 更新精算?
//		PayData.WPlace = 9999;										// 更新時は車室番号9999とする。
//	}else if( ryo_buf.pkiti == 0xffff ){							// ﾏﾙﾁ精算?
//		PayData.WPlace = LOCKMULTI.LockNo;							// 接客用駐車位置番号
//// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//	}else if( !OPECTL.Pr_LokNo ){									// 後日精算？
//		PayData.WPlace = OPECTL.Op_LokNo;
//// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//	}else{
//		num = ryo_buf.pkiti - 1;
//		PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// 接客用駐車位置番号
//	}
//	NTNET_Data152Save((void *)(&PayData.WPlace), NTNET_152_WPLACE);
	PayData.WPlace = 0L;
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	PayData.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// 機械№
	PayData.Seisan_kind = 0;										// 精算種別(自動固定)
	if( typ == 0 ){													// 精算完了
		CountGet( PAYMENT_COUNT, &PayData.Oiban );					// 追番
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
	}else{															// 精算中止
		CountGet( CANCEL_COUNT, &PayData.Oiban );					// 追番
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
// MH810100(S) K.Onodera  2020/01/16 車番チケットレス(精算中止時、割引はなかったことにする)
		memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );	// 割引情報クリア
		ryo_buf.waribik	   = 0L;									// 割引額クリア
		ryo_buf.fee_amount = 0L;									// 料金割引 割引額クリア
		ryo_buf.tim_amount = 0L;									// 時間割引 時間数クリア
		ryo_buf.kazei = ryo_buf.tyu_ryo;							// 課税対象額 割引額考慮
		NTNET_Data152_DiscDataClear();								// 精算情報データ用割引詳細エリアクリア
// MH810100(E) K.Onodera  2020/01/16 車番チケットレス(精算中止時、割引はなかったことにする)
	}
	PayData.TInTime.Year = car_in_f.year;							// 入庫 年
	PayData.TInTime.Mon  = car_in_f.mon;							//      月
	PayData.TInTime.Day  = car_in_f.day;							//      日
	PayData.TInTime.Hour = car_in_f.hour;							//      時
	PayData.TInTime.Min  = car_in_f.min;							//      分
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
	PayData.TInTime_Sec  = car_in_f.sec;							//      秒
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
	PayData.TOutTime.Year = car_ot_f.year;							// 出庫 年
	PayData.TOutTime.Mon  = car_ot_f.mon;							//      月
	PayData.TOutTime.Day  = car_ot_f.day;							//      日
	PayData.TOutTime.Hour = car_ot_f.hour;							//      時
	PayData.TOutTime.Min  = car_ot_f.min;							//      分
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
	PayData.TOutTime_Sec  = car_ot_f.sec;							//      秒
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
// MH810100(S) K.Onodera  2020/01/29 車番チケットレス(再精算対応)
// GG124100(S) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
// 	if( OpeNtnetAddedInfo.PayClass == 1 ){	// 再精算
	if ( (OpeNtnetAddedInfo.PayClass == 1) &&						// 再精算
		 (OPECTL.CalStartTrigger == CAL_TRIGGER_LCD_IN_TIME) ) {	// LCDから入庫情報受信
// GG124100(E) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
		PayData.BeforeTPayTime.Year = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear;	// 精算 年
		PayData.BeforeTPayTime.Mon  = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth;	//      月
		PayData.BeforeTPayTime.Day  = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay;	//      日
		PayData.BeforeTPayTime.Hour = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours;	//      時
		PayData.BeforeTPayTime.Min  = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute;	//      分
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
		PayData.BeforeTPayTime_Sec  = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond;	//      秒
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
	}
// MH810100(E) K.Onodera  2020/01/29 車番チケットレス(再精算対応)
// MH810105(S) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
//// MH810105(S) MH364301 インボイス対応
//	PayData.WTaxRate = Disp_Tax_Value( &PayData.TOutTime );			// 適用税率
//// MH810105(E) MH364301 インボイス対応
	pTime = GetPayStartTime();
	// 精算開始の日付をdate_time_recに変換
	pay_time.Year = pTime->year;
	pay_time.Mon  = pTime->mont;
	pay_time.Day  = pTime->date;
	pay_time.Hour  = pTime->hour;
	pay_time.Min  = pTime->minu;
	PayData.WTaxRate = Disp_Tax_Value( &pay_time );			// 適用税率
// MH810105(E) MH364301 インボイス対応 #6406 税率変更基準日に「出場済の車両を精算」で精算完了時に領収証発行すると、領収証印字の消費税率に変更前税率が印字されてしまう
	if( ryo_buf.ryo_flg < 2 ){										// 駐車券精算処理
		PayData.WPrice = ryo_buf.tyu_ryo;							// 駐車料金
	}else{
		PayData.WPrice = ryo_buf.tei_ryo;							// 定期料金
	}
	NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
	/*** 割引類は料金計算内で加算済み ***/
	PayData.syu = (char)(ryo_buf.syubet + 1);						// 駐車種別
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:車種切換後の精算中止時、駐車料金/料金種別を切換前に戻す)
// GG124100(S) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
// 	if( typ == 1 ){													// 精算中止
	if ( (typ == 1) &&												// 精算中止
		 (OPECTL.CalStartTrigger == CAL_TRIGGER_LCD_IN_TIME) ) {	// LCDから入庫情報受信
// GG124100(E) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
		PayData.WPrice = RTPay_Data.crd_info.dtZaishaInfo.ParkingFee;	// 駐車料金を初回料金計算時の料金に戻す
		NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
		PayData.syu = RTPay_Data.crd_info.dtZaishaInfo.shFeeType;		// 料金種別を受信値に戻す(RTPay_Dataの在車情報_料金種別は参照のみの為、受信値のまま)
	}
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:車種切換後の精算中止時、駐車料金/料金種別を切換前に戻す)
	NTNET_Data152Save((void *)(&PayData.syu), NTNET_152_SYUBET);
	PayData.BeforeTwari = ryo_buf.dis_tim;							// 前回時間割引額
	PayData.BeforeRwari = ryo_buf.dis_fee;							// 前回料金割引額
	PayData.BeforePwari = ryo_buf.dis_per;							// 前回％割引金額
	PayData.Wtax = ryo_buf.tax;										// 消費税
	PayData.WInPrice = ryo_buf.nyukin;								// 入金額
	NTNET_Data152Save((void *)(&PayData.WInPrice), NTNET_152_INPRICE);
	PayData.WChgPrice = ryo_buf.turisen;							// 釣銭
	NTNET_Data152Save((void *)(&PayData.WChgPrice), NTNET_152_CHGPRICE);
// 仕様変更(S) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
//	PayData.WTotalPrice = ryo_buf.nyukin - ryo_buf.turisen;			// 現金領収金額
	PayData.WTotalPrice = ryo_buf.nyukin - (ryo_buf.turisen + ryo_buf.kabarai);
// 仕様変更(E) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
	NTNET_Data152Save((void *)(&PayData.WTotalPrice), NTNET_152_CASHPRICE);
	PayData.WFusoku = ryo_buf.fusoku;								// 払戻不足金額
	NTNET_Data152Save((void *)(&PayData.WFusoku), NTNET_152_HARAIMODOSHIFUSOKU);
	memcpy ( &PayData.credit, &ryo_buf.credit, sizeof(credit_use));
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)料金計算
	PayData.zenkai = ryo_buf.zenkai;								// 前回領収額
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)料金計算
// GG129001(S) データ保管サービス対応（課税対象額をセットする）
	PayData.WBillAmount = Billingcalculation(&PayData);				// 請求額
	if (typ == 0 || typ == 1) {
		PayData.WTaxPrice = TaxAmountcalculation(&PayData , typ);	// 課税対象額
		if (PayData.WTaxPrice > PayData.WBillAmount) {
			// 実割引しないの場合、請求額より課税対象額が上回ることがあるので、
			// 請求額に合わせる
			PayData.WTaxPrice = PayData.WBillAmount;
		}
	}
	else {
		PayData.WTaxPrice = 0;
	}
	memcpy ( &PayData.RegistNum, &RegistNum, sizeof(RegistNum));
// GG129001(E) データ保管サービス対応（課税対象額をセットする）

	if( OPECTL.Apass_off_seisan ){									// 強制ｱﾝﾁﾊﾟｽOFF精算あり
		PayData.teiki.Apass_off_seisan = 1;
	}
	PayData.PayCalMax = ntnet_nmax_flg;								// 最大料金越えありフラグセット
	PayData.in_coin[0]	= (uchar)ryo_buf.in_coin[0];	// 投入枚数(10円)
	PayData.in_coin[1]	= (uchar)ryo_buf.in_coin[1];	// 投入枚数(50円)
	PayData.in_coin[2]	= (uchar)ryo_buf.in_coin[2];	// 投入枚数(100円)
	PayData.in_coin[3]	= (uchar)ryo_buf.in_coin[3];	// 投入枚数(500円)
	PayData.in_coin[4]	= (uchar)ryo_buf.in_coin[4];	// 投入枚数(1000円)
	PayData.out_coin[0] = (uchar)ryo_buf.out_coin[0] + ryo_buf.out_coin[4];	// 払出枚数(10円) + 払出枚数10円(予蓄)
	PayData.out_coin[1] = (uchar)ryo_buf.out_coin[1] + ryo_buf.out_coin[5];	// 払出枚数(50円) + 払出枚数50円(予蓄)
	PayData.out_coin[2] = (uchar)ryo_buf.out_coin[2] + ryo_buf.out_coin[6];	// 払出枚数(100円)+ 払出枚数100円(予蓄)
	PayData.out_coin[3] = (uchar)ryo_buf.out_coin[3];	// 払出枚数(500円)
	NTNET_Data152Save((void *)(&PayData.in_coin[0]), NTNET_152_INCOIN);
	NTNET_Data152Save((void *)(&PayData.out_coin[0]), NTNET_152_OUTCOIN);
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
	for( i=0; (i < DETAIL_SYU_MAX) && (0 != PayData.DetailData[i].ParkingNo); i++ ){
		if( PayData.DetailData[i].DiscSyu == NTNET_FUTURE ||
			PayData.DetailData[i].DiscSyu == NTNET_AFTER_PAY ){
			isSet = 1;	// 既に呼び出された
		}
	}
	// セット可？
	if( i != DETAIL_SYU_MAX && isSet == 0 ){
		memset( &PayData.DetailData[i], 0, sizeof(PayData.DetailData[i]) );
		if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE ){
			// 当日精算且つ、支払予定額セット有り？
			if( g_PipCtrl.stRemoteFee.Type == 0 && g_PipCtrl.stRemoteFee.FutureFee ){
				PayData.DetailData[i].ParkingNo = CPrmSS[S_SYS][1];								// 駐車場№
				PayData.DetailData[i].DiscSyu = NTNET_FUTURE;									// 割引種別：後日精算予定額
				PayData.DetailData[i].uDetail.Future.Reserve1 = 1;								// 1固定
				PayData.DetailData[i].uDetail.Future.Total = g_PipCtrl.stRemoteFee.FutureFee;	// 支払予定額
			}
			// 後日精算？
			else if( g_PipCtrl.stRemoteFee.Type == 1 ){
				PayData.DetailData[i].ParkingNo = CPrmSS[S_SYS][1];								// 駐車場№
				PayData.DetailData[i].DiscSyu = NTNET_AFTER_PAY;								// 割引種別：後日精算額
				PayData.DetailData[i].uDetail.AfterPay.Reserve1 = 1;							// 1固定
				PayData.DetailData[i].uDetail.AfterPay.Total = g_PipCtrl.stRemoteFee.Price;		// 支払額
			}
		}
		else if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_TIME ){
			// 支払予定額セット有り？
			if( g_PipCtrl.stRemoteTime.FutureFee ){
				PayData.DetailData[i].ParkingNo = CPrmSS[S_SYS][1];								// 駐車場№
				PayData.DetailData[i].DiscSyu = NTNET_FUTURE;									// 割引種別：後日精算予定額
				PayData.DetailData[i].uDetail.Future.Reserve1 = 1;								// 1固定
				PayData.DetailData[i].uDetail.Future.Total = g_PipCtrl.stRemoteTime.FutureFee;	// 支払予定額
			}
		}
		// PayDataセット有り？
		if( PayData.DetailData[i].ParkingNo ){
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DPARKINGNO, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DSYU, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DNO, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DCOUNT, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DISCOUNT, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DINFO1, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DINFO2, i);
		}
	}
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
// MH810100(S) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
// GG124100(S) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
	if ( OPECTL.CalStartTrigger == CAL_TRIGGER_LCD_IN_TIME ) {			// LCDから入庫情報受信
// GG124100(E) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
		PayData.shubetsu = RTPay_Data.shubetsu;							// 事前/未払種別

		// 精算開始時刻を取得
		pTime = GetPayStartTime();

		PayData.TUnpaidPayTime.Year = pTime->year;						// 精算 年
		PayData.TUnpaidPayTime.Mon  = pTime->mont;						//      月
		PayData.TUnpaidPayTime.Day  = pTime->date;						//      日
		PayData.TUnpaidPayTime.Hour = pTime->hour;						//      時
		PayData.TUnpaidPayTime.Min  = pTime->minu;						//      分
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
		PayData.TUnpaidPayTime_Sec  = pTime->seco;						//      秒
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
// GG124100(S) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
	}
// GG124100(E) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
// MH810100(E) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）
	// 精算時と送信データ作成が同じではないので、送信データ作成時に自動集計など発生するかもしれない
	// 精算ログに精算時に前回T合計時刻を入れて、送信データ作成時にskyの前回T合計時刻を参照しないようにする
	memcpy(&PayData.Before_Ts_Time, &sky.tsyuk.OldTime, sizeof(sky.tsyuk.OldTime));
	PayData.Before_Ts_Time.Sec = 0;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）
}

// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
//[]----------------------------------------------------------------------[]
///	@brief			QR領収証表示
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@author			R.Endo
///	@date			Create	:	2024/12/04<br>
///					Update
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	QrReciptDisp( void )
{
	ushort	dsp_tm;	
	char	qrcode[QRCODE_RECEIPT_SIZE + 4];

	// RT領収証データ登録
	RTReceipt_LogRegist_AddOiban();

	// QR領収証データ作成
	MakeQRCodeReceipt(qrcode, sizeof(qrcode));

	// QR領収証ポップアップ表示
	lcdbm_notice_dsp3(POP_QR_RECIPT, 0, 0, (uchar *)qrcode, sizeof(qrcode));	// 12:QR領収証

	// QRコード発行案内表示タイマー開始
	dsp_tm = (ushort)prm_get(COM_PRM, S_LCD, 96, 2, 1);
	Lagtim(OPETCBNO, TIMERNO_QR_RECEIPT_DISP, (dsp_tm * 50));

	// 精算完了案内タイマー延長
	if ( LagChkExe(OPETCBNO, 1) ) {
		if ( r_zero_call ) {
			dsp_tm = (ushort)prm_get(COM_PRM, S_TYP, 129, 4, 1);
		} else {
			dsp_tm = (ushort)prm_get(COM_PRM, S_TYP, 128, 4, 1);
		}
		Lagtim(OPETCBNO, 1, (dsp_tm * 50));
	}
}
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）

/*[]----------------------------------------------------------------------[]*/
/*| 領収書発行処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ryo_isu( no )                                           |*/
/*| PARAMETER    : no : 0 = 領収書(釣銭不足無し)発行                       |*/
/*|                   : 1 = 領収書(釣銭不足有り)発行                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| 入出庫時刻に固定値car_in_f,car_ot_fを使用する                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ryo_isu( char no )
{
	MsgBuf	*msb;
	T_FrmReceipt	ReceiptPreqData;								// ＜領収証印字要求＞ﾒｯｾｰｼﾞﾃﾞｰﾀ
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
// // GG129004(S) R.Endo 2024/11/19 電子領収証対応
// 	char	qrcode[QRCODE_RECEIPT_SIZE + 4];
// // GG129004(E) R.Endo 2024/11/19 電子領収証対応
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）

	if( OPECTL.RECI_SW != (char)-1 ){
		PayData_set( 0, 0 );										// 1精算情報ｾｯﾄ
	}

// MH810105(S) MH364301 インボイス対応
//	if( no == 1 || ReceiptChk() == 0 ){								// ﾚｼｰﾄ発行可?
// GG129000(S) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）
//	if(( no == 1 || ReceiptChk() == 0)
	if(( no == 1 || QRIssueChk(1) != 0 || ReceiptChk() == 0)		// QR駐車券発行あり？ レシート発行可設定あり？
// GG129000(E) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 	&& (Ope_isPrinterReady() == 1) ){								// ﾚｼｰﾄ発行可?
	&& (IS_ERECEIPT || (Ope_isPrinterReady() == 1)) ){				// ﾚｼｰﾄ発行可?
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// MH810105(E) MH364301 インボイス対応

// MH810105(S) MH364301 インボイス対応
		if (IS_INVOICE) {											// ｲﾝﾎﾞｲｽ?
			if( OPECTL.RECI_SW == (char)-1 ){						// 延長中での領収証発行
				if (OPECTL.f_DelayRyoIsu != 2) {
					// インボイス設定ありでジャーナル印字完了していない場合は
					// 延長発行しない
					return;
				}
				else if (OPECTL.f_ReIsuType != 0) {
					// 領収証再発行要求受付中は延長発行しない
					return;
				}
			}
			else{													// 延長ではない
				if (Ope_isJPrinterReady() == 0) {
					// インボイス設定時はレシートとジャーナルが片方でも
					// 動かなかった場合は領収証を印字しない
					return;
				}
				else if (OPECTL.f_DelayRyoIsu != 2) {
					// ジャーナル印字完了後にレシート印字を行う
					OPECTL.f_DelayRyoIsu = 1;						// ジャーナル印字完了待ち
					return;
				}
			}
		}
// MH810105(E) MH364301 インボイス対応

// GG129004(S) R.Endo 2024/12/11 #7560 電子領収証を発行する設定で領収証ボタンを押下してQRコードを表示させると、領収証取り出し口のLEDが点灯する
// 		LedReq( CN_TRAYLED, LED_ON );								// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON
// GG129004(E) R.Endo 2024/12/11 #7560 電子領収証を発行する設定で領収証ボタンを押下してQRコードを表示させると、領収証取り出し口のLEDが点灯する

		ryo_buf.ryos_fg = 1;										// 領収書発行ｾｯﾄ

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
		if ( prm_get(COM_PRM, S_RTP, 58, 1, 1) == 1 ) {	// 17-0058 領収証印字先 電子領収証
			if ( OPECTL.f_KanSyuu_Cmp ) {	// 完了集計実施
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
// 				// RT領収証データ登録
// 				RTReceipt_LogRegist_AddOiban();
// 
// 				// QR領収証データ作成
// 				MakeQRCodeReceipt(qrcode, sizeof(qrcode));
// 
// 				// QR領収証ポップアップ表示
// 				lcdbm_notice_dsp3(POP_QR_RECIPT, 0, 0, (uchar *)qrcode, sizeof(qrcode));	// 12:QR領収証
				// QR領収証表示
				QrReciptDisp();
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
			} else {
				// 完了集計でRT領収証データの作成が必須のため実施前であれば実施後に予約
				OPECTL.f_eReceiptReserve = 1;
			}
		} else {
// GG129004(S) R.Endo 2024/12/11 #7560 電子領収証を発行する設定で領収証ボタンを押下してQRコードを表示させると、領収証取り出し口のLEDが点灯する
			LedReq(CN_TRAYLED, LED_ON);								// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON
// GG129004(E) R.Endo 2024/12/11 #7560 電子領収証を発行する設定で領収証ボタンを押下してQRコードを表示させると、領収証取り出し口のLEDが点灯する
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

		if( ( msb = GetBuf() ) == NULL ){
			if( OPECTL.RECI_SW != (char)-1 ){
				kan_syuu();											// 精算完了集計
			}
			return;
		}
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
		if( ryo_stock == 1 ){										// ストック中
			// ストックありの場合レシートデータはすでに送信しているため
			// 印字開始のみ行う
			MsgSndFrmPrn(PREQ_STOCK_PRINT, R_PRI, 0);				// データ書き込み終了（印字開始）
			ryo_stock = 0xff;										// ストック印字完了
		}
		else if( ryo_stock == 0 ){									// ストックなし
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129002(S) ゲート式車番チケットレスシステム対応（新プリンタ印字処理不具合修正）
		memset(&ReceiptPreqData, 0, sizeof(ReceiptPreqData));
// GG129002(E) ゲート式車番チケットレスシステム対応（新プリンタ印字処理不具合修正）
		msb->msg.command = PREQ_RYOUSYUU;							// 送信ｺﾏﾝﾄﾞ：領収証印字要求
		ReceiptPreqData.prn_kind = R_PRI;							// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
// MH810105(S) MH364301 インボイス対応（ジャーナル印字タイミング変更）
		// UT-4000に合わせてレシート印字のみ要求する
		// ジャーナル印字はLog_registで要求する
//		if( OPECTL.RECI_SW != (char)-1 && PrnJnlCheck() == ON ){
//			//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
//			switch(CPrmSS[S_RTP][1]){	// ｼﾞｬｰﾅﾙ印字条件？
//				case	0:
//				case	1:
//				case	2:
//					// ｼﾞｬｰﾅﾙは０円時も記録
//					ReceiptPreqData.prn_kind = RJ_PRI;
//					break;
//				case	3:
//				case	4:
//				case	5:
//					// ｼﾞｬｰﾅﾙは駐車料金０円時以外記録
//					if( PayData.WPrice != 0 ){
//						//	駐車料金≠０
//						ReceiptPreqData.prn_kind = RJ_PRI;
//					}
//					break;
//
//				case	6:
//				case	7:
//				case	8:
//					// ｼﾞｬｰﾅﾙは現金領収０円時以外記録
//					if( PayData.WTotalPrice != 0 ){
//						//	現金領収額≠０
//						ReceiptPreqData.prn_kind = RJ_PRI;
//					}
//					break;
//				case	9:
//				case   10:
//				case   11:
//// MH321800(S) Y.Tanizaki ICクレジット対応
////					if( PayData.WTotalPrice != 0 || PayData.Electron_data.Suica.e_pay_kind ){
//					if( PayData.WTotalPrice != 0 ||
//						SuicaUseKindCheck(PayData.Electron_data.Suica.e_pay_kind) ||
//						EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ){
//// MH321800(E) Y.Tanizaki ICクレジット対応
//						//	現金領収額≠０ OR 電子媒体による精算が行われた場合
//						ReceiptPreqData.prn_kind = RJ_PRI;
//					}
//					break;
//				default:
//					ReceiptPreqData.prn_kind = R_PRI;
//			}
//		}
// MH810105(E) MH364301 インボイス対応（ジャーナル印字タイミング変更）
// MH810104 GG119201(S) 電子ジャーナル対応
//		Pri_Work[0].Receipt_data = PayData;
//		ReceiptPreqData.prn_data = &Pri_Work[0].Receipt_data;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
		memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
		ReceiptPreqData.prn_data = &Cancel_pri_work;			// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
// MH810104 GG119201(E) 電子ジャーナル対応
		ReceiptPreqData.reprint = OFF;								// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）

		memcpy( &msb->msg.data, &ReceiptPreqData, sizeof(T_FrmReceipt) );	// 送信ﾃﾞｰﾀｾｯﾄ

		PutMsg( PRNTCBNO, msb );									// ﾌﾟﾘﾝﾀﾀｽｸへ印字要求ﾒｯｾｰｼﾞ送信

// MH322914 (s) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
		ryo_inji = 1;												// 領収証印字中
// MH322914 (e) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
		}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
		}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

		sky.tsyuk.Ryosyuu_pri_cnt += 1L;							// 領収書発行枚数+1
		if(OPECTL.Pr_LokNo != 0xffff){
			syusei[OPECTL.Pr_LokNo-1].infofg |= SSS_RYOUSYU;
		}
	}

	if( OPECTL.RECI_SW != (char)-1 ){
		kan_syuu();													// 精算完了集計
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 領収証発行可／不可判定処理                                             |*/
/*|																		   |*/
/*|	PayDataセット前に ryo_buf を見て判定。								   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ReceiptChk2( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0=領収証印字可, 1=領収証印字不可                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2007/04/03                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	ReceiptChk( void )
{
	uchar	ret = 1;												// ﾚｼｰﾄ印字不可

// MH810105(S) MH364301 インボイス対応
//	switch( CPrmSS[S_RTP][1] ){										// ﾚｼｰﾄ印字条件
	long rec_pri_jadge = prm_get( COM_PRM, S_RTP, 1, 2, 1 );		// 17-0001の値を変数に保存

	rec_pri_jadge = invoice_prt_param_check( rec_pri_jadge );		// パラメータの変換を行う。

	switch( rec_pri_jadge ){										// ﾚｼｰﾄ印字条件
// MH810105(E) MH364301 インボイス対応
		case 0:
		case 3:
		case 6:
		case 9:
// GG129001(S) 領収証プリント設定の読み替え対応
		case 12:
// GG129001(E) 領収証プリント設定の読み替え対応
			// ﾚｼｰﾄは0円時も記録
			ret = 0;												// ﾚｼｰﾄ印字可
			break;

		case 1:
		case 4:
		case 7:
		case 10:
// GG129001(S) 領収証プリント設定の読み替え対応
		case 13:
// GG129001(E) 領収証プリント設定の読み替え対応
			if( ((ryo_buf.ryo_flg < 2) && (ryo_buf.tyu_ryo != 0)) ||
				((ryo_buf.ryo_flg >= 2) && (ryo_buf.tei_ryo != 0)) ){
				// 駐車料金≠0
				ret = 0;											// ﾚｼｰﾄ印字可
			}
			break;

		case 2:
		case 5:
		case 8:
		case 11:
// GG129001(S) 領収証プリント設定の読み替え対応
		case 14:
// GG129001(E) 領収証プリント設定の読み替え対応
			// ﾚｼｰﾄは現金領収0円時以外記録
			//if( PayData.WTotalPrice != 0 ){
			if( (ryo_buf.nyukin - ryo_buf.turisen) != 0){
				// 現金領収額≠0
				ret = 0;											// ﾚｼｰﾄ印字可
			}
			break;

		default:
			ret = 1;												// ﾚｼｰﾄ印字不可
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 中止処理                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : chu_isu( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	chu_isu( void )
{
	if( Chu_Syu_Status )
		return;

	if(OPECTL.op_faz == 3){											// 取消ボタンが押下された場合
		PayData_set( 0, 1 );										// 1精算情報ｾｯﾄ
		azukari_isu(1);												// 精算中止時の預り証発行
		cyu_syuu();													// 精算中止集計
		Chu_Syu_Status = 1;
	}else{
		PayData_set( 0, 0 );										// 1精算情報ｾｯﾄ
		azukari_isu(0);												// 精算時の預り証発行
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			預り証発行処理
//[]----------------------------------------------------------------------[]
///	@param[in]		cancel 	: 0:精算, 1:精算中止
///	@return			None
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	:	2008/08/25
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	azukari_isu(uchar cancel)
{
	T_FrmAzukari	AzukariPreqData;									// ＜預り証印字要求＞ﾒｯｾｰｼﾞﾃﾞｰﾀ
//// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
//	ePOP_DISP_KIND	DispCode;	// ポップアップ表示種別
//	uchar	DispStatus;			// 表示ステータス
//// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

	if( ryo_buf.fusoku != 0 ){
		if(CPrmSS[S_SYS][44] != 2){									//預り証設定が２の場合且つはジャーナルだけに出力する
			LedReq( CN_TRAYLED, LED_ON );							// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON

			memset(&AzukariData, 0, sizeof(AzukariData));
			AzukariData.WFlag = 0;									// 印字種別１（0:通常 1:復電）
			AzukariData.chuusi = cancel;							// 精算中止かどうか
			if( !OPECTL.multi_lk )									// ﾏﾙﾁ精算問合せ中以外?
				AzukariData.Lok_No = OPECTL.Pr_LokNo;				// ロック装置No
			else
				AzukariData.Lok_No = (ushort)(LOCKMULTI.LockNo%10000);	// ロック装置No

			AzukariData.WPlace = OPECTL.Op_LokNo;					// 接客用駐車位置番号
			AzukariData.Kikai_no = (uchar)CPrmSS[S_PAY][2];			// 機械№
			
// MH810100(S) Y.Yamauchi 2020/02/17 車番チケットレス（預り証印字修正）
			AzukariData.CarSearchFlg = PayData.CarSearchFlg;		// 車番/日時検索種別
			memcpy( AzukariData.CarSearchData, PayData.CarSearchData, sizeof(AzukariData.CarSearchData) );		// 車番(入力)
			memcpy( AzukariData.CarNumber, PayData.CarNumber, sizeof(AzukariData.CarNumber) );		// 車番(精算)
			memcpy( AzukariData.CarDataID, PayData.CarDataID, sizeof(AzukariData.CarDataID) );		// 車番データID
// MH810100(E) Y.Yamauchi 2020/02/17 車番チケットレス（預り証印字修正）
			CountGet( DEPOSIT_COUNT, &AzukariData.Oiban );			// 追番
			AzukariData.Seisan_kind = 0;							// 精算種別(自動固定)
			AzukariData.Oiban2 = PayData.Oiban;						// 精算／中止追番
			AzukariData.TTime.Year = PayData.TOutTime.Year;			// 精算 年
			AzukariData.TTime.Mon  = PayData.TOutTime.Mon;			//      月
			AzukariData.TTime.Day  = PayData.TOutTime.Day;			//      日
			AzukariData.TTime.Hour = PayData.TOutTime.Hour;			//      時
			AzukariData.TTime.Min  = PayData.TOutTime.Min;			//      分
			AzukariData.WTotalPrice = ryo_buf.dsp_ryo;				// 請求額		※合計＝（駐車料金－割引料金）

			AzukariData.WInPrice = ryo_buf.nyukin;					// 入金額		※お預り
// 仕様変更(S) K.Onodera 2016/11/07 集計基本データフォーマット対応
//			AzukariData.WChgPrice = ryo_buf.turisen - ryo_buf.fusoku;	// 払戻金額	※払戻額＝（釣銭金額－払出不足額）
			AzukariData.WChgPrice = (ryo_buf.turisen+ryo_buf.kabarai) - ryo_buf.fusoku;	// 払戻金額	※払戻額＝（釣銭金額+過払い－払出不足額）
// 仕様変更(E) K.Onodera 2016/11/07 集計基本データフォーマット対応
			AzukariData.Syusei = 0;									// 修正精算あり
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//			if(OPECTL.Pay_mod == 2){	//修正精算時は釣銭金額ではなくて修正払戻額とする
//				AzukariData.Syusei = 1;								// 修正精算あり
//				if(( PayData.MMTwari )&&(!(PayData.Zengakufg&0x02))){	// 修正元入金額の払戻あり
//					AzukariData.Syusei = 2;							// 修正元払戻有り
//					AzukariData.WChgPrice = PayData.MMTwari			// 修正払戻金額(払戻すべき金額-払出せなかった金額)
//												- ryo_buf.fusoku;
//				}
//			}
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
			AzukariData.WFusoku = ryo_buf.fusoku;					// 払戻不足金額	※払出不足額

			AzukariPreqData.prn_kind = R_PRI;						// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
			AzukariPreqData.prn_data = (Azukari_data *)&AzukariData;// 預り証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ

			queset( PRNTCBNO, PREQ_AZUKARI, sizeof( T_FrmAzukari ), &AzukariPreqData );	// 預り証発行

			sky.tsyuk.Azukari_pri_cnt += 1L;						// 預り証発行回数+1
// MH810100(S) 2020/05/27 車番チケットレス(#4181)
//// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
//			// ﾎﾟｯﾌﾟｱｯﾌﾟ表示要求(精算中に発生する表示要求) & 対応する音声案内を流す
//			//	code;						status															音声				ﾒｯｾｰｼﾞ
//			//	1:預り証発行(精算完了時)	0:釣銭なし, 1:釣銭あり, 2:釣銭なし(領収証), 3:釣銭あり(領収証)	0,2=35／1,3＝33		0,1=22/2,3=24
//			//	2:預り証発行(払い戻し時)	0:釣銭なし, 1:釣銭あり, 2:釣銭なし(領収証), 3:釣銭あり(領収証)	0,2=35／1,3＝33		26
//			// cancel	0 = 精算時の預り証発行		1 = 精算中止時の預り証発行
//			// ■OPECTL.RECI_SW		// 1 = 領収書ﾎﾞﾀﾝ使用	0 = 領収書ﾎﾞﾀﾝ未使用
//			// ■OPECTL.Fin_mod		// 0 = 同額		1 = 釣り無し		2 = 釣り有り	3 = ﾒｯｸｴﾗｰ(釣り有り)
//			// 1:預り証発行(精算完了時)	2:預り証発行(払い戻し時)
//			// 精算時の預り証発行
//			if (cancel == 0) {
//				DispCode = POP_AZUKARI_PAY;
//			}
//			// 精算中止時の預り証発行
//			else {
//				DispCode = POP_AZUKARI_RET;
//			}
//			// 0 = 同額, 1 = 釣り無し, 2 = 釣り有り, 3 = ﾒｯｸｴﾗｰ(釣り有り)
//			switch (OPECTL.Fin_mod) {
//				case 0:
//				case 1:
//				default:
//					// 1 = 領収書ﾎﾞﾀﾝ使用	0 = 領収書ﾎﾞﾀﾝ未使用
//					if (OPECTL.RECI_SW == 0) {
//						DispStatus = 0;		// 釣銭なし
//					}
//					else {
//						DispStatus = 2;		// 釣銭なし(領収証)
//					}
//					break;
//				case 2:
//				case 3:
//					if (OPECTL.RECI_SW == 0) {
//						DispStatus = 1;		// 釣銭あり
//					}
//					else {
//						DispStatus = 3;		// 釣銭あり(領収証)
//					}
//					break;
//			}
//			// ポップアップ表示要求(預り証発行)
//			lcdbm_notice_dsp( DispCode, DispStatus );
//// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
			azukari_popup(cancel);
// MH810100(E) 2020/05/27 車番チケットレス(#4181)
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 受付券発行処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : uke_isu( op_no, pr_no, re_pri )                         |*/
/*| PARAMETER    : op_no : 接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999)  |*/
/*|                pr_no : 内部処理用駐車位置番号(1～324)                  |*/
/*|                re_pri: 発行種別(0:通常／1：再発行／2：メンテ初回発行)  |*/
/*| RETURN VALUE : ret   : 0=発行OK  1=発行済みNG                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-09-06                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	uke_isu( unsigned long op_no, unsigned short pr_no, unsigned char re_pri )
{
	T_FrmUketuke	UketukePreqData;								// ＜受付券印字要求＞ﾒｯｾｰｼﾞﾃﾞｰﾀ

	unsigned char		remote;
	remote = (unsigned char)(re_pri & 0xf0);
	re_pri &= 0x0f;
// レビュー指摘対応(S) K.Onodera 2016/10/04 受付券は上限枚数に関係なく発行し、再発行とする
//	if( (FLAPDT.flp_data[pr_no-1].issue_cnt < (uchar)prm_get(COM_PRM, S_TYP, 62, 1, 3)) || re_pri == 1 ){	// 限度枚数以下 or 再発行
	if( (FLAPDT.flp_data[pr_no-1].issue_cnt < (uchar)prm_get(COM_PRM, S_TYP, 62, 1, 3)) || re_pri == 1 || remote == 0x20 ){	// 限度枚数以下 or 再発行 or Parkipro要求
// レビュー指摘対応(E) K.Onodera 2016/10/04 受付券は上限枚数に関係なく発行し、再発行とする
		FLAPDT.flp_data[pr_no-1].issue_cnt++;						// 駐車証明書発行回数更新
		FLAPDT.flp_data[pr_no-1].uketuke = 1;						// 発行済みｾｯﾄ
		LedReq( CN_TRAYLED, LED_ON );								// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON

		memset( &UketukeData, 0, sizeof( Uketuke_data ) );

		UketukeData.Lok_No = pr_no;
		UketukeData.WPlace = op_no;									// 駐車位置№
		UketukeData.Kikai_no = (uchar)CPrmSS[S_PAY][2];				// 機械№
		UketukeData.PrnCnt = FLAPDT.flp_data[pr_no-1].issue_cnt;	// 駐車証明書発行回数
		if( re_pri == 1 ){											// 再発行?
			UketukeData.Oiban = UketukeNoBackup[pr_no-1];			// 再発行追番
			memcpy( &UketukePreqData.PriTime, &CLK_REC, sizeof(date_time_rec) );	// 再発行日時（現在日時）ｾｯﾄ
			UketukeData.ISTime.Year = FLAPDT.flp_data[pr_no-1].u_year;	// 発行 年
			UketukeData.ISTime.Mon  = FLAPDT.flp_data[pr_no-1].u_mont;	//      月
			UketukeData.ISTime.Day  = FLAPDT.flp_data[pr_no-1].u_date;	//      日
			UketukeData.ISTime.Hour = FLAPDT.flp_data[pr_no-1].u_hour;	//      時
			UketukeData.ISTime.Min  = FLAPDT.flp_data[pr_no-1].u_minu;	//      分
		}else{														// 通常発行及びメンテナンス初回発行
			UketukeData.Oiban = CountRead_Individual( INFO_COUNT );		// 追番
			UketukeNoBackup[pr_no-1] = UketukeData.Oiban;			// 発行追番ﾊﾞｯｸｱｯﾌﾟ
			memcpy( &UketukeData.ISTime, &CLK_REC, sizeof(date_time_rec) );	// 発行 年月日時分
			FLAPDT.flp_data[pr_no-1].u_year = UketukeData.ISTime.Year;	// 発行 年保存
			FLAPDT.flp_data[pr_no-1].u_mont = UketukeData.ISTime.Mon;	//      月保存
			FLAPDT.flp_data[pr_no-1].u_date = UketukeData.ISTime.Day;	//      日保存
			FLAPDT.flp_data[pr_no-1].u_hour = UketukeData.ISTime.Hour;	//      時保存
			FLAPDT.flp_data[pr_no-1].u_minu = UketukeData.ISTime.Min;	//      分保存
		}
		if( re_pri != 0 ){												// 通常以外は係員No.を印字
			if (remote == 0)
				UketukePreqData.kakari_no = OPECTL.Kakari_Num;			// 係員No.
			else
				UketukePreqData.kakari_no = 99;							// 係員No.
		}
		UketukeData.TTime.Year = FLAPDT.flp_data[pr_no-1].year;		// 入庫 年
		UketukeData.TTime.Mon  = FLAPDT.flp_data[pr_no-1].mont;		//      月
		UketukeData.TTime.Day  = FLAPDT.flp_data[pr_no-1].date;		//      日
		UketukeData.TTime.Hour = FLAPDT.flp_data[pr_no-1].hour;		//      時
		UketukeData.TTime.Min  = FLAPDT.flp_data[pr_no-1].minu;		//      分
		if(( (prm_get( COM_PRM,S_TYP,63,1,1 ) == 2) ||
			 (prm_get( COM_PRM,S_TYP,63,1,1 ) == 4) ) &&
			 (remote == 0) )
		{
			UketukeData.Pword = FLAPDT.flp_data[pr_no-1].passwd;	// ﾊﾟｽﾜｰﾄﾞ
		}

		if (re_pri == 1) {											// 再発行
			// 再発行ならﾚｼｰﾄのみ
			UketukePreqData.prn_kind = R_PRI;						// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
		} else {
			// 再発行でなければ設定に依存
			if (prm_get(COM_PRM, S_PRN, 17, 1, 4) == 1) {
				UketukePreqData.prn_kind = RJ_PRI;					// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ&ｼﾞｬｰﾅﾙ
			} else {
				UketukePreqData.prn_kind = R_PRI;					// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
			}
		}
		UketukePreqData.prn_data = (Uketuke_data *)&UketukeData;	// 受付券印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ
		UketukePreqData.reprint  = re_pri;							// 発行種別ｾｯﾄ
		queset( PRNTCBNO, PREQ_UKETUKE, sizeof( T_FrmUketuke ), &UketukePreqData );	// 受付券発行

		if( re_pri != 1 ){											// 再発行以外?
			CountUp_Individual( INFO_COUNT );						// 受付券発行追番+1

			sky.tsyuk.Uketuke_pri_Tcnt += 1L;						// 受付券発行回数+1
			loktl.tloktl.loktldat[pr_no-1].Uketuke_pri_cnt += 1;	// 駐車位置別受付券発行回数+1

		}
		LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// 電子決済中止領収証発行時の取り出し口LED消灯ﾀｲﾏｰ
		return( 0 );												// 発行OK
	}else{
		return( 1 );												// 発行済みNG
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 受付券、ﾊﾟｽﾜｰﾄﾞ登録チェック処理                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : uke_isu( pr_no )                                        |*/
/*| PARAMETER    : pr_no : 内部処理用駐車位置番号(1～324)                  |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*| RETURN VALUE : ret   : 0=発行NG  1=発行OK                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.Sekiguchi                                             |*/
/*| Date         : 2006-09-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	pri_time_chk( unsigned short pr_no, uchar type )
{
ushort	ltim;

	struct	CAR_TIM	car_in_time;					//入庫時間格納
	struct	CAR_TIM	pri_chk_time;					//現在（発行）時間格納
	short	pritim;									//入庫～発行時間格納
	
	pri_chk_time.year	 = CLK_REC.year;			// 現在 年保存
	pri_chk_time.mon	 = CLK_REC.mont;			//      月保存
	pri_chk_time.day	 = CLK_REC.date;			//      日保存
	pri_chk_time.hour	 = CLK_REC.hour;			//      時保存
	pri_chk_time.min	 = CLK_REC.minu;			//      分保存

	car_in_time.year	= FLAPDT.flp_data[pr_no-1].year;		// 入庫 年
	car_in_time.mon		= FLAPDT.flp_data[pr_no-1].mont;		//      月
	car_in_time.day		= FLAPDT.flp_data[pr_no-1].date;		//      日
	car_in_time.hour	= FLAPDT.flp_data[pr_no-1].hour;		//      時
	car_in_time.min		= FLAPDT.flp_data[pr_no-1].minu;		//      分

	pritim = (short)ec71( &car_in_time,&pri_chk_time );			//入庫～発行間の時間取得

	if( type == 0 ){				// 受付券発行
		ltim = (ushort)prm_get( COM_PRM,S_TYP,99,3,4 );
	}else{							// 暗証番号登録
		ltim = (ushort)prm_get( COM_PRM,S_TYP,99,3,1 );
	}
	if( (ltim != 0) && ( pritim > ltim ) ){				//発行可能チェック
		return( 0 );												// 発行NG
	}else{
		return( 1 );												// 発行OK
	}

}

/*[]----------------------------------------------------------------------[]*/
/*| ﾒﾝﾃﾅﾝｽ用ﾊﾟｽﾜｰﾄﾞ毎の役割取得処理                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mtpass_get( uchar inp, uchar *pass )                    |*/
/*| PARAMETER    : inp   : ﾊﾟｽﾜｰﾄﾞ入力桁数                                 |*/
/*|              : *pass : ﾊﾟｽﾜｰﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ                                |*/
/*| RETURN VALUE : ret : 役割  1 = 係員操作                                |*/
/*|                            2 = 管理者操作                              |*/
/*|                            3 = 技術員操作(ｱﾏﾉ技術員専用)               |*/
/*|                           -1 = ﾊﾟｽﾜｰﾄﾞ不一致                           |*/
/*| ＜備考＞                                                               |*/
/*|    入力ﾊﾟﾗﾒｰﾀで与えられたﾊﾟｽﾜｰﾄﾞが設定ﾃｰﾌﾞﾙに存在した場合、            |*/
/*|    OPECTL.Kakari_Numにﾊﾟｽﾜｰﾄﾞに対応する係員No.をｾｯﾄする                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const unsigned char		SYS_PAS_ENG[8] = { 'C', 4, 3, 9, 1, 5, 3, 1 };
const unsigned char		SYS_PAS_DEV[9] = { 'C', 3, 1, 1, 3, 1, 5, 5, 7 };

char	mtpass_get( uchar inp, uchar *pass )
{
	short	s_wk;
	char	i;
	char	yakuwari = (char)-1;
	char	kakari_no;
	uchar	c_key = OFF;
	char	passlevel = 0;

	if( memcmp( SYS_PAS_DEV, pass, 9 ) == 0 ){
		// 入力ﾊﾟｽﾜｰﾄﾞ＝Ｃ３１１３１５５７

		yakuwari	= 4;												// 開発者操作ﾚﾍﾞﾙ
		kakari_no	= 99;												// 係員No.ｾｯﾄ
		passlevel	= (char)-1;											// ロック解除しない
	}
	else if( memcmp( SYS_PAS_ENG, &pass[1], 8 ) == 0 ){					// ﾊﾟｽﾜｰﾄﾞ(C4391531)一致?

		// 入力ﾊﾟｽﾜｰﾄﾞ＝Ｃ４３９１５５７

		yakuwari	= 3;												// 技術員操作ﾚﾍﾞﾙ
		kakari_no	= 99;												// 係員No.ｾｯﾄ
		passlevel	= (char)-1;											// ロック解除しない
	}
	else{
		// 入力ﾊﾟｽﾜｰﾄﾞ≠Ｃ４３９１５３１

		if( inp == 0 ){
			// ﾊﾟｽﾜｰﾄﾞ入力なし
			for( i = 0; i < 10; i++ ){									// ﾊﾟｽﾜｰﾄﾞ設定ﾃｰﾌﾞﾙ検索
				if(	(CPrmSS[S_PSW][(i*2)+1] == 0L) 
						&&
					(CPrmSS[S_PSW][(i*2)+2] != 0) ){					// ﾊﾟｽﾜｰﾄﾞなし（0000）且つ　役割≠０？
					
					// ﾊﾟｽﾜｰﾄﾞなし（0000）且つ　役割≠０
					if( (yakuwari < (char)CPrmSS[S_PSW][(i*2)+2]) || yakuwari == 0xff ){
						// 前検索ﾊﾟｽﾜｰﾄﾞなし(0000)の役割より高い役割の場合

						yakuwari	= (char)prm_get( COM_PRM, S_PSW, (i*2)+2, 1, 1);	// 役割ｾｰﾌﾞ
						passlevel	= (char)prm_get( COM_PRM, S_PSW, (i*2)+2, 1, 2);	// 役割ｾｰﾌﾞ
						kakari_no	= (char)(i+1);						// 係員No.ｾｰﾌﾞ
					}
				}
			}
		}
		else{

			// ﾊﾟｽﾜｰﾄﾞ入力あり

			s_wk = 0;
			for( i = 5; i < 9; i++ ){
				if( pass[i] == 'C' ){
					// １～４桁目に「Ｃ」キーが入力されている場合
					c_key = ON;											// 「Ｃ」ｷｰ入力あり
				}
				s_wk *= 10;
				s_wk += (short)pass[i];
			}

			if( c_key == OFF ){											// １～４桁目に「Ｃ」ｷｰが入力されていない場合のみﾊﾟｽﾜｰﾄﾞ比較を行う

				for( i = 0; i < 10; i++ ){								// ﾊﾟｽﾜｰﾄﾞ設定ﾃｰﾌﾞﾙ検索
					if(	(CPrmSS[S_PSW][(i*2)+1] == 0L)
							||
						(CPrmSS[S_PSW][(i*2)+2] == 0)){
						// ﾊﾟｽﾜｰﾄﾞ設定なし(0000)　または　役割＝０　の場合
						continue;
					}
					if( s_wk == (short)CPrmSS[S_PSW][(i*2)+1] ){		// ﾊﾟｽﾜｰﾄﾞ一致?
						// ﾊﾟｽﾜｰﾄﾞ一致
						yakuwari	= (char)prm_get( COM_PRM, S_PSW, (i*2)+2, 1, 1);	// ﾒﾝﾃﾅﾝｽの役割(ﾚﾍﾞﾙ)取得
						passlevel	= (char)prm_get( COM_PRM, S_PSW, (i*2)+2, 1, 2);	// 役割ｾｰﾌﾞ
						kakari_no	= (char)(i+1);						// ﾊﾟｽﾜｰﾄﾞに対応した係員No.ｾｯﾄ
						break;
					}
				}
			}
		}
	}
	if( yakuwari != (char)-1 ){
		// ﾊﾟｽﾜｰﾄﾞが設定ﾃｰﾌﾞﾙに存在した場合
		OPECTL.Kakari_Num = kakari_no;									// 係員No.ｾｯﾄ
		OPECTL.PasswordLevel = passlevel;
	}
	return( yakuwari );
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券出庫時刻ﾃｰﾌﾞﾙ読出し                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PassExitTimeTblRead()                                   |*/
/*| PARAMETER    : pas : 駐車場№                                          |*/
/*|                pid : ID = 1 ～ 12000                                   |*/
/*|                date_time_rec : ﾃｰﾌﾞﾙに登録されている出庫時刻[out]      |*/
/*| RETURN VALUE : ret :  0 = ﾃｰﾌﾞﾙ登録無し, 1 = ﾃｰﾌﾞﾙ登録有り             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	PassExitTimeTblRead( ulong pas, ushort pid, date_time_rec *exittime )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	ushort	i;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
	char	ret = 0;
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	for( i=0; i<pas_extimtbl.Count; i++ ){
//		if( pas == pas_extimtbl.PassExTbl[i].ParkNo && pid == pas_extimtbl.PassExTbl[i].PassId ){
//			// 駐車場№と定期券IDが同一
//			memcpy( exittime, &pas_extimtbl.PassExTbl[i].PassTime, sizeof( date_time_rec ) );
//			ret = 1;
//			break;
//		}
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券出庫時刻ﾃｰﾌﾞﾙ書込み                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PassExitTimeTblWrite()                                  |*/
/*| PARAMETER    : pas : 駐車場№                                          |*/
/*|                pid : ID = 1 ～ 12000                                   |*/
/*|                date_time_rec : ﾃｰﾌﾞﾙに登録する出庫時刻                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PassExitTimeTblWrite( ulong pas, ushort pid, date_time_rec *exittime )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	/* 定期券出庫時刻テーブルに該当データが存在していれば削除 */
//	PassExitTimeTblDelete(pas, pid);
//	
//	/* 時刻データALL0なら削除のみ実行 */
//	if (exittime->Year != 0 || exittime->Mon != 0 || exittime->Day != 0 ||
//		exittime->Hour != 0 || exittime->Min != 0) {
//		if (pas_extimtbl.Count >= PASS_EXTIMTBL_MAX) {
//			/* テーブル最大件数登録済みの場合は、最古のデータ(index=0)を削除する */
//			PassExitTimeTblDelete((ulong)pas_extimtbl.PassExTbl[0].ParkNo, pas_extimtbl.PassExTbl[0].PassId);
//		}
//		/* 最後尾にデータを登録する */
//		pas_extimtbl.PassExTbl[pas_extimtbl.Count].PassTime = *exittime;
//		pas_extimtbl.PassExTbl[pas_extimtbl.Count].ParkNo = pas;
//		pas_extimtbl.PassExTbl[pas_extimtbl.Count].PassId = pid;
//		pas_extimtbl.Count++;
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券出庫時刻ﾃｰﾌﾞﾙ削除                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PassExitTimeTblDelete()                                 |*/
/*| PARAMETER    : pas : 駐車場№                                          |*/
/*|                pid : ID = 1 ～ 12000                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PassExitTimeTblDelete( ulong pas, ushort pid )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	ushort	i, hit;
//
//	hit = 0;
//
//	for( i=0; i<pas_extimtbl.Count; i++ ){
//		if( pas == pas_extimtbl.PassExTbl[i].ParkNo && pid == pas_extimtbl.PassExTbl[i].PassId ){
//			// 駐車場№と定期券IDが同一
//			hit = 1;
//			break;
//		}
//	}
//
//	if( hit ){
//		memmove( &pas_extimtbl.PassExTbl[i],
//				 &pas_extimtbl.PassExTbl[i+1],
//				 sizeof( PASS_EXTBL )*(PASS_EXTIMTBL_MAX-i) );		// 削除(ｼﾌﾄ)
//		memset( &pas_extimtbl.PassExTbl[PASS_EXTIMTBL_MAX-1], 0, sizeof( PASS_EXTBL ) );
//		pas_extimtbl.Count--;
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}
typedef struct {
	unsigned long	mnt_st;
	unsigned long	mnt_ed;
	unsigned long	nt_st;
	unsigned long	now;
	unsigned long	hikaku;
	unsigned long	jikan_wk[3];	// ローカライズ時間（上位２byte=day 下位２byte=fun）
	char			eigyo_wk[3];	// それぞれの戻り値格納
	char			cstat_wk[3];	// 休業の要因格納
} nrmlyz;
nrmlyz NRMLYZ;
//[]-----------------------------------------------------------------------[]
// ノーマライズ日とノーマライズ分で、ノーマライズ日時を作成する				|
//																			|
//	norm_day(in) : ノーマライズ日											|
//  norm_min(in) : ノーマライズ分											|
//  Return:																	|
//			上位16ビット＝日 下位16ビット＝分								|
//																			|
//  T.Okamoto	2006/08/31	Create											|
//[]-----------------------------------------------------------------------[]
//  大小比較のための変換である、正確な差分はこれでは計算できない			|
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]
ulong time_nrmlz ( ushort norm_day, ushort norm_min )
{
	ulong		retval;
	retval = (ulong)norm_day;
	retval <<= 16;
	retval += (ulong)(norm_min);
	return retval;
}
/*[]----------------------------------------------------------------------[]*/
/*| 営業／休業ﾁｪｯｸ                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opncls()                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 1 = 営業(Open) / 2 = 休業(Close)                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	opncls( void )
{
	int		cnt;			// カウンタ
	int		i,target;
	short	sts;
// MH322914 (s) kasiyama 2016/07/11 トラブル信号で休業中に強制営業できるのを修正[共通バグNo.1249](MH341106)
	int		bufferfull = 0;
// MH322914 (e) kasiyama 2016/07/11 トラブル信号で休業中に強制営業できるのを修正[共通バグNo.1249](MH341106)

	sts = 0;

// MH322914 (s) kasiyama 2016/07/11 トラブル信号で休業中に強制営業できるのを修正[共通バグNo.1249](MH341106)
	/** バッファFULL判定 **/
	if(( prm_get( COM_PRM,S_NTN,41,1,1 ) == 2 )&&				// NT-NET 精算ﾃﾞｰﾀ送信ﾊﾞｯﾌｧFULL時休業? かつ
	   (_is_ntnet_remote())){ 									// 遠隔NT-NETの設定有効
		if(IsErrorOccuerd(prm_get(COM_PRM, S_CEN, 51, 1, 3) ? ERRMDL_LANIBK : ERRMDL_FOMAIBK, ERR_RAU_PAYMENT_BUFERFULL)) {
			// バッファフルが発生していれば休業
			bufferfull = 1;
		}
		else {
			// バッファフル解除状態なら発生しているなら発生状態に変更する
			Log_CheckBufferFull(TRUE, eLOG_PAYMENT, eLOG_TARGET_REMOTE);
		}
	}
// MH322914 (e) kasiyama 2016/07/11 トラブル信号で休業中に強制営業できるのを修正[共通バグNo.1249](MH341106)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
//	else if(( prm_get( COM_PRM,S_NTN,41,1,1 ) == 2 )&&			// NT-NET 精算ﾃﾞｰﾀ送信ﾊﾞｯﾌｧFULL時休業? かつ
	if(( prm_get( COM_PRM,S_SSS,41,1,1 ) == 2 )&&				// NT-NET 精算ﾃﾞｰﾀ送信ﾊﾞｯﾌｧFULL時休業? かつ
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
	   (_is_ntnet_normal())){ 									// NT-NETの設定有効
		if(IsErrorOccuerd(ERRMDL_NTNET, ERR_NTNET_ID22_BUFFULL)) {
			// バッファフルが発生していれば休業
			bufferfull = 1;
		}
		else {
			// バッファフル解除状態でバッファフル状態なら発生状態に変更する
			Log_CheckBufferFull(TRUE, eLOG_PAYMENT, eLOG_TARGET_NTNET);
		}
	}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

	/** ﾄﾗﾌﾞﾙ **/
	if( opncls_TrbClsChk() ){									// 休業とするﾄﾗﾌﾞﾙ要因有り
		sts = 2;
		CLOSE_stat = 7;
	}
// MH322914 (s) kasiyama 2016/07/11 トラブル信号で休業中に強制営業できるのを修正[共通バグNo.1249](MH341106)
//	/** ﾊﾞｯﾌｧFULL **/
//	if(( prm_get( COM_PRM,S_NTN,41,1,1 ) == 2 )&&				// NT-NET 精算ﾃﾞｰﾀ送信ﾊﾞｯﾌｧFULL時休業? かつ
//	   (_is_ntnet_remote())){ 									// 遠隔NT-NETの設定有効
//		if(IsErrorOccuerd(prm_get(COM_PRM, S_CEN, 51, 1, 3) ? ERRMDL_LANIBK : ERRMDL_FOMAIBK, ERR_RAU_PAYMENT_BUFERFULL)) {
//			// バッファフルが発生していれば休業
//			sts = 2;
//			CLOSE_stat = 8;										// NT-NET 送信ﾊﾞｯﾌｧFULL
//		}
//		else {
//			// バッファフル解除状態なら発生しているなら発生状態に変更する
//			Log_CheckBufferFull(TRUE, eLOG_PAYMENT, eLOG_TARGET_REMOTE);
//		}
//	}
	else if( bufferfull == 1 ) {
		sts = 2;
		CLOSE_stat = 8;										// NT-NET 送信ﾊﾞｯﾌｧFULL
	}
// MH322914 (e) kasiyama 2016/07/11 トラブル信号で休業中に強制営業できるのを修正[共通バグNo.1249](MH341106)
	/** 強制休業信号 **/
	else if( OPECTL.opncls_kyugyo == 1 ){						// 強制休業の接点入力有り
		sts = 2;
		CLOSE_stat = 10;										// 強制営休業(信号)
	}
	/** 強制営業信号 **/
	else if( OPECTL.opncls_eigyo == 1 ){						// 強制営業の接点入力有り
		CLOSE_stat = 10;										// 強制営休業(信号)
	}
	/** ﾕｰｻﾞｰﾒﾝﾃﾅﾝｽ **/
	else if( PPrmSS[S_P01][2] != 0 ){							// ﾕｰｻﾞｰﾒﾝﾃﾅﾝｽにおける強制営休業切換有り
		/*** 強制営休業 ***/
		sts = (short)PPrmSS[S_P01][2];
		CLOSE_stat = 1;											// 強制営休業
	}
	/** ｺｲﾝ金庫満杯 **/
	// 半自動精算モード時はｺｲﾝ満杯でも休業としない
	else if( (cinful() == -1) ){								// ｺｲﾝ金庫満杯で半自動ﾓｰﾄﾞでもない
		sts = 2;
		CLOSE_stat = 4;											// ｺｲﾝ金庫満杯
	}
	/** 紙幣金庫満杯 **/
	else if( (prm_get( COM_PRM,S_SYS,38,1,1 ) == 1) &&			// 紙幣金庫満杯時休業にする設定で
			 (notful() == -1) ){								// 紙幣金庫満杯
			sts = 2;
			CLOSE_stat = 3;										// 紙幣金庫満杯
	}
	/** 釣銭切れ **/
	else if( (prm_get( COM_PRM,S_SYS,38,1,2 ) == 1) &&			// 釣銭切れ時休業にする設定で
			 (Ex_portFlag[EXPORT_CHGEND] == 1)){ 				// 釣銭切れ
			sts = 2;
			CLOSE_stat = 6;										// 休業理由＝釣銭切れ
	}
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//// MH810105(S) MH364301 インボイス対応
//	/** レシート／ジャーナル紙切れ時に休業 **/
//	else if( ( IS_INVOICE &&									// インボイス適用時
//			  prm_get( COM_PRM, S_SYS, 16, 1, 1 ) == 1) &&		// レシート／ジャーナル紙切れ時に休業にする設定で
//			 ((Ope_isPrinterReady() == 0) || 					// レシート出力不可または
//			  (Ope_isJPrinterReady() == 0))) {					// ジャーナル(電子ジャーナル)出力不可
//		sts = 2;
//		CLOSE_stat = 15;										// 休業理由＝レシート／ジャーナル紙切れ
//	}
//// MH810105(E) MH364301 インボイス対応
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
	else{


	/*** 通信による営休業 or 営休業時間指定 ***/

		memset(&NRMLYZ,0,sizeof(NRMLYZ));
		//設定時間を完全ノーマライズ
		NRMLYZ.mnt_st = time_nrmlz ( (ushort)dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ),
								   (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_SYS][40]/100), (short)(CPrmSS[S_SYS][40]%100) ) );
		NRMLYZ.mnt_ed = time_nrmlz ( (ushort)dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ),
								   (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_SYS][41]/100), (short)(CPrmSS[S_SYS][41]%100) ) );
		//NTNET指示時間を完全ノーマライズ
		NRMLYZ.nt_st = time_nrmlz( FLAGPT.receive_rec.flg_ocd, FLAGPT.receive_rec.flg_oct );
		//現在時間を完全ノーマライズ
		NRMLYZ.now = time_nrmlz ( CLK_REC.ndat, CLK_REC.nmin );
		//設定時間大小変換
		if( NRMLYZ.mnt_st > NRMLYZ.mnt_ed ){ //EX>now 3:00 ,Start 8:00 , End 4:00
			if( NRMLYZ.mnt_ed > NRMLYZ.now ){
				//開始を前日にする
				NRMLYZ.mnt_st = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) - 1),
										 (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_SYS][40]/100), (short)(CPrmSS[S_SYS][40]%100) ) );
			}
		}else if( NRMLYZ.mnt_st < NRMLYZ.mnt_ed ){ //EX> now 1:00 , Start 2:00 , End 10:00 
			if( NRMLYZ.mnt_st > NRMLYZ.now ){
				//終了を前日にする
				NRMLYZ.mnt_ed = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) - 1),
									     (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_SYS][41]/100), (short)(CPrmSS[S_SYS][41]%100) ) );
			}
		}else if( NRMLYZ.mnt_st == NRMLYZ.mnt_ed ){
			NRMLYZ.mnt_st = 0L;
			NRMLYZ.mnt_ed = 0L;
		}

		// それぞれ現在より過去の指定を eigyo_wk その他　に格納し、有効な数を cnt する.
		cnt = 0;
		if( (NRMLYZ.mnt_st != 0L) && (NRMLYZ.mnt_st <= NRMLYZ.now) ){
			NRMLYZ.jikan_wk[cnt] = NRMLYZ.mnt_st;
			NRMLYZ.eigyo_wk[cnt] = 1;									//営業		(戻り値)
			NRMLYZ.cstat_wk[cnt] = 0;									//営業中	(CLOSE_stat)
			cnt ++;
		}
		if( (NRMLYZ.mnt_ed != 0L) && (NRMLYZ.mnt_ed <= NRMLYZ.now) ){
			NRMLYZ.jikan_wk[cnt] = NRMLYZ.mnt_ed;
			NRMLYZ.eigyo_wk[cnt] = 2;									//休業		(戻り値)
			NRMLYZ.cstat_wk[cnt] = 2;									//営業時間外(CLOSE_stat)
			cnt ++;
		}
		if( (NRMLYZ.nt_st != 0L) && (NRMLYZ.nt_st <= NRMLYZ.now) ){
			NRMLYZ.jikan_wk[cnt] = NRMLYZ.nt_st;
			NRMLYZ.eigyo_wk[cnt] = (char)FLAGPT.receive_rec.flg_ock ;	//NTNETの指示(戻り値)
			// NT-NETによる指示が強制営休業
			NRMLYZ.cstat_wk[cnt] = 11;
			cnt ++;
		}

		// 一番大きい時間 ( == 一番現在から近い)　指定はなにか
		if( cnt != 0 ){
			for(i=0; i<cnt; i++){
				if( NRMLYZ.hikaku <= NRMLYZ.jikan_wk[i] ){
					NRMLYZ.hikaku = NRMLYZ.jikan_wk[i];
					target = i;
				}
			}
			sts = (short)NRMLYZ.eigyo_wk[target];
			CLOSE_stat = NRMLYZ.cstat_wk[target];
		}

		// 営業の場合、通信による強制営業か営業時間内かの判定ﾌﾗｸﾞをｾｯﾄする
		if (sts != 2) {
			if (CLOSE_stat == 11) {
				// NT-NETによる強制指示
				OPEN_stat = OPEN_NTNET;
			} else {
				// 営業時間内
				OPEN_stat = OPEN_TIMESET;
			}
		}
	}
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//// MH321800(S) D.Inaba ICクレジット対応(営休業時間指定したときに休業とならないバグ修正移植)
//	if (isEC_USE() != 0) {						// 決済リーダ接続設定あり
//	// ※休業とはしない
//		if ((prm_get(COM_PRM, S_ECR, 1, 6, 1) == 0)
//		&&	(prm_get(COM_PRM, S_ECR, 2, 1, 6) == 0)
//		&&	(prm_get(COM_PRM, S_ECR, 2, 2, 1) == 0)) {
//		// 有効ブランド設定なし
//			err_chk((char)jvma_setup.mdl, ERR_EC_SETTEI_BRAND_NG, 1, 0, 0);			// E3264:ｴﾗｰﾛｸﾞ登録（登録）
//			Suica_Rec.suica_err_event.BIT.BRANDSETUP_NG = 1;						// 設定誤りﾌﾗｸﾞを登録
//		}
//	}
//// MH321800(E) D.Inaba ICクレジット対応(営休業時間指定したときに休業とならないバグ修正移植)
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない

	if( sts != 2 ){
		sts = 1;
		CLOSE_stat = 0;
	}
	/****************************************/
	/* CLOSE_stat = 0:営業中				*/
	/*              1:強制営休業			*/
	/*              2:営業時間外			*/
	/*              3:紙幣金庫満杯			*/
	/*              4:ｺｲﾝ金庫満杯			*/
	/*              5:紙幣ﾘｰﾀﾞｰ脱落			*/
	/*              6:釣銭切れ				*/
	/*              7:ｺｲﾝﾒｯｸ通信不良他		*/
	/*              8:NT-NET 送信ﾊﾞｯﾌｧFULL	*/
	/*              9:ﾌﾗｯﾌﾟ/ﾛｯｸ装置手動ﾓｰﾄﾞ	*/
	/*             10:信号で休業指示		*/
	/*             11:通信で休業指示		*/
// MH810105(S) MH364301 インボイス対応
	/*             15:ﾚｼｰﾄ/ｼﾞｬｰﾅﾙ紙切れ		*/
// MH810105(E) MH364301 インボイス対応
	/****************************************/

	return( sts );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾄﾗﾌﾞﾙ信号要因（ｴﾗｰ/ｱﾗｰﾑ）での営業／休業ﾁｪｯｸ                            |*/
/*[]----------------------------------------------------------------------[]*/
/*|	ｴﾗｰ/ｱﾗｰﾑｺｰﾄﾞ表にて、休業とするｴﾗｰ/ｱﾗｰﾑの発生有無を返す。			   |*/
/*|	無条件で休業とする項目の発生状況は Err_cls, Alm_clsにｾｯﾄされており、   |*/
/*|	その他の条件付項目はここで全数ﾁｪｯｸする。							   |*/
/*|																		   |*/
/*|	休業要因は優先順位もあり複雑な判定となる。							   |*/
/*| 本関数では上記 opncls() 関数の一部を抜き出したものであり、これより優先 |*/
/*|	度の高い条件（金庫満杯）と低い条件（釣銭切れ）はﾁｪｯｸ対象外とする。	   |*/
/*|	またNT-NET通信ﾊﾞｯﾌｧFullはﾄﾗﾌﾞﾙ要因ではないので、これも対象外とする。   |*/
/*|																		   |*/
/*|	現状条件付休業となるｴﾗｰ/ｱﾗｰﾑは 上記に示した対象外項目となる。		   |*/
/*| 本関数は将来条件付休業要因を増やした場合のために用意する。			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opncls_TrbClsChk()                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 2 = 休業要因有り、0=なし                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2005-12-09                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	short	opncls_TrbClsChk( void )
{
	/* 無条件休業ｴﾗｰ/ｱﾗｰﾑ発生中 */
	if( Err_cls || Alm_cls ){
		return	(short)2;
	}

	return	(short)0;
}

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| ﾗﾍﾞﾙﾌﾟﾘﾝﾀ関連のｴﾗｰの有無を知る                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LprnErrChk()                                            |*/
/*| PARAMETER    : char offline = 0 : ｵﾌﾗｲﾝｴﾗｰをﾁｪｯｸしない                 |*/
/*|                             = 1 : ｵﾌﾗｲﾝｴﾗｰをﾁｪｯｸする                   |*/
/*| RETURN VALUE : ret : 0=ｴﾗｰなし  1=ｴﾗｰあり                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-03-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
char	LprnErrChk( char offline )
{
	if( ERR_CHK[mod_lprn][ERR_LPRN_COMFAIL]		||					// 通信タイムアウト
		ERR_CHK[mod_lprn][ERR_LPRN_CUTTER]		||					// カッターエラー
		ERR_CHK[mod_lprn][ERR_LPRN_HEADOPEN]	||					// ヘッドオープン
		ERR_CHK[mod_lprn][ERR_LPRN_HEAD]		||					// ヘッドエラー
		ERR_CHK[mod_lprn][ERR_LPRN_SENSOR]		||					// センサエラー
		ERR_CHK[mod_lprn][ERR_LPRN_BUFF_FULL]	||					// バッファオーバー
		ERR_CHK[mod_lprn][ERR_LPRN_MEDIA]		||					// メディアエラー
		ERR_CHK[mod_lprn][ERR_LPRN_OTHER]		||					// その他のエラー
		ALM_CHK[ALMMDL_SUB][ALARM_LBPAPEREND]	||					// ラベル紙切れ
		ALM_CHK[ALMMDL_SUB][ALARM_LBRIBBONEND] )					// ラベルリボン切れ
	{
		return( 1 );
	}
	if( offline ){
		if( ERR_CHK[mod_lprn][ERR_LPRN_OFFLINE] ){					// 通信オフライン
			return( 1 );
		}
	}
	return( 0 );
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| ﾗﾍﾞﾙ発行枚数-1                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LabelCountDown( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-04-20                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	LabelCountDown( void )
{
	if( PPrmSS[S_P01][10] > 0L ){
		PPrmSS[S_P01][10]--;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算完了信号出力                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PayEndSig()                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PayEndSig( void )
{
	uchar	set;

	if( ryo_buf.ryo_flg >= 2 ){										// 定期使用?
		set = (uchar)prm_get( COM_PRM,S_PAS,(short)(4+10*(PayData.teiki.syu-1)),1,2 );	// 定期券毎の信号出力設定
	}else{
		set = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+(ryo_buf.syubet*6)),1,2 );		// 種別毎の信号出力設定
	}

	if(( set == 0 )||( set == 2 )){
		OutSignalCtrl( SIG_OUTCNT1, SIG_ONOFF );					// 精算完了信号1出力
	}
	if(( set == 1 )||( set == 2 )){
		OutSignalCtrl( SIG_OUTCNT2, SIG_ONOFF );					// 精算完了信号2出力
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 追番ｶｳﾝﾄ+1 個別追番用                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CountUp_Individual( kind )                              |*/
/*| PARAMETER    : kind = 追番の種類                                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : yanagawa                                                |*/
/*| Date         : 2012/10/25                                              |*/
/*| UpDate       : 関数名を変更しただけ                                    |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	CountUp_Individual( uchar kind )
{
	PPrmSS[S_P03][kind]++;
	if( PPrmSS[S_P03][kind] > 999999L ){
		PPrmSS[S_P03][kind] = 1L;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 追番ｶｳﾝﾄ読出し 個別追番用                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CountRead_Individual( kind )                            |*/
/*| PARAMETER    : kind = 追番の種類                                       |*/
/*| RETURN VALUE : ret  = 各種追番                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : yanagawa                                                |*/
/*| Date         : 2012/10/25                                              |*/
/*| UpDate       : 関数名を変更しただけ                                    |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	CountRead_Individual( uchar kind )
{
	return(ulong)( PPrmSS[S_P03][kind] );
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ステータスデータ取得関数                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPassData                                             |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort PassId 定期券ID　1～12000                        |*/
/*| RETURN VALUE : PAS_TBL*                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
PAS_TBL *GetPassData( ulong ParkingNo, ushort PassId)
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	long index;
//	PAS_TBL *pcdpt;
//
//	if( PassId > 0 ){
//		PassId--;
//	}else{
//		return(NULL);
//	}
//
//	if( (ulong)CPrmSS[S_SYS][65] == ParkingNo ){
//		if( CPrmSS[S_SYS][61] )		index = CPrmSS[S_SYS][61];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - 1 ) ){							// ﾃｰﾌﾞﾙ①の範囲内?
//			pcdpt = &pas_tbl[PassId];									// ﾃｰﾌﾞﾙ①
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][66] == ParkingNo ){
//		if( CPrmSS[S_SYS][62] )		index = CPrmSS[S_SYS][62];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - CPrmSS[S_SYS][61] ) ){			// ﾃｰﾌﾞﾙ②の範囲内?
//			pcdpt = &pas_tbl[ PassId + CPrmSS[S_SYS][61] - 1 ];			// ﾃｰﾌﾞﾙ②
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][67] == ParkingNo ){
//		if( CPrmSS[S_SYS][63] )		index = CPrmSS[S_SYS][63];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - CPrmSS[S_SYS][62] ) ){			// ﾃｰﾌﾞﾙ③の範囲内?
//			pcdpt = &pas_tbl[ PassId + CPrmSS[S_SYS][62] - 1 ];			// ﾃｰﾌﾞﾙ③
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][68] == ParkingNo ){
//		if( PassId < (ushort)( PAS_MAX + 1 - CPrmSS[S_SYS][63] ) ){		// ﾃｰﾌﾞﾙ④の範囲内?
//			pcdpt = &pas_tbl[ PassId + CPrmSS[S_SYS][63] - 1 ];			// ﾃｰﾌﾞﾙ④
//		}else{
//			return(NULL);
//		}
//	}else{
//		return(NULL);
//	}
//	
//	return pcdpt;
	return(NULL);
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ステータス書き込み関数                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : WritePassTbl                                            |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort PassId 定期券ID　1～12000                        |*/
/*| PARAMETER    : ushort Status ステータス　0～15,0xffff=書込まない       |*/
/*| PARAMETER    : ulong UseParking 利用中駐車場№　駐車場№1～999 1001～260000,0xffffffff=書込まない|*/
/*| PARAMETER    : ushort Valid 有効無効　0=有効,1=無効,0xffff=書込まない  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void WritePassTbl( ulong ParkingNo, ushort PassId, ushort Status, ulong UseParking, ushort Valid )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	PAS_TBL *pass_data;
//	uchar use_parking;
//	
//	pass_data = GetPassData(ParkingNo, PassId);
//	
//	if (pass_data == NULL) {
//		return;
//	}
//
//	if (UseParking != 0xFFFFFFFF) {
//		use_parking = NTNET_GetParkingKind((ulong)UseParking, PKOFS_SEARCH_MASTER);
//		if (use_parking == 0xFF) {
//			return;
//		}
//	}
//
//	if (Status != 0xFFFF) {
//		pass_data->BIT.STS = Status;
//	}
//	
//	if (UseParking != 0xFFFFFFFF) {
//		pass_data->BIT.PKN = use_parking;
//	}
//	
//	if (Valid != 0xFFFF) {
//		pass_data->BIT.INV = Valid;
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ステータスデータ取得関数                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPassData                                             |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort *Start                                           |*/
/*| PARAMETER    : ushort *End                                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Start = 0～11999，End=1～12000                           MH544401追記  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void GetPassArea( ulong ParkingNo, ushort *Start, ushort *End)
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	*Start = 12000;
//	*End = 0;
//
//	if (ParkingNo == CPrmSS[S_SYS][65]) {
//		*Start	= 0;
//		*End	= (ushort)CPrmSS[S_SYS][61];
//	} else if (ParkingNo == CPrmSS[S_SYS][66]) {
//		*Start	= (ushort)(CPrmSS[S_SYS][61] - 1);
//		*End	= (ushort)CPrmSS[S_SYS][62];
//	} else if (ParkingNo == CPrmSS[S_SYS][67]) {
//		*Start	= (ushort)(CPrmSS[S_SYS][62] - 1);
//		*End	= (ushort)CPrmSS[S_SYS][63];
//	} else if (ParkingNo == CPrmSS[S_SYS][68]) {
//		*Start	= (ushort)(CPrmSS[S_SYS][63] - 1);
//		*End	= 12000;
//	}
//
//	if (*End == 0) {
//		*End = 12000;
//	}
//	if (*End != 12000) {
//		*End -= 1;
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ステータス書き込み関数（１領域）                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FillPassTbl                                             |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort Status ステータス　0～15,0xffff=書込まない       |*/
/*| PARAMETER    : ulong UseParking 利用中駐車場№　駐車場№1～999 1001～260000,0xffffffff=書込まない|*/
/*| PARAMETER    : ushort Valid 有効無効　0=有効,1=無効,0xffff=書込まない  |*/
/*| PARAMETER    : ushort FillFlag 0:全データ更新　　1:有効データのみ更新  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void FillPassTbl( ulong ParkingNo, ushort Status, ulong UseParking, ushort Valid, ushort FillType )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	ushort i;
//	ushort Start;
//	ushort End;
//	uchar use_parking;
//
//	GetPassArea(ParkingNo, &Start, &End);
//	
//	use_parking = NTNET_GetParkingKind((ulong)UseParking, PKOFS_SEARCH_MASTER);
//
//	if (Start >= End) {
//		return;
//	}
//
//	if (FillType == 1) {
//		for (i = Start; i < End; i++) {
//			if (pas_tbl[i].BIT.INV == 0) {
//				pas_tbl[i].BIT.INV = Valid;
//				pas_tbl[i].BIT.STS = Status;
//				if (UseParking != 0xFFFFFFFF) {
//					pas_tbl[i].BIT.PKN = use_parking;
//				}
//			}
//		}
//	} else if (FillType == 2) {
//		for (i = Start; i < End; i++) {
//			if (pas_tbl[i].BIT.INV == 0 && pas_tbl[i].BIT.STS == 1) {
//				pas_tbl[i].BIT.STS = 0;
//				if (UseParking != 0xFFFFFFFF) {
//					pas_tbl[i].BIT.PKN = use_parking;
//				}
//			}
//		}
//	} else if (FillType == 3) {
//		for (i = Start; i < End; i++) {
//			if (pas_tbl[i].BIT.INV == 0 && pas_tbl[i].BIT.STS == 2) {
//				pas_tbl[i].BIT.STS = 0;
//				if (UseParking != 0xFFFFFFFF) {
//					pas_tbl[i].BIT.PKN = use_parking;
//				}
//			}
//		}
//	} else {
//		for (i = Start; i < End; i++) {
//			pas_tbl[i].BIT.INV = Valid;
//			if (Status != 0xFFFF) {
//				pas_tbl[i].BIT.STS = Status;
//			}
//			if (UseParking != 0xFFFFFFFF) {
//				pas_tbl[i].BIT.PKN = use_parking;
//			}
//		}
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}


/*[]----------------------------------------------------------------------[]*/
/*| 定期券ステータス書き込み関数                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ReadPassTbl                                             |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort PassId 定期券ID　1～12000                        |*/
/*| PARAMETER    : ushort *PassData 定期券データ                           |*/
/*| RETURN VALUE : short 0:OK -1:NG                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short ReadPassTbl( ulong ParkingNo, ushort PassId, ushort *PassData )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	PAS_TBL *pass_data;
//	
//	pass_data = GetPassData(ParkingNo, PassId);
//	
//	if (pass_data == NULL) {
//		return -1;
//	}
//
//	PassData[0] = (ushort)pass_data->BIT.INV;	// 0:有効, 1:無効
//	PassData[1] = (ushort)pass_data->BIT.STS;	// 0:初期状態, 1:出庫中, 2,入庫中
//	PassData[2] = (ushort)pass_data->BIT.PKN;	// 利用中駐車場 0=基本, 1=拡張1, 2=拡張2, 3=拡張3
//
//	return 0;
	PassData[0] = 0;
	PassData[1] = 0;
	PassData[2] = 0;

	return 0;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券更新ステータスデータ取得関数                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPassRenewalData                                      |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort PassId 定期券ID　1～12000                        |*/
/*| RETURN VALUE : PAS_RENEWAL*                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
PAS_RENEWAL *GetPassRenewalData( ulong ParkingNo, ushort PassId, char *pposi)
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	long index;
//	PAS_RENEWAL *pcdpt;
//
//	if( PassId > 0 ){
//		PassId--;
//	}else{
//		return(NULL);
//	}
//
//	*pposi = (char)-1;
//	if( (ulong)CPrmSS[S_SYS][65] == ParkingNo ){
//		if( CPrmSS[S_SYS][61] )		index = CPrmSS[S_SYS][61];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - 1 ) ){							// ﾃｰﾌﾞﾙ①の範囲内?
//			pcdpt = &pas_renewal[PassId/4];									// ﾃｰﾌﾞﾙ①
//			*pposi = (char)(PassId%4);											// ﾃｰﾌﾞﾙ①
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][66] == ParkingNo ){
//		if( CPrmSS[S_SYS][62] )		index = CPrmSS[S_SYS][62];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - CPrmSS[S_SYS][61] ) ){			// ﾃｰﾌﾞﾙ②の範囲内?
//			pcdpt = &pas_renewal[ (PassId + CPrmSS[S_SYS][61] - 1)/4 ];		// ﾃｰﾌﾞﾙ②
//			*pposi = (char)((PassId + CPrmSS[S_SYS][61] - 1)%4);			// ﾃｰﾌﾞﾙ②
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][67] == ParkingNo ){
//		if( CPrmSS[S_SYS][63] )		index = CPrmSS[S_SYS][63];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - CPrmSS[S_SYS][62] ) ){			// ﾃｰﾌﾞﾙ③の範囲内?
//			pcdpt = &pas_renewal[ (PassId + CPrmSS[S_SYS][62] - 1)/4 ];		// ﾃｰﾌﾞﾙ③
//			*pposi = (char)((PassId + CPrmSS[S_SYS][62] - 1)%4);			// ﾃｰﾌﾞﾙ③
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][68] == ParkingNo ){
//		if( PassId < (ushort)( PAS_MAX + 1 - CPrmSS[S_SYS][63] ) ){		// ﾃｰﾌﾞﾙ④の範囲内?
//			pcdpt = &pas_renewal[ (PassId + CPrmSS[S_SYS][63] - 1)/4 ];		// ﾃｰﾌﾞﾙ④
//			*pposi = (char)((PassId + CPrmSS[S_SYS][63] - 1)%4);			// ﾃｰﾌﾞﾙ④
//		}else{
//			return(NULL);
//		}
//	}else{
//		return(NULL);
//	}
//	
//	return pcdpt;
	return(NULL);
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}
/*[]----------------------------------------------------------------------[]*/
/*| 定期券更新ステータス取得関数                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ReadPassRenewalTbl                                      |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort PassId 定期券ID　1～12000                        |*/
/*| PARAMETER    : ushort *PassData 定期券データ                           |*/
/*| RETURN VALUE : short 0:OK -1:NG                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short ReadPassRenewalTbl( ulong ParkingNo, ushort PassId, ushort *PassData )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	char  pass_posi;
//	PAS_RENEWAL *pass_data;
//
//	pass_data = GetPassRenewalData(ParkingNo, PassId, &pass_posi );
//
//	if (pass_data == NULL) {
//		return -1;
//	}
//
//	switch( pass_posi ){
//
//		case 0:
//			PassData[0] = (ushort)pass_data->BIT.Bt01;
//			break;
//		case 1:
//			PassData[0] = (ushort)pass_data->BIT.Bt23;
//			break;
//		case 2:
//			PassData[0] = (ushort)pass_data->BIT.Bt45;
//			break;
//		case 3:
//			PassData[0] = (ushort)pass_data->BIT.Bt67;
//			break;
//	}
//	return 0;
	return 0;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ステータス書き込み関数                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : WritePassRenewalTbl                                     |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort PassId 定期券ID　1～12000                        |*/
/*| PARAMETER    : ushort Status ステータス　0～15,0xffff=書込まない       |*/
/*| PARAMETER    : ushort UseParking 利用中駐車場№　駐車場№1～999,0xffff=書込まない|*/
/*| PARAMETER    : ushort Valid 有効無効　0=有効,1=無効,0xffff=書込まない  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void WritePassRenewalTbl( ulong ParkingNo, ushort PassId, ushort Valid )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	char  pass_posi;
//	PAS_RENEWAL *pass_data;
//	
//	pass_data = GetPassRenewalData(ParkingNo, PassId, &pass_posi);
//	
//	if (pass_data == NULL) {
//		return;
//	}
//
//	switch( pass_posi ){
//
//		case 0:
//			pass_data->BIT.Bt01 = Valid;
//			break;
//		case 1:
//			pass_data->BIT.Bt23 = Valid;
//			break;
//		case 2:
//			pass_data->BIT.Bt45 = Valid;
//			break;
//		case 3:
//			pass_data->BIT.Bt67 = Valid;
//			break;
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ステータスデータ取得関数                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPassData                                             |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort *Start                                           |*/
/*| PARAMETER    : ushort *End                                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Start = 0～11999，End=1～12000                           MH544401追記  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short GetPassRenewalArea( ulong ParkingNo, ushort *Start, ushort *End, ushort *Sposi, ushort *Eposi )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	*Start = 12000;
//	*End = 0;
//
//	if (ParkingNo == CPrmSS[S_SYS][65]) {
//
//		*Start	= 0;
//		*Sposi	= 0;
//		*Eposi	= 0;
//		if( CPrmSS[S_SYS][61] > 1 ){
//			*End	= (ushort)((CPrmSS[S_SYS][61] - 2)/4);
//			*Eposi	= (ushort)((CPrmSS[S_SYS][61] - 2)%4);
//		}
//	} else if (ParkingNo == CPrmSS[S_SYS][66]) {
//
//		*Start	= (ushort)((CPrmSS[S_SYS][61] - 1)/4);
//		*End	= (ushort)((CPrmSS[S_SYS][62] - 2)/4);
//		*Sposi	= (ushort)((CPrmSS[S_SYS][61] - 1)%4);
//		*Eposi	= (ushort)((CPrmSS[S_SYS][62] - 2)%4);
//
//	} else if (ParkingNo == CPrmSS[S_SYS][67]) {
//
//		*Start	= (ushort)((CPrmSS[S_SYS][62] - 1)/4);
//		*End	= (ushort)((CPrmSS[S_SYS][63] - 2)/4);
//		*Sposi	= (ushort)((CPrmSS[S_SYS][62] - 1)%4);
//		*Eposi	= (ushort)((CPrmSS[S_SYS][63] - 2)%4);
//
//	} else if (ParkingNo == CPrmSS[S_SYS][68]) {
//
//		*Start	= (ushort)((CPrmSS[S_SYS][63] - 1)/4);
//		*End	= (12000 - 1)/4;
//		*Sposi	= (ushort)((CPrmSS[S_SYS][63] - 1)%4);
//		*Eposi	= (ushort)((12000 - 1)%4);
//
//	}
//
//	if( *Start == 12000 ){
//		return -1;
//	}
//
//	if (*End == 0) {
//		*End = (12000 - 1)/4;
//		*Eposi	= (ushort)((12000 - 1)%4);
//	}
//	return 0;
	return 0;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ステータス書き込み関数（１領域）                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FillPassTbl                                             |*/
/*| PARAMETER    : ushort ParkingNo 駐車場№　1～999                       |*/
/*| PARAMETER    : ushort Status 有効無効　0=有効,1=無効,0xffff=書込まない |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void FillPassRenewalTbl( ulong ParkingNo, uchar Status )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	ushort i;
//	short  ret;
//	ushort Start,End;
//	ushort Sposi,Eposi;
//	ushort Valid;
//	ushort s_adrs,e_adrs;
//
//
//	PAS_RENEWAL *pcdpt;
//
//	ret = GetPassRenewalArea(ParkingNo, &Start, &End, &Sposi, &Eposi );
//
//	if( ret == -1 ){
//		return;
//	}
//
//	Valid = 0;
//	if( Status != 0 ){
//		Valid = 1;
//	}
//
//	if( Start == End ){
//		pcdpt = &pas_renewal[Start];
//		for( i=Sposi; i < Eposi; i++ ){ 
//			switch( i ){
//			case 0:
//				pcdpt->BIT.Bt01 = Valid;
//				break;
//			case 1:
//				pcdpt->BIT.Bt23 = Valid;
//				break;
//			case 2:
//				pcdpt->BIT.Bt45 = Valid;
//				break;
//			case 3:
//				pcdpt->BIT.Bt67 = Valid;
//				break;
//			}
//		}
//	}else if( Start < End ){
//		pcdpt = &pas_renewal[Start];
//		if( Sposi == 0 ){
//			s_adrs = Start;
//		}else{
//			s_adrs = Start + 1;
//			for( i=Sposi; i < 4; i++ ){ 
//
//				switch( i ){
//
//				case 0:
//					pcdpt->BIT.Bt01 = Valid;
//					break;
//				case 1:
//					pcdpt->BIT.Bt23 = Valid;
//					break;
//				case 2:
//					pcdpt->BIT.Bt45 = Valid;
//					break;
//				case 3:
//					pcdpt->BIT.Bt67 = Valid;
//					break;
//				}
//			}
//		}
//		if( Eposi == 3 ){
//			e_adrs = End;
//		}else{
//			e_adrs = End - 1;
//			pcdpt = &pas_renewal[End];
//			for( i=0; i < Eposi; i++ ){ 
//
//				switch( i ){
//
//				case 0:
//					pcdpt->BIT.Bt01 = Valid;
//					break;
//				case 1:
//					pcdpt->BIT.Bt23 = Valid;
//					break;
//				case 2:
//					pcdpt->BIT.Bt45 = Valid;
//					break;
//				case 3:
//					pcdpt->BIT.Bt67 = Valid;
//					break;
//				}
//			}
//		}
//		if( s_adrs <= e_adrs ){
//			pcdpt = &pas_renewal[s_adrs];
//			if(Status == 1){
//				Status = 0x55;
//			}
//			memset( pcdpt, (int)Status, sizeof(PAS_RENEWAL)*(e_adrs - s_adrs + 1));
//		}
//	}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: LOG種別
///	@param[out]	*NewestDate	: 最新年月日時分
///	@param[out]	*OldestDate	: 最古年月日時分
///	@param[out]	*RegCount	: 登録件数
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/03/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Ope2_Log_NewestOldestDateGet( ushort LogSyu,
								  date_time_rec *NewestDate, date_time_rec *OldestDate,
								  ushort *RegCount )
{
	ulong			Nrm_Newest, Nrm_Oldest;
	date_time_rec	*date;
	extern uchar	SysMnt_Work[];			// ｼｽﾃﾑﾒﾝﾃ workarea (32KB)

	*RegCount = Ope_Log_TotalCountGet(LogSyu);
	if( *RegCount == 0 ){		// データなし時は現在時刻とする
		Nrm_Oldest = Nrm_YMDHM((date_time_rec*)&CLK_REC);
		Nrm_Newest = Nrm_Oldest;
	}else{
		Ope_Log_1DataGet(LogSyu, 0, SysMnt_Work);
		date = get_log_date_time_rec(LogSyu, SysMnt_Work);
		Nrm_Oldest = Nrm_YMDHM(date);
		Ope_Log_1DataGet(LogSyu, *RegCount-1, SysMnt_Work);
		Nrm_Newest = Nrm_YMDHM(date);
	}

	// ﾃﾞｰﾀ形式変換(ﾉｰﾏﾗｲｽﾞ→ date_time_rec)
	UnNrm_YMDHM(OldestDate, Nrm_Oldest);
	UnNrm_YMDHM(NewestDate, Nrm_Newest);
}
/* Suica日毎集計は旧処理にて行うので復活させる */
//[]----------------------------------------------------------------------[]
///	@brief		LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: LOG種別
///	@param[out]	*NewestDate	: 最新年月日時分
///	@param[out]	*OldestDate	: 最古年月日時分
///	@param[out]	*RegCount	: 登録件数
///	@return		void
//[]----------------------------------------------------------------------[]
///	最古＆最新日付は登録されているLOG全体をｻｰﾁして求める。
///	登録ﾃﾞｰﾀがない場合は現在時刻をｾｯﾄする。	
//[]----------------------------------------------------------------------[]
/// Author       : Okuda				
/// Date         : 2006/05/18			
/// UpDate       :						
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	DiditalCashe_NewestOldestDateGet( ushort LogSyu,
									  date_time_rec	*NewestDate,
									  date_time_rec	*OldestDate,
									  ushort *RegCount )
{
	#define	TASK_SWITCH_INTERVAL_COUNT	100	// n回検索毎にﾀｽｸｽｲｯﾁする
	extern	uchar	SysMnt_Work[];			// ｼｽﾃﾑﾒﾝﾃ workarea (32KB)
	void	Ope2_Log_NewestOldestDateGet_sub1( ulong *NewestDateTime, 	// ｻﾌﾞﾙｰﾁﾝ prottype define
											   ulong *OldestDateTime, 
											   ulong TargetDateTime );
	ushort	Count=0, i;
	ulong	Nrm_Newest, Nrm_Oldest, Nrm_Target;
	date_time_rec	wk_CLK_REC;
	ushort	usPtr;							// 検索を始める配列の位置

	Nrm_Newest = Nrm_Oldest = 0L;

	/* LOG種別毎処理 */
	switch( LogSyu ){
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	case LOG_EDYSYUUKEI:							// Ｅｄｙ集計
//		Count = Syuukei_sp.edy_inf.cnt - 1;			// 集計件数更新
//		/* 集計数分処理を繰り返す */
//		for(i = 1; i <= Count; i++)
//		{
//			/* 次の参照位置へ移動 */
//			usPtr = (Syuukei_sp.edy_inf.ptr + SYUUKEI_DAY - i) % SYUUKEI_DAY;
//			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
//			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&Syuukei_sp.edy_inf.bun[usPtr].SyuTime );
//			/* 最新&最古ﾃﾞｰﾀ更新 */
//			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
//		}
//		break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	case LOG_SCASYUUKEI:							// Ｓｕｉｃａ集計
		Count = Syuukei_sp.sca_inf.cnt - 1;			// 集計件数更新
		/* 集計数分処理を繰り返す */
		for(i = 1; i <= Count; i++)
		{
			/* 次の参照位置へ移動 */
			usPtr = (Syuukei_sp.sca_inf.ptr + SYUUKEI_DAY - i) % SYUUKEI_DAY;
			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&Syuukei_sp.sca_inf.bun[usPtr].SyuTime );
			/* 最新&最古ﾃﾞｰﾀ更新 */
			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
		}
		break;
// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
	case LOG_ECSYUUKEI:								// 決済リーダ集計
		Count = Syuukei_sp.ec_inf.cnt - 1;			// 集計件数更新
		for(i = 1; i <= Count; i++)
		{
			/* 次の参照位置へ移動 */
			usPtr = (Syuukei_sp.ec_inf.ptr + SYUUKEI_DAY_EC - i) % SYUUKEI_DAY_EC;
			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&Syuukei_sp.ec_inf.bun[usPtr].SyuTime );
			/* 最新&最古ﾃﾞｰﾀ更新 */
			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
		}
		break;
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
	}

	/* データなし時は最新＆最古日時時分を現在時刻とする */
	memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );	// 現在時間をsaveする
	if( Nrm_Newest == 0L )
		Nrm_Newest = Nrm_YMDHM( &wk_CLK_REC );
	if( Nrm_Oldest == 0L )
		Nrm_Oldest = Nrm_YMDHM( &wk_CLK_REC );

	/* 最古&最新日時 ﾃﾞｰﾀ形式変換(ﾉｰﾏﾗｲｽﾞ→ date_time_rec) */
	UnNrm_YMDHM( NewestDate, Nrm_Newest );
	UnNrm_YMDHM( OldestDate, Nrm_Oldest );
	*RegCount = Count;
}

/*[]-----------------------------------------------------------------------[]*/
/*| 年月日時分ノーマライズ値を得る											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	date_time_rec型ﾃﾞｰﾀから t_NrmYMDHM型ﾉｰﾏﾗｲｽﾞ値に変換する。				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : *wk_CLK_REC = 対象時刻ﾃﾞｰﾀへのﾎﾟｲﾝﾀ(date_time_rec型)		|*/
/*| RETURN VALUE : ﾉｰﾏﾗｲｽﾞ値 (b31-16=年月日, b15-0=時分)					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2006/05/18												|*/
/*| UpDate       :															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
unsigned long	Nrm_YMDHM( date_time_rec *wk_CLK_REC )
{
	t_NrmYMDHM	Nrm;

	Nrm.us[0] = dnrmlzm( (short)wk_CLK_REC->Year, (short)wk_CLK_REC->Mon, (short)wk_CLK_REC->Day );
	Nrm.us[1] = (ushort)tnrmlz( (short)0, (short)0, (short)wk_CLK_REC->Hour, (short)wk_CLK_REC->Min );

	return	Nrm.ul;
}

/*[]-----------------------------------------------------------------------[]*/
/*| 年月日時分データ ノーマライズ値 → CLK_REC型に変換						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	t_NrmYMDHM型ﾉｰﾏﾗｲｽﾞ値から date_time_rec型に変換する。					|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : *wk_CLK_REC = 対象時刻ﾃﾞｰﾀへのﾎﾟｲﾝﾀ(date_time_rec型)		|*/
/*|                Nrm_YMDHM   = ﾉｰﾏﾗｲｽﾞ値 (b31-16=年月日, b15-0=時分)		|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2006/05/18												|*/
/*| UpDate       :															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	UnNrm_YMDHM( date_time_rec *wk_CLK_REC, ulong Nrm_YMDHM )
{
	t_NrmYMDHM	Nrm;
	short		wks[3];


	Nrm.ul = Nrm_YMDHM;

	idnrmlzm( Nrm.us[0], &wks[0], &wks[1], &wks[2] );		// 年月日ｱﾝﾉｰﾏﾗｲｽﾞ
	wk_CLK_REC->Year = (ushort)wks[0];
	wk_CLK_REC->Mon  = (uchar)wks[1];
	wk_CLK_REC->Day  = (uchar)wks[2];

	itnrmlz ( (short)Nrm.us[1], 0, 0, &wks[0], &wks[1] );	// 時分ｱﾝﾉｰﾏﾗｲｽﾞ
	wk_CLK_REC->Hour = (uchar)wks[0];
	wk_CLK_REC->Min  = (uchar)wks[1];
}

/*[]-----------------------------------------------------------------------[]*/
/*| Target日時ﾃﾞｰﾀから最古＆最新日付を更新する								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : *NewestDateTime = 最新時刻ﾃﾞｰﾀへのﾎﾟｲﾝﾀ					|*/
/*|				   *OldestDateTime = 最古時刻ﾃﾞｰﾀへのﾎﾟｲﾝﾀ					|*/
/*|				    TargetDateTime = Target時刻								|*/
/*|				  ※ﾃﾞｰﾀ形式は b31-16=年月日, b15-0=時分 にﾉｰﾏﾗｲｽﾞ値が入る。|*/
/*| RETURN VALUE : void														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Target日時が最古より古い、又は最新より新しい場合はｴﾘｱを更新する。		|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2006/05/18												|*/
/*| UpDate       :															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	Ope2_Log_NewestOldestDateGet_sub1( ulong *NewestDateTime, 
										   ulong *OldestDateTime, 
										   ulong TargetDateTime )
{
	if( *NewestDateTime == 0L )					// 前データなし
		*NewestDateTime = TargetDateTime;		// 最新日時更新

	if( *OldestDateTime == 0L )					// 前データなし
		*OldestDateTime = TargetDateTime;

	if( *NewestDateTime < TargetDateTime )		// 最新より新しい
		*NewestDateTime = TargetDateTime;		// 最新日時更新
	else if( *OldestDateTime > TargetDateTime )	// 最古より古い
		*OldestDateTime = TargetDateTime;
}

//[]----------------------------------------------------------------------[]
///	@brief			目的LOGﾃﾞｰﾀ内に 指定年月日時範囲内のﾃﾞｰﾀが何件あるかを求める関数
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu	: ログ種別 	"LOG_PAYMENT"		= 個別精算<br>
///											"LOG_PAYSTOP"		= 精算中止<br>
///											"LOG_TTOTAL"		= T合計<br>
///											"LOG_MONEYMANAGE"	= 金銭管理<br>
///											"LOG_COINBOX"		= ｺｲﾝ金庫合計<br>
///											"LOG_NOTEBOX"		= 紙幣金庫合計
///											"LOG_EDYMEISAI"		= Ｅｄｙ利用明細<br>
///											"LOG_SCAMEISAI"		= Ｓｕｉｃａ利用明細<br>
///	@param[in]		NewDate	: 日付範囲（新日付け）
///	@param[in]		OldDate	: 日付範囲（旧日付け）
///	@param[in]		NewTime	: 日付範囲（新時間）
///	@param[in]		OldTime	: 日付範囲（旧時間）
///	@return			ret		: 指定日付範囲内のLOG件数
///	@author			Imai
///	@note			最古＆最新日付は登録されているLOG全体をｻｰﾁして求める。<br>
///	                登録ﾃﾞｰﾀがない場合は現在時刻をｾｯﾄする。
///					下記Ope2_Log_CountGet_inDate()を流用。<br>
///					Ope2_Log_CountGet_inDate()を参照。<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/07/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	Ope2_Log_CountGet_inDateTime( ushort LogSyu, ushort *NewDate, ushort *OldDate,
									ushort NewTime, ushort OldTime )
{
	extern	uchar	SysMnt_Work[];			// ｼｽﾃﾑﾒﾝﾃ workarea (32KB)
	#define	TASK_SWITCH_INTERVAL_COUNT	100	// n回検索毎にﾀｽｸｽｲｯﾁする

	ushort	Count=0, i, inData_Count=0;
	ulong	Nrm_New, Nrm_Old, Nrm_Target;
	date_time_rec	wk_CLK_REC;
	ushort	LogCount_inSRAM;				// SRAM内 LOG登録件数
	ushort	LogCount_inFROM;				// FlashROM内 LOG登録件数
	ulong	ulRet;							// LOG件数取得work
	ulong	ulWork;							// LOG件数取得work
	t_FltLogHandle handle;
	Receipt_data tmpReceipt;				// 判断処理用精算情報

	/* 検索開始日時 ﾉｰﾏﾗｲｽﾞ値get */
	wk_CLK_REC.Year = OldDate[0];
	wk_CLK_REC.Mon  = (uchar)OldDate[1];
	wk_CLK_REC.Day  = (uchar)OldDate[2];
	wk_CLK_REC.Hour = (uchar)OldTime;
	wk_CLK_REC.Min  = 0;
	Nrm_Old = Nrm_YMDHM( &wk_CLK_REC );

	/* 検索終了日時 ﾉｰﾏﾗｲｽﾞ値get */
	wk_CLK_REC.Year = NewDate[0];
	wk_CLK_REC.Mon  = (uchar)NewDate[1];
	wk_CLK_REC.Day  = (uchar)NewDate[2];
	wk_CLK_REC.Hour = (uchar)(NewTime-1);
	wk_CLK_REC.Min  = 59;
	Nrm_New = Nrm_YMDHM( &wk_CLK_REC );
	LogCount_inSRAM = 0;
	Nrm_Target = 0;

	/* LOG種別毎処理 */
	switch( LogSyu ){
	case	LOG_PAYMENT:				// 個別精算
		/** LOG件数get **/

		/* FlashROM内 LOG登録件数get */
		ulRet = FLT_GetSaleLogRecordNum( (long*) &ulWork );	// FlashROM内LOG登録件数get
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:登録作業中)
Ope2_Log_ReadNG_210:
			ulWork = 0L;							// この時に登録作業中であることは無いのでリトライはしない
		}
		LogCount_inFROM = (ushort)ulWork;			// FlashROM内 LOG登録件数set

		/* SRAM内 LOG登録件数get */

		/* LOG登録件数（Total）get */
		Count = LogCount_inSRAM + LogCount_inFROM;	// 登録件数set

		/** 最新&最古ﾃﾞｰﾀの日時を取得 **/

		/* FlashROM内ﾃﾞｰﾀ検索 */
		for( i=0; i<LogCount_inFROM; ++i ){
			if( 0 == i )
				ulRet = FLT_FindFirstSaleLog( &handle, (char*)&SysMnt_Work[0] );
			else
				ulRet = FLT_FindNextSaleLog_OnlyDate ( &handle, (char*)&SysMnt_Work[0] );
			if( 0 != (ulRet & 0xff000000) ){		// Read error (BUSY:登録作業中)
				goto Ope2_Log_ReadNG_210;			// この時に登録作業中であることは無いが
			}										// もしあったらFlash内にはLOG無しとして動作する

			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletaskを1週後に復帰する

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
				++inData_Count;
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		xPause( 0 );								// idletaskを1週後に復帰する

		/* SRAM内ﾃﾞｰﾀ検索 */
		for( i=0; i<LogCount_inSRAM; ++i ){
			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletaskを1週後に復帰する

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
				++inData_Count;
		}
		break;

	case	LOG_TTOTAL:					// T合計
		/** LOG件数get **/

		/* FlashROM内 LOG登録件数get */
		ulRet = FLT_GetTotalLogRecordNum( (long*) &ulWork );	// FlashROM内LOG登録件数get
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:登録作業中)
Ope2_Log_ReadNG_220:
			ulWork = 0L;							// この時に登録作業中であることは無いのでリトライはしない
		}
		LogCount_inFROM = (ushort)ulWork;			// FlashROM内 LOG登録件数set

		/* SRAM内 LOG登録件数get */

		/* LOG登録件数（Total）get */
		Count = LogCount_inSRAM + LogCount_inFROM;	// 登録件数set

		/** 最新&最古ﾃﾞｰﾀの日時を取得 **/

		/* FlashROM内ﾃﾞｰﾀ検索 */
		for( i=0; i<LogCount_inFROM; ++i ){
			if( 0 == i )
				ulRet = FLT_FindFirstTotalLog( &handle, (char*)&SysMnt_Work[0] );
			else
				ulRet = FLT_FindNextTotalLog_OnlyDate ( &handle, (char*)&SysMnt_Work[0] );
			if( 0 != (ulRet & 0xff000000) ){		// Read error (BUSY:登録作業中)
				goto Ope2_Log_ReadNG_220;			// この時に登録作業中であることは無いが
			}										// もしあったらFlash内にはLOG無しとして動作する

			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletaskを1週後に復帰する

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Syu_log*)&SysMnt_Work[0])->syukei_log.NowTime) );

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
				++inData_Count;
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		xPause( 0 );								// idletaskを1週後に復帰する

		/* SRAM内ﾃﾞｰﾀ検索 */
		for( i=0; i<LogCount_inSRAM; ++i ){
			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletaskを1週後に復帰する

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
				++inData_Count;
		}
		break;

	case	LOG_PAYSTOP:				// 精算中止
		Count = Ope2_Log_CountGet( LogSyu );		// 登録件数set

		for( i=0; i<Count; ++i ){

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
				++inData_Count;
		}
		break;

	case	LOG_MONEYMANAGE:			// 金銭管理
		Count = Ope2_Log_CountGet( LogSyu );		// 登録件数set

		for( i=0; i<Count; ++i ){

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
				++inData_Count;
		}
		break;

	case	LOG_COINBOX:				// ｺｲﾝ金庫合計
		Count = Ope2_Log_CountGet( LogSyu );		// 登録件数set

		for( i=0; i<Count; ++i ){

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
				++inData_Count;
		}
		break;

	case	LOG_NOTEBOX:				// 紙幣金庫合計
		Count = Ope2_Log_CountGet( LogSyu );		// 登録件数set

		for( i=0; i<Count; ++i ){

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
				++inData_Count;
		}
		break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	case	LOG_EDYARM:				// アラーム取引情報
//		Count = Ope2_Log_CountGet( LogSyu );		// 登録件数set
//
//		for( i=0; i<Count; ++i ){
//
//			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
//
//			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
//			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
//				++inData_Count;
//		}
//		break;
//
//	case	LOG_EDYSHIME:				// Ｅｄｙ締め記録
//		Count = Ope2_Log_CountGet( LogSyu );		// 登録件数set
//
//		for( i=0; i<Count; ++i ){
//
//			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
//
//			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
//			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
//				++inData_Count;
//		}
//		break;
//
//#ifdef	FUNCTION_MASK_EDY 
//	case	LOG_EDYMEISAI:							// Ｅｄｙ利用精算数
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	case	LOG_SCAMEISAI:							// Ｓｕｉｃａ利用精算数
		/* FlashROMへの精算完了ログ取得(取得成功で検索処理をする) */
		if(!(FLT_GetSaleLogRecordNum((long*)&ulWork) & 0xFF000000))
		{
			Count = (ushort)ulWork;
			for(i = 0; i < Count; i++)					/* FlashROMからデータを読み出すループ */
			{
				// 個別精算データ１件読出し
				if( 0 == i )
					ulRet = FLT_FindFirstEpayKnd( &handle, (char*)&tmpReceipt, 1, 0);
				else
					ulRet = FLT_FindNextEpayKnd( &handle, (char*)&tmpReceipt, 1);

				xPause( 0 );						// taskchgに復帰する

				if( 0 != (ulRet & 0xff000000) ){		// 読出し失敗(Y)
					break;								// FlashROM検索処理を終了
				}
				/* 取得した情報が引数からの条件と一致 */
				if(is_match_receipt(&tmpReceipt, Nrm_Old, Nrm_New, LogSyu))
				{
					/* ヒット件数更新 */
					inData_Count++;
				}
			}
			FLT_CloseLogHandle( &handle );				// Handle & Lock release
		}
		break;
	}

	return	inData_Count;
}

//[]----------------------------------------------------------------------[]
///	@brief			目的LOGﾃﾞｰﾀ内に 指定年月日時範囲内のﾃﾞｰﾀが何件あるかを求める関数
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu		: ログ種別 	"LOG_PAYMENT"		= 個別精算<br>
///											"LOG_TTOTAL"		= T合計<br>
///											"LOG_EDYMEISAI"		= Edy利用明細<br>
///											"LOG_SCAMEISAI"		= Suica利用明細<br>
///	@param[in]		NewDate		: 日付範囲（新日付け）
///	@param[in]		OldDate		: 日付範囲（旧日付け）
///	@param[in]		NewTime		: 日付範囲（新時間）
///	@param[in]		OldTime		: 日付範囲（旧時間）
///	@param[out]		FirstIndex	: 最初に条件一致したFlashROMの番号
///	@param[out]		LastIndex	: 最後に条件一致したFlashROMの番号
///	@return			ret			: 指定日付範囲内のLOG件数
///	@author			Ise
///	@note			Ope2_Log_CountGet_inDateTime関数をそのまま流用
///					本関数は上記関数+FlashROMの検索で最初・最後に条件一致した
///					番号を第６(ushort *FirstIndex)、第７(ushort *LastIndex)引数
///					に格納してコール元へ返す<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/011/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	Ope2_Log_CountGet_inDateTime_AttachOffset( ushort LogSyu, ushort *NewDate, ushort *OldDate,
									ushort NewTime, ushort OldTime, ushort *FirstIndex, ushort *LastIndex )
{
	extern	uchar	SysMnt_Work[];			// ｼｽﾃﾑﾒﾝﾃ workarea (32KB)
	#define	TASK_SWITCH_INTERVAL_COUNT	100	// n回検索毎にﾀｽｸｽｲｯﾁする

	ushort	Count=0, i, inData_Count=0;
	ulong	Nrm_New, Nrm_Old, Nrm_Target;
	date_time_rec	wk_CLK_REC;
	ushort	LogCount_inSRAM;				// SRAM内 LOG登録件数
	ushort	LogCount_inFROM;				// FlashROM内 LOG登録件数
	ulong	ulRet;							// LOG件数取得work
	ulong	ulWork;							// LOG件数取得work
	t_FltLogHandle handle;
	Receipt_data tmpReceipt;				// 判断処理用精算情報

	/* 検索開始日時 ﾉｰﾏﾗｲｽﾞ値get */
	wk_CLK_REC.Year = OldDate[0];
	wk_CLK_REC.Mon  = (uchar)OldDate[1];
	wk_CLK_REC.Day  = (uchar)OldDate[2];
	wk_CLK_REC.Hour = (uchar)OldTime;
	wk_CLK_REC.Min  = 0;
	Nrm_Old = Nrm_YMDHM( &wk_CLK_REC );

	/* 検索終了日時 ﾉｰﾏﾗｲｽﾞ値get */
	wk_CLK_REC.Year = NewDate[0];
	wk_CLK_REC.Mon  = (uchar)NewDate[1];
	wk_CLK_REC.Day  = (uchar)NewDate[2];
	wk_CLK_REC.Hour = (uchar)(NewTime-1);
	wk_CLK_REC.Min  = 59;
	Nrm_New = Nrm_YMDHM( &wk_CLK_REC );

	*FirstIndex = 0xFFFF;					// 一致したFlash番号格納用変数初期化
	*LastIndex = 0xFFFF;					// 一致したFlash番号格納用変数初期化
	LogCount_inSRAM = 0;

	/* LOG種別毎処理 */
	switch( LogSyu ){
	case	LOG_PAYMENT:				// 個別精算
		/** LOG件数get **/

		/* FlashROM内 LOG登録件数get */
		ulRet = FLT_GetSaleLogRecordNum( (long*) &ulWork );	// FlashROM内LOG登録件数get
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:登録作業中)
Ope2_Log_ReadNG_210:
			ulWork = 0L;							// この時に登録作業中であることは無いのでリトライはしない
		}
		LogCount_inFROM = (ushort)ulWork;			// FlashROM内 LOG登録件数set

		/* SRAM内 LOG登録件数get */

		/* LOG登録件数（Total）get */
		Count = LogCount_inSRAM + LogCount_inFROM;	// 登録件数set

		/** 最新&最古ﾃﾞｰﾀの日時を取得 **/

		/* FlashROM内ﾃﾞｰﾀ検索 */
		for( i=0; i<LogCount_inFROM; ++i ){
			if( 0 == i )
				ulRet = FLT_FindFirstSaleLog( &handle, (char*)&SysMnt_Work[0] );
			else
				ulRet = FLT_FindNextSaleLog_OnlyDate ( &handle, (char*)&SysMnt_Work[0] );
			if( 0 != (ulRet & 0xff000000) ){		// Read error (BUSY:登録作業中)
				goto Ope2_Log_ReadNG_210;			// この時に登録作業中であることは無いが
			}										// もしあったらFlash内にはLOG無しとして動作する

			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletaskを1週後に復帰する

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
			{
				if(!inData_Count && FirstIndex)		// 検索開始後最初に条件一致
				{
					*FirstIndex = i;				// 一致したFlashの番号を格納
				}
				if(LastIndex)						// ２回目以降条件一致した
				{
					*LastIndex = i;					// 一致したFlashの番号を格納
				}
				++inData_Count;
			}
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		xPause( 0 );								// idletaskを1週後に復帰する

		/* SRAM内ﾃﾞｰﾀ検索 */
		break;

	case	LOG_TTOTAL:					// T合計
		/** LOG件数get **/

		/* FlashROM内 LOG登録件数get */
		ulRet = FLT_GetTotalLogRecordNum( (long*) &ulWork );	// FlashROM内LOG登録件数get
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:登録作業中)
Ope2_Log_ReadNG_220:
			ulWork = 0L;							// この時に登録作業中であることは無いのでリトライはしない
		}
		LogCount_inFROM = (ushort)ulWork;			// FlashROM内 LOG登録件数set

		/* SRAM内 LOG登録件数get */

		/* LOG登録件数（Total）get */
		Count = LogCount_inSRAM + LogCount_inFROM;	// 登録件数set

		/** 最新&最古ﾃﾞｰﾀの日時を取得 **/

		/* FlashROM内ﾃﾞｰﾀ検索 */
		for( i=0; i<LogCount_inFROM; ++i ){
			if( 0 == i )
				ulRet = FLT_FindFirstTotalLog( &handle, (char*)&SysMnt_Work[0] );
			else
				ulRet = FLT_FindNextTotalLog_OnlyDate ( &handle, (char*)&SysMnt_Work[0] );
			if( 0 != (ulRet & 0xff000000) ){		// Read error (BUSY:登録作業中)
				goto Ope2_Log_ReadNG_220;			// この時に登録作業中であることは無いが
			}										// もしあったらFlash内にはLOG無しとして動作する

			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletaskを1週後に復帰する

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Syu_log*)&SysMnt_Work[0])->syukei_log.NowTime) );

			/* 範囲内時刻ﾃﾞｰﾀかﾁｪｯｸ */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// 指定時間範囲内
			{
				if(!inData_Count && FirstIndex)		// 検索開始後最初に条件一致
				{
					*FirstIndex = i;				// 一致したFlashの番号を格納
				}
				if(LastIndex)						// ２回目以降条件一致した
				{
					*LastIndex = i;					// 一致したFlashの番号を格納
				}
				++inData_Count;
			}
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		xPause( 0 );								// idletaskを1週後に復帰する
		break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	case	LOG_EDYMEISAI:							// Ｅｄｙ利用精算数
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	case	LOG_SCAMEISAI:							// Ｓｕｉｃａ利用精算数
		/* FlashROMへの精算完了ログ取得(取得成功で検索処理をする) */
		if(!(FLT_GetSaleLogRecordNum((long*)&ulWork) & 0xFF000000))
		{
			Count = (ushort)ulWork;
			for(i = 0; i < Count; i++)					/* FlashROMからデータを読み出すループ */
			{
				// 個別精算データ１件読出し
				if( 0 == i )
					ulRet = FLT_FindFirstEpayKnd( &handle, (char*)&tmpReceipt, 1, 0);
				else
					ulRet = FLT_FindNextEpayKnd( &handle, (char*)&tmpReceipt, 1);

				xPause( 0 );						// taskchgに復帰する

				if( 0 != (ulRet & 0xff000000) ){		// 読出し失敗(Y)
					break;								// FlashROM検索処理を終了
				}
				/* 取得した情報が引数からの条件と一致 */
				if(is_match_receipt(&tmpReceipt, Nrm_Old, Nrm_New, LogSyu))
				{
					if(!inData_Count)					// 検索開始後最初に条件一致
					{
						*FirstIndex = i;				// 最後に一致したFlashの番号を格納
					}
					*LastIndex = i;						// 最後に一致したFlashの番号を格納
					/* ヒット件数更新 */
					inData_Count++;
				}
			}
			FLT_CloseLogHandle( &handle );				// Handle & Lock release
		}
		break;
	}
	return	inData_Count;
}

//[]----------------------------------------------------------------------[]
///	@brief			目的LOG種別毎にﾃﾞｰﾀが何件あるかを求める関数
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu		: ログ種別 	"LOG_PAYMENT"		= 個別精算<br>
///											"LOG_TTOTAL"		= T合計<br>
///											"LOG_GTTOTAL"		= GT合計<br>
///											"LOG_EDYMEISAI"		= Edy利用明細<br>
///											"LOG_SCAMEISAI"		= Suica利用明細<br>
///	@param[out]		NewestDate	: 最新日時時分ｾｯﾄｴﾘｱへのﾎﾟｲﾝﾀ
///	@param[out]		OldestDate	: 最古日時時分ｾｯﾄｴﾘｱへのﾎﾟｲﾝﾀ
///	@param[out]		FirstIndex	: 最初に条件一致したFlashROMの番号
///	@param[out]		LastIndex	: 最後に条件一致したFlashROMの番号
///	@return			ret			: 指定日付範囲内のLOG件数
///	@author			Ise
///	@note			Ope2_Log_CountGet_inDateTime関数をそのまま流用
///					本関数は上記関数+FlashROMの検索で最初・最後に条件一致した
///					番号を第４(ushort *FirstIndex)、第５(ushort *LastIndex)引数
///					に格納してコール元へ返す<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/011/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	Ope2_Log_NewestOldestDateGet_AttachOffset(	ushort			LogSyu,
													date_time_rec	*NewestDate,
													date_time_rec	*OldestDate,
													ushort			*FirstIndex,
													ushort			*LastIndex )
{
	extern	uchar	SysMnt_Work[];			// ｼｽﾃﾑﾒﾝﾃ workarea (32KB)
	void	Ope2_Log_NewestOldestDateGet_sub1( ulong *NewestDateTime, 	// ｻﾌﾞﾙｰﾁﾝ prottype define
											   ulong *OldestDateTime, 
											   ulong TargetDateTime );
	ushort	Count=0, i;
	ulong	Nrm_Newest, Nrm_Oldest, Nrm_Target;
	date_time_rec	wk_CLK_REC;
	ulong	ulRet;							// LOG件数取得work
	t_FltLogHandle handle;
	Receipt_data *tmpReceipt;				// 判断処理用精算情報
	ushort	Count_wk=0;
	i = 0;

	Nrm_Newest = Nrm_Oldest = 0L;
	*FirstIndex = 0xFFFF;
	*LastIndex = 0xFFFF;

	/* LOG種別毎処理 */
	switch( LogSyu ){
	case	LOG_PAYMENT:				// 個別精算
		/** LOG件数get **/

		/* LOG登録件数（Total）get */
		Count = Ope2_Log_CountGet( LOG_PAYMENT );		// 登録件数set

		tmpReceipt = (Receipt_data*)&SysMnt_Work[0];

		/** 最新&最古ﾃﾞｰﾀの日時を取得 **/
		ulRet = FLT_FindFirstLog(eLOG_PAYMENT, &handle, (char*)&SysMnt_Work[0] );
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:登録作業中)
			Count = 0;								// この時に登録作業中であることは無いが
													// もしあったらLOG無しとして動作する
		} else {
			if((tmpReceipt->OutKind == 1) || (tmpReceipt->OutKind == 11) || (tmpReceipt->OutKind == 3)) {
				i = 0;// 強制出庫、不正出庫のデータの場合検索の対象とならないためスキップ(カウント対象外)
			}else{
				i = 1;
				/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
				Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );

				/* 最新&最古ﾃﾞｰﾀ更新 */
				Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
			}

			while(  i < Count ){
				ulRet = FLT_FindNextLog_OnlyDate(eLOG_PAYMENT, &handle, (char*)&SysMnt_Work[0] );

				if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:登録作業中)
					Count = 0;								// この時に登録作業中であることは無いが
					break;									// もしあったらLOG無しとして動作する
				}

				if((tmpReceipt->OutKind == 1) || (tmpReceipt->OutKind == 11) || (tmpReceipt->OutKind == 3)) {
					continue;// 強制出庫、不正出庫のデータの場合検索の対象とならないためスキップ(カウント対象外)
				} else {
					i++;
				}
				/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
				Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );

				/* 最新&最古ﾃﾞｰﾀ更新 */
				Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
			}
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		*FirstIndex = 0;							// 最後に一致した番号を格納
		*LastIndex = i;								// 最後に一致した番号を格納
		break;

	case	LOG_TTOTAL:					// T合計

		/** LOG件数get **/

		/* LOG登録件数（Total）get */
		Count = Ope2_Log_CountGet( LOG_TTOTAL );		// 登録件数set

		/** 最新&最古ﾃﾞｰﾀの日時を取得 **/
		for( i=0; i < Count; ++i ){
			if( 0 == i ){
				ulRet = FLT_FindFirstLog(eLOG_TTOTAL, &handle, (char*)&SysMnt_Work[0] );
			}else{
				ulRet = FLT_FindNextLog_OnlyDate(eLOG_TTOTAL, &handle, (char*)&SysMnt_Work[0] );
			}

			if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:登録作業中)
				Count = 0;								// この時に登録作業中であることは無いが
				break;									// もしあったらLOG無しとして動作する
			}

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Syu_log*)&SysMnt_Work[0])->syukei_log.NowTime) );

			/* 最新&最古ﾃﾞｰﾀ更新 */
			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		*FirstIndex = 0;							// 最後に一致した番号を格納
		*LastIndex = i;								// 最後に一致した番号を格納
		break;

	case	LOG_GTTOTAL:				// GT合計

		/** LOG件数get **/

		/* LOG登録件数（Total）get */
		Count = Ope2_Log_CountGet( LOG_GTTOTAL );		// 登録件数set

		/** 最新&最古ﾃﾞｰﾀの日時を取得 **/
		for( i=0; i < Count; ++i ){
			if( 0 == i ){
				ulRet = FLT_FindFirstLog(eLOG_GTTOTAL, &handle, (char*)&SysMnt_Work[0] );
			}else{
				ulRet = FLT_FindNextLog_OnlyDate(eLOG_GTTOTAL, &handle, (char*)&SysMnt_Work[0] );
			}

			if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:登録作業中)
				Count = 0;								// この時に登録作業中であることは無いが
				break;									// もしあったらLOG無しとして動作する
			}

			/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Syu_log*)&SysMnt_Work[0])->syukei_log.NowTime) );

			/* 最新&最古ﾃﾞｰﾀ更新 */
			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		*FirstIndex = 0;							// 最後に一致した番号を格納
		*LastIndex = i;								// 最後に一致した番号を格納
		break;

	case	LOG_SCAMEISAI:								// Ｓｕｉｃａ利用明細
// MH321800(S) D.Inaba ICクレジット対応
	case	LOG_ECMEISAI:								// 決済リーダ利用明細
// MH321800(E) D.Inaba ICクレジット対応	
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
	case	LOG_ECMINASHI:								// みなし決済(決済リーダ)精算数
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
			Count_wk = Ope_Log_TotalCountGet(eLOG_PAYMENT);
			for(i = 0; i < Count_wk; i++)					/* FlashROMからデータを読み出すループ */
			{
				// 個別精算データ１件読出し
				Ope_Log_1DataGet(eLOG_PAYMENT, i, SysMnt_Work);

//				/* 取得した情報が引数からの条件と一致 */
//				if(is_match_receipt(&tmpReceipt, 0, 0, LogSyu))
// MH321800(S) D.Inaba ICクレジット対応
//				if(is_match_receipt((Receipt_data*)&SysMnt_Work, 0, 0, LOG_SCAMEISAI))
				if(is_match_receipt((Receipt_data*)&SysMnt_Work, 0, 0, LogSyu))
// MH321800(E) D.Inaba ICクレジット対応	
				{
					/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
					Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );
					/* 最新&最古ﾃﾞｰﾀ更新 */
					Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
					if(!Count)							// 検索開始後最初に条件一致
					{
						*FirstIndex = i;				// 最後に一致したFlashの番号を格納
					}
					*LastIndex = i;						// 最後に一致したFlashの番号を格納
					/* ヒット件数更新 */
					Count++;
				}
			}
		break;

	}

	/* データなし時は最新＆最古日時時分を現在時刻とする */
	memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );	// 現在時間をsaveする
	if( Nrm_Newest == 0L )
		Nrm_Newest = Nrm_YMDHM( &wk_CLK_REC );
	if( Nrm_Oldest == 0L )
		Nrm_Oldest = Nrm_YMDHM( &wk_CLK_REC );

	/* 最古&最新日時 ﾃﾞｰﾀ形式変換(ﾉｰﾏﾗｲｽﾞ→ date_time_rec) */
	UnNrm_YMDHM( NewestDate, Nrm_Newest );
	UnNrm_YMDHM( OldestDate, Nrm_Oldest );

	return Count;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGから指定年月日ﾃﾞｰﾀが何件あるかを求める
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu	: LOG種別
///	@param[in]	*Date	: 指定年月日
///	@param[out]	*id		: 先頭レコードID
///	@return		ushort	: 件数
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/03/12<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope2_Log_CountGet_inDate( ushort LogSyu, ushort *Date, ushort *id )
{
	ushort			end, count;
	date_time_rec	dtrec;
	extern	uchar	SysMnt_Work[];			// ｼｽﾃﾑﾒﾝﾃ workarea (32KB)

	count = Ope_Log_TotalCountGet(LogSyu);
	if( count==0 ){					// レコードなし
		return 0;
	}

	/* 検索開始日時 ﾉｰﾏﾗｲｽﾞ値get */
	dtrec.Year = Date[0];
	dtrec.Mon  = (uchar)Date[1];
	dtrec.Day  = (uchar)Date[2];
	dtrec.Hour = 0;
	dtrec.Min  = 0;
	*id = count-1;
	if( !Ope2_Log_Get_inDate(LogSyu, Nrm_YMDHM(&dtrec), id, SysMnt_Work) ){	// 該当なし
		return 0;
	}

	dtrec.Day++;
	end = count-1;
	if( !Ope2_Log_Get_inDate(LogSyu, Nrm_YMDHM(&dtrec), &end, SysMnt_Work) ){	// 該当なし
		end++;
	}

	return (ushort)(end-*id);
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGから指定年月日の最初のレコードを得る
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: LOG種別
///	@param[in]	date		: 指定年月日(ﾉｰﾏﾗｲｽﾞ値)
///	@param[in/out]	*id		: レコードID
///	@param[out]	*pBuf		: レコードデータ
///	@return		BOOL		: TRUE=あり, FALSE=なし
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/03/12<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
BOOL Ope2_Log_Get_inDate( ushort LogSyu, ulong Nrm_Date, ushort *id, void *pBuf )
{
	BOOL			result=FALSE;
	ushort			min=0, work=*id;
	ulong			Nrm_Target;
	date_time_rec	*drec;

	drec = get_log_date_time_rec(LogSyu, pBuf);
	Ope_Log_1DataGet(LogSyu, 0, pBuf);
	drec->Hour = 0;
	drec->Min  = 0;
	Nrm_Target = Nrm_YMDHM(drec);
	if( Nrm_Target==Nrm_Date ){			// ヒット
		*id = 0;
		return TRUE;
	}

	if( *id == 1 ){
		Ope_Log_1DataGet(LogSyu, 1, pBuf);
		drec->Hour = 0;
		drec->Min  = 0;
		Nrm_Target = Nrm_YMDHM(drec);
		if( Nrm_Target == Nrm_Date ){	// ヒット
			result = TRUE;
		}
	}else{
	while( min+1<*id ){
		Ope_Log_1DataGet(LogSyu, work, pBuf);
		drec->Hour = 0;
		drec->Min  = 0;
		Nrm_Target = Nrm_YMDHM(drec);
		if( Nrm_Target==Nrm_Date ){		// ヒット
			*id = work;
			result = TRUE;
		}
		else if( Nrm_Target>Nrm_Date ){	// 古い
			*id = work;
		}
		else{
			min = work;
		}
		work = min + (*id-min)/2;
	}
	}

	return result;
}

/*[]-----------------------------------------------------------------------[]*/
/*| LOG登録件数get															|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : LogSyu =	ログ種別 	"LOG_PAYMENT"		= 個別精算		|*/
/*|										"LOG_PAYSTOP"		= 精算中止		|*/
/*|										"LOG_TTOTAL"		= T合計			|*/
/*|										"LOG_GTTOTAL"		= GT合計		|*/
/*|										"LOG_MONEYMANAGE"	= 金銭管理		|*/
/*|										"LOG_COINBOX"		= ｺｲﾝ金庫合計	|*/
/*|										"LOG_NOTEBOX"		= 紙幣金庫合計	|*/
/*|										"LOG_ABNORMAL"		= 不正出庫		|*/
/*|										"LOG_EDYMEISAI"		= Ｅｄｙ利用明細|*/
/*|										"LOG_SCAMEISAI"		= Ｓｕｉｃａ利用明細|*/
/*|										"LOG_EDYSYUUKEI"	= Ｅｄｙ集計	|*/
/*|										"LOG_SCASYUUKEI"	= Ｓｕｉｃａ集計|*/
/*| RETURN VALUE : LOG件数													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	個別精算LOG，集計LOGはFlashROM+SRAMのﾄｰﾀﾙ件数を戻す。					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2006/05/19												|*/
/*| UpDate       :															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	Ope2_Log_CountGet( ushort LogSyu )
{
	ushort	LogCount=0;

	switch( LogSyu ){
	case	LOG_PAYMENT:				// 個別精算
		// FlashROM + SRAM LOG登録件数（Total）get
		LogCount = Ope_SaleLog_TotalCountGet( PAY_LOG_CMP_STP );	// 登録件数set(不正・強制出庫を除く)
		break;

	case	LOG_TTOTAL:					// T合計
		// FlashROM + SRAM LOG登録件数get
		LogCount = Ope_Log_TotalCountGet( eLOG_TTOTAL );	// 登録件数set
		break;

	case	LOG_GTTOTAL:				// GT合計
		// FlashROM + SRAM LOG登録件数get
		LogCount = Ope_Log_TotalCountGet( eLOG_GTTOTAL );	// 登録件数set
		break;

	case	LOG_MONEYMANAGE:			// 金銭管理
		// SRAM LOG登録件数get
		LogCount = Ope_Log_TotalCountGet( eLOG_MNYMNG_SRAM );	// 登録件数set
		break;

	case	LOG_COINBOX:				// ｺｲﾝ金庫合計
		// SRAM LOG登録件数get
		LogCount = Ope_Log_TotalCountGet( eLOG_COINBOX );	// 登録件数set
		break;

	case	LOG_NOTEBOX:				// 紙幣金庫合計
		// SRAM LOG登録件数get
		LogCount = Ope_Log_TotalCountGet( eLOG_NOTEBOX );	// 登録件数set
		break;

	case	LOG_ABNORMAL:				// 不正出庫
		// FlashROM + SRAM LOG登録件数get
		LogCount = Ope_Log_TotalCountGet( eLOG_ABNORMAL );	// 登録件数set
		break;

	case	LOG_MNYMNG_SRAM:			// 金銭管理ログ登録(SRAM)
		// SRAM LOG登録件数get
		LogCount = Ope_Log_TotalCountGet( eLOG_MNYMNG_SRAM );	// 登録件数set
		break;
	}

	return	LogCount;
}

/*[]-----------------------------------------------------------------------[]*/
/*| ｺｲﾝ取出し口ｶﾞｲﾄﾞLED消灯処理											    |*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : なし                                                     |*/
/*| RETURN VALUE : なし                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : namioka                                                  |*/
/*| Date         : 2006-02-10                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	op_ReciptLedOff( void )
{
	LedReq( CN_TRAYLED, LED_OFF );				// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED消灯
}

//[]----------------------------------------------------------------------[]
///	@brief			レシートプリンタチェック
//[]----------------------------------------------------------------------[]
///	@return			ret		1=レシート出力可
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/03/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		Ope_isPrinterReady(void)
{
	if(ALM_CHK[ALMMDL_SUB][ALARM_RPAPEREND] != 0) 
		return 0;
// MH810105(S) MH364301 インボイス対応（プリンタオープンも印字出力判定に追加する）
	if(ALM_CHK[ALMMDL_SUB][ALARM_RPAPEROPEN] != 0){
		return 0;
	}
// MH810105(E) MH364301 インボイス対応（プリンタオープンも印字出力判定に追加する）
	if (OPECTL.Mnt_mod == 0) {
	// 通常時
		if (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM))
		// ﾚｼｰﾄﾌﾟﾘﾝﾀ通信不良発生中
			return 0;
	}
	return 1;
}
//[]----------------------------------------------------------------------[]
///	@brief			ジャーナルプリンタチェック
//[]----------------------------------------------------------------------[]
///	@return			ret		1=出力可
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/08/26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		Ope_isJPrinterReady(void)
{
	if (PrnJnlCheck() == OFF)
		return -1;
	if(ALM_CHK[ALMMDL_SUB][ALARM_JPAPEREND] != 0)
		return 0;
// MH810105(S) MH364301 インボイス対応（プリンタオープンも印字出力判定に追加する）
	if(ALM_CHK[ALMMDL_SUB][ALARM_JPAPEROPEN] != 0){
		return 0;
	}
// MH810105(E) MH364301 インボイス対応（プリンタオープンも印字出力判定に追加する）
// MH810104 GG119201(S) 電子ジャーナル対応
	if (ALM_CHK[ALMMDL_SUB][ALARM_SD_END] != 0) {
		return 0;
	}
// MH810104 GG119201(E) 電子ジャーナル対応
	if (OPECTL.Mnt_mod == 0) {
	// 通常時
		if (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_PRINTCOM))
		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ通信不良発生中
			return 0;
// MH810104 GG119201(S) 電子ジャーナル対応
		if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL) ||
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR) ||
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_UNCONNECTED)
		) {
			return 0;
		}
// MH810104 GG119201(E) 電子ジャーナル対応
	}
	return 1;
}
/*[]-----------------------------------------------------------------------[]*/
/*| LCD７行目にﾜｰﾆﾝｸﾞ表示				   		                            |*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : time = ﾒｯｾｰｼﾞ表示時間									|*/
/*|						    x500msの値（例：10=5sec）						|*/
/*|							   0=時間指定なし（表示は戻さない）。			|*/
/*|				   pMessage = 表示メッセージ１行分（30文字へのﾎﾟｲﾝﾀ）		|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005-11-14                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ope2_WarningDisp( short time, const uchar *pMessage )
{
	Lcd_WmsgDisp_ON( 1, COLOR_RED,  LCD_BLINK_OFF, pMessage );	// ﾒｯｾｰｼﾞ反転表示
	if( time ){									// 表示時間指定あり
		LagTim500ms( LAG500_ERROR_DISP_DELAY, (short)(time + 1), Ope2_WarningDispEnd );
												// 指定時間後にﾒｯｾｰｼﾞ消す
	}
}

void	Ope2_WarningDispEnd( void )
{
	if( Miryo_Exec_check() )					// 現在未了中の場合は
		return;									// 画面消去させない
	LagCan500ms( LAG500_ERROR_DISP_DELAY );
	Lcd_WmsgDisp_OFF();
	Lcd_WmsgDisp_OFF2();
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//// MH321800(S) D.Inaba ICクレジット対応
//	ec_MessagePtnNum = 0;
//// MH321800(E) D.Inaba ICクレジット対応
	ECCTL.ec_MessagePtnNum = 0;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
}

// MH321800(S) G.So ICクレジット対応
/*[]-----------------------------------------------------------------------[]*/
/*| LCD 6と7行目にワーニングをサイクリック表示開始 	                        |*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : interval		= メッセージ表示間隔						|*/
/*|						    		x500msの値 (例：10=5sec)				|*/
/*|						   			0を指定された場合サイクリックしない		|*/
/*|				   err_num		= 表示するエラーメッセージ(ERR_CHR[])の番号	|*/
/*|				  					[0]:1ページ6行目、[1]:1ページ7行目		|*/
/*|				  					[2]:2ページ6行目、[3]:2ページ7行目		|*/
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
/*|				   mode			= 0=通常、1=反転							|*/
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                     |*/
/*| Date         : 2019-01-29                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
//void	Ope2_ErrChrCyclicDispStart( short interval, const uchar err_num[] )
void	Ope2_ErrChrCyclicDispStart( short interval, const uchar err_num[], uchar mode )
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
{
	// 画面表示
	if( DspSts == LCD_WMSG_ON || DspSts2 == LCD_WMSG_ON ) {
	// エラーメッセージ表示中
		LagCan500ms(LAG500_ERROR_DISP_DELAY);		// タイマーキャンセル
	}

	if (interval > 0) {
		// サイクリック表示データ保持
		Ope2CyclicDisp.interval = (interval / 500) + 1;
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
		Ope2CyclicDisp.mode = mode;
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
		Ope2CyclicDisp.err_page[0][0] = err_num[0];			// 1ページ目(6行目)
		Ope2CyclicDisp.err_page[0][1] = err_num[1];			// 1ページ目(7行目)
		Ope2CyclicDisp.err_page[1][0] = err_num[2];			// 2ページ目(6行目)
		Ope2CyclicDisp.err_page[1][1] = err_num[3];			// 2ページ目(7行目)
		Ope2CyclicDisp.page = 1;							// 1ページ目を表示

		// 初回表示
		Ope2_ErrChrCyclicDisp();
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*| LCD 6と7行目にワーニングをサイクリック表示停止 	                        |*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : none														|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                     |*/
/*| Date         : 2019-01-29                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	Ope2_ErrChrCyclicDispStop( void )
{
	if (Ope2CyclicDisp.page != 0) {
		LagCan500ms(LAG500_ERROR_DISP_DELAY);		// タイマーキャンセル
		Ope2CyclicDisp.page = 0;
		Lcd_WmsgDisp_OFF2();
		Lcd_WmsgDisp_OFF();
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*| LCD 6と7行目にワーニングをサイクリック表示	 	                        |*/
/*[]-----------------------------------------------------------------------[]*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                     |*/
/*| Date         : 2019-01-29                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	Ope2_ErrChrCyclicDisp( void )
{
	uchar	page;
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
	uchar	mode;
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応

	if (Ope2CyclicDisp.page == 0) {
		return;
	}

	// 画面切り替え
	page = Ope2CyclicDisp.page;
	page--;
	Lcd_WmsgDisp_OFF2();
	Lcd_WmsgDisp_OFF();
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
//	Lcd_WmsgDisp_ON2( 0, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[Ope2CyclicDisp.err_page[page][0]] );	// メッセージ表示(6行目)
//	Lcd_WmsgDisp_ON ( 0, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[Ope2CyclicDisp.err_page[page][1]] );	// メッセージ表示(7行目)
	mode = Ope2CyclicDisp.mode;
	Lcd_WmsgDisp_ON2( (ushort)mode, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[Ope2CyclicDisp.err_page[page][0]] );	// メッセージ表示(6行目)
	Lcd_WmsgDisp_ON ( (ushort)mode, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[Ope2CyclicDisp.err_page[page][1]] );	// メッセージ表示(7行目)
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
	Ope2CyclicDisp.page = (page == 0) ? 2 : 1;

	// 指定時間後にメッセージを切換える
	LagTim500ms( LAG500_ERROR_DISP_DELAY, Ope2CyclicDisp.interval, Ope2_ErrChrCyclicDisp );
}
// MH321800(E) G.So ICクレジット対応

short chkdate2(short yyyy, short mm, short dd)
{
	/* data  check */
	if((yyyy < 1980) || (yyyy > 2079)){
		return(-1);		/* YEAR  ERROR */
	}
	if((mm < 1) || (mm > 12)){
		return(-1);		/* MONTH ERROR */
	}
	if((dd < 1) || (dd > medget(yyyy,mm))){
		return(-1);		/* DAY   ERROR */
	}

	return(0);	/* NON ERROR */
}

/*[]-----------------------------------------------------------------------[]*/
/*| 精算データ生成用付加精算情報編集処理		                            |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SetAddPayData( void )                                    |*/
/*| PARAM		 :	void													|*/
/*|	RETURN 	     :	void													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                              |*/
/*| Date         : 2012-02-13                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void SetAddPayData( void )
{
	int		i;
	int j;
	ushort curnum = 0;
	uchar uc_prm;
	ushort curnum_1 = 0;
	ushort curnum_2 = 0;
	ushort curnum_3 = 0;
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//	date_time_rec	wk_CLK_REC;
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
// MH810100(S) 2020/07/09 車番チケットレス(#4531 仕様変更 台数管理追番を台数管理をしなくても付与する)
	date_time_rec	wk_CLK_REC;
// MH810100(E) 2020/07/09 車番チケットレス(#4531 仕様変更 台数管理追番を台数管理をしなくても付与する)

	PayData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		PayData.ID = 56;
	}

	// NT-NET基本データセット
	PayData.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];// ｼｽﾃﾑID
	if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 23;							// 出口ﾃﾞｰﾀ
		}else{
			PayData.DataBasic.DataKind = 22;							// 事前ﾃﾞｰﾀ
		}
		if( OpeNtnetAddedInfo.PayMethod == 5 ){							// 精算方法=5:定期券更新
			PayData.DataBasic.DataKind = 22;							// 事前ﾃﾞｰﾀ
		}
	} else {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 57;							// 出口ﾃﾞｰﾀ
		}else{
			PayData.DataBasic.DataKind = 56;							// 事前ﾃﾞｰﾀ
		}
		if( OpeNtnetAddedInfo.PayMethod == 5 ){							// 精算方法=5:定期券更新
			PayData.DataBasic.DataKind = 56;							// 事前ﾃﾞｰﾀ
		}
	}
	PayData.DataBasic.DataKeep = 0;									// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	PayData.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];			// 駐車場№
	PayData.DataBasic.ModelCode = NTNET_MODEL_CODE;					// 機種ｺｰﾄﾞ
	PayData.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];			// 機械№
	PayData.DataBasic.Year = (uchar)( CLK_REC.year % 100 );			// 処理年
	PayData.DataBasic.Mon = (uchar)CLK_REC.mont;					// 処理月
	PayData.DataBasic.Day = (uchar)CLK_REC.date;					// 処理日
	PayData.DataBasic.Hour = (uchar)CLK_REC.hour;					// 処理時
	PayData.DataBasic.Min = (uchar)CLK_REC.minu;					// 処理分
	PayData.DataBasic.Sec = (uchar)CLK_REC.seco;					// 処理秒

	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		PayData.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_PAY);	// センター追番
	}
	PayData.SeqNo = GetNtDataSeqNo();											// シーケンシャルNo.の設定
	PayData.PayMethod = (uchar)OpeNtnetAddedInfo.PayMethod;			// 精算方法セット
	NTNET_Data152Save((void *)(&PayData.PayMethod), NTNET_152_PAYMETHOD);
	PayData.PayClass = (uchar)OpeNtnetAddedInfo.PayClass;			// 処理区分セット
	PayInfo_Class = PayData.PayClass;								// 精算情報データ用処理区分を待避
	if( PayData.chuusi == 1 ){										// 精算中止
		PayData.PayClass += 2;
		// 精算中止時の時刻をセット
		NTNetTime_152.Year = NTNetTime_152_wk.Year;					// 年
		NTNetTime_152.Mon  = NTNetTime_152_wk.Mon;					// 月
		NTNetTime_152.Day  = NTNetTime_152_wk.Day;					// 日
		NTNetTime_152.Hour = NTNetTime_152_wk.Hour;					// 時
		NTNetTime_152.Min  = NTNetTime_152_wk.Min;					// 分
		NTNetTime_152.Sec  = NTNetTime_152_wk.Sec;					// 秒
	}
	if( PayData.PayMethod == 5 ){									// 更新精算時
		if( PayData.PayClass == 0 || PayData.PayClass == 1 ){
			PayData.PayClass = 10;									// 処理区分販売完了
		}
		else if( PayData.PayClass == 2 || PayData.PayClass == 3 ){
			PayData.PayClass = 11;									// 処理区分販売取消
		}
	}

	// ntnet_decision_credit()内で設定を参照しているので、以下の処理も行う
	if( ( PayData.PayClass == 0 || PayData.PayClass == 1 ) ){
		if (ntnet_decision_credit(&PayData.credit)){				// ←金額はPayDataから取っているので統一する
			if(PayData.credit.cre_type == CREDIT_HOJIN){
				PayData.PayClass += 14;								// 14:法人カード精算 / 15:法人カード再精算
			}else{
				PayData.PayClass += 4;								// 4:ｸﾚｼﾞｯﾄ精算 / 5:ｸﾚｼﾞｯﾄ再精算
			}
		}
	}
	PayInfo_Class = PayData.PayClass;								// 精算情報データ用処理区分を待避
// 不具合修正(S) K.Onodera 2016/10/05 #1507 クレジット精算後の精算情報データ要求の処理年月日が精算時刻にならない[共通バグNo.1179]
//	if( PayData.PayClass == 0 ){
	if( PayData.PayClass == 0 || PayData.PayClass == 4 ){
// 不具合修正(E) K.Onodera 2016/10/05 #1507 クレジット精算後の精算情報データ要求の処理年月日が精算時刻にならない[共通バグNo.1179]
		// 精算開始時の時刻をセット
		NTNetTime_152.Year = NTNetTime_152_wk.Year;					// 年
		NTNetTime_152.Mon  = NTNetTime_152_wk.Mon;					// 月
		NTNetTime_152.Day  = NTNetTime_152_wk.Day;					// 日
		NTNetTime_152.Hour = NTNetTime_152_wk.Hour;					// 時
		NTNetTime_152.Min  = NTNetTime_152_wk.Min;					// 分
		NTNetTime_152.Sec  = NTNetTime_152_wk.Sec;					// 秒
	}

	for( i = 0; i < WTIK_USEMAX; i++ ){
		if (PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU) break;
		if (PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE) break;
	}
	if( i != WTIK_USEMAX ){
		// 減額/振替精算あり
		PayData.PayMode = 4;										// 精算ﾓｰﾄﾞ(遠隔精算)
	}else{
		PayData.PayMode = 0;										// 精算ﾓｰﾄﾞ(自動精算)
	}
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
	for( i = 0; i < DETAIL_SYU_MAX; i++ ){
		if (PayData.DetailData[i].DiscSyu == NTNET_FURIKAE_2) break;
	}
	if( i != DETAIL_SYU_MAX ){
		PayData.PayMode = 4;									// 精算ﾓｰﾄﾞ(遠隔精算)
	}else{
		if( !PayData.PayMode ){
			PayData.PayMode = OpeNtnetAddedInfo.PayMode;
		}
	}
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応
	NTNET_Data152Save((void *)&PayData.PayMode, NTNET_152_PAYMODE);

	for (i = 0; i < WTIK_USEMAX; i++) {
		if (PayData.DiscountData[i].DiscSyu == NTNET_SYUSEI_1) break;
	}
	if(i < WTIK_USEMAX) {
		if( PayData.PayClass == 2 || PayData.PayClass == 3 ){		// 精算中止、再精算中止
			if(i < (WTIK_USEMAX-3)){// オーバーアクセスガード
				memmove( &PayData.DiscountData[i], &PayData.DiscountData[i+3], sizeof( DISCOUNT_DATA )*(WTIK_USEMAX-(i+3)) );	// 割引情報
				memset( &PayData.DiscountData[WTIK_USEMAX-3], 0, sizeof( DISCOUNT_DATA )*3);	// 精算ﾃﾞｰﾀ割引情報作成ｴﾘｱｸﾘｱ
			}
		}else{
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//			if( PayData.Zengakufg&0x02 ){							// 未入金あり
//				if(i < (WTIK_USEMAX-2)){// オーバーアクセスガード
//					PayData.DiscountData[i+2].Discount = PayData.MMTwari;	// 未入金額セット
//				}
//			}else{
//				PayData.DiscountData[i].Discount = PayData.MMTwari;	// 払戻額セット
//			}
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
		}
	}
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	// 最後にチェックする
	if( Ope_SyubetuWariCheck(&PayData.DiscountData[0]) == TRUE){
		// クリアしたのでずらす
		// 前に１つ上書きで移動
		memmove(&PayData.DiscountData[0],&PayData.DiscountData[1],sizeof(DISCOUNT_DATA)*(WTIK_USEMAX-1));
		// 最後尾をクリアしておく
		memset(&PayData.DiscountData[WTIK_USEMAX-1],0,sizeof(DISCOUNT_DATA));
	}
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）

	for(i=0; i<WTIK_USEMAX; i++){
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
	}

	if( ryo_buf.ryos_fg == 1 ){										// 領収証発行?
		PayData.ReceiptIssue = 1;									// 領収証あり
		NTNET_Data152Save((void *)(&PayData.ReceiptIssue), NTNET_152_RECEIPTISSUE);
	}

	if( ryo_buf.ryo_flg >= 2 || PayData.PayMethod == 5 ){			// 定期券精算処理or定期券更新時
		PayData.teiki.ParkingNo =
						CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];	// 定期券駐車場№
		NTNET_Data152Save((void *)(&PayData.teiki.ParkingNo), NTNET_152_PARKNOINPASS);
		PayData.PassCheck = 1;										// ｱﾝﾁﾊﾟｽﾁｪｯｸしない
		if( prm_get( COM_PRM,S_PAS,(short)(2+10*(PayData.teiki.syu-1)),1,1 ) ){
			PayData.PassCheck = 0;									// ｱﾝﾁﾊﾟｽﾁｪｯｸする
			if( SKIP_APASS_CHK ){									// 強制ｱﾝﾁﾊﾟｽOFF
				PayData.PassCheck = 2;								// ｱﾝﾁﾊﾟｽﾁｪｯｸ(強制OFF)
			}
		}
		if(( CPrmSS[S_NTN][27] == 0 )&&								// 出口ﾃﾞｰﾀ?
		   ( PayData.PayClass != 2 && PayData.PayClass != 3 )&&
		   ( prm_get( COM_PRM,S_PAS,(short)(4+10*(PayData.teiki.syu-1)),1,1 ) )){	// 定期券種別毎ｶｳﾝﾄする設定?
			PayData.CountSet = 2;									// 在車ｶｳﾝﾄ-1する
		}else{
			PayData.CountSet = 1;									// 在車ｶｳﾝﾄしない
		}
		if( PayData.PayMethod == 5 ){
			PayData.CountSet = 1;									// 在車ｶｳﾝﾄしない
		}
	}else{
		PayData.PassCheck = 1;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
		if(( CPrmSS[S_NTN][27] == 0 )&&								// 出口ﾃﾞｰﾀ?
		   ( PayData.PayClass != 2 && PayData.PayClass != 3 )&&
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
		   ( PayData.PayMethod != 13 ) &&							// 後日精算でない？
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
		   ( prm_get( COM_PRM,S_SHA,(short)(2+((PayData.syu-1)*6)),1,1 ) )){	// 種別毎ｶｳﾝﾄする設定?
			PayData.CountSet = 2;									// 在車ｶｳﾝﾄ-1する
		}else{
			PayData.CountSet = 1;									// 在車ｶｳﾝﾄしない
		}
	}
	NTNET_Data152Save((void *)(&PayData.PassCheck), NTNET_152_ANTIPASSCHECK);
	if( PayData.OutKind == 98 ){
		PayData.CountSet = 1;										// 在車ｶｳﾝﾄしない
	}
	NTNET_Data152Save((void *)(&PayData.CountSet), NTNET_152_COUNTSET);
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	PayData.credit.CreditDate[0] = OpeNtnetAddedInfo.CreditDate[0];	// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(年)
//	PayData.credit.CreditDate[1] = OpeNtnetAddedInfo.CreditDate[1];	// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(月)
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	// 定期券ﾃﾞｰﾀは0固定
//	PayData.PascarCnt = 0;					// 定期車両カウント
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	if (PayData.ID != 56) {					// 22/56
		return;
	}
	for (i = 0; i < LOCK_MAX; i++) {
		// 料金種別毎に現在在車ｶｳﾝﾄ
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {
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
	// 定期車両カウント 0固定
	if (_is_ntnet_remote()) {												// 遠隔NT-NET設定のときのみデータセット
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039⑤駐車台数を本体で管理する
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039⑥用途別駐車台数切替なし
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][3];					// 満車台数
//				PayData.CarCnt1 = curnum;									// 現在台数
//				switch (PPrmSS[S_P02][1]) {									// 強制満空車ﾓｰﾄﾞ
//				case	1:													// 強制満車
//					PayData.Full[0] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[0] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum >= PPrmSS[S_P02][3]) {
//						PayData.Full[0] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039⑥用途別駐車台数２系統
//			case 4:															// 01-0039⑥用途別駐車台数３系統
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][7];					// 満車台数
//				PayData.CarCnt1 = curnum_1;									// 現在台数１
//				switch (PPrmSS[S_P02][5]) {									// 強制満空車モード1
//				case	1:													// 強制満車
//					PayData.Full[0] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[0] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						PayData.Full[0] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				PayData.FullNo2 = (ushort)PPrmSS[S_P02][11];				// 満車台数２
//				PayData.CarCnt2 = curnum_2;									// 現在台数２
//				switch (PPrmSS[S_P02][9]) {									// 強制満空車モード2
//				case	1:													// 強制満車
//					PayData.Full[1] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[1] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						PayData.Full[1] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				if (i == 4) {
//					PayData.FullNo3 = (ushort)PPrmSS[S_P02][15];// 満車台数３
//					PayData.CarCnt3 = curnum_3;								// 現在台数３
//					switch (PPrmSS[S_P02][13]) {							// 強制満空車モード3
//					case	1:												// 強制満車
//						PayData.Full[2] = 11;								// 「強制満車」状態セット
//						break;
//					case	2:												// 強制空車
//						PayData.Full[2] = 10;								// 「強制空車」状態セット
//						break;
//					default:												// 自動
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							PayData.Full[2] = 1;							// 「満車」状態セット
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
//			PayData.CarCntInfo.CarCntYear	= (wk_CLK_REC.Year % 100);	// 年
//			PayData.CarCntInfo.CarCntMon	= wk_CLK_REC.Mon;			// 月
//			PayData.CarCntInfo.CarCntDay	= wk_CLK_REC.Day;			// 日
//			PayData.CarCntInfo.CarCntHour	= wk_CLK_REC.Hour;			// 時
//			PayData.CarCntInfo.CarCntMin	= wk_CLK_REC.Min;			// 分
//			PayData.CarCntInfo.CarCntSeq	= GetVehicleCountSeqNo();	// 追番
//			PayData.CarCntInfo.Reserve1		= 0;						// 予備
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//		}
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
// MH810100(S) 2020/07/09 車番チケットレス(#4531 仕様変更 台数管理追番を台数管理をしなくても付与する)
			// 台数管理追番 ------------------------
			memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
			// 同一時分？
			if( GetVehicleCountDate() == Nrm_YMDHM( &wk_CLK_REC ) ){
				// 同一時分の場合は追い番を＋１
				AddVehicleCountSeqNo();
			}else{
				// 異なる時分の場合は追い番を０とする
				SetVehicleCountDate( Nrm_YMDHM( &wk_CLK_REC ) );
				ClrVehicleCountSeqNo();
			}
			PayData.CarCntInfo.CarCntYear	= (wk_CLK_REC.Year % 100);	// 年
			PayData.CarCntInfo.CarCntMon	= wk_CLK_REC.Mon;			// 月
			PayData.CarCntInfo.CarCntDay	= wk_CLK_REC.Day;			// 日
			PayData.CarCntInfo.CarCntHour	= wk_CLK_REC.Hour;			// 時
			PayData.CarCntInfo.CarCntMin	= wk_CLK_REC.Min;			// 分
			PayData.CarCntInfo.CarCntSeq	= GetVehicleCountSeqNo();	// 追番
			PayData.CarCntInfo.Reserve1		= 0;						// 予備
// MH810100(E) 2020/07/09 車番チケットレス(#4531 仕様変更 台数管理追番を台数管理をしなくても付与する)
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 不正・強制出庫用1精算情報ｾｯﾄ                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PayData_set_SK( void )                                  |*/
/*| PARAMETER    : pr_lokno  : 内部処理用駐車位置番号(1～324)              |*/
/*|              : paymethod : 精算方式                                    |*/
/*|              : payclass  : 処理区分                                    |*/
/*|              : outkind   : 0=通常精算, 1=強制出庫                      |*/
/*|              :             2=精算しない出庫, 3=不正出庫                |*/
/*|              : fustype   : 0=精算しない出庫, 1=（該当なし）            |*/
/*|                            2=修正の不正(料金有), 3=修正の不正(料金無)  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                             |*/
/*| Date         : 2012-02-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	PayData_set_SK( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind, uchar fustype )
{
	ushort	num;
	int i, j;
	ushort curnum = 0;
	uchar uc_prm;
	ushort curnum_1 = 0;
	ushort curnum_2 = 0;
	ushort curnum_3 = 0;

	PayData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		PayData.ID = 56;
	}

	num = pr_lokno - 1;

	PayData.WFlag = 0;												// 印字種別１（0:通常 1:復電）
	PayData.chuusi = 0;												// 印字種別２（0:領収証 1:精算中止 2:預り証）
	PayData.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// 機械№
	PayData.Seisan_kind = 0;										// 精算種別(自動固定)

	if(outkind == 1 || outkind == 11 ){
		CountGet( KIYOUSEI_COUNT, &PayData.Oiban );					// 強制出庫追番
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
	}else if(outkind == 3){
		CountGet( FUSEI_COUNT, &PayData.Oiban );					// 不正出庫追番
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
	}else{
		PayData.Oiban.i = 0L;										// 精算追い番
		PayData.Oiban.w = 0L;										// 精算追い番
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
	}
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		PayData.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_PAY);	// センター追番
	}
	PayData.SeqNo = GetNtDataSeqNo();											// シーケンシャルNo.の設定

	PayData.PayMethod = (uchar)paymethod;							// 精算方法
	if( (fustype == 2)||(fustype == 3) ){							// 修正精算での不正
		PayData.PayMethod = 10;										// 精算方法(修正元区分10)
	}
	NTNET_Data152Save((void *)(&PayData.PayMethod), NTNET_152_PAYMETHOD);
	PayData.PayClass = (uchar)payclass;								// 処理区分
	PayInfo_Class = PayData.PayClass;								// 精算情報データ用処理区分を待避
	PayData.PayMode = 0;											// 精算ﾓｰﾄﾞ
	NTNET_Data152Save((void *)&PayData.PayMode, NTNET_152_PAYMODE);
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );
//																	// 接客用駐車位置番号
//	NTNET_Data152Save((void *)(&PayData.WPlace), NTNET_152_WPLACE);
	PayData.WPlace = 0L;
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	PayData.TOutTime.Year = car_ot_f.year;							// 出庫	年
	PayData.TOutTime.Mon  = car_ot_f.mon;							//		月
	PayData.TOutTime.Day  = car_ot_f.day;							//		日
	PayData.TOutTime.Hour = car_ot_f.hour;							//		時
	PayData.TOutTime.Min  = car_ot_f.min;							//		分

	if(outkind == 1 || outkind == 11 ){
		PayData.KakariNo = Kakari_Numu[num];						// 係員№
	}else{
		PayData.KakariNo = 0;										// 係員№
	}
	NTNET_Data152Save((void *)(&PayData.KakariNo), NTNET_152_KAKARINO);
	PayData.OutKind = (uchar)outkind;								// 精算出庫
	NTNET_Data152Save((void *)(&PayData.OutKind), NTNET_152_OUTKIND);
	PayData.TInTime.Year = car_in_f.year;							// 入庫	年
	PayData.TInTime.Mon  = car_in_f.mon;							//		月
	PayData.TInTime.Day  = car_in_f.day;							//		日
	PayData.TInTime.Hour = car_in_f.hour;							//		時
	PayData.TInTime.Min  = car_in_f.min;							//		分

	PayData.syu = ryo_buf.tik_syu;									// 料金種別
	NTNET_Data152Save((void *)(&PayData.syu), NTNET_152_SYUBET);
	PayData.WPrice = ryo_buf.dsp_ryo;								// 駐車料金
	PayData.Wtax = ryo_buf.tax;										// 消費税
	if( fustype == 3 ){												// 修正精算の料金発生無しでの不正ﾃﾞｰﾀ
		PayData.WPrice = 0;											// 駐車料金
		PayData.Wtax = 0;											// 消費税額
	}
	NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
	PayData.PassCheck = 1;											// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	NTNET_Data152Save((void *)(&PayData.PassCheck), NTNET_152_ANTIPASSCHECK);

	if(( CPrmSS[S_NTN][27] == 0 )&&									// 出口ﾃﾞｰﾀ?
	   ( prm_get( COM_PRM,S_SHA,(short)(2+((ryo_buf.tik_syu-1)*6)),1,1 ) )){	// 種別毎ｶｳﾝﾄする設定?
		PayData.CountSet = 2;										// 在車ｶｳﾝﾄ-1する
		if( fustype == 2 ){											// 修正精算の在車ｶｳﾝﾄなしの不正ﾃﾞｰﾀ
			PayData.CountSet = 1;									// 在車ｶｳﾝﾄしない
		}
	}else{
		PayData.CountSet = 1;										// 在車ｶｳﾝﾄしない
	}
	NTNET_Data152Save((void *)(&PayData.CountSet), NTNET_152_COUNTSET);
	PayData.PayCalMax = ntnet_nmax_flg;								// 最大料金越えあり

	// NT-NET基本データセット
	PayData.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];// ｼｽﾃﾑID
	if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 23;							// 出口ﾃﾞｰﾀ
		}else{
			PayData.DataBasic.DataKind = 22;							// 事前ﾃﾞｰﾀ
		}
	} else {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 57;							// 出口ﾃﾞｰﾀ
		}else{
			PayData.DataBasic.DataKind = 56;							// 事前ﾃﾞｰﾀ
		}
	}
	PayData.DataBasic.DataKeep = 0;									// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	PayData.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];			// 駐車場№
	PayData.DataBasic.ModelCode = NTNET_MODEL_CODE;					// 機種ｺｰﾄﾞ
	PayData.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];			// 機械№
	PayData.DataBasic.Year = (uchar)( CLK_REC.year % 100 );			// 処理年
	PayData.DataBasic.Mon = (uchar)CLK_REC.mont;					// 処理月
	PayData.DataBasic.Day = (uchar)CLK_REC.date;					// 処理日
	PayData.DataBasic.Hour = (uchar)CLK_REC.hour;					// 処理時
	PayData.DataBasic.Min = (uchar)CLK_REC.minu;					// 処理分
	PayData.DataBasic.Sec = (uchar)CLK_REC.seco;					// 処理秒

// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	// 定期券ﾃﾞｰﾀは0固定
//	PayData.PascarCnt = 0;					// 定期車両カウント
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	if (PayData.ID != 56) {					// 22/56
		// 精算情報データ用強制・不正出庫データを待避
		memcpy(&PayInfoData_SK, &PayData, sizeof(PayData));
		return;
	}
	for (i = 0; i < LOCK_MAX; i++) {
		// 料金種別毎に現在在車ｶｳﾝﾄ
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {
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
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	// 定期車両カウント 0固定
//	if (_is_ntnet_remote()) {												// 遠隔NT-NET設定のときのみデータセット
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039⑤駐車台数を本体で管理する
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039⑥用途別駐車台数切替なし
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][3];					// 満車台数
//				PayData.CarCnt1 = curnum;									// 現在台数
//				switch (PPrmSS[S_P02][1]) {									// 強制満空車ﾓｰﾄﾞ
//				case	1:													// 強制満車
//					PayData.Full[0] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[0] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum >= PPrmSS[S_P02][3]) {
//						PayData.Full[0] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039⑥用途別駐車台数２系統
//			case 4:															// 01-0039⑥用途別駐車台数３系統
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][7];					// 満車台数
//				PayData.CarCnt1 = curnum_1;									// 現在台数１
//				switch (PPrmSS[S_P02][5]) {									// 強制満空車モード1
//				case	1:													// 強制満車
//					PayData.Full[0] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[0] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						PayData.Full[0] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				PayData.FullNo2 = (ushort)PPrmSS[S_P02][11];				// 満車台数２
//				PayData.CarCnt2 = curnum_2;									// 現在台数２
//				switch (PPrmSS[S_P02][9]) {									// 強制満空車モード2
//				case	1:													// 強制満車
//					PayData.Full[1] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[1] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						PayData.Full[1] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				if (i == 4) {
//					PayData.FullNo3 = (ushort)PPrmSS[S_P02][15];			// 満車台数３
//					PayData.CarCnt3 = curnum_3;								// 現在台数３
//					switch (PPrmSS[S_P02][13]) {							// 強制満空車モード3
//					case	1:												// 強制満車
//						PayData.Full[2] = 11;								// 「強制満車」状態セット
//						break;
//					case	2:												// 強制空車
//						PayData.Full[2] = 10;								// 「強制空車」状態セット
//						break;
//					default:												// 自動
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							PayData.Full[2] = 1;							// 「満車」状態セット
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
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	// 精算情報データ用強制・不正出庫データを待避
	memcpy(&PayInfoData_SK, &PayData, sizeof(PayData));
}
// 構造体のメンバーのオフセットを求めるマクロ
#define OFS_MEMBER(x, y)	( (ulong)&((x*)wp)-> ## y - (ulong)wp )

//[]----------------------------------------------------------------------[]
///	@brief		LOGレコードから日付データを得る
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: LOG種別
///	@return		ushort		: オフセット値
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/03/12<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static date_time_rec *get_log_date_time_rec( ushort LogSyu, uchar *record )
{
	ushort	ofs;
	void	*wp;
	wp		= 0;

	switch( LogSyu ){
	case eLOG_PAYMENT:
		ofs = OFS_MEMBER(Receipt_data, TOutTime);
		break;
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	case eLOG_ENTER:
//		ofs = OFS_MEMBER(enter_log, InTime);
//		break;
	case eLOG_RTPAY:
		ofs = OFS_MEMBER(RTPay_log_date, dtTimeYtoSec);
		break;
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	case eLOG_RTRECEIPT:
		ofs = OFS_MEMBER(RTReceipt_log_date, dtTimeYtoSec);
		break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	case eLOG_TTOTAL:
		ofs = OFS_MEMBER(SYUKEI, NowTime);
		break;
	case eLOG_ERROR:
		ofs = OFS_MEMBER(Err_log, Date_Time);
		break;
	case eLOG_ALARM:
		ofs = OFS_MEMBER(Arm_log, Date_Time);
		break;
	case eLOG_OPERATE:
		ofs = OFS_MEMBER(Ope_log, Date_Time);
		break;
	case eLOG_MONITOR:
		ofs = OFS_MEMBER(Mon_log, Date_Time);
		break;
	case eLOG_ABNORMAL:
		ofs = OFS_MEMBER(flp_log, Date_Time);
		break;
	case eLOG_MONEYMANAGE:
		ofs = OFS_MEMBER(TURI_KAN, NowTime);
		break;
	case eLOG_PARKING:
		ofs = OFS_MEMBER(ParkCar_log, Time);
		break;
	case eLOG_NGLOG:
		ofs = OFS_MEMBER(NGLOG_DATA, NowTime);
		break;
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
//	case eLOG_IOLOG:
//		ofs = OFS_MEMBER(IOLOG_DATA, NowTime);
	case eLOG_DC_QR:
		ofs = OFS_MEMBER(DC_QR_log_date, dtTimeYtoSec);
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
		break;
	case eLOG_CREUSE:
	case eLOG_HOJIN_USE:
		ofs = OFS_MEMBER(meisai_log, PayTime);
		break;
	case eLOG_REMOTE_SET:
		ofs = OFS_MEMBER(t_Change_data, rcv_Time);
		break;
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//	case eLOG_LONGPARK:
//		ofs = OFS_MEMBER(LongPark_log, Time);
	case eLOG_LONGPARK_PWEB:
		ofs = OFS_MEMBER(LongPark_log_Pweb, ProcDate);
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
		break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	case eLOG_RISMEVENT:
//		ofs = OFS_MEMBER(RismEvent_log, Time);
	case eLOG_DC_LANE:
		ofs = OFS_MEMBER(DC_LANE_log_date, dtTimeYtoSec);
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		break;
	case eLOG_GTTOTAL:
		ofs = OFS_MEMBER(SYUKEI, NowTime);
		break;
	case eLOG_COINBOX:
		ofs = OFS_MEMBER(COIN_SYU, NowTime);
		break;
	case eLOG_NOTEBOX:
		ofs = OFS_MEMBER(NOTE_SYU, NowTime);
		break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	case eLOG_EDYARM:
//		ofs = OFS_MEMBER(edy_arm_log, TOutTime);
//		break;
//	case eLOG_EDYSHIME:
//		ofs = OFS_MEMBER(edy_shime_log, ShimeTime);
//		break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	case eLOG_POWERON:
		ofs = OFS_MEMBER(Pon_log, Pdw_Date);	// 停電日時
		break;
	case eLOG_MNYMNG_SRAM:
		ofs = OFS_MEMBER(TURI_KAN, NowTime);
		break;
	default:	// parameter error
		ofs = 0;
		break;
	}
	return (date_time_rec*)(record+ofs);
}
/*[]----------------------------------------------------------------------[]*/
/*| フラップ上昇、ロック閉ﾀｲﾏ内出庫用1精算情報ｾｯﾄ                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PayData_set_LO( void )                                  |*/
/*| PARAMETER    : pr_lokno  : 内部処理用駐車位置番号(1～324)              |*/
/*|              : paymethod : 精算方式                                    |*/
/*|              : payclass  : 処理区分                                    |*/
/*|              : outkind   : 0=通常精算, 1=強制出庫                      |*/
/*|              :             2=精算しない出庫, 3=不正出庫                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : A.iiizumi                                               |*/
/*| Date         : 2012-06-14                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	PayData_set_LO( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind)
{
	ushort	num;
	int i, j;
	ushort curnum = 0;
	uchar uc_prm;
	ushort curnum_1 = 0;
	ushort curnum_2 = 0;
	ushort curnum_3 = 0;

	PayData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		PayData.ID = 56;
	}

	num = pr_lokno - 1;

	PayData.WFlag = 0;												// 印字種別１（0:通常 1:復電）
	PayData.chuusi = 0;												// 印字種別２（0:領収証 1:精算中止 2:預り証）
	PayData.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// 機械№
	PayData.Seisan_kind = 0;										// 精算種別(自動固定)
	CountGet( PAYMENT_COUNT, &PayData.Oiban );						// 精算追い番
	NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);

	PayData.PayMethod = (uchar)paymethod;							// 精算方法
	NTNET_Data152Save((void *)(&PayData.PayMethod), NTNET_152_PAYMETHOD);

	PayData.PayClass = (uchar)payclass;								// 処理区分
	PayData.PayMode = 0;											// 精算ﾓｰﾄﾞ
	NTNET_Data152Save((void *)&PayData.PayMode, NTNET_152_PAYMODE);
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );
//																	// 接客用駐車位置番号
//	NTNET_Data152Save((void *)(&PayData.WPlace), NTNET_152_WPLACE);
	PayData.WPlace = 0L;
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	PayData.TOutTime.Year = car_ot_f.year;							// 出庫	年
	PayData.TOutTime.Mon  = car_ot_f.mon;							//		月
	PayData.TOutTime.Day  = car_ot_f.day;							//		日
	PayData.TOutTime.Hour = car_ot_f.hour;							//		時
	PayData.TOutTime.Min  = car_ot_f.min;							//		分

	PayData.KakariNo = 0;											// 係員№
	NTNET_Data152Save((void *)(&PayData.KakariNo), NTNET_152_KAKARINO);
	PayData.OutKind = (uchar)outkind;								// 精算出庫
	NTNET_Data152Save((void *)(&PayData.OutKind), NTNET_152_OUTKIND);
	PayData.TInTime.Year = car_in_f.year;							// 入庫	年
	PayData.TInTime.Mon  = car_in_f.mon;							//		月
	PayData.TInTime.Day  = car_in_f.day;							//		日
	PayData.TInTime.Hour = car_in_f.hour;							//		時
	PayData.TInTime.Min  = car_in_f.min;							//		分

	PayData.Wtax = 0;												// 消費税
	PayData.syu = (ushort)LockInfo[num].ryo_syu;					// 料金種別
	NTNET_Data152Save((void *)(&PayData.syu), NTNET_152_SYUBET);
	PayData.WPrice = 0;												// 駐車料金
	NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
	PayData.PassCheck = 1;											// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	NTNET_Data152Save((void *)(&PayData.PassCheck), NTNET_152_ANTIPASSCHECK);

	if(( CPrmSS[S_NTN][27] == 0 )&&									// 出口ﾃﾞｰﾀ?
	   ( prm_get( COM_PRM,S_SHA,(short)(2+((ryo_buf.tik_syu-1)*6)),1,1 ) )){	// 種別毎ｶｳﾝﾄする設定?
		PayData.CountSet = 2;										// 在車ｶｳﾝﾄ-1する
	}else{
		PayData.CountSet = 1;										// 在車ｶｳﾝﾄしない
	}
	NTNET_Data152Save((void *)(&PayData.CountSet), NTNET_152_COUNTSET);

	// NT-NET基本データセット
	PayData.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];// ｼｽﾃﾑID
	if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 23;							// 出口ﾃﾞｰﾀ
		}else{
			PayData.DataBasic.DataKind = 22;							// 事前ﾃﾞｰﾀ
		}
	} else {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 57;							// 出口ﾃﾞｰﾀ
		}else{
			PayData.DataBasic.DataKind = 56;							// 事前ﾃﾞｰﾀ
		}
	}
	PayData.DataBasic.DataKeep = 0;									// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	PayData.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];			// 駐車場№
	PayData.DataBasic.ModelCode = NTNET_MODEL_CODE;					// 機種ｺｰﾄﾞ
	PayData.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];			// 機械№
	PayData.DataBasic.Year = (uchar)( CLK_REC.year % 100 );			// 処理年
	PayData.DataBasic.Mon = (uchar)CLK_REC.mont;					// 処理月
	PayData.DataBasic.Day = (uchar)CLK_REC.date;					// 処理日
	PayData.DataBasic.Hour = (uchar)CLK_REC.hour;					// 処理時
	PayData.DataBasic.Min = (uchar)CLK_REC.minu;					// 処理分
	PayData.DataBasic.Sec = (uchar)CLK_REC.seco;					// 処理秒

	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		PayData.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_PAY);	// センター追番
	}
	PayData.SeqNo = GetNtDataSeqNo();											// シーケンシャルNo.の設定
	
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	// 定期券ﾃﾞｰﾀは0固定
//	PayData.PascarCnt = 0;					// 定期車両カウント
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	if (PayData.ID != 56) {					// 22/56
		// フラップ上昇、ロック閉ﾀｲﾏ内出庫データを待避
		PayInfo_Class = 102;
		memcpy(&PayInfoData_SK, &PayData, sizeof(PayData));
		return;
	}
	for (i = 0; i < LOCK_MAX; i++) {
		// 料金種別毎に現在在車ｶｳﾝﾄ
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {
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
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	// 定期車両カウント 0固定
//	if (_is_ntnet_remote()) {												// 遠隔NT-NET設定のときのみデータセット
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039⑤駐車台数を本体で管理する
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039⑥用途別駐車台数切替なし
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][3];					// 満車台数
//				PayData.CarCnt1 = curnum;									// 現在台数
//				switch (PPrmSS[S_P02][1]) {									// 強制満空車ﾓｰﾄﾞ
//				case	1:													// 強制満車
//					PayData.Full[0] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[0] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum >= PPrmSS[S_P02][3]) {
//						PayData.Full[0] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039⑥用途別駐車台数２系統
//			case 4:															// 01-0039⑥用途別駐車台数３系統
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][7];					// 満車台数
//				PayData.CarCnt1 = curnum_1;									// 現在台数１
//				switch (PPrmSS[S_P02][5]) {									// 強制満空車モード1
//				case	1:													// 強制満車
//					PayData.Full[0] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[0] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						PayData.Full[0] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				PayData.FullNo2 = (ushort)PPrmSS[S_P02][11];				// 満車台数２
//				PayData.CarCnt2 = curnum_2;									// 現在台数２
//				switch (PPrmSS[S_P02][9]) {									// 強制満空車モード2
//				case	1:													// 強制満車
//					PayData.Full[1] = 11;									// 「強制満車」状態セット
//					break;
//				case	2:													// 強制空車
//					PayData.Full[1] = 10;									// 「強制空車」状態セット
//					break;
//				default:													// 自動
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						PayData.Full[1] = 1;								// 「満車」状態セット
//					}
//					break;
//				}
//				if (i == 4) {
//					PayData.FullNo3 = (ushort)PPrmSS[S_P02][15];			// 満車台数３
//					PayData.CarCnt3 = curnum_3;								// 現在台数３
//					switch (PPrmSS[S_P02][13]) {							// 強制満空車モード3
//					case	1:												// 強制満車
//						PayData.Full[2] = 11;								// 「強制満車」状態セット
//						break;
//					case	2:												// 強制空車
//						PayData.Full[2] = 10;								// 「強制空車」状態セット
//						break;
//					default:												// 自動
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							PayData.Full[2] = 1;							// 「満車」状態セット
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
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	// フラップ上昇、ロック閉ﾀｲﾏ内出庫データを待避
	PayInfo_Class = 102;
	memcpy(&PayInfoData_SK, &PayData, sizeof(PayData));
}

//[]----------------------------------------------------------------------[]
///	@brief			係員ｶｰﾄﾞﾁｪｯｸ処理
//[]----------------------------------------------------------------------[]
///	@return			ret : 1:OK -1:ｶｰﾄﾞｴﾗｰ 0:不正ｶｰﾄﾞﾘﾀｰﾝ 
///	@author			okuda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/12/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
short	Kakariin_card( void )
{
	short		ret = 1;	// 戻り値
	short		anm = 0;	// 案内ﾒｯｾｰｼﾞ番号
	m_kakari	*crd_data;	// 磁気ｶｰﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ
	long		pk_no;		// 駐車場Ｎｏ．
	char		card_read= OK;	// カード有効フラグ
	uchar		id_syu_settei;	// 使用券ID
	uchar		data[2];		// LOG登録時に使用する係員番号

	crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];

	if (prm_get(COM_PRM, S_PAY, 10, 1, 4) != 0) {// 新カードＩＤ使用する？
	//使用する
		id_syu_settei = (uchar)prm_get(COM_PRM,S_PAY,10,1,1);	// 使用券ＩＤ種設定取得
		switch( id_syu_settei ){
		case 1:
		case 2:
		case 3:
			id_syu_settei = (uchar)(id_syu_settei - 1);
			break;
		case 8:
			id_syu_settei = 5;
			break;
		default:// 読まない,その他(設定エラー)
			if(MAGred[MAG_GT_APS_TYPE] == 1){				// GTフォーマット
				id_syu_settei = 0;
			}else{
				ret = -1;
				anm = AVM_CARD_ERR1;
				card_read = NG;
			}
			break;
		}
		// 係員カードチェック
		if(card_read == OK){
// NOTE:Read_Tik_Chk()で「券種別」は強制的に0x41に書き換えられる。判定NGの場合、Read_Tik_Chk()で駐車場NoをNGのものに強制的
// に書き換えるためこの関数での判定は券種別がID1であることを判定すれば良い
			if( crd_data->kkr_idc == 0x41 ){// 券種別:係員カード
				if( ( crd_data->kkr_did != 0x53 )// データＩＤ=53H
						||
					( (MAGred[MAG_GT_APS_TYPE] == 0 ) &&	// APSフォーマットでPAK
					( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x41 || crd_data->kkr_park[2] != 0x4b ) ) 
						||
					( (MAGred[MAG_GT_APS_TYPE] == 1 ) &&	// GTフォーマットでPGT
					( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x47 || crd_data->kkr_park[2] != 0x54 ) ) ){
					// 係員カード以外
					ret = -1;
					anm = AVM_CARD_ERR1;
					card_read = NG;
				}
					///////ここのパスに来た時のみ係員カードの中身を判定する
			} else {
				// 係員カード以外
				ret = -1;
				anm = AVM_CARD_ERR1;
				card_read = NG;
			}
		}
	} else {
	//使用しない
		// 係員カードチェック
		if(	( crd_data->kkr_idc != 0x41 )
				||
			( crd_data->kkr_did != 0x53 )
				||
			( (MAGred[MAG_GT_APS_TYPE] == 0 ) &&
				( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x41 || crd_data->kkr_park[2] != 0x4b ) ) 
				||
			( (MAGred[MAG_GT_APS_TYPE] == 1 ) &&
				( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x47 || crd_data->kkr_park[2] != 0x54 ) ) ){
			// 係員カード以外
			ret = -1;
			anm = 44;
			card_read = NG;
		}
	///////ここのパスに来た時のみ係員カードの中身を判定する
	}
	
	if(card_read == OK){
		// 係員カード
		for( ; ; ){
			if( crd_data->kkr_type != 0x20 ){							// 係員カードではない
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}
			if( MAGred[MAG_GT_APS_TYPE] == 1 ){//GTフォーマット

				pk_no = (	( (crd_data->kkr_rsv2[4] & 0x0f) * 100000L ) +		// 駐車場Ｎｏ．取得
							( (crd_data->kkr_rsv2[5] & 0x0f) * 10000L )  +
							( (crd_data->kkr_pno[0] & 0x0f) * 1000L )  +
							( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
							( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
							( (crd_data->kkr_pno[3] & 0x0f) * 1L )
						);

			}else{//APSフォーマット
				pk_no = (	( (crd_data->kkr_pno[0] & 0x0f) * 1000L ) +		// 駐車場Ｎｏ．取得
							( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
							( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
							( (crd_data->kkr_pno[3] & 0x0f) * 1L )
						);
			}

			if( CPrmSS[S_SYS][1] != pk_no ){							// 駐車場Ｎｏ．＝基本駐車場Ｎｏ．？
				// 駐車場Ｎｏ．が一致しない場合
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}
			//GTフォーマットチェック
			if( ( prm_get( COM_PRM,S_SYS,12,1,6 ) == 0 ) && (MAGred[MAG_GT_APS_TYPE] == 1) ||
				( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1 ) && (MAGred[MAG_GT_APS_TYPE] == 0) ){
				//GTフォーマット未対応の場合
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}

			/* 係員番号 01～99ﾁｪｯｸ */
			pk_no = (	( (crd_data->kkr_kno[0] & 0x0f) * 1000 ) +		// 係員Ｎｏ．取得
						( (crd_data->kkr_kno[1] & 0x0f) * 100 )  +
						( (crd_data->kkr_kno[2] & 0x0f) * 10 )   +
						( (crd_data->kkr_kno[3] & 0x0f) * 1 )
					);

			if( (pk_no < 1) || (99 < pk_no) ){
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}

			/* 役割ｺｰﾄﾞ 1～4ﾁｪｯｸ */
			if( (crd_data->kkr_role < '1') || ('4' < crd_data->kkr_role) ){
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}
			memcpy(data,&crd_data->kkr_kno[2],sizeof(data));			// ｶｰﾄﾞNoの取得	
			if(pk_no != 99 && -1 == AteVaild_Check((ushort)pk_no)) {	// 有効係員データ登録なし？(No.99はチェック対象外)
				NgLog_write( NG_CARD_AMANO, data, sizeof(data) );		// 不正ｶｰﾄﾞLOG登録を行う
				ret = 0;
				anm = AVM_CARD_ERR3;									// このカードは無効です。
				break;
			}
			
			// ここまでbreakされないので、正常カード
			NgLog_write( OK_CARD_AMANO, data, sizeof(data) );			// 正常カードLOG登録を行う
			break;														// カードチェックＯＫ
		}

	}
	if( anm != 0 ){
		// 案内ﾒｯｾｰｼﾞ送出ありの場合
		ope_anm(anm);
	}
	return( ret );
}

//[]----------------------------------------------------------------------[]
///	@brief			係員カード内の役割コードと係員№をHEX値で返す
//[]----------------------------------------------------------------------[]
///	@param[in]		pMntLevel	ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙ値(1-4)ｾｯﾄｴﾘｱへのﾎﾟｲﾝﾀ
///	@param[in]		pPassLevel	ﾊﾟｽﾜｰﾄﾞﾚﾍﾞﾙ(0-5)ｾｯﾄｴﾘｱへのﾎﾟｲﾝﾀ
///	@param[in]		pKakariNum	係員番号(0-254)ｾｯﾄｴﾘｱへのﾎﾟｲﾝﾀ
///	@return			ret			1=リジェクト実施、0=実施してない
///	@author			Okuda
///	@note			MAG_red[] に係員カード情報がある場合のみ有効。<br>
///					メンテナンスレベルは1～4の値を返す（0の場合1とする）
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/12/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	Ope_KakariCardInfoGet( uchar *pMntLevel, uchar* pPassLevel, uchar *pKakariNum )
{
	m_kakari	*crd_data;							// 磁気ｶｰﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ
	ushort		wkus;

	crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];

	wkus = (	( (crd_data->kkr_kno[0] & 0x0f) * 1000 ) +		// 係員Ｎｏ．取得
				( (crd_data->kkr_kno[1] & 0x0f) * 100 )  +
				( (crd_data->kkr_kno[2] & 0x0f) * 10 )   +
				( (crd_data->kkr_kno[3] & 0x0f) * 1 )
			);
	*pKakariNum = (uchar)wkus;

	wkus = (ushort)(crd_data->kkr_role - '0');		// 役割コード Hex get
	if( (wkus < 1) || (4 < wkus) ){
		wkus = 1;
	}
	*pMntLevel = (uchar)wkus;
	
	wkus = (ushort)(crd_data->kkr_lev - '0');		// レベル
	if( wkus > 5 ) {
		wkus = 0;
	}
	*pPassLevel = (uchar)wkus;
}
/*[]-----------------------------------------------------------------------[]*/
/*| 精算中止時の精算情報データ生成用付加精算情報編集処理		            |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SetNoPayData( void )                                     |*/
/*| PARAM		 :	void													|*/
/*|	RETURN 	     :	void													|*/
/*[]-------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
void SetNoPayData( void )
{
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	ushort num;
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

	PayData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		PayData.ID = 56;
	}

	// NT-NET基本データセット
	PayData.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];// ｼｽﾃﾑID
	if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 23;						// 出口ﾃﾞｰﾀ
		}else{
			PayData.DataBasic.DataKind = 22;						// 事前ﾃﾞｰﾀ
		}
		if( OpeNtnetAddedInfo.PayMethod == 5 ){						// 精算方法=5:定期券更新
			PayData.DataBasic.DataKind = 22;						// 事前ﾃﾞｰﾀ
		}
	} else {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 57;						// 出口ﾃﾞｰﾀ
		}else{
			PayData.DataBasic.DataKind = 56;						// 事前ﾃﾞｰﾀ
		}
		if( OpeNtnetAddedInfo.PayMethod == 5 ){						// 精算方法=5:定期券更新
			PayData.DataBasic.DataKind = 56;						// 事前ﾃﾞｰﾀ
		}
	}
	PayData.DataBasic.DataKeep = 0;									// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	PayData.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];			// 駐車場№
	PayData.DataBasic.ModelCode = NTNET_MODEL_CODE;					// 機種ｺｰﾄﾞ
	PayData.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];			// 機械№
	PayData.DataBasic.Year = (uchar)( CLK_REC.year % 100 );			// 処理年
	PayData.DataBasic.Mon = (uchar)CLK_REC.mont;					// 処理月
	PayData.DataBasic.Day = (uchar)CLK_REC.date;					// 処理日
	PayData.DataBasic.Hour = (uchar)CLK_REC.hour;					// 処理時
	PayData.DataBasic.Min = (uchar)CLK_REC.minu;					// 処理分
	PayData.DataBasic.Sec = (uchar)CLK_REC.seco;					// 処理秒

	// 精算中止時の時刻をセット
	NTNetTime_152.Year = CLK_REC.year;								// 年
	NTNetTime_152.Mon  = CLK_REC.mont;								// 月
	NTNetTime_152.Day  = CLK_REC.date;								// 日
	NTNetTime_152.Hour = CLK_REC.hour;								// 時
	NTNetTime_152.Min  = CLK_REC.minu;								// 分
	NTNetTime_152.Sec  = (ushort)CLK_REC.seco;						// 秒

	PayData.PayMethod = (uchar)OpeNtnetAddedInfo.PayMethod;			// 精算方法セット
	NTNET_Data152Save((void *)(&PayData.PayMethod), NTNET_152_PAYMETHOD);
	PayData.PayClass = (uchar)OpeNtnetAddedInfo.PayClass;			// 処理区分セット
	PayInfo_Class = PayData.PayClass;								// 精算情報データ用処理区分を待避
	if( PayData.PayMethod == 5 ){									// 更新精算時
		if( PayData.PayClass == 0 || PayData.PayClass == 1 ){
			PayData.PayClass = 10;									// 処理区分販売完了
		}
		else if( PayData.PayClass == 2 || PayData.PayClass == 3 ){
			PayData.PayClass = 11;									// 処理区分販売取消
		}
	}

// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	num = ryo_buf.pkiti - 1;
//	PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// 接客用駐車位置番号
//	NTNET_Data152Save((void *)(&PayData.WPlace), NTNET_152_WPLACE);
	PayData.WPlace = 0L;
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

	if( ryo_buf.ryo_flg < 2 ){										// 駐車券精算処理
		PayData.WPrice = ryo_buf.tyu_ryo;							// 駐車料金
	}else{
		PayData.WPrice = ryo_buf.tei_ryo;							// 定期料金
	}
	NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
	PayData.syu = (char)(ryo_buf.syubet + 1);						// 駐車種別
	NTNET_Data152Save((void *)(&PayData.syu), NTNET_152_SYUBET);

	// ntnet_decision_credit()内で設定を参照しているので、以下の処理も行う
	if( ( PayData.PayClass == 0 || PayData.PayClass == 1 ) ){
		if (ntnet_decision_credit(&PayData.credit)){				// ←金額はPayDataから取っているので統一する
			if(PayData.credit.cre_type == CREDIT_HOJIN){
				PayData.PayClass += 14;								// 14:法人カード精算 / 15:法人カード再精算
			}else{
				PayData.PayClass += 4;								// 4:ｸﾚｼﾞｯﾄ精算 / 5:ｸﾚｼﾞｯﾄ再精算
			}
		}
	}
	PayInfo_Class = 2;												// 処理区分:精算中止
	PayData.CountSet = 1;											// 在車ｶｳﾝﾄしない
	NTNET_Data152Save((void *)(&PayData.CountSet), NTNET_152_COUNTSET);
	PayData.PassCheck = 1;											// ｱﾝﾁﾊﾟｽﾁｪｯｸしない
	NTNET_Data152Save((void *)(&PayData.PassCheck), NTNET_152_ANTIPASSCHECK);
}
/*[]-----------------------------------------------------------------------[]*/
/*| 指定金種の予蓄割当先を調べる								            |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : subtube_use_check( int )                                 |*/
/*| PARAM		 :	int		mny_type（金種 0=10円, 1=50円, 2=100円）		|*/
/*|	RETURN 	     :	int		ret		（0=未使用, 1=予蓄１, 2=予蓄２）		|*/
/*[]-------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
int	subtube_use_check( int mny_type )
{
	int		i, ret = 0;
	uchar	bit, idx, tube[] = { 0, 0 };

	bit = 0x01;
	idx = 0;
	for( i=0; i<3; i++ ){
		if( CN_SUB_SET[i] ){
			tube[idx++] |= bit;
			if( idx > 1 ){
				break;
			}
		}
		bit <<= 1;
	}
	switch( mny_type ){
	case 0:		// 10円
		if( tube[0] & 0x01 ){
			ret = 1;
		}
		if( tube[1] & 0x01 ){
			ret = 2;
		}
		break;
	case 1:		// 50円
		if( tube[0] & 0x02 ){
			ret = 1;
		}
		if( tube[1] & 0x02 ){
			ret = 2;
		}
		break;
	case 2:		// 100円
		if( tube[0] & 0x04 ){
			ret = 1;
		}
		if( tube[1] & 0x04 ){
			ret = 2;
		}
		break;
	}

	return ret;
}

// MH322914 (s) kasiyama 2016/07/15 AI-V対応
//[]----------------------------------------------------------------------[]
///	@brief			紛失精算チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		f_Button	1=紛失ﾎﾞﾀﾝ<br>
///								0=紛失券
///	@return			ret			0=紛失精算無し（設定が無い）<br>
///								1=OK（料金計算ﾃﾞｰﾀまで作成済み）
///	@author			Okuda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/10/21<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar	Ope_FunCheck( char f_Button )
{
	(void)vl_funchg( f_Button, (m_gtticstp*)MAGred );			// 料金計算ﾃﾞｰﾀ設定

	return	(uchar)1;
}
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)
//[]----------------------------------------------------------------------[]
///	@brief		遠隔精算入庫時刻指定精算 データセット
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
void Ope_Set_tyudata( void )
{
	vl_tikchg();
}
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
//[]----------------------------------------------------------------------[]
///	@brief		カード情報から、料金計算用データセット
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2019/11/15<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void Ope_Set_tyudata_Card( void )
{
	vl_lcd_tikchg();
}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車解除
//[]----------------------------------------------------------------------[]
///	@param[in]	LockNo      : 内部処理用駐車位置番号(1～324) 
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)仮対応
///	@param[in]	knd: 1=精算 2=強制／不正 
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)仮対応
///	@return		none
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/05/16<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//void	LongTermParkingRel( ulong LockNo )
void	LongTermParkingRel( ulong LockNo , uchar knd, flp_com *flp)
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
{
	ulong	ulwork;
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//	if ( prm_get(COM_PRM, S_TYP, 135, 1, 5) == 0 ){	// 長期駐車検出する
//		if((LockNo == 0)||(LockNo > 324)){
//			return;// インデックス破壊防止
//		}else{
//			if(LongParkingFlag[LockNo-1] != 0){	// 長期駐車状態：長期駐車状態あり
//				ulwork = (ulong)(( LockInfo[LockNo-1].area * 10000L ) + LockInfo[LockNo-1].posi );	// 区画情報取得
//				alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);// A0031解除 長期駐車検出
//				LongParkingFlag[LockNo-1] = 0;	// 長期駐車状態なし
//			}
//		}
//	}
	uchar	prm_wk;
	ushort	wHour;
	ushort	wHour2;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用SRAM削減）
//	if((LockNo == 0)||(LockNo > 324)){
	if((LockNo == 0)||(LockNo > LOCK_MAX)){
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用SRAM削減）
		return;// インデックス破壊防止
	}
	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);		// 長期駐車検出する
	ulwork = (ulong)(( LockInfo[LockNo-1].area * 10000L ) + LockInfo[LockNo-1].posi );	// 区画情報取得

	if(prm_wk == 0){	// 長期駐車検出(アラームデータによる通知)
		if(LongParkingFlag[LockNo-1] != 0){	// 長期駐車状態：長期駐車状態あり
			alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);// A0031解除 長期駐車検出
			LongParkingFlag[LockNo-1] = 0;	// 長期駐車状態なし
		}
	}
	else if(prm_wk == 2){	// 長期駐車検出(アラームデータによる通知+長期駐車データによる通知)
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// 長期駐車検出時間1
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;	// 設定が0の場合は強制的に48時間とする
		}
		wHour2 = (ushort)prm_get(COM_PRM,  S_TYP, 136, 4, 1);		// 長期駐車検出時間2
		// 長期駐車検出時間1の解除(アラームデータによる通知)
		if(LongParkingFlag[LockNo-1] != 0){	// 長期駐車状態：長期駐車状態あり
			alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);// A0031解除 長期駐車検出
			LongParkingFlag[LockNo-1] = 0;	// 長期駐車状態なし
		}

		// 精算／出庫により長期駐車を解除するときは長期駐車データの解除の送信は長期駐車1または長期駐車2のどちらか一つで良い
		// ParkingWebは車室単位で長期駐車を判断するため
		
		if(wHour2 == 0){
			// 長期駐車検出時間1の解除(長期駐車データによる通知)
			if(flp->flp_state.BIT.b00 != 0){	// 長期駐車1状態：長期駐車状態あり
				Make_Log_LongParking_Pweb( (ushort)LockNo, wHour, LONGPARK_LOG_RESET, knd);// 長期駐車1：解除 長期駐車データログ生成
				Log_regist( LOG_LONGPARK );// 長期駐車データログ登録
				flp->flp_state.BIT.b00 = 0;	// 長期駐車1状態なし
				flp->flp_state.BIT.b01 = 0;	// 長期駐車2状態なし
			}
		}else{
			// 長期駐車検出時間2の解除(長期駐車データによる通知)
			if(flp->flp_state.BIT.b01 != 0){		// 長期駐車2状態：長期駐車状態あり
				Make_Log_LongParking_Pweb( (ushort)LockNo, wHour2, LONGPARK_LOG_RESET, knd);// 長期駐車2：解除 長期駐車データログ生成
				Log_regist( LOG_LONGPARK );// 長期駐車データログ登録
				flp->flp_state.BIT.b00 = 0;	// 長期駐車1状態なし
				flp->flp_state.BIT.b01 = 0;	// 長期駐車2状態なし
			}else if(flp->flp_state.BIT.b00 != 0){	// 長期駐車1状態：長期駐車状態あり
				Make_Log_LongParking_Pweb( (ushort)LockNo, wHour, LONGPARK_LOG_RESET, knd);// 長期駐車1：解除 長期駐車データログ生成
				Log_regist( LOG_LONGPARK );// 長期駐車データログ登録
				flp->flp_state.BIT.b00 = 0;	// 長期駐車1状態なし
				flp->flp_state.BIT.b01 = 0;	// 長期駐車2状態なし
			}
		}
	}
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
}
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応

// MH810100(S) 2020/09/02 #4803 [08-0001：全サービス券使用限度枚数]で設定されている枚数を超えた際に表示されるPOPUPが、表示されない
//// MH810100(S) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
//BOOL IsBackupAreaFound(int nCheckIndex )
//{
//	BOOL bRet = FALSE;
//	int i = 0;
//	
//	for( i=0; i<ONL_MAX_DISC_NUM; i++ ){
//		if( memcmp(&DiscountBackUpArea[i],&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[nCheckIndex],sizeof(stDiscount2_t)) == 0){
//			bRet = TRUE;
//			break;
//		}
//	}
//	return bRet;
//	
////	ulong	DiscParkNo;				// 駐車場№(0～999999)
////	ushort 	DiscSyu;				// 種別(0～9999)
////	ushort 	DiscCardNo;				// ｶｰﾄﾞ区分(0～65000)
////	uchar 	DiscNo;					// 区分(0～9)
////	uchar 	DiscCount;				// 枚数(0～99)
////	ulong 	Discount;				// 金額/時間(0～999999)
////	ulong	UsedDisc;				// 買物割引利用時の使用済み割引（金額/時間）(0～999999)
////	ushort 	DiscInfo;				// 割引情報(0～65000)
////	ushort 	DiscCorrType;			// 対応ｶｰﾄﾞ種別(0～65000)
////	uchar 	DiscStatus;				// ｽﾃｰﾀｽ(0～9)
////	uchar 	DiscFlg;				// 割引状況(0～9)
//
//}
//// MH810100(E) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
// MH810100(E) 2020/09/02 #4803 [08-0001：全サービス券使用限度枚数]で設定されている枚数を超えた際に表示されるPOPUPが、表示されない

// MH810102(S) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
//[]----------------------------------------------------------------------[]
/// @brief		限度枚数オーバーを確認するか
//[]----------------------------------------------------------------------[]
///	@param[in]	status		: 割引きステータス
/// @return 	0: 確認しない<br>
/// 			1: 確認する
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/08
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static short check_for_limit_over(uchar status)
{
	switch ( status ) {
	case 0:	// 未割引
	case 1:	// 今回割引
		return 1;
	case 2:	// 割引済(限度枚数オーバーの判定は対象外)
	default:
		return 0;
	}
}
// MH810102(E) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
//[]----------------------------------------------------------------------[]
///	@brief			オンライン割引処理
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			1：精算処理区分	<br>
///					2：割引き種別	<br>
///					3：枚数			<br>
///					4：カード区分	<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short DiscountForOnline( void )
{
	short	wk = 0;
	short	i, iCount, mai;
	short	cardknd;
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	short	kind = 0;
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(S) K.Onodera  2020/04/08 #4128 車番チケットレス(10種以上の読取に店割引が適用されない)
	short	code = 0;
// MH810100(E) K.Onodera  2020/04/08 #4128 車番チケットレス(10種以上の読取に店割引が適用されない)
	short	data_adr;		// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ
	char	data_pos;		// 使用可能料金種別のﾃﾞｰﾀ位置
	stDiscount2_t* pDisc = NULL;
	const char USE_CNT_prm_pos[4] = { 0, 5, 3, 1 };

	pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage];

	// m_stDiscへ割引データコピー
	m_stDisc.DiscParkNo		= pDisc->DiscParkNo;		// 割引 駐車場№(0～999999)
	m_stDisc.DiscSyu		= pDisc->DiscSyu;			// 割引 種別(0～9999)
	m_stDisc.DiscCardNo		= pDisc->DiscCardNo;		// 割引 ｶｰﾄﾞ区分(0～65000)
	m_stDisc.DiscNo			= pDisc->DiscNo;			// 割引 区分(0～9)
	m_stDisc.DiscCount		= pDisc->DiscCount;			// 割引 枚数(0～99)
	m_stDisc.DiscInfo		= pDisc->DiscInfo;			// 割引 割引情報(0～65000)
	m_stDisc.DiscCorrType	= pDisc->DiscCorrType;		// 割引 対応ｶｰﾄﾞ種別(0～65000)
	m_stDisc.DiscStatus		= pDisc->DiscStatus;		// 割引 ｽﾃｰﾀｽ(0～9)
	m_stDisc.DiscFlg		= pDisc->DiscFlg;			// 割引 割引状況(0～9)
	m_stDisc.Discount		= 0;						// 割引額(割引時間) = 料金計算時に積算されるのでクリア

// MH810100(S) 2020/09/02 再精算時の複数枚対応
	g_ulZumiWari = pDisc->UsedDisc;								// 使用済み割引額を分割するよう
// MH810100(E) 2020/09/02 再精算時の複数枚対応
// MH810100(S) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
	g_ulZumiWariOrg = g_ulZumiWari;								// 使用済み割引額を分割するよう
// MH810100(E) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）

	// 精算処理区分が「精算」「再精算」以外
	if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType != 1) &&
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType != 2) ){
		// 割引ステータスが割引済み or 今回割引
		if( m_stDisc.DiscStatus == 2 || m_stDisc.DiscStatus == 1 ){
			return 1;
		}
	}
	// 非対象
	if( m_stDisc.DiscStatus == 6 ){
// MH810100(S) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
//		return 1;
		return 2;
// MH810100(E) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
	}

	// 割引種別 == 種別割引(時間=初期無料=150)
	if (m_stDisc.DiscSyu == NTNET_SYUBET_TIME) {
		// 標準通り､et40/et47で計算し､そこでｵﾝﾗｲﾝﾃﾞｰﾀにもｾｯﾄするのでここでは何もしない
		return 2;
	}

	/************************************/
	/* 割引種別による処理				*/
 	/*		■S_STO		店割引			*/
	/*		■S_SER		ｻｰﾋﾞｽ券設定		*/
	/************************************/
	switch (m_stDisc.DiscSyu) {
		case NTNET_SVS_M:			// ｻｰﾋﾞｽ券(金額=1)
		case NTNET_SVS_T:			// ｻｰﾋﾞｽ券(時間=101)
			cardknd = 11;
			break;
		case NTNET_KAK_M:			// 店割引(金額=2)
		case NTNET_KAK_T:			// 店割引(時間=102)
		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額=6)
		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間=106)
			cardknd = 12;
			break;
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
		case NTNET_TKAK_M:			// 多店舗割引（金額）
		case NTNET_TKAK_T:			// 多店舗割引（時間）
			if ( prm_get(COM_PRM, S_TAT, 1, 1, 1) == 1 ) {	// 多店舗割引あり
				cardknd = 12;
			} else {										// 多店舗割引なし
				return 2;
			}
			break;
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
		default:
			return 2;
	}

	// 08-0001	全ｻｰﾋﾞｽ券使用限度枚数	⑤⑥:全ｻｰﾋﾞｽ券使用限度枚数 00=未使用/1～99(枚)
	mai = (short)prm_get(COM_PRM, S_DIS, 1, 2, 1);

// MH810102(S) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
	if ( check_for_limit_over(m_stDisc.DiscStatus) ) {
// MH810102(E) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
		// 使用ｻｰﾋﾞｽ券枚数 >= 全ｻｰﾋﾞｽ券使用限度枚数
		if (card_use[USE_SVC] >= mai) {
			// 限度枚数超過
			return 3;
		}
// MH810102(S) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
	}
// MH810102(E) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する

	//11=ｻｰﾋﾞｽ券, 12=掛売券, 13=回数券，14=割引券

// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
	// チェック
	if ( CheckDiscount(m_stDisc.DiscSyu, m_stDisc.DiscCardNo, m_stDisc.DiscInfo) ) {
		return 4;
	}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)

	// サービス券？
	if( cardknd == 11 ){
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
// 		// サービス券種範囲チェック
// 		if( !rangechk( 1, SVS_MAX, m_stDisc.DiscCardNo ) ){
// 			return 4;
// 		}
// 
// 		// 切換先の車種設定なし、入金中、割引済みは車種切換を受け付けない
// 		if( CPrmSS[S_SER][3+3*(m_stDisc.DiscCardNo-1)] ){			// 種別切替有り?
// 			if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*(m_stDisc.DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
// 				return 4;										// 種別切替なし
// 			}
// // MH810100(S) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
// //// MH810100(S) 2020/08/06 #4579【初回精算(現金投入あり)で使用した種別切替が、再精算時に適用されない
// ////			if(( ryo_buf.nyukin )||( ryo_buf.waribik )){		// 入金済み? or 割引済み?
// //			if( ryo_buf.nyukin ){		// 入金済み? (割引はほかでチェック済みの為ここではチェックしない)
// //// MH810100(E) 2020/08/06 #4579【初回精算(現金投入あり)で使用した種別切替が、再精算時に適用されない
// //				return 4;										// 精算順序ｴﾗｰ
// //			}
// // MH810100(E) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
// 		}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
		if( 1 == (short)prm_get( COM_PRM, S_DIS, 1, 1, 6 ) ){				// 種別使用限度枚数設定？
			if( m_stDisc.DiscCardNo <= 3 ){
				data_adr = 106;												// 使用限度枚数別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)USE_CNT_prm_pos[m_stDisc.DiscCardNo];		// 使用限度枚数のﾃﾞｰﾀ位置取得
			}else if( m_stDisc.DiscCardNo <= 6 ){
				data_adr = 107;												// 使用限度枚数のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)USE_CNT_prm_pos[(m_stDisc.DiscCardNo-3)];	// 使用限度枚数のﾃﾞｰﾀ位置取得
			}else if( m_stDisc.DiscCardNo <= 9 ){
				data_adr = 108;												// 使用限度枚数のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)USE_CNT_prm_pos[(m_stDisc.DiscCardNo-6)];	// 使用限度枚数のﾃﾞｰﾀ位置取得
			}else if( m_stDisc.DiscCardNo <= 12 ){
				data_adr = 109;												// 使用限度枚数のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)USE_CNT_prm_pos[(m_stDisc.DiscCardNo-9)];	// 使用限度枚数のﾃﾞｰﾀ位置取得
			}else if( m_stDisc.DiscCardNo <= 15 ){
				data_adr = 110;												// 使用限度枚数のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)USE_CNT_prm_pos[(m_stDisc.DiscCardNo-12)];	// 使用限度枚数のﾃﾞｰﾀ位置取得
			}
			// 使用限度枚数設定オーバー
			wk = prm_get( COM_PRM, S_SER, data_adr, 2, data_pos );
// MH810102(S) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
			if ( check_for_limit_over(m_stDisc.DiscStatus) ) {
// MH810102(E) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
				if( card_use2[m_stDisc.DiscCardNo-1] >= wk ){
					return 3;
				}
// MH810102(S) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
			}
// MH810102(E) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
		}
	}
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
// 	// 店割引？
// 	else{
// 		// 店№範囲チェック
// 		if( !rangechk( 1, MISE_NO_CNT, m_stDisc.DiscCardNo ) ){
// 			return 4;
// 		}
// 
// 		// 切換先の車種設定なし、入金中、割引済みは車種切換を受け付けない
// 		if( CPrmSS[S_STO][3+3*(m_stDisc.DiscCardNo-1)] ){			// 種別切替有り?
// 			if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*(m_stDisc.DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
// 				return 4;										// 種別切替なし
// 			}
// // MH810100(S) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
// //// MH810100(S) 2020/08/06 #4579【初回精算(現金投入あり)で使用した種別切替が、再精算時に適用されない
// ////			if(( ryo_buf.nyukin )||( ryo_buf.waribik )){		// 入金済み? or 割引済み?
// //			if( ryo_buf.nyukin ){		// 入金済み? (割引はほかでチェック済みの為ここではチェックしない)
// //// MH810100(E) 2020/08/06 #4579【初回精算(現金投入あり)で使用した種別切替が、再精算時に適用されない
// //				return 4;										// 精算順序ｴﾗｰ
// //			}
// // MH810100(E) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
// 		}
// 	}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)

	if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
		// 1精算の割引種類の件数オーバー？
// MH810100(S) K.Onodera  2020/04/08 #4128 車番チケットレス(10種以上の読取に店割引が適用されない)
//		if( CardSyuCntChk( m_stDisc.DiscNo, cardknd, m_stDisc.DiscCardNo, (short)m_stDisc.DiscInfo, 0 ) ){
		// サービス券
		if( cardknd == 11 ){
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			kind = (short)m_stDisc.DiscCardNo;	// 券種セット
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			code = (short)m_stDisc.DiscInfo;	// 掛売り先コードセット
		}
		// 掛売り
		else{
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			kind = (short)m_stDisc.DiscInfo;	// 割引種類/割引種別をセット
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			code = (short)m_stDisc.DiscCardNo;	// 店№をセット
		}
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 		if( CardSyuCntChk( m_stDisc.DiscNo, cardknd, m_stDisc.DiscCardNo, code, 0 ) ){
		if( CardSyuCntChk( m_stDisc.DiscNo, cardknd, kind, code, 0 ) ){
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) K.Onodera  2020/04/08 #4128 車番チケットレス(10種以上の読取に店割引が適用されない)
			return 3;		// 限度枚数オーバー
		}
	}

	// 枚数 <- 割引ﾃﾞｰﾀ_枚数
	iCount = m_stDisc.DiscCount;

// MH810102(S) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
	if ( check_for_limit_over(m_stDisc.DiscStatus) ) {
// MH810102(E) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
		// サービス券使用枚数が限度を越えている
		if ((iCount + card_use[USE_SVC]) >= mai) {
			// 枚数 <- (使用限度枚数 - 使用ｻｰﾋﾞｽ券枚数)
			iCount = mai - card_use[USE_SVC];
		}

		// サービス券？(サービス券の場合、種別毎の枚数もチェック)
		if(( cardknd == 11 ) && prm_get( COM_PRM, S_DIS, 1, 1, 6 )){
			// サービス券種毎の使用限度枚数を超えてる？
			if( (iCount + card_use2[m_stDisc.DiscCardNo-1]) >= wk ){
				// 枚数 <- (使用限度枚数 - 使用ｻｰﾋﾞｽ券枚数)
				iCount = wk - card_use2[m_stDisc.DiscCardNo-1];
			}
		}
// MH810102(S) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する
	}
// MH810102(E) R.Endo 2021/04/08 車番チケットレス フェーズ2.5 #5477 【イオン和泉府中指摘事項】割引済サービス券は限度枚数オーバーでも適用する

	// サービス券データクリア
	memset( &vl_svs, 0x00, sizeof( struct VL_SVS ) );

// MH810103(S) R.Endo 2021/06/08 車番チケットレス フェーズ2.2 #5699 拡張駐車場№のQR割引券(店割引)が基本駐車場扱いになる
// 	// 割引区分 == 1, 2, 3
// 	if (1 <= m_stDisc.DiscNo && m_stDisc.DiscNo <= 3){
// 		// ｻｰﾋﾞｽ券ﾃﾞｰﾀ_拡張1～3ｾｯﾄ(APSなので拡張2～3を受信することはない)
// 		vl_svs.pkno = m_stDisc.DiscNo;
// 	}
// 	// 割引区分 == 0
// 		else{
// 		// ｻｰﾋﾞｽ券ﾃﾞｰﾀ_範囲外(障害割引含む)は基本ｾｯﾄ
// 		vl_svs.pkno = 0;
// 	}
	// 駐車場№チェック
	if( 0L == m_stDisc.DiscParkNo ){
		return 4;	// 駐車場№エラー
	}

	// 駐車場№種別
	if( (prm_get(COM_PRM, S_SYS, 71, 1, 6) == 1) &&				// 基本サービス券使用可
		(CPrmSS[S_SYS][1] == m_stDisc.DiscParkNo) ) {			// 基本駐車場№と一致
		vl_svs.pkno = KIHON_PKNO;	// 基本
	} else if( (prm_get(COM_PRM, S_SYS, 71, 1, 1) == 1) &&		// 拡張1サービス券使用可
		       (CPrmSS[S_SYS][2] == m_stDisc.DiscParkNo) ) {	// 拡張1駐車場№と一致
		vl_svs.pkno = KAKUCHOU_1;	// 拡張1
	} else if( (prm_get(COM_PRM, S_SYS, 71, 1, 2) == 1) &&		// 拡張2サービス券使用可
		       (CPrmSS[S_SYS][3] == m_stDisc.DiscParkNo) ) {	// 拡張2駐車場№と一致
		vl_svs.pkno = KAKUCHOU_2;	// 拡張2
	} else if( (prm_get(COM_PRM, S_SYS, 71, 1, 3) == 1) &&		// 拡張3サービス券使用可
		       (CPrmSS[S_SYS][4] == m_stDisc.DiscParkNo) ) {	// 拡張3駐車場№と一致
		vl_svs.pkno = KAKUCHOU_3;	// 拡張3
	} else{
		return 4;	// 駐車場№エラー
	}
// MH810103(E) R.Endo 2021/06/08 車番チケットレス フェーズ2.2 #5699 拡張駐車場№のQR割引券(店割引)が基本駐車場扱いになる

	// サービス券？
	if( m_stDisc.DiscSyu == NTNET_SVS_M ||
		m_stDisc.DiscSyu == NTNET_SVS_T ){

		// ｻｰﾋﾞｽ券ﾃﾞｰﾀ_ｻｰﾋﾞｽ券種 <- 割引ｶｰﾄﾞ区分
		vl_svs.sno  = (char)m_stDisc.DiscCardNo;

		// ｻｰﾋﾞｽ券ﾃﾞｰﾀ_店NO.( 0～100) <- 割引情報
		vl_svs.mno  = (short)m_stDisc.DiscInfo;

		if( m_stDisc.DiscStatus == 2 ){	// 割引済み
			vl_svs.mno |= 0x8000;
		}
	}
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
	// 多店舗割引？
	else if ( (m_stDisc.DiscSyu == NTNET_TKAK_M) ||
		      (m_stDisc.DiscSyu == NTNET_TKAK_T) ) {
		// サービス券データ サービス券種 <- 割引情報(割引種別)
		// 多店舗割引では割引種別(1～15)を格納する。
		// サービス券種は26、多店舗割引の割引種別は15、店割引の割引種類は101が最大の為、
		// 29まではサービス券、30～49は多店舗割引、50以降は店割引として区別する。
		vl_svs.sno = 0;
		if ( (m_stDisc.DiscInfo > 0) && (m_stDisc.DiscInfo < 225) ) {
			vl_svs.sno = (char)(30 + m_stDisc.DiscInfo);
		}

		// サービス券データ 店№ <- カード区分(店№)
		vl_svs.mno = (short)m_stDisc.DiscCardNo;

		if ( m_stDisc.DiscStatus == 2 ) {	// 割引済み
			vl_svs.mno |= 0x8000;
		}
	}
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
	// 店割引？
	else{

		// ｻｰﾋﾞｽ券ﾃﾞｰﾀ_ｻｰﾋﾞｽ券種 <- 0
		vl_svs.sno  = 0;
// MH810100(S) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
		// vl_svs.snoは、店割引だと0で、サービス券の場合、カード区分は、1～26になる。
		// 割引種類は、1～101なので、＋50して、vl_svs.snoに格納する。
		if(m_stDisc.DiscInfo > 0 && m_stDisc.DiscInfo < 205){
			vl_svs.sno = 50 + m_stDisc.DiscInfo;
		}
// MH810100(E) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）

		// ｻｰﾋﾞｽ券ﾃﾞｰﾀ_店NO.( 0～100) <- 割引ｶｰﾄﾞ区分
		vl_svs.mno  = (short)m_stDisc.DiscCardNo;
		if( m_stDisc.DiscStatus == 2 ){	// 割引済み
			vl_svs.mno |= 0x8000;
		}
	}

	// 掛売り券が枚数に未対応なので枚数分ﾙｰﾌﾟする
	for (i = 0; i < iCount; i++) {

		// 使用ｻｰﾋﾞｽ券, 掛売券, 割引券枚数 + 1
		card_use[USE_SVC] += 1;
		if( m_stDisc.DiscStatus < 2 ){
			card_use[USE_N_SVC] += 1;
// MH810100(S) K.Onodera  2020/04/09 #4130 車番チケットレス(サービス券券種毎限度枚数が適用されない)
// MH810102(S) R.Endo 2021/04/05 車番チケットレス フェーズ2.5 #5472 【イオン和泉府中指摘事項】再精算時の券種毎限度枚数の判定が期待通りでない
//			// サービス券？
//			if( cardknd == 11 ){
//				card_use2[m_stDisc.DiscCardNo-1] += 1;
//			}
// MH810102(E) R.Endo 2021/04/05 車番チケットレス フェーズ2.5 #5472 【イオン和泉府中指摘事項】再精算時の券種毎限度枚数の判定が期待通りでない
// MH810100(E) K.Onodera  2020/04/09 #4130 車番チケットレス(サービス券券種毎限度枚数が適用されない)
		}
// MH810102(S) R.Endo 2021/04/05 車番チケットレス フェーズ2.5 #5472 【イオン和泉府中指摘事項】再精算時の券種毎限度枚数の判定が期待通りでない
		// サービス券？
		if ( cardknd == 11 ) {
			switch ( m_stDisc.DiscStatus ) {
			case 0:	// 未割引
			case 1:	// 今回割引
			case 2:	// 割引済(以前の精算で使用されたサービス券もカウントする)
				card_use2[m_stDisc.DiscCardNo-1] += 1;
				break;
			default:
				break;
			}
		}
// MH810102(E) R.Endo 2021/04/05 車番チケットレス フェーズ2.5 #5472 【イオン和泉府中指摘事項】再精算時の券種毎限度枚数の判定が期待通りでない

		// 現在VLｶｰﾄﾞﾃﾞｰﾀ <- ｻｰﾋﾞｽ券
		vl_now = V_SAK;

		// -------------------------- //
		// サービス券として呼び出し
		// -------------------------- //
		ryo_cal( 3, OPECTL.Pr_LokNo );		// Pr_LokNo = 内部処理用駐車位置番号(1～324))

		if( OPECTL.op_faz == 0 ){
			OPECTL.op_faz = 1;			// 入金中
		}

		ac_flg.cycl_fg = 10;			// 入金

		in_mony( OPE_REQ_LCD_CALC_IN_TIME, 0 );
	}

	// 割引ﾃﾞｰﾀ_枚数 <- 枚数
	m_stDisc.DiscCount = (uchar)iCount;

	// 未割引？
	if( m_stDisc.DiscStatus == 0 ){
		m_stDisc.DiscStatus = 1;	// 今回割引へ(精算完了で割引済とする)
	}

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			オンライン割引処理
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			1：精算処理区分	<br>
///					2：割引き種別	<br>
///					3：枚数			<br>
///					4：カード区分	<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short	OnlineDiscount( void )
{
	int i = 0, j = 0;
// MH810100(S) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
//	int check_index = -1;
// MH810100(E) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
	short ret = 0;
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
	BOOL bFound = FALSE;
	unsigned long	waribikTmp = 0;
	unsigned long	waribikTmp2 = 0;
	unsigned long	zenkaiwariTmp = 0;
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	unsigned short wk_dat;
	unsigned short wk_dat2;
	DISCOUNT_DATA	disTemp;
	stDiscount_t	dis2Temp;
	int iTemp = 0;
//	BOOL	bRepay = FALSE;		// 再精算
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）

	// 有効性ﾁｪｯｸ 割引重複利用ﾁｪｯｸ	0=ﾁｪｯｸなし､1=割引重複なし､2=割引重複あり､3=ﾃﾞｰﾀ不正､99=その他異常
	// 0=ﾁｪｯｸなし､1=割引重複なしの時のみ割引処理を行う
	if( (lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup != 0) &&
		(lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup != 1) ){
// MH810100(S) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
//		return (1);
		return (101);
// MH810100(E) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
	}
// MH810100(S) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
	if(IsDupSyubetuwari() == TRUE){
		// 順番エラー
// MH810100(S) 2020/08/31 #4771 【検証課指摘事項】料金投入後の種別切替で表示されるエラーメッセージが不適切(No60)
//		lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(4 - 1) );	// ポップアップ表示要求
		lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(5 - 1) );	// ポップアップ表示要求
// MH810100(E) 2020/08/31 #4771 【検証課指摘事項】料金投入後の種別切替で表示されるエラーメッセージが不適切(No60)
		return 4;
	}
// MH810100(E) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
// MH810100(S) 2020/09/04 #4584 【検証課指摘事項】　 同一割引券種数の制限が「１」の場合（No33 再評価NG）(割引上限対応)
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	if(IsWaribikiLimitOver() == TRUE){
// 		// 順番エラー
// 		lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(6 - 1) );	// ポップアップ表示要求
// 		return 4;
// 	}
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) 2020/09/04 #4584 【検証課指摘事項】　 同一割引券種数の制限が「１」の場合（No33 再評価NG）(割引上限対応)

// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	wk_dat = (ushort)(1+6*(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType-1));
	wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = 種別時間割引値		*/
	if( wk_dat2){
		// 種別時間割引があったら
		memset( &dis2Temp, 0, sizeof(stDiscount_t) );
		// クリア前に保持しておく
		memcpy(&disTemp,&PayData.DiscountData[0],sizeof( DISCOUNT_DATA ));
		// RT_Pay用も保持
		dis2Temp.DiscParkNo = disTemp.ParkingNo;
		dis2Temp.DiscSyu = disTemp.DiscSyu;
		dis2Temp.DiscCardNo = disTemp.DiscNo;
		dis2Temp.DiscCount = disTemp.DiscCount;
		dis2Temp.Discount = disTemp.Discount;
		dis2Temp.DiscStatus = 1;		// 今回
	}
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）

// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#if 1	// ここから：クラウド料金計算のdiscount_init(cal_cloud/cal_cloud.c)に相当。
		// ※修正する際はdiscount_init側も確認すること。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
	// RT_Pay、PayDataの割引情報を一旦クリア
	memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );	// 精算データ割引情報作成エリアクリア
	memset( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0, sizeof(stDiscount_t)*ONL_MAX_DISC_NUM );
	NTNET_Data152_DiscDataClear();
	NTNET_Data152_DetailDataClear();

	// 駐車券、定期以外の使用枚数をクリア
	for( i=0; i<USE_MAX; i++ ){
		if( i != USE_TIK && i != USE_PAS ){
			card_use[i] = 0;
		}
	}
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	i = 0;	// 利用QRバックアップインデックスとして利用するためにクリア
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	memset( card_use2, 0, sizeof( card_use2 ) );
	CardUseSyu = 0;
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#endif	// ここまで：クラウド料金計算のdiscount_init(cal_cloud/cal_cloud.c)に相当。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応

	// 入庫時刻を戻す
	memcpy( &car_in, &car_in_f, sizeof( struct CAR_TIM ) );
	// 戻した後に、種別時間を加算する
	IncCatintime(&car_in);	

	// ryo_calで使用する割引情報をクリア
	ClearRyoCalDiscInfo();

	// 種別切り替えのチェック及び並び替え
	bFound = DiscountSortCheck();
	m_nDiscPage = 0;
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	if( wk_dat2){
		// 種別時間割引があったら先頭に入れておく（駐車券処理は実施済みの為）calで再度セットされるが、割引なしで現金の場合載らなくなるので、ここでセット。
		if(disTemp.ParkingNo != 0){
			memcpy( &PayData.DiscountData[0], &disTemp, sizeof( DISCOUNT_DATA ) );
		}
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j++], &dis2Temp, sizeof( stDiscount_t ) );

	}
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	if( bFound){
		// 種別切替前を覚えておく
		waribikTmp2 = ryo_buf.waribik;
		if(waribikTmp2 >= ryo_buf.zenkai){
			// waribikにzenkaiが足されてたら引いておく
			waribikTmp2 = waribikTmp2 - ryo_buf.zenkai;
		}
		// 前回割もここでおぼえておく
		zenkaiwariTmp = ryo_buf.zenkaiWari;
		// 種別切替がみつかったので、ここでまず実施しておく

		// -------------------- //
		// 割引実処理
		// -------------------- //
		ret = DiscountForOnline();
		// 結果NG？
		if( ret ){
			lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(ret - 1) );	// ポップアップ表示要求
			return 4;
		}
// MH810100(S) 2020/09/04  種別時間割引
//		// QR利用時の割引データ比較用にバックアップ
//		memcpy( &DiscountBackUpArea[0], &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage], sizeof(stDiscount2_t) );
//		// リアルタイム精算データ用データへ精算結果を書き込む
//		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[0], &m_stDisc, sizeof(m_stDisc) );
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// 		// QR利用時の割引データ比較用にバックアップ
// 		memcpy( &DiscountBackUpArea[i++], &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage], sizeof(stDiscount2_t) );
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
		// リアルタイム精算データ用データへ精算結果を書き込む
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j++], &m_stDisc, sizeof(m_stDisc) );
		m_nDiscPage++;		
// MH810100(E) 2020/09/04  種別時間割引

		// 種別切替に割引がついていたら、tmpにいれる
		waribikTmp = ryo_buf.waribik;
		ryo_buf.waribik = waribikTmp2;
		// 戻す
		ryo_buf.zenkaiWari = zenkaiwariTmp;
// MH810100(S) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
		g_checkKirikae = 1;								// 種別切替チェック済みフラグ
// MH810100(E) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
// MH810100(S) 2020/09/08 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
		if( wk_dat2){
			wk_dat = (ushort)(1+6*(ryo_buf.syubet));
			wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = 種別時間割引値		*/
			// 種別割引ありから種別割引あり
			if( wk_dat2){
				// 種別切替で入れ替わっているかもしれないので、上書きする
				for(iTemp = 0 ; iTemp < ONL_DISC_MAX; iTemp++ ){
					if(RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscSyu == NTNET_SYUBET|| 
					RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscSyu == NTNET_SYUBET_TIME){
						memcpy(&disTemp,&PayData.DiscountData[0],sizeof( DISCOUNT_DATA ));
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscParkNo = disTemp.ParkingNo;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscSyu = disTemp.DiscSyu;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscCardNo = disTemp.DiscNo;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscCount = disTemp.DiscCount;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].Discount = disTemp.Discount;
// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscTime = 0;
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscNo = 0;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscInfo = 0;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscCorrType = 0;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscStatus = 1;		// 今回
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscFlg = 0;
						break;
					}
				}
			}else{
				// 種別割引ありから種別割引なしへ
				// １つまえにずらす
				memmove(&RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[0],&RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[1],sizeof(stDiscount_t)*(ONL_MAX_DISC_NUM-1 ));
				// 一番後ろをクリア
				memset(&RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[ONL_MAX_DISC_NUM-1],0,sizeof(stDiscount_t));
				j--;

			}
		}else{
			wk_dat = (ushort)(1+6*(ryo_buf.syubet));
			wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = 種別時間割引値		*/
			// 種別割引なしから種別割引あり
			if( wk_dat2){
				memset( &dis2Temp, 0, sizeof(stDiscount_t) );
				memcpy(&disTemp,&PayData.DiscountData[0],sizeof( DISCOUNT_DATA ));
				// RT_Pay用も保持
				dis2Temp.DiscParkNo = disTemp.ParkingNo;
				dis2Temp.DiscSyu = disTemp.DiscSyu;
				dis2Temp.DiscCardNo = disTemp.DiscNo;
				dis2Temp.DiscCount = disTemp.DiscCount;
				dis2Temp.Discount = disTemp.Discount;
// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
				dis2Temp.DiscTime = 0;
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
				dis2Temp.DiscStatus = 1;		// 今回
				// 後ろに１つずらす
				memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[1], &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[0], sizeof( stDiscount_t ) );
				// ここに入れる
				memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[0], &dis2Temp, sizeof( stDiscount_t ) );
				j++;
			}
		}
// MH810100(E) 2020/09/08 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	}
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))

	// 割引は入庫情報受信毎に毎回最初からやり直す
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
//// MH810100(S) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
////	ryo_buf.zankin += ryo_buf.waribik;
////	ryo_buf.dsp_ryo += ryo_buf.waribik;
//	ryo_buf.zankin += ryo_buf.waribik - ryo_buf.zenkai;
//	ryo_buf.dsp_ryo += ryo_buf.waribik - ryo_buf.zenkai;
//// MH810100(E) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
//// MH810100(S) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
//	ryo_buf.zankin = ryo_buf.zankin - ryo_buf.zenkaiWari;
//	ryo_buf.dsp_ryo = ryo_buf.dsp_ryo - ryo_buf.zenkaiWari;
//// MH810100(E) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
// MH810101(S) R.Endo 2021/01/29 【連動評価指摘事項】有効期限切れ定期更新後の再精算で残額が不正値となる(No.02-0001)
//	ryo_buf.zankin = ryo_buf.zankin + ryo_buf.waribik - ryo_buf.zenkai - ryo_buf.zenkaiWari;
//	ryo_buf.dsp_ryo = ryo_buf.dsp_ryo + ryo_buf.waribik - ryo_buf.zenkai - ryo_buf.zenkaiWari;
	if ( (ryo_buf.zankin + ryo_buf.waribik) > (ryo_buf.zenkai + ryo_buf.zenkaiWari) ) {
		ryo_buf.zankin = ryo_buf.zankin + ryo_buf.waribik - ryo_buf.zenkai - ryo_buf.zenkaiWari;
	} else {
		ryo_buf.zankin = 0;
	}
	if ( (ryo_buf.dsp_ryo + ryo_buf.waribik) > (ryo_buf.zenkai + ryo_buf.zenkaiWari) ) {
		ryo_buf.dsp_ryo = ryo_buf.dsp_ryo + ryo_buf.waribik - ryo_buf.zenkai - ryo_buf.zenkaiWari;
	} else {
		ryo_buf.dsp_ryo = 0;
	}
// MH810101(E) R.Endo 2021/01/29 【連動評価指摘事項】有効期限切れ定期更新後の再精算で残額が不正値となる(No.02-0001)
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))

	// 割引クリア
	ryo_buf.dis_tim = ryo_buf.dis_fee = ryo_buf.dis_per = 0L;
	ryo_buf.waribik = ryo_buf.fee_amount = ryo_buf.tim_amount = 0L;
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
//// MH810100(S) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
//	ryo_buf.waribik = ryo_buf.zenkai;
//// MH810100(E) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
//// MH810100(S) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
//	ryo_buf.waribik = ryo_buf.waribik + ryo_buf.zenkaiWari;
//// MH810100(E) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
	ryo_buf.waribik = ryo_buf.zenkai + ryo_buf.zenkaiWari + waribikTmp;
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))

	ryo_buf_n.require = 0L;
	ryo_buf_n.dis     = 0L;

// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
//	// RT_Pay、PayDataの割引情報を一旦クリア
//	memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );	// 精算データ割引情報作成エリアクリア
//// MH810100(S) S.Nishimoto 2020/04/07 静的解析(20200407:151)対応
////	memset( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0, sizeof(stDiscount2_t)*ONL_MAX_DISC_NUM );
//	memset( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0, sizeof(stDiscount_t)*ONL_MAX_DISC_NUM );
//// MH810100(E) S.Nishimoto 2020/04/07 静的解析(20200407:151)対応
//	NTNET_Data152_DiscDataClear();
//	NTNET_Data152_DetailDataClear();
//// MH810100(S) 2020/07/29 #4563【連動評価指摘事項】種別割引の順番対応
//	// 種別切り替えのチェック及び並び替え
//	DiscountSortCheck();
//// MH810100(E) 2020/07/29 #4563【連動評価指摘事項】種別割引の順番対応
//
//	// 駐車券、定期以外の使用枚数をクリア
//	for( i=0; i<USE_MAX; i++ ){
//		if( i != USE_TIK && i != USE_PAS ){
//			card_use[i] = 0;
//		}
//	}
//	memset( card_use2, 0, sizeof( card_use2 ) );
//	CardUseSyu = 0;
//
//	// 入庫時刻を戻す
//	memcpy( &car_in, &car_in_f, sizeof( struct CAR_TIM ) );
// MH810100(S) 2020/06/10 #4216【連動評価指摘事項】車種設定種別時間割引設定時にQR割引(時間割引)を行っても適用されない
//	// 戻した後に、種別時間を加算する
//	IncCatintime(&car_in);	
// MH810100(E) 2020/06/10 #4216【連動評価指摘事項】車種設定種別時間割引設定時にQR割引(時間割引)を行っても適用されない
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))

// MH810100(S) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
//	for( i=0; i<ONL_MAX_DISC_NUM; i++ ){
//		if( !DiscountBackUpArea[i].DiscParkNo && lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].DiscParkNo ){
//			// インデックスを i とする
//			check_index = i;
//			break;
//		}
//	}
// MH810100(E) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)


// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
//	// ryo_calで使用する割引情報をクリア
//	ClearRyoCalDiscInfo();
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))

// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
//// MH810100(S) K.Onodera  2020/04/10 #4127 車番チケットレス(再度QRコードを翳した場合に表示されない)
//	i = 0;	// 利用QRバックアップインデックスとして利用するためにクリア
// MH810100(E) K.Onodera  2020/04/10 #4127 車番チケットレス(再度QRコードを翳した場合に表示されない)
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	// 最大割引データ数分ループ
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
//	for( m_nDiscPage=0; m_nDiscPage < ONL_DISC_MAX; m_nDiscPage++ ){
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
//	if( bFound){
//		i = 1;
//		j = 1;
//		m_nDiscPage = 1;
//	}
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
	for( ; m_nDiscPage < ONL_DISC_MAX; m_nDiscPage++ ){
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
		if(( lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage].DiscParkNo == 0L ) &&
			( lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage].DiscSyu == 0 )){
			break;
		}
		taskchg(IDLETSKNO);
// MH810100(S) 2020/06/11 指定しない種別がきたら処理しない
		switch (lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage].DiscSyu) {
			case NTNET_SVS_M:			// ｻｰﾋﾞｽ券(金額=1)
			case NTNET_SVS_T:			// ｻｰﾋﾞｽ券(時間=101)
			case NTNET_KAK_M:			// 店割引(金額=2)
			case NTNET_KAK_T:			// 店割引(時間=102)
			case NTNET_SHOP_DISC_AMT:	// 買物割引(金額=6)
			case NTNET_SHOP_DISC_TIME:	// 買物割引(時間=106)
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
			case NTNET_TKAK_M:			// 多店舗割引（金額）
			case NTNET_TKAK_T:			// 多店舗割引（時間）
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
				break;
			default:
				continue;
				break;
		}
// MH810100(E) 2020/06/11 指定しない種別がきたら処理しない
		// -------------------- //
		// 割引実処理
		// -------------------- //
		ret = DiscountForOnline();
		// 結果NG？
		if( ret ){
			// 前回QR翳しから、追加となる割引？
// MH810100(S) 2020/09/02 #4803 [08-0001：全サービス券使用限度枚数]で設定されている枚数を超えた際に表示されるPOPUPが、表示されない
//// MH810100(E) 2020/09/02 再精算時の複数枚対応
//// MH810100(S) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
////			if( m_nDiscPage == check_index ){
//			if(IsBackupAreaFound(m_nDiscPage ) == FALSE){
//// MH810100(S) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
//				lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(ret - 1) );	// ポップアップ表示要求
//// MH810100(E) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
//				// 結果がNGだった時、QR登録データの登録をしない
//				break;
//			}else{
//				continue;
//			}
			lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(ret - 1) );	// ポップアップ表示要求
			break;
// MH810100(E) 2020/09/02 #4803 [08-0001：全サービス券使用限度枚数]で設定されている枚数を超えた際に表示されるPOPUPが、表示されない
		}
		// 前回からの差分がNGでなければ正常としておく
		ret = 0;
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// // MH810100(S) K.Onodera  2020/04/10 #4127 車番チケットレス(再度QRコードを翳した場合に表示されない)
// 		// QR利用時の割引データ比較用にバックアップ
// 		memcpy( &DiscountBackUpArea[i++], &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage], sizeof(stDiscount2_t) );
// // MH810100(E) K.Onodera  2020/04/10 #4127 車番チケットレス(再度QRコードを翳した場合に表示されない)
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

		// リアルタイム精算データ用データへ精算結果を書き込む
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j++], &m_stDisc, sizeof(m_stDisc) );
	}

	return (ret);
}

// MH810100(S) K.Onodera  2020/02/12 車番チケットレス(割引済み対応)
//[]----------------------------------------------------------------------[]
///	@brief			割引済額取出し処理
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			割引済
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810100(S) 2020/09/02 再精算時の複数枚対応
// ulong	Ope_GetLastDisc( void )
ulong	Ope_GetLastDisc( long ldiscount )
// MH810100(E) 2020/09/02 再精算時の複数枚対応
{
	stDiscount2_t* pDisc;
	ulong last_disc;

// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//// MH810103(s) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない
//	if (f_sousai != 0) {
//		return 0;
//	}
//// MH810103(e) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更

	pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage];

// MH810100(S) 2020/09/02 再精算時の複数枚対応
//// MH810100(S) 2020/06/05 #4199 【連動評価指摘事項】NT-NET精算データでの割引の差分格納
////// MH810100(S) 2020/05/27 車番チケットレス(#4196)
//////	if( pDisc->DiscSyu >= NTNET_SVS_T ){	// 時間割引？
//////		last_disc = pDisc->DiscountT;	// 割引 時間から取出し(0～999999円)
//////	}else{
//////		last_disc = pDisc->DiscountM;	// 割引 金額から取出し(0～999999円)
//////	}
////	last_disc = pDisc->Discount;				// 金額/時間(0～999999)
////// MH810100(E) 2020/05/27 車番チケットレス(#4196)
//	last_disc = pDisc->UsedDisc;				// 金額/時間(0～999999)
//// MH810100(E) 2020/06/05 #4199 【連動評価指摘事項】NT-NET精算データでの割引の差分格納
	if( pDisc->DiscCount == 1){
		last_disc = pDisc->UsedDisc;				// 金額/時間(0～999999)
	}else{
		// 複数枚の場合は、分ける
		if( g_ulZumiWari > ldiscount){
			last_disc = ldiscount;
			g_ulZumiWari -= ldiscount;
		}else{
			last_disc = g_ulZumiWari;
			g_ulZumiWari = 0;
		}

	}
// MH810100(E) 2020/09/02 再精算時の複数枚対応

	return last_disc;
}
// MH810100(S) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
ulong	Ope_GetLastDiscOrg()
{	
	return g_ulZumiWariOrg;							// 使用済み割引額を分割するまえ
}
// MH810100(E) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）

// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
BOOL Ope_SyubetuWariCheck(DISCOUNT_DATA* pDisc)
{
	BOOL bRet = FALSE;
	BOOL bRepay = FALSE;
	unsigned short wk_dat;
	unsigned short wk_dat2;
	int i;

// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
	if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.FeeCalcOnOff ) {			// 料金計算結果あり
		// クラウド料金計算ではチェックしない
		return FALSE;
	}
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

	wk_dat = (ushort)(1+6*(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType-1));
	wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = 種別時間割引値		*/

	// 精算処理区分が「精算」「再精算」以外
	if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1) ||		// 1=精算
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2) ){		// 2=再精算
		// 精算処理月日あり？
		if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth > 0) &&
			(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay > 0) ){
			bRepay = TRUE;
		}
	}
	if(wk_dat2 && bRepay){	
		for( i=0; i<ONL_MAX_DISC_NUM; i++ ){
			if( lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].DiscSyu == NTNET_SYUBET_TIME ||
				lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].DiscSyu == NTNET_SYUBET  ){
				if( pDisc->Discount > lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].UsedDisc){
					// 1回目に中途半端な数使った場合
					pDisc->DiscCount = 0;		// 0枚
					pDisc->Discount = pDisc->Discount - lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].UsedDisc;
					bRet = FALSE;
				}else{
					// 2回目は載せない
					memset(pDisc,0,sizeof( DISCOUNT_DATA ));
					bRet = TRUE;
				}
				break;
			}

		}
	}
	return bRet;
}
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）

// MH810100(E) K.Onodera  2020/02/12 車番チケットレス(割引済み対応)
//[]----------------------------------------------------------------------[]
///	@brief			リアルタイム精算データのセンター追番を取得
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			センター追番
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ulong	lcdbm_RTPay_GetCenterSeqNo( REAL_SEQNO_TYPE type )
{
	if( type < REAL_SEQNO_TYPE_COUNT ){			// 追番種別が範囲内か
		return REAL_SeqNo[type];
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			リアルタイム精算データのセンター追番を更新
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_RTPay_UpdateCenterSeqNo( REAL_SEQNO_TYPE type )
{
	if( type < REAL_SEQNO_TYPE_COUNT ){			// 追番種別が範囲内か
		++REAL_SeqNo[type];
		if(REAL_SeqNo[type] == 0) {
			REAL_SeqNo[type] = 1;				// センター追番は1～0xffffffffまで
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		リアルタイム精算データのセンター追番を更新してLogに書き込む
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// call元 = LOG_PAYMENT, LOG_PAYSTOPのLog_registのﾀｲﾐﾝｸﾞでcallされる
void RTPay_LogRegist_AddOiban( void )
{
	REAL_SEQNO_TYPE type = REAL_SEQNO_PAY;
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
	if( RTPay_Data.shubetsu == 1 ){ //後日精算
		type =REAL_SEQNO_PAY_AFTER;					// 精算データ(後日精算)
	}
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応

	// リアルタイム精算データのセンター追番取得
	RTPay_Data.CenterOiban = lcdbm_RTPay_GetCenterSeqNo(type);
	RTPay_Data.CenterOibanFusei = 0;
// MH810100(S) 2020/07/27 #4557【検証課指摘事項】 リアルタイム精算データの処理日時が不正(Ａ×：39)
	// 処理年月日時分秒
	RTPay_Data.crd_info.dtTimeYtoSec.shYear = CLK_REC.year;
	RTPay_Data.crd_info.dtTimeYtoSec.byMonth = CLK_REC.mont;
	RTPay_Data.crd_info.dtTimeYtoSec.byDay = CLK_REC.date;
	RTPay_Data.crd_info.dtTimeYtoSec.byHours = CLK_REC.hour;
	RTPay_Data.crd_info.dtTimeYtoSec.byMinute = CLK_REC.minu;
	RTPay_Data.crd_info.dtTimeYtoSec.bySecond = CLK_REC.seco;
// MH810100(E) 2020/07/27 #4557【検証課指摘事項】 リアルタイム精算データの処理日時が不正(Ａ×：39)

	// センター追番 == 0？
	if( RTPay_Data.CenterOiban == 0 ){
		// 追番カウントアップして再取得
		lcdbm_RTPay_UpdateCenterSeqNo(type);
		RTPay_Data.CenterOiban = lcdbm_RTPay_GetCenterSeqNo(type);
		RTPay_Data.CenterOibanFusei = 1;
	}

	// センター追番を更新する
	lcdbm_RTPay_UpdateCenterSeqNo(type);

	// 精算ﾃﾞｰﾀ/精算中止情報のlog書込み処理
	Log_regist( LOG_RTPAY );
}

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//[]----------------------------------------------------------------------[]
///	@brief		RT領収証データのセンター追番を更新してLogに書き込む
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void RTReceipt_LogRegist_AddOiban( void )
{
	REAL_SEQNO_TYPE type = REAL_SEQNO_RECEIPT;

	// RT領収証データのセンター追番取得
	RTReceipt_Data.CenterOiban = lcdbm_RTPay_GetCenterSeqNo(type);
	RTReceipt_Data.CenterOibanFusei = 0;

	// 処理年月日時分秒
	RTReceipt_Data.receipt_info.dtTimeYtoSec.shYear = CLK_REC.year;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.byMonth = CLK_REC.mont;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.byDay = CLK_REC.date;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.byHours = CLK_REC.hour;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.byMinute = CLK_REC.minu;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.bySecond = CLK_REC.seco;

	// センター追番 == 0？
	if ( RTReceipt_Data.CenterOiban == 0 ) {
		// 追番カウントアップして再取得
		lcdbm_RTPay_UpdateCenterSeqNo(type);
		RTReceipt_Data.CenterOiban = lcdbm_RTPay_GetCenterSeqNo(type);
		RTReceipt_Data.CenterOibanFusei = 1;
	}

	// センター追番を更新する
	lcdbm_RTPay_UpdateCenterSeqNo(type);

	// 領収証データのlog書込み処理
	Log_regist(LOG_RTRECEIPT);
}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

//[]----------------------------------------------------------------------[]
///	@brief			センター追番を取得する（DC-NET通信）
//[]----------------------------------------------------------------------[]
///	@param			type	: 追番種別
//[]----------------------------------------------------------------------[]
///	@return			種別に対応するセンター追番<br>
///					0		: 追番種別不正
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ulong DC_GetCenterSeqNo( DC_SEQNO_TYPE type )
{
	if( type < DC_SEQNO_TYPE_COUNT ){			// 追番種別が範囲内か
		return DC_SeqNo_wk[type];
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			センター追番を更新する（DC-NET通信）
//[]----------------------------------------------------------------------[]
///	@param			type	: 追番種別
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_UpdateCenterSeqNo( DC_SEQNO_TYPE type )
{
	if( type < DC_SEQNO_TYPE_COUNT ){			// 追番種別が範囲内か
		++DC_SeqNo_wk[type];
		if(DC_SeqNo_wk[type] == 0) {
			DC_SeqNo_wk[type] = 1;				// センター追番は1～0xffffffffまで
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			センター追番をPushする（DC-NET通信）
//[]----------------------------------------------------------------------[]
///	@param			type	: 追番種別
//[]----------------------------------------------------------------------[]
///	@return			種別に対応するセンター追番<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_PushCenterSeqNo( DC_SEQNO_TYPE type )
{
	if( type < DC_SEQNO_TYPE_COUNT ){			// 追番種別が範囲内か
		DC_SeqNo_wk[type] =  DC_SeqNo[type];
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			センター追番をPopする（DC-NET通信）
//[]----------------------------------------------------------------------[]
///	@param			type	: 追番種別
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_PopCenterSeqNo( DC_SEQNO_TYPE type )
{
	if( type < DC_SEQNO_TYPE_COUNT ){			// 追番種別が範囲内か
		DC_SeqNo[type] =  DC_SeqNo_wk[type];
	}
}

// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera  2020/02/27 車番チケットレス(買物割引対応)
//[]----------------------------------------------------------------------[]
///	@brief			買物割引チェック処理
//[]----------------------------------------------------------------------[]
///	@param[in]		pno		: 駐車場№
///	@param[in/out]	*p_kind	: 割引種別へのポインタ
///	@param[in]		mno		: 店№
///	@param[out]		*p_info	: 割引適用№(割引情報1)へのポインタ
//[]----------------------------------------------------------------------[]
///	@return			ret		: 割引種別
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
///	@mnote			該当店№の割引が買物割引である場合、NTNET_SHOP_DISC_AMT
///					(買物割引(料金))/NTNET_SHOP_DISC_TIME(買物割引(時間))を
///					を戻し、買物割引でない場合は入力割引種別をそのまま返す
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void CheckShoppingDisc( ulong pno, ushort *p_kind, ushort mno, ushort *p_info )
{
	stDiscount2_t	*pDisc;
	ushort			i;
	ushort			wk = 0;

	for( i = 0; i < ONL_MAX_DISC_NUM; i++) {
		pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];

		if(( pDisc->DiscSyu == NTNET_SHOP_DISC_AMT ) ||	// 割引種別 = 買物割引(料金)？
		   ( pDisc->DiscSyu == NTNET_SHOP_DISC_TIME )){	// 割引種別 = 買物割引(時間)？
			if(( pDisc->DiscParkNo == pno ) &&	// 駐車場№と店№が一致？
			   ( pDisc->DiscCardNo == mno )){
				wk = (ushort)CPrmSS[S_STO][1+3*(mno-1)];
				if( wk == 1 ){
					*p_kind = NTNET_SHOP_DISC_TIME;
				}else{
					*p_kind = NTNET_SHOP_DISC_AMT;
				}
				*p_info = pDisc->DiscInfo;	// 割引適用№セット
				return;
			}
		}
	}

	return;
}
// MH810100(E) K.Onodera  2020/02/27 車番チケットレス(買物割引対応)

// MH810100(S) 2020/05/27 車番チケットレス(#4181)
void	azukari_popup(uchar cancel)
{
	ePOP_DISP_KIND	DispCode;	// ポップアップ表示種別
	uchar	DispStatus;			// 表示ステータス
	// ﾎﾟｯﾌﾟｱｯﾌﾟ表示要求(精算中に発生する表示要求) & 対応する音声案内を流す
	// cancel	0 = 精算時の預り証発行		1 = 精算中止時の預り証発行
	// ■OPECTL.RECI_SW		// 1 = 領収書ﾎﾞﾀﾝ使用	0 = 領収書ﾎﾞﾀﾝ未使用
	// ■OPECTL.Fin_mod		// 0 = 同額		1 = 釣り無し		2 = 釣り有り	3 = ﾒｯｸｴﾗｰ(釣り有り)
	// 1:預り証発行(精算完了時)	2:預り証発行(払い戻し時)
	// 精算時の預り証発行
	if (cancel == 0) {
		DispCode = POP_AZUKARI_PAY;
	}
	// 精算中止時の預り証発行
	else {
		DispCode = POP_AZUKARI_RET;
	}
	// 0 = 同額, 1 = 釣り無し, 2 = 釣り有り, 3 = ﾒｯｸｴﾗｰ(釣り有り)
	switch (OPECTL.Fin_mod) {
		case 0:
		case 1:
		default:
			// 1 = 領収書ﾎﾞﾀﾝ使用	0 = 領収書ﾎﾞﾀﾝ未使用
			if (OPECTL.RECI_SW == 0) {
				DispStatus = 0;		// 釣銭なし
			}
			else {
				DispStatus = 2;		// 釣銭なし(領収証)
			}
			break;
		case 2:
		case 3:
			if (OPECTL.RECI_SW == 0) {
				DispStatus = 1;		// 釣銭あり
			}
			else {
				DispStatus = 3;		// 釣銭あり(領収証)
			}
			break;
	}
	// ポップアップ表示要求(預り証発行)
	lcdbm_notice_dsp( DispCode, DispStatus );
}
// MH810100(E) 2020/05/27 車番チケットレス(#4181)

// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
//// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//// 種別割引をみつけて、あったら先頭に持ってくる（2つ以上あることは想定していない）
//void DiscountSortCheck()
BOOL DiscountSortCheck()
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
{
	int i;
	int nFoundCnt = ONL_DISC_MAX + 1;
	stDiscount2_t* pDisc = NULL;
	stDiscount2_t tmpDisc;
	BOOL bRepay = FALSE;
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
	BOOL bRet = FALSE;
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))

	if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1) ||		// 1=精算
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2) ||		// 2=再精算
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 4) ){		// 4=再精算中止
		bRepay = TRUE;
	}

	memset(&tmpDisc,0,sizeof(stDiscount2_t));

	// 種別切り替を見つける（設定がおかしい場合は処理を抜ける（あとでエラーとなるonlinediscountあたりで）
	for( i =0; i < ONL_DISC_MAX; i++ ){
		pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(種別切換ソートの変更)
// 		switch (pDisc->DiscSyu) {
// 			case NTNET_SVS_M:			// ｻｰﾋﾞｽ券(金額=1)
// 			case NTNET_SVS_T:			// ｻｰﾋﾞｽ券(時間=101)
// 				// サービス券種範囲チェック
// 				if( !rangechk( 1, SVS_MAX, pDisc->DiscCardNo ) ){
// // MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// //					return;
// 					return FALSE;
// // MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// 				}
// 
// 				// 切換先の車種設定
// 				if( CPrmSS[S_SER][3+3*(pDisc->DiscCardNo-1)] ){			// 種別切替有り?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*(pDisc->DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
// // MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// //						return;										// 種別切替なし
// 						return FALSE;
// // MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// 					}
// 					if(bRepay == FALSE){
// 						// 見つけた(上書き)
// 						nFoundCnt = i;
// // MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// 						bRet = TRUE;
// // MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// 					}else{
// 						// 再精算時は、使用済みの場合のみ一番上にもってくる
// 						if( pDisc->DiscStatus == 2){
// 							// 見つけた(上書き)
// 							nFoundCnt = i;
// // MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// 							bRet = TRUE;
// // MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// 						}
// 					}
// 				}
// 				break;
// 			case NTNET_KAK_M:			// 店割引(金額=2)
// 			case NTNET_KAK_T:			// 店割引(時間=102)
// 			case NTNET_SHOP_DISC_AMT:	// 買物割引(金額=6)
// 			case NTNET_SHOP_DISC_TIME:	// 買物割引(時間=106)
// 				// 店№範囲チェック
// 				if( !rangechk( 1, MISE_NO_CNT, pDisc->DiscCardNo ) ){
// // MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// //					return;
// 					return FALSE;
// // MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// 				}
// 
// 				// 切換先の車種設定
// 				if( CPrmSS[S_STO][3+3*(pDisc->DiscCardNo-1)] ){			// 種別切替有り?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*(pDisc->DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
// // MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// //						return;										// 種別切替なし
// 						return FALSE;
// // MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
// 					}
		// チェック
		if ( CheckDiscount(pDisc->DiscSyu, pDisc->DiscCardNo, pDisc->DiscInfo) ) {
			return FALSE;
		}

		// 種別切換あり
		if ( GetTypeSwitch(pDisc->DiscSyu, pDisc->DiscCardNo) ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(種別切換ソートの変更)
			if ( bRepay == FALSE ) {
				// 見つけた(上書き)
				nFoundCnt = i;
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
				bRet = TRUE;
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
			} else {
				// 再精算時は、使用済みの場合のみ一番上にもってくる
				if ( pDisc->DiscStatus == 2 ) {
					// 見つけた(上書き)
					nFoundCnt = i;
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
					bRet = TRUE;
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
				}
			}
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(種別切換ソートの変更)
// 				}
// 				break;
// 			default:
// 				continue;
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(種別切換ソートの変更)
		}
		if( nFoundCnt < (ONL_DISC_MAX + 1)){
			// 見つかった
			break;
		}
	}
	// 0でなくて、見つけた場合（0の場合は入れ替える必要がないため）
	if( nFoundCnt != 0 && nFoundCnt < (ONL_DISC_MAX + 1)){
		// tmpに移動
		memcpy(&tmpDisc,&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[nFoundCnt],sizeof(stDiscount2_t));
		for( i = nFoundCnt; i > 0; i--){
			// 1つずつ後ろにずらす
			memcpy( &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i],&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i-1],sizeof(stDiscount2_t));
		}
		// tmpを先頭に持ってくる
		memcpy(&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[0],&tmpDisc,sizeof(stDiscount2_t));

	}
// MH810100(S) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
	return bRet;
// MH810100(E) 2020/08/20 車番チケットレス(#4745 【検証課指摘事項】種別切替後の再精算で初回精算時に使用した割引が適用されていない(No58))
}

// 複数の種別割引があるかどうかチェック（再精算時は、割引ステータス0（未割引があったらNG））
BOOL IsDupSyubetuwari(void)
{
	BOOL bRet = TRUE;
	int i;
	int nFoundCnt = 0;
	stDiscount2_t* pDisc = NULL;
	BOOL bRepay = FALSE;
	BOOL bRepayNotDiscount = FALSE;

	if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1) ||		// 1=精算
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2) ||		// 2=再精算
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 4) ){		// 4=再精算中止
		bRepay = TRUE;
	}

	// 種別切り替を見つける
	for( i =0; i < ONL_DISC_MAX; i++ ){
		pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(複数種別切換チェックの変更)
// 		switch (pDisc->DiscSyu) {
// 			case NTNET_SVS_M:			// ｻｰﾋﾞｽ券(金額=1)
// 			case NTNET_SVS_T:			// ｻｰﾋﾞｽ券(時間=101)
// 				// サービス券種範囲チェック
// 				if( !rangechk( 1, SVS_MAX, pDisc->DiscCardNo ) ){
// 					continue;
// 				}
// 
// 				// 切換先の車種設定
// 				if( CPrmSS[S_SER][3+3*(pDisc->DiscCardNo-1)] ){			// 種別切替有り?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*(pDisc->DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
// 						continue;										// 種別切替なし
// 					}
// 					// 見つけた
// 					nFoundCnt++;
// 					if(bRepay){
// 						// 再精算時で、未割引かどうか
// 						if(pDisc->DiscStatus == 0){
// 							bRepayNotDiscount = TRUE;
// 						}
// 					}
// 				}
// 				break;
// 			case NTNET_KAK_M:			// 店割引(金額=2)
// 			case NTNET_KAK_T:			// 店割引(時間=102)
// 			case NTNET_SHOP_DISC_AMT:	// 買物割引(金額=6)
// 			case NTNET_SHOP_DISC_TIME:	// 買物割引(時間=106)
// 				// 店№範囲チェック
// 				if( !rangechk( 1, MISE_NO_CNT, pDisc->DiscCardNo ) ){
// 					continue;
// 				}
// 
// 				// 切換先の車種設定
// 				if( CPrmSS[S_STO][3+3*(pDisc->DiscCardNo-1)] ){			// 種別切替有り?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*(pDisc->DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
// 						continue;										// 種別切替なし
// 					}
		// チェック
		if ( CheckDiscount(pDisc->DiscSyu, pDisc->DiscCardNo, pDisc->DiscInfo) ) {
			continue;
		}

		// 種別切換あり
		if ( GetTypeSwitch(pDisc->DiscSyu, pDisc->DiscCardNo) ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(複数種別切換チェックの変更)
			// 見つけた
			nFoundCnt++;
			if ( bRepay ) {
				// 再精算時で、未割引かどうか
				if ( pDisc->DiscStatus == 0 ) {
					bRepayNotDiscount = TRUE;
				}
			}
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(複数種別切換チェックの変更)
// 				}
// 				break;
// 			default:
// 				continue;
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(複数種別切換チェックの変更)
		}
	}
	if(nFoundCnt > 1){
		// 2個以上発見
		bRet = TRUE;
	}else{
		// 1個以下
// MH810100(S) 2020/08/19 車番チケットレス(#4744 【検証課指摘事項】QR割引券(1h)が料金投入後の種別切替で無効になる(No56))
//		bRet = FALSE;
		// 1個かつ入金済みは
// MH810100(S) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
//		if( nFoundCnt && ryo_buf.nyukin){
		// 入金前に種別切替チェック済みg_checkKirikae＝1
		if( nFoundCnt && ryo_buf.nyukin && g_checkKirikae == 0){
// MH810100(E) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
			// すでに入金済み
			bRet = TRUE;
		}else{
			bRet = FALSE;
		}
// MH810100(E) 2020/08/19 車番チケットレス(#4744 【検証課指摘事項】QR割引券(1h)が料金投入後の種別切替で無効になる(No56))
	}
	// 再精算で未割引の種別切替を発見
	if(bRepayNotDiscount){
		bRet  = TRUE;
	}
	return bRet;
}

// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// // 割引情報から在車情報へCopyする
// void CopyDiscountCalcInfoToZaisha(void)
// {
// 	uchar 	loop_cnt;
// 	stDiscount2_t* pDisc2;
// 	stDiscount_t*  pDisc3;
// 	/************************************************************************************/
// 	/* 在車情報_割引01～25は割引の精算処理終了時(OnlineDiscount = ｵﾝﾗｲﾝ割引処理)で		*/
// 	/* m_stDiscからｺﾋﾟｰするのでその割引精算後のﾃﾞｰﾀを使用する							*/
// 	/************************************************************************************/
// 	taskchg(IDLETSKNO);
// 	for( loop_cnt = 0; loop_cnt < ONL_MAX_DISC_NUM; loop_cnt++) {
// 		pDisc3 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt];
// 		pDisc2 = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[loop_cnt];
// 
// 		pDisc3->DiscParkNo		= pDisc2->DiscParkNo;			// 割引 駐車場№(0～999999)
// 		pDisc3->DiscSyu			= pDisc2->DiscSyu;				// 割引 種別(0～9999)
// 		pDisc3->DiscCardNo		= pDisc2->DiscCardNo;			// 割引 ｶｰﾄﾞ区分(0～65000)
// 		pDisc3->DiscNo			= pDisc2->DiscNo;				// 割引 区分(0～9)
// 		pDisc3->DiscCount		= pDisc2->DiscCount;			// 割引 枚数(0～99)
// 		pDisc3->DiscInfo		= pDisc2->DiscInfo;				// 割引 割引情報(0～65000)
// 		pDisc3->DiscCorrType	= pDisc2->DiscCorrType;			// 割引 対応ｶｰﾄﾞ種別(0～65000)
// 		pDisc3->DiscStatus		= pDisc2->DiscStatus;			// 割引 ｽﾃｰﾀｽ(0～9)
// 		pDisc3->DiscFlg			= pDisc2->DiscFlg;				// 割引 割引状況(0～9)
// 
// 		pDisc3->Discount		= pDisc2->UsedDisc;				// 金額/時間(0～999999)
// 	}
// }
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
// MH810100(S) 2020/09/04 #4584 【検証課指摘事項】　 同一割引券種数の制限が「１」の場合（No33 再評価NG）(割引上限対応)
// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// BOOL IsWaribikiLimitOver(void)
// {
// 	BOOL bRet = FALSE;
// 	int i;
// 	int nFoundCnt = 0;
// 	stDiscount2_t* pDisc = NULL;
// 
// 	// 有効な割引個数をカウントする
// 	for( i =0; i < ONL_DISC_MAX; i++ ){
// 		pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];
// 		switch (pDisc->DiscSyu) {
// 			case NTNET_SVS_M:			// ｻｰﾋﾞｽ券(金額=1)
// 			case NTNET_SVS_T:			// ｻｰﾋﾞｽ券(時間=101)
// 			case NTNET_KAK_M:			// 店割引(金額=2)
// 			case NTNET_KAK_T:			// 店割引(時間=102)
// 			case NTNET_SHOP_DISC_AMT:	// 買物割引(金額=6)
// 			case NTNET_SHOP_DISC_TIME:	// 買物割引(時間=106)
// // MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引枚数上限チェックの変更)
// 			case NTNET_TKAK_M:			// 多店舗(金額=3)
// 			case NTNET_TKAK_T:			// 多店舗(時間=103)
// // MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引枚数上限チェックの変更)
// 				nFoundCnt++;
// 				break;
// 			default:
// 				continue;
// 		}
// 	}
// 	if( nFoundCnt > WTIK_USEMAX){
// 		bRet = TRUE;
// 	}
// 	return bRet;
// }
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) 2020/09/04 #4584 【検証課指摘事項】　 同一割引券種数の制限が「１」の場合（No33 再評価NG）(割引上限対応)
// MH810103(s) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない
void RTPay_pre_sousai(void)
{
	// ryo_calを実行する前に情報をセットする
	m_stDisc.DiscParkNo = CPrmSS[S_SYS][1];			// 基本駐車場番号ｾｯﾄ
	m_stDisc.DiscCount = 1;							// 1枚
	m_stDisc.Discount = 0;							// 0クリア
	m_stDisc.DiscNo = 0;							// 0:基本
	m_stDisc.DiscInfo = 0;
	m_stDisc.DiscCorrType = 0;
	m_stDisc.DiscStatus = 1;						// 今回割引
	m_stDisc.DiscFlg = 0;							// 

	// vl_sousai関数にてvl_svsにセットされているのでそこを参照
	if(vl_svs.mno != 0){
		// 店割
		m_stDisc.DiscSyu = NTNET_KAK_M;
		m_stDisc.DiscCardNo = vl_svs.mno;
	}else{
		// サービス券
		m_stDisc.DiscSyu = NTNET_SVS_M;
		m_stDisc.DiscCardNo = vl_svs.sno;

	}

}

void RTPay_set_sousai(void)
{
	short cnt = 0;

	// 相殺割引が実施されたので、リアルタイム精算データ用データへ精算結果を書き込む
	for( cnt = 0; cnt < ONL_DISC_MAX; cnt++ ){
		if(( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt].DiscParkNo == 0L ) &&
			( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt].DiscSyu == 0 )){
			// 空エリア発券
			break;
		}
	}
	if( cnt < ONL_DISC_MAX){
		// リアルタイム精算データ用データへ精算結果を書き込む
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt], &m_stDisc, sizeof(m_stDisc) );
	}
}
// MH810103(e) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない

// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
//[]----------------------------------------------------------------------[]
///	@brief			種別切換取得処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syu		: 種別
///	@param[in]		card_no	: カード区分
///	@return			種別切換<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2021/08/30<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
short GetTypeSwitch(ushort syu, ushort card_no)
{
	short typeswitch = 0;

	switch ( syu ) {
	case NTNET_SVS_M:			// ｻｰﾋﾞｽ券(金額=1)
	case NTNET_SVS_T:			// ｻｰﾋﾞｽ券(時間=101)
		// サービス券設定から取得
		typeswitch = (short)prm_get(COM_PRM, S_SER, (card_no * 3), 2, 1);
		break;
	case NTNET_KAK_M:			// 店割引(金額=2)
	case NTNET_KAK_T:			// 店割引(時間=102)
	case NTNET_SHOP_DISC_AMT:	// 買物割引(金額=6)
	case NTNET_SHOP_DISC_TIME:	// 買物割引(時間=106)
		// 店割引設定から取得
		typeswitch = (short)prm_get(COM_PRM, S_STO, (card_no * 3), 2, 1);
		break;
	case NTNET_TKAK_M:			// 多店舗割引(金額=3)
	case NTNET_TKAK_T:			// 多店舗割引(時間=103)
		if ( card_no > MISE_NO_CNT ) {
			// 多店舗割引設定から取得
			typeswitch = (short)prm_get(COM_PRM, S_TAT, (card_no - 69), 2, 1);
		} else {
			// 店割引設定から取得
			typeswitch = (short)prm_get(COM_PRM, S_STO, (card_no * 3), 2, 1);
		}
		break;
	default:
		break;
	}

	return typeswitch;
}

//[]----------------------------------------------------------------------[]
///	@brief			割引チェック処理
//[]----------------------------------------------------------------------[]
///	@param[in]		syu		: 種別
///	@param[in]		card_no	: カード区分
///	@param[in]		info	: 割引情報
///	@return			0  : OK<br>
///					13 : 種別規定外<br>
///					27 : 料金種別ｴﾗｰ<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2021/08/30<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
short CheckDiscount(ushort syu, ushort card_no, ushort info)
{
	short typeswitch = 0;

	switch ( syu ) {
	case NTNET_SVS_M:			// ｻｰﾋﾞｽ券(金額=1)
	case NTNET_SVS_T:			// ｻｰﾋﾞｽ券(時間=101)
		// サービス券種範囲チェック
		if ( !rangechk(1, SVS_MAX, card_no) ) {
			return 13;
		}
		break;
	case NTNET_KAK_M:			// 店割引(金額=2)
	case NTNET_KAK_T:			// 店割引(時間=102)
	case NTNET_SHOP_DISC_AMT:	// 買物割引(金額=6)
	case NTNET_SHOP_DISC_TIME:	// 買物割引(時間=106)
		// 店№範囲チェック
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
// 		if ( !rangechk(1, 100, card_no) ) {
		if ( !rangechk(1, MISE_NO_CNT, card_no) ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
			return 13;
		}
		break;
	case NTNET_TKAK_M:			// 多店舗割引(金額=3)
	case NTNET_TKAK_T:			// 多店舗割引(時間=103)
		// 店№範囲チェック
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
// 		if ( !rangechk(1, 255, card_no) ) {
		if ( !rangechk(1, T_MISE_NO_CNT, card_no) ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
			return 13;
		}
		// 種別範囲チェック
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
// 		if ( !rangechk(1, 7, info) ) {
		if ( !rangechk(1, T_MISE_SYU_CNT, info) ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
			return 13;
		}
		break;
	default:
		break;
	}

	// 種別切換先の車種設定チェック
	typeswitch = GetTypeSwitch(syu, card_no);
	if ( typeswitch ) {
		if ( prm_get(COM_PRM, S_SHA, ((typeswitch * 6) - 5), 2, 5) == 0 ) {
			return 27;
		}
	}

	return 0;
}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(オンライン割引処理の変更)
// MH810105(S) MH364301 QRコード決済対応
//[]----------------------------------------------------------------------[]
///	@brief			障害連絡票の印字処理
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/11/11
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void EcFailureContactPri( void )
{
	T_FrmReceipt	FailureContactData;
	ushort	num;

	memset(&FailureContactData, 0, sizeof(T_FrmReceipt));

	FailureContactData.prn_kind = R_PRI;		/* ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ */

	memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );

	// 車室番号がまだセットされていないため、PayData_setと同じ処理で
	// 車室番号セットを行う
	if( Cancel_pri_work.teiki.update_mon ){							// 更新精算?
		Cancel_pri_work.WPlace = 9999;								// 更新時は車室番号9999とする。
	}else if( ryo_buf.pkiti == 0xffff ){							// ﾏﾙﾁ精算?
		Cancel_pri_work.WPlace = LOCKMULTI.LockNo;					// 接客用駐車位置番号
	}else if( ryo_buf.pkiti == 0 ){									// 後日精算？
		// 遠隔精算（後日精算）は駐車位置番号＝０のため、受信した車室情報をセットする
		Cancel_pri_work.WPlace = OPECTL.Op_LokNo;
	}else{
		num = ryo_buf.pkiti - 1;
		Cancel_pri_work.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// 接客用駐車位置番号
	}
	FailureContactData.prn_data = &Cancel_pri_work;		// 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ

	// pritaskへ通知
	queset(PRNTCBNO, PREQ_RECV_FAILURECONTACT, sizeof(T_FrmReceipt), &FailureContactData);
}
// MH810105(E) MH364301 QRコード決済対応
// GG129001(S) 領収証プリント設定の読み替え対応
//// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証プリント設定）
//const uchar pri_prm_tbl[2][2][2][12] = {
const uchar pri_prm_tbl[2][2][2][15] = {
	// インボイスなし、電子マネー利用なし、ジャーナルプリンタ
//	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	// インボイスなし、電子マネー利用なし、電子ジャーナル
//	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
	// インボイスなし、電子マネー利用あり、ジャーナルプリンタ
//	0, 1, 1, 3, 4, 4, 3, 4, 4, 9,10, 4,
	0, 1, 1, 3, 4, 4, 9,10,10, 9,10,10,12,13,13,
	// インボイスなし、電子マネー利用あり、電子ジャーナル
//	0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1,
	0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1,
	// インボイスあり、電子マネー利用なし、ジャーナルプリンタ
//	0, 1, 2, 0, 4, 5, 0, 4, 8, 0, 4,11,
	0, 1, 2, 3, 4, 5,12,13,14,12,13,14,12,13,14,
	// インボイスあり、電子マネー利用なし、電子ジャーナル
//	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
	// インボイスあり、電子マネー利用あり、ジャーナルプリンタ
//	0, 1, 1, 0, 4, 4, 0, 4, 4, 0, 4, 4,
	0, 1, 1, 3, 4, 4,12,13,13,12,13,13,12,13,13,
	// インボイスあり、電子マネー利用あり、電子ジャーナル
//	0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1,
	0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1,
};
//// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証プリント設定）
// GG129001(E) 領収証プリント設定の読み替え対応
// MH810105(S) MH364301 インボイス対応
//[]----------------------------------------------------------------------[]
///	@brief			インボイス時の17-0001のパラメータ変換
//[]----------------------------------------------------------------------[]
///	@return			jnl_pri_jadge　変換後の値
///	@author			Hayakawa
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/05
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long invoice_prt_param_check( long param_jadge )
{
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証プリント設定）
//	if ( IS_INVOICE ) {							// インボイス適用時
//		if (isEMoneyReader()){					// 電子マネー運用あり
//			if (!isEJA_USE()){					// ジャーナルプリンタの場合
//				switch(param_jadge){
//				case 3:
//				case 6:
//				case 9:
//					param_jadge = 0;
//					break;
//				case 2:
//					param_jadge = 1;
//					break;
//				case 5:
//				case 7:
//				case 8:
//				case 10:
//				case 11:
//					param_jadge = 4;
//					break;
//				default:						// それ以外のparam_jadgeの値はそのまま
//					break;
//				}
//			}
//			else {								// 電子ジャーナルの場合
//				switch(param_jadge){
//				case 3:
//				case 6:
//				case 9:
//					param_jadge = 0;
//					break;
//				case 2:
//				case 4:
//				case 5:
//				case 7:
//				case 8:
//				case 10:
//				case 11:
//					param_jadge = 1;
//					break;
//				default:						// それ以外のparam_jadgeの値はそのまま
//					break;
//				}
//			}
//		}
//		else {									// 電子マネー運用なし
//			if (!isEJA_USE()){					// ジャーナルプリンタの場合
//				switch(param_jadge){
//				case 3:
//				case 6:
//				case 9:
//					param_jadge = 0;
//					break;
//				case 7:
//				case 10:
//					param_jadge = 4;
//					break;
//				default:						// それ以外のparam_jadgeの値はそのまま
//					break;
//				}
//			}
//			else {								// 電子ジャーナルの場合
//				switch(param_jadge){
//				case 3:
//				case 6:
//				case 9:
//					param_jadge = 0;
//					break;
//				case 4:
//				case 7:
//				case 10:
//					param_jadge = 1;
//					break;
//				case 5:
//				case 8:
//				case 11:
//					param_jadge = 2;
//					break;
//				default:						// それ以外のparam_jadgeの値はそのまま
//					break;
//				}
//			}
//		}
//	}
//	return param_jadge;						// インボイスを適用しない場合は引数をそのまま返す
	uchar	ec_brandchk;
	uchar	inv, emn, eja;

	ec_brandchk = 0;
	if (isEC_USE()){
		if (isEcEmoneyEnabled(0, 0) ||
			isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT) ||
			isEcBrandNoEnabledForRecvTbl(BRANDNO_QR)){
			ec_brandchk = 1;
		}
	}

	// インボイス
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//	inv = IS_INVOICE;
	inv = IS_INVOICE_PRM;
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
	// 電子マネー利用
	emn = 0;
	if (isSX10_USE() ||	ec_brandchk) {
		emn = 1;
	}
	// 電子ジャーナル接続
	eja = isEJA_USE();

// GG129001(S) 領収証プリント設定の読み替え対応
//	if (param_jadge > 11) {
	if (param_jadge > 14) {
// GG129001(E) 領収証プリント設定の読み替え対応
		if (!inv && !emn && !eja) {
			// 読み替えなし
			return param_jadge;
		}
		else {
			// 0として扱う
			return 0;
		}
	}

	// 設定読み替え
	return (long)pri_prm_tbl[inv][emn][eja][param_jadge];

// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証プリント設定）
}
// MH810105(E) MH364301 インボイス対応
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//[]----------------------------------------------------------------------[]
///	@brief		中止領収証発行対象判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0=対象外, 1=対象
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2022/06/01<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
uchar	cancelReceipt_chk( void )
{
	uchar i;
	uchar ret = 0;
	wari_tiket	wari_dt;

	// プリペイド使用/回数券使用
	if( card_use[USE_PPC] || card_use[USE_NUM] ){
		// 課税対象にする
		if( prm_get( COM_PRM, S_PRP, 1, 1, 2 ) == 0 ){
			ret = 1;
			return ret;
		}
	}

	// 使用した割引データから課税対象割引データを検索
	for( i = 0; i < WTIK_USEMAX; i++ )
	{

		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );
		if( wari_dt.tik_syu == 0 || wari_dt.tik_syu == INVALID ){
			break;
		}

		// 課税対象設定が有効であり、割引料金が0円でない場合のみ中止領収証発行対象とする
		if( cancelReceipt_Waridata_chk( &wari_dt ) &&
			wari_dt.ryokin != 0 ){
			ret = 1;
			break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		割引データ中止領収証発行対象判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0=対象外, 1=対象
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2022/06/03<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
uchar	cancelReceipt_Waridata_chk( wari_tiket *wk_waridata )
{
	uchar	ret = 0;
	ushort	svs_syu = 0;
	short	section = 0;

	if( wk_waridata->tik_syu == 0 || wk_waridata->tik_syu == INVALID ){
		return ret;
	}

	svs_syu = wk_waridata->syubetu - 1;

	switch( wk_waridata->tik_syu )
	{
		case SERVICE:
			section = S_SER;
			break;

		case KAKEURI:
			// 店No.101以上（多店舗割引）は課税対象としない
			if( wk_waridata->syubetu >= 101 ){
				return ret;
			}
			section = S_STO;
			break;

		default:
			return ret;
	}

	if( prm_get( COM_PRM, section, (short)(1 + (svs_syu * 3)), 1, 2 ) == 1 ){
		ret = 1;
	}

	return ret;
}
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
//[]----------------------------------------------------------------------[]
///	@brief		課税対象サービス券／店割引利用額の売上加算設定
//[]----------------------------------------------------------------------[]
///	@param[in]		ulong	*p_dat 14-0005,0006　券売上加算設定
///	@param[in]		uchar	taxableAdd_set　14-0007⑤
///							0:総売上／総掛売に加算しない
///							1:総売上／総掛売への加算設定(14-0005,0006)に従う
///							2:総売上に加算する
///							3:総掛売に加算する
///							4:両方加算する
///						 0xff:処理しない
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2022/08/05<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void	setting_taxableAdd( ulong *p_dat, uchar taxableAdd_set )
{

	if( p_dat == NULL ){
		return;
	}

	if( taxableAdd_set == 0xff ){									// 課税対象ではない
		return;														// 何もしない
	}
	else{															// 課税対象である
		switch (taxableAdd_set)										// 14-0007⑤に従う
		{
		case 0:	// 総売上／総掛売に加算しない
			(*p_dat) = 0L;
			break;
		default:
		case 1:	// 総売上／総掛売への加算設定(14-0005,0006)に従う
			// 何もしない
			break;
		case 2:	// 総売上に加算する
			(*p_dat) = 1L;
			break;
		case 3:	// 総掛売に加算する
			(*p_dat) = 2L;
			break;
		case 4:	// 両方加算する
			(*p_dat) = 3L;
			break;
		}
	}
}
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
//[]----------------------------------------------------------------------[]
///	@brief			領収証印字データストック
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2023/09/28
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	pri_ryo_stock( uchar kind )
{
	T_FrmReceipt	ReceiptPreqData_Stock;

	memset(&ReceiptPreqData_Stock, 0, sizeof(T_FrmReceipt));

	// 溜め印字を行うため先にデータを送信しておく
	// 新プリンタのみ有効
	if( isNewModel_R() ){							// 新プリンタ

// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 		if (Ope_isPrinterReady() == 0 ||
		if ((!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) ||
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
			(IS_INVOICE && Ope_isJPrinterReady() == 0)) {
			// インボイス設定時はレシートとジャーナルが片方でも動かなかった場合
			// 印字不可の場合はストックしない
			return;
		}

		// ストック
		if( ryo_stock == 0 ){						// ストックなし

			ReceiptPreqData_Stock.prn_kind = kind;	// 種別
			ReceiptPreqData_Stock.prn_data = &PayData;

			// ストック対象データフラグとして借りる（ここでしか使用しないため問題なし）
			// PREQ_RYOUSYUU処理でストック対象データかを判定するために使用する
			ReceiptPreqData_Stock.dummy = 1;
			queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData_Stock);

			ryo_stock = 1;							// ストックあり
		}
	}
}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

// GG129001(S) 領収証プリント設定の読み替え対応
//[]----------------------------------------------------------------------[]
///	@brief		請求額計算処理
//[]----------------------------------------------------------------------[]
///	@return		請求額（駐車料金-課税対象外割引額）
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2024/2/19<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
ulong Billingcalculation(Receipt_data *data)
{
	uchar			i;
	ulong			taxNotDis_total = 0;		// 課税対象外割引額
	ulong			billing = 0;				// 請求額
	wari_tiket		wari_dt;

	// 課税対象額、請求額を計算する
	// 前回割引額
	for( i = 0; i < WTIK_USEMAX ; i++ ){		// 割引券ﾃﾞｰﾀ検索
		disc_wari_conv_prev( &data->DiscountData[i], &wari_dt );
 		if( wari_dt.tik_syu == 0 || wari_dt.tik_syu == INVALID ){	// 割引ﾃﾞｰﾀなし
			continue;
		}
		if( wari_dt.ryokin ){					// 割引料金あり
			switch ( wari_dt.tik_syu ) {
			case C_SERVICE:			// ｻｰﾋﾞｽ券
			case C_KAKEURI:			// 掛売券
			case C_SHOPPING:		// 買物割引
				taxNotDis_total += wari_dt.ryokin;	// 課税対象外
				break;
			default:				// その他
				// 割引以外
				break;
			}
		}
	}
 	// 今回割引額（まとまる場合がある為、最初から再検索）
	for( i = 0; i < WTIK_USEMAX ; i++ ){		// 割引券ﾃﾞｰﾀ検索
		disc_wari_conv_new( &data->DiscountData[i], &wari_dt );
 		if( wari_dt.tik_syu == 0 || wari_dt.tik_syu == INVALID ){	// 割引ﾃﾞｰﾀなし
			continue;
		}
		if( wari_dt.ryokin ){					// 割引料金あり
			switch ( wari_dt.tik_syu ) {
			case SERVICE:			// ｻｰﾋﾞｽ券
			case KAKEURI:			// 掛売券
				if ( !cancelReceipt_Waridata_chk(&wari_dt) ) {
					taxNotDis_total += wari_dt.ryokin;	// 課税対象外
				}
				break;
			case KAISUU:			// 回数券
			case PREPAID:			// プリペイドカード
				if ( prm_get(COM_PRM, S_PRP, 1, 1, 2) == 1 ) {
					taxNotDis_total += wari_dt.ryokin;	// 課税対象外
				}
				break;
			case SHOPPING:			// 買物割引
			case C_SERVICE:			// ｻｰﾋﾞｽ券
			case C_KAKEURI:			// 掛売券
			case C_KAISUU:			// 精算中止回数券
			case C_SHOPPING:		// 買物割引
				taxNotDis_total += wari_dt.ryokin;	// 課税対象外
				break;
			default:				// その他
				// 割引以外
				break;
			}
		}
	}
	// 前回時間（料金）割引や前回ﾌﾟﾘﾍﾟｲﾄﾞ利用金額計算
	taxNotDis_total += (data->ppc_chusi_ryo +
						data->zenkai);

	// 請求額
	if( data->WPrice > taxNotDis_total ){ 			// 割引額より駐車料金の方が大きい場合
		billing = data->WPrice - taxNotDis_total;	// 請求額（駐車料金-課税対象外割引額）
	}
	else{
		billing = 0;
	}

	return billing;
}
// GG129001(E) 領収証プリント設定の読み替え対応

// GG129001(S) データ保管サービス対応（課税対象額をセットする）
//[]----------------------------------------------------------------------[]
///	@brief		課税対象割引額計算
//[]----------------------------------------------------------------------[]
///	@param[in]	data	: PayDataのポインタ
///	@param[in]	typ		: 0=精算完了 1=精算中止
///	@return		課税対象割引額
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2024/5/20<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
ulong	TaxAmountcalculation(Receipt_data *data , uchar typ)
{
	uchar			i;
	ulong			tax_amount = 0;			// 領収額（課税対象額）
	wari_tiket		wari_dt;

	// TODO:課税対象割引対応時に修正する
	// 課税対象額を計算する
	for( i = 0; i < WTIK_USEMAX ; i++ ){		// 割引券ﾃﾞｰﾀ検索
		disc_wari_conv_new( &data->DiscountData[i], &wari_dt );
 		if( wari_dt.tik_syu == 0 || wari_dt.tik_syu == INVALID ){	// 割引ﾃﾞｰﾀなし
			continue;
		}
		if( wari_dt.ryokin ){					// 割引料金あり
			switch ( wari_dt.tik_syu ) {
			case SERVICE:			// ｻｰﾋﾞｽ券
			case KAKEURI:			// 掛売券
				if ( cancelReceipt_Waridata_chk(&wari_dt) ) {
					tax_amount += wari_dt.ryokin;	// 課税対象
				}
				break;
			case KAISUU:			// 回数券
			case PREPAID:			// プリペイドカード
				if ( prm_get(COM_PRM, S_PRP, 1, 1, 2) == 0 ) {
					tax_amount += wari_dt.ryokin;	// 課税対象
				}
				break;
			default:				// その他
				// 割引以外
				break;
			}
		}
	}

	if (typ == 0) {
		// 課税対象額に電子ﾏﾈｰ、ｸﾚｼﾞｯﾄ、現金領収額を加算
		tax_amount += (data->Electron_data.Ec.pay_ryo +
						data->credit.pay_ryo +
						(data->WInPrice - data->WChgPrice));
	}

	return tax_amount;
}
// GG129001(E) データ保管サービス対応（課税対象額をセットする）

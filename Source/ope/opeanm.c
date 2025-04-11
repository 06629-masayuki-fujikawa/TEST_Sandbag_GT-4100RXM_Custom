/*[]----------------------------------------------------------------------[]*/
/*| ｱﾅｳﾝｽﾏｼﾝ関連制御                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4800Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include 	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"prm_tbl.h"
#include	"flp_def.h"
#include	"cnm_def.h"

char	IsSoundOnTime(void);
char	avm_alarm_flg;
char err_chk_note(void);
char	an_vol_flg;

short	AN_buf = -1;
//
// 外に出したた理由：cct.c で "このカードは使えません" を an_msag ( )で行っているが、
// クレジット精算中の "しばらくお待ち下さい" を出す前に AVM_STOP を出していることで、
// エラー発生が速い場合（まだ"しばらく..."の前放送中)、"しばらく..." が復活してしまう事の対応.
// cct.c での an_msag (  ) 時にこれを -1 にする。
//
uchar	ope_chk_paid_cash( void );

void	ope_anm( short prm )
{
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// ope_anm()内の処理を全削除
//	short	cnt, ctl, wait;
//	char	ch = 0;
//	short	msg_cnt = 0;
//	short	an_msgno[10] = {0};					// メッセージ
//		
//
//	wait = 0;
//	if( SODIAC_ERR_NONE == Avm_Sodiac_Err_flg ){
//		switch(prm){
//// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
////		case AVM_Edy_OK:			/* Edy OK 音 */
////		case AVM_Edy_NG:			/* Edy NG 音 */
//// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
//		case AVM_IC_OK:			/* ICカード検知音 */
//		case AVM_IC_NG1:			/* NG音 */
//		case AVM_IC_NG2:			/* NG音 */
//			/* 設定に持たずに直接音を鳴らすもの(効果音:ch1) */
//			an_msgno[0] = prm;
//			an_msag( an_msgno,  1, 1, 0, 1 );
//			break;
//		case AVM_RYOUKIN:						// 料金読み上げ
//		case AVM_SHASHITU:						// 車室読み上げ
//			if( 0 == prm_get( COM_PRM,S_SYS, 47, 1, 1 ) && 0 == prm_get( COM_PRM,S_SYS, 47, 1, 2 )){		
//				if(wait == 0){			// 放送中のアナウンスを中断して鳴らす
//					an_stop(0);		// 停止
//				}
//			}
//			if( prm == AVM_RYOUKIN ){
//				if(0 == prm_get( COM_PRM,S_SYS, 47, 1, 1 )){		// 料金読み上げ設定なし
//					return;
//				}
//				if(1 == prm_get( COM_PRM,S_SYS, 47, 1, 2 )){		// 車室読み上げ設定あり
//					wait = 1;										// 鳴り終わりを待つ
//				}
//			}
//			else{
//				if( 0 == prm_get( COM_PRM,S_SYS, 47, 1, 2 )){		// 車室読み上げ設定なし
//					return;
//				}
//			}
//			msg_cnt = an_msag_edit_rxm(prm);
//			if(msg_cnt != 0){ // メッセージが生成された時のみ送信する
//				// 繰り返し回数:1回,インターバル無,チャンネル1
//				if(wait == 0){										// 放送中のアナウンスを中断して鳴らす
//					an_stop(0);									// 停止
//				}
//				an_msag( (short *)an_msgbuf,  msg_cnt, 1, 0, 0 );
//			}
//			break;
//		case AVM_TEST:							// バージョン要求
//			an_test( 4 );
//			break;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ブザー音
		switch(prm){
		case AVM_BOO:
			if(OPECTL.Mnt_mod == 5){
				return;
			}
			if(avm_alarm_flg == 0 && CPrmSS[S_SYS][60] != 0){	// 警報ON
// MH810100(S) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
//				ExIOSignalwt(EXPORT_M_LD0, 0 );						// 警報発砲中のみ：L（デジタルアンプゲイン：1以上）
// MH810100(E) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
				avm_alarm_flg = 1;
// MH810100(S) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
//				an_boo2(AVM_BOO);
				PKTcmd_alarm_start(BEEP_KIND_ALARM, BEEP_TYPE_START);
// MH810100(E) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
			}
			else if(avm_alarm_flg){						// 警報OFF
// MH810100(S) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
//				ExIOSignalwt(EXPORT_M_LD0, 1 );						// 通常：H（デジタルアンプゲイン：0）設定
// MH810100(E) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
				avm_alarm_flg = 0;
// MH810100(S) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
//				an_stop(1);			// ch1 Stop
				PKTcmd_alarm_start(BEEP_KIND_ALARM, BEEP_TYPE_STOP);
// MH810100(E) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
			}
			break;
		default:												// その他アナウンス(設定参照)
			break;
		}			
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ブザー音
//		case AVM_STOP:								// 音声停止
//			an_stop(0);			// ch0 Stop
//			if(!avm_alarm_flg){						// 警報発砲中は止めない
//				an_stop(1);			// ch1 Stop
//			}
//			break;
//		case AVM_AN_TEST:							// アナウンステスト(メンテナンス)
//			an_stop(0);
//			an_msag( (short *)avm_test_no, avm_test_cnt, 1, 1, avm_test_ch );
//			break;
//
//		default:											// その他アナウンス(設定参照)
//			if(prm == AVM_BGM){													// チャンネル指定
//				ch = 1;
//			}
//			else{
//				ch = 0;
//			}
//			wait = (short)prm_get( COM_PRM,S_ANA,(1+((prm-2)*4)),1,3 );			// 中断
// MH810103 GG119202(S) 電子マネーシングル設定で案内放送を行う
//			if (prm == AVM_SELECT_EMONEY) {
//				if (isEcEmoneyEnabled(1, 0) &&
//					check_enable_multisettle() == 1) {
//					// 登録No.50は電子マネーシングル設定の場合、
//					// 放送開始条件=2とする
//					wait = 2;
//				}
//			}
// MH810103 GG119202(E) 電子マネーシングル設定で案内放送を行う
//			if(wait == 0){										// 放送中のアナウンスを中断して鳴らす
//				if(ch == 1){
//					an_stop(ch);				// 停止
//				}
//				else{
//					if(OPECTL.PassNearEnd == 0){
//						an_stop(ch);			// 停止
//					}
//					else{
//						if(prm != AVM_KIGEN_NEAR_END){
//							OPECTL.PassNearEnd = 0;
//						}
//						else{
//							an_stop(ch);		// 停止
//						}
//					}
//				}
//			}
//			msg_cnt = msg_set(an_msgno, prm);						// メッセージ№取得
//
//			cnt = (short)prm_get( COM_PRM,S_ANA,(1+((prm-2)*4)),2,4 );			// 放送回数
//			if( cnt != 0 ){
//				if( cnt == 99 ){
//					cnt = 0x0f;
//				}
//				ctl = (short)prm_get( COM_PRM,S_ANA,(1+((prm-2)*4)),2,1 );			// ｲﾝﾀｰﾊﾞﾙ
//
//				an_msag( an_msgno,  msg_cnt, cnt, ctl, ch );
//			}
//			break;
//		}
//	}
//	return;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
}
/*[]----------------------------------------------------------------------[]*/
/*| メッセージ№格納処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : msg_set(short *an_msgno, short prm )                    |*/
/*| PARAMETER    : prm : 設定ｱﾄﾞﾚｽ№25-0002～25-0030のｱﾄﾞﾚｽ№2～30         |*/
/*|              : an_msgno : メッセージ№の格納先                         |*/
/*| RETURN VALUE : msg_cnt  : メッセージ数                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2009-09-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
short	msg_set(short *an_msgno, short prm )
{
	short	msg_cnt, p1, p2;
	char	i,j;
	short	rag_time;
	
	msg_cnt = 0;

	for(i=1; i<4; i++){
		p1 = (short)( CPrmSS[S_ANA][1+((prm-2)*4)+i] / 1000L );			// ﾒｯｾｰｼﾞ№1
		p2 = (short)( CPrmSS[S_ANA][1+((prm-2)*4)+i] % 1000L );			// ﾒｯｾｰｼﾞ№2
		if(p1 != 0 ){
			an_msgno[msg_cnt] = p1;
			msg_cnt++;
		}
		if(p2 != 0 ){
			an_msgno[msg_cnt] = p2;
			msg_cnt++;
		}
	}
	
	/* 使用不可媒体のメッセージは削除する */
	if( prm == AVM_TURIARI || prm == AVM_TURINASI || prm == AVM_RYOUSYUU){
		for(i = 0;i < msg_cnt; i++){
// MH810105(S) MH364301 インボイス対応
//			if(an_msgno[i] == 147 && ( Ope_isPrinterReady() == 0 || OPECTL.RECI_SW == 1)){				// 紙切れ状態で領収証メッセージ
			if( an_msgno[i] == 147 &&
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 				((Ope_isPrinterReady() == 0 || (IS_INVOICE && Ope_isJPrinterReady() == 0)) ||
				(((!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) || (IS_INVOICE && Ope_isJPrinterReady() == 0)) ||
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
				 OPECTL.RECI_SW == 1) ){										// 紙切れ状態で領収証メッセージ
// MH810105(E) MH364301 インボイス対応
				for(j = i; j < msg_cnt; j++){
					memcpy(&an_msgno[j], &an_msgno[j+1], 2);
				}
				msg_cnt-=1;
				i-=1;
			}
		}
		if( prm != AVM_RYOUSYUU && err_chk_note() != 0 ){						// 紙幣リーダー使用不可
			an_msgno[msg_cnt] = 180;											// 紙幣利用不可メッセージ
			msg_cnt++;
		}
	}
	if( AVM_FLAP_BAN == an_msgno[msg_cnt - 1] || AVM_LOCK_BAN == an_msgno[msg_cnt - 1] ){	// “フラップ(ロック)板が下がったことを確認後、”
		if( 0 == prm_get( COM_PRM,S_STM,1,1,1 )){	// サービスタイム切換    0=なし(全車種共通)／1=あり(車種毎に設定)
			rag_time = prm_get( COM_PRM,S_STM,4,3,1 );									// 全車種共通のラグタイム取得
		} else {
			rag_time = prm_get( COM_PRM,S_STM,(short)(7+((SvsTime_Syu[ OPECTL.Pr_LokNo - 1 ] - 1)*3)),3,1 );	// 各車種共通のラグタイム取得
		}
		if( 10 > msg_cnt 												// メッセージ領域が空いていない場合は実施しない
			&& (( 3 == OPECTL.Ope_mod ) || ( 220 == OPECTL.Ope_mod ))){	// 精算完了 or ラグタイム延長の場合
			if( 0 < rag_time && 16 > rag_time ){	// ラグタイムが1～15の場合は“○分以内に出庫してください”
				an_msgno[msg_cnt] = AVM_1MINUTE + rag_time - 1;	// “○分以内に”
				msg_cnt++;
			}
		}
		if( 10 > msg_cnt ){	// メッセージ領域が空いていない場合は実施しない
			an_msgno[msg_cnt] = AVM_SYUKKO;						// “出庫して下さい”
			msg_cnt++;
		}
	}
			
	return msg_cnt;
	
}
/*[]----------------------------------------------------------------------[]*/
/*| 紙幣リーダーの使用可否判断                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : err_chk_note(void)                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret 0：使用可 1：使用不可                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2009-09-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
char err_chk_note(void)
{
	char ret,i;
	ret = 0;
	for(i=0;i<10;i++){
		if(ERR_CHK[mod_note][i]){
			ret = 1;
			break;
		}
	}
	if( ALM_CHK[1][6] ){	// 紙幣金庫満杯アラーム発生中？
		ret = 1;			// 使用不可とする。
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ドアノブ戻し忘れ防止チャイム
//[]----------------------------------------------------------------------[]
///	@param[in]		onoff	: 0:OFF, 1:ON
///	@return			void
///	@author			m.onouchi
///	@note			ドアノブ戻し忘れ防止チャイムの開始，停止を制御する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/13<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void opa_chime(char onoff)
{
	static char chime_flg = 0;

	if (onoff) {	// ON
		if (!chime_flg) {
			if (!avm_alarm_flg) {				// ドア警報なし
// MH810100(S) S.Takahashi 2020/02/21 #3915 ドアノブ戻し忘れ防止アラームが鳴らない
//				an_boo2(AVM_IC_NG2);			// 暫定的にメッセージ番号105を使用する。
				PKTcmd_alarm_start(BEEP_KIND_CHIME, BEEP_TYPE_START);
// MH810100(E) S.Takahashi 2020/02/21 #3915 ドアノブ戻し忘れ防止アラームが鳴らない
			}
		}
	} else {		// OFF
		if (chime_flg) {
			if (!avm_alarm_flg) {				// ドア警報なし
// MH810100(S) S.Takahashi 2020/02/21 #3915 ドアノブ戻し忘れ防止アラームが鳴らない
//				an_stop(1);						// チャンネル１(効果音)音声停止
				PKTcmd_alarm_start(BEEP_KIND_CHIME, BEEP_TYPE_STOP);
// MH810100(E) S.Takahashi 2020/02/21 #3915 ドアノブ戻し忘れ防止アラームが鳴らない
			}
		}
	}
	chime_flg = onoff;
}
/*[]----------------------------------------------------------------------[]*/
/*| 現在の入金額で精算完了したかチェックする                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ope_chk_paid_cash(void)                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret 0：精算完了分入金あり 1：入金不足                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2012-05-28                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	ope_chk_paid_cash( void )
{
	if( ryo_buf.dsp_ryo <= cn_GetTempCredit()) {
		return 0;		// 精算完了
	}
	else {
		return 1;		// 入金不足
	}
}


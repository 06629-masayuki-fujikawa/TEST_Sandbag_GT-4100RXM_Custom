//[]----------------------------------------------------------------------[]
///	@file		logctrl2.c
///	@brief		LOG関連関数2
/// @date		2012/05/08
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// NOTE:ログデータ登録関数は停復電処理にてコンパイラの最適化処理をかけると正常に
//      動作しないのでこのファイルのコンパイル最適化を抑止する
#include	<string.h>
#include	"system.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"ntnet.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"flp_def.h"
#include	"LKcom.h"
#include	"ntnet_def.h"
#include	"fla_def.h"
#include	"rauconstant.h"
extern ushort	Ope_Log_GetLogMaxCount( short id );
//	下記ﾃﾞｰﾀは非初期化ｴﾘｱにはいれない ---> 36_0048が変更されて通常起動した時の為
ushort		nearfullMaxReadSTS = 0;		// 0=init, 1=readREQ, 2=readDone

const ushort LogDatMax[][2] = {
	{sizeof(Receipt_data), RECODE_SIZE},	// eLOG_PAYMENT
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	{sizeof(enter_log), RECODE_SIZE},		// eLOG_ENTER
	{sizeof(RTPay_log), RECODE_SIZE},		// eLOG_RTPAY
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
	{sizeof(SYUKEI), RECODE_SIZE2},			// eLOG_TTOTAL
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	{sizeof(LCKTTL_LOG), RECODE_SIZE2},		// eLOG_LCKTTL
	{sizeof(RTReceipt_log), RECODE_SIZE2},	// eLOG_RTRECEIPT
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	{sizeof(Err_log), RECODE_SIZE},			// eLOG_ERROR
	{sizeof(Arm_log), RECODE_SIZE},			// eLOG_ALARM
	{sizeof(Ope_log), RECODE_SIZE},			// eLOG_OPERATE
	{sizeof(Mon_log), RECODE_SIZE},			// eLOG_MONITOR
	{sizeof(flp_log), RECODE_SIZE},			// eLOG_ABNORMAL
	{sizeof(TURI_KAN), RECODE_SIZE},		// eLOG_MONEYMANAGE
	{sizeof(ParkCar_log), RECODE_SIZE},		// eLOG_PARKING
	{sizeof(NGLOG_DATA), RECODE_SIZE},		// eLOG_NGLOG
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
//	{sizeof(IoLog_Data), RECODE_SIZE},		// eLOG_IOLOG
	{sizeof(DC_QR_log), RECODE_SIZE},		// eLOG_DC_QR
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
	{sizeof(meisai_log), RECODE_SIZE},		// eLOG_CREUSE
	{sizeof(meisai_log), RECODE_SIZE},		// eLOG_iDUSE
	{sizeof(meisai_log), RECODE_SIZE},		// eLOG_HOJIN_USE
	{sizeof(t_Change_data), RECODE_SIZE},	// eLOG_REMOTE_SET
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//	{sizeof(LongPark_log), RECODE_SIZE},	// eLOG_LONGPARK
	{sizeof(LongPark_log_Pweb), RECODE_SIZE},// eLOG_LONGPARK_PWEB
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	{sizeof(RismEvent_log), RECODE_SIZE},	// eLOG_RISMEVENT
	{sizeof(DC_LANE_log), RECODE_SIZE},		// eLOG_DC_LANE
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	{sizeof(SYUKEI), RECODE_SIZE2},			// eLOG_GTTOTAL
	{sizeof(Rmon_log), RECODE_SIZE},		// eLOG_REMOTE_MONITOR
	{sizeof(COIN_SYU), RECODE_SIZE2},		// eLOG_COINBOX(ramのみ)
	{sizeof(NOTE_SYU), RECODE_SIZE2},		// eLOG_NOTEBOX(ramのみ)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	{sizeof(edy_arm_log), RECODE_SIZE2},	// eLOG_EDYARM(ramのみ)
//	{sizeof(edy_shime_log), RECODE_SIZE2},	// eLOG_EDYSHIME(ramのみ)
	{0, RECODE_SIZE2},						// eLOG_EDYARM(ramのみ)
	{0, RECODE_SIZE2},						// eLOG_EDYSHIME(ramのみ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	{sizeof(Pon_log), RECODE_SIZE2},		// eLOG_POWERON(ramのみ)
	{sizeof(TURI_KAN), RECODE_SIZE2},		// eLOG_MNYMNG_SRAM(ramのみ)
	{0, 0}									// terminator
};

//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ登録
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG種別
///	@param[in]	dat		: LOGデータ
///	@param[in]	strage	: SRAMフラグ
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 2012/05/08 A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Log_Write(short Lno, void *dat, BOOL strage)
{
	int		i;
	ushort	cnt = LOG_SECORNUM(Lno);
	struct log_record	*lp = LOG_DAT+Lno;
	struct	log_record	*bp;
	ushort	wcnt;
	short	kind = 0;
// MH322914 (s) kasiyama 2016/07/13 ログ保存できない状態を改善[共通バグNo.1225](MH341106)
	ulong	ul_Lno = Lno;
	uchar	err_log[sizeof(Err_log)];
// MH322914 (e) kasiyama 2016/07/13 ログ保存できない状態を改善[共通バグNo.1225](MH341106)

	if(strage){								// Flashにデータをもつ
		if( lp->count[lp->kind] >= cnt ){	// これから書き込むSRAM領域の件数MAX
// MH322914 (s) kasiyama 2016/07/13 ログ保存できない状態を改善[共通バグNo.1225](MH341106)
//			return;
			if( FLT_Check_LogWriteReq(Lno) == TRUE){ // FROM書き込み中
				FLT_Force_ResetLogWriteReq(Lno);// 書き込み要求状態を強制的にリセットする
			}
			for(i=0; i<eLOG_TARGET_MAX; i++){
				if( lp->unread[i] > lp->count[lp->kind] ) {
					lp->unread[i] -= lp->count[lp->kind];
				} else {
					lp->unread[i] = 0;
				}
			}
			lp->count[lp->kind] = 0; // SRAMデータ状態クリア
			lp->wtp = 0;
			if(Lno == eLOG_ERROR) {
				// エラーログ書き込み中にエラーログを書き込むためエラーログを退避する
				memcpy(err_log, dat, sizeof(Err_log));
			}
			err_chk2(ERRMDL_MAIN, ERR_MAIN_SRAM_LOG_BUFFER_FULL, 2, 2, 0, &ul_Lno);	// SRAMログバッファフル
			if(Lno == eLOG_ERROR) {
				memcpy(dat, err_log, sizeof(Err_log));
			}
// MH322914 (e) kasiyama 2016/07/13 ログ保存できない状態を改善[共通バグNo.1225](MH341106)
		}
	}
	log_bakup.Lno = Lno;
	log_bakup.strage = strage;
	log_bakup.dat_p = dat;
	log_bakup.stat_kind = lp->kind;
	log_bakup.stat_wtp = lp->wtp;
	log_bakup.stat_count = lp->count[lp->kind];
	for(i=0; i<eLOG_TARGET_MAX; i++){
		log_bakup.f_unread[i] = lp->f_unread[i];
	}
	log_bakup.f_recover = 1;// 処理開始
	if(log_bakup.Lno == eLOG_TTOTAL){// T集計
		if(ac_flg.syusyu == 14){
			ac_flg.syusyu = 15;// Ｔ合計ログ登録受付
		}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	}else if(log_bakup.Lno == eLOG_LCKTTL){// 車室毎集計
//		if(ac_flg.syusyu == 15){
//			ac_flg.syusyu = 19;// 車室毎集計ログ(31車室以降)登録受付
//		}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	}else if(log_bakup.Lno == eLOG_GTTOTAL){// GT集計
		if(ac_flg.syusyu == 24){
			ac_flg.syusyu = 25;// ＧＴ合計ログ登録受付
		}
	}else if(log_bakup.Lno == eLOG_COINBOX){// コイン金庫集計ログ
		if(ac_flg.syusyu == 34){
			ac_flg.syusyu = 35;// コイン金庫集計ログ受付
		}
	}else if(log_bakup.Lno == eLOG_MONEYMANAGE){// 金銭管理ログ(NT-NETデータ用)
		if(ac_flg.syusyu == 35){// コイン金庫集計時
			ac_flg.syusyu = 36;// 金銭管理ログログ受付
		}else if(ac_flg.syusyu == 45){// 紙幣金庫集計時
			ac_flg.syusyu = 46;// 金銭管理ログログ受付
		}else if(ac_flg.syusyu == 52){// 金銭管理集計時
			ac_flg.syusyu = 53;// 金銭管理ログログ受付
		}else if(ac_flg.cycl_fg == 54){// 精算中止完了時
			ac_flg.cycl_fg = 0;
		}else if(ac_flg.cycl_fg == 17){// 精算完了時(17:個別精算ﾃﾞｰﾀ登録完了)
			ac_flg.cycl_fg = 0;
		}else if(ac_flg.cycl_fg == 27){// 精算中止完了時(27:精算中止ﾃﾞｰﾀ登録完了)
			ac_flg.cycl_fg = 0;
		}
	}else if(log_bakup.Lno == eLOG_NOTEBOX){// 紙幣金庫集計ログ
		if(ac_flg.syusyu == 44){
			ac_flg.syusyu = 45;// 紙幣金庫集計ログ受付
		}
	}else if(log_bakup.Lno == eLOG_MNYMNG_SRAM){// 金銭管理ログ(SRAM)
		if(ac_flg.syusyu == 53){
			ac_flg.syusyu = 54;// 金銭管理ログ(SRAM)受付
		}
	}else if(log_bakup.Lno == eLOG_ABNORMAL){// 不正・強制出庫ログ
		if(ac_flg.cycl_fg == 81){
			ac_flg.cycl_fg = 34;// 不正・強制出庫ログ受付
		}
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	}else if((log_bakup.Lno == eLOG_CREUSE)||(log_bakup.Lno == eLOG_iDUSE)){// クレジット精算
//		if(ac_flg.cycl_fg == 80){// クレジット利用明細登録開始
//			ac_flg.cycl_fg = 17;// 精算完了時(17:個別精算ﾃﾞｰﾀ登録完了)
//		}
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
	}else if(log_bakup.Lno == eLOG_PAYMENT){// 精算ログ
		if(ac_flg.cycl_fg == 33){// 不正・強制出庫情報登録
			ac_flg.cycl_fg = 81;// 不正・強制出庫データ(NT-NET精算データ用)
		}else if(ac_flg.cycl_fg == 43){// フラップ上昇、ロック閉タイマ内出庫
			ac_flg.cycl_fg = 45;// フラップ上昇、ロック閉タイマ内出庫(NT-NET精算データ用)
		} else {
			if(ac_flg.cycl_fg == 15){// 精算完了時
				ac_flg.cycl_fg = 16;// 精算ログ受付
			}else if(ac_flg.cycl_fg == 25){// 精算中止時①
				// cyu_syuu()中の判定によりセットするフラグが変わる
				if( ryo_buf.ryo_flg >= 2 ){// 定期使用
					ac_flg.cycl_fg = 26;// 精算ログ受付
				}else{
					ac_flg.cycl_fg = 28;// 精算ログ受付
				}
			}else if(ac_flg.cycl_fg == 52){// 精算中止時②
				// toty_syu()中の判定によりセットするフラグが変わる
				if( ryo_buf.ryo_flg >= 2 ){// 定期使用
					ac_flg.cycl_fg = 53;// 精算ログ受付
				}else{
					ac_flg.cycl_fg = 55;// 精算ログ受付
				}
// MH321800(S) T.Nagai ICクレジット対応
			}else if(ac_flg.cycl_fg == 57){		// 決済精算中止時
				ac_flg.cycl_fg = 58;			// 精算ログ受付
// MH321800(E) T.Nagai ICクレジット対応
			}
		}
	}
	if( LogDatMax[Lno][0] > RECODE_SIZE ){			// SRAMに1面しか持てない？
		// 集計データなど
		lp->kind = kind;// 必ず0にする(kind = 0より)
		log_bakup.f_recover = 10;// 集計データ受付
		while( AppServ_IsLogFlashWriting(Lno) ){	// 書込み終了まで待合わせ
			taskchg( IDLETSKNO );
		}
	}else{
		kind = lp->kind;
		log_bakup.f_recover = 20;// その他データ受付
	}

	memcpy(lp->dat.s1[kind]+lp->wtp, dat, LogDatMax[Lno][0]);
	if( strage == 0){// RAMのみで管理するログの場合
		for(i=0; i<eLOG_TARGET_MAX; i++){
			if(lp->f_unread[i] < cnt){
				lp->f_unread[i]++;// データ追加より未読カウンタ加算
			}
			else {
				// SRAMのみの場合はf_unreadが最大数の状態で書き込まれた場合はバッファフル
				lp->writeFullFlag[i] = 1;	// データフルフラグON
			}
		}
	}
	log_bakup.f_recover = 2;// RAM転送完了
	lp->count[kind]++;
	log_bakup.f_recover = 3;// RAMカウンタ更新

	for(i=0; i<eLOG_TARGET_MAX; i++){
		lp->writeLogFlag[i] = 1;			// ログ書込みフラグON
	}

	if( lp->count[kind]<cnt ){
		lp->wtp += LogDatMax[Lno][0];
		log_bakup.f_recover = 4;// RAMライトポインタ更新(データFULLでない)
	}
	else{	// full
		if( strage ){
			log_bakup.f_recover = 30;// RAM FULLによりFROM書き込み準備開始
			while( AppServ_IsLogFlashWriting(Lno) ){	// 偽なはず
				taskchg( IDLETSKNO );
			}
			for(i=0; i<eLOG_TARGET_MAX; i++){
				lp->writeFullFlag[i] = 1;	// データフルフラグON
			}
			AppServ_LogFlashWriteReq(Lno);	// FlashROMへ書込み依頼
			log_bakup.f_recover = 31;// FROM書き込み要求完了
			if( LogDatMax[Lno][0]>RECODE_SIZE){
				while( AppServ_IsLogFlashWriting(Lno) ){	// 書き込み中
					taskchg( IDLETSKNO );
				}
				lp->count[kind] = 0;
				lp->wtp = 0;
				log_bakup.f_recover = 311;// ログ書き込み待ち合わせ
			}
			else{
				lp->kind = (kind)? 0: 1;
				log_bakup.f_recover = 321;// SRAM面切り替え実施
				lp->count[lp->kind] = 0;
				lp->wtp = 0;
				log_bakup.f_recover = 323;// SRAM面カウンタ更新
			}
		}else{	// SRAMのみ
			lp->count[kind] = cnt;									// RAM最大数まで達したので常にMAXとなる
			log_bakup.f_recover = 40;// RAM FULLによりSRAMエリア更新(SRAMのみで管理しているログ)
			lp->wtp += LogDatMax[Lno][0];							// ライトポインタをログサイズ分進める
			log_bakup.f_recover = 41;// RAMライトポインタ更新
			bp = (struct log_record *)( LogDatMax[Lno][0] * cnt );	// ログ最後尾のポインタを求める
			if( lp->wtp >= (ulong)bp ){								// ライトポインタがログエリアを越えた
				log_bakup.f_recover = 42;// RAMライトポインタオーバーより元に戻す開始
				lp->wtp = 0;										// ライトポインタを先頭に戻す
				log_bakup.f_recover = 43;// RAMライトポインタオーバーより元に戻す終了
			}else{
				log_bakup.f_recover = 44;// RAMライトポインタオーバーより元に戻す(保持ログ数≧最大ログ数)
				wcnt = lp->wtp / LogDatMax[Lno][0];					// 先頭からの書込み数を求める
				for(i=0; i<eLOG_TARGET_MAX; i++){
					if( lp->unread[i] < wcnt ){						// 未読レコード番号を超えた(未読レコードが上書きされた)
						if(lp->f_unread[i] >= cnt){					// 未読データが満杯の場合更新
							lp->unread[i] = wcnt;					// 未読レコード番号を最古のログとする
						}
					}
				}
			}
		}
	}
	Log_clear_log_bakupflag();// 処理終了リカバリ用データクリア
}
//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ登録復電処理
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/05/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Log_Write_Pon(void)
{
	int		i;
	ushort	cnt;
	struct log_record	*lp;
	struct	log_record	*bp;
	ushort	wcnt;
	short	kind = 0;
	Receipt_data	*pay_p;

	if(log_bakup.f_recover == 0){// リカバリ不要
		Log_clear_log_bakupflag();
		return;
	}
	lp = &LOG_DAT[log_bakup.Lno];
	cnt = LOG_SECORNUM(log_bakup.Lno);
	// 以下一文の処理はLog_Write()の「処理開始」の部分で行なうものであるが、それ以降の処理
	// でも使用するので必ずセットする必要がある。
	kind = log_bakup.stat_kind;
	if(log_bakup.Lno == eLOG_TTOTAL){// T集計
		if(ac_flg.syusyu == 14){
			ac_flg.syusyu = 15;// Ｔ合計ログ登録受付
		}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	}else if(log_bakup.Lno == eLOG_LCKTTL){// 車室毎集計
//		if(ac_flg.syusyu == 15){
//			ac_flg.syusyu = 19;// 車室毎集計ログ(31車室以降)登録受付
//		}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	}else if(log_bakup.Lno == eLOG_GTTOTAL){// GT集計
		if(ac_flg.syusyu == 24){
			ac_flg.syusyu = 25;// ＧＴ合計ログ登録受付
		}
	}else if(log_bakup.Lno == eLOG_COINBOX){// コイン金庫集計ログ
		if(ac_flg.syusyu == 34){
			ac_flg.syusyu = 35;// コイン金庫集計ログ受付
		}
	}else if(log_bakup.Lno == eLOG_MONEYMANAGE){// 金銭管理ログ(NT-NETデータ用)
		if(ac_flg.syusyu == 35){// コイン金庫集計時
			ac_flg.syusyu = 36;// 金銭管理ログログ受付
		}else if(ac_flg.syusyu == 45){// 紙幣金庫集計時
			ac_flg.syusyu = 46;// 金銭管理ログログ受付
		}else if(ac_flg.syusyu == 52){// 金銭管理集計時
			ac_flg.syusyu = 53;// 金銭管理ログログ受付
		}else if(ac_flg.cycl_fg == 54){// 精算中止完了時
			ac_flg.cycl_fg = 0;
		}else if(ac_flg.cycl_fg == 17){// 精算完了時(17:個別精算ﾃﾞｰﾀ登録完了)
			ac_flg.cycl_fg = 0;
		}else if(ac_flg.cycl_fg == 27){// 精算中止完了時(27:精算中止ﾃﾞｰﾀ登録完了)
			ac_flg.cycl_fg = 0;
		}
	}else if(log_bakup.Lno == eLOG_NOTEBOX){// 紙幣金庫集計ログ
		if(ac_flg.syusyu == 44){
			ac_flg.syusyu = 45;// 紙幣金庫集計ログ受付
		}
	}else if(log_bakup.Lno == eLOG_MNYMNG_SRAM){// 金銭管理ログ(SRAM)
		if(ac_flg.syusyu == 53){
			ac_flg.syusyu = 54;// 金銭管理ログ(SRAM)受付
		}
	}else if(log_bakup.Lno == eLOG_ABNORMAL){// 不正・強制出庫ログ
		if(ac_flg.cycl_fg == 81){
			ac_flg.cycl_fg = 34;// 不正・強制出庫ログ受付
		}
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	}else if((log_bakup.Lno == eLOG_CREUSE)||(log_bakup.Lno == eLOG_iDUSE)){// クレジット精算
//		if(ac_flg.cycl_fg == 80){// クレジット利用明細登録開始
//			ac_flg.cycl_fg = 17;// 精算完了時(17:個別精算ﾃﾞｰﾀ登録完了)
//		}
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
	}else if(log_bakup.Lno == eLOG_PAYMENT){// 精算ログ
		if(ac_flg.cycl_fg == 33){// 不正・強制出庫情報登録
			ac_flg.cycl_fg = 81;// 不正・強制出庫データ(NT-NET精算データ用)
		}else if(ac_flg.cycl_fg == 43){// フラップ上昇、ロック閉タイマ内出庫
			ac_flg.cycl_fg = 45;// フラップ上昇、ロック閉タイマ内出庫(NT-NET精算データ用)
		} else {
			pay_p = (Receipt_data*)log_bakup.dat_p;
			// ここまでくるとPayData_set()のやりなおしができないので
			// 復電印字のためのフラグ印字種別１を（1:復電）とする
			pay_p->WFlag = 1;
			if(ac_flg.cycl_fg == 15){// 精算完了時
				ac_flg.cycl_fg = 16;// 精算ログ受付
			}else if(ac_flg.cycl_fg == 25){// 精算中止時①
				// cyu_syuu()中の判定によりセットするフラグが変わる
				if( ryo_buf.ryo_flg >= 2 ){// 定期使用
					ac_flg.cycl_fg = 26;// 精算ログ受付
				}else{
					ac_flg.cycl_fg = 28;// 精算ログ受付
				}
			}else if(ac_flg.cycl_fg == 52){// 精算中止時②
				// toty_syu()中の判定によりセットするフラグが変わる
				if( ryo_buf.ryo_flg >= 2 ){// 定期使用
					ac_flg.cycl_fg = 53;// 精算ログ受付
				}else{
					ac_flg.cycl_fg = 55;// 精算ログ受付
				}
// MH321800(S) T.Nagai ICクレジット対応
			}else if(ac_flg.cycl_fg == 57){		// 決済精算中止時
				ac_flg.cycl_fg = 58;			// 精算ログ受付
// MH321800(E) T.Nagai ICクレジット対応
			}
		}
	}
	switch(log_bakup.f_recover){
		case 1:// 処理開始
			if( LogDatMax[log_bakup.Lno][0] > RECODE_SIZE ){// SRAMに1面しか持てない？
				// 集計データのみ
				log_bakup.stat_kind = 0;
				log_bakup.f_recover = 10;// 集計データ受付
			}else{
				kind = log_bakup.stat_kind;
				log_bakup.f_recover = 20;// その他データ受付
			}
		case 10:// 集計データ受付
		case 20:// その他データ受付
			memcpy(lp->dat.s1[kind]+lp->wtp, log_bakup.dat_p, LogDatMax[log_bakup.Lno][0]);
			if( log_bakup.strage == 0){// RAMのみで管理するログの場合
				for(i=0; i<eLOG_TARGET_MAX; i++){
					if(lp->f_unread[i] < cnt){
						if(log_bakup.f_unread[i] == lp->f_unread[i]){
							// 値が同じならカウントアップ直前で停電したので更新する
							lp->f_unread[i]++;// データ追加より未読カウンタ加算
						}
					}
					else {
						// SRAMのみの場合はf_unreadが最大数の状態で書き込まれた場合はバッファフル
						lp->writeFullFlag[i] = 1;	// データフルフラグON
					}
				}
			}
			log_bakup.f_recover = 2;// RAM転送完了
		case 2:// RAM転送完了
			if(log_bakup.stat_count == lp->count[kind]){
				// 値が同じならカウントアップ直前で停電したので更新する
				lp->count[kind]++;
			}
			log_bakup.f_recover = 3;// RAMカウンタ更新
			break;
		case 3:// RAMカウンタ更新
			goto point4;
		case 30:// RAM FULLによりFROM書き込み準備開始
			goto point5;
		case 31:// FROM書き込み要求完了
			goto point6;
		case 321:// SRAM面切り替え実施
			goto point7;
		case 40:// RAM FULLによりSRAMエリア更新(SRAMのみで管理しているログ)
			goto point8;
		case 41:// RAMライトポインタ更新
			goto point9;
		case 42:// RAMライトポインタオーバーより元に戻す開始
			goto point10;
		case 44:// RAMライトポインタオーバーより元に戻す(保持ログ数≧最大ログ数)
			goto point11;
		case 4:// RAMライトポインタ更新(データFULLでない)
		case 311:// ログ書き込み待ち合わせ
		case 323:// SRAM面カウンタ更新
		case 43:// RAMライトポインタオーバーより元に戻す終了
		default:// リカバリ不要とみなす
			goto point_end;
	}
point4:
	for(i=0; i<eLOG_TARGET_MAX; i++){
		lp->writeLogFlag[i] = 1;				// ログ書込みフラグON
	}
	if( lp->count[kind]<cnt ){
		if(log_bakup.stat_wtp == lp->wtp){
			// 値が同じならカウントアップ直前で停電したので更新する
			lp->wtp += LogDatMax[log_bakup.Lno][0];
		}
		log_bakup.f_recover = 4;// RAMライトポインタ更新(データFULLでない)
	}
	else{			// full
		if( log_bakup.strage ){
			log_bakup.f_recover = 30;// RAM FULLによりFROM書き込み準備開始
point5:
			if( FLT_Check_LogWriteReq(log_bakup.Lno) == FALSE){// 停電前の要求はない
				for(i=0; i<eLOG_TARGET_MAX; i++){
					lp->writeFullFlag[i] = 1;	// データフルフラグON
				}
				// 停電前のAppServ_LogFlashWriteReq()→FLT_WriteLog()内での要求が完了していない
				// ため最初からやりなおす
				FLT_Force_ResetLogWriteReq(log_bakup.Lno);// 書き込み要求前に停電前の状態を強制的にリセットする
				AppServ_LogFlashWriteReq(log_bakup.Lno);	// FlashROMへ書込み依頼
				// 要求が完了している場合は、別で実施するFROMのリカバー処理で実行される
			}
			log_bakup.f_recover = 31;// FROM書き込み要求完了
point6:
			if( LogDatMax[log_bakup.Lno][0]>RECODE_SIZE){
				// 通常処理では書き込み完了の待ち合わせをしているが、復電処理では
				// AppServ_LogFlashWriteReq()内でタスク切り替えしないで書き込むので不要
				lp->count[kind] = 0;
				lp->wtp = 0;
				log_bakup.f_recover = 311;// ログ書き込み待ち合わせ
			}
			else{
				if(log_bakup.stat_kind == lp->kind){
					// 値が同じなら更新直前で停電したので更新する
					lp->kind = (kind)? 0: 1;
				}
				log_bakup.f_recover = 321;// SRAM面切り替え実施
point7:
				lp->count[lp->kind] = 0;
				lp->wtp = 0;
				log_bakup.f_recover = 323;// SRAM面カウンタ更新
			}
		}else{	// SRAMのみ
			lp->count[kind] = cnt;									// RAM最大数まで達したので常にMAXとなる
			log_bakup.f_recover = 40;// RAM FULLによりSRAMエリア更新(SRAMのみで管理しているログ)
point8:
			if(log_bakup.stat_wtp == lp->wtp){
				// 値が同じならカウントアップ直前で停電したので更新する
				lp->wtp += LogDatMax[log_bakup.Lno][0];// ライトポインタをログサイズ分進める
			}
			log_bakup.f_recover = 41;// RAMライトポインタ更新
point9:
			bp = (struct log_record *)( LogDatMax[log_bakup.Lno][0] * cnt );// ログ最後尾のポインタを求める
			if( lp->wtp >= (ulong)bp ){								// ライトポインタがログエリアを越えた
				log_bakup.f_recover = 42;// RAMライトポインタオーバーより元に戻す開始
point10:
				lp->wtp = 0;										// ライトポインタを先頭に戻す
				log_bakup.f_recover = 43;// RAMライトポインタオーバーより元に戻す終了
			}else{
				log_bakup.f_recover = 44;// RAMライトポインタオーバーより元に戻す(保持ログ数≧最大ログ数)
point11:
				wcnt = lp->wtp / LogDatMax[log_bakup.Lno][0];		// 先頭からの書込み数を求める
				for(i=0; i<eLOG_TARGET_MAX; i++){
					if( lp->unread[i] < wcnt ){						// 未読レコード番号を超えた(未読レコードが上書きされた)
						if(lp->f_unread[i] >= cnt){					// 未読データが満杯の場合更新
							lp->unread[i] = wcnt;					// 未読レコード番号を最古のログとする
						}
					}
				}
			}
		}
	}
point_end:
	Log_clear_log_bakupflag();// 処理終了リカバリ用データクリア
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ登録復電処理
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/05/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Log_clear_log_bakupflag(void)
{
	int i;
// LOGデータ登録復電処理用のフラグ、退避データクリア処理
	log_bakup.f_recover = 0;// 処理完了
	log_bakup.Lno = 0;
	log_bakup.strage = 0;
	log_bakup.dat_p = 0;
	log_bakup.stat_kind = 0;
	log_bakup.stat_wtp = 0;
	log_bakup.stat_count = 0;
	for(i=0; i<eLOG_TARGET_MAX; i++){
		log_bakup.f_unread[i] = 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| String Table                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2005.04.12                                               |*/
/*| UpDate      : font_change 2005-09-01 T.Hashimoto                       |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _STR_DEF_H_
#define _STR_DEF_H_

extern	const unsigned char	FUNMSG[][31];		/* Function Key */
extern	const unsigned char	FUNMSG2[][31];		/* Function Key */
extern	const unsigned char	SLMENU[][31];		/* Maintenance Top Menu */
extern	const unsigned char	UMMENU[][27];		/* User Maintenance Top Menu */
extern	const unsigned char	SMMENU[][31];		/* System Maintenance Top Menu */
extern	const unsigned char	PRMENU[][31];		/* Parameter Setting Menu */
extern	const unsigned char	BPRMENU[][31];		/* Basic Parameter Setting Menu */
extern	const unsigned char	CPRMENU[][31];		/* Cell Parameter Setting Menu */
extern	const unsigned char	RPRMENU[][31];		/* Rock Parameter Setting Menu */
extern	const unsigned char	FCMENU[][27];		/* Function Check Menu */
extern	const unsigned char	MNT_MENU_NUMBER[][5];	/* Menu */
extern	const unsigned char	FLCDMENU[][31];
extern	const unsigned char	FSIGMENU[][31];		/* Signal check Menu */
extern	const unsigned char	FPRNMENU1[][31];	/* PRN check Menu */
extern	const unsigned char	FPRNMENU2[][31];	/* PRN check Menu */
extern	const unsigned char	FPRNMENU3[][31];	/* PRN check Menu */
// MH810104 GG119201(S) 電子ジャーナル対応
extern	const unsigned char	FPRNMENU5[][31];	/* PRN check Menu */
// MH810104 GG119201(E) 電子ジャーナル対応
extern	const unsigned char	FRDMENU[][31];		/* MgRdr check Menu */
extern	const unsigned char	FCMCMENU[][31];		/* CMC check Menu */
extern	const unsigned char	MNYMENU[][31];		/* MNY ctl Menu */
extern	const unsigned char	FBNAMENU[][31];		/* BNA check Menu */
extern	const unsigned char	LOGMENU[][31];		/* ログファイル Menu */
extern	const unsigned char	MNYSETMENU[][31];	/* Money setting Menu */
extern	const unsigned char	CARDMENU[][31];		/* Card issue Menu */
extern	const unsigned char	FCNTMENUCOUNT[][29];		/* 動作カウント Menu２ */
extern	const unsigned char	LOGPRNMENU[][31];	/* 履歴情報プリント Menu */
extern	const unsigned char	EXTENDMENU_CRE[][31];	/* 拡張機能-クレジット処理 Menu */
extern	const unsigned char	EXTENDMENU_BASE[][29];	/* 拡張設定 Menu */
extern	const unsigned char	MENU_NUMBER[][3];		/* メニュー番号 */
extern	const unsigned char MIFAREMENU[][31];	/* Mifare check Menu */
extern	const unsigned char LABELSETMENU[][31];	/* Label Printer setting Menu */
extern	const unsigned char CCOMMENU[][31];
extern	const unsigned char CCOMMENU2[][31];
extern	const unsigned char	EXTENDMENU_CRE_CAPPI[][31];	/* 拡張機能-クレジット処理 Menu(Cappi) */

extern	const unsigned char PIPMENU[][31];
extern	const unsigned char CREMENU[][31];
// MH810100(S) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)
extern	const unsigned char QRMENU[][31];
extern  const unsigned char REALTI_MENU[][31];
extern  const unsigned char DC_NET_MENU[][31];
extern	const unsigned char QR_STR[][31];
extern	const unsigned char QR_TITLE[][31];
extern	const unsigned char REAL_TITLE[][31];
extern	const unsigned char REAL_STR[][31];
extern	const unsigned char REAL_UNSEND_TITLE[][9];
extern	const unsigned char REAL_UNSEND_STR[][31];
extern	const unsigned char DCNET_UNSEND_STR[][31];
extern	const unsigned char DC_UNSEND_TITLE[][9];
extern	const unsigned char QR_DATA_CHECK_TITLE[][31];
extern	const unsigned char QR_DATA_CHECK_ERR[][31];
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// extern	const unsigned char QR_DISCOUNT_DATA_CHECK_STR[][31];
// extern	const unsigned char QR_AMOUNT_DATA_CHECK_STR[][31];
extern	const unsigned char QR_DISCOUNT_DATA_CHECK_STR[10][31];
extern	const unsigned char QR_AMOUNT_DATA_CHECK_STR[12][31];
extern	const unsigned char	QR_ID_STR[][9];
extern	const unsigned char	DISC_KIND_STR[][21];
extern	const unsigned char	DISCOUNT_TYPE_STR[][19];
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
extern	const unsigned char QR_TICKET_DATA_CHECK_STR[4][31];
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
// MH810100(E) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)
extern	const unsigned char LOCKMENU[][29];	/* ロック装置開閉 Menu1 */
extern	const unsigned char KIND_3_MENU[][29];	/* 車種選択 Menu */
extern	const unsigned char KIND_2_MENU[][29];/* 車種選択 Menu */
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	const unsigned char	EXTENDMENU_EDY[][31];	/* 拡張機能 Ｅｄｙ処理 Menu */
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

extern	const unsigned char	EXTENDMENU_SCA[][31];	/* 拡張機能 Ｓｕｉｃａ処理 Menu */
extern	const unsigned char	SUICA_MENU[][31];
extern	const unsigned char	SUICA_MENU2[][31];

extern	const unsigned char	UMSTR1[][31];		/* User Maintenance Tytle & Contents */
extern	const unsigned char	UMSTR2[][31];		/* User Maintenance Tytle & Contents */
extern	const unsigned char	SMSTR1[][31];		/* System Maintenance Tytle & Contents */
extern	const unsigned char	SMSTR2[][31];		/* System Maintenance Tytle & Contents */
extern	const unsigned char	WKNO[][6];
extern	const unsigned char	PMSTR1[][31];
extern	const unsigned char	FCSTR1[][31];		/* Function Check Tytle & Contents */
extern	const unsigned char	FSWSTR[][31];
extern	const unsigned char	LEDSTR[][31];
extern	const unsigned char	ANNSTR[][31];
extern	const unsigned char	MCKSTR[][31];
extern	const unsigned char	RDRSTR[][31];
extern	const unsigned char	RDESTR[][31];
extern	const unsigned char	PRTSTR[][31];
extern	const unsigned char	CNTSTR[][31];
extern	const unsigned char	CMCSTR[][31];
extern	const unsigned char	CMESTR[][31];
extern	const unsigned char	TGTSTR[][31];
extern	const unsigned char	MNCSTR[][31];
extern	const unsigned char	BNASTR[][31];
extern	const unsigned char	BNESTR[][31];
extern	const unsigned char	LOGSTR1[][31];
extern	const unsigned char	LOGSTR2[][31];
extern	const unsigned char	LOGSTR3[][31];
extern	const unsigned char	CLRLCD_STR[][31];
extern	const unsigned char	CLRLCD_LUMINSTR[][31];
extern	const unsigned char	SHTSTR[][31];
extern	const unsigned char	SIGSTR1[][31];
// MH810100(S) S.Takahashi 2019/12/18
//extern	const unsigned char	SIGSTR2[][3];
extern	const unsigned char	SIGSTR2[][4];
// MH810100(E) S.Takahashi 2019/12/18
extern	const unsigned char	UPDNMN[][31];
extern	const unsigned char	UPLOADMN[][31];
extern	const unsigned char	DWLOADMN[][31];
extern	const char Syomei_Data_dflt[4][36];
extern	const unsigned char	BKRSMN[][31];
extern	const unsigned char	MSETSTR[][31];
extern	const unsigned char	UMSTR3[][31];
extern	const unsigned char	UMSTR4[][11];
extern	const unsigned char	UMSTR5[][11];
extern	const unsigned char	UMSTR6[][31];
extern	const unsigned char	IFCSTR[][31];
extern	const unsigned char	LBSETSTR[][31];
extern	const unsigned char	CCOMSTR[][31];
extern	const unsigned char	CCOMERR[][31];
extern	const unsigned char	CCOMSTR1[][9];
extern	const unsigned char	CCOMSTR2[][19];
extern	const unsigned char	CCOMSTR3[][31];
extern	const unsigned char	CCOMSTR4[][31];
extern	const unsigned char	CCOMSTR5[][9];
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
extern	const unsigned char	DCSTR5[][9];
extern	const unsigned char	REALSTR5[][9];
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
extern	const unsigned char	CCOMSTR6[][31];
extern	const unsigned char	CCOMSTR7[][3];
extern	const unsigned char	WEKFFNT[][3];		/* Week */
extern	const unsigned char	BASDAT[][31];		/* Time & Date Frame */
extern	const unsigned char	DAT1_0[][3];		/* 1 chara string */
extern	const unsigned char	DAT1_1[][3];		/* 1 chara string */
extern	const unsigned char	DAT1_2[][3];		/* 1 chara string */
extern	const unsigned char	DAT1_5[][3];		/* 1 chara string */
extern	const unsigned char	DAT1_6[][3];		/* 1 chara string */
extern	const unsigned char	DAT2_0[][5];		/* 2 chara string */
extern	const unsigned char	DAT2_1[][5];		/* 2 chara string */
extern	const unsigned char	DAT2_2[][5];		/* 2 chara string */
extern	const unsigned char	DAT2_3[][5];		/* 2 chara string */
extern	const unsigned char	DAT2_5[][5];		/* 2 chara string */
extern	const unsigned char	DAT3_SPC[][7];		/* 3 space string */
extern	const unsigned char	DAT3_0[][7];		/* 3 chara string */
extern	const unsigned char	DAT3_1[][7];		/* 3 chara string */
extern	const unsigned char	DAT3_2[][7];		/* 3 chara string */
extern	const unsigned char	DAT4_0[][9];		/* 4 chara string */
extern	const unsigned char	DAT4_1[][9];		/* 4 chara string */
extern	const unsigned char	DAT4_3[][9];		/* 4 chara string */
extern	const unsigned char	DAT4_4[][9];		/* 4 chara string */
extern	const unsigned char	DAT4_6[][9];		/* 4 chara string */

extern	const unsigned char	DAT5_0[][15];		/* 5 chara string */
extern	const unsigned char	DAT5_1[][11];		/* 5 chara string */
extern	const unsigned char	DAT6_0[][13];		/* 6 chara string */
extern	const unsigned char	DAT7_0[][15];		/* 7 chara string */
extern	const unsigned char	DAT1_3[][3];		/* 8 chara string */
extern	const unsigned char	DAT1_4[][3];		/* 8 chara string */
extern	const unsigned char	DAT2_6[][5];		/* 2 chara string */
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// extern	const unsigned char	DAT2_7[][5];		/* 2 chara string */
extern	const unsigned char	DAT2_7[27][5];		/* 2 chara string */
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
extern	const unsigned char	DAT3_3[][7];		/* 3 chara string */
extern	const unsigned char	DAT4_2[][9];		/* 4 chara string */
extern	const unsigned char	DAT5_3[][11];		/* 5 chara string */
extern	const unsigned char	DAT5_4[][11];		/* 5 chara string */
extern	const unsigned char	DAT6_1[][13];		/* 6 chara string */
extern	const unsigned char	DAT7_1[][15];		/* 7 chara string */
extern	const unsigned char	DAT7_2[][15];		/* 7 chara string */
extern	const unsigned char	DAT8_1[][17];		/* 8 chara string */
extern	const unsigned char	DAT10_1[][21];		/* 10 chara string */
extern	const unsigned char	HALFDAT1_0[][2];	/* half 1 chara string */
// MH810100(S) K.Onodera  2020/03/25 車番チケットレス(#3976 遠隔精算時に領収証再発行の表示が分かりづらい)
extern	const unsigned char	RECEIPT_DAT1[][5];
// MH810100(E) K.Onodera  2020/03/25 車番チケットレス(#3976 遠隔精算時に領収証再発行の表示が分かりづらい)
extern	const unsigned char	DAT8_0[][17];		/* 2 chara string */

extern	const unsigned char	ALARM_STR[][31];	/* ｴﾗｰ･ｱﾗｰﾑﾃｰﾌﾞﾙ */
extern	const unsigned char	NTNETSTR[][31];
extern	const unsigned char	NTNETSTR1[][31];

extern	const unsigned char	CAPPICHKSTR[][31];	/* Cappi check Menu */
extern	const unsigned char	CAPPICHKMENU[][31];	/* Cappi check Title Menu */
extern	const unsigned char	RYOKINTESTMENU[][31];

// 券ﾃﾞｰﾀ表示用
extern	const unsigned char	CARDSTR1[][17];
extern	const unsigned char	CARDSTR2[][31];
extern	const unsigned char	PASSSTR[][5];
extern	const unsigned char MAFCHKSTR[][31];	/* MAF check Menu */
extern	const unsigned char MAFCHKSTR1[][31];	/* MAF check Menu */
extern	const unsigned char MAFCHKSTR2[][31];	/* MAF check Menu */

extern	const unsigned char	PASSSTR2[][9];
extern	const unsigned char	CARDERRSTR[][31];
extern	const unsigned char	GT_APS_MARK[][4];

extern	const unsigned char	SUICASTR[][31];
extern	const unsigned char	SUICA_STATUS_STR[][5];

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	const unsigned char	EDYSTR[][31];
//extern	const unsigned char	EDY_UITEST_STR[][31];
//extern	const unsigned char	EDY_CARDTEST_STR[][31];
//extern	const unsigned char	EDY_STSPRINT_STR[][31];
//extern	const unsigned char	EDY_DATECHK_STR[][31];
//extern	const unsigned char	EDY_FSTCON_STR[][31];
//extern	const unsigned char	EDY_TAMPERCHK_STR[][31];
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

// ＦＴＰ＆システム切替
extern	const unsigned char	SMSTR3[][31];		/* System Maintenance Tytle & Contents */

extern	const unsigned char		UMUKERPMENU[][31];	/* 駐車証明書メニュー */
extern	const unsigned short	UMUKERPDATA[][4];	/* 駐車証明書データ */

extern	const unsigned char	FUNMSG3[][31];		/* Function Key */

// ファイル転送＆システム切替
extern	const unsigned char	FLTRFMENU[][31];	/* File Transfer Menu */
extern	const unsigned char	SYSSWMENU[][31];	/* System Switch Menu */

// メモリチェック＆ＳＣＩチェック
extern	const unsigned char	MCKSTRMODE[][31];
extern	const unsigned char	SCICHK[][31];
extern	const unsigned char	SCICH_STOP[][3];
extern	const unsigned char	SCIBAUD[][11];
extern	const unsigned char	SCIDATABIT[][3];
extern	const unsigned char	SCIPARITY[][5];

extern	const unsigned char	FNTNETMENU[][31];	// NT-NET check Menu

extern	const unsigned char	CARFAIL[][31];

extern	const unsigned char	UPLOAD_LOGDATA[][31];

extern	const unsigned char	WEEKLYFFNT[][3];	/* 特別曜日処理改修用曜日テーブル*/

extern	const unsigned char	FLPBOARDMENU[][31];
extern	const unsigned char	CRRBOARDMENU[][31];
extern	const unsigned char	CRRBOARDSTR[][31];

extern	const unsigned char	RISMCHKSTR[][31];
extern	const unsigned char	WCARDINF[][31];
extern	const char Header_Data_dflt[4][36];
extern	const char AzukariFooter_Data_dflt[4][36];
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
extern	const char CreKabaraiFooter_Data_dflt[4][36];
extern	const char EpayKabaraiFooter_Data_dflt[4][36];
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
extern	const char FutureFooter_Data_dflt[4][36];
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
extern	const char TCardFooter_Data_dflt[10][36];
extern	const unsigned char	CARFUNC[][31];

extern	const unsigned char	FTPCONNECTMENU[][31];
extern	const unsigned char	FTPCONNECTSTR[][31];
extern	const unsigned char	LANCHKSTR[][31];
extern	const unsigned char	LANTYPESTR[][19];
extern	const unsigned char	CHKPRINTSTR[][31];
extern	const unsigned char MNT_STRCHK_STR[][31];
extern	const unsigned char	FUNOPECHKSTR[][31];
extern	const unsigned char	KEY_DISP_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	CHK_CHR[][31];
extern	const unsigned char	CHK_MOJ[][31];
extern	const unsigned char	PRI_CHK_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	MAG_CHK_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	ERE_CHK_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	COIN_CHK_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	NOTE_CHK_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	HARD_CHK_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	ANN_CHK_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	SHUT_CHK_STR[][31];	/* Function Check Tytle & Contents */
extern	const unsigned char	INOUT_CHK_STR[][16];	/* Function Check Tytle & Contents */
extern	const unsigned char	CHK_CHR_BIG[][20];
extern	const unsigned char	SIG_CHK_STR[][31];
extern	const unsigned char	MOJ_INPUT_STR[][31];
extern	const unsigned char	KEY_DISP_NGSTR[][31];
extern	const unsigned char	MAG_CHK_NGSTR[][31];
extern	const unsigned char	PRI_CHK_NGSTR[][31];
extern	const unsigned char	SHUT_CHK_NGSTR[][31];
extern	const unsigned char	NOTE_CHK_NGSTR[][31];
extern	const unsigned char	COIN_CHK_NGSTR[][31];
extern	const unsigned char	ANN_CHK_NGSTR[][31];
extern	const unsigned char	CHK_NGSTR[][31];
extern	const unsigned char	CHKMODE_STR1[];				// 固定文字列①
extern	const unsigned char	CHKMODE_STR2[];				// 固定文字列②
extern	const unsigned char	PARKING_CERTIFICATE_MENU[][31];
extern	const char	*const P_CERTI_SEL[3];
extern	const unsigned char	SMSTR4[][31];		/* System Maintenance Tytle & Contents */

// MH321800(S) D.Inaba ICクレジット対応
extern	const char PinSyomei_Data_dflt[4][36];      // PIN確認済の際に印字する署名文字
extern	const unsigned char	FCMENU2[][27];		    /* 決済リーダチェックへ変更に伴う新規配列の宣言*/
extern	const unsigned char	ECR_MENU[][31];         // 決済リーダチェック項目
extern	const unsigned char	SMSTR5[][31];		    // 決済リーダチェックで使用する文字
extern	const unsigned char	EXTENDMENU_EC[][31];	/* 拡張機能 決済リーダ接続時の Menu */
extern	const unsigned char	EC_BRAND_STR[][17];     /* 動作チェック 決済リーダ処理メニュー項目 */
extern	const unsigned char	EC_STR[][9];            /* 動作チェック 決済リーダ処理メニュー文字 */
// MH321800(E) D.Inaba ICクレジット対応

// MH810104 GG119201(S) 電子ジャーナル対応
extern	const unsigned char	EXTENDMENU_EJ[][31];	/* 拡張機能 電子ジャーナルの Menu */
// MH810104 GG119201(E) 電子ジャーナル対応

#endif	// _STR_DEF_H_

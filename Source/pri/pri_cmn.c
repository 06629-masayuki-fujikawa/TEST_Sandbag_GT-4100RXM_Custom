/*[]---------------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀｰ制御部：共通処理関数												|*/
/*|																				|*/
/*|	ﾌｧｲﾙ名称	:	Pri_Cmn.c													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author		: K.Motohashi													|*/
/*|	Date		: 2005-07-19													|*/
/*|	UpDate		:																|*/
/*|																				|*/
/*[]--------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"LKmain.h"
#include	"flp_def.h"
#include	"Pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"Ope_def.h"
#include	"I2c_driver.h"
#include	"rtc_readwrite.h"
// MH810104 GG119201(S) 電子ジャーナル対応
#include	"fla_def.h"
// MH810104 GG119201(E) 電子ジャーナル対応

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ初期化処理														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnInit( void )										|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnInit( void )
{

	rct_init_sts	= 0;					// ﾚｼｰﾄﾌﾟﾘﾝﾀ	初期化状態ﾘｾｯﾄ（初期化完了待ち）
	jnl_init_sts	= 0;					// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ	初期化状態ﾘｾｯﾄ（初期化完了待ち）

	Header_Rsts		= 0xffffffff;			// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ読込状態初期化
	Footer_Rsts		= 0xffffffff;			// ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態初期化
	AcceptFooter_Rsts = 0xffffffff;			// 受付券ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態初期化
	Syomei_Rsts		= 0xffffffff;			// 支払ｺｰﾄﾞ・署名欄印字ﾃﾞｰﾀ読込状態
	Kamei_Rsts		= 0xffffffff;			// 加盟店名印字ﾃﾞｰﾀ読込状態
// MH810105(S) MH364301 QRコード決済対応
	EmgFooter_Rsts	= 0xffffffff;			// 障害連絡票ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態初期化
// MH810105(E) MH364301 QRコード決済対応

	rct_timer_end = 0;

	PrnInit_R();							// ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化

	if( PrnJnlCheck() == ON ){				// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続あり？

		//	ｼﾞｬｰﾅﾙ接続あり
// MH810104 GG119201(S) 電子ジャーナル対応
//		PrnInit_J();						// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化
// MH810104 GG119201(E) 電子ジャーナル対応
	}
	else{
		//	ｼﾞｬｰﾅﾙ接続なし
		jnl_init_sts	= 3;				// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（未接続）
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnInit_R( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnInit_R( void )
{
	PCMD_INIT( R_PRI );			// [ﾌﾟﾘﾝﾀの初期化]
// GG129000(S) 富士急ハイランド機能改善（領収証発行時のQR印字が遅くて印字途中で引っ張られる）（GM803003流用）
	PCMD_WRITE_RESET( R_PRI );	// [ページバッファークリア]
// GG129000(E) 富士急ハイランド機能改善（領収証発行時のQR印字が遅くて印字途中で引っ張られる）（GM803003流用）

	PCMD_STATUS_AUTO( R_PRI );	// [ﾌﾟﾘﾝﾀｽﾃｰﾀｽの自動送信]
	PCMD_STATUS_SEND( R_PRI );	// [ﾌﾟﾘﾝﾀｽﾃｰﾀｽの送信要求]

	PrnMode_idle( R_PRI );		// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定

	PrnOut( R_PRI );			// ﾚｼｰﾄﾌﾟﾘﾝﾀへｺﾏﾝﾄﾞ送信
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnInit_J( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnInit_J( void )
{
	PCMD_INIT( J_PRI );			// [ﾌﾟﾘﾝﾀの初期化]

	PCMD_STATUS_AUTO( J_PRI );	// [ﾌﾟﾘﾝﾀｽﾃｰﾀｽの自動送信]
	PCMD_STATUS_SEND( J_PRI );	// [ﾌﾟﾘﾝﾀｽﾃｰﾀｽの送信要求]

	PrnMode_idle( J_PRI );		// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定

	PrnOut( J_PRI );			// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀへｺﾏﾝﾄﾞ送信
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMode_idle( pri_kind )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind	= ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|							RJ_PRI : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnMode_idle( uchar pri_kind )
{

	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:			// ﾚｼｰﾄ
			PrnMode_idle_R();	// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定（ﾚｼｰﾄ）
			break;

		case	J_PRI:			// ｼﾞｬｰﾅﾙ
			PrnMode_idle_J();	// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:			// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnMode_idle_R();	// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定（ﾚｼｰﾄ）
			PrnMode_idle_J();	// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定（ｼﾞｬｰﾅﾙ）
			break;

		default:				// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMode_idle_R( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnMode_idle_R( void )
{
	PrnFontSize( FONT_A_NML, R_PRI );	// ﾌｫﾝﾄＡ指定
	PCMD_R_SPACE( R_PRI );				// [文字の右ｽﾍﾟｰｽ量(0mm)]
	PCMD_SHIFTJIS( R_PRI );				// [漢字ｺｰﾄﾞ体系選択(ｼﾌﾄJIS体系)]
	PCMD_KANJI4_CLR( R_PRI );			// [漢字の４倍角文字(解除)]
	PCMD_FONT_K_N( R_PRI );				// [漢字印字ﾓｰﾄﾞ一括指定(通常)]
	PCMD_SPACE_KANJI( R_PRI );			// [漢字ｽﾍﾟｰｽ量の指定(左0mm、右0mm)]
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMode_idle_J( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnMode_idle_J( void )
{
	PrnFontSize( FONT_A_NML, J_PRI );	// ﾌｫﾝﾄＡ指定
	PCMD_R_SPACE( J_PRI );				// [文字の右ｽﾍﾟｰｽ量(0mm)]
	PCMD_SHIFTJIS( J_PRI );				// [漢字ｺｰﾄﾞ体系選択(ｼﾌﾄJIS体系)]
	PCMD_KANJI4_CLR( J_PRI );			// [漢字の４倍角文字(解除)]
	PCMD_FONT_K_N( J_PRI );				// [漢字印字ﾓｰﾄﾞ一括指定(通常)]
	PCMD_SPACE_KANJI( J_PRI );			// [漢字ｽﾍﾟｰｽ量の指定(左0mm、右0mm)]
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ印字文字ｻｲｽﾞ設定												|*/
/*|																			|*/
/*|	＜処理概要＞															|*/
/*|	ﾊﾟﾗﾒｰﾀで指定された文字種別に対応する[印字ﾓｰﾄﾞ指定]及び[改行量指定]		|*/
/*|	ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞを編集する。												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnFontSize( font, pri_kind )						|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	font= 文字種別								|*/
/*|																			|*/
/*|						uchar	pri_kind=	ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnFontSize( uchar font, uchar pri_kind )
{
	PRN_PROC_DATA	*p_proc_data;	// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	ushort	f_data;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数
	ushort	s_data;					// 行間設定値（ﾄﾞｯﾄ数）
	ushort	kaigyou;				// 改行量ﾄﾞｯﾄ数
	uchar	cmd[3];					// 改行量指定ｺﾏﾝﾄﾞ＜ESC 3 n＞ﾃﾞｰﾀ


	//	編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰ取得
	switch( pri_kind ){	// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// 	ﾚｼｰﾄ

			p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;	// ﾚｼｰﾄ編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ

			if( gyoukan_data != 0 ){
				//	改行幅が指定されている場合（指定ﾃﾞｰﾀに従う）
				s_data = (ushort)gyoukan_data;
			}
			else{
				//	改行幅が指定されていない場合（設定ﾃﾞｰﾀに従う）
				s_data = (ushort)CPrmSS[S_PRN][1];			// 設定ﾃﾞｰﾀ取得（ﾚｼｰﾄ改行幅）
			}

			break;

		case	J_PRI:		// 	ｼﾞｬｰﾅﾙ

			p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;	// ｼﾞｬｰﾅﾙ編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ

			if( gyoukan_data != 0 ){
				//	改行幅が指定されている場合（指定ﾃﾞｰﾀに従う）
				s_data = (ushort)gyoukan_data;
			}
			else{
				//	改行幅が指定されていない場合（設定ﾃﾞｰﾀに従う）
				s_data = (ushort)CPrmSS[S_PRN][2];			// 設定ﾃﾞｰﾀ取得（ｼﾞｬｰﾅﾙ改行幅）
			}

			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	//	印字ﾓｰﾄﾞ設定（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
	switch( font ){		// 印字ﾓｰﾄﾞ（文字種別）？

		//	ﾌｫﾝﾄＡ
		case	FONT_A_NML:			// ﾌｫﾝﾄＡ 通常

			if( p_proc_data->Font_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Font_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_A_N( pri_kind );			// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＡ通常)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_A_N_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_A_TATEBAI:		// ﾌｫﾝﾄＡ 縦倍

			if( p_proc_data->Font_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Font_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_A_T( pri_kind );			// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＡ縦倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_A_T_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_A_YOKOBAI:		// ﾌｫﾝﾄＡ 横倍

			if( p_proc_data->Font_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Font_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_A_Y( pri_kind );			// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＡ横倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_A_Y_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_A_4BAI:		// ﾌｫﾝﾄＡ ４倍

			if( p_proc_data->Font_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Font_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_A_4( pri_kind );			// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＡ４倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_A_4_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		//	ﾌｫﾝﾄＢ
		case	FONT_B_NML:			// ﾌｫﾝﾄＢ 通常

			if( p_proc_data->Font_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Font_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_B_N( pri_kind );			// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＢ通常)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_B_N_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_B_TATEBAI:		// ﾌｫﾝﾄＢ 縦倍

			if( p_proc_data->Font_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Font_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_B_T( pri_kind );			// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＢ縦倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_B_T_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_B_YOKOBAI:		// ﾌｫﾝﾄＢ 横倍

			if( p_proc_data->Font_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Font_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_B_Y( pri_kind );			// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＢ横倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_B_Y_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_B_4BAI:		// ﾌｫﾝﾄＢ ４倍

			if( p_proc_data->Font_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Font_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_B_4( pri_kind );			// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＢ４倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_B_4_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		//	漢字
		case	FONT_K_NML:			// 漢字 通常

			if( p_proc_data->Kanji_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Kanji_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_K_N( pri_kind );			// [漢字印字ﾓｰﾄﾞの一括指定(漢字通常)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_K_N_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_K_TATEBAI:		// 漢字 縦倍

			if( p_proc_data->Kanji_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Kanji_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_K_T( pri_kind );			// [漢字印字ﾓｰﾄﾞの一括指定(漢字縦倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_K_T_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_K_YOKOBAI:		// 漢字 横倍

			if( p_proc_data->Kanji_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Kanji_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_K_Y( pri_kind );			// [漢字印字ﾓｰﾄﾞの一括指定(漢字横倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_K_Y_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		case	FONT_K_4BAI:		// 漢字 ４倍

			if( p_proc_data->Kanji_size != font ){

				//	現在の指定から変更になる場合
				p_proc_data->Kanji_size = font;		// 指定する文字種別をｾｯﾄ
				PCMD_FONT_K_4( pri_kind );			// [漢字印字ﾓｰﾄﾞの一括指定(漢字４倍)]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
			}
			f_data = FONT_K_4_HDOT;					// 指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数ｾｯﾄ
			break;

		default:					// 文字種別ＮＧ
			return;
	}
	//	改行量を求める
	if( (s_data + f_data) > 0xff ){		// 改行量最大値（0xff）ｵｰﾊﾞｰ？
		kaigyou = 0xff;					// YES：改行量最大値（0xff）ｾｯﾄ
	}
	else{
		kaigyou = s_data + f_data;		// NO：改行量（行間設定値＋指定ﾌｫﾝﾄ縦ﾄﾞｯﾄ数）ｾｯﾄ
	}
	if( p_proc_data->Kaigyou_size != kaigyou ){

		//	改行量（幅）が現在の指定から変更になる場合

		p_proc_data->Kaigyou_size = (uchar)kaigyou;	// 指定する改行量（幅）をｾｯﾄ

		cmd[0] = 0x1b;						// "ESC"
		cmd[1] = 0x33;						// "3"
		cmd[2] = (uchar)kaigyou;			// 改行量ﾃﾞｰﾀｾｯﾄ(n)
		PrnCmdLen( cmd , 3 , pri_kind );	// [改行量指定]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ毎編集処理制御ﾃﾞｰﾀ初期化										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_proc_data_clr( pri_kind )						|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind=	ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|							RJ_PRI : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_proc_data_clr( uchar pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ
			prn_proc_data_clr_R();	// 編集処理制御ﾃﾞｰﾀ初期化（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			prn_proc_data_clr_J();	// 編集処理制御ﾃﾞｰﾀ初期化（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			prn_proc_data_clr_R();	// 編集処理制御ﾃﾞｰﾀ初期化（ﾚｼｰﾄ）
			prn_proc_data_clr_J();	// 編集処理制御ﾃﾞｰﾀ初期化（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ編集処理制御ﾃﾞｰﾀ初期化											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_proc_data_clr_R( void )							|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_proc_data_clr_R( void )
{
	uchar	i;

	rct_proc_data.Printing		= 0;		// 印字処理状態
	rct_proc_data.Tyushi_Cmd	= 0;		// 印字中止ﾒｯｾｰｼﾞ受信ﾌﾗｸﾞ
	rct_proc_data.Split 		= 0;		// 印字処理中のﾌﾞﾛｯｸ数
	rct_proc_data.Final 		= 0;		// 分割印字最終ﾌﾞﾛｯｸ通知
	rct_proc_data.EditWait		= 0;		// 印字ﾃﾞｰﾀ編集待ち状態

	rct_proc_data.Prn_no 		= 0;		// 印字処理中の項目番号
	rct_proc_data.Prn_no_data1	= 0;		// 印字処理中の項目番号のｻﾌﾞ情報１
	rct_proc_data.Prn_no_data2	= 0;		// 印字処理中の項目番号のｻﾌﾞ情報２
	for( i=0 ; i<10 ; i++ ){				// 印字処理汎用ﾜｰｸ
		rct_proc_data.Prn_no_wk[i] = 0;
	}

	rct_proc_data.Log_Start		= 0;		// ﾛｸﾞﾃﾞｰﾀ印字開始ﾌﾗｸﾞ
	rct_proc_data.Log_Edit		= 0;		// ﾛｸﾞﾃﾞｰﾀ編集中ﾌﾗｸﾞ
	rct_proc_data.Log_Count		= 0;		// ﾛｸﾞﾃﾞｰﾀ数
	rct_proc_data.Log_DataPt	= 0;		// ﾛｸﾞﾃﾞｰﾀﾎﾟｲﾝﾀ
	rct_proc_data.Log_Sdate		= 0;		// ﾛｸﾞﾃﾞｰﾀ検索開始日
	rct_proc_data.Log_Edate		= 0;		// ﾛｸﾞﾃﾞｰﾀ検索終了日
	rct_proc_data.Log_no_wk[0]	= 0;		// ﾛｸﾞ印字処理汎用ﾜｰｸ
	rct_proc_data.Log_no_wk[1]	= 0;

	rct_proc_data.Font_size		= 0;		// ﾌｫﾝﾄ文字ｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した文字種別を格納）
	rct_proc_data.Kanji_size	= 0;		// 漢字ﾌｫﾝﾄｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した文字種別を格納）
	rct_proc_data.Kaigyou_size	= 0;		// 改行ｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した改行量（幅）をﾄﾞｯﾄ数単位で格納）

}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙ編集処理制御ﾃﾞｰﾀ初期化										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_proc_data_clr_J( void )							|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_proc_data_clr_J( void )
{
	uchar	i;

	jnl_proc_data.Printing		= 0;		// 印字処理状態
	jnl_proc_data.Tyushi_Cmd	= 0;		// 印字中止ﾒｯｾｰｼﾞ受信ﾌﾗｸﾞ
	jnl_proc_data.Split 		= 0;		// 印字処理中のﾌﾞﾛｯｸ数
	jnl_proc_data.Final 		= 0;		// 分割印字最終ﾌﾞﾛｯｸ通知
	jnl_proc_data.EditWait		= 0;		// 印字ﾃﾞｰﾀ編集待ち状態

	jnl_proc_data.Prn_no 		= 0;		// 印字処理中の項目番号
	jnl_proc_data.Prn_no_data1	= 0;		// 印字処理中の項目番号のｻﾌﾞ情報１
	jnl_proc_data.Prn_no_data2	= 0;		// 印字処理中の項目番号のｻﾌﾞ情報２
	for( i=0 ; i<10 ; i++ ){				// 印字処理汎用ﾜｰｸ
		jnl_proc_data.Prn_no_wk[i] = 0;
	}

	jnl_proc_data.Log_Start		= 0;		// ﾛｸﾞﾃﾞｰﾀ印字開始ﾌﾗｸﾞ
	jnl_proc_data.Log_Edit		= 0;		// ﾛｸﾞﾃﾞｰﾀ編集中ﾌﾗｸﾞ
	jnl_proc_data.Log_Count		= 0;		// ﾛｸﾞﾃﾞｰﾀ数
	jnl_proc_data.Log_DataPt	= 0;		// ﾛｸﾞﾃﾞｰﾀﾎﾟｲﾝﾀ
	jnl_proc_data.Log_Sdate		= 0;		// ﾛｸﾞﾃﾞｰﾀ検索開始日
	jnl_proc_data.Log_Edate		= 0;		// ﾛｸﾞﾃﾞｰﾀ検索終了日
	jnl_proc_data.Log_no_wk[0]	= 0;		// ﾛｸﾞ印字処理汎用ﾜｰｸ
	jnl_proc_data.Log_no_wk[1]	= 0;

	jnl_proc_data.Font_size		= 0;		// ﾌｫﾝﾄ文字ｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した文字種別を格納）
	jnl_proc_data.Kanji_size	= 0;		// 漢字ﾌｫﾝﾄｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した文字種別を格納）
	jnl_proc_data.Kaigyou_size	= 0;		// 改行ｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した改行量（幅）をﾄﾞｯﾄ数単位で格納）

// MH810104 GG119201(S) 電子ジャーナル対応
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//	if (isEJA_USE()) {
	{
		jnl_proc_data.Prn_Job_id = 0;
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		// 印字制御データと印字メッセージをクリアする
		memset(&eja_prn_buff.eja_proc_data, 0, sizeof(eja_prn_buff.eja_proc_data));
		memset(&eja_prn_buff.PrnMsg, 0, sizeof(eja_prn_buff.PrnMsg));
	}
// MH810104 GG119201(E) 電子ジャーナル対応
}

/*[]-----------------------------------------------------------------------[]*/
/*|		編集処理共通ﾜｰｸｴﾘｱ初期化											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_edit_wk_clr( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_edit_wk_clr( void )
{

	gyoukan_data	= 0;
	syuukei_kind	= 0;
	prn_zero		= 0;

	memset( prn_work,0,sizeof(prn_work) );
	memset( cTmp1,0,sizeof(cTmp1) );
	memset( cTmp2,0,sizeof(cTmp2) );
	memset( cMnydata,0,sizeof(cMnydata) );
	memset( cEditwk1,0,sizeof(cEditwk1) );
	memset( cEditwk2,0,sizeof(cEditwk2) );
	memset( cEditwk3,0,sizeof(cEditwk3) );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字中止ﾒｯｾｰｼﾞ受信ﾁｪｯｸ												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_Cancel_chk( pri_kind )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind=	ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI : ﾚｼｰﾄ									|*/
/*|							J_PRI : ｼﾞｬｰﾅﾙ									|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							YES : 印字中止ﾒｯｾｰｼﾞ受信あり					|*/
/*|							NO  : 印字中止ﾒｯｾｰｼﾞ受信なし					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	Inji_Cancel_chk( uchar pri_kind )
{
	PRN_PROC_DATA	*p_proc_data;	// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	uchar			ret = NO;		// 戻り値

	if( next_prn_msg == ON ){	// 印字処理中の次編集要求？

		switch( pri_kind ){		// 対象ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ
				p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ
				p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return( ret );
		}
		if( p_proc_data->Tyushi_Cmd == ON ){		// 印字中止ﾒｯｾｰｼﾞ受信済み？

			//	印字中止ﾒｯｾｰｼﾞを受信していた場合
			ret = YES;								// 戻り値に「印字中止」をｾｯﾄ
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字中止処理														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_Cancel( *msg, pri_kind )						|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|						uchar	pri_kind=	ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|							RJ_PRI : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_Cancel( MSG *msg, uchar pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ
			Inji_Cancel_R( msg );	// 印字中止処理（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			Inji_Cancel_J( msg );	// 印字中止処理（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			Inji_Cancel_R( msg );	// 印字中止処理（ﾚｼｰﾄ）
			Inji_Cancel_J( msg );	// 印字中止処理（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ印字中止処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_Cancel_R( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_Cancel_R( MSG *msg )
{
	prn_proc_data_clr_R();			// ﾚｼｰﾄ印字編集制御ｴﾘｱ初期化
	PriBothPrint = 0;
	PrnEndMsg(	msg->command,		// 「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信
				PRI_CSL_END,
				PRI_ERR_NON,
				R_PRI );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙ印字中止処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_Cancel_J( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_Cancel_J( MSG *msg )
{
	prn_proc_data_clr_J();			// ｼﾞｬｰﾅﾙ印字編集制御ｴﾘｱ初期化
	if(msg->command == BothPrintCommand){
		PriBothPrint = 0;
		BothPrintCommand = 0;
	}
// MH810104 GG119201(S) 電子ジャーナル対応
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//	if (isEJA_USE()) {
	{
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		// データ書込み終了を通知する
		PrnCmd_WriteStartEnd(1, NULL);
	}
// MH810104 GG119201(E) 電子ジャーナル対応
	PrnEndMsg(	msg->command,		// 「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信
				PRI_CSL_END,
				PRI_ERR_NON,
				J_PRI );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字異常終了処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_ErrEnd( command, pri_sts, pri_kind )			|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	command	=	印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ				|*/
/*|																			|*/
/*|						uchar	pri_sts	=	異常終了理由					|*/
/*|																			|*/
/*|						uchar	pri_kind=	ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|							RJ_PRI : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_ErrEnd( ushort command, uchar pri_sts, uchar pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ
			Inji_ErrEnd_R( command, pri_sts );	// 印字異常終了処理（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			Inji_ErrEnd_J( command, pri_sts );	// 印字異常終了処理（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			Inji_ErrEnd_R( command, pri_sts );	// 印字異常終了処理（ﾚｼｰﾄ）
			Inji_ErrEnd_J( command, pri_sts );	// 印字異常終了処理（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ印字異常終了処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_ErrEnd_R( command, pri_sts )					|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	command	=	印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ				|*/
/*|																			|*/
/*|						uchar	pri_sts	=	異常終了理由					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_ErrEnd_R( ushort command, uchar pri_sts )
{
	prn_proc_data_clr_R();			// ﾚｼｰﾄ印字編集制御ｴﾘｱ初期化
	I2cSendErrDataInit(I2C_DN_RP);
	Lagcan( PRNTCBNO, 7 );
	rct_timer_end = 0;
	PriBothPrint = 0;

	PrnEndMsg(						/*-----	印字終了ﾒｯｾｰｼﾞ送信	-----*/
				command,			// 印字要求ｺﾏﾝﾄﾞ：入力ﾊﾟﾗﾒｰﾀ
				PRI_ERR_END,		// 印字処理結果	：異常終了
				pri_sts,			// 異常終了理由	：入力ﾊﾟﾗﾒｰﾀ
				R_PRI				// ﾌﾟﾘﾝﾀ種別	：ﾚｼｰﾄ
			);
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙ印字異常終了処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_ErrEnd_J( command, pri_sts )					|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	command	=	印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ				|*/
/*|																			|*/
/*|						uchar	pri_sts	=	異常終了理由					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_ErrEnd_J( ushort command, uchar pri_sts )
{
	prn_proc_data_clr_J();			// ｼﾞｬｰﾅﾙ印字編集制御ｴﾘｱ初期化
	I2cSendErrDataInit(I2C_DN_JP);
	if( command == BothPrintCommand ){
		PriBothPrint = 0;
		BothPrintCommand = 0;
	}

	PrnEndMsg(						/*-----	印字終了ﾒｯｾｰｼﾞ送信	-----*/
				command,			// 印字要求ｺﾏﾝﾄﾞ：入力ﾊﾟﾗﾒｰﾀ
				PRI_ERR_END,		// 印字処理結果	：異常終了
				pri_sts,			// 異常終了理由	：入力ﾊﾟﾗﾒｰﾀ
				J_PRI				// ﾌﾟﾘﾝﾀ種別	：ｼﾞｬｰﾅﾙ
			);
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	End_Set( *msg, pri_kind )							|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|						uchar	pri_kind=	ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|							RJ_PRI : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	End_Set( MSG *msg, uchar pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ
			End_Set_R( msg );		// ﾚｼｰﾄ印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			End_Set_J( msg );		// ｼﾞｬｰﾅﾙ印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			End_Set_R( msg );		// ﾚｼｰﾄ印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理
			End_Set_J( msg );		// ｼﾞｬｰﾅﾙ印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	End_Set_R( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	End_Set_R( MSG *msg )
{
	MsgBuf	*OutMsg;		// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ

	if( rct_proc_data.Final == ON ){	// 最終ﾌﾞﾛｯｸ編集終了？

		//	最終ﾌﾞﾛｯｸ編集終了の場合

		if( msg->command != PREQ_LOGO_REGIST ){
										// ロゴ印字データ登録終了の場合は、ﾚｼｰﾄ先頭印字処理をしない
			Rct_top_edit( ON );			// ﾚｼｰﾄ先頭印字ﾃﾞｰﾀ編集処理（ﾛｺﾞ印字／ﾍｯﾀﾞｰ印字／用紙ｶｯﾄ制御）
		}
		PrnOut( R_PRI );				// ﾌﾟﾘﾝﾀ起動（ﾚｼｰﾄﾌﾟﾘﾝﾀ送信割込みEnable）

		PrnEndMsg(	msg->command,		// 「印字終了（正常終了）」ﾒｯｾｰｼﾞ送信
					PRI_NML_END,
					PRI_ERR_NON,
					R_PRI );
		if(PriBothPrint == 1) {
			if( (OutMsg = GetBuf()) == NULL ){						// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ取得
				return;												// 取得ＮＧ
			}
			memcpy(&OutMsg->msg, msg, sizeof(MSG));					// コマンドデータをコピー
			OutMsg->msg.data[0] = J_PRI;							// 印字対象をジャーナルにする
			PutMsg( PRNTCBNO, OutMsg );								// ﾌﾟﾘﾝﾀﾀｽｸへ印字要求（次ﾌﾞﾛｯｸ編集要求）ﾒｯｾｰｼﾞ送信
			PriBothPrint = 2;
			BothPrintCommand = msg->command;
		}
	}
	else{
		//	最終ﾌﾞﾛｯｸでない場合（次編集ﾌﾞﾛｯｸあり）
		PrnOut( R_PRI );				// ﾌﾟﾘﾝﾀ起動（ﾚｼｰﾄﾌﾟﾘﾝﾀ送信割込みEnable）
		memcpy( &NextMsg_r, msg, sizeof(NextMsg_r) );
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙ印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	End_Set_J( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	End_Set_J( MSG *msg )
{

// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//// MH810104 GG119201(S) 電子ジャーナル対応
//	if (isEJA_USE()) {
//		// データ書込み終了を通知する
//		PrnCmd_WriteStartEnd(1, NULL);
//	}
//// MH810104 GG119201(E) 電子ジャーナル対応
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	if( jnl_proc_data.Final == ON ){	// 最終ﾌﾞﾛｯｸ編集終了？

		//	最終ﾌﾞﾛｯｸ編集終了の場合
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		// 最終ブロック編集終了時にデータ書込み終了を通知する
		PrnCmd_WriteStartEnd(1, NULL);
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

		PrnOut( J_PRI );				// ﾌﾟﾘﾝﾀ起動（ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ送信割込みEnable）

		PrnEndMsg(	msg->command,		// 「印字終了（正常終了）」ﾒｯｾｰｼﾞ送信
					PRI_NML_END,
					PRI_ERR_NON,
					J_PRI );
		if(msg->command == BothPrintCommand){
			PriBothPrint = 0;
			BothPrintCommand = 0;
		}
	}
	else{
		//	最終ﾌﾞﾛｯｸでない場合（次編集ﾌﾞﾛｯｸあり）
		PrnOut( J_PRI );				// ﾌﾟﾘﾝﾀ起動（ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ送信割込みEnable）
		memcpy( &NextMsg_j, msg, sizeof(NextMsg_j) );
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字終了ﾒｯｾｰｼﾞ送信処理（to ｵﾍﾟﾚｰｼｮﾝﾀｽｸ）							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnEndMsg( comm, result, stat, pri_kind )			|*/
/*|																			|*/
/*| PARAMETER		:	ushort	comm	= 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ				|*/
/*|																			|*/
/*|						uchar	result	= 印字処理結果						|*/
/*|																			|*/
/*|						uchar	stat	= 異常終了理由						|*/
/*|																			|*/
/*|						uchar	pri_kind = ﾌﾟﾘﾝﾀ種別						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-02													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnEndMsg(	ushort	comm,
					uchar	result,
					uchar	stat,
					uchar	pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ

			PrnEndMsg_R( comm, result, stat );		// 印字終了ﾒｯｾｰｼﾞ送信処理（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			PrnEndMsg_J( comm, result, stat );		// 印字終了ﾒｯｾｰｼﾞ送信処理（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			PrnEndMsg_R( comm, result, stat );		// 印字終了ﾒｯｾｰｼﾞ送信処理（ﾚｼｰﾄ）
			PrnEndMsg_J( comm, result, stat );		// 印字終了ﾒｯｾｰｼﾞ送信処理（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ印字終了ﾒｯｾｰｼﾞ送信処理（to ｵﾍﾟﾚｰｼｮﾝﾀｽｸ）						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnEndMsg_R( comm, result, stat )					|*/
/*|																			|*/
/*| PARAMETER		:	ushort	comm	= 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ				|*/
/*|																			|*/
/*|						uchar	result	= 印字処理結果						|*/
/*|																			|*/
/*|						uchar	stat	= 異常終了理由						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-02													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnEndMsg_R(ushort	comm,
					uchar	result,
					uchar	stat )
{
	MsgBuf		*Msg;		// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
	T_FrmEnd	data;		// 印字終了ﾒｯｾｰｼﾞﾃﾞｰﾀﾊﾞｯﾌｧ

	if( (Msg = GetBuf()) == NULL ){				// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ取得
		return;									// 取得ＮＧ
	}
	Msg->msg.command = comm|INNJI_ENDMASK;		// 送信ｺﾏﾝﾄﾞ：印字終了ﾒｯｾｰｼﾞﾌﾗｸﾞｾｯﾄ

	/*-----		送信ﾃﾞｰﾀ作成	-----*/
	data.BMode		= result;
	data.BStat		= stat;
	data.BPrinStat	= (uchar)(rct_prn_buff.PrnState[0] & 0x0f);
	data.BPrikind	= R_PRI;

	memcpy(	&Msg->msg.data,						// 送信ﾃﾞｰﾀｾｯﾄ
			&data,
			sizeof(T_FrmEnd) );

	PutMsg( OPETCBNO, Msg );					// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへ印字終了ﾒｯｾｰｼﾞ送信
// MH322914 (s) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
	if( comm == PREQ_RYOUSYUU ) {
		ryo_inji = 0;							// 領収証印字終了
	}
// MH322914 (e) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙ印字終了ﾒｯｾｰｼﾞ送信処理（to ｵﾍﾟﾚｰｼｮﾝﾀｽｸ）						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnEndMsg_J( comm, result, stat )					|*/
/*|																			|*/
/*| PARAMETER		:	ushort	comm	= 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ				|*/
/*|																			|*/
/*|						uchar	result	= 印字処理結果						|*/
/*|																			|*/
/*|						uchar	stat	= 異常終了理由						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-02													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnEndMsg_J(ushort	comm,
					uchar	result,
					uchar	stat )
{
	MsgBuf		*Msg;		// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
	T_FrmEnd	data;		// 印字終了ﾒｯｾｰｼﾞﾃﾞｰﾀﾊﾞｯﾌｧ

	if( (Msg = GetBuf()) == NULL ){				// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ取得
		return;									// 取得ＮＧ
	}
	Msg->msg.command = comm|INNJI_ENDMASK;		// 送信ｺﾏﾝﾄﾞ：印字終了ﾒｯｾｰｼﾞﾌﾗｸﾞｾｯﾄ

	/*-----		送信ﾃﾞｰﾀ作成	-----*/
	data.BMode		= result;
	data.BStat		= stat;
	data.BPrinStat	= (uchar)(jnl_prn_buff.PrnState[0] & 0x0f);
	data.BPrikind	= J_PRI;

	memcpy(	&Msg->msg.data,						// 送信ﾃﾞｰﾀｾｯﾄ
			 &data,
			sizeof(T_FrmEnd) );

	PutMsg( OPETCBNO, Msg );					// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへ印字終了ﾒｯｾｰｼﾞ送信
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnNext( *Msg, pri_kind )							|*/
/*|																			|*/
/*| PARAMETER		:	MSG		*msg		= 送信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|						uchar	pri_kind	= ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnNext( MSG *Msg, uchar pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:			// ﾚｼｰﾄ
			PrnNext_R( Msg );	// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理（ﾚｼｰﾄ）
			break;

		case	J_PRI:			// ｼﾞｬｰﾅﾙ
			PrnNext_J( Msg );	// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:			// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnNext_R( Msg );	// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理（ﾚｼｰﾄ）
			PrnNext_J( Msg );	// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnNext_R( *Msg )									|*/
/*|																			|*/
/*| PARAMETER		:	MSG		*Msg		= 送信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnNext_R( MSG *Msg )
{
	MsgBuf	*OutMsg;		// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
	uchar	pri_data;		// ﾌﾟﾘﾝﾀ種別ﾃﾞｰﾀ作成ﾜｰｸ

	if( (OutMsg = GetBuf()) == NULL ){						// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ取得
		return;												// 取得ＮＧ
	}
	OutMsg->msg.command = Msg->command | INNJI_NEXTMASK;	// 送信ｺﾏﾝﾄﾞ：印字処理中ﾒｯｾｰｼﾞﾌﾗｸﾞ（INNJI_NEXTMASK）ｾｯﾄ

	memcpy(	OutMsg->msg.data,								// 送信ﾃﾞｰﾀｾｯﾄ
			Msg->data,
			sizeof(OutMsg->msg.data) );

	//	ﾌﾟﾘﾝﾀ種別ﾃﾞｰﾀ作成（ｵﾘｼﾞﾅﾙのﾌﾟﾘﾝﾀ種別ﾃﾞｰﾀ上位４ﾋﾞｯﾄにﾌﾟﾘﾝﾀ種別をｾｯﾄ）
	pri_data = R_PRI << 4;
	pri_data |= Msg->data[0];
	OutMsg->msg.data[0] = pri_data;

	PutMsg( PRNTCBNO, OutMsg );								// ﾌﾟﾘﾝﾀﾀｽｸへ印字要求（次ﾌﾞﾛｯｸ編集要求）ﾒｯｾｰｼﾞ送信
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙ印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnNext_J( *Msg )									|*/
/*|																			|*/
/*| PARAMETER		:	MSG		*Msg		= 送信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnNext_J( MSG *Msg )
{
	MsgBuf	*OutMsg;		// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
	uchar	pri_data;		// ﾌﾟﾘﾝﾀ種別ﾃﾞｰﾀ作成ﾜｰｸ

	if( (OutMsg = GetBuf()) == NULL ){						// 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ取得
		return;												// 取得ＮＧ
	}
	OutMsg->msg.command = Msg->command | INNJI_NEXTMASK;	// 送信ｺﾏﾝﾄﾞ：印字処理中ﾒｯｾｰｼﾞﾌﾗｸﾞ（INNJI_NEXTMASK）ｾｯﾄ

	memcpy(	OutMsg->msg.data,								// 送信ﾃﾞｰﾀｾｯﾄ
			Msg->data,
			sizeof(OutMsg->msg.data) );

	//	ﾌﾟﾘﾝﾀ種別ﾃﾞｰﾀ作成（ｵﾘｼﾞﾅﾙのﾌﾟﾘﾝﾀ種別ﾃﾞｰﾀ上位４ﾋﾞｯﾄにﾌﾟﾘﾝﾀ種別をｾｯﾄ）
	pri_data = J_PRI << 4;
	pri_data |= Msg->data[0];
	OutMsg->msg.data[0] = pri_data;

	PutMsg( PRNTCBNO, OutMsg );								// ﾌﾟﾘﾝﾀﾀｽｸへ印字要求（次ﾌﾞﾛｯｸ編集要求）ﾒｯｾｰｼﾞ送信
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀﾀｽｸへのﾒｯｾｰｼﾞ送信												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	MsgSndFrmPrn( cmd ,data1, data2 )					|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	cmd 	= 送信ｺﾏﾝﾄﾞ							|*/
/*|						uchar	data1	= 送信ﾃﾞｰﾀ1							|*/
/*|						uchar	data2	= 送信ﾃﾞｰﾀ2							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	MsgSndFrmPrn( ushort cmd , uchar data1, uchar data2 )
{
	MsgBuf *Msg;

	if( (Msg = GetBuf()) != NULL )
	{
		Msg->msg.command = cmd;
		Msg->msg.data[0] = data1;
		Msg->msg.data[1] = data2;
		PutMsg( PRNTCBNO , Msg );
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続有無ﾁｪｯｸ												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnJnlCheck( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							ON 	: 接続あり									|*/
/*|							OFF : 接続なし									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-17													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnJnlCheck( void )
{
	uchar	ret;

	if( prm_get( COM_PRM,S_PAY,21,1,1 ) == 0 ){		// 「ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続」設定ﾃﾞｰﾀ？
		ret = OFF;	//	接続なし
	}
	else{
		ret = ON;	//	接続あり
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		合計記録印字要求の判定												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnGoukeiChk( command )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort command	: 印字要求ｺﾏﾝﾄﾞ						|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							YES	: 合計記録									|*/
/*|							NO	: 合計記録以外								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-19													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnGoukeiChk( ushort command )
{
	uchar	ret;	// 戻り値

	if(	command == PREQ_TGOUKEI		||	// 「Ｔ合計」			印字要求
		command == PREQ_GTGOUKEI	||	// 「ＧＴ合計」			印字要求
		command == PREQ_MTGOUKEI	||	// 「ＭＴ合計」			印字要求
		command == PREQ_COKINKO_G	||	// 「コイン金庫合計」	印字要求
		command == PREQ_SIKINKO_G ){	// 「紙幣金庫合計」		印字要求

		ret = YES;
	}
	else{
		ret = NO;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		合計記録の印字先ﾌﾟﾘﾝﾀ検索											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnGoukeiPri( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							0 		  : 印字なし							|*/
/*|							R_PRI(1)  : レシート							|*/
/*|							J_PRI(2)  : ジャーナル							|*/
/*|							RJ_PRI(3) : レシート＆ジャーナル				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-19													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnGoukeiPri( ushort cmd )
{
	uchar	settei;		// 「合計記録の印字先」設定ﾃﾞｰﾀ
	uchar	ret;		// 戻り値（印字先ﾌﾟﾘﾝﾀ種別）

	settei = 0;
	if( prm_get( COM_PRM, S_TOT, 17, 1, 6 ) == 0){						// 合計記録は全て１の位に従う
		settei = (uchar)prm_get( COM_PRM, S_TOT, 17, 1, 1 );			// 「Ｔ合計／ＧＴ合計の印字先」設定ﾃﾞｰﾀ取得
	}else{
		//出力先は個別設定に従う
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		if(( cmd == PREQ_TGOUKEI ) || (cmd == PREQ_GTGOUKEI) || (cmd == PREQ_MTGOUKEI) || (cmd == PREQ_EDY_SHIME_R) ){
		if(( cmd == PREQ_TGOUKEI ) || (cmd == PREQ_GTGOUKEI) || (cmd == PREQ_MTGOUKEI) ){
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			settei = (uchar)prm_get( COM_PRM, S_TOT, 17, 1, 4 );		// 「Ｔ合計／ＧＴ合計の印字先」設定ﾃﾞｰﾀ取得
		} else if(( cmd == PREQ_COKINKO_G) || (cmd == PREQ_SIKINKO_G)){
			settei = (uchar)prm_get( COM_PRM, S_TOT, 17, 1, 2 );		// 「紙幣金庫合計／コイン金庫合計の印字先」設定ﾃﾞｰﾀ取得
		} else if( cmd == PREQ_TURIKAN){
			settei = (uchar)prm_get( COM_PRM, S_TOT, 17, 1, 3 );		// 「金銭管理合計の印字先」設定ﾃﾞｰﾀ取得
		}
	}
	switch( settei ){	// 合計記録の印字先設定？

		case	0:		// 印字先＝なし

			ret = 0;										// 印字先＝なし
			break;

		case	1:		// 印字先＝ジャーナルのみ印字

			if( PrnJnlCheck() == ON ){						// ｼﾞｬｰﾅﾙ接続？
				// ｼﾞｬｰﾅﾙ接続あり
				ret = J_PRI;								// 印字先＝ｼﾞｬｰﾅﾙ
			}
			else{
				// ｼﾞｬｰﾅﾙ接続なし
				ret = 0;									// 印字先＝なし
			}
			break;

		case	2:		// 印字先＝レシートのみ印字

			ret = R_PRI;									// 印字先＝ﾚｼｰﾄ
			break;

		case	3:		// 印字先＝ジャーナル＆レシート

			if( PrnJnlCheck() == ON ){						// ｼﾞｬｰﾅﾙ接続？
				// ｼﾞｬｰﾅﾙ接続あり
				ret = RJ_PRI;								// 印字先＝ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			}
			else{
				// ｼﾞｬｰﾅﾙ接続なし
				ret = R_PRI;								// 印字先＝ﾚｼｰﾄ
			}
			break;

		default:		// その他（設定エラー）
			ret = 0;										// 印字先＝なし
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		自動集計の印字先ﾌﾟﾘﾝﾀ検索											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAsyuukeiPri( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							0 		  : 印字なし							|*/
/*|							R_PRI(1)  : レシート							|*/
/*|							J_PRI(2)  : ジャーナル							|*/
/*|							RJ_PRI(3) : レシート＆ジャーナル				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnAsyuukeiPri( void )
{
	uchar	settei;		// 「自動集計の印字先」設定ﾃﾞｰﾀ
	uchar	ret;		// 戻り値（印字先ﾌﾟﾘﾝﾀ種別）

	settei = (uchar)prm_get( COM_PRM, S_TOT, 8, 1, 1 );		// 「自動集計の印字先」設定ﾃﾞｰﾀ取得

	switch( settei ){	// 自動集計の印字先設定？

		case	0:		// 印字先＝なし（自動集計なし）
		case	3:		// 印字先＝なし（自動集計あり）

			ret = 0;										// 印字先＝なし
			break;

		case	1:		// 印字先＝ジャーナルのみ印字

			if( PrnJnlCheck() == ON ){						// ｼﾞｬｰﾅﾙ接続？
				// ｼﾞｬｰﾅﾙ接続あり
				ret = J_PRI;								// 印字先＝ｼﾞｬｰﾅﾙ
			}
			else{
				// ｼﾞｬｰﾅﾙ接続なし
				ret = 0;									// 印字先＝なし
			}
			break;

		case	2:		// 印字先＝ジャーナル＆レシート

			if( PrnJnlCheck() == ON ){						// ｼﾞｬｰﾅﾙ接続？
				// ｼﾞｬｰﾅﾙ接続あり
				ret = RJ_PRI;								// 印字先＝ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			}
			else{
				// ｼﾞｬｰﾅﾙ接続なし
				ret = R_PRI;								// 印字先＝ﾚｼｰﾄ
			}
			break;

		default:		// その他（設定エラー）
			ret = 0;										// 印字先＝なし
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ状態（印字可／不可）ﾁｪｯｸ										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PriStsCheck( pri_kind )								|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind	= ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI : ﾚｼｰﾄ									|*/
/*|							J_PRI : ｼﾞｬｰﾅﾙ									|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							PRI_ERR_NON	 : 印字可							|*/
/*|							PRI_ERR_STAT : 印字不可（ﾌﾟﾘﾝﾀ状態異常）		|*/
/*|							PRI_ERR_BUSY : 印字不可（ﾌﾟﾘﾝﾀBUSY）			|*/
/*|							PRI_ERR_ANY  : 印字不可（その他）				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PriStsCheck( uchar pri_kind )
{
	uchar			ret = PRI_ERR_NON;	// 戻り値
	PRN_DATA_BUFF	*prn_buff;			// ﾌﾟﾘﾝﾀ送受信管理ﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	uchar			pri_sts;			// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ

	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ

			prn_buff = &rct_prn_buff;

			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
	
			prn_buff = &jnl_prn_buff;
// MH810104 GG119201(S) 電子ジャーナル対応（電子ジャーナル未接続時にレシート印字できない）
			if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_UNCONNECTED) ||
				IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL)) {
				// 電子ジャーナル未接続、通信不良時は印字不可とする
				ret = PRI_ERR_STAT;
			}
// MH810104 GG119201(E) 電子ジャーナル対応（電子ジャーナル未接続時にレシート印字できない）

			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）

			ret = PRI_ERR_ANY;												// 印字不可（その他）
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}
	if( ret == PRI_ERR_NON ){

		//	ﾌﾟﾘﾝﾀ種別＆BUSYﾁｪｯｸがＯＫな場合

		pri_sts = (uchar)(prn_buff->PrnState[0] & 0x0e);					// ﾌﾟﾘﾝﾀ現在ｽﾃｰﾀｽ取得（ﾆｱｴﾝﾄﾞｽﾃｰﾀｽは除く）

		if( pri_sts ){														// 印字可能？
			ret = PRI_ERR_STAT;												// 印字不可（ﾌﾟﾘﾝﾀ状態異常）
		}
// MH810104 GG119201(S) 電子ジャーナル対応（電子ジャーナル未接続時にレシート印字できない）
//// MH810104 GG119201(S) 電子ジャーナル対応
//		if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WAKEUP_ERR)) {
//			// 電子ジャーナル起動失敗時は印字不可とする
//			ret = PRI_ERR_STAT;
//		}
//// MH810104 GG119201(E) 電子ジャーナル対応
// MH810104 GG119201(E) 電子ジャーナル対応（電子ジャーナル未接続時にレシート印字できない）
// MH810104 GG119201(S) 電子ジャーナル対応
		else if (isEJA_USE() && (prn_buff->PrnState[0] & 0x40) != 0) {
			ret = PRI_ERR_WAIT_INIT;						// 印字不可（初期化完了待ち）
		}
// MH810104 GG119201(E) 電子ジャーナル対応
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ送信ﾃﾞｰﾀﾊﾞｯﾌｧ状態ﾁｪｯｸ											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PriOutCheck( pri_kind )								|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind	= ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI : ﾚｼｰﾄ									|*/
/*|							J_PRI : ｼﾞｬｰﾅﾙ									|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 編集開始OK									|*/
/*|							NG : 編集開始NG									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PriOutCheck( uchar pri_kind )
{
	ushort	WriteCnt;		// 送信ﾊﾞｯﾌｧ登録数
	ushort	ReadCnt;		// 送信ﾊﾞｯﾌｧ読込数
	ushort	len;			// 送信ﾊﾞｯﾌｧ数空き数

	switch(pri_kind){
		case	R_PRI:	// ﾚｼｰﾄ
			WriteCnt = rct_prn_buff.PrnBufWriteCnt;
			ReadCnt  = rct_prn_buff.PrnBufReadCnt;
			break;

		case	J_PRI:	// ｼﾞｬｰﾅﾙ
			WriteCnt = jnl_prn_buff.PrnBufWriteCnt;
			ReadCnt  = jnl_prn_buff.PrnBufReadCnt;
			break;

		default:		// その他（ﾊﾟﾗﾒｰﾀNG）
			return( NG );
	}

	if( WriteCnt == PRNQUE_CNT && ReadCnt == 0 ){
		return( OK );
	}

	if( WriteCnt >= ReadCnt )
	{
		len =  ( PRNQUE_CNT - WriteCnt ) + ReadCnt;
		if( len >= 32 ){
			return( OK );	//OK
		}
		else{
			return( NG );	//NG
		}
	}
	else
	{
		len =  ReadCnt - WriteCnt;
		if( len >= 32 ){
			return( OK );	//OK
		}
		else{
			return( NG ); 	//NG
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀへのｺﾏﾝﾄﾞ編集処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmdLen( *dat, len, pri_kind )					|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*data	= ｺﾏﾝﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|						ushort	len		= ｺﾏﾝﾄﾞﾃﾞｰﾀﾚﾝｸﾞｽ					|*/
/*|						uchar	pri_kind=	ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|							RJ_PRI : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmdLen( const uchar *dat, ushort len, uchar pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ
			PrnCmdLen_R( dat, len );	// ﾌﾟﾘﾝﾀへｺﾏﾝﾄﾞ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			PrnCmdLen_J( dat, len );	// ﾌﾟﾘﾝﾀへｺﾏﾝﾄﾞ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnCmdLen_R( dat, len );	// ﾌﾟﾘﾝﾀへｺﾏﾝﾄﾞ編集（ﾚｼｰﾄ）
			PrnCmdLen_J( dat, len );	// ﾌﾟﾘﾝﾀへｺﾏﾝﾄﾞ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| レシートプリンタ送信処理                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RP_I2CSndReq                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 								                           |*/
/*| Date         : 				                                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RP_I2CSndReq( int type )
{
	ushort			exec = 0;
	I2C_REQUEST 	request;
	MsgBuf 			*Msg;
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
	ushort len;
	ushort useBuf;		//使用バッファ数
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）

	switch( type ){
		case	0:
			if( !rct_prn_buff.PrnBufCnt ){
				if( rct_prn_buff.PrnBufWriteCnt != rct_prn_buff.PrnBufReadCnt ){
					exec = 1;
				}
			}
			break;
		case	1:
			rct_prn_buff.PrnBufCnt = 0;
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
//			rct_prn_buff.PrnBufReadCnt++;
			len = rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufReadCnt];
			//使用したバッファ数(何本か)算出
			useBuf = len / PRNBUF_SIZE;
			if( len % PRNBUF_SIZE ){
				useBuf++;		//切り上げ
			}
			rct_prn_buff.PrnBufReadCnt += useBuf;
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
			Lagcan( PRNTCBNO, 1 );
			if( rct_prn_buff.PrnBufReadCnt >= PRNQUE_CNT ){
				rct_prn_buff.PrnBufReadCnt = 0;
			}
			if( (rct_prn_buff.PrnBufWriteCnt != rct_prn_buff.PrnBufReadCnt) && !(rct_prn_buff.PrnState[0] & 0x0e)){
				exec = 1;
			}else{
				rct_prn_buff.PrnBufWriteCnt = 0;
				rct_prn_buff.PrnBufReadCnt = 0;
				if( rct_init_sts == 0 ){							// 初期化完了待ち？
					if( (Msg = GetBuf()) != NULL ){
						Msg->msg.command = PREQ_INIT_END_R;			// 初期化ｺﾏﾝﾄﾞ終了通知送信
						PutMsg( OPETCBNO , Msg );
					}
				}else{
					MsgSndFrmPrn(	PREQ_INNJI_END,					// 終了コマンド送信
									R_PRI,
									rct_prn_buff.PrnState[0] );
				}
			}
			break;
		case	2:
			if( !rct_prn_buff.PrnBufCnt ){
				MsgSndFrmPrn(	PREQ_INNJI_END,						// 終了コマンド送信
								R_PRI,
								rct_prn_buff.PrnState[0] );
			}
			break;
	}
	
	if( exec && !jnl_prn_buff.PrnBufCnt ){
		_ei();
		request.TaskNoTo	 			= I2C_TASK_PRN;					// 
		request.DeviceNo 				= I2C_DN_RP;					// RPへの要求
		request.RequestCode    			= I2C_RC_RP_SND;				// 
		request.I2cReqDataInfo.RWCnt	= rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufReadCnt];	//
		request.I2cReqDataInfo.pRWData	= rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufReadCnt];			//
		rct_prn_buff.PrnBusyCnt 		= 0;							// 送信待ちカウント初期化
		rct_prn_buff.PrnBufCnt 			= 1;							// 送信中のフラグとして使用
		Lagtim( PRNTCBNO, 1, 50 );										// 初回の待ちは１秒
		I2C_Request( &request, EXE_MODE_QUEUE );						// 
		rct_timer_end = 1;
		Lagtim( PRNTCBNO, 7, 50*2 );									// レシートプリンタ印字完了後タイマーセット(２秒)
	}
	// とりあえずNGでも何もしない
}

/*[]----------------------------------------------------------------------[]*/
/*| ジャーナルプリンタ送信処理                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : JP_I2CSndReq                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 								                           |*/
/*| Date         : 				                                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	JP_I2CSndReq( int type )
{
	ushort					exec = 0;
	I2C_REQUEST 			request;
	MsgBuf *Msg;
// MH810104 GG119201(S) 電子ジャーナル対応
	ushort	rp;
// MH810104 GG119201(E) 電子ジャーナル対応

	switch( type ){
		case	0:
			if( !jnl_prn_buff.PrnBufCnt ){
				if( jnl_prn_buff.PrnBufWriteCnt != jnl_prn_buff.PrnBufReadCnt ){
					exec = 1;
				}
			}
			break;
		case	1:
			jnl_prn_buff.PrnBufCnt = 0;
			jnl_prn_buff.PrnBufReadCnt++;
			Lagcan( PRNTCBNO, 2 );
			if( jnl_prn_buff.PrnBufReadCnt >= PRNQUE_CNT ){
				jnl_prn_buff.PrnBufReadCnt = 0;
			}
			if( (jnl_prn_buff.PrnBufWriteCnt != jnl_prn_buff.PrnBufReadCnt ) && !(jnl_prn_buff.PrnState[0] & 0x0e)){
				exec = 1;
			}else{
// MH810104 GG119201(S) 電子ジャーナル対応
				// リードポインタ
				rp = jnl_prn_buff.PrnBufReadCnt;
				if (rp == 0) {
					rp = PRNQUE_CNT - 1;
				}
				else {
					rp--;
				}
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了を待たずにデータ送信してしまう）
//				if (isEJA_USE()) {
//					jnl_prn_buff.PrnBufWriteCnt = 0;
//					jnl_prn_buff.PrnBufReadCnt = 0;
//					if (!PrnCmd_CheckSendData(&jnl_prn_buff.PrnBuf[rp][0])) {
//						// 電子ジャーナル用送信データは印字終了を通知しない
//						if (jnl_init_sts != 0) {							// 初期化完了待ち以外
//							MsgSndFrmPrn(	PREQ_INNJI_END,					// 終了コマンド送信
//											J_PRI,
//											jnl_prn_buff.PrnState[0] );
//						}
//					}
//				}
//				else {
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了を待たずにデータ送信してしまう）
// MH810104 GG119201(E) 電子ジャーナル対応
				jnl_prn_buff.PrnBufWriteCnt = 0;
				jnl_prn_buff.PrnBufReadCnt = 0;
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了を待たずにデータ送信してしまう）
				if (isEJA_USE()) {
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//					// 電子ジャーナル接続時は印字データ送信完了イベントを通知しない
//					// データ書込み完了を待つ
					if (jnl_proc_data.Final != 0) {
						// 電子ジャーナルの場合は最終ブロックの印字終了通知は送信しない
						// EJA基板のデータ書込み完了を待つ
						break;
					}
					if (PrnCmd_CheckSendData(&jnl_prn_buff.PrnBuf[rp][0])) {
						// 電子ジャーナル用データ（機器情報要求等）の送信で
						// 印字終了通知を送信しない
						break;
					}
					MsgSndFrmPrn(	PREQ_INNJI_END,					// 終了コマンド送信
									J_PRI,
									jnl_prn_buff.PrnState[0] );
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
					break;
				}
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了を待たずにデータ送信してしまう）
				if( jnl_init_sts == 0 ){							// 初期化完了待ち？
// MH810104 GG119201(S) 電子ジャーナル対応
					if (!PrnCmd_CheckSendData(&jnl_prn_buff.PrnBuf[rp][0])) {
// MH810104 GG119201(E) 電子ジャーナル対応
					if( (Msg = GetBuf()) != NULL ){
						Msg->msg.command = PREQ_INIT_END_J;			// 初期化ｺﾏﾝﾄﾞ終了通知送信
						PutMsg( OPETCBNO , Msg );
					}
// MH810104 GG119201(S) 電子ジャーナル対応
					}
// MH810104 GG119201(E) 電子ジャーナル対応
				}else{
					MsgSndFrmPrn(	PREQ_INNJI_END,					// 終了コマンド送信
									J_PRI,
									jnl_prn_buff.PrnState[0] );
				}
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了を待たずにデータ送信してしまう）
//// MH810104 GG119201(S) 電子ジャーナル対応
//				}
//// MH810104 GG119201(E) 電子ジャーナル対応
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了を待たずにデータ送信してしまう）
			}
			break;
		case	2:
// MH810104 GG119201(S) 電子ジャーナル対応
			if( jnl_init_sts == 0 ){								// 初期化完了待ち？
				break;
			}
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
			if (isEJA_USE()) {
				// 電子ジャーナル接続時はI2C_NEXT_SND_REQ（印字完了）時の
				// PREQ_INNJI_ENDのみ通知する
				break;
			}
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
			if( !jnl_prn_buff.PrnBufCnt ){
				MsgSndFrmPrn(	PREQ_INNJI_END,						// 終了コマンド送信
								J_PRI,
								jnl_prn_buff.PrnState[0] );
			}
			break;
	}
	if( exec && !rct_prn_buff.PrnBufCnt ){
		_ei();
		request.TaskNoTo	 			= I2C_TASK_PRN;					// 
		request.DeviceNo 				= I2C_DN_JP;					// JPへの要求
		request.RequestCode    			= I2C_RC_JP_SND;				// 
		request.I2cReqDataInfo.RWCnt	= jnl_prn_buff.PrnBufCntLen[jnl_prn_buff.PrnBufReadCnt];					// 
		request.I2cReqDataInfo.pRWData	= jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufReadCnt];		// 
		jnl_prn_buff.PrnBusyCnt = 0;									// 送信中のフラグとして使用
		jnl_prn_buff.PrnBufCnt = 1;										// 送信待ちカウント初期化
		Lagtim( PRNTCBNO, 2, 50 );										// 初回の待ちは１秒
		I2C_Request( &request, EXE_MODE_QUEUE );						// 
	}
	// とりあえずNGでも何もしない
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄﾌﾟﾘﾝﾀへのｺﾏﾝﾄﾞ編集処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmdLen_R( *dat , len )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*data	= ｺﾏﾝﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|						ushort	len		= ｺﾏﾝﾄﾞﾃﾞｰﾀﾚﾝｸﾞｽ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmdLen_R(const uchar *dat , ushort len)
{
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
	int useBuf;		//使用バッファ数
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
//	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// 送信ﾃﾞｰﾀﾊﾞｯﾌｧに送信ﾃﾞｰﾀｺﾋﾟｰ
//			dat,
//			(size_t)len );
//	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt] = len;			// 送信ﾃﾞｰﾀﾚﾝｸﾞｽｾｯﾄ
//	rct_prn_buff.PrnBufWriteCnt++;											// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数＋１
//	
//	if( rct_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ空きあり？
//
//		// 空きなしの場合、ｴﾗｰ処理を行う
//
//		rct_prn_buff.PrnBufWriteCnt = 0;
//		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰｴﾗｰ登録
//	}
	//使用するバッファ数(何本か)算出
	useBuf = len / PRNBUF_SIZE;
	if( len % PRNBUF_SIZE ){
		useBuf++;		//切り上げ
	}
	
	if( ( rct_prn_buff.PrnBufWriteCnt + useBuf ) > PRNQUE_CNT ){	// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ空きあり？

		// 空きなしの場合、ｴﾗｰ処理を行う
		rct_prn_buff.PrnBufWriteCnt = 0;

		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰｴﾗｰ登録
	}
	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// 送信ﾃﾞｰﾀﾊﾞｯﾌｧに送信ﾃﾞｰﾀｺﾋﾟｰ
			dat,
			(size_t)len );
	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt] = len;			// 送信ﾃﾞｰﾀﾚﾝｸﾞｽｾｯﾄ
	rct_prn_buff.PrnBufWriteCnt += useBuf;						// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数＋使用バッファ数
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
	RP_I2CSndReq( I2C_PRI_REQ );

}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀへのｺﾏﾝﾄﾞ編集処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmdLen_J( *dat , len )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*data	= ｺﾏﾝﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|						ushort	len		= ｺﾏﾝﾄﾞﾃﾞｰﾀﾚﾝｸﾞｽ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmdLen_J(const uchar *dat , ushort len)
{
	if( PrnJnlCheck() != ON ){		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続あり？
		return;						// 接続なし
	}
	memcpy(	jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufWriteCnt],				// 送信ﾃﾞｰﾀﾊﾞｯﾌｧに送信ﾃﾞｰﾀｺﾋﾟｰ
			dat,
			(size_t)len );
	jnl_prn_buff.PrnBufCntLen[jnl_prn_buff.PrnBufWriteCnt] = len;			// 送信ﾃﾞｰﾀﾚﾝｸﾞｽｾｯﾄ
	jnl_prn_buff.PrnBufWriteCnt++;											// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数＋１
	if( jnl_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ空きあり？

		// 空きなしの場合、ｴﾗｰ処理を行う

		jnl_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰｴﾗｰ登録
	}
	JP_I2CSndReq( I2C_PRI_REQ );
	
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄﾌﾟﾘﾝﾀへの文字列ﾃﾞｰﾀ編集処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmd_R( *dat )									|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= 文字列ﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmd_R( char *dat )
{
	ushort len;

	len = (ushort)strlen(dat);

	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// 送信ﾃﾞｰﾀﾊﾞｯﾌｧに送信ﾃﾞｰﾀｺﾋﾟｰ
			dat,
			(size_t)len );

	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt]=len;				// 送信ﾃﾞｰﾀﾚﾝｸﾞｽｾｯﾄ
	rct_prn_buff.PrnBufWriteCnt++;											// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数＋１
	
	if( rct_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ空きあり？

		// 空きなしの場合、ｴﾗｰ処理を行う

		rct_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰｴﾗｰ登録
	}
	RP_I2CSndReq( I2C_PRI_REQ );
	
}
/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀへの文字列ﾃﾞｰﾀ編集処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmd_J( *dat )									|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= 文字列ﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmd_J( char *dat )
{
	ushort len;
	
	len = (ushort)strlen(dat);

	if( PrnJnlCheck() != ON ){		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続あり？
		return;						// 接続なし
	}

	memcpy(	jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufWriteCnt],				// 送信ﾃﾞｰﾀﾊﾞｯﾌｧに送信ﾃﾞｰﾀｺﾋﾟｰ
			dat,
			(size_t)len );

	jnl_prn_buff.PrnBufCntLen[jnl_prn_buff.PrnBufWriteCnt]=len;				// 送信ﾃﾞｰﾀﾚﾝｸﾞｽｾｯﾄ
	jnl_prn_buff.PrnBufWriteCnt++;											// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数＋１
	if( jnl_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ空きあり？

		// 空きなしの場合、ｴﾗｰ処理を行う

		jnl_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰｴﾗｰ登録
	}
	JP_I2CSndReq( I2C_PRI_REQ );
	
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀへの１行分文字列ﾃﾞｰﾀ編集処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStr( *dat, pri_kind )							|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= 文字列ﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|						uchar	pri_kind=	ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|							RJ_PRI : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStr(const char *dat, uchar pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ
			PrnStr_R( dat );	// ﾌﾟﾘﾝﾀへ１行分文字列ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			PrnStr_J( dat );	// ﾌﾟﾘﾝﾀへ１行分文字列ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnStr_R( dat );	// ﾌﾟﾘﾝﾀへ１行分文字列ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			PrnStr_J( dat );	// ﾌﾟﾘﾝﾀへ１行分文字列ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄﾌﾟﾘﾝﾀへの１行分文字列ﾃﾞｰﾀ編集処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStr_R( *dat )									|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= 文字列ﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStr_R(const char *dat )
{

	ushort len;
	
	len = (ushort)strlen(dat);

	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// 送信ﾃﾞｰﾀﾊﾞｯﾌｧに送信ﾃﾞｰﾀｺﾋﾟｰ
			dat,
			(size_t)len );

	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt][len] = LF;				// 改行ﾃﾞｰﾀｾｯﾄ
	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt]=len+1;			// 送信ﾃﾞｰﾀﾚﾝｸﾞｽｾｯﾄ
	rct_prn_buff.PrnBufWriteCnt++;											// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数＋１

	if( rct_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ空きあり？

		// 空きなしの場合、ｴﾗｰ処理を行う

		rct_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰｴﾗｰ登録
	}
	inc_dct( MOV_PRNT_CNT, 1 );												// 「ﾚｼｰﾄﾌﾟﾘﾝﾀ印字行数」動作ｶｳﾝﾄ
	GyouCnt_r++;															// 送信済み印字ﾃﾞｰﾀ行数（ﾚｼｰﾄ）＋１

	RP_I2CSndReq( I2C_PRI_REQ );

}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄﾌﾟﾘﾝﾀへの特別日用文字列ﾃﾞｰﾀ編集処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStr_SPDAY( *dat )								|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	: 文字列ﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|						uchar	pos		: 改行ﾌﾗｸﾞ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	T.Namioka													|*/
/*|	Date	:	2006-03-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnStr_SPDAY(char *dat ,uchar pos)
{

	ushort len;
	
	len = (ushort)strlen(dat);

	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// 送信ﾃﾞｰﾀﾊﾞｯﾌｧに送信ﾃﾞｰﾀｺﾋﾟｰ
			dat,
			(size_t)len );

	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt]=len;				// 送信ﾃﾞｰﾀﾚﾝｸﾞｽｾｯﾄ
	rct_prn_buff.PrnBufWriteCnt++;											// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数＋１
	if( rct_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){						// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ空きあり？

		// 空きなしの場合、ｴﾗｰ処理を行う

		rct_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰｴﾗｰ登録
	}

	if(pos){
		rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt][len] = LF;			// 改行ﾃﾞｰﾀｾｯﾄ
		inc_dct( MOV_PRNT_CNT, 1 );											// 「ﾚｼｰﾄﾌﾟﾘﾝﾀ印字行数」動作ｶｳﾝﾄ
		GyouCnt_r++;														// 送信済み印字ﾃﾞｰﾀ行数（ﾚｼｰﾄ）＋１
	}
	RP_I2CSndReq( I2C_PRI_REQ );

}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀへの１行分文字列ﾃﾞｰﾀ編集処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStr_J( *dat )									|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= 文字列ﾃﾞｰﾀﾎﾟｲﾝﾀ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStr_J(const char *dat )
{

	ushort len;
	
	len = (ushort)strlen(dat);

	if( PrnJnlCheck() != ON ){		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続あり？
		return;						// 接続なし
	}

	memcpy(	jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufWriteCnt],				// 送信ﾃﾞｰﾀﾊﾞｯﾌｧに送信ﾃﾞｰﾀｺﾋﾟｰ
			dat,
			(size_t)len );

	jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufWriteCnt][len] = LF;				// 改行ﾃﾞｰﾀｾｯﾄ
	jnl_prn_buff.PrnBufCntLen[jnl_prn_buff.PrnBufWriteCnt]=len+1;			// 送信ﾃﾞｰﾀﾚﾝｸﾞｽｾｯﾄ
	jnl_prn_buff.PrnBufWriteCnt++;											// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数＋１
	if( jnl_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// 送信ﾃﾞｰﾀﾊﾞｯﾌｧ空きあり？

		// 空きなしの場合、ｴﾗｰ処理を行う

		jnl_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰｴﾗｰ登録
	}

// MH810104 GG119201(S) 電子ジャーナル対応
	if (!isEJA_USE()) {
// MH810104 GG119201(E) 電子ジャーナル対応
	inc_dct( MOV_JPNT_CNT, 1 );												// 「ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字行数」動作ｶｳﾝﾄ
// MH810104 GG119201(S) 電子ジャーナル対応
	}
// MH810104 GG119201(E) 電子ジャーナル対応
	GyouCnt_j++;															// 送信済み印字ﾃﾞｰﾀ行数（ｼﾞｬｰﾅﾙ）＋１
	JP_I2CSndReq( I2C_PRI_REQ );
	
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ起動処理														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOut( pri_kind )									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind	= ﾌﾟﾘﾝﾀ種別						|*/
/*|							R_PRI : ﾚｼｰﾄ									|*/
/*|							J_PRI : ｼﾞｬｰﾅﾙ									|*/
/*|							RJ_PRI : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOut( uchar pri_kind )
{
	switch( pri_kind ){		// ﾌﾟﾘﾝﾀ種別？

		case	R_PRI:		// ﾚｼｰﾄ
			PrnOut_R();		// ﾌﾟﾘﾝﾀ起動（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			PrnOut_J();		// ﾌﾟﾘﾝﾀ起動（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnOut_R();		// ﾌﾟﾘﾝﾀ起動（ﾚｼｰﾄ）
			PrnOut_J();		// ﾌﾟﾘﾝﾀ起動（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄﾌﾟﾘﾝﾀ起動処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOut_R( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOut_R( void )
{
	if(	f_Prn_R_SendTopChar == 1){							// ﾚｼｰﾄﾌﾟﾘﾝﾀ印字要求後、先頭の1文字目
		(void)Printer_Lifter_Wait(0);						// 送信前 V24系Unit 起動排他（必要あればこの関数内で待つ）
															// 排他制御権取得
		JR_Print_Wait_tim[0] = 0;							// 他V24Unit一定期間起動禁止（解除ﾀｲﾏｰｽﾀｰﾄ）
		f_Prn_R_SendTopChar = 0;							// 先頭1ｷｬﾗｸﾀ送信要求済み
	}
	rct_prn_buff.PrnBusyCnt = 0;							// ﾌﾟﾘﾝﾀﾋﾞｼﾞｰ継続監視ｶｳﾝﾀｸﾘｱ
	RP_I2CSndReq( I2C_EVENT_QUE_REQ );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ起動処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOut_J( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOut_J( void )
{
	if(	f_Prn_J_SendTopChar == 1){							// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字要求後、先頭の1文字目
		(void)Printer_Lifter_Wait(1);						// 送信前 V24系Unit 起動排他（必要あればこの関数内で待つ）
															// 排他制御権取得
		JR_Print_Wait_tim[1] = 0;							// 他V24Unit一定期間起動禁止（解除ﾀｲﾏｰｽﾀｰﾄ）
		f_Prn_J_SendTopChar = 0;							// 先頭1ｷｬﾗｸﾀ送信要求済み
	}
	jnl_prn_buff.PrnBusyCnt = 0;							// ﾌﾟﾘﾝﾀﾋﾞｼﾞｰ継続監視ｶｳﾝﾀｸﾘｱ
	JP_I2CSndReq( I2C_EVENT_QUE_REQ );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄﾌﾟﾘﾝﾀ停止処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStop_R( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStop_R( void )
{
	rct_prn_buff.PrnBufWriteCnt	= 0;
	rct_prn_buff.PrnBufReadCnt	= 0;
	rct_prn_buff.PrnBufCnt		= 0;
	if( rct_proc_data.Printing != 0 ){
		err_chk( ERRMDL_PRINTER, ERR_PRNT_R_PRINTCOM, 1, 0, 0 );// ﾚｼｰﾄﾌﾟﾘﾝﾀ通信異常登録
		PrnEndMsg(							/*-----	印字終了ﾒｯｾｰｼﾞ送信	-----*/
					rct_proc_data.Printing,	// 印字要求ｺﾏﾝﾄﾞ：入力ﾊﾟﾗﾒｰﾀ
					PRI_ERR_END,			// 印字処理結果	：異常終了
					PRI_ERR_STAT,			// 異常終了理由	：入力ﾊﾟﾗﾒｰﾀ
					R_PRI					// ﾌﾟﾘﾝﾀ種別	：ﾚｼｰﾄ
				);
		JP_I2CSndReq( I2C_PRI_REQ );		/* ジャーナルに送信するデータがあれば送信する */
	}
	prn_proc_data_clr_R();
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ停止処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStop_J( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStop_J( void )
{
	jnl_prn_buff.PrnBufWriteCnt	= 0;
	jnl_prn_buff.PrnBufReadCnt	= 0;
	jnl_prn_buff.PrnBufCnt		= 0;
	if( jnl_proc_data.Printing != 0 ){
// MH810104 GG119201(S) 電子ジャーナル対応（ジャーナルプリンタ通信不良が発生してしまう）
		if (!isEJA_USE()) {
// MH810104 GG119201(E) 電子ジャーナル対応（ジャーナルプリンタ通信不良が発生してしまう）
		err_chk( ERRMDL_PRINTER, ERR_PRNT_J_PRINTCOM, 1, 0, 0 );// ﾚｼｰﾄﾌﾟﾘﾝﾀ通信異常登録
// MH810104 GG119201(S) 電子ジャーナル対応（ジャーナルプリンタ通信不良が発生してしまう）
		}
// MH810104 GG119201(E) 電子ジャーナル対応（ジャーナルプリンタ通信不良が発生してしまう）
		PrnEndMsg(							/*-----	印字終了ﾒｯｾｰｼﾞ送信	-----*/
					jnl_proc_data.Printing,	// 印字要求ｺﾏﾝﾄﾞ：入力ﾊﾟﾗﾒｰﾀ
					PRI_ERR_END,			// 印字処理結果	：異常終了
					PRI_ERR_STAT,			// 異常終了理由	：入力ﾊﾟﾗﾒｰﾀ
					J_PRI					// ﾌﾟﾘﾝﾀ種別	：ｼﾞｬｰﾅﾙ
				);
		RP_I2CSndReq( I2C_PRI_REQ );		/* ジャーナルに送信するデータがあれば送信する */
	}
	prn_proc_data_clr_J();
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ先頭印字ﾃﾞｰﾀ編集処理（ﾛｺﾞ印字／ﾍｯﾀﾞｰ印字／用紙ｶｯﾄ制御）			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Rct_top_edit( cut )									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	cut	=	用紙ｶｯﾄ制御有無						|*/
/*|							OFF	:用紙ｶｯﾄしない								|*/
/*|							ON	:用紙ｶｯﾄする								|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Rct_top_edit( uchar cut )
{
	uchar	settei;			// 「ﾚｼｰﾄ先頭印字」設定ﾃﾞｰﾀ
	uchar	hight_size;		// 「ﾚｼｰﾄ先頭印字ﾃﾞｰﾀ」の縦ｻｲｽﾞ（ﾄﾞｯﾄ数）


	Feed_set( R_PRI, 32 );	// ﾚｼｰﾄﾍｯﾀﾞｰ上部の空き幅調整（紙送り4mm）

	//	ﾚｼｰﾄﾍｯﾀﾞｰ印字処理
	settei = (uchar)CPrmSS[S_PRN][3];	// 「ﾚｼｰﾄｶｯﾄ時の印字」設定ﾃﾞｰﾀ取得

	switch( settei ){	// 印字設定により各処理へ分岐

		case	0:		// 印字なし
			// 0x4c分紙送り(0x4c=(LOGO_HDOT_SIZE(0x90)-0x44)
			// 紙送りしないと前の印字の途中でカットしてしまう
			hight_size = 0x44;
			break;

		case	1:		// ﾛｺﾞ印字
			Prn_edit_logo( R_PRI );									// ﾛｺﾞ印字ﾃﾞｰﾀ編集
			hight_size = LOGO_HDOT_SIZE;
			break;

		case	2:		// ﾍｯﾀﾞｰ印字
			Prn_edit_headhoot( PRI_HEADER, R_PRI );					// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ編集
			hight_size = header_hight;								// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ縦ｻｲｽﾞ（ﾄﾞｯﾄ数）取得
			break;
		case	3:		// ﾛｺﾞ＋ﾍｯﾀﾞｰ
			Prn_edit_logo( R_PRI );									// ﾛｺﾞ印字ﾃﾞｰﾀ編集
			hight_size = LOGO_HDOT_SIZE;
			break;

		default:		// その他
			hight_size = 0;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}
	if( (LOGO_HDOT_SIZE - hight_size) > 0 ){

		//	印字ｻｲｽﾞ（縦）がﾛｺﾞのｻｲｽﾞ（縦）に満たない場合

		Feed_set( R_PRI, (uchar)(LOGO_HDOT_SIZE - hight_size) );	// 不足分紙送り
	}

	if( cut == ON ){	// 用紙ｶｯﾄ制御あり？

		// 用紙ｶｯﾄする場合
		// ドット指定のバックフィード対応までの仮対処
		// 対応後は
		// 		settei = 1,3	⇒  9.5mm(76 dot)
		// 		settei = 2		hight_size > LOGO_HDOT_SIZEの場合
		//							⇒ (hight_size-LOGO_HDOT_SIZE(144))+9.5mm(76 dot)
		//						hight_size <= LOGO_HDOT_SIZEの場合
		//							⇒  9.5mm(76 dot)
		// のバックフィードが必要
		if(( settei == 1 ) || ( settei == 3 )){
// NOTE:この時点でロゴ印字しているので先端が18mm進む、印字ヘッドからカッターの位置までが
// 9.5±1mmなので18-9.5-1=9.5mmのバックフィードが必要となる
			if( OPE_SIG_DOOR_Is_OPEN == 0 ){		// ドア閉?
				PCMD_BS_CUT( R_PRI );				// 9.5mmﾊﾞｯｸﾌｨｰﾄﾞしてﾌﾟﾘﾝﾀ用紙ｶｯﾄ
				f_partial_cut = 0;					// パーシャルカット動作実行フラグクリア
			}else{
				PCMD_BS_PCUT( R_PRI );				// 9.5mmﾊﾞｯｸﾌｨｰﾄﾞしてﾌﾟﾘﾝﾀ用紙ﾊﾟｰｼｬﾙｶｯﾄ
				f_partial_cut = 1;					// パーシャルカット動作実行フラグセット
			}
		}else if( settei == 2 ){
			if(hight_size > LOGO_HDOT_SIZE){
				Back_Feed_set( R_PRI, (uchar)(hight_size-LOGO_HDOT_SIZE+0x4c) );
			}
			if( OPE_SIG_DOOR_Is_OPEN == 0 ){		// ドア閉?
				PCMD_BS_CUT( R_PRI );				// 9.5mmﾊﾞｯｸﾌｨｰﾄﾞしてﾌﾟﾘﾝﾀ用紙ｶｯﾄ
				f_partial_cut = 0;					// パーシャルカット動作実行フラグクリア
			}else{
				PCMD_BS_PCUT( R_PRI );				// 9.5mmﾊﾞｯｸﾌｨｰﾄﾞしてﾌﾟﾘﾝﾀ用紙ﾊﾟｰｼｬﾙｶｯﾄ
				f_partial_cut = 1;					// パーシャルカット動作実行フラグセット
			}
		}else{
	if( OPE_SIG_DOOR_Is_OPEN == 0 ){								// ドア閉?
		PCMD_CUT( R_PRI );							// 用紙ｶｯﾄｺﾏﾝﾄﾞ編集
		f_partial_cut = 0;							// パーシャルカット動作実行フラグクリア
	} else{
		PCMD_PCUT( R_PRI );							// 用紙ﾊﾟｰｼｬﾙｶｯﾄｺﾏﾝﾄﾞ編集
		f_partial_cut = 1;							// パーシャルカット動作実行フラグセット
	}
		}
		inc_dct( MOV_CUT_CNT, 1 );					// ＜ﾚｼｰﾄ用紙ｶｯﾄ＞動作回数ｶｳﾝﾄ
		if( settei != 3 ){
			if( settei == 0 ){						// ﾛｺﾞ・ﾍｯﾀﾞ印字なし
			// ロゴとヘッダの合計サイズが18mmよりトータル18mm送る必要がある
			// Feed_set()で9.5mmフィードとカットにより自動的に2mmフィード残り6.5mmをフィードする
				Feed_set( R_PRI, RCUT_END_FEED_NP );// 紙送り（ﾛｺﾞ・ﾍｯﾀﾞ印字なし時のｶｯﾄ後に紙送りするｻｲｽﾞ分）
			}else if( settei == 2 ){				// ﾍｯﾀﾞ印字のみ
			// ヘッダ印字時の残りフィードは改行による3.5mm(Prn_LogoHead_PostHead ()内の改行1行分)
				PrnCmdLen( (uchar *)"\x0a" , 1 , R_PRI );// [改行]x1
			}
		}else{
			//ﾛｺﾞ+ﾍｯﾀﾞｰの場合は紙送りではなく、ヘッダー文字を２行分（縦倍などは１)印字する.
			//残りの行は次回、印字の前にだす
			Prn_LogoHead_PreHead ( );
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		改行ﾃﾞｰﾀ編集処理（改行量＝設定ﾃﾞｰﾀ）								|*/
/*|																			|*/
/*|		＜備考＞															|*/
/*|			改行幅＝文字種別の高さ＋設定ﾃﾞｰﾀで設定されている改行幅			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Kaigyou_set( pri_kind, font, cnt )					|*/
/*|																			|*/
/*|	PARAMETER		:	uchar		pri_kind	=	印字先ﾌﾟﾘﾝﾀ				|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|																			|*/
/*|						uchar		font		=	文字種別				|*/
/*|																			|*/
/*|						uchar		cnt			=	改行数					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-04													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Kaigyou_set( uchar pri_kind, uchar font, uchar cnt )
{
	uchar	cmd[PRNBUF_SIZE];	// 改行ｺﾏﾝﾄﾃﾞｰﾀ編集ﾜｰｸ
	uchar	i;


	if( (cnt > 0) && (cnt <= PRNBUF_SIZE) ){		// 指定改行数ﾁｪｯｸ

		PrnFontSize( font, pri_kind );				// [印字ﾓｰﾄﾞ指定]＆[改行量（幅）指定]

		for( i = 0 ; i < cnt ; i++ ){				// 要求された数分の改行ﾃﾞｰﾀ作成
			cmd[i] = 0x0a;
		}
		PrnCmdLen( cmd , (ushort)cnt , pri_kind );	// [改行]ﾃﾞｰﾀ編集
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		紙送りﾃﾞｰﾀ編集処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Feed_set( pri_kind, size )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar		pri_kind	=	印字先ﾌﾟﾘﾝﾀ				|*/
/*|							R_PRI  : ﾚｼｰﾄ									|*/
/*|							J_PRI  : ｼﾞｬｰﾅﾙ									|*/
/*|																			|*/
/*|						uchar		size		=	紙送りｻｲｽﾞ（ﾄﾞｯﾄ数）	|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-18													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Feed_set( uchar pri_kind, uchar size )
{
	uchar	cmd[3];		// 紙送りｺﾏﾝﾄﾃﾞｰﾀ編集ﾜｰｸ

	cmd[0] = 0x1b;						// "ESC"
	cmd[1] = 0x4a;						// "J"
	cmd[2] = size;						// 紙送りｻｲｽﾞｾｯﾄ(n)
	PrnCmdLen( cmd , 3 , pri_kind );	// [紙送り]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
}

/*[]-----------------------------------------------------------------------[]*/
/*|		日付ﾃﾞｰﾀﾁｪｯｸ処理													|*/
/*|																			|*/
/*|			年、月、日、時、分が全て０の場合、チェックOKとする				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	DateChk( *date )									|*/
/*|																			|*/
/*|	PARAMETER		:	date_time_rec	*date	=	日付ﾃﾞｰﾀﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ﾁｪｯｸﾃﾞｰﾀ正常								|*/
/*|							NG : ﾁｪｯｸﾃﾞｰﾀ異常								|*/
/*|																			|*/
/*|	備考：年、月、日、時、分が全て０の場合、チェックOKとする				|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	DateChk( date_time_rec *date )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	if( date->Year != 0 ){						// 年ﾃﾞｰﾀﾁｪｯｸ
		if( (date->Year<1000) || (date->Year>9999) ){
			Prn_errlg( ERR_PRNT_YEAR_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 日付（年）ﾃﾞｰﾀｴﾗｰ登録
			ret = NG;
		}
	}
	if( date->Mon != 0 ){						// 月ﾃﾞｰﾀﾁｪｯｸ
		if( (date->Mon<1) || (date->Mon>12) ){
			Prn_errlg( ERR_PRNT_MON_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );		// 日付（月）ﾃﾞｰﾀｴﾗｰ登録
			ret = NG;
		}
	}
	if( date->Day != 0 ){						// 日ﾃﾞｰﾀﾁｪｯｸ
		if( (date->Day<1) || (date->Day>31) ){
			Prn_errlg( ERR_PRNT_DAY_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );		// 日付（日）ﾃﾞｰﾀｴﾗｰ登録
			ret = NG;
		}
	}
	if( date->Hour>23 ){						// 時ﾃﾞｰﾀﾁｪｯｸ
		Prn_errlg( ERR_PRNT_HOUR_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );		// 日付（時）ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	if( date->Min>59 ){							// 分ﾃﾞｰﾀﾁｪｯｸ
		Prn_errlg( ERR_PRNT_MIN_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );			// 日付（分）ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字ﾃﾞｰﾀﾁｪｯｸ処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_data_chk( req, data )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	req		= ﾁｪｯｸするﾃﾞｰﾀ種別					|*/
/*|																			|*/
/*|						ulong	data	= ﾁｪｯｸﾃﾞｰﾀ							|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ﾁｪｯｸﾃﾞｰﾀ正常								|*/
/*|							NG : ﾁｪｯｸﾃﾞｰﾀ異常								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-08													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	Prn_data_chk( uchar req, ulong data )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果
	ushort	data1;
	ushort	data2;

	switch( req ){

		case	RYOUSYUU_PRN_SYU1:	// ＜領収証印字種別１＞	ﾁｪｯｸ要求
			if( data > RYOUSYUU_PRN_SYU1_MAX ){
				ret = NG;
			}
			break;

		case	RYOUSYUU_PRN_SYU2:	// ＜領収証印字種別２＞	ﾁｪｯｸ要求
			if( data > RYOUSYUU_PRN_SYU2_MAX ){
				ret = NG;
			}
			break;

		case	PKICHI_DATA:			// ＜駐車位置ﾃﾞｰﾀ＞	ﾁｪｯｸ要求

			Prn_pkposi_chg( data, &data1, &data2 );						// 駐車位置ﾃﾞｰﾀ変換

			if( data1 > PKICHI_KU_MAX ){								// 区画番号ﾁｪｯｸ
				ret = NG;
			}

			if( data2 > PKICHI_NO_MAX ){								// 駐車位置番号ﾁｪｯｸ
				ret = NG;
			}
			break;

		case	SEISAN_SYU:			// ＜精算種別＞	ﾁｪｯｸ要求
			if( (data > SEISAN_SYU_MAX) && (data != 0x000000ff) ){
				ret = NG;
			}
			break;

		case	RYOUKIN_SYU:		// ＜料金種別＞		ﾁｪｯｸ要求
			if( (data < RYOUKIN_SYU_MIN) || (data > RYOUKIN_SYU_MAX) ){
				ret = NG;
			}
			break;

		case	TEIKI_SYU:			// ＜定期券種別＞	ﾁｪｯｸ要求
			if( (data < TEIKI_SYU_MIN) || (data > TEIKI_SYU_MAX) ){
				ret = NG;
			}
			break;

		case	PKNO_SYU:			// ＜駐車場Ｎｏ．種別＞	ﾁｪｯｸ要求
			if( data > PKNO_SYU_MAX ){
				ret = NG;
			}
			break;

		case	SERVICE_SYU:		// ＜ｻｰﾋﾞｽ券種別＞	ﾁｪｯｸ要求
			if( (data < SERVICE_SYU_MIN) || (data > SERVICE_SYU_MAX) ){
				ret = NG;
			}
			break;

		case	TIKUSE_CNT:			// ＜割引券使用枚数＞	ﾁｪｯｸ要求
			if( (data < TIKUSE_CNT_MIN) || (data > TIKUSE_CNT_MAX) ){
				ret = NG;
			}
			break;

		case	MISE_NO:			// ＜店Ｎｏ．＞	ﾁｪｯｸ要求
			if( (data < MISE_NO_MIN) || (data > MISE_NO_MAX) ){
				ret = NG;
			}
			break;

		case	WARIBIKI_SYU:		// ＜割引種別（割引券）＞	ﾁｪｯｸ要求
			if( (data < WARIBIKI_SYU_MIN) || (data > WARIBIKI_SYU_MAX) ){
				ret = NG;
			}
			break;
// MH810100(S) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
		case	UNPAID_SYU:			// ＜精算未精算種別＞	ﾁｪｯｸ要求
			if(  data > UNPAID_SYU_MAX ){
				ret = NG;
			}
			break;
// MH810100(E) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)

		default:
			ret = NG;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		各種ﾛｸﾞ情報（ｶｳﾝﾀ、ﾎﾟｲﾝﾀ）ﾁｪｯｸ処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_log_check( cnt, wp, max, log_no )				|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	cnt		= 対象ﾊﾞｯﾌｧの現在格納件数			|*/
/*|																			|*/
/*|						ushort	wp		= 対象ﾊﾞｯﾌｧのﾗｲﾄﾎﾟｲﾝﾀ				|*/
/*|																			|*/
/*|						ushort	max		= 対象ﾊﾞｯﾌｧの最大格納件数			|*/
/*|																			|*/
/*|						uchar	log_no	= ﾛｸﾞ情報No.						|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ﾁｪｯｸﾃﾞｰﾀ正常								|*/
/*|							NG : ﾁｪｯｸﾃﾞｰﾀ異常								|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	Prn_log_check( ushort cnt, ushort wp, ushort max, uchar log_no )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果

	if( cnt > max ){	// 現在格納件数ﾁｪｯｸ

		//	現在格納件数が異常な場合
		Prn_errlg( ERR_PRNT_LOG_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ﾛｸﾞﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	if( wp >= max ){	// ﾗｲﾄﾎﾟｲﾝﾀﾁｪｯｸ

		//	ﾗｲﾄﾎﾟｲﾝﾀが異常な場合
		Prn_errlg( ERR_PRNT_LOG_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ﾛｸﾞﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}

	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		各種ﾛｸﾞ情報の最古ﾃﾞｰﾀ検索処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_log_search1( cnt, wp, max )						|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	cnt	= 対象ﾊﾞｯﾌｧの現在格納件数				|*/
/*|																			|*/
/*|						ushort	wp	= 対象ﾊﾞｯﾌｧのﾗｲﾄﾎﾟｲﾝﾀ					|*/
/*|																			|*/
/*|						ushort	max	= 対象ﾊﾞｯﾌｧの最大格納件数				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= 最古ﾃﾞｰﾀのﾃﾞｰﾀﾎﾟｲﾝﾀ（配列番号）		|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	Prn_log_search1( ushort cnt, ushort wp, ushort max )
{
	ushort	ret;		// 最古ﾃﾞｰﾀのﾃﾞｰﾀﾎﾟｲﾝﾀ（配列番号）

	if( cnt == max ){
		//	現在格納件数＝最大格納件数の場合
		ret = wp;
	}
	else{
		//	現在格納件数≠最大格納件数の場合
		ret = 0;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		各種ﾛｸﾞ情報のﾃﾞｰﾀﾎﾟｲﾝﾀ更新処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_log_search2( no, max )							|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	no	= 対象ﾊﾞｯﾌｧのﾃﾞｰﾀﾎﾟｲﾝﾀ（配列番号）		|*/
/*|																			|*/
/*|						ushort	max	= 対象ﾊﾞｯﾌｧの最大格納件数				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= 更新後のﾃﾞｰﾀﾎﾟｲﾝﾀ（配列番号）			|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	Prn_log_search2( ushort no, ushort max )
{

	no++;				// ﾃﾞｰﾀﾎﾟｲﾝﾀ更新（＋１）

	if( no >= max ){

		//	ﾃﾞｰﾀﾎﾟｲﾝﾀが最大格納数になった場合
		no = 0;
	}
	return( no );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		駐車位置ﾃﾞｰﾀ変換処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_pkposi_chg( no, *area, *pkno )					|*/
/*|																			|*/
/*|	PARAMETER		:	ulong	no		= 駐車位置ﾃﾞｰﾀ						|*/
/*|																			|*/
/*|						ushort	*area	= 区画番号ｾｯﾄｱﾄﾞﾚｽ					|*/
/*|																			|*/
/*|						ushort	*pkno	= 駐車位置番号ｾｯﾄｱﾄﾞﾚｽ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-28													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Prn_pkposi_chg( ulong no, ushort *area, ushort *pkno )
{
	*area	= (ushort)(no / 10000);		// 区画番号ｾｯﾄ
	*pkno	= (ushort)(no % 10000);		// 駐車位置番号ｾｯﾄ
}

/*[]-----------------------------------------------------------------------[]*/
/*|		駐車位置（区画／番号）ﾃﾞｰﾀ作成										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_pkno_make( lk_no )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	lk_no	= ﾛｯｸ装置No.						|*/
/*|																			|*/
/*|	RETURN VALUE	:	ulong	pk_posi	= 駐車位置（区画／番号）ﾃﾞｰﾀ		|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ulong	Prn_pkposi_make( ushort lk_no )
{
	ulong	area;			// 「区画」	設定ﾃﾞｰﾀ取得ﾜｰｸ
	ulong	posi;			// 「番号」	設定ﾃﾞｰﾀ取得ﾜｰｸ
	ulong	pk_posi = 0;	// 駐車位置ﾃﾞｰﾀ

	if( lk_no > 0 && lk_no <= LOCK_MAX ){	// ﾛｯｸ装置NO.ﾁｪｯｸ

		//	ﾛｯｸ装置No.が範囲内（１～３２４）の場合

		area =LockInfo[lk_no-1].area;		// 「区画」	設定ﾃﾞｰﾀ取得
		posi =LockInfo[lk_no-1].posi;		// 「番号」	設定ﾃﾞｰﾀ取得
		pk_posi = (area*10000) + posi;		// 駐車位置No.（区画／番号）ﾃﾞｰﾀ作成
	}
	return( pk_posi );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ制御部ｴﾗｰ情報登録処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_errlg( err_no, knd, data )						|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	err_no	=	ｴﾗｰNo.（ope_def.h参照）			|*/
/*|																			|*/
/*|						uchar	knd		=	発生／解除						|*/
/*|																			|*/
/*|											PRN_ERR_OFF   (0)：解除			|*/
/*|											PRN_ERR_ON    (1)：発生			|*/
/*|											PRN_ERR_ONOFF (2)：発生/解除	|*/
/*|																			|*/
/*|						uchar	data	=	ｴﾗｰ情報有無						|*/
/*|																			|*/
/*|											PRN_ERR_JOHO_NON(0)：無し		|*/
/*|											PRN_ERR_JOHO_ARI(1)：有り		|*/
/*|																			|*/
/*|	備考：ｴﾗｰ情報有りの場合、cErrlog[]にｴﾗｰ情報かｾｯﾄされている事			|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Prn_errlg( uchar err_no, uchar knd, uchar data )
{
		ex_errlg(							// ----- エラー情報登録 -----
					ERRMDL_PRINTER,			// 	プリンタタスクモジュールコード
					err_no,					// 	エラーコード
					knd,					// 	発生／解除
					data					// 	エラー情報有無
				);
		cErrlog[0] = 0;						// string data clear
}
//[]-----------------------------------------------------------------------[]
// ヘッダー物理的な２行分は　設定のゼロ番目から何個か？
// cnt(out) : 何個か
// Return:
//			物理的な行数を返す 0=ヘッダー印字自体ない場合
//
// T.Okamoto	2006/08/30		Create
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
uchar	Prn_LogHeadSub_Get2LineSetNum (uchar	*cnt )
{
	uchar	gyou_cnt;
	uchar	m_siz;

	gyou_cnt = (uchar)prm_get( COM_PRM, S_PRN, 4, 2, 3 );					// 「ﾍｯﾀﾞｰ行数」設定ﾃﾞｰﾀ取得
	if( gyou_cnt > HEAD_GYO_MAX ){
		gyou_cnt = HEAD_GYO_MAX;											// 不正ﾃﾞｰﾀの場合、ﾍｯﾀﾞｰ最大行数（４行）とする
	}
	*cnt = 0;
	if( gyou_cnt == 0 ) return 0;

	m_siz = (uchar)prm_get( COM_PRM, S_PRN, (short)5, 1, 4 );	// 「文字ｻｲｽﾞ」設定ﾃﾞｰﾀ取得
	if( m_siz == 2 || m_siz == 3 ){
		*cnt = 1;
		return 2;		//設定の最初が 2行分
	}else if( 1 == gyou_cnt ){
		*cnt = 1;
		return 1;	//1行しかない
	}
	*cnt = 2;
	return 2;	//設定２番目までで2行分
}
//[]-----------------------------------------------------------------------[]
// ﾛｺﾞ＋ﾍｯﾀﾞｰ印字指定の場合、ﾛｺﾞ印字,ｶｯﾄの後で、ﾍｯﾀﾞｰを２行分印字する
// ２行に満たない場合は紙送り制御を行う
//
// T.Okamoto	2006/08/30		Create
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
void	Prn_LogoHead_PreHead ( void )
{
	uchar		cnt;
	uchar		ret;

	// ２行分は　設定のゼロ番目から何個か？=>cnt , 物理行数=>ret
	ret = Prn_LogHeadSub_Get2LineSetNum ( &cnt );

	Prn_LogHead_PostHead_Offs = 0;

	if(cnt){
		Prn_edit_headhoot_new ( 0, cnt, PRI_HEADER, R_PRI ); // ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ編集
		if( header_hight == 0 ) ret = 0;					 // ﾛｰﾄﾞされていない場合
	}
	switch ( ret )
	{
	case	0:
		PrnCmdLen( (uchar *)"\x0a\x0a" , 2 , R_PRI );			// [改行]x2
		break;
	case	1:
		PrnCmdLen( (uchar *)"\x0a" , 1 , R_PRI );				// [改行]x1
	default:
		Prn_LogHead_PostHead_Offs = cnt;
		break;
	}
}

//[]-----------------------------------------------------------------------[]
// ﾛｺﾞ＋ﾍｯﾀﾞｰ印字指定の場合、ﾛｺﾞ印字,ｶｯﾄの後で、ﾍｯﾀﾞｰを２行分印字されている
// ので、のこりの行を印字する
//
// T.Okamoto	2006/08/30		Create
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
void	Prn_LogoHead_PostHead ( void )
{
	short		cnt;
	short		max;
	uchar		settei;

	if((terget_pri == R_PRI)&&(f_partial_cut != 0)){// 印字対象がレシートプリンタかつパーシャルカット実行済み
		// NOTE:パーシャルカット後、紙を切り離すために用紙を強く引っ張られるため、プラテンに負荷がかかる。
		// その状態で印字を行うと文字の印字行頭が潰れる可能性があるためその対策
		// 印字行頭潰れ防止のため1mm(8dot)フィードする(プリンタNP-2701仕様書より)
		// 実際に確認したところ1mmのずれは目視では殆ど判別不可であったので問題ない
		f_partial_cut = 0;					// パーシャルカット動作実行フラグクリア
		Feed_set( R_PRI, RCUT_END_PERTIAL_FEED );// 紙送り（1mm）
	}
	settei = (uchar)CPrmSS[S_PRN][3];	// 「ﾚｼｰﾄｶｯﾄ時の印字」設定ﾃﾞｰﾀ取得
	if( settei == 3 && Prn_LogHead_PostHead_Offs >= 1 && Prn_LogHead_PostHead_Offs < HEAD_GYO_MAX ) 
	{
		cnt = (short)Prn_LogHead_PostHead_Offs;
		max = (short)prm_get( COM_PRM, S_PRN, 4, 2, 3 );					// 「ﾍｯﾀﾞｰ行数」設定ﾃﾞｰﾀ取得
		max -= cnt;
		if(max > 0){
			Prn_edit_headhoot_new ( (uchar)cnt, (uchar)max, PRI_HEADER, R_PRI ); // ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ編集
		}
		PrnCmdLen( (uchar *)"\x0a" , 1 , R_PRI );			// [改行]x1
	}
	Prn_LogHead_PostHead_Offs = 0;
}
//[]-----------------------------------------------------------------------[]
// のこり行印字オフセット情報を初期化する
// 
//
// T.Okamoto	2006/08/30		Create
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
void	Prn_LogoHead_Init ( void )
{
	uchar		ret, cnt;

	Prn_LogHead_PostHead_Offs = 0;

	if( CPrmSS[S_PRN][3] != 3 ) return;	// 「ﾚｼｰﾄｶｯﾄ時の印字」設定ﾃﾞｰﾀ取得

	// ２行分は　設定のゼロ番目から何個か？=>cnt , 物理行数=>ret
	ret = Prn_LogHeadSub_Get2LineSetNum ( &cnt );
	if( ret ){
		Prn_LogHead_PostHead_Offs = cnt;
	}
}

uchar	Prn_JRprintSelect( uchar set_wk, uchar p_kind )
{
uchar	ret;

	ret = 0;
	switch( set_wk ){
		case 1:
			if( p_kind == J_PRI ){
				ret = 1;
			}
			break;
		case 2:
			if( p_kind == R_PRI ){
				ret = 1;
			}
			break;
		case 3:
			ret = 1;
			break;
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			バックフィードデータ編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		uchar		pri_kind	=	印字先ﾌﾟﾘﾝﾀ
///					R_PRI  : ﾚｼｰﾄ
///					J_PRI  : ｼﾞｬｰﾅﾙ
///	@param[in]		uchar		size		=	紙送りｻｲｽﾞ（ﾄﾞｯﾄ数）
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/06/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Back_Feed_set( uchar pri_kind, uchar size )
{
	uchar	cmd[3];		// 紙送りｺﾏﾝﾄﾃﾞｰﾀ編集ﾜｰｸ

	cmd[0] = 0x1b;						// "ESC"
	cmd[1] = 0x42;						// "B"
	cmd[2] = size;						// 紙送りｻｲｽﾞｾｯﾄ(n)
	PrnCmdLen( cmd , 3 , pri_kind );	// [紙送り]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
}
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//[]----------------------------------------------------------------------[]
///	@brief			ジャーナル印字処理中チェック
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return     ret  : 0K:印字中でない NG:印字中
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/09/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
uchar	PriJrnExeCheck( void )
{
	uchar	ret = OK;		// 戻り値

	if( PrnJnlCheck() == ON ){	//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
		// 本関数はジャーナルに大量に印字データを送信しないようにチェックする目的のものである
		// FT4000シリーズのRXM基板はI2Cバスでジャーナル／レシート共通になっており同時には片方しか印字できない仕様である
		// そのためレシートが印字していないこともチェックする
		if((jnl_proc_data.Printing != 0)|| // ジャーナルプリンタ印字中
		   (rct_proc_data.Printing != 0)){ // レシートプリンタ印字中
			ret = NG; // 印字中
		}else{
			ret = OK; // 印字中でない
		}
	}else{
		ret = OK; // 印字中でない
	}
	return(ret);
}
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)

// MH810104 GG119201(S) 電子ジャーナル対応
//[]----------------------------------------------------------------------[]
///	@brief			プリンタ情報要求
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 要求種別
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InfoReq(uchar kind)
{
	PRN_EJA_DATA_BUFF	*p = &eja_prn_buff;
	uchar	dat[3];
	short	timer;

// MH810104 GG119201(S) 電子ジャーナル対応（書込み中に時計設定しない）
	if (jnl_proc_data.Printing != 0) {
		// 書込み完了を待つ
		eja_prn_buff.PrnInfReqFlg = kind;
		return;
	}
// MH810104 GG119201(E) 電子ジャーナル対応（書込み中に時計設定しない）

	memset(p->PrnRcvBuf, 0, sizeof(p->PrnRcvBuf));
	p->PrnRcvBufReadCnt = 0;

	// プリンタ情報要求（ESC s n）
	memcpy(dat, "\x1b\x73", 2);
	dat[2] = kind;
	PrnCmdLen(dat, 3, J_PRI);

	SetPrnInfoReq(1);

	if (kind == PINFO_SD_TEST_RW) {
		timer = (PRN_RW_RES_WAIT_TIMER * 2) + 1;
	}
	else {
		timer = PRN_PINFO_RES_WAIT_TIMER + 1;
	}
	LagTim500ms(LAG500_EJA_PINFO_RES_WAIT_TIMER, timer, PrnCmd_InfoResTimeout);
}

//[]----------------------------------------------------------------------[]
///	@brief			プリンタ情報応答待ちタイムアウト
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InfoResTimeout(void)
{
	SetPrnInfoReq(0);

	if (jnl_init_sts == 0) {
// MH810104 GG119201(S) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
//		queset(OPETCBNO, TIMEOUT9, 0, NULL);
		queset(OPETCBNO, EJA_INIT_WAIT_TIMEOUT, 0, NULL);
// MH810104 GG119201(E) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
	}
	else {
		MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			プリンタ情報応答のデータサイズ設定
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 要求種別
///	@return			データサイズ
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PrnCmd_InfoRes_DataSize_Get(uchar kind)
{
	uchar	size = 2;

	switch (kind) {
	case PINFO_SD_INFO:				// SDカード情報要求
		size += 20;
		break;
	case PINFO_MACHINE_INFO:		// 機器情報要求
		size += 10;
		break;
	case PINFO_VER_INFO:			// ソフトバージョン要求
// MH810104 GG119201(S) 電子ジャーナル対応（SDバージョンが取得できない）
	case PINFO_SD_VER_INFO:			// SDカードバージョン要求
// MH810104 GG119201(E) 電子ジャーナル対応（SDバージョンが取得できない）
	case PINFO_SD_TEST_RW:			// SDカードテスト（リードライト）
		size += 8;
		break;
	default:
		break;
	}
	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			プリンタ情報応答受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		*pBuf	: 受信データ
///	@param[in]		RcvSize	: 受信データサイズ
///	@return			0=受信処理中, 1=受信完了
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PrnCmd_InfoRes_Proc(uchar *pBuf, ushort RcvSize)
{
	PRN_EJA_DATA_BUFF	*p = &eja_prn_buff;
	uchar	ret = 0;
	uchar	rp, DatSize;

	// 受信データセット
	rp = p->PrnRcvBufReadCnt;
	memcpy(&p->PrnRcvBuf[rp], pBuf, (size_t)RcvSize);
	p->PrnRcvBufReadCnt += (uchar)RcvSize;

	// プリンタ情報応答
	// 1byte目＝0xC0（ジャーナルプリンタ）、0xFF（電子ジャーナル）
	// 2byte目＝要求種別
	// 3byte目以降＝各データ
	if (p->PrnRcvBufReadCnt >= 2) {
		// 受信データサイズ取得
		DatSize = PrnCmd_InfoRes_DataSize_Get(p->PrnRcvBuf[1]);
		if (p->PrnRcvBufReadCnt >= DatSize) {
			// 受信完了
			LagCan500ms(LAG500_EJA_PINFO_RES_WAIT_TIMER);
			SetPrnInfoReq(0);
// MH810104 GG119201(S) 電子ジャーナル対応（リセットリトライ回数を追加）
//			p->PrnRetryCnt = 0;
// MH810104 GG119201(E) 電子ジャーナル対応（リセットリトライ回数を追加）
			ret = 1;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			プリンタ情報応答メッセージ
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InfoRes_Msg(void)
{
	PRN_EJA_DATA_BUFF	*p = &eja_prn_buff;
	ushort	msg;
	uchar	*pBuf;

	memset(eja_work_buff, 0, sizeof(eja_work_buff));
	pBuf = &p->PrnRcvBuf[2];
	switch (p->PrnRcvBuf[1]) {
	case PINFO_SD_INFO:				// SDカード情報要求
		msg = PREQ_SD_INFO;
		memcpy(eja_work_buff, pBuf, 20);
		break;
	case PINFO_MACHINE_INFO:		// 機器情報要求
		msg = PREQ_MACHINE_INFO;
		if (!memcmp(pBuf, EJA_MACHINE_INFO, 3)) {
			// 電子ジャーナル接続あり
			p->EjaUseFlg = 1;
		}
		else {
			// 電子ジャーナル接続なし
			p->EjaUseFlg = 0;
		}
		break;
	case PINFO_VER_INFO:			// ソフトバージョン要求
		msg = PREQ_SW_VER_INFO;
		memcpy(eja_work_buff, pBuf, 8);
		break;
	case PINFO_SD_VER_INFO:			// SDカードバージョン要求
		msg = PREQ_SD_VER_INFO;
		memcpy(eja_work_buff, pBuf, 8);
		break;
	case PINFO_SD_TEST_RW:			// SDカードテスト（リードライト）
		msg = PREQ_SD_TEST_RW;
		memcpy(eja_work_buff, pBuf, 8);
		break;
	default:
		return;
	}
	queset(OPETCBNO, msg, 0, NULL);
}

//[]----------------------------------------------------------------------[]
///	@brief			時刻設定（電子ジャーナル用）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_Clock(void)
{
	uchar	dat[9];

	// 時刻設定（GS E n1 n2 n3 n4 n5 n6 n7）
	memcpy(dat, "\x1d\x45", 2);
	// 年月日時分秒
	memcpy(&dat[2], &CLK_REC.year, 7);
	PrnCmdLen(dat, 9, J_PRI);
}

//[]----------------------------------------------------------------------[]
///	@brief			初期設定（電子ジャーナル用）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_Encrypt(void)
{
#pragma pack
	struct {
		uchar	cmd[2];
		ulong	dat[16];
	} code;
#pragma unpack
	uchar	zero[12];
	ushort	i;
	ulong	work;

// MH810104 GG119201(S) 電子ジャーナル対応 #5942 精算機から電子ジャーナルに送信される初期設定コマンドの機種名が「FT-4000」になる
//	const char name[] = "FT-4000";
	const char name[] = "GT-4100";
// MH810104 GG119201(E() 電子ジャーナル対応 #5942 精算機から電子ジャーナルに送信される初期設定コマンドの機種名が「FT-4000」になる

	// 初期設定（GS K Dn）
	memset(&code, 0, sizeof(code));
	memset(zero, 0, sizeof(zero));
	memcpy(code.cmd, "\x1d\x4b", 2);

	// 1ロングワード目 暗号化方式（0=AES128bit, 1=暗号化なし）
	work = prm_get(COM_PRM, S_REC, 2, 1, 1);
	if (work != 1) {
		work = 0;
	}
	code.dat[0] = work;

	// 2～4ロングワード目 暗号化キー
	for (i = 0; i < 3; i++) {
		code.dat[1+i] = prm_get(COM_PRM, S_REC, 3+i, 6, 1);
	}
	// 暗号化キー設定がオール０の場合は暗号化キーを自動生成する
	if (!memcmp(&code.dat[1], zero, 12)) {
		// 秒単位のノーマライズ値をseedとする
		srand((unsigned int)c_Normalize_sec(&CLK_REC));

		for (i = 0; i < 6; i++) {
// MH810104 GG119201(S) 電子ジャーナル対応（暗号化キーは英数字のみとする）
//			do {
//				work = (ulong)rand();
//				// 制御文字以外のASCIIコードを暗号化キーとする
//				work %= 0x7F;
//			} while (work < 0x20);
			while (1) {
				work = (ulong)rand();
				work %= 0x7A;
				// 暗号化キーは英数字のみとする
				if (('0' <= work && work <= '9') ||
					('A' <= work && work <= 'Z') ||
					('a' <= work && work <= 'z')) {
					break;
				}
			}
// MH810104 GG119201(E) 電子ジャーナル対応（暗号化キーは英数字のみとする）

			if ((i % 2) == 0) {
				work *= 1000;
			}
			CPrmSS[S_REC][3+i/2] += work;
			code.dat[1+i/2] += work;
		}
		// 1.共通パラメータのサムを再計算
		DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
		// 2.フラッシュにセーブ
		(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		// FlashROM update
	}
	
	// 5ロングワード目 駐車場No.
	code.dat[4] = prm_get(COM_PRM, S_SYS, 1, 6, 1);
	
	// 6ロングワード目 機械No.
	code.dat[5] = prm_get(COM_PRM, S_PAY, 2, 2, 1);
	
	// 7ロングワード目 デバイスID
	code.dat[6] = 0;	// 標準は0
	
	// 8～11ロングワード目 機種名
	memcpy(&code.dat[7], name, strlen(name) );

	PrnCmdLen((uchar*)&code, sizeof(code), J_PRI);
}

//[]----------------------------------------------------------------------[]
///	@brief			時刻／初期設定（電子ジャーナル用）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InitSetting(void)
{
	// 時刻設定
	PrnCmd_Clock();
	// 初期設定
	PrnCmd_Encrypt();

	if (jnl_init_sts == 0) {
// MH810104 GG119201(S) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
//		Lagtim(OPETCBNO, 9, PRN_INIT_SET_WAIT_TIMER);
		Lagtim(OPETCBNO, TIMERNO_EJA_INIT_WAIT, PRN_INIT_SET_WAIT_TIMER);
// MH810104 GG119201(E) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
	}
	else {
// MH810104 GG119201(S) 電子ジャーナル対応（時刻／初期設定待ちタイマ見直し）
//		Lagtim(PRNTCBNO, 9, PRN_CLK_SET_WAIT_TIMER);
		Lagtim(PRNTCBNO, 9, PRN_INIT_SET_WAIT_TIMER);
// MH810104 GG119201(E) 電子ジャーナル対応（時刻／初期設定待ちタイマ見直し）
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			データ書込み開始／終了送信（電子ジャーナル用）
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 0=書込み開始, 1=書込み終了
///	@param[in]		*msg	: 印字メッセージ
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_WriteStartEnd(uchar kind, MSG *msg)
{
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	if (!isEJA_USE()) {
		if (kind == 0) {
			// ジョブID
			memcpy(&jnl_proc_data.Prn_Job_id, &msg->data[PRN_QUE_OFFSET], sizeof(ulong));
			// ジャーナルプリンタは印字制御データと印字メッセージの保持のみ行う
			memcpy(&eja_prn_buff.eja_proc_data, &jnl_proc_data, sizeof(jnl_proc_data));
			memcpy(&eja_prn_buff.PrnMsg, msg, sizeof(*msg));
			// 印字データキューに同じデータがある場合は削除する
			PrnDat_DelQue(jnl_proc_data.Printing, jnl_proc_data.Prn_Job_id);
		}
		return;
	}
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	if (kind == 0) {
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		// ジョブID
		memcpy(&jnl_proc_data.Prn_Job_id, &msg->data[PRN_QUE_OFFSET], sizeof(ulong));
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		// データ書込み開始
		PCMD_WRITE_START(J_PRI);
		// 印字制御データと印字メッセージを保持する
		memcpy(&eja_prn_buff.eja_proc_data, &jnl_proc_data, sizeof(jnl_proc_data));
		memcpy(&eja_prn_buff.PrnMsg, msg, sizeof(*msg));
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		// 印字データキューに同じデータがある場合は削除する
		PrnDat_DelQue(jnl_proc_data.Printing, jnl_proc_data.Prn_Job_id);
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	}
	else if (kind == 1) {
		// データ書込み終了
		PCMD_WRITE_END(J_PRI);
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
//		Lagtim(PRNTCBNO, 8, PRN_WRITE_CMP_TIMER);
		Lagtim(PRNTCBNO, 10, PRN_WRITE_CMP_TIMER);
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			電子ジャーナル用送信データチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		*pBuf	: 送信データ
///	@return			0=電子ジャーナル用データ以外, 1=電子ジャーナル用データ
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PrnCmd_CheckSendData(uchar *pBuf)
{
	uchar	ret = 0;

	// 機器情報要求
	if (isCMD_MACHINE_INFO(pBuf)) {
		ret = 1;
	}
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	// データ書込み開始
	else if (isCMD_WRITE_START(pBuf)) {
		ret = 1;
	}
	// 暗号化キーコマンド
	else if (isCMD_ENCRYPT(pBuf)) {
		ret = 1;
	}
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			プリンタステータス受信処理（電子ジャーナル用）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_Sts_Proc(void)
{
	PRN_DATA_BUFF	*p = &jnl_prn_buff;	// ﾌﾟﾘﾝﾀﾃﾞｰﾀﾊﾞｯﾌｧﾎﾟｲﾝﾀｰｾｯﾄ
	uchar	ebit;

// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
//	ebit = (uchar)((p->PrnState[0] ^ p->PrnState[1]) & 0xc8);
	ebit = (uchar)((p->PrnState[0] ^ p->PrnState[1]) & 0xCA);
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
	if (ebit != 0) {
// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
		if ((ebit & 0x02) != 0) {
			// SDカード状態に変化あり
			if ((p->PrnState[0] & 0x02) != 0) {
				// SDカード使用不可
				if (jnl_init_sts == 0) {
					// 初期化完了待ち
					LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
					Lagcan(OPETCBNO, TIMERNO_EJA_INIT_WAIT);
					queset(OPETCBNO, EJA_INIT_WAIT_TIMEOUT, 0, NULL);
				}
				else {
					if (LagTim500ms_Is_Counting(LAG500_EJA_RESET_WAIT_TIMER)) {
						// リセット完了待ち
						LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
						// ハードリセット
						MsgSndFrmPrn(PREQ_RESET, J_PRI, 0);
					}
				}
			}
			else {
				// SD使用可
				if ((p->PrnState[0] & 0x40) != 0) {
					// 時刻／初期設定=未設定
					// 時刻／初期設定データ
					MsgSndFrmPrn(PREQ_INIT_SET_REQ, J_PRI, 0);
				}
			}
		}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）

		if ((ebit & 0x08) != 0) {
			// ファイルシステム状態に変化あり
			if ((p->PrnState[0] & 0x08) != 0) {
				// ファイルシステム異常
// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
//				Lagcan(PRNTCBNO, 8);
//				MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
				if (jnl_init_sts == 0) {
					// 初期化完了待ち
					LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
					Lagcan(OPETCBNO, TIMERNO_EJA_INIT_WAIT);
					queset(OPETCBNO, EJA_INIT_WAIT_TIMEOUT, 0, NULL);
				}
				else {
					if (LagTim500ms_Is_Counting(LAG500_EJA_RESET_WAIT_TIMER)) {
						// リセット完了待ち
						LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
						// ハードリセット
						MsgSndFrmPrn(PREQ_RESET, J_PRI, 0);
					}
					else {
						// 待機状態 or 書込み中
						Lagcan(PRNTCBNO, 9);
						Lagcan(PRNTCBNO, 10);
						// ソフトリセット
						MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
					}
				}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
			}
		}

		if ((ebit & 0x40) != 0) {
			// 時刻／初期設定状態に変化あり
			if ((p->PrnState[0] & 0x40) != 0) {
				// 時刻／初期設定=未設定
// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
//				if ((p->PrnState[0] & 0x80) != 0) {
//					// データ書込み中
//					Lagcan(PRNTCBNO, 8);
//					MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
//				}
//				else {
//					LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
//					PrnCmd_ResetTimeout();
//				}
				// 時刻／初期設定データ送信
// MH810104 GG119201(S) 電子ジャーナル対応（印字データを再送しない）
//				LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
// MH810104 GG119201(E) 電子ジャーナル対応（印字データを再送しない）
				if (jnl_init_sts == 0) {
					if (eja_prn_buff.EjaUseFlg < 0) {
						// 電子ジャーナル判定前であれば、電子ジャーナル判定後に時刻／初期設定データを送信する
					}
					else {
						// 初期化完了待ち
						queset(OPETCBNO, PREQ_MACHINE_INFO, 0, NULL);
					}
				}
				else {
// MH810104 GG119201(S) 電子ジャーナル対応（印字データを再送しない）
//					if (jnl_proc_data.Printing != 0) {
//						// 書込み中
//						Lagcan(PRNTCBNO, 10);
//						// ソフトリセット
//						MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
//					}
//					else {
//						// 待機状態 or リセット完了待ち
//						// 時刻／初期設定データ
//						MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
//					}
					if (LagTim500ms_Is_Counting(LAG500_EJA_RESET_WAIT_TIMER)) {
						// リセット完了待ち
						LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
						// 時刻／初期設定データ
// MH810104 GG119201(S) 電子ジャーナル対応（書込み中に時計設定しない）
//						MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
						MsgSndFrmPrn(PREQ_INIT_SET_REQ, J_PRI, 0);
// MH810104 GG119201(E) 電子ジャーナル対応（書込み中に時計設定しない）
					}
					else {
						if (jnl_proc_data.Printing != 0) {
							// 書込み中
							// 書込み完了待ちタイムアウトを待つ
						}
						else {
							// 待機状態 or リセット完了待ち
							// 時刻／初期設定データ
// MH810104 GG119201(S) 電子ジャーナル対応（書込み中に時計設定しない）
//							MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
							MsgSndFrmPrn(PREQ_INIT_SET_REQ, J_PRI, 0);
// MH810104 GG119201(E) 電子ジャーナル対応（書込み中に時計設定しない）
						}
					}
// MH810104 GG119201(E) 電子ジャーナル対応（印字データを再送しない）
				}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
			}
			else {
				// 時刻／初期設定=設定済み
				if (jnl_init_sts == 0) {
// MH810104 GG119201(S) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
//					Lagcan(OPETCBNO, 9);
					Lagcan(OPETCBNO, TIMERNO_EJA_INIT_WAIT);
// MH810104 GG119201(E) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
// MH810105 GG119202(S) 電子ジャーナル対応（SDマウントに時間がかかると通信不良が発生する）
//					PrnMode_idle_J();
// MH810105 GG119202(E) 電子ジャーナル対応（SDマウントに時間がかかると通信不良が発生する）
					// 初期化終了通知
					queset(OPETCBNO, PREQ_INIT_END_J, 0, NULL);
				}
				else {
					Lagcan(PRNTCBNO, 9);
					// 時刻設定完了通知
					MsgSndFrmPrn(PREQ_CLOCK_SET_CMP, J_PRI, 0);
				}
// MH810104 GG119201(S) 電子ジャーナル対応（印字データを再送しない）
//// MH810104 GG119201(S) 電子ジャーナル対応（リセットリトライ回数を追加）
////				eja_prn_buff.PrnRetryCnt = 0;
//				eja_prn_buff.PrnHResetRetryCnt = 0;
//				eja_prn_buff.PrnSResetRetryCnt = 0;
//// MH810104 GG119201(E) 電子ジャーナル対応（リセットリトライ回数を追加）
// MH810104 GG119201(E) 電子ジャーナル対応（印字データを再送しない）
			}
		}

		if ((ebit & 0x80) != 0) {
			// データ書込み状態に変化あり
			if ((p->PrnState[0] & 0x80) == 0) {
				// データ書き込み完了
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
//				Lagcan(PRNTCBNO, 8);
				Lagcan(PRNTCBNO, 10);
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了を待たずにデータ送信してしまう）
//				if (p->PrnBufWriteCnt == p->PrnBufReadCnt) {
//					p->PrnBufWriteCnt = 0;
//					p->PrnBufReadCnt = 0;
//				}
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了を待たずにデータ送信してしまう）
				// 印字終了
				PrnINJI_END(NULL);
				err_chk(ERRMDL_EJA, ERR_EJA_WRITE_ERR, 0, 0, 0);
// MH810104 GG119201(S) 電子ジャーナル対応（印字データを再送しない）
				eja_prn_buff.PrnHResetRetryCnt = 0;
				eja_prn_buff.PrnSResetRetryCnt = 0;
// MH810104 GG119201(E) 電子ジャーナル対応（印字データを再送しない）
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			起動時エラー登録処理（電子ジャーナル用）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_EJAErrRegist(void)
{
	if (!isEJA_USE()) {
// MH810104 GG119201(S) 電子ジャーナル対応
		err_pr_chk2(jnl_prn_buff.PrnState[0], 0);
// MH810104 GG119201(E) 電子ジャーナル対応
		return;
	}

	// 起動直後からSDカードニアエンド等のビットがONの場合、
	// 変化を検出できずにエラー登録ができないので
	// プリンタタスク起動時にビットがONであればエラー登録する
	err_eja_chk(jnl_prn_buff.PrnState[0], 0);
}

//[]----------------------------------------------------------------------[]
///	@brief			データ書込み再開判定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0=再開なし, 1=再開あり（データ書込み途中で復電）,
///					2=再開あり（キューあり）, 3=再開あり（データ書込み途中で復電・キューあり）
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PrnCmd_ReWriteCheck(void)
{
	uchar	ret = 0;

// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//	if (!isEJA_USE()) {
//		return ret;
//	}
	// 電子ジャーナル・ジャーナルプリンタともにデータ書込み再開処理を行う
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	if (eja_prn_buff.eja_proc_data.Printing != 0) {
		// データ書込み途中で復電したため、データ書込み開始から再開する
		memcpy(&jnl_proc_data, &eja_prn_buff.eja_proc_data, sizeof(jnl_proc_data));
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		// 印字データキューに同じデータがある場合は削除する
		PrnDat_DelQue(jnl_proc_data.Printing, jnl_proc_data.Prn_Job_id);
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ret = 1;
	}
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	if (PrnDat_GetQueCount() > 0) {
		if (ret == 0) {
			// 印字データキューにデータあり
			prn_proc_data_clr_J();

			// 印字データキューから読み取り
			PrnDat_ReadQue(&eja_prn_buff.PrnMsg);
			// ジョブIDをセット
			memcpy(&eja_prn_buff.eja_proc_data.Prn_Job_id,
					&eja_prn_buff.PrnMsg.data[PRN_QUE_OFFSET],
					sizeof(ulong));
			// 印字要求コマンドをセット
			eja_prn_buff.eja_proc_data.Printing = eja_prn_buff.PrnMsg.command;
			jnl_proc_data.Printing = eja_prn_buff.PrnMsg.command;
			jnl_proc_data.Prn_Job_id = eja_prn_buff.eja_proc_data.Prn_Job_id;
			ret = 2;
		}
		else {
			// 先にデータ書込み途中で復電した場合の印字を行う
			ret = 3;
		}
	}
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			データ書込み再開
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 0=通常, 1=復電
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_ReWriteStart(uchar kind)
{
	MsgBuf	*OutMsg, *msb;

	if (jnl_proc_data.Printing == 0) {
		return;
	}

	if (kind != 0) {
		if ((OutMsg = GetBuf()) == NULL) {
			return;
		}
		// ダミーメッセージ登録（復電ログより先に印字させるため）
		OutMsg->msg.command = PREQ_DUMMY;
		PutMsg(PRNTCBNO, OutMsg);
		PriDummyMsg = ON;
	}

	if ((msb = GetBuf()) == NULL) {
		return;
	}

	// 印字メッセージ作成
// MH810104 GG119201(S) 電子ジャーナル対応（印字データ再送処理変更）
//// MH810104 GG119201(S) 電子ジャーナル対応（印字データを再送しない）
//	if (kind == 0) {
//		memcpy(&jnl_proc_data, &eja_prn_buff.eja_proc_data, sizeof(jnl_proc_data));
//	}
//// MH810104 GG119201(E) 電子ジャーナル対応（印字データを再送しない）
//	msb->msg.command = jnl_proc_data.Printing;
//	if (jnl_proc_data.Prn_no > 0) {
//		// 印字処理中フラグをセットする
//		msb->msg.command |= INNJI_NEXTMASK;
//	}
//// MH810104 GG119201(S) 電子ジャーナル対応（印字データを再送しない）
//	else {
//		// 初回印字
//		jnl_proc_data.Printing = 0;
//	}
//// MH810104 GG119201(E) 電子ジャーナル対応（印字データを再送しない）
	if (kind == 0) {
		// 通常
		memcpy(&jnl_proc_data, &eja_prn_buff.eja_proc_data, sizeof(jnl_proc_data));
		msb->msg.command = jnl_proc_data.Printing;
		if (jnl_proc_data.Prn_no > 0) {
			// 印字処理中フラグをセットする
			msb->msg.command |= INNJI_NEXTMASK;
		}
		else {
			// 初回印字
			jnl_proc_data.Printing = 0;
		}
	}
	else {
		// 復電
		msb->msg.command = jnl_proc_data.Printing;
		// 復電時は必ず印字処理中フラグをセットする
		// terget_pri=J_PRIに限定したいため
		msb->msg.command |= INNJI_NEXTMASK;
	}
// MH810104 GG119201(E) 電子ジャーナル対応（印字データ再送処理変更）
	memcpy(&msb->msg.data, &eja_prn_buff.PrnMsg.data, sizeof(msb->msg.data));
	msb->msg.data[0] = (J_PRI << 4);
	msb->msg.data[0] |= J_PRI;

	if (kind != 0) {
		// 処理対象メッセージ保存
		TgtMsg = msb;
	}
	else {
		// 処理対象メッセージ登録
		PutMsg(PRNTCBNO, msb);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			電子ジャーナルリセット処理
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 0=ソフトリセット, 1=ハードリセット
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_EJAReset(uchar kind)
{
	PRN_EJA_DATA_BUFF	*p = &eja_prn_buff;
	short	timer;
// MH810104 GG119201(S) 電子ジャーナル対応（リセット後にステータスをクリアする）
	ulong	ist;
// MH810104 GG119201(E) 電子ジャーナル対応（リセット後にステータスをクリアする）

// MH810104 GG119201(S) 電子ジャーナル対応（リセットリトライ回数を追加）
//	if (kind == 0) {
	if (kind == 0 && p->PrnSResetRetryCnt == 0) {
		p->PrnSResetRetryCnt++;
// MH810104 GG119201(E) 電子ジャーナル対応（リセットリトライ回数を追加）
		// ソフトリセット
		PCMD_RESET(J_PRI);
	}
	else {
// MH810104 GG119201(S) 電子ジャーナル対応（リセットリトライ回数を追加）
//		if (p->PrnRetryCnt > 0) {
		if (p->PrnHResetRetryCnt > 0) {
// MH810104 GG119201(E) 電子ジャーナル対応（リセットリトライ回数を追加）
			// リトライオーバー
// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
//			Inji_ErrEnd(p->PrnMsg.command, PRI_ERR_STAT, J_PRI);
//			if (jnl_init_sts == 0) {
//				// エラー情報登録（初期化失敗）
//				jnl_init_sts = 2;
//				Prn_errlg( ERR_PRNT_INIT_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );
//				err_chk(ERRMDL_EJA, ERR_EJA_WAKEUP_ERR, 1, 0, 0);
//			}
//			else {
//				err_chk(ERRMDL_EJA, ERR_EJA_WRITE_ERR, 1, 0, 0);
//			}
			if (jnl_init_sts == 0) {
				// エラー情報登録（初期化失敗）
				jnl_init_sts = 2;
				Prn_errlg( ERR_PRNT_INIT_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );
				if (isEJA_USE()) {
					// プリンタステータスの通知がない場合は
					// SDカードマウント失敗として「SDカード使用不可」エラーを登録する
					err_chk(ERRMDL_EJA, ERR_EJA_SD_UNAVAILABLE, 1, 0, 0);
				}
				else {
					// 機器情報応答タイムアウトで「未接続」エラーを登録する
					err_chk(ERRMDL_EJA, ERR_EJA_UNCONNECTED, 1, 0, 0);
				}
			}
			else {
				// 「書込み失敗」エラーを登録する
				err_chk(ERRMDL_EJA, ERR_EJA_WRITE_ERR, 1, 0, 0);
			}
			Inji_ErrEnd(p->PrnMsg.command, PRI_ERR_STAT, J_PRI);
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
			queset(OPETCBNO, PREQ_ERR_END, 0, NULL);
			return;
		}
// MH810104 GG119201(S) 電子ジャーナル対応（リセットリトライ回数を追加）
//		p->PrnRetryCnt++;
		p->PrnHResetRetryCnt++;
// MH810104 GG119201(E) 電子ジャーナル対応（リセットリトライ回数を追加）

		// ハードリセット
		ExIOSignalwt(EXPORT_JP_RES, 1);
		xPause_PRNTSK(2);					// 20ms Wait
		ExIOSignalwt(EXPORT_JP_RES, 0);
	}

// MH810104 GG119201(S) 電子ジャーナル対応（リセット後にステータスをクリアする）
	// プリンタステータスをクリアする
	ist = _di2();
	jnl_prn_buff.PrnStateMnt = 0;
	jnl_prn_buff.PrnStWork = 0;
	jnl_prn_buff.PrnState[2] = jnl_prn_buff.PrnState[1];
	jnl_prn_buff.PrnState[1] = jnl_prn_buff.PrnState[0];
	jnl_prn_buff.PrnState[0] = jnl_prn_buff.PrnStWork;
	_ei2(ist);
// MH810104 GG119201(E) 電子ジャーナル対応（リセット後にステータスをクリアする）

// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
//	// 時刻／暗号化キー未設定のプリンタステータスを最大8秒待つこととする
//	timer = (PRN_RESET_WAIT_TIMER * 2) + 1;
	if (eja_prn_buff.EjaUseFlg < 0) {
		// リセット完了を待つ
		timer = PRN_RESET_WAIT_TIMER + 1;
	}
	else {
		// 時刻／初期設定=未設定のプリンタステータスを待つ
		timer = (PRN_SD_MOUNT_WAIT_TIMER * 2) + 1;
	}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
	LagTim500ms(LAG500_EJA_RESET_WAIT_TIMER, timer, PrnCmd_ResetTimeout);
}

//[]----------------------------------------------------------------------[]
///	@brief			電子ジャーナルリセットタイムアウト
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_ResetTimeout(void)
{
// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
//	if (jnl_init_sts == 0) {
//		// 機器情報通知
//		queset(OPETCBNO, PREQ_MACHINE_INFO, 0, NULL);
//	}
//	else {
//		// 時刻設定要求
//		MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
//	}
	if (jnl_init_sts == 0) {
		if (eja_prn_buff.EjaUseFlg < 0) {
			// 機器情報要求
			queset(OPETCBNO, PREQ_MACHINE_INFO, 0, NULL);
		}
		else {
			// ハードリセット
			queset(OPETCBNO, EJA_INIT_WAIT_TIMEOUT, 0, NULL);
		}
	}
	else {
		// ハードリセット
		MsgSndFrmPrn(PREQ_RESET, J_PRI, 0);
	}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
}

//[]----------------------------------------------------------------------[]
///	@brief			印字メッセージ再送信
//[]----------------------------------------------------------------------[]
///	@param[in]		*msg	: 印字メッセージ
///	@return			void
///	@date			2021/02/22
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void PrnCmd_MsgResend(MSG *msg)
{
	MsgBuf	*msb;

	if (!isEJA_USE()) {
		return;
	}

	if ((msb = GetBuf()) == NULL) {
		return;
	}

	// メッセージを再送信
	memcpy(&msb->msg, msg, sizeof(MSG));
	PutMsg(PRNTCBNO, msb);
}
// MH810104 GG119201(E) 電子ジャーナル対応

// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//[]----------------------------------------------------------------------[]
///	@brief			印字データキューにデータ書込み
//[]----------------------------------------------------------------------[]
///	@param[in]		command	: 印字要求コマンド
///	@param[in,out]	buf		: 印字データ（最後にジョブIDを付加する）
///	@param[in]		size	: データサイズ
///	@return			0:書込みなし、1～:ジョブID
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
ulong PrnDat_WriteQue(ushort command, uchar *buf, ushort size)
{
	ushort	w_counter[3];
	ulong	job_id;

	if (*buf == R_PRI) {
		// レシート印字要求は処理しない
		return 0;
	}
	if (command < CMND_CHK2_SNO || CMND_CHK2_ENO <= command) {
		// 印字要求コマンド以外は処理しない
		return 0;
	}
	if (command != PREQ_RYOUSYUU) {
		// 現状、領収証印字のみ対象とする
		// 他印字も対応証にする場合、この処理をコメントアウトすること
		return 0;
	}
	if (size > PRN_QUE_DAT_SIZE) {
		// サイズオーバーのため、処理しない
		return 0;
	}

	memcpy(w_counter, &prn_dat_que.count, sizeof(w_counter));

	// キューFULLチェック
	if ((w_counter[0] == PRN_QUE_COUNT_MAX) &&
		(w_counter[1] == w_counter[2])) {
		// ジャーナル紙切れ、ジャーナル故障等で領収証印字不可のため
		// キューFULLのパターンはなし
		// 発生した場合は最古データを読み捨てる
		if (++w_counter[2] >= PRN_QUE_COUNT_MAX) {
			w_counter[2] = 0;
		}
	}

	// 印字要求コマンド
	prn_dat_que.prn_dat[w_counter[1]].command = command;
	// ジョブIDを取得
	job_id = PrnDat_GetJobID();
	memcpy(&buf[PRN_QUE_OFFSET], &job_id, sizeof(job_id));
	// 印字データ
	memcpy(prn_dat_que.prn_dat[w_counter[1]].prndata, buf, (size_t)size);
	// ライトポインタ更新
	if (++w_counter[1] >= PRN_QUE_COUNT_MAX) {
		w_counter[1] = 0;
	}
	// カウンタ更新
	if (w_counter[0] < PRN_QUE_COUNT_MAX) {
		w_counter[0]++;
	}

	nmisave(&prn_dat_que.count, w_counter, sizeof(w_counter));

	return job_id;
}

//[]----------------------------------------------------------------------[]
///	@brief			印字データキューからデータ読込み
//[]----------------------------------------------------------------------[]
///	@param[in,out]	msg	: メッセージバッファ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void PrnDat_ReadQue(MSG *msg)
{
	if (msg == NULL) {
		return;
	}

	// 印字要求コマンド
	msg->command = prn_dat_que.prn_dat[prn_dat_que.rdp].command;
	// 印字データ
	memcpy(msg->data, prn_dat_que.prn_dat[prn_dat_que.rdp].prndata, sizeof(msg->data));
}

//[]----------------------------------------------------------------------[]
///	@brief			印字データキューからデータ1件削除
//[]----------------------------------------------------------------------[]
///	@param[in]		command	: 印字要求コマンド
///	@param[in]		job_id	: ジョブID
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void PrnDat_DelQue(ushort command, ulong job_id)
{
	ushort	w_counter[3];
	ulong	que_job_id;

	if (prn_dat_que.count == 0) {
		return;
	}

	memcpy(&que_job_id,
			&prn_dat_que.prn_dat[prn_dat_que.rdp].prndata[PRN_QUE_OFFSET],
			sizeof(que_job_id));
	if (command != prn_dat_que.prn_dat[prn_dat_que.rdp].command ||
		job_id != que_job_id) {
		// 印字要求コマンド、ジョブIDが異なる
		return;
	}

	memcpy(w_counter, &prn_dat_que.count, sizeof(w_counter));
	// リードポインタ更新
	if (++w_counter[2] >= PRN_QUE_COUNT_MAX) {
		w_counter[2] = 0;
	}
	// カウンタ更新
	if (w_counter[0] > 0) {
		w_counter[0]--;
	}

	nmisave(&prn_dat_que.count, w_counter, sizeof(w_counter));
}

//[]----------------------------------------------------------------------[]
///	@brief			印字データキューカウント取得
//[]----------------------------------------------------------------------[]
///	@return			キューカウント
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
ushort PrnDat_GetQueCount(void)
{
	return prn_dat_que.count;
}

//[]----------------------------------------------------------------------[]
///	@brief			ジョブID取得
//[]----------------------------------------------------------------------[]
///	@return			ジョブID
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
ulong PrnDat_GetJobID(void)
{
	if (++prn_job_id >= PRN_JOB_ID_MAX) {
		prn_job_id = 1;
	}
	return prn_job_id;
}

//[]----------------------------------------------------------------------[]
///	@brief			PRIタスク用メッセージクリア
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/18
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void PrnMsgBoxClear(void)
{
	// プリンタタスク起動前の印字要求で
	// 不要なメッセージを削除する

	// 領収証印字
	// pritask起動前にfukuden()が実行されて
	// 領収証（復）印字要求メッセージが登録される
	// 領収証印字はprn_dat_queに登録されているため、メッセージを削除して
	// 2重に印字されるのを防ぐ
	Target_MsgGet_delete1(PRNTCBNO, PREQ_RYOUSYUU);
}
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
//[]----------------------------------------------------------------------[]
///	@brief			QRコード編集処理
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_kind		ﾌﾟﾘﾝﾀ種別<br>
///										R_PRI	  : レシート<br>
///										J_PRI	  : ジャーナル<br>
///										RJ_PRI	  : ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
///	@param[in]		size			モジュールサイズ：1～20
///	@param[in]		string			データアドレス
///	@param[in]		length			データ長
///	@return			void
///	@attention		None
///	@note			None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2016/03/11<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
void	QrCode_set( uchar pri_kind, uchar size, char *string, ushort length  )
{
	// QRコード印字フォーマットは割引用QRコード(ブラザー)に合わせる。
	// そのため固定値にする必要があるエリアは固定値セットに変更。
	char *ptr;
	
	ptr = (char *)(&Logo_Fdata[0][0]);		//制御が重複することはないのでロゴのワークを使用する
	*ptr++ = ESC;
	*ptr++ = 'q';
	*ptr++ = size;							//モジュールサイズ
	*ptr++ = 2;								//誤り訂正レベル	2:Q 固定
	*ptr++ = 9;								//バージョン		9固定
	*ptr++ = 0;								//マスクパターン	0:自動 1～8:任意
// GG129000(S) 富士急ハイランド機能改善（領収証発行時のQR印字が遅くて印字途中で引っ張られる）（GM803003流用）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
//	*ptr++ = 0;								//データ種別
	if( isNewModel_R() ){
		*ptr++ = 0;							//データ種別
	}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129000(E) 富士急ハイランド機能改善（領収証発行時のQR印字が遅くて印字途中で引っ張られる）（GM803003流用）
	*ptr++ = (char)( length & 0xFF );		//データ数(L)
	*ptr++ = (char)( length >> 8 );			//データ数(H)
	memcpy( ptr, string, length );
	ptr+=length;
	PrnCmdLen( (uchar *)Logo_Fdata, (ushort)(ptr - (&Logo_Fdata[0][0])), pri_kind );	// 編集印字ﾃﾞｰﾀ登録
}

// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
//[]----------------------------------------------------------------------[]
///	@brief			レシートプリンタモデル取得タスク
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/22
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	Prn_GetModel_R( void )
{
	MsgBuf		*msb;											// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	MsgBuf		msg;											// 受信ﾒｯｾｰｼﾞ格納ﾊﾞｯﾌｧ
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;

	Target_WaitMsgID.Count = 2;
	Target_WaitMsgID.Command[0] = PREQ_MACHINE_INFO_R;			// レシート機器情報通知
	Target_WaitMsgID.Command[1] = RPT_INFORES_WAIT_TIMEOUT;		// レシート情報要求タイマ

	memset(&pri_rcv_buff_r, 0, sizeof(PRN_RCV_DATA_BUFF_R));

	// 初期起動のデータを最後まで受信処理させるため、wait
	wait2us(50000L);											// wait 100ms

	pri_rcv_buff_r.NewModelFlg = -1;
	PrnCmd_InfoReq_R(PINFO_MACHINE_INFO);						// 機器情報要求

	/*----------------------------------------------*/
	/*	ﾌﾟﾘﾝﾀ初期化完了待ち処理ﾙｰﾌﾟ					*/
	/*----------------------------------------------*/
	for( ; ; ){

		taskchg( IDLETSKNO );									// ｱｲﾄﾞﾙﾀｽｸ切替
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);		// 期待するﾒｰﾙだけ受信（他は溜めたまま）
		if( NULL == msb ){										// 期待するﾒｰﾙなし（未受信）
			continue;											// 受信ﾒｯｾｰｼﾞなし → continue
		}

		//	受信ﾒｯｾｰｼﾞあり
		memcpy( &msg , msb , sizeof(MsgBuf) );					// 受信ﾒｯｾｰｼﾞ格納
		FreeBuf( msb );											// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放

		switch( msg.msg.command ){								// ﾒｯｾｰｼﾞｺﾏﾝﾄﾞにより各処理へ分岐

			case PREQ_MACHINE_INFO_R:							// 機器情報通知
				// pri_rcv_buff_r.NewModelFlg に -1　以外がセットされているため
				// ここでは何もしない
				break;

			case RPT_INFORES_WAIT_TIMEOUT:						// レシート情報要求タイマ
				pri_rcv_buff_r.NewModelFlg = 0;					// レシートプリンタ従来モデル
				break;

			default:
				break;

		}
		if( pri_rcv_buff_r.NewModelFlg != -1 ){					// モデル確定
			break;												// モデル取得タスクを抜ける
		}
	}

	// モデルチェック
	Prn_Model_Chk_R();
}

//[]----------------------------------------------------------------------[]
///	@brief			レシートプリンタ情報要求
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 要求種別
///	@return			void
///	@date			2023/09/22
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void PrnCmd_InfoReq_R(uchar kind)
{
	uchar	dat[3];
	short	timer;

	// プリンタ情報要求（ESC s n）
	memcpy(dat, "\x1b\x73", 2);
	dat[2] = kind;
	PrnCmdLen(dat, 3, R_PRI);

	// 要求中フラグON
	SetPrnInfoReq_R(1);

	timer = 5;
	LagTim500ms(LAG500_RCT_PINFO_RES_WAIT_TIMER, timer, PrnCmd_InfoResTimeout_R);
}

//[]----------------------------------------------------------------------[]
///	@brief			レシートプリンタ情報応答待ちタイムアウト
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/22
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void PrnCmd_InfoResTimeout_R(void)
{

	// 要求中フラグOFF
	SetPrnInfoReq_R(0);
	queset(OPETCBNO, RPT_INFORES_WAIT_TIMEOUT, 0, NULL);
}

//[]----------------------------------------------------------------------[]
///	@brief			プリンタ情報応答のデータサイズ設定
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 要求種別
///	@return			データサイズ
///	@date			2023/09/26
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
uchar PrnCmd_InfoRes_DataSize_Get_R(uchar kind, uchar* buff)
{
	uchar	size = 2;
// GG129000(S) M.Fujikawa 2024/01/09 ゲート式車番チケットレスシステム対応　レシートプリンターのモデル情報が取得できない不具合
//	uchar	i = 0;
// GG129000(E) M.Fujikawa 2024/01/09 ゲート式車番チケットレスシステム対応　レシートプリンターのモデル情報が取得できない不具合

	switch (kind) {
	case PINFO_MACHINE_INFO:		// 機器情報要求
// GG129000(S) M.Fujikawa 2024/01/09 ゲート式車番チケットレスシステム対応　レシートプリンターのモデル情報が取得できない不具合
//		// 終端文字までのサイズ
//		for ( i = 0; i < PRN_RCV_BUF_SIZE_R; i++ ) {
//			if (buff[i] == 0x00) {	// 終端文字までのサイズ
//				size += i;
//				break;
//			}
//		}
		size += 8;		// モデル情報7ケタ+終端NULLの1ケタ
// GG129000(E) M.Fujikawa 2024/01/09 ゲート式車番チケットレスシステム対応　レシートプリンターのモデル情報が取得できない不具合
		break;
	default:
		break;
	}
	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			レシートプリンタ情報応答受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		*pBuf	: 受信データ
///	@param[in]		RcvSize	: 受信データサイズ
///	@return			0=受信処理中, 1=受信完了
///	@date			2023/09/26
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
uchar PrnCmd_InfoRes_Proc_R(uchar *pBuf, ushort RcvSize)
{
	PRN_RCV_DATA_BUFF_R	*p = &pri_rcv_buff_r;
	uchar	ret = 0;
	uchar	rp, DatSize;

	// 受信データセット
	rp = p->PrnRcvBufReadCnt;
	memcpy(&p->PrnRcvBuf[rp], pBuf, (size_t)RcvSize);
	p->PrnRcvBufReadCnt += (uchar)RcvSize;

	// プリンタ情報応答
	// 1byte目＝0xFF（レシート）
	// 2byte目＝要求種別
	// 3byte目以降＝各データ
	if (p->PrnRcvBufReadCnt >= 2) {
		// 受信データサイズ取得
		DatSize = PrnCmd_InfoRes_DataSize_Get_R(p->PrnRcvBuf[1], &p->PrnRcvBuf[2]);
		if (p->PrnRcvBufReadCnt >= DatSize) {
			// 受信完了
			LagCan500ms(LAG500_RCT_PINFO_RES_WAIT_TIMER);
			SetPrnInfoReq_R(0);
			ret = 1;
		}
	}


	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			プリンタ情報応答メッセージ
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InfoRes_Msg_R(void)
{
	PRN_RCV_DATA_BUFF_R	*p = &pri_rcv_buff_r;
	ushort	msg;
	uchar	*pBuf;

	pBuf = &p->PrnRcvBuf[2];
	switch (p->PrnRcvBuf[1]) {
	case PINFO_MACHINE_INFO:		// 機器情報要求
		msg = PREQ_MACHINE_INFO_R;
		if (!memcmp(pBuf, PRN_MACHINE_NEWMODEL_R, 7)) {
			// 新モデル
			pri_rcv_buff_r.NewModelFlg = 1;
		}
		else {
			// 従来モデル
			pri_rcv_buff_r.NewModelFlg = 0;
		}
		break;
	default:
		return;
	}
	queset(OPETCBNO, msg, 0, NULL);
}

//[]----------------------------------------------------------------------[]
///	@brief			レシートプリンタ取得モデルチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/22
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void Prn_Model_Chk_R(void)
{
	uchar chk_model;

	// パラメータよりモデル設定取得
	// 0=従来
	// 1=新
// GG129000(S) M.Fujikawa 2023/10/17 ゲート式車番チケットレスシステム対応　不具合#7169対応
//	chk_model = (uchar)prm_get( COM_PRM, S_PAY, 22, 1, 1 );
	chk_model = (uchar)prm_get( COM_PRM, S_PAY, 22, 1, 4 );
// GG129000(?) M.Fujikawa 2023/10/17 ゲート式車番チケットレスシステム対応　不具合#7169対応
	// 範囲外は0=従来とする
	if( chk_model > 1 ){
		chk_model = 0;
	}

	// モデル設定と取得モデルが不一致であればエラーフラグON
	// 一致していればエラーフラグOFF
	if( pri_rcv_buff_r.NewModelFlg == chk_model ){
		SetModelMiss_R(0);
	}
	else{
		SetModelMiss_R(1);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ページレイアウト指定コマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_kind：プリンタ種別
///	@param[in]		dt		: 種別
///							: 0x00=ページ縦向き（回転0度）
///							: 0x01=ページ横向き（回転90度）
///							: 0x02=ページ縦向き（回転180度）
///							: 0x03=ページ横向き（回転270度）
///	@return			void
///	@date			2023/09/29
///	@note			ページ長とページ印字領域設定を初期化します。
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	PrnCmd_PLayout_Design( uchar pri_kind, uchar dt )
{
	uchar	work[3];					// ﾍﾟｰｼﾞﾚｲｱｳﾄ指定ｺﾏﾝﾄﾃﾞｰﾀ編集ﾜｰｸ

	work[0] = 0x1b;						// "ESC"
	work[1] = 0x54;						// "T"
	work[2] = dt;						// 種別(hex)
	PrnCmdLen( work , 3 , pri_kind );	// [ﾍﾟｰｼﾞﾚｲｱｳﾄ指定]（ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集）
}

//[]----------------------------------------------------------------------[]
///	@brief			新プリンタクリア処理（バッファクリア＋ページレイアウト初期化）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/29
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	Prn_NewPrint_Clear_R( void )
{
	// ページレイアウト指定（回転0度）
	PrnCmd_PLayout_Design(R_PRI, 0x00);
	// ページバッファクリア
	PCMD_WRITE_RESET(R_PRI);
	// データ書き込み終了
	PCMD_WRITE_END(R_PRI);
	// ﾌﾟﾘﾝﾀ起動（ﾚｼｰﾄﾌﾟﾘﾝﾀ送信割込みEnable）
	PrnOut( R_PRI );
}

//[]----------------------------------------------------------------------[]
///	@brief			ストックデータ印字開始
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/29
///	@note			End_Set_R と同様の処理
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void PrnRYOUSYUU_StockPrint( void )
{
	Rct_top_edit( ON );					// ﾚｼｰﾄ先頭印字ﾃﾞｰﾀ編集処理（ﾛｺﾞ印字／ﾍｯﾀﾞｰ印字／用紙ｶｯﾄ制御）

	// データ書き込み終了
	PCMD_WRITE_END(R_PRI);

	PrnOut( R_PRI );					// ﾌﾟﾘﾝﾀ起動（ﾚｼｰﾄﾌﾟﾘﾝﾀ送信割込みEnable）
	PrnEndMsg(	PREQ_RYOUSYUU,			// 「印字終了（正常終了）」ﾒｯｾｰｼﾞ送信
				PRI_NML_END,
				PRI_ERR_NON,
				R_PRI );
}

//[]----------------------------------------------------------------------[]
///	@brief			ストックデータクリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/29
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void PrnRYOUSYUU_StockClear( void )
{
	Prn_NewPrint_Clear_R();				// 新プリンタクリア処理
	prn_proc_data_clr_R();				// ﾚｼｰﾄ印字編集制御ｴﾘｱ初期化
	PrnEndMsg(	PREQ_RYOUSYUU,			// 「印字終了（正常終了）」ﾒｯｾｰｼﾞ送信
				PRI_NML_END,
				PRI_ERR_NON,
				R_PRI );
}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
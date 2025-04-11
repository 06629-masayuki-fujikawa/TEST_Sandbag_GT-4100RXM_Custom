/*[]----------------------------------------------------------------------[]
 *| System      : UT8500 OPERATION
 *| Module      : セットアップモード処理
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005-11-30
 *| Update      :
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"pri_def.h"
#include	"tbl_rkn.h"
#include	"cnm_def.h"
#include	"flp_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"L_FLASHROM.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"mdl_def.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (backuped)									*/
/*--------------------------------------------------------------------------*/

#pragma section		_UNINIT2

#pragma section

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (not backuped)								*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (const table)									*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: OPESETUP_StartSetup
 *[]----------------------------------------------------------------------[]
 *| summary	: セットアップモード開始要求
 *| param	: Data - 受信予定のセットアップデータ
 *| return	: TRUE - 要求受付OK
 *[]----------------------------------------------------------------------[]*/
uchar	OPESETUP_StartSetup(SETUP_NTNETDATA *Data)
{
	return TRUE;
	// [本実装時]
	// 精算、ユーザメンテ、シスメンテであればFALSEを返す
	// そうでなければセットアップモードに遷移し、TRUEを返す
	// パラメータ"Data"を受信予定のデータ情報としてローカルに保存する
}

/*[]----------------------------------------------------------------------[]
 *|	name	: OPESETUP_SetupDataCancel
 *[]----------------------------------------------------------------------[]
 *| summary	: データ更新の許可/不許可を返す
 *| param	: Data - MLCからの送信キャンセル情報
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	OPESETUP_SetupDataCancel(SETUP_NTNETDATA *Data)
{
	// [本実装時]
	// ？？？？？？？？
	//     Ｑ＆Ａ中
	// ？？？？？？？？
}

/*[]----------------------------------------------------------------------[]
 *|	name	: OPESETUP_SetupDataChk
 *[]----------------------------------------------------------------------[]
 *| summary	: データ更新の許可/不許可を返す
 *| param	: ID - 受信データID
 *|			  Status - ID=80のとき、そのデータステータス
 *| return	: TRUE = データ更新許可
 *[]----------------------------------------------------------------------[]*/
uchar	OPESETUP_SetupDataChk(uchar ID, ushort Status)
{
	return TRUE;
	// [本実装時]
	// パラメータIDがセットアップデータでなければ他に処理をせず、即TRUEを返す
	// パラメータIDがセットアップデータであれば、それが受信予定のものかを確認する
	// 受信予定のものでなければFALSEを返す
	// 受信予定のものであれば受信予定データ情報からクリアする(※)
	// すべての受信予定データ情報がクリアされたら、セットアップモードを抜ける
	// 
	// (※)共通設定パラメータ(ID=80)の場合はStatusが1(=最終)のときのみクリア
	
	// ？？？？？？？？
	// 	セットアップモードからどのモードに戻る？前のop_modを記憶しておくのか？
	// ？？？？？？？？
}

/*====================================================================================[PRIVATE]====*/


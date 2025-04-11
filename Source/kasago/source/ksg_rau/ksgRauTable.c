/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		Rau通信		 																						   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	ksgRauTable.c																			   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・Rau通信 データテーブル処理																			   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2011 AMANO Corp.----------[]*/
																		/*							*/
#include	<machine.h>													/*							*/
#include	<string.h>													/*							*/
#include	"system.h"	
#include	"prm_tbl.h"	
#include	"remote_dl.h"
#include	"ksgRauTable.h"												/* ﾃﾞｰﾀﾃｰﾌﾞﾙ構造体			*/
#include	"ksg_def.h"
#include	"ksgRauModem.h"
#include	"ksgRauModemData.h"
#include	"raudef.h"
																		/*							*/
struct		KSG_RAUCONF		KSG_RauConf;
//struct		KSG_DPACONF		KSG_DpaConf;

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾃﾞｰﾀﾃｰﾌﾞﾙ初期化																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : KSG_RauInitialTbl()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauInitialTbl( void )
{																		/*							*/
	memset( &KSG_RauConf, 0x00, sizeof(KSG_RauConf));					/* 共通設定の初期値			*/
//	memset( &KSG_DpaConf, 0x00, sizeof(KSG_DpaConf));					/* 共通設定の初期値			*/

	// DOPA/FOMA切替
	KSG_uc_FomaFlag = 1;												/* FOMA固定					*/
	KSG_uc_AdapterType = 0;			// UM02-KO
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
//	if (prm_get(COM_PRM, S_CEN, 52, 1, 2) != 0) {
//		KSG_uc_AdapterType = 1;		// UM03-KO
//	}
	if (prm_get(COM_PRM, S_CEN, 52, 1, 2) == 1) {
		KSG_uc_AdapterType = 1;		// UM03-KO
	}else if (prm_get(COM_PRM, S_CEN, 52, 1, 2) == 2) {
		KSG_uc_AdapterType = 2;		// AD-04S(UM04-KO)
	}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	Rau設定の反映																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauSetRauConf()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauSetRauConf(void)
{																		/*							*/
	unsigned char	i;													/*							*/

	// 通信速度
	KSG_RauConf.speed = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 4);

	// データ長
// GG129000(S) W.Hoshino 2023/03/23 #6973 rauの初期設定時、参照先の共通パラメータアドレスが間違えている[共通改善項目 No 1549]
//	KSG_RauConf.data_len = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 4);
	KSG_RauConf.data_len = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 3);
// GG129000(E) W.Hoshino 2023/03/23 #6973 rauの初期設定時、参照先の共通パラメータアドレスが間違えている[共通改善項目 No 1549]

	// ストップビット
	KSG_RauConf.stop_bit = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 2);

	// パリティビット
	KSG_RauConf.parity_bit = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 1);

	// データ処理方法
	KSG_RauConf.deta_full_proc = (unsigned char)prm_get(COM_PRM, S_CEN, 51, 1, 1);

	// リトライ回数(0固定)
	KSG_RauConf.retry = 0;

	// タイムアウト(単位100ms)(1固定)
	KSG_RauConf.time_out = 1;

	// モデム接続リトライ(回数)(0固定)
	KSG_RauConf.modem_retry = 0;

	// モデムの有無 0:あり(FOMA) 1:なし(LAN)
	KSG_RauConf.modem_exist = (unsigned char)prm_get(COM_PRM, S_CEN, 51, 1, 3);
	if(prm_get(COM_PRM, S_PAY, 24, 1, 1) != 2){							// Rau使用条件取得
		if(RAU_Credit_Enabale == 2){
			KSG_RauConf.modem_exist = 0;
		}
		else{
			KSG_RauConf.modem_exist = 1;
		}
	}
	// Dopa/FOMA切換え
	KSG_RauConf.foma_dopa = (unsigned char)CPrmSS[S_CEN][78];

	// Dopa 処理区分 [0]=常時通信なし, [1]=常時通信あり
	KSG_RauConf.Dpa_proc_knd = (unsigned char)prm_get(COM_PRM, S_CEN, 51, 1, 2);

	// Dopa HOST局IPアドレス
	KSG_RauConf.Dpa_IP_h.SEG[0] = (unsigned char)prm_get( COM_PRM, S_CEN, 62, 3, 4 );
	KSG_RauConf.Dpa_IP_h.SEG[1] = (unsigned char)prm_get( COM_PRM, S_CEN, 62, 3, 1 );
	KSG_RauConf.Dpa_IP_h.SEG[2] = (unsigned char)prm_get( COM_PRM, S_CEN, 63, 3, 4 );
	KSG_RauConf.Dpa_IP_h.SEG[3] = (unsigned char)prm_get( COM_PRM, S_CEN, 63, 3, 1 );

	// Dopa HOST局ポート番号
	KSG_RauConf.Dpa_port_h = CPrmSS[S_CEN][64];

	// Dopa 自局IPアドレス
	KSG_RauConf.Dpa_IP_m.SEG[0] = (unsigned char)prm_get( COM_PRM, S_CEN, 65, 3, 4 );
	KSG_RauConf.Dpa_IP_m.SEG[1] = (unsigned char)prm_get( COM_PRM, S_CEN, 65, 3, 1 );
	KSG_RauConf.Dpa_IP_m.SEG[2] = (unsigned char)prm_get( COM_PRM, S_CEN, 66, 3, 4 );
	KSG_RauConf.Dpa_IP_m.SEG[3] = (unsigned char)prm_get( COM_PRM, S_CEN, 66, 3, 1 );

	// Dopa 自局ポート番号
	KSG_RauConf.Dpa_port_m = CPrmSS[S_CEN][67];

	// Dopa 着信時認証手順 0:MS-CHAP 1:PAP 2:CHAP 3:認証なし
	KSG_RauConf.Dpa_ppp_ninsho = (unsigned char)prm_get(COM_PRM, S_CEN, 52, 1, 1);

	// Dopa 無通信タイマー(秒)
	KSG_RauConf.Dpa_nosnd_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 54, 4, 1);

	// Dopa 再発呼待ちタイマー(秒)
	KSG_RauConf.Dpa_cnct_rty_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 53, 3, 4);

	// Dopa ダイアル応答待ち時間(秒)
// TODO:セクション３６にダイアル応答待ち時間がないので要確認
// とりあえず暫定として10秒をセット
	KSG_RauConf.Dpa_dial_wait_tm = 10;

	// Dopa コマンド(ACK)待ちタイマー(秒)
	KSG_RauConf.Dpa_com_wait_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 68, 3, 1);

	// Dopa データ再送待ちタイマー(分)
	KSG_RauConf.Dpa_data_rty_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 75, 3, 1);

	// Dopa TCPコネクション切断待ちタイマー(秒)
	KSG_RauConf.Dpa_discnct_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 69, 4, 1);

	// Dopa 再発呼回数
	KSG_RauConf.Dpa_cnct_rty_cn = (unsigned short)prm_get(COM_PRM, S_CEN, 53, 3, 1);

	// Dopa データ再送回数(NAK,無応答)
	KSG_RauConf.Dpa_data_rty_cn = (unsigned char)prm_get(COM_PRM, S_CEN, 70, 1, 2);

	// Dopa HOST側電文コード [0]=文字コード, [1]=バイナリコード(0固定)
	KSG_RauConf.Dpa_data_code = 0;

	// ＤｏＰａ下り回線パケット送信リトライ回数(無応答)
	KSG_RauConf.Dpa_data_snd_rty_cnt = (unsigned char)prm_get(COM_PRM, S_CEN, 73, 1, 2);

	// ＤｏＰａ下り回線パケット応答待ち時間(ＡＣＫ/ＮＡＫ)
	KSG_RauConf.Dpa_ack_wait_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 72, 3, 1);

	// ＤｏＰａ下り回線パケット受信リトライ回数(ＮＡＫ)
	KSG_RauConf.Dpa_data_rcv_rty_cnt = (unsigned char)prm_get(COM_PRM, S_CEN, 73, 1, 1);

	// ＤｏＰａ下り回線通信監視タイマ(秒)
	KSG_RauConf.Dpa_port_watchdog_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 72, 3, 4);

	// ＤｏＰａ下り回線 自局ポート番号
	KSG_RauConf.Dpa_port_m2 = CPrmSS[S_CEN][71];

	// サブネットマスク
	KSG_RauConf.netmask.SEG[0] = (unsigned char)prm_get( COM_PRM, S_MDL, 4, 3, 4 );
	KSG_RauConf.netmask.SEG[1] = (unsigned char)prm_get( COM_PRM, S_MDL, 4, 3, 1 );
	KSG_RauConf.netmask.SEG[2] = (unsigned char)prm_get( COM_PRM, S_MDL, 5, 3, 4 );
	KSG_RauConf.netmask.SEG[3] = (unsigned char)prm_get( COM_PRM, S_MDL, 5, 3, 1 );

	// LAN Port側 APN文字数 //
	KSG_RauConf.APNLen = (unsigned char)prm_get(COM_PRM, S_CEN, 79, 3, 1);

	// Access Point Name
	for(i = 0; i < 32; ++i) {
		KSG_RauConf.APNName[i] = (uchar)prm_get(COM_PRM, S_CEN, (short)(80+(i / 2)), 3, (char)((i % 2 == 0) ? 4:1));
	}

	// センタークレジット HOST局IPアドレス
	KSG_RauConf.Dpa_IP_Cre_h.SEG[0] = (unsigned char)prm_get( COM_PRM, S_CRE, 21, 3, 4 );
	KSG_RauConf.Dpa_IP_Cre_h.SEG[1] = (unsigned char)prm_get( COM_PRM, S_CRE, 21, 3, 1 );
	KSG_RauConf.Dpa_IP_Cre_h.SEG[2] = (unsigned char)prm_get( COM_PRM, S_CRE, 22, 3, 4 );
	KSG_RauConf.Dpa_IP_Cre_h.SEG[3] = (unsigned char)prm_get( COM_PRM, S_CRE, 22, 3, 1 );

	// センタークレジット HOST局ポート番号
	KSG_RauConf.Dpa_port_Cre_h = CPrmSS[S_CRE][23];

}																		/*							*/

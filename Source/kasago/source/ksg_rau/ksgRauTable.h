/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	データテーブル定義ファイル																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      : S.Takahashi																				   |*/
/*| Date        : 2012-09-10																				   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/

#include	"ksgmac.h"													/* MAC 搭載機能ﾍｯﾀﾞ			*/


/****************************************************************/
/*		ＲＡＵ設定構造体										*/
/****************************************************************/
struct KSG_RAUCONF {
	unsigned char	speed;					// 通信速度
	unsigned char	data_len;				// データ長
	unsigned char	stop_bit;				// ストップビット
	unsigned char	parity_bit;				// パリティビット
	unsigned char	deta_full_proc;			// データ処理方法
	unsigned short	retry;					// リトライ回数
	unsigned short	time_out;				// タイムアウト
	unsigned char	modem_retry;			// モデム接続リトライ
	unsigned char	modem_exist;			// モデムの有無 0:あり(FOMA) 1:なし(LAN)
	unsigned char	foma_dopa;				// 20H/'0' = DOPA   / '1' = FOMA, FOMAの場合 init_data に APN指定 ex>' 1sf.amano.co.jp'
	unsigned char	Dpa_proc_knd;			// Dopa 処理区分 [0]=常時通信なし, [1]=常時通信あり
	IPv4			Dpa_IP_h;				// Dopa HOST局IPアドレス
	unsigned int	Dpa_port_h;				// Dopa HOST局ポート番号
	IPv4			Dpa_IP_m;				// Dopa 自局IPアドレス
	unsigned int	Dpa_port_m;				// Dopa 自局ポート番号
	unsigned char	Dpa_ppp_ninsho;			// Dopa 着信時認証手順 0:MS-CHAP 1:PAP 2:CHAP 3:認証なし
	unsigned short	Dpa_nosnd_tm;			// Dopa 無通信タイマー(秒)
	unsigned short	Dpa_cnct_rty_tm;		// Dopa 再発呼待ちタイマー(秒)
	unsigned short	Dpa_dial_wait_tm;		// Dopa ダイアル応答待ち時間(秒)
	unsigned short	Dpa_com_wait_tm;		// Dopa コマンド(ACK)待ちタイマー(秒)
	unsigned short	Dpa_data_rty_tm;		// Dopa データ再送待ちタイマー(分)
	unsigned short	Dpa_discnct_tm;			// Dopa TCPコネクション切断待ちタイマー(秒)
	unsigned short	Dpa_cnct_rty_cn;		// Dopa 再発呼回数
	unsigned char	Dpa_data_rty_cn;		// Dopa データ再送回数(NAK,無応答)
	unsigned char	Dpa_data_code;			// Dopa HOST側電文コード [0]=文字コード, [1]=バイナリコード
	unsigned char	Dpa_data_snd_rty_cnt;	// ＤｏＰａ下り回線パケット送信リトライ回数(無応答)
	unsigned short	Dpa_ack_wait_tm;		// ＤｏＰａ下り回線パケット応答待ち時間(ＡＣＫ/ＮＡＫ)
	unsigned char	Dpa_data_rcv_rty_cnt;	// ＤｏＰａ下り回線パケット受信リトライ回数(ＮＡＫ)
	unsigned short	Dpa_port_watchdog_tm;	// ＤｏＰａ下り回線通信監視タイマ(秒)
	unsigned short	Dpa_port_m2;			// ＤｏＰａ下り回線 自局ポート番号
	IPv4			netmask;				// サブネットマスク
	unsigned short	APNLen;					// ＡＰＮ文字数
	unsigned char	APNName[32];			// ＡＰＮ
	IPv4			Dpa_IP_Cre_h;			// センタークレジット HOST局IPアドレス
	unsigned int	Dpa_port_Cre_h;			// センタークレジット HOST局ポート番号
};

extern	struct	KSG_RAUCONF	KSG_RauConf;

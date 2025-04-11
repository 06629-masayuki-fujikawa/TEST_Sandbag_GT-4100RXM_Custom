// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/*[]----------------------------------------------------------------------[]*
 *|		NT-NET task communications parts
 *[]----------------------------------------------------------------------[]*
 *| Author      :  J.Mitani
 *| Date        :  2005-06-08
 *| Update      :	2005.05.28	machida.k	[see _NT_DBG]	デバッグコード追加(一定周期毎に強制的にCRCエラーを発生させる)
 *|				:	2005.06.15	#001	J.Mitani 仕様変更によりENQ電文に優先モードフラグを追加
 *|				:	2005.08.16	#002	J.Mitani ゼロカットされヘッダーのみの電文を無視してしまう可能性があったのを修正｡
 *|					2005-12-20			machida.k	RAM共有化
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/
#define		NT_CRCPOLY1				0x1021										/* 左シフト						*/
#define		NT_CRCPOLY2				0x8408  									/* 右シフト						*/
#define		NT_CHAR_BIT				8											/* number of bits in a char		*/
#define		NT_L_SHIFT				0											/* 左シフト						*/
#define		NT_R_SHIFT				1											/* 右シフト						*/

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

/*----------------------------------*/
/*			area define				*/
/*----------------------------------*/

/*----------------------------------*/
/*			table define			*/
/*----------------------------------*/

/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	NT_CrcCcitt(ushort length, uchar cbuf[], uchar *result, uchar type );





/*[]----------------------------------------------------------------------[]*
 *|             NTComComm_Start()                                          |*
 *[]----------------------------------------------------------------------[]*
 *|		NT－NET開始処理										               |*
 *[]----------------------------------------------------------------------[]*/
void	NTComComm_Start( void )
{
	NTComSlave_Start(&send_blk, &receive_blk);
}

/*[]----------------------------------------------------------------------[]*
 *|             NTComComm_Clear()                                          |*
 *[]----------------------------------------------------------------------[]*
 *|		パスワード破壊時処理								               |*
 *[]----------------------------------------------------------------------[]*/
void	NTComComm_Clear( void )
{
	NTComSlave_Clear();
}

/*[]----------------------------------------------------------------------[]*
 *|             NTComComm_Main()                                           |*
 *[]----------------------------------------------------------------------[]*
 *|		NT－NETメイン処理												   |*
 *[]----------------------------------------------------------------------[]*/
void	NTComComm_Main( void )
{
	NTComSlave_Main();
}



/*[]-----------------------------------------------------------------------[]*
 *|             NTComComm_packet_check()
 *[]-----------------------------------------------------------------------[]*
 *|		受信したパケットの状態をチェックする。
 *[]-----------------------------------------------------------------------[]*
 *|		check_blk		チェック対象のブロック
 *|		seq_no			比較用シーケンスナンバー
 *|		terminal_no		比較用端末番号
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture
 *[]-----------------------------------------------------------------------[]*/
BOOL NTComComm_packet_check(T_NT_BLKDATA* check_blk, uchar seq_no, uchar terminal_no){

	ushort len;

	// 制御コードチェック
	if (check_blk->data[NT_OFS_TELEGRAM_KIND] != STX
	 && check_blk->data[NT_OFS_TELEGRAM_KIND] != EOT
	 && check_blk->data[NT_OFS_TELEGRAM_KIND] != ACK
	 && check_blk->data[NT_OFS_TELEGRAM_KIND] != ENQ
	 && check_blk->data[NT_OFS_TELEGRAM_KIND] != NAK) {
		return FALSE;
	}

	len = NT_MakeWord(&check_blk->data[NT_OFS_DATA_SIZE_HI]);

	// データサイズチェック
	if (check_blk->data[NT_OFS_TELEGRAM_KIND] == STX) {
		if (25 > len || len > 985) {
			return FALSE;
		}
	} else {	// ENQ ACK NAK EOT
		if (len != 20) {
			return FALSE;
		}
	}

	// 端末番号チェック
	if ((check_blk->data[NT_OFS_TERMINAL_NO] != 0xFF) && (terminal_no != check_blk->data[NT_OFS_TERMINAL_NO])) {
		return FALSE;
	}

	// 通信リンクNo.チェック
	//ENQ時は比較しない
	if (check_blk->data[NT_OFS_TELEGRAM_KIND] != ENQ) {
		// 現在の選択されている端末No.と比較
		if (seq_no != check_blk->data[NT_OFS_LINK_NO]) {
			return FALSE;
		}
	}
	check_blk->len = len;

	return TRUE;
}

/*[]-----------------------------------------------------------------------[]*
 *|             NTComComm_crc_check()
 *[]-----------------------------------------------------------------------[]*
 *|			 CRCをチェックする。
 *[]-----------------------------------------------------------------------[]*
 *|		buff	チェックする電文。
 *[]-----------------------------------------------------------------------[]*
 *|		TRUE = CRCエラーなし	FALSE = CRCエラーあり
 *[]-----------------------------------------------------------------------[]*/
BOOL NTComComm_crc_check(T_NT_BLKDATA* buff) {

	uchar crc[2];

	NT_CrcCcitt(buff->len, buff->data, crc, 1);

	if (crc[0] != buff->data[buff->len]
	 || crc[1] != buff->data[buff->len + 1]) {
		return FALSE;
	}

	return TRUE;
}

/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_enq_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		電文を格納するバッファ
 *|		uchar send_req_flag		送信要求フラグ
 *|		uchar terminal_no		端末No.
 *|		uchar seq_no			シーケンスNo.
 *|		uchar terminal_status	端末ステータス
 *|		uchar packet_mode		パケット優先モード
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void NTComComm_create_enq_telegram(T_NT_BLKDATA* buff, uchar send_req_flag, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar packet_mode) {

	uchar	crc[2];

	memset(buff->data, 0, 0x14);								//使う領域を０で初期化
	buff->len = 0x16;											//送信データ長　（データサイズ　＋　CRC16）

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"をコピー
	buff->data[NT_OFS_DATA_SIZE_HI]			= 0x00;				//上位は０
	buff->data[NT_OFS_DATA_SIZE_LOW]		= 0x14;				//下位は２０（14h）
	buff->data[NT_OFS_SEND_REQ_FLAG]		= send_req_flag;	//送信要求フラグ
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//端末No.
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//シーケンスNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//端末ステータス
	buff->data[NT_OFS_PACKET_MODE]			= packet_mode;		//パケット優先モード	#001
	buff->data[NT_OFS_TELEGRAM_KIND]		= ENQ;				//ENQ

	NT_CrcCcitt(0x14, buff->data, crc, 1);

	buff->data[0x14]						= crc[0];			//CRC
	buff->data[0x14 + 1]					= crc[1];			//CRC

}
/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_stx_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		電文を格納するバッファ
 *|		uchar direction			データ送信方向
 *|		uchar terminal_no		端末No.
 *|		uchar seq_no			シーケンスNo.
 *|		uchar terminal_status	端末ステータス
 *|		uchar broadcast_flag	同報受信済みフラグ
 *|		uchar response_detail	応答詳細
 *[]-----------------------------------------------------------------------[]*/
void NTComComm_create_stx_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail) {

	uchar	crc[2];
	ulong		Start_1msLifeTime;								// 電文受信時点の1msライフタイマー
	ulong		Past_1msTime;									// 受信～現在までの経過時間（x1ms）

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"をコピー
	buff->data[NT_OFS_DIRECTION]			= direction;		//送信方向
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//端末No.
	buff->data[NT_OFS_SLAVE_TERMINAL_NO1]	= 0;				//
	buff->data[NT_OFS_SLAVE_TERMINAL_NO2]	= 0;				//
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//シーケンスNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//端末ステータス
	buff->data[NT_OFS_BROADCASTED_FLAG]		= broadcast_flag;	//同報データ受信済み

	buff->data[NT_OFS_TELEGRAM_KIND]		= STX;				//STX

	buff->data[NT_OFS_RESPONSE_DETAIL]		= response_detail;	//応答詳細

	if( TRUE == NT_IsNewTypeTimeSetPacket(buff) ) {				// 新形式の時計データ電文
		memcpy( &Start_1msLifeTime, &buff->data[NT_OFS_DATA_TMPKT_FREETIMER], 4 );	// 受信時点のライフタイマー値get
		Past_1msTime = LifePastTim1msGet( Start_1msLifeTime );		// 受信時点からの経過時間get
		memcpy( &buff->data[NT_OFS_DATA_TMPKT_HOSEI], &Past_1msTime, 4 );	// 受信時点からの経過時間set
	}

	NT_CrcCcitt(buff->len, buff->data, crc, 1);

	buff->data[buff->len]						= crc[0];		//CRC
	buff->data[buff->len + 1]					= crc[1];		//CRC
	buff->len += 2;
}
/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_ack_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		電文を格納するバッファ
 *|		uchar direction			データ送信方向
 *|		uchar terminal_no		端末No.
 *|		uchar seq_no			シーケンスNo.
 *|		uchar terminal_status	端末ステータス
 *|		uchar broadcast_flag	同報受信済みフラグ
 *|		uchar response_detail	応答詳細
 *[]-----------------------------------------------------------------------[]*/
void NTComComm_create_ack_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail) {

	uchar	crc[2];

	memset(buff->data, 0, 0x14);								//使う領域を０で初期化
	buff->len = 0x16;											//送信データ長　（データサイズ　＋　CRC16）

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"をコピー
	buff->data[NT_OFS_DATA_SIZE_HI]			= 0x00;				//上位は０
	buff->data[NT_OFS_DATA_SIZE_LOW]		= 0x14;				//下位は２０（14h）
	buff->data[NT_OFS_DIRECTION]			= direction;		//送信方向
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//端末No.
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//シーケンスNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//端末ステータス
	buff->data[NT_OFS_BROADCASTED_FLAG]		= broadcast_flag;	//同報受信済みフラグ

	buff->data[NT_OFS_TELEGRAM_KIND]		= ACK;				//ENQ

	buff->data[NT_OFS_RESPONSE_DETAIL]		= response_detail;	//応答詳細

	NT_CrcCcitt(0x14, buff->data, crc, 1);

	buff->data[0x14]						= crc[0];			//CRC
	buff->data[0x14 + 1]					= crc[1];			//CRC



}
/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_nak_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		電文を格納するバッファ
 *|		uchar direction			データ送信方向
 *|		uchar terminal_no		端末No.
 *|		uchar seq_no			シーケンスNo.
 *|		uchar terminal_status	端末ステータス
 *|		uchar broadcast_flag	同報受信済みフラグ
 *|		uchar response_detail	応答詳細
 *[]-----------------------------------------------------------------------[]*/
void NTComComm_create_nak_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail) {

	uchar	crc[2];

	memset(buff->data, 0, 0x14);								//使う領域を０で初期化
	buff->len = 0x16;											//送信データ長　（データサイズ　＋　CRC16）

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"をコピー
	buff->data[NT_OFS_DATA_SIZE_HI]			= 0x00;				//上位は０
	buff->data[NT_OFS_DATA_SIZE_LOW]		= 0x14;				//下位は２０（14h）
	buff->data[NT_OFS_DIRECTION]			= direction;		//送信方向
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//端末No.
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//シーケンスNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//端末ステータス
	buff->data[NT_OFS_BROADCASTED_FLAG]		= broadcast_flag;	//同報受信済みフラグ

	buff->data[NT_OFS_TELEGRAM_KIND]		= NAK;				//ENQ

	buff->data[NT_OFS_RESPONSE_DETAIL]		= response_detail;	//応答詳細

	NT_CrcCcitt(0x14, buff->data, crc, 1);

	buff->data[0x14]						= crc[0];			//CRC
	buff->data[0x14 + 1]					= crc[1];			//CRC



}
/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_eot_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		電文を格納するバッファ
 *|		uchar direction			データ送信方向
 *|		uchar terminal_no		端末No.
 *|		uchar seq_no			シーケンスNo.
 *|		uchar terminal_status	端末ステータス
 *|		uchar broadcast_flag	同報受信済みフラグ
 *|		uchar response_detail	応答詳細
 *[]-----------------------------------------------------------------------[]*/
void NTComComm_create_eot_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail) {

	uchar	crc[2];

	memset(buff->data, 0, 0x14);								//使う領域を０で初期化
	buff->len = 0x16;											//送信データ長　（データサイズ　＋　CRC16）

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"をコピー
	buff->data[NT_OFS_DATA_SIZE_HI]			= 0x00;				//上位は０
	buff->data[NT_OFS_DATA_SIZE_LOW]		= 0x14;				//下位は２０（14h）
	buff->data[NT_OFS_DIRECTION]			= direction;		//送信方向
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//端末No.
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//シーケンスNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//端末ステータス
	buff->data[NT_OFS_BROADCASTED_FLAG]		= broadcast_flag;	//同報受信済みフラグ

	buff->data[NT_OFS_TELEGRAM_KIND]		= EOT;				//ENQ

	buff->data[NT_OFS_RESPONSE_DETAIL]		= response_detail;	//応答詳細

	NT_CrcCcitt(0x14, buff->data, crc, 1);

	buff->data[0x14]						= crc[0];			//CRC
	buff->data[0x14 + 1]					= crc[1];			//CRC

}

/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_isIdle()
 *[]-----------------------------------------------------------------------[]*
 *|		タスクのアイドル状態を取得する
 *[]-----------------------------------------------------------------------[]*/
BOOL NTComComm_isIdle( void ) {
	return NTComSlave_isIdle();
}

/* CRC16-CCITT 算出関数 (右シフトを使用する) */									/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	Calculate CRC-CCITT																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : crc_ccitt( len, cbuf, result, type )														   |*/
/*| PARAMETER    : len   : Data length																		   |*/
/*|              : cbuf  : Char data																		   |*/
/*|				 : result: Crc																				   |*/
/*|				 : type  : 0: left shift 1:rigth shift														   |*/
/*| RETURN VALUE : void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・CRCの計算を行う																						   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
/* #1 */
void	NT_CrcCcitt(
ushort	length,
uchar	cbuf[],
uchar	*result,
uchar	type )
{																				/*								*/
	ushort	i, j;
	uchar	crc_wk;
	ushort	crc;
																				/*								*/
	crc		= 0x0000;															/*								*/
	crc_wk	= 0x0000;															/*								*/
																				/*								*/
	switch( type ){																/*								*/
																				/*								*/
	case 0: 	/* 左シフト	*/													/*								*/
		for(i = 0; i < length; i++) {											/*								*/
			crc ^= (ushort)cbuf[i] << (16 - NT_CHAR_BIT);
			for (j = 0; j < NT_CHAR_BIT; j++){									/*								*/
				if (crc & 0x8000){												/*								*/
					crc = (crc << 1) ^ NT_CRCPOLY1;								/*								*/
				}else{															/*								*/
					crc <<= 1;													/*								*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	case 1: 	/* 右シフト	*/													/*								*/
		for (i = 0; i < length; i++) {											/*								*/
			crc ^= cbuf[i];														/*								*/
			for (j = 0; j < NT_CHAR_BIT; j++){									/*								*/
				if (crc & 1){													/*								*/
					crc = (crc >> 1) ^ NT_CRCPOLY2;								/*								*/
				}else{															/*								*/
					crc >>= 1;													/*								*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	}																			/*								*/
	crc_wk = (uchar)(crc & 0x00ff);
	*result = crc_wk;															/*								*/
																				/*								*/
	crc_wk = (uchar)((crc & 0xff00) >> NT_CHAR_BIT);
	*(result+1) = crc_wk;														/*								*/
																				/*								*/
}																				/*								*/

ushort NT_MakeWord(unsigned char *data)
{
	ushort	value;

	value =  *(data + 0);
	value <<= 8;
	value |= *(data + 1);

	return value;
}
void NT_Word2Byte(unsigned char *data, ushort us)
{
	*(data + 0) = (us & 0xff00) >> 8;
	*(data + 1) = (us & 0x00ff);
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

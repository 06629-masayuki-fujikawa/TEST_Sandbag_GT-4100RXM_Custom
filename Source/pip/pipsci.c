/***********************************************************************/
/*																	   */
/*	FILE		:SCI.c												   */
/*	DATE		: Aug 23, 2000										   */
/*	DESCRIPTION :Main Program										   */
/*	CPU TYPE	:H8S/2357F											   */
/*																	   */
/***********************************************************************/

#define GLOBAL extern

#include	<ctype.h>
#include	"iodefine.h"
#include	"system.h"
#include	"pip_def.h"

/********************* FUNCTION DEFINE **************************/
void sci_init_h(void);
void sci_tx_start_h(void);

/* ＨＯＳＴ側シリアルＩ／Ｆ起動 */
void sci_init_h(void)
{
	SCI10.SCR.BYTE = 0x70;			// 受信動作許可
	dummy_Read = SCI10.SSR.BYTE;
	SCI10.SSR.BYTE = 0xC4;			// エラーフラグクリア
}

/* ＨＯＳＴ側シリアルＩ／Ｆ送信開始 */
void sci_tx_start_h(void)
{
	SCI10.SCR.BIT.TE = 1;			// シリアル送信動作許可
	SCI10.TDR = PIPRAM.huc_txdata_i_h[PIPRAM.ui_txpointer_h];		// １バイト送信
	PIPRAM.ui_txpointer_h++;
	dummy_Read = SCI10.SSR.BYTE;	// Dummy Read
	PIPRAM.uc_send_end_h = 0;		// 送信完了フラグクリア

	SCI10.SCR.BIT.TIE = 1;			// データエンプティ(TIE)割り込み許可
}

/* ＨＯＳＴ側シリアルＩ／Ｆ停止 */
void sci_stop_h(void)
{
	_di();
	SCI10.SCR.BIT.TIE  = 0;			// 送信データエンプティ(TXI)割り込み禁止 
	SCI10.SCR.BIT.RIE  = 0;			// 受信割込み禁止 
	SCI10.SCR.BIT.TE   = 0;			// 送信動作禁止 
	SCI10.SCR.BIT.RE   = 0;			// 受信動作禁止 
	SCI10.SCR.BIT.TEIE = 0;			// 送信終了(TEI)割り込み禁止 
	dummy_Read = SCI10.SSR.BYTE;	// Dummy Read
	_ei();

}


/*[]----------------------------------------------------------------------[]*/
/*|		対子機通信タスク 受信ドライバ部									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"toS.h"
#include	"toSdata.h"


/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	toScomdr_Init(void);
void	toScomdr_Main(void);
void	toScomdr_RcvInit(void);


/*[]----------------------------------------------------------------------[]*/
/*|             toScomdr_Init() 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		initial routine                                                	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScomdr_Init( void )
{
	toScomdr_f_RcvCmp  = 0;		/* 受信イベントをクリア */
	toScomdr_RcvLength = 0;		/* 受信データ長をクリア */

	toS_RcvCtrl.RcvCnt   = 0;	/* 受信データ長をクリア */
	toS_RcvCtrl.OvfCount = 0;	/* 受信バッファオーバーフロー発生回数は、起動時のみリセット */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScomdr_Main() 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		communication main routine                                  	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScomdr_Main( void )
{
	/*
	*	受信イベントは、以下の順で判定後、作り出す。
	*	①受信エラー発生した？・・・・・・・・・・・・・・・・・・・・・・2
	*	②受信イベントがリセットされていない？・・・・・・・・・・・・・・非0
	*	③受信完了した？・・・・・・・・・・・・・・・・・・・・・・・・・1
	*	④リンク(伝送)タイマ(10msec)がタイムアップした？（未使用）・・・・3
	*	⑤受信(応答待ち)監視タイマ(10msec)タイマがタイムアップした？・・・4
	*	※何れにも該当しなければ、・・・・・・・・・・・・・・・・・・・・0
	*		・何も受信していない。または
	*		・受信中
	*/

	/* ①受信エラー発生 */
	if (toS_RcvCtrl.ComerrStatus != 0) {
		toScomdr_f_RcvCmp = 2;				/* 受信エラー発生を返す */
		if(toS_RcvCtrl.ComerrStatus & 0x01) {	/* over run set	*/
			IFM_SlaveError(E_IFS_Overrun, E_IF_BOTH, toScom_GetSlaveNow());	/* オーバーランエラー[同時] */
		}

		if(toS_RcvCtrl.ComerrStatus & 0x02) {	/* frame error set		*/
			IFM_SlaveError(E_IFS_Frame, E_IF_BOTH, toScom_GetSlaveNow());	/* フレーミングエラー[同時] */
		}

		if(toS_RcvCtrl.ComerrStatus & 0x04) {	/* parity error set		*/
			IFM_SlaveError(E_IFS_Parity, E_IF_BOTH, toScom_GetSlaveNow());	/* パリティエラー[同時] */
		}
		toScomdr_RcvInit();
		return;
	}

	/*
	*	②受信イベントが(リセットされずに)そのまま残っている
	*	・XXXcom_Main()が処理していないことを示す。・・・あり得ないケース
	*/
	if (toScomdr_f_RcvCmp != 0) {
		return;								/* 処理してないから、前回イベントそのままを返す。*/
	}

	/* ③受信完了した（文字間タイムアウトで、タイマー割り込み中に「PAY_RcvCtrl.RcvCmpFlg」フラグがセットされる）*/
	if (toS_RcvCtrl.RcvCmpFlg) {
		toS_RcvCtrl.RcvCmpFlg = 0;			/*（検知したので）受信完了をリセット	2005.06.13 早瀬・追加 */

		/* ここで受信電文が受信完了と認識するから、タスクレベルの受信バッファへ転送 */
		toScom_PktLength = toScomdr_RcvLength;								/* 受信電文長が確定 */
		memcpy(toScom_RcvPkt, toScomdr_RcvData, (size_t)toScom_PktLength);	/* 受信電文(データ)が確定 */

		/*
		*	ここで、PAYcomdr_RcvInit()を呼び出さないことに注意。
		*	・呼び出すと、割り込みレベルでバッファリングしている受信データ長がクリアされてしまう
		*/
		toScomdr_f_RcvCmp = 1;				/* 受信完了を返す */
		return;
	}

	/* ④この10msecタイマは、現在「未使用」*/
	if (toScom_LinkTimeout()) {
		toScomdr_f_RcvCmp = 3;				/* リンクレベルでタイムアウト発生した */
		toScomdr_RcvInit();
		return;
	}

	/* ⑤この10msecタイマは、受信監視（応答待ち）に使う。*/
	if (toScom_LineTimeout()) {
		toScomdr_f_RcvCmp = 4;				/* 子機向けコマンド送信後、応答待ち期限を経過した。またはポーリング時刻経過した */
		toScomdr_RcvInit();
		return;
	}

	/* これ以外は「受信イベント」なし */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScomdr_RcvInit() 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		communication initial routine                                  	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScomdr_RcvInit( void )
{
	toS_RcvCtrl.RcvCnt       = 0;		/* 受信データ長をクリア */
	toS_RcvCtrl.ComerrStatus = 0;		/* 受信エラーをリセット */
	toS_RcvCtrl.RcvCmpFlg    = 0;		/* 受信完了をリセット	2005.06.13 早瀬・追加 */
}

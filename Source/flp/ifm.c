/*[]----------------------------------------------------------------------[]*/
/*|		RAM Memory Data in New I/F(Master)								   |*/
/*|			・ここには、新I/F盤（親機）内の全３タスクが参照する領域を	   |*/
/*|			  定義します。												   |*/
/*|			・CPU内臓RAMと、外付けS-RAMが混在しています。				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-03-22                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"LKcom.h"
#include	"IFM.h"
#include	"LCM.h"

/*==================================*/
/*		InRAM area define			*/
/*==================================*/
/*
*	2005.04.07 原＋早瀬・取り決め
*	①タイマー関連変数
*	②スタック領域（４Kbyte）
*	③シリアル送受信バッファ（割り込みハンドラ所有のもの）
*/

/*----------------------------------*/
/*			LCMtime.c				*/
/*----------------------------------*/
//タイマーカウンタ
ushort	LCMtim_Tx10ms_Action[IF_SLAVE_MAX][IFS_ROOM_MAX];				/* 10msec単位：動作タイマー(故障検知) */
ushort	LCMtim_Tx1sec_Retry[IF_SLAVE_MAX][IFS_ROOM_MAX];				/*   1sec単位：故障時リトライ間隔 */
ushort	LCMtim_Tx1sec_Wait;						/*   1sec単位：以下に使用する */
												/*				①バージョン要求に対する応答待ち */
												/*				②全ロック開/閉 */
												/*				③全/個別ロック開閉テスト */
uchar	LCM_f_TmStart;							/* タイマー動作開始フラグ */
												/*		1=タイマー動作する（カウントする）*/
												/*		0=タイマー動作しない（カウントしない）*/

/*----------------------------------*/
/*	toS\toStime.c					*/
/*----------------------------------*/
ushort	toScom_Tx2ms_Char;						/* 文字間監視用（ 1msec Timer）*/
ushort	toScom_Tx10ms_Link;						/* 受信監視用  （10msec Timer）*/
ushort	toScom_Tx10ms_Line;						/* 回線監視用  （10msec Timer）*/
uchar	toS_f_TmStart = 0;						/* タイマー動作開始フラグ */
												/*		1=タイマー動作する（カウントする）*/
												/*		0=タイマー動作しない（カウントしない）*/
/*----------------------------------*/
/*	toS\toSsci.c					*/
/*----------------------------------*/
t_SciRcvCtrl	toS_RcvCtrl;					/* SCI受信管理情報（割り込みハンドラ使用）*/
t_SciSndCtrl	toS_SndCtrl;					/* SCI送信管理情報（割り込みハンドラ使用）*/
unsigned char	toS_RcvBuf[TOS_SCI_RCVBUF_SIZE];/* SCI受信バッファ（割り込みハンドラ使用）*/
unsigned char	toS_SndBuf[TOS_SCI_SNDBUF_SIZE];/* SCI送信バッファ（割り込みハンドラ使用）*/
t_SciRcvError	toS_RcvErr;						/* シリアル受信エラー情報（以下の発生回数）*/
												/*		①オーバーランエラー発生回数 */
												/*		②フレーミングエラー発生回数 */
												/*		③パリティエラー発生回数 */
t_SciComError	toS_ComErr;						/* 通信エラー情報 */

/*==================================*/
/*		SRAM area define			*/
/*==================================*/
/*
*	2005.04.07 原＋早瀬・取り決め
*	①初期設定データ（ロック管理テーブルに含まれる）
*	②キュー
*	③子機の状態（ロック管理テーブルに含まれる）
*/
//共通（所有者なし）
t_IFM_InitData	IFM_LockTable;				/* ロック装置管理テーブル（初期設定データ、子機の状態を含む）*/

/************************************
*	キュー
*	・セレクティング待ちリスト
*	・子機状態リスト
*	・ロック動作完結待ちリスト
*************************************/

/*
*	セレクティング待ちリスト
*	・対子機通信タスクが、子機へ送りたい「データあり」のとき、溜め込むキュー。
*	  ポーリング時刻が経過したら、ポーリング(状態要求データ)する代わりに
*	  子機へ送りつける。・・・(いきなり)セレクティング。
*	・以下が対象
*		1)(49H：'I')初期設定データ
*		2)(57H：'W')状態書き込みデータ
*		3)(52H：'R')状態要求データ（バージョン要求）
*		※(52H：'R')状態要求データ（＝ポーリング）は溜まらないので注意のこと。
*
*	2005.06.22 早瀬・変更
*	・キュー実体は、順番ボードにつながれたまま解放しないときがあるため、多めに確保する。
*/
t_IFM_Queue			toSque_Body_Select[TOS_Q_WAIT_SELECT * 2];	/* キュー実体 */
struct t_IF_Queue	toSque_Buff_Select[TOS_Q_WAIT_SELECT];		/* キュー本体 */
t_IF_Queue_ctl		toSque_Ctrl_Select;							/* キュー管理情報 */

/*
*	子機状態リスト
*	・対子機通信タスクが、受信した電文をロック管理タスクへ知らせたいとき、溜め込むキュー。
*	・以下が対象
*		1)(41H：'A')子機状態データ
*		2)(56H：'V')子機バージョンデータ
*/
t_IFM_Queue			toSque_Body_Status[LCM_Q_SLAVE_STATUS];	/* キュー実体 */
struct t_IF_Queue	toSque_Buff_Status[LCM_Q_SLAVE_STATUS];	/* キュー本体 */
t_IF_Queue_ctl		toSque_Ctrl_Status;						/* キュー管理情報 */

/*
*	ロック動作動作完結待ちリスト
*	・ロック管理タスクが、子機へ動作要求を送りたいが「動作中」のとき、溜め込むキュー。
*	  子機状態データにて「動作完結」したら、子機へ動作要求する。または
*	  子機へ送った動作要求がタイムアウトしたら、次の動作要求する。
*	・以下が対象
*		1)(57H：'W')状態書き込みデータ
*/
t_IFM_Queue			LCMque_Body_Action[LCM_Q_WAIT_ACTION];	/* キュー実体 */
struct t_IF_Queue	LCMque_Buff_Action[LCM_Q_WAIT_ACTION];	/* キュー本体 */
t_LCMque_Ctrl_Action  LCMque_Ctrl_Action;	// キュー管理情報

/* ロック装置メーカー別の信号出力時間テーブル */
t_IF_Signal	IF_SignalOut[LOCK_MAKER_END];



/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
//
uchar	IFM_GetBPSforSalve(void);

//キュー関連
void				EnQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pAdd, int iWhere);
struct t_IF_Queue	*GetQueue(t_IF_Queue_ctl *pCtl);
int					CheckQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pQue, int iWhere);
struct t_IF_Queue	*DeQueue(t_IF_Queue_ctl *pCtl, int iWhere);
struct t_IF_Queue	*GetQueueBuff(struct t_IF_Queue *pQ, ushort byteSize);
t_IFM_Queue			*GetQueueBody(t_IFM_Queue *pQ, ushort byteSize);


//エラー発生/解除履歴
void	IFM_MasterError(int iError, uchar ucNow);
void	IFM_SlaveError(int iError, uchar ucNow, int iSlave);

void	EnQueue2(t_LCMque_Ctrl_Action *pCtl, struct t_IF_Queue *pAdd, int iWhere, BOOL bRetry);
struct t_IF_Queue	*DeQueue2(t_LCMque_Ctrl_Action *pCtl);
struct t_IF_Queue	*RemoveQueue2(t_LCMque_Ctrl_Action *pCtl, int iWhere);
int	MatchRetryQueue_Lock(t_IF_Queue_ctl *pCtl, uchar ucSlave, uchar ucLock);

void	PAYcom_NoticeError(uchar ucTermNo, uchar ucError, uchar ucState);

int Seach_Close_command_sameno(t_IF_Queue_ctl *pCtl,uchar tno);
int Seach_Close_command(t_IF_Queue_ctl *pCtl,ushort scount);
int Get_Close_command_cnt(t_IF_Queue_ctl *pCtl);

#pragma section
/*----------------------------------*/
/*			table define			*/
/*----------------------------------*/

/* ロック装置メーカー別の信号出力時間テーブル（デフォルト）*/
const t_IF_Signal	IF_SignalOutDefault[LOCK_MAKER_END] = {
	{	 0,		 0},			/* [0]：接続なしには来ない */
	{	40,		40},			/* [1]：=4000msec	英田エンジニアリング製ロック装置（自転車）	*/
	{	20,		20},			/* [2]：=2000msec	英田エンジニアリング製ロック装置（バイク）	*/
	{	 4,		 4},			/* [3]：= 400msec	吉増製作所製ロック装置（自転車、バイク兼用）*/
	{	 4,		 4},			/* [4]：= 400msec	コムズジャパン製ロック装置（自転車）		*/
	{	 4,		 4}				/* [5]：= 400msec	HID製ロック装置（自転車、バイク兼用）		*/
};

/*[]----------------------------------------------------------------------[]*/
/*|             IFM_GetBPSforSalve() 		   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		伝送速度を取得（対子機）										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	IFM_GetBPSforSalve(void)
{
	/* DIP-SWから読み出した、伝送速度を返す。*/
	return IFM_LockTable.ucBitRateForSlave;
}


/*[]----------------------------------------------------------------------[]*/
/*|             IFM_MasterError()			   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		エラー通知処理(親向け)											   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-06-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	IFM_MasterError(int iError, uchar ucNow)
{
	uchar	last;
	uchar	errorCode;

	/*「認識できない」エラーは除外 */
	if (iError >= E_IFM_END ||
		iError < 0)
	{
		return;
	}

	/* 今回が「何だったか？」で分岐 */
	last = IFM_LockTable.sIFMerror[iError].ucState;
	switch (ucNow) {
		case 0:		/* 解除・・・既に発生していたら？の条件付 */
			if (last == 1) {
				IFM_LockTable.sIFMerror[iError].ucState = ucNow;	/* 解除へ */
			}
			/* 既に解除しているのなら「変化なし」なので通知不要 */
			else {
				return;
			}
			break;

		case 1:		/* 発生・・・前に発生していなければ？の条件付 */
			if (last != 1) {
				IFM_LockTable.sIFMerror[iError].ucState = ucNow;	/* 発生へ */
				IFM_LockTable.sIFMerror[iError].ulCount++;			/* 件数も数える */
			}
			/* 既に発生しているのなら「変化なし」なので通知不要 */
			else {
				return;
			}
			break;

		case 2:		/* 発生解除・同時・・・条件なし */
			IFM_LockTable.sIFMerror[iError].ucState = ucNow;		/* 発生解除・同時へ */
			IFM_LockTable.sIFMerror[iError].ulCount++;				/* 件数も数える */
			break;

		default:
			/* 何か「変」なので通知不要 */
			return;
	}

	/*
	*	引数にて渡されるエラーは、内部管理番号である。
	*	なので、
	*	精算機へのエラーコードへ変換/統合する。
	*/
	switch (iError) {
		case E_PAY_Parity:				errorCode =  1;	break;
		case E_PAY_Frame:				errorCode =  2;	break;
		case E_PAY_Overrun:				errorCode =  3;	break;
		case E_PAY_PACKET_HeadStr:		errorCode =  4;	break;
		case E_PAY_PACKET_DataSize:		errorCode =  5;	break;
		case E_PAY_PACKET_WrongKind:	errorCode =  6;	break;
		case E_PAY_NAK_Retry:			errorCode =  7;	break;
		case E_PAY_NO_Answer:			errorCode =  8;	break;
		case E_PAY_PACKET_WrongCode:
		case E_PAY_PACKET_CRC:			errorCode =  9;	break;
		case E_PAY_PACKET_Headerless:	errorCode = 10;	break;
		case E_NT_NET_Protocol:			errorCode = 11;	break;
		case E_PAY_PACKET_DUPLICATE:	errorCode = 12;	break;
		case E_PAY_PACKET_Timeover:		errorCode = 13;	break;
		case E_PAY_QFULL_PollingWait:	errorCode = 20;	break;
		case E_IFM_QFULL_SlaveStatus:	errorCode = 21;	break;
		case E_IFM_QFULL_LockAction:	errorCode = 22;	break;
		case E_IFS_QFULL_SelectingWait:	errorCode = 23;	break;
		case E_IFM_INIT_Data:			errorCode = 30;	break;
		case E_IFM_CTRL_Command:		errorCode = 31;	break;
		case E_IFM_TEST_Next:			errorCode = 32;	break;
		case E_IFM_MODE_Change:			errorCode = 33;	break;
		case E_IFM_Password:			errorCode = 90;	break;

		case E_IFM_BUG:
		default:
			return;
	}

	/* 精算機へ通知する */
	PAYcom_NoticeError(0, errorCode, ucNow);
}

/*[]----------------------------------------------------------------------[]*/
/*|             IFM_SlaveError()			   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		エラー通知処理(子向け)											   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-06-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	IFM_SlaveError(int iError, uchar ucNow, int iSlave)
{
	uchar	last;
	uchar	errorCode;

	/*「認識できない」エラーは除外 */
	if (iError >= E_IFS_END ||
		iError < 0)
	{
		return;
	}

	/* 前回が「何だったか？」で分岐 */
	if( IFS_CRR_OFFSET > iSlave ){	// CRR基板へのアクセスではない
		last = IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ucState;
		switch (ucNow) {
			case 0:		/* 解除・・・既に発生していたら？の条件付 */
				if (last == 1) {
					IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ucState = ucNow;	/* 解除へ */
				}
				/* 既に解除しているのなら「変化なし」なので通知不要 */
				else {
					return;
				}
				break;

			case 1:		/* 発生・・・前に発生していなければ？の条件付 */
				if (last != 1) {
					IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ucState = ucNow;	/* 発生へ */
					IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ulCount++;		/* 件数も数える */
				}
				/* 既に発生しているのなら「変化なし」なので通知不要 */
				else {
					return;
				}
				break;

			case 2:		/* 発生解除・同時・・・条件なし */
				IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ucState = ucNow;		/* 発生解除・同時へ */
				IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ulCount++;			/* 件数も数える */
				break;

			default:
				break;
		}
	} else {						// CRR基板へのアクセスの場合
		last = IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ucState;
		switch (ucNow) {
			case 0:		/* 解除・・・既に発生していたら？の条件付 */
				if (last == 1) {
					IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ucState = ucNow;	/* 解除へ */
				}
				/* 既に解除しているのなら「変化なし」なので通知不要 */
				else {
					return;
				}
				break;

			case 1:		/* 発生・・・前に発生していなければ？の条件付 */
				if (last != 1) {
					IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ucState = ucNow;	/* 発生へ */
					IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ulCount++;		/* 件数も数える */
				}
				/* 既に発生しているのなら「変化なし」なので通知不要 */
				else {
					return;
				}
				break;

			case 2:		/* 発生解除・同時・・・条件なし */
				IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ucState = ucNow;		/* 発生解除・同時へ */
				IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ulCount++;			/* 件数も数える */
				break;

			default:
				break;
		}
	}

	/*
	*	引数にて渡されるエラーは、内部管理番号である。
	*	なので、
	*	精算機へのエラーコードへ変換/統合する。
	*/
	switch (iError) {
		case E_IFS_Parity:				errorCode =  1;	break;
		case E_IFS_Frame:				errorCode =  2;	break;
		case E_IFS_Overrun:				errorCode =  3;	break;
		case E_IFS_PACKET_HeadStr:		errorCode =  4;	break;
		case E_IFS_PACKET_DataSize:		errorCode =  5;	break;
		case E_IFS_PACKET_WrongKind:	errorCode =  6;	break;
		case E_IFS_NAK_Retry:			errorCode =  7;	break;
		case E_IFS_NO_Answer:			errorCode =  8;	break;
		case E_IFS_PACKET_CRC:			errorCode =  9;	break;
		case E_IFS_PACKET_Headerless:	errorCode = 10;	break;
		case E_IF_Protocol:				errorCode = 11;	break;
		case E_IFS_PACKET_WrongSlave:	errorCode = 14;	break;

		case E_IFS_BUG:
		default:
			return;
	}

	/* 精算機へ通知する */
	PAYcom_NoticeError((uchar)(iSlave +1), errorCode, ucNow);
}

/*[]----------------------------------------------------------------------[]*/
/*|             EnQueue                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		キューへ挿入する。											       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	どのキューか？を指定		 						   |*/
/*|			 pQue	挿入するキューを指定								   |*/
/*|			 iWhere	どこに挿入するか？を指定（先頭から何番目）			   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	注）引数＝iWhereの意味											       |*/
/*|				 = 0：先頭へ挿入									       |*/
/*|				正値：中間へ挿入（０から数える位置。その位置の前に挿入）   |*/
/*|					  現在のキュー数以上の位置を指定すると、末尾に入れる   |*/
/*|				負値：末尾へ挿入									       |*/
/*|																	       |*/
/*|	例）現在のキュー数＝３で、０を指定すると、先頭へ挿入(新は１番目)	   |*/
/*|							  １を指定すると、２番目の前へ挿入(新は２番目) |*/
/*|							  ２を指定すると、３番目の前へ挿入(新は３番目) |*/
/*|							  ３を指定すると、末尾へ挿入(新は４番目)	   |*/
/*|							－１を指定すると、末尾へ挿入				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  早瀬                                                    |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :  2005-09-07     キュー保持中フラグの初期化を追加         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	EnQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pAdd, int iWhere)
{
struct t_IF_Queue	*pNow;
	int		ni;

	/* 現、ひとつもないときは（何処に入れるか？に関わらず）すべて同じ処理 */
	if (pCtl->usCount == 0) {
		pCtl->usCount = 1;
		pAdd->bQueue = 1;							/* キュー本体使用中へ */
		pAdd->pData->sCommon.bUsedByQue = 1;		/* キュー実体使用中へ */
		pAdd->pData->sCommon.bHold = 0;				/* キュー保持解除へ		2005.09.07 早瀬・追加 */
		pAdd->pPrev = NULL;							/* 自分の前はいない */
		pAdd->pNext = NULL;							/* 自分の次もいない */
		pCtl->pTop = pAdd;							/* 自分が先頭になる */
		pCtl->pEnd = pAdd;							/* 自分が末尾になる */
	}

	/* 現、ひとつ以上ある？*/
	else if (pCtl->usCount > 0) {

		/* 先頭に挿入？*/
		if (iWhere == 0) {
			pAdd->pPrev = NULL;						/* 自分の前はいない */
			pAdd->pNext = pCtl->pTop;				/* 自分の次は今までの先頭 */
			pCtl->pTop->pPrev = pAdd;				/* 今までの前が自分 */
			pCtl->pTop = pAdd;						/* 自分が先頭になる */
		}

		/* 末尾に挿入？*/
		else if (iWhere < 0 ||						/* 負値は「末尾」の意 */
				 iWhere >= pCtl->usCount)			/* 現在のキュー数以上 */
		{
			pAdd->pPrev = pCtl->pEnd;				/* 自分の前は今までの末尾 */
			pAdd->pNext = NULL;						/* 自分の次ははいない */
			pCtl->pEnd->pNext = pAdd;				/* 今までの次が自分 */
			pCtl->pEnd = pAdd;						/* 自分が末尾になる */
		}

		/* 中間に挿入 */
		else {
			pNow = pCtl->pTop;						/* 自分の次に来るべきもの(＝pNow)を探す */
			for (ni=0; ni < iWhere; ni++) {
				pNow = pNow->pNext;
			}										/* pNowの前に自分を挿入する */
			pAdd->pPrev = pNow->pPrev;				/* 自分の前は今までの前 */
			pAdd->pNext = pNow;						/* 自分の次は今までそのもの */
			pNow->pPrev->pNext = pAdd;				/* 今までの前の次が自分 */
			pNow->pPrev = pAdd;						/* 今までの前が自分 */
		}

		pCtl->usCount++;							/* キュー増やす */
		pAdd->bQueue = 1;							/* キュー本体使用中へ */
		pAdd->pData->sCommon.bUsedByQue = 1;		/* キュー実体使用中へ */
		pAdd->pData->sCommon.bHold = 0;				/* キュー保持解除へ		2005.09.07 早瀬・追加 */
	}

	/* ここに来たらバグ */
	else {
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             GetQueue                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		先頭のキューを取得。										       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	どのキューか？を指定		 						   |*/
/*| return : 		取得したキューへのポインタ	 						   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	注）戻り値＝0（NULLポインタ）の場合、取得できないことを意味する。      |*/
/*|		戻り値≠0（NULLポインタ）の場合、取得できてはいるが				   |*/
/*|										 指定のキューからは外されていない  |*/
/*|										 ので注意のこと。				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  早瀬                                                    |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
struct t_IF_Queue	*GetQueue(t_IF_Queue_ctl *pCtl)
{
	if (pCtl->usCount) {
		return pCtl->pTop;
	}

	return (struct t_IF_Queue *)0;
}

/*[]----------------------------------------------------------------------[]*/
/*|             CheckQueue                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		指定されたキューを探す。									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	どのキューか？を指定		 						   |*/
/*|			 pQue	探すキューを指定									   |*/
/*|			 iwhat	何を探すか？を指定									   |*/
/*| return : 		見つけたキューの位置（先頭から何番目）				   |*/
/*|			（ < 0）負値：見つからなかった							       |*/
/*[]----------------------------------------------------------------------[]*/
/*|	注）引数＝iwhatの意味											       |*/
/*|			 = 0：そのもの											       |*/
/*|			 = 1：													       |*/
/*|			 = 2：													       |*/
/*|			 = 3：													       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  早瀬                                                    |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
int		CheckQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pQue, int iWhere)
{
struct t_IF_Queue	*pNow;
	int		ni;

/*
*	「そのもの」の例
*/
	pNow = pCtl->pTop;
	for (ni=0; ; ni++) {
		if (pNow == pQue) {
			return ni;
		}
		if (!pNow->pNext) {
			return (-1);
		}
		pNow = pNow->pNext;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             DeQueue                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		指定されたキューを外す。									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	どのキューか？を指定		 						   |*/
/*|			 iWhere	どこから外すか？を指定（先頭から何番目）			   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	注）引数＝iWhereの意味											       |*/
/*|				 = 0：先頭を外す									       |*/
/*|				正値：中間を外す（０から数える位置。その位置を外す）	   |*/
/*|				（現在のキュー数－１）以上の位置を指定すると、末尾を外す   |*/
/*|				負値：末尾を外す									       |*/
/*|																	       |*/
/*|	例）現在のキュー数＝３で、０を指定すると、先頭(１番目)を外す		   |*/
/*|							  １を指定すると、２番目を外す				   |*/
/*|							  ２を指定すると、末尾(３番目)を外す		   |*/
/*|							  ３を指定すると、末尾を外す				   |*/
/*|							－１を指定すると、末尾を外す				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  早瀬                                                    |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :  2005-09-07     キュー保持中フラグの初期化を追加         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
struct t_IF_Queue	*DeQueue(t_IF_Queue_ctl *pCtl, int iWhere)
{
struct t_IF_Queue	*pDel;
	int		ni;

	/* 現、ひとつもないときは何もしない */
	if (pCtl->usCount == 0) {
		return (struct t_IF_Queue *)0;
	}

	/* 現、ひとつのときは（何処から外すか？に関わらず）すべて同じ処理 */
	if (pCtl->usCount == 1) {
		pDel = pCtl->pTop;						/* それまでの先頭が「外される対象」*/
		pCtl->usCount = 0;							/* キューなし */
		pCtl->pTop->bQueue = 0;						/* キュー本体未使用へ */
		pCtl->pTop->pData->sCommon.bUsedByQue = 0;	/* キュー実体未使用へ */
		pCtl->pTop->pData->sCommon.bHold = 0;		/* キュー保持解除へ		2005.09.07 早瀬・追加 */
		pCtl->pTop = NULL;							/* 先頭はいない */
		pCtl->pEnd = NULL;							/* 末尾はいない */
	}

	/* 現、ふたつ以上ある？*/
	else if (pCtl->usCount > 1) {
		/* 先頭を外す？*/
		if (iWhere == 0) {
			pDel = pCtl->pTop;						/* それまでの先頭が「外される対象」*/
			pCtl->pTop = pDel->pNext;				/* 新しい先頭は「それまでの先頭」の次 */
			pCtl->pTop->pPrev = NULL;				/* 新しい先頭の前はいない */
		}
		/* 末尾を外す？*/
		else if (iWhere < 0 ||						/* 負値は「末尾」の意 */
				 iWhere >= (pCtl->usCount-1))		/*（現在のキュー数－１）以上 */
		{
			pDel = pCtl->pEnd;						/* それまでの末尾が「外される対象」*/
			pCtl->pEnd = pDel->pPrev;				/* 新しい末尾は「それまでの末尾」の前 */
			pCtl->pEnd->pNext = NULL;				/* 新しい末尾の次はいない */
		}
		/* 中間を外す？*/
		else {
			pDel = pCtl->pTop;						/*「外される対象」は、*/
			for (ni=0; ni < iWhere; ni++) {			/* 先頭から[iWhere]番目 */
				pDel = pDel->pNext;					/*「外される対象」を自分とし、自分を基準にすると */
			}										/* 上から、直前、自分、直後となる */
			(pDel->pPrev)->pNext = (pDel->pNext);	/* 直前の次がいままでは自分だが、これからは直後となる */
			(pDel->pNext)->pPrev = (pDel->pPrev);	/* 直後の前がいままでは自分だが、これからは直前となる */
		}

		pCtl->usCount--;							/* キュー減らす */
		pDel->bQueue = 0;							/* キュー本体未使用へ */
		pDel->pData->sCommon.bUsedByQue = 0;		/* キュー実体未使用へ */
		pDel->pData->sCommon.bHold = 0;				/* キュー保持解除へ		2005.09.07 早瀬・追加 */
	}

	/* ここに来たらバグ */
	else {
	}

	return pDel;
}

/*[]----------------------------------------------------------------------[]*/
/*|             GetQueueBuff() 				   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		空きキュー本体を探す											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pQ			どのキュー本体か？を指定						   |*/
/*|			 byteSize	キュー本体(配列)サイズをバイト数で指定			   |*/
/*| return : 見つけたキュー本体											   |*/
/*|			 ≠0：見つけたキュー本体へのポインタ					       |*/
/*|			 ＝0：見つからなかった									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
struct t_IF_Queue	*GetQueueBuff(struct t_IF_Queue *pQ, ushort byteSize)
{
	int		ni;
	ushort	max;

	max = (ushort)(byteSize / sizeof(struct t_IF_Queue));

	for (ni=0; ni < max; ni++) {
		if (pQ->bQueue == 0) {
			return pQ;
		}
		pQ++;
	}

	return (struct t_IF_Queue *)0;
}

/*[]----------------------------------------------------------------------[]*/
/*|             GetQueueBody() 				   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		空きキュー実体を探す											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pQ			どのキュー実体か？を指定						   |*/
/*|			 byteSize	キュー実体(配列)サイズをバイト数で指定			   |*/
/*| return : 見つけたキュー実体											   |*/
/*|			 ≠0：見つけたキュー実体へのポインタ					       |*/
/*|			 ＝0：見つからなかった									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
t_IFM_Queue	*GetQueueBody(t_IFM_Queue *pQ, ushort byteSize)
{
	int		ni;
	ushort	max;

	max = (ushort)(byteSize / sizeof(t_IFM_Queue));

	for (ni=0; ni < max; ni++) {
		if (pQ->sCommon.bUsedByQue == 0) {
			return pQ;
		}
		pQ++;
	}

	return (t_IFM_Queue *)0;
}


/*[]----------------------------------------------------------------------[]*/
/*|             EnQueue2                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		キューへ挿入する。											       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	どのキューか？を指定		 						   |*/
/*|			 pQue	挿入するキューを指定								   |*/
/*|			 iWhere	どこに挿入するか？を指定（先頭から何番目）			   |*/
/*|			 bRetry	TRUE：リトライ／FALSE：新規							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2006-10-19                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	EnQueue2(t_LCMque_Ctrl_Action *pCtl, struct t_IF_Queue *pAdd, int iWhere, BOOL bRetry)
{
	if (bRetry) {
		EnQueue(&pCtl->Retry, pAdd, iWhere);
	} else {
		EnQueue(&pCtl->New, pAdd, iWhere);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             DeQueue2                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		送信用に指定されたキューを外す								       |*/
/*|		(※現在LCMque_Ctrl_Action専用)								       |*/
/*|		新規キュー→リトライキューの順でサーチ						       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	どのキューか？を指定		 						   |*/
/*|			 iWhere	どこから外すか？を指定（先頭から何番目）			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2005-10-19                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
struct t_IF_Queue	*DeQueue2(t_LCMque_Ctrl_Action *pCtl)
{
	struct t_IF_Queue	*pQbuff;
	t_IFM_Queue			*pQbody;
	int		del;
	int		index;
	struct t_IF_Queue* pPrev;

	// 新規キューをサーチ
	pQbuff = GetQueue(&pCtl->New);
	if (pQbuff) {
		index = 0;
		while(pQbuff) {
			if(IFM_Can_Action(LKcom_Search_Ifno(pQbuff->pData->sCommon.ucTerminal + 1))) {
		// ロック装置制御要求の場合のみ、
		// 新規要求送信先ロック装置に対して、リトライキューにも要求があればそれを削除
		// ※現状本関数を呼ぶのはロック装置に対する制御時のみなので無条件に以下の処理を行う
		pQbody = pQbuff->pData;
		// リトライキューをサーチ
		while (1) {
			del = MatchRetryQueue_Lock(&pCtl->Retry, pQbody->sW57.sCommon.ucTerminal, pQbody->sW57.ucLock);
			// ロック装置が一致ならばその要求を削除
			if (del >= 0) {
				DeQueue(&pCtl->Retry, del);
			}
			if (del == -1) break;
		}
		// 関連する再送タイマストップ
				LCMtim_1secTimerStop((int)pQbuff->pData->sW57.sCommon.ucTerminal, (int)pQbody->sW57.ucLock);
				DeQueue(&pCtl->New, index);
				return pQbuff;
			}
			pPrev = pQbuff;
			pQbuff = pPrev->pNext;
			++index;
		}
	}
	// リトライキューをサーチ
	pQbuff = DeQueue(&pCtl->Retry, 0);

	return pQbuff;
}

/*[]----------------------------------------------------------------------[]*/
/*|             RemoveQueue2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		最古の要求削除用に指定されたキューを外す					       |*/
/*|		(※現在LCMque_Ctrl_Action専用)								       |*/
/*|		リトライキュー→新規キューの順でサーチ						       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	どのキューか？を指定		 						   |*/
/*|			 iWhere	どこから外すか？を指定（先頭から何番目）			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2005-10-19                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
struct t_IF_Queue	*RemoveQueue2(t_LCMque_Ctrl_Action *pCtl, int iWhere)
{
	struct t_IF_Queue	*pQbuff;

	// リトライキューをサーチ
	pQbuff = DeQueue(&pCtl->Retry, iWhere);
	if (pQbuff) {
		return pQbuff;
	}
	// 新規キューをサーチ
	pQbuff = DeQueue(&pCtl->New, iWhere);

	return pQbuff;
}


/*[]----------------------------------------------------------------------[]*/
/*|             MatchRetryQueue_Lock                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		LCMリトライキューから、指定されたロック装置への制御電文要求を      |*/
/*|		サーチし、先頭からの順番を返す(0～)							       |*/
/*|		見つからなければ-1を返す									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : ucSlave	子機番号				 						   |*/
/*|			 ucLock		ロック装置番号									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2005-10-26                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
int MatchRetryQueue_Lock(t_IF_Queue_ctl *pCtl, uchar ucSlave, uchar ucLock)
{
	struct t_IF_Queue *pSearch;
	int current = 0;

	if (pCtl->usCount != 0) {
		pSearch = pCtl->pTop;
		while (current < pCtl->usCount) {
			if ((pSearch->pData->sW57.sCommon.ucTerminal == ucSlave) &&
				(pSearch->pData->sW57.ucLock == ucLock)) {
				// 一致
				return current;
			}
			pSearch = pSearch->pNext;
			current++;
		}
	}
	return -1;
}

/*[]----------------------------------------------------------------------[]*/
/*|             PAYcom_NoticeError()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		精算機へのエラーデータ通知										   |*/
/*|		・対精算機応答待ちキューへ「エラーデータ」をぶち込む。			   |*/
/*|		・初期設定データが来てからに限る（来る以前は通知せず）			   |*/
/*|		・「キューフル」時は、古いものを捨て、新しいものを残す			   |*/
/*|		・IFM_MasterError(IFM.c) および、								   |*/
/*|		  IFM_SlaveError(IFM.c) から、呼び出される						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-21                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PAYcom_NoticeError(uchar ucTermNo, uchar ucError, uchar ucState)
{
	t_IFM_mail *pQbody;
	ushort len;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];		// このバッファにはID4からセットする。

	// キュー実体へ「エラーデータ」をセット 
	pQbody->s66.sCommon.ucKind    = 0x66;			// データ種別は「(66H)エラーデータ」固定 
	pQbody->s66.sCommon.bHold     = 1;				// データ保持フラグ
	pQbody->s66.sCommon.t_no  = (uchar)ucTermNo;		// 0からの端末Noを設定
	pQbody->s66.ucErrTerminal  = ucTermNo;			// 端末情報：親なら =0 固定、子なら =1～15
	pQbody->s66.ucErrCode      = ucError;			// エラーコードは引数しだい 
	pQbody->s66.ucErrState     = ucState;			// エラー状態も引数しだい 
	len = 6;
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);		// アプリ側が受信するバッファにセット
}

/*[]----------------------------------------------------------------------[]*/
/*|             Seach_Close_command_sameno                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		LCMキューの先頭から制御電文要求をサーチし、(フラップ上昇)指示      |*/
/*|		いる電文を見つけかつ同一車室の場合、先頭からの順番を返す(0～)      |*/
/*|		見つからなければ-1を返す									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : tno	ターミナルNo										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2014-01-20                                              |*/
/*[]------------------------------------- Copyright(C) 20013 AMANO Corp.---[]*/
int Seach_Close_command_sameno(t_IF_Queue_ctl *pCtl,uchar tno)
{
	struct t_IF_Queue *pSearch;
	int current = 0;

	if (pCtl->usCount != 0) {
		pSearch = pCtl->pTop;
		while (current < pCtl->usCount) {
			if(pSearch->pData->sW57.ucLock == 0) {// フラップ装置
				if(pSearch->pData->sW57.unCommand.Byte == (uchar)'4'){ // フラップ上昇の場合
					if(pSearch->pData->sW57.sCommon.ucTerminal == tno){// 同一ターミナルNo
						// 一致
						return current;
					}
				}
			}
			pSearch = pSearch->pNext;
			current++;
		}
	}
	return -1;
}
/*[]----------------------------------------------------------------------[]*/
/*|             Seach_Close_command                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		LCMキューから、制御電文要求をサーチし、(フラップ上昇)を指示して    |*/
/*|		いる電文を見つけ、先頭からの順番を返す(0～)					       |*/
/*|		見つからなければ-1を返す									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : scount	キューの何番目から検索するか0～(0が先頭)			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2013-11-28                                              |*/
/*[]------------------------------------- Copyright(C) 20013 AMANO Corp.---[]*/
int Seach_Close_command(t_IF_Queue_ctl *pCtl,ushort scount)
{
	struct t_IF_Queue *pSearch;
	int current = 0;
	ushort i;

	if (pCtl->usCount != 0) {
		pSearch = pCtl->pTop;
		for(i = 0 ; i < scount ; i++){// キューの何番目からサーチするか
			pSearch = pSearch->pNext;
			current++;
		}
		while (current < pCtl->usCount) {
			if(pSearch->pData->sW57.ucLock == 0) {// フラップ装置
				if(pSearch->pData->sW57.unCommand.Byte == (uchar)'4'){ // フラップ上昇の場合
					// 一致
					return current;
				}
			}
			pSearch = pSearch->pNext;
			current++;
		}
	}
	return -1;
}
/*[]----------------------------------------------------------------------[]*/
/*|             Get_Close_command_cnt                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		LCMキューから、制御電文要求をサーチし、(上昇／ロック閉)を指示して  |*/
/*|		いる電文を見つけ、その個数を返す(0～)							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2013-11-28                                              |*/
/*[]------------------------------------- Copyright(C) 20013 AMANO Corp.---[]*/
int Get_Close_command_cnt(t_IF_Queue_ctl *pCtl)
{
	struct t_IF_Queue *pSearch;
	int count = 0;
	int step = 0;

	if (pCtl->usCount != 0) {
		pSearch = pCtl->pTop;
		while (step < pCtl->usCount) {
			if(pSearch->pData->sW57.ucLock == 0) {// フラップ装置
				if(pSearch->pData->sW57.unCommand.Byte == (uchar)'4'){ // フラップ上昇の場合
					count++;	// 一致
				}
			}
			pSearch = pSearch->pNext;
			step++;
		}
	}
	return count;
}




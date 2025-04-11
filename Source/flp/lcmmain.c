/*[]----------------------------------------------------------------------[]*/
/*|		Lock Management task in New I/F(Master)							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"mem_def.h"
#include	"mnt_def.h"
#include	"flp_def.h"
#include	"toS.h"
#include	"IFM.h"
#include	"LCM.h"
#include	"LCMdata.h"


/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/
#define		LCM_FLAP_MAKER_COUNT	2


/*----------------------------------*/
/*			area define				*/
/*----------------------------------*/

/* 何れどこかに移動すること。*/
BOOL	IFM_bLockAction[IF_SLAVE_MAX][IFS_ROOM_MAX];
const	struct	LOCKMAKER_REC	LCMFlapmaker_rec[LCM_FLAP_MAKER_COUNT] = {
	{	15,	{	4,	4,	1,	12,	14, 14	}	},		// サニカ
// フラップ上昇/下降時間はフラップの上昇時間 14s＋リトライ間隔 12s＋上昇(リトライ) 12s ＋αを合計し で 42sとした
// しかし、42sでは長すぎと判断し、協議の結果15sへ変更する
// 本体からのリトライ指示はしない(フラップ側でやってくれる)ため リトライは0回とする
	{	LK_TYPE_CONTACT_FLAP,	{	4,	4,	0,	12,	15,	15	}	},		// 接点フラップ
};


/*----------------------------------*/
/*		external function define	*/
/*----------------------------------*/


/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	LCM_init(void);

void	LCM_main(void);

void	LCM_ForwardSlave(void);					// 子機へ転送
void	LCM_SlaveStatus(t_IFM_Queue *pStatus);	// 変化ありなら精算機へ応答
void	LCM_FlapStatus(t_IFM_Queue *pStatus);	// 変化ありなら精算機へ応答

void	LCM_NoticeStatus_toPAY( uchar tno, uchar ucLock, uchar ucCarState, uchar ucLockState);
void	LCM_ControlLock(uchar bMode, uchar bLockOff, uchar ucSlave, uchar ucLock, uchar ucMaker);	/* ロック装置制御 */

void	LCM_RequestVirsion(void);
void	LCM_RequestVirsion_FLAP(void);
void	LCM_Request_CRRTEST( uchar );
BOOL	LCM_WaitingLock(uchar ucOperation, uchar ucSlave, uchar ucLock, uchar ucMaker, BOOL bRetry, t_LKcomCtrl *c_dp);
uchar	LCM_Is_LockOnTrouble(BOOL *pbRetry, uchar ucSensor, uchar ucLastStatus, uchar ucMaker);
uchar	LCM_Is_LockOffTrouble(uchar *pucState, uchar ucSensor, uchar ucLastStatus, uchar ucMaker);
void	LCM_NextAction(void);
void	LCM_TimeoutAction(void);
void	LCM_TimeoutTrouble(void);
void	LCM_TimerStartAction(uchar ucSlave, uchar ucLock, ushort usAction, BOOL bLockOff);

uchar	LCM_LockState_Initial(uchar ucNow, uchar ucMaker);
uchar	LCM_LockState_bySensor(uchar *pucLastState, uchar ucMaker, uchar ucLast, uchar ucNow, uchar *piRecover);
uchar	LCM_CarState_bySensor(uchar ucNow, uchar ucMaker, int iSlave, int iLock);
uchar	LCM_CarState_Initial(uchar ucNow, uchar ucMaker);

BOOL	LCM_Is_AllOver(uchar ucOperation, uchar ucSlave, uchar ucLock);
void	LCM_TimeoutTest(void);
void	LCM_TestResult(uchar lockState, int slave, int lock);
BOOL	LCM_Is_LockDirNow(uchar ucSlave, uchar ucLock);
uchar	LCM_do_ActionNow(int iSlave, int iLock);
void	LCM_CountAction(uchar lockState, int slave, int lock);

void	IFM_SetAction(uchar ucSlave, uchar ucLock);
void	IFM_ResetAction(uchar ucSlave, uchar ucLock);
BOOL	IFM_Is_Action(uchar ucSlave, uchar ucLock);
uchar	IFM_Get_ActionTerminalNo(void);
ushort	LCM_GetLimitAction(ushort usSigalOutTime);
ushort	LCM_GetLimitOneTest(ushort usLockOnTime, ushort usLockOffTime);

void LCM_TimeoutOperation(void);

void	LCM_NoticeStatusall_toPAY( void );
void	PAYcom_InitDataSave(void);
void	LCM_AnswerQueue_toPAY63(uchar type);
void	LCM_AnswerQueue_toPAY62(void);
void	LCM_AnswerQueue_toPAY64(void);
void	LCM_AnswerQueue_Timeout(void);
void	LCM_ForwardFlapTerminal(void);
void	LCM_NoticeFlapStatus_toPAY(void);
void	LCM_RequestLoopData(void);
void	LCM_RequestForceLoopControl(uchar ucOperation, uchar ucSlave);
uchar	LCM_GetFlapMakerParam(uchar type, t_LockMaker* pMakerParam);

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_init()  	                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|          ロック管理タスク初期化                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_init(void)
{

	/* タイマー･カウンタクリア */
	LCM_TimerInit();			/* timer counter clear */

	/* これでインターバルタイマがカウントし始める。*/
	LCM_f_TmStart = 1;			/* timer process start */
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_main() 	                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|          ロック管理タスク・メイン                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  20111-11-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_main(void)
{
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	int				ni;
	BOOL			bOneQueue;
	t_IFM_Queue*	pIFMQueue;

	// 一回に一キューの処理なら「TRUE(=1)」を、
	//	一回に全キューの処理なら「FALSE(=0)」をセットしてください。

	bOneQueue = 1;		/*（お試し）一回に一キューの処理とする */

	//	子機からの子機状態データを解析する。
	for (ni=0; ni < toSque_Ctrl_Status.usCount; ni++) {	/* 子機状態リストにキューあり？*/
		/* 子機通信タスクが受信した「子機状態データ」を、キュー経由で取り出す */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);		/* キュー(先頭)を外す */
		if (!pQbuff) {
			break;
		}
		pIFMQueue = pQbuff->pData;
		if( IFS_CRR_OFFSET > pIFMQueue->sCommon.ucTerminal ){	// CRR基板へのアクセスではない
			if( LKcom_Search_Ifno((uchar)(pIFMQueue->sCommon.ucTerminal + 1)) == 0 ){	// 該当ターミナルNoの接続はロック装置
				LCM_SlaveStatus(pQbuff->pData);	/* 変化ありなら精算機へ応答 */
			}
			else {		// フラップ
				LCM_FlapStatus(pQbuff->pData);	/* 変化ありなら精算機へ応答 */
			}
		} else {												// CRR基板へのアクセスの場合、フラップへ飛ばす
			LCM_FlapStatus(pQbuff->pData);	/* 変化ありなら精算機へ応答 */
		}

		/* 一回に一キューの処理なら、ここで抜ける */
		if (bOneQueue) {
			break;
		}
	}

	//	精算機からのロック装置制御要求を解析する。
	// 要求バッファにデータありかつ送信完了
	if( 0 < LKcomTerm.CtrlInfo.Count ){				// バッファにデータ有
		LCM_ForwardSlave();							// 子機へ転送
		LKcom_SndDtDec();							// バッファ1件削除
	}

	/*
	*	ロック装置タイムアウト処理
	*	・動作完結タイムアウト
	*	・故障リトライ間隔タイムアウト
	*/
	LCM_TimeoutAction();		/* 動作完結タイムアウト */
	LCM_TimeoutTrouble();		/* 故障リトライ間隔タイムアウト */
	LCM_TimeoutOperation();		// バージョン要求、ロック装置開閉テストのタイムアウト処理

}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_ForwardSlave()                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*|		精算機から受信したコマンドを子機へ転送する。					   |*/
/*|		・対精算機通信タスク経由で、コマンドはメール受信する。			   |*/
/*|		・対子機通信タスク経由で、転送はメール発信する。				   |*/
/*|		・精算機からの「ロック装置制御データ」を						   |*/
/*|		  子機へは「状態書き込みデータ」へと変換する。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		・引数は、メール実体アドレス				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_ForwardSlave(void)
{
	int		slave, lock;
	uchar	maker, doTest;
	uchar	carState, lockState;
	ushort	limitSecond, lockOnTime, lockOffTime;
	ushort	index;						// LockInfoインデックス
	uchar type;
	t_LKcomCtrl		*c_dp;
	c_dp = &LKcomTerm.CtrlData[LKcomTerm.CtrlInfo.R_Index];

	/* 処理区分																	*/
	/*		 1 ロック装置閉（ロックする）										*/
	/*		 2 ロック装置開（ロック解除）										*/
	/*		 3 接続している全てのロック装置、およびセンサーの状態要求			*/
	/*		 4 指定したロック装置、およびセンサーの状態要求						*/
	/*		 5 接続ロック装置全て閉（ロックする）								*/
	/*		 6 接続ロック装置全て開（ロック解除）								*/
	/*		 7 メンテナンス情報要求												*/
	/*		 8 ロック装置開閉テスト要求（全て）									*/
	/*		 9 ロック装置開閉テスト要求（指定）									*/
	/*		10 動作カウンタークリア（全て）										*/
	/*		11 動作カウンタークリア（指定）										*/
	/*		12 バージョン要求							2005.04.08 早瀬・追加	*/
	/*		13 エラー状態要求							2005.04.08 早瀬・追加	*/

// 強制的にタイムアウトを起こし、処理を終了させる
	if (IFM_LockTable.ucOperation != 0) {
		// ここに入るのは8、9、12のときのみ(他はキューイング/送信時にクリアしているため)
		LCMtim_1secWaitStart(0);	// 0セットで強制的にタイムアウトを発生させる
		LCM_TimeoutOperation();		// タイムアウト処理
	}

	IFM_LockTable.ucOperation = c_dp->kind;
	switch (IFM_LockTable.ucOperation) {
		//子機へ転送する
		case 1:		/* ロック装置閉（ロックする）*/
		case 2:		/* ロック装置開（ロック解除）*/
			/* 車室のチェック */
			if (c_dp->lock >= IFS_ROOM_MAX) {
				IFM_MasterError(E_IFM_CTRL_Command, E_IF_BOTH);		/* 制御データ異常[同時] */
				break;
			}
			/* ロック装置メーカーのチェック */
			slave = c_dp->tno-1;												// CRBのターミナルNo
			lock  =(uchar)(c_dp->lock-1);										// CRBのロック装置番号(1～6)
			maker = IFM_LockTable.sSlave[slave].sLock[lock].ucConnect;
			if (maker == 0 ||
				maker >= LOCK_MAKER_END)
			{
				IFM_MasterError(E_IFM_CTRL_Command, E_IF_BOTH);		/* 制御データ異常[同時] */
				break;
			}
			/* ロック制御できるか？否かを判定し、否なら待たせる処理（キューイング）*/
			LCM_WaitingLock( (uchar)c_dp->kind,									// 操作
						     (uchar)slave,										// CRBのターミナルNo
						     (uchar)lock,										// CRBのロック装置番号(1～6)
						     maker,												// ロック装置メーカー
						     0, c_dp);											// リトライなし、個別制御
			break;

		case 5:		/* 接続ロック装置全て閉（ロックする）*/
		case 6:		/* 接続ロック装置全て開（ロック解除）*/
			// フラップまたはロックの最初の端末No.が設定されているので種別を取得する
			type = LKcom_Search_Ifno(c_dp->tno);
			for (slave=0; slave < IF_SLAVE_MAX; slave++) {						// ターミナルNoごと
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRBぼロック装置番号ごと
					WACDOG;										// 装置ループの際はWDGリセット実行
					if (IFM_LockTable.sSlave[slave].sLock[lock].ucConnect) {
						if (type == LKcom_Search_Ifno(slave + 1)) {
							// 上昇／下降コマンドなら現在の状態をチェックする
							for( index = INT_CAR_START_INDEX; index < LOCK_MAX; ++index ){
								WACDOG;										// 装置ループの際はWDGリセット実行
								if( 0 == LKcom_Search_Ifno(slave + 1) ){	// ロックの場合 
									if( (LockInfo[index].if_oya == (slave + 1)) && (LockInfo[index].lok_no == (lock + 1))){
										break;									// 対象のLockInfoインデックス発見
									}
								} else {									// フラップの場合 LockInfo[index].lok_no は「1」と読み替える
									if( (LockInfo[index].if_oya == (slave + 1)) && ( 1 == (lock + 1))){
										break;									// 対象のLockInfoインデックス発見
									}
								}
							}
							if(index >= LOCK_MAX){
								index = LOCK_MAX - 1;// forループが最後まで回った時LCM_CanFlapCommand()で範囲外アクセスしないようブロックする
							}
							if(c_dp->kind == 5) {							// 全上昇
								if(FALSE == LCM_CanFlapCommand(index, 1)) {			// 上昇コマンド実行不可?
									// 現在の状態が上昇動作で上昇済みの場合は、上昇コマンドを作成しない
									continue;
								}
							}
							else {											// 全下降
								if(FALSE == LCM_CanFlapCommand(index, 0)) {			// 上昇コマンド実行不可?
									// 現在の状態が下降動作で下降済みの場合は、下降コマンドを作成しない
									continue;
								}
							}
						LCM_WaitingLock( c_dp->kind, 
									     (uchar)slave, 
									     (uchar)lock, 
									     IFM_LockTable.sSlave[slave].sLock[lock].ucConnect, 
									     0, (t_LKcomCtrl *)0);
						}
					}
				}
			}
			break;

		case 8:		/* ロック装置開閉テスト要求（全て）*/
			// テスト対象の車室を予約する（テスト結果の初期化も含む）
			
			for (slave=0; slave < IF_SLAVE_MAX; slave++) {						// ターミナルNoごと
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRBぼロック装置番号ごと
					if (IFM_LockTable.sSlave[slave].sLock[lock].ucConnect) {
						IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = c_dp->kind;			// 予約
						IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;						// 結果はクリアしとく
					}
					else {
						IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 3;						// 結果は「接続無し」で確定
					}
				}
			}
			// 開閉テスト可能な最初の車室探し・・・見つかれば、帰ってこない（動作要求して即リターン）

			for (slave=0; slave < IF_SLAVE_MAX; slave++) {						// ターミナルNoごと
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRBぼロック装置番号ごと
					WACDOG;																		// 装置ループの際はWDGリセット実行

					// 開閉テスト対象外は除外
					if (IFM_LockTable.sSlave[slave].sLock[lock].ucOperation != c_dp->kind) {
						continue;
					}
					// いま開閉テストできる？
					maker = IFM_LockTable.sSlave[slave].sLock[lock].ucConnect;
					doTest = LCM_do_ActionNow(slave, lock);
					if (doTest == 1) {
						// 見つかったら「その車室」から始め、継続は動作後に判定する
						LCM_WaitingLock(c_dp->kind, (uchar)slave, (uchar)lock, maker, 0, (t_LKcomCtrl *)0);

						//	開閉テストはここから開始
						//	・開閉テスト中をセット
						//	・いま開閉テストすべきロック装置の制限時間を決める。
						//	※次のロック装置の制限時間は、その都度決めてタイマー再開する手法にする。
						//	  その都度とは、LCM_Is_AllOver()にて、次のテスト対象ロック装置を探し出したとき。
						//
						IFM_LockTable.bTest = 1;

						lockOnTime = IF_SignalOut[maker].usLockOnTime;
						lockOffTime = IF_SignalOut[maker].usLockOffTime;
						limitSecond = LCM_GetLimitOneTest(lockOnTime, lockOffTime);
						LCMtim_1secWaitStart(limitSecond);

						// 上記で動作要求したら、もう戻らない
						return;
					}
					// いま開閉テストできないものは、未実施 
					IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = 0;		// 開閉テスト予約をキャンセル
					IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;		// 結果は「指定外」で確定
				}
			}

			/*
			*	ここに来たら、ひとつも開閉テスト未実施ということ。
			*	・ひとつもロック装置が「接続なし」
			*	・ひとつの子機へも通信できない
			*/
			/* 開閉テスト結果通知は「タイムアウト」処理にて */
			LCMtim_1secWaitStart(0);	/* ０セットして強引にタイムアウトを起こさせる */
			break;

		case 9:		/* ロック装置開閉テスト要求（指定）*/
			/* テスト結果を初期化 */
			for (slave=0; slave < IF_SLAVE_MAX; slave++) {						// ターミナルNoごと
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRBぼロック装置番号ごと
					WACDOG;										// 装置ループの際はWDGリセット実行
					if (IFM_LockTable.sSlave[slave].sLock[lock].ucConnect) {
						IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;					// 結果はクリアしとく
					}
					else {
						IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 3;					// 結果は「接続無し」で確定
					}
				}
			}
			/* テスト対象の車室か？なら予約する */
			slave = c_dp->tno-1;												// CRBのターミナルNo
			lock  =(uchar)(c_dp->lock-1);										// CRBのロック装置番号(1～6)
			if (IFM_LockTable.sSlave[slave].sLock[lock].ucConnect) {
				IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = c_dp->kind;	/* 予約 */
				IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;			/* 結果はクリアしとく */

				/* いま開閉テストできる？*/
				maker = IFM_LockTable.sSlave[slave].sLock[lock].ucConnect;
				doTest = LCM_do_ActionNow(slave, lock);
				if (doTest == 1) {
					/* できるなら「その車室」を動作させ、継続は動作後に判定する */
					LCM_WaitingLock( c_dp->kind,								// 操作
						     (uchar)slave,										// CRBのターミナルNo
						     (uchar)lock,										// CRBのロック装置番号(1～6)
						     maker,												// ロック装置メーカー
						     0, c_dp);											// リトライなし、個別制御
					/*
					*	開閉テストはここから開始
					*	・開閉テスト中をセット
					*	・全ての応答が返されるまでの制限時間を決める
					*/
					/*
					*	開閉テストは、このロック装置のみに対して。
					*	・開閉テスト中をセット
					*	・この開閉テストすべきロック装置の制限時間を決める。
					*	※開/閉なので２回分の動作時間が必要。
					*/
					IFM_LockTable.bTest = 1;

					/* 電文上の信号出力時間が範囲外なら、補正する（施錠）*/
					lockOnTime  = c_dp->clos_tm;
					if (lockOnTime == 0) {
						lockOnTime = IF_SignalOut[maker].usLockOnTime;
					}
					/* 電文上の信号出力時間が範囲外なら、補正する（開錠）*/
					lockOffTime = c_dp->open_tm;
					if (lockOffTime == 0) {
						lockOffTime = IF_SignalOut[maker].usLockOffTime;
					}
					limitSecond = LCM_GetLimitOneTest(lockOnTime, lockOffTime);
					LCMtim_1secWaitStart(limitSecond);

					/* 上記で動作要求したら、もう戻らない */
					return;
				}
				/* いま開閉テストできないものは、未実施 */
				else {
					IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = 0;	/* 開閉テスト予約をキャンセル */
					IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;		/* 結果は「指定外」で確定 */
				}
			}
			/*
			*	ここに来たら、開閉テスト未実施ということ。
			*	・指定ロック装置が「接続なし」
			*	・指定ロック装置の子機へ通信できない
			*/
			/* 開閉テスト結果通知は「タイムアウト」処理にて */
			LCMtim_1secWaitStart(0);	/* ０セットして強引にタイムアウトを起こさせる */
			break;

		//ロック管理テーブルから拾って、対精算機応答キューへ挿入
		case 3:		/* 接続している全てのロック装置、およびセンサーの状態要求 */
			if( LKcom_Search_Ifno( c_dp->tno ) == 0 ){	// 該当ターミナルNoの接続はロック装置
				LCM_NoticeStatusall_toPAY();
			}
			break;
		case 4:		/* 指定したロック装置、およびセンサーの状態要求 */
			slave = c_dp->tno-1;												// CRBのターミナルNo
			lock  =(uchar)(c_dp->lock-1);										// CRBのロック装置番号(1～6)
			carState  = IFM_LockTable.sSlave[slave].sLock[lock].ucCarState;
			lockState = IFM_LockTable.sSlave[slave].sLock[lock].ucLockState;
			LCM_NoticeStatus_toPAY(c_dp->tno,c_dp->lock, carState, lockState);
			break;
		case 10:	/* 動作カウンタークリア（全て）*/
			// フラップまたはロックの最初の端末No.が設定されているので種別を取得する
			type = LKcom_Search_Ifno(c_dp->tno);
			for (slave=0; slave < IF_SLAVE_MAX; slave++) {										// CRBのターミナルNoごと
				if(type == LKcom_Search_Ifno((uchar)(slave + 1))) {
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRBぼロック装置番号ごと
					WACDOG;													// 装置ループの際はWDGリセット実行
					IFM_LockAction.sSlave[slave].sLock[lock].ulAction  = 0;	/* 開閉動作の合計回数 */
					IFM_LockAction.sSlave[slave].sLock[lock].ulManual  = 0;	/* I/F盤SWによる手動での開・閉動作合計回数 */
					IFM_LockAction.sSlave[slave].sLock[lock].ulTrouble = 0;	/* 故障した合計回数 */
				}
				}
			}
			break;
		case 11:	/* 動作カウンタークリア（指定）*/
			slave = c_dp->tno-1;												// CRBのターミナルNo
			lock  =(uchar)(c_dp->lock-1);										// CRBのロック装置番号(1～6)
			IFM_LockAction.sSlave[slave].sLock[lock].ulAction  = 0;		/* 開閉動作の合計回数 */
			IFM_LockAction.sSlave[slave].sLock[lock].ulManual  = 0;		/* I/F盤SWによる手動での開・閉動作合計回数 */
			IFM_LockAction.sSlave[slave].sLock[lock].ulTrouble = 0;		/* 故障した合計回数 */
			break;
		case 7:		/* メンテナンス情報要求 */
			type = LKcom_Search_Ifno(c_dp->tno);
			LCM_AnswerQueue_toPAY63(type);										// (63H)メンテナンス情報応答
			break;

		case 12:	/* バージョン要求 */
			if( c_dp->tno >= IFS_CRR_OFFSET ){	// CRR基板(フラップ)に対してのバージョン要求
				LCM_RequestVirsion_FLAP();
			} else {
				if( LKcom_Search_Ifno(c_dp->tno) == 0 ){	// バージョン要求はロック装置のみ
					LCM_RequestVirsion();
				}
			}
			break;

		case 13:			// エラー状態要求
			// 未対応
			break;
		case 14:			// 制御ポート検査（全て）
			LCM_Request_CRRTEST(c_dp->tno);
			break;
		case 15:			// ループデータ要求
			if( LKcom_Search_Ifno(c_dp->tno) == 1 ){	// ループデータ要求はフラップのみ
				LCM_RequestLoopData();
			}
			break;
		case 16:			// ループ強制ON
		case 17:			// ループ強制OFF
			LCM_RequestForceLoopControl(c_dp->kind, (uchar)(c_dp->tno - 1));
			break;

		default:
			IFM_MasterError(E_IFM_CTRL_Command, E_IF_BOTH);		/* 制御データ異常[同時] */
			break;
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_WaitingLock()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		ロック制御可能判定                                                 |*/
/*|		・ロック制御できるか？否かを判定する。                             |*/
/*|		  できるなら、子機へ要求するため子機通信タスクへメール送信。       |*/
/*|		  否なら、待たせるため「ロック動作動作完結待ちリスト」へ           |*/
/*|		          キューイングする。                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-04-29                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL LCM_WaitingLock(uchar ucOperation, uchar ucSlave, uchar ucLock, uchar ucMaker, BOOL bRetry, t_LKcomCtrl *c_dp)
{
t_IFM_Queue			*pQbody;
struct t_IF_Queue	*pQbuff;
	ushort			lockOnTime, lockOffTime;
	uchar			action;
	BOOL			bRet;
	BOOL			bLockOff;
	BOOL			bQfull;			/* 2005.07.12 早瀬・追加 */
	int		del;
	int		scount,in_count;
	t_IFM_Queue	quesave;

	/* どっち方向への動作要求か？を「処理区分」から判定 */
	switch (ucOperation) {
		case 1:		/* ロック施錠 */
		case 5:		/* 全ロック施錠 */
			bLockOff = 0;
			break;
		case 2:		/* ロック開錠 */
		case 6:		/* 全ロック開錠 */
			bLockOff = 1;
			break;
		case 8:		/* 全ロック開閉テスト */
		case 9:		/* 個別ロック開閉テスト */
			/* 開閉テストは、現状態の逆方向動作から始める */
			if (LCM_Is_LockDirNow(ucSlave, ucLock)) {	/* 現「ロック施」なら */
				bLockOff = 1;							/* 開錠動作から始める */
			} else {									/* 現「ロック開」なら */
				bLockOff = 0;							/* 施錠動作から始める */
			}
			break;

		/* 不当な処理区分による呼び出し */
		default:
			return 0;	/*「待たずに要求した」扱い */
	}

	/* 要求元の信号出力時間を使うか否か？ */
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
		if (c_dp) {
			// 電文上の信号出力時間を採用
			lockOnTime = c_dp->clos_tm;						// ロック装置閉時の信号出力時間
			lockOffTime = c_dp->open_tm;					// ロック装置開時の信号出力時間
		}
		else {
			/* 過去の信号出力時間を採用 */
			lockOnTime = IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOnTime;		/* ロック装置閉時の信号出力時間 */
			lockOffTime = IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOffTime;	/* ロック装置開時の信号出力時間 */
		}
	}
	else {
		lockOnTime = 0;						// 0固定
		lockOffTime = 0;					// 0固定
	}

	/* (0)施錠/(1)開錠、いずれの動作か？*/
	if (bLockOff == 0) {/*「ロックしろ!」を子機へ要求 */
		action   = 1;	/* 施錠[閉]動作要求する */
	}
	else {				/*「ロック解除しろ!」を子機へ要求 */
		action   = 2;	/* 開錠[開]動作要求する */
	}
	IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucOperation = ucOperation;	/* 処理区分退避 */

	/* 現在動作可能、かつ、指定車室が動作中でない */
	if (IFM_LockTable.usWaitCount < IF_ACTION_MAX &&						/* 動作中を待つ最大数未満で、かつ */
		IFM_Can_Action(LKcom_Search_Ifno(ucSlave + 1)) &&					// 対象装置が動作可能か
		!IFM_Is_Action(ucSlave, ucLock))									/* その車室が「動作中」では無い */
	{
// リトライならば新規キューの要求を優先
		if (bRetry != 0) {
			// 以下のデータ作成/キューイング処理は本関数内のキューイング処理を流用
			/* 空きキュー在るか？*/
			pQbody = GetQueueBody(LCMque_Body_Action, sizeof(LCMque_Body_Action));	/* 空き実体を取得 */
			pQbuff = GetQueueBuff(LCMque_Buff_Action, sizeof(LCMque_Buff_Action));	/* 空き本体を取得 */
			if (pQbody && pQbuff) {
				pQbuff->pData = pQbody;						/* 実体をリンク */
				bQfull = 0;									/* キューフル（ロック動作動作完結待ちリスト）[解除]かも */
			}
			/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
			else {
				bQfull = 1;									/* キューフル（ロック動作動作完結待ちリスト）[発生] */
				pQbuff = RemoveQueue2(&LCMque_Ctrl_Action, 0);	/* キュー(先頭)を外す */
				pQbody = pQbuff->pData;
			}
			/* 子機向け「状態書き込みデータ」を作る */
			pQbody->sW57.sCommon.ucKind     = 'W';			/* 状態書き込みデータ */
			pQbody->sW57.sCommon.ucTerminal = ucSlave;		/* ターミナルNo.（どの子機か？を特定する）*/
			if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
			pQbody->sW57.usLockOffTime      = lockOffTime;	/* 既に決めた「開)信号出力時間」を採用 */
			pQbody->sW57.usLockOnTime       = lockOnTime;	/* 既に決めた「施)信号出力時間」を採用 */
			pQbody->sW57.ucLock             = ucLock;		/* ロック装置No. */
			pQbody->sW57.ucMakerID          = ucMaker;		/* ロック装置メーカーID */
			pQbody->sW57.unCommand.Byte     = 0;			/* 一旦、全ビットをクリア */
			pQbody->sW57.unCommand.Bit.B4   = bLockOff;		/* (0)施錠/(1)開錠、いずれの動作 */
			pQbody->sW57.unCommand.Byte     |= 0x40;		/* ビット6をセット(ガード時間対応) */
			}
			else {
				pQbody->sW57.usLockOffTime      = 0;
				pQbody->sW57.usLockOnTime       = 0;
				pQbody->sW57.ucLock             = 0;
				pQbody->sW57.ucMakerID          = 0;
				pQbody->sW57.unCommand.Byte = (uchar)('4' + bLockOff);	// フラップ上昇/下降コマンドに変換
			}

			// リトライキューにセット
			EnQueue2(&LCMque_Ctrl_Action, pQbuff, (-1), 1);		// リトライキュー

			pQbody->sW57.ucOperation = ucOperation;						/* 精算機からのどんな要求に起因しているか */
			pQbody->sW57.bWaitRetry  = bRetry;							/* 引数次第（この動作要求は、初回かリトライか）*/
			bRet = 1;	/* 待つので、要求はキューイングした */
			if (bQfull) {
				IFM_MasterError(E_IFM_QFULL_LockAction, E_IF_OCCUR);	/* キューフル（ロック動作動作完結待ちリスト）[発生] */
			} else {
				IFM_MasterError(E_IFM_QFULL_LockAction, E_IF_RECOVER);	/* キューフル（ロック動作動作完結待ちリスト）[解除]かも */
			}
			// キューイング完了後、新規送信キュー→リトライキューの順でサーチし、送信
			LCM_NextAction();

			return 1;	// ※本戻り値を参照している箇所はない
		}
		/* 既に決めた「信号出力時間」を採用 */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOnTime = lockOnTime;
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOffTime = lockOffTime;

		// 制御送信前に、同一ロック装置への要求があるか、リトライキューをサーチ
		while (1) {
			del = MatchRetryQueue_Lock(&LCMque_Ctrl_Action.Retry, ucSlave, ucLock);
			// ロック装置が一致ならばその要求を削除
			if (del >= 0) {
				DeQueue(&LCMque_Ctrl_Action.Retry, del);
			}
			if (del == -1) break;
		}
		// 関連する再送タイマストップ
		LCMtim_1secTimerStop((int)ucSlave, (int)ucLock);

		/* 子機通信タスクへ「状態書き込みデータ」メールを発信 */
		LCM_ControlLock(0, bLockOff, ucSlave, ucLock, ucMaker);
		IFM_SetAction(ucSlave, ucLock);										/* 動作要求したので、動作中へ */

		/*
		*	いま「ロック/解除」したロック装置に対して、テーブルマーク。
		*	※子機状態データを返され、動作完結を判定したらマーク外し
		*	・動作完結待ち数をカウントアップ
		*	・「ロック状態」を動作中へ(=0)
		*	・処理区分（精算機からのどんな要求に起因しているか）
		*	・動作履歴（施錠[閉]/開錠[開]何れの動作要求をしたか）
		*	・故障時リトライ回数
		*		初回なら、リトライ回数を初期化
		*		リトライなら、リトライ回数をカウントダウン（リトライする必要があるから本関数が呼び出されたので）
		*/
		IFM_LockTable.usWaitCount++;											/* 動作完結待ち数をカウント */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucOperation = ucOperation;	/* 精算機からのどんな要求に起因しているか */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucAction    = action;		/* 施錠[閉]/開錠[開]何れの動作要求をしたか */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry = IFM_LockTable.sSlave[ucSlave].usRetryCount;
		bRet = 0;	/* 待たずに要求した */
	}

	/* 現在動作不可能のため、キューイング */
	else {
		/* 空きキュー在るか？*/
		pQbody = GetQueueBody(LCMque_Body_Action, sizeof(LCMque_Body_Action));	/* 空き実体を取得 */
		pQbuff = GetQueueBuff(LCMque_Buff_Action, sizeof(LCMque_Buff_Action));	/* 空き本体を取得 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* 実体をリンク */
			bQfull = 0;									/* キューフル（ロック動作動作完結待ちリスト）[解除]かも */
		}
		/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
		else {
			bQfull = 1;									/* キューフル（ロック動作動作完結待ちリスト）[発生] */
			pQbuff = RemoveQueue2(&LCMque_Ctrl_Action, 0);	/* キュー(先頭)を外す */
			pQbody = pQbuff->pData;
		}
		/* 子機向け「状態書き込みデータ」を作る */
		pQbody->sW57.sCommon.ucKind     = 'W';			/* 状態書き込みデータ */
		pQbody->sW57.sCommon.ucTerminal = ucSlave;		/* ターミナルNo.（どの子機か？を特定する）*/
		if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
			pQbody->sW57.usLockOffTime      = lockOffTime;	/* 既に決めた「開)信号出力時間」を採用 */
			pQbody->sW57.usLockOnTime       = lockOnTime;	/* 既に決めた「施)信号出力時間」を採用 */
			pQbody->sW57.ucLock             = ucLock;		/* ロック装置No. */
			pQbody->sW57.ucMakerID          = ucMaker;		/* ロック装置メーカーID */
			pQbody->sW57.unCommand.Byte     = 0;			/* 一旦、全ビットをクリア */
			pQbody->sW57.unCommand.Bit.B4   = bLockOff;		/* (0)施錠/(1)開錠、いずれの動作 */
			pQbody->sW57.unCommand.Byte     |= 0x40;		/* ビット6をセット(ガード時間対応) */
		}
		else {		// フラップ
			pQbody->sW57.usLockOffTime      = 0;
			pQbody->sW57.usLockOnTime       = 0;
			pQbody->sW57.ucLock             = 0;
			pQbody->sW57.ucMakerID          = 0;
			pQbody->sW57.unCommand.Byte = (uchar)('4' + bLockOff);	// フラップ上昇/下降コマンドに変換
		}
		/* 末尾へキューイング */
		// キューイングするキューを選択
		if (bRetry == 0) {
			EnQueue2(&LCMque_Ctrl_Action, pQbuff, (-1), 0);		// 新規キュー
		} else {
			EnQueue2(&LCMque_Ctrl_Action, pQbuff, (-1), 1);		// リトライキュー
		}

		/*
		*	キューイングした「ロック/解除」するロック装置に対して、テーブルマーク準備。
		*	※子機状態データを返され、動作完結を判定したら、そのときテーブルマークできる
		*	・処理区分（精算機からのどんな要求に起因しているか）
		*	・リトライ待ちフラグ（この動作要求は、初回かリトライか）
		*/
		pQbody->sW57.ucOperation = ucOperation;									/* 精算機からのどんな要求に起因しているか */
		pQbody->sW57.bWaitRetry  = bRetry;										/* 引数次第（この動作要求は、初回かリトライか）*/

		bRet = 1;	/* 待つので、要求はキューイングした */

		/*
		*	2005.07.12 早瀬・修正
		*	キューフル発生/解除を判定した、そのときに「IFM_MasterError(IFM.c)」を呼び出してはならない。
		*	※詳しくは「PAYcom_NoticeError(PAYnt_net.c)」を参照のこと
		*/
		if (bQfull) {
			IFM_MasterError(E_IFM_QFULL_LockAction, E_IF_OCCUR);		/* キューフル（ロック動作動作完結待ちリスト）[発生] */
		} else {
			IFM_MasterError(E_IFM_QFULL_LockAction, E_IF_RECOVER);		/* キューフル（ロック動作動作完結待ちリスト）[解除]かも */
		}

		// 出庫優先制御：「下降」指示（出庫指示のタイミング）で新規キューの付けかえを行う
		if((10 == prm_get(COM_PRM, S_TYP, 100, 2, 1))&& 		// 03-0100⑤⑥=10：運用はフラップのみ かつ
		  (0 == prm_get(COM_PRM, S_TYP, 133, 1, 1))&& 			// 03-0133⑥：出庫優先制御あり かつ
		  (bRetry == 0)&&										// 新規キューに登録 かつ
		  // リトライキューはリトライ時にセットされること、また新規キューの処理がすべて終了してから動作するため制御の対象にするメリットは少ないため対象外とする
		  (bLockOff != 0)){										// 「下降」の場合
			// NOTE:本処理は対象車室の上昇動作が終了していない時に下降指示が来た場合、そのまま出庫優先制御を行うと
			// 上昇指示と下降指示の順番が逆転し、車がいない状態でフラップが上昇する可能性があるため
			// 下降指示が来たときに同一車室の上昇指示が残っていた場合、キューから削除する
			scount = Seach_Close_command_sameno(&LCMque_Ctrl_Action.New,ucSlave);// キューの先頭（最古）から検索開始
			if(scount != -1){// 同車室のフラップ上昇要求有り
				pQbuff = DeQueue(&LCMque_Ctrl_Action.New, scount);//該当キューを削除
			}
			if(LCMque_Ctrl_Action.New.usCount >= 2 ){// 対象のキューが2個以上のときのみ実行する
				in_count = Get_Close_command_cnt(&LCMque_Ctrl_Action.New);// キューの中に何個「フラップ上昇要求」があるか取得
				if(in_count > 0){//「フラップ上昇要求」が存在する時のみ実行
					scount = 0;// キューの先頭（最古）から検索開始
					for(;;){
						scount = Seach_Close_command(&LCMque_Ctrl_Action.New,scount);
						if(scount != -1){// フラップ上昇要求有り
							pQbuff = DeQueue(&LCMque_Ctrl_Action.New, scount);
							memset(&quesave,0,sizeof(quesave));
							memcpy(&quesave,pQbuff->pData,sizeof(quesave));
							// ここでは「DeQueue」直後の取得なのであるとみなす。
							pQbody = GetQueueBody(LCMque_Body_Action, sizeof(LCMque_Body_Action));	// 空き実体を取得
							pQbuff = GetQueueBuff(LCMque_Buff_Action, sizeof(LCMque_Buff_Action));	// 空き本体を取得
							if (pQbody && pQbuff) {
								pQbuff->pData = pQbody;	// 実体をリンク
							}
							// 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）
							else {
								pQbuff = RemoveQueue2(&LCMque_Ctrl_Action, 0);// キュー(先頭)を外す
								pQbody = pQbuff->pData;
							}
							memcpy(pQbody,&quesave,sizeof(quesave));
							EnQueue2(&LCMque_Ctrl_Action, pQbuff, (-1), 0);// 新規キュー
							in_count--;// キューのつけなおしをした「フラップ上昇要求」の数を減らす
							if(in_count <= 0){// すべてつけなおし完了した
								break;
							}
						}else{// フラップ上昇要求無し
							break;
						}
					}
				}
			}
		}
	}

// 後処理のいる制御以外はクリアする
	if ((ucOperation != 8) &&	// 開テスト
		(ucOperation != 9) &&	// 閉テスト
		(ucOperation != 12)) {	// バージョン要求 以外なら
		IFM_LockTable.ucOperation = 0;	// 現在制御種別クリア
	}

	return bRet;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_RequestVirsion()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		バージョン要求                                                     |*/
/*|		・精算機から受信した「バージョン要求」を子機へ転送する。           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-04-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_RequestVirsion(void)
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;
	uchar			ni;

	//	・バージョン要求の前に、常にテーブルには「no slave」を初期値設定しておく。
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {			/* 子機の数分 */
		if(LKcom_Search_Ifno((uchar)(ni + 1)) == 0) {
			// ロック装置なら"no slave"で初期化する
			memcpy(IFM_LockTable.sSlave[ni].cVersion, "no slave", sizeof(IFM_LockTable.sSlave[ni].cVersion));
		} else {
			// ロック装置以外は0で初期化
			memset(IFM_LockTable.sSlave[ni].cVersion, 0, sizeof(IFM_LockTable.sSlave[ni].cVersion));
		}
	}

	// 以下の処理はtoS_main()関数から引用
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		//	以下は除外。
		//	・通信する必要がない子機（ロック装置が接続無し）
		//	・初期設定データを送ったのにACK応答していない子機
		//	・無応答スキップ中の子機
		if (!IFM_LockTable.sSlave[ni].bComm ||
			!IFM_LockTable.sSlave[ni].bInitialACK ||
			IFM_LockTable.sSlave[ni].bSkip)
		{
			IFM_LockTable.sSlave[ni].ucContinueCnt = 0;
			continue;
		}
		/* バージョン要求/応答は１回こっきり */
		else {
			if(0 != LKcom_Search_Ifno((uchar)(ni + 1))) {			//ロック装置以外はバージョンチェックをしない
				IFM_LockTable.sSlave[ni].ucContinueCnt = 0;
				continue;
			}
			IFM_LockTable.sSlave[ni].ucContinueCnt = 1;
		}

		/* 空きキュー在るか？*/
		pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* 空き実体を取得 */
		pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* 空き本体を取得 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* 実体をリンク */
			/* キューフル[解除]かも */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
		}
		/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
		else {
			/* キューフル[発生] */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
			pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* キュー(先頭)を外す */
			pQbody = pQbuff->pData;
		}
		/* メールで「状態要求データ」が来たら、それは『バージョン要求』だと決め打ちで良い */
		pQbody->sCommon.ucKind     = 'R';				/* 状態要求データ（バージョン要求）*/
		pQbody->sCommon.ucTerminal = ni;				/* ターミナルNo.（どの子機へか？を特定する）*/
		pQbody->sR52.ucRequest     = 1;					/* 要求内容は、バージョン要求 */
		/* 末尾へキューイング */
		EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
	}
	/* バージョン要求を出すので、その応答が返されるまでの制限時間を決める */
	LCMtim_1secWaitStart(IFS_VERSION_WAIT_TIME);
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_RequestVirsion_FLAP()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		バージョン要求(FLAP)                                               |*/
/*|		・精算機から受信した「バージョン要求」を子機へ転送する。           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-04-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_RequestVirsion_FLAP(void)
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;
	uchar			ni;

	//	バージョン要求の前に、常にテーブルには「no slave」を初期値設定しておく。
	for (ni=0; ni < IFS_CRR_MAX; ni++) {			/* CRRの接続最大数分 */
		memcpy(IFM_LockTable.sSlave_CRR[ni].cVersion, "no slave", sizeof(IFM_LockTable.sSlave_CRR[ni].cVersion));	// "no slave"で初期化する
	}

	// 以下の処理はtoS_main()関数から引用
	for (ni=0; ni < IFS_CRR_MAX; ni++) {
		//	通信する必要がないCRR基板(ターミナルNo.が振られていない）は送信しない
		if (!IFM_LockTable.sSlave_CRR[ni].bComm ){
			continue;
		}

		/* 空きキュー在るか？*/
		pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* 空き実体を取得 */
		pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* 空き本体を取得 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* 実体をリンク */
			/* キューフル[解除]かも */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
		}
		/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
		else {
			/* キューフル[発生] */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
			pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* キュー(先頭)を外す */
			pQbody = pQbuff->pData;
		}
		/* メールで「状態要求データ」が来たら、それは『バージョン要求』だと決め打ちで良い */
		pQbody->sCommon.ucKind     = 'R';				/* 状態要求データ（バージョン要求）*/
		pQbody->sCommon.ucTerminal = ni+IFS_CRR_OFFSET;	/* CRR基板No. +100をすることでターミナルNo.と区別する*/
		/* 末尾へキューイング */
		EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
	}
	/* バージョン要求を出すので、その応答が返されるまでの制限時間を決める */
	LCMtim_1secWaitStart(IFS_VERSION_WAIT_TIME);
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Request_CRRTEST()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR折り返しテスト                                                  |*/
/*|		・精算機から受信した「テスト要求」を子機へ転送する。               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2013/01/24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_Request_CRRTEST( uchar tno )
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;

	//	通信する必要がないCRR基板(ターミナルNo.が振られていない）は送信しない
	if (!IFM_LockTable.sSlave_CRR[tno - IFS_CRR_OFFSET ].bComm ){
		return;
	}

	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* 実体をリンク */
		/* キューフル[解除]かも */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		/* キューフル[発生] */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
		pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}
	pQbody->sCommon.ucKind     = 'T';		/* 折り返しテスト要求 */
	pQbody->sCommon.ucTerminal = tno;		/* CRR基板No. */
	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_SlaveStatus()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		子機から受信した「子機状態データ」の変化を検知する。			   |*/
/*|		・対子機通信タスクからキューにて本タスクへ受信が知らされます。	   |*/
/*|		・本関数にて、車両の入出庫検知、および							   |*/
/*|		  ロック装置動作結果を検知します。								   |*/
/*|		・検知結果は精算機へ通知します。（初期設定データ受信後に限る）	   |*/
/*|		・「子機バージョンデータ」の検知も、本関数にて行います。		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_SlaveStatus(t_IFM_Queue *pStatus)
{
	int			lock, slave;
t_IF_LockSensor	now, last, change;
	uchar		lockState, carState;
	uchar		lastStatus, maker;
	BOOL		bChange;
	BOOL		bRetry;
	uchar		recover;

	switch (pStatus->sCommon.ucKind) {
		case 'A':	/* 子機状態データ */
			slave = pStatus->sCommon.ucTerminal;
			for (lock=0; lock < IFS_ROOM_MAX; lock++) {
				WACDOG;										// 装置ループの際はWDGリセット実行
				now         = pStatus->sA41.unSensor[lock];
				last        = IFM_LockTable.sSlave[slave].sLock[lock].unSensor;
				change.Byte = (uchar)((now.Byte ^ last.Byte) & 0x1f);
				maker       = IFM_LockTable.sSlave[slave].sLock[lock].ucConnect;

				/* 接続無しは除外 */
				if (maker == 0) {
					continue;			/* 次の車室へ */
				}

				/*
				*	2005.05.20 早瀬・追加
				*	初期設定データ受信直後は、現状態通知する対処
				*	・状態変化通知済みフラグは、初期設定データ受信直後にリセットされている。
				*	・ここでは、変化していようがいまいが必ず「通常状態データ」を送りつける。
				*	・ロック装置が接続されているものに限る。
				*/
				if (!IFM_LockTable.sSlave[slave].sLock[lock].bChangeState)		/* 状態変化未通知？*/
				{
					IFM_LockTable.sSlave[slave].sLock[lock].bChangeState = 1;	/* 状態変化通知済みへ */

					/* いまの状態から作り出す */
					lockState = LCM_LockState_Initial(now.Byte, maker);
					carState  = LCM_CarState_Initial(now.Byte, maker);
					/* 変化にしたいので、前回を書き換える（強引？）*/
					IFM_LockTable.sSlave[slave].sLock[lock].ucCarState  = 0;	/* 初期値へ */
					IFM_LockTable.sSlave[slave].sLock[lock].ucLockState = 0;	/* 初期値へ */

					goto CHANGE;		/* 変化判定へ */
				}

				/* 変化なしは除外 */
				if (!change.Byte) {
					continue;			/* 次の車室へ */
				}

				/* 変化ありは更新 */
				IFM_LockTable.sSlave[slave].sLock[lock].unSensor = now;

				/*
				*	車両検知状態を確定させる。
				*/
				if (change.Bit.B2) {		/* 車両検知センサー状態が変化した？*/
					carState = LCM_CarState_bySensor(now.Byte, maker, slave, lock);
				}
				else {						/* 車両検知センサー状態が変化なしなら、車両あり/車両無しは現状態のまま */
					carState = IFM_LockTable.sSlave[slave].sLock[lock].ucCarState;
				}

				/*
				*	ロック装置状態を確定させる。
				*	・動作中は、ロック状態更新不要（動作完結後に変化通知する）
				*	  ただし、車両検知は受け付け更新したいので、この前に済ませる。
				*/

				/*
				*	いま動作中は除外
				*
				*	ただし、
				*	以下の条件がある。（ことがわかった）
				*	2005.06.27	早瀬・バグ修正
				*	・No.80)AAA 入庫状態で施錠→開錠した後、比較的早いうちに「出庫」すると、
				*	 「車両無し」状態変化が精算機に通知されない事がある。
				*	 →せっかく、車両変化検知は受け付け(仮)更新したのに、
				*	  「いま動作中」のため、除外の憂き目に遭い、通知されなかった。
				*	  「いま動作中」でも、車両変化検知だけは、通知するようにする。
				*/
				if (now.Bit.B3) {
					/* ロック閉センサー状態またはロック開センサー状態に変化があったら */
					if((change.Bit.B1 == 1) || (change.Bit.B0 == 1)){
						/* 稼動情報をカウント */
						lockState = IFM_LockTable.sSlave[slave].sLock[lock].ucLockState;// LCM_CountAction()内の故障回数の判定に必要
						LCM_CountAction(lockState, slave, lock);
					}
					/* 車両変化検知したのなら、*/
					if (carState != IFM_LockTable.sSlave[slave].sLock[lock].ucCarState) {
						/* (相方の)ロック装置状態は、前回状態で確定させておく */
						lockState = IFM_LockTable.sSlave[slave].sLock[lock].ucLockState;
						goto CHANGE;		/* 変化判定へ */
					}
					/* 車両変化なしなら、(そのときが)除外 */
					else {
						continue;			/* 次の車室へ */
					}
				}

				lastStatus = IFM_LockTable.sSlave[slave].sLock[lock].ucLockState;
				/* 動作完結になった（動作を伴うもの）*/
				if (change.Bit.B3 &&		/* ロック装置動作状態が */
					  !now.Bit.B3)			/* 動作中(1)→動作完結(0)へ変化した？*/
				{
					/* 動作完結したので．．．*/
					IFM_ResetAction((uchar)slave, (uchar)lock);	/* 動作中を解除 */
					if (IFM_LockTable.usWaitCount) {
						IFM_LockTable.usWaitCount--;				/* 動作完結待ち数を戻す（カウントダウン）*/
					}
					LCMtim_10msTimerStop(slave, lock);			// ここで、動作完結するまでのタイマー解除 

					/* 施錠/開錠センサー状態で「正常/故障」を判定する */
					if (now.Bit.B4 == 0) {
						lockState = LCM_Is_LockOnTrouble( &bRetry, now.Byte, lastStatus, maker);	/* 施錠動作に対して、故障判定 */
					}
					else {
						lockState = LCM_Is_LockOffTrouble(&bRetry, now.Byte, lastStatus, maker);	/* 開錠動作に対して、故障判定 */
					}

					/* 開閉テストの結果判定 */
					if (IFM_LockTable.bTest) {
						LCM_TestResult(lockState, (int)slave, lock);
					}

					/* リトライ（故障確定/故障かも）の条件 */
					if (bRetry &&					/* リトライするのは、*/
						(!IFM_LockTable.bTest))		// 開閉テストでは無く
					{
						/* 故障リトライのためのリトライ間隔時間でタイマー開始 */
						if (IFM_LockTable.sSlave[slave].sLock[lock].usRetry)	// リトライ
						{
							LCMtim_1secTimerStart(slave, lock, IFM_LockTable.sSlave[slave].usRetryInterval);	// ここから、リトライ時間タイマー開始 
						}
					}

					/* 動作完結したので、動作待ちがあれば次を要求する */
					LCM_NextAction();
				}
				/*
				*	前々から動作完結のまま（動作を伴わない）以下の可能性あり
				*	・施錠センサーが変化した・・・・・・故障？
				*	・開錠センサーが変化した・・・・・・故障？
				*	・ロック装置動作方向が変化した・・・動作を伴わない変化は、ありえない
				*	・施錠/開錠センサー変化なし ・・・・車両センサのみの変化だった。
				*/
				else {
					lockState = LCM_LockState_bySensor(&IFM_LockTable.sSlave[slave].sLock[lock].ucLockState, maker, last.Byte, now.Byte, &recover);
					/*
					*	2005.05.17 早瀬・追加
					*	リカバーしないか？するなら、どっち方向のリカバーか？
					*	「recover」値で判定
					*		=0：リカバーしない
					*		=1：施錠動作リカバー
					*		=2：開錠動作リカバー
					*/
					if (recover) {
						/* ロック制御できるか？否かを判定し、否なら待たせる処理（キューイング）*/
						LCM_WaitingLock(recover, (uchar)slave, (uchar)lock, maker, 0, (t_LKcomCtrl *)0);
					}
				}
CHANGE:
				/* ロック管理テーブルのそれと変わった？*/
				bChange = 0;
				if (IFM_LockTable.sSlave[slave].sLock[lock].ucCarState != carState) {
					IFM_LockTable.sSlave[slave].sLock[lock].ucCarState = carState;		/* 変わったので更新 */
					bChange = 1;
				}
				if (IFM_LockTable.sSlave[slave].sLock[lock].ucLockState != lockState) {
					IFM_LockTable.sSlave[slave].sLock[lock].ucLockState = lockState;	/* 変わったので更新 */
					bChange = 1;
				}

				/* 前回と変ってなければ除外 */
				if (!bChange) {
					continue;			/* 次の車室へ */
				}

				if (!IFM_LockTable.bTest)	// 開閉テスト中も、除外
				{
					LCM_NoticeStatus_toPAY((uchar)(slave+1), (uchar)(lock+1), carState, lockState);		// 精算機機へ「状態変化」を知らせる
				}
			}
			break;

		case 'V':	/* 子機バージョンデータ */
			/*
			*	2005.05.20 早瀬・追加
			*	バージョン要求・応答対処
			*/
			slave = pStatus->sCommon.ucTerminal;
			memcpy(IFM_LockTable.sSlave[slave].cVersion, pStatus->sV56.ucVersion, sizeof(IFM_LockTable.sSlave[slave].cVersion));
			if (IFM_LockTable.sSlave[slave].ucContinueCnt) {
				IFM_LockTable.sSlave[slave].ucContinueCnt--;
			}
			if (LCM_Is_AllOver(IFM_LockTable.ucOperation, (uchar)slave, 0)) {
				LCMtim_1secWaitStart(0);	/* ０セットして強引にタイムアウトを起こさせる */
			}
			break;

		default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_CountAction()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		稼動情報をカウント												   |*/
/*|		・動作要求に対する応答が返ってきたときにカウントする。			   |*/
/*|		  なので、ここ一箇所のみ。タイムアウト時はカウントせず。		   |*/
/*|		・開閉動作の合計回数（自動・手動の合計、閉して開すると2回と数える）|*/
/*|		・I/F盤SWによる手動での開・閉動作合計回数（メンテナンス操作）	   |*/
/*|		・故障した合計回数（リミットSW状態が変化しない等）				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_CountAction(uchar lockState, int slave, int lock)
{
	/* 正常、故障に関わらず */
	IFM_LockAction.sSlave[slave].sLock[lock].ulAction += 1;			/* 開閉動作の合計回数 */

	/* 故障判定の結果から */
	if (lockState == 3 ||										/* 施錠動作で故障、または*/
		lockState == 4)											/* 開錠動作で故障 */
	{
		IFM_LockAction.sSlave[slave].sLock[lock].ulTrouble++;	/* 故障した合計回数 */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_CarState_Initial()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|		入出庫判定（車両センサー無変化）								   |*/
/*|		・施錠/開錠センサー変化による入出庫判定をする。					   |*/
/*|		・「イニシャルする」での起動直後は、センサー変化では無くセンサー値 |*/
/*|		  のみで入出庫判定しなければならない。							   |*/
/*|		・戻り値（結果）は以下											   |*/
/*|			0 = 接続なし（バグ？）										   |*/
/*|			1 = 入庫（車両あり）										   |*/
/*|			2 = 出庫（車両無し）										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_CarState_Initial(uchar ucNow, uchar ucMaker)
{
t_IF_LockSensor	now;
	uchar		carState;

	/* 単純に「車両センサー」のみで判定可能 */
	now.Byte = ucNow;
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:
		case LOCK_AIDA_bike:		/* 英田バイクの『入出庫判定』は、子機側で吸収した	2005.05.24 早瀬・追加 */
		case LOCK_YOSHIMASU_both:
		case LOCK_KOMUZU_bicycle:
		case LOCK_HID_both:
			if (now.Bit.B2) {		/* いま車両ありなら、*/
				carState = 1;		/* 入庫 */
			} else {				/* いま車両無しは、*/
				carState = 2;		/* 出庫 */
			}
			break;

		default:
			carState = 0;			/* 接続なし */
			break;
	}

	return carState;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_LockState_Initial()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		初期判定（施/開センサー無変化）									   |*/
/*|		・施錠/開錠センサー無変化による状態判定をする。					   |*/
/*|		・「イニシャルする」での起動直後は、センサー変化では無くセンサー値 |*/
/*|		  のみで状態判定しなければならない。							   |*/
/*|		・戻り値（結果）は以下											   |*/
/*|			1 = 施錠済み（正常）										   |*/
/*|			2 = 開錠済み（正常）										   |*/
/*|			3 = 施錠異常（故障）										   |*/
/*|			4 = 開錠異常（故障）										   |*/
/*|			6 = 指定ロック装置 接続なし（バグ？）						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_LockState_Initial(uchar ucNow, uchar ucMaker)
{
t_IF_LockSensor	now;
	uchar		lockState;

	/* メーカーにより判定が異なる */
	now.Byte = ucNow;
	switch (ucMaker) {

		/* 施錠/開錠共にセンサーあり */
		case LOCK_AIDA_bicycle:
			if (now.Bit.B0 ^ now.Bit.B1) {
				if (now.Bit.B0) {
					lockState = 1;	/* 施錠済み（正常）*/
				} else {
					lockState = 2;	/* 開錠済み（正常）*/
				}
			}
			else if (now.Bit.B0 & now.Bit.B1) {
				lockState = 3;		/* 施錠異常（故障）・・・共にON */
			} else {
				lockState = 4;		/* 開錠異常（故障）・・・共にOFF */
			}
			break;

		/* 施錠/開錠共にセンサー無し */
		case LOCK_AIDA_bike:
		case LOCK_HID_both:
			lockState = 2;			/* 開錠済み（正常）*/
			break;

		/* 施錠センサーのみあり（開錠センサー無し）*/
		case LOCK_YOSHIMASU_both:
		case LOCK_KOMUZU_bicycle:
			if (now.Bit.B0) {
				lockState = 1;		/* 施錠済み（正常）*/
			} else {
				lockState = 2;		/* 開錠済み（正常）*/
			}
			break;

		default:
			lockState = 6;			/* 指定ロック装置 接続なし */
			break;
	}

	return lockState;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_CarState_bySensor()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		入出庫判定（車両センサー変化）									   |*/
/*|		・車両センサー変化による入出庫判定をする。						   |*/
/*|		・戻り値（結果）は以下											   |*/
/*|			0 = 接続なし（バグ？）										   |*/
/*|			1 = 入庫（車両あり）										   |*/
/*|			2 = 出庫（車両無し）										   |*/
/*|			3 = 強制出庫（車両無し。メンテナンスモード中手動で出庫させた） |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_CarState_bySensor(uchar ucNow, uchar ucMaker, int iSlave, int iLock)
{
t_IF_LockSensor	now;
	uchar		carState;

	/* 単純に「車両センサー」のみで判定可能 */
	now.Byte = ucNow;
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:
		case LOCK_AIDA_bike:		/* 英田バイクの『入出庫判定』は、子機側で吸収した	2005.05.24 早瀬・追加 */
		case LOCK_YOSHIMASU_both:
		case LOCK_KOMUZU_bicycle:
		case LOCK_HID_both:
			if (now.Bit.B2) {		/* (車両無し→)車両あり への変化なら、*/
				carState = 1;		/* 入庫 */
			} else {				/* (車両あり→)車両無し への変化は、*/
				carState = 2;		/* とりあえず、出庫 */
			}
			break;

		default:
			carState = 0;			/* 接続なし */
			break;
	}

	/*
	*	強制出庫の判定（以下のAND条件）
	*	・出庫と判定した後、
	*	・初期設定データ受信後で
	*	・(直前の動作が)開錠動作だった
	*/
	if (carState == 2 &&
		now.Bit.B4)
	{
		carState = 3;	/* 手動モードで強制出庫 */

		/* ロック装置メンテナンス動作履歴を更新		2005.06.07 早瀬・仕様変更 */
		IFM_LockTable.sSlave[iSlave].sLock[iLock].ucActionMainte = 3;
	}

	return carState;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_LockState_bySensor()                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		故障判定（待機中の施/開センサー変化）							   |*/
/*|		・開閉動作を伴わない施錠/開錠センサー変化による故障判定判定をする。|*/
/*|		・「英田自転車」に限る。（駐車場のフラップ板を見据えている）	   |*/
/*|		・待機中に「フラップ板」は踏まれたりしてその位置が微妙となる可能性 |*/
/*|		  があり、それを『ホームポジション』に戻すことをリカバー機能として |*/
/*|		  備えておきたいらしい。		  								   |*/
/*|		・戻り値（結果）は以下											   |*/
/*|			1 = 施錠済み（正常）										   |*/
/*|			2 = 開錠済み（正常）										   |*/
/*|			3 = 施錠異常（故障）										   |*/
/*|			4 = 開錠異常（故障）										   |*/
/*|			5 = 故障解除												   |*/
/*|			6 = 指定ロック装置 接続なし（バグ？）						   |*/
/*|		・第５引数「piRecover」にて、以下を返す							   |*/
/*|			0= リカバーしない											   |*/
/*|			1= リカバー施錠動作											   |*/
/*|			2= リカバー開錠動作											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_LockState_bySensor(uchar *pucLastState, uchar ucMaker, uchar ucLast, uchar ucNow, uchar *piRecover)
{
t_IF_LockSensor	after, before;
	uchar		newState;

	*piRecover = 0;					/*「リカバーしない」に初期化 */
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:		/* 英田自転車 */
			break;

		default:					/* 以外は対象外 */
			return *pucLastState;	/* 状態は変えずにリターン */
	}

	before.Byte = ucLast;
	after.Byte  = ucNow;
	newState    = *pucLastState;	/* 新状態は「前回状態」を基準にする */
	switch (*pucLastState) {
		case 1:		/* ロック装置閉済み（正常）	*/
		case 2:		/* ロック装置開済み（正常）	*/
		case 5:		/* 故障解除					*/
			/* 変化前＝閉だった（施=1/開=0）*/
			if (before.Bit.B0 == 1 &&
				before.Bit.B1 == 0)
			{
				if (after.Bit.B0 == 0) {		/* 施→0 */
					*piRecover = 1;				/* 自動リカバー「施錠動作」*/
				}
				else if (after.Bit.B1 == 1) {	/* 開→1 */
					newState = 3;				/* 施錠動作異常（故障）へ */
				}
			}
			/* 変化前＝開だった（施=0/開=1）*/
			if (before.Bit.B0 == 0 &&
				before.Bit.B1 == 1)
			{
				if (after.Bit.B1 == 0) {		/* 開→0 */
					*piRecover = 2;				/* 自動リカバー「開錠動作」*/
				}
				else if (after.Bit.B0 == 1) {	/* 施→1 */
					newState = 4;				/* 開錠動作異常（故障）へ */
				}
			}
			break;

		case 3:		/* 施錠動作異常（故障）		*/
			/* 変化前＝共にONだった（施=1/開=1）*/
			if (before.Bit.B0 == 1 &&
				before.Bit.B1 == 1)
			{
				if (after.Bit.B0 == 0) {		/* 施→0 */
					*piRecover = 1;				/* 自動リカバー「施錠動作」*/
				}
				else if (after.Bit.B1 == 0) {	/* 開→0 */
					newState = 5;				/* 故障解除（施）へ */
				}
			}
			/* 変化前＝共にOFFだった（施=0/開=0）*/
			if (before.Bit.B0 == 0 &&
				before.Bit.B1 == 0)
			{
				if (after.Bit.B1 == 1) {		/* 開→1 */
					newState = 3;				/* 施錠動作異常のままだけど、状態通知したいから */
					*pucLastState = 0;			/* 前回状態を変える（強引？）*/
				}
				else if (after.Bit.B0 == 1) {	/* 施→1 */
					newState = 5;				/* 故障解除（施）へ */
				}
			}
			/* 変化前＝逆だった（施=0/開=1）*/
			if (before.Bit.B0 == 0 &&
				before.Bit.B1 == 1)
			{
				if (after.Bit.B0 == 1 ||		/* 施→1 */
					after.Bit.B1 == 0)			/* 開→0 */
				{
					newState = 3;				/* 施錠動作異常のままだけど、状態通知したいから */
					*pucLastState = 0;			/* 前回状態を変える（強引？）*/
				}
			}
			break;

		case 4:		/* 開錠動作異常（故障）		*/
			/* 変化前＝共にONだった（施=1/開=1）*/
			if (before.Bit.B0 == 1 &&
				before.Bit.B1 == 1)
			{
				if (after.Bit.B1 == 0) {		/* 開→0 */
					*piRecover = 2;				/* 自動リカバー「開錠動作」*/
				}
				else if (after.Bit.B0 == 0) {	/* 施→0 */
					newState = 5;				/* 故障解除（開）へ */
				}
			}
			/* 変化前＝共にOFFだった（施=0/開=0）*/
			if (before.Bit.B0 == 0 &&
				before.Bit.B1 == 0)
			{
				if (after.Bit.B0 == 1) {		/* 施→1 */
					newState = 4;				/* 開錠動作異常のままだけど、状態通知したいから */
					*pucLastState = 0;			/* 前回状態を変える（強引？）*/
				}
				else if (after.Bit.B1 == 1) {	/* 開→1 */
					newState = 5;				/* 故障解除（開）へ */
				}
			}
			/* 変化前＝逆だった（施=1/開=0）*/
			if (before.Bit.B0 == 1 &&
				before.Bit.B1 == 0)
			{
				if (after.Bit.B0 == 0 ||		/* 施→0 */
					after.Bit.B1 == 1)			/* 開→1 */
				{
					newState = 4;				/* 開錠動作異常のままだけど、状態通知したいから */
					*pucLastState = 0;			/* 前回状態を変える（強引？）*/
				}
			}
			break;

		default:
			break;
	}

	return newState;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Is_LockOnTrouble()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|		施錠動作故障判定												   |*/
/*|		・施錠動作に対して、故障判定をする。							   |*/
/*|		・メーカーによって、以下の例外あり。							   |*/
/*|			1)英田バイクの施錠センサーは、ピンを差すだけで反応し		   |*/
/*|			  ロック動作とは連動しない。								   |*/
/*|			2)HID は施錠/開錠センサー何れも無い。						   |*/
/*|		  よって、この２メーカーは「正常(故障なし)」とする。			   |*/
/*|		・戻り値（結果）は以下											   |*/
/*|			1 = 施錠動作済み（正常）									   |*/
/*|			3 = 施錠動作（故障）										   |*/
/*|			5 = 故障解除												   |*/
/*|		※施錠センサー、開錠センサーの何れかが無いロック装置は、		   |*/
/*|		  施錠センサーがONでも、										   |*/
/*|		  リトライの対象（=1）を返す。									   |*/
/*|		  ただし、														   |*/
/*|		  戻り値は「ロック装置状態」を、1=施錠動作済み（正常）で返す。	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_Is_LockOnTrouble(BOOL *pbRetry, uchar ucSensor, uchar ucLastStatus, uchar ucMaker)
{
t_IF_LockSensor	sensor;
	uchar		newStatus;

	/* センサー状態を退避 */
	sensor.Byte = ucSensor;

	/* ロック装置メーカーにより、存在するセンサーが異なる */
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:		/* 英田自転車 */
			*pbRetry  = 0;			/* リトライなしで初期化 */
			break;					/* 故障判定へ */

		/* 英田バイク、HID は故障なし・・・例外 */
		case LOCK_AIDA_bike:		/* 英田バイク：利用者が自身で差すピンなのであいまい */
		case LOCK_HID_both:			/* HID：       施錠、開錠両センサーとも無い */
			*pbRetry = 1;			/* リトライ対象（故障かも）に初期化		2005.05.16 早瀬・変更 */
			return 1;				/* 施錠済み（正常）にするしかない・・・故障判定できないので即リターン */

		case LOCK_YOSHIMASU_both:	/* 吉増 */
		case LOCK_KOMUZU_bicycle:	/* コムズ */
			sensor.Bit.B1 = 0;		/* 開錠センサー無しなので「0」にして故障判定へ臨む */
			*pbRetry = 1;			/* リトライ対象（故障かも）に初期化		2005.05.16 早瀬・変更 */
			break;					/* 故障判定へ */

		default :
			*pbRetry = 0;			/* リトライすることは無意味 */
			return 3;				/* 施錠動作異常 */
	}

	/*
	*	故障判定
	*	※施錠動作完結後が前提。
	*	・施錠センサーON であること・・・必須
	*	・開錠センサーOFFであること・・・任意
	*/
	if (sensor.Bit.B0 == 1 &&		/* 施錠センサー */
		sensor.Bit.B1 == 0)			/* 開錠センサー */
	{
		if (ucLastStatus == 3 ||	/* 前回、施錠動作異常か？または、*/
			ucLastStatus == 4)		/* 前回、開錠動作異常なら、*/
		{
			newStatus = 5;			/* 故障解除 */
		} else {
			newStatus = 1;			/* 施錠済み（正常）*/
		}
	}
	else {
		newStatus = 3;				/* 施錠動作異常 */
		*pbRetry  = 1;				/* リトライ対象（故障確定）*/
	}

	return newStatus;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Is_LockOffTrouble()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		開錠動作故障判定												   |*/
/*|		・開錠動作に対して、故障判定をする。							   |*/
/*|		・メーカーによって、以下の例外あり。							   |*/
/*|			1)英田バイクの施錠センサーは、ピンを差すだけで反応し		   |*/
/*|			  ロック動作とは連動しない。								   |*/
/*|			2)HID は施錠/開錠センサー何れも無い。						   |*/
/*|		  よって、この２メーカーは「正常(故障なし)」とする。			   |*/
/*|		・戻り値（結果）は以下											   |*/
/*|			2 = 開錠動作済み（正常）									   |*/
/*|			4 = 開錠動作（故障）										   |*/
/*|			5 = 故障解除												   |*/
/*|		※施錠センサー、開錠センサーの何れかが無いロック装置は、		   |*/
/*|		  施錠センサーがOFFでも、										   |*/
/*|		  リトライの対象（=1）を返す。									   |*/
/*|		  ただし、														   |*/
/*|		  戻り値は「ロック装置状態」を、2=開錠動作済み（正常）で返す。	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_Is_LockOffTrouble(BOOL *piRetry, uchar ucSensor, uchar ucLastStatus, uchar ucMaker)
{
t_IF_LockSensor	sensor;
	uchar		newStatus;

	/* センサー状態を退避 */
	sensor.Byte = ucSensor;

	/* ロック装置メーカーにより、存在するセンサーが異なる */
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:		/* 英田自転車 */
			*piRetry  = 0;			/* リトライなしで初期化 */
			break;					/* 故障判定へ */

		/* 英田バイク、HID は故障なし・・・例外 */
		case LOCK_AIDA_bike:		/* 英田バイク：利用者が自身で差すピンなのであいまい */
		case LOCK_HID_both:			/* HID：       施錠、開錠両センサーとも無い */
			*piRetry = 1;			/* リトライ対象（故障かも）に初期化		2005.05.16 早瀬・変更 */
			return 2;				/* 開錠済み（正常）にするしかない・・・故障判定できないので即リターン */

		case LOCK_YOSHIMASU_both:	/* 吉増 */
		case LOCK_KOMUZU_bicycle:	/* コムズ */
			sensor.Bit.B1 = 1;		/* 開錠センサー無しなので「1」にして故障判定へ臨む */
			*piRetry  = 1;			/* リトライ対象（故障かも）に初期化 */
			break;					/* 故障判定へ */

		default :
			*piRetry = 0;			/* リトライすることは無意味 */
			return 4;				/* 開錠動作異常 */
	}

	/*
	*	故障判定
	*	※開錠動作完結後が前提。
	*	・施錠センサーOFFであること・・・必須
	*	・開錠センサーON であること・・・任意
	*/
	if (sensor.Bit.B0 == 0 &&		/* 施錠センサー */
		sensor.Bit.B1 == 1)			/* 開錠センサー */
	{
		if (ucLastStatus == 3 ||	/* 前回、施錠動作異常か？または、*/
			ucLastStatus == 4)		/* 前回、開錠動作異常なら、*/
		{
			newStatus = 5;			/* 故障解除 */
		} else {
			newStatus = 2;			/* 開錠済み（正常）*/
		}
	}
	else {
		newStatus = 4;				/* 開錠動作異常 */
		*piRetry  = 1;				/* リトライ対象（故障確定）*/
	}

	return newStatus;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Is_LockDirNow()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		現動作方向判定													   |*/
/*|		・いま、施錠方向にいるか？を「動作方向ビット」から判定する。	   |*/
/*|		・開閉テスト中、どちらの方向に動作させるか？の判定に使用する。	   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	戻り値：															   |*/
/*|			=1 施錠方向にいる。											   |*/
/*|			=0 開錠方向にいる。											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	LCM_Is_LockDirNow(uchar ucSlave, uchar ucLock)
{
	BOOL	nowDir;

	/* .ucActionは、直前の「ロック装置動作履歴」を示す */
	/*	その値の意味は、*/
	/*		=0：何の動作要求もしていない */
	/*		=1：施錠（閉）動作要求した */
	/*		=2：開錠（開）動作要求した */
	switch (IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucAction) {
		case 1:		/* 直前に「施錠（閉）動作要求した」なら、*/
			nowDir = 1;		/*「施錠方向にいる」へ */
			break;

		case 2:		/* 直前に「開錠（開）動作要求した」なら、*/
			nowDir = 0;		/*「開錠方向にいる」へ */
			break;

		case 0:		/*「何の動作要求もしていない」か、*/
		default:	/* 上記以外なら、*/
			/* 現「ロック状態」から判断するしかない */
			/*	★「イニシャルする」で立ち上がり、まだ一度もロック動作実施していないケースを想定 */
			switch (IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucLockState) {
				case 1:		/* ロック装置閉済み（正常）	*/
				case 3:		/* 閉動作異常（故障）		*/
					nowDir = 1;		/*「施錠方向にいる」へ */
					break;

				case 2:		/* ロック装置開済み（正常）	*/
				case 4:		/* 開動作異常（故障）		*/
					nowDir = 0;		/*「開錠方向にいる」へ */
					break;

				case 5:		/* 故障解除					*/
				case 6:		/* 指定ロック装置接続無し	*/
				default:	/* 上記以外 */
									/* あり得ないケースなので、*/
					nowDir = 0;		/*「開錠方向にいる」と割り切る */
					break;
			}
			break;
	}

	return	nowDir;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_do_ActionNow()                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*|		動作制御可能判定												   |*/
/*|		・指定ロック装置（=車室）に対して、動作制御可能か？否かを判定する。|*/
/*|		・主に、開閉テスト時に使用する。								   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	戻り値：															   |*/
/*|			=2 いま制御不可（通信傷害中なので）							   |*/
/*|			=1 制御可能													   |*/
/*|			=0 制御不可													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_do_ActionNow(int iSlave, int iLock)
{
	/*
	*	以下は除外。
	*	・通信する必要がない子機（ロック装置が接続無し）
	*	・初期設定データを送ったのにACK応答していない子機
	*/
	if (!IFM_LockTable.sSlave[iSlave].bComm ||
		!IFM_LockTable.sSlave[iSlave].bInitialACK)
	{
		return 0;
	}
	else {
		/* 無応答スキップ中の子機は「いまは動作できない」*/
		if (IFM_LockTable.sSlave[iSlave].bSkip) {
			return 2;
		}

		/* ロック接続なしは「動作できない」*/
		if (!IFM_LockTable.sSlave[iSlave].sLock[iLock].ucConnect) {
			return 0;
		}
		/* ロック接続ありは「動作できる」*/
		if (IFM_LockTable.sSlave[iSlave].sLock[iLock].ucConnect < LOCK_MAKER_END) {
			return 1;
		}
		/* メーカー不明も「動作できない」*/
		else {
			return 0;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_NextAction()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		次の動作要求													   |*/
/*|		・動作待ちがあれば次を要求する。								   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_NextAction(void)
{
t_IFM_Queue			*pQbody;
struct t_IF_Queue	*pQbuff;
	uchar			action;
	BOOL			bLockOff;
	uchar			ucSlave, ucLock, ucMaker;

	/* キュー(先頭)を外す */
	pQbuff = DeQueue2(&LCMque_Ctrl_Action);
	if (pQbuff) {
		pQbody = pQbuff->pData;
		ucSlave = pQbody->sW57.sCommon.ucTerminal;
		ucLock  = pQbody->sW57.ucLock;
		ucMaker = pQbody->sW57.ucMakerID;

		if( LKcom_Search_Ifno( ucSlave + 1 ) == 0 ){		// ロック装置
			/* (0)施錠/(1)開錠、いずれの動作か？*/
			if (pQbody->sW57.unCommand.Bit.B4 == 0) {
				bLockOff = 0;	/*「ロックしろ!」を子機へ要求 */
				action   = 1;	/* 施錠[閉]動作要求する */
			} else {
				bLockOff = 1;	/*「ロック解除しろ!」を子機へ要求 */
				action   = 2;	/* 開錠[開]動作要求する */
			}
		}
		else {			// フラップ
			if (pQbody->sW57.unCommand.Byte == '4') {		// フラップ上昇
				bLockOff = 0;
				action   = 1;
			}
			else if(pQbody->sW57.unCommand.Byte == '5') {	// フラップ下降
				bLockOff = 1;
				action   = 2;
			}
			else if(pQbody->sW57.unCommand.Byte == '1') {	// ループ強制OFF
				bLockOff = 3;
				action   = 3;
			}
			else {											// ループ強制ON
				bLockOff = 2;
				action   = 4;
			}
		}

		/* 外したキューの「信号出力時間」を採用 */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOnTime = pQbody->sW57.usLockOnTime;
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOffTime = pQbody->sW57.usLockOffTime;

		/* 子機通信タスクへ「状態書き込みデータ」メールを発信 */
		LCM_ControlLock(0, bLockOff, ucSlave, ucLock, ucMaker);
		IFM_SetAction(ucSlave, ucLock);											/* 動作要求したので、動作中へ */

		/*
		*	いま「ロック/解除」したロック装置に対して、テーブルマーク。
		*	※子機状態データを返され、動作完結を判定したらマーク外し
		*	・動作完結待ち数をカウントアップ
		*	・「ロック状態」を動作中へ(=0)
		*	・処理区分（精算機からのどんな要求に起因しているか）
		*	・動作履歴（施錠[閉]/開錠[開]何れの動作要求をしたか）
		*	・故障時リトライ回数
		*		初回なら、リトライ回数を初期化
		*		リトライなら、リトライ回数をカウントダウン（リトライする必要があるから本関数が呼び出されたので）
		*/
		IFM_LockTable.usWaitCount++;											/* 動作完結待ち数をカウント */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucOperation = pQbody->sW57.ucOperation;	/* 精算機からのどんな要求に起因しているか */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucAction    = action;		/* 施錠[閉]/開錠[開]何れの動作要求をしたか */
		if (pQbody->sW57.bWaitRetry == 0) {										/* 初回は、故障時リトライ回数初期化 */
			IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry = IFM_LockTable.sSlave[ucSlave].usRetryCount;
		}
		else {																	/* リトライなので、*/
			/* リトライ回数が「99」回以上は、無限リトライなので数える必要なし */
			if (IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry != 0 &&		/*「=0」のときは除外	2005.07.12 早瀬・追加 */
				IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry < 99)
			{
				IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry--;			/* リトライ回数を数える */
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimeoutAction()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		動作完結タイムアウト											   |*/
/*|		・本関数にて子機へのロック装置制御要求で、動作完結しなかったか、   |*/
/*|		  否か？を判定します。											   |*/
/*|		・動作完結しなかったら、故障と判断します。						   |*/
/*|		・故障を精算機へ通知します。									   |*/
/*|		・故障リトライ間隔時間でタイマー起動します。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimeoutAction(void)
{
	int		slave, lock;
	uchar	lockState;
	BOOL	bChange;

	/* 全タイマーを見る */
	for (slave=0; slave < IF_SLAVE_MAX; slave++) {
		for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {
			WACDOG;										// 装置ループの際はWDGリセット実行
			if (LCMtim_10msTimeout(slave,lock)) {
				LCMtim_10msTimerStop(slave,lock);						/* その動作完結タイマー停止 */

				/* 施錠[閉]動作要求していたのなら、*/
				if (IFM_LockTable.sSlave[slave].sLock[lock].ucAction == 1) {
					lockState = 3;			/* 施錠動作異常 */
				}	
				/* 開錠[開]動作要求していたのなら、*/
				else if (IFM_LockTable.sSlave[slave].sLock[lock].ucAction == 2) {
					lockState = 4;			/* 開錠動作異常 */
				}
				else {
					continue;
				}

				/* 開閉テストの結果判定 */
				if (IFM_LockTable.bTest) {
					LCM_TestResult(lockState, slave, lock);
				}

				/* 稼動情報をカウント */
				LCM_CountAction(lockState, slave, lock);

				/* 変化あり/無しを判定 */
				if (IFM_LockTable.sSlave[slave].sLock[lock].ucLockState == lockState) {
					bChange = 0;
				} else {
					bChange = 1;
					IFM_LockTable.sSlave[slave].sLock[lock].ucLockState = lockState;
				}

				/* 変化通知（精算機へ知らせる）の条件 */
				if (!IFM_LockTable.bTest)		// 開閉テスト中は、除外
				{

					if (bChange) {
						/* 精算機へ故障を知らせる */
						LCM_NoticeStatus_toPAY(	(uchar)slave + 1, (uchar)lock + 1,
												IFM_LockTable.sSlave[slave].sLock[lock].ucCarState,
												IFM_LockTable.sSlave[slave].sLock[lock].ucLockState);
					}
					/* 故障リトライのためのリトライ間隔時間でタイマー起動 */
					if (IFM_LockTable.sSlave[slave].sLock[lock].usRetry)	/* リトライ */
					{
						LCMtim_1secTimerStart(slave, lock, IFM_LockTable.sSlave[slave].usRetryInterval);	/* ここから、リトライ時間タイマー開始 */
					}
				}

				/*
				*	2005.08.25 早瀬・追加
				*	次の「新たな動作要求」を受け付けるため、
				*	動作完結しなかった要求を解放する。
				*/
				/* 動作完結した。ことにしたいので．．．*/
				IFM_ResetAction((uchar)slave, (uchar)lock);	/* 動作中を解除 */
				if (IFM_LockTable.usWaitCount) {
					IFM_LockTable.usWaitCount--;				/* 動作完結待ち数を戻す（カウントダウン）*/
				}
				LCMtim_10msTimerStop(slave, lock);			/* ここで、動作完結するまでのタイマー解除 */
				/* 開閉テストの結果判定 */
				if (IFM_LockTable.bTest) {
					LCM_TestResult(lockState, slave, lock);
				}

				/* リトライの条件 */
				if (!IFM_LockTable.bTest &&								/* 開閉テストでは無く */
					IFM_LockTable.sSlave[slave].sLock[lock].usRetry)	/* リトライ回数が残っている*/
				{
					LCMtim_1secTimerStart(slave, lock, IFM_LockTable.sSlave[slave].usRetryInterval);	/* ここから、リトライ時間タイマー開始 */
				}

				/* 稼動情報をカウント */

				/* 動作完結したので、動作待ちがあれば次を要求する */
				LCM_NextAction();
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimeoutTrouble()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		故障リトライ間隔タイムアウト									   |*/
/*|		・本関数にて子機へのロック装置制御要求で、故障リトライ間隔時間が   |*/
/*|		  経過したか、否か？を判定します。								   |*/
/*|		・経過したら、ロック動作要求をリトライします。					   |*/
/*|		・ロック制御できるか？否かを判定し、							   |*/
/*|		  否なら待たせる処理（キューイング）します。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimeoutTrouble(void)
{
	int		slave, lock;

	/* 全タイマーを見る */
	for (slave=0; slave < IF_SLAVE_MAX; slave++) {
		for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {
			WACDOG;										// 装置ループの際はWDGリセット実行
			if (LCMtim_1secTimeout(slave, lock)) {
				LCMtim_1secTimerStop(slave, lock);				/* そのリトライ間隔タイマー停止 */

				/* リトライする？ */
				if (IFM_LockTable.sSlave[slave].sLock[lock].usRetry) {
					/* ロック制御できるか？否かを判定し、否なら待たせる処理（キューイング）*/
					LCM_WaitingLock(IFM_LockTable.sSlave[slave].sLock[lock].ucOperation,
									(uchar)slave,
									(uchar)lock,
									IFM_LockTable.sSlave[slave].sLock[lock].ucConnect,
									1,			/* キューイングするなら、故障リトライが要因 */
									(t_LKcomCtrl *)0);
				}
				/* もうリトライしない */
				else {
					continue;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimeoutTest()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		開閉テスト応答タイムアウト										   |*/
/*|		・本関数にて、開閉テストのための制限時間が経過した時の			   |*/
/*|		  結果応答を返します。											   |*/
/*|		・制限時間内に開閉テスト完結したも、本関数が呼び出されます。	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimeoutTest(void)
{
	int		slave, lock;
	int		del;

	IFM_LockTable.bTest = 0;		/* 開閉テスト中をクリア */

	/* タイムアウトなので、未実施テスト結果を記載 */
	for (slave=0; slave < IF_SLAVE_MAX; slave++) {
		for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {
			WACDOG;										// 装置ループの際はWDGリセット実行
			/* 全開閉テスト・・・残りの結果は全て「NG」*/
			if (IFM_LockTable.sSlave[slave].sLock[lock].ucOperation == 8) {
				IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 2;	/* NG */
// リトライ送信の発生を防ぐ
				// リトライキューをサーチ
				while (1) {
					del = MatchRetryQueue_Lock(&LCMque_Ctrl_Action.Retry, (uchar)slave, (uchar)lock);
					// ロック装置が一致ならばその要求を削除
					if (del >= 0) {
						DeQueue(&LCMque_Ctrl_Action.Retry, del);
					}
					if (del == -1) break;
				}
				// 関連する再送タイマストップ
				LCMtim_1secTimerStop(slave, lock);
			}
			/* 個別開閉テスト・・・当該結果は「NG」*/
			if (IFM_LockTable.sSlave[slave].sLock[lock].ucOperation == 9) {
				IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 2;	/* NG */
// リトライ送信の発生を防ぐ
				// リトライキューをサーチ
				while (1) {
					del = MatchRetryQueue_Lock(&LCMque_Ctrl_Action.Retry, (uchar)slave, (uchar)lock);
					// ロック装置が一致ならばその要求を削除
					if (del >= 0) {
						DeQueue(&LCMque_Ctrl_Action.Retry, del);
					}
					if (del == -1) break;
				}
				// 関連する再送タイマストップ
				LCMtim_1secTimerStop(slave, lock);
				break;
			}
		}
	}
	LCM_AnswerQueue_toPAY62();			// (62H)開閉テスト応答
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimerStartAction()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|		動作完結タイマー起動											   |*/
/*|		・引数指定の信号出力時間に「＋α」時間を加えてタイマー開始します。 |*/
/*|		・ロック管理タスクから、対子機通信タスクへのメール発信後に		   |*/
/*|		  呼び出されます。												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-09                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimerStartAction(uchar ucSlave, uchar ucLock, ushort usAction, BOOL bLockOff)
{

	/* ここから、動作完結するまでのタイマー起動 */
	usAction += IFS_ANSWER_WAIT_TIME;								/* ＋αは、子機15台×一巡に要する時間（msec単位）*/
	LCMtim_10msTimerStart((int)ucSlave, (int)ucLock, usAction);
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_SetStartAction()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		動作中セット													   |*/
/*|		・(ロック装置)動作完結を判定するため、テーブル上のセンサー状態の   |*/
/*|		 「動作中ビット（bit:3）」を立てる。							   |*/
/*|		・子機側でも信号出力中に、このビットを立てているけど、			   |*/
/*|		  親機側のポーリング間隔によっては、ビットON期間をとり逃がす	   |*/
/*|		  可能性あり。													   |*/
/*|		・これを回避するには、子に届いた時点で親が(ビット立てを)肩代わり   |*/
/*|		  するしかない。												   |*/
/*|		※対子機通信タスクから「状態書き込み」を送信した後のACK受信時に	   |*/
/*|		  呼び出されます。												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-17                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_SetStartAction(t_IFM_Queue *pMail)
{
	uchar	ucSlave;
	uchar	ucLock;

	/*「状態書き込み」以外は除外 */
	if (pMail->sCommon.ucKind != 'W') {
		return;
	}
	ucSlave = pMail->sCommon.ucTerminal;
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){		// ロック装置の場合のみチェック
	/* メンテナンス開始/終了は除外・・・以下の条件は、LCM_NoticeMainte_toIFS(このソース)を参照のこと */
	if (pMail->sW57.usLockOffTime == 0 &&	/* ロック装置開時の信号出力時間	=0固定（メンテナンス開始/終了を知らせるのが目的）*/
		pMail->sW57.usLockOnTime  == 0 &&	/* ロック装置閉時の信号出力時間	=0固定（メンテナンス開始/終了を知らせるのが目的）*/
		pMail->sW57.ucLock        == 0 &&	/* ロック装置No.				=0固定（メンテナンス開始/終了を知らせるのが目的）*/
		pMail->sW57.ucMakerID     == 0)		/* ロック装置メーカーID			=0固定（メンテナンス開始/終了を知らせるのが目的）*/
	{
		return;
	}

	/* 動作中ビットを、ここでONしとかないと動作完結判定できない可能性あり */
	}
	ucLock  = pMail->sW57.ucLock;
	IFM_LockTable.sSlave[ucSlave].sLock[ucLock].unSensor.Bit.B3 = 1;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Is_AllOver()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		要求セットを全て処理したか？否かを判定							   |*/
/*|		・処理区分＝12（バージョン要求）								   |*/
/*|				  ＝８（全ロック開閉テスト）							   |*/
/*|				  ＝９（指定ロック開閉テスト）							   |*/
/*|		  について、全て処理したか？否かを返します。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	戻り値：															   |*/
/*|			=1 全て処理した												   |*/
/*|			=0 未だ処理は継続する										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-09                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	LCM_Is_AllOver(uchar ucOperation, uchar ucSlave, uchar ucLock)
{
	int		ni, nj;
	ushort	limitSecond, lockOnTime, lockOffTime;
	uchar	maker;

	switch (ucOperation) {
		case 12:	/* バージョン要求 */
		case 15:	/* ループデータ要求 */
			for (ni = 0; ni < IF_SLAVE_MAX; ni++) {
				if (IFM_LockTable.sSlave[ni].ucContinueCnt) {
					return 0;		/* 未だある */
				}
			}
			return 1;	/* 全て処理し終わった */

		case 8:		/* 全ロック開閉テスト */
			for (ni=(int)ucSlave; ni < IF_SLAVE_MAX; ni++) {
				for (nj=(int)ucLock; nj < IFS_ROOM_MAX; nj++) {
					WACDOG;										// 装置ループの際はWDGリセット実行
					/* 次の全ロック開閉テスト対象を探す */
					if (IFM_LockTable.sSlave[ni].sLock[nj].ucOperation == ucOperation)
					{
						/* いま開閉テストできるなら */
						if (LCM_do_ActionNow(ni, nj)) {
							maker = IFM_LockTable.sSlave[ni].sLock[nj].ucConnect;
							LCM_WaitingLock(ucOperation, (uchar)ni, (uchar)nj, maker, 0, (t_LKcomCtrl *)0);

							/*
							*	・次に開閉テストすべきロック装置の制限時間を決める。
							*	※ここで決めるから、タイマー再開する。
							*/
							lockOnTime = IF_SignalOut[maker].usLockOnTime;
							lockOffTime = IF_SignalOut[maker].usLockOffTime;
							limitSecond = LCM_GetLimitOneTest(lockOnTime, lockOffTime);
							LCMtim_1secWaitStart(limitSecond);
							return 0;	/* 未だある */
						}
						/* いま開閉テストできないものは、未実施 */
						else {
							IFM_LockTable.sSlave[ni].sLock[nj].ucOperation = 0;		/* 開閉テスト予約をキャンセル */
							IFM_LockTable.sSlave[ni].sLock[nj].ucResult = 0;		/* 結果は「指定外」で確定 */
						}
					}
				}
			}
			return 1;	/* 全て処理し終わった */

		case 9:		/* 個別ロック開閉テスト */
			return 1;	/* 全て処理し終わった */

		default :
			return 1;	/* 全て処理し終わった */
	}
}


/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TestResult()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		開閉テストの結果判定											   |*/
/*|		・開閉テスト（全ロック/指定ロック）において、					   |*/
/*|		  そのテスト結果を判定し										   |*/
/*|		・終わりなら、強引に(制限時間タイムアウト)を起こさせ、			   |*/
/*|		  結果応答を促し												   |*/
/*|		・続くなら、次のロック装置をテストすべく、動作要求を出す。		   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-09                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TestResult(uchar lockState, int slave, int lock)
{
	uchar	result;

	if (lockState == 1 ||	/* 動作結果が「施動作済み（正常）」*/
		lockState == 2 ||	/* 〃「開動作済み（正常）」*/
		lockState == 5)		/* 〃「故障解除」*/
	{
		result = 1;		/* (テスト結果)正常 */
	}
	else {
		result = 2;		/* (テスト結果)NG */
	}

	/* 開閉テスト１回目？*/
	if (IFM_LockTable.sSlave[slave].sLock[lock].ucResult == 0) {
		IFM_LockTable.sSlave[slave].sLock[lock].ucResult = result;	/* 仮結果 */
		/* 逆方向へ再動作 */
		LCM_WaitingLock(	IFM_LockTable.sSlave[slave].sLock[lock].ucOperation,
							(uchar)slave,
							(uchar)lock,
							IFM_LockTable.sSlave[slave].sLock[lock].ucConnect,
							0,
							(t_LKcomCtrl *)0);
	}
	/* 開閉テスト２回目？*/
	else {
		/*
		*	１回目の結果OKなら、２回目の結果が「本結果」
		*	１回目の結果NGなら、２回目の結果に関わらず１回目の結果を採用→つまり「NG」
		*/
		if (IFM_LockTable.sSlave[slave].sLock[lock].ucResult == 1) {
			IFM_LockTable.sSlave[slave].sLock[lock].ucResult = result;	/* 本結果 */
		}

		/* 開閉テスト終わった？*/
		IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = 0;		/* 開閉テスト予約をクリア（テスト済みなので）*/
		if (LCM_Is_AllOver(IFM_LockTable.ucOperation, (uchar)slave, (uchar)lock)) {
			LCMtim_1secWaitStart(0);	/* ０セットして強引にタイムアウトを起こさせる */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             IFM_SetAction()	                                           |*/
/*|             IFM_ResetAction()                                          |*/
/*|             IFM_Is_Action()	                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		指定ロック装置（=車室）に対し									   |*/
/*|		・動作中（=1）へセット											   |*/
/*|		・動作完結（=0）へリセット										   |*/
/*|		・動作中か？否かを返す。										   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	備考：																   |*/
/*|		子機へ動作要求メールを送信したとき「動作中」とするため呼び出され、 |*/
/*|		子機状態データにて動作完結が知らされたとき「動作完結」とするために |*/
/*|		呼び出される。													   |*/
/*|		動作要求を受けた『指定ロック装置（=車室）』が、いま「動作中」なら  |*/
/*|		その動作完結まで待たせるか？否かを、ロック管理タスクレベルで	   |*/
/*|		判定するため、使用される。										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-09                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	IFM_SetAction(uchar ucSlave, uchar ucLock)
{
	IFM_bLockAction[ucSlave][ucLock] = 1;
}
void	IFM_ResetAction(uchar ucSlave, uchar ucLock)
{
	IFM_bLockAction[ucSlave][ucLock] = 0;
}
BOOL	IFM_Is_Action(uchar ucSlave, uchar ucLock)
{
	return IFM_bLockAction[ucSlave][ucLock];
}

uchar	IFM_Get_ActionTerminalNo(void)
{
	uchar tno;
	uchar lock;
	
	for(tno = 0; tno < IF_SLAVE_MAX; ++tno) {
		WACDOG;												// 装置ループの際はWDGリセット実行
		for(lock = 0; lock < IFS_ROOM_MAX_USED; ++lock) {
			if(IFM_bLockAction[tno][lock]) {
				return (uchar)(tno + 1);
			}
		}
	}
	return 0;
}

BOOL	IFM_Can_Action(char type)
{
	uchar tno;
	uchar lock;
	uchar cmp_cnt = 1;// 「1」は同時に1台しかフラップ／ロック装置を制御できないという意味
	uchar act_cnt = 0;
	if(0 != prm_get(COM_PRM, S_TYP, 134, 1, 1)){// 03-0134⑥：フラップ装置2台同時制御機能
		if(0 != type){// 対象はフラップ
			cmp_cnt = 2;// 2台まで同時制御可能とする
		}
	}
	
	for(tno = 0; tno < IF_SLAVE_MAX; ++tno) {
		WACDOG;												// 装置ループの際はWDGリセット実行
		for(lock = 0; lock < IFS_ROOM_MAX_USED; ++lock) {
			if(IFM_bLockAction[tno][lock]) {
				if(type == LKcom_Search_Ifno((uchar)(tno + 1))) {
					act_cnt++;
				}
			}
			if(act_cnt >= cmp_cnt ){// 動作中が同時制御可能台数以上で動作追加動作不可
				return FALSE;
			}
		}
	}
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_GetLimitAction()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		動作完結するまでの制限時間取得（10msec単位）                       |*/
/*|		・対子機通信タスクへロック装置動作を要求してから                   |*/
/*|		  動作完結の状態変化通知が届くまでの制限時間を求める。             |*/
/*|		・開/閉、動作信号出力時間をベースにする。                          |*/
/*|		・＋α算出のための考慮すべき要因は、以下。                         |*/
/*|		  1)子機へのポーリング間隔                                         |*/
/*|		  2)子機が、親機から状態要求を受け、応答するまでの実測値           |*/
/*|		  3)ポーリング対象の子機が、1台のみになったときのポーリング間隔    |*/
/*|		  4)伝送速度に依存する文字間タイムアウト時間                       |*/
/*|                                                                        |*/
/*|		★引数にて渡される時間の単位は、100msecとする。                    |*/
/*|		★取得する時間(戻り値)の単位は、10msecとする。                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-06-17                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	LCM_GetLimitAction(ushort usSigalOutTime)
{
	ulong	betweenChar, plusAlpha;
	ushort	limit_10msec;

	/* 文字間タイムアウトしないと受信完了と認識しない */
	betweenChar = (ulong)(toS_RcvCtrl.usBetweenChar);				// 親と子。単位は2msecより ｢×2÷2＝1倍｣

	/* ＋α分を求める。単位は1msec */
	plusAlpha = (ulong)((IFM_LockTable.toSlave.usPolling * 10 + IF_DEFAULT_SLAVE_ANSWER_TIME) * IF_SLAVE_MAX);
	if (plusAlpha == 0) {
		plusAlpha = (ulong)(IF_DEFAULT_toSLAVE_POLLING_ALONE * 10 + IF_DEFAULT_SLAVE_ANSWER_TIME);
	}

	/* 10msec単位へ揃える（切り上げ）*/
	limit_10msec = (ushort)(usSigalOutTime * 10);					/* 引数は、100msec単位なので10倍に */
	limit_10msec += (ushort)((plusAlpha + betweenChar + 9) / 10);	/* その他は、1msec単位なので1/10に */
																	/* ※｢＋9｣は、10msec単位で切り上げるため */

	return limit_10msec;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_GetLimitOneTest()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		開閉テスト制限時間取得（１ロック装置分）                           |*/
/*|		・開/閉、２回分の動作の制限時間を求める。                          |*/
/*|		・開/閉、動作完結するまでの時間をベースにする。                    |*/
/*|                                                                        |*/
/*|		★引数にて渡される時間の単位は、100msecとする。                    |*/
/*|		★取得する時間(戻り値)の単位は、1secとする。                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-06-17                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	LCM_GetLimitOneTest(ushort usLockOnTime, ushort usLockOffTime)
{
	ushort	limitSecond;
	ulong	on_msec, off_msec;

	on_msec  = (ulong)(LCM_GetLimitAction(usLockOnTime)  * 10);	/* 10msec単位を、1msec単位へ */
	off_msec = (ulong)(LCM_GetLimitAction(usLockOffTime) * 10);	/* 10msec単位を、1msec単位へ */

	/* 1msec単位を、1sec単位へ（切り上げ）*/
	limitSecond = (ushort)((on_msec + off_msec + 999) / 1000);		/* ｢＋999｣は、1sec単位で切り上げるため */

	return limitSecond;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_NoticeStatus_toPAY()                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|   精算機への状態変化通知（ロック装置別）                               |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_NoticeStatus_toPAY( uchar tno, uchar ucLock, uchar ucCarState, uchar ucLockState)
{
	t_IFM_mail	*pQbody;
	int	i;
	int imax;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];// このバッファにはID4からセットする。
	// キュー実体へ「通常状態データ」をセット
	pQbody->s61.sCommon.ucKind           = LK_RCV_COND_ID;	// 通常状態データ
	pQbody->s61.sCommon.bHold            = 1;				// データ保持フラグ
	pQbody->s61.sCommon.t_no             = tno;				// 端末No(CRBの)
	pQbody->s61.ucCount                  = 1;				// 状態データ数。メンテナンス通知のためには「１」で十分 
	if(LKcom_Search_Ifno(tno) == 0) {
		i = FLAP_NUM_MAX;					// ロック装置インデックス
		imax = LOCK_MAX;						// ロック装置最大インデックス
	}
	else {
		i = CRR_CTRL_START;					// フラップ装置インデックス
		imax = TOTAL_CAR_LOCK_MAX;			// フラップ装置最大インデックス
	}

	for( ; i<imax; ++i ){
		WACDOG;										// 装置ループの際はWDGリセット実行
		if(LKcom_Search_Ifno(tno) == 0) {									// ロックの場合
			if( (LockInfo[i].if_oya == tno)&&(LockInfo[i].lok_no == ucLock)){// ターミナルNoとロック装置番号一致
				// LockInfoはLOCK_MAXの値がMAXであるが、255車室未満よりそのままセット
				pQbody->s61.sLock[0].ucLock = (uchar)i;// LockInfoのインデックスをセット
				break;
			}
		} else {															// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
			if( (LockInfo[i].if_oya == tno)&&( 1 == ucLock)){// ターミナルNoとロック装置番号一致
				// LockInfoはLOCK_MAXの値がMAXであるが、255車室未満よりそのままセット
				pQbody->s61.sLock[0].ucLock = (uchar)i;// LockInfoのインデックスをセット
				break;
			}
		}
	}
	pQbody->s61.sLock[0].ucCarState  = ucCarState;			// 車両検知状態		=0固定（メンテナンス開始/終了を知らせるのが目的）
	pQbody->s61.sLock[0].ucLockState = ucLockState;			// ロック装置状態	(07H)メンテナンス中/(08H)メンテナンス解除の何れか 
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],7);				// アプリ側が受信するバッファにセット
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_NoticeStatus_toPAY()                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|   精算機への状態変化通知(全て)                                         |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_NoticeStatusall_toPAY( void )
{
	int i;
	t_IFM_mail *pQbody;
	uchar count;
	ushort len;
	uchar lok_no_local;

	count = 0;
	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];// このバッファにはID4からセットする。
	// キュー実体へ「通常状態データ」をセット
	pQbody->s61.sCommon.ucKind           = LK_RCV_COND_ID;	// 通常状態データ
	pQbody->s61.sCommon.bHold            = 1;				// データ保持フラグ
	pQbody->s61.sCommon.t_no             = LOCK_REQ_ALL_TNO;// CRB全てを意味するのでFFとする
	len = 4;												// 種別、保持フラグ、ターミナルNo、データ数の計4Byte

	for( i=FLAP_NUM_MAX; i<LOCK_MAX; i++ ){
		WACDOG;										// 装置ループの際はWDGリセット実行
		if(LKcom_Search_Ifno(LockInfo[i].if_oya) == 0) {	// ロックの場合
			lok_no_local = LockInfo[i].lok_no;
		} else {											// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
			lok_no_local = 1;
		}			
		if( (0 != LockInfo[i].if_oya) && (0 != lok_no_local) ){// 車室パラメータにターミナルNo、ロック装置連番有有
			if (IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucConnect) {
				// LockInfoはLOCK_MAXの値がMAXであるが、255車室未満よりそのままセット
				pQbody->s61.sLock[count].ucLock = (uchar)i;// LockInfoのインデックスをセット	
				// 車両検知状態	
				pQbody->s61.sLock[count].ucCarState = IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucCarState;
				// ロック装置状態	(07H)メンテナンス中/(08H)メンテナンス解除の何れか
				pQbody->s61.sLock[count].ucLockState = IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucLockState;
				count++;
				pQbody->s61.ucCount = count;// データ数
				len += 5;// ロック装置No,車両検知状態,ロック装置状態,予備2Byteの計5Byte
			}
		}
	}
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);				// アプリ側が受信するバッファにセット
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_AnswerQueue_toPAY63()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		精算機へのメンテナンス情報要求応答								   |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_AnswerQueue_toPAY63(uchar type)
{
	int	i, cnt;
	t_IFM_mail *pQbody;
	ushort len;
	uchar lok_no_local;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];// このバッファにはID4からセットする。
	// キュー実体へ「通常状態データ」をセット
	pQbody->s63.sCommon.ucKind           = LK_RCV_MENT_ID;	// メンテナンス情報要求応答
	pQbody->s63.sCommon.bHold            = 1;				// データ保持フラグ
	if(type == 0) {
		pQbody->s63.sCommon.t_no         = LOCK_REQ_ALL_TNO;// CRB全てを意味するのでFFとする
	}
	else {
		pQbody->s63.sCommon.t_no         = FLAP_REQ_ALL_TNO;// CRB全てを意味するのでFFとする
	}
	len = 3;												// 種別、保持フラグ、ターミナルNo

// CRA電文のメンテナンス情報要求応答はCRAで管理しているロック装置の連番の情報をセットしたが、
// CRB IF →CRA IF に変換して使用する場合はLockInfoのインデックスに対応するロック装置連番をセットする）
	for( i=0, cnt=0; i< LOCK_MAX ; i++ ){
		WACDOG;										// 装置ループの際はWDGリセット実行
		if(LKcom_Search_Ifno(LockInfo[i].if_oya) == 0) {	// ロックの場合
			lok_no_local = LockInfo[i].lok_no;
		} else {											// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
			lok_no_local = 1;
		}			
		if( (0 != LockInfo[i].if_oya) && (0 != lok_no_local) ){// 車室パラメータにターミナルNo、ロック装置連番有有
			if (IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucConnect) {
				c_int32toarray(pQbody->s63.sLock[cnt].ucAction,
							   IFM_LockAction.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ulAction,4);// 開閉動作の合計回数
				c_int32toarray(pQbody->s63.sLock[cnt].ucManual,
							   IFM_LockAction.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ulManual,4);// I/F盤SWによる手動での開・閉動作合計回数
				c_int32toarray(pQbody->s63.sLock[cnt].ucTrouble,
							   IFM_LockAction.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ulTrouble,4);// 故障した合計回数
			}
		}
		len += 12;// 4 * 3byte
		cnt++;
	}

	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);				// アプリ側が受信するバッファにセット
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_AnswerQueue_toPAY62()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		精算機へのロック装置開閉テスト応答								   |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_AnswerQueue_toPAY62(void)
{
	int	i, cnt;
	t_IFM_mail *pQbody;
	ushort len;
	uchar lok_no_local;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];// このバッファにはID4からセットする。
	// キュー実体へ「通常状態データ」をセット
	pQbody->s62.sCommon.ucKind           = LK_RCV_TEST_ID;	// ロック装置開閉テスト応答
	pQbody->s62.sCommon.bHold            = 1;				// データ保持フラグ
	pQbody->s62.sCommon.t_no             = LOCK_REQ_ALL_TNO;// CRB全てを意味するのでFFとする
	len = 3;												// 種別、保持フラグ、ターミナルNoの計3Byte

// CRA電文のメンテナンス情報要求応答はCRAで管理しているロック装置の連番の情報をセットしたが、
// CRB IF →CRA IF に変換して使用する場合はLockInfoのインデックスに対応するロック装置連番をセットする）
	for( i=FLAP_NUM_MAX, cnt=0; i< (FLAP_NUM_MAX+IFM_ROOM_MAX_USED) ; i++ ){
		WACDOG;										// 装置ループの際はWDGリセット実行
		if(LKcom_Search_Ifno(LockInfo[i].if_oya) == 0) {	// ロックの場合
			lok_no_local = LockInfo[i].lok_no;
		} else {											// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
			lok_no_local = 1;
		}			
		if( (0 != LockInfo[i].if_oya) && (0 != lok_no_local) ){// 車室パラメータにターミナルNo、ロック装置連番有有
			if (IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucConnect) {
				pQbody->s62.sLock[cnt].ucResult = IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucResult;
			}
		}
		len++;// 1byte
		cnt++;
	}

	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);				// アプリ側が受信するバッファにセット
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_AnswerQueue_toPAY64()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		精算機へのバージョン応答										   |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_AnswerQueue_toPAY64(void)
{
	int	 ni;
	t_IFM_mail *pQbody;
	ushort len;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];					// このバッファにはID4からセットする。
	// キュー実体へ「通常状態データ」をセット
	pQbody->s64.sCommon.ucKind           = LK_RCV_VERS_ID;		// バージョン応答
	pQbody->s64.sCommon.bHold            = 1;					// データ保持フラグ
	pQbody->s64.sCommon.t_no             = LOCK_REQ_ALL_TNO;	// CRB全てを意味するのでFFとする
	len = 11;													// 種別、保持フラグ、ターミナルNo、I/F盤親機バージョンの計11Byte
	// I/F盤親機は存在しないので省略
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		memcpy(pQbody->s64.sIFSlave[ni].cViersion, IFM_LockTable.sSlave[ni].cVersion, 8); // ソフトバージョン（子の部番）は、子に問い合わせる？
		len += 8;// 8byte
	}
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);					// アプリ側が受信するバッファにセット
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_AnswerQueue_Timeout()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		テスト完了通知													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2013/1/24                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_AnswerQueue_Timeout(void)
{
	t_IFM_mail *pQbody;
	ushort len;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];				// このバッファにはID4からセットする。
	// キュー実体へ「通常状態データ」をセット
	pQbody->sCommon.ucKind           = CRR_RCV_TEST_ID;		// テスト完了
	pQbody->sCommon.bHold            = 1;					// データ保持フラグ
	pQbody->sCommon.t_no             = LOCK_REQ_ALL_TNO;	// CRB全てを意味するのでFFとする
	len = 3;												// 種別、保持フラグ、ターミナルNo、I/F盤親機バージョンの計11Byte
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);				// アプリ側が受信するバッファにセット
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_ControlLock()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		ロック装置制御													   |*/
/*|		・精算機から受信した「ロック装置制御」を解析した後、呼び出され     |*/
/*|		  該当の子機へ「状態書き込み」としてを送る。					   |*/
/*|		・メンテナンスモードにて、手動スイッチによる「ロック装置制御」を   |*/
/*|		  該当の子機へ「状態書き込み」としてを送る。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-03-31                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_ControlLock(uchar bMode, uchar bLockOff, uchar ucSlave, uchar ucLock, uchar ucMaker)
{
	/* 子機向け「状態書き込みデータ」を作る */
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;
	ushort	actionTimer;
	ushort	lockOnTime, lockOffTime;

	/* 信号出力時間は、テーブルから（値が不当の場合、補正する）*/
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
		lockOnTime = IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOnTime;		/* ロック装置閉時の信号出力時間 */
		lockOffTime = IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOffTime;	/* ロック装置開時の信号出力時間 */
		if (lockOnTime == 0) {
			lockOnTime = IF_SignalOut[ucMaker].usLockOnTime;
		}
		if (lockOffTime == 0) {
			lockOffTime = IF_SignalOut[ucMaker].usLockOffTime;
		}
	}
	else {
		lockOnTime = 0;
		lockOffTime = 0;
	}

	// 空きキュー在るか？
	pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	// 空き実体を取得
	pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	// 空き本体を取得
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;												// 実体をリンク

		// キューフル[解除]かも
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
	}
	// 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）
	else {
		/* キューフル[発生] */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);

		pQbuff = DeQueue(&toSque_Ctrl_Select, 0);							// キュー(先頭)を外す
		pQbody = pQbuff->pData;
	}
	//「状態書き込みデータ」をセット
	pQbody->sW57.sCommon.ucKind     = 'W';									// 状態書き込みデータ 
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
		pQbody->sW57.usLockOffTime      = lockOffTime;							// ロック装置開時の信号出力時間 
		pQbody->sW57.usLockOnTime       = lockOnTime;							// ロック装置閉時の信号出力時間 
		pQbody->sW57.ucLock             = ucLock;								// ロック装置No. 
		pQbody->sW57.ucMakerID          = ucMaker;								// ロック装置メーカーID 
		pQbody->sW57.unCommand.Byte     = 0;									// 一旦、全ビットをクリア 
		pQbody->sW57.unCommand.Bit.B4   = bLockOff;								// (0)施錠/(1)開錠、いずれの動作か？
		pQbody->sW57.unCommand.Bit.B7   = bMode;								// (0)通常運用/(1)メンテナンス中 
		pQbody->sW57.unCommand.Byte     |= 0x40;								// ビット6をセット(ガード時間対応) 
	}
	else {
		pQbody->sW57.usLockOffTime      = 0;
		pQbody->sW57.usLockOnTime       = 0;
		pQbody->sW57.ucLock             = 0;
		pQbody->sW57.ucMakerID          = 0;
		if(bLockOff == 0 || bLockOff == 1) {
			pQbody->sW57.unCommand.Byte = (uchar)('4' + bLockOff);			// フラップ上昇/下降コマンドに変換
		}
		else {
			pQbody->sW57.unCommand.Byte = (uchar)('1' + bLockOff - 2);		// フラップ強制OFF/ONコマンドに変換
		}
	}
	pQbody->sW57.sCommon.ucTerminal = ucSlave;								// ターミナルNo.（どの子機か？を特定する）

	// 末尾へキューイング
	EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
	/*
	*	2005.05.26 早瀬・解説
	*	子機向け「状態書き込みデータ」には、150msec単位での時間を送っているので、
	*	親が監視するときには実時間へ変換しなくてはならない。
	*	おまけに、この監視に使うタイマは10msecタイマーなので
	*	さらに注意のこと。
	*
	*	2005.05.26 早瀬・バグ解消
	*	この「150msec単位からの変換」を忘れたために、
	*	吉増、コムズなど短い時間のものはOK（＋αに吸収されて）だが、
	*	英田バイクは、信号出力実時間よりも早く、動作完結タイムアウトしてしまい
	*	（実時間=2000msec に対して、タイムアウト値=[20＋150]msec。20は×100msec計算前の値、150=＋α(=一巡に要する時間=子機15台×ポーリング間隔)）
	*	故障扱いで動作完結したと認識し、その後の状態データで、本当の「動作完結」を知ることになる。
	*	単発動作では、実害は少ないが、それでも状態変化が常に「(早めのタイムアウト)故障→故障解除(本当の動作完結時)」となる。
	*	開閉テスト動作では、致命的で、
	*	初回の"施(閉)"動作で、テスト完了（2回動作したと認識）してしまい、次の"開"動作は非テスト扱いになり、
	*	リトライ対象となって、何度もリトライ実施しハチャメチャになるバグに悩まされた。
	*/
	/* 100msec単位を変換しつつ10msecタイマ値へ */
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 1 ){
		lockOnTime = IFM_LockTable.sSlave[ucSlave].usFlapUpTime;
		lockOffTime = IFM_LockTable.sSlave[ucSlave].usFlapDownTime;
	}
	if (bLockOff) {
		actionTimer = LCM_GetLimitAction(lockOffTime);
	} else {
		actionTimer = LCM_GetLimitAction(lockOnTime);
	}
	/* ここから、動作完結するまでのタイマー起動 */
	LCM_TimerStartAction(ucSlave, ucLock, actionTimer, bLockOff);
}


/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimeoutOperation()                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		バージョン要求、ロック装置開閉テストがタイムアウトしたときの処理   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2006-10-18                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void LCM_TimeoutOperation(void)
{
	if (LCMtim_1secWaitTimeout()) {
		LCMtim_1secWaitStop();			/* その制限時間タイマー停止 */
		switch (IFM_LockTable.ucOperation) {
			case 12:
				LCM_AnswerQueue_toPAY64();		// (64H)バージョン応答
				break;

			case 8:
			case 9:
				LCM_TimeoutTest();		/* 開閉テスト応答タイムアウト */
				break;
			default:
				break;
		}
		IFM_LockTable.ucOperation = 0;	// 現在制御種別クリア
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             PAYcom_InitDataSave()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|     CRBへ送信するための初期設定データを保存する。                      |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : なし                                                          |*/
/*|	return : 戻り値なし                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2010-11-15                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PAYcom_InitDataSave( void )
{
	int	t_no_max, tno, i, cnt,lock;
	t_LockMaker flapMaker;
	uchar	wk_mak;
	uchar	crr_no;		// CRR基板No.
	uchar	point_no;	// 接点No.
	uchar	lok_no_local;
	t_no_max = (int)LKcom_GetAccessTarminalCount();				// フラップ制御盤、ロック装置用IF盤子機接続台数
	/* ロック装置の接続（状態）*/

	if(t_no_max > LOCK_IF_MAX){//ターミナルNoが最大数をオーバーしていたらガード
		t_no_max = LOCK_IF_MAX;
	}
	
	IFM_LockTable.toSlave.usLineWatch    = (ushort)(CPrmSS[S_TYP][56]/10);				// 受信監視タイマー
	IFM_LockTable.toSlave.usPolling      = (ushort)(CPrmSS[S_TYP][57]/10);				// POL間隔
	IFM_LockTable.toSlave.ucRetry        = (uchar )prm_get( 0, S_TYP, 58, 2, 3 );		// リトライ回数
	IFM_LockTable.toSlave.ucSkipNoAnswer = (uchar )prm_get( 0, S_TYP, 58, 2, 1 );		// 無応答エラー判定回数

	if(( GetCarInfoParam() & 0x01 )){							// 駐輪設定あり
		for( tno=1; tno <= t_no_max; tno++ ){					// 内部的なロック装置の車室番号スタート位置(FLAP_NUM_MAX)からターミナルNo分検索する
			if( LKcom_Search_Ifno( (uchar)tno ) == 0 ){				// 該当ターミナルNoの接続は駐輪
				wk_mak = child_mk[tno-1];// ロック装置メーカーの情報を取得
				// ロック装置メーカーは車室ごとではなく実質CRB単位でしか設定できない
				if( wk_mak != 0 ){
					IFM_LockTable.sSlave[tno-1].usEntryTimer    = LockMaker[(uchar)(wk_mak-1)].in_tm;	// 入庫車両検知タイマー
					IFM_LockTable.sSlave[tno-1].usExitTimer     = LockMaker[(uchar)(wk_mak-1)].ot_tm;	// 出庫車両検知タイマー
					IFM_LockTable.sSlave[tno-1].usRetryCount    = LockMaker[(uchar)(wk_mak-1)].r_cnt;	// リトライ回数
					IFM_LockTable.sSlave[tno-1].usRetryInterval = LockMaker[(uchar)(wk_mak-1)].r_tim;	// リトライ間隔
				}else{
					IFM_LockTable.sSlave[tno-1].usEntryTimer = 0;
					IFM_LockTable.sSlave[tno-1].usExitTimer = 0;
					IFM_LockTable.sSlave[tno-1].usRetryCount = 0;
					IFM_LockTable.sSlave[tno-1].usRetryInterval = 0;
				}
				for( i=FLAP_NUM_MAX, cnt=0; i<LOCK_MAX; i++ ){
					WACDOG;										// 装置ループの際はWDGリセット実行
					if(LKcom_Search_Ifno(tno) == 0) {	// ロックの場合
						lok_no_local = LockInfo[i].lok_no-1;
					} else {							// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
						lok_no_local = 0;
					}			
					if( (LockInfo[i].if_oya == tno)&&(LockInfo[i].lok_syu != 0) ){						// ターミナルNo一致かつ接続あり？
						IFM_LockTable.sSlave[tno-1].sLock[lok_no_local].ucConnect = get_lktype(LockInfo[i].lok_syu);
						IFM_LockTable.sSlave[tno-1].sLock[lok_no_local].usLockOnTime = LockMaker[wk_mak-1].clse_tm * 10;
						IFM_LockTable.sSlave[tno-1].sLock[lok_no_local].usLockOffTime = LockMaker[wk_mak-1].open_tm * 10;
						cnt++;
						if( cnt>=LK_LOCK_MAX ){
							break;
						}
					}
				}
			}
		}
		for( i=0; i<LOCK_MAKER_END; i++ ){																// ロック装置メーカー別信号出力時間
			IF_SignalOut[i].usLockOnTime  = (uchar)(LockMaker[i].clse_tm * 10);							// close 時間
			IF_SignalOut[i].usLockOffTime = (uchar)(LockMaker[i].open_tm * 10);							// open  時間
		}

		//ここからはCRBのPAY_InitDataCheck()関数から参考
		// 子機(CRB)所有ロック装置が、全て「接続なし」/どれか「接続あり」を判定
		for (tno=0; tno < t_no_max; tno++) {															// 子機の数分
			cnt = 0;// 加算累計値=0なら、その子機は全て接続なしと判断できる
			WACDOG;																						// 装置ループの際はWDGリセット実行
			for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {											// その子機のロック装置の数分
				cnt += IFM_LockTable.sSlave[tno].sLock[lock].ucConnect;
			}
			// 初期設定データ受信直後は、現状態通知する対処
			IFM_LockTable.sSlave[tno].sLock[lock].bChangeState = 0;										// (一度も)状態変化通知していない
			// 初期設定データ受信直後は、車両・ロック状態を初期化する対処
			// 「接続無し」の車室に対して状態変化は無いけど、状態要求はあり得るので・・・
			if (IFM_LockTable.sSlave[tno].sLock[lock].ucConnect == 0) {
				IFM_LockTable.sSlave[tno].sLock[lock].ucCarState  = 0;									// 車両状態＝「0：接続無し」
				IFM_LockTable.sSlave[tno].sLock[lock].ucLockState = 6;									// ロック状態＝「6：指定ロック装置接続無し」
			}
			// 子機所有ロック装置が、全て「接続なし」/どれか「接続あり」を判定
			if(cnt>0){
				IFM_LockTable.sSlave[tno].bSomeone = 1;
			}
			IFM_LockTable.sSlave[tno].bComm = IFM_LockTable.sSlave[tno].bSomeone;						// 子機へ通信する必要あり/無しを判定
			IFM_LockTable.sSlave[tno].bInitialACK = 0;													// 子機から初期設定データの正常応答を返されていない
			IFM_LockTable.sSlave[tno].bInitialsndACK = 0;												// 起動時に子機へ初期設定データを送信するためのフラグ

			// 再送カウンタ、スキップカウンタは初期値をセットが必要
			IFM_LockTable.sSlave[tno].ucNakRetry  = IFM_LockTable.toSlave.ucRetry;
			IFM_LockTable.sSlave[tno].ucSkipCount = IFM_LockTable.toSlave.ucSkipNoAnswer;
		}
	}

	// フラップの設定
	if(( GetCarInfoParam() & 0x04 )){									// フラップ接続有
		for( tno=0; tno < t_no_max; tno++ ){							// ターミナルNo分検索する
			if( LKcom_Search_Ifno( (uchar)(tno + 1) ) == 1 ){					// 該当ターミナルNoの接続はフラップ
				IFM_LockTable.sSlave[tno].bInitialACK = 0;				// 子機から初期設定データの正常応答を返されていない
				IFM_LockTable.sSlave[tno].bInitialsndACK = 1;			// 初期設定データは送信しないので送信済みにする
				IFM_LockTable.sSlave[tno].bInitialACK = 1;				// 初期設定データは送信しないので受信済みにする
				
				for( i=INT_CAR_START_INDEX, cnt=0; i<TOTAL_CAR_LOCK_MAX; i++ ){
					WACDOG;										// 装置ループの際はWDGリセット実行
					if( (LockInfo[i].if_oya == tno + 1)&&(LockInfo[i].lok_syu != 0) ){				// ターミナルNo一致かつ接続あり？
						IFM_LockTable.sSlave[tno].bSomeone = 1;										// 接続あり
						IFM_LockTable.sSlave[tno].bComm = IFM_LockTable.sSlave[tno].bSomeone;		// 子機へ通信する必要あり/無しを判定
						IFM_LockTable.sSlave[tno].sLock[0].ucConnect = 
							LCM_GetFlapMakerParam(LockInfo[i].lok_syu, &flapMaker);		// [0]のみ使用
						
						IFM_LockTable.sSlave[tno].usEntryTimer    = flapMaker.in_tm;	// 入庫車両検知タイマー
						IFM_LockTable.sSlave[tno].usExitTimer     = flapMaker.ot_tm;	// 出庫車両検知タイマー
						IFM_LockTable.sSlave[tno].usRetryCount    = flapMaker.r_cnt;	// リトライ回数
						IFM_LockTable.sSlave[tno].usRetryInterval = flapMaker.r_tim;	// リトライ間隔

						IFM_LockTable.sSlave[tno].usFlapUpTime  = flapMaker.clse_tm * 10;	// close 時間
						IFM_LockTable.sSlave[tno].usFlapDownTime = flapMaker.open_tm * 10;	// open  時間
						// CRR基板の有効/無効を判断する
						crr_no = LockInfo[i].lok_no / 100;
						point_no = LockInfo[i].lok_no % 100;
						if( LockInfo[i].lok_syu == LK_TYPE_CONTACT_FLAP && crr_no < IFS_CRR_MAX && point_no > 0 && point_no <= IFS_CRRFLAP_MAX){	// ロック種別が接点フラップ(16) & CRR基板 0～2 & 接点 1～15
							IFM_LockTable.sSlave_CRR[crr_no].bComm = 1;	// CRR基板にターミナルNo.の割付が有る場合、有効にする
							IFM_LockTable.ucConnect_Tbl[ crr_no ][ point_no - 1 ] = LockInfo[i].if_oya;
						}
						break;
					}
				}
			}
			else {
				// 未接続またはフラップ以外の場合はステータスをクリアする
				memset(&IFM_FlapSensor[tno], -1, sizeof(IFM_FlapSensor[tno]));
			}
			
		}
		
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_FlapStatus()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		子機から受信した「子機状態データ」の変化を検知する。			   |*/
/*|		・対子機通信タスクからキューにて本タスクへ受信が知らされます。	   |*/
/*|		・本関数にて、車両の入出庫検知、および							   |*/
/*|		  ロック装置動作結果を検知します。								   |*/
/*|		・検知結果は精算機へ通知します。（初期設定データ受信後に限る）	   |*/
/*|		・「子機バージョンデータ」の検知も、本関数にて行います。		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	LCM_FlapStatus(t_IFM_Queue *pStatus)
{
	int	i;
	uchar tno;
	uchar lockState = 0;
	uchar carState = 0;

	switch (pStatus->sCommon.ucKind) {
		case 'A':	/* 子機状態データ */
			tno = pStatus->sCommon.ucTerminal;
			/* 接続無しは除外 */
			if (IFM_LockTable.sSlave[tno].sLock[0].ucConnect == 0) {
				break;
			}

			// 対象フラップのLockInfoインデックス取得
			for( i=INT_CAR_START_INDEX; i<TOTAL_CAR_LOCK_MAX; i++ ){
				WACDOG;										// 装置ループの際はWDGリセット実行
				if( (LockInfo[i].if_oya == tno + 1)&&(LockInfo[i].lok_syu != 0) ){	// ターミナルNo一致かつ接続あり？
					break;
				}
			}
			if(i >= TOTAL_CAR_LOCK_MAX) {
				return;
			}

			// 状態変化が無ければfcmainに通知しない
			if((0 == memcmp(&pStatus->sFlapA41.t_FlapSensor, 				// 今回の状態(受信データ)
							&IFM_FlapSensor[tno],							// 前回の状態
							sizeof(pStatus->sFlapA41.t_FlapSensor))) &&
				IFM_LockTable.sSlave[tno].sLock[0].bChangeState == 1) {
				return;
			}
			IFM_LockTable.sSlave[tno].sLock[0].bChangeState = 1;

			// 前回のステータスと今回を比較し「通常状態データ」を作成する
			lockState = 0;
			if(pStatus->sFlapA41.t_FlapSensor.c_FlapSensor == '1' &&		// 下限
					IFM_FlapSensor[tno].c_LockPlate == '1' &&				// 前回：下降中
					pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '0') {	// 現在：待機中
				lockState = 2;
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_FlapSensor == '2' &&	// 上限
					IFM_FlapSensor[tno].c_LockPlate == '2' &&				// 前回：上昇中
					pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '0') {	// 現在：待機中
				lockState = 1;
			}
		
			if(IFM_FlapSensor[tno].c_LockPlate != '3' &&			// エラーまたは不正ロック以外から
				IFM_FlapSensor[tno].c_LockPlate != '4' &&
				IFM_FlapSensor[tno].c_LockPlate != '8') {
				if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '3') {			// 下降エラー
					lockState = 4;
				}
				else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '4' ||	// 上昇エラー
						pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '8') {	// 不正ロック
					lockState = 3;
				}
			}
					
			if(lockState != 0) {											
				/* 動作完結したので．．．*/
				IFM_ResetAction((uchar)tno, 0);			/* 動作中を解除 */
				if (IFM_LockTable.usWaitCount) {
					IFM_LockTable.usWaitCount--;		/* 動作完結待ち数を戻す（カウントダウン）*/
				}

			   	LCMtim_10msTimerStop((int)tno, 0);			// ここで、動作完結するまでのタイマー解除 

				// 上昇/下降テストの結果判定
				if (IFM_LockTable.bTest) {
					LCM_TestResult(lockState, (int)tno, 0);
				}

				if(lockState == 3 || lockState == 4) {		// エラーはリトライ
					if (IFM_LockTable.sSlave[tno].sLock[0].usRetry && !IFM_LockTable.bTest)	// リトライ可能？
					{
						LCMtim_1secTimerStart(tno, 0, IFM_LockTable.sSlave[tno].usRetryInterval);	// ここから、リトライ時間タイマー開始 
					}
				}
				
				/* 稼動情報をカウント */
				LCM_CountAction(lockState, (int)tno, 0);

				/* 動作完結したので、動作待ちがあれば次を要求する */
				LCM_NextAction();
			}
			
			// 車両検知状態の設定
			if(pStatus->sFlapA41.t_FlapSensor.c_LoopSensor != '0') {
				switch(pStatus->sFlapA41.t_FlapSensor.c_LoopSensor) {
				case '1':										// OFF状態
				case '3':										// 異常
				case '4':										// 強制OFF
				case '6':										// 不明
					carState = 2;								// 車両無し
					break;
				case '2':										// ON状態
				case '5':										// 強制ON
					carState = 1;								// 車両有り
					break;
				case '0':										// 接続無し
				default:
					carState = 0;								// 接続無し
					break;
				}
			}
			else {
				switch(pStatus->sFlapA41.t_FlapSensor.c_ElectroSensor) {
				case '1':										// OFF状態
					carState = 2;								// 車両無し
					break;
				case '2':										// ON状態
					carState = 1;								// 車両有り
					break;
				case '0':										// 接続無し
				default:
					carState = 0;								// 接続無し
					break;
				}
			}

			// ロック装置状態の設定
			if(IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_FORCE_DOWN ||				// 前回メンテモード中
					IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_FORCE_UP) {
				if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_FORCE_DOWN &&
				   pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_FORCE_UP) {
					// 前回がメンテモード中で、現在がメンテモード以外ならメンテモード解除
					lockState = 8;														// 状態：メンテモード解除
					
					// メンテモード解除直後の状態を通知するため、状態変化未通知状態にして
					// 次回の状態データ受信で通知を行うようにする
					IFM_LockTable.sSlave[tno].sLock[0].bChangeState = 0;				// 状態変化未通知へ */
				}
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_DOWN_ERR) {	// ロック板：下降エラー
				lockState = 4;															// 状態：下降異常
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_UP_ERR ||	// ロック板：上昇エラー
					pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_INVALID) {	// ロック板：不正ロック
				lockState = 3;															// 状態：上昇異常
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_WAIT) {		// ロック板：待機中
				if('1' == pStatus->sFlapA41.t_FlapSensor.c_FlapSensor) {				// フラップセンサ：下限
					lockState = 2;														// 状態：下降済み
				}
				else if('2' == pStatus->sFlapA41.t_FlapSensor.c_FlapSensor) {			// フラップセンサ：上限
					lockState = 1;														// 状態：上昇済み
				}
				else if('0' == pStatus->sFlapA41.t_FlapSensor.c_FlapSensor ||			// フラップセンサ：中間
						'3' == pStatus->sFlapA41.t_FlapSensor.c_FlapSensor) {			// フラップセンサ：異常
					if(FLAPDT.flp_data[ i ].nstat.bits.b02 == 1 &&						// 上昇動作
					   FLAPDT.flp_data[ i ].nstat.bits.b01 == 0) {						// 下降済み
						lockState = 3;													// 状態：上昇異常
						pStatus->sFlapA41.t_FlapSensor.c_LockPlate = FLP_LOCK_UP_ERR;
					}
					if(FLAPDT.flp_data[ i ].nstat.bits.b02 == 0 &&						// 下降動作
					   FLAPDT.flp_data[ i ].nstat.bits.b01 == 1) {						// 上昇済み
						lockState = 4;													// 状態：下降異常
						pStatus->sFlapA41.t_FlapSensor.c_LockPlate = FLP_LOCK_DOWN_ERR;
					}
					else if(pStatus->sFlapA41.t_FlapSensor.c_FlapSensor == '0' &&		// フラップセンサ：中間
							lockState != 3) {											// 状態：上昇異常 以外
						lockState = 1;													// 状態：上昇済み
					}
				}
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_DOWN || 		// ロック板：下降中
					pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_UP) {		// ロック板：上昇中
				// 上昇中／下降中は通知の必要は無いが、センサ状態が変化するかもしれないので、
				// フラップ状態を0で通知する
				lockState = 0;
			}

			if(IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_DOWN_ERR ||
			   IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_UP_ERR ||
			   IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_INVALID) {
				if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_DOWN_ERR &&
				   pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_UP_ERR &&
				   pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_INVALID) {
					lockState = 5;															// 状態：故障解除
				}
			}
			if(IFM_FlapSensor[tno].c_LockPlate != FLP_LOCK_FORCE_DOWN &&					// 前回メンテモード以外
			   IFM_FlapSensor[tno].c_LockPlate != FLP_LOCK_FORCE_UP) {
				if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_FORCE_DOWN ||		// 今回メンテモード
				   pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_FORCE_UP) {
					// 現在の状態が強制下降／上昇ならメンテモード中へ以降
					lockState = 7;															// 状態：メンテモード中
				}
			}

			// 今回の状態を退避する
			memcpy(&IFM_FlapSensor[tno], &pStatus->sFlapA41.t_FlapSensor, sizeof(pStatus->sFlapA41.t_FlapSensor));
			
			LCM_NoticeStatus_toPAY(tno + 1, (uchar)1, carState, lockState);
			
			break;
		case 'S':											// ループデータ応答
			tno = pStatus->sCommon.ucTerminal;
			memcpy(&IFM_LockTable.sSlave[tno].tLoopCount, &pStatus->sFlapS53.t_LoopCounter, sizeof(IFM_LockTable.sSlave[tno].tLoopCount));
			
			if (IFM_LockTable.sSlave[tno].ucContinueCnt) {
				IFM_LockTable.sSlave[tno].ucContinueCnt--;
			}
			if (LCM_Is_AllOver(IFM_LockTable.ucOperation, (uchar)tno, 0)) {
				LCMtim_1secWaitStart(0);	/* ０セットして強引にタイムアウトを起こさせる */
			}
			break;
		case 'v':	/* CRRバージョンデータ */
			tno = pStatus->sCommon.ucTerminal - IFS_CRR_OFFSET;	// この時点では+100されているため、ここで-100が必要
			memcpy(IFM_LockTable.sSlave_CRR[tno].cVersion, &pStatus->sFlapv76.ucVersion, sizeof(IFM_LockTable.sSlave_CRR[tno].cVersion));
			break;
		case 't':	/* CRR折り返しテスト結果 */
			memcpy( &MntLockTest[0],&pStatus->sFlapt74.ucTestResult, IFS_CRRFLAP_MAX );			/* 折り返しテストの結果を渡す	*/
			LCM_AnswerQueue_Timeout();
			break;
		default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_RequestLoopData()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		フラップへのループデータ要求                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	LCM_RequestLoopData(void)
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;
	uchar			ni;

	//	ループデータをクリアする（画面に「＊＊＊＊」を表示する状態に初期化）
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {			/* 子機の数分 */
		// フラップ・ロックに関わらず全て0xffにする
		memset(&IFM_LockTable.sSlave[ni].tLoopCount, 0xff, sizeof(IFM_LockTable.sSlave[ni].tLoopCount));
	}

	// 以下の処理はtoS_main()関数から引用
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		//	以下は除外。
		//	・通信する必要がない子機（ロック装置が接続無し）
		//	・初期設定データを送ったのにACK応答していない子機
		//	・無応答スキップ中の子機
		if (!IFM_LockTable.sSlave[ni].bComm ||
			!IFM_LockTable.sSlave[ni].bInitialACK ||
			IFM_LockTable.sSlave[ni].bSkip) {
			IFM_LockTable.sSlave[ni].ucContinueCnt = 0;
			continue;
		}
		/* バージョン要求/応答は１回こっきり */
		else {
			IFM_LockTable.sSlave[ni].ucContinueCnt = 1;
		}

		/* 空きキュー在るか？*/
		pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* 空き実体を取得 */
		pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* 空き本体を取得 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* 実体をリンク */
			/* キューフル[解除]かも */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
		}
		/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
		else {
			/* キューフル[発生] */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
			pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* キュー(先頭)を外す */
			pQbody = pQbuff->pData;
		}
		/* メールで「状態要求データ」が来たら、それは『バージョン要求』だと決め打ちで良い */
		pQbody->sCommon.ucKind     = 'R';				/* 状態要求データ（バージョン要求）*/
		pQbody->sCommon.ucTerminal = ni;				/* ターミナルNo.（どの子機へか？を特定する）*/
		pQbody->sR52.ucRequest     = 2;					/* 要求内容は、ループデータ要求 */
		/* 末尾へキューイング */
		EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
	}
	/* バージョン要求を出すので、その応答が返されるまでの制限時間を決める */
	LCMtim_1secWaitStart(IFS_VERSION_WAIT_TIME);
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_RequestForceLoopControl()                              |*/
/*[]----------------------------------------------------------------------[]*/
/*|		強制ループON/OFF要求                                               |*/
/*|		・「強制ループON/OFF要求」を指定フラップに転送する。               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	LCM_RequestForceLoopControl(uchar ucOperation, uchar ucSlave)
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;

	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){		// 該当ターミナルNoの接続はロック装置
		return;
	}

	// 強制ループON/OFFは、フラップの状態に関係無く即時優先キューに格納する
	
	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* 実体をリンク */
		/* キューフル[解除]かも */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		/* キューフル[発生] */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
		pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}
	/* メールで「状態要求データ」が来たら、それは『ージョン要求』だと決め打ちで良い */
	pQbody->sCommon.ucKind     = 'W';				/* 状態要求データ（バージョン要求）	*/
	pQbody->sW57.usLockOffTime = 0;					/* 0固定							*/
	pQbody->sW57.usLockOnTime  = 0;					/* 0固定							*/
	pQbody->sW57.ucLock        = 0;					/* 0固定							*/
	pQbody->sW57.ucMakerID     = 0;					/* 0固定							*/
	pQbody->sCommon.ucTerminal = ucSlave;			/* ターミナルNo.（どの子機へか？を特定する）*/
	if(ucOperation == 16) {
		pQbody->sW57.unCommand.Byte = '2';			/* 要求内容は、バージョン要求 */
	}
	else {
		pQbody->sW57.unCommand.Byte = '1';			/* 要求内容は、バージョン要求 */
	}	

	pQbody->sW57.ucOperation = ucOperation;			/* 精算機からのどんな要求に起因しているか */
	pQbody->sW57.bWaitRetry  = 0;					/* 引数次第（この動作要求は、初回かリトライか）*/

	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_HasFlapForceControl()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		強制上昇/下降操作中のフラップが存在するか。						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
BOOL	LCM_HasFlapForceControl()
{
	uchar	tno;
	
	for( tno=0; tno < LOCK_IF_MAX; ++tno ){							// ターミナルNo分検索する
		if( LKcom_Search_Ifno( (uchar)(tno + 1) ) == 1 ){			// 該当ターミナルNoの接続はフラップ
			if(IFM_FlapSensor[tno].c_LockPlate == 5 || 				// 強制下降中
			   IFM_FlapSensor[tno].c_LockPlate == 6) {				// 強制上昇中
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_GetFlapMakerParam()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		指定フラップ種別のパラメータを取得する。						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
uchar	LCM_GetFlapMakerParam(uchar type, t_LockMaker* pMakerParam)
{
	long	i;
	
	memset(pMakerParam, 0, sizeof(t_LockMaker));
	
	for(i = 0; i < LCM_FLAP_MAKER_COUNT; ++i) {
		if(LCMFlapmaker_rec[i].adr == type) {
			memcpy(pMakerParam, &LCMFlapmaker_rec[i].dat, sizeof(t_LockMaker));
			break;
		}
	}
	
	if(i == LCM_FLAP_MAKER_COUNT) {
		return 0;
	}

	return (uchar)(i + 1);
}
/*[]----------------------------------------------------------------------[]*/
/*|     LCM_CanFlapCommand(index, direction)                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|     param:	index	LockInfoのインデックス							   |*/
/*|     		direction	1: フラップ上昇								   |*/
/*|     					0: フラップ下降								   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		指定されたLockInfoインデックスのフラップが操作可能かチェックする   |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	LCM_CanFlapCommand(ushort index, uchar direction)
{
	ushort	tno; 
	t_IF_FlapSensor* pFlapState;
	
	tno = LockInfo[index].if_oya;
	if( tno > LOCK_IF_MAX ) {
		return FALSE;
	}

	if( LKcom_Search_Ifno( tno ) == 0 ){				// 該当ターミナルNoの接続はロック装置
		return TRUE;									// ロック装置はコマンド実行可能
	}
	
	pFlapState = &IFM_FlapSensor[LockInfo[index].if_oya - 1];
	
	if( pFlapState->c_LockPlate == FLP_LOCK_DOWN ||		// 下降中
		pFlapState->c_LockPlate == FLP_LOCK_UP ) {		// 上昇中
		return FALSE;									// 上昇または下降中は操作不可
	}
	
	if( pFlapState->c_LockPlate == FLP_LOCK_WAIT) {		// 待機中
		if( direction == 0 && 							// 下降コマンド要求
			pFlapState->c_FlapSensor == '1' ) {			// 現在下降
			return FALSE;								// 同じ方向は動作不可
		}
		else if(direction == 1 &&						// 上昇コマンド要求
			pFlapState->c_FlapSensor == '2' ) {			// 現在下降
			return FALSE;								// 同じ方向は動作不可
		}
	}
	return TRUE;										// コマンド実行可能
}

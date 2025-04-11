/*[]----------------------------------------------------------------------[]*/
/*|		対子機通信タスク 送受信処理（新I/F盤 親）						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"mem_def.h"
#include	"mnt_def.h"
#include	"IFM.h"
#include	"toS.h"
#include	"toSdata.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"lkcom.h"
#include	"LKmain.h"

/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	toScom_Init(void);
void	toScom_TimValInit(void);
void	toScom_Main(void);
void	toScom_Polling(void);

void	toScom_Event1(void);
BOOL	toScom_CheckMatrix(uchar *pData, ushort uiLength);
void	toSmail_SendStatusToLock(uchar ucTerm, t_IF_Packet *pAns);
void	toSmail_SendVersionToLock(uchar ucTerm, t_IF_Packet *pAns);
void	toSmail_SendVersionToFLAP(uchar ucTerm, t_IF_Packet *pAns);
void	toSmail_SendTestResultCRR(uchar ucTerm, t_IF_Packet *pAns);

void	toScom_Event2(void);
void	toScom_Event3(void);
void	toScom_Event4(void);

void	toScom_ComPortReset(void);

uchar	toScom_SendI49(t_IF_I49_mail *pI49);
uchar	toScom_SendI49_CRR(t_Flap_I49_mail *pI49);
uchar	toScom_SendV56(int iSlave);
uchar	toScom_SendT54( uchar iSlave );
uchar	toScom_SendR52(int iSlave, uchar ucRequest);
uchar	toScom_SendW57(t_IF_W57_mail *pW57);
uchar	toScom_SendFlapW57(t_Flap_W57_mail *pFlapW57);
BOOL	toScom_WaitSciSendCmp(unsigned short Length);
void	toScom_gotoIDLE(void);
void	toScom_InitMatrix(void);

void	toScom_NextInitData(uchar ucSlave);	/* 初期設定データ優先送信 */
void	toScom_InitFlapData();				/* Flap初期設定データ送信 */
BOOL	toScom_NextRetry(void);				/* 優先再送 */

void	toScom_ChackAnswer(uchar ucSlave, uchar ucLastCmd, t_IF_Packet *psAnswer);
BOOL	toScom_IsSkip(uchar ucNowSlave);
BOOL	toScom_IsSkipRecover(uchar ucNowSlave);
BOOL	toScom_IsRetryOver(uchar ucNowSlave);
void	toScom_ResetRetry(uchar ucNowSlave);
void	toScom_ResetSkip(uchar ucNowSlave);
BOOL	toScom_Send_or_NextRetry(uchar ucNowSlave, uchar what_do);
void	toScom_SetNextPolling(int iPolling);
void	toSmail_SendStatusToFlap(uchar ucTerm, t_IF_Packet *pAns);
void	toSmail_SendLoopData(uchar ucTerm, t_IF_Packet *pAns);


/* 外部参照関数（デバッグ用途なので、ここで宣言）*/
extern void	dbgprint_Queue(int line, char *file, t_IF_Queue_ctl *pQueue, char *pcQueName);	/* 実体は、toSmain.c にある */


/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Init() 		                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		toS communication parts initialize routine			               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Init(void)
{
	toScom_TimValInit();				/* timer value & retry count set */

	toScom_2msTimerStop();				/* 文字間監視用（ 2msec Timer）*/
	toScom_LinkTimerStop();				/* 受信監視用  （10msec Timer）*/
	toScom_LineTimerStop();				/* 回線監視用  （10msec Timer）*/

	toScom_gotoIDLE();					/* goto IDLE (and timer stop) */

	toScom_SetNextPolling(0);			/* ポーリング間隔を決める */
	toScom_toSlave.ucOrder  = (uchar)-1;	/* 状態要求する相手(ポーリングする)子機は、(インデックス扱いなので)０から始める */
										/* ポーリング時は＋１されるため初期値を－１にする								*/
	toScom_ucAckSlave = TOSCOM_NO_ACK_TERMINAL;
	toScom_first = 1;	// 初期化状態
	toScom_StartTime = LifeTimGet();
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_TimValInit() 		                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		timer value & retry count initialize.                          	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| 	This value will change when initial setting data receive.		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_TimValInit(void)
{
	/*
	*	伝送速度によって、
	*	文字間監視タイマ値を決める。
	*	・１チャラクタ転送時間から算出する？
	*	・スタートビット1＋8ビット長＋パリティなし＋ストップビット1＝10ビットで1キャラクタ。
	*	・ 2400bps：1秒で 240キャラクタ、1キャラクタは4.1666666msec
	*	   9600bps：1秒で 960キャラクタ、1キャラクタは1.0416666msec
	*	  19200bps：1秒で1920キャラクタ、1キャラクタは0.5208333msec
	*	  38400bps：1秒で3840キャラクタ、1キャラクタは0.2604166msec
	*/
	switch(prm_get( COM_PRM,S_TYP,75,1,1 ))		// 03-0075⑥ ボーレート
	{
	case 1:										// 19200bps
		toS_RcvCtrl.usBetweenChar = 2;			//「NT-NET通信仕様書」頁15/21から t5＝3msec(2msタイマーより4ms)
		break;
	case 2:										// 9600bps
		toS_RcvCtrl.usBetweenChar = 3;			//「NT-NET通信仕様書」頁15/21から t5＝5msec(2msタイマーより6ms)
		break;
	case 4:										// 2400bps
		toS_RcvCtrl.usBetweenChar = 10;			//「NT-NET通信仕様書」頁15/21から t5＝20msec(2msタイマー)
		break;
	default:									// 38400bps
		// NOTE:他のシリアルデバイス等が同時に動作し(I2CやSPIなど)、過剰に負荷がかかった時にタスクレベルで
		// 処理が追いつかなくなる可能性があるため、文字間タイマーをわざと伸ばし(マージン2倍)時間かせぎをする
		toS_RcvCtrl.usBetweenChar = 2*2;		//「NT-NET通信仕様書」頁15/21から t5＝2msec(2msタイマー)
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Main() 		                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		toS function communication parts Main routine					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Main( void )
{
	/*
	*	toScomdr_f_RcvCmpの値は、toScomdr_Main(toScomdr.c)にて作られる。
	*/
	/* シリアル受信イベントは？*/
	switch (toScomdr_f_RcvCmp) {
		case 1:							/* 1= 親機からの電文を(末尾まで)受信 */
			toScom_Event1();
			break;
		case 2:							/* 2= シリアル受信エラー発生（SCIレベル）*/
			toScom_Event2();
			break;
		case 3:							/* 3= 親子間リンク(伝送)レベルでタイムアウト発生（未使用）*/
			toScom_Event3();
			break;
		case 4:							/* 4= 親の要求に対して子が無応答、またはポーリング時刻経過 */
			toScom_Event4();
			break;

		default:
			break;
	}

	/* シリアル受信イベントをリセット */
	toScomdr_f_RcvCmp = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Event1() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		受信完了処理													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Event1( void )
{
t_IF_Packet	*pPacket;
	ushort	dataSize;
	ushort	CRCsize;
	uchar	termNo;
	BOOL	bCRC;
	BOOL	bAnswer;
	uchar	nowTerm;
	uchar	nowCmd;
	nowTerm = 0;

	/*
	*	タスクレベルの受信バッファを使用する。
	*	・割り込みハンドラレベルの受信バッファ→ドライバレベルの受信バッファ（割り込みハンドラが転送）
	*	・ドライバレベルの受信バッファ→タスクレベルの受信バッファ（ドライバが転送）
	*/

	/* ターミナルNo.までを受信して｢いない｣なら、応答子機を特定できない */
	bAnswer = toScom_CheckMatrix(toScom_RcvPkt, toScom_PktLength);
	if (!bAnswer) {
		IFM_SlaveError(E_IFS_PACKET_Headerless, E_IF_BOTH, (int)nowTerm);
		toS_ComErr.ulRcv_Size++;
		toScom_Matrix.State  = 0;		/* 次回受信のために。S0：受信待機（アイドル）へ */
		return;		/* 無視 */
	}

	/*
	*	受信完了でここに来る。
	*	ので、受信電文のチェック内容は以下。（順番が重要）
	*		①ヘッダ文字="IFCOM" は正しいか？
	*		②自分が送った相手か？
	*		③データ長(データサイズ)は正しいか？
	*		④CRCは一致するか？
	*		・データ種別は正しいか？
	*		・送信したコマンドの正しい応答か？
	*/
	pPacket  = (t_IF_Packet *)&toScom_RcvPkt[0];					/* 電文チェックのためテンプレート合わせ */
	nowTerm  = toScom_toSlave.ucNow;								/* 現通信相手を取得しておく */

	/* ①ヘッダ文字エラー */
/* 子機「ヘッダ文字取り逃がし」デバッグ用 */
	if (memcmp(pPacket->sHead.c_String, IF_HEADER_STR, IF_HEADER_LEN))
	{
		/* ヘッダ文字エラー[同時] */
		IFM_SlaveError(E_IFS_PACKET_HeadStr, E_IF_BOTH, (int)nowTerm);
		return;		/* 無視 */
	}

	/* ②自分が送った相手からではない。*/
	if( IFS_CRR_OFFSET > nowTerm ){	// CRR基板へのアクセスではない
		termNo   = (uchar)astoinl(&pPacket->sHead.c_TerminalNo[0], 2);	// ターミナルNoを取得
		if (termNo != (nowTerm + 1)) {
			/* 相手が違う[同時] */
			IFM_SlaveError(E_IFS_PACKET_WrongSlave, E_IF_BOTH, (int)nowTerm);
			return;		/* 無視 */
		}
	} else {						// CRR基板へのアクセス
		if( 'V' == IFM_LockTable.sSlave_CRR[nowTerm - IFS_CRR_OFFSET].ucWhatLastCmd 		// 直前送信コマンドが'V'バージョン確認の場合
			|| 'I' == IFM_LockTable.sSlave_CRR[nowTerm - IFS_CRR_OFFSET].ucWhatLastCmd 		// 直前送信コマンドが'I'初期化の場合
			|| 'T' == IFM_LockTable.sSlave_CRR[nowTerm - IFS_CRR_OFFSET].ucWhatLastCmd ){	// 直前送信コマンドが'T'初期化の場合
			if( 'F' != pPacket->sHead.c_TerminalNo[0]
				|| (nowTerm + 1) - IFS_CRR_OFFSET != astoinl(&pPacket->sHead.c_TerminalNo[1], 1) ){	// 'F1'～'F3'が正しい相手
				/* 相手が違う[同時] */
				IFM_SlaveError(E_IFS_PACKET_WrongSlave, E_IF_BOTH, (int)nowTerm);
				return;		/* 無視 */
			}
		}
	}

	/* ③データ長エラー */
	dataSize = (ushort)c_arraytoint32(&pPacket->sHead.c_Size[0], 2);/* データサイズを取得しておく */
	if (toScom_PktLength != dataSize) {
		/* データサイズ異常[同時] */
		IFM_SlaveError(E_IFS_PACKET_DataSize, E_IF_BOTH, (int)nowTerm);
		return;		/* 無視 */
	}

	/*
	*	④CRCエラーチェック
	*	・電文上のCRCデータ位置は、	toScom_RcvPkt[dataSize-2]と
	*								toScom_RcvPkt[dataSize-1]
	*	・CRC対象は、CRC自身を含まず。
	*/
	CRCsize = dataSize - 2;
	bCRC     = CheckCRC(toScom_RcvPkt, CRCsize, &toScom_RcvPkt[CRCsize]);
	if( IFS_CRR_OFFSET > nowTerm ){	// CRR基板へのアクセスではない
		nowCmd   = IFM_LockTable.sSlave[nowTerm].ucWhatLastCmd;		/* 直前送信コマンドを取得しておく */
	} else {						// CRR基板へのアクセス
		nowCmd   = IFM_LockTable.sSlave_CRR[nowTerm - IFS_CRR_OFFSET].ucWhatLastCmd;	// CRR基板へのアクセスの場合、nowTerm には+100した値をいれてあるため、-100が必要
	}
	if (!bCRC) {
		/* CRCエラー[同時] */
		IFM_SlaveError(E_IFS_PACKET_CRC, E_IF_BOTH, (int)nowTerm);
		return;		/* 無視 */
	}

	/*
	*	ここにくれば、受信電文OK
	*/
	/* 何の要求に対して、何が返されたか？を見極める */
	toScom_ChackAnswer(nowTerm, nowCmd, pPacket);
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_ChackAnswer() 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		子機応答チェック												   |*/
/*|		・何の要求に対して、何が返されたか？を見極める。				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_ChackAnswer(uchar ucSlave, uchar ucLastCmd, t_IF_Packet *psAnswer)
{
	int		iAnswer;	/* どんな応答だったか？区分 */
						/*		1：正規の応答 */
						/*		2：NAK返信 */
						/*		3：無応答 */
						/*		4：不当な応答 */
	BOOL	bRetry;
	char	cType;		// 0:ロック, 1:フラップ
	iAnswer = 1;

	/*
	*	どんな要求に対する応答か？による
	*	・期待した応答なら、    送信完了なので保持していた送信データは不要となる。
	*	・期待を外した応答なら、送信未完なので保持していた送信データは必要となる。・・・再送のため
	*/
	if (psAnswer) {
		switch (ucLastCmd) {

			case 'I':	/* (49H)「初期設定データ」を送っていたのなら「ACK」を期待する */
			case 'W':	/* (57H)「状態書込みデータ」を送っていたのなら「ACK」を期待する */
				if( IFS_CRR_OFFSET > ucSlave ){	// CRR基板へのアクセスではない
					switch (psAnswer->sHead.c_Kind[0]) {
						//期待通り
						case IF_ACK:	/* 肯定応答 */
							toScom_Send_or_NextRetry(ucSlave, 1);	/* 送信データは不要 */
							iAnswer = 1;		/* 正規の応答 */
							// ACKを受信した端末No.を退避
							toScom_ucAckSlave = ucSlave;
							/*
							*	2005.05.20 早瀬・追加(仮)
							*	バージョン要求・応答対処
							*/
							if (ucLastCmd == 'I') {
								/* このあと「バージョン要求」を出すことができる様にする */
								IFM_LockTable.sSlave[ucSlave].bInitialACK = 1;	/* 「初期設定データ」を送ってACK返信された */
							}
							break;

						case IF_NAK:	/* 否定応答 */
							bRetry = toScom_Send_or_NextRetry(ucSlave, 2);	/*「(NAK返信のため)連続再送」する */
							if (bRetry) {
								iAnswer = 2;	/* NAK再送 */
							} else {
								iAnswer = 3;	/* NAKリトライオーバー */
							}
							break;

						//期待外れ
						case 'A':		/* 子機状態データ */
						case 'V':		/* 子機バージョンデータ */
						default:		/* 非応答 */
							toScom_Send_or_NextRetry(ucSlave, 3);	/*「(無応答のため)次回再送」扱い */
							iAnswer = 4;		/* 不当な応答 */
							break;
					}
				} else {						// CRR基板へのアクセス
					switch (psAnswer->sHead.c_Kind[0]) {
						case IF_ACK:	/* 肯定応答 */
						default:		/* 非応答 */
							DeQueue(&toSque_Ctrl_Select, 0);			/* (優先)キューを外す。*/
							iAnswer = 1;		/* 正規の応答で無くても、ポーリング間隔を変えないため、1を返す */
							break;
					}
				}
				break;

			/* (52H)「状態要求データ」を送っていたのなら「子機状態データ」を期待する */
			case 'R':
				/*
				*	「状態要求データ」は、「(無応答のため)次回再送」しない扱いなので、例外
				*/
				switch (psAnswer->sHead.c_Kind[0]) {
					//期待通り
					case 'A':		/* 子機状態データ */
						cType = LKcom_GetAccessTarminalType((uchar)(ucSlave + 1));
						if(cType == LKCOM_TYPE_LOCK) {						// ロック装置
							if(psAnswer->unBody.sA41.c_Type[0] != (uchar)'0') {	// 装置とデータ種別が一致しているか
								err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_TYPEDISCODE, (char)1, 0, 0 );	// 装置・データ不一致エラー登録
								toScom_Send_or_NextRetry(ucSlave, 3);			/*「(無応答のため)次回再送」扱い */
								iAnswer = 5;		/* 無応答として後処理する */
								break;
							}
						}
						else if(cType == LKCOM_TYPE_FLAP){					// フラップ
							if(psAnswer->unBody.sFlapA41.c_Type[0] != (uchar)'F') {	// 装置とデータ種別が一致しているか
								err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_TYPEDISCODE, (char)1, 0, 0 );	// 装置・データ不一致エラー登録
								toScom_Send_or_NextRetry(ucSlave, 3);
								iAnswer = 5;		/* 無応答として後処理する */
								break;
							}
						}
						if (psAnswer->unBody.sA41.c_Request[0] == '0') {	/*'0':親機に対する要求なし？*/
							if(cType == LKCOM_TYPE_LOCK) {					// ロック装置
								toSmail_SendStatusToLock(ucSlave, psAnswer);	/* →ロック管理タスクへ知らせる */
								if(0 == IFM_LockTable.sSlave[ucSlave].bInitialsndACK){	// 起動時に初期設定データを送っていない
									IFM_LockTable.sSlave[ucSlave].bInitialsndACK = 1;// 初期設定データを送信した
									toScom_NextInitData(ucSlave);
								}
							}
							else if(cType == LKCOM_TYPE_FLAP) {				// フラップ
								toSmail_SendStatusToFlap(ucSlave, psAnswer);	/* →ロック管理タスクへ知らせる */
							}
						}
						else if (psAnswer->unBody.sA41.c_Request[0] == '1') {/*'1':初期設定データ要求？*/
							if(cType == LKCOM_TYPE_LOCK) {					// ロック装置
								toSmail_SendStatusToLock(ucSlave, psAnswer);	/* →ロック管理タスクへ知らせる */
								IFM_LockTable.sSlave[ucSlave].bInitialsndACK = 1;// 初期設定データを送信した
								toScom_NextInitData(ucSlave);					/* →次は「初期設定データ」を優先して送る（初期設定データを送っていないので）*/
							}
						}													/*   返されたセンサー状態は無効なので、ロック管理タスクへ知らせる必要なし */
						else {												/*'2':センサー未確定 */
							;												/* →何もしない		2005.08.26 早瀬・追加 */
						}													/*   返されたセンサー状態は未確定なので、ロック管理タスクへ知らせる必要なし */
						toScom_Send_or_NextRetry(ucSlave, 1);				/* 送信データは不要 */
						iAnswer = 1;		/* 正規の応答 */
						break;

					case IF_NAK:	/* 否定応答 */
						bRetry = toScom_Send_or_NextRetry(ucSlave, 2);				/*「(NAK返信のため)連続再送」する */
						if (bRetry) {
							iAnswer = 2;	/* NAK再送 */
						} else {
							iAnswer = 3;	/* NAKリトライオーバー */
						}
						break;

					case 'S':		// ループデータ応答
						toSmail_SendLoopData(ucSlave, psAnswer);			/* →ロック管理タスクへ知らせる */
						toScom_Send_or_NextRetry(ucSlave, 1);				/* 送信データは不要 */
						break;

					//期待外れ
					case IF_ACK:	/* 肯定応答 */
					case 'V':		/* 子機バージョンデータ */
					default:		/* 非応答 */
						toScom_Send_or_NextRetry(ucSlave, 3);				/*「(無応答のため)次回再送」扱い */
						iAnswer = 4;		/* 不当な応答 */
						break;
				}
				break;

			case 'V':	/* (56H)「状態要求データ（バージョン要求）」を送っていたのなら「子機バージョンデータ」を期待する */
				if( IFS_CRR_OFFSET > ucSlave ){	// CRR基板へのアクセスではない
					switch (psAnswer->sHead.c_Kind[0]) {
						//期待通り
						case 'V':		/* 子機バージョンデータ */
							toSmail_SendVersionToLock(ucSlave, psAnswer);		/* ロック管理タスクへ知らせる */
							toScom_Send_or_NextRetry(ucSlave, 1);				/* 送信データは不要 */
							iAnswer = 1;		/* 正規の応答 */
							break;

						case IF_NAK:	/* 否定応答 */
							bRetry = toScom_Send_or_NextRetry(ucSlave, 2);	/*「(NAK返信のため)連続再送」する */
							if (bRetry) {
								iAnswer = 2;	/* NAK再送 */
							} else {
								iAnswer = 3;	/* NAKリトライオーバー */
							}
							break;

						//期待外れ
						case IF_ACK:	/* 肯定応答 */
						case 'A':		/* 子機状態データ */
						default:		/* 非応答 */
							toScom_Send_or_NextRetry(ucSlave, 4);	/* 再送は行わない */
							iAnswer = 4;		/* 不当な応答 */
							break;
					}
				} else {						// CRR基板へのアクセス
					switch (psAnswer->sHead.c_Kind[0]) {
						//期待通り
						case 'v':		/* CRRバージョンデータ */
							toSmail_SendVersionToFLAP(ucSlave, psAnswer);		/* ロック管理タスクへ知らせる */
							DeQueue(&toSque_Ctrl_Select, 0);			/* (優先)キューを外す。*/
							iAnswer = 1;		/* 正規の応答 */
							break;
						default:		/* リトライはしない */
							DeQueue(&toSque_Ctrl_Select, 0);			/* (優先)キューを外す。*/
							iAnswer = 1;		/* 正規の応答ではないが、ポーリング間隔を変えないため、1を返す */
							break;
					}
				}
				break;
			case 'T':	/* (54H)「CRR折り返しテスト」を送っていたのなら「折り返しテスト結果」を期待する */
				switch (psAnswer->sHead.c_Kind[0]) {
					//期待通り
					case 't':		/* CRR折り返しテスト結果 */
						toSmail_SendTestResultCRR(ucSlave, psAnswer);		/* ロック管理タスクへ知らせる */
						DeQueue(&toSque_Ctrl_Select, 0);			/* (優先)キューを外す。*/
						iAnswer = 1;		/* 正規の応答 */
						break;
					default:		/* リトライはしない */
						DeQueue(&toSque_Ctrl_Select, 0);			/* (優先)キューを外す。*/
						iAnswer = 1;		/* 正規の応答ではないが、ポーリング間隔を変えないため、1を返す */
						break;
				}
				break;
			/* ここに来たら、バグです。*/
			default:
				toScom_Send_or_NextRetry(ucSlave, 3);			/*「(無応答のため)次回再送」する */
				iAnswer = 4;				/* 不当な応答 */
				break;
		}
	}
	/* 期待外れ・・・無応答 */
	else {
		if( IFS_CRR_OFFSET > ucSlave ){	// CRR基板へのアクセスではない
			if (ucLastCmd == 'V') {
				toScom_Send_or_NextRetry(ucSlave, 4);		/* 再送は行わない */
			} else {
				toScom_Send_or_NextRetry(ucSlave, 3);		/*「(無応答のため)次回再送」する */
			}
			iAnswer = 5;
		} else {						// CRR基板へのアクセスの場合、無応答でも何もしない
			DeQueue(&toSque_Ctrl_Select, 0);	/* (優先)キューを外す。*/
			iAnswer = 1;						/* 正規の応答ではないが、ポーリング間隔を変えないため、1を返す */
		}
	}

	/* どんな応答で、どう対処したか？*/
	switch (iAnswer) {
		case 1:		/* 正規の応答なら、*/
			toScom_LineTimerStop();					/* 回線監視タイマ解除 */
			toScom_toSlave.bWaitAnser = 0;			/* 正しい応答なので、応答待ちクリア */
			toScom_SetNextPolling(iAnswer);			/* ポーリング間隔を決める */
			break;

		case 2:		/* NAK再送なら、*/
			break;									/* 既に再送しているので、ここでは何もしない */

		case 3:		/* NAKリトライオーバー */
		case 4:		/* 不当な応答 */
		case 5:		/* 無応答 */
			toScom_LineTimerStop();					/* 回線監視タイマ解除 */
			toScom_toSlave.bWaitAnser = 0;			/* 正しい応答じゃ無いけど、応答待ちでもないからクリア */
			toScom_SetNextPolling(iAnswer);			/* ポーリング間隔を決める */
			break;

		/* ここに来たら、バグです。*/
		default:
			/* プログラム不備・・・バグかも？[同時] */
			IFM_SlaveError(E_IFS_BUG, E_IF_BOTH, (int)ucSlave);
			break;
	}

	/* エラー履歴づくり */
	switch (iAnswer) {
		case 1:		/* 正規の応答・・・無応答(スキップ)[解除]かも */
			IFM_SlaveError(E_IFS_NO_Answer, E_IF_RECOVER, (int)ucSlave);
			break;

		case 2:		/* NAK再送 */
			break;

		case 3:		/* NAKリトライオーバー・・・[同時] */
			IFM_SlaveError(E_IFS_NAK_Retry, E_IF_BOTH, (int)ucSlave);
			break;

		case 4:		/* 不当な応答 */
			switch (psAnswer->sHead.c_Kind[0]) {
				case 'A':
				case 'V':
				case IF_ACK:
				case IF_NAK:	/* 期待した応答が返されない・・・プロトコル異常[同時] */
					IFM_SlaveError(E_IF_Protocol, E_IF_BOTH, (int)ucSlave);
					break;

				default:		/* 不当(データ種別)電文[同時] */
					IFM_SlaveError(E_IFS_PACKET_WrongKind, E_IF_BOTH, (int)ucSlave);
					break;
			}
			break;

		case 5:		/* 無応答 */
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_IsSkip() 		                                   |*/
/*|             toScom_IsSkipRecover() 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		スキップ判定													   |*/
/*|		・無応答スキップ判定											   |*/
/*|		・無応答スキップ解除判定										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
/* 無応答スキップ判定 */
BOOL	toScom_IsSkip(uchar ucNowSlave)
{
	uchar	skipLimit = IFM_LockTable.toSlave.ucSkipNoAnswer;

	/* 無限回はスキップせずなのでカウント不要 */
	if (skipLimit >= 99) {
		return 0;
	}
	/* 有限回はスキップするのでカウント要 */
	else {
		/* スキップまで「猶予」あり？*/
		if (IFM_LockTable.sSlave[ucNowSlave].ucSkipCount) {
			IFM_LockTable.sSlave[ucNowSlave].ucSkipCount--;
			return 0;
		}
		/* スキップすることなった */
		else {
			/* 無応答(スキップ)[発生] */
			IFM_SlaveError(E_IFS_NO_Answer, E_IF_OCCUR, (int)ucNowSlave);
			IFM_LockTable.sSlave[ucNowSlave].ucSkipCount = skipLimit;	/* カウントは始めからやり直し */
			IFM_LockTable.sSlave[ucNowSlave].bSkip = 1;					/* スキップ中へ */
			IFM_LockTable.sSlave[ucNowSlave].bComm = 0;					/* スキップするので「子機への通信不要」へ */
			return 1;
		}
	}
}
/* 無応答スキップ解除判定 */
BOOL	toScom_IsSkipRecover(uchar ucNowSlave)
{
	uchar	skipLimit = IFM_LockTable.toSlave.ucSkipNoAnswer;

	/* 無限回はスキップせずなのでカウント不要 */
	if (skipLimit >= 99) {
		return 0;
	}
	/* 有限回はスキップ解除するのでカウント要 */
	else {
		/* スキップ解除まで「猶予」あり？*/
		if (IFM_LockTable.sSlave[ucNowSlave].ucSkipCount) {
			IFM_LockTable.sSlave[ucNowSlave].ucSkipCount--;
			return 0;
		}
		/* スキップ解除することなった */
		else {
			IFM_LockTable.sSlave[ucNowSlave].ucSkipCount = 1;	/* カウントは復活なので「１」へ */
			IFM_LockTable.sSlave[ucNowSlave].bSkip = 0;			/* スキップ解除へ */
			IFM_LockTable.sSlave[ucNowSlave].bComm = 1;			/* スキップ解除するので「子機への通信必要」へ */
			return 1;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_IsRetryOver() 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		リトライオーバー判定											   |*/
/*|		・リトライオーバーしたか否か？を返す。							   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_IsRetryOver(uchar ucNowSlave)
{
	uchar	retryLimit = IFM_LockTable.toSlave.ucRetry;

	/* 無限回はリトライしっぱなしなのでカウント不要 */
	if (retryLimit >= 99) {
		return 0;
	}
	/* 有限回はリトライオーバーが見たいのでカウント要 */
	else {
		/* 再送回数内なら、カウントダウンのみ */
		if (IFM_LockTable.sSlave[ucNowSlave].ucNakRetry) {
			IFM_LockTable.sSlave[ucNowSlave].ucNakRetry--;
			return 0;
		}
		/* 再送回数オーバーなら、*/
		else {							/* カウントは始めからやり直し */
			IFM_LockTable.sSlave[ucNowSlave].ucNakRetry = retryLimit;
			return 1;					/*「リトライオーバーした」を返す */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_ResetRetry() 	                                   |*/
/*|             toScom_ResetSkip() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		スキップ判定													   |*/
/*|		・NAKリトライ判定回数をセット（初期値設定）する					   |*/
/*|		・無応答スキップ判定回数をセット（初期値設定）する				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_ResetRetry(uchar ucNowSlave)
{
	IFM_LockTable.sSlave[ucNowSlave].ucNakRetry = IFM_LockTable.toSlave.ucRetry;
}
void	toScom_ResetSkip(uchar ucNowSlave)
{
	IFM_LockTable.sSlave[ucNowSlave].ucSkipCount = IFM_LockTable.toSlave.ucSkipNoAnswer;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_CheckMatrix()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		Check protocol from IFS received data	                     	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_CheckMatrix(uchar *pData, ushort uiLength)
{
	BOOL		answer;

	/* 応答不能で初期化 */
	answer  = 0;

	/* 受信データ長による判定 */
	if (uiLength < 5) {
		toScom_Matrix.State  = 1;			/* S1：ヘッダ文字待ち */
	}
	else if (uiLength < (5+2)) {
		toScom_Matrix.State  = 2;			/* S2：データサイズ待ち */
	}
	/* データサイズまでは受信した */
	else {
		if (uiLength < (5+2+1)) {
			toScom_Matrix.State  = 3;		/* S3：データ種別待ち */
		}
		else if (uiLength < (5+2+1+2)) {
			toScom_Matrix.State  = 4;		/* S4：ターミナルNo.待ち */
		}
		/* ターミナルNo.までは受信した */
		else {
			answer = 1;						/* 応答可能 */
			toScom_Matrix.State  = 0;		/* S0：受信待機（アイドル）へ */
		}
	}

	return answer;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendStatusToLock()                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		ロック管理タスクへの状態変化通知								   |*/
/*|		・ロック管理テーブル上のセンサー状態と変化があるときのみ通知する。 |*/
/*|		・通知は、それまでのメールを改め、キュー方式にする。			   |*/
/*|		・メール不採用の理由は以下。									   |*/
/*|			1)ロック管理タスクは、										   |*/
/*|			  対精算機通信タスクからの「ロック制御データ」を			   |*/
/*|			  本タスクへメール転送するため、							   |*/
/*|			  同時期に本タスクが「子機状態データ」をメールすると、		   |*/
/*|			  そのハンドリング手順にてデッドロックを引き起こす可能性あり。 |*/
/*|			2)ロック管理タスクは、本タスクからの「子機状態データ」と	   |*/
/*|			  対精算機通信タスクからの「ロック制御データ」を			   |*/
/*|			  同時期に受け取った場合、「子機状態データ」を優先処理すべき。 |*/
/*|			  メール通知にした場合ロック管理タスクは、その判定が面倒。	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-04-18	早瀬・変更。上記変更。                     |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendStatusToLock(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* キュー実体 */
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	int				ni;
	BOOL			bQfull;		/* 2005.07.12 早瀬・追加 */

#if 0
	BOOL		bSomeone;		/*（一車室でも）変化ありフラグ */
	/*
	*	状態データを通知すべきか？
	*	・変化を知る
	*
	*	2005.04.19 早瀬・考察
	*	本来、
	*	対子機通信タスクが「状態データを通知すべきか否か？」を判断する必要は無い。
	*	判断するのは、ロック管理タスクの役目であり、越権行為である。
	*	しかしながら、
	*	キューには限りがあるので「通知すべき状態データ」のみをキューイングすることは
	*	キュー数の節約になる。
	*	S-RAMに余裕があれば、『本来の役割分担』に徹し
	*	S-RAMに余裕が無ければ『越権行為』も止むを得ない。
	*
	*	※調整段階で、この条件コンパイルを生かすか？殺すか？を判断してください。
	*/
	bSomeone = 0;
	for (ni=0; ni < 8; ni++) {
		if (IFM_LockTable.sSlave[ucTerm].sLock[ni].unSensor.Byte != pAns->unBody.sA41.unSensor[ni].Byte) {
			bSomeone = 1;	/* 変化ありなので、*/
			break;			/*（もうこれ以上）調べる必要なし */
		}
	}
	if (!bSomeone) {		/* 変化無しなら、*/
		return;				/* 即リターン */
	}
#endif

	/*
	*	ここに来たら「変化あり」ということ
	*/
	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* キュー実体をリンク */
		bQfull = 0;									/* キューフル（子機状態リスト）[解除]かも */
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		bQfull = 1;									/* キューフル（子機状態リスト）[発生] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}

	/*「子機状態データ」キュー実体を作る */
	pQbody->sA41.sCommon.ucKind     = 'A';
	pQbody->sA41.sCommon.ucTerminal = ucTerm;
	for (ni=0; ni < 8; ni++) {
		pQbody->sA41.unSensor[ni]   = pAns->unBody.sA41.unSensor[ni];
	}
	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	/*
	*	2005.07.12 早瀬・修正
	*	キューフル発生/解除を判定した、そのときに「IFM_MasterError(IFM.c)」を呼び出してはならない。
	*	※詳しくは「PAYcom_NoticeError(PAYnt_net.c)」を参照のこと
	*/
	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* キューフル（子機状態リスト）[発生] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* キューフル（子機状態リスト）[解除]かも */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendVersionToLock()                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|		ロック管理タスクへのバージョン通知								   |*/
/*|		・子機バージョン情報を受信したとき呼び出され、					   |*/
/*|		  ロック管理タスクへ通知する。									   |*/
/*|		・通知は、それまでのメールを改め、キュー方式にする。			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-04-18	早瀬・変更。上記変更。                     |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendVersionToLock(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* キュー実体 */
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	BOOL			bQfull;		/* 2005.07.12 早瀬・追加 */

	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* キュー実体をリンク */
		bQfull = 0;									/* キューフル（子機状態リスト）[解除]かも */
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		bQfull = 1;									/* キューフル（子機状態リスト）[発生] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}

	/*「子機状態データ」キュー実体を作る */
	pQbody->sV56.sCommon.ucKind     = 'V';
	pQbody->sV56.sCommon.ucTerminal = ucTerm;
	memcpy(pQbody->sV56.ucVersion, pAns->unBody.sV56.c_Version, sizeof(pQbody->sV56.ucVersion));

	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	/*
	*	2005.07.12 早瀬・修正
	*	キューフル発生/解除を判定した、そのときに「IFM_MasterError(IFM.c)」を呼び出してはならない。
	*	※詳しくは「PAYcom_NoticeError(PAYnt_net.c)」を参照のこと
	*/
	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* キューフル（子機状態リスト）[発生] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* キューフル（子機状態リスト）[解除]かも */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendVersionToFLAP()                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|		フラップ管理タスクへのバージョン通知							   |*/
/*|		・子機バージョン情報を受信したとき呼び出され、					   |*/
/*|		  フラップ管理タスクへ通知する。								   |*/
/*|		・通知は、それまでのメールを改め、キュー方式にする。			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-04-18	早瀬・変更。上記変更。                     |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendVersionToFLAP(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* キュー実体 */
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	BOOL			bQfull;		/* 2005.07.12 早瀬・追加 */

	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* キュー実体をリンク */
		bQfull = 0;									/* キューフル（子機状態リスト）[解除]かも */
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		bQfull = 1;									/* キューフル（子機状態リスト）[発生] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}

	/*「子機状態データ」キュー実体を作る */
	pQbody->sFlapv76.sCommon.ucKind     = 'v';
	pQbody->sFlapv76.sCommon.ucTerminal = ucTerm;
	memcpy(pQbody->sFlapv76.ucVersion, pAns->unBody.sFlapv76.c_Version, sizeof(pQbody->sFlapv76.ucVersion));

	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	/*
	*	2005.07.12 早瀬・修正
	*	キューフル発生/解除を判定した、そのときに「IFM_MasterError(IFM.c)」を呼び出してはならない。
	*	※詳しくは「PAYcom_NoticeError(PAYnt_net.c)」を参照のこと
	*/
	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* キューフル（子機状態リスト）[発生] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* キューフル（子機状態リスト）[解除]かも */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendTestResultCRR()                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR基板折り返しテスト結果										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa		                                           |*/
/*| Date        :  2013/1/24                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendTestResultCRR(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* キュー実体 */
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	BOOL			bQfull;		/* 2005.07.12 早瀬・追加 */

	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* キュー実体をリンク */
		bQfull = 0;									/* キューフル（子機状態リスト）[解除]かも */
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		bQfull = 1;									/* キューフル（子機状態リスト）[発生] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}

	/*「子機状態データ」キュー実体を作る */
	pQbody->sFlapt74.sCommon.ucKind     = 't';
	pQbody->sFlapt74.sCommon.ucTerminal = ucTerm;
	memcpy(pQbody->sFlapt74.ucTestResult, pAns->unBody.sFlapt74.c_TestResult, sizeof(pQbody->sFlapt74.ucTestResult));

	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* キューフル（子機状態リスト）[発生] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* キューフル（子機状態リスト）[解除]かも */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Event2() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		シリアル受信エラー発生（SCIレベル）                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Event2(void)
{
	toScom_gotoIDLE();				/* アイドル状態へ戻る(タイマーも止める) */

/*
*	2005.04.28
*	フレーミングエラーが初めて発生した。
*	ので、そのリカバー。
*/
	toScom_SetNextPolling(0);		/* ポーリングを再開する */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Event3() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		リンク(伝送)レベルでタイムアウト発生（未使用）					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Event3(void)
{

	toScom_LinkTimerStop();			/* そのタイマ解除 */

	/* どの状態でタイムアウト発生したか？*/
	switch (toScom_Matrix.State) {
		case 0:		/* S0：ニュートラル */
			break;

		case 1:		/* S1：ヘッダ文字待ち */
		case 2:		/* S2：データサイズ待ち */
		case 3:		/* S3：データ種別待ち */
			break;

		case 4:		/* S4：ターミナルNo.待ち */
		case 5:		/* S5：データ待ち */
		case 6:		/* S6：CRC待ち */
			break;
	}
	toScom_InitMatrix();			/* ニュートラルへ */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Event4() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		Communication Line time-out occurred   	                   		   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		無応答/ポーリング時刻経過										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Event4(void)
{
	/* 回線監視タイマ解除 */
	toScom_LineTimerStop();

	/* 回線監視タイムアウトか（無応答）？*/
	if (toScom_toSlave.bWaitAnser) {

		/*
		*	子機がだんまり（存在しない？）なので「タイムアウト」した。
		*	・次回再送の対象となる。
		*	・応答パケットが帰ってこないので、引数には NULLポインタ指定
		*	  呼び出し先では、引数が NULLポインタなら「無応答」と判断してくれる。
		*/
		if( IFS_CRR_OFFSET > toScom_toSlave.ucNow ){	// CRR基板へのアクセスではない
			toScom_ChackAnswer(toScom_toSlave.ucNow, IFM_LockTable.sSlave[toScom_toSlave.ucNow].ucWhatLastCmd, (t_IF_Packet *)0);
		} else {					// CRR基板へのアクセスの場合
			toScom_ChackAnswer(toScom_toSlave.ucNow, IFM_LockTable.sSlave_CRR[ toScom_toSlave.ucNow - IFS_CRR_OFFSET ].ucWhatLastCmd, (t_IF_Packet *)0);
		}
	}
	/* ポーリング間隔経過か？*/
	else {
		toScom_Polling();			/* 子機へポーリング(状態要求を出す) */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Polling() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|											                       		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Polling(void)
{
struct t_IF_Queue	*pSend;
	int		ni;
	uchar	next, now;
	BOOL	bContinuePolling;
	ushort	Init_timer;

	/*
	*	既に、toS_main(toSmain.c) が
	*	メールボックスから受信メールを取り出し
	*	子機セレクティング・キューへ格納している。
	*	・初期設定データ
	*	・状態書込みデータ
	*	・状態要求データ（バージョン要求）・・・2005.04.14 早瀬・追加
	*		1)メール受信を検知
	*		2)メールありならキューにぶち込む
	*	ここでは、そのキューから取り出して適切な処理をする。
	*/

	/*
	*	いきなりセレクティング？
	*	・順番を狂わす優先的送信コマンドがあるか？
	*	・キューに溜まっているなら「優先的送信コマンドがある！」ということ。
	*/
	pSend = NULL;
	if( toScom_first == 1 ){	// 起動時1回目
		Init_timer = TOSCOM_INIT_TIME_FIRST;	// 3秒で送信
	} else {
		Init_timer = TOSCOM_INIT_TIME;			// 60秒毎に送信
	}
	if (LifePastTimGet(toScom_StartTime) > Init_timer){
		toScom_first = 0;						// 初期化フラグを落とす
		toScom_InitFlapData();					// CRR基板(フラップ)初期設定送信
		toScom_StartTime = LifeTimGet();
	}
	if (toScom_ucAckSlave == TOSCOM_NO_ACK_TERMINAL) {		// ACK受信端末なしで
		if(pSend = GetQueue(&toSque_Ctrl_Select)) {			// キューにデータがあれば送信

		/*
		*	新規送信で呼び出すための準備
		*	・現在の通信相手子機は、キュー実体のそれ
		*	・直前送信コマンドは「優先キューにある」（セレクティング最初のケース）
		*	・新規なので、再送カウンタ、スキップカウンタは初期値をセット
		*	★この「優先的送信コマンド」が送信失敗したら、（NAK返信か無応答だったら）
		*	  キューから外され順番ボードにつながれるので、次回はこのキューには無い。
		*/
		now = pSend->pData->sCommon.ucTerminal;				/* キュー実体の送信先子機を取得 */
		toScom_toSlave.ucNow = now;							/* 現在の通信相手にする */
		if( IFS_CRR_OFFSET > now ){													/* CRR基板へのアクセスではない	*/
			IFM_LockTable.sSlave[now].ucWhereLastCmd = 1;		/*「優先キューにある」*/
		} else {
			IFM_LockTable.sSlave_CRR[now - IFS_CRR_OFFSET].ucWhereLastCmd = 1;		/*「優先キューにある」*/
		}
		toScom_Send_or_NextRetry(now, 0);					/* 新規送信 */
		toScom_ResetRetry(now);								/* 再送カウンタ初期化 */
		toScom_ResetSkip(now);								/* スキップカウンタ初期化 */
		}
	}
	/*
	*	ポーリング
	*	・キューに溜まっていないから、順番に「状態要求/再送コマンド」を出す。
	*	・順番ボードに無い・・・状態要求データ
	*	  順番ボードにある・・・前回無応答扱いとなった、以下の何れか
	*		(49H：'I')初期設定データ
	*		(57H：'W')状態書き込み
	*		(52H：'R')状態要求（バージョン要求）
	*/
	if(pSend == NULL) {										// ACK受信端末ありor送信データ無し
		if(toScom_ucAckSlave == TOSCOM_NO_ACK_TERMINAL) {
			next = (uchar)(toScom_toSlave.ucOrder + 1);		/* 次の順番の通信相手を仮決めする */
		}
		else {
			// Wコマンド送信後の動作中状態を取得する為に、ACKを受信した端末に対してポーリングを行う
			next = toScom_ucAckSlave;
			toScom_ucAckSlave = TOSCOM_NO_ACK_TERMINAL;		// ACK待ち端末無しとする
		}
		for (ni=0; ni < IF_SLAVE_MAX; ni++, next++) {		/* 子の台数分 */
			if (next >= IF_SLAVE_MAX) {						/* 次が最大数以上なら、*/
				next = 0;									/* ローテーション */
			}
			/* 通信可能な子機に限る（スキップ中は「FALSE」なので対象外）*/
			if (IFM_LockTable.sSlave[next].bComm) {			/* 次の子機へ通信する必要あるか？*/
				toScom_toSlave.ucOrder = (uchar)next;		/* 次の順番の通信相手を確定 */

				/*
				*	新規送信で呼び出すための準備
				*	・現在の通信相手子機は、上で決めた順番
				*	・直前送信コマンドは「いま、何処にあるか？」判定
				*		順番ボード上に、既にある・・・（セレクティング再送のケース）
				*		順番ボード上に、無い・・・・・（ポーリングのための「状態要求データ」のケース）
				*/
				toScom_toSlave.ucNow = next;			/* 上で決めた順番の相手にする */
				if (IFM_LockTable.sSlave[next].psNext) {
					IFM_LockTable.sSlave[next].ucWhereLastCmd = 2;	/*「順番ボードにある」*/
				} else {
					IFM_LockTable.sSlave[next].ucWhereLastCmd = 0;	/*「何処にもない」*/
				}
				toScom_Send_or_NextRetry(next, 0);			/* 新規送信/再送 */
				break;										/* この一回だけポーリグ */
		 	}
		 	/* 通信不能な子機でも・・・ */
		 	else if (IFM_LockTable.sSlave[next].bSkip) {	/* スキップ中なら、*/
				toScom_IsSkipRecover(next);					/* 復活のチャンスあり */
			}
		}

		/* 全ての子機が、通信不能になった時のために・・・*/
		bContinuePolling = toScom_Is_LineTimerStop();		/* タイマが動いてないなら、*/
		if (bContinuePolling) {								/* 二度と本関数に来なくなることを避けるため */
			toScom_SetNextPolling(5);						/* 無応答扱いでポーリング間隔を決める */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_ComPortReset() 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		communication device reset. and other process. (*1 proc)		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba                                                 |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_ComPortReset(void)
{
	/* SCIポートを停止(割り込みも禁止にする) */
	toSsci_Stop();

	/* 受信ドライバを初期化 */
	toScomdr_Init();

	/*
	*	SCIポートを初期化
	*	・調歩同期方式
	*	・2400/$9600/19200/38400bps
	*	・8ビットデータ長
	*	・ストップビット=1
	*	・パリティなし
	*/
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SendI49()                                           |*/
/*|             toScom_SendR52()                                           |*/
/*|             toScom_SendW57()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		send polling/selecting code to TF								   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	input  : terminal number to be polling/selecting.					   |*/
/*|	return : 1=OK,  0=Couldn't send 									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_SendI49(t_IF_I49_mail *pI49)
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	ushort		ni;
	BOOL		bResult;
	int			iSlave;

	/* ヘッダ */
	iSlave   = (int)pI49->sCommon.ucTerminal;
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_IF_Body_I49);	/* 初期設定データ長を取得しておく */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* ヘッダ文字列("IFCOM"固定) */
c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);					/* データサイズ */
			    pPacket->sHead.c_Kind[0]     = pI49->sCommon.ucKind;					/* データ種別（'I'：49H）*/
	intoas(  &pPacket->sHead.c_TerminalNo[0],  (ushort)(iSlave + 1), 2);				/* 相手ターミナルNo. */
	/* ボディ */
	for (ni=0; ni < 8; ni++) {									/* ロック装置の接続（状態）*/
		intoas(&pPacket->unBody.sI49.c_Connect[ni], (ushort)pI49->ucConnect[ni], 1);
	}															/* 入出庫車両検知タイマー・・・x150msecはどうする？*/
	intoas(&pPacket->unBody.sI49.c_EntryTimer[0],   (ushort)pI49->usEntryTimer, 2);
	intoas(&pPacket->unBody.sI49.c_ExitTimer[0],    (ushort)pI49->usExitTimer,  2);
	/* CRC */
	/* 対象サイズは、CRC自身を含まず。CRC16-CCITT 算出関数呼び出し時には、「右シフト」を指定する */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sI49.ucCRC16, CRC16_R_SHIFT);

	/* 子機へ送信 */
	toScom_toSlave.ucNow  = (uchar)iSlave;					/* 現在の通信相手子機 */
	IFM_LockTable.sSlave[iSlave].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* その子機への送信コマンドを憶えておく */
	toScom_toSlave.bWaitAnser = 1;								/* 応答待ち中へ */
	toScom_toSlave.ucActionLock = 0xff;							/* ロック装置No.は、送る対象外 */
	bResult = toScom_WaitSciSendCmp(dataSize);						/* 送信完了を待つ */

//デバッグ
toScom_toSlave.dbg.LastCmd = pPacket->sHead.c_Kind[0];
toScom_toSlave.dbg.sndI49++;

	return	bResult;
}

BOOL	toScom_SendR52(int iSlave, uchar ucRequest)
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	BOOL		bResult;

	/* ヘッダ */
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_IF_Body_R52);	/* 状態要求データ長を取得しておく */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* ヘッダ文字列("IFCOM"固定) */
c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);			/* データサイズ */
			    pPacket->sHead.c_Kind[0]     = 'R';								/* データ種別（'R'：52H）*/
	intoas(  &pPacket->sHead.c_TerminalNo[0],  (ushort)(iSlave + 1), 2);		/* 相手ターミナルNo. */
	/* ボディ */
	intoas(&pPacket->unBody.sR52.c_Request[0],(ushort)ucRequest, 1);			/* 要求内容（'0'：子機状態データ要求/'1'：バージョン要求）*/
	/* CRC */
	/* 対象サイズは、CRC自身を含まず。CRC16-CCITT 算出関数呼び出し時には、「右シフト」を指定する */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sR52.ucCRC16, CRC16_R_SHIFT);

	/* 子機へ送信 */
	toScom_toSlave.ucNow  = (uchar)iSlave;					/* 現在の通信相手子機 */
	/*「子機状態データ要求」？*/
	if (ucRequest == 0 || ucRequest == 2) {
		IFM_LockTable.sSlave[iSlave].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* その子機への送信コマンドを('R'のまま)憶えておく */
	}
	/* 「バージョン要求」*/
	else {
		IFM_LockTable.sSlave[iSlave].ucWhatLastCmd = 'V';		/* その子機への送信コマンドは'V'に書き換え（強引？）*/
	}
	toScom_toSlave.bWaitAnser = 1;								/* 応答待ち中へ */
	toScom_toSlave.ucActionLock = 0xff;							/* ロック装置No.は、送る対象外 */
	bResult = toScom_WaitSciSendCmp(dataSize);						/* 送信完了を待つ */

//デバッグ
toScom_toSlave.dbg.LastCmd = pPacket->sHead.c_Kind[0];
toScom_toSlave.dbg.sndR52++;

	return	bResult;
}

BOOL	toScom_SendW57(t_IF_W57_mail *pW57)
/*「ucAction」は使わない。停止/動作では無く「(0)動作完結/(1)動作中を表す状態（書き込み不可）」となった */
/*「ucMaker」を追加した。ロック装置メーカーID（(49H：'I')初期設定データの「ロック装置の接続」同様）	2005.03.30 早瀬・追加 */
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	BOOL		bResult;
	int			iSlave;

	/* ヘッダ */
	iSlave   = (int)pW57->sCommon.ucTerminal;
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];											/* 送信バッファの先頭をポイント */
	dataSize = sizeof(t_IF_Header) + sizeof(t_IF_Body_W57);							/* 状態書込みデータ長を取得しておく */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* ヘッダ文字列("IFCOM"固定) */
c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);					/* データサイズ */
			    pPacket->sHead.c_Kind[0]     = pW57->sCommon.ucKind;					/* データ種別（'W'：57H）*/
	intoas(  &pPacket->sHead.c_TerminalNo[0],  (ushort)(iSlave + 1), 2);				/* 相手ターミナルNo. */
	/* ボディ */
	if( LKcom_Search_Ifno( (uchar)(iSlave + 1) ) == 0) {
	intoas(  &pPacket->unBody.sW57.c_LockOnTime[0], (ushort)pW57->usLockOnTime,  2);	/* ロック装置閉時の信号出力時間 */
	intoas(  &pPacket->unBody.sW57.c_LockOffTime[0],(ushort)pW57->usLockOffTime, 2);	/* ロック装置開時の信号出力時間 */
	intoas(  &pPacket->unBody.sW57.c_RoomNo[0],     (ushort)(pW57->ucLock + 1),  1);	/* ロック装置No. */
	intoas(  &pPacket->unBody.sW57.c_MakerID[0],    (ushort)pW57->ucMakerID, 1);		/* ロック装置メーカーID		2005.03.30 早瀬・追加 */
				pPacket->unBody.sW57.unCommand =              pW57->unCommand;			/* 書き込みセンサー状態を直代入 */
																						/*	bit4：(0)ロックしろ/(1)ロック解除しろ */
																						/*	bit7：(0)メンテナンス解除/(1)メンテナンス中 */
	}
	else {		// フラップ
		intoas(  &pPacket->unBody.sW57.c_LockOnTime[0], 0,  2);	/* ロック装置閉時の信号出力時間 */
		intoas(  &pPacket->unBody.sW57.c_LockOffTime[0], 0, 2);	/* ロック装置開時の信号出力時間 */
		pPacket->unBody.sW57.c_RoomNo[0] ='0';					/* ロック装置No. */
		pPacket->unBody.sW57.c_MakerID[0] = '0';				/* ロック装置メーカーID		2005.03.30 早瀬・追加 */
		pPacket->unBody.sW57.unCommand = pW57->unCommand;		/* 書き込みセンサー状態を直代入 */
	}
	/* CRC */
	/* 対象サイズは、CRC自身を含まず。CRC16-CCITT 算出関数呼び出し時には、「右シフト」を指定する */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sW57.ucCRC16, CRC16_R_SHIFT);

	/* 子機へ送信 */
	toScom_toSlave.ucNow  = (uchar)iSlave;					/* 現在の通信相手子機 */
	IFM_LockTable.sSlave[iSlave].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* その子機への送信コマンドを憶えておく */
	toScom_toSlave.bWaitAnser = 1;								/* 応答待ち中へ */
	toScom_toSlave.ucActionLock = (uchar)pW57->ucLock;			/* 送信する「ロック装置No.」を保持しておく */
	bResult = toScom_WaitSciSendCmp(dataSize);						/* 送信完了を待つ */
	return	bResult;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SendI49_CRR()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR基板(FLAP)初期設定											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2012/12/6                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toScom_SendI49_CRR(t_Flap_I49_mail *pI49)
{
	t_IF_Packet	*pPacket;
	ushort		dataSize;
	ushort		ni;
	BOOL		bResult;
	int			iSlave;

	/* ヘッダ */
	iSlave   = (int)pI49->sCommon.ucTerminal;
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_FLAP_Body_I49);				/* 初期設定データ長を取得しておく */
	memcpy( &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* ヘッダ文字列("IFCOM"固定) */
	c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);		/* データサイズ */
	pPacket->sHead.c_Kind[0] = pI49->sCommon.ucKind;						/* データ種別（'I'：49H）*/
	pPacket->sHead.c_TerminalNo[0] = 'F';
	switch( iSlave - IFS_CRR_OFFSET ){										/* 相手CRR基板No. */
	case 0:
		pPacket->sHead.c_TerminalNo[1] = '1';
		break;
	case 1:
		pPacket->sHead.c_TerminalNo[1] = '2';
		break;
	case 2:
		pPacket->sHead.c_TerminalNo[1] = '3';
		break;
	default:	break;
	}
	/* ボディ */
	for (ni=0; ni < IFS_CRRFLAP_MAX; ni++) {									/* ロック装置の接続（状態）*/
		intoas(&pPacket->unBody.sFlapI49.ucConnect[ni][0], (ushort)pI49->ucConnect[ni], 2);
	}
	/* CRC */
	/* 対象サイズは、CRC自身を含まず。CRC16-CCITT 算出関数呼び出し時には、「右シフト」を指定する */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sFlapI49.ucCRC16, CRC16_R_SHIFT);

	/* 子機へ送信 */
	toScom_toSlave.ucNow  = (uchar)iSlave;									/* 現在の通信相手子機 +100したままの値にしておく*/
	IFM_LockTable.sSlave_CRR[iSlave- IFS_CRR_OFFSET].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* その子機への送信コマンドを憶えておく */
	toScom_toSlave.bWaitAnser = 1;											/* 応答待ち中へ */
	toScom_toSlave.ucActionLock = 0xff;										/* ロック装置No.は、送る対象外 */
	bResult = toScom_WaitSciSendCmp(dataSize);								/* 送信完了を待つ */

	return	bResult;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SendV56()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR基板(FLAP)バージョン確認										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2012/12/6                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toScom_SendV56(int iSlave)
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	BOOL		bResult;

	/* ヘッダ */
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_FLAP_Body_V56);					/* 初期設定データ長を取得しておく */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* ヘッダ文字列("IFCOM"固定) */
	c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);			/* データサイズ */
			    pPacket->sHead.c_Kind[0]     = 'V';								/* データ種別（'V'：56H）*/
	pPacket->sHead.c_TerminalNo[0] = 'F';
	switch( iSlave - IFS_CRR_OFFSET ){											/* 相手CRR基板No. */
	case 0:
		pPacket->sHead.c_TerminalNo[1] = '1';
		break;
	case 1:
		pPacket->sHead.c_TerminalNo[1] = '2';
		break;
	case 2:
		pPacket->sHead.c_TerminalNo[1] = '3';
		break;
	default:	break;
	}
	/* ボディ */
	memset( pPacket->unBody.sFlapV56.dummy, '0' ,8);
	/* CRC */
	/* 対象サイズは、CRC自身を含まず。CRC16-CCITT 算出関数呼び出し時には、「右シフト」を指定する */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sFlapV56.ucCRC16, CRC16_R_SHIFT);

	/* 子機へ送信 */
	toScom_toSlave.ucNow  = (uchar)iSlave;									/* 現在の通信相手子機 +100したままの値にしておく*/
	IFM_LockTable.sSlave_CRR[iSlave- IFS_CRR_OFFSET].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* その子機への送信コマンドを憶えておく */
	toScom_toSlave.bWaitAnser = 1;											/* 応答待ち中へ */
	toScom_toSlave.ucActionLock = 0xff;										/* ロック装置No.は、送る対象外 */
	bResult = toScom_WaitSciSendCmp(dataSize);								/* 送信完了を待つ */

	return	bResult;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SendT54()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR基板折り返しテスト											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2012/1/24                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toScom_SendT54(uchar iSlave)
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	BOOL		bResult;

	/* ヘッダ */
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_FLAP_Body_T54);					/* 初期設定データ長を取得しておく */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* ヘッダ文字列("IFCOM"固定) */
	c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);			/* データサイズ */
			    pPacket->sHead.c_Kind[0]     = 'T';								/* データ種別（'T'：54H）*/
	pPacket->sHead.c_TerminalNo[0] = 'F';
	switch( iSlave - IFS_CRR_OFFSET ){											/* 相手CRR基板No. */
	case 0:
		pPacket->sHead.c_TerminalNo[1] = '1';
		break;
	case 1:
		pPacket->sHead.c_TerminalNo[1] = '2';
		break;
	case 2:
		pPacket->sHead.c_TerminalNo[1] = '3';
		break;
	default:	break;
	}
	/* ボディ */
	memset( pPacket->unBody.sFlapT54.dummy, '0' ,8);
	/* CRC */
	/* 対象サイズは、CRC自身を含まず。CRC16-CCITT 算出関数呼び出し時には、「右シフト」を指定する */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sFlapT54.ucCRC16, CRC16_R_SHIFT);
	
	/* 子機へ送信 */
	toScom_toSlave.ucNow  = iSlave;											/* 現在の通信相手子機 +100したままの値にしておく*/
	IFM_LockTable.sSlave_CRR[iSlave- IFS_CRR_OFFSET].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* その子機への送信コマンドを憶えておく */
	toScom_toSlave.bWaitAnser = 1;											/* 応答待ち中へ */
	toScom_toSlave.ucActionLock = 0xff;										/* ロック装置No.は、送る対象外 */
	bResult = toScom_WaitSciSendCmp(dataSize);								/* 送信完了を待つ */

	toScom_LineTimerStop();	// ポーリングタイマーを停止する

	return	bResult;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_WaitSciSendCmp()    	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		SCI送信（送信完了待ち）											   |*/
/*|		・指定された送信バイト数分、									   |*/
/*|		・送信バッファ先頭からSCI出力する。								   |*/
/*|		・送信完了まで待ちます。										   |*/
/*|		★送信完了待ちタイムアウトは「？」secです。						   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	input  : 送信バイト数												   |*/
/*|	return : 送信結果													   |*/
/*|				1 = 正常終了（送信完了）								   |*/
/*|				0 = 異常終了（送信未完。？秒待っても送信完了にならない）   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba                                                 |*/
/*| Date        :  2004-01-27                                              |*/
/*| Update      :  2005-01-18	早瀬・改造                                 |*/
/*| 				・送信前のWaitは不要			                       |*/
/*| 				・RS485通信なので送信ドライバ(ﾄﾗﾝｽﾐｯﾀ)許可/禁止を対処  |*/
/*| Update      :  2005-06-13	早瀬・改造                                 |*/
/*| 				・送信前のWaitを復活			                       |*/
/*| 				・RS485通信の送信ドライバ(ﾄﾗﾝｽﾐｯﾀ)許可/禁止を          |*/
/*| 				  割り込みハンドラへ移動                               |*/
/*| 				・関数の戻り値をBoolean型に変更                        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_WaitSciSendCmp(unsigned short Length)
{
	ulong	StartTime;

	/* 送信前の待ち・・・子機に対しても「待ち」必要 */
	if (toScom_Timer.usSendWait != 0) {
		xPause2ms((ulong)toScom_Timer.usSendWait);
	}

	/* 送信要求はNG ? */
	if (!toSsci_SndReq(Length)) {
		return	(BOOL)0;					/* 送信エラーでリターン */
	}

	/* 送信完了を待つ */
	StartTime = LifeTim2msGet();
	// 精算完了時に音声再生とFROM書込みのためタイムアウトしてしまうため時間を長く取る
	while (0 == LifePastTim2msGet(StartTime, 500L)) {		/* 1sec経過したか？*/
		taskchg( IDLETSKNO );				// IDLEタスクへ切り替え 
		if (1 == toSsci_IsSndCmp()) {		/* 送信完了か？*/

			/* ここから応答監視スタート */
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);

			return	(BOOL)1;				/* 送信正常終了でリターン */
		}
	}
	/* ここから応答監視スタート */
	// ここで監視をスタートしないとポーリングが停止する
	toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);
	/* ここに来るのは、現在時刻＋100msec(1sec？)経過しても送信完了しなかったとき。*/
	return	(BOOL)0;						/* 送信エラーでリターン */
	/* This case will be only flow control useing. 	*/
	/* in this time, flow control doesn't use.		*/
	/* Therefore I must return OK status.			*/
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_gotoIDLE() 		    	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		phase change to IDLE                     						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba                                                 |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_gotoIDLE(void)
{
	toScom_2msTimerStop();			/* 文字間監視タイマ解除 */
	toScom_LinkTimerStop();			/* 受信監視タイマ解除 */
	toScom_LineTimerStop();			/* 回線監視タイマ解除 */

	toScom_InitMatrix();			/* toS端末間リンクレベルをアイドルへ初期化 */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_InitMatrix()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		initialize toS protocol information data	                   	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_InitMatrix(void)
{
	toScom_Matrix.State = 0;		/* ニュートラル(S0)へ */

	toScom_LinkTimerStop();			/* 受信監視タイマ解除 */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_NextInitData()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		初期設定データ優先送信						                   	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_NextInitData(uchar ucSlave)
{
t_IFM_Queue			*pQbody;	/* キュー実体 */
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	int				ni;
	BOOL			bQfull;		/* 2005.07.12 早瀬・追加 */

	if(!( GetCarInfoParam() & 0x01 )){// 駐輪設定なし
		// 通信I/Fがロック式/フラップ式共に共通であるがロック式のみ初期設定データが必要となるため
		return;
	}
	//NOTE:初期設定データ自身はPAYcom_InitDataSave()関数でセットされる

	/*
	*	その子機へ「初期設定データ」を送る。
	*	・次のセレクティングのために、キューイングする。
	*	・いままでの、その子機の情報はチャラにする。
	*	  注）「何を」チャラにするか？
	*		1)センサー情報
	*		2)スキップ中フラグ
	*		3)バージョン
	*		4)他は．．．なに？
	*/
	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* 実体をリンク */
		bQfull = 0;									/* キューフル（セレクティング待ちリスト）[解除]かも */
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		bQfull = 1;									/* キューフル（セレクティング待ちリスト）[発生] */
		pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}
	/* ロック装置管理テーブルからそれぞれの子機用に変換 */
	pQbody->sCommon.ucKind     = 'I';											/* 初期設定データ */
	pQbody->sCommon.ucTerminal = ucSlave;										/* ターミナルNo.（どの子機へか？を特定する）*/
	for (ni=0; ni < 8; ni++) {
		pQbody->sI49.ucConnect[ni] = IFM_LockTable.sSlave[ucSlave].sLock[ni].ucConnect;	/* ロック装置の接続 */
	}
	pQbody->sI49.usEntryTimer  = IFM_LockTable.sSlave[ucSlave].usEntryTimer;	/* 入庫車両検知タイマー */
	pQbody->sI49.usExitTimer   = IFM_LockTable.sSlave[ucSlave].usExitTimer;		/* 出庫車両検知タイマー */
	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));

	/*
	*	2005.07.12 早瀬・修正
	*	キューフル発生/解除を判定した、そのときに「IFM_MasterError(IFM.c)」を呼び出してはならない。
	*	※詳しくは「PAYcom_NoticeError(PAYnt_net.c)」を参照のこと
	*/
	if (bQfull) {
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);				/* キューフル（セレクティング待ちリスト）[発生] */
	} else {
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);			/* キューフル（セレクティング待ちリスト）[解除]かも */
	}

	/* いままでの、その子機の情報はチャラにする */
	for (ni=0; ni < 8; ni++) {
		IFM_LockTable.sSlave[ucSlave].sLock[ni].unSensor.Byte = 0;			/* センサー情報をクリア */
	}
	IFM_LockTable.sSlave[ucSlave].bSkip = 0;								/* スキップ中を解除 */
	memset(IFM_LockTable.sSlave[ucSlave].cVersion, 0, 8);					/* バージョンをクリア */
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_InitFlapData()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		Flap初期設定データ送信						                   	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2012/12/6                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_InitFlapData()
{
t_IFM_Queue			*pQbody;	/* キュー実体 */
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	int				i,ni;
	BOOL			bQfull;	

	/* ロック装置管理テーブルからそれぞれの子機用に変換 */
	for( i = 0 ; i < IFS_CRR_MAX ; i++ ){
		//	通信する必要がないCRR基板(ターミナルNo.が振られていない）は送信しない
		if (!IFM_LockTable.sSlave_CRR[i].bComm ){
			continue;
		}
		/* 空きキュー在るか？*/
		pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* 空き実体を取得 */
		pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* 空き本体を取得 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* 実体をリンク */
			bQfull = 0;									/* キューフル（セレクティング待ちリスト）[解除]かも */
		}
		/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
		else {
			bQfull = 1;									/* キューフル（セレクティング待ちリスト）[発生] */
			pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* キュー(先頭)を外す */
			pQbody = pQbuff->pData;
		}

		pQbody->sCommon.ucKind     = 'I';											/* 初期設定データ */
		pQbody->sCommon.ucTerminal = i + IFS_CRR_OFFSET;							/* CRR基板No. +100することでターミナルNo.との差別化をする*/
		for (ni=0; ni < IFS_CRRFLAP_MAX; ni++) {
			pQbody->sFlapI49.ucConnect[ni] = IFM_LockTable.ucConnect_Tbl[i][ni];	// ターミナル番号の指定
		}
		/* 末尾へキューイング */
		EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
		if (bQfull) {
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);				/* キューフル（セレクティング待ちリスト）[発生] */
		} else {
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);			/* キューフル（セレクティング待ちリスト）[解除]かも */
		}
	}

}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Send_or_NextRetry()                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		新規送信/連続再送/次回再送										   |*/
/*|		・子機への電文送信において、新規送信/連続再送/次回再送の		   |*/
/*|		  それぞれのケース別に応じた送信プランを実行する。				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_Send_or_NextRetry(uchar ucNowSlave, uchar what_do)
{
struct t_IF_Queue	*pWhat;		/* 調べるキュー */
t_IFM_Queue			*pMail;		/* 送信/移動するバケット */
	uchar			where;
	uchar			nowCmd;
	BOOL			bRetryOver;
	BOOL			bSkip;
	uchar			ucRequest;
	BOOL			bRet = 0;

	/*
	*	子機からACKが返されたときのみ、キューを外すのが正解。
	*	NAKが返されたら再送すべきだから．．．。
	*
	*	2005.04.04
	*	再送は、連続ではなく次回のセレクティングに廻されるので
	*	一旦外して再度末尾につなぎ直すことで、対処。
	*
	*	2005.04.11
	*	NAK再送は、連続に変更された。
	*	よって、リトライ回数内なら即再送する。
	*	なので、キュー/順番ボードに残したまま。
	*	無応答は、次回ポーリング順番のとき再送する。
	*	なので、順番ボードにつなぎ換える。
	*/
	if( IFS_CRR_OFFSET > ucNowSlave ){							/* CRR基板へのアクセスではない	*/
		where = IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd;
	} else {
		where = IFM_LockTable.sSlave_CRR[ucNowSlave - IFS_CRR_OFFSET].ucWhereLastCmd;
	}
	/* 何をやるか？は、通信結果次第 */
	switch (what_do) {

		/*
		*	送信のみ（結果なし）
		*	・送信コマンドを決める
		*	・送信する。
		*/
		case 0:
			/* 何処にあるか？で取ってくる位置が異なる */
			switch (where) {
				case 0:		/* 何処にもない（ポーリングのための「状態要求データ」のケース）*/
					nowCmd = 'R';	/*「状態要求データ」*/
					break;
				case 1:		/* 優先キューにある（セレクティング最初のケース）*/
					pWhat = GetQueue(&toSque_Ctrl_Select);
					nowCmd = pWhat->pData->sCommon.ucKind;
					pMail = (t_IFM_Queue *)pWhat->pData;

					/* 既に順番待ちがいる→こいつが完結するまで新たなコマンドは出せないのでは？ */
					if( IFS_CRR_OFFSET > ucNowSlave ){			/* CRR基板へのアクセスではない	*/
						if (IFM_LockTable.sSlave[ucNowSlave].psNext) {
						}
					}
					break;
				case 2:		/* 順番ボードにある（セレクティング再送のケース。無応答のため）*/
					pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
					nowCmd = pMail->sCommon.ucKind;
					break;
				default:
					return bRet;
			}
			/* それが何のコマンドか？で、呼び出す送信関数が異なる */
			switch (nowCmd) {
				case 'I':	/* 初期設定デーデータ */
					if( IFS_CRR_OFFSET > ucNowSlave ){	// CRR基板へのアクセスではない
						toScom_SendI49(&pMail->sI49);
					} else {
						toScom_SendI49_CRR(&pMail->sFlapI49);
					}
					break;
				case 'W':	/* 状態書き込みデータ */
					toScom_SendW57(&pMail->sW57);
					break;
				case 'R':	/* 状態要求データ */
					if( IFS_CRR_OFFSET > ucNowSlave ){	// CRR基板へのアクセスではない
						if (where == 0) {		/* ポーリングなら、子機状態データ要求 */
							ucRequest = 0;
						} else {				/* キューなら、(たぶん)バージョン要求 */
							ucRequest = pMail->sR52.ucRequest;
						}
						toScom_SendR52((int)ucNowSlave, ucRequest);
					} else {
						toScom_SendV56((int)ucNowSlave);
					}
					break;
				case 'T':	/* 折り返しテスト */
					toScom_SendT54(ucNowSlave);
					break;
				default:
					return bRet;
			}
			break;

		/*
		*	通信手順成功（正常応答が返された）
		*	・送信済みコマンドの後始末
		*	  1)優先キューにあるなら、外す
		*	  2)順番ボードには「なし」にする
		*	  3)直前送信コマンドは「何処にもない」にする
		*	・再送カウンタ初期化
		*	・スキップカウンタ初期化
		*/
		case 1:
			/* 何処にあったか？で後始末が異なる */
			switch (where) {
				case 0:		/* 何処にもない（ポーリングのための「状態要求データ」のケース）*/
					break;								/* 何もしない */
				case 1:		/* 優先キューにある（セレクティング最初のケース）*/
					pWhat = DeQueue(&toSque_Ctrl_Select, 0);	/* (優先)キューを外す。*/
					pMail = (t_IFM_Queue *)pWhat->pData;
					/*
					*	2005.05.17 早瀬・追加
					*	「正常応答が返された」ってことは相手子機に届いたってこと。
					*	なので、この関数を呼び出し（LCM\LCMmain.c にある）
					*	『状態書き込み』電文だったなら「動作中ビットを立てる」
					*/
					LCM_SetStartAction(pMail);
					break;
				case 2:		/* 順番ボードにある（セレクティング再送のケース。無応答のため）*/
					pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
					if (!pMail) {
						break;
					}
					pMail->sCommon.bUsedByQue = 0;		/* 順番ボードにあったものは「未使用」にする */
					/*
					*	2005.05.17 早瀬・追加
					*	「正常応答が返された」ってことは相手子機に届いたってこと。
					*	なので、この関数を呼び出し（LCM\LCMmain.c にある）
					*	『状態書き込み』電文だったなら「動作中ビットを立てる」
					*/
					LCM_SetStartAction(pMail);
					break;
				default:
					break;
			}
			IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)0;	/* 順番ボードには「なし」にする */
			IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 0;		/* 直前送信コマンドは「何処にもない」にする */

			toScom_ResetRetry(toScom_toSlave.ucNow);	/* 再送カウンタ初期化 */
			toScom_ResetSkip(toScom_toSlave.ucNow);		/* スキップカウンタ初期化 */
			break;

		/*
		*	通信手順失敗（NAK返信されたので、連続再送）
		*	・再送可能/リトライオーバーを判定
		*	・再送可能なら
		*		1)再送コマンドを決める
		*		2)再送する。
		*		3)送信したコマンドを憶えておく
		*	・リトライオーバーなら、
		*		1)送信済みコマンドの後始末（通信手順成功時と同様）
		*	・スキップカウンタ初期化
		*/
		case 2:
			/* 再送可能？*/
			bRetryOver = toScom_IsRetryOver(ucNowSlave);	/* 再送可能/リトライオーバーで処理が異なる */
			if (!bRetryOver) {

				/* 何処にあったか？で取ってくる位置が異なる。*/
				switch (where) {
					case 0:		/* 何処にもない（ポーリングのための「状態要求データ」のケース）*/
						nowCmd = 'R';	/*「状態要求データ」*/
						break;
					case 1:		/* 優先キューにある（セレクティング最初のケース）*/
						pWhat = GetQueue(&toSque_Ctrl_Select);
						nowCmd = pWhat->pData->sCommon.ucKind;
						pMail = (t_IFM_Queue *)pWhat->pData;
						break;
					case 2:		/* 順番ボードにある（セレクティング再送のケース。無応答のため）*/
						pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
						nowCmd = pMail->sCommon.ucKind;
						break;
					default:
						return bRet;
				}
				/* それが何のコマンドか？で、呼び出す送信関数が異なる */
				switch (nowCmd) {
					case 'I':	/* 初期設定デーデータ */
						toScom_SendI49(&pMail->sI49);
						break;
					case 'W':	/* 状態書き込みデータ */
						toScom_SendW57(&pMail->sW57);
						break;
					case 'R':	/* 状態要求データ */
						if (where == 0) {		/* ポーリングなら、子機状態データ要求 */
							ucRequest = 0;
						} else {				/* キューなら、(たぶん)バージョン要求 */
							ucRequest = pMail->sR52.ucRequest;
						}
						toScom_SendR52((int)ucNowSlave, ucRequest);
						break;
					default:
						return bRet;
				}
				bRet = 1;	/* NAK再送した */
			}
			/* リトライオーバー */
			else {
				/* 何処にあったか？で後始末が異なる */
				switch (where) {
					case 0:		/* 何処にもない（ポーリングのための「状態要求データ」のケース）*/
						break;								/* 何もしない */
					case 1:		/* 優先キューにある（セレクティング最初のケース）*/
						DeQueue(&toSque_Ctrl_Select, 0);	/* (優先)キューを外す。*/
						break;
					case 2:		/* 順番ボードにある（セレクティング再送のケース。無応答のため）*/
						pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
						if (!pMail) {
							break;
						}
						pMail->sCommon.bUsedByQue = 0;	/* 順番ボードにあったものは「未使用」にする */
						break;
					default:
						break;
				}
				IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)0;	/* 順番ボードには「なし」にする */
				IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 0;		/* 直前送信コマンドは「何処にもない」にする */
			}

			toScom_ResetSkip(toScom_toSlave.ucNow);		/* スキップカウンタ初期化 */
			break;

		/*
		*	通信手順失敗（無応答だったので、次回再送）
		*	・スキップあり/なしを判定
		*	・スキップなしなら、
		*		1)優先キューにあるなら、付け替え（次回再送のために）
		*	・スキップありなら、
		*		1)送信済みコマンドの後始末（通信手順成功時と同様）
		*/
		case 3:
			/* まだスキップではない？*/
			bSkip = toScom_IsSkip(ucNowSlave);		/* スキップあり/なしで処理が異なる */
			if (!bSkip) {
				/* 何処にあったか？で付け替えが発生する */
				switch (where) {
					case 0:		/* 何処にもない（ポーリングのための「状態要求データ」のケース）*/
						break;
					case 1:		/* 優先キューにある（セレクティング最初のケース）*/
						pWhat = DeQueue(&toSque_Ctrl_Select, 0);				/* (優先)キューを外す。*/
						IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)pWhat->pData;	/* 付け替え */
						pWhat->pData->sCommon.bUsedByQue = 1;					/* 順番ボードに付け替えなので「使用」にする */
						IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 2;	/* 場所は順番ボードへ */
						break;
					case 2:		/* 順番ボードにある（セレクティング再送のケース。無応答のため）*/
						break;
				}
			}
			/* スキップになった */
			else {
				/* 何処にあったか？で後始末が異なる */
				switch (where) {
					case 0:		/* 何処にもない（ポーリングのための「状態要求データ」のケース）*/
						break;								/* 何もしない */
					case 1:		/* 優先キューにある（セレクティング最初のケース）*/
						DeQueue(&toSque_Ctrl_Select, 0);	/* (優先)キューを外す。*/
						break;
					case 2:		/* 順番ボードにある（セレクティング再送のケース。無応答のため）*/
						pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
						if (!pMail) {
							break;
						}
						pMail->sCommon.bUsedByQue = 0;	/* 順番ボードにあったものは「未使用」にする */
						break;
					default:
						break;
				}
				IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)0;	/* 順番ボードには「なし」にする */
				IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 0;		/* 直前送信コマンドは「何処にもない」にする */
			}

			break;

		case 4:
			/* 何処にあったか？で後始末が異なる */
			switch (where) {
				case 0:		/* 何処にもない（ポーリングのための「状態要求データ」のケース）*/
					break;								/* 何もしない */
				case 1:		/* 優先キューにある（セレクティング最初のケース）*/
					DeQueue(&toSque_Ctrl_Select, 0);	/* (優先)キューを外す。*/
					break;
				case 2:		/* 順番ボードにある（セレクティング再送のケース。無応答のため）*/
					pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
					if (!pMail) {
						break;
					}
					pMail->sCommon.bUsedByQue = 0;		/* 順番ボードにあったものは「未使用」にする */
					break;
				default:
					break;
			}
			IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)0;	/* 順番ボードには「なし」にする */
			IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 0;		/* 直前送信コマンドは「何処にもない」にする */

			toScom_ResetRetry(toScom_toSlave.ucNow);	/* 再送カウンタ初期化 */
			toScom_ResetSkip(toScom_toSlave.ucNow);		/* スキップカウンタ初期化 */

			break;

		default:
			break;
	}

	return bRet;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SetNextPolling()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		次のポーリング時刻を決める										   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_SetNextPolling(int iPolling)
{
	int		ni;
	int		alive;
	ushort	usPolling;

	/* ポーリングできる人数を求める */
	alive = 0;
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		if (IFM_LockTable.sSlave[ni].bComm) {
			alive++;
		}
	}

	/* ひとりっきりにポーリングする間隔 */
	if (alive == 1) {
// ポーリング間隔は設定に従うのが基本
		usPolling = IFM_LockTable.toSlave.usPolling;	// 初期設定データに従う
	}
	/* 誰にもポーリングできない（誰かが生き返るまで＝スキップ解除するまで）ときの間隔 */
	else if (alive == 0) {
		usPolling = IFM_LockTable.toSlave.usPolling;	// 初期設定データに従う
	}
	/* ２人以上生きているときにポーリングする間隔 */
	else {
		if (iPolling == 0 ||								/* 起動時、または */
			iPolling == 1)									/* 正規応答時は、*/
		{													/* ポーリングするので、*/
			usPolling = IFM_LockTable.toSlave.usPolling;	/* 初期設定データに従う */
		}													/* それ以外は、*/
		else {												/* すぐ次のポーリングしたいので、*/
			usPolling = 0;									/* 即出し */
		}
	}

	/* ポーリング用タイマ再開 */
	toScom_LineTimerStart(usPolling);
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_GetSlaveNow()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		今の通信相手を取得する											   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
int		toScom_GetSlaveNow(void)
{
	return (int)toScom_toSlave.ucNow;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendStatusToFlap()                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		ロック管理タスクへの状態変化通知								   |*/
/*|		・ロック管理テーブル上のセンサー状態と変化があるときのみ通知する。 |*/
/*|		・通知は、それまでのメールを改め、キュー方式にする。			   |*/
/*|		・メール不採用の理由は以下。									   |*/
/*|			1)ロック管理タスクは、										   |*/
/*|			  対精算機通信タスクからの「ロック制御データ」を			   |*/
/*|			  本タスクへメール転送するため、							   |*/
/*|			  同時期に本タスクが「子機状態データ」をメールすると、		   |*/
/*|			  そのハンドリング手順にてデッドロックを引き起こす可能性あり。 |*/
/*|			2)ロック管理タスクは、本タスクからの「子機状態データ」と	   |*/
/*|			  対精算機通信タスクからの「ロック制御データ」を			   |*/
/*|			  同時期に受け取った場合、「子機状態データ」を優先処理すべき。 |*/
/*|			  メール通知にした場合ロック管理タスクは、その判定が面倒。	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-04-18	早瀬・変更。上記変更。                     |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendStatusToFlap(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* キュー実体 */
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	BOOL			bQfull;		/* 2005.07.12 早瀬・追加 */

	/*
	*	ここに来たら「変化あり」ということ
	*/
	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* キュー実体をリンク */
		bQfull = 0;									/* キューフル（子機状態リスト）[解除]かも */
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		bQfull = 1;									/* キューフル（子機状態リスト）[発生] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}

	/*「子機状態データ」キュー実体を作る */
	pQbody->sFlapA41.sCommon.ucKind     = 'A';
	pQbody->sFlapA41.sCommon.ucTerminal = ucTerm;
	pQbody->sFlapA41.t_FlapSensor.c_LoopSensor		= pAns->unBody.sFlapA41.t_FlapSensor.c_LoopSensor;
	pQbody->sFlapA41.t_FlapSensor.c_ElectroSensor   = pAns->unBody.sFlapA41.t_FlapSensor.c_ElectroSensor;
	pQbody->sFlapA41.t_FlapSensor.c_LockPlate		= pAns->unBody.sFlapA41.t_FlapSensor.c_LockPlate;
	pQbody->sFlapA41.t_FlapSensor.c_FlapSensor		= pAns->unBody.sFlapA41.t_FlapSensor.c_FlapSensor;
	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	/*
	*	2005.07.12 早瀬・修正
	*	キューフル発生/解除を判定した、そのときに「IFM_MasterError(IFM.c)」を呼び出してはならない。
	*	※詳しくは「PAYcom_NoticeError(PAYnt_net.c)」を参照のこと
	*/
	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* キューフル（子機状態リスト）[発生] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* キューフル（子機状態リスト）[解除]かも */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendLoopData()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|		ループデータ応答の受信を通知する								   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	toSmail_SendLoopData(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* キュー実体 */
struct t_IF_Queue	*pQbuff;	/* キュー本体 */
	BOOL			bQfull;		/* 2005.07.12 早瀬・追加 */

	/* 空きキュー在るか？*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* 空き実体を取得 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* 空き本体を取得 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* キュー実体をリンク */
		bQfull = 0;									/* キューフル（子機状態リスト）[解除]かも */
	}
	/* 空きキュー無しなら、先頭(最古)を外して使う（古いものは捨てる）*/
	else {
		bQfull = 1;									/* キューフル（子機状態リスト）[発生] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* キュー(先頭)を外す */
		pQbody = pQbuff->pData;
	}

	/*「子機状態データ」キュー実体を作る */
	pQbody->sFlapS53.sCommon.ucKind     = 'S';
	pQbody->sFlapS53.sCommon.ucTerminal = ucTerm;
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucLoopCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucLoopCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucLoopCount));
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucOffBaseCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucOffBaseCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucOffBaseCount));
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucOnLevelCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucOnLevelCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucOnLevelCount));
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucOffLevelCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucOffLevelCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucOffLevelCount));
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucOnBaseCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucOnBaseCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucOnBaseCount));
	/* 末尾へキューイング */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* キューフル（子機状態リスト）[発生] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* キューフル（子機状態リスト）[解除]かも */
	}
}

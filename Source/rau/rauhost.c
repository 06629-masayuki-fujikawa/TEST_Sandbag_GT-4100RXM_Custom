/************************************************************************************/
/*																					*/
/*		システム名　:　RAUシステム													*/
/*		ファイル名	:  rauhost.c													*/
/*		機能		:  Host側通信、状態遷移処理										*/
/*																					*/
/************************************************************************************/

#define	GLOBAL extern
#include	<string.h>
#include	"system.h"
#include	"ope_def.h"
#include	"prm_tbl.h"

#include	"rau.h"
#include	"rauIDproc.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"iodefine.h"
#include	<stdlib.h>

#include	"trsocket.h"

#include	"ksgRauModem.h"

#define	RAU_RCV_SEQ_TIME_UP			300		// 受信シーケンスキャンセルカウント（300秒）
#define	RAU_SND_SEQ_TIME_UP			300		// 送信シーケンスキャンセルカウント（300秒）
#define	RAU_ERR_RCV_SEQ_TIM_OVER	0x0A	// エラーコード１０(受信シーケンスフラグレスポンスタイムアウト)

void	RAU_Mh_sdl(void);						// ホスト通信側の状態遷移処理
void	DPA2Port_RestoreOfSendData(int port);	// パケットの再送信
void	DPA2Port_SendNextDataToHost(int port);	// 次のパケットの送信準備
void	DPA2Port_AckWait_TimeOut(int port);		// ＡＣＫ待ちタイマのタイムアウト処理
void	DPA2Port_AckWait_Disconnected(int port);		// ＡＣＫ待ち中のTCP切断時処理
void	RAU_Mh_sum_check(void);						// ホスト通信側のチェックサムチェック処理
void	DPA2Port_BeginToSendThroughData(void);	// 下り回線への送信データ準備
void	DPA2Port_BeginToSendTableData(void);	// 上り回線への送信データ準備
int		DPA2Port_RecvDataCheck(int port);		// 受信データの種別チェック
uchar	RAU_dat_check(int port);				// ホスト通信側の受信データ判別処理
void	RAU_DpaSndHedSet(uchar ucLastPacket, int port);	// ホスト通信側のＤｏＰａヘッダー部登録処理
void	DPA2Port_DpaSeqIncrement(int port);		// データ送信用シーケンス番号のインクリメント
void	RAUhost_Init(void);
void	RAUhost_SetSendRequeat(RAU_SEND_REQUEST *send_request);
void	RAUhost_ClearSendRequeat(void);
void	RAUhost_CreateResponseData(uchar *pBuff, uchar ucResult, ushort uiExtra, ulong uiSeqNum, ushort *uiLen);
void	RAUhost_SetError(uchar ucErrCode);
void	DPA2Port_CreateDopaPacket(int port);
BOOL	RAUhost_GetNtData(void);
BOOL	DPA2Port_GetNtData(T_SEND_DATA_ID_INFO *pSendDataInfo, T_SEND_NT_DATA *pSendNtData);
void	RAUhost_CreateDopaPacketTsum(void);
BOOL	RAUhost_GetNtDataTsum(void);
short	RAUhost_CreateDopaPacketCash(void);
uchar	RAUhost_GetSndSeqFlag(void);
uchar	RAUhost_GetRcvSeqFlag(void);
void	RAUhost_SetSndSeqFlag(uchar ucSndSeqFlag);
void	RAUhost_SetRcvSeqFlag(uchar ucRcvSeqFlag);
BOOL	RAUhost_isDataReadEnd(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
short	RAUhost_IdentifyDataType(int port);
void	RAUhost_DwonLine_common_timeout(void);
short	RAUhost_CreateDopaPacketCTI(void);
BOOL	RAUhost_GetNtDataCenterTermInfo(void);
extern	BOOL RAUdata_GetCenterTermInfoData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff);

extern	void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );
extern	void	RAUarc_ErrEnque( uchar ModuleCode, uchar ErrCode, uchar f_Occur );
extern	BOOL	RAUdata_GetNtData(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
extern	void	RAUdata_Create0Data(RAU_NTDATA_HEADER *pNtDataHeader, uchar ucDataId);
extern	void	RAUdata_ReadNtData(uchar *pData, ushort wReadLen, RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
extern	BOOL	RAUdata_GetTsumData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff);
extern	BOOL	RAUdata_GetCashData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff);
extern	void	RAUdata_TableClear(uchar ucTableKind);
extern	void	RAUque_UpDate(T_SEND_DATA_ID_INFO *pSendIdInfo);
extern	T_SEND_DATA_ID_INFO*	RAUque_GetSendDataInfo( void );
extern	void	RAUque_DeleteQue(void);
extern	BOOL	RAUque_IsLastID( void );
extern	eRAU_TABLEDATA_SET	RAUid_SetRcvNtData(uchar *pData, ushort uiDataLen, ushort uiTcpBlkNo, uchar ucTcpBlkSts);
extern	void	RAUdata_GetNtDataInfo(T_SEND_DATA_ID_INFO *pSendDataInfo, RAU_SEND_REQUEST *pReqInfo, uchar ucSendReq);
extern	void	RAUdata_GetNtDataInfoThrough(T_SEND_DATA_ID_INFO *pSendDataInfo);
extern	int		count_tsum_data(struct _ntman *ntman, uchar count_tsum);
extern	int		check_cash_que(char *que);
extern  void    data_rau_init(void);
extern	void	RAU_IniDatUpdate( void );
extern	void	RAU_Word2Byte(unsigned char *data, ushort uShort);
extern	void RAUid_CommuTest_Error(void);
extern	void	RAUid_CommuTest_Result(void);

extern	void	RAUdata_DeleteTsumGroup(ushort ucSendCount);
extern	ulong	RAU_c_1mPastTimGet( ulong StartTime );
extern	void	Credit_SetError(uchar ucErrCode);

const uchar uc_dpa_rem_str[13]={'A','M','A','N','O','_','R','E','M','O','T','E',0};
/*------------------------------------------------------------------------------*/
#pragma	section	_UNINIT3		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:アドレス空間0x06039000-0x060FFFFF(1MB)

tRauCtrl RauCtrl;
tRauConfig	RauConfig;

tCreditCtrl CreditCtrl;
uchar	RAU_LogData[RAU_LOG_BUFF_SIZE];
uchar	RAU_NtLogData[RAU_LOG_BUFF_SIZE];
uchar	RAU_NtBlockLogData[RAU_LOG_BUFF_SIZE];
/*------------------------------------------------------------------------------*/
#pragma	section	
/*------------------------------------------------------------------------------*/
ushort	RAU_NtLogDataIndex = 0;
ushort	RAU_NtLogDataSize = 0;

ushort	RAUhost_ConvertNtBlockData(const uchar* pData, ushort size, uchar* pNtBlockData, uchar *blockCount);
short	RAUhost_CreateTableDataPacket(void);
void	RAUhost_CreateTsumData(void);
void	RAUhost_CreateTsumData_r10(ushort type);
short	RAUhost_ConvertDataIdToLogData(uchar dataId);
int		RAUhost_Receive(int port);

extern	int		KSG_gPpp_RauStarted;				// PPP接続フラグ

//==============================================================================
//	ホストとのデータ送受信
//
//	@argument	なし
//
//	@return		なし
//
//	@attention	ホストからの受信データをキューに詰めるのは
//				ＤＰＡモジュールの担当です。
//
//	@note		ＲＡＵモジュールに処理が移ったとき定期的にコールされる
//				ルーチンです。キューに溜まった受信データの取り出しと
//				送信キューへのデータ書込みを担当します。
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void RAU_Mh_sdl(void)
{
	if( RAU_f_RcvSeqCnt_rq == 1 ){								// ホストがＩＢＷからのデータを要求しているとき(タイマの作動条件)
		if(	RAU_Tm_RcvSeqCnt >= RAU_RCV_SEQ_TIME_UP ){			// 要求データの受信から５０秒経ってもＩＢＷからのデータが来ないと
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);		// 応答なしと判断して受信シーケンスを０に戻す。
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 			RAU_f_RcvSeqCnt_rq = 0;								// 受信シーケンスキャンセルタイマ動作許可(解除)
// 			RAU_Tm_RcvSeqCnt = 0;								// 受信シーケンスキャンセルタイマ(停止)
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			RAUhost_SetError(ERR_RAU_RCV_SEQ_TIM_OVER);			// エラーコード１０(受信シーケンスフラグレスポンスタイムアウト)
			if( RemoteCheck_Buffer116_117.CheckNo != 0 ){		// 通信チェック要求データをＨＯＳＴから受け付けている。
				if( RemoteCheck_Buffer116_117.Re.sult.Code < 7 ){	// 「応答待ち」より前の段階(「他データ受信」してないとき)
					RemoteCheck_Buffer116_117.Re.sult.Code = 8;		// 結果コード(無応答)とする。
				}
				RAU_f_CommuTest_ov = 1;							// 通信チェックタイマ強制タイムアウト
			}
		}
	}
	if(RAU_f_SndSeqCnt_rq == 1){
		if(RAU_Tm_SndSeqCnt >= RAU_SND_SEQ_TIME_UP){
			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);
		}
	}

	if( RauCT_SndReqID == 100 ){								// 通信チェックデータ(ＩＤ１００)送信要求時
		if(KSG_RauGetMdmCutState() != 0 ){						// モデム切断ステータスが"切断要求"や"切断中"になった。
			RemoteCheck_Buffer100_101.Re.sult.Code = 5;			// 結果コード(サーバーダウン)とする。
			RAU_f_CommuTest_ov = 1;								// 通信チェックタイマ強制タイムアウト
		} else {
			if( RemoteCheck_Buffer100_101.CheckNo == 0 ){		// 通信チェック中断処理開始(中断要求電文の受信によりバッファが０クリアされるため)
				RauCT_SndReqID = 0;								// 通信チェックデータ(ＩＤ１００)送信要求クリア
				RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// 送信シーケンスを０(ＨＯＳＴへのテーブル送信待機状態)に戻す。
//				if( DPA_TCPcnct_req == 1 ){						// 上り回線のコネクション要求中だった。
				if( RauCtrl.TCPcnct_req == 1 ){					// 上り回線のコネクション要求中だった。
					RauCtrl.TCPcnct_req = 0;					// 接続をやめさせる。
				}
				if( RauCtrl.tcpConnectFlag.port.upload ) {		// 上り回線ＴＣＰ接続中だった。
					RAU_uc_retrycnt_h = 0;						// 上り回線の送信リトライカウンタクリア
					RAU_Tm_Ackwait.tm = 0;						// 上り回線のＡＣＫ受信待ちタイマクリア
					RAU_ui_data_length_h = 0;					// 受信データ長クリア
					RAU_uc_mode_h = S_H_IDLE;					// ＡＣＫ待ちを解除してアイドルに移行
					RAU_Tm_TCP_DISCNCT.tm = 0;
				}
			}
		}
	}

	if( RAU_f_CommuTest_ov == 1 ){	// 通信チェックタイマタイムアウト(ＩＢＫがＨＯＳＴまたはＩＢＷに対して通信チェック結果データを送信)
		RAU_Tm_CommuTest.tm = 0;	// 通信チェックタイマタイマ(停止)
		RAUid_CommuTest_Error();
	}

	if (RAUhost_Receive(UPLINE) != 0) {				// 受信キューからデータ読み込み(上り回線からのデータ)
		switch( DPA2Port_RecvDataCheck(UPLINE) ){	// 受信データ(RAU_huc_rcv_work_h)の種別判定
		case 1:						// ＡＣＫ受信
			DPA2Port_SendNextDataToHost(UPLINE);	// 次の送信データを送信キューに詰める。
			break;
		case 2:						// ＮＡＫ受信
			DPA2Port_AckWait_TimeOut(UPLINE);		// データの再送信処理(規定回数オーバーでモデム切断要求)
			break;
		default:					// 破棄データ・その他データ
			break;					// 上り回線からＡＣＫ・ＮＡＫ以外の受信は本来あり得ないので読み捨て。
		}
	} else {
		if( RAU_uc_mode_h == S_H_IDLE ){					// 上り回線がＡＣＫ待ちでないとき
			DPA2Port_BeginToSendTableData();			// 上り回線に対して送信しなければいけないテーブルデータがあれば送信準備をする。
		}
		if( RAU_GetUpNetState() == RAU_NET_PPP_OPENED ) {	// TCP切断
			if( RAU_uc_mode_h == S_H_ACK ){			// ＡＣＫ受信待ち
				if( RAU_f_Ackwait_reconnect_rq == 0 && RAU_f_Ackwait_reconnect_ov == 0 ){			// 再接続タイマ起動していない
					DPA2Port_AckWait_Disconnected(UPLINE);	// 上り回線用TCP切断時処理
				}
			}
		}
		if( RAU_f_Ackwait_reconnect_ov == 1 ){					// 再接続タイマタイムアウト
			RAU_Tm_Ackwait_reconnect.tm = 0;				// 再接続タイマキャンセル
			DPA2Port_RestoreOfSendData(UPLINE);			// パケットの再送信
			RauCtrl.TCPcnct_req = 1;				// TCP接続要求
		}
		if( RAU_f_Ackwait_ov == 1 ){						// 上り回線用ＡＣＫ待ちタイマタイムアウト
			RAU_Tm_Ackwait.tm = 0;							// 上り回線用ＡＣＫ受信待ちタイマキャンセル
			if( RAU_uc_mode_h == S_H_ACK ){					// ＡＣＫ受信待ち
				if( KSG_RauGetMdmCutState() == 0 ){			// モデム切断ステータスが"解除"状態のとき
					RAU_uc_retrycnt_h++;					// リトライカウント
					DPA2Port_AckWait_TimeOut(UPLINE);	// 上り回線用タイムアウト処理
				}
			}
		}
	}

	if (RAUhost_Receive(DOWNLINE) != 0) {			// 受信キューにデータあり(下り回線からのデータ)
		switch( DPA2Port_RecvDataCheck(DOWNLINE) ){	// 受信データ(RAU_huc_rcv_work_h)の種別判定
		case 1:						// ＡＣＫ受信
			DPA2Port_SendNextDataToHost(DOWNLINE);	// 次の送信データを送信キューに詰める。
			break;
		case 2:						// ＮＡＫ受信
			DPA2Port_AckWait_TimeOut(DOWNLINE);		// データの再送信処理(規定回数オーバーでスルーデータ削除)
			break;
		case 3:						// その他データ
			RAU_Mh_sum_check();			// 受信データのテーブルへの保存＆ＡＣＫまたはＮＡＫを下り回線の送信キューに詰める。
			break;
		default:					// 破棄データ
			break;					// 期待していたＡＣＫ・ＮＡＫでないため読み捨て。
		}
		if( RemoteCheck_Buffer100_101.CheckNo != 0 ){	// 通信チェック要求データをＩＢＫから受け付けている。
			RemoteCheck_Buffer100_101.Re.sult.Code = 7;	// 結果コードを(他データ受信)とする。
		}
	} else {
		if( RAU_uc_mode_h2 == S_H_IDLE ){				// 下り回線がＡＣＫ待ちでないとき
			DPA2Port_BeginToSendThroughData();		// 下り回線に対して送信しなければいけないスルーデータがあれば送信準備をする。
		}
		if( RAU_GetDownNetState() == RAU_NET_PPP_OPENED ) {	// TCP切断
			if( RAU_uc_mode_h2 == S_H_ACK ){			// ＡＣＫ受信待ち
				DPA2Port_AckWait_Disconnected(DOWNLINE);	// 下り回線用TCP切断時処理
			}
		}
		if( RAU_f_Ackwait2_ov == 1 ){					// ＡＣＫ待ちタイマタイムアウト
			RAU_Tm_Ackwait2.tm = 0;						// ＡＣＫ受信待ちタイマキャンセル
			if( RAU_uc_mode_h2 == S_H_ACK ){			// ＡＣＫ受信待ち
				RAU_uc_retrycnt_h2++;					// リトライカウント
				DPA2Port_AckWait_TimeOut(DOWNLINE);	// 下り回線用タイムアウト処理
			}
		}
	}
}

//==============================================================================
//	０カット後のデータ長算出
//
//	@argument	*pData		データ列
//	@argument	length		データ列の長さ
//
//	@return		０カット後のデータ列の長さ
//
//	@note		データ列の右から０が連続している数を元のデータ列長から差し引く
//
//	@author		2007.02.26:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
ushort GetZeroCut_len(uchar *pData, ushort length)
{
	if( length != 0 ){
		while( length ){
			if( pData[length-1] != 0x00 ){
				break;
			}
			length--;
		}
	}

	return length;
}

//==============================================================================
//	通信チェック結果データの生成
//
//	@argument	*pData		生成したデータを格納するバッファ
//	@argument	id			作成するデータのＩＤ
//
//	@return		データ列の長さ
//
//	@note		ＩＢＫが保持している通信チェック結果データをリモートパケット化する。
//
//	@author		2007.02.26:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
ushort RAUhost_CreateDopaPacketTest(uchar *pData, uchar id)
{
	ushort	len;

	switch( id ){
	case 100:
		len = GetZeroCut_len((uchar *)&RemoteCheck_Buffer100_101.data_len, 36);		// ＮＴ－ＮＥＴデータ部０カット
		RAU_Word2Byte((unsigned char *)&RemoteCheck_Buffer100_101.data_len, len);	// 共通部＋固有部の(０カット後)データ長
		memcpy(&pData[DOPA_HEAD_SIZE], (char *)&RemoteCheck_Buffer100_101.data_len, len );
		break;
	case 117:
		len = GetZeroCut_len((uchar *)&RemoteCheck_Buffer116_117.data_len, 35);		// ＮＴ－ＮＥＴデータ部０カット
		RAU_Word2Byte((unsigned char *)&RemoteCheck_Buffer116_117.data_len, len);	// 共通部＋固有部の(０カット後)データ長
		memcpy(&pData[DOPA_HEAD_SIZE], (char *)&RemoteCheck_Buffer116_117.data_len, len );
		break;
	default:
		return( 0 );
	}
	len = len + DOPA_HEAD_SIZE + 2;									// パケット長(ヘッダ長＋データ長＋ＣＲＣ)
	c_int32toarray(pData, (ulong)len, 4);							// パケット長セット
	memcpy( &pData[4], uc_dpa_rem_str, 12 );						// "AMANO_REMOTE"
	c_int32toarray(&pData[16], RAU_ui_RAUDPA_seq, 3);					// シーケンスNo.
	pData[19] = 0;													// データ部コード(未使用０固定)
	pData[20] = 0;													// 電文識別コード(0:中断要求なし, 1:中断要求あり)
	pData[21] = 0;													// ＴＣＰブロックナンバー
	pData[22] = 1;													// ＴＣＰブロックナンバー
	pData[23] = 1;													// ＴＣＰブロックステータス
	pData[24] = 0;													// 予備
	pData[25] = 0;													// 予備
	RAU_CrcCcitt( len - 2, 0, pData, NULL, &pData[len - 2], 1 );	// ＣＲＣ算出

	return( len );
}

//==============================================================================
//	パケットの再送信
//
//	@argument	port	対象ポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		なし
//
//	@attention	ＤｏＰａヘッダは送信失敗時と同じものを使用します。
//
//	@note		ＨＯＳＴからＮＡＫを受信またはＡＣＫが来ないときにコールされ
//				送信に失敗したパケットをテーブル復元して送信キューに入れ直します。
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_RestoreOfSendData(int port)
{
	if( port == UPLINE ){												// 上り回線
			DpaSndQue_Set( RAU_huc_txdata_h, RAU_ui_txdlength_h, UPLINE );		// 前回送信したＤｏＰａパケットを上り回線用の送信キューにデータをセット
			RAU_Tm_TCP_DISCNCT.tm = 0;									// ＴＣＰ切断待ちタイマ停止
			RAU_Tm_Ackwait.bits0.bit_0_13 = RauConfig.Dpa_com_wait_tm;	// ＡＣＫ待ちタイマ起動
			RAU_f_Ackwait_rq = 1;
			RAU_f_Ackwait_ov = 0;
	} else {															// 下り回線
			DpaSndQue_Set( RAU_huc_txdata_h2, RAU_ui_txdlength_h2, DOWNLINE );	// 前回送信したＤｏＰａパケット下り回線用の送信キューにデータをセット
			RAU_Tm_Port_Watchdog.tm = 0;								// 下り回線通信監視タイマ(停止)
			RAU_Tm_Ackwait2.bits0.bit_0_13 = RauConfig.Dpa_ack_wait_tm;	// ＡＣＫ待ちタイマ起動
			RAU_f_Ackwait2_rq = 1;
			RAU_f_Ackwait2_ov = 0;
	}
}

//==============================================================================
//	受信データの種別チェック
//
//	@argument	port	データを受信したポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		0		破棄データ
//	@return		1		ＡＣＫ受信
//	@return		2		ＮＡＫ受信
//	@return		3		データ受信
//
//	@attention	ＮＡＫ９９(再送信の停止要求)を受信したときはＡＣＫ扱いとする。
//
//	@note		ホストから受信したデータを解析し、その種別を返します。
//				シーケンス番号が期待値と異なるときは破棄データとなります。
//
//======================================== Copyright(C) 2006 AMANO Corp. =======
int DPA2Port_RecvDataCheck(int port)
{
	int		ResData;
	ulong	SendDataSeqNo, RecvDataSeqNo;
	uchar	ucCrc[2];

	ResData = RAU_dat_check(port);	// 受信データ(RAU_huc_rcv_work_h)の種別判定
	switch( ResData ){
	case 1:						// ＡＣＫ受信
	case 2:						// ＮＡＫ受信
		if( (port == UPLINE)&&(RauCtrl.DPA_TCP_DummySend != 0) ){		// 上り回線で"AMANO__DUMMY"を送信した場合の応答
			if( RauConfig.tcp_disconect_flag == 0 ) {					// 切断あり
				RAU_Tm_TCP_DISCNCT.tm = 0;
				RAU_f_TCPdiscnct_rq = 0;
				RAU_f_TCPdiscnct_ov = 1;	// 上り回線のＴＣＰ接続を切断
			}
			RauCtrl.DPA_TCP_DummySend = 0;
			if(RauCT_SndReqID == 100) {									// 通信テストによるダミーデータ送信?
				RAUid_CommuTest_Result();
				break;
			}
			ResData = 0;				// 破棄データとする。
			break;
		}

		if( (port == UPLINE)&&(RAU_uc_mode_h == S_H_IDLE) ){	// 上り回線からのデータ受信でＡＣＫ待ちでないとき
			ResData = 0;		// 破棄データとする。
			break;
		}
		if( (port == DOWNLINE)&&(RAU_uc_mode_h2 == S_H_IDLE) ){	// 下り回線からのデータ受信でＡＣＫ待ちでないとき
			ResData = 0;		// 破棄データとする。
			break;
		}


		RecvDataSeqNo = c_arraytoint32( &RAU_huc_rcv_work_h[16], 3 );		// 受信データ(ＡＣＫ・ＮＡＫ)のシーケンスNo.を取得
		if( port == UPLINE ){	// 上り回線
			SendDataSeqNo = c_arraytoint32( &RAU_uc_txdata_h_keep[16], 3 );	// 送信データのシーケンスNo.を取得
		} else {				// 下り回線
			SendDataSeqNo = c_arraytoint32( &RAU_uc_txdata_h_keep2[16], 3 );// 送信データのシーケンスNo.を取得
		}

		if( RecvDataSeqNo != SendDataSeqNo ){		// 送信データに対するＡＣＫ・ＮＡＫでない
			ResData = 0;							// 破棄データとする。
			break;									// 無応答タイマがタイムアウトするか正常な応答が来るまで待つ
		}
		RAU_CrcCcitt(RAU_ui_data_length_h-2, 0, RAU_huc_rcv_work_h, NULL, ucCrc, 1);	// ＣＲＣ算出

		if( (RAU_huc_rcv_work_h[RAU_ui_data_length_h-2] != ucCrc[0])
			||( RAU_huc_rcv_work_h[RAU_ui_data_length_h-1] != ucCrc[1]) ){	// ＣＲＣが異なる。
			ResData = 0;							// 破棄データとする。
			if( port == UPLINE ){	// 上り回線
				RAUhost_SetError(ERR_RAU_DPA_RECV_CRC);		// エラーコード５４(上り回線受信データＣＲＣエラー)
			} else {				// 下り回線
				RAUhost_SetError(ERR_RAU_DPA_RECV_CRC_D);	// エラーコード８２(下り回線受信データＣＲＣエラー)
			}
			break;									// 無応答タイマがタイムアウトするか正常な応答が来るまで待つ
		}
		break;
	default:	// その他のデータ
		ResData = 0;			// 破棄データとする。
		break;
	case 0:		// データ受信
		ResData = 3;
		break;
	}

	return( ResData );
}

//==============================================================================
//	次のパケットの送信準備
//
//	@argument	port			対象ポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		なし
//
//	@attention	ＤｏＰａの２ポート対応専用ルーチンです。
//
//	@note		１パケットのデータ送信に対してホストからのＡＣＫを受信したとき
//				本関数が呼び出されます。データの終端でない場合に
//				次のパケットを作成する準備をします。
//
//	@see		Mh_next_send()
//
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_SendNextDataToHost(int port)
{
	short	logId;
	ushort	overwriteCount;				// 送信中に上書きされた未送信データ数
	uchar	endDataId;
	short	totalLogId;

	if( (pRAUhost_SendIdInfo->send_req == 2) &&
		(pRAUhost_SendIdInfo->send_data_count == 0) ) {
		// バッチ送信時の0件データは送信していないので送信シーケンスのインクリメントはしない
	}
	else {
		DPA2Port_DpaSeqIncrement( port );	// 送信シーケンスNo.インクリメント
	}

	if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		endDataId = 53;
	}
	else {
		endDataId = 41;
	}

	if( port == UPLINE ){	// 上り回線
		if( RauCT_SndReqID != 0 ){									// 通信チェックデータを送信済み
			if( RauConfig.tcp_disconect_flag == 0 ) {					// 切断あり
				RAU_Tm_TCP_DISCNCT.tm = 0;
				RAU_f_TCPdiscnct_rq = 0;
				RAU_f_TCPdiscnct_ov = 1;	// 上り回線のＴＣＰ接続を切断
			}
			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// 送信を終了したので送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)に戻す。
			RauCT_SndReqID = 0;
			RAU_uc_retrycnt_h = 0;										// リトライカウンタをイニシャライズ（０クリア）
			RAU_Tm_Ackwait.tm = 0;										// 上り回線用ＡＣＫ受信待ちタイマクリア
			RAU_uc_retrycnt_reconnect_h = 0;							// TCP切断時リトライカウンタをイニシャライズ（０クリア）
			RAU_Tm_Ackwait_reconnect.tm = 0;							// 上り回線用再接続タイマクリア
			RAU_ui_data_length_h = 0;
			RAU_uc_mode_h = S_H_IDLE;									// ＡＣＫ待ちを解除
			return;
		}

		RAUque_UpDate(pRAUhost_SendIdInfo);	// 参照しているテーブルの送信状況を更新

		// Ｔ合計残りログ分送信処理
		if(pRAUhost_SendIdInfo->pbuff_info->uc_DataId == endDataId){
			if(pRAUhost_SendIdInfo->pbuff_info->ui_syuType == 1) {
				totalLogId = eLOG_TTOTAL;
			}
			else {
				totalLogId = eLOG_GTTOTAL;
			}
			if( (pRAUhost_SendIdInfo->send_data_count <= pRAUhost_SendIdInfo->send_complete_count)	// 現在参照しているテーブルのデータは全部送った。
				|| (pRAUhost_SendIdInfo->send_data_count == 0) ){	// ０件データの場合は、次のテーブルも送信予定になってないか見る。
				// 送信完了分のLOGリードポインタ更新
				Ope_Log_TargetVoidReadPointerUpdate(totalLogId, eLOG_TARGET_REMOTE);
				if(pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus == RAU_NEARFULL_NEARFULL){
					pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus = RAU_NEARFULL_CANCEL;
					pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatusBefore = RAU_NEARFULL_CANCEL;
				}
				// Ｔ合計データの場合は残りログが存在すれば続けて送信
				if(Ope_Log_UnreadCountGet(totalLogId, eLOG_TARGET_REMOTE) > 0){
					if( (RauConfig.id30_41_mask_flg == 0) &&				// Ｔ合計集計データの送信マスクなし
						(RauConfig.serverTypeSendTsum != 0) ){				// 34-0026④ 0:送信しない 1:送信する
						RAUdata_DeleteTsumGroup(pRAUhost_SendIdInfo->send_complete_count);
						pRAUhost_SendIdInfo->send_complete_count = 0;
						pRAUhost_SendIdInfo->fill_up_data_count = 0;
						pRAUhost_SendIdInfo->crc_err_data_count = 0;
						if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
							if(totalLogId == eLOG_TTOTAL) {
								pRAUhost_SendIdInfo->pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
								pRAUhost_SendIdInfo->pbuff_info->ui_syuType = 1;	// T合計
							}
							else {
								pRAUhost_SendIdInfo->pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
								pRAUhost_SendIdInfo->pbuff_info->ui_syuType = 11;	// GT合計
							}
							RAUhost_CreateTsumData_r10(pRAUhost_SendIdInfo->pbuff_info->ui_syuType);
						}
						else{
// MH322914 (s) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
							pRAUhost_SendIdInfo->pbuff_info->ui_syuType = 1;	// T合計
// MH322914 (e) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
							RAUhost_CreateTsumData();
						}
						pRAUhost_SendIdInfo->send_data_count = count_tsum_data(&RAU_tsum_man.man, 0);
						pRAUhost_SendIdInfo->send_complete_count = 0;

						RAU_uc_retrycnt_h = 0;		// リトライカウンタをイニシャライズ（０クリア）
						RAU_Tm_Ackwait.tm = 0;		// 上り回線用ＡＣＫ受信待ちタイマクリア
						RAU_uc_retrycnt_reconnect_h = 0;		// リトライカウンタをイニシャライズ（０クリア）
						RAU_Tm_Ackwait_reconnect.tm = 0;		// 上り回線用再接続待ちタイマクリア
						RAU_ui_data_length_h = 0;
						RAU_uc_mode_h = S_H_IDLE;	// ＡＣＫ待ちを解除

						RAU_uiTcpBlockNo = 1;	// 次のテーブルデータを送るときはＴＣＰブロック№１から開始
						RAU_ucTcpBlockSts = 0;	// 次のパケットが最終になる場合は後で１になる。
						return;
					}
				}
			}
		}

		if( (RAUque_IsLastID() == FALSE)||(RAU_ucTcpBlockSts == 0) ){	// 送信完了したのは途中のパケット
			if(pRAUhost_SendIdInfo->pbuff_info->uc_DataId != endDataId){
				// 送信完了分のLOGリードポインタ更新
				logId = RAUhost_ConvertDataIdToLogData(pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
				if(logId != -1){
					overwriteCount = Ope_Log_TargetVoidReadPointerUpdate(logId, eLOG_TARGET_REMOTE);
					if(overwriteCount) {
						// 送信中に未送信データが上書きされていれば送信済みデータ数に加算する
						pRAUhost_SendIdInfo->send_complete_count += overwriteCount;
					}
					if(pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus == RAU_NEARFULL_NEARFULL){
						pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus = RAU_NEARFULL_CANCEL;
						pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatusBefore = RAU_NEARFULL_CANCEL;
					}
				}
			}
			if( (pRAUhost_SendIdInfo->send_data_count <= pRAUhost_SendIdInfo->send_complete_count)	// 現在参照しているテーブルのデータは全部送った。
				|| (pRAUhost_SendIdInfo->send_data_count == 0) ){	// ０件データの場合は、次のテーブルも送信予定になってないか見る。
				pRAUhost_SendIdInfo = RAUque_GetSendDataInfo();		// ここで次のテーブルに切り替え(次の送信予定テーブルがなければＮＵＬＬが返る)
				memset( &RAUhost_SendNtDataInfo, 0x00, sizeof(T_SEND_NT_DATA) );
				RAU_uiTcpBlockNo = 1;	// 次のテーブルデータを送るときはＴＣＰブロック№１から開始
				RAU_ucTcpBlockSts = 0;	// 次のパケットが最終になる場合は後で１になる。
// MH810100(S) 2020/07/10 車番チケットレス(#4532 【検証課指摘事項】精算で硬貨投入後に取り消しボタンを押すと、以降の精算で精算データがNT-NETサーバに送信されない(No.25))
				// 次がなく、切断有なら一度切断する
				if(pRAUhost_SendIdInfo == NULL){
					if( RauConfig.tcp_disconect_flag == 0 ) {					// 切断あり
						RAU_Tm_TCP_DISCNCT.tm = 0;
						RAU_f_TCPdiscnct_rq = 0;
						RAU_f_TCPdiscnct_ov = 1;	// 上り回線のＴＣＰ接続を切断
					}
				}
// MH810100(E) 2020/07/10 車番チケットレス(#4532 【検証課指摘事項】精算で硬貨投入後に取り消しボタンを押すと、以降の精算で精算データがNT-NETサーバに送信されない(No.25))
			}
		} else {													// 送信完了したのは最終パケット
			// 送信完了分のLOGリードポインタ更新
			if(pRAUhost_SendIdInfo->pbuff_info->uc_DataId != endDataId){
				logId = RAUhost_ConvertDataIdToLogData(pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
				if(logId != -1){
					Ope_Log_TargetVoidReadPointerUpdate(logId, eLOG_TARGET_REMOTE);
					if(pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus == RAU_NEARFULL_NEARFULL){
						pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus = RAU_NEARFULL_CANCEL;
						pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatusBefore = RAU_NEARFULL_CANCEL;
					}
				}
			}
			if( RauConfig.tcp_disconect_flag == 0 ) {					// 切断あり
				RAU_Tm_TCP_DISCNCT.tm = 0;
				RAU_f_TCPdiscnct_rq = 0;
				RAU_f_TCPdiscnct_ov = 1;	// 上り回線のＴＣＰ接続を切断
			}

			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// 送信を終了したので送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)に戻す。
			RAUque_DeleteQue();			// テーブルから送信完了したデータを削除
			pRAUhost_SendIdInfo = NULL;	// テーブルの参照をやめる。
		}
		RAU_uc_retrycnt_h = 0;		// リトライカウンタをイニシャライズ（０クリア）
		RAU_Tm_Ackwait.tm = 0;		// 上り回線用ＡＣＫ受信待ちタイマクリア
		RAU_uc_retrycnt_reconnect_h = 0;		// リトライカウンタをイニシャライズ（０クリア）
		RAU_Tm_Ackwait_reconnect.tm = 0;		// 上り回線用再接続待ちタイマクリア
		RAU_ui_data_length_h = 0;
		RAU_uc_mode_h = S_H_IDLE;	// ＡＣＫ待ちを解除
	} else {										// 下り回線
		RAUque_UpDate(pRAUhost_SendThroughInfo);	// 送信しているデータの情報を更新
		if( RAU_ucTcpBlockSts2 == 0 ){					// 送信完了したのは途中のパケット
			if( pRAUhost_SendThroughInfo->send_data_count <= pRAUhost_SendThroughInfo->send_complete_count ){	// 送信スルーデータテーブルのデータは全部送った。
				pRAUhost_SendThroughInfo = NULL;	// 次に送るデータはない。
				memset( &RAUhost_SendNtThroughInfo, 0x00, sizeof(T_SEND_NT_DATA) );
				RAU_uiTcpBlockNo2 = 1;	// 次のスルーデータを送るときはＴＣＰブロック№１から開始  
				RAU_ucTcpBlockSts2 = 0;	// 次のパケットが最終になる場合は後で１になる。  
			}
		} else {					// 送信完了したのは最終パケット
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンス０(通常状態)に戻す。
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 			RAU_f_RcvSeqCnt_rq = 0;
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]

			if( RauConfig.Dpa_port_watchdog_tm == 0 ){		// 下り回線通信監視タイマ使用禁止
				RAU_Tm_Port_Watchdog.tm = 0;
			} else {										// 下り回線通信監視タイマ使用許可
				RAU_Tm_Port_Watchdog.bits0.bit_0_13 = RauConfig.Dpa_port_watchdog_tm;
				RAU_f_Port_Watchdog_rq = 1;					// 下り回線通信監視タイマ(起動)
				RAU_f_Port_Watchdog_ov = 0;					// タイムアウトするまでに相手がＴＣＰコネクションをクローズしないとこちらから切ります。
			}

			RAUdata_TableClear(0);	// スルーデータの削除
			memset(&RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE], 0, sizeof(T_SEND_DATA_ID_INFO));	// スルーデータ用テーブル情報クリア
			pRAUhost_SendThroughInfo = NULL;
		}
		RAU_uc_retrycnt_h2 = 0;		// リトライカウンタをイニシャライズ（０クリア）
		RAU_Tm_Ackwait2.tm = 0;		// ＡＣＫ受信待ちタイマキャンセル
		RAU_ui_data_length_h = 0;
		RAU_uc_mode_h2 = S_H_IDLE;	// ＡＣＫ待ちを解除
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_Mh_sum_check
 *[]----------------------------------------------------------------------[]
 *| summary	: チェックサムのチェック処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void RAU_Mh_sum_check(void)
{
	eRAU_TABLEDATA_SET	bRet;
	ulong	ui_w_seq;
	uchar	ucCrc[2];
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	if( RauConfig.Dpa_port_watchdog_tm == 0 ){	// 下り回線監視タイマ使用禁止
		RAU_Tm_Port_Watchdog.tm = 0;
	} else {							// 下り回線監視タイマ使用許可
		RAU_Tm_Port_Watchdog.bits0.bit_0_13 = RauConfig.Dpa_port_watchdog_tm;
		RAU_f_Port_Watchdog_rq = 1;			// 下り回線監視タイマ(起動)
		RAU_f_Port_Watchdog_ov = 0;
	}
	ui_w_seq = c_arraytoint32( &RAU_huc_rcv_work_h[16], 3 );		// 受信データのシーケンスNo.を取得
	RAU_CrcCcitt(RAU_ui_data_length_h-2, 0, RAU_huc_rcv_work_h, NULL, ucCrc, 1 );	// ＣＲＣ算出
	if( ( ucCrc[0] == RAU_huc_rcv_work_h[RAU_ui_data_length_h-2] )&&	// 算出したＣＲＣと
		( ucCrc[1] == RAU_huc_rcv_work_h[RAU_ui_data_length_h-1] ) ){	// 取得したＣＲＣが一致

		if( (( ui_w_seq != 0 )&&			// シーケンスNo.0は強制受信(リセット)なので除外
			( ui_w_seq == RAU_ui_seq_bak )) ){	// 前回のシーケンスNo.と比較
			bRet = RAU_DATA_NORMAL;			// 前回と同一なデータを受信(→データを破棄してＡＣＫ返信)
		} else {
			u.uc[0] = RAU_huc_rcv_work_h[21];	// ＴＣＰブロックナンバーの取り出し
			u.uc[1] = RAU_huc_rcv_work_h[22];
			bRet = RAUid_SetRcvNtData(&RAU_huc_rcv_work_h[26], RAU_ui_data_length_h - 28, u.us, RAU_huc_rcv_work_h[23]);	// 受信データ(要求データ)の処理
		}
		switch( bRet ){
		case RAU_DATA_NORMAL:
			RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_ACK, 0, ui_w_seq, &RAU_ui_txdlength_h2);		// ＡＣＫデータの作成
			RAU_ui_seq_bak = ui_w_seq;																// 前回のシーケンスNo.を保存
			break;
		case RAU_DATA_CONNECTING_FROM_HOST:	// RAUid_SetRcvNtData関数でテーブル送信中にテーブル送信要求を受信するとこれを返す。
			RAU_uc_rcvretry_cnt = 0;
			RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 90, ui_w_seq, &RAU_ui_txdlength_h2);		// ＮＡＫ９０データ作成
			break;
		default:
			if( RAU_uc_rcvretry_cnt < RauConfig.Dpa_data_rcv_rty_cnt ){	// 受信リトライ回数チェック
				RAU_uc_rcvretry_cnt++;
				RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 0, ui_w_seq, &RAU_ui_txdlength_h2);	// ＮＡＫデータの作成
			} else {										// リトライ回数オーバー
				RAU_uc_rcvretry_cnt = 0;
				RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 99, ui_w_seq, &RAU_ui_txdlength_h2);	// ＮＡＫ９９データ作成
			}
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
	} else {	// 受信データ長とヘッダのデータ長が不一致またはチェックサムＮＧ
		RAUhost_SetError(ERR_RAU_DPA_RECV_CRC_D);				// エラーコード８２(下り回線受信データＣＲＣエラー)
		if( RAU_uc_rcvretry_cnt < RauConfig.Dpa_data_rcv_rty_cnt ){			// 受信リトライ回数チェック
			RAU_uc_rcvretry_cnt++;
			RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 0, ui_w_seq, &RAU_ui_txdlength_h2);	// ＮＡＫデータ作成
		} else {												// リトライ回数オーバー
			RAU_uc_rcvretry_cnt = 0;
			RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 99, ui_w_seq, &RAU_ui_txdlength_h2);	// ＮＡＫ９９データ作成
		}
	}

	memset( RAU_huc_rcv_work_h, 0x00, RAU_RCV_MAX_H );				// 受信データバッファ(ワーク)クリア
	RAU_ui_data_length_h = 0;										// 受信データ長クリア

	DpaSndQue_Set(RAU_huc_txdata_h2, RAU_ui_txdlength_h2, DOWNLINE);	// 下り回線送信キューに返信データをセット

	RAU_uc_mode_h2 = S_H_IDLE;	// ＡＣＫ待ちを解除してアイドルに移行
}

//==============================================================================
//	ＡＣＫ待ちタイマのタイムアウト処理
//
//	@argument	port			対象ポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		なし
//
//	@attention	ＤｏＰａの２ポート対応専用ルーチンです。
//
//	@note		ホストに対してデータを送信したときＡＣＫの来ないまま一定時間が過ぎると
//				本関数が呼び出されます。
//
//	@see		Mh_time_out
//
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_AckWait_TimeOut(int port)
{
	if( port == UPLINE ){	// 上り回線
		if( RAU_uc_retrycnt_h <= RauConfig.Dpa_data_rty_cn ){	// リトライカウンタチェック
			DPA2Port_RestoreOfSendData( port );			// パケットの再送信
		} else {										// リトライオーバー
			DPA2Port_DpaSeqIncrement( port );			// モデム切断後データの再送信をするのでシーケンスNo.を変える。
			RAU_uc_retrycnt_h = 0;							// リトライカウンタクリア
			RAU_Tm_Ackwait.tm = 0;							// 上り回線用ＡＣＫ受信待ちタイマクリア
			RAU_uc_retrycnt_reconnect_h = 0;				// リトライカウンタクリア
			RAU_Tm_Ackwait_reconnect.tm = 0;				// 上り回線用再接続待ちタイマクリア
			RAU_uc_mode_h = S_H_IDLE;						// ＡＣＫ待ちを解除してアイドルに移行
			RAUque_DeleteQue();							// テーブルから送信完了したデータを削除
			pRAUhost_SendIdInfo = NULL;					// テーブルの参照をやめる。
			KSG_RauSetMdmCutState(1);					// モデム切断ステータスを"切断要求"にする。
			RAUhost_SetError(ERR_RAU_DPA_SEND_RETRY_OVER);	// エラーコード９５(上り回線データ再送回数オーバー)をＩＢＷに通知
			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// 送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)に戻す。
		}
	} else {				// 下り回線
		if( RAU_uc_retrycnt_h2 <= RauConfig.Dpa_data_snd_rty_cnt ){	// リトライカウンタチェック
			DPA2Port_RestoreOfSendData( port );			// パケットの再送信
		} else {										// リトライオーバー
			DPA2Port_DpaSeqIncrement( port );			// もうこのスルーデータを送らないのでシーケンスNo.を変える。
			RAU_uc_retrycnt_h2 = 0;							// リトライカウンタクリア
			RAU_Tm_Ackwait2.tm = 0;							// 下り回線用ＡＣＫ受信待ちタイマクリア
			RAU_uc_mode_h2 = S_H_IDLE;						// ＡＣＫ待ちを解除してアイドルに移行
			// スルーデータ送信ＡＣＫ待ちリトライオーバー時、送信情報がクリアされていなかったのでクリアする
			memset(&RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE], 0, sizeof(T_SEND_DATA_ID_INFO));	// スルーデータ用テーブル情報クリア
			pRAUhost_SendThroughInfo = NULL;			// テーブルの参照をやめる。
			KSG_RauSetMdmCutState(1);					// モデム切断ステータスを"切断要求"にする。
			RAUhost_SetError(ERR_RAU_DPA_SEND_RETRY_OVER_D);// エラーコード８７(下り回線データ再送回数オーバー)をＩＢＷに通知
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンス０(通常状態)に戻す。
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 			RAU_f_RcvSeqCnt_rq = 0;							// ホストがＩＢＷからのデータを要求している状態の解除
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			RAUdata_TableClear(0);						// スルーデータの削除
		}
	}
}

//==============================================================================
//	ＡＣＫ待ちタイマのTCP切断時処理
//
//	@argument	port			対象ポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		なし
//
//	@attention	ＤｏＰａの２ポート対応専用ルーチンです。
//
//	@note		ホストに対してデータを送信したときＡＣＫの来ないままTCP切断されると
//				本関数が呼び出されます。
//				ParkingWebロードバランサー対応で追加
//
//	@see		Mh_time_out
//
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_AckWait_Disconnected(int port)
{
	RAU_uc_retrycnt_reconnect_h++;	// TCP切断時リトライカウント
	if( port == UPLINE ){	// 上り回線
		if( RAU_uc_retrycnt_reconnect_h <= RauConfig.Dpa_data_rty_cn_disconnected ){	// リトライカウンタチェック
			RAU_uc_retrycnt_h = 0;							// リトライカウンタクリア
			RAU_Tm_Ackwait.tm = 0;							// 上り回線用ＡＣＫ受信待ちタイマクリア
			RAU_Tm_Ackwait_reconnect.bits0.bit_0_13 = RauConfig.Dpa_com_wait_reconnect_tm;	// 再接続タイマ起動
			RAU_f_Ackwait_reconnect_rq = 1;
			RAU_f_Ackwait_reconnect_ov = 0;
		} else {										// リトライオーバー
			DPA2Port_DpaSeqIncrement( port );			// モデム切断後データの再送信をするのでシーケンスNo.を変える。
			RAU_uc_retrycnt_h = 0;							// リトライカウンタクリア
			RAU_Tm_Ackwait.tm = 0;							// 上り回線用ＡＣＫ受信待ちタイマクリア
			RAU_uc_retrycnt_reconnect_h = 0;				// リトライカウンタクリア
			RAU_Tm_Ackwait_reconnect.tm = 0;				// 上り回線用再接続待ちタイマクリア
			RAU_uc_mode_h = S_H_IDLE;						// ＡＣＫ待ちを解除してアイドルに移行
			RAUque_DeleteQue();							// テーブルから送信完了したデータを削除
			pRAUhost_SendIdInfo = NULL;					// テーブルの参照をやめる。
			KSG_RauSetMdmCutState(1);					// モデム切断ステータスを"切断要求"にする。
			RAUhost_SetError(ERR_RAU_DPA_SEND_RETRY_OVER);	// エラーコード９５(上り回線データ再送回数オーバー)をＩＢＷに通知
			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// 送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)に戻す。
		}
	} else {				// 下り回線
		// ACK待ちリトライオーバーと同じ処理を行う。ただし、モデムの切断は行わない。
		DPA2Port_DpaSeqIncrement( port );			// もうこのスルーデータを送らないのでシーケンスNo.を変える。
		RAU_uc_retrycnt_h2 = 0;							// リトライカウンタクリア
		RAU_Tm_Ackwait2.tm = 0;							// 下り回線用ＡＣＫ受信待ちタイマクリア
		RAU_uc_mode_h2 = S_H_IDLE;						// ＡＣＫ待ちを解除してアイドルに移行
		// スルーデータ送信ＡＣＫ待ちリトライオーバー時、送信情報がクリアされていなかったのでクリアする
		memset(&RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE], 0, sizeof(T_SEND_DATA_ID_INFO));	// スルーデータ用テーブル情報クリア
		pRAUhost_SendThroughInfo = NULL;			// テーブルの参照をやめる。
		RAUhost_SetError(ERR_RAU_DPA_SEND_RETRY_OVER_D);// エラーコード８７(下り回線データ再送回数オーバー)をＩＢＷに通知
		RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンス０(通常状態)に戻す。
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 		RAU_f_RcvSeqCnt_rq = 0;							// ホストがＩＢＷからのデータを要求している状態の解除
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
		RAUdata_TableClear(0);						// スルーデータの削除
	}
}

//==============================================================================
//	ホストへ送信するテーブルデータの準備
//
//	@argument	なし
//
//	@return		なし
//
//	@attention	複数パケットに跨るデータを送信する場合、２パケット目以降のデータは
//				DPA2Port_SendNextDataToHost()が用意したものをキューに詰めます。
//
//	@note		ホストへ上り回線を使用してテーブルデータの送信が行えるかを判断し
//				(可能であれば)送信データを１パケット分用意して送信キューに詰め込みます。
//
//	@see		DPA2Port_SendNextDataToHost
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_BeginToSendTableData(void)
{
	int		i;

	if( KSG_RauGetMdmCutState() != 0 )	// モデム切断ステータスが"切断要求"や"切断中"のとき
		return;

	if( KSG_gPpp_RauStarted == 0 )	// モデムステータスが"通信中"でないとき
		return;

	if( (RauCtrl.tcpConnectFlag.port.upload == 1) && (Rau_SedEnable == 1) ){	// ＴＣＰ接続中 && 送信可
		Rau_SedEnable = 0;
		if( RauCT_SndReqID != 0 ){											// 通信チェックデータ送信要求あり
			RAU_ui_txdlength_h = RAUhost_CreateDopaPacketTest(RAU_huc_txdata_h, RauCT_SndReqID);
			if( RAU_ui_txdlength_h ){											// 通信チェック結果[ＮＧ]データのパケット化成功
				RAU_uc_mode_h = S_H_ACK;										// ＡＣＫ待ち状態へ遷移
				RAU_Tm_TCP_DISCNCT.tm = 0;									// ＴＣＰ切断待ちタイマ停止
// MH322914 (s) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
//				RAU_f_No_Action_ov = 1;									// ダミーデータを送信するために無通信タイマをタイムアウトさせる
				if(	prm_get( COM_PRM,S_NTN,121,1,1) != 0) {					// 駐車場センター形式
					RAU_f_No_Action_ov = 1;									// ダミーデータを送信するために無通信タイマをタイムアウトさせる
				}
				else {
					DpaSndQue_Set( RAU_huc_txdata_h, RAU_ui_txdlength_h, UPLINE );		// 上り回線用の送信キューにデータをセット
				}
// MH322914 (e) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
				memcpy( RAU_uc_txdata_h_keep, RAU_huc_txdata_h, DOPA_HEAD_SIZE );	// 送信するパケットのヘッダ部分(ＤｏＰａヘッダ)を記憶
				RAU_ui_txdlength_h_keep = RAU_ui_txdlength_h;
				RAU_Tm_Ackwait.bits0.bit_0_13 = RauConfig.Dpa_com_wait_tm;		// ＡＣＫ待ちタイマ起動
				RAU_f_Ackwait_rq = 1;
				RAU_f_Ackwait_ov = 0;
				return;
			}
		}
	}

	if( (RauCtrl.tcpConnectFlag.port.upload == 1)	// ＴＣＰ接続中
		&&(pRAUhost_SendIdInfo != NULL)		// 現在テーブルを参照中
		&&((pRAUhost_SendIdInfo->send_data_count > pRAUhost_SendIdInfo->send_complete_count)||(pRAUhost_SendIdInfo->send_data_count == 0)) ){	// 現在参照しているテーブルに送信すべきデータがまだある。


		if( RAUhost_Error01 == 1 ){													// エラーコード０１(上り回線通信エラー)発生中
			RAUhost_Error01 = 0;
			RAU_err_chk(ERR_RAU_HOST_COMMUNICATION, 0, 0, 0, NULL);	// エラー０１(解除)をＩＢＷに通知
		}

		if(RAUhost_IdentifyDataType( UPLINE ) == -1) {	// 送信するデータを１パケット分生成してバッファ"RAU_huc_txdata_h"に充填
			DPA2Port_SendNextDataToHost(UPLINE);		// 次の送信データを送信キューに詰める。
			return;
		}

		RAU_uc_mode_h = S_H_ACK;		// ＡＣＫ待ち状態へ遷移
		RAU_Tm_TCP_DISCNCT.tm = 0;	// ＴＣＰ切断待ちタイマ停止

		DpaSndQue_Set( RAU_huc_txdata_h, RAU_ui_txdlength_h, UPLINE );		// 上り回線用の送信キューにデータをセット

		memcpy( RAU_uc_txdata_h_keep, RAU_huc_txdata_h, DOPA_HEAD_SIZE );	// 送信するパケットのヘッダ部分(ＤｏＰａヘッダ)を記憶
		RAU_ui_txdlength_h_keep = RAU_ui_txdlength_h;

		RAU_Tm_Ackwait.bits0.bit_0_13 = RauConfig.Dpa_com_wait_tm;		// ＡＣＫ待ちタイマ起動
		RAU_f_Ackwait_rq = 1;
		RAU_f_Ackwait_ov = 0;
	} else {
		if( ((RauConfig.tcp_disconect_flag == 0) &&
// MH810100(S) 2020/07/10 車番チケットレス(#4532 【検証課指摘事項】精算で硬貨投入後に取り消しボタンを押すと、以降の精算で精算データがNT-NETサーバに送信されない(No.25))
//			 (RAU_GetUpNetState() == RAU_NET_PPP_OPENED))	// 上り回線のコネクションステータスはＰＰＰ接続中
			// つながってたらそのまま
			 ((RAU_GetUpNetState() == RAU_NET_PPP_OPENED) ||		// 上り回線のコネクションステータスはＰＰＰ接続中
			  (RAU_GetUpNetState() == RAU_NET_TCP_CONNECTIED)) ) 	// 上り回線のコネクションステータスはＴＣＰ接続中
// MH810100(E) 2020/07/10 車番チケットレス(#4532 【検証課指摘事項】精算で硬貨投入後に取り消しボタンを押すと、以降の精算で精算データがNT-NETサーバに送信されない(No.25))
		||  ((RauConfig.tcp_disconect_flag != 0) &&
			 ((RAU_GetUpNetState() == RAU_NET_PPP_OPENED) ||		// 上り回線のコネクションステータスはＰＰＰ接続中
			  (RAU_GetUpNetState() == RAU_NET_TCP_CONNECTIED))) ){	// 上り回線のコネクションステータスはＴＣＰ接続中
			if( pRAUhost_SendIdInfo == NULL ){	// 現在テーブル未参照
				if( RAU_f_TCPtime_wait_ov == 1 ){		// ＴＣＰコネクションＴＩＭＥ＿ＷＡＩＴ解除待ちタイマタイムアウト
					for(i=0; i<RAU_SEND_THROUGH_DATA_TABLE; i++ ){	// すべてのテーブル情報(下り回線スルーデータは除く)をクリア
						memset( &RAUque_SendDataInfo[i], 0, sizeof(RAUque_SendDataInfo[0]) );
					}
					RAUdata_GetNtDataInfo(RAUque_SendDataInfo, &RAUhost_SendRequest, RAUhost_SendRequestFlag);	// 全テーブルより"送信対象データ"等の情報を取得する。
					RAUque_CurrentSendData = (ushort)-1;			// これから初めて送信するデータのテーブル番号(0:入庫データ, 1:出庫データ, 2:精算データ…)
					RAUhost_SendRequestFlag = 0;					// ＩＢＷまたはホストからの送信要求データありフラグ(解除)
					pRAUhost_SendIdInfo = RAUque_GetSendDataInfo();	// これから送信するデータが存在するテーブルを参照(RAUque_CurrentSendData ← 送信動作に入る最初のテーブル番号)
					memset( &RAUhost_SendNtDataInfo, 0x00, sizeof(T_SEND_NT_DATA) );
					RAU_uc_retrycnt_h = 0;			// リトライカウンタをイニシャライズ（０クリア）
					RAU_uc_retrycnt_reconnect_h = 0;			// リトライカウンタをイニシャライズ（０クリア）

					if( pRAUhost_SendIdInfo != NULL ){	// 全テーブルを見たけれど、どのテーブルの送信対象になっていなかったときはＮＵＬＬ
						RauCtrl.TCPcnct_req = 1;		// ＴＣＰ接続要求フラグ(接続要求)
						RAU_uiTcpBlockNo = 1;
						RAU_ucTcpBlockSts = 0;
						RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_TABLE_SEND);	// 送信シーケンス１(ＨＯＳＴへのテーブル送信開始状態)にする。

						if( RauCT_SndReqID != 0 ){						// 通信チェックデータ送信要求あり
							RauCT_SndReqID = 0;							// 送信中止
						}
					} else {
						if( RauCT_SndReqID != 0 ){			// 通信チェックデータ送信要求あり
							RauCtrl.TCPcnct_req = 1;		// ＴＣＰ接続要求フラグ(接続要求)
							RAU_uiTcpBlockNo = 1;
							RAU_ucTcpBlockSts = 0;
							RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_TABLE_SEND);	// 送信シーケンス１(ＨＯＳＴへのテーブル送信開始状態)にする。
							Rau_SedEnable = 1;
							return;
						}
						else {
							// 切断ありならば切断する
							if( RauConfig.tcp_disconect_flag == 0 && RauCtrl.tcpConnectFlag.port.upload == 1 ) {	// 切断ありでTCP接続中
								RAU_Tm_DISCONNECT.tm = 0;
								RAU_f_discnnect_rq = 0;
								RAU_f_discnnect_ov = 1;	// 上り回線のＴＣＰ接続を切断
							}
							RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// 送信を終了したので送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)に戻す。
							RAU_uc_retrycnt_h = 0;									// リトライカウンタをイニシャライズ（０クリア）
							RAU_Tm_Ackwait.tm = 0;									// 上り回線用ＡＣＫ受信待ちタイマクリア
							RAU_uc_retrycnt_reconnect_h = 0;						// リトライカウンタをイニシャライズ（０クリア）
							RAU_Tm_Ackwait_reconnect.tm = 0;						// 上り回線用再接続待ちタイマクリア
							RAU_ui_data_length_h = 0;
							RAU_uc_mode_h = S_H_IDLE;								// ＡＣＫ待ちを解除
						}
					}
				}
// MH810105(S) R.Endo 2021/12/27 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			// NOTE:以下の通信断検知処理について
			//   ・IBKでは動作しない処理として削除され、通信断検知は別途作成とされている。
			//   ・デバッガを使用して通す手順は以下の通り。
			//     1. 通信チェックデータ送信要求あり(RauCT_SndReqID != 0)の送信シーケンス１設定処理
			//        (RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_TABLE_SEND))にブレークポイントを設定する。
			//     2. LAN Simulatorから通信チェック要求データを送信する。
			//     3. ブレークポイントで停止後、LAN Simulatorをクローズする。
			//     4. 上記判定文にブレークポイントを設定し、処理を再開する。
			//     5. ブレークポイントで停止後、TCP接続要求フラグ(RauCtrl.TCPcnct_req)を接続要求なし(0)に書き換え、
			//        処理を再開する。
// MH810105(E) R.Endo 2021/12/27 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			} else if( RauCtrl.TCPcnct_req == 0 && RauCtrl.tcpConnectFlag.port.upload == 0){	// 相手から上り回線のＴＣＰコネクションがクローズされた。
				RAU_uc_retrycnt_h = 0;								// リトライカウンタをイニシャライズ（０クリア）
				RAU_Tm_Ackwait.tm = 0;								// 上り回線用ＡＣＫ受信待ちタイマクリア
				RAU_uc_retrycnt_reconnect_h = 0;					// リトライカウンタをイニシャライズ（０クリア）
				RAU_Tm_Ackwait_reconnect.tm = 0;					// 上り回線用再接続待ちタイマクリア
				RAU_ui_data_length_h = 0;							// 受信データ長クリア
				RAU_uc_mode_h = S_H_IDLE;							// ＡＣＫ待ちを解除してアイドルに移行
				RAUque_DeleteQue();								// テーブルから送信完了したデータのみ消す。
				pRAUhost_SendIdInfo = NULL;						// テーブルの参照をやめる。
				KSG_RauSetMdmCutState(1);						// モデム切断ステータスを"切断要求"にする。
				RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// 送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)
// MH810105(S) R.Endo 2021/12/27 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 				RAU_f_RcvSeqCnt_rq = 0;							// ホストがＩＢＷからのデータを要求している状態の解除
				// NOTE:以下の受信シーケンス０設定処理について
				//   上位PCがIBKを経由してIBWにデータ要求している最中に上り回線が上位PCより切断された場合、
				//   IBKとしてはIBWからの応答データを上位PCに送信する必要がない為、IBWからの応答待ちを解除している。
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンス０(通常状態)に戻す。
// MH810105(E) R.Endo 2021/12/27 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			}
		}
	}
}

//==============================================================================
//	ホストへ送信するスルーデータの準備
//
//	@argument	なし
//
//	@return		なし
//
//	@attention	スルーデータが送信できるのは受信シーケンスが２のときに限られます。
//
//	@note		ホストへ下り回線を使用してスルーデータの送信が行えるかを判断し
//				(可能であれば)送信データを１パケット分用意して送信キューに詰め込みます。
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_BeginToSendThroughData(void)
{
	if( KSG_RauGetMdmCutState() != 0 )	// モデム切断ステータスが"切断要求"や"切断中"のとき
		return;

	if( KSG_gPpp_RauStarted == 0 )	// モデムステータスが"通信中"でないとき
		return;

	if( (RauCtrl.tcpConnectFlag.port.download == 1)	// ＴＣＰ接続中
		&&(pRAUhost_SendThroughInfo != NULL)	// 現在テーブルを参照中
		&&(pRAUhost_SendThroughInfo->send_data_count > pRAUhost_SendThroughInfo->send_complete_count) ){	// 現在参照しているテーブルに送信すべきデータがまだある。


		RAUhost_IdentifyDataType( DOWNLINE );	// 送信するデータを１パケット分生成してバッファ"RAU_huc_txdata_h2"に充填

		RAU_uc_mode_h2 = S_H_ACK;		// ＡＣＫ待ち状態へ遷移

		DpaSndQue_Set( RAU_huc_txdata_h2, RAU_ui_txdlength_h2, DOWNLINE );	// 下り回線用の送信キューにデータをセット

		memcpy( RAU_uc_txdata_h_keep2, RAU_huc_txdata_h2, DOPA_HEAD_SIZE );	// 送信するパケットのヘッダ部分(ＤｏＰａヘッダ)を記憶
		RAU_ui_txdlength_h_keep2 = RAU_ui_txdlength_h2;

		RAU_Tm_Ackwait2.bits0.bit_0_13 = RauConfig.Dpa_ack_wait_tm;	// ＡＣＫ待ちタイマ起動
		RAU_f_Ackwait2_rq = 1;
		RAU_f_Ackwait2_ov = 0;
	} else {
		if( RAU_GetDownNetState() == RAU_NET_TCP_CONNECTIED){	// 下り回線のコネクションステータスはＴＣＰ接続確立中
			if( pRAUhost_SendThroughInfo == NULL ){		// 送信要求がかかってから最初の送信対象データ
				if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){		// 受信シーケンス２(ＨＯＳＴからの要求データ受信完了状態)
					memset( &RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE], 0, sizeof(RAUque_SendDataInfo[0]) );
					RAUdata_GetNtDataInfoThrough(RAUque_SendDataInfo);				// スルーデータテーブルより"送信対象データ数"等の情報を取得する。

					if( RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE].send_req != 0 ){	// スルーデータが送信テーブルにある
						pRAUhost_SendThroughInfo = &RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE];
					} else {
						pRAUhost_SendThroughInfo = NULL;
					}
					memset( &RAUhost_SendNtThroughInfo, 0x00, sizeof(T_SEND_NT_DATA) );
					RAU_uc_retrycnt_h2 = 0;			// リトライカウンタをイニシャライズ（０クリア）

					if( pRAUhost_SendThroughInfo != NULL ){
						RAU_uiTcpBlockNo2 = 1;
						RAU_ucTcpBlockSts2 = 0;
						RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_THROUGH_SEND);	// 受信シーケンス３(ＨＯＳＴへのスルーデータ送信開始状態)にする。
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 						//NOTE:メインから応答データ受信後、HOSTへデータ送信のリトライ中にタイムアウトが起きると受信シーケンスフラグが「0」にリセットされ
// 						//テーブルデータの送信が開始されフェーズがずれることがある。現状、メインから受信したスルーデータを最終パケットまで送信した時点で
// 						//タイマクリアしているが、受信シーケンスキャンセルタイマはメインからスルーデータを受信した時点でクリアすべきである
// 						RAU_f_RcvSeqCnt_rq = 0;//メインから応答データを受信したので受信シーケンスキャンセルタイマ停止
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
					}
				}
			}
		}
		else{	// ＴＣＰ接続確立中ではない
			// 下り回線でスルーデータ応答を送信する際にＴＣＰ未接続時の場合、テーブルにデータが
			// 残り、次回ＴＣＰ接続確立時に送信されてしまう為、テーブルから削除しておく
			if( pRAUhost_SendThroughInfo == NULL ){
			// 応答送信前
				if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){		// 受信シーケンス２(ＨＯＳＴからの要求データ受信完了状態)
					RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);					// 受信シーケンス０(通常状態)に戻す。
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 					RAU_f_RcvSeqCnt_rq = 0;											// 受信シーケンスキャンセルタイマ動作許可(解除)
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
				}
			}
			else {
			// 応答送信中
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);						// 受信シーケンス０(通常状態)に戻す。
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 				RAU_f_RcvSeqCnt_rq = 0;												// 受信シーケンスキャンセルタイマ動作許可(解除)
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
				RAUdata_TableClear(0);												// スルーデータの削除
				pRAUhost_SendThroughInfo = NULL;									// 送信スルーデータテーブル情報ポインタクリア
				if( RAU_uc_mode_h2 == S_H_ACK ){									// ＡＣＫ受信待ち
					DPA2Port_DpaSeqIncrement(DOWNLINE);								// シーケンスNo.インクリメント
				}
				RAU_uc_retrycnt_h2 = 0;												// リトライカウンタをイニシャライズ（０クリア）
				RAU_Tm_Ackwait2.tm = 0;												// ＡＣＫ受信待ちタイマキャンセル
				RAU_ui_data_length_h = 0;											// 受信データ長クリア
				RAU_uc_mode_h2 = S_H_IDLE;											// ＡＣＫ待ちを解除
			}
		}
	}
}

//==============================================================================
//	受信データの種別判定
//
//	@argument	port	対象ポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		0		データ受信
//	@return		1		ＡＣＫ受信
//	@return		2		ＮＡＫ受信
//
//	@attention	ＮＡＫ９９(再送信の停止要求)を受信したときはＡＣＫ扱いとする。
//
//	@note		ホストから受信したデータを解析し、その種別を返します。
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
uchar RAU_dat_check(int port)
{
	uchar	result;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	u.uc[0] = RAU_huc_rcv_work_h[21];
	u.uc[1] = RAU_huc_rcv_work_h[22];

	result = 0;
	if( u.us == 1 ){					// ＴＣＰブロックナンバーが１
		if( RAU_huc_rcv_work_h[23] == 1 ){	// ＴＣＰブロックステータスが１
			switch( RAU_huc_rcv_work_h[26] ){
			case 0x06:					// ＡＣＫ
				result = 1;
				break;
			case 0x15:					// ＮＡＫ
				u.uc[0] = RAU_huc_rcv_work_h[24];
				u.uc[1] = RAU_huc_rcv_work_h[25];
				if( u.us == 99 ){			// 予備項目９９(ＮＡＫ９９)
					if( port == UPLINE ){	// 上り回線
						RAUhost_SetError(ERR_RAU_DPA_RECV_NAK99);	// エラーコード９７(上り回線送信データ強制停止受信)をＩＢＷに通知
					} else {				// 下り回線
						RAUhost_SetError(ERR_RAU_DPA_RECV_NAK99_D);	// エラーコード８８(下り回線送信データ強制停止受信)をＩＢＷに通知
					}
					result = 1;			// ＡＣＫと同じ扱い
				} else {
					result = 2;
				}
				break;
			}
		}
	}
	return	result;
}


/****************************************************************/
/*	関数名		=	RAU_DpaSndHedSet							*/
/*	機能		=	Dopa送信データ ヘッダー部登録処理			*/
/*	備考		=												*/
/****************************************************************/
void	RAU_DpaSndHedSet(uchar ucLastPacket, int port)
{
	ulong	seq_no;
	uchar	*pTxData;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	if( port == UPLINE ){	// 上り回線
		seq_no = RAU_ui_RAUDPA_seq;
		u.us = RAU_uiTcpBlockNo;
		pTxData = RAU_huc_txdata_h;
		c_int32toarray(&pTxData[0], (ulong)RAU_ui_txdlength_h, 4);	// データ長セット
	}
	else if(port == CREDIT){
		seq_no = Credit_RAUDPA_seq;
		u.us = Credit_TcpBlockNo;
		pTxData = RAU_huc_txdata_CRE;
		c_int32toarray(&pTxData[0], (ulong)RAU_ui_txdlength_CRE, 4);	// データ長セット
	} else {				// 下り回線
		seq_no = RAU_ui_RAUDPA_seq2;
		u.us = RAU_uiTcpBlockNo2;
		pTxData = RAU_huc_txdata_h2;
		c_int32toarray(&pTxData[0], (ulong)RAU_ui_txdlength_h2, 4);// データ長セット
	}

	memcpy( &pTxData[4], uc_dpa_rem_str, 12 );					// "AMANO_REMOTE"セット

	c_int32toarray(&pTxData[16], seq_no, 3);					// シーケンスNo.セット

	pTxData[19] = 0;											// データ部コード(未使用０固定)
	pTxData[20] = 0;											// 電文識別コード(0:中断要求なし, 1:中断要求あり)

	pTxData[21] = u.uc[0];										// ＴＣＰブロックナンバー
	pTxData[22] = u.uc[1];

	pTxData[23] = ucLastPacket;									// ＴＣＰブロックステータス

	pTxData[24] = 0x00;											// 予備
	pTxData[25] = 0x00;											// 予備
}

//==============================================================================
//	データ送信用シーケンス番号のインクリメント
//
//	@argument	port			対象ポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		なし
//
//	@attention	ＤｏＰａの２ポート対応専用ルーチンです。
//
//	@note		送信するパケットデータに付加するシーケンシャル番号と
//				ＴＣＰブロック番号の加算を行います。
//
//	@see		RAU_DpaSeqAdd
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_DpaSeqIncrement(int port)
{
	if( port == UPLINE ){	// 上り回線
		// シーケンシャル番号
		RAU_ui_RAUDPA_seq++;
		if( RAU_ui_RAUDPA_seq > 999 ){
			RAU_ui_RAUDPA_seq = 1;
		}

		// ＴＣＰブロック番号
		if( RAU_uiTcpBlockNo == 65535 ){
			RAU_uiTcpBlockNo = 1;
		} else {
			RAU_uiTcpBlockNo++;
		}
	} else {				// 下り回線
		// シーケンシャル番号
		RAU_ui_RAUDPA_seq2++;
		if( RAU_ui_RAUDPA_seq2 > 999 ){
			RAU_ui_RAUDPA_seq2 = 1;
		}

		// ＴＣＰブロック番号
		if( RAU_uiTcpBlockNo2 == 65535 ){
			RAU_uiTcpBlockNo2 = 1;
		} else {
			RAU_uiTcpBlockNo2++;
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_Init()
 *[]----------------------------------------------------------------------[]*
 *|	初期化関数
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_Init(void)
{
	RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// 送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)
	RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンス０(通常状態)
	RAU_ucTcpBlockSts = 0;
	RAU_uiTcpBlockNo = 1;
	RAU_ucTcpBlockSts2 = 0;
	RAU_uiTcpBlockNo2 = 1;
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 	RAU_f_RcvSeqCnt_rq = 0;
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]

	RAUhost_SendRequestFlag = 0;			// 送信要求データありフラグ
	RAUhost_Error01 = 0;					// エラーコード０１発生状況
	memset(&RAUhost_SendNtDataInfo, 0x00, sizeof(T_SEND_NT_DATA));		// 送信対象のテーブルバッファから取得したNTデータの情報
	memset(&RAUhost_SendNtThroughInfo, 0x00, sizeof(T_SEND_NT_DATA));	// 送信スルーデータテーブルバッファから取得したNTデータの情報
	pRAUhost_SendIdInfo = NULL;				// 送信対象のテーブルバッファ情報の参照
	pRAUhost_SendThroughInfo = NULL;		// 送信スルーデータテーブルの参照をやめる。

	memset(&RemoteCheck_Buffer100_101, 0, 64);
	memset(&RemoteCheck_Buffer116_117, 0, 64);
	RauCT_SndReqID = 0;
	Rau_SedEnable = 0;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_SetError()
 *[]----------------------------------------------------------------------[]*
 *|	エラーを設定する
 *[]----------------------------------------------------------------------[]*
 *|	patam	ucErrCode	エラーコード
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_SetError(uchar ucErrCode)
{
	switch( ucErrCode ){
	case ERR_RAU_HOST_COMMUNICATION:							// エラーコード０１(上り回線通信エラー)
		RAU_err_chk(ucErrCode, 0x01,0, 0, NULL );				// エラー(発生)をＩＢＷに通知
		RAUhost_Error01 = 1;									// エラー０１発生フラグセット

		RAU_uc_retrycnt_h = 0;										// 上り回線の送信リトライカウンタクリア
		RAU_Tm_Ackwait.tm = 0;										// 上り回線のＡＣＫ受信待ちタイマクリア
		RAU_uc_retrycnt_reconnect_h = 0;							// 上り回線の送信リトライカウンタクリア
		RAU_Tm_Ackwait_reconnect.tm = 0;							// 上り回線の再接続待ちタイマクリア

		RAUque_DeleteQue();										// テーブルから送信完了したデータのみ消す。
		pRAUhost_SendIdInfo = NULL;								// テーブルの参照をやめる。

		RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// 送信シーケンス０を(ＨＯＳＴへのテーブル送信待機状態)に戻す。

		RAU_ui_data_length_h = 0;									// 受信データ長クリア
		RAU_uc_mode_h = S_H_IDLE;									// ＡＣＫ待ちを解除してアイドルに移行
		break;
	case ERR_RAU_TCP_CONNECT:									// エラーコード９１(上り回線ＴＣＰコネクションタイムアウト)
		if( RAUhost_Error01 == 0 ){								// エラーコード０１発生中はエラーとしないので…
			RAU_err_chk(ucErrCode, 0x02, 0, 0, NULL );			// エラー(発生解除同時)をＩＢＷに通知
		}
		break;
	default:
		RAU_err_chk(ucErrCode, 0x02, 0, 0, NULL);				// エラー(発生解除同時)をＩＢＷに通知
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_SetSendRequeat()
 *[]----------------------------------------------------------------------[]*
 *|	送信要求を設定する
 *[]----------------------------------------------------------------------[]*
 *|	patam	pSendRequest	送信要求データ
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_SetSendRequeat(RAU_SEND_REQUEST *pSendRequest) {

	RAUhost_SendRequestFlag = 1;
	memcpy(&RAUhost_SendRequest, pSendRequest, sizeof(RAU_SEND_REQUEST));
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_ClearSendRequeat()
 *[]----------------------------------------------------------------------[]*
 *|	送信要求を解除する
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_ClearSendRequeat(void) {
	RAUhost_SendRequestFlag = 0;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_CreateResponseData()
 *[]----------------------------------------------------------------------[]*
 *|	応答Dopaパケットを生成する
 *[]----------------------------------------------------------------------[]*
 *|	patam	pBuff		電文を生成するバッファ
 *|	patam	ucResult	応答結果
 *|	patam	uiExtra		予備項目
 *|	patam	uiSeqNum	シーケンス№
 *|	patam	uiLen		送信サイズ
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_CreateResponseData(uchar *pBuff, uchar ucResult, ushort uiExtra, ulong uiSeqNum, ushort *uiLen)
{
	uchar		ucCrc[2];
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	c_int32toarray(pBuff, 29, 4);									// データ長セット
	memcpy(&pBuff[4], uc_dpa_rem_str, 12);							// "AMANO_REMOTE"セット
	c_int32toarray(&pBuff[16], uiSeqNum, 3);						// シーケンスNo.セット
	pBuff[19] = 0;													// データ部コード(未使用０固定)
	pBuff[20] = 0;													// 電文識別コード(0:中断要求なし, 1:中断要求あり)
	u.us = 1;
	pBuff[21] = u.uc[0];											// ＴＣＰブロックナンバー
	pBuff[22] = u.uc[1];
	pBuff[23] = 1;													// ＴＣＰブロックステータス
	u.us = uiExtra;
	pBuff[24] = u.uc[0];											// 予備
	pBuff[25] = u.uc[1];											// 予備
	pBuff[26] = ucResult;											// 結果

	RAU_CrcCcitt(27, 0, pBuff, NULL, ucCrc, 1 );					// ＣＲＣ算出
	pBuff[27] = ucCrc[0];
	pBuff[28] = ucCrc[1];

	*uiLen = 29;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_IdentifyDataType()
 *[]----------------------------------------------------------------------[]*
 *|	ＩＤによってパケットの生成方法を切り換える。
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	short	0:パケット生成OK -1:パケット生成NG
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short RAUhost_IdentifyDataType(int port){

	short	ret = 0;

	if( port == UPLINE ){
	switch(pRAUhost_SendIdInfo->pbuff_info->uc_DataId){
	case 41:														// Ｔ合計データ
	case 53:														// フォーマットRevNo.10 Ｔ／ＧＴ合計データ
		RAUhost_CreateDopaPacketTsum();
		break;
	case 126:														// 金銭管理データ
		ret = RAUhost_CreateDopaPacketCash();
		break;
	case 65:														// センター用端末情報データ
		ret = RAUhost_CreateDopaPacketCTI();
		break;
	case 20:														// 入庫データ
// MH810100(S) m.saito 2020/05/15 車番チケットレス(#4169 精算データの前に0件データを送信してしまう不具合)
	case 22:														// 精算データ（事前）
// MH810100(E) m.saito 2020/05/15 車番チケットレス(#4169 精算データの前に0件データを送信してしまう不具合)
	case 23:														// 精算データ（出口）
	case 120:														// エラーデータ
	case 121:														// アラームデータ
	case 122:														// モニタデータ
	case 123:														// 操作モニタデータ
	case 131:														// コイン金庫集計合計データ
	case 133:														// 紙幣金庫集計合計データ
	case 236:														// 駐車台数データ
	case 135:														// 釣銭管理データ
	case 125:														// 遠隔監視データ
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	case 61:														// 長期駐車状態データ
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
		ret = RAUhost_CreateTableDataPacket();
		break;
	default:
		DPA2Port_CreateDopaPacket( port );
		break;
	}
	}else{
		DPA2Port_CreateDopaPacket( port );
	}
	return ret;
}

//==============================================================================
//	ホストへ送信するデータパケットの作成
//
//	@argument	port			対象ポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		なし
//
//	@attention	ＤｏＰａの２ポート対応専用ルーチンです。
//
//	@note		送信するパケットデータに付加するシーケンシャル番号と
//				ＴＣＰブロック番号の加算を行います。
//
//	@see		RAUhost_CreateDopaPacket
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_CreateDopaPacket(int port)
{
	ushort				uiReadLen;
	ushort				uiRemain;
	uchar				ucReadEnd;
	T_SEND_DATA_ID_INFO	*pSendDataInfo;
	T_SEND_NT_DATA		*pSendNtData;
	uchar				*pTxData;
	ushort				txdlength;
	ushort				PacketDataSize;		// パケットのデータ部サイズ(ＤｏＰａ:１４２０バイト, ＸＰｏｒｔ:５００バイト)
	ulong				Start_1msLifeTime;	// 電文受信時点の1msライフタイマー
	ulong				Past_1msTime;		// 受信～現在までの経過時間（x1ms）
	union {
		ushort	us;
		uchar	uc[2];
	} u;

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// モデムあり:ＤｏＰａ
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// モデムなし:Ethernet
	}

	if( port == UPLINE ){	// 上り回線
		pSendDataInfo = pRAUhost_SendIdInfo;
		pSendNtData = &RAUhost_SendNtDataInfo;
		pTxData = RAU_huc_txdata_h;
	}
	else if(port == CREDIT){	// クレジット回線
		pSendDataInfo = &Credit_SendIdInfo;
		pSendNtData = &Credit_SendNtDataInfo;
		pTxData = RAU_huc_txdata_CRE;
	} else {				// 下り回線
		pSendDataInfo = pRAUhost_SendThroughInfo;
		pSendNtData = &RAUhost_SendNtThroughInfo;
		pTxData = RAU_huc_txdata_h2;
	}

	ucReadEnd = 0;
	txdlength = 0;
	if( pSendDataInfo->send_data_count > 0 ){	// 送信データあり
		while( ucReadEnd == 0 ){
			// ＮＴデータの情報を取得する｡
			if( DPA2Port_GetNtData(pSendDataInfo, pSendNtData) == FALSE ){	// ＮＴデータ取得失敗
				if(port == CREDIT){
					/* クレジットでは0件データ送信しない */
					txdlength = 0;
					ucReadEnd = 2;
					break;
				}
				RAUdata_Create0Data( (RAU_NTDATA_HEADER*)&pTxData[DOPA_HEAD_SIZE], pSendDataInfo->pbuff_info->uc_DataId );	// ０件データの作成
				txdlength = 10;
				ucReadEnd = 2;
				break;
			}

			// ＮＴデータの残量を算出
			if( pSendNtData->nt_data_end > pSendNtData->nt_data_next ){
				uiRemain = pSendNtData->nt_data_end - pSendNtData->nt_data_next;
			} else {
				uiRemain = (&(pSendDataInfo->pbuff_info->pc_AreaTop[pSendDataInfo->pbuff_info->ul_AreaSize]) - pSendNtData->nt_data_next)
							+ (pSendNtData->nt_data_end - pSendDataInfo->pbuff_info->pc_AreaTop);
			}

			// ＮＴデータから読み出す量を決定
			if( (PacketDataSize - txdlength) < uiRemain ){
				uiReadLen = (PacketDataSize - txdlength);
			} else {
				uiReadLen = uiRemain;
			}

			// ＤｏＰａパケットにＮＴデータを充填
			RAUdata_ReadNtData(&pTxData[DOPA_HEAD_SIZE + txdlength], uiReadLen, pSendDataInfo->pbuff_info, pSendNtData);
			/* 遠隔NT-NET送信データ，下り回線,電文データ種別(ID4)が229 で               */
			/* Main基板から受信した時計データ電文を外部へ送信するところであると判断する */
			if(	( port == DOWNLINE ) &&								// 下り回線
				( pTxData[DOPA_HEAD_SIZE + txdlength +8] == 229) )	// NT-NET電文IDが229
			{
				/* ここまでで時計データ電文のリモートパケットと確定                                                 */
				/* NT-NET時計データ電文は従来の形式(33byte)とe-timing同期に対応した形式(41byte)の2通り存在する。    */
				/* 形式の違いを0カット前のデータサイズで判別しe-timing同期に対応した形式の場合には特殊処理を行う。  */
				u.uc[0] = pTxData[DOPA_HEAD_SIZE + txdlength +2];
				u.uc[1] = pTxData[DOPA_HEAD_SIZE + txdlength +3];								// 0カット前のデータサイズget
				if( u.us >= 41 ){																// 新時計データ電文（補正値付き）であれば
					memcpy( &Start_1msLifeTime, &pTxData[DOPA_HEAD_SIZE + txdlength +47], 4 );	// 受信時点のライフタイマー値get
					Past_1msTime = RAU_c_1mPastTimGet( Start_1msLifeTime );						// 受信時点からの経過時間get
					memcpy( &pTxData[DOPA_HEAD_SIZE + txdlength +43], &Past_1msTime, 4 );		// 受信時点からの経過時間set
				}
			}
			txdlength += uiReadLen;	// 読み込み量を増加
			if( RAUhost_isDataReadEnd(pSendDataInfo->pbuff_info, pSendNtData) ){	// １ＮＴデータの最後まで書き込み終了
				pSendDataInfo->fill_up_data_count++;	// fillをインクリメント
			}

			// 書き込み終了チェック
			if( pSendDataInfo->send_data_count <= (pSendDataInfo->send_complete_count
												 + pSendDataInfo->fill_up_data_count
												 + pSendDataInfo->crc_err_data_count) ){
				ucReadEnd = 2;	// 参照しているテーブルには、これ以上のデータがない｡
			} else if ( txdlength >= PacketDataSize ){	// パケットサイズいっぱいに詰め込んだ。
				ucReadEnd = 1;
			}
		}
	} else if(port == CREDIT){
		txdlength = 0;
		ucReadEnd = 2;
	} else {	// 送信データなし
		RAUdata_Create0Data( (RAU_NTDATA_HEADER*)&pTxData[DOPA_HEAD_SIZE], pSendDataInfo->pbuff_info->uc_DataId );	// ０件データの作成
		txdlength = 10;
		ucReadEnd = 2;
	}
	txdlength = DOPA_HEAD_SIZE + txdlength + 2;		// データ長修正｡

	if( port == UPLINE ){										// 上り回線
//		if( (uc_data_cancel_h == 1)								// 中断要求あり(非対応)
//			||(RAUque_IsLastID() == TRUE)&&(ucReadEnd == 2) ){	// 送信対象になっている最後のＩＤの全データの充填が終わった。
		if( ucReadEnd == 2 ){									// 参照しているテーブル(ＩＤ)の全データの充填が終わった。
			RAU_ucTcpBlockSts = 1;									// ＴＣＰブロックステータス１（＝終端）
		}
		RAU_ui_txdlength_h = txdlength;
		RAU_DpaSndHedSet(RAU_ucTcpBlockSts, port);					// ヘッダー設定
	}
	else if(port == CREDIT){
		if( ucReadEnd == 2 ){									// 全スルーデータの充填が終わった。
			Credit_TcpBlockSts = 1;									// ＴＣＰブロックステータス１（＝終端）
		}
		RAU_ui_txdlength_CRE = txdlength;
		RAU_DpaSndHedSet(Credit_TcpBlockSts, port);					// ヘッダー設定
	} else {													// 下り回線
		if( ucReadEnd == 2 ){									// 全スルーデータの充填が終わった。
			RAU_ucTcpBlockSts2 = 1;									// ＴＣＰブロックステータス１（＝終端）
		}
		RAU_ui_txdlength_h2 = txdlength;
		RAU_DpaSndHedSet(RAU_ucTcpBlockSts2, port);					// ヘッダー設定
	}

	RAU_CrcCcitt( txdlength - 2, 0, pTxData, NULL, &pTxData[txdlength - 2], 1 );	// ＣＲＣ算出
}

//==============================================================================
//	送信ＮＴデータの取得
//
//	@argument	*pSendDataInfo	送信対象テーブルの情報
//	@argument	*pSendNtData	送信データのＮＴ‐ＮＥＴデータ単位での情報
//
//	@return		TRUE			取得成功
//	@return		FALSE			取得失敗
//
//	@attention	ＤｏＰａの２ポート対応専用ルーチンです。
//
//	@note		ホストへ送信するＮＴデータをテーブル情報より取得します。
//
//	@see		RAUhost_GetNtData
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
BOOL DPA2Port_GetNtData(T_SEND_DATA_ID_INFO *pSendDataInfo, T_SEND_NT_DATA *pSendNtData)
{
	if( pSendNtData->nt_data_start == NULL ){	// ＮＵＬＬだったらデータを取得する｡
		while( pSendDataInfo->send_data_count > pSendDataInfo->crc_err_data_count ){	// ＣＲＣエラーのＮＴデータは送信対象とはしない(読み飛ばす)
			if( RAUdata_GetNtData(pSendDataInfo->pbuff_info, pSendNtData) == TRUE ){	// ＮＴデータの情報を取得 →pSendNtData
				return TRUE;	// 取得したデータはＣＲＣ正常
			} else {
				pSendDataInfo->crc_err_data_count++;	// データがＣＲＣエラーだったので次のデータを取得しに行く
			}
		}
		return FALSE;	// 全部ＣＲＣエラーだったら、仕方がないので０件データを送る。
	} else {
		if( RAUhost_isDataReadEnd(pSendDataInfo->pbuff_info, pSendNtData) ){	// 取得済みのＮＴデータを最後まで読んでいる｡
			while( pSendDataInfo->send_data_count > (pSendDataInfo->send_complete_count
				+ pSendDataInfo->fill_up_data_count + pSendDataInfo->crc_err_data_count) ){
				if( RAUdata_GetNtData(pSendDataInfo->pbuff_info, pSendNtData) == TRUE ){
					return TRUE;	// 取得したデータはＣＲＣ正常
				} else {
					pSendDataInfo->crc_err_data_count++;	// データがＣＲＣエラーだったので次のデータを取得しに行く
				}
			}
		} else {
			return TRUE;
		}
	}
	return FALSE;	// 途中から全部ＣＲＣエラー
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_CreateDopaPacketTsum(void) {

	ushort		uiReadLen;
	ushort		uiRemain;
	uchar		ucReadEnd;
	ushort		PacketDataSize;		// パケットのデータ部サイズ(ＤｏＰａ:１４２０バイト, ＸＰｏｒｔ:５００バイト)

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// モデムあり:ＤｏＰａ
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// モデムなし:Ethernet
	}

	ucReadEnd = 0;
	RAU_ui_txdlength_h = 0;

	if (pRAUhost_SendIdInfo->send_data_count > 0) {

		// データあり
		while (ucReadEnd == 0) {

			// ＮＴデータの情報を取得する｡
			if (RAUhost_GetNtDataTsum() == FALSE) {
				// データ0件
				RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
				RAU_ui_txdlength_h = 10;
				ucReadEnd = 2;
				break;
			}

			// ＮＴデータの残量を算出
			uiRemain = RAUhost_SendNtDataInfo.nt_data_end - RAUhost_SendNtDataInfo.nt_data_next;


			// ＮＴデータから読み出す量を決定
			if ((PacketDataSize - RAU_ui_txdlength_h) < uiRemain) {
				uiReadLen = (PacketDataSize - RAU_ui_txdlength_h);
			} else {
				uiReadLen = uiRemain;
			}

			// ＤｏｐａパケットにＮＴデータを充填
			memcpy(&RAU_huc_txdata_h[DOPA_HEAD_SIZE + RAU_ui_txdlength_h], RAUhost_SendNtDataInfo.nt_data_next, uiReadLen );
			

			// 次回書き込み位置をコピーした長さ分進める｡
			RAUhost_SendNtDataInfo.nt_data_next += uiReadLen;

			// 読み込み量を増加
			RAU_ui_txdlength_h += uiReadLen;

			if (RAUhost_SendNtDataInfo.nt_data_end <= RAUhost_SendNtDataInfo.nt_data_next) {
				// １ＮＴデータの最後まで書き込んだので、fillをインクリメント
				pRAUhost_SendIdInfo->fill_up_data_count++;
			}

			// 書き込み終了チェック
			if (pRAUhost_SendIdInfo->send_data_count <= (pRAUhost_SendIdInfo->send_complete_count
														+ pRAUhost_SendIdInfo->fill_up_data_count
														+ pRAUhost_SendIdInfo->crc_err_data_count)) {
				// これ以上データがなければ、送信｡
				ucReadEnd = 2;
			} else if (RAU_ui_txdlength_h >= PacketDataSize) {
				// バッファを満たしていたら送信｡
				ucReadEnd = 1;
			}
			break;
		}
	} else {
		// データ0件
		RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
		RAU_ui_txdlength_h = 10;
		ucReadEnd = 2;
	}

	// データ長修正｡
	RAU_ui_txdlength_h = DOPA_HEAD_SIZE + RAU_ui_txdlength_h + 2;

	// 中断要求あり
	if( ucReadEnd == 2 ){	// 参照しているテーブル(ＩＤは複数)の全データの充填が終わった。
		RAU_ucTcpBlockSts = 1;	// ＴＣＰブロックステータス１（＝終端）
	}

	// ヘッダー設定
	RAU_DpaSndHedSet(RAU_ucTcpBlockSts, UPLINE);

	// ＣＲＣ算出
	RAU_CrcCcitt(RAU_ui_txdlength_h - 2, 0, RAU_huc_txdata_h, NULL, &RAU_huc_txdata_h[RAU_ui_txdlength_h - 2], 1 );


}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL RAUhost_GetNtDataTsum(void) {

	BOOL ret;

	ret = FALSE;

	if (RAUhost_SendNtDataInfo.nt_data_start == NULL) {				// NULLだったらデータを取得する｡

		while (pRAUhost_SendIdInfo->send_data_count > pRAUhost_SendIdInfo->crc_err_data_count ) {

			if (RAUdata_GetTsumData(&RAUhost_SendNtDataInfo, RAU_special_table_send_buffer) == TRUE) {
				ret = TRUE;
				break;
			} else {
				// データがCRCエラーだったので次のデータを取得しに行く
				pRAUhost_SendIdInfo->crc_err_data_count++;
			}

		}
		if (pRAUhost_SendIdInfo->send_data_count <= pRAUhost_SendIdInfo->crc_err_data_count) {
			// 全部CRCエラーだったら、仕方がないので0件データを送る。
			ret = FALSE;
		}
	} else {
		if (RAUhost_SendNtDataInfo.nt_data_end <= RAUhost_SendNtDataInfo.nt_data_next) {			// データの最後まで読んでいる｡

			while (pRAUhost_SendIdInfo->send_data_count > (pRAUhost_SendIdInfo->send_complete_count
															+ pRAUhost_SendIdInfo->fill_up_data_count
															+ pRAUhost_SendIdInfo->crc_err_data_count)) {

				if (RAUdata_GetTsumData(&RAUhost_SendNtDataInfo, RAU_special_table_send_buffer) == TRUE) {
					ret = TRUE;
					break;
				} else {
					// データがCRCエラーだったので次のデータを取得しに行く
					pRAUhost_SendIdInfo->crc_err_data_count++;
				}
			}
		} else {
			ret = TRUE;
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	short	0:パケット正常作成 -1:パケット作成なし
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short RAUhost_CreateDopaPacketCash(void) {

	ushort		uiReadLen;
	ushort		uiRemain;
	uchar		ucReadEnd;
	ushort		PacketDataSize;		// パケットのデータ部サイズ(ＤｏＰａ:１４２０バイト, ＸＰｏｒｔ:５００バイト)
	ushort		logCount;
	uchar		blockCount;
	BOOL		startFlg = TRUE;
	short		ret;

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// モデムあり:ＤｏＰａ
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// モデムなし:Ethernet
	}

	ucReadEnd = 0;
	RAU_ui_txdlength_h = 0;

	if (pRAUhost_SendIdInfo->send_data_count > 0) {

		// データあり
		while (ucReadEnd == 0) {
			if(RAU_NtLogDataIndex == 0) {
				logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
				// ログデータが無ければ0件データを送信する
				if(logCount == 0) {
					// データ0件
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					break;
				}
				Ope_Log_TargetDataVoidRead(eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
				startFlg = FALSE;

// 精算に関連するとき、前回と同じなら送信しない or 要求があったときは０データを送信する
				ret = NTNET_Edit_Data126((TURI_KAN *)RAU_LogData, (DATA_KIND_126 *)RAU_NtLogData);
				if(ret == 0) {
					// データ0件
					if (pRAUhost_SendIdInfo->send_req == 2) {
						pRAUhost_SendIdInfo->send_data_count = 0;
						return -1;		// 自発送信時は送信しない
					}
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					// 金銭管理データの0件データが無限送信される不具合対策
					pRAUhost_SendIdInfo->fill_up_data_count++;	// 充填済みとする
					break;
				}

				RAU_NtLogDataIndex = 0;
				RAU_NtLogDataSize = RAUhost_ConvertNtBlockData(RAU_NtLogData, sizeof(DATA_KIND_126), RAU_NtBlockLogData, &blockCount);
			
			}

			// ＮＴデータの残量を算出
			uiRemain = RAU_NtLogDataSize - RAU_NtLogDataIndex;


			// ＮＴデータから読み出す量を決定
			if ((PacketDataSize - RAU_ui_txdlength_h) < uiRemain) {
				uiReadLen = (PacketDataSize - RAU_ui_txdlength_h);
			} else {
				uiReadLen = uiRemain;
			}

			// ＤｏｐａパケットにＮＴデータを充填
			memcpy(&RAU_huc_txdata_h[DOPA_HEAD_SIZE + RAU_ui_txdlength_h],
							&RAU_NtBlockLogData[RAU_NtLogDataIndex], uiReadLen );


			// 次回書き込み位置をコピーした長さ分進める｡
			RAU_NtLogDataIndex += uiReadLen;

			// 読み込み量を増加
			RAU_ui_txdlength_h += uiReadLen;

			if(RAU_NtLogDataSize <= RAU_NtLogDataIndex) {
				// １ＮＴデータの最後まで書き込んだので、fillをインクリメント
				pRAUhost_SendIdInfo->fill_up_data_count++;
				RAU_NtLogDataIndex = 0;
			}				

			// 書き込み終了チェック
			if (pRAUhost_SendIdInfo->send_data_count <= (pRAUhost_SendIdInfo->send_complete_count
														+ pRAUhost_SendIdInfo->fill_up_data_count
														+ pRAUhost_SendIdInfo->crc_err_data_count)) {
				// これ以上データがなければ、送信｡
				ucReadEnd = 2;
			} else if (RAU_ui_txdlength_h >= PacketDataSize) {
				// バッファを満たしていたら送信｡
				ucReadEnd = 1;
			}

		}
	} else {
		if(pRAUhost_SendIdInfo->send_req == 2) {
			pRAUhost_SendIdInfo->send_data_count = 0;
			return -1;
		}
		// データ0件
		RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
		RAU_ui_txdlength_h = 10;
		ucReadEnd = 2;
	}

	// データ長修正｡
	RAU_ui_txdlength_h = DOPA_HEAD_SIZE + RAU_ui_txdlength_h + 2;

	// 中断要求あり
	if( ucReadEnd == 2 ){	// 参照しているテーブル(ＩＤ)の全データの充填が終わった。
		RAU_ucTcpBlockSts = 1;	// ＴＣＰブロックステータス１（＝終端）
	}

	// ヘッダー設定
	RAU_DpaSndHedSet(RAU_ucTcpBlockSts, UPLINE);

	// ＣＲＣ算出
	RAU_CrcCcitt(RAU_ui_txdlength_h - 2, 0, RAU_huc_txdata_h, NULL, &RAU_huc_txdata_h[RAU_ui_txdlength_h - 2], 1 );

	return 0;
}

//==============================================================================
//	送信シーケンスフラグのセット
//
//	@argument	ucSndSeqFlag	送信シーケンスフラグ
//
//	@return		なし
//
//	@note		上り回線の使用状況により状態を変化させます。
//
//	@see		RAUhost_SetReceiveSeqFlag
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void RAUhost_SetSndSeqFlag(uchar ucSndSeqFlag)
{
	RAU_ucSendSeqFlag = ucSndSeqFlag;
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 	if(ucSndSeqFlag == RAU_RCV_SEQ_FLG_NORMAL){
	if ( ucSndSeqFlag == RAU_SND_SEQ_FLG_NORMAL ) {
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
		RAU_f_SndSeqCnt_rq = 0;
	}
	else{
		RAU_f_SndSeqCnt_rq = 1;
	}
	RAU_Tm_SndSeqCnt = 0;
}

//==============================================================================
//	受信シーケンスフラグのセット
//
//	@argument	ucRcvSeqFlag	受信シーケンスフラグ
//
//	@return		なし
//
//	@note		下り回線の使用状況により状態を変化させます。
//
//	@see		RAUhost_SetReceiveSeqFlag
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void RAUhost_SetRcvSeqFlag(uchar ucRcvSeqFlag)
{
	RAU_ucReceiveSeqFlag = ucRcvSeqFlag;
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
	switch ( ucRcvSeqFlag ) {
	case RAU_RCV_SEQ_FLG_WAIT:			// １：ＨＯＳＴからの要求データ受信完了待ち状態(ＩＢＷからのスルーデータ受信禁止)
		RAU_Tm_RcvSeqCnt = 0;
		RAU_f_RcvSeqCnt_rq = 1;
		break;
	case RAU_RCV_SEQ_FLG_THROUGH_RCV:	// ２：ＨＯＳＴからの要求データ受信完了状態
		break;
	case RAU_RCV_SEQ_FLG_THROUGH_SEND:	// ３：ＨＯＳＴへのスルーデータ送信開始状態(ＩＢＷからのスルーデータ受信禁止)
	// NOTE: メインから応答データ受信後、HOSTへデータ送信のリトライ中にタイムアウトが起きると受信シーケンスフラグが「0」にリセットされ
	//       テーブルデータの送信が開始されフェーズがずれることがある。現状、メインから受信したスルーデータを最終パケットまで送信した時点で
	//       タイマクリアしているが、受信シーケンスキャンセルタイマはメインからスルーデータを受信した時点でクリアすべきである。
	case RAU_RCV_SEQ_FLG_NORMAL:		// ０：通常状態(ＩＢＷからの要求データ受信許可)
	default:
		RAU_f_RcvSeqCnt_rq = 0;
		break;
	}
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
}

//==============================================================================
//	送信シーケンスフラグの取得
//
//	@argument	なし
//
//	@return		フラグの値
//
//	@note		現在の上り回線の状態を取得します。
//
//	@see		RAUhost_GetReceiveSeqFlag
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
uchar RAUhost_GetSndSeqFlag(void)
{
	return RAU_ucSendSeqFlag;
}

//==============================================================================
//	シーケンスフラグの取得
//
//	@argument	なし
//
//	@return		フラグの値
//
//	@note		現在の下り回線の状態を取得します。
//
//	@see		RAUhost_GetReceiveSeqFlag
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
uchar RAUhost_GetRcvSeqFlag(void)
{
	return RAU_ucReceiveSeqFlag;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_isDataReadEnd()
 *[]----------------------------------------------------------------------[]*
 *|	現在バッファに充填中のデータが最後まで充填し終わったか判定する。
 *[]----------------------------------------------------------------------[]*
 *|	patam	pBuffInfo		バッファ情報
 *|	patam	pSendNtDataInf	送信データ情報
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAUhost_isDataReadEnd(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf) {

	BOOL ret;

	ret = FALSE;

	// ＮＴデータの残量を算出
	if (pSendNtDataInf->nt_data_end > pSendNtDataInf->nt_data_start) {
		// 	start < end	:	end <= next
		if (pSendNtDataInf->nt_data_end <= pSendNtDataInf->nt_data_next) {
			ret = TRUE;
		}
	} else {
		// 	start > end	:	end <= next && next < start
		if ((pSendNtDataInf->nt_data_end <= pSendNtDataInf->nt_data_next) && (pSendNtDataInf->nt_data_next < pSendNtDataInf->nt_data_start)) {
			ret = TRUE;
		}
	}

	return ret;
}
//==============================================================================
// TCP Connection 要求失敗時のクリア処理
//
// TCP Connection 3回ﾘﾄﾗｲ失敗時.
// (関数.RAUhost_SetError( ) : case RAU_ERR_HOST_COMMUNICATION:
// と同等の処理を行う.
//
// CD が OFF された時に TCP CONNECTION中の場合にコールされる.
//==============================================================================
void RAUhost_TcpConnReq_Clear ( void )
{
	if ( RauCtrl.TCPcnct_req == 1 && pRAUhost_SendIdInfo != NULL ){
		RAU_uc_retrycnt_h = 0;										// 上り回線の送信リトライカウンタクリア
		RAU_Tm_Ackwait.tm = 0;										// 上り回線のＡＣＫ受信待ちタイマクリア
		RAU_uc_retrycnt_reconnect_h = 0;							// 上り回線の送信リトライカウンタクリア
		RAU_Tm_Ackwait_reconnect.tm = 0;							// 上り回線の再接続待ちタイマクリア

		RAUque_DeleteQue();										// テーブルから送信完了したデータのみ消す。
		pRAUhost_SendIdInfo = NULL;								// テーブルの参照をやめる。

		RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// 送信シーケンス０を(ＨＯＳＴへのテーブル送信待機状態)に戻す。

		RAU_ui_data_length_h = 0;									// 受信データ長クリア
		RAU_uc_mode_h = S_H_IDLE;									// ＡＣＫ待ちを解除してアイドルに移行
	}
}
/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_DwonLine_common_timeout()
 *[]----------------------------------------------------------------------[]*
 *|	下り回線通信監視タイマーがタイムアウトした時の処理
 *[]----------------------------------------------------------------------[]*
 *|	patam	none
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	A.Iiizumi
 *| Date	:	2010-07-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_DwonLine_common_timeout(void)
{
	if(RAUhost_GetRcvSeqFlag()!=RAU_RCV_SEQ_FLG_NORMAL ){
// 受信シーケンス０以外（途中のパケットまでを受信中）にFOMAの電波が切れタイムアウトした時
// 受信シーケンスを０に戻し、アイドル状態に戻す。
// 受信シーケンスを０に戻さないと次にHOSTからスルーデータを受信した時エラー（受信ブロック番号異常 コード81）となるため
		RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンス０(通常状態)
		RAUdata_TableClear(0);// スルーデータの削除
		RAU_uiLastTcpBlockNum = 0;
		RAU_uiDopaReceiveLen = 0;
	}
}
/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_CreateTableDataPacket()
 *[]----------------------------------------------------------------------[]*
 *|	ホストへ送信するテーブルデータパケットの作成
 *[]----------------------------------------------------------------------[]*
 *|	patam	none
 *[]----------------------------------------------------------------------[]*
 *|	return	short	0:パケット正常作成 -1:パケット作成なし
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
short RAUhost_CreateTableDataPacket(void)
{
	ushort		uiReadLen;
	ushort		uiRemain;
	uchar		ucReadEnd;
	ushort		PacketDataSize;		// パケットのデータ部サイズ(ＤｏＰａ:１４２０バイト, ＸＰｏｒｔ:５００バイト)
	ushort		logCount;
	ushort		readCount = 0;
	ushort		unreadCount = 0;
	short		logId = 0;
	uchar		dataId;
	ushort		size;
	uchar		blockCount;
	BOOL		startFlg = TRUE;
	DATA_KIND_120 *dataKind120;
	DATA_KIND_121 *dataKind121;
	DATA_KIND_122 *dataKind122;
	DATA_KIND_123 *dataKind123;
	DATA_KIND_63 *dataKind63;
	DATA_KIND_64 *dataKind64;
	Err_log		*errLog;
// MH321800(S) T.Nagai ICクレジット対応
	Receipt_data	*p_RcptDat;
// MH321800(E) T.Nagai ICクレジット対応

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// モデムあり:ＤｏＰａ
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// モデムなし:Ethernet
	}

	ucReadEnd = 0;
	RAU_ui_txdlength_h = 0;

	if (pRAUhost_SendIdInfo->send_data_count > 0) {

		// データあり
		while (ucReadEnd == 0) {
			if(RAU_NtLogDataIndex == 0) {
				logId = RAUhost_ConvertDataIdToLogData(pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
				if(logId == -1){
					// データ0件
					if(pRAUhost_SendIdInfo->send_req == 2) {
						pRAUhost_SendIdInfo->send_data_count = 0;
						return -1;
					}
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					break;
				}
				logCount = Ope_Log_UnreadCountGet(logId, eLOG_TARGET_REMOTE);
				// ログデータが無ければ0件データを送信する
				if(logCount == 0){
					// データ0件
					if(pRAUhost_SendIdInfo->send_req == 2) {
						pRAUhost_SendIdInfo->send_data_count = 0;
						return -1;
					}
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					break;
				}
// MH810100(S)
				// 連続を考慮して一度taskchg
				taskchg(IDLETSKNO);
// MH810100(E)
				Ope_Log_TargetDataVoidRead(logId, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
				startFlg = FALSE;

				size = 0;
				switch(pRAUhost_SendIdInfo->pbuff_info->uc_DataId){
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//				case 20:											// 入庫データ
//					dataId = NTNET_Edit_isData20_54((enter_log *)RAU_LogData);
//					if(dataId == 20){
//						NTNET_Edit_Data20((enter_log *)RAU_LogData, (DATA_KIND_20 *)RAU_NtLogData);
//						size = sizeof(DATA_KIND_20);
//						readCount++;
//					}
//					else if(dataId == 54){
//						if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
////							size = NTNET_Edit_Data54_r10((enter_log *)RAU_LogData, (DATA_KIND_54_r10 *)RAU_NtLogData);
//							size = NTNET_Edit_Data54_r13((enter_log *)RAU_LogData, (DATA_KIND_54_r13 *)RAU_NtLogData);
//// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
//						}
//						else {
//							size = NTNET_Edit_Data54((enter_log *)RAU_LogData, (DATA_KIND_54 *)RAU_NtLogData);
//						}
//						readCount++;
//					}
//					break;
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
				case 23:											// 精算データ
					dataId = NTNET_Edit_isData22_56((Receipt_data *)RAU_LogData);
					if(dataId == 22){
// MH321800(S) T.Nagai ICクレジット対応
						p_RcptDat = (Receipt_data *)RAU_LogData;
						if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_receive == 0) {
// MH321800(E) T.Nagai ICクレジット対応
						NTNET_Edit_Data22((Receipt_data *)RAU_LogData, (DATA_KIND_22 *)RAU_NtLogData);
						size = sizeof(DATA_KIND_22);
						readCount++;
// MH321800(S) T.Nagai ICクレジット対応
						}
						else {
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
// MH321800(E) T.Nagai ICクレジット対応
					}
					else if(dataId == 56){
// MH321800(S) T.Nagai ICクレジット対応
						p_RcptDat = (Receipt_data *)RAU_LogData;
						if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_receive == 0) {
// MH321800(E) T.Nagai ICクレジット対応
// MH810105(S) MH364301 QRコード決済対応
//						if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
////							size = NTNET_Edit_Data56_r10((Receipt_data *)RAU_LogData, (DATA_KIND_56_r10 *)RAU_NtLogData);
//// MH321800(S) Y.Tanizaki ICクレジット対応
////							size = NTNET_Edit_Data56_r14((Receipt_data *)RAU_LogData, (DATA_KIND_56_r14 *)RAU_NtLogData);
//							size = NTNET_Edit_Data56_r17((Receipt_data *)RAU_LogData, (DATA_KIND_56_r17 *)RAU_NtLogData);
//// MH321800(E) Y.Tanizaki ICクレジット対応
//// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
//						}
//						else {
//							size = NTNET_Edit_Data56((Receipt_data *)RAU_LogData, (DATA_KIND_56 *)RAU_NtLogData);
//						}
						// SetAddPayData()で既に設定34-0121を参照しているので、
						// ここで判定する必要はない
						size = NTNET_Edit_Data56_rXX((Receipt_data *)RAU_LogData, (DATA_KIND_56_rXX *)RAU_NtLogData);
// MH810105(E) MH364301 QRコード決済対応
						readCount++;
// MH321800(S) T.Nagai ICクレジット対応
						}
						else {
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
// MH321800(E) T.Nagai ICクレジット対応
					}
					break;
				case 120:											// エラーデータ
					if(prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1) {
						NTNET_Edit_Data63((Err_log *)RAU_LogData, (DATA_KIND_63 *)RAU_NtLogData);
						dataKind63 = (DATA_KIND_63 *)RAU_NtLogData;

						errLog = (Err_log *)RAU_LogData;
						if( (dataKind63->Errsyu == ERRMDL_FLAP_CRB ) &&
							(errLog->Errinf == 0) &&
							((dataKind63->Errcod == ERR_FLAPLOCK_LOCKCLOSEFAIL) ||
							 (dataKind63->Errcod == ERR_FLAPLOCK_LOCKOPENFAIL) ||
							 (dataKind63->Errcod == ERR_FLAPLOCK_DOWNRETRYOVER) ||
							 (dataKind63->Errcod == ERR_FLAPLOCK_DOWNRETRY)) ){
							// err_chk2で上記のエラーログ登録時に18-0019④⑥=0ならエラー情報を付加
							// しないようにしているので、ここではエラー種別、エラーコード、エラー情報なし
							// の場合はエラーデータを送信しないようにする
							// ※エラー情報なしの為、車室を元にフラップ、ロックの切り分けができないので
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
// GM849100(S) 名鉄協商コールセンター対応（E96XXを遠隔で送信しない）
						else if(dataKind63->Errsyu == ERRMDL_ALARM) {		// E96XX
							// E96XXは送信しない
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
// GM849100(E) 名鉄協商コールセンター対応（E96XXを遠隔で送信しない）
						else {
							if(dataKind63->Errlev >= RauConfig.error_send_level){
								size = sizeof(DATA_KIND_63);
								readCount++;
							}
							else{
								unreadCount++;
								pRAUhost_SendIdInfo->send_data_count--;
							}
						}
						break;
					}
					NTNET_Edit_Data120((Err_log *)RAU_LogData, (DATA_KIND_120 *)RAU_NtLogData);
					dataKind120 = (DATA_KIND_120 *)RAU_NtLogData;

					// ﾌﾗｯﾌﾟ装置上昇下降ﾛｯｸで印字しない設定時は通信も印字も発生中表示も行わない(ﾛｸﾞ登録のみ)
					errLog = (Err_log *)RAU_LogData;
					if( (dataKind120->Errsyu == ERRMDL_FLAP_CRB ) &&
						(errLog->Errinf == 0) &&
						((dataKind120->Errcod == ERR_FLAPLOCK_LOCKCLOSEFAIL) ||
						 (dataKind120->Errcod == ERR_FLAPLOCK_LOCKOPENFAIL) ||
						 (dataKind120->Errcod == ERR_FLAPLOCK_DOWNRETRYOVER) ||
						 (dataKind120->Errcod == ERR_FLAPLOCK_DOWNRETRY)) ){
						// err_chk2で上記のエラーログ登録時に18-0019④⑥=0ならエラー情報を付加
						// しないようにしているので、ここではエラー種別、エラーコード、エラー情報なし
						// の場合はエラーデータを送信しないようにする
						// ※エラー情報なしの為、車室を元にフラップ、ロックの切り分けができないので
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}
					else {
						if(dataKind120->Errlev >= RauConfig.error_send_level){
							size = sizeof(DATA_KIND_120);
							readCount++;
						}
						else{
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
					}
					break;
				case 121:											// アラームデータ
					if(prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1) {
						NTNET_Edit_Data64((Arm_log *)RAU_LogData, (DATA_KIND_64 *)RAU_NtLogData);
						dataKind64 = (DATA_KIND_64 *)RAU_NtLogData;
						if(dataKind64->Armlev >= RauConfig.alarm_send_level){
							size = sizeof(DATA_KIND_64);
							readCount++;
						}
						else{
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
						break;
					}
					NTNET_Edit_Data121((Arm_log *)RAU_LogData, (DATA_KIND_121 *)RAU_NtLogData);
					dataKind121 = (DATA_KIND_121 *)RAU_NtLogData;
					if(dataKind121->Armlev >= RauConfig.alarm_send_level){
						size = sizeof(DATA_KIND_121);
						readCount++;
					}
					else{
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}
					break;
				case 122:											// モニタデータ
					NTNET_Edit_Data122((Mon_log *)RAU_LogData, (DATA_KIND_122 *)RAU_NtLogData);
					dataKind122 = (DATA_KIND_122 *)RAU_NtLogData;
// MH810103 GG119202 (s) #5320 モニタコードの「R0252」、「R0253」が通信データで送信されてしまう					
//					if(dataKind122->Monlev >= RauConfig.monitor_send_level){
					if( 0 == chk_mon_send_ntnet(((Mon_log*)RAU_LogData)->MonKind,((Mon_log*)RAU_LogData)->MonCode)){
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}else if(dataKind122->Monlev >= RauConfig.monitor_send_level){
// MH810103 GG119202 (e) #5320 モニタコードの「R0252」、「R0253」が通信データで送信されてしまう					
						size = sizeof(DATA_KIND_122);
						readCount++;
					}
					else{
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}
					break;
				case 123:											// 操作モニタデータ
					NTNET_Edit_Data123((Ope_log *)RAU_LogData, (DATA_KIND_123 *)RAU_NtLogData);
					dataKind123 = (DATA_KIND_123 *)RAU_NtLogData;
// MH810103 GG119202 (s) #5320 モニタコードの「R0252」、「R0253」が通信データで送信されてしまう					
//					if(dataKind123->OpeMonlev >= RauConfig.opemonitor_send_level){
					if( 0 == chk_opemon_send_ntnet(((Ope_log*)RAU_LogData)->OpeKind,((Ope_log*)RAU_LogData)->OpeCode)){
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}else if(dataKind123->OpeMonlev >= RauConfig.opemonitor_send_level){
// MH810103 GG119202 (e) #5320 モニタコードの「R0252」、「R0253」が通信データで送信されてしまう					
						size = sizeof(DATA_KIND_123);
						readCount++;
					}
					else{
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}
					break;
				case 131:											// コイン金庫集計合計データ
					NTNET_Edit_Data131((COIN_SYU *)RAU_LogData, (DATA_KIND_130 *)RAU_NtLogData);
					size = sizeof(DATA_KIND_130);
					readCount++;
					break;
				case 133:											// 紙幣金庫集計合計データ
					NTNET_Edit_Data133((NOTE_SYU *)RAU_LogData, (DATA_KIND_132 *)RAU_NtLogData);
					size = sizeof(DATA_KIND_132);
					readCount++;
					break;
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//				case 236:											// 駐車台数データ
//					dataId = NTNET_Edit_isData236_58((ParkCar_log *)RAU_LogData);
//					if(dataId == 236){
//						NTNET_Edit_Data236((ParkCar_log *)RAU_LogData, (DATA_KIND_236 *)RAU_NtLogData);
//						size = sizeof(DATA_KIND_236);
//						readCount++;
//					}
//					else if(dataId == 58){
//						if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//							size = NTNET_Edit_Data58_r10((ParkCar_log *)RAU_LogData, (DATA_KIND_58_r10 *)RAU_NtLogData);
//						}
//						else {
//							size = NTNET_Edit_Data58((ParkCar_log *)RAU_LogData, (DATA_KIND_58 *)RAU_NtLogData);
//						}
//						readCount++;
//					}
//					break;
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
				case 135:											// 釣銭管理集計データ
					NTNET_Edit_Data135((TURI_KAN *)RAU_LogData, (DATA_KIND_135 *)RAU_NtLogData);
					size = sizeof(DATA_KIND_135);
					readCount++;
					break;
				case 125:
					NTNET_Edit_Data125_DL((Rmon_log *)RAU_LogData, (DATA_KIND_125_DL *)RAU_NtLogData);
					size = sizeof(DATA_KIND_125_DL);
					readCount++;
					break;
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//				case 61:											//長期駐車情報データ
//					NTNET_Edit_Data61((LongPark_log_Pweb *)RAU_LogData, (DATA_KIND_61_r10 *)RAU_NtLogData);
//					size = sizeof(DATA_KIND_61_r10);
//					readCount++;
//					break;
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
				default:
					// データ0件
					if(pRAUhost_SendIdInfo->send_req == 2) {
						pRAUhost_SendIdInfo->send_data_count = 0;
						return -1;
					}
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					readCount++;
					break;
				}
				if(size != 0){
					RAU_NtLogDataIndex = 0;
					RAU_NtLogDataSize = RAUhost_ConvertNtBlockData(RAU_NtLogData, size, RAU_NtBlockLogData, &blockCount);
				}
				else{
					if(logCount == unreadCount){
						pRAUhost_SendIdInfo->send_data_count = 0;
						if(pRAUhost_SendIdInfo->send_req == 2) {
							return -1;
						}
						// 全件送信レベル未満は0件データ送信
						RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
						RAU_ui_txdlength_h = 10;
						ucReadEnd = 2;
						break;
					}
					if((logCount - (readCount + unreadCount)) > 0){
						continue;
					}
					else{
						ucReadEnd = 2;
						break;
					}
				}
			}
			// ＮＴデータの残量を算出
			uiRemain = RAU_NtLogDataSize - RAU_NtLogDataIndex;

			// ＮＴデータから読み出す量を決定
			if ((PacketDataSize - RAU_ui_txdlength_h) < uiRemain) {
				Ope_Log_UndoTargetDataVoidRead(logId, eLOG_TARGET_REMOTE);	// 空読み位置を戻す
				ucReadEnd = 1;
				break;
			} else {
				uiReadLen = uiRemain;
			}

			// ＤｏｐａパケットにＮＴデータを充填
			memcpy(&RAU_huc_txdata_h[DOPA_HEAD_SIZE + RAU_ui_txdlength_h],
							&RAU_NtBlockLogData[RAU_NtLogDataIndex], uiReadLen );
			

			// 次回書き込み位置をコピーした長さ分進める｡
			RAU_NtLogDataIndex += uiReadLen;

			// 読み込み量を増加
			RAU_ui_txdlength_h += uiReadLen;

			if(RAU_NtLogDataSize <= RAU_NtLogDataIndex) {
				// １ＮＴデータの最後まで書き込んだので、fillをインクリメント
				pRAUhost_SendIdInfo->fill_up_data_count++;
				RAU_NtLogDataIndex = 0;
			}				
			
			// 書き込み終了チェック
			if (pRAUhost_SendIdInfo->send_data_count <= (pRAUhost_SendIdInfo->send_complete_count
														+ pRAUhost_SendIdInfo->fill_up_data_count
														+ pRAUhost_SendIdInfo->crc_err_data_count)) {
				// これ以上データがなければ、送信｡
				ucReadEnd = 2;
			} else if (RAU_ui_txdlength_h >= PacketDataSize) {
				// バッファを満たしていたら送信｡
				ucReadEnd = 1;
			}

		}
	} else {
		if(pRAUhost_SendIdInfo->send_req == 2) {
			pRAUhost_SendIdInfo->send_data_count = 0;
			return -1;
		}
		// データ0件
		RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
		RAU_ui_txdlength_h = 10;
		ucReadEnd = 2;
	}

	// データ長修正｡
	RAU_ui_txdlength_h = DOPA_HEAD_SIZE + RAU_ui_txdlength_h + 2;

	// 中断要求あり
	if( ucReadEnd == 2 ){	// 参照しているテーブル(ＩＤ)の全データの充填が終わった。
		RAU_ucTcpBlockSts = 1;	// ＴＣＰブロックステータス１（＝終端）
	}

	// ヘッダー設定
	RAU_DpaSndHedSet(RAU_ucTcpBlockSts, UPLINE);

	// ＣＲＣ算出
	RAU_CrcCcitt(RAU_ui_txdlength_h - 2, 0, RAU_huc_txdata_h, NULL, &RAU_huc_txdata_h[RAU_ui_txdlength_h - 2], 1 );

	return 0;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_CreateTsumData()
 *[]----------------------------------------------------------------------[]*
 *|	集計データの作成、tsum格納
 *[]----------------------------------------------------------------------[]*
 *|	patam	none
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void RAUhost_CreateTsumData(void)
{
	ushort		logCount;
	uchar		i,j;
	ushort		size;
	ushort		zerosize;
	uchar		blockCount;
	BOOL		startFlg = TRUE;
	t_RemoteNtNetBlk*	pPacketData;				// パケットデータ

	// tsum領域のリセット
	reset_tsum((char*)RAU_table_data.total, RAU_TOTAL_SIZE);
	
	logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
	// ログデータが無ければ0件データを送信する
	if(logCount != 0) {
		Ope_Log_TargetDataVoidRead(eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
		startFlg = FALSE;
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		for(i=0; i<8; i++) {
		for(i=0; i<7; i++) {
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			switch(i) {
			case 0:
				// 集計基本データ(30)
				NTNET_Edit_SyukeiKihon((SYUKEI *)RAU_LogData, 1, (DATA_KIND_30 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_30);
				break;
			case 1:
				// 料金種別毎集計データ(31)
				NTNET_Edit_SyukeiRyokinMai((SYUKEI *)RAU_LogData, 1, (DATA_KIND_31 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_31);
				break;
			case 2:
				// 分類集計データ(32)
				NTNET_Edit_SyukeiBunrui((SYUKEI *)RAU_LogData, 1, (DATA_KIND_32 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_32);
				break;
			case 3:
				// 割引集計データ(33)
				NTNET_Edit_SyukeiWaribiki((SYUKEI *)RAU_LogData, 1, (DATA_KIND_33 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_33);
				break;
			case 4:
				// 定期集計データ(34)
				NTNET_Edit_SyukeiTeiki((SYUKEI *)RAU_LogData, 1, (DATA_KIND_34 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_34);
				break;
			case 5:
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//				// 車室毎集計データ(35)
//				NTNET_Edit_SyukeiShashitsuMai((SYUKEI *)RAU_LogData, 1, (DATA_KIND_35 *)RAU_NtLogData);
//				size = sizeof(DATA_KIND_35);
//				break;
//			case 6:
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
				// 金銭集計データ(36)
				NTNET_Edit_SyukeiKinsen((SYUKEI *)RAU_LogData, 1, (DATA_KIND_36 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_36);
				break;
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//			case 7:
			case 6:
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
				// 集計終了通知データ(41)
				NTNET_Edit_SyukeiSyuryo((SYUKEI *)RAU_LogData, 1, (DATA_KIND_41 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_41);
				break;
			default:
				break;
			}

			RAU_NtLogDataSize = RAUhost_ConvertNtBlockData(RAU_NtLogData, size, RAU_NtBlockLogData, &blockCount);

			// tsum領域書込み
			for(j=0; j<blockCount; j++){
				pPacketData = (t_RemoteNtNetBlk*)(RAU_NtBlockLogData + (j * 970));
				zerosize = pPacketData->header.packet_size[0] * 0x100;
				zerosize += pPacketData->header.packet_size[1];
				write_tsum((char*)pPacketData, zerosize);
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_CreateTsumData_r10()
 *[]----------------------------------------------------------------------[]*
 *|	集計データの作成、tsum格納(フォーマットRevNo.10用)
 *[]----------------------------------------------------------------------[]*
 *|	patam	ushort	type	: 1=Ｔ合計 2=ＧＴ合計
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void RAUhost_CreateTsumData_r10(ushort type)
{
	ushort		logCount;
	uchar		i,j;
	ushort		size;
	ushort		zerosize;
	uchar		blockCount;
	BOOL		startFlg = TRUE;
	t_RemoteNtNetBlk*	pPacketData;				// パケットデータ

	// tsum領域のリセット
	reset_tsum((char*)RAU_table_data.total, RAU_TOTAL_SIZE);

	if(type == 1) {
		logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
	}
	else if(type == 11){
		logCount = Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
	}
	else {
		return;
	}
	// ログデータが無ければ0件データを送信する
	if(logCount != 0) {
		if(type == 1) {
			Ope_Log_TargetDataVoidRead(eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
		}
		else {
			Ope_Log_TargetDataVoidRead(eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
		}
		startFlg = FALSE;
		for(i=0; i<5; i++) {
			switch(i) {
			case 0:
				// 集計基本データ(42)
// 仕様変更(S) K.Onodera 2016/11/04 集計基本データフォーマット対応
//				NTNET_Edit_SyukeiKihon_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_42 *)RAU_NtLogData);
//				size = sizeof(DATA_KIND_42);
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//				NTNET_Edit_SyukeiKihon_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_42_r13 *)RAU_NtLogData);
//				size = sizeof(DATA_KIND_42_r13);
				NTNET_Edit_SyukeiKihon_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_42 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_42);
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/11/04 集計基本データフォーマット対応
				break;
			case 1:
				// 料金種別毎集計データ(43)
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//				NTNET_Edit_SyukeiRyokinMai_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_43 *)RAU_NtLogData);
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//				NTNET_Edit_SyukeiRyokinMai_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_43 *)RAU_NtLogData);
				NTNET_Edit_SyukeiRyokinMai_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_43 *)RAU_NtLogData);
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
				size = sizeof(DATA_KIND_43);
				break;
			case 2:
				// 割引集計データ(45)
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//				NTNET_Edit_SyukeiWaribiki_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_45 *)RAU_NtLogData);
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//				NTNET_Edit_SyukeiWaribiki_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_45 *)RAU_NtLogData);
				NTNET_Edit_SyukeiWaribiki_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_45 *)RAU_NtLogData);
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
				size = sizeof(DATA_KIND_45);
				break;
			case 3:
				// 定期集計データ(46)
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//				NTNET_Edit_SyukeiTeiki_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_46 *)RAU_NtLogData);
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//				NTNET_Edit_SyukeiTeiki_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_46 *)RAU_NtLogData);
				NTNET_Edit_SyukeiTeiki_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_46 *)RAU_NtLogData);
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
				size = sizeof(DATA_KIND_46);
				break;
			case 4:
				// 集計終了通知データ(53)
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
//				NTNET_Edit_SyukeiSyuryo_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_53 *)RAU_NtLogData);
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//				NTNET_Edit_SyukeiSyuryo_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_53 *)RAU_NtLogData);
				NTNET_Edit_SyukeiSyuryo_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_53 *)RAU_NtLogData);
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
				size = sizeof(DATA_KIND_53);
				break;
			default:
				break;
			}

			RAU_NtLogDataSize = RAUhost_ConvertNtBlockData(RAU_NtLogData, size, RAU_NtBlockLogData, &blockCount);

			// tsum領域書込み
			for(j=0; j<blockCount; j++){
				pPacketData = (t_RemoteNtNetBlk*)(RAU_NtBlockLogData + (j * 970));
				zerosize = pPacketData->header.packet_size[0] * 0x100;
				zerosize += pPacketData->header.packet_size[1];
				write_tsum((char*)pPacketData, zerosize);
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_ConvertNtBlockData()
 *[]----------------------------------------------------------------------[]*
 *|	ＮＴデータ1ブロック作成
 *[]----------------------------------------------------------------------[]*
 *|	patam	const uchar* pData	:
 *|			ushort size			:
 *|			uchar* pNtBlockData	:
 *[]----------------------------------------------------------------------[]*
 *|	return	1ブロック目のサイズ
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	RAUhost_ConvertNtBlockData(const uchar* pData, ushort size, uchar* pNtBlockData, uchar *blockCount)
{
	DATA_BASIC*	pBasicData;							// 送信NT-NETデータ
	t_RemoteNtNetBlk*	pPacketData;				// パケットデータ
	uchar*	pNtData;
	ushort	remainDataSize;							// 残りデータサイズ
	ushort	packetDataSize;							// パケットデータサイズ
	ushort	size0Cut;								// 0カットしたデータサイズ
	ushort	headerSize;								// パケットヘッダサイズ
	ushort	totalSize = 0;
	uchar	block = 0;								// ブロック
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	ushort	readNtdata = 0;
	
	remainDataSize = size - 3;
	
	pBasicData = (DATA_BASIC*)pData;
	
	pNtData = (uchar*)pData + 3;
	headerSize = sizeof(t_RemoteNtNetBlk) - NTNET_BLKDATA_MAX;	// ヘッダサイズ
	
	while(remainDataSize)
	{
		pPacketData = (t_RemoteNtNetBlk*)(pNtBlockData + (block * 970));
		++block;												// ブロック更新(1～)

		// 1パケットのサイズを求める
		if(remainDataSize >= NTNET_BLKDATA_MAX) {
			packetDataSize = NTNET_BLKDATA_MAX;
		}
		else {
			packetDataSize = remainDataSize;
		}

		//データを送信バッファに格納
		memcpy(pPacketData->data.data, pNtData + readNtdata, packetDataSize);
		readNtdata += packetDataSize;
		remainDataSize -= packetDataSize;						// 残りデータサイズ更新

		// 0カットサイズを求める
		size0Cut = packetDataSize;
		while(size0Cut) {
			if(pPacketData->data.data[size0Cut - 1] != 0) {
				break;
			}
			--size0Cut;
		}
		
		// パケットヘッダの作成
		u.us = headerSize + size0Cut;							// データサイズ(ヘッダ+0カット後データ)
		pPacketData->header.packet_size[0] = u.uc[0];
		pPacketData->header.packet_size[1] = u.uc[1];
		u.us = packetDataSize;									// データサイズ(0カット前データ)
		pPacketData->header.len_before0cut[0] = u.uc[0];
		pPacketData->header.len_before0cut[1] = u.uc[1];
		pPacketData->data.blk_no = block;						// ID1(ブロック番号)
		if(remainDataSize) {
			pPacketData->data.is_lastblk = 0;					// ID2(中間)
		}
		else {
			pPacketData->data.is_lastblk = 1;					// ID2(最終)
		}
		pPacketData->data.system_id = pBasicData->SystemID;		// ID3(システムID)
		pPacketData->data.data_type = pBasicData->DataKind;		// ID4(データID)
		pPacketData->data.save = pBasicData->DataKeep;			// データ保持フラグ
		
		totalSize += size0Cut + 10;
	}
	*blockCount = block;
	return totalSize;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_ConvertDataIdToLogData()
 *[]----------------------------------------------------------------------[]*
 *|	ＮＴデータ1ブロック作成
 *[]----------------------------------------------------------------------[]*
 *|	patam	none
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
short	RAUhost_ConvertDataIdToLogData(uchar dataId)
{
	short	logId = -1;

	switch(dataId){
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	case 20:											// 入庫データ
//		logId = eLOG_ENTER;
//		break;
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
	case 23:											// 精算データ
		logId = eLOG_PAYMENT;
		break;
	case 120:											// エラーデータ
		logId = eLOG_ERROR;
		break;
	case 121:											// アラームデータ
		logId = eLOG_ALARM;
		break;
	case 122:											// モニタデータ
		logId = eLOG_MONITOR;
		break;
	case 123:											// 操作モニタデータ
		logId = eLOG_OPERATE;
		break;
	case 131:											// コイン金庫集計合計データ
		logId = eLOG_COINBOX;
		break;
	case 133:											// 紙幣金庫集計合計データ
		logId = eLOG_NOTEBOX;
		break;
	case 236:											// 駐車台数データ
		logId = eLOG_PARKING;
		break;
	case 41:											// Ｔ集計
		logId = eLOG_TTOTAL;
		break;
	case 53:											// フォーマットRevNo.10 Ｔ／ＧＴ集計
		if(pRAUhost_SendIdInfo->pbuff_info->ui_syuType == 1) {
			logId = eLOG_TTOTAL;
		}
		else {
			logId = eLOG_GTTOTAL;
		}
		break;
	case 126:											// 金銭管理
		logId = eLOG_MONEYMANAGE;
		break;
	case 135:											// 釣銭管理集計データ
		logId = eLOG_MNYMNG_SRAM;
		break;
	case 125:											// 遠隔監視
		logId = eLOG_REMOTE_MONITOR;
		break;
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	case 61:											// 長期駐車(ParkingWeb用)
		logId = eLOG_LONGPARK_PWEB;
		break;
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	default:
		break;
	}

	return logId;
}

//==============================================================================
//	サーバーからのデータ受信
//
//	@argument	port	対象ポート(UPLINE:上り回線, DOWNLINE:下り回線)
//
//	@return		0		データ受信なし
//	@return		1		データ受信あり
//
//	@author		2013.07.13:MATSUSHITA
//======================================== Copyright(C) 2013 AMANO Corp. =======

/*
//  リモートパケット
//	+0		パケットデータ長			(4)		－＋　－－－－－－－－－＋
//	+4		"AMANO_REMOTE"				(12)	　｜					｜
//	+16		シーケンス長				(3)		　｜					｜
//	+19		データ部コード				(1)		　｜リモートヘッダ		｜パケットデータ長
//	+20		電文識別コード				(1)		　｜ (26 byte)			｜
//	+21		リモートブロックNo.			(2)		　｜					｜
//	+23		リモートブロックステータス	(1)		　｜					｜
//	+24		予備						(2)		－＋					｜
//	+26		リモートブロックデータ		(1420/max)						｜
//	+??		CRC16						(2)		－－－－－－－－－－－－＋
//
//  応答データ
//	+0		パケットデータ長			(4)		－＋
//	+4		"AMANO_REMOTE"				(12)	　｜
//	+16		シーケンス長				(3)		　｜
//	+19		データ部コード				(1)		　｜リモートヘッダ
//	+20		電文識別コード				(1)		　｜
//	+21		リモートブロックNo.			(2)		　｜
//	+23		リモートブロックステータス	(1)		　｜
//	+24		応答ステータス				(2)		－＋
//	+26		ACK/NAK						(1)
//	+27		CRC16						(2)
*/
int		RAUhost_Receive(int port)
{
	uchar	*buff;
	ushort	bufflen, len, wlen;
	ulong	pklen;
	int		ret;
	int		ch;
	int		ofs;

// 受信データ読取り
	ch = port - 1;					// UPLINE(1)、DOWNLINE(2)、CREDIT(3)→0～2
	buff = RAU_huc_rcv_buf[ch];		// ポート毎受信バッファ
	bufflen = RAU_huc_rcv_len[ch];	// ポート毎受信済みデータ長
	len = 0;						// 今回受信データ長
	wlen = 0;

	if (RAU_DpaRcvQue_Read(RAU_huc_rcv_tmp, &len, port) != 0) {
	// データがあればポート毎のバッファに転送
		wlen = RAU_RCV_MAX_H - bufflen;
		if (wlen > len) {
		// 空きエリアが充分にある
			wlen = len;				// 全て転送する
		}
		memcpy(&buff[bufflen], RAU_huc_rcv_tmp, wlen);
		bufflen += wlen;
		len -= wlen;				// バッファ保存分を引く（≠０ならtmpにデータが残っている）
	}

// 受信パケットチェック
	ret = 0;
	while(bufflen > 4) {
	// パケットデータ長の次のデータがある
		if (buff[4] == uc_dpa_rem_str[0]) {
		// シグニチャ先頭が一致
			if (bufflen < 16) {
				break;		// シグニチャ全体が無ければ待つ
			}
			if (memcmp(&buff[4], uc_dpa_rem_str, 12) == 0) {
			// 受信パケット検出
				pklen = c_arraytoint32(buff, 4);				// パケット長取得
				if (pklen > RAU_RCV_MAX_H) {
					switch (port) {
					case UPLINE:
						RAUhost_SetError(ERR_RAU_DPA_RECV_LONGER);
						break;
					case DOWNLINE:
						RAUhost_SetError(ERR_RAU_DPA_RECV_LONGER_D);
						break;
					case CREDIT:
						Credit_SetError(ERR_CREDIT_DPA_RECV_LONGER);
						break;
					default:
						break;
					}
					bufflen = 0;		// 長さがおかしいので全データ破棄
					len = 0;
				}
				else if (pklen <= bufflen) {
				// 全データ受信済み
					// クレジット回線
					if (port == CREDIT) {
						memcpy(RAU_huc_rcv_work_CRE, buff, pklen);		// 受信パケットを解析用ワークに転送
						RAU_ui_data_length_CRE = pklen;					// パケット長をセット
					}
					// 上り回線、下り回線
					else {
						memcpy(RAU_huc_rcv_work_h, buff, pklen);		// 受信パケットを解析用ワークに転送
						RAU_ui_data_length_h = pklen;					// パケット長をセット
					}
					bufflen -= pklen;
					if (bufflen != 0) {
						memcpy(buff, &buff[pklen], bufflen);	// 残りがあれば前につめる
					}
					ret = 1;
				}
				break;
			}
		// シグニチャが不一致
		}
	// シグニチャが合わないのでずらしてやり直し
		for (ofs = 5; ofs < bufflen; ofs++) {
			if (buff[ofs] == uc_dpa_rem_str[0]) {
				break;
			}
		}
		ofs -= 4;
		bufflen -= ofs;
		memcpy(buff, &buff[ofs], bufflen);
	}
	if (len != 0) {
	// 残余があれば保存
		memcpy(&buff[bufflen], &RAU_huc_rcv_tmp[wlen], len);		// この場合オーバーフローは無いはず
		bufflen += len;
	}
	RAU_huc_rcv_len[ch] = bufflen;
	return ret;
}

short RAUhost_CreateDopaPacketCTI(void)
{
	ushort		uiReadLen;
	ushort		uiRemain;
	uchar		ucReadEnd;
	ushort		PacketDataSize;		// パケットのデータ部サイズ(ＤｏＰａ:１４２０バイト, ＸＰｏｒｔ:５００バイト)

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// モデムあり:ＤｏＰａ
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// モデムなし:Ethernet
	}

	ucReadEnd = 0;
	RAU_ui_txdlength_h = 0;

	if (pRAUhost_SendIdInfo->send_data_count > 0) {

		// データあり
		while (ucReadEnd == 0) {

			// ＮＴデータの情報を取得する｡
			if (RAUhost_GetNtDataCenterTermInfo() == FALSE) {
				// データ0件は送らない
				return -1;
			}

			// ＮＴデータの残量を算出
			uiRemain = RAUhost_SendNtDataInfo.nt_data_end - RAUhost_SendNtDataInfo.nt_data_next;

			// ＮＴデータから読み出す量を決定
			if ((PacketDataSize - RAU_ui_txdlength_h) < uiRemain) {
				uiReadLen = (PacketDataSize - RAU_ui_txdlength_h);
			} else {
				uiReadLen = uiRemain;
			}

			// ＤｏｐａパケットにＮＴデータを充填
			memcpy(&RAU_huc_txdata_h[DOPA_HEAD_SIZE + RAU_ui_txdlength_h], RAUhost_SendNtDataInfo.nt_data_next, uiReadLen );

			// 次回書き込み位置をコピーした長さ分進める｡
			RAUhost_SendNtDataInfo.nt_data_next += uiReadLen;

			// 読み込み量を増加
			RAU_ui_txdlength_h += uiReadLen;

			if (RAUhost_SendNtDataInfo.nt_data_end <= RAUhost_SendNtDataInfo.nt_data_next) {
				// １ＮＴデータの最後まで書き込んだので、fillをインクリメント
				pRAUhost_SendIdInfo->fill_up_data_count++;
			}

			// 書き込み終了チェック
			if (pRAUhost_SendIdInfo->send_data_count <= (pRAUhost_SendIdInfo->send_complete_count
														+ pRAUhost_SendIdInfo->fill_up_data_count
														+ pRAUhost_SendIdInfo->crc_err_data_count)) {
				// これ以上データがなければ、送信｡
				ucReadEnd = 2;
			} else if (RAU_ui_txdlength_h >= PacketDataSize) {
				// バッファを満たしていたら送信｡
				ucReadEnd = 1;
			}

		}
	} else {
		// データ0件は送らない
		return -1;
	}

	// データ長修正｡
	RAU_ui_txdlength_h = DOPA_HEAD_SIZE + RAU_ui_txdlength_h + 2;

	// 中断要求あり
	if( ucReadEnd == 2 ){	// 参照しているテーブル(ＩＤ)の全データの充填が終わった。
		RAU_ucTcpBlockSts = 1;	// ＴＣＰブロックステータス１（＝終端）
	}

	// ヘッダー設定
	RAU_DpaSndHedSet(RAU_ucTcpBlockSts, UPLINE);

	// ＣＲＣ算出
	RAU_CrcCcitt(RAU_ui_txdlength_h - 2, 0, RAU_huc_txdata_h, NULL, &RAU_huc_txdata_h[RAU_ui_txdlength_h - 2], 1 );

	return 0;
}

BOOL RAUhost_GetNtDataCenterTermInfo(void)
{
	BOOL ret;

	ret = FALSE;

	if (RAUhost_SendNtDataInfo.nt_data_start == NULL) {				// NULLだったらデータを取得する｡

		while (pRAUhost_SendIdInfo->send_data_count > pRAUhost_SendIdInfo->crc_err_data_count ) {

			if (RAUdata_GetCenterTermInfoData(&RAUhost_SendNtDataInfo, RAU_special_table_send_buffer) == TRUE) {
				ret = TRUE;
				break;
			} else {
				// データがCRCエラーだったので次のデータを取得しに行く
				pRAUhost_SendIdInfo->crc_err_data_count++;
			}

		}
		if (pRAUhost_SendIdInfo->send_data_count <= pRAUhost_SendIdInfo->crc_err_data_count) {
			// 全部CRCエラーだったら、仕方がないので何も送らない。
			ret = FALSE;
		}
	} else {
		if (RAUhost_SendNtDataInfo.nt_data_end <= RAUhost_SendNtDataInfo.nt_data_next) {			// データの最後まで読んでいる｡

			while (pRAUhost_SendIdInfo->send_data_count > (pRAUhost_SendIdInfo->send_complete_count
															+ pRAUhost_SendIdInfo->fill_up_data_count
															+ pRAUhost_SendIdInfo->crc_err_data_count)) {

				if (RAUdata_GetCenterTermInfoData(&RAUhost_SendNtDataInfo, RAU_special_table_send_buffer) == TRUE) {
					ret = TRUE;
					break;
				} else {
					// データがCRCエラーだったので次のデータを取得しに行く
					pRAUhost_SendIdInfo->crc_err_data_count++;
				}
			}
		} else {
			ret = TRUE;
		}
	}
	return ret;
}

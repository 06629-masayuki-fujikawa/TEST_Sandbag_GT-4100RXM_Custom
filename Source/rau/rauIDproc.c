/****************************************************************************/
/*                                                                          */
/*      システム名　:　RAUシステム                                          */
/*                                                                          */
/*      ファイル名  :  IDPROC.C                                             */
/*      機能            :  ＩＤ毎詳細処理                                   */
/*                                                                          */
/*  Version     :                                                           */
/*                                                                          */
/*  改版履歴                                                                */
/****************************************************************************/

#include	<string.h>
#include	<stddef.h>
#include	"system.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"rauIDproc.h"
#include	"rau.h"
#include	"ksgRauModemData.h"

const uchar RAU_TableDataTbl[]={1,4,100,103,109,142,243,154,156};	// ＨＯＳＴから受信する遠隔システム電文(システムＩＤが１０～１９のもの)でＩＢＷからの応答電文のあるもの
// MH322914(S) K.Onodera 2016/08/08 AI-V対応
const uchar RAU_TableDataTbl_PIP[]={1,3,5,7,10};					// ＨＯＳＴから受信する遠隔システム電文(システムＩＤが１６のもの)でＩＢＷからの応答電文のあるもの
// MH322914(E) K.Onodera 2016/08/08 AI-V対応
const uchar RAU_RemoteDataTbl[]={80};	// ＨＯＳＴから受信する遠隔固有電文(システムＩＤが３０のもの)でＩＢＷからの応答電文のあるもの

uchar	RAU_ibw_receive_data[RAU_IBW_TEMP_BUFF_SIZE];	// ※遠隔PCへの送信(IBWからの受信)のみ32blockに拡張 2006.08.31:m-onouchi
uchar	RAU_host_receive_data[RAU_HOST_TEMP_BUFF_SIZE];

extern		RAU_TIME_AREA	RAU_Tm_Port_Watchdog;// 下り回線通信監視タイマ(下り回線のＴＣＰ接続強制切断時間)
extern	int		KSG_gPpp_RauStarted;				// PPP接続フラグ

extern	void	RAU_data_table_init(void);
extern	uchar	RAU_ID98new_SaveToTmpBuf( uchar *pData, uchar Kind );
extern	void	RAU_7segErrSet( uchar Kind, uchar f_OnOff );
extern	eRAU_TABLEDATA_SET	RAUdata_SetTableData(uchar *pData, ushort wDataLen, uchar ucDataId);
extern	eRAU_TABLEDATA_SET	RAUdata_SetThroughData(uchar *pData, ushort wDataLen, uchar ucDirection);
extern	BOOL	RAUdata_CheckNearFull( void );
extern	void	RAUdata_CreatRauConfig1(RAU_CONFIG1 *pConfig1);
extern	void	RAUdata_CreatRauConfig2(RAU_CONFIG2 *pConfig2);
extern	void	RAUdata_TableClear(uchar ucTableKind);
extern	void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );
extern	void	RAU_Word2Byte(unsigned char *data, ushort uShort);
extern	void	RAUhost_SetSendRequeat(RAU_SEND_REQUEST *send_request);
extern	void	RAUhost_SetRcvSeqFlag(uchar ucRcvSeqFlag);
extern	void	RAUhost_SetError(uchar ucErrCode);

void	RAUid_Init(void);
eRAU_TABLEDATA_SET	RAUid_SetNtBlockData(t_RemoteNtNetBlk *pBlockData, ushort size);
eRAU_TABLEDATA_SET	RAUid_IbwDataAnalyze(uchar ucSystemId, uchar ucDataId, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_HostDataAnalyze(uchar ucSystemId, uchar ucDataId, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_IbwThroughDataAnalyze(uchar ucDataId, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_HostThroughDataAnalyze(uchar ucDataId, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_IbkControl(RAU_CONTROL_REQUEST *pCtrlReq);
BOOL	RAU_isTableData(uchar ucDataId);
void	RAUid_SendRequest(RAU_SEND_REQUEST *pSendReq);
BOOL	RAUid_TableDataClear(RAU_CLEAR_REQUEST *pClearReq);
BOOL	RAUid_TableDataInitialize( void );
eRAU_TABLEDATA_SET RAUid_CommuTest(RAU_COMMUNICATION_TEST *pNTdata);
BOOL	RAUid_CheckRcvDopaData(uchar *pData, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_SetRcvNtData(uchar *pData, ushort uiDataLen, ushort uiTcpBlkNo, uchar ucTcpBlkSts);
BOOL	RAUid_RcvSeqCnt_Start(uchar ucSystemId, uchar ucDataId);
void	RAUid_Unpack0Cut(uchar *ucData);

extern	void	RAUdata_CreateNearFullData(RAU_NEAR_FULL *pNearFullData);
extern	void RAUdata_CreatTableDataCount(RAU_TABLE_DATA_COUNT *pDataCount);

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_Init()
 *[]----------------------------------------------------------------------[]*
 *|	初期化
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-07-20
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUid_Init(void) {
	RAU_ucLastNtBlockNum = 0;
	RAU_uiArcReceiveLen = 0;

	RAU_uiLastTcpBlockNum = 0;
	RAU_uiDopaReceiveLen = 0;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_SetNtBlockData()
 *[]----------------------------------------------------------------------[]*
 *|	param	pBlockData	受信データのＮＴブロック
 *|			size		受信データのＮＴブロックサイズ
 *[]----------------------------------------------------------------------[]*
 *|	return	eRAU_TABLEDATA_SET
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_SetNtBlockData(t_RemoteNtNetBlk *pBlockData, ushort size)
{
	eRAU_TABLEDATA_SET	ret;
	eRAU_TABLEDATA_SET	temp;
	uchar ucBlockNum;
	uchar ucBlockSts;
	union {
		uchar	uc[2];
		ushort	us;
	}u;
	ulong		wkul;

	ucBlockNum = pBlockData->data.blk_no;
	ucBlockSts = pBlockData->data.is_lastblk;

	if (RAU_ucLastNtBlockNum == 0) {
		// 現在ブロック０（電文確定直後）
		if (ucBlockNum == 1) {
			// 今回１
			memcpy(&RAU_ibw_receive_data[2], pBlockData, size);
			RAU_uiArcReceiveLen = size;
			RAU_ucLastNtBlockNum = 1;
			ret = RAU_DATA_NORMAL;
		} else {
			// 今回1以外　　**ブロックナンバー異常今回無効**
			RAUhost_SetError(RAU_ERR_BLK_INVALID);	// エラーコード２０(ＮＴ－ＮＥＴブロック番号異常[今回無効])
			RAU_uiArcReceiveLen = 0;
			RAU_ucLastNtBlockNum = 0;
			ret = RAU_DATA_BLOCK_INVALID1;
		}
	} else {
		// 現在ブロック0以外
		if (ucBlockNum == 1) {
			// ブロックナンバー１書き込み　**ブロックナンバー異常今回有効**
			RAUhost_SetError(RAU_ERR_BLK_VALID);	// エラーコード１９(ＮＴ－ＮＥＴブロック番号異常[今回有効])
			memcpy(&RAU_ibw_receive_data[2], pBlockData, size);
			RAU_uiArcReceiveLen = size;
			RAU_ucLastNtBlockNum = 1;
			ret = RAU_DATA_BLOCK_INVALID2;
		} else if ((RAU_ucLastNtBlockNum + 1) == ucBlockNum) {
			// ブロックナンバー比較　前回＋１
			memcpy(&RAU_ibw_receive_data[RAU_uiArcReceiveLen + 2], pBlockData, size);
			RAU_uiArcReceiveLen += size;
			RAU_ucLastNtBlockNum++;
			ret = RAU_DATA_NORMAL;
		} else {
			// それ以外　　**ブロックナンバー異常今回無効**
			RAUhost_SetError(RAU_ERR_BLK_INVALID);	// エラーコード２０(ＮＴ－ＮＥＴブロック番号異常[今回無効])
			RAU_uiArcReceiveLen = 0;
			RAU_ucLastNtBlockNum = 0;
			ret = RAU_DATA_BLOCK_INVALID1;
		}
	}

	if ((ret == RAU_DATA_NORMAL) || (ret == RAU_DATA_BLOCK_INVALID2)) {
		// 最終ブロック
		if (ucBlockSts == 1) {
			// ＩＤ識別処理内でテーブルバッファにコピー

			u.us = RAU_uiArcReceiveLen + 2;
			RAU_ibw_receive_data[0] = u.uc[0];
			RAU_ibw_receive_data[1] = u.uc[1];
			/* ARCNET受信データ，システムID(ID3)が10～19,電文データ種別(ID4)が229 で */
			/* Main基板から受信した時計データ電文と判断する */
			if(	(10 <= RAU_ibw_receive_data[9]) &&
				(RAU_ibw_receive_data[9] <= 19) &&	// システムＩＤ（ＩＤ３）：遠隔システム系統
				(RAU_ibw_receive_data[10] == 229) )	// データＩＤ（ＩＤ４）  ：時計データ
			{
				/* ここまでで時計データ電文受信Arcnetフレームと確定                                                 */
				/* NT-NET時計データ電文は従来の形式(33byte)とe-timing同期に対応した形式(41byte)の2通り存在する。    */
				/* 形式の違いを0カット前のデータサイズで判別しe-timing同期に対応した形式の場合には特殊処理を行う。  */
				u.uc[0] = RAU_ibw_receive_data[4];
				u.uc[1] = RAU_ibw_receive_data[5];					// 0カット前のデータサイズget
				if( u.us >= 41 ){									// 新時計データ電文（補正値付き）であれば
					RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);		// 0カットされたデータを復元する
					u.us += NT_DOPANT_HEAD_SIZE;					// 0カット前データサイズ+リモートヘッダ(10byte)=パケットデータサイズ
					RAU_ibw_receive_data[2] = u.uc[0];				// パケットデータサイズを伸張
					RAU_ibw_receive_data[3] = u.uc[1];				// (末尾4byteにライフタイマー値をセットするためフルサイズになる)
					RAU_uiArcReceiveLen = u.us;							// NT-NET電文のデータ長を再度セット(時計データは1ブロックであるため成り立つ)
					u.us += 2;										// NT-NET電文のデータ長+データ長格納エリア(2byte)を再度セット
					RAU_ibw_receive_data[0] = u.uc[0];				// 
					RAU_ibw_receive_data[1] = u.uc[1];				// 
					wkul = RAU_c_1mTim_Start();
					memcpy( &RAU_ibw_receive_data[49], &wkul, 4 );	// 電文に現在のライフタイマー値をセット
				}
			}
			RAU_CrcCcitt(RAU_uiArcReceiveLen, 0, &RAU_ibw_receive_data[2], (uchar*)NULL, &RAU_ibw_receive_data[RAU_uiArcReceiveLen + 2], 1 );

			RAU_ucLastNtBlockNum = 0;

			temp = RAUid_IbwDataAnalyze(RAU_ibw_receive_data[9], RAU_ibw_receive_data[10], RAU_uiArcReceiveLen + 4);
			if (temp != RAU_DATA_NORMAL) {
				ret = temp;
			}
			RAU_uiArcReceiveLen = 0;
		} else {
			// 最終ブロックではない、ブロックナンバー３２に到達　**ブロックナンバー異常今回無効**	
			if (ucBlockNum >= RAU_SEND_NTBLKNUM_MAX && ret == RAU_DATA_NORMAL) {	// ※IBWからの受信(遠隔PCへの送信)のみ32blockに拡張 2006.08.31:m-onouchi
				// ここまで正常判定で３２ブロック以上だったら、エラー
				RAUhost_SetError(RAU_ERR_BLK_OVER_ARC);	// エラーコード５０(ＮＴ－ＮＥＴ受信ブロックオーバー)
				RAU_uiArcReceiveLen = 0;
				RAU_ucLastNtBlockNum = 0;
				ret = RAU_DATA_BLOCK_INVALID1;
			}
		}
	} 
	return ret;
}

//==============================================================================
//	ホストから受信したＮＴ－ＮＥＴ電文の保存
//
//	@argument	*pData		ホストから受信したデータ(ＤｏＰａヘッダを除いたデータ部＋ＣＲＣ)
//	@argument	uiDataLen	ホストから受信したデータの長さ(ＤｏＰａヘッダとＣＲＣを除いたデータ部)
//	@argument	uiTcpBlkNo	ＴＣＰブロックＮｏ．
//	@argument	ucTcpBlkSts	ＴＣＰブロックステータス
//
//	@return		RAU_DATA_NORMAL					データの保存完了または送信要求データ(ＩＤ８１)を正常受信
//	@return		RAU_DATA_BLOCK_INVALID1			ＤｏＰａパケットまたはＮＴ－ＮＥＴ電文の構造エラー
//	@return		RAU_DATA_CONNECTING_FROM_HOST	テーブルデータ送信中にホストから送信要求を受けたため受け付け不可
//	@return		RAU_DATA_BUFFERFULL				バッファが一杯
//
//	@attention	システムＩＤ３０のデータは受信スルーデータテーブルには入れられません。
//
//	@note		ホストより１ＤｏＰａパケット受信する毎にコールされ
//				最終パケットを受信したとき、受信シーケンスが変更されると共に
//				取得したＮＴ－ＮＥＴ電文を受信スルーデータテーブルに保存します。
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
eRAU_TABLEDATA_SET RAUid_SetRcvNtData(uchar *pData, ushort uiDataLen, ushort uiTcpBlkNo, uchar ucTcpBlkSts)
{
	eRAU_TABLEDATA_SET	ret;
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	ulong		wkul;

	ret = RAU_DATA_BLOCK_INVALID1;
	if( RAU_uiLastTcpBlockNum == 0 ){	// 最後に受信したＴＣＰブロックの番号(０は初期化時かブロックステータス１のデータを処理したとき)
		if( uiTcpBlkNo == 1 ){		// 最初のＴＣＰブロック(ＴＣＰブロック番号が１)
			memcpy(&RAU_host_receive_data[2], pData, uiDataLen);	// ホストから受信したパケットを(巨大な電文でも必ず入る)バッファの先頭に格納
			RAU_uiLastTcpBlockNum = 1;								// 最後に受信したＴＣＰブロックの番号
			RAU_uiDopaReceiveLen = uiDataLen;						// 受信したデータのサイズ(最初なのでそのまま代入)
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 			RAU_Tm_RcvSeqCnt = 0;									// タイマ値クリア
// 			RAU_f_RcvSeqCnt_rq = 1;									// 受信シーケンスキャンセルタイマ動作許可(起動)
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_WAIT);		// ＨＯＳＴから何か受信したので受信シーケンスを１に変更
			RAUdata_TableClear(0);								// スルーデータの削除
			ret = RAU_DATA_NORMAL;
		} else {					// 最初なのにＴＣＰブロック番号が１じゃないのはＮＧ
			RAUhost_SetError(ERR_RAU_DPA_BLK_INVALID_D);		// エラーコード８１(下り回線受信データブロック番号異常[今回無効])
			RAU_uiLastTcpBlockNum = 0;
			RAU_uiDopaReceiveLen = 0;
		}
	} else {						// 途中のパケットを受信中
		if( uiTcpBlkNo == 1 ){		// 途中パケット受信中にＴＣＰブロック番号１のブロックを受信
			RAUhost_SetError(ERR_RAU_DPA_BLK_VALID_D);			// エラーコード８０(下り回線受信データブロック番号異常[今回有効])
			memcpy(&RAU_host_receive_data[2], pData, uiDataLen);	// ホストから受信したパケットを(巨大な電文でも必ず入る)バッファの先頭に格納
			RAU_uiLastTcpBlockNum = 1;								// 最後に受信したＴＣＰブロックの番号
			RAU_uiDopaReceiveLen = uiDataLen;						// 受信したデータのサイズ(最初なのでそのまま代入)
			ret = RAU_DATA_NORMAL;
		} else if( (RAU_uiLastTcpBlockNum + 1) == uiTcpBlkNo ){	// ＴＣＰブロック番号が連番になっていることを確認。
			if( RAU_uiDopaReceiveLen + uiDataLen <= RAU_HOST_TEMP_BUFF_SIZE ){	// 受信したパケットを保存してもバッファは溢れない。
				memcpy(&RAU_host_receive_data[2 + RAU_uiDopaReceiveLen], pData, uiDataLen);	// バッファの続き部分に受信したパケットを格納
				RAU_uiLastTcpBlockNum = uiTcpBlkNo;			// 最後に受信したＴＣＰブロックの番号
				RAU_uiDopaReceiveLen += uiDataLen;			// 今まで受信したデータのサイズ(合計)
				ret = RAU_DATA_NORMAL;
			} else {				// 受信したパケットを保存するとバッファが溢れる。
// MH322914 (s) kasiyama 2016/07/12 受信ＮＧによりテーブル送信が停止する問題の改善[共通バグNo.1256](MH341106)
				RAUhost_SetError(ERR_RAU_DPA_BLK_INVALID_D);	// エラーコード８１(下り回線受信データブロック番号異常[今回無効])
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンスを０に戻す
// MH322914 (e) kasiyama 2016/07/12 受信ＮＧによりテーブル送信が停止する問題の改善[共通バグNo.1256](MH341106)
				RAU_uiLastTcpBlockNum = 0;
				RAU_uiDopaReceiveLen = 0;
			}
// MH322914 (s) kasiyama 2016/07/12 受信ＮＧによりテーブル送信が停止する問題の改善[共通バグNo.1256](MH341106)
		}
		else if (uiTcpBlkNo <= RAU_uiLastTcpBlockNum) {
		// 受信済みのブロックについてＡＣＫで応答する
			ret = RAU_DATA_NORMAL;
// MH322914 (e) kasiyama 2016/07/12 受信ＮＧによりテーブル送信が停止する問題の改善[共通バグNo.1256](MH341106)
		} else {	// ＴＣＰブロック番号が連番になっていない
			RAUhost_SetError(ERR_RAU_DPA_BLK_INVALID_D);		// エラーコード８１(下り回線受信データブロック番号異常[今回無効])
// MH322914 (s) kasiyama 2016/07/12 受信ＮＧによりテーブル送信が停止する問題の改善[共通バグNo.1256](MH341106)
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);		// 受信シーケンスを０に戻す
// MH322914 (e) kasiyama 2016/07/12 受信ＮＧによりテーブル送信が停止する問題の改善[共通バグNo.1256](MH341106)
			RAU_uiLastTcpBlockNum = 0;
			RAU_uiDopaReceiveLen = 0;
		}
	}

	if( ucTcpBlkSts == 1 && ret == RAU_DATA_NORMAL ){	// 最終ＴＣＰブロックをエラーなく正常にバッファに格納できた。
		if( RAUid_CheckRcvDopaData(&RAU_host_receive_data[2], RAU_uiDopaReceiveLen) == TRUE ){	// ＮＴデータの構造チェック(ＯＫ)
			/* 遠隔NT-NET受信データ，システムID(ID3)が10～19,電文データ種別(ID4)が119 で */
			/* 外部から受信した時計データ電文と判断する                                  */
			if(	(10 <= RAU_host_receive_data[9]) &&
				(RAU_host_receive_data[9] <= 19) &&		// システムＩＤ（ＩＤ３）：遠隔システム系統
				(RAU_host_receive_data[10] == 119) )	// データＩＤ（ＩＤ４）  ：時計データ
			{
				/* ここまでで時計データ電文受信Arcnetフレームと確定                                                 */
				/* NT-NET時計データ電文は従来の形式(33byte)とe-timing同期に対応した形式(41byte)の2通り存在する。    */
				/* 形式の違いを0カット前のデータサイズで判別しe-timing同期に対応した形式の場合には特殊処理を行う。  */
				u.uc[0] = RAU_host_receive_data[4];
				u.uc[1] = RAU_host_receive_data[5];					// 0カット前のデータサイズget
				if( u.us >= 41 ){								// 新時計データ電文（補正値付き）であれば
					RAUid_Unpack0Cut(&RAU_host_receive_data[2]);	// 0カットされたデータを復元する
					u.us += NT_DOPANT_HEAD_SIZE;				// 0カット前データサイズ+リモートヘッダ(10byte)=パケットデータサイズ
					RAU_host_receive_data[2] = u.uc[0];				// パケットデータサイズを伸張
					RAU_host_receive_data[3] = u.uc[1];				// (末尾4byteにライフタイマー値をセットするためフルサイズになる)
					RAU_uiDopaReceiveLen = u.us;					// NT-NET電文のデータ長を再度セット(時計データは1ブロックであるため成り立つ)
					wkul = RAU_c_1mTim_Start();
					memcpy( &RAU_host_receive_data[49], &wkul, 4 );	// 電文に現在のライフタイマー値をセット
				}
			}
			u.us = RAU_uiDopaReceiveLen + 2;		// 総受信サイズをバッファの先頭２バイト部分に保存
			RAU_host_receive_data[0] = u.uc[0];
			RAU_host_receive_data[1] = u.uc[1];
			RAU_CrcCcitt(RAU_uiDopaReceiveLen, 0, &RAU_host_receive_data[2], (uchar*)NULL, &RAU_host_receive_data[RAU_uiDopaReceiveLen + 2], 1 );	// ＣＲＣ算出
			ret = RAUid_HostDataAnalyze(RAU_host_receive_data[9], RAU_host_receive_data[10], RAU_uiDopaReceiveLen + 4);	// 送信要求データ(ＩＤ８１)以外は受信スルーデータテーブルに保存
		} else {
			ret = RAU_DATA_BLOCK_INVALID1;		// ＮＴデータの構造に問題あり。
		}
		RAU_uiLastTcpBlockNum = 0;
		RAU_uiDopaReceiveLen = 0;
		//1ID受信完了にて下り回線の通信監視タイマーの監視を停止する
		RAU_Tm_Port_Watchdog.tm = 0;			// 下り回線通信監視タイマ(停止)
	}

	return ret;
}
/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	IBKで処理対象になっているテーブルデータか判断する｡
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAU_isTableData(uchar ucDataId) {

	BOOL ret = FALSE;

	switch (ucDataId) {
	case 0x14:		// 入庫データ
	case 0x15:		// 出庫データ
	case 0x16:		// 精算データ（事前）
	case 0x17:		// 精算データ（出口）
	case 0x36:		// 入庫データ
	case 0x37:		// 出庫データ
	case 0x38:		// 精算データ（事前）
	case 0x39:		// 精算データ（出口）
	case 0x1E:		// T合計集計データ
	case 0x1F:		// T合計集計データ
	case 0x20:		// T合計集計データ
	case 0x21:		// T合計集計データ
	case 0x22:		// T合計集計データ
	case 0x23:		// T合計集計データ
	case 0x24:		// T合計集計データ
	case 0x25:		// T合計集計データ
	case 0x26:		// T合計集計データ
	case 0x29:		// T合計集計データ
	case 0x78:		// エラーデータテーブル
	case 0x79:		// アラームデータテーブル
	case 0x7A:		// モニタデータテーブル
	case 0x7B:		// 操作モニタデータテーブル
	case 0x83:		// コイン金庫集計合計データテーブル
	case 0x85:		// 紙幣金庫集計合計データテーブル
	case 0xEC:		// 駐車台数データテーブル
	case 0x3A:		// Web用駐車台数データテーブル
	case 0xED:		// 区画台数・満車データテーブル
	case 0x7E:		// 金銭管理データ
		ret = TRUE;
		break;
	default:
		ret = FALSE;
		break;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_IbwDataAnalyze()
 *[]----------------------------------------------------------------------[]*
 *|	ＩＢＷから受信したＮＴデータを分析する｡
 *[]----------------------------------------------------------------------[]*
 *|	patam		なし
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_IbwDataAnalyze(uchar ucSystemId, uchar ucDataId, ushort uiDataLen)
{
	eRAU_TABLEDATA_SET	ret = RAU_DATA_NORMAL;		// #002(S/E) 2007.12.13 m-onouchi

	if( RemoteCheck_Buffer116_117.CheckNo != 0 ){	// 通信チェック要求データをＨＯＳＴから受け付けている。
		RemoteCheck_Buffer116_117.Re.sult.Code = 7;	// 結果コードを(他データ受信)とする。
	}

	switch( ucSystemId ){	// システムＩＤ（ＩＤ３）
	case 0x1E:				// ３０(遠隔固有電文)
		ret = RAUid_IbwThroughDataAnalyze(ucDataId, uiDataLen);
		break;
	case 40:
		if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){		// 受信シーケンス２(ＨＯＳＴからの要求データ受信完了状態)のとき
			ret = RAUdata_SetThroughData(RAU_ibw_receive_data, uiDataLen, 0);	// 送信スルーデータバッファ(ＩＢＫ→ＨＯＳＴ)に登録
		} else {	// 受信シーケンス２(ＨＯＳＴからの要求データ受信完了状態とき以外はＩＢＷからのスルーデータを受け付けない(再送を要求)
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
		break;
	default:
		if(10 <= ucSystemId && ucSystemId <= 19){	// 遠隔システム系統
			ret = RAUdata_SetTableData(RAU_ibw_receive_data, uiDataLen, ucDataId);	// テーブルバッファに登録
			if (RAUdata_CheckNearFull() == TRUE) {									// ニアフル状態チェック
				RAUdata_CreateNearFullData((RAU_NEAR_FULL*)&RAU_temp_data[2]);		// 状態変化したので通知
				RAU_Word2Byte(RAU_temp_data, 32);
				RAU_CrcCcitt(30, 0, &RAU_temp_data[2], (uchar*)NULL, &RAU_temp_data[32], 1 );
				RAUdata_SetThroughData(RAU_temp_data, 34, 1);
			}
			break;
		}
		break;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_HostDataAnalyze()
 *[]----------------------------------------------------------------------[]*
 *|	ＨＯＳＴから受信したＮＴデータを分析する｡
 *[]----------------------------------------------------------------------[]*
 *|	patam		なし
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_HostDataAnalyze(uchar ucSystemId, uchar ucDataId, ushort uiDataLen) {

	eRAU_TABLEDATA_SET	ret;

	switch( ucSystemId ){	// システムＩＤ（ＩＤ３）
	case 0x1E:				// ３０(遠隔固有電文)
		ret = RAUid_HostThroughDataAnalyze(ucDataId, uiDataLen);
		break;
	default:
		ret = RAUdata_SetThroughData(RAU_host_receive_data, uiDataLen, 1);	// スルーデータとして登録
		if( ret == RAU_DATA_NORMAL ){									// 登録成功
			if( RAUid_RcvSeqCnt_Start(ucSystemId, ucDataId) ){			// 受信シーケンスキャンセルタイマの起動(ＩＢＷからの応答電文のあるデータのみ)
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_THROUGH_RCV);		// ＨＯＳＴからの要求データを登録したので受信シーケンスを２に変更
			} else {													// ＩＢＷからの応答電文のないデータ
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);			// ＨＯＳＴからの要求データを登録したので受信シーケンスを０に変更
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 				RAU_f_RcvSeqCnt_rq = 0;									// 受信シーケンスキャンセルタイマ動作許可(解除)
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			}
		}
		break;
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_ThroughDataAnalyze()
 *[]----------------------------------------------------------------------[]*
 *|	ＩＢＫ内で処理するスルーデータを分析する｡
 *[]----------------------------------------------------------------------[]*
 *|	patam		なし
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_IbwThroughDataAnalyze(uchar ucDataId, ushort uiDataLen)
{
	eRAU_TABLEDATA_SET	ret;

	// ＩＢＷから受信したシステムＩＤ３０(遠隔固有電文)
	if( ucDataId == 0x3C ){			// 遠隔ＩＢＫ制御データ(ＩＢＷ→ＩＢＫ)
		if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ||		// 受信シーケンス０または
			RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){	// 受信シーケンス２とき遠隔ＩＢＫ制御データを受け付ける。
			RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
			ret = RAUid_IbkControl((RAU_CONTROL_REQUEST*)(&RAU_ibw_receive_data[2]));	// 要求フラグに応じた処理(件数, 設定１, 設定２)
		} else {
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	} else if( ucDataId == 0x3D ){	// 送信要求データ(ＩＢＷ→ＩＢＫ)
		if( RAUhost_GetSndSeqFlag()==RAU_SND_SEQ_FLG_NORMAL &&	// 送信シーケンス０かつ
			RAUhost_GetRcvSeqFlag()==RAU_RCV_SEQ_FLG_NORMAL ){	// 受信シーケンス０のときのみクリア要求を受け付ける。
			RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
			RAUid_SendRequest((RAU_SEND_REQUEST*)&RAU_ibw_receive_data[2]);				// ホスト部に送信要求フラグを設定
			ret = RAU_DATA_NORMAL;
		} else {
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	} else if (ucDataId == 0x3E) {	// テーブルクリア要求データ(ＩＢＷ→ＩＢＫ)
		if( ( RAUhost_GetSndSeqFlag()==RAU_SND_SEQ_FLG_NORMAL ||	// 送信シーケンス０または上りTCP切断中、かつ
			  RauCtrl.tcpConnectFlag.port.upload != 1 )&&
			RAUhost_GetRcvSeqFlag()==RAU_RCV_SEQ_FLG_NORMAL ){		// 受信シーケンス０のときのみクリア要求を受け付ける。
			RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
			RAUid_TableDataClear((RAU_CLEAR_REQUEST*)(&RAU_ibw_receive_data[2]));	// テーブルデータクリア
			ret = RAU_DATA_NORMAL;
			if (RAUdata_CheckNearFull() == TRUE) {								// ニアフル状態チェック
				RAUdata_CreateNearFullData((RAU_NEAR_FULL*)&RAU_temp_data[2]);		// 状態変化していたら通知
				RAU_Word2Byte(RAU_temp_data, 32);
				RAU_CrcCcitt(30, 0, &RAU_temp_data[2], (uchar*)NULL, &RAU_temp_data[32], 1 );
				RAUdata_SetThroughData(RAU_temp_data, 34, 1);	// 受信スルーデータバッファ(ＩＢＫ→ＩＢＷ)に登録
			}
		} else {
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	} else if (ucDataId == 0x42) {	// 遠隔ＩＢＫイニシャライズデータ(ＩＢＷ→ＩＢＫ)
		if( RAUhost_GetSndSeqFlag()==RAU_SND_SEQ_FLG_NORMAL &&	// 送信シーケンス０かつ
			RAUhost_GetRcvSeqFlag()==RAU_RCV_SEQ_FLG_NORMAL ){	// 受信シーケンス０のときのみクリア要求を受け付ける。
			RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
			RAUid_TableDataInitialize();									// イニシャライズ
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);					// 受信シーケンスを０(通常状態)にする。
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 			RAU_f_RcvSeqCnt_rq = 0;
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			ret = RAU_DATA_NORMAL;
		} else {
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	} else if (ucDataId == 0x64){	// ＩＤ１００：通信チェック要求データ(ＩＢＷ→ＩＢＫ)
		RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);		// 電文が０カットされている場合、ここでＣＲＣが破壊される。
		RAU_Word2Byte(&RAU_ibw_receive_data[0], 62);	// ＮＴ－ＮＥＴ電文サイズ＋２(フルサイズ)
		RAU_Word2Byte(&RAU_ibw_receive_data[2], 60);	// 共通部＋固有部の(０カット後)データ長
		RAU_CrcCcitt(60, 0, &RAU_ibw_receive_data[2], (uchar*)NULL, &RAU_ibw_receive_data[62], 1 );	// ＣＲＣ再算出
		ret = RAUid_CommuTest((RAU_COMMUNICATION_TEST*)RAU_ibw_receive_data);
	} else if (ucDataId == 0x75){	// ＩＤ１１７：通信チェック結果データ(ＩＢＷ→ＩＢＫ)
		RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
		ret = RAUid_CommuTest((RAU_COMMUNICATION_TEST*)RAU_ibw_receive_data);
	} else {						// それ以外
		if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){		// 受信シーケンス２(ＨＯＳＴからの要求データ受信完了状態)のとき
			ret = RAUdata_SetThroughData(RAU_ibw_receive_data, uiDataLen, 0);	// 送信スルーデータバッファ(ＩＢＫ→ＨＯＳＴ)に登録
		} else {	// 受信シーケンス２(ＨＯＳＴからの要求データ受信完了状態とき以外はＩＢＷからのスルーデータを受け付けない(再送を要求)
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	}

	return ret;
}
/*[]----------------------------------------------------------------------[]*
 *|             RAUid_ThroughDataAnalyze()
 *[]----------------------------------------------------------------------[]*
 *|	ＩＢＫ内で処理するスルーデータを分析する｡
 *[]----------------------------------------------------------------------[]*
 *|	patam		なし
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET RAUid_HostThroughDataAnalyze(uchar ucDataId, ushort uiDataLen)
{
	eRAU_TABLEDATA_SET	ret;
	RAU_SEND_REQUEST	*pSendReq;

	// ＨＯＳＴから受信したシステムＩＤ３０(遠隔固有電文)
	if( ucDataId == 0x51 ){	// 送信要求データ
		if( RAUhost_GetSndSeqFlag()==RAU_SND_SEQ_FLG_NORMAL ){	// 送信シーケンス０のとき許可
			RAUid_Unpack0Cut(&RAU_host_receive_data[2]);
			pSendReq = (RAU_SEND_REQUEST*)&RAU_host_receive_data[2];
			pSendReq->turi_manage = 0;							// ホストからの釣銭管理送信要求は行わない
			RAUid_SendRequest((RAU_SEND_REQUEST*)&RAU_host_receive_data[2]);	// ホスト部に送信要求フラグを設定
			ret = RAU_DATA_NORMAL;
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);					// ＨＯＳＴからの送信要求に対してＩＢＫが独自に対応するのでシーケンスを０に変更
		} else {	// 送信シーケンス１(ＨＯＳＴへのテーブル送信開始状態)のときはＮＡＫ９０をＨＯＳＴに返す。
			ret = RAU_DATA_CONNECTING_FROM_HOST;
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);					// 受信シーケンスを０に変更
		}
	} else if (ucDataId == 0x65){	// ＩＤ１０１：通信チェック結果データ(ＨＯＳＴ→ＩＢＫ)
		ret = RAUid_CommuTest((RAU_COMMUNICATION_TEST*)RAU_host_receive_data);
	} else if (ucDataId == 0x74){	// ＩＤ１１６：通信チェック要求データ(ＨＯＳＴ→ＩＢＫ)
		RAUid_Unpack0Cut(&RAU_host_receive_data[2]);	// 電文が０カットされている場合、ここでＣＲＣが破壊される。
		RAU_Word2Byte(&RAU_host_receive_data[0], 62);	// ＮＴ－ＮＥＴ電文サイズ＋２(フルサイズ)
		RAU_Word2Byte(&RAU_host_receive_data[2], 60);	// 共通部＋固有部の(０カット後)データ長
		RAU_CrcCcitt(60, 0, &RAU_host_receive_data[2], (uchar*)NULL, &RAU_host_receive_data[62], 1 );	// ＣＲＣ再算出
		ret = RAUid_CommuTest((RAU_COMMUNICATION_TEST*)RAU_host_receive_data);
	} else {				// それ以外
		ret = RAUdata_SetThroughData(RAU_host_receive_data, uiDataLen, 1);	// スルーデータバッファに登録
		if( ret == RAU_DATA_NORMAL ){									// 登録成功
			if( RAUid_RcvSeqCnt_Start(0x1E, ucDataId) ){				// 受信シーケンスキャンセルタイマの起動(ＩＢＷからの応答電文のあるデータ)
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_THROUGH_RCV);		// ＨＯＳＴからの要求データを登録したので受信シーケンスを２に変更
			} else {													// ＩＢＷからの応答電文のないデータ
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);			// ＨＯＳＴからの要求データを登録したので受信シーケンスを０に変更
// MH810105(S) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
// 				RAU_f_RcvSeqCnt_rq = 0;									// 受信シーケンスキャンセルタイマ動作許可(解除)
// MH810105(E) R.Endo 2021/12/07 車番チケットレス3.0 #6191 【現場指摘】NT-NET送信要求データ受信から5分後にE7210発生/解除 [共通改善項目 No1524]
			}
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
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_IbkControl(RAU_CONTROL_REQUEST *pCtrlReq) {

	// 各種要求フラグをチェック（仕様上同時に複数のフラグが設定されることはないので、上から順次チェックする｡）
	eRAU_TABLEDATA_SET ret;

	if(pCtrlReq->table_data_count == 1){										// テーブル件数データ要求
		RAUdata_CreatTableDataCount((RAU_TABLE_DATA_COUNT*)&RAU_temp_data[2]);		// システムＩＤ３０, データＩＤ８３
		RAU_Word2Byte(RAU_temp_data, 50);											// 先頭２バイトに１ＮＴ－ＮＥＴ電文(≠ブロックではない)の長さ＋２をセット(５０バイト)
		RAU_CrcCcitt(48, 0, &RAU_temp_data[2], (uchar*)NULL, &RAU_temp_data[50], 1 );	// 終端２バイトにＣＲＣを算出してセット(先頭２バイト部分の長さには含まれていない)
		ret = RAUdata_SetThroughData(RAU_temp_data, 52, 1);							// 受信スルーデータバッファ(ＩＢＫ→ＩＢＷ)に登録
	} else {																	// 既知の要求フラグが確認できなかった
		ret = RAU_DATA_BUFFERFULL;
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
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUid_SendRequest(RAU_SEND_REQUEST *pSendReq) {

	RAUhost_SetSendRequeat(pSendReq);
}


/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	RAUid_TableDataClear(RAU_CLEAR_REQUEST *pClearReq) {

//	data_table_init();

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(RT精算データ対応)
//	// 入庫データ			（ID20）
//	if (pClearReq->in_parking == 1) {
//		RAUdata_TableClear(20);
//		Ope_Log_UnreadToRead(eLOG_PAYMENT, eLOG_TARGET_REMOTE);			// 入庫
//	}
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(RT精算データ対応)
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	// 出庫データ			（ID21）
//	if (pClearReq->out_parking == 1) {
//		RAUdata_TableClear(21);
//	}
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	// 精算データ			（ID22 23）
	if (pClearReq->paid_data == 1) {
		RAUdata_TableClear(22);
		RAUdata_TableClear(23);
		Ope_Log_UnreadToRead(eLOG_PAYMENT, eLOG_TARGET_REMOTE);			// 精算
	}
	// T合計集計データ		（ID30～38・41）
	if (pClearReq->total == 1) {
		if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			RAUdata_TableClear(53);
			Ope_Log_UnreadToRead(eLOG_TTOTAL, eLOG_TARGET_REMOTE);		// T集計
			Ope_Log_UnreadToRead(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);		// GT集計
		}
		else {
			RAUdata_TableClear(41);
			Ope_Log_UnreadToRead(eLOG_TTOTAL, eLOG_TARGET_REMOTE);			// T集計
		}
	}
	// エラーデータ			（ID120）
	if (pClearReq->error == 1) {
		RAUdata_TableClear(120);
		Ope_Log_UnreadToRead(eLOG_ERROR, eLOG_TARGET_REMOTE);			// ｴﾗｰ
	}
	// アラームデータ		（ID121）
	if (pClearReq->alarm == 1) {
		RAUdata_TableClear(121);
		Ope_Log_UnreadToRead(eLOG_ALARM, eLOG_TARGET_REMOTE);			// ｱﾗｰﾑ
	}
	// モニターデータ		（ID122）
	if (pClearReq->monitor == 1) {
		RAUdata_TableClear(122);
		Ope_Log_UnreadToRead(eLOG_MONITOR, eLOG_TARGET_REMOTE);			// ﾓﾆﾀ
	}
	// 操作モニターデータ	（ID123）
	if (pClearReq->ope_monitor == 1) {
		RAUdata_TableClear(123);
		Ope_Log_UnreadToRead(eLOG_OPERATE, eLOG_TARGET_REMOTE);			// 操作ﾓﾆﾀ
	}
	// コイン金庫集計データ	（ID131）
	if (pClearReq->coin_total == 1) {
		RAUdata_TableClear(131);
		Ope_Log_UnreadToRead(eLOG_COINBOX, eLOG_TARGET_REMOTE);			// ｺｲﾝ金庫集計	(ramのみ)
	}
	// 紙幣金庫集計データ	（ID133）
	if (pClearReq->money_toral == 1) {
		RAUdata_TableClear(133);
		Ope_Log_UnreadToRead(eLOG_NOTEBOX, eLOG_TARGET_REMOTE);			// 紙幣金庫集計	(ramのみ)
	}
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	// 駐車台数・満車データ	（ID236）
//	if (pClearReq->parking_num == 1) {
//		RAUdata_TableClear(236);
//		Ope_Log_UnreadToRead(eLOG_PARKING, eLOG_TARGET_REMOTE);			// 駐車台数ﾃﾞｰﾀ
//	}
//	// 区画台数・満車データ	（ID237）
//	if (pClearReq->area_num == 1) {
//		RAUdata_TableClear(237);
//	}
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	// 金銭管理データ		（ID126）
	if (pClearReq->money_manage == 1) {
		RAUdata_TableClear(126);
		Ope_Log_UnreadToRead(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);		// 金銭管理		(ramのみ)
	}
	// 釣銭管理集計データ	（ID135）
	if (pClearReq->turi_manage == 1) {
		RAUdata_TableClear(135);
		Ope_Log_UnreadToRead(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);		// 釣銭管理		(ramのみ)
	}
	// 遠隔監視データ		（ID125）
	if (pClearReq->rmon == 1) {
		RAUdata_TableClear(125);
		Ope_Log_UnreadToRead(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);	// 遠隔監視データ
	}
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	// 長期駐車状態データ		（ID61）
//	if (pClearReq->long_park == 1) {
//		RAUdata_TableClear(61);
//		Ope_Log_UnreadToRead(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);	// 長期駐車状態データ
//	}
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	return TRUE;
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
BOOL RAUid_TableDataInitialize( void )
{
	int		i;
	short	NearFullStatus[RAU_TABLE_MAX];

	for( i=0; i<RAU_TABLE_MAX; i++ ){
		NearFullStatus[i] = rau_data_table_info[i].i_NearFullStatus;		// クリアする前のバッファ状態を取得
	}

	RAU_data_table_init();	// 全テーブル初期化(i_NearFullStatus, i_NearFullStatusBefore 共に解除状態)

	for( i=0; i<RAU_TABLE_MAX; i++ ){
		rau_data_table_info[i].i_NearFullStatusBefore = NearFullStatus[i];	// 前回のバッファ状態とする。
	}

	return TRUE;
}

//==============================================================================
//	通信チェックデータの受信処理
//
//	@argument	*pNTdata	通信チェック要求データ or 結果データ
//
//	@return		RAU_DATA_NORMAL					成功
//	@return		RAU_DATA_BLOCK_INVALID1			ブロックナンバー異常によりデータ破棄(今回無効)
//	@return		RAU_DATA_BLOCK_INVALID2			ブロックナンバー異常によりデータ破棄(今回有効)
//	@return		RAU_DATA_CONNECTING_FROM_HOST	ホストからの接続中のため受け付け不可(データ送信要求)
//	@return		RAU_DATA_BUFFERFULL				バッファが一杯
//
//	@note		ＨＯＳＴとＩＢＷから送信されてくる通信チェック要求データまたは
//				結果データを解析し適切なバッファに登録する。
//
//	@author		2007.02.13:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
eRAU_TABLEDATA_SET RAUid_CommuTest(RAU_COMMUNICATION_TEST *pNTdata)
{
	eRAU_TABLEDATA_SET		ret;
	RAU_COMMUNICATION_TEST	*pBuff;
	uchar					Direction = 0;
	uchar					ResultCode;

	if(KSG_gPpp_RauStarted) {							// PPP確立中
		switch( pNTdata->id4 ){							// 受信した電文
		case 100:										// ＩＢＷから受信した通信チェック要求データ
			pBuff = &RemoteCheck_Buffer100_101;
			if( pNTdata->Re.quest.Kind == 0 ){			// 要求種別(チェック開始)
				if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ){
					if( RAUhost_GetSndSeqFlag() == RAU_SND_SEQ_FLG_NORMAL ){	// 上下回線未使用
						if( pNTdata->Re.quest.TimeOut != 0 ){
							RAU_Tm_CommuTest.bits0.bit_0_13 = ((int)~pNTdata->Re.quest.TimeOut)+1;
							RAU_f_CommuTest_rq = 1;		// 通信チェック用タイマ起動
							RAU_f_CommuTest_ov = 0;
						}
						*pBuff = *pNTdata;
						ResultCode = 0;
						RauCT_SndReqID = pNTdata->id4;	// 通信チェックデータ(ＩＤ１００)送信要求セット
						Direction = 0;					// 送信方向(ＨＯＳＴ)
						ret = RAU_DATA_NORMAL;			// ＩＢＷにはＡＲＣブロック"受信結果"正常(0x00)を返す。
					} else {							// 上り回線のみ使用中
						ResultCode = 2;					// 結果コード(回線使用中)
						Direction = 1;					// 送信方向(ＩＢＷ)
					}
				} else {								// 下り回線使用中
					ResultCode = 7;						// 結果コード(回線使用中)
					Direction = 1;						// 送信方向(ＩＢＷ)
				}
			} else {									// 要求種別(チェック中止)
				if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ){	// 下り回線未使用
					if( pNTdata->CheckNo == pBuff->CheckNo ){				// チェック№一致
						RAU_Tm_CommuTest.tm = 0;		// 通信チェックタイマタイマ(停止)
						if( RauCtrl.TCPcnct_req == 1 ){	// 上り回線ＴＣＰコネクション要求フラグ(オン状態)
							ResultCode = 4;				// 結果コード(接続中)
						} else {						// 上り回線のＴＣＰコネクションは一度確立済み
							ResultCode = 6;				// 結果コード(応答待ち)
						}
					} else {
						ResultCode = 1;					// 結果コード(未チェック)
					}
				} else {								// 下り回線使用中
					ret = RAU_DATA_CONNECTING_FROM_HOST;// ＩＢＷにはＡＲＣブロック"受信結果"データ破棄(0x90)を返す。
					ResultCode = 0;
				}
				Direction = 1;							// 送信方向(ＩＢＷ)
			}
			break;
		case 116:										// ＨＯＳＴから受信した通信チェック要求データ
			pBuff = &RemoteCheck_Buffer116_117;
			if( pNTdata->Re.quest.Kind == 0 ){			// 要求種別(チェック開始)
				if( pNTdata->Re.quest.TimeOut != 0 ){
					RAU_Tm_CommuTest.bits0.bit_0_13 = ((int)~pNTdata->Re.quest.TimeOut)+1;
					RAU_f_CommuTest_rq = 1;				// 通信チェック用タイマ起動
					RAU_f_CommuTest_ov = 0;
				}
				*pBuff = *pNTdata;
				Direction = 1;							// 送信方向(ＩＢＷ)
				RAUdata_SetThroughData((uchar *)pBuff, 64, Direction);	// 受信スルーデータバッファ(ＩＢＫ→ＩＢＷ)に登録
				ResultCode = 0;
				ret = RAU_DATA_NORMAL;					// ＨＯＳＴにはＡＣＫを返す。
			} else {									// 要求種別(チェック中止)
				if( pNTdata->CheckNo == pBuff->CheckNo ){	// チェック№一致
					RAU_Tm_CommuTest.tm = 0;			// 通信チェックタイマタイマ(停止)
					memset(pBuff, 0, 64);				// バッファクリア
					RauCT_SndReqID = 0;					// 通信チェックデータ(ＩＤ１１６)送信要求クリア
					ResultCode = 6;						// 結果コード(応答待ち)
				} else {
					ResultCode = 1;						// 結果コード(未チェック)
				}
				Direction = 0;							// 送信方向(ＨＯＳＴ)
			}
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ＨＯＳＴからの要求データを処理したので受信シーケンスを０に変更
			break;
		default:
			if( pNTdata->id4 == 101 ){					// ＨＯＳＴから受信した通信チェック結果データ
				pBuff = &RemoteCheck_Buffer100_101;
				Direction = 1;							// 送信方向(ＩＢＷ)
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ＨＯＳＴからの結果データを処理するので受信シーケンスを０に変更
			} else {
				pBuff = &RemoteCheck_Buffer116_117;
				Direction = 0;							// 送信方向(ＨＯＳＴ)
			}
			if( pNTdata->CheckNo == pBuff->CheckNo ){	// チェック№一致
				RAU_Tm_CommuTest.tm = 0;				// 通信チェックタイマタイマ(停止)
				if( Direction == 1 ){					// 送信方向(ＩＢＷ)
					ret = RAUdata_SetThroughData((uchar *)pNTdata, 64, Direction);	// 受信スルーデータバッファ(ＩＢＫ→ＩＢＷ)に登録
					if( ret == RAU_DATA_NORMAL ){
						memset(pBuff, 0, 64);
						RauCT_SndReqID = 0;				// 通信チェックデータ送信要求クリア
					}
				} else {								// 送信方向(ＨＯＳＴ)
					*pBuff = *pNTdata;
					RauCT_SndReqID = pNTdata->id4;		// 通信チェックデータ(ＩＤ１１７)送信要求セット
					ret = RAU_DATA_NORMAL;				// ＩＢＷにはＡＲＣブロック"受信結果"正常(0x00)を返す。
				}
			} else {
				ret = RAU_DATA_NORMAL;					// 読み捨ててチェック№が一致するデータを待つ。
			}
			ResultCode = 0;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
	} else {	// ＤｏＰａ通信端末またはＸＰｏｒｔがオンライン状態になっていない
		if( pNTdata->id4 == 100 ){
			pBuff = &RemoteCheck_Buffer100_101;
		} else {
			pBuff = &RemoteCheck_Buffer116_117;
		}
		Direction = 1;									// 送信方向(ＩＢＷ)
		if(RauConfig.modem_exist == 0) {				// モデムあり
			if( KSG_mdm_status < 2 ){					// DTE確立前
				ResultCode = 11;						// 11:FOMAモジュール接続不良
			}
			else if( KSG_mdm_status < 3 ){				// 未接続
				ResultCode = 12;						// 12:ダイアル発信失敗
			}
			else if(KSG_gPpp_RauStarted == 0) {
				ResultCode = 13;						// 13:PPP未接続
			}
		}
		else {											// LAN設定
			// LAN設定の場合ここは通らないはず
			ResultCode = 3;								// 結果コード(接続不能)
		}

	}

	if( ResultCode != 0 ){	// "通信テストＯＫ"以外は通信チェック結果[ＮＧ]データを出力
		*pBuff = *pNTdata;
		pBuff->Re.quest.Kind = 0;
		pBuff->Re.quest.TimeOut = 0;
		pBuff->NTpacket_len[1] = 62;					// 先頭２バイトは(先頭２バイトを含め)ＣＲＣ記入部を除いた電文長
		pBuff->data_len[1] = 60;						// データ部を０カットした長さ＋ヘッダ部の１０バイト
		pBuff->data_len_zero_cut[1] = 50;				// ０カットする前のデータ部の長さ
		pBuff->id4 = pNTdata->id4 + 1;					// 通信チェック結果[ＮＧ]データ
		pBuff->Re.sult.Code = ResultCode;				// 結果コード
		RAU_CrcCcitt(60, 0, &pBuff->data_len[0], (uchar*)NULL, pBuff->crc, 1 );	// 末尾２バイトにＣＲＣを算出して記入
		if( Direction == 0 ){							// 送信方向(ＨＯＳＴ)
			RauCT_SndReqID = pBuff->id4;				// 通信チェック結果[ＮＧ]データ(１１７)送信要求セット
		} else {										// 送信方向(ＩＢＷ)
			ret = RAUdata_SetThroughData((uchar *)pBuff, 64, Direction);	// スルーデータバッファに登録
			memset(pBuff, 0, 64);						// これでチェック№が０になり通信チェックデータ送信要求(RauCT_SndReqID)が０でないので中断となる。
		}
	}

	return ret;
}

//==============================================================================
//	通信チェックのエラー処理
//
//	@argument	void
//
//	@return		void
//
//	@attention	通信チェックタイマのオーバフローフラグが関数コールの条件です。
//
//	@note		通信チェック中のタイムアウトまたは通信障害が発生するとコールされ
//				ＩＢＷまたはＨＯＳＴへ通信チェック結果[ＮＧ]データを送信する。
//
//	@author		2007.02.13:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
void RAUid_CommuTest_Error(void)
{
	RAU_COMMUNICATION_TEST	*pBuff;
	uchar					Direction;

	if( RemoteCheck_Buffer100_101.CheckNo != 0 ){			// 通信チェック要求データをＩＢＷから受け付けている。
		pBuff = &RemoteCheck_Buffer100_101;
		Direction = 1;										// 送信方向(ＩＢＷ)
	} else if( RemoteCheck_Buffer116_117.CheckNo != 0 ){	// 通信チェック要求データをＨＯＳＴから受け付けている。
		pBuff = &RemoteCheck_Buffer116_117;
		Direction = 0;										// 送信方向(ＨＯＳＴ)
	} else {
		return;
	}
	pBuff->Re.quest.Kind = 0;
	pBuff->Re.quest.TimeOut = 0;
	pBuff->NTpacket_len[1] = 62;					// 先頭２バイトは(先頭２バイトを含め)ＣＲＣ記入部を除いた電文長
	pBuff->data_len[1] = 60;						// データ部を０カットした長さ＋ヘッダ部の１０バイト
	pBuff->data_len_zero_cut[1] = 50;				// ０カットする前のデータ部の長さ
	pBuff->id4 = pBuff->id4 + 1;					// 通信チェック結果[ＮＧ]データ
	if( pBuff->Re.sult.Code == 0){
		if( (Direction == 1)&&(RauCtrl.TCPcnct_req == 1) ){
			pBuff->Re.sult.Code = 4;				// 結果コードを(接続中)とする。
		} else {
			pBuff->Re.sult.Code = 6;				// 結果コード(応答待ち)
		}
	}
	RAU_CrcCcitt(60, 0, &pBuff->data_len[0], (uchar*)NULL, pBuff->crc, 1 );	// 末尾２バイトにＣＲＣを算出して記入
	if( Direction == 0 ){							// 送信方向(ＨＯＳＴ)
		if( RAUhost_GetSndSeqFlag() == RAU_SND_SEQ_FLG_NORMAL ){	// 上り回線未使用
			RauCT_SndReqID = pBuff->id4;			// 通信チェック結果[ＮＧ]データ(ＩＤ１１７)送信要求セット
		} else {
			memset(pBuff, 0, 64);
			RauCT_SndReqID = 0;						// 通信チェックデータ送信要求クリア
		}
	} else {										// 送信方向(ＩＢＷ)
		if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ){	// 下り回線未使用
			RAUdata_SetThroughData((uchar *)pBuff, 64, Direction);	// スルーデータバッファに登録
		}
		memset(pBuff, 0, 64);	// これでチェック№が０になり通信チェックデータ送信要求(RauCT_SndReqID)が０でないので中断扱いとなる。
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_CheckRcvDopaData()
 *[]----------------------------------------------------------------------[]*
 *|	受信したDopaパケットのNTデータとしての正当性をチェックする｡
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	TRUE	正常
 *|	return	FALSE	異常
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAUid_CheckRcvDopaData(uchar *pData, ushort uiDataLen) {

	ushort uiPos;
	uchar	i;
	uchar	ucLastBlkNo;
	BOOL ret;
	union {
		uchar	uc[2];
		ushort	us;
	}u;

	uiPos = 0;
	ucLastBlkNo = 0;
	ret = FALSE;

	for (i = 0; i < RAU_RECEIVE_NTBLKNUM_MAX; i++) {	// 26ブロック分チェックして最終ブロックが見つからなければ、異常。
		u.uc[0] =  pData[uiPos];
		u.uc[1] =  pData[uiPos + 1];

		if (10 <= u.us && u.us <= 970) {				// 長さチェック
			// 1ブロックあたりの長さが10以上970以下であれば正常｡

			if (ucLastBlkNo + 1 == pData[uiPos + 5]) {	// ブロックNo.チェック
				// 前回のブロックNO.＋１と一致しなければ、異常｡
				// ブロックNo.をインクリメント
				ucLastBlkNo++;
			} else {
				break;
			}

			if (pData[uiPos + 6] == 0x01) {				// 最終ブロックチェック
				// 26ブロック以内に最終ブロックが見つかれば正常｡
				ret = TRUE;
				break;
			}

			// 次ブロック先頭位置を格納
			uiPos += u.us;
		} else {
			break;
		}
	}
	return ret;
}

//==============================================================================
//	受信シーケンスキャンセルタイマの起動
//
//	@argument	ucSystemId		システムＩＤ
//	@argument	ucDataId		データＩＤ
//
//	@return		TRUE			タイマ起動あり(ＩＢＷからの応答電文があるもの)
//	@return		FALSE			タイマ起動なし(ＩＢＷからの応答電文がないもの)
//
//	@note		ＨＯＳＴから受信する遠隔システム電文でＩＢＷからの応答電文の
//				あるものの場合ＩＢＷからの応答を待つタイマを起動する。
//
//	@see		RAUid_GetRcvSeqFlagTable
//	@see		RAUid_GetRcvSeqFlagRemote
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
BOOL RAUid_RcvSeqCnt_Start(uchar ucSystemId, uchar ucDataId)
{
	int	i;

	switch (ucSystemId) {
	case 30:											// システムＩＤが３０(遠隔固有電文)
		for( i = 0; i < sizeof(RAU_RemoteDataTbl); i++ ){
			if( RAU_RemoteDataTbl[i] == ucDataId ){
				RAU_f_RcvSeqCnt_rq = 1;					// 受信シーケンスキャンセルタイマ動作許可(起動)
				RAU_Tm_RcvSeqCnt = 0;					// タイマ値クリア
				return TRUE;
			}
		}
		break;
	case 40:											// システムＩＤが４０
		// システムID 40のデータで応答が必要な電文を受信する場合、ucDataIdをチェックすること
		break;
// MH322914(S) K.Onodera 2016/08/08 AI-V対応
	case PIP_SYSTEM:
		for( i = 0; i < sizeof(RAU_TableDataTbl_PIP); i++ ){
			if( RAU_TableDataTbl_PIP[i] == ucDataId ){
				RAU_f_RcvSeqCnt_rq = 1;				// 受信シーケンスキャンセルタイマ動作許可(起動)
				RAU_Tm_RcvSeqCnt = 0;				// タイマ値クリア
				return TRUE;
			}
		}
		break;
// MH322914(E) K.Onodera 2016/08/08 AI-V対応
	default:
		if( 10 <= ucSystemId && ucSystemId <= 19 ){		// システムＩＤが１０～１９(遠隔システム電文)
			for( i = 0; i < sizeof(RAU_TableDataTbl); i++ ){
				if( RAU_TableDataTbl[i] == ucDataId ){
					RAU_f_RcvSeqCnt_rq = 1;				// 受信シーケンスキャンセルタイマ動作許可(起動)
					RAU_Tm_RcvSeqCnt = 0;				// タイマ値クリア
					return TRUE;
				}
			}
		}
		break;
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_Unpack0Cut()
 *[]----------------------------------------------------------------------[]*
 *|	0カットされたデータを復元する｡
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUid_Unpack0Cut(uchar *ucData) {

	ushort uiBefore0Cut;
	ushort uiAfter0Cut;

	union {
		uchar	uc[2];
		ushort	us;
	}u;

	u.uc[0] = ucData[0];
	u.uc[1] = ucData[1];

	// 0カット後のサイズを取得
	uiAfter0Cut = u.us - NT_DOPANT_HEAD_SIZE;

	u.uc[0] = ucData[2];
	u.uc[1] = ucData[3];

	// 0カット前のサイズを取得
	uiBefore0Cut = u.us;

	while (uiBefore0Cut > uiAfter0Cut) {
		ucData[uiAfter0Cut + NT_DOPANT_HEAD_SIZE] = 0x00;
		uiAfter0Cut++;
	}
}

//==============================================================================
//	通信チェックのエラー処理
//
//	@argument	void
//
//	@return		void
//
//	@attention	ダミーデータによる通信テスト結果をOPE送信する
//
//	@note		通信チェック時のダミーデータの応答受信時のコールする
//======================================== Copyright(C) 2012 AMANO Corp. =======
void RAUid_CommuTest_Result(void)
{
	RAU_COMMUNICATION_TEST	*pBuff;

	if( RemoteCheck_Buffer100_101.CheckNo == 0 ){	// 通信チェック要求データを受け付けていない
		return;
	}
	pBuff = &RemoteCheck_Buffer100_101;

	RAU_Tm_CommuTest.tm = 0;						// 通信チェックタイマタイマ(停止)
	
	pBuff->Re.quest.Kind = 0;
	pBuff->Re.quest.TimeOut = 0;
	pBuff->NTpacket_len[1] = 62;					// 先頭２バイトは(先頭２バイトを含め)ＣＲＣ記入部を除いた電文長
	pBuff->data_len[1] = 60;						// データ部を０カットした長さ＋ヘッダ部の１０バイト
	pBuff->data_len_zero_cut[1] = 50;				// ０カットする前のデータ部の長さ
	pBuff->id4 = pBuff->id4 + 1;					// 通信チェック結果[ＮＧ]データ
	pBuff->Re.sult.Code = 0;

	RAU_CrcCcitt(60, 0, &pBuff->data_len[0], (uchar*)NULL, pBuff->crc, 1 );	// 末尾２バイトにＣＲＣを算出して記入
	
	if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ){	// 下り回線未使用
		RAUdata_SetThroughData((uchar *)pBuff, 64, 1);	// スルーデータバッファに登録（1:OPE方向）
	}
	RauCT_SndReqID = 0;								// 通信チェックデータ送信要求クリア
	memset(pBuff, 0, 64);
	RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// 送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)
}

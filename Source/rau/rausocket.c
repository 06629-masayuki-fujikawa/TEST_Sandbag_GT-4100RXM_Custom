/****************************************************************************/
/*																			*/
/*		システム名　:　RAUシステム											*/
/*		ファイル名	:  rausocket.c											*/
/*		機能		:  ソケット通信処理										*/
/*																			*/
/****************************************************************************/

#include	"system.h"
#include	"ope_def.h"
#include	"rau.h"
#include	"raudef.h"
#include	"trsocket.h"
#include	"trsntp.h"
#include	"prm_tbl.h"
#include	"message.h"
/*------------------------------------------------------------------------------*/
#pragma	section	_UNINIT3		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:アドレス空間0x06039000-0x060FFFFF(1MB)
RAU_SOCK_CONDITION	RAU_upSockCondition;			// 上り回線ソケット通信状態
RAU_SOCK_CONDITION	RAU_downSockCondition;			// 下り回線ソケット通信状態
RAU_SOCK_CONDITION	RAU_creditSockCondition;			// クレジット回線ソケット通信状態
uchar	RAU_SendBuff[RAU_DATA_MAX + 1];			// 下り回線送信バッファ
uchar	RAU_RecvBuff[RAU_DATA_MAX + 1];			// 下り回線受信バッファ

/*------------------------------------------------------------------------------*/
#pragma	section	
/*------------------------------------------------------------------------------*/
int		RAU_upListenSocket = -1;					// 上り回Listenソケット
int		RAU_upSocket = -1;							// 上り回線ソケット
int		RAU_downListenSocket = -1;					// 下り回Listenソケット
int		RAU_downSocket = -1;						// 下り回線ソケット
uchar	Credit_TcpConnect = 0;							// 1:connect中
int		RAU_creditSocket = -1;						// クレジット回線ソケット
int		RAU_sntpListenSocket = -1;					// SNTPListenソケット
int		socketDesc = TM_SOCKET_ERROR;				// SNTP ソケットの状態
int		StateFlag = TM_SNTP_STATUS_SEND;			// SNTP 状態フラグ
uchar	sntp_start = 0;								// SNTP 時刻補正スタートフラグ
ushort	sntp_rmsg = 0;								// SNTP メッセージ送信先(自動/手動)
static	ttNtpPacket sntpdata;						// SNTP受信パケット
static	ulong transmitTimestamp[2];					// SNTP要求時刻 0:sec積算値 1:msec
uchar	RAU_AntennaLevelState = 0;					// アンテナレベルチェック開始(1)・終了(0)状態
uchar	RAU_TcpConnect = 0;							// 1:connect中
const	char	RAU_dumy_str[RAU_DUMMY_STR_LEN + 1]={0x00,0x00,0x00,0x10,'A','M','A','N','O','_','_','D','U','M','M','Y',0};
uchar	RAU_SendDummyContinueCount = 0;				// ダミーデータ連続送信回数

extern	int		KSG_gPpp_RauStarted;				// PPP接続フラグ
extern	unsigned char	KSG_AntAfterPppStarted;

static	uchar	RAU_Prev_ppp_State;					// 直前のPPP接続フラグ
static	RAU_LINE_STATE	RAU_UpSockProc(void);
static	RAU_LINE_STATE	RAU_DownSockProc(void);
char	leapyear_check( short pa_yer );
void	RAU_SNTPProc(void);
static	int		RAU_CreateUpSocket(void);
static	int		RAU_CreateDownSocket(void);
static	int		RAU_ConnectUpline(void);
static	void	RAU_CloseUpline(BOOL resetReq);
static	void	RAU_CloseDownline(void);
static	uchar	DpaSndQue_Read(uchar *pData, ushort *len, uchar kind, int port);
static	void RAU_DpaRcvQue_Set(uchar *pData, ushort len, int port);
void 	RAU_upSocketCallback(int sock, int event);
void 	RAU_downSocketCallback(int sock, int event);
static	RAU_LINE_STATE	RAU_CreditSockProc(void);
static	int RAU_CreateCreditSocket(void);
static	int	RAU_ConnectCreditline(void);
static	void	RAU_CloseCreditline(BOOL resetReq);
void 	RAU_creditSocketCallback(int sock, int event);
static	void	RAU_ClearLineState(void);

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SockMain
 *[]----------------------------------------------------------------------[]
 *| summary	: ソケット処理メイン
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SockMain(void)
{
	RAU_LINE_STATE upState = RAU_LINE_NO_ERROR;		// 上回線処理結果
	RAU_LINE_STATE downState = RAU_LINE_NO_ERROR;	// 下回線処理結果
	RAU_LINE_STATE creState;						// クレジット回線処理結果
	uchar	modemCutState;
	
	if(KSG_gPpp_RauStarted == 0) {					// PPPが切断されている
		RAU_Tm_DISCONNECT.tm = 0;					// コネクション切断待ちタイマ(停止)
		RAU_Tm_No_Action.tm = 0;					// 無通信タイマ(停止)
		RAU_SendDummyContinueCount = 0;				// ダミーデータ連続送信回数クリア
		RAU_Tm_Port_Watchdog.tm = 0;				// 下り回線通信監視タイマ(停止)
	}

	if((RAU_AntennaLevelState == 0) && (KSG_AntAfterPppStarted == 0)){	// アンテナレベル測定中ではない
		if(RAU_Prev_ppp_State == 1 && KSG_gPpp_RauStarted == 0){		// PPPが切断された
			if(RAU_upSocket != -1) {
				RAU_CloseUpline(TRUE);
			}
			
			if(RAU_downSocket != -1) {
				RAU_CloseDownline();
			}
			
			if(RAU_downListenSocket != -1) {
				tfClose(RAU_downListenSocket);
				RAU_downListenSocket = -1;
			}
			if(RAU_creditSocket != -1){
				RAU_CloseCreditline(TRUE);
			}
			// データ再送待ちタイマ起動
			RAU_Tm_data_rty.bits0.bit_0_13 = RauConfig.Dpa_data_rty_tm;	
			RAU_f_data_rty_rq = 1;
			RAU_f_data_rty_ov = 0;
		}
		if(RAU_Prev_ppp_State == 0 && KSG_gPpp_RauStarted == 1) {		// PPPが確立された
			queset(OPETCBNO, CRE_EVT_CONN_PPP, 0, NULL);				// OPEに通知
		}
		RAU_Prev_ppp_State = KSG_gPpp_RauStarted;
	}
	else {
		RAU_Prev_ppp_State = 0;
	}

	if(_is_ntnet_remote()){
	// 上り回線処理
		upState = RAU_UpSockProc();
	
	// 下り回線処理
		downState = RAU_DownSockProc();
	}
	
	creState = RAU_CreditSockProc();
	/* クレジット通信で異常が発生しても上り、下りのコネクションは維持する */
	/* 同様に上り、下りの通信で異常が発生してもクレジットのコネクションは維持する */
	if(creState != RAU_LINE_SEND){
		if(CreditCtrl.TCPdiscnct_req){
			/* TCP切断要求 */
			CreditCtrl.TCPdiscnct_req = 0;
			RAU_CloseCreditline(TRUE);
		}
	}

	if(RauConfig.modem_exist == 0 && 								// モデム接続で
		((upState != RAU_LINE_SEND || downState != RAU_LINE_SEND) && creState != RAU_LINE_SEND) ) {	// 送信データなし
		if(KSG_RauGetMdmCutState() != 0) {							// モデム切断要求あり
			KSG_RauSetMdmCutState(2);								// モデム切断中に変更
			if((RauCtrl.tcpConnectFlag.port.upload == 0 ||
				RauCtrl.tcpConnectFlag.port.download == 0) && RauCtrl.tcpConnectFlag.port.credit == 0){		// TCP未接続
				KSG_RauClosePPPSession();							// PPP切断
				KSG_RauSetMdmCutState(0);
			}
			else {
				// 上下回線のTCPを切断する
				RAU_CloseUpline(TRUE);
				RAU_CloseDownline();
				RAU_CloseCreditline(TRUE);
			}
		}
		else {
			if(RAU_f_No_Action_ov == 1){			// 回線が切断されるのを防ぐために無通信タイマのタイムアウト
				RauCtrl.TCPcnct_req = 1;			// ＴＣＰ接続要求フラグ(接続要求)
				// ダミーデータ"AMANO__DUMMY"を送信する
				if(RauCtrl.tcpConnectFlag.port.upload == 1) {	// 上り回線TCP接続中
					send(RAU_upSocket, RAU_dumy_str, RAU_DUMMY_STR_LEN, 0);
					RauCtrl.DPA_TCP_DummySend = 1;								// ダミーパケット送信フラグ
					RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;	// パケットを送信するので無通信タイマ延長
					RAU_f_No_Action_ov = 0;
					++RAU_SendDummyContinueCount;				// ダミーデータ連続送信回数加算
					if(RAU_SendDummyContinueCount >= 3) {		// 連続してダミーデータが送信され続けているか
						RAU_ClearLineState();					// 上下回線の状態をクリアする
					}
				}
			}
			else {
				if(RauConfig.Dpa_nosnd_tm) {		// 無通信タイマ設定あり
					if(RAU_f_No_Action_rq == 0) {	// 無通信タイマ停止中
						if(KSG_gPpp_RauStarted) {	// PPP接続中
							// 無通信タイマを起動
							RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;
							RAU_f_No_Action_rq = 1;	// 無通信タイマ(起動)
							RAU_f_No_Action_ov = 0;
						}
					}
				}
			}
		}
	}
	else {
		if(upState != RAU_LINE_SEND || downState != RAU_LINE_SEND) {	// 　上下回線とも送信データなし
			modemCutState = KSG_RauGetMdmCutState();
			if(modemCutState == 1) {					// モデム切断要求あり
				KSG_RauSetMdmCutState(2);				// モデム切断中に変更
				// 上下回線のTCPを切断する
				if(RauCtrl.tcpConnectFlag.port.upload == 1) {
					RAU_CloseUpline(TRUE);
				}
				if(RauCtrl.tcpConnectFlag.port.download == 1) {	// TCP未接続
					RAU_CloseDownline();
				}
				if(RauCtrl.tcpConnectFlag.port.credit == 1) {		// クレジット回線接続中
					RAU_CloseCreditline(TRUE);
				}
			}
			else if(modemCutState == 2) {
				KSG_RauSetMdmCutState(0);
			}
		}
	}
	RAU_SNTPProc();				// SNTP時刻同期処理

}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SockInit
 *[]----------------------------------------------------------------------[]
 *| summary	: ソケット処理初期化処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SockInit(void)
{
	RAU_upSockCondition = RAU_SOCK_INIT;			// 上り回線ソケット通信状態
	RAU_downSockCondition = RAU_SOCK_INIT;			// 下り回線ソケット通信状態
	RAU_upSocket = -1;								// 上り回線ソケット
	RAU_downSocket = -1;							// 下り回線ソケット
	SetUpNetState(NET_STA_IDLE);
	SetDownNetState(NET_STA_IDLE);
	RAU_creditSockCondition = RAU_SOCK_INIT;
	RAU_creditSocket = -1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_UpSockProc
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線ソケット処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	RAU_LINE_STATE	RAU_UpSockProc(void)
{
	int ret;
	int		sockError;
	ushort	sendLen;										// 送信データ長
	RAU_LINE_STATE state = RAU_LINE_NO_ERROR;

	switch(RAU_upSockCondition) {
	case RAU_SOCK_INIT:										// 初期化
		if(KSG_gPpp_RauStarted) {							// PPP接続中？
			RAU_upSockCondition = RAU_SOCK_OPEN;
			RauCtrl.connRetryCount = 0;						// リトライ回数クリア
		}
		break;
	case RAU_SOCK_OPEN:										// ソケットオープン
		if(RAU_AntennaLevelState) {							// アンテナレベルチェック中
			RAU_upSockCondition = RAU_SOCK_CLOSED;
			RauCtrl.connRetryCount = 0;						// リトライ回数クリア
			break;
		}
		if(KSG_gPpp_RauStarted) {
			if(RAU_upSocket != -1) {
				tfClose(RAU_upSocket);
				RAU_upSocket = -1;
				RAUhost_TcpConnReq_Clear();
			}
			
			if(RauCtrl.TCPcnct_req == 1) {					// TCP接続要求あり
				if(RAU_f_data_rty_ov) {						// データ再送待ちタイマがタイムアウトするまでは接続を開始しない。
				RAU_upSocket = RAU_CreateUpSocket();		// 上り回線ソケット作成
				if(RAU_upSocket != -1) {
					if(-1 != RAU_ConnectUpline()) {
						// TCP接続開始
						RAU_upSockCondition = RAU_SOCK_CONNECTING;
					}
					else {
						tfClose(RAU_upSocket);
						RAU_upSockCondition = RAU_SOCK_CLOSING;
					}
				}
				}
				else {
					RAUhost_TcpConnReq_Clear();
					RauCtrl.TCPcnct_req = 0;				// 接続要求をクリアする
					RauCtrl.connRetryCount = 0;				// リトライ回数クリア
				}
			}
			else {
				RauCtrl.connRetryCount = 0;					// リトライ回数クリア
			}
		}
		else {
			RauCtrl.connRetryCount = 0;						// リトライ回数クリア
		}
		break;
	case RAU_SOCK_CONNECTING:								// TCP確立中
		// 何もしない
		break;
	case RAU_SOCK_CONNECTED:								// TCP確立
		RAU_upSockCondition = RAU_SOCK_IDLE;
		RauCtrl.tcpConnectFlag.port.upload = 1;
		break;
	case RAU_SOCK_LISTENING:								// TCP接続待ち
		break;
	case RAU_SOCK_IDLE:										// TCP接続中
		if(RAU_upSocket != -1) {							// ソケット有効（TCP接続中）
			RauCtrl.TCPcnct_req = 0;						// 既に接続中なのでTCP接続要求をクリア

			// 切断要求があればセッションクローズ
			if(RAU_f_TCPdiscnct_ov == 1) {
				RAU_f_TCPdiscnct_ov = 0;
				RAU_CloseUpline(TRUE);
				break;
			}
			
			// 送信処理
			if(DpaSndQue_Read(RAU_SendBuff, &sendLen, 0, UPLINE)) {	// 送信データあり？
				ret = send(RAU_upSocket, (char*)RAU_SendBuff, (int)sendLen, 0);
				if(ret == -1) {
					sockError = tfGetSocketError(RAU_upSocket);
					if(sockError == TM_EWOULDBLOCK) {		// 受信データなし(ノンブロッキングのためエラーではない)
						;
					}
					else {
						RAU_CloseUpline(TRUE);
						break;
					}
				}
				else if(ret > RAU_DATA_MAX) {
					RAUhost_SetError(ERR_RAU_DPA_SEND_LEN);	// 上り回線送信データ長異常(92)
					RAU_CloseUpline(TRUE);
					break;
				}
				state = RAU_LINE_SEND;
				
				// 無通信タイマ起動中なら、送信したためタイマを延長する
				if( RAU_f_No_Action_rq == 1 ){
					RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;
					RAU_f_No_Action_ov = 0;
					RAU_SendDummyContinueCount = 0;			// ダミーデータ連続送信回数クリア
				}
			}
			
			// 受信処理
// MH322914(S) S.Takahashi 2017/02/16 上りデータパケット結合対策
//			ret = recv(RAU_upSocket, (char*)RAU_RecvBuff, RAU_DATA_MAX, 0);	// データ受信
			ret = recv(RAU_upSocket, (char*)RAU_RecvBuff, RAU_NET_RES_LENGTH_MAX, 0);	// データ受信
// MH322914(E) S.Takahashi 2017/02/16 上りデータパケット結合対策
			if(ret > 0) {									// 受信データあり
// MH322914(S) S.Takahashi 2017/02/16 上りデータパケット結合対策
//				if(ret > RAU_NET_RES_LENGTH_MAX) {
//					RAUhost_SetError(ERR_RAU_DPA_RECV_LEN);	// 上り回線受信データ長異常(93)
//					ret = RAU_DATA_MAX;
//				}
// MH322914(E) S.Takahashi 2017/02/16 上りデータパケット結合対策
				// 受信データを格納する
				RAU_DpaRcvQue_Set(RAU_RecvBuff, (ushort)ret, UPLINE);
			}
			else if(ret == -1) {							// エラー発生?
				sockError = tfGetSocketError(RAU_upSocket);
				if(sockError == TM_EWOULDBLOCK) {			// 受信データなし(ノンブロッキングのためエラーではない)
					;
				}
				else {										// エラー発生
					RAU_CloseUpline(TRUE);
				}
			}
		}
		break;
	case RAU_SOCK_CLOSING:									// TCP切断中
		// 何もしない
		if(RAU_upSocket == -1) {
			RAU_upSockCondition = RAU_SOCK_CLOSED;
		}
		break;
	case RAU_SOCK_CLOSED:									// TCP切断
		RauCtrl.tcpConnectFlag.port.upload = 0;
		if(KSG_gPpp_RauStarted) {
			RAU_upSockCondition = RAU_SOCK_OPEN;
		}
		else if(RAU_AntennaLevelState == 1) {				// アンテナレベルチェック中
			;												// 何もしない(RAU_SOCK_CLOSEDのまま)
		}
		else {
			RAU_upSockCondition = RAU_SOCK_INIT;
		}
		break;
	default:
		break;
	}
	
	return state;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_DownSockProc
 *[]----------------------------------------------------------------------[]
 *| summary	: 下り回線ソケット処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	RAU_LINE_STATE	RAU_DownSockProc(void)
{
	int ret;
	struct	sockaddr	addr;								// 接続したアドレス
	int		acceptSock;										// 接続したソケット
	int		addeSize;
	int		sockError;
	ushort	sendLen;										// 送信データ長
	RAU_LINE_STATE state = RAU_LINE_NO_ERROR;

	switch(RAU_downSockCondition) {
	case RAU_SOCK_INIT:										// 初期化
		if(KSG_gPpp_RauStarted) {							// PPP接続中？
			RAU_downSockCondition = RAU_SOCK_OPEN;
		}
		break;
	case RAU_SOCK_OPEN:										// ソケットオープン
		if(KSG_gPpp_RauStarted) {
			if(RAU_downListenSocket != -1) {
				tfClose(RAU_downListenSocket);				// オープン済みならクローズする
				RAU_downListenSocket = -1;
			}

			RAU_downListenSocket = RAU_CreateDownSocket();	// ソケット作成
			if(RAU_downListenSocket != -1) {
				RAU_downSockCondition = RAU_SOCK_LISTENING;
			}
			else {
				sockError = tfGetSocketError(RAU_downListenSocket);
			}
		}
		break;
	case RAU_SOCK_CONNECTING:								// TCP確立中
		break;
	case RAU_SOCK_CONNECTED:								// TCP確立
		break;
	case RAU_SOCK_LISTENING:								// TCP接続待ち
		if(RAU_downListenSocket != -1) {
			if(RAU_AntennaLevelState) {							// アンテナレベルチェック中
				tfClose(RAU_downListenSocket);
				RAU_downListenSocket = -1;
				RAU_downSockCondition = RAU_SOCK_CLOSED;
				break;
			}
			ret = listen(RAU_downListenSocket, 3);
			if(ret != -1) {									// 接続あり
				acceptSock = accept(RAU_downListenSocket, &addr, &addeSize);
				if(acceptSock != -1) {
					if(RAU_downSocket == -1) {				// 現在未接続
						RAU_downSocket = acceptSock;
						// ソケットのコールバック登録
						tfRegisterSocketCB(RAU_downSocket, RAU_downSocketCallback, 
											TM_CB_REMOTE_CLOSE | TM_CB_SOCKET_ERROR |
											TM_CB_RESET | TM_CB_CLOSE_COMPLT | TM_CB_CONNECT_COMPLT);
						
						// 下り回線状態変更
						RAU_downSockCondition = RAU_SOCK_IDLE;
						RauCtrl.tcpConnectFlag.port.download = 1;
					}
					else {									// 既に接続済み
						tfClose(acceptSock);
					}
				}
				else {
					sockError = tfGetSocketError(RAU_downListenSocket);
				}
			}
			else {
				sockError = tfGetSocketError(RAU_downListenSocket);
				if(sockError == TM_EWOULDBLOCK) {			// 接続なし(ノンブロッキングのためエラーではない)
					;
				}
				else {
					tfClose(RAU_downListenSocket);
					RAU_downListenSocket = -1;
					RAU_downSockCondition = RAU_SOCK_OPEN;
				}
			}
		}
		else {
			RAU_downSockCondition = RAU_SOCK_CLOSED;		// CLOSEDに遷移しOPEN後にリスニングソケットを作成する
		}
		break;
	case RAU_SOCK_IDLE:										// TCP接続中
		if(RAU_downSocket != -1) {							// ソケット有効（TCP接続中）
			// 下り回線通信監視がタイムアウトの場合は、HOST応答なしとして切断する
			if(RAU_f_Port_Watchdog_ov == 1) {
				RAU_CloseDownline();						// ホストの応答なしとみなし回線切断
				RAU_Tm_Port_Watchdog.tm = 0;				// 下り回線監視タイマ(停止)
				RAUhost_DwonLine_common_timeout();			// 受信シーケンスを０に戻す
			}
			
			// 送信処理
			if(DpaSndQue_Read(RAU_SendBuff, &sendLen, 0, DOWNLINE)) {	// 送信データあり？
				ret = send(RAU_downSocket, (char*)RAU_SendBuff, (int)sendLen, 0);
				if(ret == -1) {
					sockError = tfGetSocketError(RAU_upSocket);
					if(sockError == TM_EWOULDBLOCK) {		// 受信データなし(ノンブロッキングのためエラーではない)
						;
					}
					else {
						RAU_CloseDownline();				// セッションクローズ
						break;
					}
				}
				else if(ret > RAU_DATA_MAX) {
					RAUhost_SetError(ERR_RAU_DPA_SEND_LEN_D);	// 下り回線送信データ長異常(192)
					RAU_CloseDownline();					// セッションクローズ
					break;
				}
				state = RAU_LINE_SEND;

				// 無通信タイマ起動中なら、送信したためタイマを延長する
				if( RAU_f_No_Action_rq == 1 ){
					RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;
					RAU_SendDummyContinueCount = 0;			// ダミーデータ連続送信回数クリア
					RAU_f_No_Action_ov = 0;
				}
			}
			
			// 受信処理
			ret = recv(RAU_downSocket, (char*)RAU_RecvBuff, RAU_DATA_MAX, 0);	// データ受信
			if(ret > 0) {									// 受信データあり
				if(ret > RAU_DATA_MAX) {
					RAUhost_SetError(ERR_RAU_DPA_RECV_LEN_D);	// 下り回線受信データ長異常(193)
					ret = RAU_DATA_MAX;
				}
				// 受信データを格納する
				RAU_DpaRcvQue_Set(RAU_RecvBuff, (ushort)ret, DOWNLINE);
			}
			else if(ret == -1) {							// エラー発生?
				sockError = tfGetSocketError(RAU_downSocket);
				if(sockError == TM_EWOULDBLOCK) {			// 受信データなし(ノンブロッキングのためエラーではない)
					;
				}
				else {										// エラー発生
					RAU_CloseDownline();
					RAU_downSockCondition = RAU_SOCK_CLOSING;
					RauCtrl.tcpConnectFlag.port.download = 0;
				}
			}
		}
		break;
	case RAU_SOCK_CLOSING:									// TCP切断中
		if(RAU_downSocket == -1) {
			RAU_downSockCondition = RAU_SOCK_CLOSED;
		}
		break;
	case RAU_SOCK_CLOSED:									// TCP切断
		RauCtrl.tcpConnectFlag.port.download = 0;
		if(KSG_gPpp_RauStarted) {
			RAU_downSockCondition = RAU_SOCK_OPEN;
		}
		else if(RAU_AntennaLevelState == 1) {				// アンテナレベルチェック中
			;												// 何もしない(RAU_SOCK_CLOSEDのまま)
		}
		else {
			RAU_downSockCondition = RAU_SOCK_INIT;
			
		}
		break;
	default:
		break;
	}
	
	return state;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SNTPStart
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP時刻補正開始処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SNTPStart( uchar onoff ,ushort msg, ulong now_sec, ushort now_msec )
{
	sntp_start = onoff;
	sntp_rmsg = msg;
	if(socketDesc != TM_SOCKET_ERROR) {
		tfClose(socketDesc);
	}
	socketDesc = TM_SOCKET_ERROR;				// SNTP ソケットの状態
	StateFlag = TM_SNTP_STATUS_SEND;			// SNTP 状態フラグ
	// 要求開始
	if (sntp_start) {
		// 要求時刻(UTC)
		transmitTimestamp[0] = now_sec;
		transmitTimestamp[1] = now_msec;
	}
	// 要求停止
	else {
		memset(transmitTimestamp, 0, sizeof(transmitTimestamp));
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SNTPProc
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP時刻同期処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SNTPProc(void)
{
	int ret;

	struct sockaddr_in ipv4_Setting;
	ttSockAddrStoragePtr ipAddrPtr;
	char	myIP[16];
	
	if( sntp_start == 1 ){	// SNTP スタートフラグがONの場合のみ実施
		if(KSG_gPpp_RauStarted == 1 ) {										// PPP接続中
			ipv4_Setting.sin_len = sizeof( ipv4_Setting );					// length of struct sockaddr_in
			ipv4_Setting.sin_family = AF_INET;								// AF_INET
			ipv4_Setting.sin_port = _SNTP_PORTNO;							// SNTP ポート番号
			memset(myIP, 0x00, sizeof(myIP));
			sprintf(myIP, "%03d.%03d.%03d.%03d",
						prm_get(COM_PRM, S_CEN, 62, 3, 4),
						prm_get(COM_PRM, S_CEN, 62, 3, 1),
						prm_get(COM_PRM, S_CEN, 63, 3, 4),
						prm_get(COM_PRM, S_CEN, 63, 3, 1));
			ipv4_Setting.sin_addr.s_addr = inet_addr(myIP);	
			ipAddrPtr = (ttSockAddrStoragePtr) &ipv4_Setting;				// SNTPサーバーIPアドレス
			ret = tfSntpGetTimeByUnicast( TM_BLOCKING_OFF, &socketDesc, &StateFlag, ipAddrPtr );	// SNTP時刻補正実行
			
			if( ret == TM_EWOULDBLOCK && StateFlag == TM_SNTP_STATUS_RECV ){
				// 関数を繰り返しコールするため、ここでは何もしない
			} else {	// 正常終了or無効なSNTPハンドル
				sntp_start = 0;									// 完了 SNTP スタートフラグを落とす
			}
		}
		else {																// PPP未接続
			sntp_start = 0;
			queset(OPETCBNO, sntp_rmsg, 0, NULL);
		}
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SNTPGetPacket
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP受信パケット取得処理
 *[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void	RAU_SNTPGetPacket( void *voidPtr )
{
	ttNtpPacket *pkt;

	if (voidPtr) {
		pkt = voidPtr;
		memcpy(pkt, &sntpdata, sizeof(sntpdata));
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SNTPStart
 *[]----------------------------------------------------------------------[]
 *| summary	: 閏年チェック
 *| return	: 0:平年、1：閏年
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
char	leapyear_check( short pa_yer )
{
	char	cm_ret;
	cm_ret = 0;			// 平年にしておく
	if (( (pa_yer % 4) == 0 )&&( (pa_yer % 100) != 0 )){	// 4で割り切れる and 100で割り切れない
		cm_ret = 1;		// 閏年
	}
	if ((pa_yer % 400) == 0 ){								// 100で割り切れても、400で割り切れると閏年
		cm_ret = 1;		// 閏年
	}
	return( cm_ret );
}
/*[]----------------------------------------------------------------------[]
 *|	name	: tfKernelGetSystemTime
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP現在時刻取得処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int tfKernelGetSystemTime( ttUser32Bit * daysPrt, ttUser32Bit * secondsPtr )
{
	*daysPrt = 0;
	*secondsPtr = transmitTimestamp[0];

	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: tfKernelSetSystemTime
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP時刻設定処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int tfKernelSetSystemTime( ttUser32BitPtr secondsPtr, void * voidPtr , int size )
{
	ttNtpPacket *pkt;

	pkt = voidPtr;

	// 時刻応答ではない
	if (pkt->sntpMode != TM_SNTP_MODE_SERVER) {
		return -1;
	}
	// 要求時刻不一致
	if (transmitTimestamp[0] != 0) {
		if (transmitTimestamp[0] != pkt->originateTimestampHigh &&
			0 != pkt->originateTimestampLow) {
			return -1;
		}
	}
	// SNTPパケットを保存
	memcpy(&sntpdata, pkt, sizeof(*pkt));
	queset(OPETCBNO, sntp_rmsg, 0, NULL);
	
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetUpNetState
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線の状態を取得する
 *| retrun	: RAU_NET_IDLE				未接続
 *|			  RAU_NET_IDLE				PPP確立中
 *|			  RAU_NET_TCP_CONNECTIED	TCP確立中
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
RAU_NET_STATE	RAU_GetUpNetState(void)
{
	if(KSG_gPpp_RauStarted) {						// PPP確立中
		if(RauCtrl.tcpConnectFlag.port.upload) {	// 上り回線TCP確立中
			return RAU_NET_TCP_CONNECTIED;
		}
		else {
			return RAU_NET_PPP_OPENED;
		}
	}
	
	return RAU_NET_IDLE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetDownNetState
 *[]----------------------------------------------------------------------[]
 *| summary	: 下り回線の状態を取得する
 *| retrun	: RAU_NET_IDLE				未接続
 *|			  RAU_NET_IDLE				PPP確立中
 *|			  RAU_NET_TCP_CONNECTIED	TCP確立中
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
RAU_NET_STATE	RAU_GetDownNetState(void)
{
	if(KSG_gPpp_RauStarted) {						// PPP確立中
		if(RauCtrl.tcpConnectFlag.port.download) {	// 下り回線TCP確立中
			return RAU_NET_TCP_CONNECTIED;
		}
		else {
			return RAU_NET_PPP_OPENED;
		}
	}
	
	return RAU_NET_IDLE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_DpaRcvQue_Init()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command receive queue initialize.                             |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_DpaRcvQue_Init( void )
{
	memset(&DPA_Rcv_Ctrl, 0, sizeof(DPA_Rcv_Ctrl));
	memset(&DPA2_Rcv_Ctrl, 0, sizeof(DPA2_Rcv_Ctrl));
}


/*[]----------------------------------------------------------------------[]*/
/*|             RAU_DpaRcvQue_Set()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command set to receive queue.                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pointer for receive data.                                     |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void RAU_DpaRcvQue_Set(uchar *pData, ushort len, int port)
{
	short	length;										// データ長
	short	oldLen;										// パケット途中のデータ長
	short	remain;										// 残りデータ長
	ushort	index = 0;
	
	remain = (short)len;
	if( port == DOWNLINE ){	// 下り回線からの受信データをセットする。
		if( len > RAU_DATA_MAX ) {
			len = RAU_DATA_MAX;
		}
		
		while(index < len) {
			if(DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len == 0) {
				// パケットの先頭から受信
				if(remain < 4) {							// データ長を受信していない
					// データ長が不明のため次ぐのデータを取得する
					// パケット受信途中のためインデックス、カウンタは更新しない
					DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len = remain;
					memcpy( DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Dat, pData, remain );
					break;
				}
			}

			// 受信データ格納（パケットの途中からまたは途中までの可能性あり）
			memcpy( &DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Dat[DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len], 
								pData + index, remain );		// とりあえず全てコピー
			
			// 1パケット分のデータ長を設定する
			length = (short)RAU_Byte2Long(DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Dat);
			oldLen = DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len;
			DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len = length;
			
			if(length <= remain + oldLen) {						// 1パケット全て受信？
				// 全データ受信していればインデックス、カウンタを更新
				++DPA_Rcv_Ctrl.Count;							// queue regist count update
				++DPA_Rcv_Ctrl.WriteIdx;						// next write index update
				DPA_Rcv_Ctrl.WriteIdx &= 3;
			}
			index += remain < length ? remain : length;			// 次のパケットとなる先頭インデックスを設定
			remain -= (length - DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len);
		}
	}
	else if( port == CREDIT ){
		if( len > RAU_DATA_MAX ) {
			len = RAU_DATA_MAX;
		}
		
		while(index < len) {
			if(DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len == 0) {
				// パケットの先頭から受信
				if(remain < 4) {							// データ長を受信していない
					// データ長が不明のため次のデータを取得する
					// パケット受信途中のためインデックス、カウンタは更新しない
					DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len = remain;
					memcpy( DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Dat, pData, remain );
					break;
				}
			}

			// 受信データ格納（パケットの途中からまたは途中までの可能性あり）
			memcpy( &DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Dat[DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len], 
								pData + index, remain );		// とりあえず全てコピー
			
			// 1パケット分のデータ長を設定する
			length = (short)RAU_Byte2Long(DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Dat);
			oldLen = DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len;
			DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len = length;
			
			if(length <= remain + oldLen) {						// 1パケット全て受信？
				// 全データ受信していればインデックス、カウンタを更新
				++DPA_Credit_RcvCtrl.Count;							// queue regist count update
				++DPA_Credit_RcvCtrl.WriteIdx;						// next write index update
				DPA_Credit_RcvCtrl.WriteIdx &= 3;
			}
			index += remain < length ? remain : length;			// 次のパケットとなる先頭インデックスを設定
			remain -= (length - DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len);
		}
	} else {				// 上り回線からの受信データをセットする。
		memcpy(DPA2_Rcv_Ctrl.dpa_data[DPA2_Rcv_Ctrl.WriteIdx], pData, RAU_NET_RES_LENGTH_MAX);

		++DPA2_Rcv_Ctrl.Count;
		++DPA2_Rcv_Ctrl.WriteIdx;
		DPA2_Rcv_Ctrl.WriteIdx &= 3;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_DpacRcvQue_Read()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command read from receive queue.                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pointer for set command data (receive mail style).            |*/
/*| return : 1=received data exist,  0=data none                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar RAU_DpaRcvQue_Read(uchar *pData, ushort *len, int port)
{
	ushort	w_len;

	if( port == DOWNLINE ){	// 下り回線からの受信データを取り出す。
		if( 0 == DPA_Rcv_Ctrl.Count ) {
			return	(uchar)0;
		}

		w_len = DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.ReadIdx ].Len;
		DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.ReadIdx ].Len = 0;
		
		if( w_len > RAU_DATA_MAX ) {
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memcpy( pData, DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.ReadIdx ].Dat, w_len );

		--DPA_Rcv_Ctrl.Count;							// queue regist count update
		++DPA_Rcv_Ctrl.ReadIdx;							// next read index update
		DPA_Rcv_Ctrl.ReadIdx &= 3;
	}
	else if( port == CREDIT ){
		if( 0 == DPA_Credit_RcvCtrl.Count ) {
			return	(uchar)0;
		}

		w_len = DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.ReadIdx ].Len;
		DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.ReadIdx ].Len = 0;
		
		if( w_len > RAU_DATA_MAX ) {
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memcpy( pData, DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.ReadIdx ].Dat, w_len );

		--DPA_Credit_RcvCtrl.Count;							// queue regist count update
		++DPA_Credit_RcvCtrl.ReadIdx;							// next read index update
		DPA_Credit_RcvCtrl.ReadIdx &= 3;
	} else {				// 上り回線からの受信データを取り出す。
		if( 0 == DPA2_Rcv_Ctrl.Count ) {	// データなし
			return	(uchar)0;
		}

		*len = RAU_NET_RES_LENGTH_MAX;	// 上り回線の受信データはＡＣＫかＮＡＣしかないのでバッファは小さい。
		memcpy(pData, DPA2_Rcv_Ctrl.dpa_data[DPA2_Rcv_Ctrl.ReadIdx], RAU_NET_RES_LENGTH_MAX);

		--DPA2_Rcv_Ctrl.Count;
		++DPA2_Rcv_Ctrl.ReadIdx;
		DPA2_Rcv_Ctrl.ReadIdx &= 3;
	}

	return	(uchar)1;
}

/*[]----------------------------------------------------------------------[]*/
/*|             DpaSndQue_Init()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command send queue initialize.                                |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	DpaSndQue_Init( void )
{
	DPA_Snd_Ctrl.Count = 0;
	DPA_Snd_Ctrl.ReadIdx = 0;
	DPA_Snd_Ctrl.WriteIdx = 0;
	DPA_Snd_Ctrl.dummy = 0;

	DPA2_Snd_Ctrl.Count = 0;
	DPA2_Snd_Ctrl.ReadIdx = 0;
	DPA2_Snd_Ctrl.WriteIdx = 0;
	DPA2_Snd_Ctrl.dummy = 0;
}


/*[]----------------------------------------------------------------------[]*/
/*|             DpaSndQue_Set()                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command set to send queue.                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pointer for send data.                                        |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void DpaSndQue_Set(uchar *pData, ushort len, int port)
{
	if( len > RAU_DATA_MAX ){
		len = RAU_DATA_MAX;
	}
	if( port == DOWNLINE ){	// 下り回線への送信データをセットする。
		DPA2_Snd_Ctrl.dpa_data[DPA2_Snd_Ctrl.WriteIdx].Len = len;

		memcpy(DPA2_Snd_Ctrl.dpa_data[DPA2_Snd_Ctrl.WriteIdx].Dat, pData, len);

		++DPA2_Snd_Ctrl.Count;
		++DPA2_Snd_Ctrl.WriteIdx;
		DPA2_Snd_Ctrl.WriteIdx &= 3;
	}
	else if( port == CREDIT ){
		DPA_Credit_SndCtrl.dpa_data[ DPA_Credit_SndCtrl.WriteIdx ].Len = len;

		memcpy( DPA_Credit_SndCtrl.dpa_data[ DPA_Credit_SndCtrl.WriteIdx ].Dat, pData, len );

		++DPA_Credit_SndCtrl.Count;							// queue regist count update
		++DPA_Credit_SndCtrl.WriteIdx;						// next write index update
		DPA_Credit_SndCtrl.WriteIdx &= 3;
	} else {				// 上り回線への送信データをセットする。
		DPA_Snd_Ctrl.dpa_data[ DPA_Snd_Ctrl.WriteIdx ].Len = len;

		memcpy( DPA_Snd_Ctrl.dpa_data[ DPA_Snd_Ctrl.WriteIdx ].Dat, pData, len );

		++DPA_Snd_Ctrl.Count;							// queue regist count update
		++DPA_Snd_Ctrl.WriteIdx;						// next write index update
		DPA_Snd_Ctrl.WriteIdx &= 3;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             DpaSndQue_Read()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command read from send queue.                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pointer for set command data.                                 |*/
/*| return : 1=received data exist,  0=data none                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar DpaSndQue_Read(uchar *pData, ushort *len, uchar kind, int port)
{
	ushort	w_len;

	if( port == DOWNLINE ){	// 下り回線への送信データを取り出す。
		if( 0 == DPA2_Snd_Ctrl.Count ){	// データなし
			return	(uchar)0;
		}
		if( kind ){
			return	(uchar)1;
		}

		w_len = DPA2_Snd_Ctrl.dpa_data[DPA2_Snd_Ctrl.ReadIdx].Len;
		if( w_len > RAU_DATA_MAX ){
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memcpy(pData, DPA2_Snd_Ctrl.dpa_data[DPA2_Snd_Ctrl.ReadIdx].Dat, w_len);

		--DPA2_Snd_Ctrl.Count;
		++DPA2_Snd_Ctrl.ReadIdx;
		DPA2_Snd_Ctrl.ReadIdx &= 3;
	}
	else if( port == CREDIT ){
		if( 0 == DPA_Credit_SndCtrl.Count ){	// データなし
			return	(uchar)0;
		}
		if( kind ){
			return	(uchar)1;
		}

		w_len = DPA_Credit_SndCtrl.dpa_data[DPA_Credit_SndCtrl.ReadIdx].Len;
		if( w_len > RAU_DATA_MAX ){
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memset(pData, 0x00, RAU_RCV_MAX_H );
		memcpy(pData, DPA_Credit_SndCtrl.dpa_data[DPA_Credit_SndCtrl.ReadIdx].Dat, w_len);

		--DPA_Credit_SndCtrl.Count;
		++DPA_Credit_SndCtrl.ReadIdx;
		DPA_Credit_SndCtrl.ReadIdx &= 3;
	} else {
		if( 0 == DPA_Snd_Ctrl.Count ){
			return	(uchar)0;
		}
		if( kind ){									// 送信ﾃﾞｰﾀの有無のみﾁｪｯｸ
			return	(uchar)1;
		}

		w_len = DPA_Snd_Ctrl.dpa_data[ DPA_Snd_Ctrl.ReadIdx ].Len;

		if( w_len > RAU_DATA_MAX ){
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memcpy(pData, DPA_Snd_Ctrl.dpa_data[ DPA_Snd_Ctrl.ReadIdx ].Dat, w_len);

		--DPA_Snd_Ctrl.Count;							// queue regist count update
		++DPA_Snd_Ctrl.ReadIdx;							// next read index update
		DPA_Snd_Ctrl.ReadIdx &= 3;
	}

	return	(uchar)1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CreateUpSocket
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線のソケットを作成する
 *[]----------------------------------------------------------------------[]
 *| return	: -1以外	作成したソケットディスクリプタ
 *|			  -1		作成失敗
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int RAU_CreateUpSocket(void)
{
	int		sock = -1;						// ソケットディスクリプタ
	int		on = 1;							// オプション有効
	int		maxrt;							// タイムアウト時間
	struct sockaddr_in	myAddr;				// 自局アドレス・ポート番号

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// ソケット作成
	if(sock == -1) {
		return sock;
	}
	
	// ノンブロッキング
	if(-1 == tfBlockingState(sock, TM_BLOCKING_OFF)) {
		tfClose(sock);
		return -1;
	}

	// TIME_WAITでもポートのオープンを可能にする
	if(-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) {
		tfClose(sock);
		return -1;
	}

	// connectのタイムアウト時間を変更する
	maxrt = RAU_CONN_TIMEOUT;
	if(-1 == setsockopt(sock, IP_PROTOTCP, TCP_MAXRT, (char*)&maxrt, sizeof(maxrt))) {
		tfClose(sock);
		return -1;
	}

	// ソケットイベントコールバック関数の登録
	tfRegisterSocketCB(sock, RAU_upSocketCallback, 
						TM_CB_REMOTE_CLOSE | TM_CB_SOCKET_ERROR |
						TM_CB_RESET | TM_CB_CLOSE_COMPLT | TM_CB_CONNECT_COMPLT);

	// 自局ポートを割り当てる
	memset(&myAddr, 0x00, sizeof(myAddr));
	myAddr.sin_family      = AF_INET;
	myAddr.sin_port        = htons(RauConfig.Dpa_port_m);	// 上り自局ポート番号
	myAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_m);		// 自局IPアドレス
	if(-1 == bind(sock, (struct sockaddr *)&myAddr, sizeof(myAddr))) {
		tfClose(sock);
		sock = -1;
	}
	
	return sock;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CreateDownSocket
 *[]----------------------------------------------------------------------[]
 *| summary	: 下り回線のソケットを作成する
 *[]----------------------------------------------------------------------[]
 *| return	: -1以外	作成したソケットディスクリプタ
 *|			  -1		作成失敗
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int RAU_CreateDownSocket(void)
{
	int		sock = -1;						// ソケットディスクリプタ
	struct sockaddr_in	myAddr;				// 自局アドレス・ポート番号
	
	// Listen用のソケット作成
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// ソケット作成
	if(sock == -1) {
		return sock;
	}
	
	// ノンブロッキング
	if(-1 == tfBlockingState(sock, TM_BLOCKING_OFF)) {
		tfClose(sock);
		return -1;
	}
	
	// 自局ポートを割り当てる
	memset(&myAddr, 0x00, sizeof(myAddr));
	myAddr.sin_family      = AF_INET;
	myAddr.sin_port        = htons(RauConfig.Dpa_port_m2);	// 下り自局ポート番号
	myAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_m);		// 自局IPアドレス
	if(-1 == bind(sock, (struct sockaddr *)&myAddr, sizeof(myAddr))) {
		tfClose(sock);
		sock = -1;
	}
	
	return sock;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_ConnectUpline
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線のTCP接続を開始する
 *[]----------------------------------------------------------------------[]
 *| return	: -1以外	確立開始
 *|			  -1		確立失敗
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int	RAU_ConnectUpline(void)
{
	struct sockaddr_in	serverAddr;			// HOSTアドレス・ポート番号
	int	sockError;							// ソケットエラーコード
	int	ret;
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family      = AF_INET;
	serverAddr.sin_port        = htons(RauConfig.Dpa_port_h);	// 下りHOSTポート番号
	serverAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_h);		// HOSTIPアドレス
	
	ret = connect(RAU_upSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(ret == -1) {
		sockError = tfGetSocketError(RAU_upSocket);
		if(sockError == TM_EINPROGRESS ||			// ノンブロッキングで接続中はエラーではない
			sockError == TM_EISCONN) {				// 既に接続中はエラーではない(connect～GetSockError間で接続完了)
			ret = 0;								// エラーとしない
		}
	}
	if(ret == 0) {
		RAU_TcpConnect = 1;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CloseUpline
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線のTCP接続を切断する
 *| param	: resetReq	TRUE:TCPコネクション要因のクリア
 *[]----------------------------------------------------------------------[]
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_CloseUpline(BOOL resetReq)
{
	tfClose(RAU_upSocket);					// ソケットクローズ
	RAU_upSockCondition = RAU_SOCK_CLOSING;	// クローズイベント待ち
	RauCtrl.tcpConnectFlag.port.upload = 0;

	// TIME_WAIT解除待ちタイマ(起動)
	if(RauConfig.Dpa_discnct_tm != 0){
		RAU_Tm_TCP_TIME_WAIT.bits0.bit_0_13 = RauConfig.Dpa_discnct_tm;
		RAU_f_TCPtime_wait_rq = 1;
		RAU_f_TCPtime_wait_ov = 0;
	}
	else{
		RAU_f_TCPtime_wait_rq = 0;
		RAU_f_TCPtime_wait_ov = 1;
	}
	if(resetReq) {
		RAUhost_TcpConnReq_Clear();
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CloseDownline
 *[]----------------------------------------------------------------------[]
 *| summary	: 下り回線のTCP接続を切断する
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_CloseDownline(void)
{
	tfClose(RAU_downSocket);					// ソケットクローズ
	RAU_downSockCondition = RAU_SOCK_CLOSING;	// クローズイベント待ち
	RauCtrl.tcpConnectFlag.port.download = 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_downSocketCallback
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線のソケットイベントコールバック関数
 *[]----------------------------------------------------------------------[]
 *| param	: sock		ソケットディスクリプタ
 *|			  event		通知イベント
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void 	RAU_upSocketCallback(int sock, int event)
{
	if(RAU_upSocket != sock) {									// ソケットが異なるまたは既にクローズ済み？
		return;
	}
	
	if( (event & TM_CB_RESET) || (event & TM_CB_REMOTE_CLOSE) || (event & TM_CB_SOCKET_ERROR) ) {
			if(RAU_upSocket != -1) {
				if(RAU_TcpConnect == 1) {
					if(RauCtrl.connRetryCount >= RAU_CONN_RETRY_MAX) {	// connectリトライオーバー
						RAUhost_SetError(ERR_RAU_HOST_COMMUNICATION);
						RAU_CloseUpline(TRUE);
						RauCtrl.TCPcnct_req = 0;						// TCP接続要求クリア
						RauCtrl.connRetryCount = 0;						// リトライ回数クリア
						// データ再送待ちタイマ起動
						RAU_Tm_data_rty.bits0.bit_0_13 = RauConfig.Dpa_data_rty_tm;	
						RAU_f_data_rty_rq = 1;
						RAU_f_data_rty_ov = 0;
						RAU_TcpConnect = 0;
					}
					else {
						RAUhost_SetError(ERR_RAU_TCP_CONNECT);			// エラーコード９１(上り回線ＴＣＰコネクションタイムアウト)を通知
						tfClose(RAU_upSocket);
						RAU_upSockCondition = RAU_SOCK_CLOSING;
						++RauCtrl.connRetryCount;
					}
				}
				else {
					RAU_upSockCondition = RAU_SOCK_CLOSED;		/* クローズ完了状態へ移行				*/
					RAU_CloseUpline(TRUE);
// MH322914 (s) kasiyama 2016/07/22 切断後途中ブロックから送信してしまう問題の改善[共通バグNo.1311]
					// 切れた認識の場合は、クリアする（RAU_CloseUpline関数内でRAUhost_TcpConnReq_Clear関数を呼ぶがRauCtrl.TCPcnct_reqが0であるため、クリアされないので、ここでクリアする）
					RAU_uc_retrycnt_h = 0;										// 上り回線の送信リトライカウンタクリア
					RAU_Tm_Ackwait.tm = 0;										// 上り回線のＡＣＫ受信待ちタイマクリア
					RAU_uc_retrycnt_reconnect_h = 0;							// 上り回線の送信リトライカウンタクリア
					RAU_Tm_Ackwait_reconnect.tm = 0;							// 上り回線の再接続待ちタイマクリア
			
					RAUque_DeleteQue();										// テーブルから送信完了したデータのみ消す。
					pRAUhost_SendIdInfo = NULL;								// テーブルの参照をやめる。
			
					RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// 送信シーケンス０を(ＨＯＳＴへのテーブル送信待機状態)に戻す。
			
					RAU_ui_data_length_h = 0;									// 受信データ長クリア
					RAU_uc_mode_h = S_H_IDLE;									// ＡＣＫ待ちを解除してアイドルに移行
// MH322914 (e) kasiyama 2016/07/22 切断後途中ブロックから送信してしまう問題の改善[共通バグNo.1311]
					RAU_TcpConnect = 0;
				}
			}
			else {
				RAU_upSockCondition = RAU_SOCK_CLOSED;			/* クローズ完了状態へ移行				*/
				RAU_TcpConnect = 0;
			}
	}
	else if (event & TM_CB_CLOSE_COMPLT) {
			RAU_upSockCondition = RAU_SOCK_CLOSED;				/* クローズ完了状態へ移行				*/
			RAU_upSocket = -1;
	}
	else if (event & TM_CB_CONNECT_COMPLT) {
			RauCtrl.TCPcnct_req = 0;							// TCP接続要求クリア
			RauCtrl.connRetryCount = 0;							// リトライ回数クリア
			RAU_TcpConnect = 0;
			RAU_upSockCondition = RAU_SOCK_CONNECTED;			/* コネクト完了状態へ移行				*/
	}
	return;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_downSocketCallback
 *[]----------------------------------------------------------------------[]
 *| summary	: 下り回線のソケットイベントコールバック関数
 *[]----------------------------------------------------------------------[]
 *| param	: sock		ソケットディスクリプタ
 *|			  event		通知イベント
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void 	RAU_downSocketCallback(int sock, int event)
{
	if(RAU_downSocket != sock) {								// ソケットが異なるまたは既にクローズ済み？
		return;
	}
	
	if( (event & TM_CB_RESET) || (event & TM_CB_REMOTE_CLOSE) || (event & TM_CB_SOCKET_ERROR) ) {
		if(RAU_downSocket != -1) {
			RAU_CloseDownline();
		}
		else {
			RAU_upSockCondition = RAU_SOCK_CLOSED;				/* クローズ完了状態へ移行				*/
		}
	}
	else if (event & TM_CB_CLOSE_COMPLT) {
		RAU_downSockCondition = RAU_SOCK_CLOSED;				/* クローズ完了状態へ移行				*/
		RAU_downSocket = -1;
	}
	return;
}
void	SetUpNetState(uchar state)
{
	RauCtrl.upNetState = state;
}

uchar	GetUpNetState(void)
{
	return RauCtrl.upNetState;
}

void	SetDownNetState(uchar state)
{
	RauCtrl.downNetState = state;
}

uchar	GetDownNetState(void)
{
	return RauCtrl.downNetState;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SetAntennaLevelState
 *[]----------------------------------------------------------------------[]
 *| summary	: アンテナレベルチェック開始・終了を指示する
 *[]----------------------------------------------------------------------[]
 *| param	: state		0:開始 1:終了
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SetAntennaLevelState(uchar	state)
{
	RAU_AntennaLevelState = state;

	if(RAU_AntennaLevelState == 1) {						// アンテナレベルチェック開始
		if(RauCtrl.tcpConnectFlag.port.upload == 1) {		// 上り回線接続中
			RAU_CloseUpline(TRUE);
		}
		
		if(RauCtrl.tcpConnectFlag.port.download == 1) {		// 下り回線接続中
			RAU_CloseDownline();
			tfClose(RAU_downListenSocket);
			RAU_downListenSocket = -1;
		}
		if(RauCtrl.tcpConnectFlag.port.credit == 1) {		// クレジット回線接続中
			RAU_CloseCreditline(TRUE);
		}
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetAntennaLevelState
 *[]----------------------------------------------------------------------[]
 *| summary	: アンテナレベルチェック状態を取得する
 *[]----------------------------------------------------------------------[]
 *| param	: state		0:開始 1:終了
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	RAU_GetAntennaLevelState(void)
{
	return RAU_AntennaLevelState;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CreditSockProc
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット回線ソケット処理
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	RAU_LINE_STATE	RAU_CreditSockProc(void)
{
	int ret;
	int		sockError;
	ushort	sendLen;										// 送信データ長
	RAU_LINE_STATE state = RAU_LINE_NO_ERROR;

	switch(RAU_creditSockCondition) {
	case RAU_SOCK_INIT:										// 初期化
		if(KSG_gPpp_RauStarted) {							// PPP接続中？
			RAU_creditSockCondition = RAU_SOCK_OPEN;
			CreditCtrl.connRetryCount = 0;						// リトライ回数クリア
		}
		break;
	case RAU_SOCK_OPEN:										// ソケットオープン
		if(RAU_AntennaLevelState) {							// アンテナレベルチェック中
			RAU_creditSockCondition = RAU_SOCK_CLOSED;
			CreditCtrl.connRetryCount = 0;						// リトライ回数クリア
			break;
		}
		if(KSG_gPpp_RauStarted) {
			if(RAU_creditSocket != -1) {
				tfClose(RAU_creditSocket);
				RAU_creditSocket = -1;
				Credit_TcpConnReq_Clear();
			}
			if(CreditCtrl.TCPcnct_req == 1) {					// TCP接続要求あり
//				if(Credit_data_rty_ov) {						// データ再送待ちタイマがタイムアウトするまでは接続を開始しない。
					RAU_creditSocket = RAU_CreateCreditSocket();		// 上り回線ソケット作成
					if(RAU_creditSocket != -1) {
						if(-1 != RAU_ConnectCreditline()) {
							// TCP接続開始
							RAU_creditSockCondition = RAU_SOCK_CONNECTING;
						}
						else {
							tfClose(RAU_creditSocket);
							RAU_creditSockCondition = RAU_SOCK_CLOSING;
						}
					}
//				}
//				else {
//					Credit_TcpConnReq_Clear();
//					CreditCtrl.TCPcnct_req = 0;				// 接続要求をクリアする
//					CreditCtrl.connRetryCount = 0;				// リトライ回数クリア
//				}
			}
			else {
				CreditCtrl.connRetryCount = 0;					// リトライ回数クリア
			}
		}
		else {
			CreditCtrl.connRetryCount = 0;						// リトライ回数クリア
		}
		break;
	case RAU_SOCK_CONNECTING:								// TCP確立中
		// 何もしない
		break;
	case RAU_SOCK_CONNECTED:								// TCP確立
		RAU_creditSockCondition = RAU_SOCK_IDLE;
		RauCtrl.tcpConnectFlag.port.credit = 1;
		break;
	case RAU_SOCK_LISTENING:								// TCP接続待ち
		break;
	case RAU_SOCK_IDLE:										// TCP接続中
		if(RAU_creditSocket != -1){
			CreditCtrl.TCPcnct_req = 0;						// 既に接続中なのでTCP接続要求をクリア
			// 送信処理
			if(DpaSndQue_Read(RAU_SendBuff, &sendLen, 0, CREDIT)) {	// 送信データあり？
				ret = send(RAU_creditSocket, (char*)RAU_SendBuff, (int)sendLen, 0);
				if(ret == -1) {
					sockError = tfGetSocketError(RAU_creditSocket);
					if(sockError == TM_EWOULDBLOCK) {		// 受信データなし(ノンブロッキングのためエラーではない)
						;
					}
					else {
						RAU_CloseCreditline(TRUE);
						break;
					}
				}
				else if(ret > RAU_DATA_MAX) {
					Credit_SetError(ERR_CREDIT_FMA_SEND_LEN);	// 上り回線送信データ長異常(92)
					RAU_CloseCreditline(TRUE);
					break;
				}
				state = RAU_LINE_SEND;
				
//				// 無通信タイマ起動中なら、送信したためタイマを延長する
//				if( RAU_f_No_Action_rq == 1 ){
//					RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;
//					RAU_f_No_Action_ov = 0;
//				}
			}
			
			// 受信処理
			ret = recv(RAU_creditSocket, (char*)RAU_RecvBuff, RAU_DATA_MAX, 0);	// データ受信
			if(ret > 0) {									// 受信データあり
				if(ret > RAU_DATA_MAX) {
					Credit_SetError(ERR_CREDIT_FMA_RECV_LEN);	// 上り回線受信データ長異常(93)
					ret = RAU_DATA_MAX;
				}
				// 受信データを格納する
				RAU_DpaRcvQue_Set(RAU_RecvBuff, (ushort)ret, CREDIT);
			}
			else if(ret == -1) {							// エラー発生?
				sockError = tfGetSocketError(RAU_creditSocket);
				if(sockError == TM_EWOULDBLOCK) {			// 受信データなし(ノンブロッキングのためエラーではない)
					;
				}
				else {										// エラー発生
					RAU_CloseCreditline(TRUE);
				}
			}
		}
		break;
	case RAU_SOCK_CLOSING:									// TCP切断中
		// 何もしない
		if(RAU_creditSocket == -1) {
			RAU_creditSockCondition = RAU_SOCK_CLOSED;
		}
		break;
	case RAU_SOCK_CLOSED:									// TCP切断
		RauCtrl.tcpConnectFlag.port.credit = 0;
		if(KSG_gPpp_RauStarted) {
			RAU_creditSockCondition = RAU_SOCK_OPEN;
		}
		else if(RAU_AntennaLevelState == 1) {				// アンテナレベルチェック中
			;												// 何もしない(RAU_SOCK_CLOSEDのまま)
		}
		else {
			RAU_creditSockCondition = RAU_SOCK_INIT;
		}
		break;
	default:
		break;
	}
	
	return state;
	
}

static	int RAU_CreateCreditSocket(void)
{
	int		sock = -1;						// ソケットディスクリプタ
	int		on = 1;							// オプション有効
	int		maxrt;							// タイムアウト時間
	struct sockaddr_in	myAddr;				// 自局アドレス・ポート番号

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// ソケット作成
	if(sock == -1) {
		return sock;
	}
	
	// ノンブロッキング
	if(-1 == tfBlockingState(sock, TM_BLOCKING_OFF)) {
		tfClose(sock);
		return -1;
	}

	// TIME_WAITでもポートのオープンを可能にする
	if(-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) {
		tfClose(sock);
		return -1;
	}

	// connectのタイムアウト時間を変更する
	maxrt = RAU_CONN_TIMEOUT;
	if(-1 == setsockopt(sock, IP_PROTOTCP, TCP_MAXRT, (char*)&maxrt, sizeof(maxrt))) {
		tfClose(sock);
		return -1;
	}

	// ソケットイベントコールバック関数の登録
	tfRegisterSocketCB(sock, RAU_creditSocketCallback, 
						TM_CB_REMOTE_CLOSE | TM_CB_SOCKET_ERROR |
						TM_CB_RESET | TM_CB_CLOSE_COMPLT | TM_CB_CONNECT_COMPLT);

	// 自局ポートを割り当てる
	memset(&myAddr, 0x00, sizeof(myAddr));
	myAddr.sin_family      = AF_INET;
	myAddr.sin_port        = htons(RauConfig.Dpa_port_m3);	// 上り自局ポート番号
	myAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_m);		// 自局IPアドレス
	if(-1 == bind(sock, (struct sockaddr *)&myAddr, sizeof(myAddr))) {
		tfClose(sock);
		sock = -1;
	}
	
	return sock;
	
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_ConnectUpline
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線のTCP接続を開始する
 *[]----------------------------------------------------------------------[]
 *| return	: -1以外	確立開始
 *|			  -1		確立失敗
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	int	RAU_ConnectCreditline(void)
{
	struct sockaddr_in	serverAddr;			// HOSTアドレス・ポート番号
	int	sockError;							// ソケットエラーコード
	int	ret;
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family      = AF_INET;
	serverAddr.sin_port        = htons(RauConfig.Dpa_port_Cre);	// クレジットサーバーポート番号
	serverAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_Cre);		// HOSTIPアドレス
	
	ret = connect(RAU_creditSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(ret == -1) {
		sockError = tfGetSocketError(RAU_creditSocket);
		if(sockError == TM_EINPROGRESS ||			// ノンブロッキングで接続中はエラーではない
			sockError == TM_EISCONN) {				// 既に接続中はエラーではない(connect～GetSockError間で接続完了)
			ret = 0;								// エラーとしない
		}
	}
	if(ret == 0) {
		Credit_TcpConnect = 1;
	}
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CloseUpline
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線のTCP接続を切断する
 *| param	: resetReq	TRUE:TCPコネクション要因のクリア
 *[]----------------------------------------------------------------------[]
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	void	RAU_CloseCreditline(BOOL resetReq)
{
	tfClose(RAU_creditSocket);					// ソケットクローズ
	RAU_creditSockCondition = RAU_SOCK_CLOSING;	// クローズイベント待ち
	RauCtrl.tcpConnectFlag.port.credit = 0;
	CreditCtrl.TCPdiscnct_req = 0;
	
	// TIME_WAIT解除待ちタイマ(起動)
	if(RauConfig.Credit_discnct_tm != 0){
		CRE_tm_TCP_TIME_WAIT.bits0.bit_0_13 = RauConfig.Credit_discnct_tm;
		Credit_TCPtime_wait_rq = 1;
		Credit_TCPtime_wait_ov = 0;
	}
	else{
		Credit_TCPtime_wait_rq = 0;
		Credit_TCPtime_wait_ov = 1;
	}
	if(resetReq) {
		Credit_TcpConnReq_Clear();
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_creditSocketCallback
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線のソケットイベントコールバック関数
 *[]----------------------------------------------------------------------[]
 *| param	: sock		ソケットディスクリプタ
 *|			  event		通知イベント
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void 	RAU_creditSocketCallback(int sock, int event)
{
	if(RAU_creditSocket != sock) {									// ソケットが異なるまたは既にクローズ済み？
		return;
	}
	
	if( (event & TM_CB_RESET) || (event & TM_CB_REMOTE_CLOSE) || (event & TM_CB_SOCKET_ERROR) ) {
			if(RAU_creditSocket != -1) {
				if(Credit_TcpConnect == 1) {
					if(CreditCtrl.connRetryCount >= RauConfig.Credit_cnct_rty_cnt) {	// connectリトライオーバー
						Credit_SetError(ERR_CREDIT_HOST_COMMUNICATION);
						RAU_CloseCreditline(TRUE);
						CreditCtrl.TCPcnct_req = 0;						// TCP接続要求クリア
						CreditCtrl.connRetryCount = 0;						// リトライ回数クリア
						// データ再送待ちタイマ起動
						Credit_TcpConnect = 0;
						/* 接続失敗は送信要求の失敗なのでエラーイベントを投げる */
						queset(OPETCBNO, CRE_EVT_SNDERR, 0, NULL);
					}
					else {
						Credit_SetError(ERR_CREDIT_TCP_CONNECT);			// エラーコード９１(ＴＣＰコネクションタイムアウト)を通知
						tfClose(RAU_creditSocket);
						RAU_creditSockCondition = RAU_SOCK_CLOSING;
						++CreditCtrl.connRetryCount;
					}
				}
				else {
					RAU_creditSockCondition = RAU_SOCK_CLOSED;		/* クローズ完了状態へ移行				*/
					RAU_CloseCreditline(TRUE);
					Credit_TcpConnect = 0;
				}
			}
			else {
				RAU_creditSockCondition = RAU_SOCK_CLOSED;			/* クローズ完了状態へ移行				*/
				Credit_TcpConnect = 0;
			}
	}
	else if (event & TM_CB_CLOSE_COMPLT) {
			RAU_creditSockCondition = RAU_SOCK_CLOSED;				/* クローズ完了状態へ移行				*/
			RAU_creditSocket = -1;
	}
	else if (event & TM_CB_CONNECT_COMPLT) {
			CreditCtrl.TCPcnct_req = 0;							// TCP接続要求クリア
			CreditCtrl.connRetryCount = 0;							// リトライ回数クリア
			Credit_TcpConnect = 0;
			RAU_creditSockCondition = RAU_SOCK_CONNECTED;			/* コネクト完了状態へ移行				*/
	}
	return;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetUpNetState
 *[]----------------------------------------------------------------------[]
 *| summary	: 上り回線の状態を取得する
 *| retrun	: RAU_NET_IDLE				未接続
 *|			  RAU_NET_IDLE				PPP確立中
 *|			  RAU_NET_TCP_CONNECTIED	TCP確立中
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
RAU_NET_STATE	RAU_GetCreditNetState(void)
{
	if(KSG_gPpp_RauStarted) {						// PPP確立中
		if(RauCtrl.tcpConnectFlag.port.credit) {	// 上り回線TCP確立中
			return RAU_NET_TCP_CONNECTIED;
		}
		else {
			return RAU_NET_PPP_OPENED;
		}
	}
	
	return RAU_NET_IDLE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_ClearLineState
 *[]----------------------------------------------------------------------[]
 *| summary	: 回線状態をクリアする
 *| retrun	: none
 *[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
void	RAU_ClearLineState(void)
{

	// ダミーデータ連続送信回数クリア
	RAU_SendDummyContinueCount = 0;
}
void RAU_StopDownline(void)
{
	// RAU_downSocketをclose
	RAU_CloseDownline();
	// RAU_downListenSocketをclose
	tfClose(RAU_downListenSocket);
	RAU_downListenSocket = -1;
	// 状態遷移しないようにするため、RAU_SOCK_CONNECTEDをセットする
	RAU_downSockCondition = RAU_SOCK_CONNECTED;
}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
	// UT4000の "ntcom.h"をベースにFT4000の差分を移植して対応
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
#ifndef	___NTCOMH___
#define	___NTCOMH___
/*[]----------------------------------------------------------------------[]
 *|	filename: ntcom.h
 *[]----------------------------------------------------------------------[]
 *| summary	: NTモジュール RAM共有化用ラッパー定義
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
#include	"ntcomdef.h"

extern	unsigned char		NTCom_memory_pass[10];				// NTCom pass word

extern	T_NTCOM_NORMAL_DATABUF	NTComOpeRecvNormalDataBuf;		// NTComタスク通常受信バッファ（対OPE層）
extern	T_NTCOM_PRIOR_DATABUF	NTComOpeRecvPriorDataBuf;		// NTComタスク優先受信バッファ（対OPE層）
extern	T_NTCOM_NORMAL_DATABUF	NTComOpeSendNormalDataBuf;		// NTComタスク通常送信バッファ（対OPE層）
extern	T_NTCOM_PRIOR_DATABUF	NTComOpeSendPriorDataBuf;		// NTComタスク優先送信バッファ（対OPE層）

extern	T_NTCom_SndWork		NTCom_SndWork;
extern	T_NT_SND_TELEGRAM	NTCom_Tele_Broadcast;
extern	T_NT_SND_TELEGRAM	NTCom_Tele_Prior;
extern	T_NT_SND_TELEGRAM	NTCom_Tele_Normal;


/* ntcomcomm.c */
extern	T_NT_BLKDATA		send_blk;							// 送信バッファ		初期化時に引数で渡す
extern	T_NT_BLKDATA		receive_blk;						// 受信バッファ

extern	ushort NT_MakeWord(unsigned char *data);
extern	void NT_Word2Byte(unsigned char *data, ushort us);
extern	void NTComComm_create_eot_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail);


/* ntcomcomdr.c */
extern	T_NT_COMDR_CTRL		NTComComdr_Ctrl;					/* COMDR制御データ */


/* ntcomdata.c */
//#define			_NTComData_ResetTelegram(tele)	((tele)->curblk = -1)
#define			_NTComData_GetBlkNum(tele)		((tele)->blknum)
#define			_NTComData_GetRemainBlkNum(tele)	((tele)->blknum - (tele)->curblk)

extern	BOOL	NTComData_IsValidTelegram(T_NT_TELEGRAM *telegram);
extern	BOOL	NTComData_PeekRcvTele_Broadcast(T_NT_TELEGRAM *telegram);
extern	BOOL	NTComData_PeekRcvTele_Prior(T_NT_TELEGRAM *telegram);
extern	BOOL	NTComData_PeekRcvTele_Normal(T_NT_TELEGRAM *telegram);
extern	void	NTComData_InitBuffer(T_NT_BUFFER *buffer, T_NT_BLKDATA *mempool, int mempool_size);
extern	void	NTComData_ResetTelegram(T_NT_TELEGRAM *telegram);


/* データ管理機能 制御データ */
extern	T_NTComData_Ctrl		NTComData_Ctrl;
/* 通常データバッファ */
extern	T_NT_BUFFER			NTCom_SndBuf_Normal;				/* NTネットへの送信用 */
extern	T_NT_BUFFER			NTCom_RcvBuf_Normal;				/* NTネットからの受信用 */
/* _z_NTCom_RcvBuf_Normal内の電文情報 */
/* 最大8つの端末から受信したデータをFIFOでIBWに送信するため、下記データを使用してスケジューリングする */
extern	T_NT_TELEGRAM_LIST	NTCom_RcvTeleSchedule;
/* 優先データバッファ */
extern	T_NT_BUFFER			NTCom_SndBuf_Prior;					/* #005 */
extern	T_NT_BUFFER			NTCom_RcvBuf_Prior;					/* NTネットからの受信用 */
/* 同報データバッファ */
extern	T_NT_BUFFER			NTCom_SndBuf_Broadcast;				/* NTネットへの送信用 */
extern	T_NT_BUFFER			NTCom_RcvBuf_Broadcast;				/* NTネットからの受信用 */
/* データバッファメモリプール */
extern	T_NT_BUFMEM_POOL	NTCom_BufMemPool;
/* エラー情報バッファ */
extern	T_NT_ERR_INFO		NTCom_Err;
/* 停電保証制御データ */
extern	T_NT_FUKUDEN		NTCom_Fukuden;

extern	eNT_DATASET	NTComData_SetSndBlock(T_NT_BLKDATA *blk);


/* ntcommain.c */
extern	uchar				NTCom_Condit;							/* task condition */
extern	uchar				NTCom_SciPort;							/* use SCI channel number (0 or 2) */
/** received command from IBW **/
//extern	t_NTCom_RcvQueCtrl	NTCom_RcvQueCtrl;						/* queue control data */
															/* If you want to get this data, you call "NTCom_ArcRcvQue_Read()" */
/** initial setting command received flag **/
extern	T_NT_INITIAL_DATA	NTCom_InitData;							/* initial setting data */
extern	t_NT_ComErr			NTCom_ComErr;

extern	void	NTCom_err_chk(char code, char kind, char f_data, char err_ctl, void *pData);


/* ntcomsci.c */
/* SCI制御データ */
extern	T_NT_SCI_CTRL		NTComSci_Ctrl;

extern	void	NTComSci_Stop(void);


/* ntcomslave.c */
extern	T_NT_BLKDATA		*NTComSlave_send_blk;					// 送信バッファ		初期化時に引数で渡す
extern	T_NT_BLKDATA		*NTComSlave_receive_blk;					// 受信バッファ
extern	T_NTCOM_SLAVE_CTRL	NTComComm_Ctrl_Slave;						// 従局管理用構造体
extern	T_NT_TELEGRAM		normal_telegram;						// 通常データ電文情報
extern	T_NT_TELEGRAM		prior_telegram;						// 優先データ電文情報
extern	T_NT_TELEGRAM		*last_telegram;						// 最後に送った電文


/* ntcomtimer.c */
/* 1ms精度グループ */
extern	T_NT_TIM_CTRL		NTComTimer_1ms[NT_TIM_1MS_CH_MAX];
/* 10ms精度グループ */
extern	T_NT_TIM_CTRL		NTComTimer_10ms[NT_TIM_10MS_CH_MAX];
/* 100ms精度グループ */
extern	T_NT_TIM_CTRL		NTComTimer_100ms[NT_TIM_100MS_CH_MAX];
/* タイマONOFF制御 */
extern	BOOL				NTComTimer_Enable;	/* TRUE=有効 */

extern	void	NTCom_TimoutChk();


/* ntcombuf.c */
extern	uchar	NTCom_GetSendPriorDataID(void);
extern	uchar	NTCom_GetSendNormalDataID(void);
extern	void	NTCom_ClearData(uchar mode);

extern	eNTNET_RESULT	NTCom_SetSendData(const uchar* pData, ushort size, uchar type);
extern	ushort	NTCom_GetSendData(uchar type, T_NT_BLKDATA* pRecvData);
extern	BOOL	NTCom_CanSetSendData(uchar type);

extern	uchar	read_rotsw(void);			// ロータリースイッチの状態検出
extern	void	NTCom_Init( void );			// NT task initial routine
extern	void	NTComData_Clear(void);		// データ管理モジュール内のデータ全てクリア
extern	void	NTComComm_Clear( void );	// パスワード破壊時処理
extern	void	NTCom_FuncStart( void );	// NTCom function start
extern	void	NTCom_FuncMain( void );		// credit function Main routine
											// タイマー機能
extern	ushort	NTComTimer_Create(ushort unit, ushort time, void (*func)(void), BOOL cyclic);
extern	BOOL	NTComTimer_Start(ushort id);// タイマースタート
											// 受信パケット取得
extern	eNT_COMDR_STS	NTComComdr_GetRcvPacket(T_NT_BLKDATA *buf);
											// 受信したパケットの状態をチェックする。
											// パケット送信(送信完了かタイムアウトまでリターンしない))
extern	eNT_COMDR_SND	NTComComdr_SendPacket(T_NT_BLKDATA *buf, int wait, int timeout);
extern	BOOL	NTComComm_packet_check(T_NT_BLKDATA* check_blk, uchar seq_no, uchar terminal_no);
extern	void	NTComComm_create_stx_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail);
extern	void	NTComComm_create_ack_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail);
											// CRCをチェックする。
extern	BOOL	NTComComm_crc_check(T_NT_BLKDATA* buff);
											// 端末接続状態変化通知
extern	BOOL	NTComData_SetTerminalStatus(int terminal, eNT_STS sts, T_NT_TELEGRAM *telegram_normal, T_NT_TELEGRAM *telegram_prior);
											// ブロックポインタの位置にあるデータを取得する
extern	int		NTComData_GetSendBlock(T_NT_TELEGRAM *telegram, T_NT_BLKDATA *blk);
											// バッファから電文データを削除する
extern	void	NTComData_DeleteTelegram(T_NT_TELEGRAM *telegram);
											// ブロックポインタの位置にあるデータを取得する
extern	int		NTComData_GetSendBlock(T_NT_TELEGRAM *telegram, T_NT_BLKDATA *blk);
extern	void	NTComComm_create_nak_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail);
											// 受信バッファにデータセット
extern	eNT_DATASET	NTComData_SetRcvBlock(T_NT_BLKDATA *blk);
											// 送信可能な優先データ電文の情報を取得
extern	BOOL	NTComData_PeekSndTele_Prior(int terminal, T_NT_TELEGRAM *telegram);
											// 送信可能な通常データ電文の情報を取得
extern	BOOL	NTComData_PeekSndTele_Normal(int terminal, T_NT_TELEGRAM *telegram);
											// バッファ状態取得
extern	void	NTComData_GetBufferStatus(T_NT_BUF_STATUS *bufsts, int terminal);
											// 受信バッファに含まれるブロック数を取得する
extern	void	NTComData_GetRcvBufferCount(T_NT_BUF_COUNT *bufcnt);
											// ブロックポインタを移動する
extern	int		NTComData_SeekBlock(T_NT_TELEGRAM *telegram, int offset, eNT_SEEK origin);
extern	void	NTComData_Start(void);		// データ管理モジュール初期化
extern	void	NTComTimer_Init(void);		// タイマー機能初期化
extern	void	NTComComm_Start( void );	// NT－NET開始処理
extern	void	NTComComm_Main( void );		// NT－NETメイン処理
											// ブロックポインタの位置にあるデータを取得する
extern	int		NTComData_GetRecvBlock(T_NT_TELEGRAM *telegram, T_NTCom_SndWork *sndWork);
											// NTComComdrモジュール初期化
extern	void	NTComComdr_Init(ushort timeout_c2c, uchar port, uchar speed, uchar dbit, uchar sbit, uchar pbit);
											// NTCom READER communication parts initialize routine
extern	void	NTComSlave_Start(T_NT_BLKDATA* send, T_NT_BLKDATA* receive);
											// NTCom READER communication parts initialize routine
extern	void 	NTComSlave_Clear( void );
											// NTCom READER function communication parts Main routine
extern	void	NTComSlave_Main( void );
											// 新形式の時計データ電文か否かの判定関数
extern	BOOL	NT_IsNewTypeTimeSetPacket( T_NT_BLKDATA *blk );
extern	BOOL 	NTComSlave_isIdle( void );	// タスクのアイドル状態を取得する
											// NTComComdrモジュール初期化
extern	void	NTComComdr_Init(ushort timeout_c2c, uchar port, uchar speed, uchar dbit, uchar sbit, uchar pbit);
											// ポート初期化
extern	void	NTComSci_Init(T_NT_SCI_CALLBACK RcvFunc, T_NT_SCI_CALLBACK StsFunc,
				uchar Port, uchar Speed, uchar Dbits, uchar Sbits, uchar Pbit);
extern	void	NTComSci_Start(void);		// ポート動作開始
											// シリアル送信を開始する
extern	BOOL	NTComSci_SndReq(uchar *buf, ushort len);
extern	BOOL	NTComSci_IsSndCmp(void);	// シリアル送信完了を取得する

#endif/* ___NTCOMH___ */
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

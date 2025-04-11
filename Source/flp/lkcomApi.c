/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		IF盤通信-LOCKﾒｲﾝｲﾝﾀｰﾌｪｰｽ																			   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	lkcomApi.c																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・IF盤との通信及びMAINﾀｽｸとのｲﾝﾀｰﾌｪｰｽ																	   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include	<string.h>															/*								*/
#include	<stdlib.h>															/*								*/
#include	"iodefine.h"														/*								*/
#include	"system.h"															/*								*/
#include	"flp_def.h"															/*								*/
#include	"prm_tbl.h"															/*								*/
#include	"LKcom.h"															/*								*/
#include	"LKmain.h"															/*								*/
#include	"mem_def.h"															/*								*/
#include	"rkn_def.h"															/*								*/
#include	"rkn_cal.h"															/*								*/
#include	"ope_def.h"															/*								*/
#include	"common.h"															/*								*/
#include	"ntnet.h"															/*								*/
#include	"message.h"
#include	"mnt_def.h"															/*								*/
#include	"IFM.h"

// ﾀｰﾐﾅﾙ状態定義
enum {
	IBCCOM_IBCSND_NORMAL,			// 正常
	IBCCOM_IBCSND_ERROR,			// 異常
	IBCCOM_IBCSND_ERRRSLT,			// IBC前回ﾃﾞｰﾀ未処理
};

// IBCﾀｰﾐﾅﾙ送信状態
enum {
	IBCCOM_IBCSND_IDLE,				// ｱｲﾄﾞﾙ
	IBCCOM_IBCSND_WAIT_SEND,		// 送信完了待ち
};

#define	WAIT_RESPONSE_TIME		(60000+1) / 500		// 送信結果応答待ち時間(60s)
#define	TIMEOUT_RETRY_MAX		3					// 送信結果応答受信ﾀｲﾑｱｳﾄにおけるﾘﾄﾗｲ回数
#define	TIMEOUT_RETRY_MAX_RSLT	1					// 送信結果ﾃﾞｰﾀがIBC前回ﾃﾞｰﾀ未処理追加だったときのﾘﾄﾗｲ回数
#define	BLOCK_RETRY_MAX			3					// ﾌﾞﾛｯｸNo.異常時におけるﾘﾄﾗｲ回数
#define	SERIAL_NO_MAX			99					// 追い番最大数
#define	QUE_NUM_MAX				10					// 各ｷｭｰへの最大ｽﾄｯｸ数

/* 受信データ区分 */
#define DV_FLPCTRL_I	0x05	/* ﾌﾗｯﾌﾟ状態 */
#define DV_FLPCTRL_A	0x06	/* 全ﾌﾗｯﾌﾟ状態 */
#define DV_FLPTST		0x07	/* ﾃｽﾄ要求 */
#define DV_FLPMNT		0x08	/* ﾒﾝﾃﾅﾝｽ情報要求 */

uchar	DownLockFlag[LOCK_MAX];


// function
char		LKcom_SetDtPkt( uchar, uchar, ushort, uchar );						/*								*/
uchar		LKcom_RcvDataGet( ushort *cnt );									/*								*/
void		LKcom_SetLockMaker( void );											/*								*/
void		LKopeApiLKCtrl( ushort, uchar );									/* ﾛｯｸ装置指示					*/
void		LKopeApiLKCtrl_All(uchar, uchar);
uchar		get_lktype(uchar lok_syu);
short		LKopeGetLockNum( uchar, ushort, ushort * );							/*								*/

#if (1 == AUTO_PAYMENT_PROGRAM)
void LK_AutoPayment_Send(uchar index, uchar tno, uchar lkno, uchar kind);
#endif
																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ﾃﾞｰﾀﾊﾟｹｯﾄ送信要求（API）																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKcom_SetDtPkt(par1,par2,par3)															   |*/
/*| PARAMETER	: par1	: ﾃﾞｰﾀID.																			   |*/
/*|				: par2	: 端末№																			   |*/
/*|				: par3	: ﾛｯｸ装置№																			   |*/
/*|				: par4	: 処理区分(制御ﾃﾞｰﾀに必要となる処理区分)											   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・ｱﾌﾟﾘｹｰｼｮﾝにて送信したいﾃﾞｰﾀをﾊﾞｯﾌｧへｾｯﾄする															   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
char	LKcom_SetDtPkt(															/*								*/
uchar	did,																	/* ﾃﾞｰﾀID						*/
uchar	tno,																	/* 端末No.(-1とする)			*/
ushort	lkno,																	/* ﾛｯｸ装置№(1～324)			*/
uchar	kind )																	/* 処理区分(制御ﾃﾞｰﾀのみ有効)	*/
{																				/*								*/
ushort			i;																/*								*/
t_LKcomCtrl		*c_dp;															/*								*/
ushort			wk_l;															/* ﾜｰｸ：ﾛｯｸ装置№0～324-1		*/
	wk_l = lkno - 1;															/*								*/

	if( IFS_CRR_OFFSET > tno ){													/* CRR基板へのアクセスではない	*/
		if( LKcom_Search_Ifno( tno ) ){											/* ﾀｰﾐﾅﾙ番号からCRRかCRAかを判断 */
			if(!( GetCarInfoParam() & 0x04 )){									/* CRR基板接続が無ければ抜ける	*/
				return(0);														/* 								*/
			}
		}else{
			if(!( GetCarInfoParam() & 0x01 )){									/* CRA基板接続が無ければ抜ける	*/
				return(0);														/* 								*/
			}
		}
	}
	switch( did ){																/*								*/
																				/*								*/
																				/*								*/
	case LK_SND_CTRL_ID:														/* ﾛｯｸ装置制御ﾃﾞｰﾀ				*/
		if( LOCK_CTRL_BUFSIZE_MAX <= LKcomTerm.CtrlInfo.Count ){				/* ﾊﾞｯﾌｧFULL状態				*/
			++LKcomTerm.CtrlInfo.R_Index;										/* 古いﾃﾞｰﾀを消去				*/
			if( LOCK_CTRL_BUFSIZE_MAX <= LKcomTerm.CtrlInfo.R_Index ){			/* 								*/
				LKcomTerm.CtrlInfo.R_Index = 0;									/*								*/
			}																	/* 								*/
			--LKcomTerm.CtrlInfo.Count;											/* 								*/
		}																		/*								*/
																				/*								*/
		c_dp = &LKcomTerm.CtrlData[LKcomTerm.CtrlInfo.W_Index];					/*								*/
																				/*								*/
		c_dp->did4 = LK_SND_CTRL_ID;											/* ﾃﾞｰﾀID						*/
		c_dp->kind = kind;														/* 処理区分						*/
		c_dp->tno = tno;														// CRBインターフェースのターミナルNo
		if( wk_l < LOCK_MAX ){													/* 車室0～324-1?				*/
			if(LKcom_Search_Ifno(tno) == 0) {									// ロックの場合
				c_dp->lock = LockInfo[wk_l].lok_no;								/* ﾛｯｸ装置No.					*/
			} else {															// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
				c_dp->lock = 1;													/* ﾛｯｸ装置No.					*/
			}
			if(wk_l < FLAP_NUM_MAX)												/*  装置種別がフラップ			*/
			{
				c_dp->clos_tm = 0;												/* 閉動作信号出力時間			*/
				c_dp->open_tm = 0;												/* 開動作信号出力時間			*/
				if(kind == 1)													/* 動作要求が上昇 				*/
				{
					/* フラップ上昇開始情報登録 */
					IoLog_write(IOLOG_EVNT_FLAP_UP_STA, (ushort)LockInfo[wk_l].posi, 0, 0);
				}
				else if(kind == 2)												/* 動作要求が下降 				*/
				{
					/* フラップ下降開始情報登録 */
					IoLog_write(IOLOG_EVNT_FLAP_DW_STA, (ushort)LockInfo[wk_l].posi, 0, 0);
				}
			}
			else																/* 装置種別がロック				*/
			{
				c_dp->clos_tm = LockMaker[ LockInfo[wk_l].lok_syu - 1 ].clse_tm;	/* 閉動作信号出力時間			*/
				c_dp->open_tm = LockMaker[ LockInfo[wk_l].lok_syu - 1 ].open_tm;	/* 開動作信号出力時間			*/
				if(kind == 1)													/* 動作要求が上昇 				*/
				{
					/* 駐輪ロック閉開始情報登録 */
					IoLog_write(IOLOG_EVNT_LOCK_CL_STA, (ushort)LockInfo[wk_l].posi, 0, 0);
				}
				else if(kind == 2)												/* 動作要求が下降 				*/
				{
					/* 駐輪ロック開開始情報登録 */
					IoLog_write(IOLOG_EVNT_LOCK_OP_STA, (ushort)LockInfo[wk_l].posi, 0, 0);
				}
			}
		}else{																	/*								*/
			c_dp->lock = 0;														/* ﾛｯｸ装置No.					*/
			c_dp->clos_tm = 0;													/* 閉動作信号出力時間			*/
			c_dp->open_tm = 0;													/* 開動作信号出力時間			*/
			for(i = INT_CAR_START_INDEX; i < LOCK_MAX; i++)						/* 車室情報管理テーブル検索 	*/
			{
				WACDOG;															// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
				if(LockInfo[i].if_oya == tno)									/* IF基盤が一致 				*/
				{
					if(i < FLAP_NUM_MAX)										/* 装置種別がフラップ			*/
					{
						if(kind == 5)											/* 動作要求が上昇 				*/
						{
							/* フラップ上昇開始情報登録 */
							IoLog_write(IOLOG_EVNT_FLAP_UP_STA, (ushort)LockInfo[i].posi, 0, 0);
						}
						else if(kind == 6)										/* 動作要求が下降 				*/
						{
							/* フラップ下降開始情報登録 */
							IoLog_write(IOLOG_EVNT_FLAP_DW_STA, (ushort)LockInfo[i].posi, 0, 0);
						}
					}
					else														/* 装置種別がロック				*/
					{
						if(kind == 5)											/* 動作要求が上昇 				*/
						{
							/* 駐輪ロック閉開始情報登録 */
							IoLog_write(IOLOG_EVNT_LOCK_CL_STA, (ushort)LockInfo[i].posi, 0, 0);
						}
						else if(kind == 6)										/* 動作要求が下降 				*/
						{
							/* 駐輪ロック開開始情報登録 */
							IoLog_write(IOLOG_EVNT_LOCK_OP_STA, (ushort)LockInfo[i].posi, 0, 0);
						}
					}
				}
			}
		}																		/*								*/
																				/*								*/
		++LKcomTerm.CtrlInfo.W_Index;											/*								*/
		if( LOCK_CTRL_BUFSIZE_MAX <= LKcomTerm.CtrlInfo.W_Index ){				/*								*/
			LKcomTerm.CtrlInfo.W_Index = 0;										/*								*/
		}																		/*								*/
		++LKcomTerm.CtrlInfo.Count;												/*								*/
		break;																	/*								*/
																				/*								*/
	default:																	/*								*/
		break;																	/*								*/
	}																			/*								*/
	return( 0 );																/*								*/
}																				/*								*/
																				/*								*/
/*	ｱﾌﾟﾘｹｰｼｮﾝ側で使用する関数						 	*/
/*	受信したﾃﾞｰﾀからﾃﾞｰﾀﾌｫｰﾏｯﾄに応じたﾃﾞｰﾀを取得する	*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	受信ﾃﾞｰﾀ取得(API)																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKcom_RcvDataGet()																		   |*/
/*| PARAMETER	: par1	: 格納ﾌﾞﾛｯｸ数																		   |*/
/*|				: par2	: 格納ﾀｲﾌﾟ	0:今回のﾃﾞｰﾀのみｱﾌﾟﾘﾊﾞｯﾌｧへ格納											   |*/
/*|									1:今回のﾃﾞｰﾀを一時ﾊﾞｯﾌｧへ格納											   |*/
/*|									2:今回と一時ﾊﾞｯﾌｧのﾃﾞｰﾀをｱﾌﾟﾘﾊﾞｯﾌｧへ格納								   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・ｱﾌﾟﾘｹｰｼｮﾝへ引き渡し用ﾊﾞｯﾌｧに受信ﾃﾞｰﾀ件数があればﾃﾞｰﾀの内容をある程度解析し変化があれば通知する		   |*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	LKcom_RcvDataGet( ushort *cnt )											/*								*/
{																				/*								*/
																				/*								*/
t_LKcomTest		*lk_tst;														/*								*/
t_LKcomMnt		*lk_mnt;														/*								*/
t_LKcomSubLock	*lk_lock2;														/*								*/
t_LKcomEcod		*lk_ecod;														/*								*/

																				/*								*/
																				/*								*/
ushort	d_siz;																	/* ﾃﾞｰﾀｻｲｽﾞ						*/
uchar	d_id;																	/* ﾃﾞｰﾀID						*/
uchar	tno;																	/* 端末№						*/
uchar	ret;																	/*								*/
ushort	i, j;																	/*								*/
																				/* 0:変化なし					*/
																				/* 1:変化あり(処理あり)			*/
																				/* 2:変化あり(処理なし)			*/
																				/*								*/
union {																			/*								*/
	uchar	uc[2];																/*								*/
	ushort	us;																	/*								*/
}u;																				/*								*/

ushort	lkmax = FLAP_IFS_MAX;
																				/*								*/
	ret = 0;																	/*								*/
	if( LKcom_RcvData.dcnt == 0 ){												/* 受信ﾃﾞｰﾀなし					*/
		return( ret );															/* 何もしないで抜ける			*/
	}																			/*								*/
																				/*								*/
	u.uc[0] = LKcom_RcvData.data[0];											/* ﾃﾞｰﾀｻｲｽﾞ取得					*/
	u.uc[1] = LKcom_RcvData.data[1];											/*								*/
	d_siz = u.us;																/*								*/

// data[2]=ID,[3]=データ保持,[4]ターミナルNo,[5]以降データ
	tno = (uchar)(LKcom_RcvData.data[4]);										/* 端末№						*/
	d_id = LKcom_RcvData.data[2];												/* ﾃﾞｰﾀID4取得					*/
	memcpy( &LKcom_RcvDtWork[0], &LKcom_RcvData.data[2],(size_t)((ushort)(d_siz-2)) );	/*						*/
																				/*								*/
	if( d_id <= LK_RCV_ECOD_ID ){												/* CRR/CRAからの受信電文		*/
		if(LOCK_REQ_ALL_TNO == tno){// ロック装置全てに対する要求の結果の場合
		// 通常状態データ（全て）、メンテナンス情報要求応答、バージョン要求応答の場合
			lkmax = LK_LOCK_MAX;												//接続装置最大数(ﾛｯｸ装置は90)
		}
		else if( FLAP_REQ_ALL_TNO == tno) {
			lkmax = INT_FLAP_MAX;											//1ﾀｰﾐﾅﾙの接続装置最大数(内蔵Flap装置は10)
		}else if( LKcom_Search_Ifno( (uchar)tno ) ){
			if(!( GetCarInfoParam() & 0x04 )){									/* CRR基板接続が無ければ抜ける	*/
				LKcom_RcvDataDel();												/* 受信ﾃﾞｰﾀ削除					*/
				return( 8 );
			}else{
				lkmax = INT_FLAP_MAX;											//1ﾀｰﾐﾅﾙの接続装置最大数(内蔵Flap装置は10)
			}
		}else{
			lkmax = LK_LOCK_MAX;												//接続装置最大数(ﾛｯｸ装置は90)
		}
	}
	switch( d_id )																/*								*/
	{																			/*								*/
																				/*								*/
	case LK_RCV_COND_ID:														/* 状態ﾃﾞｰﾀID(可変)				*/
		memset( &LockBuf, 0xff, sizeof( t_LKBUF )*lkmax );						// 0xffffで書き換える
		*cnt = 0;
		if( LKcom_RcvDtWork[3] != 0 ){											// 状態データのデータ数(車室数)有
			for( i=0; i<LKcom_RcvDtWork[3]; i++ ){								// 送信されてきた情報分ループ
				// ロック装置
				//NOTE:LCM_NoticeStatus_toPAYまたはLCM_NoticeStatusall_toPAYでデータはセットされる
				lk_lock2 = (t_LKcomSubLock *)&LKcom_RcvDtWork[4+(3*i)];
				if( (lk_lock2->lock_cnd == 7) || (lk_lock2->lock_cnd == 8) ){	/*								*/
					// 受信ﾃﾞｰﾀ＝「手動ﾓｰﾄﾞ中」または「手動ﾓｰﾄﾞ解除」
					LockBuf[*cnt].LockNoBuf = (ushort)tno;						/* 装置No.にﾀｰﾐﾅﾙNo.をｾｯﾄ		*/
				}																/*								*/
				else{															/*								*/
					LockBuf[*cnt].LockNoBuf = (ushort)(lk_lock2->lock_no);		// LockInfoのインデックスをセット
				}
				LockBuf[*cnt].car_cnd  = lk_lock2->car_cnd;
				LockBuf[*cnt].lock_cnd  = lk_lock2->lock_cnd;
				*cnt += 1;
			}
		}
		if( *cnt )	ret = 2;
		LKcom_RcvDataDel();														// 受信ﾃﾞｰﾀ削除
		break;
																				/*								*/
	case LK_RCV_TEST_ID:														/* ﾃｽﾄﾃﾞｰﾀID					*/
		lk_tst = (t_LKcomTest *)&LKcom_RcvDtWork;								/*								*/
		memcpy( &MntLockTest[0],&lk_tst->lock_tst[0], (size_t)lkmax );			/* ﾛｯｸ装置開閉回数				*/
		queset( OPETCBNO, CTRL_PORT_CHK_RECV, 0, NULL );
		LKcom_RcvDataDel();														/* 受信ﾃﾞｰﾀ削除					*/
		ret = 3;																/*								*/
		break;																	/*								*/
																				/*								*/
	case LK_RCV_MENT_ID:														/* ﾒﾝﾃﾅﾝｽﾃﾞｰﾀID					*/
		lk_mnt = (t_LKcomMnt *)&LKcom_RcvDtWork[3];	
		// ﾃﾞｰﾀ受信用変換ﾃｰﾌﾞﾙ作成
		// 受信IF盤No.のﾛｯｸ装置連番毎に接続有無の設定を検索し接続ありなら装置No.を変換ﾃｰﾌﾞﾙへｾｯﾄする
		memset( MntLockWkTbl, 0, sizeof( MntLockWkTbl ) );	// ﾃﾞｰﾀ受信用変換ﾃｰﾌﾞﾙ0ｸﾘｱ

		if( lkmax == LK_LOCK_MAX ){// 駐輪
			for( i=0 ; i<BIKE_LOCK_MAX ; i++ ){				// ﾛｯｸ装置連番最大まで受信ﾃﾞｰﾀを検索
				MntLockDoCount[i][0]  = *((ulong*)lk_mnt->lock_mnt[i].auto_cnt);		// 動作回数ｾｰﾌﾞ
				MntLockDoCount[i][1]  = *((ulong*)lk_mnt->lock_mnt[i].manu_cnt);		// 手動回数ｾｰﾌﾞ
				MntLockDoCount[i][2]  = *((ulong*)lk_mnt->lock_mnt[i].trbl_cnt);		// 故障回数ｾｰﾌﾞ
			}
		}
		else if( lkmax == INT_FLAP_MAX ) {	// フラップ
			for( i=INT_CAR_START_INDEX, j=0 ; i< INT_CAR_START_INDEX + INT_FLAP_MAX ; i++, j++ ){	// フラップ連番最大まで受信ﾃﾞｰﾀを検索
				MntFlapDoCount[i][0]  = *((ulong*)lk_mnt->lock_mnt[i].auto_cnt);		// 動作回数ｾｰﾌﾞ
				MntFlapDoCount[i][1]  = *((ulong*)lk_mnt->lock_mnt[i].manu_cnt);		// 手動回数ｾｰﾌﾞ
				MntFlapDoCount[i][2]  = *((ulong*)lk_mnt->lock_mnt[i].trbl_cnt);		// 故障回数ｾｰﾌﾞ
			}
		}
		LKcom_RcvDataDel();														/* 受信ﾃﾞｰﾀ削除					*/
		ret = 4;																/*								*/
		break;																	/*								*/
																				/*								*/
																				/*								*/
	case LK_RCV_ECOD_ID:														/*								*/
		lk_ecod = (t_LKcomEcod *)&LKcom_RcvDtWork;								/*								*/
		LKopeErrRegist( tno, lk_ecod );											/*								*/
		LKcom_RcvDataDel();														/* 受信ﾃﾞｰﾀ削除					*/
		ret = 7;																/*								*/
		break;																	/*								*/
	case CRR_RCV_TEST_ID:
		queset( OPETCBNO, CTRL_PORT_CHK_RECV, 0, NULL );						/* 受信完了を伝える				*/
		LKcom_RcvDataDel();														/* 受信ﾃﾞｰﾀ削除					*/
		ret = 9;																/*								*/
		break;																	/*								*/
	default:																	/* それ以外のIDの場合			*/
		/* ありえないがｾｰﾌﾃｨ機能を入れる */
		LKcom_RcvDataDel();														/* 受信ﾃﾞｰﾀ削除					*/
		ret = 8;																/*								*/
		break;																	/*								*/
																				/*								*/
	}																			/*								*/
	return( ret );																/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	受信済み状態ﾃﾞｰﾀ解析処理																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKcom_RcvDataAnalys()																		   |*/
/*| PARAMETER	: par1	: 受信したﾃﾞｰﾀ数																	   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: R.HARA(AMANO)																				   |*/
/*| Date		: 2005-09-12																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・ｱﾌﾟﾘｹｰｼｮﾝへ引き渡し用ﾊﾞｯﾌｧに受信ﾃﾞｰﾀ件数があればﾃﾞｰﾀの内容をある程度解析し変化があれば通知する		   |*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	LKcom_RcvDataAnalys( ushort cnt )										/*								*/
{																				/*								*/
	flp_com		*lk_tbl;														/*								*/
	ushort		nst_bak, ost_bak;												/* 現ｽﾃｰﾀｽのﾊﾞｯｸｱｯﾌﾟ			*/
	uchar		wk_hit;															/* 装置毎の状態の変化ﾌﾗｸﾞ		*/
	uchar		ret;															/*								*/
	ulong		lkNo_w;															/* 接客用ﾛｯｸ装置№				*/
	uchar		tno_wk;
	uchar		lk_kind;
	uchar		tno_max;
	uchar		set_mode;
	ushort		moni_no;
	uchar		InOutLog=0;
																				/*								*/
																				/*								*/
	ret = 0;																	/*								*/
	wk_hit = 0;																	/*								*/
																				/*								*/
	lk_tbl = &FLAPDT.flp_data[LockBuf[cnt].LockNoBuf];							/*								*/
	nst_bak = lk_tbl->nstat.word;												/* 現ｽﾃｰﾀｽのﾊﾞｯｸｱｯﾌﾟ			*/
	ost_bak = lk_tbl->ostat.word;												/* 旧ｽﾃｰﾀｽのﾊﾞｯｸｱｯﾌﾟ			*/
	lk_tbl->ostat.word = lk_tbl->nstat.word;									/* 現ｽﾃｰﾀｽを旧ｽﾃｰﾀｽにｺﾋﾟｰ		*/
	lkNo_w = (ulong)( LockInfo[LockBuf[cnt].LockNoBuf].area*10000L +			/*								*/
					  LockInfo[LockBuf[cnt].LockNoBuf].posi );					/* 接客用ﾛｯｸ装置№				*/
																				/*								*/
	if( lk_tbl->nstat.bits.b09 && LockBuf[cnt].lock_cnd != 0 ){					/* NT-NETﾌﾗｸﾞON					*/
		if( lk_tbl->nstat.bits.b15 == 1 ){
			NTNET_Snd_Data05( lkNo_w, LockBuf[cnt].car_cnd, LockBuf[cnt].lock_cnd );/* NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成	*/
			lk_tbl->nstat.bits.b14 = 0;
		}
		lk_tbl->nstat.bits.b09 = 0;												/* NT-NETﾌﾗｸﾞOFF				*/
	}																			/*								*/

	/*----------------*/
	/* 手動ﾓｰﾄﾞのﾁｪｯｸ */
	/*----------------*/
	if( (LockBuf[cnt].lock_cnd == 7) || (LockBuf[cnt].lock_cnd == 8) ){			/* 「手動ﾓｰﾄﾞ発生」or「手動ﾓｰﾄﾞ解除」？	*/

		tno_wk = (uchar)LockBuf[cnt].LockNoBuf;

		if( (tno_wk & 0x80) == 0 ){												/* 装置種別？							*/

			// ﾌﾗｯﾌﾟ装置
			lk_kind = LK_KIND_FLAP;												/* 装置種別（ﾌﾗｯﾌﾟ）					*/
			tno_max = FLAP_IF_MAX;												/* ﾌﾗｯﾌﾟ装置ﾀｰﾐﾅﾙNo.最大値				*/

			if( LockBuf[cnt].lock_cnd == 7 ){
				// 手動ﾓｰﾄﾞ発生
				set_mode = ON;													/* 手動ﾓｰﾄﾞﾌﾗｸﾞ更新ﾃﾞｰﾀ（ON）ｾｯﾄ		*/
				moni_no  = OPMON_FLAP_MMODE_ON;									/* ﾓﾆﾀｰ情報No.（ﾌﾗｯﾌﾟ装置手動ﾓｰﾄﾞ発生）	*/
			}
			else{
				// 手動ﾓｰﾄﾞ解除
				set_mode = OFF;													/* 手動ﾓｰﾄﾞﾌﾗｸﾞ更新ﾃﾞｰﾀ（OFF）ｾｯﾄ		*/
				moni_no  = OPMON_FLAP_MMODE_OFF;								/* ﾓﾆﾀｰ情報No.（ﾌﾗｯﾌﾟ装置手動ﾓｰﾄﾞ解除）	*/
			}
		}
		else{
			// ﾛｯｸ装置
			lk_kind = LK_KIND_LOCK;												/* 装置種別（ﾛｯｸ装置）					*/
			tno_max = LOCK_IF_MAX;												/* ﾛｯｸ装置ﾀｰﾐﾅﾙNo.最大値				*/
			tno_wk = (uchar)(tno_wk & 0x7f);									/* ﾀｰﾐﾅﾙNo.変換							*/
			if( LKcom_Search_Ifno( tno_wk )){
				lk_kind = LK_KIND_INT_FLAP;										/* 装置種別（ﾛｯｸ装置）					*/
			}
			if( LockBuf[cnt].lock_cnd == 7 ){
				// 手動ﾓｰﾄﾞ発生
				set_mode = ON;													/* 手動ﾓｰﾄﾞﾌﾗｸﾞ更新ﾃﾞｰﾀ（ON）ｾｯﾄ		*/
				moni_no  = OPMON_LOCK_MMODE_ON;									/* ﾓﾆﾀｰ情報No.（ﾛｯｸ装置手動ﾓｰﾄﾞ発生）	*/
				moni_no  = ( lk_kind == LK_KIND_INT_FLAP ? 
							OPMON_FLAP_MMODE_ON:OPMON_LOCK_MMODE_ON);			/* ﾓﾆﾀｰ情報No.（ﾛｯｸ装置手動ﾓｰﾄﾞ発生）	*/
			}
			else{
				// 手動ﾓｰﾄﾞ解除
				set_mode = OFF;													/* 手動ﾓｰﾄﾞﾌﾗｸﾞ更新ﾃﾞｰﾀ（OFF）ｾｯﾄ		*/
				moni_no  = OPMON_LOCK_MMODE_OFF;								/* ﾓﾆﾀｰ情報No.（ﾛｯｸ装置手動ﾓｰﾄﾞ解除）	*/
				moni_no  = ( lk_kind == LK_KIND_INT_FLAP ? 
							OPMON_FLAP_MMODE_OFF:OPMON_LOCK_MMODE_OFF);			/* ﾓﾆﾀｰ情報No.（ﾛｯｸ装置手動ﾓｰﾄﾞ発生）	*/
			}
		}
		if( (tno_wk >= 1) && (tno_wk <= tno_max) ){								/* ﾀｰﾐﾅﾙNo.ﾁｪｯｸ							*/

			if( lk_kind == LK_KIND_FLAP ){										/* 装置種別？							*/
				// ﾌﾗｯﾌﾟ装置
				flp_m_mode[tno_wk-1] = set_mode;								/* ﾌﾗｯﾌﾟ装置手動ﾓｰﾄﾞﾌﾗｸﾞ更新			*/
			}
			else{
				// ﾛｯｸ装置
				lok_m_mode[tno_wk-1] = set_mode;								/* ﾛｯｸ装置手動ﾓｰﾄﾞﾌﾗｸﾞ更新				*/
			}
			wmonlg( moni_no, NULL, (ulong)tno_wk );								/* ﾓﾆﾀ登録：手動ﾓｰﾄﾞ発生／解除			*/
			queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );						/* OpeMainへ通知（営業／休業切替ﾁｪｯｸ）	*/
		}
		return( ret );
	}

/*----------------------------------------------------------------------*/
/* 接続なしは「車両検知状態」と「ﾛｯｸ装置状態」の両方に存在する			*/
/* どちらか一方が接続なしの場合には「接続なし」と判断する。				*/
/* 接続なしの場合には接続なしbitをONにし、その他の情報は全てｸﾘｱする		*/
/* この時､閉ﾛｯｸ、開ﾛｯｸであれば閉ﾛｯｸ、開ﾛｯｸを解除する。					*/
/* 入庫状態の場合には出庫扱い（強制出庫）とする。						*/
/* ﾛｯｸ装置閉状態であれば開状態（ﾃﾞﾌｫﾙﾄ）とする。						*/
/*----------------------------------------------------------------------*/
																				/*								*/
	/*----------------*/
	/* 接続有無のﾁｪｯｸ */
	/*----------------*/
	if( (LockBuf[cnt].car_cnd == 0 )||											/* 車両接続状態＝[接続なし]		*/
		(LockBuf[cnt].lock_cnd == 6 )) 											/* ﾛｯｸ装置状態 ＝[接続なし]		*/
	{																			/*								*/
		if( lk_tbl->ostat.bits.b08 == 0 )										/* 旧：接続あり(あり→なし)		*/
		{																		/*								*/
			lk_tbl->nstat.word |= 0x0100;										/* 現：接続なし					*/
			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
			/*--------------------------------------*/
			/* ｴﾗｰ印字：接続なし発生(親機№､装置№) */
			/*--------------------------------------*/
		}																		/*								*/
		else																	/* 旧：接続なし(なし→なし)		*/
		{																		/*								*/
			lk_tbl->nstat.word = nst_bak;										/* 状態の変更しない				*/
			lk_tbl->ostat.word = ost_bak;										/* 状態の変更しない				*/
		}																		/*								*/
		return( ret );															/* 変化ありの時は状態ﾁｪｯｸしない	*/
	}																			/* 接続なしなので以後のﾁｪｯｸなし	*/
	else																		/* [接続あり]					*/
	{																			/*								*/
		if( lk_tbl->ostat.bits.b08 == 1 )										/* 旧：接続なし(なし→あり)		*/
		{																		/*								*/
			lk_tbl->nstat.bits.b08 = 0;											/* 現：接続あり					*/
			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
			/*--------------------------------------*/
			/* ｴﾗｰ印字：接続なし解除(親機№､装置№) */
			/*--------------------------------------*/
		}																		/*								*/
	}																			/*								*/
																				/*								*/
																				/*								*/
	/*--------------*/
	/* 車両検知状態 */
	/*--------------*/
	switch( LockBuf[cnt].car_cnd )												/* 車両検知状態のﾁｪｯｸ			*/
	{																			/*								*/
	case 1:																		/* [車両あり]					*/
		if( lk_tbl->ostat.bits.b00 == 0 ){										/* 旧：車両なし					*/
			/* 車両なし→車両あり */											/*								*/
			lk_tbl->nstat.bits.b00 = 1;											/* 現：車両あり					*/
			if( lk_tbl->nstat.bits.b03 == 1 ){									/*								*/
				lk_tbl->nstat.bits.b03 = 0;										/* 現：強制出庫解除				*/
			}																	/*								*/
			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
		}else{																	/* 旧：車両あり					*/
			/* 車両あり→車両あり */											/*								*/
			if( lk_tbl->ostat.bits.b03 == 1 ){									/* 旧：強制出庫中				*/
				lk_tbl->nstat.bits.b03 = 0;										/* 現：強制出庫解除				*/
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	case 2:																		/* [車両なし]					*/
		if( lk_tbl->ostat.bits.b00 == 1 ){										/* 旧：車両あり					*/
			/* 車両あり→車両なし */											/*								*/
			lk_tbl->nstat.bits.b00 = 0;											/* 現:車両なし					*/
			if( lk_tbl->nstat.bits.b03 == 1 ){									/*								*/
				lk_tbl->nstat.bits.b03 = 0;										/* 現：強制出庫解除				*/
			}																	/*								*/

			// 車両あり → なしのとき、状態異常ﾌﾗｸﾞｸﾘｱ
			lk_tbl->nstat.bits.b12 = 0;		// 上昇異常回復
			lk_tbl->nstat.bits.b13 = 0;		// 下降異常回復

			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
		}else{																	/* 旧：車両なし					*/
			/* 車両なし→車両なし */											/*								*/
			if( lk_tbl->ostat.bits.b03 == 1 ){									/* 旧：強制出庫中				*/
				lk_tbl->nstat.bits.b03 = 0;										/* 現：強制出庫解除				*/
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	case 3:																		/* 手動ﾓｰﾄﾞで強制出庫			*/
		if( lk_tbl->ostat.bits.b03 == 0 ){										/* 旧:強制出庫中でない			*/
			lk_tbl->nstat.bits.b03 = 1;											/* 現:強制出庫					*/
			if( lk_tbl->ostat.bits.b00 == 1 ){									/*								*/
				lk_tbl->nstat.bits.b00 = 0;										/* 現:車両なし					*/

				// 車両あり → なしのとき、状態異常ﾌﾗｸﾞｸﾘｱ
				lk_tbl->nstat.bits.b12 = 0;		// 上昇異常回復
				lk_tbl->nstat.bits.b13 = 0;		// 下降異常回復
			}																	/*								*/
			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
		}																		/*								*/
		break;																	/*								*/
	}																			/*								*/
	/*--------------*/
	/* ﾛｯｸ装置状態	*/
	/*--------------*/
	switch( LockBuf[cnt].lock_cnd )												/*								*/
	{																			/*								*/
	case 1:																		/* 上昇済み(ﾛｯｸ装置閉済み)		*/
		if( lk_tbl->ostat.bits.b01 == 0 )										/* 下降済み(ﾛｯｸ装置開済み)		*/
		{																		/*								*/
			/* 下降済み→上昇済み */											/*								*/
			lk_tbl->nstat.bits.b01 = 1;											/* 上昇済み(ﾛｯｸ装置閉済み)		*/
			if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
				wmonlg( OPMON_FLAPUP, NULL, lkNo_w );							/* ﾓﾆﾀ登録(ﾌﾗｯﾌﾟ閉)				*/
			} else {
				wmonlg( OPMON_LOCKUP, NULL, lkNo_w );							/* ﾓﾆﾀ登録(ﾛｯｸ閉)				*/
			}
			if( lk_tbl->ostat.bits.b04 == 1 )									/* 上昇ﾛｯｸ中(閉ﾛｯｸ中)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* 上昇ﾛｯｸ解除(閉ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで閉ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* 下降ﾛｯｸ中(開ﾛｯｸ中)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* 下降ﾛｯｸ解除(開ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで開ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
			}																	/*								*/
			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
			InOutLog = 1;														/* 入出庫ログ登録あり(上昇/閉)	*/
		}																		/*								*/
		else																	/*								*/
		{																		/*								*/
			/* 上昇済み→上昇済み */											/*								*/
			if( lk_tbl->ostat.bits.b04 == 1 )									/* 上昇ﾛｯｸ中(閉ﾛｯｸ中)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* 上昇ﾛｯｸ解除(閉ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで閉ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
				InOutLog = 1;													/* 入出庫ログ登録あり(上昇/閉)	*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* 下降ﾛｯｸ中(開ﾛｯｸ中)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* 下降ﾛｯｸ解除(開ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで開ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
				InOutLog = 1;													/* 入出庫ログ登録あり(上昇/閉)	*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b07 == 1 ){
				FmvTimer( (ushort)(LockBuf[cnt].LockNoBuf + 1), -1 );				/* ﾌﾗｯﾌﾟ動作監視ﾀｲﾏｰ停止		*/
			}
			if( !DownLockFlag[LockBuf[cnt].LockNoBuf] ){						/* 初回処理						*/
				// 車両なしなら、状態変化ありとする
				if (lk_tbl->nstat.bits.b00 == 0) {								/* 車両なし						*/
					ret = 2;													/* 状態変化あり					*/
					wk_hit = 1;													/*								*/
					break;														/*								*/
				}																/*								*/
			}																	/*								*/
			else if( DownLockFlag[LockBuf[cnt].LockNoBuf] == 1 ){				/* 初回処理済み					*/
				DownLockFlag[LockBuf[cnt].LockNoBuf]++;							/* 2回目以降の状態変化			*/
			}																	/*								*/
		}																		/*								*/
		lk_tbl->nstat.bits.b12 = 0;		// 上昇異常回復
		break;																	/*								*/
	case 2:																		/* 下降済み(ﾛｯｸ装置開済み)		*/
		if( lk_tbl->ostat.bits.b01 == 1 )										/* 上昇済み(ﾛｯｸ装置閉済み)		*/
		{																		/*								*/
			/* 上昇済み→下降済み */											/*								*/
			lk_tbl->nstat.bits.b01 = 0;											/* 下降済み(ﾛｯｸ装置開済み)		*/
// 不具合修正(S) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
			FurikaeDestFlapNo = 0;
// 不具合修正(E) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
			if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
				wmonlg( OPMON_FLAPDOWN, NULL, lkNo_w );							/* ﾓﾆﾀ登録(ﾌﾗｯﾌﾟ閉)				*/
			} else {
				wmonlg( OPMON_LOCKDOWN, NULL, lkNo_w );							/* ﾓﾆﾀ登録(ﾛｯｸ閉)				*/
			}
			if( lk_tbl->ostat.bits.b04 == 1 )									/* 上昇ﾛｯｸ中(閉ﾛｯｸ中)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* 上昇ﾛｯｸ解除(閉ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで閉ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* 下降ﾛｯｸ中(開ﾛｯｸ中)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* 下降ﾛｯｸ解除(開ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで開ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
			}																	/*								*/
			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
			InOutLog = 2;														/* 入出庫ログ登録あり(下降/開)	*/
		}																		/*								*/
		else																	/*								*/
		{																		/*								*/
			/* 下降済み→下降済み */											/*								*/
			if( lk_tbl->ostat.bits.b04 == 1 )									/* 上昇ﾛｯｸ中(閉ﾛｯｸ中)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* 上昇ﾛｯｸ解除(閉ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで閉ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
				InOutLog = 2;													/* 入出庫ログ登録あり(下降/開)	*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* 下降ﾛｯｸ中(開ﾛｯｸ中)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* 下降ﾛｯｸ解除(開ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで開ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
				InOutLog = 2;													/* 入出庫ログ登録あり(下降/開)	*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b07 == 1 ){
				FmvTimer( (ushort)(LockBuf[cnt].LockNoBuf + 1), -1 );				/* ﾌﾗｯﾌﾟ動作監視ﾀｲﾏｰ停止		*/
			}
		}																		/*								*/
		lk_tbl->nstat.bits.b13 = 0;		// 下降異常回復
		break;																	/*								*/
																				/*								*/
	case 3:																		/* 上昇ﾛｯｸ(閉動作異常)			*/
		if( lk_tbl->ostat.bits.b04 == 0 )										/* 上昇ﾛｯｸなし(閉ﾛｯｸなし)		*/
		{																		/*								*/
			lk_tbl->nstat.bits.b01 = 1;											/* 上昇済み(ﾛｯｸ装置閉済み)		*/
			lk_tbl->nstat.bits.b04 = 1;											/* 上昇ﾛｯｸ(閉ﾛｯｸ)				*/
			// 上昇ﾛｯｸ発生、かつ現在車両が存在するなら、上昇異常ﾌﾗｸﾞON
			if (lk_tbl->nstat.bits.b00 == 1) {
				lk_tbl->nstat.bits.b12 = 1;		// 上昇異常発生
			}
			if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
				wmonlg( OPMON_FLAPUP, NULL, lkNo_w );							/* ﾓﾆﾀ登録(ﾌﾗｯﾌﾟ閉)				*/
			} else {
				wmonlg( OPMON_LOCKUP, NULL, lkNo_w );							/* ﾓﾆﾀ登録(ﾛｯｸ閉)				*/
			}
			/*--------------------*/
			/* ｴﾗｰ発生：閉ﾛｯｸ発生 */
			/*--------------------*/
			lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 1 );			/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* 下降ﾛｯｸあり(開ﾛｯｸあり)		*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* 下降ﾛｯｸ解除(開ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで開ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
			}																	/*								*/
			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
			InOutLog = 3;														/* 入出庫ログ登録あり(上昇/閉ロック)	*/
		}																		/*								*/
		else																	/*								*/
		{																		/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* 下降ﾛｯｸあり(開ﾛｯｸあり)		*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* 下降ﾛｯｸ解除(開ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで開ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
			}																	/*								*/
			FmvTimer( (ushort)(LockBuf[cnt].LockNoBuf + 1), -1 );				/* ﾌﾗｯﾌﾟ動作監視ﾀｲﾏｰ停止		*/
		}																		/*								*/
		break;																	/*								*/
																				/*								*/
	case 4:																		/* 下降ﾛｯｸ(開動作異常)			*/
		if( lk_tbl->ostat.bits.b05 == 0 )										/* 下降ﾛｯｸなし(開ﾛｯｸなし)		*/
		{																		/*								*/
			if( !DownLockFlag[LockBuf[cnt].LockNoBuf] ){						/* 初回処理						*/
				// 車両なしで下降ﾛｯｸの場合は上昇済みとして扱う
				if (lk_tbl->nstat.bits.b00 == 0) {								/* 車両なし						*/
					lk_tbl->nstat.bits.b01 = 1;									/* 上昇済み(ﾛｯｸ装置閉済み)		*/
				}																/*								*/
			}																	/*								*/
			else if( DownLockFlag[LockBuf[cnt].LockNoBuf] == 1 ){				/* 初回処理済み					*/
				lk_tbl->nstat.bits.b01 = 0;										/* 下降済み(ﾛｯｸ装置開済み)		*/
// 不具合修正(S) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
				FurikaeDestFlapNo = 0;
// 不具合修正(E) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
				DownLockFlag[LockBuf[cnt].LockNoBuf]++;							/* 2回目以降の状態変化			*/
			}																	/*								*/
			lk_tbl->nstat.bits.b05 = 1;											/* 下降ﾛｯｸ(開ﾛｯｸ)				*/
			// 下降ﾛｯｸ発生、かつ現在車両が存在するなら、下降異常ﾌﾗｸﾞON
			if (lk_tbl->nstat.bits.b00 == 1) {
				lk_tbl->nstat.bits.b13 = 1;		// 下降異常発生
			}
			if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
				wmonlg( OPMON_FLAPDOWN, NULL, lkNo_w );							/* ﾓﾆﾀ登録(ﾌﾗｯﾌﾟ閉)				*/
			} else {
				wmonlg( OPMON_LOCKDOWN, NULL, lkNo_w );							/* ﾓﾆﾀ登録(ﾛｯｸ閉)				*/
			}
			/*--------------------*/
			/* ｴﾗｰ発生：開ﾛｯｸ発生 */
			/*--------------------*/
			lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 1 );			/*								*/
			if( lk_tbl->ostat.bits.b04 == 1 )									/* 上昇ﾛｯｸあり(閉ﾛｯｸあり)		*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* 上昇ﾛｯｸ解除(閉ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで閉ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
			}																	/*								*/
			ret = 2;															/* 状態変化あり					*/
			wk_hit = 1;															/*								*/
			InOutLog = 4;														/* 入出庫ログ登録あり(下降/開ロック)	*/
		}																		/*								*/
		else																	/*								*/
		{																		/*								*/
			if( lk_tbl->ostat.bits.b04 == 1 )									/* 上昇ﾛｯｸあり(閉ﾛｯｸあり)		*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* 上昇ﾛｯｸ解除(閉ﾛｯｸ解除)		*/
				/*--------------------------------------*/
				/* ｴﾗｰ解除：故障解除受信なしで閉ﾛｯｸ解除 */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
			}																	/*								*/
			FmvTimer( (ushort)(LockBuf[cnt].LockNoBuf + 1), -1 );				/* ﾌﾗｯﾌﾟ動作監視ﾀｲﾏｰ停止		*/
			if( !DownLockFlag[LockBuf[cnt].LockNoBuf] ){
				// 下降ロック発生時は、状態変化があったように振舞う
				ret = 2;														/* 状態変化あり					*/
				wk_hit = 1;														/*								*/
				DownLockFlag[LockBuf[cnt].LockNoBuf] = 1;						/* 初回処理済み					*/
			}
		}																		/*								*/
		break;																	/*								*/
	case 5:																		/* 故障解除						*/
		if( lk_tbl->ostat.bits.b04 == 1 ){										/* 上昇ﾛｯｸ(閉ﾛｯｸ)				*/
			lk_tbl->nstat.bits.b04 = 0;											/* 上昇ﾛｯｸ(閉ﾛｯｸ)解除			*/
			/*--------------------*/
			/* ｴﾗｰ解除：閉ﾛｯｸ解除 */
			/*--------------------*/
			lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );			/*								*/
		}																		/*								*/
		if( lk_tbl->ostat.bits.b05 == 1 ){										/* 下降ﾛｯｸ(開ﾛｯｸ)				*/
			lk_tbl->nstat.bits.b05 = 0;											/* 下降ﾛｯｸ(開ﾛｯｸ)解除			*/
			/*--------------------*/
			/* ｴﾗｰ解除：開ﾛｯｸ解除 */
			/*--------------------*/
			lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );			/*								*/
		}																		/*								*/
		if( lk_tbl->ostat.bits.b02 == 1 )										/* 上昇動作(閉動作)				*/
		{																		/*								*/
			lk_tbl->nstat.bits.b01 = 1;											/* 上昇済み(ﾛｯｸ装置閉済み)		*/
			if( lk_tbl->ostat.bits.b05 == 1 ){									/* 下降ﾛｯｸ(開ﾛｯｸ)				*/
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					wmonlg( OPMON_FLAPUP, NULL, lkNo_w );						/* ﾓﾆﾀ登録(ﾌﾗｯﾌﾟ閉)				*/
				} else {
					wmonlg( OPMON_LOCKUP, NULL, lkNo_w );						/* ﾓﾆﾀ登録(ﾛｯｸ閉)				*/
				}
			}
			InOutLog = 1;														/* 入出庫ログ登録あり(上昇/閉)	*/
		}																		/*								*/
		else																	/* 下降動作(開動作)				*/
		{																		/*								*/
			lk_tbl->nstat.bits.b01 = 0;											/* 下降済み(ﾛｯｸ装置開済み)		*/
// 不具合修正(S) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
			FurikaeDestFlapNo = 0;
// 不具合修正(E) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
			if( lk_tbl->ostat.bits.b04 == 1 ){									/* 上昇ﾛｯｸ(閉ﾛｯｸ)				*/
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					wmonlg( OPMON_FLAPDOWN, NULL, lkNo_w );						/* ﾓﾆﾀ登録(ﾌﾗｯﾌﾟ閉)				*/
				} else {
					wmonlg( OPMON_LOCKDOWN, NULL, lkNo_w );						/* ﾓﾆﾀ登録(ﾛｯｸ閉)				*/
				}
			}
			InOutLog = 2;														/* 入出庫ログ登録あり(上昇/閉)	*/
		}																		/*								*/
		ret = 2;																/* 状態変化あり					*/
		wk_hit = 1;																/*								*/
		break;																	/*								*/
	}																			/*								*/
	if( !DownLockFlag[LockBuf[cnt].LockNoBuf] ){								/* 起動時の初回状態データ未受信 */
		// 下降ロック状態処理は起動時に下降ロックでなければ、以降下降ロックとなっても処理しない（今まで通りとする）
		DownLockFlag[LockBuf[cnt].LockNoBuf] = 1;								/* 初回処理済み					*/
	}
	if( wk_hit != 1 ){															/* 状態の変化なし				*/
		LockBuf[cnt].LockNoBuf = 0xfffe;										/* 状態変化なしｾｯﾄ				*/
		if( wk_hit != 2 ){														/*								*/
			lk_tbl->nstat.word = nst_bak;										/* 現ｽﾃｰﾀｽを元に戻す			*/
			lk_tbl->ostat.word = ost_bak;										/* 旧ｽﾃｰﾀｽを元に戻す			*/
		}																		/*								*/
	}																			/*								*/

	// (InOutLog != 0) の場合は (wk_hit == 1) となる。
	// このため (InOutLog != 0) の場合は (LockBuf[cnt].LockNoBuf = 0xfffe) とはならないため、
	// 以下処理は問題なし。
	if( InOutLog ){																/* 入出庫ログ登録あり			*/
		lkNo_w = LockInfo[LockBuf[cnt].LockNoBuf].posi;
		switch( InOutLog ){
			case	1:
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					/* フラップ上昇完了情報登録 */
					IoLog_write(IOLOG_EVNT_FLAP_UP_FIN, (ushort)lkNo_w, 0, 0);
				} else {
					/* 駐輪ロック閉完了情報登録 */
					IoLog_write(IOLOG_EVNT_LOCK_CL_FIN, (ushort)lkNo_w, 0, 0);
				}
				break;
			case	2:
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					/* フラップ下降完了情報登録 */
					IoLog_write(IOLOG_EVNT_FLAP_DW_FIN, (ushort)lkNo_w, 0, 0);
				} else {
					/* 駐輪ロック開完了情報登録 */
					IoLog_write(IOLOG_EVNT_LOCK_OP_FIN, (ushort)lkNo_w, 0, 0);
				}
				break;
			case	3:
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					/* フラップ上昇未完情報登録 */
					IoLog_write(IOLOG_EVNT_FLAP_UP_UFN, (ushort)lkNo_w, 0, 0);
				} else {
					/* ロック閉動作未完情報登録 */
					IoLog_write(IOLOG_EVNT_LOCK_CL_UFN, (ushort)lkNo_w, 0, 0);
				}
				break;
			case	4:
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					/* フラップ下降未完情報登録 */
					IoLog_write(IOLOG_EVNT_FLAP_DW_UFN, (ushort)lkNo_w, 0, 0);
				} else {
					/* ロック開動作未完情報登録 */
					IoLog_write(IOLOG_EVNT_LOCK_OP_UFN, (ushort)lkNo_w, 0, 0);
				}
				break;
		}
	}
	return( ret );																/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF盤設定情報ﾃｰﾌﾞﾙ作成(API)																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKInti_SetLockMaker()																		   |*/
/*| PARAMETER	: NON	:																					   |*/
/*| RETURN VALUE: ret	: 																					   |*/
/*| 			: 		: 																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・LockInfoﾃｰﾌﾞﾙから親機別に子機毎のﾛｯｸ装置種別を取得し所定のﾊﾞｯﾌｧへ格納する。							   |*/
/*|	・電源ON時、設定変更時に必ずｺｰﾙすること																	   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	LKcom_SetLockMaker( void )
{
ushort	i;
	memset( &child_mk, 0x00, sizeof( child_mk ) );
	for( i=CRR_CTRL_START; i<LOCK_MAX; i++ )
	{
		WACDOG;																	// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( LockInfo[i].if_oya != 0 )
		{
			if( (LockInfo[i].lok_syu != 0)&&									// ロック装置種別
				(LockInfo[i].lok_no  != 0) ){									// 接続ターミナルNoに対するロック装置連番
				child_mk[LockInfo[i].if_oya-1] = LockInfo[i].lok_syu;			// ロック装置種別をセット
			}
		}
	}
}
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF盤制御(API)																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKopeApiLKCtrl()																		   |*/
/*| PARAMETER	: NON	:																					   |*/
/*| RETURN VALUE: ret	: 																					   |*/
/*| 			: 		: 																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		: A.iiizumi 2011/11/10																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・																										   |*/
/*|	・																										   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	LKopeApiLKCtrl( ushort tb_no, uchar kind )								/* ﾛｯｸ装置指示					*/
{																				/*								*/
	ushort	lk_no	= 0;														/* ﾛｯｸ装置番号					*/
	uchar	wk_tno	= 0;														/*								*/
#if (1 == AUTO_PAYMENT_PROGRAM)
	// ﾀﾞﾐｰへ送信(の代わりに通知)する
	if (OPECTL.Seisan_Chk_mod == ON) {
		if (tb_no == 0) return;
		// ﾌﾗｯﾌﾟ送信
		if (tb_no <= BIKE_START_INDEX) {
			lk_no  = (ushort)(tb_no-1);
			wk_tno = LockInfo[lk_no].if_oya;
			LK_AutoPayment_Send(MAX_AUTO_PAYMENT_CAR, wk_tno, (uchar)1, kind);	// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
		}

		// ﾛｯｸ送信
		if ((tb_no > INT_CAR_START_INDEX) && (tb_no <= LOCK_MAX)) {
			lk_no  = (ushort)(tb_no-1);
			wk_tno = LockInfo[lk_no].if_oya;
			// ﾌﾗｯﾌﾟ用電文作成関数を流用しているため、装置Noは車室ﾊﾟﾗﾒｰﾀから取得する(lok_no)
			LK_AutoPayment_Send(MAX_AUTO_PAYMENT_CAR+1, wk_tno, (uchar)LockInfo[lk_no].lok_no, kind);
		}
		return;
	}
#endif

	if( kind == (uchar)(LK_SND_P_CHK&0x000f) ){
		// ポート検査の場合は、車室番号ではなく、ﾀｰﾐﾅﾙ番号が渡されてくる。
		LKcom_SetDtPkt( LK_SND_CTRL_ID, (uchar)tb_no + IFS_CRR_OFFSET -1, 0, kind );	// CRR基板へアクセスする場合、ターミナルNo.＋IFS_CRR_OFFSETすることで、ターミナルNo.と差別化する
		return;
	}
	if (tb_no == 0) return;

	// ﾛｯｸ送信
	if ((tb_no > CRR_CTRL_START) && (tb_no <= LOCK_MAX)) {
		lk_no  = (ushort)(tb_no-1);
		wk_tno = LockInfo[lk_no].if_oya;
		LKcom_SetDtPkt( LK_SND_CTRL_ID, wk_tno, tb_no, kind );
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: LKopeApiLKCtrl_All
 *[]----------------------------------------------------------------------[]
 *| summary	: 全ﾌﾗｯﾌﾟ/全ﾛｯｸ装置に同時に制御ｺｰﾄﾞを送信する
 *| param	: 
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
void	LKopeApiLKCtrl_All(uchar mtype, uchar kind)
{
	uchar	t_no_max, i;
	t_no_max = (uchar)LKcom_GetAccessTarminalCount();				// フラップ制御盤、ロック装置用IF盤子機接続台数

	if ((mtype == _MTYPE_LOCK) ) {
		for (i = 1; i <= t_no_max; i++) {
			if(0 == LKcom_Search_Ifno( i )){
			// NOTE:ここではCRAのI/F形式であり、ターミナルNoはCRB単位となるので一つでも
			// CRB接続の端末が見つかったら電文をセットして抜ける
				LKcom_SetDtPkt(LK_SND_CTRL_ID, i, 0, kind);
				break;
			}
		}
	}
	if ((mtype == _MTYPE_INT_FLAP) ) {
		if( kind != 12 ){	// バージョン情報以外の場合
			for (i = 1; i <= t_no_max; i++) {
				if(1 == LKcom_Search_Ifno( i )){
					LKcom_SetDtPkt(LK_SND_CTRL_ID, i, 0, kind);
					break;
				}
			}
		} else {			// フラップのバージョン情報はCRR基板へのアクセスにする
			for (i=0; i < IFS_CRR_MAX; i++) {
				if ( 1 == IFM_LockTable.sSlave_CRR[i].bComm ){
					LKcom_SetDtPkt(LK_SND_CTRL_ID, i+IFS_CRR_OFFSET, 0, kind);	// CRR基板へアクセスする場合、ターミナルNo.＋IFS_CRR_OFFSETすることで、ターミナルNo.と差別化する
					break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF盤情報テーブル（LockInfo）検索(API)																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKopeGetLockNum()																			   |*/
/*| PARAMETER	: par1	: 区画(1～26)																		   |*/
/*| 			: par2	: 駐車位置番号(1～9999)																   |*/
/*| 			: par3	: LockInfo配列番号+1																   |*/
/*| RETURN VALUE: ret	: 1=該当車室あり 0=該当車室なし														   |*/
/*| 			: 		: 																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-05-18																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・																										   |*/
/*|	・																										   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
short	LKopeGetLockNum( uchar snum, ushort pnum, ushort *posi )				/*								*/
{																				/*								*/
	ushort	i;																	/*								*/
	uchar	work;																/*								*/
																				/*								*/
	work = GetCarInfoParam();													/*								*/
	for( i = 0; i < LOCK_MAX; i++ ){											/* 324車室検索					*/
		WACDOG;																// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( i < INT_CAR_START_INDEX ) {											/*								*/
			if( !( work & 0x02 ) ) {											/*								*/
				continue;														/*								*/
			}																	/*								*/
		} else if( i < BIKE_START_INDEX ) {										/*								*/
			if( !( work & 0x04 ) ) {											/*								*/
				continue;														/*								*/
			}																	/*								*/
		} else {																/*								*/
			if( !( work & 0x01 ) ) {											/*								*/
				continue;														/*								*/
			}																	/*								*/
		}																		/*								*/
		if(( LockInfo[i].area == snum )&&										/* 区画と駐車位置が一致			*/
		   ( LockInfo[i].posi == pnum )){										/*								*/
			*posi = i+1;														/* LockInfo配列番号+1(1～324)	*/
			return( 1 );														/* 該当車室あり					*/
		}																		/*								*/
	}																			/*								*/
	return( 0 );																/* 該当車室なし					*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	エラーデータ受信時のエラー登録																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKopeErrRegist()																			   |*/
/*| PARAMETER	: tno	: 親機端末№																		   |*/
/*| 			: lk_ecod: t_LKcomEcodのポインタ															   |*/
/*| RETURN VALUE: ret	なし																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																					   |*/
/*| Date		: 2005-08-08																				   |*/
/*| Update		:																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	LKopeErrRegist( uchar tno, t_LKcomEcod *lk_ecod )
{
	ushort	i;
	char occ;
	i = 0;
	if( IFS_CRR_OFFSET > lk_ecod->tno ){	// CRR基板へのアクセスではない
		if( lk_ecod->tno > LOCK_IF_MAX ){ // 端末の最大数でガード
			 return;
		}
	} else {								// CRR基板へのアクセスの場合、100～102だけが有効
		if( lk_ecod->tno < IFS_CRR_OFFSET || lk_ecod->tno > IFS_CRR_OFFSET + 2 )
			 return;
	}
		
	switch( lk_ecod->err ){
		case	1:// パリティエラー
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_PARITYERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	2:// フレーミングエラー
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_FLAMERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	3:// オーバーランエラー
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_OVERRUNERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	4:// 電文異常（ヘッダ文字）
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_HEADERERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	5:// 電文異常（データ長NG）
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LENGTHERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	6:// 電文異常（データIDNG）
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_IDERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	7:// NAKリトライオーバー
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_NAKRYOVER, (char)lk_ecod->occr, 0, 0 );
			break;
		case	8:// 送信リトライオーバー（通信異常）
			if( lk_ecod->tno > 0 && lk_ecod->tno < IFS_CRR_OFFSET){	// CRR基板のリトライオーバーは不要なので一応制限しておく
				// 子機のｴﾗｰ
				if(IFM_LockTable.sSlave[tno - 1].bSomeone != 1) {	// 端末接続なしか
					break;
				}
				occ = (char)lk_ecod->occr;
				if( occ > 2 ){
					 occ = 0;
				}
				ErrBinDatSet( (ulong)lk_ecod->tno - 1, 0 );
				err_chk( ERRMDL_FLAP_CRB, (char)(ERR_FLAPLOCK_COMFAIL+tno-1), (char)occ, 2, 0 );
			}
			break;
		case	9:// CRC異常
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_CRC_ERR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	10:// 電文異常（ヘッダ部未受信）
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_NOHEADER, (char)lk_ecod->occr, 0, 0 );
			break;
		case	11:// プロトコル異常
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_PLOTOCOLERR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	14:// 相手違い
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_RESERR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	21:// キューフル（子機状態リスト）
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_QUEFULL1, (char)lk_ecod->occr, 0, 0 );
			break;
		case	22:// キューフル（ロック動作待ちリスト）
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_QUEFULL2, (char)lk_ecod->occr, 0, 0 );
			break;
		case	23:// キューフル（子機セレクティング待ちリスト）
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_QUEFULL3, (char)lk_ecod->occr, 0, 0 );
			break;
		case	31:// 制御データ異常
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_DATAERR, (char)lk_ecod->occr, 0, 0 );
			break;
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ロック装置に開／閉ロックが発生しているかチェック														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKopeLockErrCheck()																		   |*/
/*| PARAMETER	: OpenCloose	: 0=閉ロック、1=開ロック													   |*/
/*| RETURN VALUE: ret	=0:ｴﾗｰなし、<>0:発生している数														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																					   |*/
/*| Date		: 2005-08-08																				   |*/
/*| Update		:																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
short	LKopeLockErrCheck( short OpenClose, uchar mtype )
{
	int	i, errnum;

	int	start, end;

	if (mtype == _MTYPE_LOCK) {
		start = FLAP_NUM_MAX;		//ｻｰﾁ開始車室番号(ﾛｯｸ装置は100から)
		end = LOCK_MAX;				//ｻｰﾁ終了車室番号(ﾛｯｸ装置は323まで)
	} else if (mtype == _MTYPE_FLAP) {
		start = CAR_START_INDEX;	//ｻｰﾁ開始車室番号(ﾌﾗｯﾌﾟ装置は0から)
		end = CRR_CTRL_START;		//ｻｰﾁ終了車室番号(IFﾌﾗｯﾌﾟ装置は49まで)
	} else if (mtype == _MTYPE_INT_FLAP) {
		start = INT_CAR_START_INDEX;	//ｻｰﾁ開始車室番号(ﾌﾗｯﾌﾟ装置は0から)
		end = FLAP_NUM_MAX;			//ｻｰﾁ終了車室番号(内蔵ﾌﾗｯﾌﾟ装置は99まで)
	} else if (mtype == _MTYPE_ALL) {
		start = CAR_START_INDEX;
		end = LOCK_MAX;
	} else {
		return 0;		// ここには来ない
	}

	errnum = 0;

	if( OpenClose == 0 ){
		for( i = start; i < end; i++ ){
			WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if( LockInfo[i].lok_syu ){
				if( FLAPDT.flp_data[i].nstat.bits.b04 ){
					errnum++;
				}
			}
		}
	}else{
		for( i = start; i < end; i++ ){
			WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if( LockInfo[i].lok_syu ){
				if( FLAPDT.flp_data[i].nstat.bits.b05 ){
					errnum++;
				}
			}
		}
	}
	return( (short)errnum );
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	エラー情報のセット																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ErrBinDatSet()																			   |*/
/*| PARAMETER	: dat	: ｴﾗｰ情報（ﾊﾞｲﾅﾘﾃﾞｰﾀ）																   |*/
/*|               knd	: 0:通信異常 1:開閉ﾛｯｸ																   |*/
/*| RETURN VALUE: 																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																					   |*/
/*| Date		: 2005-08-08																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	ErrBinDatSet( ulong dat, short knd )
{
	ulong	ul;

	if( knd == 0 ){
		ul = dat;
	}else{
		// 開閉ﾛｯｸのときは車室№をｾｯﾄ
		ul = (ulong)LockInfo[dat].area * 10000;
		ul += (ulong)LockInfo[dat].posi;
	}
	Err_work.ErrBinDat = ul;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ロック異常エラー判定																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: lk_err_chk()																				   |*/
/*| PARAMETER	: lkno	: ﾛｯｸ装置番号(0～323)																   |*/
/*|               no	: ｴﾗｰ番号																			   |*/
/*|               knd	: 0:解除 1:発生																		   |*/
/*| RETURN VALUE: 																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																					   |*/
/*| Date		: 2005-09-21																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	lk_err_chk( ushort lkno, char no, char kind )
{
	char	mtype;		// ｴﾗｰ番号(no)は同一の値のため、そのまま使用する

	if (lkno < FLAP_NUM_MAX) {
		// ﾌﾗｯﾌﾟ装置
		if( lkno < INT_CAR_START_INDEX )
			mtype = ERRMDL_IFFLAP;
		else
			mtype = ERRMDL_FLAP_CRB;
	} else {
		// ﾛｯｸ装置
		mtype = ERRMDL_FLAP_CRB;
	}

	if( CPrmSS[S_TYP][70] ){

		// ﾀｲﾏｰ時間後(ﾀｲﾏｰ時間ｴﾗｰを維持すると)ｴﾗｰ登録
		if( kind == 0 ){	// 解除
			ErrBinDatSet( (ulong)lkno, 1 );
			err_chk( mtype, no, kind, 2, 1 );
			if( no == ERR_LOCKCLOSEFAIL ){
				// ﾛｯｸ装置閉異常
				FLPCTL.Flp_uperr_tm[lkno] = -1;	// ﾀｲﾏｰｽﾄｯﾌﾟ
			}else{
				// ﾛｯｸ装置開異常
				FLPCTL.Flp_dwerr_tm[lkno] = -1;	// ﾀｲﾏｰｽﾄｯﾌﾟ
			}
		}else{				// 発生
			if( no == ERR_LOCKCLOSEFAIL ){
				// ﾛｯｸ装置閉異常
				FLPCTL.Flp_uperr_tm[lkno] = (short)(((( CPrmSS[S_TYP][70] / 100 ) * 60 ) + ( CPrmSS[S_TYP][70] % 100 )) / 5 );
			}else{
				// ﾛｯｸ装置開異常
				FLPCTL.Flp_dwerr_tm[lkno] = (short)(((( CPrmSS[S_TYP][70] / 100 ) * 60 ) + ( CPrmSS[S_TYP][70] % 100 )) / 5 );
			}
		}
	}else{
		// 即時ｴﾗｰ登録
		ErrBinDatSet( (ulong)lkno, 1 );
		err_chk( mtype, no, kind, 2, 1 );
		FLPCTL.Flp_uperr_tm[lkno] = -1;		// ﾀｲﾏｰｽﾄｯﾌﾟ
		FLPCTL.Flp_dwerr_tm[lkno] = -1;		// ﾀｲﾏｰｽﾄｯﾌﾟ
	}
	/* フラップ下降ロック発生時保護処理 */
	if (lkno < FLAP_NUM_MAX && no == ERR_FLAPLOCK_LOCKOPENFAIL) {
		flp_DownLock_lk_err_chk( lkno, no, kind );
	}
}


// ﾛｯｸ方式分離
/*[]----------------------------------------------------------------------[]
 *|	name	: get_lktype
 *[]----------------------------------------------------------------------[]
 *| summary	: ロック方式取得
 *| param	: lok_syu:ロック装置種別
 *| return	: 該当ロック方式
 *[]----------------------------------------------------------------------[]*/
uchar get_lktype(uchar lok_syu)
{
	uchar type;
	
	if (lok_syu <= 6) {
		type = (uchar)prm_get( COM_PRM,S_TYP,60,1,(char)((6-lok_syu)+1) );
		if (type == 0) {
			type = (uchar)((lok_syu == 6) ? 3 : lok_syu);
		}
	}
	else {
		lok_syu -= 6;
		type = (uchar)prm_get( COM_PRM,S_TYP,61,1,(char)((6-lok_syu)+1) );
	}

	return type;
}

// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ/ﾛｯｸからの受信ﾀﾞﾐｰ関数                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LK_AutoPayment_Rcv                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-11-13                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void LK_AutoPayment_Rcv(ushort index, uchar tno, uchar lkno, uchar kind)
{
	uchar  data[64];
	ushort size;
	uchar  carcnd, lkcnd;
	t_LKcomLock*		lck_tmp;
	t_LKcomSubLock*		lck_sub;
	t_IFM_mail	*pQbody;
	int	i;
	int imax;

	// ﾃﾞｰﾀ種別
	switch (kind) {
	case AUTOPAY_STS_IN:		// 車両検知(入庫)
		carcnd = 1;
		lkcnd = 2;
		break;
	case AUTOPAY_STS_OUT:		// 車両検知OFF
		carcnd = 2;
		lkcnd = 2;
		break;
	case AUTOPAY_STS_UP:		// ﾌﾗｯﾌﾟ/ﾛｯｸON 車両在り
		carcnd = 1;
		lkcnd = 1;
		break;
	case AUTOPAY_STS_DOWN:		// ﾌﾗｯﾌﾟ/ﾛｯｸOFF 車両在り
		carcnd = 1;
		lkcnd = 2;
		break;
	default:
		return;
	}

	memset(data, 0, sizeof(data));
	pQbody = (t_IFM_mail *)&data[0];						// このバッファにはID4からセットする。
	// キュー実体へ「通常状態データ」をセット
	pQbody->s61.sCommon.ucKind           = LK_RCV_COND_ID;	// 通常状態データ
	pQbody->s61.sCommon.bHold            = 1;				// データ保持フラグ
	pQbody->s61.sCommon.t_no             = tno;				// 端末No(CRBの)
	pQbody->s61.ucCount                  = 1;				// 状態データ数。メンテナンス通知のためには「１」で十分 
	if(LKcom_Search_Ifno(tno) == 0) {
		i = FLAP_NUM_MAX;									// ロック装置インデックス
		imax = LOCK_MAX;									// ロック装置最大インデックス
	}
	else {
		i = CRR_CTRL_START;									// フラップ装置インデックス
		imax = TOTAL_CAR_LOCK_MAX;							// フラップ装置最大インデックス
	}

	for( ; i<imax; ++i ){
		WACDOG;												// 装置ループの際はWDGリセット実行
		if(LKcom_Search_Ifno(tno) == 0) {									// ロックの場合
			if( (LockInfo[i].if_oya == tno)&&(LockInfo[i].lok_no == lkno)){// ターミナルNoとロック装置番号一致
				// LockInfoはLOCK_MAXの値がMAXであるが、255車室未満よりそのままセット
				pQbody->s61.sLock[0].ucLock = (uchar)i;// LockInfoのインデックスをセット
				break;
			}
		} else {															// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
			if( (LockInfo[i].if_oya == tno)&&(1 == lkno)){// ターミナルNoとロック装置番号一致
				// LockInfoはLOCK_MAXの値がMAXであるが、255車室未満よりそのままセット
				pQbody->s61.sLock[0].ucLock = (uchar)i;// LockInfoのインデックスをセット
				break;
			}
		}
	}
	pQbody->s61.sLock[0].ucCarState  = carcnd;				// 車両検知状態		=0固定（メンテナンス開始/終了を知らせるのが目的）
	pQbody->s61.sLock[0].ucLockState = lkcnd;				// ロック装置状態	(07H)メンテナンス中/(08H)メンテナンス解除の何れか 
	LKcom_RcvDataSave(&data[0],7);							// アプリ側が受信するバッファにセット
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ/ﾛｯｸへの要求送信応答ﾀﾞﾐｰ関数                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LK_AutoPayment_Send                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-11-13                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void LK_AutoPayment_Send(uchar index, uchar tno, uchar lkno, uchar kind)
{
	switch (kind) {
	case 1:		// ﾌﾗｯﾌﾟ板上昇
		// 上昇完了の応答を返す
		LK_AutoPayment_Rcv((ushort)index, tno, lkno, AUTOPAY_STS_UP);
		break;
	case 2:		// ﾌﾗｯﾌﾟ板下降
		// 下降完了の応答を返す
		LK_AutoPayment_Rcv((ushort)index, tno, lkno, AUTOPAY_STS_DOWN);
		// 車両無しの応答を返す
		LK_AutoPayment_Rcv((ushort)index, tno, lkno, AUTOPAY_STS_OUT);
		break;
	default :
		break;
	}
}

#endif

/*[]----------------------------------------------------------------------[]*/
/*| ターミナル番号からCRA・CRRからの電文かを判断する                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_Search_Ifno	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka	                                           |*/
/*| Date         : 2009-06-05                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
char	LKcom_Search_Ifno( uchar if_no )
{
	char	ret=0;
	if(LKCOM_TYPE_FLAP == LKcom_GetAccessTarminalType(if_no)) {
		ret = 1;
	}
		
	return ret;

}

/*[]----------------------------------------------------------------------[]*/
/*| 全フラップ・ロック装置の状態を要求する			                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_AllInfoReq	                	                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka	                            	               |*/
/*| Date         : 2009-10-04                                              |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	LKcom_AllInfoReq( uchar kind )
{
	uchar type = GetCarInfoParam();
	
	switch( kind ){
		case _MTYPE_LOCK:
			if( !(type & 0x01) ){
				return;
			}
			break;
		case _MTYPE_INT_FLAP:
			if( !(type & 0x04) ){
				return;
			}
			break;
		case _MTYPE_FLAP:
			if( !(type & 0x02) ){
				return;
			}
			break;
		default:
			return;
	}
	queset( FLPTCBNO, LK_SND_A_STS, 1, &kind );		// 全状態要求制御送信要求
}
/*[]----------------------------------------------------------------------[]*/
/*| 車室番号から装置の種別を取得する				                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_RoomNoToType	               	                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2009-10-04                                              |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	LKcom_RoomNoToType( ulong roomNo )
{
	ushort i;
	
	for( i=INT_CAR_START_INDEX; i < LOCK_MAX; ++i ){
		WACDOG;										// 装置ループの際はWDGリセット実行
		if(LockInfo[i].posi == roomNo) {
			if(i < BIKE_START_INDEX) {
				return 1;
			}
			else {
				return 0;
			}
		}
	}
	return (ushort)-1;
}
/*[]----------------------------------------------------------------------[]*/
/*| 共通設定から接続する端末数を取得する	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_GetAccessTarminalCount                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : なし                                                          |*/
/*|	return : 端末数                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2012-06-01                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	LKcom_GetAccessTarminalCount(void)
{
	long	terminalCount;								// 端末数
	
	terminalCount = prm_get(COM_PRM, S_TYP, 41, 2, 1);
	if(terminalCount == 0) {							// 直接制御(FT-4000)か
		terminalCount = prm_get(COM_PRM, S_TYP, 42, 2, 5);
	}
	
	return (ushort)terminalCount;
}
/*[]----------------------------------------------------------------------[]*/
/*| 指定した端末No.の装置種別を取得する	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_GetAccessTarminalType                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : tno	: 端末No.(1～）                                        |*/
/*|	return : 装置種別                                                      |*/
/*|	           LKCOM_TYPE_NONE	: 装置なし                                 |*/
/*|	           LKCOM_TYPE_FLAP	: 装置はフラップ                           |*/
/*|	           LKCOM_TYPE_LOCK	: 装置はロック                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2012-06-01                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	LKcom_InitAccessTarminalType(void)
{
	ushort	terminalMax;								// 端末数
	ushort	terminal;									// 端末カウンタ
	
	memset(LKcom_Type, 0, sizeof(LKcom_Type));
	
	terminalMax = LKcom_GetAccessTarminalCount();
	if(terminalMax > LOCK_IF_MAX){
		terminalMax = LOCK_IF_MAX;// LKcom_Typeのオーバーライト抑止
	}
	for(terminal = 0; terminal < terminalMax; ++terminal) {
		LKcom_Type[terminal] = LKcom_GetAccessTarminalType(terminal + 1);
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 指定した端末No.の装置種別を取得する	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_GetAccessTarminalType                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : tno	: 端末No.(1～）                                        |*/
/*|	return : 装置種別                                                      |*/
/*|	           LKCOM_TYPE_NONE	: 装置なし                                 |*/
/*|	           LKCOM_TYPE_FLAP	: 装置はフラップ                           |*/
/*|	           LKCOM_TYPE_LOCK	: 装置はロック                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2012-06-01                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	LKcom_GetAccessTarminalType(ushort tno)
{
	ushort	index;
	uchar	work;
	ushort	startIndex;
	ushort	endIndex;
	
	if(tno == 0) {
		return LKCOM_TYPE_NONE;
	}
	
	work = GetCarInfoParam();						// ロック／フラップ装置制御設定取得
	switch(work){
		case 0x01 : 								// ロック装置のみ設定ありの場合
			startIndex = BIKE_START_INDEX;
			endIndex = LOCK_MAX;
			break;
		case 0x04 : 								// フラップ装置のみ設定ありの場合
			startIndex = INT_CAR_START_INDEX;
			endIndex = BIKE_START_INDEX;
			break;
		case 0x05 : 								// フラップとロック共に設定ありの場合
			startIndex = INT_CAR_START_INDEX;
			endIndex = LOCK_MAX;
			break;
		default :									// 未設定の場合
			startIndex = 0;
			endIndex = 0;
			break;
	}
	for(index = startIndex; index < endIndex; ++index ) {
		WACDOG;										// 装置ループの際はWDGリセット実行
		if(LockInfo[index].if_oya == tno) {
			if(index < BIKE_START_INDEX) {
				return LKCOM_TYPE_FLAP;				// 装置はフラップ（駐車）
			}
			else {
				return LKCOM_TYPE_LOCK;				// 装置はロック（駐輪）
			}
		}
	}
	return	LKCOM_TYPE_NONE;
}
// MH322914(S) K.Onodera 2016/09/07 AI-V対応：エラーアラーム
/*[]----------------------------------------------------------------------[]*/
/*| 車室番号からインデックスを取得する				                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_RoomNoToType	               	                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2009-10-04                                              |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
int	LKcom_RoomNoToIndex( ulong roomNo )
{
	int		i;
	uchar	hit = 0;
	
	for( i=INT_CAR_START_INDEX; i < LOCK_MAX; ++i ){
		WACDOG;										// 装置ループの際はWDGリセット実行
		if(LockInfo[i].posi == roomNo) {
			hit = 1;
			break;
		}
	}
	if( !hit ){
		i = -1;
	}
	return i;
}
// MH322914(E) K.Onodera 2016/09/07 AI-V対応：エラーアラーム
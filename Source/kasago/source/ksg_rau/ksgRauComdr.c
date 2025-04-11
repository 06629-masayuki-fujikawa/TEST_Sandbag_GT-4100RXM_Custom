/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		ドライバー関数																						   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	ksgRauComdr.c																			   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・SCI受信データのCOMインターフェース																	   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

#include	<string.h>															/*								*/
#include	"system.h"															/*								*/
#include	"prm_tbl.h"
#include	"iodefine.h"														/*								*/
#include 	"ksgRauTable.h"														/* ﾃｰﾌﾞﾙﾃﾞｰﾀ定義				*/
#include	"ksgRauSci.h"														/*								*/
#include 	"ksg_def.h"
#include 	"ope_def.h"

/*[]--- ﾌﾟﾛﾄﾀｲﾌﾟ宣言 -----------------------------------------------------------------------------------------[]*/
void		KSG_RauComdrMain( void );												/*								*/
void		KSG_RauComdrCharSetToRcvArea( void );									/*								*/
void		KSG_RauComdrRcvInit( void );											/*								*/
void		KSG_RauMdmEarlyDataInit( void );

t_RauComQueCtrl				KSG_RauComdrDataQueueCtrl;
t_KSG_RauComdrDataQueue		KSG_RauComdrDataQueue[KSG_RauComdr_QUEUE_COUNT];
ushort 						KSG_RauComdr_Prev_W_Index;

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             KSG_RauComdrRcvInit()																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		communication initial routine																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	KSG_RauComdrRcvInit( void )												/*								*/
{																				/*								*/
	KSG_RauRcvCtrl.OvfCount		= 0;											/*								*/
	KSG_RauRcvCtrl.ComerrStatus	= 0;											/*								*/
}																				/*								*/

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             KSG_RauComdrMain()																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	KSG_RauComdrMain( void )												/*								*/
{																				/*								*/
	KSG_RauComdrCharSetToRcvArea();												/* 受信ｲﾍﾞﾝﾄ判定				*/
	KSG_RauComdrRcvInit();														/*								*/
}																				/*								*/

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             KSG_RauComdrCharSetToRcvArea()																   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		received character set to temporary area															   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	KSG_RauComdrCharSetToRcvArea( void )									/*								*/
{																				/*								*/
	ushort	checkLimitIndex;													// 受信済みのインデックスの上限
																				/*								*/
	if( KSG_RauComdr_f_RcvCmp != 0){											/* received already received(Y)	*/
		return;
	}																			/*								*/
																				/*								*/
	if( KSG_RauRcvCtrl.ComerrStatus != 0 ){										/*								*/
		if( KSG_RauRcvCtrl.ComerrStatus & 0x01 ){								/* オーバーランエラー 			*/
// TODO:遠隔NT-NETでシリアル関連のエラー登録が必要か要確認
//			err_chk( ERRMDL_NTNETDOPA, ERR_NTNETDOPA_OVERRUN_ERR, 2, 0, 0 ); 	
		}
		if( KSG_RauRcvCtrl.ComerrStatus & 0x02 ){								/* フレーミングエラー 			*/
// TODO:遠隔NT-NETでシリアル関連のエラー登録が必要か要確認
//			err_chk( ERRMDL_NTNETDOPA, ERR_NTNETDOPA_FRAMING_ERR, 2, 0, 0 ); 	
		}
		if( KSG_RauRcvCtrl.ComerrStatus & 0x04 ){								/* パリティエラー 				*/
// TODO:遠隔NT-NETでシリアル関連のエラー登録が必要か要確認
//			err_chk( ERRMDL_NTNETDOPA, ERR_NTNETDOPA_PARITY_ERR, 2, 0, 0 ); 	
		}
		KSG_RauRcvCtrl.ComerrStatus = 0;
		KSG_RauComdr_f_RcvCmp = 2;												/*								*/
		return;
	}																			/*								*/
																				/*								*/
	if( KSG_RauRcvCtrl.OvfCount != 0 ){ 										/*								*/
		KSG_RauComdr_f_RcvCmp = 5;												/*								*/
		return;
	}																			/*								*/
																				/*								*/
	// 受信が完了したデータを全て処理する
	checkLimitIndex = KSG_RauComdrDataQueueCtrl.W_Index;
	if(KSG_RauComdr_Prev_W_Index != checkLimitIndex) {
		while(KSG_RauComdr_Prev_W_Index != checkLimitIndex) {
			tfRecvInterface(gInterfaceHandle_PPP_rau);							/* 受信通知						*/

			++KSG_RauComdr_Prev_W_Index;
			if(KSG_RauComdr_Prev_W_Index >= KSG_RauComdr_QUEUE_COUNT) {
				KSG_RauComdr_Prev_W_Index = 0;
			}
		}
		KSG_RauComdr_f_RcvCmp = 1;
	}
	else {
		KSG_RauComdr_f_RcvCmp = 3;
	}
	return;
}																				/*								*/

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             KSG_RauComInit()																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		SCI初期化																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void KSG_RauComInit( void )
{
	KSG_RauMdmEarlyDataInit();													/* ﾓﾃﾞﾑ ﾃﾞｰﾀ初期化			*/

	KSG_RauComdr_f_RcvCmp = 0;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             ppp0DrvOpen()																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		KASAGO用I/F																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
int ppp0DrvOpen(ttUserInterface interfaceHandle)
{
	KSG_RauComdrRcvInit();
	memset(&KSG_RauComdrDataQueue, 0, sizeof(KSG_RauComdrDataQueue));
    return(TM_DEV_OKAY);
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             ppp0DrvClose()																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		KASAGO用I/F																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
int ppp0DrvClose(ttUserInterface interfaceHandle)
{
    return(TM_DEV_OKAY);
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             ppp0DrvIoctl()																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		KASAGO用I/F																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
int ppp0DrvIoctl(ttUserInterface interfaceHandle,
                int flag,void TM_FAR *   optionPtr,int optionLen)
{
    return(TM_DEV_OKAY);
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             ppp0DrvSend()																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		KASAGO用I/F																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
int ppp0DrvSend( ttUserInterface interfaceHandle,
                char TM_FAR *   dataPtr,int dataLength,int flag )
{
	ulong 	StartTime;															/*								*/
	ulong 	SendWaitTime;														/*								*/
																				/*								*/
	memcpy(KSG_RauSndBuf, dataPtr, (size_t)dataLength);
																				/*								*/
	if( KSG_RauSciSndReq( (ushort)dataLength ) != 0 ){							/* send NG ?					*/
		return	(uchar)1;														/*								*/
	}																			/*								*/
	StartTime = LifeTim2msGet();												/* now life time get			*/
	SendWaitTime = LifeTim2msGet();												/* now life time get			*/
																				/*								*/
	while( 0 == LifePastTim2msGet(StartTime, 600L) ){							/* till timeout (1sec)			*/
		if( LifePastTimGet(SendWaitTime) >= 10 ){								// 100ms以上経過？
			WACDOG;
			SendWaitTime = LifeTim2msGet();										/* now life time get			*/
		}
																				/*								*/
		if( 1 == KSG_RauSciIsSndCmp() ){										/* send complete (Y) 			*/
			if (flag==TM_USER_BUFFER_LAST){
				/* 送信完了通知*/
				tfSendCompleteInterface (interfaceHandle, TM_DEV_SEND_COMPLETE_DRIVER);
			}
			return	(uchar)0;													/*								*/
		}																		/*								*/
	}																			/*								*/
    return(TM_DEV_OKAY);
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             ppp0DrvRecv()																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		KASAGO用I/F																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
int ppp0DrvRecv(ttUserInterface   interfaceHandle,char TM_FAR * TM_FAR * dataPtr,int  TM_FAR * dataLength,ttUserBufferPtr bufHandlePtr)
{
    int		errorCode;
	t_KSG_RauComdrDataQueue* pQueue;
    errorCode = TM_DEV_OKAY;

	if(KSG_RauComdrDataQueueCtrl.Count) { 
		pQueue = &KSG_RauComdrDataQueue[KSG_RauComdrDataQueueCtrl.R_Index++];
		if(KSG_RauComdrDataQueueCtrl.R_Index >= KSG_RauComdr_QUEUE_COUNT) {
			KSG_RauComdrDataQueueCtrl.R_Index = 0;
		}
		--KSG_RauComdrDataQueueCtrl.Count;
		/* 受信バッファのアドレスを格納*/
		*dataPtr = (char *)&pQueue->RcvData[0];
		/* 長さを格納 */
		*dataLength = pQueue->RcvLength;
	}
	/* (重要) バッファハンドルを無効にする*/
	*bufHandlePtr = (ttUserBufferPtr)0;

	return(errorCode);
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             ppp0DrvFreeReceiveBuffer()																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		KASAGO用I/F																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
int ppp0DrvFreeReceiveBuffer(ttUserInterface interfaceHandle,char TM_FAR *   dataPtr)
{
	KSG_RauComdrRcvInit();
	
    return TM_DEV_OKAY;
}

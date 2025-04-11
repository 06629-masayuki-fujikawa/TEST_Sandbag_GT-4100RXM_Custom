/*[]----------------------------------------------------------------------[]*/
/*| LED control function                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : M.Okuda, modified by Hara                                |*/
/*| Date        : 2005-02-01                                               |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"lcd_def.h"
#include	"can_def.h"

/*[]----------------------------------------------------------------------[]*/
/*| LED Initial                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LedInit                                                 |*/
/*| PARAMETER    : none                                                    |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda, modified by Hara                               |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LedInit( void )
{
	unsigned char	CtlCode;
	unsigned char	LEDPattan;

	/*** all LED off ***/
	// メイン基板制御のLED OFF
	CP_RD_SHUTLED = 0;									// 磁気リーダーシャッタLED OFF

	// フロント基板制御のLED OFF
	LEDPattan = 0;										// フロント基板全てのLED OFF
	CtlCode = CAN_CTRL_NONE;
	can_snd_data3(CtlCode, LEDPattan);					// CANコントロール送信

	/*** control area initial ***/
	memset( &LedCtrl, 0, sizeof(t_LedCtrl) );
}

/*[]----------------------------------------------------------------------[]*/
/*| LED 制御                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LedReq                                                  |*/
/*| PARAMETER    : LedKind : 制御するLEDの種類                             |*/
/*|                OnOff   : 0=消灯，1=点灯，2=点滅(LEDのみ)               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda, modified by Hara                               |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LedReq( unsigned char LedKind, unsigned char OnOff )
{
	unsigned char	i;
	unsigned char	CtlCode;
	unsigned char	LEDPattan;
	LEDPattan = 0;

	if(LedKind >= LEDCT_MAX) return;

	/*** 点滅要求に対する処理 ***/

	/*** 点滅要求を受けた場合 ***/
	if( 2 == OnOff ){												// 点滅 req (Y)
		if( 0 == LedCtrl.Phase ){									// 現在点滅中のLEDなし(Y)
			LedCtrl.Count = LED_OFF_TIME - 1;						// 次の20msで点灯開始
			LedCtrl.OnOff = LED_OFF;
			LedCtrl.Phase = 1;
		}
	}

	/*** 点滅要求で無い場合 ***/
	else{
		if( 0 != LedCtrl.Phase ){									// 現在点滅中のLEDあり(Y)

			/** 今回の点滅キャンセルで、他に点滅しているものはないか捜す **/
			LedCtrl.Request[ LedKind ] = OnOff;
			for( i=0; i<LEDCT_MAX; ++i ){
				if( LED_ONOFF == LedCtrl.Request[i] ){
					break;
				}
			}
			if( LEDCT_MAX == i ){									// 他にはいない
				LedCtrl.Phase = 0;									// 点滅Stop
			}
		}
	}


	/*** 消灯／点灯処理 ***/

	switch( LedKind ){
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//		case RD_SHUTLED:											// 磁気リーダーシャッタLED制御
//			if( 1 == OnOff ){
//				CP_RD_SHUTLED = 1;									// LED ON
//			}else if(0 == OnOff){
//				CP_RD_SHUTLED = 0;									// LED OFF
//			}
//			break;
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)

		case CN_TRAYLED:											// 釣銭取出し口LED制御
			CtlCode = CAN_CTRL_NONE;
			if( 1 == OnOff ){
				LEDPattan = CAN_LED_TRAY;							// LED ON
			}else if(0 == OnOff){
				LEDPattan = 0;										// LED OFF
			}
			can_snd_data3(CtlCode, LEDPattan);						// CANコントロール送信
			break;
	}

	LedCtrl.Request[ LedKind ] = OnOff;
}

/*[]----------------------------------------------------------------------[]*/
/*| LED 状況取得                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : IsLedCtrl                                               |*/
/*| PARAMETER    : LedKind : 制御するLEDの種類                             |*/
/*| RETURN VALUE : 0:消灯，1:点灯，2=点滅（LEDのみ）                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda, modified by Hara                               |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	IsLedReq( unsigned char LedKind )
{
	return LedCtrl.Request[ LedKind ];
}

/*[]----------------------------------------------------------------------[]*/
/*| LED 点滅制御                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LedOnOffCtrl                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda, modified by Hara                               |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LedOnOffCtrl( void )
{
	char CtlCode, LEDPattan;
	
	if( 0 == LedCtrl.Phase ){										// 点滅要求なし
		return;
	}

	/*** 点滅制御処理 ***/
	if( LED_OFF == LedCtrl.OnOff ){									// 現在消灯状態
		++LedCtrl.Count;
		if( LED_OFF_TIME <= LedCtrl.Count ){						// ONにするﾀｲﾐﾝｸﾞ (Y)
			LedCtrl.OnOff = LED_ON;
			LedCtrl.Count = 0;

// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
// #if (LEDCT_MAX >= 1)
// 			if( LED_ONOFF == LedCtrl.Request[RD_SHUTLED] ){			// 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED 点滅要求中 (Y)
// 				CP_RD_SHUTLED = 1;									// LED ON
// 			}
// #endif
// #if (LEDCT_MAX >= 2)
// 			if( LED_ONOFF == LedCtrl.Request[CN_TRAYLED] ){			// 釣銭取出し口ｶﾞｲﾄﾞLED 点滅要求中 (Y)
// 				CtlCode = CAN_CTRL_NONE;
// 				LEDPattan = CAN_LED_TRAY;							// LED ON
// 				can_snd_data3(CtlCode, LEDPattan);					// CANコントロール送信
// 			}
// #endif
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)

#if (LEDCT_MAX >= 3)
			}
#endif

		}
	}

	else{															// 現在点灯状態
		++LedCtrl.Count;
		if( LED_ON_TIME <= LedCtrl.Count ){							// OFFにするﾀｲﾐﾝｸﾞ (Y)
			LedCtrl.OnOff = LED_OFF;
			LedCtrl.Count = 0;

// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
// #if (LEDCT_MAX >= 1)
// 			if( LED_ONOFF == LedCtrl.Request[RD_SHUTLED] ){			// 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED 点滅要求中 (Y)
// 				CP_RD_SHUTLED = 0;									// LED ON
// 			}
// #endif
//
//#if (LEDCT_MAX >= 2)
#if (LEDCT_MAX >= 0)
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)

			if( LED_ONOFF == LedCtrl.Request[CN_TRAYLED] ){			// 釣銭取出し口ｶﾞｲﾄﾞLED 点滅要求中 (Y)
				CtlCode = CAN_CTRL_NONE;
				LEDPattan = 0;										// LED OFF
				can_snd_data3(CtlCode, LEDPattan);					// CANコントロール送信
			}
#endif

#if (LEDCT_MAX >= 3)
			if( LED_ONOFF == LedCtrl.Request[2] ){					// 釣銭取出し口ｶﾞｲﾄﾞLED 点滅要求中 (Y)
			}
#endif
		}
	}
}

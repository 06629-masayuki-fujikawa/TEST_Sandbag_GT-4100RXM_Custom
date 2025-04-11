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
	// ���C��������LED OFF
	CP_RD_SHUTLED = 0;									// ���C���[�_�[�V���b�^LED OFF

	// �t�����g������LED OFF
	LEDPattan = 0;										// �t�����g��S�Ă�LED OFF
	CtlCode = CAN_CTRL_NONE;
	can_snd_data3(CtlCode, LEDPattan);					// CAN�R���g���[�����M

	/*** control area initial ***/
	memset( &LedCtrl, 0, sizeof(t_LedCtrl) );
}

/*[]----------------------------------------------------------------------[]*/
/*| LED ����                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LedReq                                                  |*/
/*| PARAMETER    : LedKind : ���䂷��LED�̎��                             |*/
/*|                OnOff   : 0=�����C1=�_���C2=�_��(LED�̂�)               |*/
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

	/*** �_�ŗv���ɑ΂��鏈�� ***/

	/*** �_�ŗv�����󂯂��ꍇ ***/
	if( 2 == OnOff ){												// �_�� req (Y)
		if( 0 == LedCtrl.Phase ){									// ���ݓ_�Œ���LED�Ȃ�(Y)
			LedCtrl.Count = LED_OFF_TIME - 1;						// ����20ms�œ_���J�n
			LedCtrl.OnOff = LED_OFF;
			LedCtrl.Phase = 1;
		}
	}

	/*** �_�ŗv���Ŗ����ꍇ ***/
	else{
		if( 0 != LedCtrl.Phase ){									// ���ݓ_�Œ���LED����(Y)

			/** ����̓_�ŃL�����Z���ŁA���ɓ_�ł��Ă�����̂͂Ȃ����{�� **/
			LedCtrl.Request[ LedKind ] = OnOff;
			for( i=0; i<LEDCT_MAX; ++i ){
				if( LED_ONOFF == LedCtrl.Request[i] ){
					break;
				}
			}
			if( LEDCT_MAX == i ){									// ���ɂ͂��Ȃ�
				LedCtrl.Phase = 0;									// �_��Stop
			}
		}
	}


	/*** �����^�_������ ***/

	switch( LedKind ){
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		case RD_SHUTLED:											// ���C���[�_�[�V���b�^LED����
//			if( 1 == OnOff ){
//				CP_RD_SHUTLED = 1;									// LED ON
//			}else if(0 == OnOff){
//				CP_RD_SHUTLED = 0;									// LED OFF
//			}
//			break;
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)

		case CN_TRAYLED:											// �ޑK��o����LED����
			CtlCode = CAN_CTRL_NONE;
			if( 1 == OnOff ){
				LEDPattan = CAN_LED_TRAY;							// LED ON
			}else if(0 == OnOff){
				LEDPattan = 0;										// LED OFF
			}
			can_snd_data3(CtlCode, LEDPattan);						// CAN�R���g���[�����M
			break;
	}

	LedCtrl.Request[ LedKind ] = OnOff;
}

/*[]----------------------------------------------------------------------[]*/
/*| LED �󋵎擾                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : IsLedCtrl                                               |*/
/*| PARAMETER    : LedKind : ���䂷��LED�̎��                             |*/
/*| RETURN VALUE : 0:�����C1:�_���C2=�_�ŁiLED�̂݁j                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda, modified by Hara                               |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	IsLedReq( unsigned char LedKind )
{
	return LedCtrl.Request[ LedKind ];
}

/*[]----------------------------------------------------------------------[]*/
/*| LED �_�Ő���                                                           |*/
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
	
	if( 0 == LedCtrl.Phase ){										// �_�ŗv���Ȃ�
		return;
	}

	/*** �_�Ő��䏈�� ***/
	if( LED_OFF == LedCtrl.OnOff ){									// ���ݏ������
		++LedCtrl.Count;
		if( LED_OFF_TIME <= LedCtrl.Count ){						// ON�ɂ������ݸ� (Y)
			LedCtrl.OnOff = LED_ON;
			LedCtrl.Count = 0;

// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
// #if (LEDCT_MAX >= 1)
// 			if( LED_ONOFF == LedCtrl.Request[RD_SHUTLED] ){			// ���Cذ�ް�޲��LED �_�ŗv���� (Y)
// 				CP_RD_SHUTLED = 1;									// LED ON
// 			}
// #endif
// #if (LEDCT_MAX >= 2)
// 			if( LED_ONOFF == LedCtrl.Request[CN_TRAYLED] ){			// �ޑK��o�����޲��LED �_�ŗv���� (Y)
// 				CtlCode = CAN_CTRL_NONE;
// 				LEDPattan = CAN_LED_TRAY;							// LED ON
// 				can_snd_data3(CtlCode, LEDPattan);					// CAN�R���g���[�����M
// 			}
// #endif
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)

#if (LEDCT_MAX >= 3)
			}
#endif

		}
	}

	else{															// ���ݓ_�����
		++LedCtrl.Count;
		if( LED_ON_TIME <= LedCtrl.Count ){							// OFF�ɂ������ݸ� (Y)
			LedCtrl.OnOff = LED_OFF;
			LedCtrl.Count = 0;

// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
// #if (LEDCT_MAX >= 1)
// 			if( LED_ONOFF == LedCtrl.Request[RD_SHUTLED] ){			// ���Cذ�ް�޲��LED �_�ŗv���� (Y)
// 				CP_RD_SHUTLED = 0;									// LED ON
// 			}
// #endif
//
//#if (LEDCT_MAX >= 2)
#if (LEDCT_MAX >= 0)
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)

			if( LED_ONOFF == LedCtrl.Request[CN_TRAYLED] ){			// �ޑK��o�����޲��LED �_�ŗv���� (Y)
				CtlCode = CAN_CTRL_NONE;
				LEDPattan = 0;										// LED OFF
				can_snd_data3(CtlCode, LEDPattan);					// CAN�R���g���[�����M
			}
#endif

#if (LEDCT_MAX >= 3)
			if( LED_ONOFF == LedCtrl.Request[2] ){					// �ޑK��o�����޲��LED �_�ŗv���� (Y)
			}
#endif
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Lag Timer module                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  M.Okuda                                                 |*/
/*| Date        :  2002-07-30                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"Message.h"

/*** external RAM with lithium battery ***/
/*** internal RAM ***/
extern short    LagTim10msReq[LAG10_MAX];
extern void     ( *LagTim10msFunc[LAG10_MAX] )( void );
extern short    LagTim20msReq[LAG20_MAX];
extern void     ( *LagTim20msFunc[LAG20_MAX] )( void );
extern short    LagTim500msReq[LAG500_MAX];
extern void     ( *LagTim500msFunc[LAG500_MAX] )( void );

/*** ROM constant ***/
/*** local functions ***/

/*==========================================================================*/
/*	ү���ޑ��M�^	LAG�^�C�}�[												*/
/*==========================================================================*/

/*[]----------------------------------------------------------------------[]*/
/*| �����ϰ����                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lagtim                                                  |*/
/*| PARAMETER    : ucId = TaskID ("xxxTBCNO")                              |*/
/*|                ucNo = Timer number (1�` )                              |*/
/*|                usTime = Wait time (x20ms unit)                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-25                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
Lagtim( unsigned char ucId , unsigned char ucNo , unsigned short usTime )
{
	switch( ucId ){
		case OPETCBNO: /* ���ڰ�������p */
			if( ucNo <= OPE_TIM_MAX ){
				_di();
				OPE_Timer[ucNo-1] = usTime;
				_ei();
			}
			break;
		case PRNTCBNO: /* ���������p */
			if( ucNo <= PRN_TIM_MAX ){
				_di();
				PRN_Timer[ucNo-1] = usTime;
				_ei();
			}
			break;
		case NTNETTCBNO: /* NTNET����p */
			if( ucNo <= NTNET_TIM_MAX ){
				_di();
				NTNET_Timer[ucNo-1] = usTime;
				_ei();
			}
			break;
		case NTNETDOPATCBNO: /* NTNET-DOPA����p */
			if( ucNo <= NTNETDOPA_TIM_MAX ){
				_di();
				NTNETDOPA_Timer[ucNo-1] = usTime;
				_ei();
			}
			break;
		case REMOTEDLTCBNO: /* ���u�޳�۰������p */
			if( ucNo <= REMOTEDL_TIM_MAX ){
				_di();
				REMOTEDL_Timer[ucNo-1] = usTime;
				_ei();
			}
			break;
		case CANTCBNO: /* CAN����p */
			if( ucNo <= CAN_TIM_MAX ){
				_di();
				CAN_Timer[ucNo-1] = usTime;
				_ei();
			}
			break;
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
		case PKTTCBNO: // PKT����p */
			if( ucNo <= PKT_TIM_MAX ){
				_di();
				PKT_Timer[ucNo-1] = usTime;
				_ei();
			}		
			break;
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
		default:
			break;
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �����ϰ�į��                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lagcan                                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-25                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
Lagcan( unsigned char ucId , unsigned char ucNo )
{
	switch( ucId ){
		case OPETCBNO: /* ���ڰ�������p */
			if( ucNo <= OPE_TIM_MAX ){
				_di();
				OPE_Timer[ucNo-1] = 0;
				_ei();
			}
			break;
		case PRNTCBNO: /* ���������p */
			if( ucNo <= PRN_TIM_MAX ){
				_di();
				PRN_Timer[ucNo-1] = 0;
				_ei();
			}
			break;
		case NTNETTCBNO: /* NTNET����p */
			if( ucNo <= NTNET_TIM_MAX ){
				_di();
				NTNET_Timer[ucNo-1] = 0;
				_ei();
			}
			break;
		case NTNETDOPATCBNO: /* NTNET-DOPA����p */
			if( ucNo <= NTNETDOPA_TIM_MAX ){
				_di();
				NTNETDOPA_Timer[ucNo-1] = 0;
				_ei();
			}
			break;
		case REMOTEDLTCBNO: /* NTNET-DOPA����p */
			if( ucNo <= REMOTEDL_TIM_MAX ){
				_di();
				REMOTEDL_Timer[ucNo-1] = 0;
				_ei();
			}
			break;
		case CANTCBNO: /* CAN����p */
			if( ucNo <= CAN_TIM_MAX ){
				_di();
				CAN_Timer[ucNo-1] = 0;
				_ei();
			}
			break;
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
		case PKTTCBNO: // PKT����p
			if( ucNo <= PKT_TIM_MAX ){
				_di();
				PKT_Timer[ucNo-1] = 0;
				_ei();
			}
			break;
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
		default:
			break;
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 20ms LAG�^�C�}�[ �^�C���A�E�g�`�F�b�N����                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagChk                                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void	LagChk( void )
{
    unsigned char	i;

	for( i = 0; i < OPE_TIM_MAX; i++ ){
		if( OPE_Timer[i] != 0 ){
			OPE_Timer[i]--;
			if( OPE_Timer[i] == 0 ){
				queset( OPETCBNO, (unsigned short)(TIMEOUT1+i), 0, NULL ); /* timeout set */
			}
		}
	}
	for( i = 0; i < PRN_TIM_MAX; i++ ){
		if( PRN_Timer[i] != 0 ){
			PRN_Timer[i]--;
			if( PRN_Timer[i] == 0 ){
				queset( PRNTCBNO, (unsigned short)(TIMEOUT1+i), 0, NULL ); /* timeout set */
			}
		}
	}
	for( i = 0; i < NTNET_TIM_MAX; i++ ){
		if( NTNET_Timer[i] != 0 ){
			NTNET_Timer[i]--;
			if( NTNET_Timer[i] == 0 ){
				queset( NTNETTCBNO, (unsigned short)(TIMEOUT1+i), 0, NULL ); /* timeout set */
			}
		}
	}
	for( i = 0; i < NTNETDOPA_TIM_MAX; i++ ){
		if( NTNETDOPA_Timer[i] != 0 ){
			NTNETDOPA_Timer[i]--;
			if( NTNETDOPA_Timer[i] == 0 ){
				queset( NTNETDOPATCBNO, (unsigned short)(TIMEOUT1+i), 0, NULL ); /* timeout set */
			}
		}
	}
	for( i = 0; i < REMOTEDL_TIM_MAX; i++ ){
		if( REMOTEDL_Timer[i] != 0 ){
			REMOTEDL_Timer[i]--;
			if( REMOTEDL_Timer[i] == 0 ){
				queset( REMOTEDLTCBNO, (unsigned short)(TIMEOUT1+i), 0, NULL ); /* timeout set */
			}
		}
	}
	for( i = 0; i < CAN_TIM_MAX; i++ ){
		if( CAN_Timer[i] != 0 ){
			CAN_Timer[i]--;
			if( CAN_Timer[i] == 0 ){
				queset( CANTCBNO, (unsigned short)(TIMEOUT1+i), 0, NULL ); /* timeout set */
			}
		}
	}
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
	for( i = 0; i < PKT_TIM_MAX; i++ ){
		if( PKT_Timer[i] != 0 ){
			PKT_Timer[i]--;
			if( PKT_Timer[i] == 0 ){
				queset( PKTTCBNO, (unsigned short)(TIMEOUT1+i), 0, NULL ); /* timeout set */
			}
		}
	}
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
}
/*[]----------------------------------------------------------------------[]*/
/*| �^�X�N�^�C�}�[���쒆�`�F�b�N                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagChkExe                                               |*/
/*| PARAMETER    : uchar 0�F��~���A1�F���쒆                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : A.Iiizumi                                               |*/
/*| Date         : 2014-03-18                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned char
LagChkExe( unsigned char ucId , unsigned char ucNo )
{
	unsigned char ret = 0;//�����l�͒�~��
	unsigned short timer = 0;//�����l�͒�~��
	switch( ucId ){
		case OPETCBNO: /* ���ڰ�������p */
			if( ucNo <= OPE_TIM_MAX ){
				_di();
				timer = OPE_Timer[ucNo-1];
				_ei();
			}
			break;
		case PRNTCBNO: /* ���������p */
			if( ucNo <= PRN_TIM_MAX ){
				_di();
				timer = PRN_Timer[ucNo-1];
				_ei();
			}
			break;
		case NTNETTCBNO: /* NTNET����p */
			if( ucNo <= NTNET_TIM_MAX ){
				_di();
				timer = NTNET_Timer[ucNo-1];
				_ei();
			}
			break;
		case NTNETDOPATCBNO: /* NTNET-DOPA����p */
			if( ucNo <= NTNETDOPA_TIM_MAX ){
				_di();
				timer = NTNETDOPA_Timer[ucNo-1];
				_ei();
			}
			break;
		case REMOTEDLTCBNO: /* NTNET-DOPA����p */
			if( ucNo <= REMOTEDL_TIM_MAX ){
				_di();
				timer = REMOTEDL_Timer[ucNo-1];
				_ei();
			}
			break;
		case CANTCBNO: /* CAN����p */
			if( ucNo <= CAN_TIM_MAX ){
				_di();
				timer = CAN_Timer[ucNo-1];
				_ei();
			}
			break;
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
		case PKTTCBNO: // PKT����p
			if( ucNo <= PKT_TIM_MAX ){
				_di();
				timer = PKT_Timer[ucNo-1];
				_ei();
			}
			break;
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
		default:
			break;
	}

	if(timer != 0){// �^�C�}�[���쒆
		ret = 1;
	}
	return(ret);
}
/*==========================================================================*/
/*	�֐�Call�^	LAG�^�C�}�[													*/
/*==========================================================================*/

/*[]----------------------------------------------------------------------[]*/
/*| LAG�^�C�}�[������                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagX0msInit                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void	Lag10msInit( void )
{
    char    i;

    for(i=0; i<LAG10_MAX; ++i){
        LagTim10msReq[i] = 0;
        LagTim10msFunc[i] = (void*)NULL;
    }
}

void	Lag20msInit( void )
{
    char    i;

    for(i=0; i<LAG20_MAX; ++i){
        LagTim20msReq[i] = 0;
        LagTim20msFunc[i] = (void*)NULL;
    }
}

void	Lag500msInit( void )
{
    char    i;

    for(i=0; i<LAG500_MAX; ++i){
        LagTim500msReq[i] = 0;
        LagTim500msFunc[i] = (void*)NULL;
    }
}

/*[]----------------------------------------------------------------------[]*/
/*| 10ms LAG�^�C�}�[�o�^                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagTim10ms                                              |*/
/*| PARAMETER    : no   : timer no.(0�` )                                  |*/
/*|                time : lag time (unit:10mS)                             |*/
/*|                func : function address                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void	LagTim10ms( char no, short time, void ( *func )( void ) )
{
    unsigned long   ist;        /* ���݂̊�����t��� */

    ist = _di2();
    LagTim10msReq [ no ] = time;
    LagTim10msFunc[ no ] = func;
    _ei2( ist );
}

/*[]----------------------------------------------------------------------[]*/
/*| 20ms LAG�^�C�}�[�o�^                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagTim20ms                                              |*/
/*| PARAMETER    : no   : timer no.(0�` )                                  |*/
/*|                time : lag time (unit:20mS)                             |*/
/*|                func : function address                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void LagTim20ms( char no, short time, void ( *func )( void ) )
{
    unsigned long   ist;

    ist = _di2();
    LagTim20msReq [ no ] = time;
    LagTim20msFunc[ no ] = func;
    _ei2( ist );
}

/*[]----------------------------------------------------------------------[]*/
/*| 500ms LAG�^�C�}�[�o�^                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagTim10ms                                              |*/
/*| PARAMETER    : no   : timer no.(0�` )                                  |*/
/*|                time : lag time (unit:500mS)                            |*/
/*|                func : function address                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void LagTim500ms( char no, short time, void ( *func )( void ) )
{
    unsigned long   ist;

    ist = _di2();
    LagTim500msReq [no] = time;
    LagTim500msFunc[no] = func;
    _ei2( ist );
}

/*[]----------------------------------------------------------------------[]*/
/*| X0ms LAG�^�C�}�[�L�����Z��                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagCanX0ms                                              |*/
/*| PARAMETER    : no   : timer no.(0�` )                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void	LagCan10ms( char no )
{
    unsigned long   ist;        /* ���݂̊�����t��� */

    ist = _di2();
    LagTim10msReq [no] = 0;
    LagTim10msFunc[no] = (void*)NULL;
    _ei2( ist );
}

void	LagCan20ms( char no )
{
    unsigned long   ist;

    ist = _di2();
    LagTim20msReq [no] = 0;
    LagTim20msFunc[no] = (void*)NULL;
    _ei2( ist );
}

void	LagCan500ms( char no )
{
    unsigned long   ist;

    ist = _di2();
    LagTim500msReq [no] = 0;
    LagTim500msFunc[no] = (void*)NULL;
    _ei2( ist );
}

/*[]----------------------------------------------------------------------[]*/
/*| X0ms LAG�^�C�}�[���������i�^�C���A�E�g�`�F�b�N�j                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagChkX0ms                                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void	LagChk10ms( void )
{
    char    i;

    for( i=0; i<LAG10_MAX; ++i ){
        if( 0 != LagTim10msReq[i] ){
            --LagTim10msReq[i];
            if( 0 == LagTim10msReq[i] ){
                ( *LagTim10msFunc[i] )();  /* execute lag function */
            }
        }
    }
}

void	LagChk20ms( void )
{
    char    i;

    for( i=0; i<LAG20_MAX; ++i ){
        if( 0 != LagTim20msReq[i] ){
            --LagTim20msReq[i];
            if( 0 == LagTim20msReq[i] ){
                ( *LagTim20msFunc[i] )();  /* execute lag function */
            }
        }
    }
}

void	LagChk500ms( void )
{
    char    i;

    for( i=0; i<LAG500_MAX; ++i ){
        if( 0 != LagTim500msReq[i] ){
            --LagTim500msReq[i];
            if( 0 == LagTim500msReq[i] ){
                ( *LagTim500msFunc[i] )();  /* execute lag function */
            }
        }
    }
}

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
/*[]----------------------------------------------------------------------[]*/
/*| X0ms LAG�^�C�}�[�ғ������擾�����i�ғ����`�F�b�N�j                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagTim500ms_Is_Counting                                 |*/
/*|              : LagTim20ms_Is_Counting                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 1=���Ē�(�N����)�A0=���Ē��łȂ�                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2006-02-16                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	LagTim20ms_Is_Counting( char no )
{
	if( 0 == LagTim20msReq[no] ){
		return	(uchar)0;
	}
	return	(uchar)1;
}

uchar	LagTim500ms_Is_Counting( char no )
{
	if( 0 == LagTim500msReq[no] ){
		return	(uchar)0;
	}
	return	(uchar)1;
}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

/*==========================================================================*/
/*	Easy�^�C�}�[															*/
/*==========================================================================*/

/*[]----------------------------------------------------------------------[]*/
/*| x10ms Life timer count get                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LifeTimGet                                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ���݂�x10ms�ώZ�^�C�}�l                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
unsigned long	LifeTimGet( void )
{
	return LifeTimer10ms;
}

/*[]----------------------------------------------------------------------[]*/
/*| x10ms Life timer Past time cal                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LifeTimGet                                              |*/
/*| PARAMETER    : StartTime = �^�C�}�J�n���� LifeTimGet()�œ�������       |*/
/*| RETURN VALUE : StartTime���_����̌o�ߎ���(x10ms)��Ԃ�                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
unsigned long	LifePastTimGet( unsigned long StartTime )
{
	unsigned long  NowTime;
	unsigned long  PastTime;

	NowTime = LifeTimGet();				/* ���݂�x10ms�ώZ���E���g�lget */
	if( NowTime >= StartTime ){
		PastTime = NowTime - StartTime;
	}
	else{
		PastTime = NowTime + ((0xffffffffL - StartTime) + 1);
	}
	return PastTime;
}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
/*[]----------------------------------------------------------------------[]*/
/*| x1ms Life timer count get                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LifeTim1msGet                                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ���݂�x1ms�ώZ�^�C�}�l                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
unsigned long	LifeTim1msGet( void )
{
	return LifeTimer1ms;
}

/*[]----------------------------------------------------------------------[]*/
/*| x1ms Life timer Past time cal                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LifeTim1msGet                                           |*/
/*| PARAMETER    : StartTime = �^�C�}�J�n���� LifeTimGet()�œ�������       |*/
/*| RETURN VALUE : StartTime���_����̌o�ߎ���(x1ms)��Ԃ�                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
unsigned long	LifePastTim1msGet( unsigned long StartTime )
{
	unsigned long  NowTime;
	unsigned long  PastTime;

	NowTime = LifeTim1msGet();				/* ���݂�x1ms�ώZ���E���g�lget */
	if( NowTime >= StartTime ){
		PastTime = NowTime - StartTime;
	}
	else{
		PastTime = NowTime + ((0xffffffffL - StartTime) + 1);
	}
	return PastTime;
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/*[]----------------------------------------------------------------------[]*/
/*| x2ms Life timer count get                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LifeTim2msGet                                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ���݂�x2ms�ώZ�^�C�}�l                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
unsigned long	LifeTim2msGet( void )
{
	return LifeTimer2ms;
}

/*[]----------------------------------------------------------------------[]*/
/*| x2ms Life timer Past time cal                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LifeTim2msGet                                           |*/
/*| PARAMETER    : StartTime = �^�C�}�J�n���� LifeTimGet()�œ�������       |*/
/*| RETURN VALUE : StartTime���_����̌o�ߎ���(x2ms)��Ԃ�                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-07-30                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
unsigned char	LifePastTim2msGet( unsigned long StartTime, unsigned long WaitTime )
{
	unsigned long  NowTime;
	unsigned long  PastTime;

	NowTime = LifeTim2msGet();				/* ���݂�x2ms�ώZ���E���g�lget */
	if( NowTime >= StartTime ){
		PastTime = NowTime - StartTime;
	}
	else{
		PastTime = NowTime + ((0xffffffffL - StartTime) + 1);
	}

	if( PastTime >= WaitTime ){
		return (unsigned char)1;
	}
	return (unsigned char)0;
}
/*[]----------------------------------------------------------------------[]*/
/*| �����ϰ����                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lagtim_1ms                                              |*/
/*| PARAMETER    : ucId = TaskID ("xxxTBCNO")                              |*/
/*|                ucNo = Timer number (1�` )                              |*/
/*|                usTime = Wait time (x20ms unit)                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void Lagtim_1ms( unsigned char ucId , unsigned char ucNo , unsigned short usTime )
{
	switch( ucId ){
		case OPETCBNO: /* ���ڰ�������p */
			if( ucNo <= OPE_TIM1MS_MAX ){
				_di();
				OPE_Tim_Base1ms[ucNo-1] = usTime;
				_ei();
			}
			break;
		case REMOTEDLTCBNO: /* ���u�޳�۰������p */
			if( ucNo <= REMOTEDL_TIM1MS_MAX ){
				_di();
				REMOTEDL_Tim_Base1ms[ucNo-1] = usTime;
				_ei();
			}
			break;
		default:
			break;
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| �����ϰ�į��                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lagcan_1ms                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void Lagcan_1ms( unsigned char ucId , unsigned char ucNo )
{
	switch( ucId ){
		case OPETCBNO: /* ���ڰ�������p */
			if( ucNo <= OPE_TIM1MS_MAX ){
				_di();
				OPE_Tim_Base1ms[ucNo-1] = 0;
				_ei();
			}
			break;
		case REMOTEDLTCBNO: /* NTNET-DOPA����p */
			if( ucNo <= REMOTEDL_TIM1MS_MAX ){
				_di();
				REMOTEDL_Tim_Base1ms[ucNo-1] = 0;
				_ei();
			}
			break;
		default:
			break;
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 1ms LAG�^�C�}�[ �^�C���A�E�g�`�F�b�N����                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagChk_1ms                                             |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void	LagChk_1ms( void )
{
    unsigned char	i;

	for( i = 0; i < OPE_TIM1MS_MAX; i++ ){
		if( OPE_Tim_Base1ms[i] != 0 ){
			OPE_Tim_Base1ms[i]--;
			if( OPE_Tim_Base1ms[i] == 0 ){
				queset( OPETCBNO, (unsigned short)(TIMEOUT1_1MS+i), 0, NULL ); /* timeout set */
			}
		}
	}
	for( i = 0; i < REMOTEDL_TIM1MS_MAX; i++ ){
		if( REMOTEDL_Tim_Base1ms[i] != 0 ){
			REMOTEDL_Tim_Base1ms[i]--;
			if( REMOTEDL_Tim_Base1ms[i] == 0 ){
				queset( REMOTEDLTCBNO, (unsigned short)(TIMEOUT1_1MS+i), 0, NULL ); /* timeout set */
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �o�ߎ���(x1ms�P��)�𓾂�@��2ms�P��									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Input  : StartTime = when started time value (got by c_Tim_Start())	   |*/
/*| Return : �o�ߎ��ԁ@= x1ms�l				 							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  okuda	                                               |*/
/*| Date        :  2009-05-12                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
unsigned long c_2msPastTimeGet( unsigned long StartTime )
{
	unsigned long	NowTime;
	unsigned long	PastTime;

	NowTime = LifeTim2msGet();				// Now 2ms counter value get
	if( NowTime >= StartTime ){
		PastTime = NowTime - StartTime;
	}
	else{
		PastTime = NowTime + ((0xffffffffL - StartTime) + 1);
	}

	PastTime <<= 1;							// �o�ߎ��Ԃ�1ms�P�ʂɂ���(x2)
	return	PastTime;
}

/*[]----------------------------------------------------------------------[]*/
/*| �o�ߎ���(x1ms�P��)�𓾂�@��2ms�P��									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Input  : StartTime = when started time value (got by c_Tim_Start())	   |*/
/*| Return : �o�ߎ��ԁ@= x1ms�l				 							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  okuda	                                               |*/
/*| Date        :  2009-05-12                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
unsigned long c_PastTime_Subtraction( unsigned long Time, unsigned long SubValue )
{
	unsigned long	Result;

	if( Time >= SubValue ){
		Result = Time - SubValue;
	}else{
		SubValue = SubValue - Time - 1;
		Result = 0xffffffffL - SubValue;
	}
	return	Result;
}

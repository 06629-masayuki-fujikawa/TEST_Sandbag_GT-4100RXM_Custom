/*[]----------------------------------------------------------------------[]*/
/*| Input port sens (KEY etc)                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"lcd_def.h"

const unsigned short port_in_tbl[PortInTblMax] = {					// Scan Event Table
		KEY_RTSW1,	KEY_RTSW2,	KEY_RTSW3,	KEY_RTSW4,				// �g��I/O���Ͳ����
		KEY_DIPSW1,	KEY_DIPSW2,	KEY_DIPSW3,	KEY_DIPSW4,
		0,			0,			KEY_FANET,	KEY_FANST,
		KEY_MODECHG,KEY_DOORNOBU,KEY_MIN,	KEY_RXIIN,
};

/*[]----------------------------------------------------------------------[]*/
/*| �����߰ď���ذ��                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : key_read                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void : ucKeyBefore                                      |*/
/*|              : key_dat[0-6,8-28]                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  R.Hara                                                 |*/
/*| Date         :  2005-02-01                                             |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		KEY_INIT_RD		5
void	key_read( void )
{
	unsigned short	wbit;											// �߰� Ͻ��pܰ��ޯ̧
	unsigned short	ucbit;											// �ω��߰� ܰ��ޯ̧
	unsigned char	i,j;
	unsigned char datcnt;											// Same Data Count
	unsigned char door_scan;

	// ���͐M����ԏ��񃊁[�h
	datcnt = 0;
	for( j = 0; j < 20; j++ ){										// Max 20�ƍ�
		ucbit = ex_port_red();
		wait2us( 50L );												// 100us wait
		if( ucbit == ex_port_red() ){								// Same?(Y)
			datcnt++;
		}else{
			datcnt = 0;
		}
		if( datcnt >= KEY_INIT_RD ){								// 100us 5��ƍ����ē���?(Y)
			break;
		}															// 100us20��NG�͍ŏI�ް��L��
	}
	port_in_dat.WORD = ucbit;
	port_data[0] = ucbit;
	port_data[1] = ucbit;
	port_data[2] = ucbit;
	// �N�����̓��̓|�[�g�̏�Ԃ��C�x���g�Ƃ��Ēʒm����(�A���K�v�ȃC�x���g�̂�)
	for( wbit = 0x0001, i = 0; i < PortInTblMax; wbit <<= 1, i++ ){
		if( port_in_tbl[i] != 0 ){
			if(( port_in_tbl[i] == KEY_FANET)||
			   ( port_in_tbl[i] == KEY_FANST)||
			   ( port_in_tbl[i] == KEY_MIN)||
			   ( port_in_tbl[i] == KEY_RXIIN) ){
				port_indat_onff[i] = !( ucbit & wbit );					// ON/OFF Event Set
				queset( OPETCBNO, port_in_tbl[i], 1, &port_indat_onff[i] );
			}
		}
	}

	// �h�A�J�Z���T��ԏ��񃊁[�h
	datcnt = 0;
	for( j = 0; j < 20; j++ ){										// Max 20�ƍ�
		door_scan = CP_DOOR_OPEN_SEN;
		wait2us( 50L );												// 100us wait
		if( door_scan == CP_DOOR_OPEN_SEN ){						// Same?(Y)
			datcnt++;
		}else{
			datcnt = 0;
		}
		if( datcnt >= KEY_INIT_RD ){								// 100us 5��ƍ����ē���?(Y)
			break;
		}															// 100us20��NG�͍ŏI�ް��L��
	}
	doorDat = door_scan;
	door_data[0] = door_scan;
	door_data[1] = door_scan;
	door_data[2] = door_scan;
	if (doorDat) {
		door_indat_onff = 1;
	} else {
		door_indat_onff = 0;
	}
	queset( OPETCBNO, KEY_DOOR1, 1, &door_indat_onff );
	f_port_scan_OK = 1;												// ���荞�݃X�L��������
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �����߰ď���ذ�ށi�G���[�E�A���[���ɏ�Ԃ��Z�b�g���邽�߁j             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : door_int_read                                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  t.hashimoto                                            |*/
/*| Date         :  2013-03-04                                             |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
void	door_int_read( void )
{
	unsigned char	j;
	unsigned char datcnt;											// Same Data Count
	unsigned char door_scan;

	// �h�A�J�Z���T��ԏ��񃊁[�h
	datcnt = 0;
	for( j = 0; j < 20; j++ ){										// Max 20�ƍ�
		door_scan = CP_DOOR_OPEN_SEN;
		wait2us( 50L );												// 100us wait
		if( door_scan == CP_DOOR_OPEN_SEN ){						// Same?(Y)
			datcnt++;
		}else{
			datcnt = 0;
		}
		if( datcnt >= KEY_INIT_RD ){								// 100us 5��ƍ����ē���?(Y)
			break;
		}															// 100us20��NG�͍ŏI�ް��L��
	}
	doorDat = door_scan;
}

/*[]----------------------------------------------------------------------[]*/
/*| �g��I/O�߰ēǏo��                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : direct_port_red()                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : �߰ď��(Low Active)                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-01-23                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
unsigned short	ex_port_red( void )
{
	unsigned short dat;

	dat = EXIO_CS4_EXIO;
	return dat;
}

/*[]----------------------------------------------------------------------[]*/
/*| �g��I/O�߰ē��͏���(5ms�Ԋu)                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SCAN_INP_Exec( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Argument     : Tpu1TGIA���荞�݂��纰ق���܂�                         |*/
/*|                �{�����͊��荞�ݏ������纰ق����ׁA                   |*/
/*|                IO�𒼐ڐ��䂵�������Ԃ�Z�k���܂�                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-01-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
/* SCAN LINE1                           SCAN LINE2                          */
/*       RL0 Ӱ�ސݒ�SW0                      RL0 ���g�p                    */
/*       RL1 Ӱ�ސݒ�SW1                      RL1 ���g�p                    */
/*       RL2 Ӱ�ސݒ�SW2                      RL2 FAN��~���x���o           */
/*       RL3 Ӱ�ސݒ�SW3                      RL3 FAN�쓮�J�n���x���o       */
/*       RL4 �ݒ�SW_CAN�I�[�ݒ�               RL4 �ݒ跰���                */
/*       RL5 �ݒ�SW_�\��1                     RL5 �ޱ��޷���Ԏg�p          */
/*       RL6 �ݒ�SW_�\��2                     RL6 RMX-1��ėp����         */
/*       RL7 �ݒ�SW_�\��3                     RL7 RXI-1��ėp����         */
/*[]----------------------------------------------------------------------[]*/
void	SCAN_INP_Exec( void )
{
	unsigned short	wbit;											// �߰� Ͻ��pܰ��ޯ̧
	unsigned short	ucbit;											// �ω��߰� ܰ��ޯ̧
	unsigned char	i;
	if( f_port_scan_OK == 0){
		return;														// ���荞�݃X�L�����֎~���
	}
	/* �g��I/O�|�[�g��3��ǂ݂������Ȃ�Ώ���,�قȂ�ꍇ�͏I��	*/
	port_data[2] = port_data[1];
	port_data[1] = port_data[0];
	port_data[0] = ex_port_red();									// Input

	if( (port_data[0] == port_data[1]) && (port_data[1] == port_data[2]) ){
		// 3��A����v
		if( ( ucbit = (unsigned short)( port_in_dat.WORD ^ port_data[0] ) ) != 0 ){	// �ǂ�����BIT�ω�����?
			for( wbit = 0x0001, i = 0; i < PortInTblMax; wbit <<= 1, i++ ){
				if( ucbit & wbit ){									// �ω�����?
					if( port_in_tbl[i] != 0 ){
						port_indat_onff[i] = !( port_data[0] & wbit );			// ON/OFF Event Set
						queset( OPETCBNO, port_in_tbl[i], 1, &port_indat_onff[i] );
					}
				}
			}
			port_in_dat.WORD	= port_data[0];
		}
	}

	/* �h�A�J�Z���T��Ԃ�3��ǂ݂������Ȃ�Ώ���,�قȂ�ꍇ�͏I��	*/
	door_data[2] = door_data[1];
	door_data[1] = door_data[0];
	door_data[0] = CP_DOOR_OPEN_SEN;
	if( (door_data[0] == door_data[1]) && (door_data[1] == door_data[2]) ){
		// 3��A����v
		if(doorDat != door_data[0]){
			if (door_data[0]) {
				door_indat_onff = 1;
			} else {
				door_indat_onff = 0;
			}
			queset( OPETCBNO, KEY_DOOR1, 1, &door_indat_onff );
		}
		doorDat = door_data[0];
	}

	/* �h�A�m�u�Z���T��Ԃ�3��ǂ݂������Ȃ�Ώ���,�قȂ�ꍇ�͏I��	*/
	doornobu_data[2] = doornobu_data[1];
	doornobu_data[1] = doornobu_data[0];
	doornobu_data[0] = ExIOSignalrd(INSIG_DOOR);
	if( (doornobu_data[0] == doornobu_data[1]) && (doornobu_data[1] == doornobu_data[2]) ){
		// 3��A����v
		if (doornobu_data[0]) {
			doornobuDat = 0;
		} else {
			doornobuDat = 1;
		}
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �޻ް Err                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BUZERR( void )                                          |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nakayama, modified by Hara                            |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	BUZERR( void )
{
}

/*[]----------------------------------------------------------------------[]*/
/*| �޻ް Pi                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BUZPI( void )                                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nakayama, modified by Hara                            |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	BUZPI( void )
{
// MH810100(S) K.Onodera 2019/12/02 �Ԕԃ`�P�b�g���X�i�A�i�E���X�Ή��j
//	unsigned char	CtlCode;
//	unsigned char	LEDPattan;
//	unsigned char	volume;
//
//	volume = (ushort)prm_get( COM_PRM,S_PAY,29,1,2 );
//	CtlCode = CAN_CTRL_PI1 + volume;								// �u�U�[���i�s�b��j
//	LEDPattan = 0;													// LED OFF
//	can_snd_data3(CtlCode, LEDPattan);								// CAN�R���g���[�����M
// MH810100(E) K.Onodera 2019/12/02 �Ԕԃ`�P�b�g���X�i�A�i�E���X�Ή��j
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
	// �޻ް���v��
	lcdbm_beep_start(0);			// �޻ް���v��	��� = 0=�߯���1=���ߨ���2=�����ߨ��
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
}

/*[]----------------------------------------------------------------------[]*/
/*| �޻ް PiPi                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BUZPIPI( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nakayama, modified by Hara                            |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	BUZPIPI( void )
{
// MH810100(S) K.Onodera 2019/12/02 �Ԕԃ`�P�b�g���X�i�A�i�E���X�Ή��j
//	unsigned char	CtlCode;
//	unsigned char	LEDPattan;
//	unsigned char	volume;
//
//	volume = (ushort)prm_get( COM_PRM,S_PAY,29,1,2 );
//	CtlCode = CAN_CTRL_PI2 + volume;								// �u�U�[���i�s�s�b��j
//	LEDPattan = 0;													// LED OFF
//	can_snd_data3(CtlCode, LEDPattan);								// CAN�R���g���[�����M
// MH810100(E) K.Onodera 2019/12/02 �Ԕԃ`�P�b�g���X�i�A�i�E���X�Ή��j
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
	// �޻ް���v��
	lcdbm_beep_start(1);			// �޻ް���v��	��� = 0=�߯���1=���ߨ���2=�����ߨ��
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
}

/*[]----------------------------------------------------------------------[]*/
/*| �޻ް PiPiPi                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BUZPIPIPI( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nakayama, modified by Hara                            |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	BUZPIPIPI( void )
{
// MH810100(S) K.Onodera 2019/12/02 �Ԕԃ`�P�b�g���X�i�A�i�E���X�Ή��j
//	unsigned char	CtlCode;
//	unsigned char	LEDPattan;
//	unsigned char	volume;
//
//	volume = (ushort)prm_get( COM_PRM,S_PAY,29,1,2 );
//	CtlCode = CAN_CTRL_PI3 + volume;								// �u�U�[���i�s�s�s�b��j
//	LEDPattan = 0;													// LED OFF
//	can_snd_data3(CtlCode, LEDPattan);								// CAN�R���g���[�����M
// MH810100(E) K.Onodera 2019/12/02 �Ԕԃ`�P�b�g���X�i�A�i�E���X�Ή��j
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
	// �޻ް���v��
	lcdbm_beep_start(2);			// �޻ް���v��	��� = 0=�߯���1=���ߨ���2=�����ߨ��
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
}

/*[]----------------------------------------------------------------------[]*/
/*| �o���߰ď�����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ptout_Init( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ptout_Init( void )
{
	ExIOSignalwt(EXPORT_RXI_OUT3, SIG_OFF);		// RXI-1�ėp�o���߰�3 H(b�ړ_)
	SD_LCD = 0;														// �o�b�N���C�g�_����ԏ�����
}

/*[]----------------------------------------------------------------------[]*/
/*| �����߰ĂŎ��W����װя�񏉊���                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|	�ޱ�x��C����ذ�ް�E���C�U���ݻ �̱װя��𖢔��������ɏ���������B   |*/
/*|	����ɂ��PowerON�����甭�����Ă���װт����o����B					   |*/
/*|	�{�֐��� module_wait() ���{���Call���邱�ƁB						   |*/
/*|	�װт��莞��ҰقɂĒʒm���AGetMessage()�űװє��肷��B				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2006/03/20                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	PortAlm_Init( void )
{
	unsigned long	ist;			// ���݂̊�����t���

	ist = _di2();
	_ei2( ist );
}

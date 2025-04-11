/*[]----------------------------------------------------------------------[]*/
/*| ���o�͐M������                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-02-01                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"

/*[]----------------------------------------------------------------------[]*/
/*| 1shot�o�͐M������                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OutSignal_1shotCtrl( SigNo )                            |*/
/*| PARAMETER    : SigNo  : RYB���߰Ĕԍ��i0�`15�F�o�͐M��1=0,�M��16=15�j  |*/
/*|				   OnTime : �M��ON���ԁi1�`65535�F�ݒ�l�j				   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	OutSignalCtrl() ����Call�����֐��B								   |*/
/*|	���؂���̐M�������OutSignalCtrl()���g�p���邱�ƁB					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2005-12-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	void	Out1shotSig_Entry( uchar SigNo, ushort OnTime )
{
	_di();
	OpeSig_1shotInfo[SigNo] = OnTime;					// ON����save
	f_OpeSig_1shot = 1;									// 1shot����
	_ei();
}

/*[]----------------------------------------------------------------------[]*/
/*| 1shot�o�͐M������ 20ms�C���^�[�o���N�������i�����ݏ�������Call�����j |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OutSignal_1shotCtrl( SigNo )                            |*/
/*| PARAMETER    : SigNo  : RYB���߰Ĕԍ��i0�`15�F�o�͐M��1=0,�M��16=15�j  |*/
/*|				   OnTime : �M��ON���ԁi1�`65535�F�ݒ�l�j				   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	OutSignalCtrl() ����Call�����֐��B								   |*/
/*|	���؂���̐M�������OutSignalCtrl()���g�p���邱�ƁB					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2005-12-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Out1shotSig_Interval( void )
{
	uchar	i;
	uchar	f_On=0;										// 1=1shot�p���o�͒��M������
	unsigned char   port_bit;

	if( 1 != f_OpeSig_1shot )
		return;

	for( i=0; i<OutPortMax; ++i ){						// �o���߰Đ���
		if( 0 != OpeSig_1shotInfo[i] ){					// 1shot ON��
			--OpeSig_1shotInfo[i];
			if( 0 == OpeSig_1shotInfo[i] ){				// timeout
				port_bit = 14 - i;						// CS4(bit8�`bit14)�ɏo��
				ExIOSignalwt( port_bit, SIG_OFF );		// �g��I/O�߰ďo�͐M��OFF
			}
			else										// ON�p����
				f_On = 1;
		}
	}
	f_OpeSig_1shot = f_On;
}

/*[]----------------------------------------------------------------------[]*/
/*| �o�͐M������                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OutSignalCtrl( Kind, OnOff )                            |*/
/*| PARAMETER    : Kind  : �o�͐M���̎��                                  |*/
/*|                OnOff : 0=OFF�C1=ON�C2=ON-OFF(�ݼ���)                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OutSignalCtrl( uchar Kind, uchar OnOff )
{
	unsigned char	i;
	ushort	DelayTime;
	unsigned char   port_bit;

	for( i = 0; i < OutPortMax; i++ ){
		if( CPrmSS[S_OTP][(i*2)+1] == Kind ){
			port_bit = 14 - i;									// CS4(bit8�`bit14)�ɏo��
			ExIOSignalwt( port_bit, OnOff );					// �g��I/O�߰ďo��
			if( OnOff == SIG_ONOFF ){								// �ݼ���
				switch( Kind ){
					case SIG_OUTCNT1:								// ���Z�����M��1(�o�ɐM��1)
						DelayTime = (ushort)(CPrmSS[S_OTP][(i*2)+2]/20L+1L);
						Out1shotSig_Entry( i , DelayTime );
						break;
					case SIG_OUTCNT2:								// ���Z�����M��2(�o�ɐM��2)
						DelayTime = (ushort)(CPrmSS[S_OTP][(i*2)+2]/20L+1L);
						Out1shotSig_Entry( i , DelayTime );
						break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �g���߰ĐM���o��                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ExIOSignalwt( SigNo, OnOff )                            |*/
/*| PARAMETER    : SigNo : 0�`15=�߰Ă̎��                                |*/
/*|                OnOff : 0=OFF�C1=ON                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-01-17                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	ExIOSignalwt( uchar SigNo, uchar OnOff )
{
	const ushort	msk[16] = {0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000};

	if( 15 >= SigNo ){
		if (SigNo == EXPORT_RXI_OUT3) {	// �o�͐M��3(b�ړ_)
			if (OnOff) {				// 1=ON�C2=ON-OFF(�ݼ���)
				OnOff = 0;				// ON��L�o�͂����邽�ߔ��]
			} else {					// 0=OFF
				OnOff = 1;				// OFF��H�o�͂����邽�ߔ��]
			}
		}
		if( OnOff ){
			SD_EXIOPORT |= msk[SigNo];
		}else{
			SD_EXIOPORT &= (ushort)(~msk[SigNo]);
		}
		EXIO_CS4_EXIO = SD_EXIOPORT;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �g���߰Ă̏�ԓǏo��                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ExIOSignalrd( SigNo )                                   |*/
/*| PARAMETER    : SigNo : 0�`15=�߰Ă̎��                                |*/
/*| RETURN VALUE : ret   : 0=OFF�C1=ON                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-01-17                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	ExIOSignalrd( uchar SigNo )
{
	unsigned short rDat;
	ushort			ret = 0;
	const ushort	msk[16] = {0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000};

	rDat = EXIO_CS4_EXIO;
	if( 15 >= SigNo ){
		//SigNo�Ŏw�肵���|�[�g��ON/OFF�𔻒肷��
		if( 0 == ( msk[SigNo] & rDat ))
		{
			ret =  1;				//SigNo�Ŏw�肵���|�[�g��LOW
		}
		else
		{
			ret = 0;				//SigNo�Ŏw�肵���|�[�g��HIGH
		}
	} else {
		ret = 0;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���[�^���[�X�C�b�`�̏�Ԍ��o
//[]----------------------------------------------------------------------[]
///	@return			���[�^���[�X�C�b�`�̏��(0�`9)
///	@author			m.onouchi
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/20<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char read_rotsw(void)
{
	union {
		unsigned char	byte;
		struct {
			unsigned char	reserve:4;
			unsigned char	b3:1;
			unsigned char	b2:1;
			unsigned char	b1:1;
			unsigned char	b0:1;
		} bit;
	} rotsw;

	rotsw.byte = ~EXIO_CS4_EXIO;	// �_�����](0:OFF, 1:ON)
	rotsw.bit.reserve = 0;

	return (rotsw.byte);
}

/*[]----------------------------------------------------------------------[]*/
/*| ���͐M������                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : InSignalCtrl( Msg, OnOff )                              |*/
/*| PARAMETER    : Msg   : ү����                                          |*/
/*|                						                                   |*/
/*| RETURN VALUE : OnOff : 0=OFF�C1=ON                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	InSignalCtrl( ushort Kind )
{

	uchar	on_off = 0;

	//���ʃp�����[�^�̓��͐M���ݒ�1�E2�ɓo�^���Ă���ꍇ
	if( CPrmSS[S_INP][1] == Kind )
	{
	 	on_off = ExIOSignalrd(15);							/* RXI-1��ėp���̓|�[�g�ǂݏo��			*/

		/* �����c�Ƃ������͋����x�Ƃ̏ꍇ */
		switch ( Kind )										/*											*/
		{													/*											*/
			case INSIG_OPOPEN:								/* �����c��									*/
				OPECTL.opncls_eigyo = on_off;				/*  �����c�ƐM��ON/OFF�ݒ�					*/
				queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
				break;
			case INSIG_OPCLOSE:								/* �����x��									*/
				OPECTL.opncls_kyugyo = on_off;				/*  �����x�ƐM��ON/OFF�ݒ�					*/
				queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
				break;
			default:
				break;
		}
	}
	return on_off;
}


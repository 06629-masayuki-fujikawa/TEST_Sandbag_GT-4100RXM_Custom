/*[]----------------------------------------------------------------------[]*/
/*| 入出力信号制御                                                         |*/
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
/*| 1shot出力信号制御                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OutSignal_1shotCtrl( SigNo )                            |*/
/*| PARAMETER    : SigNo  : RYB内ﾎﾟｰﾄ番号（0～15：出力信号1=0,信号16=15）  |*/
/*|				   OnTime : 信号ON時間（1～65535：設定値）				   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	OutSignalCtrl() からCallされる関数。								   |*/
/*|	ｱﾌﾟﾘからの信号制御はOutSignalCtrl()を使用すること。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2005-12-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	void	Out1shotSig_Entry( uchar SigNo, ushort OnTime )
{
	_di();
	OpeSig_1shotInfo[SigNo] = OnTime;					// ON時間save
	f_OpeSig_1shot = 1;									// 1shotあり
	_ei();
}

/*[]----------------------------------------------------------------------[]*/
/*| 1shot出力信号制御 20msインターバル起動処理（割込み処理からCallされる） |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OutSignal_1shotCtrl( SigNo )                            |*/
/*| PARAMETER    : SigNo  : RYB内ﾎﾟｰﾄ番号（0～15：出力信号1=0,信号16=15）  |*/
/*|				   OnTime : 信号ON時間（1～65535：設定値）				   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	OutSignalCtrl() からCallされる関数。								   |*/
/*|	ｱﾌﾟﾘからの信号制御はOutSignalCtrl()を使用すること。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2005-12-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Out1shotSig_Interval( void )
{
	uchar	i;
	uchar	f_On=0;										// 1=1shot継続出力中信号あり
	unsigned char   port_bit;

	if( 1 != f_OpeSig_1shot )
		return;

	for( i=0; i<OutPortMax; ++i ){						// 出力ﾎﾟｰﾄ数分
		if( 0 != OpeSig_1shotInfo[i] ){					// 1shot ON中
			--OpeSig_1shotInfo[i];
			if( 0 == OpeSig_1shotInfo[i] ){				// timeout
				port_bit = 14 - i;						// CS4(bit8～bit14)に出力
				ExIOSignalwt( port_bit, SIG_OFF );		// 拡張I/Oﾎﾟｰﾄ出力信号OFF
			}
			else										// ON継続中
				f_On = 1;
		}
	}
	f_OpeSig_1shot = f_On;
}

/*[]----------------------------------------------------------------------[]*/
/*| 出力信号制御                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OutSignalCtrl( Kind, OnOff )                            |*/
/*| PARAMETER    : Kind  : 出力信号の種類                                  |*/
/*|                OnOff : 0=OFF，1=ON，2=ON-OFF(ﾜﾝｼｮｯﾄ)                   |*/
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
			port_bit = 14 - i;									// CS4(bit8～bit14)に出力
			ExIOSignalwt( port_bit, OnOff );					// 拡張I/Oﾎﾟｰﾄ出力
			if( OnOff == SIG_ONOFF ){								// ﾜﾝｼｮｯﾄ
				switch( Kind ){
					case SIG_OUTCNT1:								// 精算完了信号1(出庫信号1)
						DelayTime = (ushort)(CPrmSS[S_OTP][(i*2)+2]/20L+1L);
						Out1shotSig_Entry( i , DelayTime );
						break;
					case SIG_OUTCNT2:								// 精算完了信号2(出庫信号2)
						DelayTime = (ushort)(CPrmSS[S_OTP][(i*2)+2]/20L+1L);
						Out1shotSig_Entry( i , DelayTime );
						break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 拡張ﾎﾟｰﾄ信号出力                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ExIOSignalwt( SigNo, OnOff )                            |*/
/*| PARAMETER    : SigNo : 0～15=ﾎﾟｰﾄの種類                                |*/
/*|                OnOff : 0=OFF，1=ON                                     |*/
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
		if (SigNo == EXPORT_RXI_OUT3) {	// 出力信号3(b接点)
			if (OnOff) {				// 1=ON，2=ON-OFF(ﾜﾝｼｮｯﾄ)
				OnOff = 0;				// ONでL出力させるため反転
			} else {					// 0=OFF
				OnOff = 1;				// OFFでH出力させるため反転
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
/*| 拡張ﾎﾟｰﾄの状態読出し                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ExIOSignalrd( SigNo )                                   |*/
/*| PARAMETER    : SigNo : 0～15=ﾎﾟｰﾄの種類                                |*/
/*| RETURN VALUE : ret   : 0=OFF，1=ON                                     |*/
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
		//SigNoで指定したポートのON/OFFを判定する
		if( 0 == ( msk[SigNo] & rDat ))
		{
			ret =  1;				//SigNoで指定したポートがLOW
		}
		else
		{
			ret = 0;				//SigNoで指定したポートがHIGH
		}
	} else {
		ret = 0;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ロータリースイッチの状態検出
//[]----------------------------------------------------------------------[]
///	@return			ロータリースイッチの状態(0～9)
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

	rotsw.byte = ~EXIO_CS4_EXIO;	// 論理反転(0:OFF, 1:ON)
	rotsw.bit.reserve = 0;

	return (rotsw.byte);
}

/*[]----------------------------------------------------------------------[]*/
/*| 入力信号制御                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : InSignalCtrl( Msg, OnOff )                              |*/
/*| PARAMETER    : Msg   : ﾒｯｾｰｼﾞ                                          |*/
/*|                						                                   |*/
/*| RETURN VALUE : OnOff : 0=OFF，1=ON                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	InSignalCtrl( ushort Kind )
{

	uchar	on_off = 0;

	//共通パラメータの入力信号設定1・2に登録している場合
	if( CPrmSS[S_INP][1] == Kind )
	{
	 	on_off = ExIOSignalrd(15);							/* RXI-1基板汎用入力ポート読み出し			*/

		/* 強制営業もしくは強制休業の場合 */
		switch ( Kind )										/*											*/
		{													/*											*/
			case INSIG_OPOPEN:								/* 強制営業									*/
				OPECTL.opncls_eigyo = on_off;				/*  強制営業信号ON/OFF設定					*/
				queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
				break;
			case INSIG_OPCLOSE:								/* 強制休業									*/
				OPECTL.opncls_kyugyo = on_off;				/*  強制休業信号ON/OFF設定					*/
				queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
				break;
			default:
				break;
		}
	}
	return on_off;
}


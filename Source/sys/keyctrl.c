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
		KEY_RTSW1,	KEY_RTSW2,	KEY_RTSW3,	KEY_RTSW4,				// 拡張I/O入力ｲﾍﾞﾝﾄ
		KEY_DIPSW1,	KEY_DIPSW2,	KEY_DIPSW3,	KEY_DIPSW4,
		0,			0,			KEY_FANET,	KEY_FANST,
		KEY_MODECHG,KEY_DOORNOBU,KEY_MIN,	KEY_RXIIN,
};

/*[]----------------------------------------------------------------------[]*/
/*| 入力ﾎﾟｰﾄ初期ﾘｰﾄﾞ                                                       |*/
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
	unsigned short	wbit;											// ﾎﾟｰﾄ ﾏｽｸ用ﾜｰｸﾊﾞｯﾌｧ
	unsigned short	ucbit;											// 変化ﾎﾟｰﾄ ﾜｰｸﾊﾞｯﾌｧ
	unsigned char	i,j;
	unsigned char datcnt;											// Same Data Count
	unsigned char door_scan;

	// 入力信号状態初回リード
	datcnt = 0;
	for( j = 0; j < 20; j++ ){										// Max 20照合
		ucbit = ex_port_red();
		wait2us( 50L );												// 100us wait
		if( ucbit == ex_port_red() ){								// Same?(Y)
			datcnt++;
		}else{
			datcnt = 0;
		}
		if( datcnt >= KEY_INIT_RD ){								// 100us 5回照合して同一?(Y)
			break;
		}															// 100us20回NGは最終ﾃﾞｰﾀ有効
	}
	port_in_dat.WORD = ucbit;
	port_data[0] = ucbit;
	port_data[1] = ucbit;
	port_data[2] = ucbit;
	// 起動時の入力ポートの状態をイベントとして通知する(但し必要なイベントのみ)
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

	// ドア開センサ状態初回リード
	datcnt = 0;
	for( j = 0; j < 20; j++ ){										// Max 20照合
		door_scan = CP_DOOR_OPEN_SEN;
		wait2us( 50L );												// 100us wait
		if( door_scan == CP_DOOR_OPEN_SEN ){						// Same?(Y)
			datcnt++;
		}else{
			datcnt = 0;
		}
		if( datcnt >= KEY_INIT_RD ){								// 100us 5回照合して同一?(Y)
			break;
		}															// 100us20回NGは最終ﾃﾞｰﾀ有効
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
	f_port_scan_OK = 1;												// 割り込みスキャン許可
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 入力ﾎﾟｰﾄ初期ﾘｰﾄﾞ（エラー・アラームに状態をセットするため）             |*/
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

	// ドア開センサ状態初回リード
	datcnt = 0;
	for( j = 0; j < 20; j++ ){										// Max 20照合
		door_scan = CP_DOOR_OPEN_SEN;
		wait2us( 50L );												// 100us wait
		if( door_scan == CP_DOOR_OPEN_SEN ){						// Same?(Y)
			datcnt++;
		}else{
			datcnt = 0;
		}
		if( datcnt >= KEY_INIT_RD ){								// 100us 5回照合して同一?(Y)
			break;
		}															// 100us20回NGは最終ﾃﾞｰﾀ有効
	}
	doorDat = door_scan;
}

/*[]----------------------------------------------------------------------[]*/
/*| 拡張I/Oﾎﾟｰﾄ読出し                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : direct_port_red()                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ﾎﾟｰﾄ状態(Low Active)                                    |*/
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
/*| 拡張I/Oﾎﾟｰﾄ入力処理(5ms間隔)                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SCAN_INP_Exec( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Argument     : Tpu1TGIA割り込みからｺｰﾙされます                         |*/
/*|                本処理は割り込み処理からｺｰﾙされる為、                   |*/
/*|                IOを直接制御し処理時間を短縮します                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-01-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
/* SCAN LINE1                           SCAN LINE2                          */
/*       RL0 ﾓｰﾄﾞ設定SW0                      RL0 未使用                    */
/*       RL1 ﾓｰﾄﾞ設定SW1                      RL1 未使用                    */
/*       RL2 ﾓｰﾄﾞ設定SW2                      RL2 FAN停止温度検出           */
/*       RL3 ﾓｰﾄﾞ設定SW3                      RL3 FAN駆動開始温度検出       */
/*       RL4 設定SW_CAN終端設定               RL4 設定ｷｰ状態                */
/*       RL5 設定SW_予備1                     RL5 ﾄﾞｱﾉﾌﾞｷｰ状態使用          */
/*       RL6 設定SW_予備2                     RL6 RMX-1基板汎用入力         */
/*       RL7 設定SW_予備3                     RL7 RXI-1基板汎用入力         */
/*[]----------------------------------------------------------------------[]*/
void	SCAN_INP_Exec( void )
{
	unsigned short	wbit;											// ﾎﾟｰﾄ ﾏｽｸ用ﾜｰｸﾊﾞｯﾌｧ
	unsigned short	ucbit;											// 変化ﾎﾟｰﾄ ﾜｰｸﾊﾞｯﾌｧ
	unsigned char	i;
	if( f_port_scan_OK == 0){
		return;														// 割り込みスキャン禁止状態
	}
	/* 拡張I/Oポートを3回読みし同じならば処理,異なる場合は終了	*/
	port_data[2] = port_data[1];
	port_data[1] = port_data[0];
	port_data[0] = ex_port_red();									// Input

	if( (port_data[0] == port_data[1]) && (port_data[1] == port_data[2]) ){
		// 3回連続一致
		if( ( ucbit = (unsigned short)( port_in_dat.WORD ^ port_data[0] ) ) != 0 ){	// どこかのBIT変化あり?
			for( wbit = 0x0001, i = 0; i < PortInTblMax; wbit <<= 1, i++ ){
				if( ucbit & wbit ){									// 変化あり?
					if( port_in_tbl[i] != 0 ){
						port_indat_onff[i] = !( port_data[0] & wbit );			// ON/OFF Event Set
						queset( OPETCBNO, port_in_tbl[i], 1, &port_indat_onff[i] );
					}
				}
			}
			port_in_dat.WORD	= port_data[0];
		}
	}

	/* ドア開センサ状態を3回読みし同じならば処理,異なる場合は終了	*/
	door_data[2] = door_data[1];
	door_data[1] = door_data[0];
	door_data[0] = CP_DOOR_OPEN_SEN;
	if( (door_data[0] == door_data[1]) && (door_data[1] == door_data[2]) ){
		// 3回連続一致
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

	/* ドアノブセンサ状態を3回読みし同じならば処理,異なる場合は終了	*/
	doornobu_data[2] = doornobu_data[1];
	doornobu_data[1] = doornobu_data[0];
	doornobu_data[0] = ExIOSignalrd(INSIG_DOOR);
	if( (doornobu_data[0] == doornobu_data[1]) && (doornobu_data[1] == doornobu_data[2]) ){
		// 3回連続一致
		if (doornobu_data[0]) {
			doornobuDat = 0;
		} else {
			doornobuDat = 1;
		}
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾞｻﾞｰ Err                                                              |*/
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
/*| ﾌﾞｻﾞｰ Pi                                                               |*/
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
// MH810100(S) K.Onodera 2019/12/02 車番チケットレス（アナウンス対応）
//	unsigned char	CtlCode;
//	unsigned char	LEDPattan;
//	unsigned char	volume;
//
//	volume = (ushort)prm_get( COM_PRM,S_PAY,29,1,2 );
//	CtlCode = CAN_CTRL_PI1 + volume;								// ブザー音（ピッ♪）
//	LEDPattan = 0;													// LED OFF
//	can_snd_data3(CtlCode, LEDPattan);								// CANコントロール送信
// MH810100(E) K.Onodera 2019/12/02 車番チケットレス（アナウンス対応）
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
	// ﾌﾞｻﾞｰ鳴動要求
	lcdbm_beep_start(0);			// ﾌﾞｻﾞｰ鳴動要求	種類 = 0=ﾋﾟｯ♪､1=ﾋﾟﾋﾟｨ♪､2=ﾋﾟﾋﾟﾋﾟｨ♪
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾞｻﾞｰ PiPi                                                             |*/
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
// MH810100(S) K.Onodera 2019/12/02 車番チケットレス（アナウンス対応）
//	unsigned char	CtlCode;
//	unsigned char	LEDPattan;
//	unsigned char	volume;
//
//	volume = (ushort)prm_get( COM_PRM,S_PAY,29,1,2 );
//	CtlCode = CAN_CTRL_PI2 + volume;								// ブザー音（ピピッ♪）
//	LEDPattan = 0;													// LED OFF
//	can_snd_data3(CtlCode, LEDPattan);								// CANコントロール送信
// MH810100(E) K.Onodera 2019/12/02 車番チケットレス（アナウンス対応）
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
	// ﾌﾞｻﾞｰ鳴動要求
	lcdbm_beep_start(1);			// ﾌﾞｻﾞｰ鳴動要求	種類 = 0=ﾋﾟｯ♪､1=ﾋﾟﾋﾟｨ♪､2=ﾋﾟﾋﾟﾋﾟｨ♪
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾞｻﾞｰ PiPiPi                                                           |*/
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
// MH810100(S) K.Onodera 2019/12/02 車番チケットレス（アナウンス対応）
//	unsigned char	CtlCode;
//	unsigned char	LEDPattan;
//	unsigned char	volume;
//
//	volume = (ushort)prm_get( COM_PRM,S_PAY,29,1,2 );
//	CtlCode = CAN_CTRL_PI3 + volume;								// ブザー音（ピピピッ♪）
//	LEDPattan = 0;													// LED OFF
//	can_snd_data3(CtlCode, LEDPattan);								// CANコントロール送信
// MH810100(E) K.Onodera 2019/12/02 車番チケットレス（アナウンス対応）
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
	// ﾌﾞｻﾞｰ鳴動要求
	lcdbm_beep_start(2);			// ﾌﾞｻﾞｰ鳴動要求	種類 = 0=ﾋﾟｯ♪､1=ﾋﾟﾋﾟｨ♪､2=ﾋﾟﾋﾟﾋﾟｨ♪
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
}

/*[]----------------------------------------------------------------------[]*/
/*| 出力ﾎﾟｰﾄ初期化                                                         |*/
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
	ExIOSignalwt(EXPORT_RXI_OUT3, SIG_OFF);		// RXI-1汎用出力ﾎﾟｰﾄ3 H(b接点)
	SD_LCD = 0;														// バックライト点灯状態初期化
}

/*[]----------------------------------------------------------------------[]*/
/*| 入力ﾎﾟｰﾄで収集するｱﾗｰﾑ情報初期化                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|	ﾄﾞｱ警報，紙幣ﾘｰﾀﾞｰ脱落，振動ｾﾝｻ のｱﾗｰﾑ情報を未発生方向に初期化する。   |*/
/*|	これによりPowerON時から発生しているｱﾗｰﾑを検出する。					   |*/
/*|	本関数は module_wait() 実施後にCallすること。						   |*/
/*|	ｱﾗｰﾑあり時はﾒｰﾙにて通知し、GetMessage()でｱﾗｰﾑ判定する。				   |*/
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
	unsigned long	ist;			// 現在の割込受付状態

	ist = _di2();
	_ei2( ist );
}
